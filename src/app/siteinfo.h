///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  userinfo.h - Header file for userinfo persistence control block
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

#ifndef _inc_siteinfo_h
#define _inc_siteinfo_h

#include "persistence.h"
#include "../lib/mpw.h"
#include "../lib/str_ptr.h"
#include <vector>

#define SITEINFO_HAS_USERNAME       0x01
#define SITEINFO_HAS_RECOVERY       0x02
#define SITEINFO_HAS_ANSWERS        0x04
#define SITEINFO_REQUIRES_LOGIN     0x08

class siteinfo
{
public:
    siteinfo(str_ptr sitename, uint8_t counter, MPM_Password_Type style) : m_sitename(sitename), m_counter(counter), m_style(style), m_options(0) {}
    siteinfo(const siteinfo& other) : m_sitename(other.m_sitename), m_counter(other.m_counter), m_style(other.m_style), m_options(other.m_options), m_answer_words(other.m_answer_words) {}

    bool                    is_site(const char * s) const   { return m_sitename == s; }
    const char *            get_sitename(void) const        { return m_sitename; }
    uint8_t                 get_counter(void) const         { return m_counter; }
    void                    set_counter(uint8_t c)          { m_counter = c; }
    MPM_Password_Type       get_style(void) const           { return m_style; }
    uint8_t                 get_options(void) const         { return m_options; }

    static siteinfo         load(persistence& p);
    void                    save(persistence& p) const;

private:
    str_ptr                 m_sitename;
    uint8_t                 m_counter;
    MPM_Password_Type       m_style;
    uint8_t                 m_options;
    std::vector<str_ptr>    m_answer_words;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
inline siteinfo siteinfo::load(persistence& p)
{
    auto site = p.readstr();
    auto count = p.read8();
    auto style = p.read8();
    return siteinfo(site, count, (MPM_Password_Type)style);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void siteinfo::save(persistence& p) const
{
    p.writestr(m_sitename);
    p.write8(m_counter);
    p.write8(m_style);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif