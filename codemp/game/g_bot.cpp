/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
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

// g_bot.c

#include "g_local.h"
#include "ai_main.h"

static int		g_numBots;
static char		*g_botInfos[MAX_BOTS];


int				g_numArenas;
static char		*g_arenaInfos[MAX_ARENAS];


#define BOT_BEGIN_DELAY_BASE		2000
#define BOT_BEGIN_DELAY_INCREMENT	1500

#define BOT_SPAWN_QUEUE_DEPTH	16

typedef struct {
	int		clientNum;
	int		spawnTime;
} botSpawnQueue_t;

static botSpawnQueue_t	botSpawnQueue[BOT_SPAWN_QUEUE_DEPTH];

vmCvar_t bot_minplayers;

#ifdef __NPC_MINPLAYERS__
vmCvar_t npc_enemies;
vmCvar_t npc_civilians;
vmCvar_t npc_vendors;
#endif //__NPC_MINPLAYERS__

static float Cvar_VariableValue( const char *var_name ) {
	char buf[MAX_CVAR_VALUE_STRING];

	trap->Cvar_VariableStringBuffer(var_name, buf, sizeof(buf));
	return atof(buf);
}

/*
===============
G_ParseInfos
===============
*/
int G_ParseInfos( char *buf, int max, char *infos[] ) {
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( (const char **)(&buf) );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		info[0] = '\0';
		while ( 1 ) {
			token = COM_ParseExt( (const char **)(&buf), qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( (const char **)(&buf), qfalse );
			if ( !token[0] ) {
				strcpy( token, "<NULL>" );
			}
			Info_SetValueForKey( info, key, token );
		}
		//NOTE: extra space for arena number
		infos[count] = (char *) G_Alloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);
		if (infos[count]) {
			strcpy(infos[count], info);
			count++;
		}
	}
	return count;
}

/*
===============
G_LoadArenasFromFile
===============
*/
static void G_LoadArenasFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_ARENAS_TEXT];

	len = trap->FS_Open( filename, &f, FS_READ );
	if ( !f ) {
		trap->Print( S_COLOR_RED "file not found: %s\n", filename );
		return;
	}
	if ( len >= MAX_ARENAS_TEXT ) {
		trap->Print( S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_ARENAS_TEXT );
		trap->FS_Close( f );
		return;
	}

	trap->FS_Read( buf, len, f );
	buf[len] = 0;
	trap->FS_Close( f );

	g_numArenas += G_ParseInfos( buf, MAX_ARENAS - g_numArenas, &g_arenaInfos[g_numArenas] );
}

int G_GetMapTypeBits(char *type)
{
	int typeBits = 0;

	if( *type ) {
		if( strstr( type, "ffa" ) ) {
			typeBits |= (1 << GT_FFA);
			typeBits |= (1 << GT_TEAM);
		}
		if( strstr( type, "warzone" ) ) {
			typeBits |= (1 << GT_WARZONE);
			//typeBits |= (1 << GT_TEAM);
		}
		if( strstr( type, "duel" ) ) {
			typeBits |= (1 << GT_DUEL);
			typeBits |= (1 << GT_POWERDUEL);
		}
		if( strstr( type, "powerduel" ) ) {
			typeBits |= (1 << GT_DUEL);
			typeBits |= (1 << GT_POWERDUEL);
		}
		if( strstr( type, "ctf" ) ) {
			typeBits |= (1 << GT_CTF);
		}
	} else {
		typeBits |= (1 << GT_FFA);
	}

	return typeBits;
}

qboolean G_DoesMapSupportGametype(const char *mapname, int gametype)
{
	int			typeBits = 0;
	int			thisLevel = -1;
	int			n = 0;
	char		*type = NULL;

	if (!g_arenaInfos[0])
	{
		return qfalse;
	}

	if (!mapname || !mapname[0])
	{
		return qfalse;
	}

	for( n = 0; n < g_numArenas; n++ )
	{
		type = Info_ValueForKey( g_arenaInfos[n], "map" );

		if (Q_stricmp(mapname, type) == 0)
		{
			thisLevel = n;
			break;
		}
	}

	if (thisLevel == -1)
	{
		return qfalse;
	}

	type = Info_ValueForKey(g_arenaInfos[thisLevel], "type");

	typeBits = G_GetMapTypeBits(type);
	if (typeBits & (1 << gametype))
	{ //the map in question supports the gametype in question, so..
		return qtrue;
	}

	return qfalse;
}

//rww - auto-obtain nextmap. I could've sworn Q3 had something like this, but I guess not.
const char *G_RefreshNextMap(int gametype, qboolean forced)
{
	int			typeBits = 0;
	int			thisLevel = 0;
	int			desiredMap = 0;
	int			n = 0;
	char		*type = NULL;
	qboolean	loopingUp = qfalse;
	vmCvar_t	mapname;

	if (!g_autoMapCycle.integer && !forced)
	{
		return NULL;
	}

	if (!g_arenaInfos[0])
	{
		return NULL;
	}

	trap->Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	for( n = 0; n < g_numArenas; n++ )
	{
		type = Info_ValueForKey( g_arenaInfos[n], "map" );

		if (Q_stricmp(mapname.string, type) == 0)
		{
			thisLevel = n;
			break;
		}
	}

	desiredMap = thisLevel;

	n = thisLevel+1;
	while (n != thisLevel)
	{ //now cycle through the arena list and find the next map that matches the gametype we're in
		if (!g_arenaInfos[n] || n >= g_numArenas)
		{
			if (loopingUp)
			{ //this shouldn't happen, but if it does we have a null entry break in the arena file
			  //if this is the case just break out of the loop instead of sticking in an infinite loop
				break;
			}
			n = 0;
			loopingUp = qtrue;
		}

		type = Info_ValueForKey(g_arenaInfos[n], "type");
		
		typeBits = G_GetMapTypeBits(type);
		if (typeBits & (1 << gametype))
		{
			desiredMap = n;
			break;
		}

		n++;
	}

	if (desiredMap == thisLevel)
	{ //If this is the only level for this game mode or we just can't find a map for this game mode, then nextmap
	  //will always restart.
		trap->Cvar_Set( "nextmap", "map_restart 0");
	}
	else
	{ //otherwise we have a valid nextmap to cycle to, so use it.
		type = Info_ValueForKey( g_arenaInfos[desiredMap], "map" );
		trap->Cvar_Set( "nextmap", va("map %s", type));
	}

	return Info_ValueForKey( g_arenaInfos[desiredMap], "map" );
}

/*
===============
G_LoadArenas
===============
*/

#define MAX_MAPS 256
#define MAPSBUFSIZE (MAX_MAPS * 32)

