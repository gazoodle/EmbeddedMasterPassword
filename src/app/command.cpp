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
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SKIP_WHITESPACE(p)      while((*p == ' ') || (*p == '\t')) p++;
///////////////////////////////////////////////////////////////////////////////////////////////////
#define COMMAND_SEPARATOR       ";"
#define ARGUMENT_SEPARATOR      ","
///////////////////////////////////////////////////////////////////////////////////////////////////
#define _ARG_STR(x)             #x
#define CHECK_ARG(n)            if ( n == NULL ) { \
                                    IO << F("Expected argument <") << F(_ARG_STR(n)) << ">" << endl; \
                                    return; \
                                } \
                                SKIP_WHITESPACE(n);
#define FIRST_ARG(n)            SKIP_WHITESPACE(pdata); \
                                char * saveptr; \
                                const char * n = strtok_r( pdata, ARGUMENT_SEPARATOR, &saveptr ); \
                                CHECK_ARG(n)
#define NEXT_ARG(n)             const char * n = strtok_r( NULL, ARGUMENT_SEPARATOR, &saveptr ); \
                                CHECK_ARG(n)
///////////////////////////////////////////////////////////////////////////////////////////////////
command::command(void)
{
    memset(m_users, 0, sizeof(m_users));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
command::~command(void)
{
    release_users();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::release_users(void)
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
    SKIP_WHITESPACE(pcommand);

    char * saveptr;
    char * token = strtok_r( pcommand, COMMAND_SEPARATOR, &saveptr );
    // While there are commands to process ...
    while( token != NULL )
    {
        SKIP_WHITESPACE(token);
        // Dispatch this command
        dispatch(token);
        token = strtok_r( NULL, COMMAND_SEPARATOR, &saveptr );
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

    //
    //  Sites
    //  =====
    else if ( strncmp( pcommand, "addsite ", 8) == 0 )
        handle_add_site(pcommand+8);
    else if ( strncmp( pcommand, "sites", 6 ) == 0 )
        handle_list_sites();
    else if ( strncmp( pcommand, "removeall", 10 ) == 0 )
        handle_removeall();
    else if ( strncmp( pcommand, "setcounter ", 11) == 0 )
        handle_setcounter(pcommand+11);
    else if ( strncmp( pcommand, "settype ", 8) == 0 )
        handle_settype(pcommand+8);
    else if ( strncmp( pcommand, "sethasusername ", 15) == 0 )
        handle_sethasusername(pcommand+15);
    else if ( strncmp( pcommand, "sethasrecovery ", 15) == 0 )
        handle_sethasrecovery(pcommand+15);
    else if ( strncmp( pcommand, "addanswer ", 10) == 0 )
        handle_addanswer(pcommand+10);
    else if ( strncmp( pcommand, "removeanswer ", 13) == 0 )
        handle_removeanswer(pcommand+13);

    //
    //  Generation
    //
    else if ( strncmp( pcommand, "site ", 5) == 0 )
        handle_site(pcommand+5);



    else if ( strncmp( pcommand, "help", 5 ) == 0 )         // Includes \0 to avoid matching substrings
        handle_help();
    else if ( strncmp( pcommand, "reset", 6 ) == 0 )        // Includes \0 to avoid matching substrings
        handle_reset();
    else if ( strncmp( pcommand, "erase", 6) == 0 )         // Includes \0 to avoid matching substrings
        handle_erase();
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
        << F("login <user>, <password>          - Login <user> with <password>") << endl
        << F("logout <token>                    - Logout user <token>") << endl
        << F("user <token>                      - Switch to user <token>") << endl
        << F("users                             - List remembered users") << endl
        << F("adduser <user>                    - Add <user> to the remembered user list") << endl
        << F("removeuser <user>                 - Remove <user> from the remembered user list") << endl
        << endl
        << endl
        << F("Sites") << endl
        << F("-----") << endl
        << F("sites                             - List remembered sites for current user") << endl
        << F("addsite <site>                    - Add remembered site <site>") << endl
        << F("removesite <site>                 - Remove remembered site <site> for current user") << endl
        << F("setcounter <site>, <counter>      - Set <site> counter to <counter> (Defaults to 1)") << endl
        << F("settype <site>, <type>            - Set <site> password type to <type> (Defaults to Long)") << endl
        << F("sethasusername <site>, <state>    - Set <site> generated username to <state> (Defaults to false)") << endl
        << F("sethasrecovery <site>, <state>    - Set <site> generated recovery phrase to <state> (Defaults to false)" ) << endl
        << F("addanswer <site>, <word>          - Add a generated recovery phrase based on <word> to <site>") << endl
        << F("removeanswer <site>, <word>       - Remove generated recovery phrase for <word> from <site>") << endl
        //<< F("requirelogin <site>               - Require the user to login again to generate information for <site>") << endl
        << F("removeall                         - Remove all sites for current user") << endl
        << endl
        << endl
        << F("Passwords etc") << endl
        << F("-------------") << endl
        << F("site <site>                       - Generate passwords, usernames, and recovery answers for the site <site>") << endl
        << endl
        << endl
        << F("Maintenance") << endl
        << F("-----------") << endl
        << F("exit                              - Exit the EMPW program (only available on cli version)") << endl
        << F("reset                             - Reset EMPW program (users need to log in again)") << endl
        << F("erase                             - Erase all remembered sites for all users") << endl
        << F("help                              - Show this help screen") << endl
        << endl
        << endl
        << F("Multiple commands can be issued in one go, separated by ';', e.g.") << endl
        << endl
        << F("  user 12345; example.com") << endl
        << endl
        << F("  - Switch to user with token 12345") << endl
        << F("  - Generate security info for example.com") << endl

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
    m_current_user = NULL;

    // Expects <user>,<password>
    FIRST_ARG(username);
    NEXT_ARG(password);

    uint8_t user_index = find_user(username, false);
    if ( user_index == USER_NOT_FOUND )
    {
        // Dynamic/temp user already present, so this gets logged out
        if ( m_users[MAX_PERSISTENT_USERS] != 0 )
            delete m_users[MAX_PERSISTENT_USERS];
        m_users[MAX_PERSISTENT_USERS] = new userinfo(username);
        user_index = MAX_PERSISTENT_USERS;
    }

    m_current_user = m_users[user_index];

    #ifdef ARDUINO
    uint32_t start = 0;
    #else
    std::clock_t start = 0;
    #endif
    m_current_user->get_mpw().login(username, password,
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
    IO  << F("User [") << username << F("] logged in") << endl
        << F("TOKEN:") << m_current_user->get_mpw().get_login_token() << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_logout(char * pdata)
{
    FIRST_ARG(usertoken);
    uint32_t token = atoi(usertoken);
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
    FIRST_ARG(usertoken);
    uint32_t token = atoi(usertoken);
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
    FIRST_ARG(username);
    uint8_t existing_user = find_user(username, false);
    if ( existing_user != USER_NOT_FOUND )
    {
        IO << "Cannot add user `" << username << "`. Already present in the system" << endl;
        return;
    }

    // Find first empty slot
    for(uint8_t i=0; i<MAX_PERSISTENT_USERS; i++)
    {
        if ( m_users[i] == 0 )
        {
            m_users[i] = new userinfo(username);
            save();
            return;
        }
    }

    IO << "Insufficient space to add user `" << username << "`. Please remove an existing user." << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_remove_user(char * pdata)
{
    FIRST_ARG(username);
    uint8_t existing_user = find_user(username, false);
    if ( existing_user == USER_NOT_FOUND )
    {
        IO << "Cannot find user `" << username << "` to remove." << endl;
        return;
    }

    delete m_users[existing_user];
    m_users[existing_user] = 0;
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int strcmpi(const char * s1, const char * s2)
{
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    if (l1 != l2)
        return l1-l2;
    while(l1--)
    {
        char c1 = tolower(*s1++);
        char c2 = tolower(*s2++);
        if ( c1 != c2)
            return c1-c2;
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
MPM_Password_Type command::get_style(const char * style)
{
    SKIP_WHITESPACE(style);

    #define _STYLE_STR(x) #x
    #define STYLE_STR(x) _STYLE_STR(x)
    #define CHECK_STYLE(s, e)   { if ( strcmpi( style, STYLE_STR(e)) == 0 ) return e; }

    CHECK_STYLE(s, Basic);
    CHECK_STYLE(s, Long);
    CHECK_STYLE(s, Maximum);
    CHECK_STYLE(s, Medium);
    CHECK_STYLE(s, Name);
    CHECK_STYLE(s, Phrase);
    CHECK_STYLE(s, PIN);
    CHECK_STYLE(s, Short);

    int i = atoi(style);
    if ( i > 0 )
        return (MPM_Password_Type)i;

    // If all else fails, assume as long password
    return MPM_Password_Type::Long;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool command::check_login(void) const
{
    if ( m_current_user == 0 )
    {
        IO << F("No current user, please login") << endl;
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
siteinfo* command::find_site(const char * sitename, bool show_complaint_on_failure)
{
    if ( !check_login())
        return NULL;
    SKIP_WHITESPACE(sitename);
    for(std::vector<siteinfo>::iterator i=m_current_user->get_sites().begin(); i!=m_current_user->get_sites().end(); i++)
        if ( i->is_site(sitename) )
            return &*i;
    if ( show_complaint_on_failure )
    {
        IO << "Cannot find site `" << sitename << "`. Command not executed" << endl;
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_add_site(char * pdata)
{
    if ( !check_login())
        return;

    // Expects <site>
    FIRST_ARG(sitename);

    IO << "add site [" << sitename << "]" << endl;
    m_current_user->get_sites().push_back(siteinfo(sitename));
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_list_sites(void)
{
    if ( !check_login())
        return;

    for(std::vector<siteinfo>::const_iterator i=m_current_user->get_sites().begin(); i!=m_current_user->get_sites().end(); i++)
    {
        IO << i->get_sitename() << "/" << i->get_counter() << "/" << i->get_style() << endl;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_removeall(void)
{
    if ( !check_login())
        return;
    m_current_user->get_sites().clear();
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_setcounter(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(counter);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    s->set_counter(atoi(counter));
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_settype(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(style);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    s->set_style(get_style(style));
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_sethasusername(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(state);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    bool set = strcmpi(state, "true") == 0;
    s->set_options( set ? SET_FLAG( s->get_options(), SITEINFO_HAS_USERNAME ) : RESET_FLAG( s->get_options(), SITEINFO_HAS_USERNAME ));
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_sethasrecovery(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(state);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    bool set = strcmpi(state, "true") == 0;
    s->set_options( set ? SET_FLAG( s->get_options(), SITEINFO_HAS_RECOVERY ) : RESET_FLAG( s->get_options(), SITEINFO_HAS_RECOVERY ));
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_addanswer(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(answer);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    s->set_options( SET_FLAG( s->get_options(), SITEINFO_HAS_ANSWERS ));
    s->get_answers().push_back(answer);
    save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_removeanswer(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    NEXT_ARG(answer);
    siteinfo * s = find_site(sitename, true);
    if ( s == NULL )
        return;
    for(std::vector<str_ptr>::iterator i=s->get_answers().begin(); i!=s->get_answers().end(); i++)
    {
        if ( *i == answer )
        {
            s->get_answers().erase(i);
            if ( s->get_answers().size() == 0 )
            {
                s->set_options( RESET_FLAG( s->get_options(), SITEINFO_HAS_ANSWERS ));
            }
            save();
            return;
        }
    }

    IO << "Couldn't find answer word `" << answer << "` for site `" << sitename << "` to remove" << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void command::handle_site(char * pdata)
{
    if ( !check_login())
        return;
    FIRST_ARG(sitename);
    // Create default siteinfo
    siteinfo def(sitename);
    // Locate saved site if there is one
    auto psite = find_site(sitename, false);
    if ( psite == NULL )
        psite = &def;

    if ( IS_FLAG_SET( psite->get_options(), SITEINFO_HAS_USERNAME ))
    {
        IO << "user: " << m_current_user->get_mpw().generate( psite->get_sitename(), MPW_USERNAME_COUNTER, MPW_USERNAME_TYPE, NULL, MPW_Scope_Identification ) << endl;
    }
    IO << "password: " << m_current_user->get_mpw().generate( psite->get_sitename(), psite->get_counter(), psite->get_style(), NULL, MPW_Scope_Authentication ) << endl;
    if ( IS_FLAG_SET( psite->get_options(), SITEINFO_HAS_RECOVERY ))
    {
        IO << "recovery: " << m_current_user->get_mpw().generate( psite->get_sitename(), MPW_RECOVERY_COUNTER, MPW_RECOVERY_TYPE, NULL, MPW_Scope_Recovery ) << endl;
    }
    if ( IS_FLAG_SET( psite->get_options(), SITEINFO_HAS_ANSWERS ))
    {
        for( std::vector<str_ptr>::const_iterator i = psite->get_answers().begin(); i != psite->get_answers().end(); i++ )
        {
            IO << "recovery[" << *i << "]: " << m_current_user->get_mpw().generate( psite->get_sitename(), MPW_RECOVERY_COUNTER, MPW_RECOVERY_TYPE, *i, MPW_Scope_Recovery ) << endl;
        }
    }
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
void command::handle_erase(void)
{
    release_users();
    persistence p;
    p.erase();
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
