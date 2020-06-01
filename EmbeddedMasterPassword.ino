///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Embedded Master Password Sketch
//
//  Copyright (C) 2020, Gazoodle (https://github.com/gazoodle)
//  Algorithm Copyright (C) 2011-2018, Maarten Billemont, Lyndir (https://masterpassword.app/)
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

#include <Arduino.h>
#include "src/app/command.h"

command command_processor;
///////////////////////////////////////////////////////////////////////////////////////////////////
void setup(void)
{
    command_processor.setup();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void loop(void)
{
    command_processor.loop();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
