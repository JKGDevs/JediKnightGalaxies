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
#include "bg_saga.h"

#include "../GLua/glua.h"

extern void NPC_Humanoid_Cloak( gentity_t *self );
extern void NPC_Humanoid_Decloak( gentity_t *self );

qboolean PM_SaberInTransition( int move );
qboolean PM_SaberInStart( int move );
qboolean PM_SaberInReturn( int move );
qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
qboolean saberCheckKnockdown_DuelLoss(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);
qboolean BG_SabersOff( playerState_t *ps );

void P_SetTwitchInfo(gclient_t	*client)
{
	client->ps.painTime = level.time;
	client->ps.painDirection ^= 1;
}

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_shield;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;

		//cap them since we can't send negative values in here across the net
		if (client->ps.damagePitch < 0)
		{
			client->ps.damagePitch = 0;
		}
		if (client->ps.damageYaw < 0)
		{
			client->ps.damageYaw = 0;
		}
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && !(player->s.eFlags & EF_DEAD) ) {

		// don't do more than two pain sounds a second
		// nmckenzie: also don't make him loud and whiny if he's only getting nicked.
		if ( level.time - client->ps.painTime < 500 || count < 10) {
			return;
		}
		P_SetTwitchInfo(client);
		player->pain_debounce_time = level.time + 700;
		
		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;

		if (client->damage_shield && !client->damage_blood)
		{
			client->ps.damageType = 1; //pure shields
		}
		else if (client->damage_shield)
		{
			client->ps.damageType = 2; //shields and health
		}
		else
		{
			client->ps.damageType = 0; //pure health
		}
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_shield = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	int			waterlevel;

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex(/*"*drown.wav"*/"sound/player/gurp1.wav"));
				} else if (rand()&1) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_SHIELD, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if ( waterlevel && (ent->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if ( ent->health > 0 && ent->pain_debounce_time <= level.time )
		{
			if ( ent->watertype & CONTENTS_LAVA )
				G_Damage( ent, NULL, NULL, NULL, NULL, 30*waterlevel, 0, MOD_LAVA );

			if ( ent->watertype & CONTENTS_SLIME )
				G_Damage( ent, NULL, NULL, NULL, NULL, 10*waterlevel, 0, MOD_SLIME );
		}
	}
}





//==============================================================
extern void G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback );
void DoImpact( gentity_t *self, gentity_t *other, qboolean damageSelf )
{
	float magnitude, my_mass;
	vec3_t	velocity;
	int cont;
	qboolean easyBreakBrush = qtrue;

	if( self->client )
	{
		VectorCopy( self->client->ps.velocity, velocity );
		if( !self->mass )
		{
			my_mass = 10;
		}
		else
		{
			my_mass = self->mass;
		}
	}
	else 
	{
		VectorCopy( self->s.pos.trDelta, velocity );
		if ( self->s.pos.trType == TR_GRAVITY )
		{
			velocity[2] -= 0.25f * g_gravity.value;
		}
		if( !self->mass )
		{
			my_mass = 1;
		}
		else if ( self->mass <= 10 )
		{
			my_mass = 10;
		}
		else
		{
			my_mass = self->mass;///10;
		}
	}

	magnitude = VectorLength( velocity ) * my_mass / 10;

	/*
	if(pointcontents(self.absmax)==CONTENT_WATER)//FIXME: or other watertypes
		magnitude/=3;							//water absorbs 2/3 velocity

	if(self.classname=="barrel"&&self.aflag)//rolling barrels are made for impacts!
		magnitude*=3;

	if(self.frozen>0&&magnitude<300&&self.flags&FL_ONGROUND&&loser==world&&self.velocity_z<-20&&self.last_onground+0.3<time)
		magnitude=300;
	*/
	if ( other->material == MAT_GLASS 
		|| other->material == MAT_GLASS_METAL 
		|| other->material == MAT_GRATE1
		|| ((other->flags&FL_BBRUSH)&&(other->spawnflags&8/*THIN*/))
		|| (other->r.svFlags&SVF_GLASS_BRUSH) )
	{
		easyBreakBrush = qtrue;
	}

	if ( !self->client || self->client->ps.lastOnGround+300<level.time || ( self->client->ps.lastOnGround+100 < level.time && easyBreakBrush ) )
	{
		vec3_t dir1, dir2;
		float force = 0, dot;

		if ( easyBreakBrush )
			magnitude *= 2;

		//damage them
		if ( magnitude >= 100 && other->s.number < ENTITYNUM_WORLD )
		{
			VectorCopy( velocity, dir1 );
			VectorNormalize( dir1 );
			if( VectorCompare( other->r.currentOrigin, vec3_origin ) )
			{//a brush with no origin
				VectorCopy ( dir1, dir2 );
			}
			else
			{
				VectorSubtract( other->r.currentOrigin, self->r.currentOrigin, dir2 );
				VectorNormalize( dir2 );
			}

			dot = DotProduct( dir1, dir2 );

			if ( dot >= 0.2 )
			{
				force = dot;
			}
			else
			{
				force = 0;
			}

			force *= (magnitude/50);

			cont = trap->PointContents( other->r.absmax, other->s.number );
			if( (cont&CONTENTS_WATER) )//|| (self.classname=="barrel"&&self.aflag))//FIXME: or other watertypes
			{
				force /= 3;							//water absorbs 2/3 velocity
			}

			/*
			if(self.frozen>0&&force>10)
				force=10;
			*/

			if( ( force >= 1 && other->s.number != 0 ) || force >= 10)
			{
	/*			
				dprint("Damage other (");
				dprint(loser.classname);
				dprint("): ");
				dprint(ftos(force));
				dprint("\n");
	*/
				if ( other->r.svFlags & SVF_GLASS_BRUSH )
				{
					other->splashRadius = (float)(self->r.maxs[0] - self->r.mins[0])/4.0f;
				}
				if ( other->takedamage )
				{
					G_Damage( other, self, self, velocity, self->r.currentOrigin, force, DAMAGE_NO_SHIELD, MOD_CRUSH);//FIXME: MOD_IMPACT
				}
				else
				{
					G_ApplyKnockback( other, dir2, force );
				}
			}
		}

		if ( damageSelf && self->takedamage )
		{
			//Now damage me
			//FIXME: more lenient falling damage, especially for when driving a vehicle
			if ( self->client && self->client->ps.fd.forceJumpZStart )
			{//we were force-jumping
				if ( self->r.currentOrigin[2] >= self->client->ps.fd.forceJumpZStart )
				{//we landed at same height or higher than we landed
					magnitude = 0;
				}
				else
				{//FIXME: take off some of it, at least?
					magnitude = (self->client->ps.fd.forceJumpZStart-self->r.currentOrigin[2])/3;
				}
			}
			//if(self.classname!="monster_mezzoman"&&self.netname!="spider")//Cats always land on their feet
				if( ( magnitude >= 100 + self->health && self->s.number != 0 && self->s.weapon != WP_SABER ) || ( magnitude >= 700 ) )//&& self.safe_time < level.time ))//health here is used to simulate structural integrity
				{
					if ( (self->s.weapon == WP_SABER || (self->s.number >= 0 && self->s.number < MAX_CLIENTS)) && self->client && self->client->ps.groundEntityNum < ENTITYNUM_NONE && magnitude < 1000 )
					{//players and jedi take less impact damage
						//allow for some lenience on high falls
						magnitude /= 2;
						/*
						if ( self.absorb_time >= time )//crouching on impact absorbs 1/2 the damage
						{
							magnitude/=2;
						}
						*/
					}
					magnitude /= 40;
					magnitude = magnitude - force/2;//If damage other, subtract half of that damage off of own injury
					if ( magnitude >= 1 )
					{
		//FIXME: Put in a thingtype impact sound function
		/*					
						dprint("Damage self (");
						dprint(self.classname);
						dprint("): ");
						dprint(ftos(magnitude));
						dprint("\n");
		*/
						/*
						if ( self.classname=="player_sheep "&& self.flags&FL_ONGROUND && self.velocity_z > -50 )
							return;
						*/
						if (self->s.eType == ET_NPC)
							G_Damage (self, NULL, NULL, NULL, NULL, magnitude*5, DAMAGE_NO_SHIELD, MOD_FALLING);
						else
							G_Damage( self, NULL, NULL, NULL, self->r.currentOrigin, magnitude/2, DAMAGE_NO_SHIELD, MOD_FALLING );//FIXME: MOD_IMPACT
					}
				}
		}

		//FIXME: slow my velocity some?

		// NOTENOTE We don't use lastimpact as of yet
//		self->lastImpact = level.time;

		/*
		if(self.flags&FL_ONGROUND)
			self.last_onground=time;
		*/
	}
}

void Client_CheckImpactBBrush( gentity_t *self, gentity_t *other )
{
	if ( !other || !other->inuse )
	{
		return;
	}
	if (!self || !self->inuse || !self->client ||
		self->client->tempSpectate >= level.time ||
		self->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //hmm.. let's not let spectators ram into breakables.
		return;
	}

	/*
	if (BG_InSpecialJump(self->client->ps.legsAnim))
	{ //don't do this either, qa says it creates "balance issues"
		return;
	}
	*/

	if ( other->material == MAT_GLASS 
		|| other->material == MAT_GLASS_METAL 
		|| other->material == MAT_GRATE1
		|| ((other->flags&FL_BBRUSH)&&(other->spawnflags&8/*THIN*/))
		|| ((other->flags&FL_BBRUSH)&&(other->health<=10))
		|| (other->r.svFlags&SVF_GLASS_BRUSH) )
	{//clients only do impact damage against easy-break breakables
		DoImpact( self, other, qfalse );
	}
}


