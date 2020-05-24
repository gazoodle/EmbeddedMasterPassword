///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HMAC implementation
//
//  References
//      https://en.wikipedia.org/wiki/HMAC
//      https://tools.ietf.org/html/rfc4231
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
template <class HASH_ALGO>
HMAC<HASH_ALGO>::HMAC(const uint8_t *key, uint32_t key_size)
{
    // Slightly redundant code in the case that the supplied 
    // key is larger than hash key buffer
    memset( m_key, 0, sizeof( m_key ));
    // Algorithm states that if the key is larger than the algorithm chunk size, it must be hashed
    // otherwise it is used verbatim with the remaining characters zero filled
    if ( key_size > HASH_ALGO::BLOCK_SIZE_BYTES )
    {
        m_hash_algorithm.enqueue(key, key_size);
        memcpy( m_key, m_hash_algorithm.digest(), HASH_ALGO::HASH_SIZE_BYTES );
    }
    else
    {
        memcpy( m_key, key, key_size );
    }
    // Reset HMAC ready for action
    reset();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class HASH_ALGO>
HMAC<HASH_ALGO>::HMAC(const uint8_t *key, uint32_t key_size, const uint8_t *message, uint32_t message_size) : HMAC(key, key_size)
{
    m_hash_algorithm.enqueue(message, message_size);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class HASH_ALGO>
void HMAC<HASH_ALGO>::reset(void)
{
    // Reset the hasher
    m_hash_algorithm.reset();
    // Initialize the hash algorithm using the inner padding
    for(uint8_t i=0;i<sizeof(m_key);i++)
        m_hash_algorithm.enqueue(m_key[i] ^ HMAC_INNER_PADDING );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class HASH_ALGO>
const uint8_t * HMAC<HASH_ALGO>::digest(void)
{
    // Stash the inner hash
    uint8_t innerHash[HASH_ALGO::HASH_SIZE_BYTES];
    memcpy( innerHash, m_hash_algorithm.digest(), HASH_ALGO::HASH_SIZE_BYTES );
    // Reset hasher for another round
    m_hash_algorithm.reset();
    // Put the padded outer key in
    for(uint8_t i=0;i<sizeof(m_key);i++)
        m_hash_algorithm.enqueue(m_key[i] ^ HMAC_OUTER_PADDING);
    // And add the inner hash
    m_hash_algorithm.enqueue(innerHash, HASH_ALGO::HASH_SIZE_BYTES );
    // Complete hash and return
    const uint8_t * ret = m_hash_algorithm.digest();
    return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
