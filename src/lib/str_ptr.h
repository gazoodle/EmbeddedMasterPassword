///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  str_ptr.h - Header file for string pointer class
//
//      A small reference counted char * manager. This works on the principle that
//      most strings that need to be shared in a program very rarely have large numbers
//      of references. Certainly for most cases this is less than 255 uses, so we simply 
//      overallocate strings with an extra byte at the start of the string and use that
//      as a reference counter. This means that the ownership of memory cleanup is the
//      responsibility of the str_ptr holder that decrements the usage count to zero.
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

#ifndef _inc_str_ptr_h
#define _inc_str_ptr_h

#include <stdint.h>
#include <stdlib.h>
#include "io.h"

class str_ptr
{
public:
    str_ptr(void) : m_ptr(0) {}
    str_ptr(uint8_t len) : m_ptr(0) { alloc(len); }
    str_ptr(const char * p) : m_ptr(0) { copy(p); }
    str_ptr(const str_ptr& other) : m_ptr(other.m_ptr) { increment(); }
    ~str_ptr(void);

public:
    // Operators
    bool operator == (const char * p) const { if (!has_string()) return false; return strcmp(p, m_ptr+1) == 0; }
    str_ptr& operator = (const str_ptr& p)  { m_ptr = p.m_ptr; increment(); return *this; }
    str_ptr& operator = (const char * p)    { copy(p); return *this; }
    operator const char *() const           { if (has_string()) return m_ptr+1; return 0;}

    // Methods
    bool    has_string(void) const          { return m_ptr != 0; }
    uint8_t length(void) const              { return strlen(m_ptr+1); }
    void    setat(uint8_t i, char c)        { m_ptr[i+1] = c; }

#ifndef TEST_SUITE
private:
#endif
    uint8_t refcount(void) const            { if (has_string()) return m_ptr[0]; return 0; }
    uint8_t increment(void) const           { return ++m_ptr[0]; }
    uint8_t decrement(void) const           { return --m_ptr[0]; }
    void    alloc(uint8_t len);
    void    copy(const char * p);

#ifndef TEST_SUITE
private:
#endif
    char * m_ptr;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
inline str_ptr::~str_ptr(void)
{
    if ( m_ptr == 0 )
        return;
    if ( decrement() > 0 )
        return;
    // Debugging point to help track string leaks during development
    //IO << "Free string [" << m_ptr+1 << "]" << endl;
    free(m_ptr);
    m_ptr = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void inline str_ptr::alloc(uint8_t len)
{
    // Make sure it's empty
    if (m_ptr != 0)
    {
        IO << "Error. Already used string ptr [" << &m_ptr[1] << "](" << (int)m_ptr[0] << ")" << endl;
        empw_exit(EXITCODE_LOGIC_FAULT);
    }
    // Allocate space for the string, the reference count and the null terminator
    m_ptr = (char *)malloc(len+2);
    if ( m_ptr == 0 )
    {
        IO << "Failed to allocate space (" << len << ") for string " << endl;
        empw_exit(EXITCODE_NO_MEMORY);
    }
    // Make sure it's clear
    memset(m_ptr, 0, len+2);
    increment();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void inline str_ptr::copy(const char * p)
{
    alloc(strlen(p));
    strcpy(m_ptr+1, p);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif