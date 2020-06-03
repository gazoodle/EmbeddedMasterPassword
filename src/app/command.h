///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  command.h - Header file for EMPW command processor
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

#ifndef _inc_command_h
#define _inc_command_h

#include "userinfo.h"
#include "../version.h"
#include "persistence.h"
#include <algorithm>


#define MAX_PERSISTENT_USERS        (9)
#define MAX_COMMAND_LINE_LENGTH     (180)
#define USER_NOT_FOUND              (255)

class command
{
public:
    command(void);
    ~command(void);

    void setup(void);
    void loop(void);
    bool is_running(void);
    void handle_command(char * pcommand);

private:
    void banner(void);
    void reset(void);
    bool dispatch(char * pcommand);

    // Command functions
    void handle_help(void);
    void handle_reset(void);
    
    // User commands
    void handle_login(char * pdata);
    void handle_logout(char * pdata);
    void handle_switch_user(char * pdata);
    void handle_list_users(void);
    void handle_add_user(char * pdata);
    void handle_remove_user(char * pdata);

    void handle_site(char * pdata);

    void handle_generate(char * pdata);

private:
    uint8_t find_user(const char * uname, bool include_dynamic) const;
    uint8_t find_user(uint32_t token) const;
    void    load(void);
    void    save(void);

private:
    userinfo*                       m_users[MAX_PERSISTENT_USERS+1];
    userinfo*                       m_current_user;

    char                            m_command_buffer[MAX_COMMAND_LINE_LENGTH];
    uint8_t                         m_command_index;
#ifndef ARDUINO
    bool                            m_is_running;
#endif
};




#endif