void G_LoadArenas( void ) {
#if 0
	int			numdirs;
	char		filename[128];
	char		filename[MAX_QPATH];
	char		dirlist[1024];
	char*		dirptr;
	int			i, n;
	int			dirlen;

	g_numArenas = 0;

	// get all arenas from .arena files
	numdirs = trap->FS_GetFileList("scripts", ".arena", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		Q_strncpyz( filename, "scripts/", sizeof( filename ) );
		strcat(filename, dirptr);
		G_LoadArenasFromFile(filename);
	}
//	trap->Print( va( "%i arenas parsed\n", g_numArenas ) );
	
	for( n = 0; n < g_numArenas; n++ ) {
		Info_SetValueForKey( g_arenaInfos[n], "num", va( "%i", n ) );
	}

	G_RefreshNextMap(level.gametype, qfalse);

#else // Ensiform's version

	int			numFiles;
	char		filelist[MAPSBUFSIZE];
	char		filename[MAX_QPATH];
	char*		fileptr;
	int			i, n;
	int			len;

	g_numArenas = 0;

	// get all arenas from .arena files
	numFiles = trap->FS_GetFileList("scripts", ".arena", filelist, ARRAY_LEN(filelist) );

	fileptr  = filelist;
	i = 0;

	if (numFiles > MAX_MAPS)
		numFiles = MAX_MAPS;

	for(; i < numFiles; i++) {
		len = strlen(fileptr);
		Com_sprintf(filename, sizeof(filename), "scripts/%s", fileptr);
		G_LoadArenasFromFile(filename);
		fileptr += len + 1;
	}
//	trap->Print( va( "%i arenas parsed\n", g_numArenas ) );
	
	for( n = 0; n < g_numArenas; n++ ) {
		Info_SetValueForKey( g_arenaInfos[n], "num", va( "%i", n ) );
	}

	G_RefreshNextMap(level.gametype, qfalse);
#endif

}

/*
===============
G_GetArenaInfoByNumber
===============
*/
const char *G_GetArenaInfoByMap( const char *map ) {
	int			n;

	for( n = 0; n < g_numArenas; n++ ) {
		if( Q_stricmp( Info_ValueForKey( g_arenaInfos[n], "map" ), map ) == 0 ) {
			return g_arenaInfos[n];
		}
	}

	return NULL;
}

#if 0
/*
=================
PlayerIntroSound
=================
*/
static void PlayerIntroSound( const char *modelAndSkin ) {
	char	model[MAX_QPATH];
	char	*skin;

	Q_strncpyz( model, modelAndSkin, sizeof(model) );
	skin = Q_strrchr( model, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = model;
	}

	if( Q_stricmp( skin, "default" ) == 0 ) {
		skin = model;
	}

	trap->SendConsoleCommand( EXEC_APPEND, va( "play sound/player/announce/%s.wav\n", skin ) );
}
#endif

/*
===============
G_AddRandomBot
===============
*/
void G_AddRandomBot( int team ) {
	int		i, n, num;
	float	skill;
	char	*value, netname[36], *teamstr;
	gclient_t	*cl;

	num = 0;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< sv_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			//JAC: Invalid clientNum was being used
			if ( !(g_entities[i].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= sv_maxclients.integer) {
			num++;
		}
	}
	num = random() * num;
	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		//
		for ( i=0 ; i< sv_maxclients.integer ; i++ ) {
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) {
				continue;
			}
			//JAC: Invalid clientNum was being used
			if ( !(g_entities[i].r.svFlags & SVF_BOT) ) {
				continue;
			}
			if ( team >= 0 && cl->sess.sessionTeam != team ) {
				continue;
			}
			if ( !Q_stricmp( value, cl->pers.netname ) ) {
				break;
			}
		}
		if (i >= sv_maxclients.integer) {
			num--;
			if (num <= 0) {
				skill = Cvar_VariableValue( "g_npcspskill" );
				if (team == TEAM_RED) teamstr = "red";
				else if (team == TEAM_BLUE) teamstr = "blue";
				else teamstr = "";
				Q_strncpyz(netname, value, sizeof(netname));
				Q_CleanStr(netname);
				trap->SendConsoleCommand( EXEC_INSERT, va("addbot \"%s\" %.2f %s %i\n", netname, skill, teamstr, 0) );
				return;
			}
		}
	}
}

/*
===============
G_RemoveRandomBot
===============
*/
int G_RemoveRandomBot( int team ) {
	int i;
	gclient_t	*cl;

	for ( i=0 ; i< sv_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		//JAC: Invalid clientNum was being used
		if ( !(g_entities[i].r.svFlags & SVF_BOT) )
			continue;

		//JAC: this entity is actually following another entity so the ps data is for a different entity.
		//		Bots never spectate like this so, skip this player.
		if ( cl->sess.sessionTeam == TEAM_SPECTATOR && cl->sess.spectatorState == SPECTATOR_FOLLOW )
			continue;

		if ( team >= 0 && cl->sess.sessionTeam != team )
			continue;

		trap->SendConsoleCommand( EXEC_INSERT, va("clientkick %d\n", i) );
		return qtrue;
	}
	return qfalse;
}

/*
===============
G_CountHumanPlayers
===============
*/
int G_CountHumanPlayers( int team ) {
	int i, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< sv_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( g_entities[i].r.svFlags & SVF_BOT ) {
			continue;
		}
		if ( team >= 0 && cl->sess.sessionTeam != team ) {
			continue;
		}
		num++;
	}
	return num;
}

/*
===============
G_CountBotPlayers
===============
*/
int G_CountBotPlayers( int team ) {
	int i, n, num;
	gclient_t	*cl;

	num = 0;
	for ( i=0 ; i< sv_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED )
			continue;
		if ( !(g_entities[i].r.svFlags & SVF_BOT) )
			continue;
		if ( team >= 0 && cl->sess.sessionTeam != team )
			continue;
		num++;
	}
	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime )
			continue;
		if ( botSpawnQueue[n].spawnTime > level.time )
			continue;
		num++;
	}
	return num;
}

/*
===============
G_CheckMinimumPlayers
===============
*/
void G_CheckMinimumPlayers( void ) {
	int minplayers;
	int humanplayers, botplayers;
	static int checkminimumplayers_time;

	if (level.intermissiontime) return;
	//only check once each 10 seconds
	if (checkminimumplayers_time > level.time - 10000) {
		return;
	}
	checkminimumplayers_time = level.time;
	trap->Cvar_Update(&bot_minplayers);
	minplayers = bot_minplayers.integer;
	if (minplayers <= 0) return;

	if (minplayers > sv_maxclients.integer)
	{
		minplayers = sv_maxclients.integer;
	}

	humanplayers = G_CountHumanPlayers( -1 );
	botplayers = G_CountBotPlayers(	-1 );

	if ((humanplayers+botplayers) < minplayers)
	{
		G_AddRandomBot(-1);
	}
	else if ((humanplayers+botplayers) > minplayers && botplayers)
	{
		// try to remove spectators first
		if (!G_RemoveRandomBot(TEAM_SPECTATOR))
		{
			// just remove the bot that is playing
			G_RemoveRandomBot(-1);
		}
	}
}

#ifdef __NPC_MINPLAYERS__

extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];

extern void SP_NPC_spawner( gentity_t *self);
extern void SP_NPC_spawner_tc(gentity_t *self, char *tc);

int checkminimumnpcs_time = 0;

