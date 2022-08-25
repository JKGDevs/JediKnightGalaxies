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

/*
* =====================
Versioning Explanation
* =====================
Phase.Major.Minor(suffix)
eg: v3.2.458a

Phase:			This represents an entirely new type of gameplay or design.  v1: JKG Versus, v2: JKG Coop, v3: JKG RPG, v4: JKG LMO
				See the readme for more information on game phases (these are similar to expansion packs).

Major:			This represents large new feature sets, bug fixes, improvements etc that still align to the current phase goal.  
				These are usually based on a Github Issue List Milestone. eg: https://github.com/JKGDevs/JediKnightGalaxies/milestones

Minor:			This represents small new feature sets (if any), but is mostly quality of life, security patches, and bug fixes.

suffix:			This represents a small patch or hotfix on the server.  Used for server-side only changes where clients are not required to update.  
				Uses a lowercase alphabet "number", eg: a, b, c, etc. These should only be used for hotfixes/emergencies.
*/

// Current version of the multi player game
#define JKG_VERSION "1.3.23"

#ifdef _DEBUG
	#define JK_VERSION "" GIT_BRANCH " / " GIT_HASH " (debug)"
	#define JKG_VERSION_SUFFIX ""
	
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
