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
#include "ghoul2/G2.h"
#include "bg_saga.h"

// GLua include
#include "../GLua/glua.h"
#include "jkg_bans.h"
#include "jkg_damageareas.h"
#include "jkg_utilityfunc.h"
#include "qcommon/game_version.h"

extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int gWPNum;

// Warzone...
extern void Calculate_Warzone_Flag_Spawns ( void );
extern gentity_t *SelectWarzoneSpawnpoint ( gentity_t *ent );

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
static vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

extern int g_siegeRespawnCheck;

void WP_SaberAddG2Model( gentity_t *saberent, const char *saberModel, qhandle_t saberSkin );
void WP_SaberRemoveG2Model( gentity_t *saberent );
extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );

forcedata_t Client_Force[MAX_CLIENTS];

/*QUAKED info_player_duel (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for duelists in duel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_duel1 (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for lone duelists in powerduel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel1( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_duel2 (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for paired duelists in powerduel.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_duel2( gentity_t *ent )
{
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
Targets will be fired when someone spawns in on them.
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_start_red (1 0 0) (-16 -16 -24) (16 16 32) INITIAL
For Red Team DM starts

Targets will be fired when someone spawns in on them.
equivalent to info_player_deathmatch

INITIAL - The first time a player enters the game, they will be at an 'initial' spot.

"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_start_red(gentity_t *ent) {
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_start_blue (1 0 0) (-16 -16 -24) (16 16 32) INITIAL
For Blue Team DM starts

Targets will be fired when someone spawns in on them.
equivalent to info_player_deathmatch

INITIAL - The first time a player enters the game, they will be at an 'initial' spot.

"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_start_blue(gentity_t *ent) {
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32) RED BLUE
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
RED - In a Siege game, the intermission will happen here if the Red (attacking) team wins
BLUE - In a Siege game, the intermission will happen here if the Blue (defending) team wins
*/
void SP_info_player_intermission( gentity_t *ent ) {

}

/*QUAKED info_player_intermission_red (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.

In a Siege game, the intermission will happen here if the Red (attacking) team wins
target - ent to look at
target2 - ents to use when this intermission point is chosen
*/
void SP_info_player_intermission_red( gentity_t *ent ) {

}

/*QUAKED info_player_intermission_blue (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.

In a Siege game, the intermission will happen here if the Blue (defending) team wins
target - ent to look at
target2 - ents to use when this intermission point is chosen
*/
void SP_info_player_intermission_blue( gentity_t *ent ) {

}

/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client) {
			return qtrue;
		}

	}

	return qfalse;
}

qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest ) 
{
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( dest, mover->r.mins, mins );
	VectorAdd( dest, mover->r.maxs, maxs );
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) 
	{
		hit = &g_entities[touch[i]];
		if ( hit == mover )
		{
			continue;
		}

		if ( hit->r.contents & mover->r.contents )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[64];
	gentity_t	*list_spot[64];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	//in Team DM, look for a team start spot first, if any
	if ( level.gametype >= GT_TEAM 
		&& team != TEAM_FREE 
		&& team != TEAM_SPECTATOR )
	{
		char *classname = NULL;
		int uMon = 0;
		if ( team == TEAM_RED )
		{
			classname = "info_player_start_red";
		}
		else
		{
			classname = "info_player_start_blue";
		}
veryNastyHackHere:
		while ((spot = G_Find (spot, FOFS(classname), classname)) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}

			if(((spot->flags & FL_NO_BOTS) && isbot) ||
			   ((spot->flags & FL_NO_HUMANS) && !isbot))
			{
				// spot is not for this human/bot player
				continue;
			}

			VectorSubtract( spot->s.origin, avoidPoint, delta );
			dist = VectorLength( delta );
			for (i = 0; i < numSpots; i++) {
				if ( dist > list_dist[i] ) {
					if ( numSpots >= 64 )
						numSpots = 64-1;
					for (j = numSpots; j > i; j--) {
						list_dist[j] = list_dist[j-1];
						list_spot[j] = list_spot[j-1];
					}
					list_dist[i] = dist;
					list_spot[i] = spot;
					numSpots++;
					if (numSpots > 64)
						numSpots = 64;
					break;
				}
			}
			if (i >= numSpots && numSpots < 64) {
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
		uMon++;
		if(uMon <= 1)
		{
			if ( team == TEAM_RED )
			{
				classname = "team_ctf_redplayer";
			}
			else
			{
				classname = "team_ctf_blueplayer";
			}
			goto veryNastyHackHere;
		}
	}

	if ( !numSpots )
	{//couldn't find any of the above
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
			if ( SpotWouldTelefrag( spot ) ) {
				continue;
			}
			VectorSubtract( spot->s.origin, avoidPoint, delta );
			dist = VectorLength( delta );
			for (i = 0; i < numSpots; i++) {
				if ( dist > list_dist[i] ) {
					if ( numSpots >= 64 )
						numSpots = 64-1;
					for (j = numSpots; j > i; j--) {
						list_dist[j] = list_dist[j-1];
						list_spot[j] = list_spot[j-1];
					}
					list_dist[i] = dist;
					list_spot[i] = spot;
					numSpots++;
					if (numSpots > 64)
						numSpots = 64;
					break;
				}
			}
			if (i >= numSpots && numSpots < 64) {
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
		if (!numSpots) {
			spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
			if (!spot)
			{
				trap->Error( ERR_DROP, "Couldn't find a spawn point" );
				return NULL;
			}
			VectorCopy (spot->s.origin, origin);
			origin[2] += 9;
			VectorCopy (spot->s.angles, angles);
			return spot;
		}
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

gentity_t *SelectDuelSpawnPoint( int team, vec3_t avoidPoint, vec3_t origin, vec3_t angles )
{
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[64];
	gentity_t	*list_spot[64];
	int			numSpots, rnd, i, j;
	char		*spotName;

	if (team == DUELTEAM_LONE)
	{
		spotName = "info_player_duel1";
	}
	else if (team == DUELTEAM_DOUBLE)
	{
		spotName = "info_player_duel2";
	}
	else if (team == DUELTEAM_SINGLE)
	{
		spotName = "info_player_duel";
	}
	else
	{
		spotName = "info_player_deathmatch";
	}
tryAgain:

	numSpots = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), spotName)) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );
		for (i = 0; i < numSpots; i++) {
			if ( dist > list_dist[i] ) {
				if ( numSpots >= 64 )
					numSpots = 64-1;
				for (j = numSpots; j > i; j--) {
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				if (numSpots > 64)
					numSpots = 64;
				break;
			}
		}
		if (i >= numSpots && numSpots < 64) {
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if (!numSpots)
	{
		if (Q_stricmp(spotName, "info_player_deathmatch"))
		{ //try the loop again with info_player_deathmatch as the target if we couldn't find a duel spot
			spotName = "info_player_deathmatch";
			goto tryAgain;
		}

		//If we got here we found no free duel or DM spots, just try the first DM spot
		spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			trap->Error( ERR_DROP, "Couldn't find a spawn point" );
		VectorCopy (spot->s.origin, origin);
		origin[2] += 9;
		VectorCopy (spot->s.angles, angles);
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy (list_spot[rnd]->s.origin, origin);
	origin[2] += 9;
	VectorCopy (list_spot[rnd]->s.angles, angles);

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, team, isbot );

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}		
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles, team_t team, qboolean isbot ) {
	gentity_t	*spot;

	spot = NULL;
	if(level.gametype >= GT_TEAM)
	{
		if(team == TEAM_RED)
		{
			while ((spot = G_Find (spot, FOFS(classname), "team_ctf_redplayer")) != NULL) {
				if ( spot->spawnflags & 1 ) {
					break;
				}
			}
			if ( spot && !SpotWouldTelefrag( spot ) ) {
				return spot;
			}
		}
		else
		{
			while ((spot = G_Find (spot, FOFS(classname), "team_ctf_blueplayer")) != NULL) {
				if ( spot->spawnflags & 1 ) {
					break;
				}
			}
			if ( spot && !SpotWouldTelefrag( spot ) ) {
				return spot;
			}
		}
	}
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( spot->spawnflags & 1 ) {
			break;
		}
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( vec3_origin, origin, angles, team, isbot );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

#define BODY_SINK_TIME		30000//45000

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > BODY_SINK_TIME + 2500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap->UnlinkEntity( (sharedEntity_t *)ent );
		ent->physicsObject = qfalse;
		return;	
	}
//	ent->nextthink = level.time + 100;
//	ent->s.pos.trBase[2] -= 1;

	G_AddEvent(ent, EV_BODYFADE, 0);
	ent->nextthink = level.time + 18000;
	ent->takedamage = qfalse;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
static qboolean CopyToBodyQue( gentity_t *ent ) {
	gentity_t		*body;
	int			contents;
	int			islight = 0;

	if (level.intermissiontime)
	{
		return qfalse;
	}

	trap->UnlinkEntity ((sharedEntity_t *)ent);

	// if client is in a nodrop area, don't leave the body
	contents = trap->PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return qfalse;
	}

	if (ent->client && (ent->client->ps.eFlags & EF_DISINTEGRATION))
	{ //for now, just don't spawn a body if you got disint'd
		return qfalse;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap->UnlinkEntity ((sharedEntity_t *)body);
	body->s = ent->s;

	//avoid oddly angled corpses floating around
	body->s.angles[PITCH] = body->s.angles[ROLL] = body->s.apos.trBase[PITCH] = body->s.apos.trBase[ROLL] = 0;

	body->s.g2radius = 100;

	body->s.eType = ET_BODY;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc

	if (ent->client && (ent->client->ps.eFlags & EF_DISINTEGRATION))
	{
		body->s.eFlags |= EF_DISINTEGRATION;
	}

	if(ent->client)
	{
		VectorCopy(ent->client->ps.lastHitLoc, body->s.origin2);
	}

	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.loopIsSoundset = qfalse;
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	body->s.weapon = ent->s.bolt2;

	if (body->s.weapon == WP_SABER && ent->client->ps.saberInFlight)
	{
		body->s.weapon = WP_BLASTER; //lie to keep from putting a saber on the corpse, because it was thrown at death
	}

	//G_AddEvent(body, EV_BODY_QUEUE_COPY, ent->s.clientNum);
	//Now doing this through a modified version of the rcg reliable command.
	if (ent->client && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
	{
		islight = 1;
	}

	trap->SendServerCommand(-1, va("ircg %i %i %i %i %i", ent->s.number, body->s.number, body->s.weapon, body->s.weaponVariation, islight));

	body->r.svFlags = ent->r.svFlags | SVF_BROADCAST;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->s.torsoAnim = body->s.legsAnim = ent->client->ps.legsAnim;

	body->s.customRGBA[0] = ent->client->ps.customRGBA[0];
	body->s.customRGBA[1] = ent->client->ps.customRGBA[1];
	body->s.customRGBA[2] = ent->client->ps.customRGBA[2];
	body->s.customRGBA[3] = ent->client->ps.customRGBA[3];

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + BODY_SINK_TIME;
	body->think = BodySink;


	body->die = body_die;

	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}

	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap->LinkEntity ((sharedEntity_t *)body);

	return qtrue;
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

void MaintainBodyQueue(gentity_t *ent)
{ //do whatever should be done taking ragdoll and dismemberment states into account.
	qboolean doRCG = qfalse;

	if (ent->client->tempSpectate > level.time)
	{
		ent->client->noCorpse = qtrue;
	}

	if (!ent->client->noCorpse && !ent->client->ps.fallingToDeath)
	{
		if (!CopyToBodyQue (ent))
		{
			doRCG = qtrue;
		}
	}
	else
	{
		ent->client->noCorpse = qfalse; //clear it for next time
		ent->client->ps.fallingToDeath = qfalse;
		doRCG = qtrue;
	}

	if (doRCG)
	{ //bodyque func didn't manage to call ircg so call this to assure our limbs and ragdoll states are proper on the client.
		trap->SendServerCommand(-1, va("rcg %i", ent->s.clientNum));
	}
}


void JKG_PermaSpectate(gentity_t *ent)
{
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		ent->client->tempSpectate = Q3_TIMEINFINITE;
		ent->health = ent->client->ps.stats[STAT_HEALTH] = 1;
		ent->flags |= FL_NOTARGET;	// So NPC's wont see you
		ent->client->ps.weapon = WP_NONE;
		ent->client->ps.weaponVariation = 0;
		ent->client->ps.stats[STAT_WEAPONS] = 0;
		ent->takedamage = qfalse;
		//trap->LinkEntity(ent);
	}
}

qboolean JKG_ClientAlive(gentity_t* ent)
{
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		return qfalse;
	}

	if (ent->client->tempSpectate > level.time) {
		return qfalse;
	}

	if (ent->health <= 0 || ent->client->ps.stats[STAT_HEALTH] <= 0) {
		return qfalse;
	}

	if (ent->client->deathcamTime && level.time > ent->client->deathcamTime) {
		return qfalse;
	}

	return qtrue;
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	gentity_t	*tent;
	MaintainBodyQueue(ent);

	trap->UnlinkEntity ((sharedEntity_t *)ent);
		
	ClientSpawn(ent, qtrue);

	// add a teleportation effect
	tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	tent->s.clientNum = ent->s.clientNum;
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return (team_t)count;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int		len, colorlessLen;
	char	ch;
	char	*p;
	int		spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while( 1 ) {
		ch = *in++;
		if( !ch ) {
			break;
		}

		// don't allow leading spaces
		if( !*p && ch == ' ' ) {
			continue;
		}

		//[FixMultilineNames]
		if ((unsigned char)ch < ' ') {
			continue; // No control char names
		}

		// check colors
		if( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if( !*in ) {
				break;
			}

			//[AllowBlackNames]
			/*
			// don't allow black in a name, period
			if( ColorIndex(*in) == 0 ) {
				in++;
				continue;
			}
			*/
			//[/AllowBlackNames]

			// make sure room in dest for both chars
			if( len > outSize - 2 ) {
				break;
			}


			*out++ = ch;
			if (*in == '\n') { // Multiline, remove
				len += 1;
				continue;
			}
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if( ch == ' ' ) {
			spaces++;
			if( spaces > 3 ) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "Mysterious Soldier", outSize );
	}
}

#ifdef _DEBUG
void G_DebugWrite(const char *path, const char *text)
{
	fileHandle_t f;

	trap->FS_Open( path, &f, FS_APPEND );
	trap->FS_Write( text, strlen(text), f );
	trap->FS_Close( f );
}
#endif

qboolean G_SaberModelSetup(gentity_t *ent)
{
	int i = 0;
	qboolean fallbackForSaber = qtrue;

	while (i < MAX_SABERS)
	{
		if (ent->client->saber[i].model[0])
		{
			//first kill it off if we've already got it
			if (ent->client->weaponGhoul2[i])
			{
				trap->G2API_CleanGhoul2Models(&(ent->client->weaponGhoul2[i]));
			}
			trap->G2API_InitGhoul2Model(&ent->client->weaponGhoul2[i], ent->client->saber[i].model, 0, 0, -20, 0, 0);

			if (ent->client->weaponGhoul2[i])
			{
				int j = 0;
				char *tagName;
				int tagBolt;

				if (ent->client->saber[i].skin)
				{
					trap->G2API_SetSkin(ent->client->weaponGhoul2[i], 0, ent->client->saber[i].skin, ent->client->saber[i].skin);
				}

				if (ent->client->saber[i].saberFlags & SFL_BOLT_TO_WRIST)
				{
					trap->G2API_SetBoltInfo(ent->client->weaponGhoul2[i], 0, 3+i);
				}
				else
				{ // bolt to right hand for 0, or left hand for 1
					trap->G2API_SetBoltInfo(ent->client->weaponGhoul2[i], 0, i);
				}

				//Add all the bolt points
				while (j < ent->client->saber[i].numBlades)
				{
					tagName = va("*blade%i", j+1);
					tagBolt = trap->G2API_AddBolt(ent->client->weaponGhoul2[i], 0, tagName);

					if (tagBolt == -1)
					{
						if (j == 0)
						{ //guess this is an 0ldsk3wl saber
							tagBolt = trap->G2API_AddBolt(ent->client->weaponGhoul2[i], 0, "*flash");
							fallbackForSaber = qfalse;
							break;
						}

						if (tagBolt == -1)
						{
							assert(0);
							break;

						}
					}
					j++;

					fallbackForSaber = qfalse; //got at least one custom saber so don't need default
				}

				//Copy it into the main instance
				trap->G2API_CopySpecificGhoul2Model(ent->client->weaponGhoul2[i], 0, ent->ghoul2, i+1); 
			}
		}
		else
		{
			break;
		}

		i++;
	}

	return fallbackForSaber;
}

/*
===========
SetupGameGhoul2Model

There are two ghoul2 model instances per player (actually three).  One is on the clientinfo (the base for the client side 
player, and copied for player spawns and for corpses).  One is attached to the centity itself, which is the model acutally 
animated and rendered by the system.  The final is the game ghoul2 model.  This is animated by pmove on the server, and
is used for determining where the lightsaber should be, and for per-poly collision tests.
===========
*/
void *g2SaberInstance = NULL;

qboolean BG_IsValidCharacterModel(const char *modelName, const char *skinName);
qboolean BG_ValidateSkinForTeam( char *modelName, char *skinName, int team, float *colors, int redTeam, int blueTeam, int clientNum );

void SetupGameGhoul2Model(gentity_t *ent, char *modelname, char *skinName)
{
	int handle;
	char		afilename[MAX_QPATH];
#if 0
	char		/**GLAName,*/ *slash;
#endif
	char		GLAName[MAX_QPATH];
	vec3_t	tempVec = {0,0,0};

	if (strlen(modelname) >= MAX_QPATH )
	{
		Com_Error( ERR_FATAL, "SetupGameGhoul2Model(%s): modelname exceeds MAX_QPATH.\n", modelname );
	}
	if (skinName && strlen(skinName) >= MAX_QPATH )
	{
		Com_Error( ERR_FATAL, "SetupGameGhoul2Model(%s): skinName exceeds MAX_QPATH.\n", skinName );
	}

	// First things first.  If this is a ghoul2 model, then let's make sure we demolish this first.
	if (ent->ghoul2 && trap->G2_HaveWeGhoul2Models(ent->ghoul2))
	{
		trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
	}

	//rww - just load the "standard" model for the server"
	if (!precachedKyle)
	{
		int defSkin;

		Com_sprintf( afilename, sizeof( afilename ), "models/players/kyle/model.glm" );
		handle = trap->G2API_InitGhoul2Model(&precachedKyle, afilename, 0, 0, -20, 0, 0);

		if (handle<0)
		{
			return;
		}

		defSkin = trap->R_RegisterSkin("models/players/kyle/model_default.skin");
		trap->G2API_SetSkin(precachedKyle, 0, defSkin, defSkin);
	}

	if (precachedKyle && trap->G2_HaveWeGhoul2Models(precachedKyle))
	{
		if (d_perPlayerGhoul2.integer || ent->s.number >= MAX_CLIENTS ||
			G_PlayerHasCustomSkeleton(ent))
		{ //rww - allow option for perplayer models on server for collision and bolt stuff.
			char modelFullPath[MAX_QPATH];
			char truncModelName[MAX_QPATH];
			char skin[MAX_QPATH];
			int skinHandle = 0;
			int i = 0;
			char *p;

			
			if (skinName && skinName[0])
			{
				strcpy(skin, skinName);
				strcpy(truncModelName, modelname);
			}
			else
			{
				strcpy(skin, "default");

				strcpy(truncModelName, modelname);
				p = Q_strrchr(truncModelName, '/');

				if (p)
				{
					*p = 0;
					p++;

					while (p && *p)
					{
						skin[i] = *p;
						i++;
						p++;
					}
					skin[i] = 0;
					i = 0;
				}

				if (!BG_IsValidCharacterModel(truncModelName, skin))
				{
					strcpy(truncModelName, "kyle");
					strcpy(skin, "default");
				}

				if ( level.gametype >= GT_TEAM )
				{
					//JAC: Also adjust customRGBA for team colors.
					float colorOverride[3];

					colorOverride[0] = colorOverride[1] = colorOverride[2] = 0.0f;

					BG_ValidateSkinForTeam( truncModelName, skin, ent->client->sess.sessionTeam, colorOverride, level.redTeam, level.blueTeam, ent-g_entities );
					if (colorOverride[0] != 0.0f ||
						colorOverride[1] != 0.0f ||
						colorOverride[2] != 0.0f)
					{
						ent->client->ps.customRGBA[0] = colorOverride[0]*255.0f;
						ent->client->ps.customRGBA[1] = colorOverride[1]*255.0f;
						ent->client->ps.customRGBA[2] = colorOverride[2]*255.0f;
					}

					//BG_ValidateSkinForTeam( truncModelName, skin, ent->client->sess.sessionTeam, NULL );
				}
			}

			if (skin[0])
			{
				char *useSkinName;

				if (strchr(skin, '|'))
				{//three part skin
					useSkinName = va("models/players/%s/|%s", truncModelName, skin);
				}
				else
				{
					useSkinName = va("models/players/%s/model_%s.skin", truncModelName, skin);
				}

				skinHandle = trap->R_RegisterSkin(useSkinName);
			}

			strcpy(modelFullPath, va("models/players/%s/model.glm", truncModelName));
			handle = trap->G2API_InitGhoul2Model(&ent->ghoul2, modelFullPath, 0, skinHandle, -20, 0, 0);

			if (handle<0)
			{ //Huh. Guess we don't have this model. Use the default.

				if (ent->ghoul2 && trap->G2_HaveWeGhoul2Models(ent->ghoul2))
				{
					trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
				}
				ent->ghoul2 = NULL;
				trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
			}
			else
			{
				trap->G2API_SetSkin(ent->ghoul2, 0, skinHandle, skinHandle);

				GLAName[0] = 0;
				trap->G2API_GetGLAName( ent->ghoul2, 0, GLAName);

				if (!GLAName[0] || (!strstr(GLAName, "players/_humanoid/") && ent->s.number < MAX_CLIENTS && !G_PlayerHasCustomSkeleton(ent)))
				{ //a bad model
					trap->G2API_CleanGhoul2Models(&(ent->ghoul2));
					ent->ghoul2 = NULL;
					trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
				}

				if (ent->s.number >= MAX_CLIENTS)
				{
					ent->s.modelGhoul2 = 1; //so we know to free it on the client when we're removed.

					if (skin[0])
					{ //append it after a *
						strcat( modelFullPath, va("*%s", skin) );
					}

					ent->s.modelindex = G_ModelIndex(modelFullPath);
				}
			}
		}
		else
		{
			trap->G2API_DuplicateGhoul2Instance(precachedKyle, &ent->ghoul2);
		}
	}
	else
	{
		return;
	}

	//Attach the instance to this entity num so we can make use of client-server
	//shared operations if possible.
	trap->G2API_AttachInstanceToEntNum(ent->ghoul2, ent->s.number, qtrue);

	// The model is now loaded.

	GLAName[0] = 0;

	if (!BGPAFtextLoaded)
	{
		if (BG_ParseAnimationFile("models/players/_humanoid/animation.cfg", bgHumanoidAnimations, qtrue) == -1)
		{
			Com_Printf( "Failed to load humanoid animation file\n");
			return;
		}
	}

	if (ent->s.number >= MAX_CLIENTS || G_PlayerHasCustomSkeleton(ent))
	{
		ent->localAnimIndex = -1;

		GLAName[0] = 0;
		trap->G2API_GetGLAName(ent->ghoul2, 0, GLAName);

		if (GLAName[0] &&
			!strstr(GLAName, "players/_humanoid/") &&
			!strstr (GLAName, "players/_humanoidJKG/") /*&&
			!strstr(GLAName, "players/rockettrooper/")*/)
		{ //it doesn't use humanoid anims.
			char *slash = Q_strrchr( GLAName, '/' );
			if ( slash )
			{
				strcpy(slash, "/animation.cfg");

				ent->localAnimIndex = BG_ParseAnimationFile(GLAName, NULL, qfalse);
			}
		}
		else
		{ //humanoid index.
			if (strstr(GLAName, "players/rockettrooper/"))
			{
				ent->localAnimIndex = 1;
			}
			else if ( strstr (GLAName, "players/_humanoidJKG/") )
			{
			    ent->localAnimIndex = 2;
			}
			else
			{
				ent->localAnimIndex = 0;
			}
		}

		if (ent->localAnimIndex == -1)
		{
			Com_Error(ERR_DROP, "NPC had an invalid GLA\n");
		}
	}
	else
	{
		GLAName[0] = 0;
		trap->G2API_GetGLAName(ent->ghoul2, 0, GLAName);

		if (strstr(GLAName, "players/rockettrooper/"))
		{
			//assert(!"Should not have gotten in here with rockettrooper skel");
			ent->localAnimIndex = 1;
		}
		else if ( Q_stricmpn (GLAName, "players/_humanoidJKG/", 21) == 0 )
		{
		    ent->localAnimIndex = 2;
		}
		else
		{
			ent->localAnimIndex = 0;
		}
	}
	
	if (ent->client->ps.weapon == WP_SABER || ent->s.number < MAX_CLIENTS)
	{ //a player or NPC saber user
		trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand");
		trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand");

		//rhand must always be first bolt. lhand always second. Whichever you want the
		//jetpack bolted to must always be third.
		trap->G2API_AddBolt(ent->ghoul2, 0, "*chestg");

		//claw bolts
		trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand_cap_r_arm");
		trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand_cap_l_arm");

		trap->G2API_SetBoneAnim(ent->ghoul2, 0, "model_root", 0, 12, BONE_ANIM_OVERRIDE_LOOP, 1.0f, level.time, -1, -1);
		trap->G2API_SetBoneAngles(ent->ghoul2, 0, "upper_lumbar", tempVec, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, NULL, 0, level.time);
		trap->G2API_SetBoneAngles(ent->ghoul2, 0, "cranium", tempVec, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, NULL, 0, level.time);

		if (!g2SaberInstance)
		{
			trap->G2API_InitGhoul2Model(&g2SaberInstance, "models/weapons2/saber/saber_w.glm", 0, 0, -20, 0, 0);

			if (g2SaberInstance)
			{
				// indicate we will be bolted to model 0 (ie the player) on bolt 0 (always the right hand) when we get copied
				trap->G2API_SetBoltInfo(g2SaberInstance, 0, 0);
				// now set up the gun bolt on it
				trap->G2API_AddBolt(g2SaberInstance, 0, "*blade1");
			}
		}

		if (G_SaberModelSetup(ent))
		{
			if (g2SaberInstance)
			{
				trap->G2API_CopySpecificGhoul2Model(g2SaberInstance, 0, ent->ghoul2, 1); 
			}
		}
	}
	else if(ent->s.number < MAX_CLIENTS) {
		trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand");
	}

	if (ent->s.number >= MAX_CLIENTS)
	{ //some extra NPC stuff
		if (trap->G2API_AddBolt(ent->ghoul2, 0, "lower_lumbar") == -1)
		{ //check now to see if we have this bone for setting anims and such
			ent->noLumbar = qtrue;
		}
	}
}




/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap->SetUserinfo
if desired.
============
*/
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride);

