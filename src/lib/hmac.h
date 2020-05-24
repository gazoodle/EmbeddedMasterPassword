///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  hmac.h - Header file for HMAC implementation
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

#ifndef _inc_hmac_h
#define _inc_hmac_h

// Padding values chosen to have a large Hamming distance (https://en.wikipedia.org/wiki/Hamming_distance)
#define HMAC_OUTER_PADDING  (uint8_t)0x5C
#define HMAC_INNER_PADDING  (uint8_t)0x36

template <class HASH_ALGO>
class HMAC
{
public:
    HMAC(const uint8_t *key, uint32_t key_size);
    HMAC(const uint8_t *key, uint32_t key_size, const uint8_t *message, uint32_t message_size );
    // Convenience constructors
    HMAC(const char *key) : HMAC(reinterpret_cast<const uint8_t *>(key), strlen(key)) { }
    HMAC(const char *key, const char *message) : HMAC(reinterpret_cast<const uint8_t *>(key), strlen(key), reinterpret_cast<const uint8_t *>(message), strlen(message)) { }
    ~HMAC(void) { memset(m_key, 0, sizeof(m_key)); }

    static const uint8_t    BLOCK_SIZE_BYTES = HASH_ALGO::BLOCK_SIZE_BYTES;
    static const uint8_t    HASH_SIZE_BYTES = HASH_ALGO::HASH_SIZE_BYTES;

public:
    void reset(void);
    void enqueue(uint8_t byte) { m_hash_algorithm.enqueue(byte); }
    void enqueue(const uint8_t *bytes, uint32_t count) { m_hash_algorithm.enqueue(bytes, count); }
    void enqueue_be(uint32_t val) { m_hash_algorithm.enqueue_be(val); };
    const uint8_t * digest(void);

private:
    uint8_t     m_key[ HASH_ALGO::BLOCK_SIZE_BYTES ];
    HASH_ALGO   m_hash_algorithm;
};

#include "hmac-impl.h"

#endif