/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if(!ent->client)
	{
		return;	// fixin' base bugs --eez
	}
	if (ent->client && ent->client->isHacking)
	{ //loop hacking sound
		ent->client->ps.loopSound = level.snd_hack;
		ent->s.loopIsSoundset = qfalse;
	}
	else if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		ent->client->ps.loopSound = level.snd_fry;
		ent->s.loopIsSoundset = qfalse;
	} else {
		ent->client->ps.loopSound = 0;
		ent->s.loopIsSoundset = qfalse;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pmove ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pmove->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pmove->touchents[j] == pmove->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pmove->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->r.absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if (hit == ent)
		{
			continue;
		}
		/*if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}*/

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			// [USE_ITEMS] Disable automatic item pickup for players
			if ( ent->s.eType == ET_NPC ) {
				if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) )
					continue;
			}
			else if ( ent->s.number < MAX_CLIENTS )
			{
				if( !hit->item || hit->item->giType != IT_TEAM )
					continue;

				if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) )
					continue;
			}
			// [/USE_ITEMS]
		} else {
			if ( !trap->EntityContact( mins, maxs, (sharedEntity_t *)hit, 0 ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if ( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount ) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}


/*
============
G_MoverTouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_MoverTouchPushTriggers( gentity_t *ent, vec3_t oldOrg ) 
{
	int			i, num;
	float		step, stepSize, dist;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs, dir, size, checkSpot;
	const vec3_t	range = { 40, 40, 52 };

	// non-moving movers don't hit triggers!
	if ( !VectorLengthSquared( ent->s.pos.trDelta ) ) 
	{
		return;
	}

	VectorSubtract( ent->r.mins, ent->r.maxs, size );
	stepSize = VectorLength( size );
	if ( stepSize < 1 )
	{
		stepSize = 1;
	}

	VectorSubtract( ent->r.currentOrigin, oldOrg, dir );
	dist = VectorNormalize( dir );
	for ( step = 0; step <= dist; step += stepSize )
	{
		VectorMA( ent->r.currentOrigin, step, dir, checkSpot );
		VectorSubtract( checkSpot, range, mins );
		VectorAdd( checkSpot, range, maxs );

		num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->r.absmin, because that has a one unit pad
		VectorAdd( checkSpot, ent->r.mins, mins );
		VectorAdd( checkSpot, ent->r.maxs, maxs );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = &g_entities[touch[i]];

			if ( hit->s.eType != ET_PUSH_TRIGGER )
			{
				continue;
			}

			if ( hit->touch == NULL ) 
			{
				continue;
			}

			if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
			{
				continue;
			}


			if ( !trap->EntityContact( mins, maxs, (sharedEntity_t *)hit, 0 ) ) 
			{
				continue;
			}

			memset( &trace, 0, sizeof(trace) );

			if ( hit->touch != NULL ) 
			{
				hit->touch(hit, ent, &trace);
			}
		}
	}
}


void DeathcamClamp( gentity_t *ent ) {
	// Check if we went out of range and if so, clamp it to the range
	vec3_t delta;
	vec3_t vector;
	vec3_t fwd;
	vec3_t up;
	vec3_t right;
	int dist;

	VectorSubtract(ent->client->ps.origin, ent->client->deathcamCenter, delta);
	dist = VectorLength(delta);
	if (dist > ent->client->deathcamRadius) {
		// We went outta range, clamp it
		vectoangles(delta, vector);
		AngleVectors(vector, fwd, right, up);
		VectorMA(ent->client->deathcamCenter, ent->client->deathcamRadius, fwd, ent->client->ps.origin);
		VectorCopy( ent->client->ps.origin, ent->s.origin );
	}
}

static void SV_PMTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask ) {
	trap->Trace( results, start, mins, maxs, end, passEntityNum, contentMask, qfalse, 0, 10 );
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pmove;
	gclient_t	*client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400;	// faster than normal
		client->ps.basespeed = 400;

		//hmm, shouldn't have an anim if you're a spectator, make sure
		//it gets cleared.
		client->ps.legsAnim = 0;
		client->ps.legsTimer = 0;
		client->ps.torsoAnim = 0;
		client->ps.torsoTimer = 0;

		// set up for pmove
		memset (&pmove, 0, sizeof(pmove));
		pmove.ps = &client->ps;
		pmove.cmd = *ucmd;
		pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pmove.trace = SV_PMTrace;
		pmove.pointcontents = trap->PointContents;

		pmove.noSpecMove = g_noSpecMove.integer;

		pmove.animations = NULL;
		pmove.nonHumanoid = qfalse;

		//Set up bg entity data
		pmove.baseEnt = (bgEntity_t *)g_entities;
		pmove.entSize = sizeof(gentity_t);

		// perform a pmove
		Pmove (&pmove);
		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		if (client->deathcamTime) {
			DeathcamClamp(ent);
		} else if (ent->client->tempSpectate < level.time) {
			G_TouchTriggers( ent );
		}
		trap->UnlinkEntity( (sharedEntity_t *)ent );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	if (client->tempSpectate < level.time && !client->deathcamTime)
	{
		// attack button cycles through spectators
		if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {
			Cmd_FollowCycle_f( ent, 1 );
		}

		//block/ironsights button cycles backwards through spectators
		if ((client->buttons & BUTTON_IRONSIGHTS) && !(client->oldbuttons & BUTTON_IRONSIGHTS)) {
			Cmd_FollowCycle_f(ent, -1);
		}

		if (client->sess.spectatorState == SPECTATOR_FOLLOW && (ucmd->upmove > 0 || ucmd->generic_cmd == GENCMD_SABERATTACKCYCLE) )
		{ //jump or sabercycle/firemode now removes you from follow mode
			StopFollowing(ent);
		}
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & BUTTON_ATTACK) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap->DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap->SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t	*client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) 
	{
		client->timeResidual -= 1000;

		// count down health when over max
		if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
			ent->health--;
		}

		// count down armor when over max
		if ( client->ps.stats[STAT_SHIELD] > client->ps.stats[STAT_MAX_SHIELD] ) {
			client->ps.stats[STAT_SHIELD]--;
		}
	}

	// Replenish accuracy
	if( client->accuracyDebounce <= level.time && client->ps.stats[STAT_ACCURACY] > 0 )
	{
		client->ps.stats[STAT_ACCURACY] -= 1;
		if(client->ps.stats[STAT_ACCURACY] <= 0)
		{
			client->ps.stats[STAT_ACCURACY] = 0;
		}
		client->accuracyDebounce = level.time + 
			GetWeaponData( client->ps.weapon, client->ps.weaponVariation )->firemodes[ent->s.firingMode].weaponAccuracy.msToDrainAccuracy;
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;
	if ( client->buttons & BUTTON_ATTACK & ( client->oldbuttons ^ client->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = qtrue;
	}
}

extern void NPC_SetAnim(gentity_t	*ent,int setAnimParts,int anim,int setAnimFlags);

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
qboolean BG_InKnockDownOnly( int anim );

void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int		i;//, j;
	int		event;
	gclient_t *client;
	int		damage;
	vec3_t	dir;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL:
		case EV_ROLL:
			{
				int delta = client->ps.eventParms[ i & (MAX_PS_EVENTS-1) ];
				qboolean knockDownage = qfalse;

				if (ent->client && ent->client->ps.fallingToDeath)
				{
					break;
				}

				if ( ent->s.eType != ET_PLAYER )
				{
					break;		// not in the player model
				}
				
				if ( dmflags.integer & DF_NO_FALLING )
				{
					break;
				}

				if (BG_InKnockDownOnly(ent->client->ps.legsAnim))
				{
					if (delta <= 14)
					{
						break;
					}
					knockDownage = qtrue;
				}
				else
				{
					if (delta <= 44)
					{
						break;
					}
				}

				if (knockDownage)
				{
					damage = delta*1; //you suffer for falling unprepared. A lot. Makes throws and things useful, and more realistic I suppose.
				}
				else
				{
					damage = delta*0.16; //good enough for now, I guess
				}

				VectorSet (dir, 0, 0, 1);
				ent->pain_debounce_time = level.time + 200;	// no normal pain sound

				if (ent->s.eType == ET_NPC)
					G_Damage (ent, NULL, NULL, NULL, NULL, damage*5, DAMAGE_NO_SHIELD, MOD_FALLING);
				else
					G_Damage (ent, NULL, NULL, NULL, NULL, damage, DAMAGE_NO_SHIELD, MOD_FALLING);

				if (ent->health < 1)
				{
					G_Sound(ent, CHAN_AUTO, G_SoundIndex( "sound/player/fallsplat.wav" ));
				}
			}
			break;
		case EV_FIRE_WEAPON:
			FireWeapon( ent, ent->s.firingMode );
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			break;

		case EV_ALT_FIRE:
			FireWeapon( ent, ent->s.firingMode );
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			break;

		case EV_SABER_ATTACK:
			ent->client->dangerTime = level.time;
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
			break;

		case EV_CHANGE_WEAPON:
		{
			ent->client->pers.partyUpdate = qtrue;
			break;
		}

		case EV_GRENADE_COOK:
		{
			if ( ent->grenadeCookTime == 0 )
			{
				ent->grenadeCookTime = level.time + GetWeaponData( ent->client->ps.weapon, ent->client->ps.weaponVariation )->weaponReloadTime;
				ent->grenadeWeapon = ent->client->ps.weapon;
				ent->grenadeVariation = ent->client->ps.weaponVariation;
			}

			break;
		}

		default:
			break;
		}
	}

}


//[KnockdownSys][SPPortComplete]
void G_ThrownDeathAnimForDeathAnim( gentity_t *hitEnt, vec3_t impactPoint )
{//racc - sets an alternate "being thrown" death animation based on current death animation.
	int anim = -1;
	if ( !hitEnt || !hitEnt->client )
	{
		return;
	}
	switch ( hitEnt->client->ps.legsAnim )
	{
	case BOTH_DEATH9://fall to knees, fall over
	case BOTH_DEATH10://fall to knees, fall over
	case BOTH_DEATH11://fall to knees, fall over
	case BOTH_DEATH13://stumble back, fall over
	case BOTH_DEATH17://jerky fall to knees, fall over
	case BOTH_DEATH18://grab gut, fall to knees, fall over
	case BOTH_DEATH19://grab gut, fall to knees, fall over
	case BOTH_DEATH20://grab shoulder, fall forward
	case BOTH_DEATH21://grab shoulder, fall forward
	case BOTH_DEATH3://knee collapse, twist & fall forward
	case BOTH_DEATH7://knee collapse, twist & fall forward
		{
			float dot;
			vec3_t dir2Impact, fwdAngles, facing;
			VectorSubtract( impactPoint, hitEnt->r.currentOrigin, dir2Impact );
			dir2Impact[2] = 0;
			VectorNormalize( dir2Impact );
			VectorSet( fwdAngles, 0, hitEnt->client->ps.viewangles[YAW], 0 );
			AngleVectors( fwdAngles, facing, NULL, NULL );
			dot = DotProduct( facing, dir2Impact );//-1 = hit in front, 0 = hit on side, 1 = hit in back
			if ( dot > 0.5f )
			{//kicked in chest, fly backward
				switch ( Q_irand( 0, 4 ) )
				{//FIXME: don't start at beginning of anim?
				case 0:
					anim = BOTH_DEATH1;//thrown backwards
					break;
				case 1:
					anim = BOTH_DEATH2;//fall backwards
					break;
				case 2:
					anim = BOTH_DEATH15;//roll over backwards
					break;
				case 3:
					anim = BOTH_DEATH22;//fast fall back
					break;
				case 4:
					anim = BOTH_DEATH23;//fast fall back
					break;
				}
			}
			else if ( dot < -0.5f )
			{//kicked in back, fly forward
				switch ( Q_irand( 0, 5 ) )
				{//FIXME: don't start at beginning of anim?
				case 0:
					anim = BOTH_DEATH14;
					break;
				case 1:
					anim = BOTH_DEATH24;
					break;
				case 2:
					anim = BOTH_DEATH25;
					break;
				case 3:
					anim = BOTH_DEATH4;//thrown forwards
					break;
				case 4:
					anim = BOTH_DEATH5;//thrown forwards
					break;
				case 5:
					anim = BOTH_DEATH16;//thrown forwards
					break;
				}
			}
			else
			{//hit on side, spin
				switch ( Q_irand( 0, 2 ) )
				{//FIXME: don't start at beginning of anim?
				case 0:
					anim = BOTH_DEATH12;
					break;
				case 1:
					anim = BOTH_DEATH14;
					break;
				case 2:
					anim = BOTH_DEATH15;
					break;
				case 3:
					anim = BOTH_DEATH6;
					break;
				case 4:
					anim = BOTH_DEATH8;
					break;
				}
			}
		}
		break;
	}
	if ( anim != -1 )
	{
		NPC_SetAnim( hitEnt, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
	}
}
//[/SPPortComplete][/KnockdownSys]

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

static const float maxForceSightDistance = Square( 1500.0f ) * 1500.0f; // x^2, optimisation
static const float maxForceSightFOV = 100.0f;

void G_UpdateClientBroadcasts( gentity_t *self ) {
	int i;
	gentity_t *other;

	// we are always sent to ourselves
	// we are always sent to other clients if we are in their PVS
	// if we are not in their PVS, we must set the broadcastClients bit field
	// if we do not wish to be sent to any particular entity, we must set the broadcastClients bit field and the
	//	SVF_BROADCASTCLIENTS bit flag
	self->r.broadcastClients[0] = 0u;
	self->r.broadcastClients[1] = 0u;

	for ( i = 0, other = g_entities; i < MAX_CLIENTS; i++, other++ ) {
		qboolean send = qfalse;
		float dist;
		vec3_t angles;
	
		if ( !other->inuse || other->client->pers.connected != CON_CONNECTED ) {
			// no need to compute visibility for non-connected clients
			continue;
		}

		if ( other == self ) {
			// we are always sent to ourselves anyway, this is purely an optimisation
			continue;
		}

		VectorSubtract( self->client->ps.origin, other->client->ps.origin, angles );
		dist = VectorLengthSquared( angles );
		vectoangles( angles, angles );

		// broadcast this client to everyone using force sight if we are in distance/field of view
		if ((other->client->ps.fd.forcePowersActive & (1 << FP_SEE))) {
			if (dist < maxForceSightDistance
				&& InFieldOfVision(other->client->ps.viewangles, maxForceSightFOV, angles))
			{
				send = qtrue;
			}
		}

		if ( send ) {
			Q_AddToBitflags( self->r.broadcastClients, i, 32 );
		}
	}
	trap->LinkEntity( (sharedEntity_t *)self );
}

void G_AddPushVecToUcmd( gentity_t *self, usercmd_t *ucmd )
{
	vec3_t	forward, right, moveDir;
	float	pushSpeed, fMove, rMove;

	if ( !self->client )
	{
		return;
	}
	pushSpeed = VectorLengthSquared(self->client->pushVec);
	if(!pushSpeed)
	{//not being pushed
		return;
	}

	AngleVectors(self->client->ps.viewangles, forward, right, NULL);
	VectorScale(forward, ucmd->forwardmove/127.0f * self->client->ps.speed, moveDir);
	VectorMA(moveDir, ucmd->rightmove/127.0f * self->client->ps.speed, right, moveDir);
	//moveDir is now our intended move velocity

	VectorAdd(moveDir, self->client->pushVec, moveDir);
	self->client->ps.speed = VectorNormalize(moveDir);
	//moveDir is now our intended move velocity plus our push Vector

	fMove = 127.0 * DotProduct(forward, moveDir);
	rMove = 127.0 * DotProduct(right, moveDir);
	ucmd->forwardmove = floor(fMove);//If in the same dir , will be positive
	ucmd->rightmove = floor(rMove);//If in the same dir , will be positive

	if ( self->client->pushVecTime < level.time )
	{
		VectorClear( self->client->pushVec );
	}
}

qboolean G_StandingAnim( int anim )
{//NOTE: does not check idles or special (cinematic) stands
	switch ( anim )
	{
	case BOTH_STAND1:
	case BOTH_STAND2:
	case BOTH_STAND3:
	case BOTH_STAND4:
		return qtrue;
		break;
	}
	return qfalse;
}

qboolean G_ActionButtonPressed(int buttons)
{
	if (buttons & BUTTON_ATTACK)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_GESTURE)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_USE)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_FORCEGRIP)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_FORCEPOWER)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_FORCE_LIGHTNING)
	{
		return qtrue;
	}
	else if (buttons & BUTTON_FORCE_DRAIN)
	{
		return qtrue;
	}

	return qfalse;
}

void G_CheckClientIdle( gentity_t *ent, usercmd_t *ucmd ) 
{
	vec3_t viewChange;
	qboolean actionPressed;
	int buttons;

	if ( !ent || !ent->client || ent->health <= 0 || ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->sess.sessionTeam == TEAM_SPECTATOR || (ent->client->ps.pm_flags & PMF_FOLLOW))
	{
		return;
	}

	buttons = ucmd->buttons;

	if (ent->r.svFlags & SVF_BOT)
	{ //they press use all the time..
		buttons &= ~BUTTON_USE;
	}
	actionPressed = G_ActionButtonPressed(buttons);

	VectorSubtract(ent->client->ps.viewangles, ent->client->idleViewAngles, viewChange);
	if ( !VectorCompare( vec3_origin, ent->client->ps.velocity ) 
		|| actionPressed || ucmd->forwardmove || ucmd->rightmove || ucmd->upmove 
		|| !G_StandingAnim( ent->client->ps.legsAnim ) 
		|| (ent->health+ent->client->ps.stats[STAT_SHIELD]) != ent->client->idleHealth
		|| VectorLength(viewChange) > 10
		|| ent->client->ps.legsTimer > 0
		|| ent->client->ps.torsoTimer > 0
		|| ent->client->ps.weaponTime > 0
		|| ent->client->ps.weaponstate == WEAPON_CHARGING
		|| ent->client->ps.zoomMode
		|| (ent->client->ps.weaponstate != WEAPON_READY && ent->client->ps.weapon != WP_SABER)
		|| ent->client->ps.forceHandExtend != HANDEXTEND_NONE
		|| ent->client->ps.saberBlocked != BLOCKED_NONE
		|| ent->client->ps.saberBlocking >= level.time
		|| ent->client->ps.weapon == WP_MELEE
		|| (ent->client->ps.weaponId != ent->client->pers.cmd.weapon && ent->s.eType != ET_NPC))
	{//FIXME: also check for turning?
		qboolean brokeOut = qfalse;

		if ( !VectorCompare( vec3_origin, ent->client->ps.velocity ) 
			|| actionPressed || ucmd->forwardmove || ucmd->rightmove || ucmd->upmove 
			|| (ent->health+ent->client->ps.stats[STAT_SHIELD]) != ent->client->idleHealth
			|| ent->client->ps.zoomMode
			|| (ent->client->ps.weaponstate != WEAPON_READY && ent->client->ps.weapon != WP_SABER)
			|| (ent->client->ps.weaponTime > 0 && ent->client->ps.weapon == WP_SABER)
			|| ent->client->ps.weaponstate == WEAPON_CHARGING
			|| ent->client->ps.forceHandExtend != HANDEXTEND_NONE
			|| ent->client->ps.saberBlocked != BLOCKED_NONE
			|| ent->client->ps.saberBlocking >= level.time
			|| ent->client->ps.weapon == WP_MELEE
			|| (ent->client->ps.weaponId != ent->client->pers.cmd.weapon && ent->s.eType != ET_NPC))
		{
			//if in an idle, break out
			switch ( ent->client->ps.legsAnim )
			{
			case BOTH_STAND1IDLE1:
			case BOTH_STAND2IDLE1:
			case BOTH_STAND2IDLE2:
			case BOTH_STAND3IDLE1:
			case BOTH_STAND5IDLE1:
				ent->client->ps.legsTimer = 0;
				brokeOut = qtrue;
				break;
			}
			switch ( ent->client->ps.torsoAnim )
			{
			case BOTH_STAND1IDLE1:
			case BOTH_STAND2IDLE1:
			case BOTH_STAND2IDLE2:
			case BOTH_STAND3IDLE1:
			case BOTH_STAND5IDLE1:
				ent->client->ps.torsoTimer = 0;
				ent->client->ps.weaponTime = 0;
				ent->client->ps.saberMove = LS_READY;
				brokeOut = qtrue;
				break;
			}
		}
		//
		ent->client->idleHealth = (ent->health+ent->client->ps.stats[STAT_SHIELD]);
		VectorCopy(ent->client->ps.viewangles, ent->client->idleViewAngles);
		if ( ent->client->idleTime < level.time )
		{
			ent->client->idleTime = level.time;
		}

		if (brokeOut && ent->client->ps.weaponstate == WEAPON_CHARGING)
		{
			ent->client->ps.torsoAnim = TORSO_RAISEWEAP1;
		}
	}
	else if ( level.time - ent->client->idleTime > 10000 && !(ent->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) ) )
	{//been idle for 10 seconds --eez
		int	idleAnim = -1;
		switch ( ent->client->ps.legsAnim )
		{
		case BOTH_STAND1:
			idleAnim = BOTH_STAND1IDLE1;
			break;
		case BOTH_STAND2:
		case BOTH_SABERFAST_STANCE:			// fixed this to properly work --eez
		case BOTH_SABERSLOW_STANCE:
			idleAnim = /*BOTH_STAND2IDLE1;*/Q_irand(BOTH_STAND2IDLE1,BOTH_STAND2IDLE2);		// ..why was this ever commented out --eez
			break;
		case BOTH_STAND3:
			idleAnim = BOTH_STAND3IDLE1;
			break;
		case BOTH_STAND5:
			idleAnim = BOTH_STAND5IDLE1;
			break;
		}

		if ( idleAnim != -1 && /*PM_HasAnimation( ent, idleAnim )*/idleAnim > 0 && idleAnim < MAX_ANIMATIONS )
		{
			G_SetAnim(ent, ucmd, SETANIM_BOTH, idleAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);

			//don't idle again after this anim for a while
			//ent->client->idleTime = level.time + PM_AnimLength( ent->client->clientInfo.animFileIndex, (animNumber_t)idleAnim ) + Q_irand( 0, 2000 );
			ent->client->idleTime = level.time + ent->client->ps.legsTimer + Q_irand( 0, 5000 );		// changed to 5000 because it seemed like they did this too often --eez
		}
	}
}