typedef struct userinfoValidate_s {
	const char		*field, *fieldClean;
	unsigned int	minCount, maxCount;
} userinfoValidate_t;

#define UIF( x, _min, _max ) { STRING(\\) #x STRING(\\), STRING( x ), _min, _max }
static userinfoValidate_t userinfoFields[] = {
	UIF( cl_guid,			0, 0 ), // not allowed, q3fill protection
	UIF( cl_punkbuster,		0, 0 ), // not allowed, q3fill protection
	UIF( ip,				0, 1 ), // engine adds this at the end
	UIF( name,				1, 1 ),
	UIF( rate,				1, 1 ),
	UIF( snaps,				1, 1 ),
	UIF( model,				1, 1 ),
//	UIF( forcepowers,		1, 1 ),
//	UIF( color1,			1, 1 ),	// NOT IN JKG
//	UIF( color2,			1, 1 ),	// NOT IN JKG
//	UIF( handicap,			1, 1 ),
	UIF( sex,				0, 1 ),
	UIF( cg_predictItems,	0, 1 ), // NOT IN JKG anymore, changing to optional for time being
//	UIF( saber1,			1, 1 ), // NOT IN JKG
//	UIF( saber2,			1, 1 ),	// NOT IN JKG
	UIF( char_color_red,	1, 1 ),
	UIF( char_color_green,	1, 1 ),
	UIF( char_color_blue,	1, 1 ),
//	UIF( teamtask,			1, 1 ),	// NOT IN JKG
	UIF( password,			0, 1 ), // optional
	UIF( teamoverlay,		0, 1 ), // only registered in cgame, not sent when connecting
};
static size_t numUserinfoFields = ARRAY_LEN( userinfoFields );

static const char *userinfoValidateExtra[USERINFO_VALIDATION_MAX] = {
	"Size",					// USERINFO_VALIDATION_SIZE
	"# of slashes",			// USERINFO_VALIDATION_SLASH
	"Extended ascii",		// USERINFO_VALIDATION_EXTASCII
	"Control characters",	// USERINFO_VALIDATION_CONTROLCHARS
};

void Svcmd_ToggleUserinfoValidation_f( void ) {
	if ( trap->Argc() == 1 ) {
		int i=0;
		for ( i=0; i<numUserinfoFields; i++ ) {
			if ( (g_userinfoValidate.integer & (1<<i)) )	trap->Print( "%2d [X] %s\n", i, userinfoFields[i].fieldClean );
			else											trap->Print( "%2d [ ] %s\n", i, userinfoFields[i].fieldClean );
		}
		for ( ; i<numUserinfoFields+USERINFO_VALIDATION_MAX; i++ ) {
			if ( (g_userinfoValidate.integer & (1<<i)) )	trap->Print( "%2d [X] %s\n", i, userinfoValidateExtra[i-numUserinfoFields] );
			else											trap->Print( "%2d [ ] %s\n", i, userinfoValidateExtra[i-numUserinfoFields] );
		}
		return;
	}
	else {
		char arg[8]={0};
		int index;

		trap->Argv( 1, arg, sizeof( arg ) );
		index = atoi( arg );

		if ( index < 0 || index > numUserinfoFields+USERINFO_VALIDATION_MAX-1 ) {
			Com_Printf( "ToggleUserinfoValidation: Invalid range: %i [0, %i]\n", index, numUserinfoFields+USERINFO_VALIDATION_MAX-1 );
			return;
		}

		trap->Cvar_Set( "g_userinfoValidate", va( "%i", (1 << index) ^ (g_userinfoValidate.integer & ((1 << (numUserinfoFields + USERINFO_VALIDATION_MAX)) - 1)) ) );
		trap->Cvar_Update( &g_userinfoValidate );

		if ( index < numUserinfoFields )	Com_Printf( "%s %s\n", userinfoFields[index].fieldClean,				((g_userinfoValidate.integer & (1<<index)) ? "Validated" : "Ignored") );
		else								Com_Printf( "%s %s\n", userinfoValidateExtra[index-numUserinfoFields],	((g_userinfoValidate.integer & (1<<index)) ? "Validated" : "Ignored") );
	}
}

char *G_ValidateUserinfo( const char *userinfo )
{
	unsigned int		i=0, count=0, currentInfo=0;
	size_t				length = strlen( userinfo ), fieldLen = 0;
	userinfoValidate_t	*info = NULL;

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_SIZE)) )
	{// size checks
		if ( length < 1 )
			return "Userinfo too short";
		else if ( length >= MAX_INFO_STRING )
			return "Userinfo too long";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_SLASH)) )
	{// slash checks
		// there must be a leading slash
		if ( userinfo[0] != '\\' )
			return "Missing leading slash";

		// no trailing slashes allowed, engine will append ip\\ip:port
		if ( userinfo[length-1] == '\\' )
			return "Trailing slash";

		// format for userinfo field is: \\key\\value
		// so there must be an even amount of slashes
		for ( i=0, count=0; i<length; i++ ) {
			if ( userinfo[i] == '\\' )
				count++;
		}
		if ( (count&1) ) // odd
			return "Bad number of slashes";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_EXTASCII)) )
	{// extended characters are impossible to type, may want to disable
		for ( i=0, count=0; i<length; i++ ) {
			if ( userinfo[i] < 0 )
				count++;
		}
		if ( count )
			return "Extended ASCII characters found";
	}

	if ( g_userinfoValidate.integer & (1<<(numUserinfoFields+USERINFO_VALIDATION_CONTROLCHARS)) )
	{// disallow \n \r ; and \"
		if ( Q_strchrs( userinfo, "\n\r;\"" ) )
			return "Invalid characters found";
	}

	for ( currentInfo=0; currentInfo<numUserinfoFields; currentInfo++ )
	{
		info = &userinfoFields[currentInfo];
		if ( g_userinfoValidate.integer & (1<<currentInfo) )
		{
			fieldLen = strlen( info->field );
			for ( i=0, count=0; i<length; i++ ) {
				if ( !Q_stricmpn( &userinfo[i], info->field, fieldLen ) ) {
					count++;
					i += fieldLen-1;
				}
			}
			if ( info->minCount && !count )
				return va( "%s field not found", info->fieldClean );
			else if ( count > info->maxCount )
				return va( "Too many %s fields (%i)", info->fieldClean, info->maxCount );
		}
	}

	return NULL;
}

