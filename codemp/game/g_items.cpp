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

#include "g_local.h"
#include "ghoul2/G2.h"
#include "qcommon/q_shared.h"

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_ARMOR		20
#define	RESPAWN_TEAM_WEAPON	30
#define	RESPAWN_HEALTH		30
#define	RESPAWN_AMMO		40
#define	RESPAWN_HOLDABLE	60
#define	RESPAWN_MEGAHEALTH	120
#define	RESPAWN_POWERUP		120

// Item Spawn flags
#define ITMSF_SUSPEND		1
#define ITMSF_NOPLAYER		2
#define ITMSF_ALLOWNPC		4
#define ITMSF_NOTSOLID		8
#define ITMSF_VERTICAL		16
#define ITMSF_INVISIBLE		32

extern gentity_t *droppedRedFlag;
extern gentity_t *droppedBlueFlag;


//======================================================================
#define MAX_MEDPACK_HEAL_AMOUNT		25
#define MAX_MEDPACK_BIG_HEAL_AMOUNT	50
#define MAX_SENTRY_DISTANCE			256

// For more than four players, adjust the respawn times, up to 1/4.
int adjustRespawnTime(float preRespawnTime, int itemType, int itemTag)
{
	float respawnTime = preRespawnTime;

	if (itemType == IT_WEAPON)
	{
		if (itemTag == WP_THERMAL ||
			itemTag == WP_TRIP_MINE ||
			itemTag == WP_DET_PACK)
		{ //special case for these, use ammo respawn rate
			respawnTime = RESPAWN_AMMO;
		}
	}

	return((int)respawnTime);
}

#define JETPACK_TOGGLE_TIME		200
void Jetpack_Off(gentity_t *ent)
{ //create effects?
	assert(ent && ent->client);

	if (!ent->client->jetPackOn)
	{ //aready off
		return;
	}

	ent->client->jetPackOn = qfalse;
	G_Sound(ent, CHAN_AUTO, G_SoundIndex(  "sound/jkg/jetpack/jetoff" /*"sound/boba/JETON"*/ ));
}

void Jetpack_On(gentity_t *ent)
{ //create effects?
	assert(ent && ent->client);

	if (ent->client->jetPackOn)
	{ //aready on
		return;
	}

	if (ent->client->ps.fd.forceGripBeingGripped >= level.time)
	{ //can't turn on during grip interval
		return;
	}

	if (ent->client->ps.fallingToDeath)
	{ //too late!
		return;
	}

	G_Sound(ent, CHAN_AUTO, G_SoundIndex(  "sound/jkg/jetpack/jeton" /*"sound/boba/JETON"*/ ));

	ent->client->jetPackOn = qtrue;
}

void ItemUse_Jetpack( gentity_t *ent )
{
	assert(ent && ent->client);

	if (ent->client->jetPackToggleTime >= level.time)
	{
		return;
	}

	if (ent->health <= 0 ||
		ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		(ent->client->ps.eFlags & EF_DEAD) ||
		ent->client->ps.pm_type == PM_DEAD)
	{ //can't use it when dead under any circumstances.
		return;
	}

	if (!ent->client->jetPackOn &&
		ent->client->ps.jetpackFuel < 5)
	{ //too low on fuel to start it up
		return;
	}

	if (ent->client->jetPackOn)
	{
		Jetpack_Off(ent);
	}
	else
	{
		Jetpack_On(ent);
	}

	ent->client->jetPackToggleTime = level.time + JETPACK_TOGGLE_TIME;
}

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int			i;
	gclient_t	*client;

	if ( !other->client->ps.powerups[ent->item->giTag] ) {
		// round timing to seconds to make multiple powerup timers
		// count in sync
		other->client->ps.powerups[ent->item->giTag] = 
			level.time - ( level.time % 1000 );

		G_LogWeaponPowerup(other->s.number, ent->item->giTag);
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;

	// give any nearby players a "denied" anti-reward
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		vec3_t		delta;
		float		len;
		vec3_t		forward;
		trace_t		tr;

		client = &level.clients[i];
		if ( client == other->client ) {
			continue;
		}
		if ( client->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			continue;
		}

    // if same team in team game, no sound
    // cannot use OnSameTeam as it expects to g_entities, not clients
  	if ( level.gametype >= GT_TEAM && other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
      continue;
    }

		// if too far away, no sound
		VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
		len = VectorNormalize( delta );
		if ( len > 192 ) {
			continue;
		}

		// if not facing, no sound
		AngleVectors( client->ps.viewangles, forward, NULL, NULL );
		if ( DotProduct( delta, forward ) < 0.4f ) {
			continue;
		}

		// if not line of sight, no sound
		trap->Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID , 0, 0, 0);
		if ( tr.fraction != 1.0f ) {
			continue;
		}

		// anti-reward
		client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
	}
	return RESPAWN_POWERUP;
}

