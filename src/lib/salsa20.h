///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  salsa20.h - Header file for Salsa20 implementation
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

#ifndef _inc_salsa20_h
#define _inc_salsa20_h

#define SALSA20_ENTRY_COUNT         (16)

class Salsa20Block
{ 
public:
    union {
        uint8_t     as_bytes[4];
        uint16_t    as_word16[2];
        uint32_t    as_word32;
    } entry[SALSA20_ENTRY_COUNT];

    void inline Add(const Salsa20Block& other)
    {
        for(uint8_t i=0;i<SALSA20_ENTRY_COUNT;i++)
            entry[i].as_word32 += other.entry[i].as_word32;
    }
    void inline Xor(const Salsa20Block&a)
    {
        for(uint8_t i=0;i<SALSA20_ENTRY_COUNT;i++)
            entry[i].as_word32 ^= a.entry[i].as_word32;
    }
    void inline Xor(const Salsa20Block&a, const Salsa20Block& b)
    {
        for(uint8_t i=0;i<SALSA20_ENTRY_COUNT;i++)
            entry[i].as_word32 = a.entry[i].as_word32 ^ b.entry[i].as_word32;
    }
    void inline QuarterRound(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        entry[b].as_word32 ^= RL(entry[a].as_word32 + entry[d].as_word32, 7);
        entry[c].as_word32 ^= RL(entry[b].as_word32 + entry[a].as_word32, 9);
        entry[d].as_word32 ^= RL(entry[c].as_word32 + entry[b].as_word32, 13);
        entry[a].as_word32 ^= RL(entry[d].as_word32 + entry[c].as_word32, 18);
    }
    void inline ColumnRounds(void)
    {
        QuarterRound(0, 4, 8, 12);
        QuarterRound(5, 9, 13, 1);
        QuarterRound(10, 14, 2, 6);
        QuarterRound(15, 3, 7, 11);
    }
    void inline RowRows(void)
    {
        QuarterRound(0, 1, 2, 3);
        QuarterRound(5, 6, 7, 4);
        QuarterRound(10, 11, 8, 9);
        QuarterRound(15, 12, 13, 14);
    }
    void inline DoubleRound(void)
    {
        ColumnRounds();
        RowRows();
    }
};

#endif
