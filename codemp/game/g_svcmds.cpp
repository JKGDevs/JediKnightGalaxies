/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"

// Include GLua 
#include "../GLua/glua.h"
#ifdef __UNUSED__
#include "jkg_navmesh_creator.h"
#endif //__UNUSED__


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct ipFilter_s {
	uint32_t mask, compare;
} ipFilter_t;

#define	MAX_IPFILTERS (1024)

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int	numIPFilters;

extern void AIMod_TimeMapPaths();

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter( char *s, ipFilter_t *f ) {
	char num[128];
	int i, j;
	byteAlias_t b, m;

	b.ui = m.ui = 0u;

	for ( i=0; i<4; i++ ) {
		if ( *s < '0' || *s > '9' ) {
			if ( *s == '*' ) {
				// 'match any'
				// b[i] and m[i] to 0
				s++;
				if ( !*s )
					break;
				s++;
				continue;
			}
			trap->Print( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while ( *s >= '0' && *s <= '9' )
			num[j++] = *s++;

		num[j] = 0;
		b.b[i] = (byte)atoi( num );
		m.b[i] = 0xFF;

		if ( !*s )
			break;

		s++;
	}

	f->mask = m.ui;
	f->compare = b.ui;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans( void ) {
	byteAlias_t b, m;
	int i, j;
	char ip[NET_ADDRSTRMAXLEN], iplist_final[MAX_CVAR_VALUE_STRING];

	*iplist_final = 0;
	for ( i=0; i<numIPFilters; i++ ) {
		if ( ipFilters[i].compare == 0xFFFFFFFFu )
			continue;

		b.ui = ipFilters[i].compare;
		m.ui = ipFilters[i].mask;
		*ip = 0;
		for ( j=0; j<4; j++ ) {
			if ( m.b[j] != 0xFF )
				Q_strcat( ip, sizeof( ip ), "*" );
			else
				Q_strcat( ip, sizeof( ip ), va( "%i", b.b[j] ) );
			Q_strcat( ip, sizeof( ip ), (j<3) ? "." : " " );
		}
		if ( strlen( iplist_final )+strlen( ip ) < MAX_CVAR_VALUE_STRING )
			Q_strcat( iplist_final, sizeof( iplist_final ), ip );
		else {
			Com_Printf( "g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n" );
			break;
		}
	}

	trap->Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket( char *from ) {
	int i;
	uint32_t in;
	byteAlias_t m;
	char *p;

	i = 0;
	p = from;
	while ( *p && i < 4 ) {
		m.b[i] = 0;
		while ( *p >= '0' && *p <= '9' ) {
			m.b[i] = m.b[i]*10 + (*p - '0');
			p++;
		}
		if ( !*p || *p == ':' )
			break;
		i++, p++;
	}

	in = m.ui;

	for ( i=0; i<numIPFilters; i++ ) {
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare )
			return g_filterBan.integer != 0;
	}

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str ) {
	int i;

	for ( i=0; i<numIPFilters; i++ ) {
		if ( ipFilters[i].compare == 0xFFFFFFFFu )
			break; // free spot
	}
	if ( i == numIPFilters ) {
		if ( numIPFilters == MAX_IPFILTERS ) {
			trap->Print( "IP filter list is full\n" );
			return;
		}
		numIPFilters++;
	}

	if ( !StringToFilter( str, &ipFilters[i] ) )
		ipFilters[i].compare = 0xFFFFFFFFu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans( void ) {
	char *s = NULL, *t = NULL, str[MAX_CVAR_VALUE_STRING] = {0};

	Q_strncpyz( str, g_banIPs.string, sizeof( str ) );

	for ( t=s=g_banIPs.string; *t; t=s ) {
		s = strchr( s, ' ' );
		if ( !s )
			break;

		while ( *s == ' ' )
			*s++ = 0;

		if ( *t )
			AddIP( t );
	}
}

/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap->Argc() < 2 ) {
		trap->Print("Usage: addip <ip-mask>\n");
		return;
	}

	trap->Argv( 1, str, sizeof( str ) );

	AddIP( str );
}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap->Argc() < 2 ) {
		trap->Print("Usage: removeip <ip-mask>\n");
		return;
	}

	trap->Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			trap->Print ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	trap->Print ( "Didn't find %s.\n", str );
}

void Svcmd_ListIP_f (void)
{
	int		i, count = 0;
	byteAlias_t b;

	for(i = 0; i < numIPFilters; i++) {
		if ( ipFilters[i].compare == 0xffffffffu )
			continue;

		b.ui = ipFilters[i].compare;
		trap->Print ("%i.%i.%i.%i\n", b.b[0], b.b[1], b.b[2], b.b[3]);
		count++;
	}
	trap->Print ("%i bans.\n", count);
}

