///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Master Password Implementation
//
//  References
//      https://en.wikipedia.org/wiki/PBKDF2
//      https://tools.ietf.org/html/rfc2898#page-9
//      https://tools.ietf.org/html/rfc8018#page-11
//      https://tools.ietf.org/html/rfc7914#page-12
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

#include "mpw.h"

#include <stdio.h>

#ifdef ENABLE_MPW_EXTENSIONS        
const char * MPW_Template_Vast[] = {
    "anoxxxxxxxxxxxxxxxxxxxxxxxxxxx",
    "axxxxxxxxxxxxxxxxxxxxxxxxxxxno"
};
#endif

const char * MPW_Template_Maximum[] = {
    "anoxxxxxxxxxxxxxxxxx",
    "axxxxxxxxxxxxxxxxxno"
};

const char * MPW_Template_Long[] = {
    "CvcvnoCvcvCvcv",
	"CvcvCvcvnoCvcv",
	"CvcvCvcvCvcvno",
	"CvccnoCvcvCvcv",
	"CvccCvcvnoCvcv",
	"CvccCvcvCvcvno",
	"CvcvnoCvccCvcv",
	"CvcvCvccnoCvcv",
	"CvcvCvccCvcvno",
	"CvcvnoCvcvCvcc",
	"CvcvCvcvnoCvcc",
	"CvcvCvcvCvccno",
	"CvccnoCvccCvcv",
	"CvccCvccnoCvcv",
	"CvccCvccCvcvno",
	"CvcvnoCvccCvcc",
	"CvcvCvccnoCvcc",
	"CvcvCvccCvccno",
	"CvccnoCvcvCvcc",
	"CvccCvcvnoCvcc",
	"CvccCvcvCvccno"   
};

const char * MPW_Template_Medium[] = {
    "CvcnoCvc",
    "CvcCvcno"
};

const char * MPW_Template_Basic[] = {
    "aaanaaan",
    "aannaaan",
    "aaannaaa"
};

const char * MPW_Template_Short[] = {
    "Cvcn"
};

const char * MPW_Template_Pin[] = {
    "nnnn"
};

#ifdef ENABLE_MPW_EXTENSIONS        
const char * MPW_Template_Pin_Six[] = {
    "nnnnnn"
};
#endif

const char * MPW_Template_Name[] = {
    "cvccvcvcv"
};

const char * MPW_Template_Phrase[] = {
    "cvcc cvc cvccvcv cvc",
    "cvc cvccvcvcv cvcv",
    "cv cvccv cvc cvcvccv"
};

#ifdef ENABLE_MPW_EXTENSIONS        
const char * MPW_Template_BigPhrase[] = {
    "cvcc cvc cvccvcv cvc cvccvcv cvcc",
    "cvcc cvcc cvc cvccvcvcv cvcv cvcc",
    "cv cvccv cvc cvcvccv cvccvcvcv cvc cvc",
};
#endif

