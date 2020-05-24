///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  io.h - Abstract input/output helpers
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

#ifndef _inc_io_h
#define _inc_io_h


#ifdef ARDUINO
#include <Arduino.h>

#define FPTR(x) reinterpret_cast<const __FlashStringHelper *>(x)

#else

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// TODO: Needs tidying up and fixing 
class Print
{
public:
    void print(const int8_t& v) const       { printf("%d", v );}
    void print(const int8_t& v, const int& base) { printf("%x", v ); }
    void print(const int16_t& v) const      { printf("%d", v );}
    void print(const int32_t& v) const      { printf("%d", v );}
    void print(const uint8_t& v) const      { printf("%u", v );}
    void print(const uint16_t& v) const     { printf("%u", v );}
    void print(const uint32_t& v) const     { printf("%u", v );}
    void print(const long unsigned int& v) const { printf("%lu", v );}
    void print(char c) const                { printf("%c", c); }
    void print(const char * val) const      { printf("%s", val); }

    void println(void) const                { printf("\n"); }
};


#define HEX 16

#endif

// Prevent streaming lib from including legacy Arduino header in the event we're not building for Arduino
#define NO_INC_WPROGRAM
#include <Streaming.h>

class _IO : public Print
{
public:
    _IO() {}

    void begin(unsigned long baud);
    void flush(void);
    bool available(void);
    int read(void);

    //void info(char* fmt, ...);

    //void println(char * p);
    //void println(const __FlashStringHelper* p);


    // Mandatory function for single char output
    size_t write(uint8_t b);

private:
    int         m_last_char;
};

extern _IO IO;





#endif

