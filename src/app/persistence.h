///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  persistence.h - Header file for reading & writing persistence data
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

#ifndef _inc_persistence_h
#define _inc_persistence_h

#include <stdint.h>
#include "../lib/str_ptr.h"

#ifndef ARDUINO
#define EEPROM_SIZE     1024
#endif
#define UNINITIALIZED_EEPROM        (0xff)


// Look at compression library
// Look at using SD card on Teensy 4.1

// Other

class persistence
{
public:
    persistence(void);
    ~persistence(void);

    void        save(void);
    void        load(void);

    uint8_t     read8(void);
    void        write8(uint8_t v);

    str_ptr     readstr(void);
    void        writestr(const str_ptr& s);

private:
    bool        m_dirty;
    uint16_t    m_index;
#ifndef ARDUINO
    uint8_t     EEPROM[EEPROM_SIZE];
#endif
};

#endif

