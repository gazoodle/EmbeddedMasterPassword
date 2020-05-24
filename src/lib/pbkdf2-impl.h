///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PBKDF2 implementation
//
//  References
//      https://en.wikipedia.org/wiki/PBKDF2
//      https://tools.ietf.org/html/rfc2898#page-9
//      https://tools.ietf.org/html/rfc8018#page-11
//      https://tools.ietf.org/html/rfc7914#page-12
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
template<class HASH_ALGO, uint16_t dkLen>
PBKDF2<HASH_ALGO, dkLen>::PBKDF2(const uint8_t *password, uint32_t password_len, const uint8_t *salt, uint32_t salt_len, uint32_t c )
{
    // Initialize the hash algorithm with the password
    HASH_ALGO   hash_algorithm(password, password_len);

    uint32_t    remainder(dkLen);
    uint8_t     work[HASH_ALGO::HASH_SIZE_BYTES];
    uint8_t     U1[HASH_ALGO::HASH_SIZE_BYTES];
    uint8_t*    output = m_key_buffer;

    // While there are bytes to be generated
    for( uint32_t block=1; remainder > 0 ; block++)
    {
        hash_algorithm.enqueue( salt, salt_len );
        hash_algorithm.enqueue_be( block );
        memcpy(U1, hash_algorithm.digest(), sizeof(U1));
        memcpy(work, U1, sizeof(U1));

        for( uint32_t i = 1; i < c; i++ )
        {
            hash_algorithm.reset();
            hash_algorithm.enqueue( U1, sizeof(U1));
            memcpy(U1, hash_algorithm.digest(), sizeof(U1));
            for( uint8_t j=0; j < sizeof(U1); j++)
                work[j] ^= U1[j];
        }

        uint32_t use_len = ( remainder < HASH_ALGO::HASH_SIZE_BYTES ) ? remainder : HASH_ALGO::HASH_SIZE_BYTES;
        memcpy( output, work, use_len );

        remainder -= use_len;
        output += use_len;

        hash_algorithm.reset();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