const char * MPW_Template_Class_Characters(char c)
{
    switch(c)
    {
        case 'V':   return "AEIOU";
        case 'C':   return "BCDFGHJKLMNPQRSTVWXYZ";
        case 'v':   return "aeiou";
        case 'c':   return "bcdfghjklmnpqrstvwxyz";
        case 'A':   return "AEIOUBCDFGHJKLMNPQRSTVWXYZ";
        case 'a':   return "AEIOUaeiouBCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz";
        case 'n':   return "0123456789";
        case 'o':   return "@&%?,=[]_:-+*$#!'^~;()/.";
        case 'x':   return "AEIOUaeiouBCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz0123456789!@#$%^&*()";
        case ' ':   return " ";
        default:
            IO << F("Unhandled template character class `") << c << F("`, exiting ...") << endl;
            empw_exit(EXITCODE_LOGIC_FAULT);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MPW& MPW::login(const char *name, const char *password, progress_func progress)
{
    // Gather some reused data
    uint32_t name_len = strlen(name);
    uint32_t seed_buffer_len = sizeof(MPW_Scope_Authentication) - 1 + sizeof(uint32_t) + name_len;
    uint8_t *seed_buffer = (uint8_t*)malloc(seed_buffer_len);
    if ( seed_buffer == 0 )
    {
        IO << F("Failed to allocate seed_buffer for MPW login") << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }
    // Fill the seed buffer
    memcpy( seed_buffer, MPW_Scope_Authentication, sizeof(MPW_Scope_Authentication)-1);
    push_int( &seed_buffer[sizeof(MPW_Scope_Authentication)-1], name_len );
    memcpy( &seed_buffer[sizeof(MPW_Scope_Authentication) - 1 + sizeof(uint32_t) ], name, name_len );
    // Perform the scrypt algorithm with this seed buffer and the password, output goes to the master key buffer in the class
    //scrypt_hash( reinterpret_cast<const uint8_t *>(password), strlen(password), seed_buffer, seed_buffer_len );
    m_master_key = m_master_key_holder.hash(reinterpret_cast<const uint8_t *>(password), strlen(password), seed_buffer, seed_buffer_len, progress);
    // Clean up please
    free(seed_buffer);
    // Allow fluent syntax
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void MPW::logout(void)
{
    if ( m_site_password != 0 )
    {
        free(m_site_password);
        m_site_password = 0;
    }

    if ( m_site_key_holder != 0 )
    {
        delete m_site_key_holder;
        m_site_key_holder  = 0;
        m_site_key = 0;
    }

    m_master_key_holder.Reset();
    m_master_key = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
MPW& MPW::site( const char * sitename, uint32_t site_counter )
{
    uint32_t sitename_len = strlen(sitename);
    uint32_t seed_buffer_len = sizeof(MPW_Scope_Authentication) - 1 + sizeof(uint32_t) + sitename_len + sizeof(uint32_t);
    uint8_t *seed_buffer = (uint8_t*)malloc(seed_buffer_len);
    if ( seed_buffer == 0 )
    {
        IO << F("Failed to allocate seed_buffer for MPW site") << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }

    // Fill seed buffer
    memcpy( seed_buffer, MPW_Scope_Authentication, sizeof(MPW_Scope_Authentication) - 1 );
    push_int( &seed_buffer[sizeof(MPW_Scope_Authentication) - 1], sitename_len );
    memcpy( &seed_buffer[sizeof(MPW_Scope_Authentication) - 1 + sizeof(uint32_t)], sitename, sitename_len );
    push_int( &seed_buffer[sizeof(MPW_Scope_Authentication) - 1 + sizeof(uint32_t) + sitename_len], site_counter );

    m_site_key_holder = new HMAC<SHA256>(m_master_key, MASTER_KEY_LEN, seed_buffer, seed_buffer_len);
    if ( m_site_key_holder == 0 )
    {
        IO << F("Failed to create HMAC for site") << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }
    m_site_key = m_site_key_holder->digest();
    // Allow fluent syntax
    return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const char * MPW::get_password_template( MPM_Password_Type type )
{
    switch(type)
    {
        case Maximum:   return MPW_Template_Maximum[m_site_key[0] % countof(MPW_Template_Maximum)];
        case Long:      return MPW_Template_Long[m_site_key[0] % countof(MPW_Template_Long)];
        case Medium:    return MPW_Template_Medium[m_site_key[0] % countof(MPW_Template_Medium)];
        case Basic:     return MPW_Template_Basic[m_site_key[0] % countof(MPW_Template_Basic)];
        case Short:     return MPW_Template_Short[m_site_key[0] % countof(MPW_Template_Short)];
        case PIN:       return MPW_Template_Pin[m_site_key[0] % countof(MPW_Template_Pin)];
        case Name:      return MPW_Template_Name[m_site_key[0] % countof(MPW_Template_Name)];
        case Phrase:    return MPW_Template_Phrase[m_site_key[0] % countof(MPW_Template_Phrase)];
#ifdef ENABLE_MPW_EXTENSIONS        
        case Vast:      return MPW_Template_Vast[m_site_key[0] % countof(MPW_Template_Vast)];
        case PIN_Six:   return MPW_Template_Pin_Six[m_site_key[0] % countof(MPW_Template_Pin_Six)];
        case BigPhrase: return MPW_Template_BigPhrase[m_site_key[0] % countof(MPW_Template_BigPhrase)];
#endif
        default:
            IO << F("Unhandled password template type (") << type << F("), exit") << endl;
            empw_exit(EXITCODE_LOGIC_FAULT);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
const char * MPW::generate( MPM_Password_Type type )
{
    const char * pwd_template = get_password_template(type);

    // Alloc password buffer
    uint8_t pwd_len = strlen(pwd_template);
    if ( m_site_password != 0 )
        free(m_site_password);
    m_site_password = (char *)malloc(pwd_len+1);
    if ( m_site_password == 0 )
    {
        IO << F("Failed to allocate password buffer") << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }
    memset(m_site_password,0,pwd_len+1);
    // Fill it up!
    for(uint8_t i=0;i<pwd_len;i++)
    {
        const char * password_chars = MPW_Template_Class_Characters(pwd_template[i]);
        m_site_password[i] = password_chars[m_site_key[i+1] % strlen( password_chars )];
    }

    return m_site_password;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t MPW::get_login_token(void) const
{
    return (uint32_t)((size_t)this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