qboolean ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent = g_entities + clientNum;
	gclient_t *client = ent->client;
	int team = TEAM_FREE;
	int health = 100;
	int maxHealth = 100;
	int maxStamina = 100;
	char *s = NULL;
	char *value = NULL;
	char userinfo[MAX_INFO_STRING] = {0};
	char oldClientinfo[MAX_INFO_STRING] = {0};
	char model[MAX_QPATH] = {0};
	char forcePowers[MAX_QPATH] = {0};
	char oldname[MAX_NETNAME] = {0};
	char c1[MAX_INFO_STRING] = {0};
	char c2[MAX_INFO_STRING] = {0};
	char sex[MAX_INFO_STRING] = {0};
	qboolean modelChanged = qfalse;

	trap->GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if( !(g_entities[clientNum].r.svFlags & SVF_BOT) )
	{
		s = G_ValidateUserinfo( userinfo );	// <eezstreet> I think this is being used incorrectly in this case, as it doesn't validate them properly?
		if ( s && *s )
		{
			G_SecurityLogPrintf( "Client %d (%s) failed userinfo validation: %s [IP: %s]\n", clientNum, ent->client->pers.netname, s, client->sess.IP );
			trap->DropClient( clientNum, va( "%s was dropped due to invalid userinfo.", s ) );
			return qfalse;
		}
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) && !(ent->r.svFlags & SVF_BOT) )
		client->pers.localClient = qtrue;

	// set name
	Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers.netname, sizeof( client->pers.netname ) );
	Q_strncpyz( client->pers.netname_nocolor, client->pers.netname, sizeof( client->pers.netname_nocolor ) );
	Q_StripColor( client->pers.netname_nocolor );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR && client->sess.spectatorState == SPECTATOR_SCOREBOARD )
	{
		Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
		Q_strncpyz( client->pers.netname_nocolor, "scoreboard", sizeof( client->pers.netname_nocolor ) );
	}

	if ( client->pers.connected == CON_CONNECTED )
	{
		if ( strcmp( oldname, client->pers.netname ) ) 
		{
			if ( client->pers.netnameTime > level.time  )
			{
				trap->SendServerCommand( clientNum, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NONAMECHANGE")) );

				Info_SetValueForKey( userinfo, "name", oldname );
				trap->SetUserinfo( clientNum, userinfo );			
				Q_strncpyz( client->pers.netname, oldname, sizeof( client->pers.netname ) );
				Q_strncpyz( client->pers.netname_nocolor, oldname, sizeof( client->pers.netname_nocolor ) );
				Q_StripColor( client->pers.netname_nocolor );

			}
			else
			{				
				trap->SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " %s %s\n\"", oldname, G_GetStringEdString( "MP_SVGAME", "PLRENAME" ), client->pers.netname ) );
				client->pers.netnameTime = level.time + 5000;
			}
		}
	}

	// set model
	Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );

	if ( d_perPlayerGhoul2.integer )
	{
		if ( Q_stricmp( model, client->modelname ) )
		{
			Q_strncpyz( client->modelname, model, sizeof( client->modelname ) );
			modelChanged = qtrue;
		}
	}

	client->ps.customRGBA[0] = (value=Info_ValueForKey( userinfo, "char_color_red" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[1] = (value=Info_ValueForKey( userinfo, "char_color_green" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;
	client->ps.customRGBA[2] = (value=Info_ValueForKey( userinfo, "char_color_blue" ))	? Com_Clampi( 0, 255, atoi( value ) ) : 255;

	//Prevent skins being too dark
	if ( g_charRestrictRGB.integer && ((client->ps.customRGBA[0]+client->ps.customRGBA[1]+client->ps.customRGBA[2]) < 100) )
		client->ps.customRGBA[0] = client->ps.customRGBA[1] = client->ps.customRGBA[2] = 255;

	client->ps.customRGBA[3]=255;

	Q_strncpyz( forcePowers, Info_ValueForKey( userinfo, "forcepowers" ), sizeof( forcePowers ) );

	//JAC: update our customRGBA for team colors. 
	if ( level.gametype >= GT_TEAM )
	{
		char skin[MAX_QPATH] = {0};
		vec3_t colorOverride = {0.0f};

		VectorClear( colorOverride );

		BG_ValidateSkinForTeam( model, skin, client->sess.sessionTeam, colorOverride, level.redTeam, level.blueTeam, clientNum  );
		if ( colorOverride[0] != 0.0f || colorOverride[1] != 0.0f || colorOverride[2] != 0.0f )
			VectorScaleM( colorOverride, 255.0f, client->ps.customRGBA );
	}

	// bots set their team a few frames later
	if ( level.gametype >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT )
	{
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) )
			team = TEAM_RED;
		else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) )
			team = TEAM_BLUE;
		else
			team = PickTeam( clientNum ); // pick the team with the least number of players
	}
	else
		team = client->sess.sessionTeam;

	//Testing to see if this fixes the problem with a bot's team getting set incorrectly.
	team = client->sess.sessionTeam;

	health = maxHealth; //atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	// When the hell would the below ever be valid? NEVER --eez
	// if ( client->pers.maxHealth < 1 || client->pers.maxHealth > maxHealth ) {
	// 	client->pers.maxHealth = 100;
	// }
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	

	//also set maxforce/stamina
	client->pers.maxStamina = maxStamina;
	client->ps.stats[STAT_MAX_STAMINA] = client->pers.maxStamina; //force and stamina are the same thing in JKG

	if (level.gametype >= GT_TEAM) {
		s = Info_ValueForKey( userinfo, "teamoverlay" );
		if ( ! *s || atoi( s ) != 0 ) {
			client->pers.teamInfo = qtrue;
		} else {
			client->pers.teamInfo = qfalse;
		}
	}

	// colors
	Q_strncpyz( c1, Info_ValueForKey( userinfo, "color1" ), sizeof( c1 ) );
	Q_strncpyz( c2, Info_ValueForKey( userinfo, "color2" ), sizeof( c2 ) );

	Q_strncpyz( sex, Info_ValueForKey( userinfo, "sex"), sizeof( sex ) );

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	s = va("n\\%s\\t\\%i\\model\\%s\\w\\%i\\l\\%i\\dt\\%i\\sex\\%s",
		client->pers.netname, client->sess.sessionTeam, model, 
		client->sess.wins, client->sess.losses, client->sess.duelTeam, sex);
	trap->GetConfigstring( CS_PLAYERS+clientNum, oldClientinfo, sizeof( oldClientinfo ) );
	trap->SetConfigstring( CS_PLAYERS+clientNum, s );

	if ( modelChanged ) //only going to be true for allowable server-side custom skeleton cases
	{ //update the server g2 instance if appropriate
		char *modelname = Info_ValueForKey( userinfo, "model" );
		SetupGameGhoul2Model( ent, modelname, NULL );

		if ( ent->ghoul2 && ent->client )
			ent->client->renderInfo.lastG2 = NULL; //update the renderinfo bolts next update.

		client->torsoAnimExecute = client->legsAnimExecute = -1;
		client->torsoLastFlip = client->legsLastFlip = qfalse;
	}

	if ( g_logClientInfo.integer )
	{
		if ( strcmp( oldClientinfo, s ) )
			G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
		else
			G_LogPrintf( "ClientUserinfoChanged: %i <no change>\n", clientNum );
	}

	return qtrue;
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/

static qboolean CompareIPs( const char *ip1, const char *ip2 )
{
	while ( 1 ) {
		if ( *ip1 != *ip2 )
			return qfalse;
		if ( !*ip1 || *ip1 == ':' )
			break;
		ip1++;
		ip2++;
	}

	return qtrue;
}

extern int NextIDCode;

extern qboolean g_dontPenalizeTeam; //g_cmds.c
#define NET_ADDRSTRMAXLEN 48 // maximum length of an IPv6 address string including trailing '\0'
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
//	char		*areabits;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING] = {0},
				tmpIP[NET_ADDRSTRMAXLEN] = {0},
				guid[33] = {0};
	gentity_t	*ent;
	gentity_t	*te;
	char *luaresp;

	ent = &g_entities[ clientNum ];
	ent->LuaUsable = 1; // So we can use it in lua (PlayerConnected hook)
	ent->s.number = clientNum;
	ent->classname = "connecting";

	// ClientConnectionActive[clientNum] is set to 0
	// This is for q3fill detection
	// Once a sequenced packet has been received this goes back to 1
	// Since q3fill will never do this, it stays at 0
	// Which causes the server to kick and ban the client after 1 second

	trap->GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	value = Info_ValueForKey( userinfo, "ja_guid" );
	if( value[0] )
		Q_strncpyz( guid, value, sizeof( guid ) );
	else if( isBot )
		Q_strncpyz( guid, "BOT", sizeof( guid ) );
	else
		Q_strncpyz( guid, "NOGUID", sizeof( guid ) );

	// check to see if they are on the banned IP list
	//value = Info_ValueForKey (userinfo, "ip");
	//Q_strncpyz(TmpIP, value, sizeof(TmpIP)); // Used later

	// FIXME: Need to replace the ban code
	/*if ( ( banreason = JKG_Bans_IsBanned( svs->clients[clientNum].netchan.remoteAddress ) ) != NULL ) {
		return banreason;
		//return "You are banned from this server.";
	}*/
	if (level.serverInit) {
		// We're on the init map, deny all connections
		return "Server is initializing, please wait...";
	}

	value = Info_ValueForKey (userinfo, "clver");
#ifndef _DEBUG
	if ( !isBot && Q_stricmp(value, JKG_VERSION)) {
		return "Please update your client-side.";
	}
