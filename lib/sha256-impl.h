///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SHA-256 implementation
//
//  References
//      https://tools.ietf.org/html/rfc6234
//      https://en.wikipedia.org/wiki/SHA-2
//      https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf
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
inline SHA256::SHA256(const uint8_t *message, uint32_t message_size ) : SHA256()
{
    enqueue(message, message_size);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::reset(void)
{
    // Rewind everything so we can start a new digest
    m_message_size = 0;

    // Initialize hash values
    // First 32bits on the fractional parts of the square roots on the first 8 primes 2..19
    uint32_t * hash_buffer = reinterpret_cast<uint32_t *>(m_hash_buffer);
    hash_buffer[0] = 0x6a09e667;
    hash_buffer[1] = 0xbb67ae85;
    hash_buffer[2] = 0x3c6ef372;
    hash_buffer[3] = 0xa54ff53a;
    hash_buffer[4] = 0x510e527f;
    hash_buffer[5] = 0x9b05688c;
    hash_buffer[6] = 0x1f83d9ab;
    hash_buffer[7] = 0x5be0cd19;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::enqueue(uint8_t byte)
{
    uint8_t * schedule_buffer = reinterpret_cast<uint8_t *>(m_message_schedule_array);
    // Collect message bytes in the schedule array. The XOR 3 is a clever
    // little trick to perform the endian swap required to put the bytes
    // in the correct place for the algoritm
    schedule_buffer[ (m_message_size & SHA256_MESSAGE_SCHEDULE_SIZE_MASK) ^ 3 ] = byte;
    // Increment counters
    m_message_size++;
    // If the chunk is full, process it
    if ( (m_message_size & SHA256_MESSAGE_SCHEDULE_SIZE_MASK) == 0 )
        hash_chunk();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::enqueue(const uint8_t *bytes, uint32_t count)
{
    while(count--)
        enqueue(*bytes++);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::enqueue_be(uint32_t val)
{
    enqueue(val>>24);
    enqueue(val>>16);
    enqueue(val>>8);
    enqueue(val);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::finalize(void)
{
    // Save the current message size
    uint32_t current_message_size = m_message_size;

    enqueue(0x80);
    while( ( m_message_size & SHA256_MESSAGE_SCHEDULE_SIZE_MASK ) != 56 )
        enqueue(0x00);
    // Algorithm specifies storage of a 64bit count of the bits in the message without the padding.
    // This code pushes the bytes of a multiplication by 8 on the schedule array
    enqueue(0x00);
    enqueue(0x00);
    enqueue(0x00);
    enqueue(current_message_size>>29);
    enqueue(current_message_size>>21);
    enqueue(current_message_size>>13);
    enqueue(current_message_size>>5);
    enqueue(current_message_size<<3);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SHA256::hash_chunk(void)
{
    uint32_t * hash_buffer = reinterpret_cast<uint32_t *>(m_hash_buffer);

    // Collect working variables
    uint32_t a=hash_buffer[0];
    uint32_t b=hash_buffer[1];
    uint32_t c=hash_buffer[2];
    uint32_t d=hash_buffer[3];
    uint32_t e=hash_buffer[4];
    uint32_t f=hash_buffer[5];
    uint32_t g=hash_buffer[6];
    uint32_t h=hash_buffer[7];

    // Initialize array of round constants:
    // First 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311
    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    for( uint8_t idx=0; idx<SHA256_MESSAGE_SCHEDULE_SIZE; idx++ )
    {
        if ( idx >= BLOCK_SIZE_BYTES/4 )
        {
            // Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array:                
            uint32_t s0 = RR(m_message_schedule_array[idx-15], 7) ^ RR(m_message_schedule_array[idx-15], 18) ^ (m_message_schedule_array[idx-15]>>3);
            uint32_t s1 = RR(m_message_schedule_array[idx-2], 17) ^ RR(m_message_schedule_array[idx-2], 19) ^ (m_message_schedule_array[idx-2]>>10);
            m_message_schedule_array[idx] = m_message_schedule_array[idx-16] + s0 + m_message_schedule_array[idx-7] + s1;
        }

        uint32_t S1 = RR(e, 6) ^ RR(e, 11) ^ RR(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + k[idx] + m_message_schedule_array[idx];
        uint32_t S0 = RR(a, 2) ^ RR(a, 13) ^ RR(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g; 
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    // Add the compressed chunk to the current hash value:
    hash_buffer[0] += a;
    hash_buffer[1] += b;
    hash_buffer[2] += c;
    hash_buffer[3] += d;
    hash_buffer[4] += e;
    hash_buffer[5] += f;
    hash_buffer[6] += g;
    hash_buffer[7] += h;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline const uint8_t * SHA256::digest(void)
{
    finalize();

    // Finally convert the hash buffer to big-endian
    uint32_t * hash_buffer = reinterpret_cast<uint32_t *>(m_hash_buffer);
    for(uint8_t i=0; i<8; i++)
        hash_buffer[i] = SWAP_ENDS(hash_buffer[i]);

    return m_hash_buffer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////