void NPC_Accelerate( gentity_t *ent, qboolean fullWalkAcc, qboolean fullRunAcc )
{
	if ( !ent->client || !ent->NPC )
	{
		return;
	}

	if ( !ent->NPC->stats.acceleration )
	{//No acceleration means just start and stop
		ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
	}
	//FIXME:  in cinematics always accel/decel?
	else if ( ent->NPC->desiredSpeed <= ent->NPC->stats.walkSpeed )
	{//Only accelerate if at walkSpeeds
		if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed + ent->NPC->stats.acceleration )
		{
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed += ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed )
		{
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
		else if ( fullWalkAcc && ent->NPC->desiredSpeed < ent->NPC->currentSpeed - ent->NPC->stats.acceleration )
		{//decelerate even when walking
			ent->NPC->currentSpeed -= ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
		{//stop on a dime
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
	}
	else//  if ( ent->NPC->desiredSpeed > ent->NPC->stats.walkSpeed )
	{//Only decelerate if at runSpeeds
		if ( fullRunAcc && ent->NPC->desiredSpeed > ent->NPC->currentSpeed + ent->NPC->stats.acceleration )
		{//Accelerate to runspeed
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed += ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed )
		{//accelerate instantly
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
		else if ( fullRunAcc && ent->NPC->desiredSpeed < ent->NPC->currentSpeed - ent->NPC->stats.acceleration )
		{
			ent->NPC->currentSpeed -= ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
		{
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
	}
}

/*
-------------------------
NPC_GetWalkSpeed
-------------------------
*/

static int NPC_GetWalkSpeed( gentity_t *ent )
{
	int	walkSpeed = 0;

	if ( ( ent->client == NULL ) || ( ent->NPC == NULL ) )
		return 0;

	switch ( ent->client->playerTeam )
	{
	case NPCTEAM_PLAYER:	//To shutup compiler, will add entries later (this is stub code)
	default:
		walkSpeed = ent->NPC->stats.walkSpeed*1.1;
		break;
	}

	return walkSpeed;
}

/*
-------------------------
NPC_GetRunSpeed
-------------------------
*/
static int NPC_GetRunSpeed( gentity_t *ent )
{
	int	runSpeed = 0;

	if ( ( ent->client == NULL ) || ( ent->NPC == NULL ) )
		return 0;
/*
	switch ( ent->client->playerTeam )
	{
	case TEAM_BORG:
		runSpeed = ent->NPC->stats.runSpeed;
		runSpeed += BORG_RUN_INCR * (g_npcspskill->integer%3);
		break;

	case TEAM_8472:
		runSpeed = ent->NPC->stats.runSpeed;
		runSpeed += SPECIES_RUN_INCR * (g_npcspskill->integer%3);
		break;

	case TEAM_STASIS:
		runSpeed = ent->NPC->stats.runSpeed;
		runSpeed += STASIS_RUN_INCR * (g_npcspskill->integer%3);
		break;

	case TEAM_BOTS:
		runSpeed = ent->NPC->stats.runSpeed;
		break;

	default:
		runSpeed = ent->NPC->stats.runSpeed;
		break;
	}
*/
	// team no longer indicates species/race.  Use NPC_class to adjust speed for specific npc types
	switch( ent->client->NPC_class)
	{
	case CLASS_PROBE:	// droid cases here to shut-up compiler
	case CLASS_GONK:
	case CLASS_R2D2:
	case CLASS_R5D2:
	case CLASS_MARK1:
	case CLASS_MARK2:
	case CLASS_PROTOCOL:
	case CLASS_ATST: // hmm, not really your average droid
	case CLASS_MOUSE:
	case CLASS_SEEKER:
	case CLASS_REMOTE:
		runSpeed = ent->NPC->stats.runSpeed;
		break;

	default:
		runSpeed = ent->NPC->stats.runSpeed*1.3f; //rww - seems to slow in MP for some reason.
		break;
	}

	return runSpeed;
}

//Seems like a slightly less than ideal method for this, could it be done on the client?
extern qboolean FlyingCreature( gentity_t *ent );
void G_CheckMovingLoopingSounds( gentity_t *ent, usercmd_t *ucmd )
{
	if ( ent->client )
	{
		if ( (ent->NPC&&!VectorCompare( vec3_origin, ent->client->ps.moveDir ))//moving using moveDir
			|| ucmd->forwardmove || ucmd->rightmove//moving using ucmds
			|| (ucmd->upmove&&FlyingCreature( ent ))//flier using ucmds to move
			|| (FlyingCreature( ent )&&!VectorCompare( vec3_origin, ent->client->ps.velocity )&&ent->health>0))//flier using velocity to move
		{
			switch( ent->client->NPC_class )
			{
			case CLASS_R2D2:
				ent->s.loopSound = G_SoundIndex( "sound/chars/r2d2/misc/r2_move_lp.wav" );
				break;
			case CLASS_R5D2:
				ent->s.loopSound = G_SoundIndex( "sound/chars/r2d2/misc/r2_move_lp2.wav" );
				break;
			case CLASS_MARK2:
				ent->s.loopSound = G_SoundIndex( "sound/chars/mark2/misc/mark2_move_lp" );
				break;
			case CLASS_MOUSE:
				ent->s.loopSound = G_SoundIndex( "sound/chars/mouse/misc/mouse_lp" );
				break;
			case CLASS_PROBE:
				ent->s.loopSound = G_SoundIndex( "sound/chars/probe/misc/probedroidloop" );
			default:
				break;
			}
		}
		else
		{//not moving under your own control, stop loopSound
			if ( ent->client->NPC_class == CLASS_R2D2 || ent->client->NPC_class == CLASS_R5D2 
					|| ent->client->NPC_class == CLASS_MARK2 || ent->client->NPC_class == CLASS_MOUSE 
					|| ent->client->NPC_class == CLASS_PROBE )
			{
				ent->s.loopSound = 0;
			}
		}
	}
}

void G_HeldByMonster( gentity_t *ent, usercmd_t *ucmd )
{
	if ( ent 
		&& ent->client
		&& ent->client->ps.hasLookTarget )//NOTE: lookTarget is an entity number, so this presumes that client 0 is NOT a Rancor...
	{
		gentity_t *monster = &g_entities[ent->client->ps.lookTarget];
		if ( monster && monster->client )
		{
			//take the monster's waypoint as your own
			ent->waypoint = monster->waypoint;
			if ( monster->s.NPC_class == CLASS_RANCOR )
			{//only possibility right now, may add Wampa and Sand Creature later
				BG_AttachToRancor( monster->ghoul2, //ghoul2 info
					monster->r.currentAngles[YAW],
					monster->r.currentOrigin,
					level.time,
					NULL,
					monster->modelScale,
					(monster->client->ps.eFlags2&EF2_GENERIC_NPC_FLAG),
					ent->client->ps.origin,
					ent->client->ps.viewangles,
					NULL );
			}
			VectorClear( ent->client->ps.velocity );
			G_SetOrigin( ent, ent->client->ps.origin );
			SetClientViewAngle( ent, ent->client->ps.viewangles );
			G_SetAngles( ent, ent->client->ps.viewangles );
			trap->LinkEntity( (sharedEntity_t *)ent );//redundant?
		}
	}
	// don't allow movement, weapon switching, and most kinds of button presses
	ucmd->forwardmove = 0;
	ucmd->rightmove = 0;
	ucmd->upmove = 0;
}

typedef enum tauntTypes_e
{
	TAUNT_TAUNT = 0,
	TAUNT_BOW,
	TAUNT_MEDITATE,
	TAUNT_FLOURISH,
	TAUNT_GLOAT
} tauntTypes_t;

void G_SetTauntAnim( gentity_t *ent, int taunt )
{
	if (ent->client->pers.cmd.upmove ||
		ent->client->pers.cmd.forwardmove ||
		ent->client->pers.cmd.rightmove)
	{ //hack, don't do while moving
		return;
	}
	if ( taunt != TAUNT_TAUNT )
	{//normal taunt always allowed
		if ( level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL )
		{//no taunts unless in Duel
			return;
		}
	}

	if ( ent->client->ps.torsoTimer < 1 
		&& ent->client->ps.forceHandExtend == HANDEXTEND_NONE 
		&& ent->client->ps.legsTimer < 1 
		&& ent->client->ps.weaponTime < 1 
		&& ent->client->ps.saberLockTime < level.time )
	{
		int anim = -1;
		switch ( taunt )
		{
		case TAUNT_TAUNT:
			if ( ent->client->ps.weapon != WP_SABER )
			{
				anim = BOTH_ENGAGETAUNT;
			}
			else if ( ent->client->saber[0].tauntAnim != -1 )
			{
				anim = ent->client->saber[0].tauntAnim;
			}
			else if ( ent->client->saber[1].model[0] &&
					ent->client->saber[1].tauntAnim != -1 )
			{
				anim = ent->client->saber[1].tauntAnim;
			}
			else
			{
				switch ( ent->client->ps.fd.saberAnimLevel )
				{
				case SS_MAKASHI:
				case SS_JUYO:
					if ( ent->client->ps.saberHolstered == 1 &&
							ent->client->saber[1].model[0] )
					{//turn off second saber
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOff );
					}
					else if ( ent->client->ps.saberHolstered == 0 )
					{//turn off first
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOff );
					}
					ent->client->ps.saberHolstered = 2;
					anim = BOTH_GESTURE1;
					break;
				case SS_SHII_CHO:
				case SS_SORESU:
				case SS_ATARU:
					anim = BOTH_ENGAGETAUNT;
					break;
				case SS_DUAL:
					if ( ent->client->ps.saberHolstered == 1 &&
							ent->client->saber[1].model[0] )
					{//turn on second saber
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOn );
					}
					else if ( ent->client->ps.saberHolstered == 2 )
					{//turn on first
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
					}
					ent->client->ps.saberHolstered = 0;
					anim = BOTH_DUAL_TAUNT;
					break;
				case SS_STAFF:
					if ( ent->client->ps.saberHolstered > 0 )
					{//turn on all blades
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
					}
					ent->client->ps.saberHolstered = 0;
					anim = BOTH_STAFF_TAUNT;
					break;
				}
			}
			break;
		case TAUNT_BOW:
			if ( ent->client->saber[0].bowAnim != -1 )
			{
				anim = ent->client->saber[0].bowAnim;
			}
			else if ( ent->client->saber[1].model[0] &&
					ent->client->saber[1].bowAnim != -1 )
			{
				anim = ent->client->saber[1].bowAnim;
			}
			else
			{
				anim = BOTH_BOW;
			}
			if ( ent->client->ps.saberHolstered == 1 &&
					ent->client->saber[1].model[0] )
			{//turn off second saber
				G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOff );
			}
			else if ( ent->client->ps.saberHolstered == 0 )
			{//turn off first
				G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOff );
			}
			ent->client->ps.saberHolstered = 2;
			break;
		case TAUNT_MEDITATE:
			if ( ent->client->saber[0].meditateAnim != -1 )
			{
				anim = ent->client->saber[0].meditateAnim;
			}
			else if ( ent->client->saber[1].model[0] &&
					ent->client->saber[1].meditateAnim != -1 )
			{
				anim = ent->client->saber[1].meditateAnim;
			}
			else
			{
				anim = BOTH_MEDITATE;
			}
			// Jedi Knight Galaxies, Fix sound bug
			if (ent->client->ps.weapon == WP_SABER) {
				if ( ent->client->ps.saberHolstered == 1 &&
						ent->client->saber[1].model[0] )
				{//turn off second saber
					G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOff );
				}
				else if ( ent->client->ps.saberHolstered == 0 )
				{//turn off first
					G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOff );
				}
				ent->client->ps.saberHolstered = 2;
			}
			break;
		case TAUNT_FLOURISH:
			if ( ent->client->ps.weapon == WP_SABER )
			{
				if ( ent->client->ps.saberHolstered == 1 &&
						ent->client->saber[1].model[0] )
				{//turn on second saber
					G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOn );
				}
				else if ( ent->client->ps.saberHolstered == 2 )
				{//turn on first
					G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
				}
				ent->client->ps.saberHolstered = 0;
				if ( ent->client->saber[0].flourishAnim != -1 )
				{
					anim = ent->client->saber[0].flourishAnim;
				}
				else if ( ent->client->saber[1].model[0] &&
						ent->client->saber[1].flourishAnim != -1 )
				{
					anim = ent->client->saber[1].flourishAnim;
				}
				else
				{
					switch ( ent->client->ps.fd.saberAnimLevel )
					{
					case SS_MAKASHI:
					case SS_JUYO:
						anim = BOTH_SHOWOFF_FAST;
						break;
					case SS_SHII_CHO:
						anim = BOTH_SHOWOFF_MEDIUM;
						break;
					case SS_SORESU:
					case SS_ATARU:
						anim = BOTH_SHOWOFF_STRONG;
						break;
					case SS_DUAL:
						anim = BOTH_SHOWOFF_DUAL;
						break;
					case SS_STAFF:
						anim = BOTH_SHOWOFF_STAFF;
						break;
					}
				}
			}
			break;
		case TAUNT_GLOAT:
			if ( ent->client->saber[0].gloatAnim != -1 )
			{
				anim = ent->client->saber[0].gloatAnim;
			}
			else if ( ent->client->saber[1].model[0] &&
					ent->client->saber[1].gloatAnim != -1 )
			{
				anim = ent->client->saber[1].gloatAnim;
			}
			else
			{
				switch ( ent->client->ps.fd.saberAnimLevel )
				{
				case SS_MAKASHI:
				case SS_JUYO:
					anim = BOTH_VICTORY_FAST;
					break;
				case SS_SHII_CHO:
					anim = BOTH_VICTORY_MEDIUM;
					break;
				case SS_SORESU:
				case SS_ATARU:
					if ( ent->client->ps.saberHolstered )
					{//turn on first
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
					}
					ent->client->ps.saberHolstered = 0;
					anim = BOTH_VICTORY_STRONG;
					break;
				case SS_DUAL:
					if ( ent->client->ps.saberHolstered == 1 &&
							ent->client->saber[1].model[0] )
					{//turn on second saber
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOn );
					}
					else if ( ent->client->ps.saberHolstered == 2 )
					{//turn on first
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
					}
					ent->client->ps.saberHolstered = 0;
					anim = BOTH_VICTORY_DUAL;
					break;
				case SS_STAFF:
					if ( ent->client->ps.saberHolstered )
					{//turn on first
						G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOn );
					}
					ent->client->ps.saberHolstered = 0;
					anim = BOTH_VICTORY_STAFF;
					break;
				}
			}
			break;
		}
		if ( anim != -1 )
		{
			if ( ent->client->ps.groundEntityNum != ENTITYNUM_NONE ) 
			{
				ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
				ent->client->ps.forceDodgeAnim = anim;
				ent->client->ps.forceHandExtendTime = level.time + BG_AnimLength(ent->localAnimIndex, (animNumber_t)anim);
			}
			if ( taunt != TAUNT_MEDITATE 
				&& taunt != TAUNT_BOW )
			{//no sound for meditate or bow
				G_AddEvent( ent, EV_TAUNT, taunt );
			}
		}
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void JKG_PMTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );


gentity_t *currentPMEnt = 0;
// Should only be called from within Pmove, if the weapon is changed
// Call this BEFORE ps.weapon changes!
void G_PM_SwitchWeaponClip(playerState_t *ps, int newweapon, int newvariation, usercmd_t& cmd) {
	gentity_t *ent = currentPMEnt;
	weaponData_t* newWeapon = GetWeaponData(newweapon, newvariation);
	weaponData_t* oldWeapon = GetWeaponData(ps->weapon, ps->weaponVariation);
	int oldWeaponIndex = BG_GetWeaponIndexFromClass(ent->client->ps.weapon, ent->client->ps.weaponVariation);
	int newWeaponIndex = BG_GetWeaponIndexFromClass(newweapon, newvariation);
	int oldFiringMode = ent->client->firingModes[oldWeaponIndex];
	int newFiringMode = ent->client->firingModes[newWeaponIndex];

	// Determine whether our new weapon is valid.
	int selectedWeapon = cmd.invensel;
	if (selectedWeapon >= ent->inventory->size() || selectedWeapon < 0) {
		if(!ent->NPC)	//npcs are broke so don't warn us about them  --TEMP (needs a real fix for npcs to use jkg weapons)
			Com_Printf("Client %i selected inventory item %i (their inventory is only size %i!!)\n", ps->clientNum, selectedWeapon, ent->inventory->size());
		return;
	}

	// Store the current ammo amount
	if ( !oldWeapon->firemodes[0].useQuantity && oldWeapon->firemodes[0].clipSize != -1 )
	{
		ent->client->clipammo[oldWeaponIndex][oldFiringMode] = ent->client->ps.stats[STAT_AMMO];
		ent->client->ammoTable[ent->client->ps.ammoType] = ent->client->ps.stats[STAT_TOTALAMMO];
		ent->client->ammoTypes[oldWeaponIndex][oldFiringMode] = ent->client->ps.ammoType;
	}

	if ( !newWeapon->firemodes[0].useQuantity && newWeapon->firemodes[0].clipSize != -1 )
	{
		// Get the new weapon's ammo stored in STAT_AMMO
		ent->client->ps.ammoType = ent->client->ammoTypes[newWeaponIndex][newFiringMode];
		ent->client->ps.stats[STAT_AMMO] = ent->client->clipammo[newWeaponIndex][newFiringMode];
		ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
	}
	else if (newWeapon->firemodes[0].useQuantity) {
		// Get the weapon's ammo from the actual item's quantity
		itemInstance_t* item = &(*ent->inventory)[selectedWeapon];

		ent->client->ps.stats[STAT_AMMO] = item->quantity;
		ent->client->ps.stats[STAT_TOTALAMMO] = item->quantity;
		ent->client->ps.ammoType = 0;
	}
}

// Change the firing mode between guns so that they all store the firing mode that you're on. Leads to less derp in general
void G_PM_SwitchWeaponFiringMode(playerState_t *ps, int newweapon, int newvariation)
{
	gentity_t *ent = currentPMEnt;
	int weapon, variation;
	if ( !BG_GetWeaponByIndex (newweapon, &weapon, &variation) )
	{
	    return;
	}

	ent->client->ps.firingMode = ent->client->firingModes[ BG_GetWeaponIndexFromClass(newweapon, newvariation) ];
}

gentity_t *WP_FireGenericGrenade( gentity_t *ent, int firemode, vec3_t origin, vec3_t dir );
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	pmove_t		pmove;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;
	qboolean	isNPC = qfalse;
	qboolean	controlledByPlayer = qfalse;
	int i;

	client = ent->client;
	// Store the current entity for G_PM_SwitchWeaponClip
	currentPMEnt = ent;

	if (ent->s.eType == ET_NPC)
	{
		isNPC = qtrue;
	}

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED && !isNPC) {
		return;
	}

	if (!(client->ps.pm_flags & PMF_FOLLOW))
	{

		// Time to do it the right way! (read: not the way base does it)
		int j = 0;

		if(!SaberStances[client->ps.fd.saberAnimLevel].moves[LS_READY].anim)
		{
			for(i = client->ps.fd.saberAnimLevel+1; j < MAX_STANCES; j++)
			{
				i++;
				if(i >= MAX_STANCES)
				{
					i = 0;
				}
				if(SaberStances[i].moves[LS_READY].anim)
				{
					client->ps.fd.saberAnimLevel = i;
					break;
				}
			}
		}

		if (client->saber[0].model[0] && client->saber[1].model[0])
		{
			// Use a duals friendly stance whenever we're using duals, derp.
			if(!SaberStances[client->ps.fd.saberAnimLevel].isDualsFriendly)
			{
				for(i=0; i < MAX_STANCES; i++)
				{
					if(SaberStances[i].isDualsFriendly)
					{
						client->ps.fd.saberAnimLevel = i;
					}
				}
			}
		}
	}

	//JKG: passively gain credits over time, helps balance game for sucky players & new joins
	if (jkg_passiveCreditsAmount.integer > 0 && ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		int reward = 0;

		//passive rewards start after jkg_passiveCreditsWait (typically 1 minute)
		if(level.time > level.startTime + jkg_passiveCreditsWait.integer)
		{
			if (ent->client->pers.lastCreditTime + jkg_passiveCreditsRate.integer < level.time) //if the time of our last reward + time < than current time  
			{
				ent->client->pers.lastCreditTime = level.time;
				reward += jkg_passiveCreditsAmount.integer;

				//bonus reward if you are the underdog
				if (jkg_passiveUnderdogBonus.integer > 0)
				{
					//who is currently winning?
					auto my_team = ent->client->sess.sessionTeam;
					int curr_winner = -1;
					if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE])
						curr_winner = TEAM_RED;
					else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE])
						curr_winner = TEAM_BLUE;	
					else
						curr_winner = -1;	//tie

					if (my_team != curr_winner && curr_winner!= -1)
						reward += (jkg_passiveCreditsAmount.integer * .20);	//bonus money for being a loser
				}

			}
		}

		if(reward > 0)
		{
			ent->client->ps.credits += reward;		//award
			#ifdef _DEBUG
			trap->SendServerCommand(ent->s.number, va("notify 1 \"Salary: +%i Credits\"", reward));		//notify player its pay day, if debug mode enabled (otherwise this is too spammy)
			//consider a sound here
			#endif
		}
	}

	// Automatically regenerate health
	if (jkg_healthRegen.value > 0 && JKG_ClientAlive(ent)) 
	{
		if (ent->lastHealTime < level.time && (ent->damagePlumTime + jkg_healthRegenDelay.value) < level.time)
		{
			int maxHealth = ent->client->ps.stats[STAT_MAX_HEALTH];
			int pctage = (maxHealth < 100) ? jkg_healthRegen.value : (maxHealth / 100) * jkg_healthRegen.value;		// Add 1% (or 1 HP, whichever is higher)
			ent->health = ent->client->ps.stats[STAT_HEALTH] = (((ent->health + pctage) > maxHealth) ? maxHealth : ent->health + pctage);
			ent->lastHealTime = level.time + jkg_healthRegenSpeed.value;
		}
	}
	
	//regen shield (if equipped)
	if (ent->client->shieldEquipped && ent->client->ps.stats[STAT_SHIELD] <= ent->client->ps.stats[STAT_MAX_SHIELD] && JKG_ClientAlive(ent)) 
	{
		if(ent->client->ps.stats[STAT_SHIELD] < ent->client->ps.stats[STAT_MAX_SHIELD]) //if not full
		{
			if (ent->client->shieldRechargeLast + ent->client->shieldRechargeTime < level.time)
			{
				if (ent->client->shieldRegenLast + ent->client->shieldRegenTime < level.time)
				{
					ent->client->ps.stats[STAT_SHIELD]++;
					ent->client->shieldRegenLast = level.time + ent->client->shieldRegenTime;
				}

				if (!ent->client->shieldRecharging)
				{
					// In the previous frame, our shield was not recharging
					ent->client->shieldRecharging = qtrue;

					// Play the sound effect for the shield recharging, if one exists
					for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it)
					{
						if (it->equipped && it->id->itemType == ITEM_SHIELD)
						{
							if (it->id->shieldData.rechargeSoundEffect[0])
							{
								G_Sound(ent, CHAN_AUTO, G_SoundIndex(it->id->shieldData.rechargeSoundEffect));

								// Play the effect for shield recharging
								gentity_t* evEnt;
								evEnt = G_TempEntity(ent->r.currentOrigin, EV_SHIELD_RECHARGE);
								evEnt->s.otherEntityNum = ent->s.number;
							}
							break;
						}
					}
				}
			}
		}

		else	//if full
		{
			if (ent->client->shieldRecharging)
			{
				// In the previous frame, our shield was recharging - we need to turn charging off
				ent->client->shieldRecharging = qfalse;

				// Play the sound effect for the shield recharging, if one exists
				for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it)
				{
					if (it->equipped && it->id->itemType == ITEM_SHIELD)
					{
						if (it->id->shieldData.rechargeSoundEffect[0])
						{
							G_Sound(ent, CHAN_AUTO, G_SoundIndex(it->id->shieldData.chargedSoundEffect));

							// Play the effect for shield recharging
							gentity_t* evEnt;
							evEnt = G_TempEntity(ent->r.currentOrigin, EV_SHIELD_RECHARGE);		//--futuza: we are reusing the shield recharge effect, but a new one would be nice
							evEnt->s.otherEntityNum = ent->s.number;
						}
						break;
					}
				}
			}
		}
	}