#endif

	if ( !isBot && g_needpass.integer ) {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			strcmp( g_password.string, value) != 0) {
			static char sTemp[1024];
			Q_strncpyz(sTemp, G_GetStringEdString("MP_SVGAME","INVALID_ESCAPE_TO_MAIN"), sizeof (sTemp) );
			return sTemp;// return "Invalid password";
		}
	}

	value = Info_ValueForKey (userinfo, "ip");
	Q_strncpyz( tmpIP, isBot ? "Bot" : value, sizeof( tmpIP ) );

	if(jkg_antifakeplayer.integer)
	{
		if(firstTime && !isBot)
		{
			int i;
			int count = 0;

			for(i = 0; i < level.maxclients; i++)
			{
				if ( level.clients[i].pers.connected != CON_DISCONNECTED && i != clientNum )
				{
					if( CompareIPs( tmpIP, level.clients[i].sess.IP ) )
						count++;
				}
			}
			if(count > 1)
			{
				return "Too many connections from the same IP.";
			}
		}
	}

	// FAILSAFE
	// The client ent should not be in use at this point.
	// If it IS in use, forcefully run the ent through the disconnection code to reset the slot
	if ( ent->inuse && !isBot ) { // Didz: Just realized a problem with my fix, won't disconnect clients that /were/ still on if a bot replaces them.
        G_LogPrintf( "Forcing disconnect on active client: %i\n", ent-g_entities );
        // so lets just fix up anything that should happen on a disconnect
        ClientDisconnect( ent-g_entities );
	}

	// they can connect
	client = &level.clients[ clientNum ];
	ent->client = client;

	//assign the pointer for bg entity access
	ent->playerState = &ent->client->ps;

	if ( ent->health && ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR && clientNum == ent->client->ps.clientNum) {
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die( ent, ent, ent, 100000, MOD_TEAM_CHANGE );
		g_dontPenalizeTeam = qfalse;
	}

	memset( client, 0, sizeof(*client) );

	Q_strncpyz( client->pers.guid, guid, sizeof( client->pers.guid ) );

	client->pers.connected = CON_CONNECTING;
	client->pers.connectTime = level.time; //JAC: Added

	ent->IDCode = NextIDCode++;
	ent->UsesELS = 1;
	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitClientSessionData( client, userinfo, isBot );
	}
	G_ReadClientSessionData( client );

	if (!isBot && firstTime) {
		client->sess.validated = 0;
		client->sess.noq3fill = 0;
		client->sess.connTime = level.time;
	} else {
		client->sess.validated = 1;
		client->sess.noq3fill = 1;
	}

	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			client->pers.connected = CON_DISCONNECTED;
			return "BotConnectfailed";
		}
	}

	// All the built-in tests succeeded, let lua do the final check
	luaresp = (char *)GLua_Hook_PlayerConnect(clientNum, firstTime, isBot);
	if (luaresp) {
		client->pers.connected = CON_DISCONNECTED;
		return luaresp;
	}

	G_LogPrintf( "ClientConnect: %i [%s] (%s) \"%s^7\"\n", clientNum, tmpIP, guid, client->pers.netname );
	// FIXME:
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"", client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCONNECT")) );
	}

	if ( level.gametype >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}
	

	// count current clients and rank for scoreboard
	CalculateRanks();

	te = G_TempEntity( vec3_origin, EV_CLIENTJOIN );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = clientNum;

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap->AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

	TeamInitialize( clientNum );
	
	client->ps.credits = jkg_startingCredits.integer-1;	// hack to give us our starting gear
	client->storedCredits = jkg_startingCredits.integer-1;
	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum, qboolean allowTeamReset ) {
	gentity_t	*ent;
	gclient_t	*client;
	gentity_t	*tent;
	int			flags, credits, i;
	char		userinfo[MAX_INFO_VALUE], *modelname;

	ent = g_entities + clientNum;
	JKG_CBB_SendAll(clientNum);
	JKG_PlayerIsolationClear(clientNum);
	//eezstreet add
	//memset(ent->inventory, 0, sizeof(ent->inventory));
	ent->inventory = g_entities[clientNum].inventory;	// FIXME: ????

	// 6 april 2015
	// clear our inventory on ClientBegin because I forgot that this was a thing
	trap->SendServerCommand(clientNum, "pInv clr");
	ent->client->ps.credits = 0;
	//ent->client->ps.spent = 0;
	if ((ent->r.svFlags & SVF_BOT) && g_gametype.integer >= GT_TEAM)
	{
		if (allowTeamReset)
		{
			const char *team = "Red";
			int preSess;

			//SetTeam(ent, "");
			ent->client->sess.sessionTeam = PickTeam(-1);
			trap->GetUserinfo(clientNum, userinfo, MAX_INFO_STRING);

			if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
			{
				ent->client->sess.sessionTeam = TEAM_RED;
			}

			if (ent->client->sess.sessionTeam == TEAM_RED)
			{
				team = "Red";
			}
			else
			{
				team = "Blue";
			}

			Info_SetValueForKey( userinfo, "team", team );

			trap->SetUserinfo( clientNum, userinfo );

			ent->client->ps.persistant[ PERS_TEAM ] = ent->client->sess.sessionTeam;

			preSess = ent->client->sess.sessionTeam;
			G_ReadClientSessionData( ent->client );
			ent->client->sess.sessionTeam = (team_t)preSess;
			G_WriteClientSessionData(ent->client);
			if ( !ClientUserinfoChanged( clientNum ) )
				return;
			ClientBegin(clientNum, qfalse);
			return;
		}
	}
	/*

	// Alright, let's set up the lives for LMS.
#ifdef __JKG_NINELIVES__
	if(g_gametype.integer == GT_LMS_NINELIVES)
	{
		client->ns.iLivesLeft = 9;	// Nine lives
	}
#endif
#ifdef __JKG_ROUNDBASED__
	if(g_gametype.integer == GT_LMS_ROUNDS)
	{
		client->ns.iLivesLeft = 1;	// One life for each round
	}
#endif

	*/

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap->UnlinkEntity( (sharedEntity_t *)ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	if (ent->assists != nullptr) {
		ent->assists->clear();
	}

	//assign the pointer for bg entity access
	ent->playerState = &ent->client->ps;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.lastCreditTime = 0;
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	credits = client->ps.credits;

	i = 0;

	while (i < NUM_FORCE_POWERS)
	{
		if (ent->client->ps.fd.forcePowersActive & (1 << i))
		{
			WP_ForcePowerStop(ent, i);
		}
		i++;
	}

	i = TRACK_CHANNEL_1;

	while (i < NUM_TRACK_CHANNELS)
	{
		if (ent->client->ps.fd.killSoundEntIndex[i-50] && ent->client->ps.fd.killSoundEntIndex[i-50] < MAX_GENTITIES && ent->client->ps.fd.killSoundEntIndex[i-50] > 0)
		{
			G_MuteSound(ent->client->ps.fd.killSoundEntIndex[i-50], CHAN_VOICE);
		}
		i++;
	}
	i = 0;

	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;
	// Check the credit count. that should probably stick.
	client->ps.credits = credits;

	client->ps.hasDetPackPlanted = qfalse;

	//first-time force power initialization
	WP_InitForcePowers( ent );

	//init saber ent
	WP_SaberInitBladeData( ent );

	trap->GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
	modelname = Info_ValueForKey (userinfo, "model");
	SetupGameGhoul2Model(ent, modelname, NULL);

	if (ent->ghoul2 && ent->client)
	{
		ent->client->renderInfo.lastG2 = NULL; //update the renderinfo bolts next update.
	}

	// Inform GLua of this player
	GLua_Hook_PlayerBegin(clientNum);

	// Give this player Operator rights if he's 127.0.0.1
	if (!Q_strncmp(client->sess.IP, "127.0.0.1:",10)) {
		// He's localhost, give operator rights
		//client->sess.adminRank = ADMRANK_OPERATOR;
		;	//temporary fix, do nothing since this defaults host to a cheating bastard --Futuza
	}

	if (g_gametype.integer == GT_POWERDUEL && client->sess.sessionTeam != TEAM_SPECTATOR &&
		client->sess.duelTeam == DUELTEAM_FREE)
	{
		SetTeam(ent, "s");
	}
	else
	{
		G_SetSaber(ent, 0, DEFAULT_SABER, qfalse);

		// locate ent at a spawn point
		ClientSpawn( ent, qfalse );
	}

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		// send event
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;

		if ( g_gametype.integer != GT_DUEL || g_gametype.integer == GT_POWERDUEL ) {
			trap->SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " %s\n\"", client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLENTER")) );
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();

	G_ClearClientLog(clientNum);
}

//Convenient interface to set all my limb breakage stuff up -rww
void G_BreakArm(gentity_t *ent, int arm)
{
	int anim = -1;

	assert(ent && ent->client);

	if (ent->localAnimIndex >= NUM_RESERVED_ANIMSETS)
	{ //no broken limbs for vehicles and non-humanoids
		return;
	}

	if (!arm)
	{ //repair him
		ent->client->ps.brokenLimbs = 0;
		return;
	}

	if (arm == BROKENLIMB_LARM)
	{
		if (ent->client->saber[1].model[0] &&
			ent->client->ps.weapon == WP_SABER &&
			!ent->client->ps.saberHolstered &&
			ent->client->saber[1].soundOff)
		{ //the left arm shuts off its saber upon being broken
			G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
		}
	}

	ent->client->ps.brokenLimbs = 0; //make sure it's cleared out
	ent->client->ps.brokenLimbs |= (1 << arm); //this arm is now marked as broken

	//Do a pain anim based on the side. Since getting your arm broken does tend to hurt.
	if (arm == BROKENLIMB_LARM)
	{
		anim = BOTH_PAIN2;
	}
	else if (arm == BROKENLIMB_RARM)
	{
		anim = BOTH_PAIN3;
	}

	if (anim == -1)
	{
		return;
	}

	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);

	//This could be combined into a single event. But I guess limbs don't break often enough to
	//worry about it.
	G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );
	//FIXME: A nice bone snapping sound instead if possible
	G_Sound(ent, CHAN_AUTO, G_SoundIndex( va("sound/player/bodyfall_human%i.wav", Q_irand(1, 3)) ));
}

