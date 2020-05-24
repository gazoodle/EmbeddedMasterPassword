///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  pbkdf2.h - Header file for PBKDF2 implementation
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

#ifndef _inc_pbkdf2_h
#define _inc_pbkdf2_h

#include <stdint.h>

template<class HASH_ALGO, uint16_t dkLen>
class PBKDF2
{
public:
    PBKDF2(const uint8_t *password, uint32_t password_len, const uint8_t *salt, uint32_t salt_len, uint32_t c );
    // Convenience constructor
    PBKDF2(const char *password, const char *salt, uint32_t c ) : PBKDF2( reinterpret_cast<const uint8_t *>(password), strlen(password), reinterpret_cast<const uint8_t *>(salt), strlen(salt), c) {}
    ~PBKDF2(void) { memset(m_key_buffer, 0, sizeof(m_key_buffer) );}
    uint8_t * result(void) { return m_key_buffer; }

private:
    uint8_t m_key_buffer[dkLen];
};

#include "pbkdf2-impl.h"

#endif