// remove debuffs that shields protect from: now done in jkg_damageareas.cpp!
	// remove toxins and other debuffs that filters protects
	if (ent->inventory->size() > 0 && JKG_ClientAlive(ent))
	{
		// check inventory for filter items
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it)
		{
			if (it->equipped && ( it->id->itemType == ITEM_ARMOR || it->id->itemType == ITEM_CLOTHING) )
			{
				if (it->id->armorData.pArm->filter)
				{
					JKG_CheckFilterRemoval(&ent->client->ps);
				}
				break;
			}
		}
	}

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	if ( client && (client->ps.eFlags2&EF2_HELD_BY_MONSTER) )
	{
		G_HeldByMonster( ent, ucmd );
	}

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
	} 

	// ironsights related crap here
	if ( !isNPC && ent->client->ps.ironsightsDebounceStart && ent->client->ps.ironsightsDebounceStart <= level.time )
	{
		ent->client->ps.ironsightsDebounceStart = 0;
	}

	if (isNPC && (ucmd->serverTime - client->ps.commandTime) < 1)
	{
		ucmd->serverTime = client->ps.commandTime + 100;
	}

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
	}

	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap->Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap->Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) 
	{
		if ( ent->s.number < MAX_CLIENTS )
		{//players do nothing in intermissions
			ClientIntermissionThink( client );
			return;
		}
	}

	// spectators don't do much
	if ( client->sess.sessionTeam == TEAM_SPECTATOR || client->tempSpectate > level.time ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		if (client->deathcamTime && level.time > client->deathcamTime) {
			if ( ucmd->buttons & BUTTON_ATTACK || client->deathcamForceRespawn) {
				respawn( ent );
			}
		}
		return;
	}

	if (ent && ent->client && (ent->client->ps.eFlags & EF_INVULNERABLE))
	{
		if (ent->client->invulnerableTimer <= level.time)
		{
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
		}
	}

	if (ent->s.eType != ET_NPC)
	{
		// check for inactivity timer, but never drop the local client of a non-dedicated server
		if ( !ClientInactivityTimer( client ) ) {
			return;
		}
	}

	//Check if we should have a fullbody push effect around the player
	if (client->pushEffectTime > level.time)
	{
		client->ps.eFlags |= EF_BODYPUSH;
	}
	else if (client->pushEffectTime)
	{
		client->pushEffectTime = 0;
		client->ps.eFlags &= ~EF_BODYPUSH;
	}


	// Set our pmove type
	if (client->pmnomove) {
		client->ps.pm_type = PM_NOMOVE;
	}
	else if (client->pmlock) {
		client->ps.pm_type = PM_LOCK;
	}
	else if (client->pmfreeze) {
		ent->s.eFlags |= EF_FROZEN;
		client->ps.pm_type = PM_FREEZE;
	}
	else if (client->noclip || client->ps.eFlags & EF_DISINTEGRATION) {
		client->ps.pm_type = PM_NOCLIP;
	}
	else if (client->ps.stats[STAT_HEALTH] <= 0) {
		client->ps.pm_type = PM_DEAD;
	}
	else if (client->ps.forceGripChangeMovetype) {
		client->ps.pm_type = client->ps.forceGripChangeMovetype;
	}
	else if (client->ps.eFlags & EF_JETPACK_ACTIVE) {
		client->ps.pm_type = PM_JETPACK;
	}
	else {
		client->ps.pm_type = PM_NORMAL;
	}

	if (!(client->ps.eFlags & EF_JETPACK_ACTIVE)) {
		client->ps.eFlags &= ~EF_JETPACK_FLAMING;
	}

