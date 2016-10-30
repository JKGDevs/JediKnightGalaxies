/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#ifndef _GAME_VERSION_H
#define _GAME_VERSION_H

#define _STR(x) #x
#define STR(x) _STR(x)

#include "../win32/AutoVersion.h"
#include "../git.h"

#define JKG_VERSION "1.5.05"
// Current version of the multi player game
#ifdef _DEBUG
	#define JK_VERSION "" GIT_BRANCH " / " GIT_HASH " (debug)"
	#define JKG_VERSION_SUFFIX "d"
	
#elif defined FINAL_BUILD
	#define JK_VERSION "" GIT_BRANCH " / " GIT_HASH ""
	#define JKG_VERSION_SUFFIX ""
	
#else
	#define JK_VERSION "" GIT_BRANCH " / " GIT_HASH " (internal)"
	#define JKG_VERSION_SUFFIX "r"
#endif

#define	GAMEVERSION	"Jedi Knight Galaxies v" JKG_VERSION JKG_VERSION_SUFFIX

#endif

//end