qboolean JKG_CheckBelowWaypoint( int wp )
{
	trace_t tr;
	vec3_t org, org2;

	VectorCopy(gWPArray[wp]->origin, org);
	VectorCopy(gWPArray[wp]->origin, org2);
	org2[2] = -65536.0f;//org[2] - 256;

	trap->Trace( &tr, org, NULL, NULL, org2, -1, MASK_PLAYERSOLID|CONTENTS_TRIGGER, 0, 0, 0);
	
	if ( tr.startsolid )
	{
		//trap->Print("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.allsolid )
	{
		//trap->Print("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.fraction == 1 )
	{
		//trap->Print("Waypoint %i is too high above ground.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_LAVA )
	{
		//trap->Print("Waypoint %i is in lava.\n", wp);
		return qfalse;
	}
	
	if ( tr.contents & CONTENTS_SLIME )
	{
		//trap->Print("Waypoint %i is in slime.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_TRIGGER )
	{
		//trap->Print("Waypoint %i is in trigger.\n", wp);
		return qfalse;
	}

	return qtrue;
}

extern gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, team_t team, qboolean isbot );
extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);
qboolean JKG_CheckRoutingFrom( int wp )
{
#ifdef __CHECK_ROUTING_BEFORE_WAYPOINT_SPAWNS__
	gentity_t *spot = NULL;

	/*while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}*/

	// Check for routing to a spawnpoint from a (spawn) waypoint...
	spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");

	if (spot->wpCurrent <= 0) // Should only need to do this part once...
		spot->wpCurrent = DOM_GetBestWaypoint(spot->s.origin, -1, -1);

	spot->longTermGoal = wp;

	memset(spot->pathlist, -1, MAX_WPARRAY_SIZE);
	
	spot->pathsize = ASTAR_FindPathFast(spot->wpCurrent, spot->longTermGoal, spot->pathlist, qfalse);

	//if (spot->pathsize <= 0) // Alt A* Pathing...
		//spot->pathsize = DOM_FindIdealPathtoWP(NULL, spot->wpCurrent, spot->longTermGoal, -1, spot->pathlist);

	if (spot->pathsize > 0)
	{
		//trap->Print("Routing was %i. Spot is at %f %f %f.\n", spot->pathsize, spot->s.origin[0], spot->s.origin[1], spot->s.origin[2]);
		return qtrue; // Found a route... This waypoint looks good to spawn NPCs at!
	}

	return qfalse;
#else //!__CHECK_ROUTING_BEFORE_WAYPOINT_SPAWNS__
	return qtrue;
#endif //__CHECK_ROUTING_BEFORE_WAYPOINT_SPAWNS__
}

void G_CheckVendorNPCs( void )
{
	//if minimum vendors are not enabled
	if (jkg_minVendors.integer < 1)
		return;
	//
	// We always should have some vendors on maps...
	//
	int		botplayers = 0;
	int		minvendors = jkg_minVendors.integer;


	//find previous existing vendors
	for (int i = level.maxclients; i < MAX_GENTITIES; i++)
	{
		gentity_t *npc = &g_entities[i];

		if (!npc) continue;
		if (npc->s.eType != ET_NPC) continue;
		if (!npc->client) continue; // ?

		if (npc->inventory->size() > 0)	//does it have something in the inventory?
			++botplayers;

		else
		{
			switch (npc->client->NPC_class)
			{// UQ1: Vendor types... Stand still for now...
				case CLASS_GENERAL_VENDOR:
				case CLASS_WEAPONS_VENDOR:
				case CLASS_ARMOR_VENDOR:
				case CLASS_SUPPLIES_VENDOR:
				case CLASS_EQUIPMENT_VENDOR:
				case CLASS_FOOD_VENDOR:
				case CLASS_MEDICAL_VENDOR:
				case CLASS_GAMBLER_VENDOR:
				case CLASS_TRADE_VENDOR:
				case CLASS_ODDITIES_VENDOR:
				case CLASS_DRUG_VENDOR:
				case CLASS_TRAVELLING_VENDOR:
					++botplayers;
					break;
			default:
				switch (npc->s.NPC_class)
				{// UQ1: Vendor types... Stand still for now...
					case CLASS_GENERAL_VENDOR:
					case CLASS_WEAPONS_VENDOR:
					case CLASS_ARMOR_VENDOR:
					case CLASS_SUPPLIES_VENDOR:
					case CLASS_EQUIPMENT_VENDOR:
					case CLASS_FOOD_VENDOR:
					case CLASS_MEDICAL_VENDOR:
					case CLASS_GAMBLER_VENDOR:
					case CLASS_TRADE_VENDOR:
					case CLASS_ODDITIES_VENDOR:
					case CLASS_DRUG_VENDOR:
					case CLASS_TRAVELLING_VENDOR:
						++botplayers;
						break;
					default:
						break;
				}
				break;
			}
		}
	}

	//wait for the level to start before spawning additional vendors, and make sure vendors belong in the gametype
	if ( botplayers < minvendors && (level.gametype == GT_FFA || level.gametype == GT_CTF || level.gametype == GT_TEAM) && level.time > 3000)	//note: wait about 3 seconds seems about right
	{
		gentity_t	*npc = NULL;
		int			waypoint = irand(0, gWPNum-1);
		int			tries = 0;


		/*while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint ))
		{
			gWPArray[waypoint]->inuse = qfalse; // set it bad!

			if (tries > 10)
			{
				return; // Try again on next check...
			}

			// Find a new one... This is probably a bad waypoint...
			waypoint = irand(0, gWPNum-1);
			tries++;
		}*/

		while (!JKG_CheckBelowWaypoint(waypoint))
		{
			//we tried enough times to find a valid spot
			if (tries > 10)
				return; // Try again on next check...

			// Find a new place to spawn, the first one was dangerous
			Rand_Init(time(0)); //reseed 
			waypoint = irand(0, gWPNum - 1);
			tries++;
		}

		npc = G_Spawn();

		// UQ1: Always spawn travelling vendor NPCs...
		npc->NPC_type = "vendor_travelling";
		npc->s.NPC_class = CLASS_TRAVELLING_VENDOR;
		npc->NPC_targetname = "travelvendor";

		VectorCopy(gWPArray[waypoint]->origin, npc->s.origin);
		npc->s.origin[2]+=32; // Drop down...

		npc->s.angles[PITCH] = 0;
		npc->s.angles[YAW] = irand(0,359);
		npc->s.angles[ROLL] = 0;

		botplayers++;
		trap->Print("[%i/%i] Spawning (travelling vendor NPC) %s at waypoint %i.\n", botplayers, minvendors, npc->NPC_type, waypoint);
		npc->s.eFlags |= EF_RADAROBJECT;

		char treasure[] = "genericvendor";
		SP_NPC_spawner_tc(npc, treasure);
		//SP_NPC_spawner(npc);		//not working?
		
	}

}

void G_CheckCivilianNPCs( void )
{
	int		botplayers = 0;
	int		minplayers = 0;
	int		i = 0;

	trap->Cvar_Update(&npc_civilians);
	minplayers = npc_civilians.integer;

	if (minplayers <= 0) return;

	if (minplayers > 512)
	{
		minplayers = 512;
	}

	for (i = level.maxclients; i < MAX_GENTITIES; i++)
	{
		gentity_t *npc = &g_entities[i];

		if (!npc) continue;
		if (npc->s.eType != ET_NPC) continue;
		if (npc->client->NPC_class != CLASS_CIVILIAN) continue;

		botplayers++;
	}

	if (botplayers < minplayers)
	{
		gentity_t	*npc = NULL;
		int			waypoint = irand(0, gWPNum-1);
		int			random = irand(0,36);
		int			tries = 0;

		while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) /*|| !JKG_CheckRoutingFrom( waypoint )*/)
		{
			gWPArray[waypoint]->inuse = qfalse; // set it bad!

			if (tries > 10)
			{
				return; // Try again on next check...
			}

			// Find a new one... This is probably a bad waypoint...
			waypoint = irand(0, gWPNum-1);
			tries++;
		}

		npc = G_Spawn();

		switch (random)
		{
		case 0:
			npc->NPC_type = "civilian_bartender";
			break;
		case 1:
			npc->NPC_type = "civilian_bartender";
			break;
		case 2:
			npc->NPC_type = "civilian_bartender";
			break;
		case 3:
			npc->NPC_type = "civilian_bartender";
			break;
		case 4:
			npc->NPC_type = "civilian_human_merc";
			break;
		case 5:
			npc->NPC_type = "civilian_human_merc";
			break;
		case 6:
			npc->NPC_type = "civilian_human_merc";
			break;
		case 7:
			npc->NPC_type = "civilian_human_merc";
			break;
		case 8:
			npc->NPC_type = "civilian_merchant";
			break;
		case 9:
			npc->NPC_type = "civilian_merchant";
			break;
		case 10:
			npc->NPC_type = "civilian_merchant";
			break;
		case 11:
			npc->NPC_type = "civilian_merchant";
			break;
		case 12:
			npc->NPC_type = "civilian_protocol";
			break;
		case 13:
			//npc->NPC_type = "civilian_protocol";
			//break;
		case 14:
			//npc->NPC_type = "civilian_protocol";
			//break;
		case 15:
			npc->NPC_type = "civilian_r2d2";
			break;
		case 16:
			npc->NPC_type = "civilian_r2d2";
			break;
		case 17:
			//npc->NPC_type = "civilian_r2d2";
			//break;
		case 18:
			npc->NPC_type = "civilian_rodian";
			break;
		case 19:
			npc->NPC_type = "civilian_rodian";
			break;
		case 20:
			npc->NPC_type = "civilian_rodian2";
			break;
		case 21:
			npc->NPC_type = "civilian_rodian2";
			break;
		case 22:
			npc->NPC_type = "civilian_trandoshan";
			break;
		case 23:
			npc->NPC_type = "civilian_trandoshan";
			break;
		case 24:
			npc->NPC_type = "civilian_trandoshan";
			break;
		case 25:
			npc->NPC_type = "civilian_trandoshan";
			break;
		case 26:
			npc->NPC_type = "civilian_ugnaught";
			break;
		case 27:
			npc->NPC_type = "civilian_ugnaught";
			break;
		case 28:
			npc->NPC_type = "civilian_ugnaught2";
			break;
		case 29:
			npc->NPC_type = "civilian_ugnaught2";
			break;
		case 30:
			npc->NPC_type = "civilian_weequay";
			break;
		case 31:
			npc->NPC_type = "civilian_weequay2";
			break;
		case 32:
			npc->NPC_type = "civilian_weequay3";
			break;
		case 33:
			npc->NPC_type = "civilian_weequay4";
			break;
		case 34:
			npc->NPC_type = "civilian_r5d2";
			break;
		case 35:
			npc->NPC_type = "civilian_r5d2";
			break;
		default:
			npc->NPC_type = "civilian_r5d2";
			break;
		}

		VectorCopy(gWPArray[waypoint]->origin, npc->s.origin);
		npc->s.origin[2]+=32; // Drop down...

		npc->s.angles[PITCH] = 0;
		npc->s.angles[YAW] = irand(0,359);
		npc->s.angles[ROLL] = 0;

		trap->Print("[%i/%i] Spawning (civilian NPC) %s at waypoint %i.\n", botplayers+1, minplayers, npc->NPC_type, waypoint);

		SP_NPC_spawner( npc );
	}
}

