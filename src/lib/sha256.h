///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  sha256.h - Header file for SHA256 implementation
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

#ifndef _inc_sha256_h
#define _inc_sha256_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <io.h>

#define SHA256_MESSAGE_SCHEDULE_SIZE        64              // Compression algorithm buffer
#define SHA256_MESSAGE_SCHEDULE_SIZE_MASK   (SHA256_MESSAGE_SCHEDULE_SIZE-1)

#define RR(v,n)             (((v)<<(32-(n))) | ((v)>>(n)))
#define RL(v,n)             (((v)<<(n)) | ((v)>>(32-(n))))
#define SWAP_ENDS(u32)      ((( (u32) & 0xff000000 ) >> 24 ) | (((u32) & 0x00ff0000 ) >> 8 ) | (((u32) & 0x0000ff00 ) << 8 ) | (((u32) & 0x000000ff ) << 24 ))
#define countof(x)          ( (sizeof(x)) / (sizeof(x[0])) )

class SHA256
{
public:
    SHA256(void) { reset(); }
    SHA256(const uint8_t *message, uint32_t message_size );
    // Convenience constructors
    SHA256(const char *message) : SHA256(reinterpret_cast<const uint8_t *>(message), strlen(message)) { }
    ~SHA256(void) { memset( m_message_schedule_array, 0, sizeof(m_message_schedule_array) ); memset( m_hash_buffer, 0, sizeof(m_hash_buffer) ); }

    static const uint8_t    BLOCK_SIZE_BYTES = 64;
    static const uint8_t    HASH_SIZE_BYTES = 32;

public:
    void reset(void);
    void enqueue(uint8_t byte);
    void enqueue(const uint8_t *bytes, uint32_t count);
    void enqueue_be(uint32_t val);

    const uint8_t * digest(void);

private:
    void hash_chunk(void);
    void finalize(void);

private:
    uint32_t    m_message_schedule_array[ SHA256_MESSAGE_SCHEDULE_SIZE ];
    uint32_t    m_message_size;
    uint8_t     m_hash_buffer[ HASH_SIZE_BYTES ];
};

#include "sha256-impl.h"

#endif