//Update the ghoul2 instance anims based on the playerstate values
qboolean BG_SaberStanceAnim( int anim );
void G_UpdateClientAnims(gentity_t *self, float animSpeedScale)
{
	static int f;
	static int torsoAnim;
	static int legsAnim;
	static int firstFrame, lastFrame;
	static int aFlags;
	static float animSpeed, lAnimSpeedScale;
	qboolean setTorso = qfalse;

	torsoAnim = (self->client->ps.torsoAnim);
	legsAnim = (self->client->ps.legsAnim);

	if (self->client->ps.saberLockFrame)
	{
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "model_root", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "lower_lumbar", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "Motion", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		return;
	}
	
	// JKG: Check to see if we have a debuff which freezes us
	for (int i = 0; i < PLAYERBUFF_BITS; i++)
	{
		if (self->client->ps.buffsActive & (1 << i))
		{
			if (buffTable[self->client->ps.buffs[i].buffID].passive.overridePmoveType.first)
			{
				if (buffTable[self->client->ps.buffs[i].buffID].passive.overridePmoveType.second == PM_FREEZE)
				{
					const animation_t *torsoAnimData = &bgAllAnims[self->localAnimIndex].anims[self->client->ps.freezeTorsoAnim];
					const animation_t *legsAnimData = &bgAllAnims[self->localAnimIndex].anims[self->client->ps.freezeLegsAnim];
					int legsAnimFrame = legsAnimData->firstFrame + legsAnimData->numFrames;
					int torsoAnimFrame = torsoAnimData->firstFrame + torsoAnimData->numFrames;

					trap->G2API_SetBoneAnim(self->ghoul2, 0, "model_root", legsAnimFrame, legsAnimFrame, BONE_ANIM_OVERRIDE_FREEZE | BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
					trap->G2API_SetBoneAnim(self->ghoul2, 0, "lower_lumbar", torsoAnimFrame, torsoAnimFrame, BONE_ANIM_OVERRIDE_FREEZE | BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);

					return;
				}
			}
		}
	}

	if (self->localAnimIndex >= NUM_RESERVED_ANIMSETS &&
		bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame == 0 &&
		bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames == 0)
	{ //We'll allow this for non-humanoids.
		goto tryTorso;
	}

	if (self->client->legsAnimExecute != legsAnim || self->client->legsLastFlip != self->client->ps.legsFlip)
	{
		animSpeed = 50.0f / bgAllAnims[self->localAnimIndex].anims[legsAnim].frameLerp;
		lAnimSpeedScale = (animSpeed *= animSpeedScale);

		if (bgAllAnims[self->localAnimIndex].anims[legsAnim].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		if (animSpeed < 0)
		{
			lastFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame;
			firstFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame + bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames;
		}
		else
		{
			firstFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame;
			lastFrame = bgAllAnims[self->localAnimIndex].anims[legsAnim].firstFrame + bgAllAnims[self->localAnimIndex].anims[legsAnim].numFrames;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on server position, but it's here just for the sake of matching them.

		trap->G2API_SetBoneAnim(self->ghoul2, 0, "model_root", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
		self->client->legsAnimExecute = legsAnim;
		self->client->legsLastFlip = self->client->ps.legsFlip;
	}

tryTorso:
	if (self->localAnimIndex >= NUM_RESERVED_ANIMSETS &&
		bgAllAnims[self->localAnimIndex].anims[torsoAnim].firstFrame == 0 &&
		bgAllAnims[self->localAnimIndex].anims[torsoAnim].numFrames == 0)

	{ //If this fails as well just return.
		return;
	}

	if ((self->client->torsoAnimExecute != torsoAnim || self->client->torsoLastFlip != self->client->ps.torsoFlip) &&
		!self->noLumbar)
	{
		aFlags = 0;
		animSpeed = 0;

		f = torsoAnim;

		BG_SaberStartTransAnim(self->s.number, self->client->ps.fd.saberAnimLevel, self->client->ps.weapon, f, &animSpeedScale,
			self->client->ps.brokenLimbs, SaberStances[self->client->ps.fd.saberAnimLevel].moves[self->client->ps.saberMove].animspeedscale, 
			self->client->ps.saberSwingSpeed, self->client->ps.saberMove);

		if( self->client->ps.weaponstate == WEAPON_RELOADING )
		{
			JKG_ReloadAnimation(self->client->ps.firingMode, self->client->ps.weaponId, 
				self->client->ps.torsoAnim, bgAllAnims[self->localAnimIndex].anims, &animSpeedScale);
		}

		animSpeed = 50.0f / bgAllAnims[self->localAnimIndex].anims[f].frameLerp;
		lAnimSpeedScale = (animSpeed *= animSpeedScale);

		if (bgAllAnims[self->localAnimIndex].anims[f].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on client position, but it's here just for the sake of matching them.

		if (animSpeed < 0)
		{
			lastFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame;
			firstFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame + bgAllAnims[self->localAnimIndex].anims[f].numFrames;
		}
		else
		{
			firstFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame;
			lastFrame = bgAllAnims[self->localAnimIndex].anims[f].firstFrame + bgAllAnims[self->localAnimIndex].anims[f].numFrames;
		}

		trap->G2API_SetBoneAnim(self->ghoul2, 0, "lower_lumbar", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, /*firstFrame why was it this before?*/-1, 150);

		self->client->torsoAnimExecute = torsoAnim;
		self->client->torsoLastFlip = self->client->ps.torsoFlip;
		
		setTorso = qtrue;
	}

	if (setTorso &&
		self->localAnimIndex < NUM_RESERVED_ANIMSETS)
	{ //only set the motion bone for humanoids.
		trap->G2API_SetBoneAnim(self->ghoul2, 0, "Motion", firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
	}

#if 0 //disabled for now
	if (self->client->ps.brokenLimbs != self->client->brokenLimbs ||
		setTorso)
	{
		if (self->localAnimIndex <= 1 && self->client->ps.brokenLimbs &&
			(self->client->ps.brokenLimbs & (1 << BROKENLIMB_LARM)))
		{ //broken left arm
			char *brokenBone = "lhumerus";
			animation_t *armAnim;
			int armFirstFrame;
			int armLastFrame;
			int armFlags = 0;
			float armAnimSpeed;

			armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_DEAD21 ];
			self->client->brokenLimbs = self->client->ps.brokenLimbs;

			armFirstFrame = armAnim->firstFrame;
			armLastFrame = armAnim->firstFrame+armAnim->numFrames;
			armAnimSpeed = 50.0f / armAnim->frameLerp;
			armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

			trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
		}
		else if (self->localAnimIndex <= 1 && self->client->ps.brokenLimbs &&
			(self->client->ps.brokenLimbs & (1 << BROKENLIMB_RARM)))
		{ //broken right arm
			char *brokenBone = "rhumerus";
			char *supportBone = "lhumerus";

			self->client->brokenLimbs = self->client->ps.brokenLimbs;

			//Only put the arm in a broken pose if the anim is such that we
			//want to allow it.
			if ((//self->client->ps.weapon == WP_MELEE ||
				self->client->ps.weapon != WP_SABER ||
				BG_SaberStanceAnim(self->client->ps.torsoAnim) ||
				PM_RunningAnim(self->client->ps.torsoAnim)) &&
				(!self->client->saber[1].model[0] || self->client->ps.weapon != WP_SABER))
			{
				int armFirstFrame;
				int armLastFrame;
				int armFlags = 0;
				float armAnimSpeed;
				animation_t *armAnim;

				if (self->client->ps.weapon == WP_MELEE ||
					self->client->ps.weapon == WP_SABER ||
					self->client->ps.weapon == WP_BRYAR_PISTOL)
				{ //don't affect this arm if holding a gun, just make the other arm support it
					armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_ATTACK2 ];

					//armFirstFrame = armAnim->firstFrame;
					armFirstFrame = armAnim->firstFrame+armAnim->numFrames;
					armLastFrame = armAnim->firstFrame+armAnim->numFrames;
					armAnimSpeed = 50.0f / armAnim->frameLerp;
					armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

					trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
				}
				else
				{ //we want to keep the broken bone updated for some cases
					trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				}

				if (self->client->ps.torsoAnim != BOTH_MELEE1 &&
					self->client->ps.torsoAnim != BOTH_MELEE2 &&
					(self->client->ps.torsoAnim == TORSO_WEAPONREADY2 || self->client->ps.torsoAnim == BOTH_ATTACK2 || self->client->ps.weapon < WP_BRYAR_PISTOL))
				{
					//Now set the left arm to "support" the right one
					armAnim = &bgAllAnims[self->localAnimIndex].anims[ BOTH_STAND2 ];
					armFirstFrame = armAnim->firstFrame;
					armLastFrame = armAnim->firstFrame+armAnim->numFrames;
					armAnimSpeed = 50.0f / armAnim->frameLerp;
					armFlags = (BONE_ANIM_OVERRIDE_LOOP|BONE_ANIM_BLEND);

					trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, armFirstFrame, armLastFrame, armFlags, armAnimSpeed, level.time, -1, 150);
				}
				else
				{ //we want to keep the support bone updated for some cases
					trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				}
			}
			else
			{ //otherwise, keep it set to the same as the torso
				trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
				trap->G2API_SetBoneAnim(self->ghoul2, 0, supportBone, firstFrame, lastFrame, aFlags, lAnimSpeedScale, level.time, -1, 150);
			}
		}
		else if (self->client->brokenLimbs)
		{ //remove the bone now so it can be set again
			char *brokenBone = NULL;
			int broken = 0;

			//Warning: Don't remove bones that you've added as bolts unless you want to invalidate your bolt index
			//(well, in theory, I haven't actually run into the problem)
			if (self->client->brokenLimbs & (1<<BROKENLIMB_LARM))
			{
				brokenBone = "lhumerus";
				broken |= (1<<BROKENLIMB_LARM);
			}
			else if (self->client->brokenLimbs & (1<<BROKENLIMB_RARM))
			{ //can only have one arm broken at once.
				brokenBone = "rhumerus";
				broken |= (1<<BROKENLIMB_RARM);

				//want to remove the support bone too then
				trap->G2API_SetBoneAnim(self->ghoul2, 0, "lhumerus", 0, 1, 0, 0, level.time, -1, 0);
				trap->G2API_RemoveBone(self->ghoul2, "lhumerus", 0);
			}

			assert(brokenBone);

			//Set the flags and stuff to 0, so that the remove will succeed
			trap->G2API_SetBoneAnim(self->ghoul2, 0, brokenBone, 0, 1, 0, 0, level.time, -1, 0);

			//Now remove it
			trap->G2API_RemoveBone(self->ghoul2, brokenBone, 0);
			self->client->brokenLimbs &= ~broken;
		}
	}
#endif
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
extern qboolean WP_HasForcePowers( const playerState_t *ps );
void ClientSpawn(gentity_t *ent, qboolean respawn) {
	int					index;
	vec3_t				spawn_origin, spawn_angles;
	gclient_t			*client;
	int					i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int					persistant[MAX_PERSISTANT];
	gentity_t			*spawnPoint = NULL;
	int					flags, gameFlags;
	int					savedPing;
	int					accuracy_hits, accuracy_shots;
	int					eventSequence;
	char				userinfo[MAX_INFO_STRING];
	forcedata_t			savedForce;
	int					saveSaberNum = ENTITYNUM_NONE;
	int					maxHealth;
	saberInfo_t			saberSaved[MAX_SABERS];
	int					l = 0;
	void				*g2WeaponPtrs[MAX_SABERS];
	char				*value;
	char				*saber;
	qboolean			changedSaber = qfalse;
	int                 savedWeaponId = 0;
	qboolean			haveItem = qfalse;
	qboolean			use_secondary_spawnpoint = qfalse;
	int					savedCredits;
	int					savedAmmo[MAX_AMMO_TYPES] {0};
	int					savedAmmoTypes[MAX_WEAPON_TABLE_SIZE][MAX_FIREMODES]{};
	int					savedClipAmmo[MAX_WEAPON_TABLE_SIZE][MAX_FIREMODES]{};
	int					savedAmmoType, savedFiringMode;
	int					savedArmor[MAX_ARMOR];

	index = ent - g_entities;
	client = ent->client;

	/* This player deserves an update, since he just joined a new team */
	ent->client->pers.partyUpdate = qtrue;

	//first we want the userinfo so we can see if we should update this client's saber -rww
	trap->GetUserinfo( index, userinfo, sizeof(userinfo) );
	while (l < MAX_SABERS)
	{
		switch (l)
		{
		case 0:
			saber = &ent->client->sess.saberType[0];
			break;
		case 1:
			saber = &ent->client->sess.saber2Type[0];
			break;
		default:
			saber = NULL;
			break;
		}

		value = Info_ValueForKey (userinfo, va("saber%i", l+1));
		if (saber &&
			value &&
			(Q_stricmp(value, saber) || !saber[0] || !ent->client->saber[0].model[0]))
		{ //doesn't match up (or our session saber is BS), we want to try setting it
			if (G_SetSaber(ent, l, value, qfalse))
			{
				changedSaber = qtrue;
			}
			else if (!saber[0] || !ent->client->saber[0].model[0])
			{ //Well, we still want to say they changed then (it means this is siege and we have some overrides)
				changedSaber = qtrue;
			}
		}
		l++;
	}

	if (changedSaber)
	{ //make sure our new info is sent out to all the other clients, and give us a valid stance
		ClientUserinfoChanged( ent->s.number );

		//make sure the saber models are updated
		G_SaberModelSetup(ent);

		l = 0;
		while (l < MAX_SABERS)
		{ //go through and make sure both sabers match the userinfo
			switch (l)
			{
			case 0:
				saber = &ent->client->sess.saberType[0];
				break;
			case 1:
				saber = &ent->client->sess.saber2Type[0];
				break;
			default:
				saber = NULL;
				break;
			}

			value = Info_ValueForKey (userinfo, va("saber%i", l+1));

			if (Q_stricmp(value, saber))
			{ //they don't match up, force the user info
				Info_SetValueForKey(userinfo, va("saber%i", l+1), saber);
				trap->SetUserinfo( ent->s.number, userinfo );
			}
			l++;
		}

		if (ent->client->saber[0].model[0] &&
			ent->client->saber[1].model[0])
		{ //dual
			//ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = SS_DUAL;
			int iterator = 0;
			for(; iterator < MAX_STANCES; iterator++)
			{
				// Find the first stance that's duals-only
				if( SaberStances[ iterator ].isDualsOnly )
				{
					ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = iterator;
					break;
				}
			}
		}
		else if ((ent->client->saber[0].saberFlags&SFL_TWO_HANDED))
		{ //staff
			//ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = SS_STAFF;
			int iterator = 0;
			for(; iterator < MAX_STANCES; iterator++)
			{
				// Find the first stance that's duals-only
				if( SaberStances[ iterator ].isStaffOnly )
				{
					ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = iterator;
					break;
				}
			}
		}
		else
		{
			if (ent->client->sess.saberLevel < 0)
			{
				ent->client->sess.saberLevel = 0;
			}
			/*else if (ent->client->sess.saberLevel > SS_SORESU)
			{
				ent->client->sess.saberLevel = SS_SORESU;
			}*/
			else if( !SaberStances[ent->client->ps.fd.saberAnimLevel].moves[LS_READY].anim )
			{
				ent->client->sess.saberLevel = 0;
			}
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel;

			if (ent->client->ps.fd.saberAnimLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
			{
				ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel = ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE];
			}
		}

		//let's just make sure the styles we chose are cool
		if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
		{
			WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
		}
	}
	l = 0;

	if (client->ps.fd.forceDoInit)
	{ //force a reread of force powers
		WP_InitForcePowers( ent );
		client->ps.fd.forceDoInit = 0;
	}

	if (!SaberStances[ent->client->ps.fd.saberAnimLevel].isDualsOnly &&
		!SaberStances[ent->client->ps.fd.saberAnimLevel].isStaffOnly &&
		ent->client->ps.fd.saberAnimLevel == ent->client->ps.fd.saberDrawAnimLevel &&
		ent->client->ps.fd.saberAnimLevel == ent->client->sess.saberLevel)
	{
		if (ent->client->sess.saberLevel < 0)
		{
			ent->client->sess.saberLevel = 0;
		}
		if (!SaberStances[ent->client->ps.fd.saberAnimLevel].moves[LS_READY].anim)
		{
			ent->client->sess.saberLevel = 0;
		}
		ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel;

		if (ent->client->ps.fd.saberAnimLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
		{
			ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberDrawAnimLevel = ent->client->sess.saberLevel = ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE];
		}
	}

	// UQ1: Record their teamowner as well... For NPCs and WarZone to use...
	ent->s.teamowner = client->sess.sessionTeam;
	
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
	
		if (g_gametype.integer == GT_WARZONE)
		{
			// In case spawns havn't been allocated as yet...
			Calculate_Warzone_Flag_Spawns();

			spawnPoint = SelectWarzoneSpawnpoint(ent);
			VectorCopy(spawnPoint->s.origin, spawn_origin);
			VectorCopy(spawnPoint->s.angles, spawn_angles);

			trap->SendServerCommand( -1, va("tkt %i %i", redtickets, bluetickets ));
		}

		else
		{
			if (!GLua_Hook_SelectSpectatorSpawn(ent->s.number, &spawnPoint, spawn_origin, spawn_angles))
				spawnPoint = SelectSpectatorSpawnPoint ( spawn_origin, spawn_angles );
		}
	}
	else if (g_gametype.integer == GT_WARZONE && client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		// In case spawns havn't been allocated as yet...
		Calculate_Warzone_Flag_Spawns();
		use_secondary_spawnpoint = qtrue;
	}
	else 
	{
		do {
			// the first spawn should be at a good looking spot
			if ( !client->pers.initialSpawn /*&& client->pers.localClient*/ ) {
				client->pers.initialSpawn = qtrue;
				if (!GLua_Hook_SelectInitialSpawn(ent->s.number, &spawnPoint, client->sess.sessionTeam, spawn_origin, spawn_angles))
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles, client->sess.sessionTeam, (ent->r.svFlags & SVF_BOT) != 0 );
			} else {
				// don't spawn near existing origin if possible
				if (!GLua_Hook_SelectSpawn(ent->s.number, &spawnPoint, client->sess.sessionTeam, client->ps.origin, spawn_origin, spawn_angles))
					spawnPoint = SelectSpawnPoint ( client->ps.origin, spawn_origin, spawn_angles, client->sess.sessionTeam, (ent->r.svFlags & SVF_BOT) != 0 );
			}
			/*}*/

			if (!spawnPoint) {
				break;
			}
			
			// Tim needs to prevent bots from spawning at the initial point
			// on q3dm0...
			if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}
			// just to be symetric, we have a nohumans option...
			if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}

			break;

		} while ( 1 );
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT );
	flags ^= EF_TELEPORT_BIT;
	gameFlags = ent->client->mGameFlags & ( PSG_VOTED | PSG_TEAMVOTED);

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	savedWeaponId = client->ps.weaponId;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	savedForce = client->ps.fd;

	saveSaberNum = client->ps.saberEntityNum;

	savedCredits = client->ps.credits;

	l = 0;
	while (l < MAX_SABERS)
	{
		saberSaved[l] = client->saber[l];
		g2WeaponPtrs[l] = client->weaponGhoul2[l];
		l++;
	}

	i = 0;
	while (i < HL_MAX)
	{
		ent->locationDamage[i] = 0;
		i++;
	}

	memcpy(savedAmmoTypes, client->ammoTypes, sizeof(savedAmmoTypes));
	memcpy(savedAmmo, client->ammoTable, sizeof(savedAmmo));
	memcpy(savedClipAmmo, client->clipammo, sizeof(savedClipAmmo));
	memcpy(savedArmor, client->ps.armor, sizeof(savedArmor));
	savedAmmoType = ent->client->ps.ammoType;
	savedFiringMode = ent->client->ps.firingMode;

	// <----- The client gets cleared out ----->
	memset (client, 0, sizeof(*client)); // bk FIXME: Com_Memset?
	// <--------------------------------------->

	memcpy(client->ammoTable, savedAmmo, sizeof(savedAmmo));
	memcpy(client->ammoTypes, savedAmmoTypes, sizeof(savedAmmoTypes));
	memcpy(client->clipammo, savedClipAmmo, sizeof(savedClipAmmo));
	memcpy(client->ps.armor, savedArmor, sizeof(savedArmor));
	ent->client->ps.firingMode = savedFiringMode;
	ent->client->ps.ammoType = savedAmmoType;
	ent->client->ps.stats[STAT_AMMO] = ent->client->clipammo[savedWeaponId][ent->client->ps.firingMode];
	ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];

	client->bodyGrabIndex = ENTITYNUM_NONE;

	//Get the skin RGB based on his userinfo
	value = Info_ValueForKey (userinfo, "char_color_red");
	if (value)
	{
		client->ps.customRGBA[0] = atoi(value);
	}
	else
	{
		client->ps.customRGBA[0] = 255;
	}

	value = Info_ValueForKey (userinfo, "char_color_green");
	if (value)
	{
		client->ps.customRGBA[1] = atoi(value);
	}
	else
	{
		client->ps.customRGBA[1] = 255;
	}

	value = Info_ValueForKey (userinfo, "char_color_blue");
	if (value)
	{
		client->ps.customRGBA[2] = atoi(value);
	}
	else
	{
		client->ps.customRGBA[2] = 255;
	}

	if ((client->ps.customRGBA[0]+client->ps.customRGBA[1]+client->ps.customRGBA[2]) < 100)
	{ //hmm, too dark!
		client->ps.customRGBA[0] = client->ps.customRGBA[1] = client->ps.customRGBA[2] = 255;
	}

	client->ps.customRGBA[3]=255;

	l = 0;
	while (l < MAX_SABERS)
	{
		client->saber[l] = saberSaved[l];
		client->weaponGhoul2[l] = g2WeaponPtrs[l];
		l++;
	}

	//or the saber ent num
	client->ps.saberEntityNum = saveSaberNum;
	client->saberStoredIndex = saveSaberNum;

	client->ps.fd = savedForce;

	client->ps.duelIndex = ENTITYNUM_NONE;

	if (client->ps.jetpack) {
		client->ps.jetpackFuel = jetpackTable[client->ps.jetpack - 1].fuelCapacity;
	}
	else {
		client->ps.jetpackFuel = 0;
	}
	client->ps.cloakFuel = 100;

	// start out with full block points --eez
	client->ps.blockPoints = 100;

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;
	client->ps.credits = savedCredits;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	client->airOutTime = level.time + 12000;

	// set max health
	maxHealth = 100;
	client->pers.maxHealth = maxHealth;//atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > maxHealth ) {
		client->pers.maxHealth = 100;
	}

	// set max stamina
	if (client->pers.maxStamina < 1 || client->pers.maxStamina > 100)
	{
		client->pers.maxStamina = 100;
	}

	// clear entity values
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
	client->ps.stats[STAT_MAX_STAMINA] = client->pers.maxStamina;

	client->ps.eFlags = flags;
	client->mGameFlags = gameFlags;

	client->ps.groundEntityNum = ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->playerState = &ent->client->ps;
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);
	client->ps.crouchheight = CROUCH_MAXS_2;
	client->ps.standheight = DEFAULT_MAXS_2;

	client->ps.clientNum = index;
	//give default weapons
	client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );
	
    client->ps.weapon = 0;
    client->ps.weaponId = BG_GetWeaponIndexFromClass (client->ps.weapon, 0);
	
	// MOAR OVERRIDING OF WEAPONS.
	if ( respawn )
	{
	    int weapon = 0;
	    int variation = 0;
	    if ( !BG_GetWeaponByIndex (savedWeaponId, &weapon, &variation) )
	    {
	        weapon = variation = 0;
	        savedWeaponId = 0;
	    }
	    
        client->ps.weaponId = savedWeaponId;
        client->ps.weapon = weapon;
        client->ps.weaponVariation = variation;
	}

	if ( client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		client->ps.stats[STAT_WEAPONS] = 0;
	}
	else
	{
		if (level.startingWeapon[0])
		{
			const weaponData_t *weapon = BG_GetWeaponByClassName (level.startingWeapon);
			if(weapon)
			{
				//FIXME: The below assumes that there is a valid weapon item
				int itemID = BG_GetItemByWeaponIndex(BG_GetWeaponIndex((unsigned int)weapon->weaponBaseIndex, (unsigned int)weapon->weaponModIndex))->itemID;

				for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
					if (it->id) {
						if (it->id->itemID == itemID) {
							haveItem = qtrue;
							break;
						}
					}
				}

				if(!haveItem && ent->inventory->size() < 1)
				{
					// Don't have any sort of item in our inventory
					if(ent->client->ps.credits < jkg_startingCredits.integer)
					{
						itemInstance_t item = BG_ItemInstance(itemID, 1);
						ent->client->ps.credits = jkg_startingCredits.integer;
						//ent->client->ps.spent = 0;

						int delta = level.time - level.startTime;	//how long has the match been going?
						//award missing passive credits if enabled
						if (jkg_passiveCreditsAmount.integer > 0)
						{
							//award if we joined at least jkg_passiveCreditsWait late (typically 1 minute)
							if (delta > jkg_passiveCreditsWait.integer)
							{
								int reward = 0;
								reward = (jkg_passiveCreditsAmount.integer * (delta / jkg_passiveCreditsRate.integer));				//calculate amount we would have got
								if (jkg_passiveCreditsWait.integer > jkg_passiveCreditsRate.integer)
									reward -= (jkg_passiveCreditsAmount.integer * (jkg_passiveCreditsWait.integer / jkg_passiveCreditsRate.integer));		//minus the initial wait before credits are disbursed
								client->ps.credits += reward;
							}
						}

						//underdog reward if you join the losing team late
						if (jkg_underdogBonus.integer > 0 && (delta > jkg_passiveCreditsWait.integer))
						{
							//who is currently winning?
							auto my_team = ent->client->sess.sessionTeam; int curr_winner = -1; int money = 0;

							if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE])
								curr_winner = TEAM_RED;
							else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE])
								curr_winner = TEAM_BLUE;
							else
								curr_winner = -1;	//tie

							//if we are the loser
							if (my_team != curr_winner && my_team != TEAM_SPECTATOR && curr_winner != -1)
							{
								int score_diff = 0; 

								//find score difference
								if (my_team == TEAM_RED)
									score_diff = level.teamScores[TEAM_BLUE] - level.teamScores[TEAM_RED];
								else if (my_team == TEAM_BLUE)
									score_diff = level.teamScores[TEAM_RED] - level.teamScores[TEAM_BLUE];
								else
									;

								//calculate reward based on how much time in the match is left
								float match_percent = ((delta / ((float)(timelimit.integer * 60000))) * 100);
								if (30 <= match_percent && match_percent < 45)
									money += (jkg_startingCredits.integer * 0.25);
								else if (45 <= match_percent && match_percent < 60)
									money += (jkg_startingCredits.integer * 0.4);
								else if (60 <= match_percent && match_percent < 65)
									money += (jkg_startingCredits.integer * 0.7);
								else if (65 <= match_percent && match_percent < 70)
									money += (jkg_startingCredits.integer * 0.8);
								else if (70 <= match_percent && match_percent < 80)
									money += jkg_startingCredits.integer;
								else if (80 <= match_percent && match_percent < 101)
									money += (jkg_startingCredits.integer * 0.25) + jkg_startingCredits.integer;
								else
									money += (jkg_startingCredits.integer * 0.10);

								if ((score_diff < 3 && level.gametype != GT_CTF) || (level.gametype == GT_CTF && score_diff < 201))
									money = money / 2;

								trap->SendServerCommand(ent->s.number, va("notify 1 \"Underdog Bonus: +%i Credits\"", money));
								client->ps.credits += money;
							}

						}

						BG_GiveItem(ent, item, true);

						// Give max ammo for both firing modes
						for (int i = 0; i < weapon->numFiringModes; i++) {
							ammo_t* ammo = weapon->firemodes[i].ammoDefault;
							BG_GiveAmmo(ent, ammo);	// give us the actual ammo
							ent->client->clipammo[item.id->weaponData.varID][i] = weapon->firemodes[i].clipSize;
							ent->client->ammoTypes[item.id->weaponData.varID][i] = weapon->firemodes[i].ammoDefault->ammoIndex;
						}
					}
				}
			}
		}
	}

	//rww - Set here to initialize the circling seeker drone to off.
	//A quick note about this so I don't forget how it works again:
	//ps.genericEnemyIndex is kept in sync between the server and client.
	//When it gets set then an entitystate value of the same name gets
	//set along with an entitystate flag in the shared bg code. Which
	//is why a value needs to be both on the player state and entity state.
	//(it doesn't seem to just carry over the entitystate value automatically
	//because entity state value is derived from player state data or some
	//such)
	client->ps.genericEnemyIndex = -1;

	if (client->ps.fallingToDeath)
	{
		client->ps.fallingToDeath = 0;
		client->noCorpse = qtrue;
	}

	//Do per-spawn force power initialization
	WP_SpawnInitForcePowers( ent );

	//check if we need to give more hp due to items
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) 
	{
		if (it->equipped && ( it->id->itemType == ITEM_ARMOR || it->id->itemType == ITEM_CLOTHING) )
		{
			ent->client->ps.stats[STAT_MAX_HEALTH] += it->id->armorData.pArm->hp;
			client->ps.stats[STAT_MAX_STAMINA] += it->id->armorData.pArm->stamina;
		}
	}
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH];  // health will count down towards max_health (if it exceeds)
	
	ent->playerState->forcePower = client->ps.stats[STAT_MAX_STAMINA];  //most common usage is: pm->ps->forcePower

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap->GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {

	} else {
		G_KillBox( ent );
		trap->LinkEntity ((sharedEntity_t *)ent);

		client->ps.torsoTimer = client->ps.legsTimer = 0;

		if (client->ps.weapon == WP_SABER)
		{
			G_SetAnim(ent, NULL, SETANIM_BOTH, BOTH_STAND1TO2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		}
		else
		{
			G_SetAnim(ent, NULL, SETANIM_TORSO, TORSO_RAISEWEAP1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			if( client->ps.ironsightsTime & IRONSIGHTS_MSB )
				client->ps.legsAnim = GetWeaponData (client->ps.weapon, client->ps.weaponVariation)->anims.sights.legsAnim;
			else
				client->ps.legsAnim = GetWeaponData (client->ps.weapon, client->ps.weaponVariation)->anims.ready.legsAnim;
		}
		client->ps.weaponstate = WEAPON_RAISING;
		client->ps.weaponTime = client->ps.torsoTimer;
	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		if (spawnPoint) G_UseTargets( spawnPoint, ent );
		
#ifdef __WAYPOINT_SPAWNS__
		if (gWPNum > 0)
		{
			vec3_t		org;
			gentity_t	*npc = NULL;
			int			waypoint = irand(0, gWPNum-1);
			int			random = irand(0,10);
			int			tries = 0;

			while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint ))
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

			VectorCopy(gWPArray[waypoint]->origin, org);
			org[2]+=48;
			G_SetOrigin(ent, gWPArray[waypoint]->origin);
			VectorCopy(org, ent->client->ps.origin);
			VectorCopy(org, ent->r.currentOrigin);
			VectorCopy(org, ent->s.origin);
			VectorCopy(org, ent->s.pos.trBase);
		}
#endif //__WAYPOINT_SPAWNS__

		if ( client->sess.sessionTeam != TEAM_SPECTATOR && use_secondary_spawnpoint)
		{// Warzone gametype flag spawnpoint usage...
			spawnPoint = SelectWarzoneSpawnpoint( ent );

			VectorCopy(spawnPoint->s.origin, ent->client->ps.origin);
			VectorCopy(spawnPoint->s.origin, ent->s.origin);
			VectorCopy(spawnPoint->s.origin, ent->r.currentOrigin);

			G_SetOrigin(ent, spawnPoint->s.origin);
			VectorCopy(spawnPoint->s.origin, ent->client->ps.origin);
			VectorCopy(spawnPoint->s.origin, ent->r.currentOrigin);
			VectorCopy(spawnPoint->s.origin, ent->s.origin);
			VectorCopy(spawnPoint->s.origin, ent->s.pos.trBase);

			if (client->sess.sessionTeam == TEAM_RED)
			{// Decrease the tickets for this team...
				redtickets--;
			}
			else
			{// Decrease the tickets for this team...
				bluetickets--;
			}

			trap->SendServerCommand( -1, va("tkt %i %i", redtickets, bluetickets ));
		}
	}

	//set teams for NPCs to recognize
	ent->s.teamowner = NPCTEAM_PLAYER;
	client->playerTeam = (npcteam_t)ent->s.teamowner;
	client->enemyTeam = NPCTEAM_ENEMY;

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities, NULL );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap->LinkEntity( (sharedEntity_t *)ent );
	}

	if (g_spawnInvulnerability.integer)
	{
		ent->client->ps.eFlags |= EF_INVULNERABLE;
		ent->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
	}

	// UQ1: Again, use an event :)
	if (!(ent->r.svFlags & SVF_BOT))
		trap->SendServerCommand(ent->s.number, "dcr");

	// Iterate through all items in the inventory and reequip stuff that might have been unequipped by clearing out the client data
	if (ent->inventory) {
		for (i = 0; i < ent->inventory->size(); i++) {
			auto it = ent->inventory->begin() + i;
			if (it->equipped && it->id->itemType == ITEM_SHIELD) {
				JKG_ShieldEquipped(ent, i, qfalse);
			}
			else if (it->equipped && it->id->itemType == ITEM_JETPACK) {
				JKG_JetpackEquipped(ent, i);
			}
			else if (it->id->itemType == ITEM_WEAPON) {
				// It's a weapon, automatically reload us
				weaponData_t* wp = GetWeaponData(it->id->weaponData.weapon, it->id->weaponData.variation);
				for (int j = 0; j < wp->numFiringModes; j++)
				{
					if (wp->firemodes[j].clipSize > 0)
					{ // this fire mode has a clip, reload us
						int diff = wp->firemodes[j].clipSize - ent->client->clipammo[it->id->weaponData.varID][j];
						int ammoType = ent->client->ammoTypes[it->id->weaponData.varID][j];

						if (diff > ent->client->ammoTable[ammoType])
						{
							diff = ent->client->ammoTable[ammoType];
						}

						// add to the ammo in this clip for this firing mode and subtract from the pooled ammo
						ent->client->clipammo[it->id->weaponData.varID][j] += diff;
						ent->client->ammoTable[ammoType] -= diff;
					}
				}
			}
		}
	}

	if (ent->client->shieldEquipped)
	{
		ent->client->ps.stats[STAT_SHIELD] = ent->client->ps.stats[STAT_MAX_SHIELD];
	}

	if (ent->client->jetpackEquipped)
		ent->client->ps.jetpackFuel = jetpackTable[ent->client->ps.jetpack - 1].fuelCapacity;	//fill jetpack to capacity when spawned

	/*if (client->ps.jetpack) {
		client->ps.jetpackFuel = jetpackTable[client->ps.jetpack - 1].fuelCapacity;		//fill jetpack
	}*/

	ent->client->ps.heat = 0.0f;		//set initial heat on spawn
	ent->client->ps.overheated = false;
	if (ent->client->ps.maxHeat < 1 || ent->client->ps.heatThreshold < 1)
	{
		const weaponData_t* wp = GetWeaponData(ent->client->ps.weapon, ent->client->ps.weaponVariation);
		ent->client->ps.maxHeat = wp->firemodes[ent->client->ps.firingMode].maxHeat;
		ent->client->ps.heatThreshold = wp->firemodes[ent->client->ps.firingMode].heatThreshold;

		//double check the weapon has a maxHeat and heatThreshold settings, if not set it to 100 and 75%.
		if (ent->client->ps.maxHeat < 1)
			ent->client->ps.maxHeat = 100;

		if (ent->client->ps.heatThreshold < 1)
			ent->client->ps.heatThreshold = ent->client->ps.maxHeat * 0.75;
	}

	ent->client->ps.consumableTime = level.time;	//mark when consumables can be used as 'now'

	GLua_Hook_PlayerSpawned(ent->s.number);

	// run the presend to set anything else
	if ( ent->client->sess.spectatorState != SPECTATOR_FOLLOW )
		ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );

	//rww - make sure client has a valid icarus instance
	trap->ICARUS_FreeEnt( (sharedEntity_t *)ent );
	trap->ICARUS_InitEnt( (sharedEntity_t *)ent );

	// set their weapon
	trap->SendServerCommand(client->ps.clientNum, "aciset 1");
}