#define	SLOWDOWN_DIST	128.0f
#define	MIN_NPC_SPEED	16.0f

	if (client->bodyGrabIndex != ENTITYNUM_NONE)
	{
		gentity_t *grabbed = &g_entities[client->bodyGrabIndex];

		if (!grabbed->inuse || grabbed->s.eType != ET_BODY ||
			(grabbed->s.eFlags & EF_DISINTEGRATION) ||
			(grabbed->s.eFlags & EF_NODRAW))
		{
			if (grabbed->inuse && grabbed->s.eType == ET_BODY)
			{
				grabbed->s.ragAttach = 0;
			}
			client->bodyGrabIndex = ENTITYNUM_NONE;
		}
		else
		{
			mdxaBone_t rhMat;
			vec3_t rhOrg, tAng;
			vec3_t bodyDir;
			float bodyDist;

			ent->client->ps.forceHandExtend = HANDEXTEND_DRAGGING;

			if (ent->client->ps.forceHandExtendTime < level.time + 500)
			{
				ent->client->ps.forceHandExtendTime = level.time + 1000;
			}

			VectorSet(tAng, 0, ent->client->ps.viewangles[YAW], 0);
			trap->G2API_GetBoltMatrix(ent->ghoul2, 0, 0, &rhMat, tAng, ent->client->ps.origin, level.time,
				NULL, ent->modelScale); //0 is always going to be right hand bolt
			BG_GiveMeVectorFromMatrix(&rhMat, ORIGIN, rhOrg);

			VectorSubtract(rhOrg, grabbed->r.currentOrigin, bodyDir);
			bodyDist = VectorLength(bodyDir);

			if (bodyDist > 40.0f)
			{ //can no longer reach
				grabbed->s.ragAttach = 0;
				client->bodyGrabIndex = ENTITYNUM_NONE;
			}
			else if (bodyDist > 24.0f)
			{
				bodyDir[2] = 0; //don't want it floating
				//VectorScale(bodyDir, 0.1f, bodyDir);
				VectorAdd(grabbed->epVelocity, bodyDir, grabbed->epVelocity);
				G_Sound(grabbed, CHAN_AUTO, G_SoundIndex("sound/player/roll1.wav"));
			}
		}
	}
	else if (ent->client->ps.forceHandExtend == HANDEXTEND_DRAGGING)
	{
		ent->client->ps.forceHandExtend = HANDEXTEND_WEAPONREADY;
	}

	if (ent->NPC)
	{
		//FIXME: swoop should keep turning (and moving forward?) for a little bit?
		if ( ent->NPC->combatMove == qfalse )
		{
			qboolean Flying = (ucmd->upmove && (ent->client->ps.eFlags2&EF2_FLYING));//ent->client->moveType == MT_FLYSWIM);
			qboolean Climbing = (ucmd->upmove && ent->watertype&CONTENTS_LADDER );

			//client->ps.friction = 6;

			if ( ucmd->forwardmove || ucmd->rightmove || Flying )
			{
				//if ( ent->NPC->behaviorState != BS_FORMATION )
				{//In - Formation NPCs set thier desiredSpeed themselves
					if ( ucmd->buttons & BUTTON_WALKING )
					{
						ent->NPC->desiredSpeed = NPC_GetWalkSpeed( ent );//ent->NPC->stats.walkSpeed;
					}
					else//running
					{
						ent->NPC->desiredSpeed = NPC_GetRunSpeed( ent );//ent->NPC->stats.runSpeed;
					}

					if ( ent->NPC->currentSpeed >= 80 && !controlledByPlayer )
					{//At higher speeds, need to slow down close to stuff
						//Slow down as you approach your goal
					//	if ( ent->NPC->distToGoal < SLOWDOWN_DIST && client->race != RACE_BORG && !(ent->NPC->aiFlags&NPCAI_NO_SLOWDOWN) )//128
						if ( ent->NPC->distToGoal < SLOWDOWN_DIST && !(ent->NPC->aiFlags&NPCAI_NO_SLOWDOWN) )//128
						{
							if ( ent->NPC->desiredSpeed > MIN_NPC_SPEED )
							{
								float slowdownSpeed = ((float)ent->NPC->desiredSpeed) * ent->NPC->distToGoal / SLOWDOWN_DIST;

								ent->NPC->desiredSpeed = ceil(slowdownSpeed);
								if ( ent->NPC->desiredSpeed < MIN_NPC_SPEED )
								{//don't slow down too much
									ent->NPC->desiredSpeed = MIN_NPC_SPEED;
								}
							}
						}
					}
				}
			}
			else if ( Climbing )
			{
				ent->NPC->desiredSpeed = ent->NPC->stats.walkSpeed;
			}
			else
			{//We want to stop
				ent->NPC->desiredSpeed = 0;
			}

			NPC_Accelerate( ent, qfalse, qfalse );

			if ( ent->NPC->currentSpeed <= 24 && ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
			{//No-one walks this slow
				client->ps.speed = ent->NPC->currentSpeed = 0;//Full stop
				ucmd->forwardmove = 0;
				ucmd->rightmove = 0;
			}
			else
			{
				if (ent->NPC->currentSpeed <= ent->NPC->stats.walkSpeed)
				{//Play the walkanim
					ucmd->buttons |= BUTTON_WALKING;
				}
				else
				{
					ucmd->buttons &= ~BUTTON_WALKING;
				}

				if (ent->NPC->currentSpeed > 0)
				{//We should be moving
					if (Climbing || Flying)
					{
						if (!ucmd->upmove)
						{//We need to force them to take a couple more steps until stopped
							ucmd->upmove = ent->NPC->last_ucmd.upmove;//was last_upmove;
						}
					}
					else if (!ucmd->forwardmove && !ucmd->rightmove)
					{//We need to force them to take a couple more steps until stopped
						ucmd->forwardmove = ent->NPC->last_ucmd.forwardmove;//was last_forwardmove;
						ucmd->rightmove = ent->NPC->last_ucmd.rightmove;//was last_rightmove;
					}
				}

				client->ps.speed = ent->NPC->currentSpeed;

				//Slow down on turns - don't orbit!!!
				float turndelta = (180 - fabs(AngleDelta(ent->r.currentAngles[YAW], ent->NPC->desiredYaw))) / 180;

				if (turndelta < 0.75f)
				{
					client->ps.speed = 0;
				}
				else if (ent->NPC->distToGoal < 100 && turndelta < 1.0)
				{//Turn is greater than 45 degrees or closer than 100 to goal
					client->ps.speed = floor(((float)(client->ps.speed))*turndelta);
				}
			}
		}
		else
		{	
			ent->NPC->desiredSpeed = ( ucmd->buttons & BUTTON_WALKING ) ? NPC_GetWalkSpeed( ent ) : NPC_GetRunSpeed( ent );

			client->ps.speed = ent->NPC->desiredSpeed;
		}

		if (ucmd->buttons & BUTTON_WALKING)
		{ //sort of a hack I guess since MP handles walking differently from SP (has some proxy cheat prevention methods)
			if (ucmd->forwardmove > bgConstants.walkingSpeed)
 			{
				ucmd->forwardmove = bgConstants.walkingSpeed;	
 			}
			else if (ucmd->forwardmove < -bgConstants.walkingSpeed)
 			{
				ucmd->forwardmove = -bgConstants.walkingSpeed;
 			}
 			
			if (ucmd->rightmove > bgConstants.walkingSpeed)
 			{
				ucmd->rightmove = bgConstants.walkingSpeed;
 			}
			else if ( ucmd->rightmove < -bgConstants.walkingSpeed)
 			{
				ucmd->rightmove = -bgConstants.walkingSpeed;
 			}

			//ent->client->ps.speed = ent->client->ps.basespeed = NPC_GetRunSpeed( ent );
		}
		client->ps.basespeed = client->ps.speed;
	}
	else if ((!ent->NPC || ent->s.NPC_class != CLASS_VEHICLE)) //if riding a vehicle it will manage our speed and such
	{
		// set speed
		client->ps.speed = g_speed.value;

		if (client->bodyGrabIndex != ENTITYNUM_NONE)
		{ //can't go nearly as fast when dragging a body around
			client->ps.speed *= 0.2f;
		}

		client->ps.basespeed = client->ps.speed;
	}

	if ( !ent->NPC || !(ent->NPC->aiFlags&NPCAI_CUSTOM_GRAVITY) )
	{//use global gravity
		if (!client->customGravity)
		{
			client->ps.gravity = g_gravity.value;
		}
	}

	if (ent->client->ps.duelInProgress)
	{
		gentity_t *duelAgainst = &g_entities[ent->client->ps.duelIndex];

		//Keep the time updated, so once this duel ends this player can't engage in a duel for another
		//10 seconds. This will give other people a chance to engage in duels in case this player wants
		//to engage again right after he's done fighting and someone else is waiting.
		ent->client->ps.fd.privateDuelTime = level.time + 10000;

		if (ent->client->ps.duelTime < level.time)
		{
			//Bring out the sabers
			if (ent->client->ps.weapon == WP_SABER 
				&& ent->client->ps.saberHolstered 
				&& ent->client->ps.duelTime )
			{
				ent->client->ps.saberHolstered = 0;

				if (ent->client->saber[0].soundOn)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
				}
				if (ent->client->saber[1].soundOn)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
				}

				G_AddEvent(ent, EV_PRIVATE_DUEL, 2);

				ent->client->ps.duelTime = 0;
			}

			if (duelAgainst 
				&& duelAgainst->client 
				&& duelAgainst->inuse 
				&& duelAgainst->client->ps.weapon == WP_SABER 
				&& duelAgainst->client->ps.saberHolstered 
				&& duelAgainst->client->ps.duelTime)
			{
				duelAgainst->client->ps.saberHolstered = 0;

				if (duelAgainst->client->saber[0].soundOn)
				{
					G_Sound(duelAgainst, CHAN_AUTO, duelAgainst->client->saber[0].soundOn);
				}
				if (duelAgainst->client->saber[1].soundOn)
				{
					G_Sound(duelAgainst, CHAN_AUTO, duelAgainst->client->saber[1].soundOn);
				}

				G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 2);

				duelAgainst->client->ps.duelTime = 0;
			}
		}
		else
		{
			client->ps.speed = 0;
			client->ps.basespeed = 0;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
		}

		if (!duelAgainst || !duelAgainst->client || !duelAgainst->inuse ||
			duelAgainst->client->ps.duelIndex != ent->s.number)
		{
			ent->client->ps.duelInProgress = 0;
			G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
		}
		else if (duelAgainst->health < 1 || duelAgainst->client->ps.stats[STAT_HEALTH] < 1)
		{
			ent->client->ps.duelInProgress = 0;
			duelAgainst->client->ps.duelInProgress = 0;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
			G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 0);

			//Winner gets full health.. providing he's still alive
			if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
			{
				if (ent->health < ent->client->ps.stats[STAT_MAX_HEALTH])
				{
					ent->client->ps.stats[STAT_HEALTH] = ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
				}

				if (g_spawnInvulnerability.integer)
				{
					ent->client->ps.eFlags |= EF_INVULNERABLE;
					ent->client->invulnerableTimer = level.time + g_spawnInvulnerability.integer;
				}
			}

			//Private duel announcements are now made globally because we only want one duel at a time.
			if (ent->health > 0 && ent->client->ps.stats[STAT_HEALTH] > 0)
			{
				trap->SendServerCommand( -1, va("cp \"%s %s %s!\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELWINNER"), duelAgainst->client->pers.netname) );
			}
			else
			{ //it was a draw, because we both managed to die in the same frame
				trap->SendServerCommand( -1, va("cp \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELTIE")) );
			}
		}
		else
		{
			vec3_t vSub;
			float subLen = 0;

			VectorSubtract(ent->client->ps.origin, duelAgainst->client->ps.origin, vSub);
			subLen = VectorLength(vSub);

			if (subLen >= 1024)
			{
				ent->client->ps.duelInProgress = 0;
				duelAgainst->client->ps.duelInProgress = 0;

				G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
				G_AddEvent(duelAgainst, EV_PRIVATE_DUEL, 0);

				trap->SendServerCommand( -1, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELSTOP")) );
			}
		}
	}

	// K, do some saber code stuff here regarding blocks --eez
	if ( ent->client->ps.weapon == WP_SABER &&
		!ent->NPC && !BG_SabersOff(&ent->client->ps))								// NPCs don't use this method, they do it on their own terms --eez
	{
		if( ent->client->pers.cmd.buttons & BUTTON_IRONSIGHTS /*&&			// holding sights button
			(ent->client->ps.groundEntityNum != ENTITYNUM_NONE ) */ &&		// not in air
			//ent->client->ps.forceAllowDeactivateTime > level.time &&	// not using a force power
			(  ent->client->ps.torsoTimer <= 0 || ent->client->saberBlockDebounce >= level.time || // NOT ATTACKING (swingblocks not permitted, period)
			(ent->client->ps.saberMove >= LS_R_TL2BR && ent->client->ps.saberMove <= LS_R_T2B &&
			ent->client->ps.torsoTimer < 400) ))														// OR, we're returning from an attack (slight delay)
		{
			if( !(ent->client->ps.saberActionFlags & (1 << SAF_BLOCKING)) )
			{
				ent->client->ps.saberActionFlags |= (1 << SAF_BLOCKING);				// Marks the client for being in manual block mode.
				ent->client->saberBlockTime = level.time;				// Manual blocking at appropriate times will reduce the force drop from blaster bolts.
			}

			if( ent->client->pers.cmd.buttons & BUTTON_ATTACK )		//if projectile blocking
			{
				if( !(ent->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) ) )
				{
					ent->client->ps.saberActionFlags |= ( 1 << SAF_PROJBLOCKING );
					ent->client->saberProjBlockTime = level.time;
					
				}
				//ent->client->ps.saberMove = LS_REFLECT_UP;
				ent->client->pers.cmd.buttons &= ~BUTTON_ATTACK;
			}
			else
			{
				ent->client->ps.saberActionFlags &= ~( 1 << SAF_PROJBLOCKING );
			}
		}

		//if blocking and in the air
		else if ( ent->client->ps.weaponTime >= 0 && ent->client->ps.saberActionFlags & (1 << SAF_BLOCKING) &&
			ent->client->pers.cmd.buttons & BUTTON_IRONSIGHTS &&
			ent->client->ps.groundEntityNum != ENTITYNUM_NONE )
		{
			// FIXME: decide if blocking in the air should be allowed

			/*if (ent->client->pers.cmd.buttons & BUTTON_ATTACK)		//if projectile blocking
			{
				if (!(ent->client->ps.saberActionFlags & (1 << SAF_PROJBLOCKING)))
				{
					ent->client->ps.saberActionFlags |= (1 << SAF_PROJBLOCKING);
					ent->client->saberProjBlockTime = level.time;
				}
			*/
			//ent->client->ps.saberMove = LS_REFLECT_UP;
			//ent->client->pers.cmd.buttons &= ~BUTTON_ATTACK;
		}

		//we're not blocking
		else
		{
			ent->client->ps.saberActionFlags &= ~(1 << SAF_BLOCKING);
			ent->client->ps.saberActionFlags &= ~( 1 << SAF_PROJBLOCKING );
			ent->client->blockingLightningAccumulation = 0;
		}
	}

	if (ent->client->doingThrow > level.time)
	{
		gentity_t *throwee = &g_entities[ent->client->throwingIndex];

		if (!throwee->inuse || !throwee->client || throwee->health < 1 ||
			throwee->client->sess.sessionTeam == TEAM_SPECTATOR ||
			(throwee->client->ps.pm_flags & PMF_FOLLOW) ||
			throwee->client->throwingIndex != ent->s.number)
		{
			ent->client->doingThrow = 0;
			ent->client->ps.forceHandExtend = HANDEXTEND_NONE;

			if (throwee->inuse && throwee->client)
			{
				throwee->client->ps.heldByClient = 0;
				throwee->client->beingThrown = 0;

				if (throwee->client->ps.forceHandExtend != HANDEXTEND_POSTTHROWN)
				{
					throwee->client->ps.forceHandExtend = HANDEXTEND_NONE;
				}
			}
		}
	}

	if (ent->client->beingThrown > level.time)
	{
		gentity_t *thrower = &g_entities[ent->client->throwingIndex];

		if (!thrower->inuse || !thrower->client || thrower->health < 1 ||
			thrower->client->sess.sessionTeam == TEAM_SPECTATOR ||
			(thrower->client->ps.pm_flags & PMF_FOLLOW) ||
			thrower->client->throwingIndex != ent->s.number)
		{
			ent->client->ps.heldByClient = 0;
			ent->client->beingThrown = 0;

			if (ent->client->ps.forceHandExtend != HANDEXTEND_POSTTHROWN)
			{
				ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
			}

			if (thrower->inuse && thrower->client)
			{
				thrower->client->doingThrow = 0;
				thrower->client->ps.forceHandExtend = HANDEXTEND_NONE;
			}
		}
		else if (thrower->inuse && thrower->client && thrower->ghoul2 &&
			trap->G2_HaveWeGhoul2Models(thrower->ghoul2))
		{
			{
				float pDif = 40.0f;
				vec3_t boltOrg, pBoltOrg;
				vec3_t tAngles;
				vec3_t vDif;
				vec3_t entDir, otherAngles;
				vec3_t fwd, right;

				//Always look at the thrower.
				VectorSubtract( thrower->client->ps.origin, ent->client->ps.origin, entDir );
				VectorCopy( ent->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( ent, otherAngles );

				VectorCopy(thrower->client->ps.viewangles, tAngles);
				tAngles[PITCH] = tAngles[ROLL] = 0;

				//Get the direction between the pelvis and position of the hand
				VectorCopy(thrower->client->ps.origin, pBoltOrg);
				AngleVectors(tAngles, fwd, right, 0);
				boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
				boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
				boltOrg[2] = pBoltOrg[2];

				VectorSubtract(ent->client->ps.origin, boltOrg, vDif);
				if (VectorLength(vDif) > 32.0f && (thrower->client->doingThrow - level.time) < 4500)
				{ //the hand is too far away, and can no longer hold onto us, so escape.
					ent->client->ps.heldByClient = 0;
					ent->client->beingThrown = 0;
					thrower->client->doingThrow = 0;

					thrower->client->ps.forceHandExtend = HANDEXTEND_NONE;
					G_EntitySound( thrower, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

					ent->client->ps.forceDodgeAnim = 2;
					ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
					ent->client->ps.forceHandExtendTime = level.time + 500;
					ent->client->ps.velocity[2] = 400;
					G_PreDefSound(ent->client->ps.origin, PDSOUND_FORCEJUMP);
				}
				else if ((client->beingThrown - level.time) < 4000)
				{ //step into the next part of the throw, and go flying back
					float vScale = 400.0f;
					ent->client->ps.forceHandExtend = HANDEXTEND_POSTTHROWN;
					ent->client->ps.forceHandExtendTime = level.time + 1200;
					ent->client->ps.forceDodgeAnim = 0;

					thrower->client->ps.forceHandExtend = HANDEXTEND_POSTTHROW;
					thrower->client->ps.forceHandExtendTime = level.time + 200;

					ent->client->ps.heldByClient = 0;

					ent->client->ps.heldByClient = 0;
					ent->client->beingThrown = 0;
					thrower->client->doingThrow = 0;

					AngleVectors(thrower->client->ps.viewangles, vDif, 0, 0);
					ent->client->ps.velocity[0] = vDif[0]*vScale;
					ent->client->ps.velocity[1] = vDif[1]*vScale;
					ent->client->ps.velocity[2] = 400;

					G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
					G_EntitySound( thrower, CHAN_VOICE, G_SoundIndex("*jump1.wav") );

					//Set the thrower as the "other killer", so if we die from fall/impact damage he is credited.
					ent->client->ps.otherKiller = thrower->s.number;
					ent->client->ps.otherKillerTime = level.time + 8000;
					ent->client->ps.otherKillerDebounceTime = level.time + 100;
				}
				else
				{ //see if we can move to be next to the hand.. if it's not clear, break the throw.
					vec3_t intendedOrigin;
					trace_t tr;
					trace_t tr2;

					VectorSubtract(boltOrg, pBoltOrg, vDif);
					VectorNormalize(vDif);

					VectorClear(ent->client->ps.velocity);
					intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
					intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
					intendedOrigin[2] = thrower->client->ps.origin[2];

					trap->Trace(&tr, intendedOrigin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, ent->clipmask, 0, 0, 0);
					trap->Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID, 0, 0, 0);

					if (tr.fraction == 1.0 && !tr.startsolid && tr2.fraction == 1.0 && !tr2.startsolid)
					{
						VectorCopy(intendedOrigin, ent->client->ps.origin);
						
						if ((client->beingThrown - level.time) < 4800)
						{
							ent->client->ps.heldByClient = thrower->s.number+1;
						}
					}
					else
					{ //if the guy can't be put here then it's time to break the throw off.
						ent->client->ps.heldByClient = 0;
						ent->client->beingThrown = 0;
						thrower->client->doingThrow = 0;

						thrower->client->ps.forceHandExtend = HANDEXTEND_NONE;
						G_EntitySound( thrower, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

						ent->client->ps.forceDodgeAnim = 2;
						ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
						ent->client->ps.forceHandExtendTime = level.time + 500;
						ent->client->ps.velocity[2] = 400;
						G_PreDefSound(ent->client->ps.origin, PDSOUND_FORCEJUMP);
					}
				}
			}
		}
	}
	else if (ent->client->ps.heldByClient)
	{
		ent->client->ps.heldByClient = 0;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pmove, 0, sizeof(pmove));

	if ( ent->flags & FL_FORCE_GESTURE ) {
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
	}

	if (ent->client && ent->client->ps.fallingToDeath &&
		(level.time - FALL_FADE_TIME) > ent->client->ps.fallingToDeath)
	{ //die!
		if (ent->health > 0)
		{
			gentity_t *otherKiller = ent;
			if (ent->client->ps.otherKillerTime > level.time &&
				ent->client->ps.otherKiller != ENTITYNUM_NONE)
			{
				otherKiller = &g_entities[ent->client->ps.otherKiller];

				if (!otherKiller->inuse)
				{
					otherKiller = ent;
				}
			}

			if (ent->s.eType == ET_NPC)
			{
				player_die(ent, ent, ent, 100000, MOD_FALLING);
				ent->client->ps.fallingToDeath = 0;
				ent->think = G_FreeEntity;
			}
			else // UQ1: This messes up all sorts of stuff...
			{
				G_Damage(ent, otherKiller, otherKiller, NULL, ent->client->ps.origin, 9999, DAMAGE_NO_PROTECTION, MOD_FALLING);
				ent->client->ps.fallingToDeath = 0;
			}
			//player_die(ent, ent, ent, 100000, MOD_FALLING);
	//		if (!ent->NPC)
	//		{
	//			respawn(ent);
	//		}
	//		ent->client->ps.fallingToDeath = 0;

			G_MuteSound(ent->s.number, CHAN_VOICE); //stop screaming, because you are dead!
		}
	}

	if (ent->client->ps.otherKillerTime > level.time &&
		ent->client->ps.groundEntityNum != ENTITYNUM_NONE &&
		ent->client->ps.otherKillerDebounceTime < level.time)
	{
		ent->client->ps.otherKillerTime = 0;
		ent->client->ps.otherKiller = ENTITYNUM_NONE;
	}
	else if (ent->client->ps.otherKillerTime > level.time &&
		ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
	{
		if (ent->client->ps.otherKillerDebounceTime < (level.time + 100))
		{
			ent->client->ps.otherKillerDebounceTime = level.time + 100;
		}
	}

	//FIXME: need to do this before check to avoid walls and cliffs (or just cliffs?)
	G_AddPushVecToUcmd( ent, ucmd );

	//play/stop any looping sounds tied to controlled movement
	G_CheckMovingLoopingSounds( ent, ucmd );

	pmove.ps = &client->ps;
	pmove.cmd = *ucmd;
	if ( pmove.ps->pm_type == PM_DEAD ) {
		pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( ent->r.svFlags & SVF_BOT ) {
		pmove.tracemask = MASK_PLAYERSOLID | CONTENTS_MONSTERCLIP;
	}
	else {
		pmove.tracemask = MASK_PLAYERSOLID;
	}
	pmove.trace = JKG_PMTrace; //trap->Trace;
	pmove.pointcontents = trap->PointContents;
	pmove.debugLevel = g_debugMove.integer;
	pmove.noFootsteps = ( dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pmove.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pmove.pmove_msec = pmove_msec.integer;
	pmove.pmove_float = pmove_float.integer;

	pmove.animations = bgAllAnims[ent->localAnimIndex].anims;//NULL;

	//rww - bgghoul2
	pmove.ghoul2 = NULL;

#ifdef _DEBUG
	if (g_disableServerG2.integer)
	{

	}
	else
#endif
	if (ent->ghoul2)
	{
		if (ent->localAnimIndex >= NUM_RESERVED_ANIMSETS)
		{ //if it isn't humanoid then we will be having none of this.
			pmove.ghoul2 = NULL;
		}
		else
		{
			pmove.ghoul2 = ent->ghoul2;
			pmove.g2Bolts_LFoot = trap->G2API_AddBolt(ent->ghoul2, 0, "*l_leg_foot");
			pmove.g2Bolts_RFoot = trap->G2API_AddBolt(ent->ghoul2, 0, "*r_leg_foot");
		}
	}

	//I'll just do this every frame in case the scale changes in realtime (don't need to update the g2 inst for that)
	VectorCopy(ent->modelScale, pmove.modelScale);
	//rww end bgghoul2

	pmove.gametype = level.gametype;
	pmove.debugMelee = g_debugMelee.integer;
	pmove.stepSlideFix = g_stepSlideFix.integer;

	// BIG FIXME: do this only when userinfo changes ... !
	if(ent->client && !ent->NPC)
	{
		char			userinfo[MAX_INFO_STRING];
		char			*text;
		userinfo[0] = '\0';

		//Okay, first, grab the sex
		trap->GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
		text = Info_ValueForKey(userinfo, "sex");

		if(!Q_stricmp(text, "female") || !Q_stricmp(text, "f"))
		{
			pmove.gender = GENDER_FEMALE;
		}
		else if (!Q_stricmp(text, "male") || !Q_stricmp(text, "m"))
		{
			pmove.gender = GENDER_MALE;
		}
		else
		{
			pmove.gender = GENDER_NEUTER;
		}
	}

	pmove.noSpecMove = g_noSpecMove.integer;

	pmove.nonHumanoid = (ent->localAnimIndex >= NUM_RESERVED_ANIMSETS);

	VectorCopy( client->ps.origin, client->oldOrigin );

	//Set up bg entity data
	pmove.baseEnt = (bgEntity_t *)g_entities;
	pmove.entSize = sizeof(gentity_t);

	if (ent->client->ps.saberLockTime > level.time)
	{
		gentity_t *blockOpp = &g_entities[ent->client->ps.saberLockEnemy];

		if (blockOpp && blockOpp->inuse && blockOpp->client)
		{
			vec3_t lockDir, lockAng;

			VectorSubtract( blockOpp->r.currentOrigin, ent->r.currentOrigin, lockDir );
			vectoangles(lockDir, lockAng);
			SetClientViewAngle( ent, lockAng );
		}

		if ( ent->client->ps.saberLockHitCheckTime < level.time )
		{//have moved to next frame since last lock push
			ent->client->ps.saberLockHitCheckTime = level.time;//so we don't push more than once per server frame
			if ( ( ent->client->buttons & BUTTON_ATTACK ) && ! ( ent->client->oldbuttons & BUTTON_ATTACK ) )
			{
				if ( ent->client->ps.saberLockHitIncrementTime < level.time )
				{//have moved to next frame since last saberlock attack button press
					int lockHits = 0;
					ent->client->ps.saberLockHitIncrementTime = level.time;//so we don't register an attack key press more than once per server frame
					//NOTE: FP_SABER_OFFENSE level already taken into account in PM_SaberLocked
					if ( (ent->client->ps.fd.forcePowersActive&(1<<FP_RAGE)) )
					{//raging: push harder
						lockHits = 1+ent->client->ps.fd.forcePowerLevel[FP_RAGE];
					}
					else
					{//normal attack
						lockHits = 1;
					}
					if ( ent->client->ps.fd.forceRageRecoveryTime > level.time 
						&& Q_irand( 0, 1 ) )
					{//finished raging: weak
						lockHits -= 1;
					}
					lockHits += ent->client->saber[0].lockBonus;
					if ( ent->client->saber[1].model[0]
						&& !ent->client->ps.saberHolstered )
					{
						lockHits += ent->client->saber[1].lockBonus;
					}
					ent->client->ps.saberLockHits += lockHits;
					if ( g_saberLockRandomNess.integer )
					{
						ent->client->ps.saberLockHits += Q_irand( 0, g_saberLockRandomNess.integer );
						if ( ent->client->ps.saberLockHits < 0 )
						{
							ent->client->ps.saberLockHits = 0;
						}
					}
				}
			}
			if ( ent->client->ps.saberLockHits > 0 )
			{
				if ( !ent->client->ps.saberLockAdvance )
				{
					ent->client->ps.saberLockHits--;
				}
				ent->client->ps.saberLockAdvance = qtrue;
			}
		}
	}
	else
	{
		ent->client->ps.saberLockFrame = 0;
		//check for taunt
		if ( (pmove.cmd.generic_cmd == GENCMD_ENGAGE_DUEL) && (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL) )
		{//already in a duel, make it a taunt command
			pmove.cmd.buttons |= BUTTON_GESTURE;
		}
	}

	if (ent->s.number >= MAX_CLIENTS)
	{
		VectorCopy(ent->r.mins, pmove.mins);
		VectorCopy(ent->r.maxs, pmove.maxs);
	}

	// Copy the ammo from the client ammo table into their networked stat
	int weaponClass = BG_GetWeaponIndex(ent->client->ps.weapon, ent->client->ps.weaponVariation);
	weaponData_t* weaponData = GetWeaponData(ent->client->ps.weapon, ent->client->ps.weaponVariation);
	int inventoryItem = pmove.cmd.invensel;
	if (inventoryItem > 0 && inventoryItem < ent->inventory->size()) {
		// If we're using a weapon that uses stack quantity instead of ammo for its firing mode, we need to copy the quantity to the ammo
		if (weaponData->numFiringModes > 0 && weaponData->firemodes[0].useQuantity) {
			itemInstance_t* item = &(*ent->inventory)[inventoryItem];
			ent->client->ps.stats[STAT_AMMO] = item->quantity;
			ent->client->ps.stats[STAT_TOTALAMMO] = item->quantity;
		}
		else if (weaponData->firemodes[ent->client->ps.firingMode].clipSize) {
			ent->client->ps.stats[STAT_AMMO] = ent->client->clipammo[weaponClass][ent->client->ps.firingMode];
			ent->client->ps.ammoType = ent->client->ammoTypes[weaponClass][ent->client->ps.firingMode];
			ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
		}
	}

	/* JKG - When a cooked grenade should explode in your hand.. */
	if ( ent->grenadeCookTime && ent->grenadeCookTime < level.time )
	{
		gentity_t	*bolt;
		int			 preWeapon, preVariation;
	
		/* Remember the weapon and variation used before forcing a switch to make the correct grenade explode */
		preWeapon = ent->s.weapon;
		preVariation = ent->s.weaponVariation;

		/* Force the weapon and variation to be whatever has been cooked */
		ent->s.weapon = ent->grenadeWeapon;
		ent->s.weaponVariation = ent->grenadeVariation;

		/* Prepare the dropped grenade as a primary grenade and set the explosion time */
		bolt = WP_FireGenericGrenade( ent, qfalse, ent->client->ps.origin, ent->client->ps.origin );
		bolt->genericValue5	= 0;
		bolt->count = 1;

		/* Overrule the origin of this entity, make it set the origin in ourselves (it's in our hand after all) */
		G_SetOrigin( bolt, ent->client->ps.origin );

		/* Force two updates; The first will set the explode and the second will force the explode */
		bolt->think( bolt );
		bolt->think( bolt );

		/* Set the weapon state back, as if we never had a weapon */
		ent->client->ps.weaponstate = WEAPON_FIRING;
		ent->client->ps.weaponTime += 1000;
		ent->client->ps.weaponChargeTime = 0;

		/* Remove a count since we don't do this now. */
		//if(pm->ps->weapon == WP_THERMAL)
			BG_AdjustItemStackQuantity(ent, inventoryItem, -1);

		/* Return the correct weapon and variation for the client */
		ent->s.weapon = preWeapon;
		ent->s.weaponVariation = preVariation;

		/* Remove the cooked grenade timer */
		ent->grenadeCookTime = 0;
		
	}
	
	if ( BG_IsSprinting (&ent->client->ps, &pmove.cmd, qfalse) )
	{
	    ent->client->ps.eFlags |= EF_SPRINTING;
    }
    else
    {
        ent->client->ps.eFlags &= ~EF_SPRINTING;
    }

	/* Run the Pmove (this will generate bg_pmove events */
	// VMove is not called yet. This will be used when vehicle rewrite happens.
	/*if( ent->client->ps.m_iVehicleNum >= MAX_CLIENTS &&
		ent->client->ps.clientNum < MAX_CLIENTS )
	{
		vmove_t vm;
		memcpy(&vm.cmd, &ent->client->pers.cmd, sizeof(usercmd_t));
		vm.cmd.upmove = 0;
		
		vm.ps = &ent->client->ps;

		vm.isRider = qtrue;

		Vmove(&vm);

	}
	else*/
	{
		Pmove (&pmove);
	}

	// It's possible for the weapon and variation to change in the middle of a pmove, so we re-get it here.
	weaponData = GetWeaponData(ent->client->ps.weapon, ent->client->ps.weaponVariation);
	if ( weaponData->firemodes[ent->client->ps.firingMode].clipSize )
	{
		ent->client->clipammo[ ent->client->ps.weaponId ][ent->client->ps.firingMode] = ent->client->ps.stats[STAT_AMMO];
	}

	if (ent->client->solidHack)
	{
		if (ent->client->solidHack > level.time)
		{ //whee!
			ent->r.contents = 0;
		}
		else
		{
			ent->r.contents = CONTENTS_BODY;
			ent->client->solidHack = 0;
		}
	}
	
	if ( ent->NPC )
	{
		VectorCopy( ent->client->ps.viewangles, ent->r.currentAngles );
	}

	if (pmove.checkDuelLoss)
	{
		if (pmove.checkDuelLoss > 0 && (pmove.checkDuelLoss <= MAX_CLIENTS || (pmove.checkDuelLoss < (MAX_GENTITIES-1) && g_entities[pmove.checkDuelLoss-1].s.eType == ET_NPC) ) )
		{
			gentity_t *clientLost = &g_entities[pmove.checkDuelLoss-1];

			if (clientLost && clientLost->inuse && clientLost->client && Q_irand(0, 40) > clientLost->health)
			{
				vec3_t attDir;
				VectorSubtract(ent->client->ps.origin, clientLost->client->ps.origin, attDir);
				VectorNormalize(attDir);

				VectorClear(clientLost->client->ps.velocity);
				clientLost->client->ps.forceHandExtend = HANDEXTEND_NONE;
				clientLost->client->ps.forceHandExtendTime = 0;

				gGAvoidDismember = 1;
				G_Damage(clientLost, ent, ent, attDir, clientLost->client->ps.origin, 9999, DAMAGE_NO_PROTECTION, MOD_SABER);

				if (clientLost->health < 1)
				{
					gGAvoidDismember = 2;
					G_CheckForDismemberment(clientLost, ent, clientLost->client->ps.origin, 999, (clientLost->client->ps.legsAnim), qfalse);
				}

				gGAvoidDismember = 0;
			}
			else if (clientLost && clientLost->inuse && clientLost->client &&
				clientLost->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN && clientLost->client->ps.saberEntityNum)
			{ //if we didn't knock down it was a circle lock. So as punishment, make them lose their saber and go into a proper anim
				saberCheckKnockdown_DuelLoss(&g_entities[clientLost->client->ps.saberEntityNum], clientLost, ent);
			}
		}

		pmove.checkDuelLoss = 0;
	}

	if (pmove.cmd.generic_cmd &&
		(pmove.cmd.generic_cmd != ent->client->lastGenCmd || ent->client->lastGenCmdTime < level.time))
	{
		ent->client->lastGenCmd = pmove.cmd.generic_cmd;

		switch(pmove.cmd.generic_cmd)
		{
		case 0:
			break;
		case GENCMD_SABERSWITCH:
			Cmd_ToggleSaber_f(ent);
			break;
		case GENCMD_ENGAGE_DUEL:
			if ( level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL )
			{//already in a duel, made it a taunt command
			}
			else
			{
				Cmd_EngageDuel_f(ent);
			}
			break;
		case GENCMD_RELOAD:
			Cmd_Reload_f(ent);
			break;
		case GENCMD_ZOOM:
			break;
		case GENCMD_SABERATTACKCYCLE:
			Cmd_SaberAttackCycle_f(ent);
			break;
		case GENCMD_TAUNT:
			G_SetTauntAnim( ent, TAUNT_TAUNT );
			break;
		case GENCMD_BOW:
			G_SetTauntAnim( ent, TAUNT_BOW );
			break;
		case GENCMD_MEDITATE:
			G_SetTauntAnim( ent, TAUNT_MEDITATE );
			break;
		case GENCMD_FLOURISH:
			G_SetTauntAnim( ent, TAUNT_FLOURISH );
			break;
		case GENCMD_GLOAT:
			G_SetTauntAnim( ent, TAUNT_GLOAT );
			break;
		default:
			break;
		}
	}

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qfalse );
		//rww - 12-03-02 - Don't snap the origin of players! It screws prediction all up.
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qfalse );
	}

	if (isNPC)
	{
		ent->s.eType = ET_NPC;
	}

	SendPendingPredictableEvents( &ent->client->ps );

	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;		// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	if (ent->s.eType != ET_NPC)
	{ //let vehicles that are getting broken apart do their own crazy sizing stuff
		VectorCopy (pmove.mins, ent->r.mins);
		VectorCopy (pmove.maxs, ent->r.maxs);
	}

	ent->waterlevel = pmove.waterlevel;
	ent->watertype = pmove.watertype;

	// execute client events
	ClientEvents( ent, oldEventSequence );

	if ((ent->client->pers.cmd.buttons & BUTTON_USE) && ent->client->ps.useDelay < level.time)
	{
		TryUse(ent);
		ent->client->ps.useDelay = level.time + 100;
	}

	// link entity now, after any personal teleporters have been used
	trap->LinkEntity ((sharedEntity_t *)ent);
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	//test for solid areas in the AAS file
//	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts( ent, &pmove );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// Did we kick someone in our pmove sequence?
	if (client->ps.forceKickFlip)
	{
		gentity_t *faceKicked = &g_entities[client->ps.forceKickFlip-1];

		if (faceKicked && faceKicked->client && (!OnSameTeam(ent, faceKicked) || g_friendlyFire.integer) &&
			(!faceKicked->client->ps.duelInProgress || faceKicked->client->ps.duelIndex == ent->s.number) &&
			(!ent->client->ps.duelInProgress || ent->client->ps.duelIndex == faceKicked->s.number))
		{
			if ( faceKicked && faceKicked->client && faceKicked->health && faceKicked->takedamage )
			{//push them away and do pain
				vec3_t oppDir;
				int strength = (int)VectorNormalize2( client->ps.velocity, oppDir );

				strength *= 0.05;

				VectorScale( oppDir, -1, oppDir );

				if( !(faceKicked->client->ps.saberActionFlags & ( 1 << SAF_BLOCKING ) ) )
				{// Don't damage them, that way they don't spam
					G_Damage( faceKicked, ent, ent, oppDir, client->ps.origin, strength, DAMAGE_NO_SHIELD, MOD_MELEE );
				}

				if ( faceKicked->client->ps.weapon != WP_SABER ||
					 faceKicked->client->ps.fd.saberAnimLevel != FORCE_LEVEL_3 ||
					 (!BG_SaberInAttack(faceKicked->client->ps.saberMove) && !PM_SaberInStart(faceKicked->client->ps.saberMove) && !PM_SaberInReturn(faceKicked->client->ps.saberMove) && !PM_SaberInTransition(faceKicked->client->ps.saberMove)) )
				{
					if (faceKicked->health > 0 &&
						faceKicked->client->ps.stats[STAT_HEALTH] > 0 &&
						faceKicked->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN)
					{
						if (BG_KnockDownable(&faceKicked->client->ps) && Q_irand(1, 10) <= 3 &&
							!(faceKicked->client->ps.saberActionFlags & ( 1 << SAF_BLOCKING )))
						{ //only actually knock over sometimes, but always do velocity hit
							faceKicked->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
							faceKicked->client->ps.forceHandExtendTime = level.time + 1100;
							faceKicked->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
						}

						faceKicked->client->ps.otherKiller = ent->s.number;
						faceKicked->client->ps.otherKillerTime = level.time + 5000;
						faceKicked->client->ps.otherKillerDebounceTime = level.time + 100;

						faceKicked->client->ps.velocity[0] = oppDir[0]*(strength*40);
						faceKicked->client->ps.velocity[1] = oppDir[1]*(strength*40);
						faceKicked->client->ps.velocity[2] = 200;
					}
				}

				G_Sound( faceKicked, CHAN_AUTO, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );
			}
		}

		client->ps.forceKickFlip = 0;
	}

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 
		&& !(client->ps.eFlags2&EF2_HELD_BY_MONSTER)//can't respawn while being eaten
		&& ent->s.eType != ET_NPC ) {
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime && !gDoSlowMoDuel ) {
			// forcerespawn is to prevent users from waiting out powerups
			// Jedi Knight Galaxies
			// Custom respawn handling here
			if (ent->r.svFlags & SVF_BOT)
			{
				respawn(ent);
				return;
			}
			else if (!client->deathcamTime) {
				// No deathcam set-up yet, do so now
				client->deathcamTime = level.time + (!jkg_deathTimer.integer ? 0 : 5000);
				client->deathcamForceRespawn = 0;
				GLua_Hook_PlayerDeathcam(ent->s.number, &client->deathcamTime, &client->deathcamForceRespawn);
				VectorCopy(client->ps.origin, client->deathcamCenter);
				SnapVector(client->deathcamCenter);
				client->deathcamRadius = 200; // 200 units before we hit the edges of our movement area
				trap->SendServerCommand(ent->s.number, va("dc %i %i %i %i %i", client->deathcamTime, client->deathcamRadius, (int)client->deathcamCenter[0], (int)client->deathcamCenter[1], (int)client->deathcamCenter[2]));
				JKG_PermaSpectate(ent);
				
				//--futuza to do in phase 2: 
				/* leave a corpse behind, that corpse will eventually decay, but should last for a few mins
				   should be able to loot corpses for dropped items, items will be removed from players inventory
				   based on drop rules, unless they have high notiriety only a few items are dropped
				   allow players in the death cam to be seen by players using force sense as 'ghosts'
				   allow them to be revived by revive items or force powers during the deathcam

				   //MaintainBodyQueue(ent);  --futuza: might be useful here for this?
				*/
				
			}

		}
		else if (gDoSlowMoDuel)
		{
			client->respawnTime = level.time + 1000;
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );

	G_UpdateClientBroadcasts ( ent );

	//try some idle anims on ent if getting no input and not moving for some time
	G_CheckClientIdle( ent, ucmd );
}

