//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  IO class implementation
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

#include "io.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Global implementation of IO object
_IO IO;
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void _IO::begin(unsigned long baud)
{
    #ifdef ARDUINO
    Serial.begin(baud);
    // Wait for Serial port to connect. Need only on native USB port
    while(!Serial)
        ;
    #else
    m_last_char = EOF;
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
size_t _IO::write(uint8_t b)
{
    #ifdef ARDUINO
    return Serial.write(b);
    #else
    return 0;
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void _IO::flush(void)
{
    #ifndef ARDUINO
    fflush(stdout);
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool _IO::available(void)
{
    #ifdef ARDUINO
    return Serial.available();
    #else
    if ( m_last_char != EOF)
        return true;
    m_last_char = getchar();
    return false;
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int _IO::read(void)
{
    #ifdef ARDUINO
    return Serial.read();
    #else
    while( m_last_char == EOF )
        m_last_char = getchar();
    int retval = m_last_char;
    m_last_char = EOF;
    return retval;
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
