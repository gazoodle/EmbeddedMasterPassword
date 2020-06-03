//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Persistence class implementation
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

#include "persistence.h"
#include "../lib/io.h"

#ifdef ARDUINO
#include <EEPROM.h>
#else
#include <cstdio>
#define DATA_FILE       "./cli.dat"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
persistence::persistence(void) : m_index(0)
{
#ifdef ARDUINO
    IO << "EEPROM end is " << E2END << endl;
#else
    memset(EEPROM, UNINITIALIZED_EEPROM, sizeof(EEPROM));
    FILE *fp = fopen( DATA_FILE, "rb");
    if ( fp != 0 )
    {
        fread( EEPROM, sizeof(EEPROM), 1, fp);
        fclose(fp);
    }
#endif
    m_dirty = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
persistence::~persistence(void)
{
    if ( !m_dirty )
        return;
#ifndef ARDUINO
    FILE *fp = fopen( DATA_FILE, "wb");
    if ( fp != 0 )
    {
        fwrite( EEPROM, sizeof(EEPROM), 1, fp );
        fclose(fp);
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void persistence::erase(void)
{
    while( has_space() )
    {
        uint8_t v = read8();
        if ( v != UNINITIALIZED_EEPROM )
        {
            m_index--;
            write8(UNINITIALIZED_EEPROM);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool persistence::has_space(void)
{
    #ifdef ARDUINO
    return ( m_index < E2END );
    #else
    return ( m_index < sizeof(EEPROM) );
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t persistence::read8(void)
{
    return EEPROM[m_index++];
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void    persistence::write8(uint8_t v)
{
    if ( !has_space() )
    {
        IO << "Cannot write any more to the persistent storage, there is no space left" << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }
    EEPROM[m_index++] = v;
    m_dirty = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
str_ptr     persistence::readstr(void)
{
    uint8_t len = read8();
    str_ptr retval(len);
    for(uint8_t i=0;i<len;i++)
        retval.setat(i, read8());
    return retval;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void        persistence::writestr(const str_ptr& s)
{
    uint8_t     len = s.length();
    const char *p = s;
    write8(len);
    for(uint8_t i=0;i<len;i++)
        write8(*p++);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
