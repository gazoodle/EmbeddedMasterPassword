///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  version.h - Header file for Embedded Master Password versions
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

/*
    HISTORY
    =======

    0.0.x       Apr 2020        -   Proof of concept code
    0.1.x       May 2020        -   Initial development


    Using Semantic versioning https://semver.org/
*/

#ifndef _inc_version_h
#define _inc_version_h


#define EMPW_MAJOR_VERSION      0
#define EMPW_MINOR_VERSION      1
#define EMPW_PATCH              5


#define _VERSION_STRINGIZE(x)    #x
#define VERSION_STRINGIZE(x)    _VERSION_STRINGIZE(x)

#define EMPW_VERSION_STRING VERSION_STRINGIZE(EMPW_MAJOR_VERSION) "." VERSION_STRINGIZE(EMPW_MINOR_VERSION) "." VERSION_STRINGIZE(EMPW_PATCH)

#endif