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

#ifndef _inc_userinfo_h
#define _inc_userinfo_h

#include "../lib/mpw.h"
#include "../lib/str_ptr.h"
#include "persistence.h"

class userinfo
{
private:
    userinfo() {}
    userinfo(const userinfo& other) {}
public:
    userinfo(const char * username) : m_username(username) {}
    userinfo(const str_ptr& username) : m_username(username) {}
    ~userinfo(){}

    bool                is_user(const char * u) const { return m_username == u; }
    const char *        get_user_name(void)     const { return m_username; }
    MPW&                get_mpw(void)                 { return m_mpw; }

    static userinfo *   load(persistence& p);
    void                save(persistence& p) const;

private:
    MPW     m_mpw;
    str_ptr m_username;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
inline userinfo * userinfo::load(persistence& p)
{
    return new userinfo(p.readstr());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void userinfo::save(persistence& p) const
{
    p.writestr(m_username);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif