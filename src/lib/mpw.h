///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  mpw.h - Header file for Master Password implementation
//
//  Implementation Copyright (C) 2020, Gazoodle (https://github.com/gazoodle)
//  Algorithm Copyright (C) 2011-2018, Maarten Billemont, Lyndir (https://masterpassword.app/)
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

#ifndef _inc_mpw_h
#define _inc_mpw_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "scrypt.h"

#define MASTER_KEY_LEN              64
#define SCRYPT_N                    32768
#define SCRYPT_R                    8
#define SCRYPT_P                    2

typedef enum {
    Maximum = 1,
    Long,
    Medium,
    Basic,
    Short,
    PIN,
    Name,
    Phrase,

#ifdef ENABLE_MPW_EXTENSIONS
    // From here, these are new patterns, not in the master version
    PIN_Six,
    Vast,
    BigPhrase,
#endif

    Raw = 0xffff

} MPM_Password_Type;

#define MPW_USERNAME_COUNTER    (1)
#define MPW_USERNAME_TYPE       (Name)
#define MPW_RECOVERY_COUNTER    (1)
#define MPW_RECOVERY_TYPE       (Phrase)

class MPW
{
private:
    MPW(const MPW& other){}
public:
    MPW(void) : m_master_key(NULL), m_login_token(0), m_site_password(NULL) {}
    ~MPW(void) { logout(); }

    // User managment
    MPW&            login(const char *name, const char *password, progress_func progress);
    void            logout(void);
    bool            is_logged_in(void) const { return m_master_key != 0; }
    uint32_t        get_login_token(void) const;

    // Generate response
    const char *    generate( const char *site_name, uint32_t site_counter, MPM_Password_Type type, const char * context, const char * scope );

private:
    const char *    get_password_template( uint8_t c, MPM_Password_Type type );
    void            generate_login_token(void);

private:
    void push_int( uint8_t *buf, uint32_t val )
    {
        buf[0] = val>>24;
        buf[1] = val>>16;
        buf[2] = val>>8;
        buf[3] = val;
    }

private:
    scrypt<SCRYPT_N, SCRYPT_R, SCRYPT_P, MASTER_KEY_LEN>        m_master_key_holder;
    const uint8_t*                                              m_master_key;
    uint32_t                                                    m_login_token;
    char *                                                      m_site_password;
};


#define MPW_Namespace               "com.lyndir.masterpassword"
#define MPW_Scope_Authentication    MPW_Namespace
#define MPW_Scope_Identification    MPW_Scope_Authentication ".login"
#define MPW_Scope_Recovery          MPW_Scope_Authentication ".answer"
#define MPW_Scope_Token             MPW_Scope_Authentication ".token"

#endif