/*
===============
G_CheckMinimumNpcs
===============
*/

int		NPC_SPAWN_TEAM = TEAM_RED;

extern vec3_t NPC_SPAWNPOINT;
extern int NPC_SPAWNFLAG;

extern void NPC_SelectWarzoneSpawnpoint ( int TEAM );
extern int GetNumberOfWarzoneFlags ( void );
extern int WARZONE_GetNumberOfBlueFlags();
extern int WARZONE_GetNumberOfRedFlags();

void G_CheckMinimumNpcs( void ) {
	int			minplayers;
	int			botplayers = 0, i;

	if (gWPNum <= 0)
	{
		return;
	}

	if (g_gametype.integer == GT_WARZONE)
	{
		if (GetNumberOfWarzoneFlags() <= 0
			|| WARZONE_GetNumberOfBlueFlags() <= 0
			|| WARZONE_GetNumberOfRedFlags() <= 0)
			return; // Wait for flags to be set up...
	}

	if (level.intermissiontime) return;

	//only check once each 1 seconds - NPCs can spawn faster then bots...
	if (level.numConnectedClients > 0 && checkminimumnpcs_time > level.time - 1000) {
		return;
	}

	checkminimumnpcs_time = level.time;
	trap->Cvar_Update(&npc_enemies);
	minplayers = npc_enemies.integer;

	// Add vendors...
	G_CheckVendorNPCs();
	
	// Add civilians...
	G_CheckCivilianNPCs();

	if (minplayers <= 0) return;

	if (minplayers > 512)
	{
		minplayers = 512;
	}

	for (i = level.maxclients; i < MAX_GENTITIES; i++)
	{
		gentity_t *npc = &g_entities[i];

		if (!npc) continue;
		if (npc->s.eType != ET_NPC) continue;
		if (npc->client->NPC_class == CLASS_CIVILIAN
			|| npc->client->NPC_class == CLASS_PRISONER) continue;

		botplayers++;
	}

	if (botplayers < minplayers)
	{
		gentity_t	*npc = NULL;
		int			waypoint = irand(0, gWPNum-1);
		int			random = irand(0,12);
		int			tries = 0;

		if (g_gametype.integer == GT_WARZONE)
		{// Who (which team) needs this NPC???
			int RED_NPCS = 0;
			int BLUE_NPCS = 0;

			for (i = level.maxclients; i < MAX_GENTITIES; i++)
			{
				gentity_t *npc = &g_entities[i];

				if (!npc) continue;
				if (npc->s.eType != ET_NPC) continue;
				if (npc->client->NPC_class == CLASS_CIVILIAN
					/*|| npc->client->NPC_class == CLASS_PRISONER*/) continue;

				if (npc->client->playerTeam == NPCTEAM_ENEMY)
					RED_NPCS++;
				else if (npc->client->playerTeam == NPCTEAM_PLAYER)
					BLUE_NPCS++;
			}

			if (RED_NPCS < BLUE_NPCS)
			{
				NPC_SPAWN_TEAM = TEAM_RED;
			}
			else if (RED_NPCS > BLUE_NPCS)
			{
				NPC_SPAWN_TEAM = TEAM_BLUE;
			}
			else // Equal... Spawn red...
			{
				NPC_SPAWN_TEAM = TEAM_RED;
			}
		}

		npc = G_Spawn();

		if (NPC_SPAWN_TEAM == TEAM_RED)
		{// Imperial NPCs...

			if (random >= 7)
			{
				npc->NPC_type = "stormtrooper";
			}
			if (random >= 6)
			{
				npc->NPC_type = "human_merc";
			}
			else if (random >= 5)
			{
				npc->NPC_type = "stofficer";
			}
			else if (random >= 4)
			{
				npc->NPC_type = "rockettrooper";
			}
			else if (random >= 3)
			{
				npc->NPC_type = "stcommander";
			}
			else if (random >= 2)
			{// Officers/Specials...
				int rnd2 = irand(0,7);
				switch (rnd2)
				{
				case 0:
					npc->NPC_type = "impofficer";
					break;
				case 1:
					npc->NPC_type = "impworker";
					break;
				/*case 2:
					npc->NPC_type = "human_merc";
					break;*/
				case 3:
					npc->NPC_type = "boba_fett";
					break;
				case 4:
					npc->NPC_type = "imperial";
					break;
				default:
					npc->NPC_type = "impworker";
					break;
				}
			}
			else if (random >= 1)
			{// Sith...
				int rnd2 = irand(0,18);
				switch (rnd2)
				{
				case 0:
					npc->NPC_type = "reborn_dual";
					break;
				case 1:
					npc->NPC_type = "reborn_new";
					break;
				case 2:
					npc->NPC_type = "reborn_staff";
					break;
				case 3:
					//npc->NPC_type = "reborn_twin";
					//break;
				case 4:
					//npc->NPC_type = "rebornacrobat";
					//break;
				case 5:
					npc->NPC_type = "rebornchiss";
					break;
				case 6:
					//npc->NPC_type = "rebornfencer";
					//break;
				case 7:
					//npc->NPC_type = "rebornforceuser";
					//break;
				case 8:
					npc->NPC_type = "rebornrodian";
					break;
				case 9:
					npc->NPC_type = "reborntrandoshan";
					break;
				case 10:
					npc->NPC_type = "rebornweequay";
					break;
				case 11:
					npc->NPC_type = "rebornboss";
					break;
				case 12:
					npc->NPC_type = "tavion";
					break;
				case 13:
					npc->NPC_type = "tavion_new";
					break;
				case 14:
					npc->NPC_type = "shadowtrooper";
					break;
				case 15:
					//npc->NPC_type = "saber_droid";
					//break;
				case 16:
					npc->NPC_type = "alora";
					break;
				case 17:
					npc->NPC_type = "alora_dual";
					break;
				default:
					npc->NPC_type = "reborn";
					break;
				}
			}
			else
			{
				npc->NPC_type = "stofficeralt";
			}

			// Next NPC spawns as rebel...
			if ( g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_WARZONE )
				NPC_SPAWN_TEAM = TEAM_BLUE;
		}
		else
		{// Rebel NPCs...
			if (random >= 8)
			{
				npc->NPC_type = "rebel";
			}
			else if (random >= 5)
			{
				npc->NPC_type = "rebel2";
			}
			else if (random >= 3)
			{
				npc->NPC_type = "prisoner";
			}
			else if (random >= 2)
			{// Officers/Specials...
				int rnd2 = irand(0,7);
				switch (rnd2)
				{
				case 0:
					npc->NPC_type = "jan";
					break;
				case 1:
					npc->NPC_type = "lando";
					break;
				case 2:
					npc->NPC_type = "chewie";
					break;
				case 3:
					npc->NPC_type = "bespincop";
					break;
				case 4:
					npc->NPC_type = "bespincop";
					break;
				case 5:
					npc->NPC_type = "bespincop";
					break;
				default:
					npc->NPC_type = "bespincop2";
					break;
				}
			}
			else if (random >= 1)
			{// Jedi...
				int rnd2 = irand(0,18);
				switch (rnd2)
				{
				case 0:
					npc->NPC_type = "jedi";
					break;
				case 1:
					npc->NPC_type = "jedi2";
					break;
				case 2:
					npc->NPC_type = "jedi_hf1";
					break;
				case 3:
					npc->NPC_type = "jedi_hf2";
					break;
				case 4:
					npc->NPC_type = "jedi_hm1";
					break;
				case 5:
					npc->NPC_type = "jedi_hm2";
					break;
				case 6:
					npc->NPC_type = "jedi_kdm1";
					break;
				case 7:
					npc->NPC_type = "jedi_kdm2";
					break;
				case 8:
					npc->NPC_type = "jedi_rm1";
					break;
				case 9:
					npc->NPC_type = "jedi_rm2";
					break;
				case 10:
					npc->NPC_type = "jedi_tf1";
					break;
				case 11:
					npc->NPC_type = "jedi_tf2";
					break;
				case 12:
					npc->NPC_type = "jedi_zf1";
					break;
				case 13:
					npc->NPC_type = "jedi_zf2";
					break;
				case 14:
					npc->NPC_type = "JediF";
					break;
				case 15:
					npc->NPC_type = "JediMaster";
					break;
				case 16:
					npc->NPC_type = "JediTrainer";
					break;
				case 17:
					npc->NPC_type = "Kyle_boss";
					break;
				default:
					npc->NPC_type = "Luke";
					break;
				}
			}
			else
			{
				npc->NPC_type = "prisoner2";
			}

			// Next NPC spawns as imperial...
			if ( g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_WARZONE )
				NPC_SPAWN_TEAM = TEAM_RED;
		}
		
		if (g_gametype.integer == GT_WARZONE)
		{
			NPC_SelectWarzoneSpawnpoint( NPC_SPAWN_TEAM );

			npc->s.teamowner = NPC_SPAWN_TEAM;

			if (NPC_SPAWNPOINT[0] == 0 && NPC_SPAWNPOINT[1] == 0 && NPC_SPAWNPOINT[2] == 0)
			{// Bad spot returned... Fallback to normal waypoint spawn...
				while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) /*|| !JKG_CheckRoutingFrom( waypoint )*/)
				{
					gWPArray[waypoint]->inuse = qfalse; // set it bad!

					if (tries > 10)
					{
						return; // Try again on next check...
					}

					// Find a new one... This is probably a bad waypoint...
					waypoint = irand(0, gWPNum-1);
					tries++;
				}

				VectorCopy(gWPArray[waypoint]->origin, npc->s.origin);
				npc->s.origin[2]+=32; // Drop down...

				trap->Print("[%i/%i] Spawning (warzone NPC) %s at (fallback) waypoint %i.\n", botplayers+1, minplayers, npc->NPC_type, waypoint);
			}
			else
			{// Found a good warzone spawnpoint... Using it...
				VectorCopy(NPC_SPAWNPOINT, npc->s.origin);
				npc->s.origin[2]+=32; // Drop down...

				trap->Print("[%i/%i] Spawning (warzone NPC) %s at flag %i.\n", botplayers+1, minplayers, npc->NPC_type, NPC_SPAWNFLAG);
			}

			// Update ticket counts...
			if (NPC_SPAWN_TEAM == TEAM_RED)
				redtickets--;
			else if (NPC_SPAWN_TEAM == TEAM_BLUE)
				bluetickets--;

			trap->SendServerCommand( -1, va("tkt %i %i", redtickets, bluetickets ));
		}
		else
		{
			while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) /*|| !JKG_CheckRoutingFrom( waypoint )*/)
			{
				gWPArray[waypoint]->inuse = qfalse; // set it bad!

				if (tries > 10)
				{
					return; // Try again on next check...
				}

				// Find a new one... This is probably a bad waypoint...
				waypoint = irand(0, gWPNum-1);
				tries++;
			}

			VectorCopy(gWPArray[waypoint]->origin, npc->s.origin);
			npc->s.origin[2]+=32; // Drop down...

			trap->Print("[%i/%i] Spawning (enemy NPC) %s at waypoint %i.\n", botplayers+1, minplayers, npc->NPC_type, waypoint);
		}

		npc->s.angles[PITCH] = 0;
		npc->s.angles[YAW] = irand(0,359);
		npc->s.angles[ROLL] = 0;
		
		SP_NPC_spawner( npc );
	}
}
#endif //__NPC_MINPLAYERS__