//======================================================================

void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	//int ammoIndex = GetWeaponAmmoIndex( weapon, ent->s.weaponVariation );
	//int ammoMax = GetWeaponAmmoMax( weapon, ent->s.weaponVariation );
	int ammoIndex = weapon;
	int ammoMax = GetAmmoMax(ammoIndex);

	// TODO: Add proper ammo array here
	if ( ent->client->ammoTable[ammoIndex] < ammoMax )
	{
		ent->client->ammoTable[ammoIndex] += count;

		if ( ent->client->ammoTable[ammoIndex] > ammoMax )
		{
			ent->client->ammoTable[ammoIndex] = ammoMax;
		}
	}
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	if (ent->item->giTag == -1)
	{ //an ammo_all, give them a bit of everything
		Add_Ammo(other, AMMO_BLASTER, 50);
		Add_Ammo(other, AMMO_POWERCELL, 50);
		Add_Ammo(other, AMMO_METAL_BOLTS, 50);
		Add_Ammo(other, AMMO_ROCKETS, 2);
	}
	else
	{
		Add_Ammo (other, ent->item->giTag, quantity);
	}

	return adjustRespawnTime(RESPAWN_AMMO, ent->item->giType, ent->item->giTag);
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other) {
	int		quantity;
	weaponData_t *wep;

	if ( ent->count < 0 ) {
		quantity = 0; // None for you, sir!
	} else {
		if ( ent->count ) {
			quantity = ent->count;
		} else {
			wep = GetWeaponData(ent->item->giTag, ent->s.weaponVariation);
			quantity = wep->ammoOnPickup;
		}

		// dropped items and teamplay weapons always have full ammo
		if ( ! (ent->flags & FL_DROPPED_ITEM) && level.gametype != GT_TEAM ) {
			if ( other->client->ammoTable[ ent->item->giTag ] < quantity*0.5f ) {
				quantity = quantity - other->client->ammoTable[ ent->item->giTag ];
			} else {
				quantity = quantity*0.5f;		// only add half the value.
			}
		}
	}

	// Check if the player already has the weapon in his inventory
	if (other->client->ps.stats[STAT_WEAPONS] & ( 1 << ent->item->giTag )) {
		// The player already has it
		// In this case, check if the current clip is fully empty
		// If so, fill it up (as far as possible) and put the remaining ammo in the clips
		// Otherwise, just add the clips
		
		// Of course, the weapon has to use clips :P otherwise we use the default method

		if ( GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation ) && !other->client->clipammo[ent->item->giTag]) {
			// Gun is empty, fill it up
			if (quantity < GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation )) {
				other->client->clipammo[ent->item->giTag] = quantity;
			} else {
				// More than enough to fill the clip
				other->client->clipammo[ent->item->giTag] = GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation );
				quantity -= GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation );
				Add_Ammo( other, GetWeaponAmmoIndex( ent->item->giTag, ent->s.weaponVariation ), quantity );
			}
		} else {
			// Just add the ammo
			//Add_Ammo( other, ent->item->giTag, quantity );
			Add_Ammo( other, GetWeaponAmmoIndex( ent->item->giTag, ent->s.weaponVariation ), quantity );
		}
	} else {
		// add the weapon
		other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );

		// Always fill up the clip inside the weapon on pickup (unless the weapon does not use clips)
		if ( GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation ))
		{
			if (GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation )) {
				other->client->clipammo[ent->item->giTag] = quantity;
				quantity = 0;
			} else {
				// More than enough to fill the clip
				other->client->clipammo[ent->item->giTag] = GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation );
				quantity -= GetWeaponAmmoClip( ent->item->giTag, ent->s.weaponVariation );
			}
		}
		//Add_Ammo( other, ent->item->giTag, quantity );
		Add_Ammo( other, GetWeaponAmmoIndex( ent->item->giTag, ent->s.weaponVariation ), quantity );
	}
	G_LogWeaponPickup(other->s.number, ent->item->giTag);
	
	// team deathmatch has slow weapon respawns
	if ( level.gametype >= GT_TEAM ) 
	{
		return adjustRespawnTime(RESPAWN_TEAM_WEAPON, ent->item->giType, ent->item->giTag);
	}

	return adjustRespawnTime(5, ent->item->giType, ent->item->giTag);
}


//======================================================================