/*
===================
Svcmd_EntityInfo_f
===================
*/
void	Svcmd_EntityInfo_f (void) {
	int totalents;
	int inuse;
	int i;
	gentity_t *e;

	inuse = 0;
	for (e = &g_entities[0], i=0; i < level.num_entities ; e++, i++) {
		if (e->inuse) {
			inuse++;
		}
	}
	trap->Print("Normal entity slots in use: %i/%i (%i slots allocated)\n", inuse, MAX_GENTITIES, level.num_entities);
	totalents = inuse;

	inuse = 0;
	for (e = &g_entities[MAX_GENTITIES], i=0; i < level.num_logicalents ; e++, i++) {
		if (e->inuse) {
			inuse++;
		}
	}
	trap->Print("Logical entity slots in use: %i/%i (%i slots allocated)\n", inuse, MAX_LOGICENTITIES, level.num_logicalents);
	totalents += inuse;
	trap->Print("Total entity count: %i/%i\n", totalents, MAX_ENTITIESTOTAL);
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		trap->Print("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			trap->Print("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			trap->Print("ET_PLAYER           ");
			break;
		case ET_ITEM:
			trap->Print("ET_ITEM             ");
			break;
		case ET_MISSILE:
			trap->Print("ET_MISSILE          ");
			break;
		case ET_MOVER:
			trap->Print("ET_MOVER            ");
			break;
		case ET_BEAM:
			trap->Print("ET_BEAM             ");
			break;
		case ET_PORTAL:
			trap->Print("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			trap->Print("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			trap->Print("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			trap->Print("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			trap->Print("ET_INVISIBLE        ");
			break;
		case ET_NPC:
			trap->Print("ET_NPC              ");
			break;
		default:
			trap->Print("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			trap->Print("%s", check->classname);
		}
		trap->Print("\n");
	}
}

qboolean StringIsInteger( const char *s );
/*
===================
ClientForString
===================
*/
gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanInput[MAX_STRING_CHARS];

	// numeric values could be slot numbers
	if ( StringIsInteger( s ) ) {
		idnum = atoi( s );
		if ( idnum >= 0 && idnum < level.maxclients ) {
			cl = &level.clients[idnum];
			if ( cl->pers.connected == CON_CONNECTED ) {
				return cl;
			}
		}
	}

	Q_strncpyz( cleanInput, s, sizeof(cleanInput) );
	Q_StripColor( cleanInput );

	// check for a name match
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname_nocolor, cleanInput ) ) {
			return cl;
		}
	}

	trap->Print( "User %s is not on the server\n", s );
	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	if ( trap->Argc() < 3 ) {
		trap->Print("Usage: forceteam <player> <team>\n");
		return;
	}

	// find the player
	trap->Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap->Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap->Argv( 0, cmd, sizeof( cmd ) );

	if (!Q_stricmp (cmd, "lua_reload")) {
		trap->Print("Doing soft GLua restart\n");
		GLua_SoftRestart();
		return qtrue;
	}
	if (!Q_stricmp (cmd, "lua_restart")) {
		trap->Print("Doing hard GLua restart\n");
		GLua_Close();
		GLua_Init();
		GLua_Hook_GameInit(level.time, 0);
		GLua_Hook_MapLoadFinished();
		return qtrue;
	}
	if (!Q_stricmp (cmd, "lua_run")) {
		char line[1024] = {0};
		char buff[1024] = {0};
		int i, argc;
		argc = trap->Argc();
		for (i=1; i < argc; i++) {
			trap->Argv(i, buff, sizeof(buff));
			Q_strcat(line, 1023, buff);
			Q_strcat(line, 1023, " ");
			//Com_sprintf(line, sizeof(line), "%s%s ", line, buff);
		}
		GLua_Run(line);
		return qtrue;
	}

#if 0
	if (!Q_stricmp (cmd, "disasm")) {
		Cmd_DisAsmDirect_f();
		return qtrue;
	}


	if (!Q_stricmp (cmd, "stresslevel")) {
		JKG_GLS_StressLevelInfo();
		return qtrue;
	}
#endif

	if (!Q_stricmp(cmd, "toggleallowvote")) {
		Svcmd_ToggleAllowVote_f();
		return qtrue;
	}

	if (GLua_RconCommand(cmd))
		return qtrue;

	if ( !Q_stricmp (cmd, "entinfo") ) {
		Svcmd_EntityInfo_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		Svcmd_ListIP_f();
		return qtrue;
	}

	if ( !Q_stricmp( cmd, "toggleuserinfovalidation" ) ) {
		Svcmd_ToggleUserinfoValidation_f();
		
		return qtrue;
	}
	if (dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap->SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}

		if ( Q_stricmp (cmd, "timemappaths") == 0 )
		{
			AIMod_TimeMapPaths();
			return qtrue;
		}
		// everything else will NOT also be printed as a say command
		//trap->SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(0) ) );
		//return qtrue;
	}

	return qfalse;
}

