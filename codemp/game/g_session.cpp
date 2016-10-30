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

#include "g_local.h"
#include <json/cJSON.h>

/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

// Jedi Knight Galaxies
// Alrighty, we're recoding this system completely.
// Instead of using cvars (lol wtf), we're gonna save the session data into a allocation :o
//clientSession_t *g_sess; //[MAX_CLIENTS] // Store session data in here

// Called on game shutdown
void G_WriteClientSessionData( const gclient_t *client ) {
	
	const clientSession_t *sess = &client->sess;
	cJSON *root;
	fileHandle_t f;
	char fileName[MAX_QPATH] = {};

	Com_sprintf( fileName, sizeof(fileName), "session/client%02i.json", (int)(client - level.clients) );
	Com_Printf( "Writing session file %s\n", fileName );
	root = cJSON_CreateObject();
	cJSON_AddIntegerToObject( root, "sessionTeam", sess->sessionTeam );
	cJSON_AddIntegerToObject( root, "spectatorNum", sess->spectatorNum );
	cJSON_AddIntegerToObject( root, "spectatorState", sess->spectatorState );
	cJSON_AddIntegerToObject( root, "spectatorClient", sess->spectatorClient );
	cJSON_AddIntegerToObject( root, "wins", sess->wins );
	cJSON_AddIntegerToObject( root, "losses", sess->losses );
	cJSON_AddIntegerToObject( root, "setForce", sess->setForce );
	cJSON_AddIntegerToObject( root, "saberLevel", sess->saberLevel );
	cJSON_AddIntegerToObject( root, "selectedFP", sess->selectedFP );
	cJSON_AddIntegerToObject( root, "updateUITime", sess->updateUITime );
	cJSON_AddIntegerToObject( root, "teamLeader", sess->teamLeader );
	cJSON_AddIntegerToObject( root, "duelTeam", sess->duelTeam );
	cJSON_AddIntegerToObject( root, "siegeDesiredTeam", sess->siegeDesiredTeam );
	cJSON_AddStringToObject( root, "siegeClass", *sess->siegeClass ? sess->siegeClass : "none" );
	cJSON_AddStringToObject( root, "saberType", *sess->saberType ? sess->saberType : "" );
	cJSON_AddStringToObject( root, "saber2Type", *sess->saber2Type ? sess->saber2Type : "" );
	cJSON_AddStringToObject( root, "IP", sess->IP );
	
	cJSON_AddIntegerToObject( root, "connTime", sess->connTime );
	cJSON_AddIntegerToObject( root, "noq3fill", sess->noq3fill );
	cJSON_AddIntegerToObject( root, "validated", sess->validated );
	cJSON_AddIntegerToObject( root, "adminRank", sess->adminRank );
	cJSON_AddIntegerToObject( root, "canUseCheats", sess->canUseCheats ? 1 : 0 );
	
	trap->FS_Open( fileName, &f, FS_WRITE );

	Q_FSWriteJSON( root, f );
}

// Called on a reconnect
void G_ReadClientSessionData( gclient_t *client ) {
	clientSession_t *sess = &client->sess;
	cJSON *root = NULL, *object = NULL;
	char fileName[MAX_QPATH] = {};
	char *buffer = NULL;
	fileHandle_t f = NULL_FILE;
	unsigned int len = 0;
	const char *tmp = NULL;
	char jsonError[MAX_STRING_CHARS] = {};

	Com_sprintf( fileName, sizeof(fileName), "session/client%02i.json", client - level.clients );
	len = trap->FS_Open( fileName, &f, FS_READ );

	// no file
	if ( !f || !len || len == -1 ) {
		trap->FS_Close( f );
		return;
	}

	buffer = (char *)malloc( len + 1 );
	if ( !buffer ) {
		return;
	}

	trap->FS_Read( buffer, len, f );
	trap->FS_Close( f );
	buffer[len] = '\0';

	// read buffer
	root = cJSON_Parse( buffer, jsonError, sizeof(jsonError) );
	free( buffer );

	if ( !root ) {
		Com_Printf( "G_ReadSessionData(%02i): could not parse session data\n", client - level.clients );
		return;
	}

	if ( (object = cJSON_GetObjectItem( root, "sessionTeam" )) ) {
		sess->sessionTeam = (team_t)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "spectatorNum" )) ) {
		sess->spectatorNum = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "spectatorState" )) ) {
		sess->spectatorState = (spectatorState_t)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "spectatorClient" )) ) {
		sess->spectatorClient = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "wins" )) ) {
		sess->wins = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "losses" )) ) {
		sess->losses = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "setForce" )) ) {
		sess->setForce = (qboolean)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "saberLevel" )) ) {
		sess->saberLevel = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "selectedFP" )) ) {
		sess->selectedFP = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "updateUITime" )) ) {
		sess->updateUITime = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "teamLeader" )) ) {
		sess->teamLeader = (qboolean)cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "duelTeam" )) ) {
		sess->duelTeam = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "siegeDesiredTeam" )) ) {
		sess->siegeDesiredTeam = cJSON_ToInteger( object );
	}

	if ( (object = cJSON_GetObjectItem( root, "siegeClass" )) ) {
		if ( (tmp = cJSON_ToString( object )) ) {
			Q_strncpyz( sess->siegeClass, tmp, sizeof(sess->siegeClass) );
		}
	}
	if ( (object = cJSON_GetObjectItem( root, "saberType" )) ) {
		if ( (tmp = cJSON_ToString( object )) ) {
			Q_strncpyz( sess->saberType, tmp, sizeof(sess->saberType) );
		}
	}
	if ( (object = cJSON_GetObjectItem( root, "saber2Type" )) ) {
		if ( (tmp = cJSON_ToString( object )) ) {
			Q_strncpyz( sess->saber2Type, tmp, sizeof(sess->saber2Type) );
		}
	}
	if ( (object = cJSON_GetObjectItem( root, "IP" )) ) {
		if ( (tmp = cJSON_ToString( object )) ) {
			Q_strncpyz( sess->IP, tmp, sizeof(sess->IP) );
		}
	}
	if ( (object = cJSON_GetObjectItem( root, "connTime" )) ) {
		sess->connTime = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "noq3fill" )) ) {
		sess->noq3fill = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "validated" )) ) {
		sess->validated = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "adminRank" )) ) {
		sess->adminRank = cJSON_ToInteger( object );
	}
	if ( (object = cJSON_GetObjectItem( root, "canUseCheats" )) ) {
		sess->canUseCheats = !!cJSON_ToInteger( object );
	}

	client->ps.fd.saberAnimLevel = client->sess.saberLevel;
	client->ps.fd.saberDrawAnimLevel = client->sess.saberLevel;
	client->ps.fd.forcePowerSelected = client->sess.selectedFP;

	cJSON_Delete( root );
	root = NULL;
}