/*
===============
G_CheckBotSpawn
===============
*/
void G_CheckBotSpawn( void ) {
	int		n;

#ifdef __NPC_MINPLAYERS__
	G_CheckMinimumNpcs();
#endif //__NPC_MINPLAYERS__

	G_CheckMinimumPlayers();

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			continue;
		}
		if ( botSpawnQueue[n].spawnTime > level.time ) {
			continue;
		}
		ClientBegin( botSpawnQueue[n].clientNum, qfalse );
		botSpawnQueue[n].spawnTime = 0;
	}
}

/*
===============
AddBotToSpawnQueue
===============
*/
static void AddBotToSpawnQueue( int clientNum, int delay ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( !botSpawnQueue[n].spawnTime ) {
			botSpawnQueue[n].spawnTime = level.time + delay;
			botSpawnQueue[n].clientNum = clientNum;
			return;
		}
	}

	trap->Print( S_COLOR_YELLOW "Unable to delay spawn\n" );
	ClientBegin( clientNum, qfalse );
}

/*
===============
G_RemoveQueuedBotBegin

Called on client disconnect to make sure the delayed spawn
doesn't happen on a freed index
===============
*/
void G_RemoveQueuedBotBegin( int clientNum ) {
	int		n;

	for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ ) {
		if( botSpawnQueue[n].clientNum == clientNum ) {
			botSpawnQueue[n].spawnTime = 0;
			return;
		}
	}
}

