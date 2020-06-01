///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  platform.h - Header file for Embedded Master Password version control
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

#ifndef _inc_platform_h
#define _inc_platform_h

// Various exit codes to help diagnose problems
#define     EXITCODE_NO_MEMORY          -2          // Can't malloc, new or otherwise get space for something
#define     EXITCODE_LOGIC_FAULT        -3          // Logic inconsistence, assertion failure etc


#ifdef ARDUINO


// Embedded systems don't really have a concept of exit, so we dump the exit code to the IO stream 
// and then go into infinite loop as there's really nothing else we can do.
#define empw_exit(n)    { IO << F("Exit ") << n << endl; while(1); } 


#else /* !ARDUINO */

// Arduino has a macro F() which places string constants in flash memory reducing normal memory usage

#define F(s)    s

// And you also need aliases for some overloaded routines that deal with strings in flash memory



// Exit the program
#define empw_exit(n)    exit(n)


#endif

#endif