// Called on a first-time connect
void G_InitClientSessionData( gclient_t *client, char *userinfo, qboolean isBot ) {
	clientSession_t	*sess = &client->sess;
	const char		*value;

	client->sess.siegeDesiredTeam = TEAM_FREE;

	// initial team determination
	if ( level.gametype >= GT_TEAM ) {
		if ( g_teamAutoJoin.integer && !(g_entities[client-level.clients].r.svFlags & SVF_BOT) ) {
			sess->sessionTeam = PickTeam( -1 );
			client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right
		} else {
			// always spawn as spectator in team games
			if (!isBot)
			{
				sess->sessionTeam = TEAM_SPECTATOR;	
			}
			else
			{ //Bots choose their team on creation
				value = Info_ValueForKey( userinfo, "team" );
				if (value[0] == 'r' || value[0] == 'R')
				{
					sess->sessionTeam = TEAM_RED;
				}
				else if (value[0] == 'b' || value[0] == 'B')
				{
					sess->sessionTeam = TEAM_BLUE;
				}
				else
				{
					sess->sessionTeam = PickTeam( -1 );
				}
				client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right
			}
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( level.gametype ) {
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_DUEL:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_POWERDUEL:
				//sess->duelTeam = DUELTEAM_LONE; //default
				{
					int loners = 0;
					int doubles = 0;

					G_PowerDuelCount(&loners, &doubles, qtrue);

					if (!doubles || loners > (doubles/2))
					{
						sess->duelTeam = DUELTEAM_DOUBLE;
					}
					else
					{
						sess->duelTeam = DUELTEAM_LONE;
					}
				}
				sess->sessionTeam = TEAM_SPECTATOR;
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	AddTournamentQueue(client);

	sess->siegeClass[0] = 0;

	G_WriteClientSessionData( client );
}

static const char *metaFileName = "session/meta.json";

void G_ReadSessionData( void ) {
	char *buffer = NULL;
	fileHandle_t f = NULL_FILE;
	unsigned int len = 0u;
	cJSON *root;
	char jsonError[MAX_STRING_CHARS] = {};

	trap->Print( "G_ReadSessionData: reading %s...", metaFileName );
	len = trap->FS_Open( metaFileName, &f, FS_READ );

	// no file
	if ( !f || !len || len == -1 ) {
		trap->Print( "failed to open file, clearing session data...\n" );
		level.newSession = qtrue;
		return;
	}

	buffer = (char *)malloc( len + 1 );
	if ( !buffer ) {
		trap->Print( "failed to allocate buffer, clearing session data...\n" );
		level.newSession = qtrue;
		return;
	}

	trap->FS_Read( buffer, len, f );
	trap->FS_Close( f );
	buffer[len] = '\0';

	// read buffer
	root = cJSON_Parse( buffer, jsonError, sizeof(jsonError) );

	// if the gametype changed since the last session, don't use any client sessions
	if ( level.gametype != cJSON_ToInteger( cJSON_GetObjectItem( root, "gametype" ) ) ) {
		level.newSession = qtrue;
		trap->Print( "gametype changed, clearing session data..." );
	}

	free( buffer );
	cJSON_Delete( root );
	root = NULL;
	trap->Print( "done\n" );
}

void G_WriteSessionData( void ) {
	int i;
	fileHandle_t f;
	const gclient_t *client = NULL;
	cJSON *root = cJSON_CreateObject();

	cJSON_AddIntegerToObject( root, "gametype", level.gametype );

	trap->Print( "G_WriteSessionData: writing %s...", metaFileName );
	trap->FS_Open( metaFileName, &f, FS_WRITE );

	Q_FSWriteJSON( root, f );

	for ( i = 0, client = level.clients; i < level.maxclients; i++, client++ ) {
		if ( client->pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( client );
		}
	}

	trap->Print( "done\n" );
}
