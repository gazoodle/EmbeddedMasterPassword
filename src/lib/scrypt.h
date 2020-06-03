///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  scrypt.h - Header file for scrypt implementation
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

#ifndef _inc_scrypt_h
#define _inc_scrypt_h

#include "sha256.h"
#include "hmac.h"
#include "pbkdf2.h"
#include "salsa20.h"
#include "scrypt-mixer.h"
#include <functional>

#define SCRYPT_YIELD_FREQUENCY      (64)

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
extern "C" uint8_t external_psram_size;
#endif

template<uint32_t N, uint32_t r, uint32_t p, uint32_t dkLen>
class scrypt
{
private:
    scrypt(const scrypt& other) {}
public:
    scrypt() : m_final(0) {}
    ~scrypt() { Reset(); }

    const uint8_t * hash( const uint8_t * passphrase, uint32_t passphrase_size, const uint8_t * salt, uint32_t salt_size, progress_func progress);
    // Convenience function
    const uint8_t * hash( const char * passphrase, const char * salt, progress_func progress)
    {
        return hash(reinterpret_cast<const uint8_t *>(passphrase), strlen(passphrase), reinterpret_cast<const uint8_t *>(salt), strlen(salt), progress);
    }

    void Reset(void);

private:
#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
    inline void GlobalMixer(Salsa20Block* block, progress_func progress, uint32_t global_size);
    void StackAndMallocMixer(Salsa20Block* block, progress_func progress);
#endif

private:
    PBKDF2<HMAC<SHA256>,dkLen>* m_final;
};

#include "scrypt-impl.h"

#endif