/*
===========
G_ClearVote

===========
*/

void G_ClearVote( gentity_t *ent ) {
	if ( level.voteTime ) {
		if ( ent->client->mGameFlags & PSG_VOTED ) {
			if ( ent->client->pers.vote == 1 ) {
				level.voteYes--;
				trap->SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
			}
			else if ( ent->client->pers.vote == 2 ) {
				level.voteNo--;
				trap->SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
			}
		}
		ent->client->mGameFlags &= ~(PSG_VOTED);
		ent->client->pers.vote = 0;
	}
}

/*
===========
G_ClearTeamVote

===========
*/

void G_ClearTeamVote( gentity_t *ent, int team ) {
	int voteteam;

	if ( team == TEAM_RED )			voteteam = 0;
	else if ( team == TEAM_BLUE )	voteteam = 1;
	else							return;

	if ( level.teamVoteTime[voteteam] ) {
		if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
			if ( ent->client->pers.teamvote == 1 ) {
				level.teamVoteYes[voteteam]--;
				trap->SetConfigstring( CS_TEAMVOTE_YES, va( "%i", level.teamVoteYes[voteteam] ) );
			}
			else if ( ent->client->pers.teamvote == 2 ) {
				level.teamVoteNo[voteteam]--;
				trap->SetConfigstring( CS_TEAMVOTE_NO, va( "%i", level.teamVoteNo[voteteam] ) );
			}
		}
		ent->client->mGameFlags &= ~(PSG_TEAMVOTED);
		ent->client->pers.teamvote = 0;
	}
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap->DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	TeamDisconnect( clientNum );

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	if (ent->inventory != nullptr) {
		ent->inventory->clear();
	}

	if (ent->assists != nullptr) {
		ent->assists->clear();
	}

	GLua_Hook_PlayerDisconnect(clientNum);

	i = 0;

	while (i < NUM_FORCE_POWERS)
	{
		if (ent->client->ps.fd.forcePowersActive & (1 << i))
		{
			WP_ForcePowerStop(ent, i);
		}
		i++;
	}

	i = TRACK_CHANNEL_1;

	while (i < NUM_TRACK_CHANNELS)
	{
		if (ent->client->ps.fd.killSoundEntIndex[i-50] && ent->client->ps.fd.killSoundEntIndex[i-50] < MAX_GENTITIES && ent->client->ps.fd.killSoundEntIndex[i-50] > 0)
		{
			G_MuteSound(ent->client->ps.fd.killSoundEntIndex[i-50], CHAN_VOICE);
		}
		i++;
	}
	i = 0;

	if ( ent->client->ewebIndex )
	{
		gentity_t *eweb = &g_entities[ent->client->ewebIndex];

		ent->client->ps.emplacedIndex = 0;
		ent->client->ewebIndex = 0;
		ent->client->ewebHealth = 0;
		G_FreeEntity( eweb );
	}

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
	}

	G_LogPrintf( "ClientDisconnect: %i [%s] (%s) \"%s^7\"\n", clientNum, ent->client->sess.IP, ent->client->pers.guid, ent->client->pers.netname );

	// if we are playing in tourney mode, give a win to the other player and clear his frags for this round
	if ( (level.gametype == GT_DUEL )
		&& !level.intermissiontime ) {
		if ( level.sortedClients[1] == clientNum ) {
			level.clients[ level.sortedClients[0] ].ps.persistant[PERS_SCORE] = 0;
			level.clients[ level.sortedClients[0] ].sess.wins++;
			ClientUserinfoChanged( level.sortedClients[0] );
		}
		else if ( level.sortedClients[0] == clientNum ) {
			level.clients[ level.sortedClients[1] ].ps.persistant[PERS_SCORE] = 0;
			level.clients[ level.sortedClients[1] ].sess.wins++;
			ClientUserinfoChanged( level.sortedClients[1] );
		}
	}

	if (ent->ghoul2 && trap->G2_HaveWeGhoul2Models(ent->ghoul2))
	{
		trap->G2API_CleanGhoul2Models(&ent->ghoul2);
	}
	i = 0;
	while (i < MAX_SABERS)
	{
		if (ent->client->weaponGhoul2[i] && trap->G2_HaveWeGhoul2Models(ent->client->weaponGhoul2[i]))
		{
			trap->G2API_CleanGhoul2Models(&ent->client->weaponGhoul2[i]);
		}
		i++;
	}

	G_ClearVote( ent );
	G_ClearTeamVote( ent, ent->client->sess.sessionTeam );

	trap->UnlinkEntity ((sharedEntity_t *)ent);

	// Jedi Knight Galaxies
	GLua_Wipe_EntDataSlot(ent);
	ent->IDCode = 0;
	ent->LuaUsable = 0;

	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;
	ent->r.contents = 0;

	trap->SetConfigstring( CS_PLAYERS + clientNum, "");


	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}

	G_ClearClientLog(clientNum);
}


