///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SCRYPT implementation
//
//  References
//      https://tools.ietf.org/html/rfc7914
//      https://en.wikipedia.org/wiki/Scrypt
//
//  Copyright (C) 2020, Gazoodle (https://github.com/gazoodle)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
template<uint32_t N, uint32_t r, uint32_t p, uint32_t dkLen>
const uint8_t * scrypt<N,r,p,dkLen>::hash( const uint8_t * passphrase, uint32_t passphrase_size, const uint8_t * salt, uint32_t salt_size, progress_func progress )
{
    if (progress)(progress)(0);

    // Calculate some useful parameters to start with
    const uint32_t mix_size = p * 128 * r;

    // Initialize working area, generating expensive salt
    PBKDF2<HMAC<SHA256>,mix_size> initial(passphrase, passphrase_size, salt, salt_size, 1);
    uint8_t* second_salt = initial.result();

    // Mix the seed using the ROMix Algorithm
    Salsa20Block* block = reinterpret_cast<Salsa20Block*>(second_salt);

    #if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
    uint32_t global_size = external_psram_size * 1024 * 1024;
    if ( global_size > 0 )
    {
        // If someone has been kind enought to solder a PSRAM chip or two on the boards
        // we can use that to great effect
        scrypt_mixer<N,r,p,dkLen,0, 0> mixer((Salsa20Block*)(0x70000000), global_size);
        mixer.Mix(block, progress);
    }
    else
    {
        //  Emperical testing has shown these values work on Teensy 4.0 or 4.1 without external PSRAM
        #define ROMIX_SPARSE_V_MALLOC_MAX   (493568)
        #define ROMIX_SPARSE_V_STACK_MAX    (419840-(6*1024))
        scrypt_mixer<N,r,p,dkLen,ROMIX_SPARSE_V_STACK_MAX, ROMIX_SPARSE_V_MALLOC_MAX> mixer;
        mixer.Mix(block, progress);
    }
    #elif defined(ARDUINO_FEATHER_ESP32)
    scrypt_mixer<N,r,p,dkLen,0,131072> mixer;
    mixer.Mix(block, progress);
    #else
    // Generic version uses fully populated V array
    scrypt_mixer<N,r,p,dkLen,0,N*r*2*sizeof(Salsa20Block)> mixer;
    mixer.Mix(block, progress);
    #endif

    // Do final hash on the second salt
    Reset();
    m_final = new PBKDF2<HMAC<SHA256>,dkLen>(passphrase, passphrase_size, second_salt, mix_size, 1);

    if (progress)(progress)(100);
    return m_final->result();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<uint32_t N, uint32_t r, uint32_t p, uint32_t dkLen>
void scrypt<N,r,p,dkLen>::Reset(void)
{
    if ( m_final != 0 )
    {
        delete m_final;
        m_final = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
