//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Command processor class implementation
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

#include "command.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
command::command(void)
{
    memset(m_users, 0, sizeof(m_users));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
command::~command(void)
{
    for(uint8_t i=0;i<countof(m_users);i++)
    {
        if ( m_users[i] != 0 )
        {
            delete m_users[i];
            m_users[i] = 0;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::setup(void)
{
#ifndef ARDUINO
    m_is_running = true;
#endif
    IO.begin(115200);
    banner();
    reset();
    load();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::loop(void)
{
    if (IO.available() == 0)
        return;

    m_command_buffer[m_command_index] = IO.read();
  
    if ( ( m_command_buffer[m_command_index] == '\n' ) || ( m_command_index == MAX_COMMAND_LINE_LENGTH-1 ))
    {
        m_command_buffer[m_command_index] = 0;
        handle_command(m_command_buffer);
        reset();
        return;
    }

    m_command_index++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::banner(void)
{
    IO << F("### Embedded Master Password v") << EMPW_VERSION_STRING << F(" ###") << endl;
    IO << F("Build date ") << __DATE__ << F(" ") << __TIME__ << endl;
    IO << "Running on "
#if defined(ARDUINO_TEENSY40)
    << F("Teensy 4.0")
#elif defined(ARDUINO_TEENSY41)
    << F("Teensy 4.1 (") << external_psram_size << F("MB External PSRAM)")
#elif defined(ARDUINO_FEATHER_ESP32)
    << F("Adafruit HUZZAH32 – ESP32")
#elif defined(CONSOLE)
    << F("Linux 64")
#else
    #error "Unsupported configuration"
#endif
    << endl;

    IO << F("This implementation, Copyright © 2020, Gazoodle (https://github.com/gazoodle)") << endl;
    IO << F("Algorithm, Copyright © 2011-2020, Maarten Billemont (https://masterpassword.app/)") << endl << endl;
    IO << F("For instructions use command `help`") << endl;
    IO << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::reset(void)
{
    // If the command processor is running as a cli program, and the user issues the
    // exit command, this test prevents the console from outputting a pointless prompt
    if ( !is_running() )
        return;
    m_command_index = 0;
#ifndef ARDUINO    
    IO << F("EMPW> ");
#endif
    IO.flush();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool command::is_running(void)
{
    #ifdef ARDUINO
    return true;
    #else
    return m_is_running;
    #endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  handle_command can take multiple commands on the same line, separated by the command
//  separator string (defaults to ";""). This allows clients that don't need the user/site
//  state to be performed in a single call, e.g.
//
//      user 12345; site example.com; generate long
//
//      - Switch to user with token 12345
//      - Select site example.com
//      - Generate a long password
//
void command::handle_command(char * pcommand)
{
    // While there are commands to process ...
    while( *pcommand != 0 )
    {
        char * start = pcommand;
        // Hunt for command separator
        while( *pcommand != 0 && *pcommand != ';' )
            pcommand++;
        // Zap it
        if ( *pcommand != 0 )
        {
            *pcommand++ = 0;
            while(*pcommand == ' ')
                pcommand++;
        }

        // Dispatch this command
        dispatch(start);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool command::dispatch(char * pcommand)
{
 #ifndef ARDUINO
    if ( strncmp( pcommand, "exit", 5) == 0 )
    {
        m_is_running = false;
        return false;
    } 
    else if ( strncmp( pcommand, "save", 5 ) == 0 )
    {
        save();
    }
    else
    #endif


    //
    //  Users
    //  =====
    if ( strncmp( pcommand, "login ", 6 ) == 0 )
        handle_login(pcommand+6);
    else if ( strncmp( pcommand, "logout ", 7 ) == 0 )
        handle_logout(pcommand+7);
    else if ( strncmp( pcommand, "user ", 5 ) == 0 )
        handle_switch_user(pcommand+5);
    else if ( strncmp( pcommand, "users", 6 ) == 0 )
        handle_list_users();
    else if ( strncmp( pcommand, "adduser ", 8) == 0 )
        handle_add_user(pcommand+8);
    else if ( strncmp( pcommand, "removeuser ", 11) == 0 )
        handle_remove_user(pcommand+11);




    else if ( strncmp( pcommand, "help", 5 ) == 0 )        // Includes \0 to avoid matching substrings
        handle_help();
    else if ( strncmp( pcommand, "reset", 6 ) == 0 )       // Includes \0 to avoid matching substrings
        handle_reset();
    else if ( strncmp( pcommand, "site ", 5 ) == 0)
        handle_site(pcommand+5);
    else if ( strncmp( pcommand, "generate ", 8 ) ==  0)
        handle_generate(pcommand+8);
    else
    {
        IO << "Unhandled command [" << pcommand << "]. Please see help below for more information " << endl;
        handle_help();
        return false;
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_help(void)
{
    IO
        << F("Users") << endl 
        << F("-----") << endl
        << F("login <user>, <password>       - Login <user> with <password>") << endl
        << F("logout <token>                 - Logout user <token>") << endl
        << F("user <token>                   - Switch to user <token>") << endl
        << F("users                          - List remembered users") << endl
        << F("adduser <user>                 - Add <user> to the remembered user list") << endl
        << F("removeuser <user>              - Remove <user> from the remembered user list") << endl
        << endl
        << endl
        << F("Sites") << endl
        << F("-----") << endl
        << F("site <site>[, <counter>]       - Generate site key for <site> with instance counter <counter>") << endl
        << F("sites                          - List remembered sites for current user") << endl
        << F("addsite <site>[, <counter>]    - Add/Update remembered site <site> with instance counter <counter>") << endl
        << F("removesite <site>              - Remove remebered site <site> for current user") << endl
        << F("removeall                      - Remove all sites for current user") << endl
        << endl
        << endl
        << F("Password") << endl
        << F("--------") << endl
        << F("generate [<style>]             - Generate password for current user & site in the style <style> specified") << endl
		<< F("                                 <style> is one of Maximum, Long (Default), Medium, Basic, Short,") << endl 
        << F("                                                   PIN, Name, Phrase") << endl
        << endl
        << endl
        << F("Maintenance") << endl
        << F("-----------") << endl
        << F("exit                           - Exit the EMPW program (only available on cli version)") << endl
        << F("reset                          - Reset EMPW program (users need to log in again)") << endl
        << F("erase                          - Erase all remembered sites for all users") << endl
        << F("help                           - Show this help screen") << endl
        << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t command::find_user(const char * uname, bool include_dynamic) const
{
    uint8_t c = MAX_PERSISTENT_USERS;
    if ( include_dynamic ) c++;
    for(uint8_t i=0;i<c;i++)
    {
        if ( m_users[i] == 0 )
        {
            continue;
        }
        if ( m_users[i]->is_user( uname ) )
        {
            return i;
        }
    }

    // Didn't find the user ...
    return USER_NOT_FOUND;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t command::find_user(uint32_t token) const
{
    for(uint8_t i=0;i<countof(m_users);i++)
    {
        if ( m_users[i] == 0 )
        {
            continue;
        }
        if ( m_users[i]->get_mpw().get_login_token() == token )
        {
            return i;
        }
    }

    // Didn't find the user ...
    return USER_NOT_FOUND;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ARDUINO
#include <ctime>
#endif
void command::handle_login(char * pdata)
{
    m_current_user = 0;

    // Expects <user>,[ ]<password>
    const char * uname = pdata;
    while(*pdata && *pdata!=',')
        pdata++;
    if (*pdata == ',')
        *pdata++ = '\0';
    while(*pdata == ' ') pdata++;

    uint8_t user_index = find_user(uname, false);
    if ( user_index == USER_NOT_FOUND )
    {
        // Dynamic/temp user already present, so this gets logged out
        if ( m_users[MAX_PERSISTENT_USERS] != 0 )
            delete m_users[MAX_PERSISTENT_USERS];
        m_users[MAX_PERSISTENT_USERS] = new userinfo(uname);
        user_index = MAX_PERSISTENT_USERS;
    }

    m_current_user = m_users[user_index];

    #ifdef ARDUINO
    uint32_t start = 0;
    #else
    std::clock_t start = 0;
    #endif
    m_current_user->get_mpw().login(uname, pdata,
        [&] (uint8_t percent) {
            #ifdef ARDUINO
            if ( ( millis() - start ) > 1000 )
            #else
            if ( ( std::clock() - start ) > CLOCKS_PER_SEC )
            #endif
            {
                #ifdef ARDUINO
                start = millis();
                #else
                start = std::clock();
                #endif
                IO << F("Calculating ... ") << percent << F("%") << endl;
            }
        });
    IO  << F("User [") << uname << F("] logged in") << endl
        << F("TOKEN:") << m_current_user->get_mpw().get_login_token() << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_logout(char * pdata)
{
    uint32_t token;
    sscanf(pdata, "%u", &token);

    uint8_t user_index = find_user(token);
    if ( user_index == USER_NOT_FOUND )
    {
        IO << "Couldn't find user with token " << token << endl;
    }
    else
    {
        m_users[user_index]->get_mpw().logout();
        IO << "Logged out user " << token << endl;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_switch_user(char * pdata)
{
    uint32_t token;
    sscanf(pdata, "%u", &token);

    uint8_t user_index = find_user(token);
    if ( user_index == USER_NOT_FOUND )
    {
        IO << "Couldn't find user with token " << token << endl;
    }
    else
    {
        m_current_user = m_users[user_index];
        IO << "Switched to user `" << m_current_user->get_user_name() << "`" << endl;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_list_users(void)
{
    for(uint8_t i=0; i<countof(m_users); i++)
    {
        if ( m_users[i] != 0 )
        {
            if ( m_users[i] == m_current_user )
                IO << F("* ");
            else
                IO << F("  ");
            IO << "`" << m_users[i]->get_user_name() << "`";
            if ( m_users[i]->get_mpw().is_logged_in())
                IO << F(" (Logged in)");
            IO << endl;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_add_user(char * pdata)
{
    uint8_t existing_user = find_user(pdata, false);
    if ( existing_user != USER_NOT_FOUND )
    {
        IO << "Cannot add user `" << pdata << "`. Already present in the system" << endl;
        return;
    }

    // Find first empty slot
    for(uint8_t i=0; i<MAX_PERSISTENT_USERS; i++)
    {
        if ( m_users[i] == 0 )
        {
            m_users[i] = new userinfo(pdata);
            save();
            return;
        }
    }

    IO << "Insufficient space to add user `" << pdata << "`. Please remove an existing user." << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_remove_user(char * pdata)
{
    uint8_t existing_user = find_user(pdata, false);
    if ( existing_user == USER_NOT_FOUND )
    {
        IO << "Cannot find user `" << pdata << "` to remove." << endl;
        return;
    }

    delete m_users[existing_user];
    m_users[existing_user] = 0;
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_site(char * pdata)
{
    if ( m_current_user == 0 )
    {
        IO << F("No current user, please login") << endl;
        return;
    }

    // Expects <site>,<counter>
    const char * site = pdata;
    while(*pdata && *pdata!=',')
        pdata++;
    if (*pdata == ',')
        *pdata++ = '\0';
    while(*pdata == ' ') pdata++;
    uint32_t counter=1;
    sscanf(pdata, "%u", &counter);
    m_current_user->get_mpw().site(site, counter);
    IO << F("Using site [") << site << F("]") << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_generate(char * pdata)
{
    if ( m_current_user == 0 )
    {
        IO << F("No current user selected or logged in") << endl;
        return;
    }

    int token;
    sscanf(pdata, "%d", &token);

    IO << m_current_user->get_mpw().generate((MPM_Password_Type)token) << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_reset(void)
{
#if defined(CONSOLE)
    banner();
    reset();
#elif defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
    // https://forum.pjrc.com/threads/59935-Reboot-Teensy-programmatically?p=232143&viewfull=1#post232143
    SCB_AIRCR = 0x05FA0004;
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::load(void)
{
    persistence p;
    
    uint8_t file_version = p.read8();
    if ( file_version == UNINITIALIZED_EEPROM )
        return;

    if ( file_version != 0 )
    {
        IO << "Cannot load version " << file_version << " persistent data" << endl;
        return;
    }

    uint8_t num_users = std::min( p.read8(), (uint8_t)MAX_PERSISTENT_USERS );
    for( uint8_t i=0; i<num_users; i++)
        m_users[i] = userinfo::load(p);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::save(void)
{
    persistence p;

    p.write8(EMPW_MAJOR_VERSION);

    uint8_t num_users = 0;
    for(uint8_t i=0;i<MAX_PERSISTENT_USERS;i++)
    {
        if ( m_users[i] != 0 )
            num_users++;
    }

    p.write8(num_users);

    for(size_t i=0;i<MAX_PERSISTENT_USERS;i++)
    {
        if ( m_users[i] != 0 )
            m_users[i]->save(p);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