/*
===============
G_BotConnect
===============
*/
qboolean G_BotConnect( int clientNum, qboolean restart ) {
	bot_settings_t	settings;
	char			userinfo[MAX_INFO_STRING];

	trap->GetUserinfo( clientNum, userinfo, sizeof(userinfo) );

	Q_strncpyz( settings.personalityfile, Info_ValueForKey( userinfo, "personality" ), sizeof(settings.personalityfile) );
	settings.skill = atof( Info_ValueForKey( userinfo, "skill" ) );
	Q_strncpyz( settings.team, Info_ValueForKey( userinfo, "team" ), sizeof(settings.team) );

	if (!BotAISetupClient( clientNum, &settings, restart )) {
		trap->DropClient( clientNum, "BotAISetupClient failed" );
		return qfalse;
	}

	return qtrue;
}

/*
===============
G_AddBot
===============
*/
static void G_AddBot( const char *name, float skill, const char *team, int delay, char *altname) {
	int				clientNum;
	char			*botinfo;
	gentity_t		*bot;
	char			*key;
	char			*s;
	char			*botname;
	char			*model;
	//	char			*headmodel;
	char			userinfo[MAX_INFO_STRING];
	int				preTeam = 0;

	clientNum = trap->BotAllocateClient();
	if ( clientNum == -1 ) {
//		trap->Print( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
//		trap->Print( S_COLOR_RED "Start server with more 'open' slots.\n" );
		trap->SendServerCommand( -1, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "UNABLE_TO_ADD_BOT")));
		return;
	}

	// get the botinfo from bots.txt
	botinfo = G_GetBotInfoByName( name );
	if ( !botinfo ) {
		trap->Print( S_COLOR_RED "Error: Bot '%s' not defined\n", name );
		trap->BotFreeClient( clientNum );
		return;
	}

	// create the bot's userinfo
	userinfo[0] = '\0';

	botname = Info_ValueForKey( botinfo, "funname" );
	if( !botname[0] ) {
		botname = Info_ValueForKey( botinfo, "name" );
	}
	// check for an alternative name
	if (altname && altname[0]) {
		botname = altname;
	}
	Info_SetValueForKey( userinfo, "name", botname );
	Info_SetValueForKey( userinfo, "rate", "25000" );
	Info_SetValueForKey( userinfo, "snaps", "20" );
	Info_SetValueForKey( userinfo, "skill", va("%1.2f", skill) );

	key = "model";
	model = Info_ValueForKey( botinfo, key );
	if ( !*model ) {
		model = "kyle/default";
	}
	Info_SetValueForKey( userinfo, key, model );

	/*	key = "headmodel";
	headmodel = Info_ValueForKey( botinfo, key );
	if ( !*headmodel ) {
	headmodel = model;
	}
	Info_SetValueForKey( userinfo, key, headmodel );
	key = "team_headmodel";
	Info_SetValueForKey( userinfo, key, headmodel );
	*/
	key = "gender";
	s = Info_ValueForKey( botinfo, key );
	if ( !*s ) {
		s = "male";
	}
	Info_SetValueForKey( userinfo, "sex", s );

	s = Info_ValueForKey(botinfo, "personality");
	if (!*s )
	{
		Info_SetValueForKey( userinfo, "personality", "botfiles/default.jkb" );
	}
	else
	{
		Info_SetValueForKey( userinfo, "personality", s );
	}

	// have the server allocate a client slot
	clientNum = trap->BotAllocateClient();
	if ( clientNum == -1 ) {
		//		trap->Print( S_COLOR_RED "Unable to add bot.  All player slots are in use.\n" );
		//		trap->Print( S_COLOR_RED "Start server with more 'open' slots.\n" );
		trap->SendServerCommand( -1, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "UNABLE_TO_ADD_BOT")));
		return;
	}

	// initialize the bot settings
	if( !team || !*team ) {
		if ( level.gametype >= GT_TEAM ) {
			if ( PickTeam( clientNum ) == TEAM_RED )
				team = "red";
			else
				team = "blue";
		}
		else
			team = "red";
	}
	//	Info_SetValueForKey( userinfo, "characterfile", Info_ValueForKey( botinfo, "aifile" ) );
	Info_SetValueForKey( userinfo, "skill", va( "%5.2f", skill ) );
	Info_SetValueForKey( userinfo, "team", team );

	bot = &g_entities[ clientNum ];
	bot->r.svFlags |= SVF_BOT;
	bot->inuse = qtrue;

	// register the userinfo
	trap->SetUserinfo( clientNum, userinfo );

	if ( level.gametype >= GT_TEAM )
	{
		if ( team && !Q_stricmp( team, "red" ) )
			bot->client->sess.sessionTeam = TEAM_RED;
		else if ( team && !Q_stricmp( team, "blue" ) )
			bot->client->sess.sessionTeam = TEAM_BLUE;
		else
			bot->client->sess.sessionTeam = PickTeam( -1 );
	}

	preTeam = bot->client->sess.sessionTeam;

	// have it connect to the game as a normal client
	if ( ClientConnect( clientNum, qtrue, qtrue ) ) {

		//
		// UQ1: Added - sound directories for talking...
		//
		strcpy(bot->client->botSoundDir, Info_ValueForKey( botinfo, "name" ));

		if (StringContainsWord(bot->client->botSoundDir, "stormtrooper"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "st1");
			else
				strcpy(bot->client->botSoundDir, "st2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "stofficer"))
		{
			strcpy(bot->client->botSoundDir, "stofficer1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "cultist"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "cultist1");
			else if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "cultist2");
			else
				strcpy(bot->client->botSoundDir, "cultist3");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "gran"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "gran1");
			else
				strcpy(bot->client->botSoundDir, "gran2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "hazardtrooper"))
		{
			strcpy(bot->client->botSoundDir, "hazardtrooper1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "humanmerc"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "humanmerc1");
			else
				strcpy(bot->client->botSoundDir, "humanmerc2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "officer"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "io1");
			else
				strcpy(bot->client->botSoundDir, "io2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "jedi"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "jedi1");
			else
				strcpy(bot->client->botSoundDir, "jedi2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "merchant"))
		{
			strcpy(bot->client->botSoundDir, "merchant1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "noghri"))
		{
			strcpy(bot->client->botSoundDir, "noghri1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "prisoner"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "prisoner1");
			else
				strcpy(bot->client->botSoundDir, "prisoner2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rebel"))
		{
			strcpy(bot->client->botSoundDir, "rebel_pilot1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "reborn"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "reborn1");
			else
				strcpy(bot->client->botSoundDir, "reborn2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rocket"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rockettrooper_officer");
			else
				strcpy(bot->client->botSoundDir, "rockettrooper1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rodian"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rodian1");
			else
				strcpy(bot->client->botSoundDir, "rodian2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rosh"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rosh");
			else
				strcpy(bot->client->botSoundDir, "rosh_boss");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "trandoshan"))
		{
			strcpy(bot->client->botSoundDir, "trandoshan1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "worker"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "worker1");
			else
				strcpy(bot->client->botSoundDir, "worker2");
		}
		trap->Print("Bot sound dir set to [%s].\n", bot->client->botSoundDir);
		//
		// UQ1: END - sound directories for talking...
		//

		key = "model";
		model = Info_ValueForKey( botinfo, key );
		if ( !*model ) {
			model = "kyle/default";
		}
		Info_SetValueForKey( userinfo, key, model );

		ClientUserinfoChanged( clientNum );

		return;
	}

	if (bot->client->sess.sessionTeam != preTeam)
	{
		trap->GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

		if (bot->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			bot->client->sess.sessionTeam = (team_t)preTeam;
		}

		if (bot->client->sess.sessionTeam == TEAM_RED)
		{
			team = "Red";
		}
		else
		{
			team = "Blue";
		}

		Info_SetValueForKey( userinfo, "team", team );

		trap->SetUserinfo( clientNum, userinfo );

		bot->client->ps.persistant[ PERS_TEAM ] = bot->client->sess.sessionTeam;

		G_ReadClientSessionData( bot->client );
		if ( !ClientUserinfoChanged( clientNum ) )
			return;
	}

	if (level.gametype == GT_DUEL ||
		level.gametype == GT_POWERDUEL)
	{
		int loners = 0;
		int doubles = 0;

		bot->client->sess.duelTeam = 0;
		G_PowerDuelCount(&loners, &doubles, qtrue);

		if (!doubles || loners > (doubles/2))
		{
			bot->client->sess.duelTeam = DUELTEAM_DOUBLE;
		}
		else
		{
			bot->client->sess.duelTeam = DUELTEAM_LONE;
		}

		bot->client->sess.sessionTeam = TEAM_SPECTATOR;
		SetTeam(bot, "s");
	}
	else
	{
		if( delay == 0 ) {
			ClientBegin( clientNum, qfalse );

			//
			// UQ1: Added - sound directories for talking...
			//
			strcpy(bot->client->botSoundDir, Info_ValueForKey( botinfo, "name" ));

			if (StringContainsWord(bot->client->botSoundDir, "stormtrooper"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "st1");
				else
					strcpy(bot->client->botSoundDir, "st2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "stofficer"))
			{
				strcpy(bot->client->botSoundDir, "stofficer1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "cultist"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "cultist1");
				else if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "cultist2");
				else
					strcpy(bot->client->botSoundDir, "cultist3");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "gran"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "gran1");
				else
					strcpy(bot->client->botSoundDir, "gran2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "hazardtrooper"))
			{
				strcpy(bot->client->botSoundDir, "hazardtrooper1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "humanmerc"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "humanmerc1");
				else
					strcpy(bot->client->botSoundDir, "humanmerc2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "officer"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "io1");
				else
					strcpy(bot->client->botSoundDir, "io2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "jedi"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "jedi1");
				else
					strcpy(bot->client->botSoundDir, "jedi2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "merchant"))
			{
				strcpy(bot->client->botSoundDir, "merchant1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "noghri"))
			{
				strcpy(bot->client->botSoundDir, "noghri1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "prisoner"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "prisoner1");
				else
					strcpy(bot->client->botSoundDir, "prisoner2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "rebel"))
			{
				strcpy(bot->client->botSoundDir, "rebel_pilot1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "reborn"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "reborn1");
				else
					strcpy(bot->client->botSoundDir, "reborn2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "rocket"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "rockettrooper_officer");
				else
					strcpy(bot->client->botSoundDir, "rockettrooper1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "rodian"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "rodian1");
				else
					strcpy(bot->client->botSoundDir, "rodian2");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "rosh"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "rosh");
				else
					strcpy(bot->client->botSoundDir, "rosh_boss");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "trandoshan"))
			{
				strcpy(bot->client->botSoundDir, "trandoshan1");
			}
			else if (StringContainsWord(bot->client->botSoundDir, "worker"))
			{
				if (irand(0,1) == 0)
					strcpy(bot->client->botSoundDir, "worker1");
				else
					strcpy(bot->client->botSoundDir, "worker2");
			}
			trap->Print("Bot sound dir set to [%s].\n", bot->client->botSoundDir);
			//
			// UQ1: END - sound directories for talking...
			//

			key = "model";
			model = Info_ValueForKey( botinfo, key );
			if ( !*model ) {
				model = "kyle/default";
			}
			Info_SetValueForKey( userinfo, key, model );

			ClientUserinfoChanged( clientNum );
			return;
		}

		AddBotToSpawnQueue( clientNum, delay );

		//
		// UQ1: Added - sound directories for talking...
		//
		strcpy(bot->client->botSoundDir, Info_ValueForKey( botinfo, "name" ));

		if (StringContainsWord(bot->client->botSoundDir, "stormtrooper"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "st1");
			else
				strcpy(bot->client->botSoundDir, "st2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "stofficer"))
		{
			strcpy(bot->client->botSoundDir, "stofficer1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "cultist"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "cultist1");
			else if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "cultist2");
			else
				strcpy(bot->client->botSoundDir, "cultist3");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "gran"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "gran1");
			else
				strcpy(bot->client->botSoundDir, "gran2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "hazardtrooper"))
		{
			strcpy(bot->client->botSoundDir, "hazardtrooper1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "humanmerc"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "humanmerc1");
			else
				strcpy(bot->client->botSoundDir, "humanmerc2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "officer"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "io1");
			else
				strcpy(bot->client->botSoundDir, "io2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "jedi"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "jedi1");
			else
				strcpy(bot->client->botSoundDir, "jedi2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "merchant"))
		{
			strcpy(bot->client->botSoundDir, "merchant1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "noghri"))
		{
			strcpy(bot->client->botSoundDir, "noghri1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "prisoner"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "prisoner1");
			else
				strcpy(bot->client->botSoundDir, "prisoner2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rebel"))
		{
			strcpy(bot->client->botSoundDir, "rebel_pilot1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "reborn"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "reborn1");
			else
				strcpy(bot->client->botSoundDir, "reborn2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rocket"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rockettrooper_officer");
			else
				strcpy(bot->client->botSoundDir, "rockettrooper1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rodian"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rodian1");
			else
				strcpy(bot->client->botSoundDir, "rodian2");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "rosh"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "rosh");
			else
				strcpy(bot->client->botSoundDir, "rosh_boss");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "trandoshan"))
		{
			strcpy(bot->client->botSoundDir, "trandoshan1");
		}
		else if (StringContainsWord(bot->client->botSoundDir, "worker"))
		{
			if (irand(0,1) == 0)
				strcpy(bot->client->botSoundDir, "worker1");
			else
				strcpy(bot->client->botSoundDir, "worker2");
		}
		trap->Print("Bot sound dir set to [%s].\n", bot->client->botSoundDir);
		//
		// UQ1: END - sound directories for talking...
		//

		key = "model";
		model = Info_ValueForKey( botinfo, key );
		if ( !*model ) {
			model = "kyle/default";
		}
		Info_SetValueForKey( userinfo, key, model );

		ClientUserinfoChanged( clientNum );
	}
}


/*
===============
Svcmd_AddBot_f
===============
*/
void Svcmd_AddBot_f( void ) {
	float			skill;
	int				delay;
	char			name[MAX_TOKEN_CHARS];
	char			altname[MAX_TOKEN_CHARS];
	char			string[MAX_TOKEN_CHARS];
	char			team[MAX_TOKEN_CHARS];

	// are bots enabled?
	if ( !trap->Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	// name
	trap->Argv( 1, name, sizeof( name ) );
	if ( !name[0] ) {
		trap->Print( "Usage: Addbot <botname> [skill 1-5] [team] [msec delay] [altname]\n" );
		return;
	}

	// skill
	trap->Argv( 2, string, sizeof( string ) );
	if ( !string[0] ) {
		skill = 4;
	}
	else {
		skill = atof( string );
	}

	// team
	trap->Argv( 3, team, sizeof( team ) );

	// delay
	trap->Argv( 4, string, sizeof( string ) );
	if ( !string[0] ) {
		delay = 0;
	}
	else {
		delay = atoi( string );
	}

	// alternative name
	trap->Argv( 5, altname, sizeof( altname ) );

	G_AddBot( name, skill, team, delay, altname );

	// if this was issued during gameplay and we are playing locally,
	// go ahead and load the bot's media immediately
	if ( level.time - level.startTime > 1000 &&
		trap->Cvar_VariableIntegerValue( "cl_running" ) ) {
		trap->SendServerCommand( -1, "loaddefered\n" );	// FIXME: spelled wrong, but not changing for demo
	}
}

/*
===============
Svcmd_BotList_f
===============
*/
void Svcmd_BotList_f( void ) {
	int i;
	char name[MAX_NETNAME];
	char funname[MAX_NETNAME];
	char model[MAX_QPATH];
	char personality[MAX_QPATH];

	trap->Print("name             model            personality              funname\n");
	for (i = 0; i < g_numBots; i++) {
		Q_strncpyz(name, Info_ValueForKey( g_botInfos[i], "name" ), sizeof( name ));
		if ( !*name ) {
			Q_strncpyz(name, "Mysterious Soldier", sizeof( name ));
		}
		Q_strncpyz(funname, Info_ValueForKey( g_botInfos[i], "funname"), sizeof( funname ));
		if ( !*funname ) {
			funname[0] = '\0';
		}
		Q_strncpyz(model, Info_ValueForKey( g_botInfos[i], "model" ), sizeof( model ));
		if ( !*model ) {
			Q_strncpyz(model, "kyle/default", sizeof( model ));
		}
		Q_strncpyz(personality, Info_ValueForKey( g_botInfos[i], "personality"), sizeof( personality ));
		if (!*personality ) {
			Q_strncpyz(personality, "botfiles/kyle.jkb", sizeof( personality ));
		}
		trap->Print("%-16s %-16s %-20s %-20s\n", name, model, COM_SkipPath(personality), funname);
	}
}

#if 0
/*
===============
G_SpawnBots
===============
*/
static void G_SpawnBots( char *botList, int baseDelay ) {
	char		*bot;
	char		*p;
	float		skill;
	int			delay;
	char		bots[MAX_INFO_VALUE];

	skill = trap->Cvar_VariableValue( "g_npcspskill" );
	if( skill < 1 ) {
		trap->Cvar_Set( "g_npcspskill", "1" );
		skill = 1;
	}
	else if ( skill > 5 ) {
		trap->Cvar_Set( "g_npcspskill", "5" );
		skill = 5;
	}

	Q_strncpyz( bots, botList, sizeof(bots) );
	p = &bots[0];
	delay = baseDelay;
	while( *p ) {
		//skip spaces
		while( *p && *p == ' ' ) {
			p++;
		}
		if( !*p ) {
			break;
		}

		// mark start of bot name
		bot = p;

		// skip until space of null
		while( *p && *p != ' ' ) {
			p++;
		}
		if( *p ) {
			*p++ = 0;
		}

		// we must add the bot this way, calling G_AddBot directly at this stage
		// does "Bad Things"
		trap->SendConsoleCommand( EXEC_INSERT, va("addbot \"%s\" %f free %i\n", bot, skill, delay) );

		delay += BOT_BEGIN_DELAY_INCREMENT;
	}
}
#endif

/*
===============
G_LoadBotsFromFile
===============
*/
static void G_LoadBotsFromFile( char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_BOTS_TEXT];

	len = trap->FS_Open( filename, &f, FS_READ );
	if ( !f ) {
		trap->Print( S_COLOR_RED "file not found: %s\n", filename );
		return;
	}
	if ( len >= MAX_BOTS_TEXT ) {
		trap->Print( S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_BOTS_TEXT );
		trap->FS_Close( f );
		return;
	}

	trap->FS_Read( buf, len, f );
	buf[len] = 0;
	trap->FS_Close( f );

	g_numBots += G_ParseInfos( buf, MAX_BOTS - g_numBots, &g_botInfos[g_numBots] );
}

/*
===============
G_LoadBots
===============
*/
static void G_LoadBots( void ) {
	vmCvar_t	botsFile;
	int			numdirs;
	char		filename[128];
	char		dirlist[1024];
	char*		dirptr;
	int			i;
	int			dirlen;

	if ( !trap->Cvar_VariableIntegerValue( "bot_enable" ) ) {
		return;
	}

	g_numBots = 0;

	trap->Cvar_Register( &botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM );
	if( *botsFile.string ) {
		G_LoadBotsFromFile(botsFile.string);
	}
	else {
		//G_LoadBotsFromFile("scripts/bots.txt");
		G_LoadBotsFromFile("botfiles/bots.txt");
	}

	// get all bots from .bot files
	numdirs = trap->FS_GetFileList("scripts", ".bot", dirlist, 1024 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		G_LoadBotsFromFile(filename);
	}
//	trap->Print( va( "%i bots parsed\n", g_numBots ) );
}

/*
===============
G_GetBotInfoByNumber
===============
*/
char *G_GetBotInfoByNumber( int num ) {
	if( num < 0 || num >= g_numBots ) {
		trap->Print( S_COLOR_RED "Invalid bot number: %i\n", num );
		return NULL;
	}
	return g_botInfos[num];
}

/*
===============
G_GetBotInfoByName
===============
*/
char *G_GetBotInfoByName( const char *name ) {
	int		n;
	char	*value;

	for ( n = 0; n < g_numBots ; n++ ) {
		value = Info_ValueForKey( g_botInfos[n], "name" );
		if ( !Q_stricmp( value, name ) ) {
			return g_botInfos[n];
		}
	}

	return NULL;
}

//rww - pd
void LoadPath_ThisLevel(void);
void WaypointsCleanup();
//end rww

/*
===============
G_InitBots
===============
*/
void G_InitBots( void ) {
	G_LoadBots();
	G_LoadArenas();

	trap->Cvar_Register( &bot_minplayers, "bot_minplayers", "0", CVAR_SERVERINFO | CVAR_ARCHIVE );

#ifdef __NPC_MINPLAYERS__
	trap->Cvar_Register( &npc_enemies, "npc_enemies", "0", CVAR_SERVERINFO | CVAR_ARCHIVE );
	trap->Cvar_Register( &npc_civilians, "npc_civilians", "0", CVAR_SERVERINFO | CVAR_ARCHIVE );
	trap->Cvar_Register( &npc_vendors, "npc_vendors", "0", CVAR_SERVERINFO | CVAR_ARCHIVE );
#endif //__NPC_MINPLAYERS__

	//rww - new bot route stuff
	LoadPath_ThisLevel();
	//end rww
}

void G_CleanupBots()
{
	WaypointsCleanup();
}