void HealingPlum( gentity_t *ent, vec3_t origin, int amount );

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;

	// small and mega healths will go over the max
	if ( ent->item->quantity != 5 && ent->item->quantity != 100 ) {
		max = other->client->ps.stats[STAT_MAX_HEALTH];
	} else {
		max = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->health += quantity;

	HealingPlum(ent, ent->r.currentOrigin, quantity);

	if (other->health > max ) {
		other->health = max;
	}
	other->client->ps.stats[STAT_HEALTH] = other->health;

	if ( ent->item->quantity == 100 ) {		// mega health respawns slow
		return RESPAWN_MEGAHEALTH;
	}

	return adjustRespawnTime(RESPAWN_HEALTH, ent->item->giType, ent->item->giTag);
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) 
{
	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
	if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_ARMOR] * ent->item->giTag )
	{
		other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_ARMOR] * ent->item->giTag;
	}

	return adjustRespawnTime(RESPAWN_ARMOR, ent->item->giType, ent->item->giTag);
}

//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
	// randomly select from teamed entities
	if (ent->team) {
		gentity_t	*master;
		int	count;
		int choice;

		if ( !ent->teammaster ) {
			trap->Error( ERR_DROP, "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
			;
	}

	ent->r.contents = CONTENTS_ITEM; //CONTENTS_TRIGGER;
	//ent->s.eFlags &= ~EF_NODRAW;
	ent->s.eFlags &= ~(EF_NODRAW | EF_ITEMPLACEHOLDER);
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap->LinkEntity((sharedEntity_t *)ent);

	if ( ent->item->giType == IT_POWERUP ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/respawn1" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	ent->nextthink = 0;
}

qboolean CheckItemCanBePickedUpByNPC( gentity_t *item, gentity_t *pickerupper )
{
	if ( (item->flags&FL_DROPPED_ITEM) 
		&& item->activator != &g_entities[0] 
		&& pickerupper->s.number 
		&& pickerupper->s.weapon == WP_NONE 
		&& pickerupper->enemy 
		&& pickerupper->painDebounceTime < level.time
		&& pickerupper->NPC && pickerupper->NPC->surrenderTime < level.time //not surrendering
		&& !(pickerupper->NPC->scriptFlags&SCF_FORCED_MARCH) //not being forced to march
		/*&& item->item->giTag != INV_SECURITY_KEY*/ )
	{//non-player, in combat, picking up a dropped item that does NOT belong to the player and it *not* a security key
		if ( level.time - item->s.time < 3000 )//was 5000
		{
			return qfalse;
		}
		return qtrue;
	}
	return qfalse;
}

/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;
	qboolean	predict;

	if (ent->genericValue10 > level.time &&
		other &&
		other->s.number == ent->genericValue11)
	{ //this is the ent that we don't want to be able to touch us for x seconds
		return;
	}

	if (ent->s.eFlags & EF_ITEMPLACEHOLDER)
	{
		return;
	}

	if (ent->s.eFlags & EF_NODRAW)
	{
		return;
	}

	if (ent->item->giType == IT_WEAPON &&
		ent->s.powerups &&
		ent->s.powerups < level.time)
	{
		ent->s.generic1 = 0;
		ent->s.powerups = 0;
	}

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup

	// the same pickup rules are used for client side and server side
	if ( !BG_CanItemBeGrabbed( level.gametype, &ent->s, &other->client->ps ) ) {
		return;
	}

	
	if ( other->client->NPC_class == CLASS_ATST || 
		other->client->NPC_class == CLASS_GONK || 
		other->client->NPC_class == CLASS_MARK1 || 
		other->client->NPC_class == CLASS_MARK2 || 
		other->client->NPC_class == CLASS_MOUSE || 
		other->client->NPC_class == CLASS_PROBE || 
		other->client->NPC_class == CLASS_PROTOCOL || 
		other->client->NPC_class == CLASS_R2D2 || 
		other->client->NPC_class == CLASS_R5D2 || 
		other->client->NPC_class == CLASS_SEEKER || 
		other->client->NPC_class == CLASS_REMOTE || 
		other->client->NPC_class == CLASS_RANCOR || 
		other->client->NPC_class == CLASS_WAMPA || 
		//other->client->NPC_class == CLASS_JAWA || //FIXME: in some cases it's okay?
		other->client->NPC_class == CLASS_UGNAUGHT || //FIXME: in some cases it's okay?
		other->client->NPC_class == CLASS_SENTRY )
	{//FIXME: some flag would be better
		//droids can't pick up items/weapons!
		return;
	}

	if ( CheckItemCanBePickedUpByNPC( ent, other ) )
	{
		if ( other->NPC && other->NPC->goalEntity && other->NPC->goalEntity->enemy == ent )
		{//they were running to pick me up, they did, so clear goal
			other->NPC->goalEntity = NULL;
			other->NPC->squadState = SQUAD_STAND_AND_SHOOT;
		}
	}
	else if ( !(ent->spawnflags &  ITMSF_ALLOWNPC) )
	{// NPCs cannot pick it up
		if ( other->s.eType == ET_NPC )
		{// Not the player?
			qboolean dontGo = qfalse;
			if (ent->item->giType == IT_AMMO &&
				ent->item->giTag == -1 &&
				other->s.NPC_class == CLASS_VEHICLE &&
				other->m_pVehicle &&
				other->m_pVehicle->m_pVehicleInfo->type == VH_WALKER)
			{ //yeah, uh, atst gets healed by these things
                if (other->maxHealth &&
					other->health < other->maxHealth)
				{
					other->health += 80;
					if (other->health > other->maxHealth)
					{
						other->health = other->maxHealth;
					}
					G_ScaleNetHealth(other);
					dontGo = qtrue;
				}
			}

			if (!dontGo)
			{
				return;
			}
		}
	}

	G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );

	predict = other->client->pers.predictItemPickup;

	// call the item-specific pickup function
	switch( ent->item->giType ) {
	case IT_WEAPON:
		respawn = Pickup_Weapon(ent, other);
		predict = qtrue;
		break;
	case IT_AMMO:
		respawn = Pickup_Ammo(ent, other);
		predict = qtrue;
		break;
	case IT_ARMOR:
		respawn = Pickup_Armor(ent, other);
		predict = qtrue;
		break;
	case IT_HEALTH:
		respawn = Pickup_Health(ent, other);
		predict = qtrue;
		break;
	case IT_POWERUP:
		respawn = Pickup_Powerup(ent, other);
		predict = qfalse;
		break;
	case IT_TEAM:
		respawn = Pickup_Team(ent, other);
		break;
	default:
		return;
	}

	if ( !respawn ) {
		return;
	}

	// play the normal pickup sound
	if (predict) {
		if (other->client)
		{
			BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, ent->s.number, &other->client->ps);
		}
		else
		{
			G_AddPredictableEvent( other, EV_ITEM_PICKUP, ent->s.number );
		}
	} else {
		G_AddEvent( other, EV_ITEM_PICKUP, ent->s.number );
	}

	// powerup pickups are global broadcasts
	if ( /*ent->item->giType == IT_POWERUP ||*/ ent->item->giType == IT_TEAM) {
		// if we want the global sound to play
		if (!ent->speed) {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			te->r.svFlags |= SVF_BROADCAST;
		} else {
			gentity_t	*te;

			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
			te->s.eventParm = ent->s.modelindex;
			// only send this temp entity to a single client
			te->r.svFlags |= SVF_SINGLECLIENT;
			te->r.singleClient = other->s.number;
		}
	}

	// fire item targets
	G_UseTargets (ent, other);

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// non zero wait overrides respawn time
	if ( ent->wait ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += crandom() * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	if (!(ent->flags & FL_DROPPED_ITEM) && (ent->item->giType==IT_WEAPON || ent->item->giType==IT_POWERUP))
	{
		ent->s.eFlags |= EF_ITEMPLACEHOLDER;
		ent->s.eFlags &= ~EF_NODRAW;
	}
	else
	{
		ent->s.eFlags |= EF_NODRAW;
		ent->r.svFlags |= SVF_NOCLIENT;
	}
	ent->r.contents = 0;

	if (ent->genericValue9)
	{ //dropped item, should be removed when picked up
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	// ZOID
	// A negative respawn times means to never respawn this item (but don't 
	// delete it).  This is used by items that are respawned by third party 
	// events such as ctf flags
	if ( respawn <= 0 ) {
		ent->nextthink = 0;
		ent->think = 0;
	} else {
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
	}
	trap->LinkEntity( (sharedEntity_t *)ent );
}


//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) {
	gentity_t	*dropped;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	if (dropped->s.modelindex < 0)
	{
		dropped->s.modelindex = 0;
	}
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);

	dropped->r.contents = CONTENTS_ITEM; //CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->flags |= FL_BOUNCE_HALF;
	if (level.gametype == GT_CTF && item->giType == IT_TEAM) { // Special case for CTF flags
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + 30000;
		Team_CheckDroppedItem( dropped );

		//rww - so bots know
		if (strcmp(dropped->classname, "team_CTF_redflag") == 0)
		{
			droppedRedFlag = dropped;
		}
		else if (strcmp(dropped->classname, "team_CTF_blueflag") == 0)
		{
			droppedBlueFlag = dropped;
		}
	} else { // auto-remove after 30 seconds
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	dropped->flags = FL_DROPPED_ITEM;

	if (item->giType == IT_WEAPON || item->giType == IT_POWERUP)
	{
		dropped->s.eFlags |= EF_DROPPEDWEAPON;
	}

	vectoangles(velocity, dropped->s.angles);
	dropped->s.angles[PITCH] = 0;

	if (item->giTag == WP_TRIP_MINE ||
		item->giTag == WP_DET_PACK)
	{
		dropped->s.angles[PITCH] = -90;
	}

	if (item->giTag != WP_BOWCASTER &&
		item->giTag != WP_DET_PACK &&
		item->giTag != WP_THERMAL)
	{
		dropped->s.angles[ROLL] = -90;
	}

	dropped->physicsObject = qtrue;

	trap->LinkEntity((sharedEntity_t *)dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle ) {
	vec3_t	velocity;
	vec3_t	angles;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;
	
	return LaunchItem( item, ent->s.pos.trBase, velocity );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;

	if (level.gametype != GT_CTF &&
		ent->item->giType == IT_TEAM)
	{
		int killMe = 0;

		switch (ent->item->giTag)
		{
			case PW_REDFLAG:
			case PW_BLUEFLAG:
				killMe = 1;
				break;
			default:
				break;
		}

		if (killMe)
		{
			G_FreeEntity( ent );
			return;
		}
	}

	VectorSet (ent->r.mins, -8, -8, -0);
	VectorSet (ent->r.maxs, 8, 8, 16);

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_ITEM;
	ent->touch = Touch_Item;
	// using an item causes it to respawn
	ent->use = Use_Item;

	if ( ent->spawnflags & ITMSF_SUSPEND ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor

		//if it is directly even with the floor it will return startsolid, so raise up by 0.1f
		//and temporarily subtract 0.1f from the z maxs so that going up doesn't push into the ceiling
		ent->s.origin[2] += 0.1f;
		ent->r.maxs[2] -= 0.1f;

		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap->Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID , 0, 0, 0);
		if ( tr.startsolid ) {
			trap->Print ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		//add the 0.1f back after the trace
		ent->r.maxs[2] += 0.1f;

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	trap->LinkEntity((sharedEntity_t *)ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

	// Set up team stuff
	Team_InitGame();

	if( level.gametype == GT_CTF ) {
		gitem_t	*item;

		// check for the two flags
		item = BG_FindItem( "team_CTF_redflag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			trap->Print( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map\n" );
		}
		item = BG_FindItem( "team_CTF_blueflag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			trap->Print( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map\n" );
		}
	}
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	RegisterItem( BG_FindItemForWeapon( WP_BRYAR_PISTOL ) );
	RegisterItem( BG_FindItemForWeapon( WP_STUN_BATON ) );
	RegisterItem( BG_FindItemForWeapon( WP_MELEE ) );
	RegisterItem( BG_FindItemForWeapon( WP_SABER ) );
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		trap->Error( ERR_DROP, "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

//	trap->Print( "%i items registered\n", count );
	trap->SetConfigstring(CS_ITEMS, string);
}

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) {

	char name[128];

	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return trap->Cvar_VariableIntegerValue( name );
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );
	if ( G_ItemDisabled(item) )
		return;

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50f;		// items are bouncy

	if ( item->giType == IT_POWERUP ) {
		G_SoundIndex( "sound/items/respawn1" );
		G_SpawnFloat( "noglobalsound", "0", &ent->speed);
	}
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	if ((ent->s.weapon == WP_DET_PACK && ent->s.eType == ET_GENERAL && ent->physicsObject))
	{ //detpacks only
		if (ent->touch)
		{
			ent->touch(ent, &g_entities[trace->entityNum], trace);
			return;
		}
	}

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0f;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

	if (ent->s.shouldtarget && ent->s.eType == ET_GENERAL && ent->physicsObject)
	{ // sentry guns
		if (ent->touch)
		{
			ent->touch(ent, &g_entities[trace->entityNum], trace);
		}
	}
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to ENTITYNUM_NONE, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == ENTITYNUM_NONE ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
	trap->Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
		ent->r.ownerNum, mask, 0, 0, 0 );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 0;
	}

	trap->LinkEntity( (sharedEntity_t *)ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap->PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		if (ent->item && ent->item->giType == IT_TEAM) {
			Team_FreeEntity(ent);
		} else {
			G_FreeEntity( ent );
		}
		return;
	}

	G_BounceItem( ent, &tr );
}