/*
==================
G_CheckClientTimeouts

Checks whether a client has exceded any timeouts and act accordingly
==================
*/
void G_CheckClientTimeouts ( gentity_t *ent )
{
	// Only timeout supported right now is the timeout to spectator mode
	if ( !g_timeouttospec.integer )
	{
		return;
	}

	// Already a spectator, no need to boot them to spectator
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		return;
	}

	// See how long its been since a command was received by the client and if its 
	// longer than the timeout to spectator then force this client into spectator mode
	if ( level.time - ent->client->pers.cmd.serverTime > g_timeouttospec.integer * 1000 )
	{
		SetTeam ( ent, "spectator" );
	}
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum,usercmd_t *ucmd ) {
	gentity_t *ent;

	ent = g_entities + clientNum;

	if (clientNum < MAX_CLIENTS)
	{
		trap->GetUsercmd( clientNum, &ent->client->pers.cmd );
	}

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if (ucmd)
	{
		ent->client->pers.cmd = *ucmd;
	}

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		ClientThink_real( ent );
	}
	else if ( clientNum >= MAX_CLIENTS ) {
		ClientThink_real( ent );
	}
}


void G_RunClient( gentity_t *ent ) {
	if ( ent->client->lastCmdTime < level.time - 1000 ) {
     trap->GetUsercmd( ent-g_entities, &ent->client->pers.cmd );
 
     ent->client->lastCmdTime = level.time;
 
     // fill with seemingly valid data
     ent->client->pers.cmd.serverTime = level.time;
     ent->client->pers.cmd.buttons = 0;
     ent->client->pers.cmd.forwardmove = ent->client->pers.cmd.rightmove = ent->client->pers.cmd.upmove = 0;
 
     ClientThink_real( ent );
     return;
   }
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;

	if (ent->s.eType == ET_NPC)
	{
		assert(0);
		return;
	}

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		int		clientNum;//, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				//flags = (cl->mGameFlags & ~(PSG_VOTED | PSG_TEAMVOTED)) | (ent->client->mGameFlags & (PSG_VOTED | PSG_TEAMVOTED));
				//ent->client->mGameFlags = flags;
				unsigned int credits = ent->client->ps.credits;
				usercmd_t ucmd;

				ent->client->ps.eFlags = cl->ps.eFlags;
				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.credits = credits;

				// k, let's do some sort of stuffs
				trap->GetUsercmd(ent - g_entities, &ucmd);

				if(ucmd.buttons & BUTTON_IRONSIGHTS)
				{
					ent->client->ps.isInSights = qtrue;
				}
				else if(ucmd.buttons & BUTTON_SPRINT)
				{
					ent->client->ps.isSprinting = qtrue;
				}
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients, qtrue );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;
	qboolean isNPC = qfalse;
	usercmd_t clientcmd;
	int selectedItem;

	trap->GetUsercmd(ent->s.number, &clientcmd);

	if (ent->s.eType == ET_NPC)
	{
		isNPC = qtrue;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SpectatorClientEndFrame( ent );
		return;
	}

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ent->client->ps.powerups[ i ] < level.time ) {
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		if ( ent->s.number < MAX_CLIENTS )
		{//players do nothing in intermissions
			return;
		}
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	//Raz: add to ps instead of s
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->client->ps.eFlags |= EF_CONNECTION;
	}
	else
	{
		ent->client->ps.eFlags &= ~EF_CONNECTION;
	}

	// Jedi Knight Galaxies - Update ammo of current weapon
	selectedItem = clientcmd.invensel;
	if (selectedItem >= 0 && selectedItem < ent->inventory->size()) {
		weaponData_t* weaponData = GetWeaponData(ent->client->ps.weapon, ent->client->ps.weaponVariation);

		if (weaponData->numFiringModes > 0 && weaponData->firemodes[0].useQuantity) {
			// If useQuantity on the first firing mode is true, then we need to copy from the item stack's quantity
			itemInstance_t* item = &(*ent->inventory)[selectedItem];
			ent->client->ps.stats[STAT_TOTALAMMO] = item->quantity;
			ent->client->ps.stats[STAT_AMMO] = item->quantity;
		}
		else {
			// Use the regular ammo table instead
			ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
			if (weaponData->firemodes[ent->client->ps.firingMode].clipSize) {
				ent->client->ps.stats[STAT_AMMO] = 
					ent->client->clipammo[BG_GetWeaponIndex(ent->client->ps.weapon, ent->client->ps.weaponVariation)][ent->client->ps.firingMode];
			}
			else {
				ent->client->ps.stats[STAT_AMMO] = ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
			}
		}
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	// set the latest info
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qfalse );
		//rww - 12-03-02 - Don't snap the origin of players! It screws prediction all up.
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qfalse );
	}

	if (isNPC)
	{
		ent->s.eType = ET_NPC;
	}

	SendPendingPredictableEvents( &ent->client->ps );

	// set the bit for the reachability area the client is currently in
//	i = trap->AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}


