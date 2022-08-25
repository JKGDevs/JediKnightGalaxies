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

// g_combat.c

//#include "g_local.h"
#include "b_local.h"
#include "bg_saga.h"

// Include GLua
#include "../GLua/glua.h"

extern void G_LetGoOfWall( gentity_t *ent );
//rww - pd
void BotDamageNotification(gclient_t *bot, gentity_t *attacker);
//end rww

/*
 *	Remove the entity and fire off its targets.
 *	Only really used by turrets.
 */
void G_RemoveAndFireTargets (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	if(self->target)
	{
		G_UseTargets(self, attacker);
	}

	//remove my script_targetname
	G_FreeEntity( self );
}

/*
 *	Retrieves the hit location
 */
int G_GetHitLocation(gentity_t *target, vec3_t ppoint)
{
	vec3_t			point, point_dir;
	vec3_t			forward, right, up;
	vec3_t			tangles, tcenter;
	float			tradius;
	float			udot, fdot, rdot;
	int				Vertical, Forward, Lateral;
	int				HitLoc;

	// Get target forward, right and up.
	if(target->client)
	{
		// Ignore player's pitch and roll.
		VectorSet(tangles, 0, target->r.currentAngles[YAW], 0);
	}

	AngleVectors(tangles, forward, right, up);

	// Get center of target.
	VectorAdd(target->r.absmin, target->r.absmax, tcenter);
	VectorScale(tcenter, 0.5f, tcenter);

	// Get radius width of target.
	tradius = (fabs(target->r.maxs[0]) + fabs(target->r.maxs[1]) + fabs(target->r.mins[0]) + fabs(target->r.mins[1]))/4;

	// Get impact point.
	if(ppoint && !VectorCompare(ppoint, vec3_origin))
	{
		VectorCopy(ppoint, point);
	}
	else
	{
		return HL_NONE;
	}

	VectorSubtract(point, tcenter, point_dir);
	VectorNormalize(point_dir);

	// Get bottom to top (vertical) position index
	udot = DotProduct(up, point_dir);
	if(udot>.800)
	{
		Vertical = 4;
	}
	else if(udot>.400)
	{
		Vertical = 3;
	}
	else if(udot>-.333)
	{
		Vertical = 2;
	}
	else if(udot>-.666)
	{
		Vertical = 1;
	}
	else
	{
		Vertical = 0;
	}

	// Get back to front (forward) position index.
	fdot = DotProduct(forward, point_dir);
	if(fdot>.666)
	{
		Forward = 4;
	}
	else if(fdot>.333)
	{
		Forward = 3;
	}
	else if(fdot>-.333)
	{
		Forward = 2;
	}
	else if(fdot>-.666)
	{
		Forward = 1;
	}
	else
	{
		Forward = 0;
	}

	// Get left to right (lateral) position index.
	rdot = DotProduct(right, point_dir);
	if(rdot>.666)
	{
		Lateral = 4;
	}
	else if(rdot>.333)
	{
		Lateral = 3;
	}
	else if(rdot>-.333)
	{
		Lateral = 2;
	}
	else if(rdot>-.666)
	{
		Lateral = 1;
	}
	else
	{
		Lateral = 0;
	}

	HitLoc = Vertical * 25 + Forward * 5 + Lateral;

	if(HitLoc <= 10)
	{
		// Feet.
		if ( rdot > 0 )
		{
			return HL_FOOT_RT;
		}
		else
		{
			return HL_FOOT_LT;
		}
	}
	else if(HitLoc <= 50)
	{
		// Legs.
		if ( rdot > 0 )
		{
			return HL_LEG_RT;
		}
		else
		{
			return HL_LEG_LT;
		}
	}
	else if(HitLoc == 56||HitLoc == 60||HitLoc == 61||HitLoc == 65||HitLoc == 66||HitLoc == 70)
	{
		// Hands.
		if ( rdot > 0 )
		{
			return HL_HAND_RT;
		}
		else
		{
			return HL_HAND_LT;
		}
	}
	else if(HitLoc == 83||HitLoc == 87||HitLoc == 88||HitLoc == 92||HitLoc == 93||HitLoc == 97)
	{
		// Arms.
		if ( rdot > 0 )
		{
			return HL_ARM_RT;
		}
		else
		{
			return HL_ARM_LT;
		}
	}
	else if((HitLoc >= 107 && HitLoc <= 109)||(HitLoc >= 112 && HitLoc <= 114)||(HitLoc >= 117 && HitLoc <= 119))
	{
		// Head.
		return HL_HEAD;
	}
	else
	{
		if(udot < 0.3f)
		{
			return HL_WAIST;
		}
		else if(fdot < 0)
		{
			if(rdot > 0.4f)
			{
				return HL_BACK_RT;
			}
			else if(rdot < -0.4f)
			{
				return HL_BACK_LT;
			}
			else if(fdot < 0)
			{
				return HL_BACK;
			}
		}
		else
		{
			if(rdot > 0.3f)
			{
				return HL_CHEST_RT;
			}
			else if(rdot < -0.3f)
			{
				return HL_CHEST_LT;
			}
			else if(fdot < 0)
			{
				return HL_CHEST;
			}
		}
	}
	return HL_NONE;
}


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
===========
GenericPlum
===========
*/
void GenericPlum(gentity_t *ent, vec3_t origin, char *str, int color)
{
	//--futuza todo:
		/*
		make generalized plum system that will take:
		-entity producing plum
		-origin vector
		-a string (max length 16) containing text of plum
		-color to display string

		Use this later on for stuff like:
		"Dodge!" (in yellow)
		"9000 XP" (in purple)
		"Crit!" (in orange?)
		"VIP" (white)
		etc.

		Possibly color unneeded just use colorcodes embedded in string?  "^xf00Red Text"

		Function probably needs to be defined somewwhere more generically, as g_combat is pretty random, and it could be useful for non-combat.
		See: CG_ScorePlum()
		*/
	return;
}


/*
===========
DamagePlum
===========
*/
#include <numeric>
void DamagePlum( gentity_t *ent, vec3_t origin, int damage, int meansOfDeath, int shield, qboolean weak ) {
	meansOfDamage_t* means = JKG_GetMeansOfDamage(meansOfDeath);
	
	if (means->plums.noDamagePlums)
	{	// this means of death has no damage plums 
		return;
	}

	if (ent->damagePlumTime != level.time) {
		ent->damagePlum = G_TempEntity( origin, EV_DAMAGEPLUM );
		ent->damagePlumTime = level.time;
	}
	else //if it happens in the same frame
	{
		//its the same means
		if (meansOfDeath == ent->damagePlum->s.eventParm)
		{
			//stack up the damage
			if (ent->damagePlum->s.time)
				damage += ent->damagePlum->s.time;

			//stack up all the shield damage
			if (ent->damagePlum->s.generic1)
				shield += ent->damagePlum->s.generic1;
		}
	}
	ent->damagePlum->s.time = damage;
	ent->damagePlum->s.eventParm = meansOfDeath;
	ent->damagePlum->s.generic1 = shield;
	ent->damagePlum->s.groundEntityNum = weak; //note may be inaccurate as it just overwrites whatever the prior value was if it happens in the same frame
}

/*
===========
HealingPlum
===========
*/

void HealingPlum( gentity_t *ent, vec3_t origin, int amount ) {
	// Since this probably wont happen too often, we'll just make a new even for this
	gentity_t *plum;
	plum = G_TempEntity( origin, EV_DAMAGEPLUM );
	plum->s.time = amount;
	plum->s.eventParm = MOD_HEAL;

	if ( ent && ent->client && !ent->NPC )
	{
		ent->client->pers.partyUpdate = qtrue;
	}
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
extern qboolean g_dontPenalizeTeam; //g_cmds.c
void AddScore( gentity_t *ent, vec3_t origin, int score )
{
	if ( !ent->client ) {
		return;
	}

	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( level.gametype >= GT_TEAM && level.gametype != GT_CTF && !g_dontPenalizeTeam )
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;
	float		angle;
	int			i;
	gentity_t	*drop;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( weapon == WP_BRYAR_PISTOL) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING || self->client->ps.weaponstate == WEAPON_RELOADING ) {
		    int variation;
			if ( !BG_GetWeaponByIndex (self->client->pers.cmd.weapon, &weapon, &variation) )
			{
			    weapon = WP_NONE;
			}
		}
		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	if (self->client->noDrops) {
		weapon = WP_NONE;
	}

	self->s.bolt2 = weapon;

	if ( weapon >= WP_BRYAR_PISTOL && weapon != WP_EMPLACED_GUN && weapon != WP_TURRET )
	{
		gentity_t *te;
		gentity_t *droppedWeapon = NULL;

		// find the item type for this weapon
		item = BG_FindItemForWeapon( (weapon_t)weapon );

		// tell all clients to remove the weapon model on this guy until he respawns
		te = G_TempEntity( vec3_origin, EV_DESTROY_WEAPON_MODEL );
		te->r.svFlags |= SVF_BROADCAST;
		te->s.eventParm = self->s.number;

		// spawn the item
		droppedWeapon = Drop_Item( self, item, 0 );
		droppedWeapon->s.weaponVariation = self->s.weaponVariation;
	}

	// drop all the powerups if not in teamplay
	if ( level.gametype != GT_TEAM ) {
		angle = 45;
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if ( self->client->ps.powerups[ i ] > level.time ) {
				item = BG_FindItemForPowerup( (powerup_t)i );
				if ( !item ) {
					continue;
				}
				drop = Drop_Item( self, item, angle );
				// decide how many seconds it has left
				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}
				angle += 45;
			}
		}
	}
}

/*
==================
G_RemoveCorpse

Removes a corpse.
==================
*/

void G_RemoveCorpse(gentity_t *ent)
{
	trap->UnlinkEntity( (sharedEntity_t *)ent );
	ent->physicsObject = qfalse;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	// NOTENOTE No gibbing right now, this is star wars.
	qboolean doDisint = qfalse;

	if (self->s.eType == ET_NPC)
	{ //well, just rem it then, so long as it's done with its death anim and it's not a standard weapon.
		if ( self->client && self->client->ps.torsoTimer <= 0 &&
			 (meansOfDeath == MOD_UNKNOWN ||
			  meansOfDeath == MOD_WATER ||
			  meansOfDeath == MOD_SLIME ||
			  meansOfDeath == MOD_LAVA ||
			  meansOfDeath == MOD_CRUSH ||
			  meansOfDeath == MOD_TELEFRAG ||
			  meansOfDeath == MOD_FALLING ||
			  meansOfDeath == MOD_SUICIDE ||
			  meansOfDeath == MOD_TRIGGER_HURT) )
		{
			self->think = G_FreeEntity;
			self->nextthink = level.time;
		}
		return;
	}

	if (self->health < (GIB_HEALTH+1))
	{
		self->health = GIB_HEALTH+1;

		if (self->client && (level.time - self->client->respawnTime) < 2000)
		{
			doDisint = qfalse;
		}
		else
		{
			doDisint = qtrue;
		}
	}

	if (self->client && (self->client->ps.eFlags & EF_DISINTEGRATION))
	{
		return;
	}
	else if (self->s.eFlags & EF_DISINTEGRATION)
	{
		return;
	}

	// JKG - Disintegration suppressor
	if (self->client && self->client->noDisintegrate) {
		doDisint = qfalse;
	}

	if (doDisint)
	{
		if (self->client)
		{
			self->client->ps.eFlags |= EF_DISINTEGRATION;
			VectorCopy(self->client->ps.origin, self->client->ps.lastHitLoc);
		}
		else
		{
			self->s.eFlags |= EF_DISINTEGRATION;
			VectorCopy(self->r.currentOrigin, self->s.origin2);

			//since it's the corpse entity, tell it to "remove" itself
			self->think = G_RemoveCorpse;
			self->nextthink = level.time + 1000;
		}
		return;
	}
}

/*
 *	Checks to see if we should use a special death animation, and returns it.
 */
static int G_CheckSpecialDeathAnim( gentity_t *self, vec3_t point, int damage, int mod, int hitLoc )
{
	int deathAnim = -1;

	if ( BG_InRoll( &self->client->ps, self->client->ps.legsAnim ) )
	{
		deathAnim = BOTH_DEATH_ROLL;		//# Death anim from a roll
	}
	else if ( BG_FlippingAnim( self->client->ps.legsAnim ) )
	{
		deathAnim = BOTH_DEATH_FLIP;		//# Death anim from a flip
	}
	else if ( BG_InKnockDownOnGround( &self->client->ps ) )
	{//since these happen a lot, let's handle them case by case
		int animLength = bgAllAnims[self->localAnimIndex].anims[self->client->ps.legsAnim].numFrames * fabs((float)(bgHumanoidAnimations[self->client->ps.legsAnim].frameLerp));
		switch ( self->client->ps.legsAnim )
		{
		case BOTH_KNOCKDOWN1:
			if ( animLength - self->client->ps.legsTimer > 100 )
			{//on our way down
				if ( self->client->ps.legsTimer > 600 )
				{//still partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_KNOCKDOWN2:
			if ( animLength - self->client->ps.legsTimer > 700 )
			{//on our way down
				if ( self->client->ps.legsTimer > 600 )
				{//still partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_KNOCKDOWN3:
			if ( animLength - self->client->ps.legsTimer > 100 )
			{//on our way down
				if ( self->client->ps.legsTimer > 1300 )
				{//still partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		case BOTH_KNOCKDOWN4:
			if ( animLength - self->client->ps.legsTimer > 300 )
			{//on our way down
				if ( self->client->ps.legsTimer > 350 )
				{//still partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			else
			{//crouch death
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			break;
		case BOTH_KNOCKDOWN5:
			if ( self->client->ps.legsTimer < 750 )
			{//flat
				deathAnim = BOTH_DEATH_LYING_DN;
			}
			break;
		case BOTH_GETUP1:
			if ( self->client->ps.legsTimer < 350 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 800 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );
				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 450 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_GETUP2:
			if ( self->client->ps.legsTimer < 150 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 850 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 500 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_GETUP3:
			if ( self->client->ps.legsTimer < 250 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 600 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;
				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 150 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		case BOTH_GETUP4:
			if ( self->client->ps.legsTimer < 250 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 600 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 850 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_GETUP5:
			if ( self->client->ps.legsTimer > 850 )
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 1500 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		case BOTH_GETUP_CROUCH_B1:
			if ( self->client->ps.legsTimer < 800 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 400 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_GETUP_CROUCH_F1:
			if ( self->client->ps.legsTimer < 800 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 150 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		case BOTH_FORCE_GETUP_B1:
			if ( self->client->ps.legsTimer < 325 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 725 )
			{//spinning up
				deathAnim = BOTH_DEATH_SPIN_180;	//# Death anim when facing backwards
			}
			else if ( self->client->ps.legsTimer < 900 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 50 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_FORCE_GETUP_B2:
			if ( self->client->ps.legsTimer < 575 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 875 )
			{//spinning up
				deathAnim = BOTH_DEATH_SPIN_180;	//# Death anim when facing backwards
			}
			else if ( self->client->ps.legsTimer < 900 )
			{//crouching
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else
			{//lying down
				//partially up
				deathAnim = BOTH_DEATH_FALLING_UP;
			}
			break;
		case BOTH_FORCE_GETUP_B3:
			if ( self->client->ps.legsTimer < 150 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 775 )
			{//flipping
				deathAnim = BOTH_DEATHBACKWARD2; //backflip
			}
			else
			{//lying down
				//partially up
				deathAnim = BOTH_DEATH_FALLING_UP;
			}
			break;
		case BOTH_FORCE_GETUP_B4:
			if ( self->client->ps.legsTimer < 325 )
			{//standing up
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 150 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_FORCE_GETUP_B5:
			if ( self->client->ps.legsTimer < 550 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 1025 )
			{//kicking up
				deathAnim = BOTH_DEATHBACKWARD2; //backflip
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 50 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_FORCE_GETUP_B6:
			if ( self->client->ps.legsTimer < 225 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 425 )
			{//crouching up
				vec3_t fwd;
				float thrown = 0;

				AngleVectors( self->client->ps.viewangles, fwd, NULL, NULL );
				thrown = DotProduct( fwd, self->client->ps.velocity );

				if ( thrown < -150 )
				{
					deathAnim = BOTH_DEATHBACKWARD1;	//# Death anim when crouched and thrown back
				}
				else
				{
					deathAnim = BOTH_DEATH_CROUCHED;	//# Death anim when crouched
				}
			}
			else if ( self->client->ps.legsTimer < 825 )
			{//flipping up
				deathAnim = BOTH_DEATHFORWARD3; //backflip
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 225 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_UP;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_UP;
				}
			}
			break;
		case BOTH_FORCE_GETUP_F1:
			if ( self->client->ps.legsTimer < 275 )
			{//standing up
			}
			else if ( self->client->ps.legsTimer < 750 )
			{//flipping
				deathAnim = BOTH_DEATH14;
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 100 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		case BOTH_FORCE_GETUP_F2:
			if ( self->client->ps.legsTimer < 1200 )
			{//standing
			}
			else
			{//lying down
				if ( animLength - self->client->ps.legsTimer > 225 )
				{//partially up
					deathAnim = BOTH_DEATH_FALLING_DN;
				}
				else
				{//down
					deathAnim = BOTH_DEATH_LYING_DN;
				}
			}
			break;
		}
	}

	return deathAnim;
}

/*
 *	Chooses a death animation based on where the target was hit.
 */
int G_PickDeathAnim( gentity_t *self, vec3_t point, int damage, int mod, int hitLoc )
{//FIXME: play dead flop anims on body if in an appropriate _DEAD anim when this func is called
	int deathAnim = -1;
	int max_health;
	int legAnim = 0;
	vec3_t objVelocity;

	if (!self || !self->client)
	{
		if (!self || self->s.eType != ET_NPC)
		{ //g2animent
			return 0;
		}
	}

	if (self->client)
	{
		max_health = self->client->ps.stats[STAT_MAX_HEALTH];
	}
	else
	{
		max_health = 60;
	}

	if (self->client)
	{
		VectorCopy(self->client->ps.velocity, objVelocity);
	}
	else
	{
		VectorCopy(self->s.pos.trDelta, objVelocity);
	}

	if ( hitLoc == HL_NONE )
	{
		hitLoc = G_GetHitLocation( self, point );//self->hitLoc
	}

	if (self->client)
	{
		legAnim = self->client->ps.legsAnim;
	}
	else
	{
		legAnim = self->s.legsAnim;
	}

	if (gGAvoidDismember)
	{
		return BOTH_RIGHTHANDCHOPPEDOFF;
	}

	//dead flops
	switch( legAnim )
	{
	case BOTH_DEATH1:		//# First Death anim
	case BOTH_DEAD1:
	case BOTH_DEATH2:			//# Second Death anim
	case BOTH_DEAD2:
	case BOTH_DEATH8:			//# 
	case BOTH_DEAD8:
	case BOTH_DEATH13:			//# 
	case BOTH_DEAD13:
	case BOTH_DEATH14:			//# 
	case BOTH_DEAD14:
	case BOTH_DEATH16:			//# 
	case BOTH_DEAD16:
	case BOTH_DEADBACKWARD1:		//# First thrown backward death finished pose
	case BOTH_DEADBACKWARD2:		//# Second thrown backward death finished pose
	case BOTH_DEATH10:			//# 
	case BOTH_DEAD10:
	case BOTH_DEATH15:			//# 
	case BOTH_DEAD15:
	case BOTH_DEADFORWARD1:		//# First thrown forward death finished pose
	case BOTH_DEADFORWARD2:		//# Second thrown forward death finished pose
	case BOTH_DEADFLOP1:
		deathAnim = -2;
		break;
	case BOTH_DEAD3:				//# Third Death finished pose
	case BOTH_DEAD4:				//# Fourth Death finished pose
	case BOTH_DEAD5:				//# Fifth Death finished pose
	case BOTH_DEAD6:				//# Sixth Death finished pose
	case BOTH_DEAD7:				//# Seventh Death finished pose
	case BOTH_DEAD9:				//# 
	case BOTH_DEAD11:			//#
	case BOTH_DEAD12:			//# 
	case BOTH_DEAD17:			//# 
	case BOTH_DEAD18:			//# 
	case BOTH_DEAD19:			//# 
	case BOTH_LYINGDEAD1:		//# Killed lying down death finished pose
	case BOTH_STUMBLEDEAD1:		//# Stumble forward death finished pose
	case BOTH_FALLDEAD1LAND:		//# Fall forward and splat death finished pose
	case BOTH_DEATH3:			//# Third Death anim
	case BOTH_DEATH4:			//# Fourth Death anim
	case BOTH_DEATH5:			//# Fifth Death anim
	case BOTH_DEATH6:			//# Sixth Death anim
	case BOTH_DEATH7:			//# Seventh Death anim
	case BOTH_DEATH9:			//# 
	case BOTH_DEATH11:			//#
	case BOTH_DEATH12:			//# 
	case BOTH_DEATH17:			//# 
	case BOTH_DEATH18:			//# 
	case BOTH_DEATH19:			//# 
	case BOTH_DEATHFORWARD1:		//# First Death in which they get thrown forward
	case BOTH_DEATHFORWARD2:		//# Second Death in which they get thrown forward
	case BOTH_DEATHBACKWARD1:	//# First Death in which they get thrown backward
	case BOTH_DEATHBACKWARD2:	//# Second Death in which they get thrown backward
	case BOTH_DEATH1IDLE:		//# Idle while close to death
	case BOTH_LYINGDEATH1:		//# Death to play when killed lying down
	case BOTH_STUMBLEDEATH1:		//# Stumble forward and fall face first death
	case BOTH_FALLDEATH1:		//# Fall forward off a high cliff and splat death - start
	case BOTH_FALLDEATH1INAIR:	//# Fall forward off a high cliff and splat death - loop
	case BOTH_FALLDEATH1LAND:	//# Fall forward off a high cliff and splat death - hit bottom
		deathAnim = -2;
		break;
	}
	if ( deathAnim == -1 )
	{
		if (self->client)
		{
			deathAnim = G_CheckSpecialDeathAnim( self, point, damage, mod, hitLoc );
		}

		if (deathAnim == -1)
		{
			//death anims
			switch( hitLoc )
			{
			case HL_FOOT_RT:
			case HL_FOOT_LT:
				if ( mod == MOD_SABER && !Q_irand( 0, 2 ) )
				{
					return BOTH_DEATH10;//chest: back flip
				}
				else if ( !Q_irand( 0, 2 ) )
				{
					deathAnim = BOTH_DEATH4;//back: forward
				}
				else if ( !Q_irand( 0, 1 ) )
				{
					deathAnim = BOTH_DEATH5;//same as 4
				}
				else
				{
					deathAnim = BOTH_DEATH15;//back: forward
				}
				break;
			case HL_LEG_RT:
				if ( !Q_irand( 0, 2 ) )
				{
					deathAnim = BOTH_DEATH4;//back: forward
				}
				else if ( !Q_irand( 0, 1 ) )
				{
					deathAnim = BOTH_DEATH5;//same as 4
				}
				else
				{
					deathAnim = BOTH_DEATH15;//back: forward
				}
				break;
			case HL_LEG_LT:
				if ( !Q_irand( 0, 2 ) )
				{
					deathAnim = BOTH_DEATH4;//back: forward
				}
				else if ( !Q_irand( 0, 1 ) )
				{
					deathAnim = BOTH_DEATH5;//same as 4
				}
				else
				{
					deathAnim = BOTH_DEATH15;//back: forward
				}
				break;
			case HL_BACK:
				if ( !VectorLengthSquared( objVelocity ) )
				{
					deathAnim = BOTH_DEATH17;//head/back: croak
				}
				else
				{
					if ( !Q_irand( 0, 2 ) )
					{
						deathAnim = BOTH_DEATH4;//back: forward
					}
					else if ( !Q_irand( 0, 1 ) )
					{
						deathAnim = BOTH_DEATH5;//same as 4
					}
					else
					{
						deathAnim = BOTH_DEATH15;//back: forward
					}
				}
				break;
			case HL_CHEST_RT:
			case HL_ARM_RT:
			case HL_HAND_RT:
			case HL_BACK_RT:
				if ( damage <= max_health*0.25f )
				{
					deathAnim = BOTH_DEATH9;//chest right: snap, fall forward
				}
				else if ( damage <= max_health*0.5f )
				{
					deathAnim = BOTH_DEATH3;//chest right: back
				}
				else if ( damage <= max_health*0.75f )
				{
					deathAnim = BOTH_DEATH6;//chest right: spin
				}
				else 
				{
					//TEMP HACK: play spinny deaths less often
					if ( Q_irand( 0, 1 ) )
					{
						deathAnim = BOTH_DEATH8;//chest right: spin high
					}
					else
					{
						switch ( Q_irand( 0, 2 ) )
						{
						default:
						case 0:
							deathAnim = BOTH_DEATH9;//chest right: snap, fall forward
							break;
						case 1:
							deathAnim = BOTH_DEATH3;//chest right: back
							break;
						case 2:
							deathAnim = BOTH_DEATH6;//chest right: spin
							break;
						}
					}
				}
				break;
			case HL_CHEST_LT:
			case HL_ARM_LT:
			case HL_HAND_LT:
			case HL_BACK_LT:
				if ( damage <= max_health*0.25f )
				{
					deathAnim = BOTH_DEATH11;//chest left: snap, fall forward
				}
				else if ( damage <= max_health*0.5f )
				{
					deathAnim = BOTH_DEATH7;//chest left: back
				}
				else if ( damage <= max_health*0.75f )
				{
					deathAnim = BOTH_DEATH12;//chest left: spin
				}
				else
				{
					//TEMP HACK: play spinny deaths less often
					if ( Q_irand( 0, 1 ) )
					{
						deathAnim = BOTH_DEATH14;//chest left: spin high
					}
					else
					{
						switch ( Q_irand( 0, 2 ) )
						{
						default:
						case 0:
							deathAnim = BOTH_DEATH11;//chest left: snap, fall forward
							break;
						case 1:
							deathAnim = BOTH_DEATH7;//chest left: back
							break;
						case 2:
							deathAnim = BOTH_DEATH12;//chest left: spin
							break;
						}
					}
				}
				break;
			case HL_CHEST:
			case HL_WAIST:
				if ( damage <= max_health*0.25f || !VectorLengthSquared( objVelocity ) )
				{
					if ( !Q_irand( 0, 1 ) )
					{
						deathAnim = BOTH_DEATH18;//gut: fall right
					}
					else
					{
						deathAnim = BOTH_DEATH19;//gut: fall left
					}
				}
				else if ( damage <= max_health*0.5f )
				{
					deathAnim = BOTH_DEATH2;//chest: backward short
				}
				else if ( damage <= max_health*0.75f )
				{
					if ( !Q_irand( 0, 1 ) )
					{
						deathAnim = BOTH_DEATH1;//chest: backward med
					}
					else
					{
						deathAnim = BOTH_DEATH16;//same as 1
					}
				}
				else
				{
					deathAnim = BOTH_DEATH10;//chest: back flip
				}
				break;
			case HL_HEAD:
				if ( damage <= max_health*0.5f )
				{
					deathAnim = BOTH_DEATH17;//head/back: croak
				}
				else
				{
					deathAnim = BOTH_DEATH13;//head: stumble, fall back
				}
				break;
			default:
				break;
			}
		}
	}

	// Validate.....
	if ( deathAnim == -1 || !BG_HasAnimation( self->localAnimIndex, deathAnim ))
	{
		// I guess we'll take what we can get.....
		deathAnim = BG_PickAnim( self->localAnimIndex, BOTH_DEATH1, BOTH_DEATH25 );
	}

	return deathAnim;
}

/*
-------------------------
G_AlertTeam
AI stuff, should be moved out of here
-------------------------
*/

void G_AlertTeam( gentity_t *victim, gentity_t *attacker, float radius, float soundDist )
{
	int			radiusEnts[ 128 ];
	gentity_t	*check;
	vec3_t		mins, maxs;
	int			numEnts;
	int			i;
	float		distSq, sndDistSq = (soundDist*soundDist);

	if ( attacker == NULL || attacker->client == NULL )
		return;

	//Setup the bbox to search in
	for ( i = 0; i < 3; i++ )
	{
		mins[i] = victim->r.currentOrigin[i] - radius;
		maxs[i] = victim->r.currentOrigin[i] + radius;
	}

	//Get the number of entities in a given space
	numEnts = trap->EntitiesInBox( mins, maxs, radiusEnts, 128 );

	//Cull this list
	for ( i = 0; i < numEnts; i++ )
	{
		check = &g_entities[radiusEnts[i]];

		//Validate clients
		if ( check->client == NULL )
			continue;

		//only want NPCs
		if ( check->NPC == NULL )
			continue;

		//Don't bother if they're ignoring enemies
//		if ( check->svFlags & SVF_IGNORE_ENEMIES )
//			continue;

		//This NPC specifically flagged to ignore alerts
		if ( check->NPC->scriptFlags & SCF_IGNORE_ALERTS )
			continue;

		//This NPC specifically flagged to ignore alerts
		if ( !(check->NPC->scriptFlags&SCF_LOOK_FOR_ENEMIES) )
			continue;

		//this ent does not participate in group AI
		if ( (check->NPC->scriptFlags&SCF_NO_GROUPS) )
			continue;

		//Skip the requested avoid check if present
		if ( check == victim )
			continue;

		//Skip the attacker
		if ( check == attacker )
			continue;

		//Must be on the same team
		if ( check->client->playerTeam != victim->client->playerTeam )
			continue;

		//Must be alive
		if ( check->health <= 0 )
			continue;

		if ( check->enemy == NULL )
		{//only do this if they're not already mad at someone
			distSq = DistanceSquared( check->r.currentOrigin, victim->r.currentOrigin );
			if ( distSq > 16384 /*128 squared*/ && !trap->InPVS( victim->r.currentOrigin, check->r.currentOrigin ) )
			{//not even potentially visible/hearable
				continue;
			}
			//NOTE: this allows sound alerts to still go through doors/PVS if the teammate is within 128 of the victim...
			if ( soundDist <= 0 || distSq > sndDistSq )
			{//out of sound range
				if ( !InFOV( victim, check, check->NPC->stats.hfov, check->NPC->stats.vfov ) 
					||  !NPC_ClearLOS2( check, victim->r.currentOrigin ) )
				{//out of FOV or no LOS
					continue;
				}
			}

			//FIXME: This can have a nasty cascading effect if setup wrong...
			G_SetEnemy( check, attacker );
		}
	}
}

/*
-------------------------
G_DeathAlert
AI stuff, should be moved out of here
-------------------------
*/

#define	DEATH_ALERT_RADIUS			512
#define	DEATH_ALERT_SOUND_RADIUS	512

void G_DeathAlert( gentity_t *victim, gentity_t *attacker )
{//FIXME: with all the other alert stuff, do we really need this?
	G_AlertTeam( victim, attacker, DEATH_ALERT_RADIUS, DEATH_ALERT_SOUND_RADIUS );
}

/*
-------------------------
G_CheckVictoryScript
AI stuff, should be moved out of here
-------------------------
*/
void GLua_NPCEV_OnVictory(gentity_t *self);
void G_CheckVictoryScript(gentity_t *self)
{

	if (self->NPC && self->client && self->NPC->isLuaNPC) {
		GLua_NPCEV_OnVictory(self);
	}
	if ( !G_ActivateBehavior( self, BSET_VICTORY ) )
	{
		if ( self->NPC && self->s.weapon == WP_SABER )
		{//Jedi taunt from within their AI
			self->NPC->blockedSpeechDebounceTime = 0;//get them ready to taunt
			return;
		}
		if ( self->client && self->client->NPC_class == CLASS_GALAKMECH )
		{
			self->wait = 1;
			TIMER_Set( self, "gloatTime", Q_irand( 5000, 8000 ) );
			self->NPC->blockedSpeechDebounceTime = 0;//get him ready to taunt
			return;
		}
		//FIXME: any way to not say this *right away*?  Wait for victim's death anim/scream to finish?
		if ( self->NPC && self->NPC->group && self->NPC->group->commander && self->NPC->group->commander->NPC && self->NPC->group->commander->NPC->rank > self->NPC->rank && !Q_irand( 0, 2 ) )
		{//sometimes have the group commander speak instead
			self->NPC->group->commander->NPC->greetingDebounceTime = level.time + Q_irand( 2000, 5000 );
			//G_AddVoiceEvent( self->NPC->group->commander, Q_irand(EV_VICTORY1, EV_VICTORY3), 2000 );
		}
		else if ( self->NPC )
		{
			self->NPC->greetingDebounceTime = level.time + Q_irand( 2000, 5000 );
			//G_AddVoiceEvent( self, Q_irand(EV_VICTORY1, EV_VICTORY3), 2000 );
		}
	}
}

void G_AddPowerDuelScore(int team, int score)
{
	int i = 0;
	gentity_t *check;

	while (i < MAX_CLIENTS)
	{
		check = &g_entities[i];
		if (check->inuse && check->client &&
			check->client->pers.connected == CON_CONNECTED && !check->client->iAmALoser &&
			check->client->ps.stats[STAT_HEALTH] > 0 &&
			check->client->sess.sessionTeam != TEAM_SPECTATOR &&
			check->client->sess.duelTeam == team)
		{ //found a living client on the specified team
			check->client->sess.wins += score;
			ClientUserinfoChanged(check->s.number);
		}
		i++;
	}
}

void G_AddPowerDuelLoserScore(int team, int score)
{
	int i = 0;
	gentity_t *check;

	while (i < MAX_CLIENTS)
	{
		check = &g_entities[i];
		if (check->inuse && check->client &&
			check->client->pers.connected == CON_CONNECTED &&
			(check->client->iAmALoser || (check->client->ps.stats[STAT_HEALTH] <= 0 && check->client->sess.sessionTeam != TEAM_SPECTATOR)) &&
			check->client->sess.duelTeam == team)
		{ //found a living client on the specified team
			check->client->sess.losses += score;
			ClientUserinfoChanged(check->s.number);
		}
		i++;
	}
}

/*
==================
JKG_CanAwardAssist

Checks if an individual hit record can award an assist.
==================
*/
qboolean JKG_CanAwardAssist(gentity_t* deadEnt, gentity_t* killer, entityHitRecord_t hitRecord) {
	if (hitRecord.entWhoHit == nullptr || deadEnt == nullptr || killer == nullptr) {
		return qfalse;
	}

	// Non-players can't be awarded assists
	if (hitRecord.entWhoHit - g_entities >= MAX_CLIENTS) {
		return qfalse;
	}

	// Not allowed an assist when we are on the same team...
	if (OnSameTeam(deadEnt, hitRecord.entWhoHit)) {
		return qfalse;
	}

	// ...or are ourself
	if (deadEnt == hitRecord.entWhoHit) {
		return qfalse;
	}

	// Also don't give us an assist if we haven't actually "assisted"
	if (hitRecord.timeHit + ASSIST_LAST_TIME < level.time) {
		return qfalse;
	}

	// Don't give us an assist if we were the killer!
	if (killer == hitRecord.entWhoHit) {
		return qfalse;
	}

	return qtrue;
}

/*
==================
JKG_CanAwardBounty

==================
*/
qboolean JKG_CanAwardBounty(gentity_t* dead, gentity_t* attacker) {
	if (!jkg_bounty.integer || !jkg_killsPerBounty.integer) {
		// Don't award bounties when the cvar is turned off.
		return qfalse;
	}

	if (dead == attacker) {
		// Can't award bounty to self.
		return qfalse;
	}

	if (!dead->client || !attacker->client || attacker->s.number >= MAX_CLIENTS) {
		// Can't award bounty when the dead is not a client, or the attacker is not a player
		return qfalse;
	}

	if (OnSameTeam(dead, attacker)) {
		// Can't award bounty when the attacker and dead are on the same team.
		return qfalse;
	}

	return qtrue;
}

/*
===========================
JKG_HandleUnclaimedBounties

===========================
*/
qboolean JKG_HandleUnclaimedBounties(gentity_t* deadguy)
{
	int multiplier = (deadguy->client->numKillsThisLife > jkg_maxKillStreakBounty.integer) ? jkg_maxKillStreakBounty.integer : deadguy->client->numKillsThisLife;
	gentity_t* player; int reward = jkg_bounty.integer*multiplier;	//set default reward as jkg_bounty
	int team_amt{ 0 };	//# of players on the team to reward

	int teamToReward = deadguy->client->sess.sessionTeam;	//get dead guy's team
	if (teamToReward == TEAM_RED)	//if he was red, blue team gets reward
		teamToReward = TEAM_BLUE;
	else if (teamToReward == TEAM_BLUE)	//if he was blue, red team gets reward
		teamToReward = TEAM_RED;
	else
		teamToReward = -1;	//if he was neither then no reward

	for (int i = 0; i < sv_maxclients.integer; i++)
	{
		player = &g_entities[i];
		if (!player->inuse || (player - g_entities >= MAX_CLIENTS) || player == nullptr || player->client == nullptr || player->client->sess.sessionTeam != teamToReward)
			continue;

		if(player->client->sess.sessionTeam == teamToReward)
			team_amt++;
	}

	if (team_amt < 1)	//nobody there
		return false;

	//calculate team reward split
	reward = (reward / team_amt);																//equally distribute reward among team
	reward = (reward < jkg_teamKillBonus.integer) ? jkg_teamKillBonus.integer : reward;			//unless its less than teamKillBonus
	if (team_amt == 1)																			//if only one player, don't give him the whole reward since its not a direct kill
		reward = reward * 0.5;

	for (int i = 0; i < sv_maxclients.integer; i++)
	{
		player = &g_entities[i];
		if (!player->inuse || (player - g_entities >= MAX_CLIENTS) || player == nullptr || player->client == nullptr)	//don't reward spectators, nonclients, etc
			continue;

		//if we're not on deadguy's team, we deserve a reward!
		if (player->client->sess.sessionTeam == teamToReward)
		{
			trap->SendServerCommand(player->s.number, va("notify 1 \"Team Bounty Claimed: +%i Credits\"", reward));
			player->client->ps.credits += reward;
			trap->SendServerCommand(player->s.number, "hitmarker");
		}
	}
	return true;
}

/*
==================
player_die
==================
*/
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

extern void AI_DeleteSelfFromGroup( gentity_t *self );
extern void AI_GroupMemberKilled( gentity_t *self );
extern qboolean NPC_Humanoid_WaitingAmbush( gentity_t *self );
void CheckExitRules( void );
extern void Rancor_DropVictim( gentity_t *self );

extern qboolean g_dontFrickinCheck;
extern qboolean g_endPDuel;
extern qboolean g_noPDuelCheck;
void G_CheckForBlowingUp (gentity_t *ent, gentity_t *enemy, vec3_t point, int damage, int deathAnim, qboolean postDeath);
void GLua_NPCEV_OnDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;
	int			sPMType = 0;
	meansOfDamage_t* means = nullptr;
	usercmd_t	cmd;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	if ( !attacker )
		return;
		
	if (self->s.eType == ET_PLAYER && self->s.clientNum > MAX_CLIENTS)
		return; // UQ1: Secondary entity???

	if (self->s.eType == ET_PLAYER)
	{
		trap->GetUsercmd(self->client->ps.clientNum, &cmd);
	}

	//check player stuff
	g_dontFrickinCheck = qfalse;

	if (level.gametype == GT_POWERDUEL)
	{ //don't want to wait til later in the frame if this is the case
		CheckExitRules();

		if ( level.intermissiontime )
		{
			return;
		}
	}

	self->client->ps.emplacedIndex = 0;
	// JKG (Xy): Reset these to qfalse. Not sure if you want to keep these while you're dead
	// but it doesn't seem like you would.
	self->client->pmfreeze = qfalse;
	self->client->pmlock = qfalse;
	self->client->pmnomove = qfalse;

	// unfreeze the torso and legs animation so that we can play the death animation
	// this will look a bit weird on the Cryoban buff but it's pretty rare that players die anyway from that
	self->client->ps.freezeLegsAnim = 0;
	self->client->ps.freezeTorsoAnim = 0;
	self->s.freezeLegsAnim = 0;
	self->s.freezeTorsoAnim = 0;

	if (meansOfDeath != -1) {
		means = JKG_GetMeansOfDamage(meansOfDeath);

		if (means->disintegrate)
		{
			// do disintegration
			self->client->ps.eFlags |= EF_DISINTEGRATION;
			self->s.eFlags |= EF_DISINTEGRATION;
			self->r.contents = 0;
			VectorClear(self->client->ps.lastHitLoc);
			VectorClear(self->client->ps.velocity);
		}
	}

	// K, let's see if we can raise the killstreak on the attacker
	if(JKG_CanAwardBounty(self, attacker))
	{
		attacker->client->numKillsThisLife++;
	}

	// JKG: Give credits for each kill
	if(attacker->s.number < MAX_CLIENTS && ( self->s.number < MAX_CLIENTS || self->s.eType == ET_NPC))
	{
		// TODO: Divide equally amongst party (once new party interface is done)
		if(!OnSameTeam(attacker, self) && attacker != self)
		{
			int credits;
			// ITS A FEATURE NOT A BUG: use team capture credits for CTF instead of kills, so objectives are more important
			if (g_gametype.integer == GT_CTF) {
				credits = jkg_creditsPerTeamCapture.integer;
			}
			else {
				credits = jkg_creditsPerKill.integer;
			}

			int multiplier = 0;
			self->client->numKillsThisLife > jkg_maxKillStreakBounty.integer ? multiplier = jkg_maxKillStreakBounty.integer : multiplier = self->client->numKillsThisLife;
			
			if(jkg_maxKillStreakBounty.integer < jkg_killsPerBounty.integer)
				Com_Printf("Warning jkg_maxKillStreakBounty < jkg_killsPerBounty, bounties will never be rewarded!\n");

			int bounty = (self->client->numKillsThisLife >= jkg_killsPerBounty.integer) ? multiplier*jkg_bounty.integer : 0;
			attacker->client->ps.credits += (credits + bounty);
			if(bounty > 0)
			{
				trap->SendServerCommand(attacker-g_entities, va("notify 1 \"Kill: +%i Credits, +%i Bounty\"", credits, bounty));
			}
			else
			{
				trap->SendServerCommand(attacker-g_entities, va("notify 1 \"Kill: +%i Credits\"", credits));
			}

		}
	}
	if(JKG_CanAwardBounty(self, attacker))
	{
		if(self->client->numKillsThisLife >= jkg_killsPerBounty.integer)
		{
			trap->SendServerCommand(-1, va("chat 100 \"%s^7's bounty was claimed by %s.\"", self->client->pers.netname, attacker->client->pers.netname));
		}
		if(attacker->client->numKillsThisLife == jkg_killsPerBounty.integer)
		{
			trap->SendServerCommand(-1, va("chat 100 \"%s ^7has a bounty on their head!\"", attacker->client->pers.netname));
		}
		self->client->numKillsThisLife = 0;
	}
	else if (self->client->numKillsThisLife >= jkg_killsPerBounty.integer)
	{

		if (JKG_HandleUnclaimedBounties(self))	//splits bounty amongst team
		{
			self->client->numKillsThisLife = 0;
			trap->SendServerCommand(-1, va("chat 100 \"%s^7's bounty was claimed by the opposing team.\"", self->client->pers.netname));
		}
		else
			trap->SendServerCommand(-1, va("chat 100 \"%s" S_COLOR_WHITE "'s %i point bounty remains unclaimed.\"", self->client->pers.netname, self->client->numKillsThisLife));
	}

	if(self->s.number < MAX_CLIENTS)
	{
		if (self->assists) {
			for (auto it = self->assists->begin(); it != self->assists->end(); ++it) {
				int awardedCredits;
				int creditsPerKill;

				if (!JKG_CanAwardAssist(self, attacker, *it)) {
					continue;
				}

				if (g_gametype.integer == GT_CTF) {
					creditsPerKill = jkg_creditsPerTeamCapture.integer;
				}
				else {
					creditsPerKill = jkg_creditsPerKill.integer;
				}

				// One point of damage is worth 1% of total credits earned per kill
				awardedCredits = (creditsPerKill / 100.0f) * it->damageDealt;

				// Don't trigger it if we don't have any credits to award for this assist
				if (awardedCredits > 0) {
					int minAssistReward = (creditsPerKill * ( (51 <= jkg_minAssistAwardRatio.integer ? 50 : jkg_minAssistAwardRatio.integer) / 100.0f));		//the minimum bonus added to assist credits, awardRatio cannot exceed 50% of creditsPerKill

					if (minAssistReward)	//only award if greater than 0
						awardedCredits = ((awardedCredits < minAssistReward) ? minAssistReward : awardedCredits);

					if (awardedCredits >= creditsPerKill) {
						// Always award less than a full kill's worth of credits.
						awardedCredits = creditsPerKill - 1;
					}

					it->entWhoHit->client->ps.credits += awardedCredits;

					// Do a hitmarker, as a hint that they got a reward
					trap->SendServerCommand(it->entWhoHit - g_entities,
						va("notify 1 \"Assist: +%i Credits\"", awardedCredits));
					trap->SendServerCommand(it->entWhoHit - g_entities, "hitmarker");
				}
			}
		}
		self->assists->clear();

		
		//award bonus credits to teammates:
		if(jkg_teamKillBonus.integer > 0 && !g_dontPenalizeTeam)
		{
			gentity_t* player; int reward;

			for (int j = 0; j < sv_maxclients.integer; j++)
			{
				reward = 0;
				player = &g_entities[j];
				if (!player->inuse || (player - g_entities >= MAX_CLIENTS)  || attacker == nullptr || attacker->client == nullptr || player == attacker)	//don't reward spectators, nonclients or the killer
					continue;

				//if I have more deaths than the # of kills doubled - I get extra credits
				if ( player->client->ps.persistant[PERS_RANK] * 2 < player->client->ps.persistant[PERS_KILLED])
					reward += jkg_teamKillBonus.integer;

				if (!OnSameTeam(self, attacker))	//if the victim and the attacker aren't on the same team
				{

					if (player->client->sess.sessionTeam == attacker->client->sess.sessionTeam)		//and the person we're considering to reward is on the attackers team
					{
						reward += jkg_teamKillBonus.integer;
						trap->SendServerCommand(player->s.number, va("notify 1 \"Team Kill Bonus: +%i Credits\"", reward));
						player->client->ps.credits += reward;

						//consider doing some sort of sound to hint at reward here  --futuza
					}
				}
				else	//it's treason then
				{
					if (player->client->sess.sessionTeam != attacker->client->sess.sessionTeam)		//reward the opposite team, if our team suicides/teamkills!
					{
							reward += jkg_teamKillBonus.integer;
							trap->SendServerCommand(player->s.number, va("notify 1 \"Team Kill Bonus: +%i Credits\"", reward));
							player->client->ps.credits += reward;
					}
				}
			}
		}
	}

	G_BreakArm(self, 0); //unbreak anything we have broken
	self->client->ps.saberEntityNum = self->client->saberStoredIndex; //in case we died while our saber was knocked away.

	self->client->bodyGrabIndex = ENTITYNUM_NONE;
	self->client->bodyGrabTime = 0;

	// JKG - check if he was holding a primed (not cooked) thermal detonator
	if (self->client && self->client->ps.weapon == WP_THERMAL && !self->grenadeCookTime && self->client->ps.weaponstate == WEAPON_CHARGING) {
		WP_RecalculateTheFreakingMuzzleCrap( self ); // Fix to keep the grenades from spawning at enemies :P --eez
		WP_FireGenericWeapon(self, self->client->ps.firingMode);

		if (self->s.eType == ET_PLAYER)
		{
			BG_AdjustItemStackQuantity(self, cmd.invensel, -1);
		}
	}

	if (attacker->NPC)
	{
		G_CheckVictoryScript(attacker);
	}

	if ( self->NPC )
	{
		if ( self->client && NPC_Humanoid_WaitingAmbush( self ) )
		{//ambushing trooper
			self->client->noclip = qfalse;
		}
		NPC_FreeCombatPoint( self->NPC->combatPoint, qfalse );
		if ( self->NPC->group )
		{
			//lastInGroup = (self->NPC->group->numGroup < 2);
			AI_GroupMemberKilled( self );
			AI_DeleteSelfFromGroup( self );
		}

		if ( self->NPC->tempGoal )
		{
			G_FreeEntity( self->NPC->tempGoal );
			self->NPC->tempGoal = NULL;
		}

		//if ( self->client->NPC_class == CLASS_BOBAFETT && self->client->moveType == MT_FLYSWIM )
		if ( self->s.NPC_class == CLASS_RANCOR )
		{
			Rancor_DropVictim( self );
		}
		
		/* JKG - Muzzle Calculation */
		if ( self->client->weaponGhoul2[0] )
		{
			trap->G2API_CleanGhoul2Models( &self->client->weaponGhoul2[0] );
		}
		/* JKG - Muzzle Calculation End */
	}
	else
	{
		// Stuff that does not happen to NPC, but does happen to players --eez
	}
	if ( attacker && attacker->NPC && attacker->NPC->group && attacker->NPC->group->enemy == self )
	{
		attacker->NPC->group->enemy = NULL;
	}

	//Cheap method until/if I decide to put fancier stuff in (e.g. sabers falling out of hand and slowly
	//holstering on death like sp)
	if (self->client->ps.weapon == WP_SABER &&
		!self->client->ps.saberHolstered &&
		self->client->ps.saberEntityNum)
	{
		if (!self->client->ps.saberInFlight &&
			self->client->saber[0].soundOff)
		{
			G_Sound(self, CHAN_AUTO, self->client->saber[0].soundOff);
		}
		if (self->client->saber[1].soundOff &&
			self->client->saber[1].model[0])
		{
			G_Sound(self, CHAN_AUTO, self->client->saber[1].soundOff);
		}
	}

	//Use any target we had
	G_UseTargets( self, self );

	if (g_slowmoDuelEnd.integer && (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL) && attacker && attacker->inuse && attacker->client)
	{
		if (!gDoSlowMoDuel)
		{
			gDoSlowMoDuel = qtrue;
			gSlowMoDuelTime = level.time;
		}
	}

	//Make sure the jetpack is turned off.
	Jetpack_Off(self);

	self->client->ps.heldByClient = 0;
	self->client->beingThrown = 0;
	self->client->doingThrow = 0;
	self->client->isHacking = 0;
	self->client->ps.hackingTime = 0;

	if (inflictor && inflictor->activator && !inflictor->client && !attacker->client &&
		inflictor->activator->client && inflictor->activator->inuse &&
		inflictor->s.weapon == WP_TURRET)
	{
		attacker = inflictor->activator;
	}

	//if he was charging or anything else, kill the sound
	G_MuteSound(self->s.number, CHAN_WEAPON);

	BlowDetpacks(self); //blow detpacks if they're planted

	self->client->ps.fd.forceDeactivateAll = 1;

	if ((self == attacker || (attacker && !attacker->client)) &&
		(meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_FALLING || meansOfDeath == MOD_TRIGGER_HURT || meansOfDeath == MOD_UNKNOWN) &&
		self->client->ps.otherKillerTime > level.time)
	{
		attacker = &g_entities[self->client->ps.otherKiller];
	}

	self->client->ps.pm_type = PM_DEAD;
	self->client->ps.pm_flags &= ~PMF_STUCK_TO_WALL;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		gentity_t *killerEnt = &g_entities[killer];
		
		if (killerEnt && killerEnt->inuse && killerEnt->s.eType == ET_NPC)
		{
			if (killerEnt->client->pers.netname[0])
				killerName = killerEnt->client->pers.netname; // UQ1: NPCs have names now...
			else
				killerName = va("A %s NPC", killerEnt->NPC_type);
		}
		else
		{
			killer = ENTITYNUM_WORLD;
			killerName = "<world>";
		}
	}

	if (means) {
		obit = means->ref;
	}
	else {
		obit = "<bad obituary>";
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
		killer, self->s.number, meansOfDeath, killerName, 
		self->client->pers.netname, obit );

	if ( g_austrian.integer 
		&& level.gametype == GT_DUEL
		&& level.numPlayingClients >= 2 )
	{
		int spawnTime = (level.clients[level.sortedClients[0]].respawnTime > level.clients[level.sortedClients[1]].respawnTime) ? level.clients[level.sortedClients[0]].respawnTime : level.clients[level.sortedClients[1]].respawnTime;
		G_LogPrintf("Duel Kill Details:\n");
		G_LogPrintf("Kill Time: %d\n", level.time-spawnTime );
		G_LogPrintf("victim: %s, hits on enemy %d\n", self->client->pers.netname, self->client->ps.persistant[PERS_HITS] );
		if ( attacker && attacker->client )
		{
			G_LogPrintf("killer: %s, hits on enemy %d, health: %d\n", attacker->client->pers.netname, attacker->client->ps.persistant[PERS_HITS], attacker->health );
			//also - if MOD_SABER, list the animation and saber style
			if ( meansOfDeath == MOD_SABER )
			{
				G_LogPrintf("killer saber style: %d, killer saber anim %s\n", attacker->client->ps.fd.saberAnimLevel, animTable[(attacker->client->ps.torsoAnim)].name );
			}
		}
	}

	G_LogWeaponKill(killer, meansOfDeath);
	G_LogWeaponDeath(self->s.number, self->s.weapon);
	if (attacker && attacker->client && attacker->inuse)
	{
		G_LogWeaponFrag(killer, self->s.number);
	}

	// broadcast the death event to everyone
	if (self->s.eType != ET_NPC && !g_noPDuelCheck)
	{
		ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
		ent->s.eventParm = meansOfDeath;
		ent->s.otherEntityNum = self->s.number;
		ent->s.otherEntityNum2 = killer;
		ent->r.svFlags = SVF_BROADCAST;	// send to everyone
	}

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	if (self == attacker)
	{
		self->client->ps.fd.suicides++;
	}

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self || OnSameTeam (self, attacker ) ) {
			if (level.gametype == GT_DUEL)
			{ //in duel, if you kill yourself, the person you are dueling against gets a kill for it
				int otherClNum = -1;
				if (level.sortedClients[0] == self->s.number)
				{
					otherClNum = level.sortedClients[1];
				}
				else if (level.sortedClients[1] == self->s.number)
				{
					otherClNum = level.sortedClients[0];
				}

				if (otherClNum >= 0 && otherClNum < MAX_CLIENTS &&
					g_entities[otherClNum].inuse && g_entities[otherClNum].client &&
					otherClNum != attacker->s.number)
				{
					AddScore( &g_entities[otherClNum], self->r.currentOrigin, 1 );
				}
				else
				{
					AddScore( attacker, self->r.currentOrigin, -1 );
				}
			}
			else
			{
				AddScore( attacker, self->r.currentOrigin, -1 );
			}
		} 
		else 
		{
			AddScore( attacker, self->r.currentOrigin, 1 );
			attacker->client->lastKillTime = level.time;

		}
	} else {
		if (level.gametype == GT_DUEL)
		{ //in duel, if you kill yourself, the person you are dueling against gets a kill for it
			int otherClNum = -1;
			if (level.sortedClients[0] == self->s.number)
			{
				otherClNum = level.sortedClients[1];
			}
			else if (level.sortedClients[1] == self->s.number)
			{
				otherClNum = level.sortedClients[0];
			}

			if (otherClNum >= 0 && otherClNum < MAX_CLIENTS &&
				g_entities[otherClNum].inuse && g_entities[otherClNum].client &&
				otherClNum != self->s.number)
			{
				AddScore( &g_entities[otherClNum], self->r.currentOrigin, 1 );
			}
			else
			{
				AddScore( self, self->r.currentOrigin, -1 );
			}
		}
		else
		{
			AddScore( self, self->r.currentOrigin, -1 );
		}
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE || meansOfDeath == MOD_TEAM_CHANGE) {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap->PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ) && !self->client->ps.fallingToDeath) {
		if (self->s.eType != ET_NPC)
		{
			TossClientItems( self );
		}
	}
	else {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
		}
	}

	if ( MOD_TEAM_CHANGE == meansOfDeath )
	{
		// Give them back a point since they didn't really die.
		AddScore( self, self->r.currentOrigin, 1 );
	}
	else
	{
		Cmd_Score_f( self );		// show scores
	}

	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.weaponVariation = 0;
	self->s.powerups = 0;
	if (self->s.eType != ET_NPC && !(self->client->ps.eFlags & EF_DISINTEGRATION))
	{ //handled differently for NPCs. Don't do this when disintegrating
		self->r.contents = CONTENTS_CORPSE;
		self->r.maxs[2] = -8;
	}
	self->client->ps.zoomMode = 0;	// Turn off zooming when we die

	self->s.loopSound = 0;
	self->s.loopIsSoundset = qfalse;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	if (self->s.eType == ET_PLAYER) {
		self->client->respawnTime = level.time + 6500;
	} else {
		self->client->respawnTime = level.time + 1700;
	}

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	if(!(self->client->ps.eFlags & EF_DISINTEGRATION))
	{
		// normal death
		
		static int i;

		anim = G_PickDeathAnim(self, self->pos1, damage, meansOfDeath, HL_NONE);
		
		if (anim >= 1)
		{ //Some droids don't have death anims
			// for the no-blood option, we need to prevent the health
			// from going to gib level
			if ( self->health <= GIB_HEALTH ) {
				self->health = GIB_HEALTH+1;
			}
			if (self->s.eType != ET_PLAYER) {
				self->client->respawnTime = level.time + 1000;//((self->client->animations[anim].numFrames*40)/(50.0f / self->client->animations[anim].frameLerp))+300;
			}
			sPMType = self->client->ps.pm_type;
			self->client->ps.pm_type = PM_NORMAL; //don't want pm type interfering with our setanim calls.

			if (self->inuse)
			{ //not disconnecting
				G_SetAnim(self, NULL, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_RESTART);
			}

			self->client->ps.pm_type = sPMType;
			
			// JKG - Add custom handling for dismemberment here

			//[FullDismemberment]
			//weapon dismemberment so saber isn't the only one  
			if (means->dismemberment.canDismember)
			{ //update the anim on the actual skeleton (so bolt point will reflect the correct position) and then check for dismem
				G_UpdateClientAnims(self, 1.0f);
				G_CheckForDismemberment(self, attacker, self->pos1, damage, anim, qfalse);
			}

			//GIBBING!!!
			if (means->dismemberment.blowChunks)			//telefrag
			{
				G_UpdateClientAnims(self, 1.0f);
				G_CheckForBlowingUp(self, attacker, self->pos1, damage, anim, qfalse);
			}
			//[/FullDismemberment]

		}
		else if (self->NPC && self->client && self->client->NPC_class != CLASS_MARK1)
		{ //in this case if we're an NPC it's my guess that we want to get removed straight away.
			self->think = G_FreeEntity;
			self->nextthink = level.time;
		}

		// the body can still be gibbed
		if (!self->NPC)
		{ //don't remove NPCs like this!
			self->die = body_die;
		}

		//It won't gib, it will disintegrate (because this is Star Wars).
		self->takedamage = qtrue;

		// globally cycle through the different death animations
		i = ( i + 1 ) % 3;
	}
	else
	{
		// don't free it right away if we're disintegrating, otherwise the client won't be able to see the effect.
		self->think = G_FreeEntity;
		self->nextthink = level.time + 2000;
		self->takedamage = qfalse;
	}

	G_AddEvent(self, EV_DEATH1 + Q_irand(0, 2), 0);

	if (self != attacker)
	{ //don't make NPCs want to murder you on respawn for killing yourself!
		G_DeathAlert(self, attacker);
	}

	if ( self->NPC )
	{//If an NPC, make sure we start running our scripts again- this gets set to infinite while we fall to our deaths
		self->NPC->nextBStateThink = level.time;
	}

	if ( G_ActivateBehavior( self, BSET_DEATH ) )
	{
		//deathScript = qtrue;
	}
	// If the NPC is a Lua NPC, run the OnDie event
	if (self->NPC && self->NPC->isLuaNPC) {
		GLua_NPCEV_OnDie(self, inflictor, attacker, damage, meansOfDeath);
	}
	
	GLua_Hook_PlayerDeath(self->s.number, inflictor, attacker, damage, meansOfDeath);

	if ( self->NPC && (self->NPC->scriptFlags&SCF_FFDEATH) )
	{
		if ( G_ActivateBehavior( self, BSET_FFDEATH ) )  
		{//FIXME: should running this preclude running the normal deathscript?
			//deathScript = qtrue;
		}
		G_UseTargets2( self, self, self->target4 );
	}

	// Free up any timers we may have on us.
	TIMER_Clear2( self );

	trap->LinkEntity ((sharedEntity_t *)self);

	if ( self->NPC )
	{
		self->NPC->timeOfDeath = level.time;//this will change - used for debouncing post-death events
	}


	if (level.gametype == GT_POWERDUEL && !g_noPDuelCheck)
	{ //powerduel checks
		if (self->client->sess.duelTeam == DUELTEAM_LONE)
		{ //automatically means a win as there is only one
			G_AddPowerDuelScore(DUELTEAM_DOUBLE, 1);
			G_AddPowerDuelLoserScore(DUELTEAM_LONE, 1);
			g_endPDuel = qtrue;
		}
		else if (self->client->sess.duelTeam == DUELTEAM_DOUBLE)
		{
			int i = 0;
			gentity_t *check;
			qboolean heLives = qfalse;

			while (i < MAX_CLIENTS)
			{
				check = &g_entities[i];
				if (check->inuse && check->client && check->s.number != self->s.number &&
					check->client->pers.connected == CON_CONNECTED && !check->client->iAmALoser &&
					check->client->ps.stats[STAT_HEALTH] > 0 &&
					check->client->sess.sessionTeam != TEAM_SPECTATOR &&
					check->client->sess.duelTeam == DUELTEAM_DOUBLE)
				{ //still an active living paired duelist so it's not over yet.
					heLives = qtrue;
					break;
				}
				i++;
			}

			if (!heLives)
			{ //they're all dead, give the lone duelist the win.
				G_AddPowerDuelScore(DUELTEAM_LONE, 1);
				G_AddPowerDuelLoserScore(DUELTEAM_DOUBLE, 1);
				g_endPDuel = qtrue;
			}
		}
	}
}


/*
================
CheckShield
================
*/
int CheckShield (gentity_t *ent, int damage, int dflags, meansOfDamage_t* means)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (means == nullptr)
		return 0;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_SHIELD)
		return 0;

	if (means->modifiers.ignoreShield)
		return 0;

	count = client->ps.stats[STAT_SHIELD];

	save = damage;
	save *= means->modifiers.shield;

	// save is the most damage that the armor is elibigle to protect, of course, but it's limited by the total armor.
	if (save >= count) {
		save = count;
	}

	return save;
}

/*
================
ShieldHitEffect
================
*/

void ShieldHitEffect(gentity_t* targ, vec3_t dir, int take)
{
	// Play the shield hit effect
	gentity_t	*evEnt;
	evEnt = G_TempEntity(targ->r.currentOrigin, EV_SHIELD_HIT);
	evEnt->s.otherEntityNum = targ->s.number;
	evEnt->s.eventParm = DirToByte(dir);
	evEnt->s.time2 = take;

	if (targ->client) {
		targ->client->shieldRechargeLast = targ->client->shieldRegenLast = level.time;
		targ->client->shieldRecharging = qfalse;

		// Break the shield if it's dead
		if (targ->client->ps.stats[STAT_SHIELD] <= 0) {
			gentity_t* evEnt2;
			evEnt2 = G_TempEntity(targ->r.currentOrigin, EV_SHIELD_BROKEN);
			evEnt2->s.otherEntityNum = targ->s.number;

			// Play the sound on the server, since clients don't know about other clients's inventories
			for (auto it = targ->inventory->begin(); it != targ->inventory->end(); ++it) {
				if (it->equipped && it->id->itemType == ITEM_SHIELD) {
					if (it->id->shieldData.brokenSoundEffect[0]) {
						G_Sound(targ, CHAN_AUTO, G_SoundIndex(it->id->shieldData.brokenSoundEffect));
					}
				}
			}
		}
	}
}

/*
 *	Applies knockback to the target as a result of damage
 */
void G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback )
{
	vec3_t	kvel;
	float	mass;

	if (targ->client && targ->client->pmfreeze)
	{	// don't apply knockback to frozen targets
		return;
	}

	if ( targ->physicsBounce > 0 )	//overide the mass
		mass = targ->physicsBounce;
	else
		mass = 200;

	if ( g_gravity.value > 0 )
	{
		VectorScale( newDir, g_knockback.value * (float)knockback / mass * 0.8f, kvel );
		kvel[2] = newDir[2] * g_knockback.value * (float)knockback / mass * 1.5f;
	}
	else
	{
		VectorScale( newDir, g_knockback.value * (float)knockback / mass, kvel );
	}

	if ( targ->client )
	{
		VectorAdd( targ->client->ps.velocity, kvel, targ->client->ps.velocity );
	}
	else if ( targ->s.pos.trType != TR_STATIONARY && targ->s.pos.trType != TR_LINEAR_STOP && targ->s.pos.trType != TR_NONLINEAR_STOP )
	{
		VectorAdd( targ->s.pos.trDelta, kvel, targ->s.pos.trDelta );
		VectorCopy( targ->r.currentOrigin, targ->s.pos.trBase );
		targ->s.pos.trTime = level.time;
	}

	// set the timer so that the other client can't cancel
	// out the movement immediately
	if ( targ->client && !targ->client->ps.pm_time ) 
	{
		int		t;

		t = knockback * 2;
		if ( t < 50 ) {
			t = 50;
		}
		if ( t > 200 ) {
			t = 200;
		}
		targ->client->ps.pm_time = t;
		targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	}
}

/*
===================================
rww - beginning of the majority of the dismemberment and location based damage code.
===================================
*/

void G_GetDismemberLoc(gentity_t *self, vec3_t boltPoint, int limbType)
{ //Just get the general area without using server-side ghoul2
	vec3_t fwd, right, up;

	AngleVectors(self->r.currentAngles, fwd, right, up);

	VectorCopy(self->r.currentOrigin, boltPoint);

	switch (limbType)
	{
	case G2_MODELPART_HEAD:
		boltPoint[0] += up[0]*24;
		boltPoint[1] += up[1]*24;
		boltPoint[2] += up[2]*24;
		break;
	case G2_MODELPART_WAIST:
		boltPoint[0] += up[0]*4;
		boltPoint[1] += up[1]*4;
		boltPoint[2] += up[2]*4;
		break;
	case G2_MODELPART_LARM:
		boltPoint[0] += up[0]*18;
		boltPoint[1] += up[1]*18;
		boltPoint[2] += up[2]*18;

		boltPoint[0] -= right[0]*10;
		boltPoint[1] -= right[1]*10;
		boltPoint[2] -= right[2]*10;
		break;
	case G2_MODELPART_RARM:
		boltPoint[0] += up[0]*18;
		boltPoint[1] += up[1]*18;
		boltPoint[2] += up[2]*18;

		boltPoint[0] += right[0]*10;
		boltPoint[1] += right[1]*10;
		boltPoint[2] += right[2]*10;
		break;
	case G2_MODELPART_RHAND:
		boltPoint[0] += up[0]*8;
		boltPoint[1] += up[1]*8;
		boltPoint[2] += up[2]*8;

		boltPoint[0] += right[0]*10;
		boltPoint[1] += right[1]*10;
		boltPoint[2] += right[2]*10;
		break;
	case G2_MODELPART_LLEG:
		boltPoint[0] -= up[0]*4;
		boltPoint[1] -= up[1]*4;
		boltPoint[2] -= up[2]*4;

		boltPoint[0] -= right[0]*10;
		boltPoint[1] -= right[1]*10;
		boltPoint[2] -= right[2]*10;
		break;
	case G2_MODELPART_RLEG:
		boltPoint[0] -= up[0]*4;
		boltPoint[1] -= up[1]*4;
		boltPoint[2] -= up[2]*4;

		boltPoint[0] += right[0]*10;
		boltPoint[1] += right[1]*10;
		boltPoint[2] += right[2]*10;
		break;
	default:
		break;
	}

	return;
}

void G_GetDismemberBolt(gentity_t *self, vec3_t boltPoint, int limbType)
{
	int useBolt = self->genericValue5;
	vec3_t properOrigin, properAngles, addVel;
	//vec3_t legAxis[3];
	mdxaBone_t	boltMatrix;
	float fVSpeed = 0;
	char *rotateBone = NULL;

	switch (limbType)
	{
	case G2_MODELPART_HEAD:
		rotateBone = "cranium";
		break;
	case G2_MODELPART_WAIST:
		if (self->localAnimIndex < NUM_RESERVED_ANIMSETS)
		{ //humanoid
			rotateBone = "thoracic";
		}
		else
		{
			rotateBone = "pelvis";
		}
		break;
	case G2_MODELPART_LARM:
		rotateBone = "lradius";
		break;
	case G2_MODELPART_RARM:
		rotateBone = "rradius";
		break;
	case G2_MODELPART_RHAND:
		rotateBone = "rhand";
		break;
	case G2_MODELPART_LLEG:
		rotateBone = "ltibia";
		break;
	case G2_MODELPART_RLEG:
		rotateBone = "rtibia";
		break;
	default:
		rotateBone = "rtibia";
		break;
	}

	useBolt = trap->G2API_AddBolt(self->ghoul2, 0, rotateBone);

	VectorCopy(self->client->ps.origin, properOrigin);
	VectorCopy(self->client->ps.viewangles, properAngles);

	//try to predict the origin based on velocity so it's more like what the client is seeing
	VectorCopy(self->client->ps.velocity, addVel);
	VectorNormalize(addVel);

	if (self->client->ps.velocity[0] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[0]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[0];
	}
	if (self->client->ps.velocity[1] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[1]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[1];
	}
	if (self->client->ps.velocity[2] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[2]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[2];
	}

	fVSpeed *= 0.08f;

	properOrigin[0] += addVel[0]*fVSpeed;
	properOrigin[1] += addVel[1]*fVSpeed;
	properOrigin[2] += addVel[2]*fVSpeed;

	properAngles[0] = 0;
	properAngles[1] = self->client->ps.viewangles[YAW];
	properAngles[2] = 0;

	trap->G2API_GetBoltMatrix(self->ghoul2, 0, useBolt, &boltMatrix, properAngles, properOrigin, level.time, NULL, self->modelScale);

	boltPoint[0] = boltMatrix.matrix[0][3];
	boltPoint[1] = boltMatrix.matrix[1][3];
	boltPoint[2] = boltMatrix.matrix[2][3];

	trap->G2API_GetBoltMatrix(self->ghoul2, 1, 0, &boltMatrix, properAngles, properOrigin, level.time, NULL, self->modelScale);

	if (self->client && limbType == G2_MODELPART_RHAND)
	{ //Make some saber hit sparks over the severed wrist area
		vec3_t boltAngles;
		gentity_t *te;

		boltAngles[0] = -boltMatrix.matrix[0][1];
		boltAngles[1] = -boltMatrix.matrix[1][1];
		boltAngles[2] = -boltMatrix.matrix[2][1];

		te = G_TempEntity( boltPoint, EV_SABER_HIT );
		te->s.otherEntityNum = self->s.number;
		te->s.otherEntityNum2 = ENTITYNUM_NONE;
		te->s.weapon = 0;//saberNum
		te->s.legsAnim = 0;//bladeNum

		VectorCopy(boltPoint, te->s.origin);
		VectorCopy(boltAngles, te->s.angles);
		
		if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
		{ //don't let it play with no direction
			te->s.angles[1] = 1;
		}

		te->s.eventParm = 16; //lots of sparks
	}
}

void LimbTouch( gentity_t *self, gentity_t *other, trace_t *trace )
{
}

void LimbThink( gentity_t *ent )
{
	float gravity = 3.0f;
	float mass = 0.09f;
	float bounce = 1.3f;

	switch (ent->s.modelGhoul2)
	{
	case G2_MODELPART_HEAD:
		mass = 0.08f;
		bounce = 1.4f;
		break;
	case G2_MODELPART_WAIST:
		mass = 0.1f;
		bounce = 1.2f;
		break;
	case G2_MODELPART_LARM:
	case G2_MODELPART_RARM:
	case G2_MODELPART_RHAND:
	case G2_MODELPART_LLEG:
	case G2_MODELPART_RLEG:
	default:
		break;
	}

	if (ent->speed < level.time)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	if (ent->genericValue5 <= level.time)
	{ //this will be every frame by standard, but we want to compensate in case sv_fps is not 20.
		G_RunExPhys(ent, gravity, mass, bounce, qtrue, NULL, 0);
		ent->genericValue5 = level.time + 50;
	}

	ent->nextthink = level.time;
}

extern qboolean BG_GetRootSurfNameWithVariant( void *ghoul2, const char *rootSurfName, char *returnSurfName, int returnSize );

void G_Dismember( gentity_t *ent, gentity_t *enemy, vec3_t point, int limbType, float limbRollBase, float limbPitchBase, int deathAnim, qboolean postDeath )
{
	vec3_t	newPoint, dir, vel;
	gentity_t *limb;
	char	limbName[MAX_QPATH];
	char	stubName[MAX_QPATH];
	char	stubCapName[MAX_QPATH];

	if (limbType == G2_MODELPART_HEAD)
	{
		Q_strncpyz( limbName , "head", sizeof( limbName  ) );
		Q_strncpyz( stubCapName, "torso_cap_head", sizeof( stubCapName ) );
	}
	else if (limbType == G2_MODELPART_WAIST)
	{
		Q_strncpyz( limbName, "torso", sizeof( limbName ) );
		Q_strncpyz( stubCapName, "hips_cap_torso", sizeof( stubCapName ) );
	}
	else if (limbType == G2_MODELPART_LARM)
	{
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "l_arm", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "torso", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_l_arm", stubName );
	}
	else if (limbType == G2_MODELPART_RARM)
	{
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "r_arm", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "torso", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_r_arm", stubName );
	}
	else if (limbType == G2_MODELPART_RHAND)
	{
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "r_hand", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "r_arm", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_r_hand", stubName );
	}
	else if (limbType == G2_MODELPART_LLEG)
	{
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "l_leg", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "hips", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_l_leg", stubName );
	}
	else if (limbType == G2_MODELPART_RLEG)
	{
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "r_leg", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "hips", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_r_leg", stubName );
	}
	else
	{//umm... just default to the right leg, I guess (same as on client)
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "r_leg", limbName, sizeof(limbName) );
		BG_GetRootSurfNameWithVariant( ent->ghoul2, "hips", stubName, sizeof(stubName) );
		Com_sprintf( stubCapName, sizeof( stubCapName), "%s_cap_r_leg", stubName );
	}

	if (ent->ghoul2 && limbName[0] && trap->G2API_GetSurfaceRenderStatus(ent->ghoul2, 0, limbName))
	{ //is it already off? If so there's no reason to be doing it again, so get out of here.
		return;
	}

	VectorCopy( point, newPoint );
	limb = G_Spawn();
	limb->classname = "playerlimb";

	/*
	if (limbType == G2_MODELPART_WAIST)
	{ //slight hack
		newPoint[2] += 1;
	}
	*/

	G_SetOrigin( limb, newPoint );
	VectorCopy( newPoint, limb->s.pos.trBase );
	limb->think = LimbThink;
	limb->touch = LimbTouch;
	limb->speed = level.time + Q_irand(8000, 16000);
	limb->nextthink = level.time + FRAMETIME;

	limb->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	limb->clipmask = MASK_SOLID;
	limb->r.contents = CONTENTS_TRIGGER;
	limb->physicsObject = qtrue;
	VectorSet( limb->r.mins, -6.0f, -6.0f, -3.0f );
	VectorSet( limb->r.maxs, 6.0f, 6.0f, 6.0f );

	limb->s.g2radius = 200;

	limb->s.eType = ET_GENERAL;
	limb->s.weapon = G2_MODEL_PART;
	limb->s.modelGhoul2 = limbType;
	limb->s.modelindex = ent->s.number;
	if (!ent->client)
	{
		limb->s.modelindex = -1;
		limb->s.otherEntityNum2 = ent->s.number;
	}

	VectorClear(limb->s.apos.trDelta);

	if (ent->client)
	{
		VectorCopy(ent->client->ps.viewangles, limb->r.currentAngles);
		VectorCopy(ent->client->ps.viewangles, limb->s.apos.trBase);
	}
	else
	{
		VectorCopy(ent->r.currentAngles, limb->r.currentAngles);
		VectorCopy(ent->r.currentAngles, limb->s.apos.trBase);
	}

	//Set up the ExPhys values for the entity.
	limb->epGravFactor = 0;
	VectorClear(limb->epVelocity);
	VectorSubtract( point, ent->r.currentOrigin, dir );
	VectorNormalize( dir );
	if (ent->client)
	{
		VectorCopy(ent->client->ps.velocity, vel);
	}
	else
	{
		VectorCopy(ent->s.pos.trDelta, vel);
	}
	VectorMA( vel, 80, dir, limb->epVelocity );

	//add some vertical velocity
	if (limbType == G2_MODELPART_HEAD ||
		limbType == G2_MODELPART_WAIST)
	{
		limb->epVelocity[2] += 10;
	}

	if (enemy && enemy->client && ent && ent != enemy && ent->s.number != enemy->s.number &&
		enemy->client->ps.weapon == WP_SABER && enemy->client->olderIsValid &&
		(level.time - enemy->client->lastSaberStorageTime) < 200)
	{ //The enemy has valid saber positions between this and last frame. Use them to factor in direction of the limb.
		vec3_t dif;
		float totalDistance;
		const float distScale = 1.2f;

		//scale down the initial velocity first, which is based on the speed of the limb owner.
		//ExPhys object velocity operates on a slightly different scale than Q3-based physics velocity.
		VectorScale(limb->epVelocity, 0.4f, limb->epVelocity);

		VectorSubtract(enemy->client->lastSaberBase_Always, enemy->client->olderSaberBase, dif);
		totalDistance = VectorNormalize(dif);

		VectorScale(dif, totalDistance*distScale, dif);
		VectorAdd(limb->epVelocity, dif, limb->epVelocity);

		if (ent->client && (ent->client->ps.torsoTimer > 0 || !BG_InDeathAnim(ent->client->ps.torsoAnim)))
		{ //if he's done with his death anim we don't actually want the limbs going far
			vec3_t preVel;

			VectorCopy(limb->epVelocity, preVel);
			preVel[2] = 0;
			totalDistance = VectorNormalize(preVel);

			if (totalDistance < 40.0f)
			{
				float mAmt = 40.0f;//60.0f/totalDistance;

				limb->epVelocity[0] = preVel[0]*mAmt;
				limb->epVelocity[1] = preVel[1]*mAmt;
			}
		}
		else if (ent->client)
		{
			VectorScale(limb->epVelocity, 0.3f, limb->epVelocity);
		}
	}

	if (ent->s.eType == ET_NPC && ent->ghoul2 && limbName[0] && stubCapName[0])
	{ //if it's an npc remove these surfs on the server too. For players we don't even care cause there's no further dismemberment after death.
		trap->G2API_SetSurfaceOnOff(ent->ghoul2, limbName, 0x00000100);
		trap->G2API_SetSurfaceOnOff(ent->ghoul2, stubCapName, 0);
	}

	//Raz: Limbs now have team colours.
	if ( level.gametype >= GT_TEAM && ent->s.eType != ET_NPC )
	{//Team game
		switch ( ent->client->sess.sessionTeam )
		{
		case TEAM_RED:
			limb->s.customRGBA[0] = 255;
			limb->s.customRGBA[1] = 0;
			limb->s.customRGBA[2] = 0;
			break;

		case TEAM_BLUE:
			limb->s.customRGBA[0] = 0;
			limb->s.customRGBA[1] = 0;
			limb->s.customRGBA[2] = 255;
			break;

		default:
			limb->s.customRGBA[0] = ent->s.customRGBA[0];
			limb->s.customRGBA[1] = ent->s.customRGBA[1];
			limb->s.customRGBA[2] = ent->s.customRGBA[2];
			limb->s.customRGBA[3] = ent->s.customRGBA[3];
			break;
		}
	}
	else
	{//FFA
		limb->s.customRGBA[0] = ent->s.customRGBA[0];
		limb->s.customRGBA[1] = ent->s.customRGBA[1];
		limb->s.customRGBA[2] = ent->s.customRGBA[2];
		limb->s.customRGBA[3] = ent->s.customRGBA[3];
	}

	trap->LinkEntity( (sharedEntity_t *)limb );
}

void DismembermentTest(gentity_t *self)
{
	int sect = G2_MODELPART_HEAD;
	vec3_t boltPoint;

	while (sect <= G2_MODELPART_RLEG)
	{
		G_GetDismemberBolt(self, boltPoint, sect);
		G_Dismember( self, self, boltPoint, sect, 90, 0, BOTH_DEATH1, qfalse );
		sect++;
	}
}

void DismembermentByNum(gentity_t *self, int num)
{
	int sect = G2_MODELPART_HEAD;
	vec3_t boltPoint;

	switch (num)
	{
	case 0:
		sect = G2_MODELPART_HEAD;
		break;
	case 1:
		sect = G2_MODELPART_WAIST;
		break;
	case 2:
		sect = G2_MODELPART_LARM;
		break;
	case 3:
		sect = G2_MODELPART_RARM;
		break;
	case 4:
		sect = G2_MODELPART_RHAND;
		break;
	case 5:
		sect = G2_MODELPART_LLEG;
		break;
	case 6:
		sect = G2_MODELPART_RLEG;
		break;
	default:
		break;
	}

	G_GetDismemberBolt(self, boltPoint, sect);
	G_Dismember( self, self, boltPoint, sect, 90, 0, BOTH_DEATH1, qfalse );
}

int G_GetHitQuad( gentity_t *self, vec3_t hitloc )
{
	vec3_t diff, fwdangles={0,0,0}, right;
	vec3_t clEye;
	float rightdot;
	float zdiff;
	int hitLoc = gPainHitLoc;

	if (self->client)
	{
		VectorCopy(self->client->ps.origin, clEye);
		clEye[2] += self->client->ps.viewheight;
	}
	else
	{
		VectorCopy(self->s.pos.trBase, clEye);
		clEye[2] += 16;
	}

	VectorSubtract( hitloc, clEye, diff );
	diff[2] = 0;
	VectorNormalize( diff );

	if (self->client)
	{
		fwdangles[1] = self->client->ps.viewangles[1];
	}
	else
	{
		fwdangles[1] = self->s.apos.trBase[1];
	}
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff);
	zdiff = hitloc[2] - clEye[2];
	
	if ( zdiff > 0 )
	{
		if ( rightdot > 0.3f )
		{
			hitLoc = G2_MODELPART_RARM;
		}
		else if ( rightdot < -0.3f )
		{
			hitLoc = G2_MODELPART_LARM;
		}
		else
		{
			hitLoc = G2_MODELPART_HEAD;
		}
	}
	else if ( zdiff > -20 )
	{
		if ( rightdot > 0.1f )
		{
			hitLoc = G2_MODELPART_RARM;
		}
		else if ( rightdot < -0.1f )
		{
			hitLoc = G2_MODELPART_LARM;
		}
		else
		{
			hitLoc = G2_MODELPART_HEAD;
		}
	}
	else
	{
		if ( rightdot >= 0 )
		{
			hitLoc = G2_MODELPART_RLEG;
		}
		else
		{
			hitLoc = G2_MODELPART_LLEG;
		}
	}

	return hitLoc;
}

int gGAvoidDismember = 0;

void UpdateClientRenderBolts(gentity_t *self, vec3_t renderOrigin, vec3_t renderAngles);


qboolean G_GetHitLocFromSurfName( gentity_t *ent, const char *surfName, int *hitLoc, vec3_t point, vec3_t dir, vec3_t bladeDir, int mod )
{
	qboolean dismember = qfalse;
	int actualTime;
	int kneeLBolt = -1;
	int kneeRBolt = -1;
	int handRBolt = -1;
	int handLBolt = -1;
	int footRBolt = -1;
	int footLBolt = -1;

	*hitLoc = HL_NONE;

	if ( !surfName || !surfName[0] )
	{
		return qfalse;
	}

	if( !ent->client )
	{
		return qfalse;
	}

	if (!point)
	{
		return qfalse;
	}

	if ( ent->client 
		&& ( ent->client->NPC_class == CLASS_R2D2 
			|| ent->client->NPC_class == CLASS_R5D2 
			|| ent->client->NPC_class == CLASS_GONK
			|| ent->client->NPC_class == CLASS_MOUSE
			|| ent->client->NPC_class == CLASS_SENTRY
			|| ent->client->NPC_class == CLASS_INTERROGATOR
			|| ent->client->NPC_class == CLASS_SENTRY
			|| ent->client->NPC_class == CLASS_PROBE ) )
	{//we don't care about per-surface hit-locations or dismemberment for these guys 
		return qfalse;
	}

	if (ent->localAnimIndex < NUM_RESERVED_ANIMSETS)
	{ //humanoid
		handLBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand");
		handRBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*r_hand");
		kneeLBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*hips_l_knee");
		kneeRBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*hips_r_knee");
		footLBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*l_leg_foot");
		footRBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*r_leg_foot");
	}

	if ( ent->client && (ent->client->NPC_class == CLASS_ATST) )
	{
		//FIXME: almost impossible to hit these... perhaps we should
		//		check for splashDamage and do radius damage to these parts?
		//		Or, if we ever get bbox G2 traces, that may fix it, too
		if (!Q_stricmp("head_light_blaster_cann",surfName))
		{
			*hitLoc = HL_ARM_LT;
		}
		else if (!Q_stricmp("head_concussion_charger",surfName))
		{
			*hitLoc = HL_ARM_RT;
		}
		return(qfalse);
	}
	else if ( ent->client && (ent->client->NPC_class == CLASS_MARK1) )
	{
		if (!Q_stricmp("l_arm",surfName))
		{
			*hitLoc = HL_ARM_LT;
		}
		else if (!Q_stricmp("r_arm",surfName))
		{
			*hitLoc = HL_ARM_RT;
		}
		else if (!Q_stricmp("torso_front",surfName))
		{
			*hitLoc = HL_CHEST;
		}
		else if (!Q_stricmp("torso_tube1",surfName))
		{
			*hitLoc = HL_GENERIC1;
		}
		else if (!Q_stricmp("torso_tube2",surfName))
		{
			*hitLoc = HL_GENERIC2;
		}
		else if (!Q_stricmp("torso_tube3",surfName))
		{
			*hitLoc = HL_GENERIC3;
		}
		else if (!Q_stricmp("torso_tube4",surfName))
		{
			*hitLoc = HL_GENERIC4;
		}
		else if (!Q_stricmp("torso_tube5",surfName))
		{
			*hitLoc = HL_GENERIC5;
		}
		else if (!Q_stricmp("torso_tube6",surfName))
		{
			*hitLoc = HL_GENERIC6;
		}
		return(qfalse);
	}
	else if ( ent->client && (ent->client->NPC_class == CLASS_MARK2) )
	{
		if (!Q_stricmp("torso_canister1",surfName))
		{
			*hitLoc = HL_GENERIC1;
		}
		else if (!Q_stricmp("torso_canister2",surfName))
		{
			*hitLoc = HL_GENERIC2;
		}
		else if (!Q_stricmp("torso_canister3",surfName))
		{
			*hitLoc = HL_GENERIC3;
		}
		return(qfalse);
	}
	else if ( ent->client && (ent->client->NPC_class == CLASS_GALAKMECH) )
	{
		if (!Q_stricmp("torso_antenna",surfName)||!Q_stricmp("torso_antenna_base",surfName))
		{
			*hitLoc = HL_GENERIC1;
		}
		else if (!Q_stricmp("torso_shield",surfName))
		{
			*hitLoc = HL_GENERIC2;
		}
		else
		{
			*hitLoc = HL_CHEST;
		}
		return(qfalse);
	}

	//FIXME: check the hitLoc and hitDir against the cap tag for the place 
	//where the split will be- if the hit dir is roughly perpendicular to 
	//the direction of the cap, then the split is allowed, otherwise we
	//hit it at the wrong angle and should not dismember...
	actualTime = level.time;
	if ( !Q_strncmp( "hips", surfName, 4 ) )
	{//FIXME: test properly for legs
		*hitLoc = HL_WAIST;
		if ( ent->client != NULL && ent->ghoul2 )
		{
			mdxaBone_t	boltMatrix;
			vec3_t	tagOrg, angles;

			VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
			if (kneeLBolt>=0)
			{
				trap->G2API_GetBoltMatrix( ent->ghoul2, 0, kneeLBolt, 
								&boltMatrix, angles, ent->r.currentOrigin,
								actualTime, NULL, ent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
				if ( DistanceSquared( point, tagOrg ) < 100 )
				{//actually hit the knee
					*hitLoc = HL_LEG_LT;
				}
			}
			if (*hitLoc == HL_WAIST)
			{
				if (kneeRBolt>=0)
				{
					trap->G2API_GetBoltMatrix( ent->ghoul2, 0, kneeRBolt, 
									&boltMatrix, angles, ent->r.currentOrigin,
									actualTime, NULL, ent->modelScale );
					BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
					if ( DistanceSquared( point, tagOrg ) < 100 )
					{//actually hit the knee
						*hitLoc = HL_LEG_RT;
					}
				}
			}
		}
	}
	else if ( !Q_strncmp( "torso", surfName, 5 ) )
	{
		if ( !ent->client )
		{
			*hitLoc = HL_CHEST;
		}
		else
		{
			vec3_t	t_fwd, t_rt, t_up, dirToImpact;
			float frontSide, rightSide, upSide;
			AngleVectors( ent->client->renderInfo.torsoAngles, t_fwd, t_rt, t_up );

			if (ent->client->renderInfo.boltValidityTime != level.time)
			{
				vec3_t renderAng;

				renderAng[0] = 0;
				renderAng[1] = ent->client->ps.viewangles[YAW];
				renderAng[2] = 0;

				UpdateClientRenderBolts(ent, ent->client->ps.origin, renderAng);
			}

			VectorSubtract( point, ent->client->renderInfo.torsoPoint, dirToImpact );
			frontSide = DotProduct( t_fwd, dirToImpact );
			rightSide = DotProduct( t_rt, dirToImpact );
			upSide = DotProduct( t_up, dirToImpact );
			if ( upSide < -10 )
			{//hit at waist
				*hitLoc = HL_WAIST;
			}
			else
			{//hit on upper torso
				if ( rightSide > 4 )
				{
					*hitLoc = HL_ARM_RT;
				}
				else if ( rightSide < -4 )
				{
					*hitLoc = HL_ARM_LT;
				}
				else if ( rightSide > 2 )
				{
					if ( frontSide > 0 )
					{
						*hitLoc = HL_CHEST_RT;
					}
					else
					{
						*hitLoc = HL_BACK_RT;
					}
				}
				else if ( rightSide < -2 )
				{
					if ( frontSide > 0 )
					{
						*hitLoc = HL_CHEST_LT;
					}
					else
					{
						*hitLoc = HL_BACK_LT;
					}
				}
				else if ( upSide > -3 && mod == MOD_SABER )
				{
					*hitLoc = HL_HEAD;
				}
				else if ( frontSide > 0 )
				{
					*hitLoc = HL_CHEST;
				}
				else
				{
					*hitLoc = HL_BACK;
				}
			}
		}
	}
	else if ( !Q_strncmp( "head", surfName, 4 ) )
	{
		*hitLoc = HL_HEAD;
	}
	else if ( !Q_strncmp( "r_arm", surfName, 5 ) )
	{
		*hitLoc = HL_ARM_RT;
		if ( ent->client != NULL && ent->ghoul2 )
		{
			mdxaBone_t	boltMatrix;
			vec3_t	tagOrg, angles;

			VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
			if (handRBolt>=0)
			{
				trap->G2API_GetBoltMatrix( ent->ghoul2, 0, handRBolt, 
								&boltMatrix, angles, ent->r.currentOrigin,
								actualTime, NULL, ent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
				if ( DistanceSquared( point, tagOrg ) < 256 )
				{//actually hit the hand
					*hitLoc = HL_HAND_RT;
				}
			}
		}
	}
	else if ( !Q_strncmp( "l_arm", surfName, 5 ) )
	{
		*hitLoc = HL_ARM_LT;
		if ( ent->client != NULL && ent->ghoul2 )
		{
			mdxaBone_t	boltMatrix;
			vec3_t	tagOrg, angles;

			VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
			if (handLBolt>=0)
			{
				trap->G2API_GetBoltMatrix( ent->ghoul2, 0, handLBolt, 
								&boltMatrix, angles, ent->r.currentOrigin,
								actualTime, NULL, ent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
				if ( DistanceSquared( point, tagOrg ) < 256 )
				{//actually hit the hand
					*hitLoc = HL_HAND_LT;
				}
			}
		}
	}
	else if ( !Q_strncmp( "r_leg", surfName, 5 ) )
	{
		*hitLoc = HL_LEG_RT;
		if ( ent->client != NULL && ent->ghoul2 )
		{
			mdxaBone_t	boltMatrix;
			vec3_t	tagOrg, angles;

			VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
			if (footRBolt>=0)
			{
				trap->G2API_GetBoltMatrix( ent->ghoul2, 0, footRBolt, 
								&boltMatrix, angles, ent->r.currentOrigin,
								actualTime, NULL, ent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
				if ( DistanceSquared( point, tagOrg ) < 100 )
				{//actually hit the foot
					*hitLoc = HL_FOOT_RT;
				}
			}
		}
	}
	else if ( !Q_strncmp( "l_leg", surfName, 5 ) )
	{
		*hitLoc = HL_LEG_LT;
		if ( ent->client != NULL && ent->ghoul2 )
		{
			mdxaBone_t	boltMatrix;
			vec3_t	tagOrg, angles;

			VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
			if (footLBolt>=0)
			{
				trap->G2API_GetBoltMatrix( ent->ghoul2, 0, footLBolt, 
								&boltMatrix, angles, ent->r.currentOrigin,
								actualTime, NULL, ent->modelScale );
				BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
				if ( DistanceSquared( point, tagOrg ) < 100 )
				{//actually hit the foot
					*hitLoc = HL_FOOT_LT;
				}
			}
		}
	}
	else if ( !Q_strncmp( "r_hand", surfName, 6 ) || !Q_strncmp( "w_", surfName, 2 ) )
	{//right hand or weapon
		*hitLoc = HL_HAND_RT;
	}
	else if ( !Q_strncmp( "l_hand", surfName, 6 ) )
	{
		*hitLoc = HL_HAND_LT;
	}
	/*
#ifdef _DEBUG
	else
	{
		Com_Printf( "ERROR: surface %s does not belong to any hitLocation!!!\n", surfName );
	}
#endif //_DEBUG
	*/

	//if ( g_dismemberment->integer >= 11381138 || !ent->client->dismembered )
	if (g_dismember.integer == 100)
	{ //full probability...
		if ( ent->client && ent->client->NPC_class == CLASS_PROTOCOL )
		{
			dismember = qtrue;
		}
		else if ( dir && (dir[0] || dir[1] || dir[2]) &&
			bladeDir && (bladeDir[0] || bladeDir[1] || bladeDir[2]) )
		{//we care about direction (presumably for dismemberment)
			//if ( g_dismemberProbabilities->value<=0.0f||G_Dismemberable( ent, *hitLoc ) )
			if (1) //Fix me?
			{//either we don't care about probabilties or the probability let us continue
				char *tagName = NULL;
				float	aoa = 0.5f;
				//dir must be roughly perpendicular to the hitLoc's cap bolt
				switch ( *hitLoc )
				{
					case HL_LEG_RT:
						tagName = "*hips_cap_r_leg";
						break;
					case HL_LEG_LT:
						tagName = "*hips_cap_l_leg";
						break;
					case HL_WAIST:
						tagName = "*hips_cap_torso";
						aoa = 0.25f;
						break;
					case HL_CHEST_RT:
					case HL_ARM_RT:
					case HL_BACK_LT:
						tagName = "*torso_cap_r_arm";
						break;
					case HL_CHEST_LT:
					case HL_ARM_LT:
					case HL_BACK_RT:
						tagName = "*torso_cap_l_arm";
						break;
					case HL_HAND_RT:
						tagName = "*r_arm_cap_r_hand";
						break;
					case HL_HAND_LT:
						tagName = "*l_arm_cap_l_hand";
						break;
					case HL_HEAD:
						tagName = "*torso_cap_head";
						aoa = 0.25f;
						break;
					case HL_CHEST:
					case HL_BACK:
					case HL_FOOT_RT:
					case HL_FOOT_LT:
					default:
						//no dismemberment possible with these, so no checks needed
						break;
				}
				if ( tagName )
				{
					int tagBolt = trap->G2API_AddBolt( ent->ghoul2, 0, tagName );
					if ( tagBolt != -1 )
					{
						mdxaBone_t	boltMatrix;
						vec3_t	tagOrg, tagDir, angles;

						VectorSet( angles, 0, ent->r.currentAngles[YAW], 0 );
						trap->G2API_GetBoltMatrix( ent->ghoul2, 0, tagBolt, 
										&boltMatrix, angles, ent->r.currentOrigin,
										actualTime, NULL, ent->modelScale );
						BG_GiveMeVectorFromMatrix( &boltMatrix, ORIGIN, tagOrg );
						BG_GiveMeVectorFromMatrix( &boltMatrix, NEGATIVE_Y, tagDir );
						if ( DistanceSquared( point, tagOrg ) < 256 )
						{//hit close
							float dot = DotProduct( dir, tagDir );
							if ( dot < aoa && dot > -aoa )
							{//hit roughly perpendicular
								dot = DotProduct( bladeDir, tagDir );
								if ( dot < aoa && dot > -aoa )
								{//blade was roughly perpendicular
									dismember = qtrue;
								}
							}
						}
					}
				}
			}
		}
		else
		{ //hmm, no direction supplied.
			dismember = qtrue;
		}
	}
	return dismember;
}

void G_CheckForDismemberment(gentity_t *ent, gentity_t *enemy, vec3_t point, int damage, int deathAnim, qboolean postDeath)
{
	int hitLoc = -1, hitLocUse = -1;
	vec3_t boltPoint;
	int dismember = g_dismember.integer;

	if (ent->localAnimIndex >= NUM_RESERVED_ANIMSETS)
	{
		if (!ent->NPC)
		{
			return;
		}

		if (ent->client->NPC_class != CLASS_PROTOCOL)
		{ //this is the only non-humanoid allowed to do dismemberment.
			return;
		}
	}

	if (!dismember)
	{
		return;
	}

	// JKG - Dismemberment suppression
	if (ent->client && ent->client->noDismember) {
		return;
	}

	if (gGAvoidDismember == 1)
	{
		return;
	}

	if (gGAvoidDismember != 2)
	{ //this means do the dismemberment regardless of randomness and damage
		if (Q_irand(0, 100) > dismember)
		{
			return;
		}

		if (damage < 10)	// Insufficient damage to explain dismemberment
		{
			return;
		}
	}

	if (gGAvoidDismember == 2)
	{
		hitLoc = HL_HAND_RT;
	}
	else
	{
		if (ent->client && ent->client->g2LastSurfaceTime == level.time)
		{
			char hitSurface[MAX_QPATH];

			trap->G2API_GetSurfaceName(ent->ghoul2, ent->client->g2LastSurfaceHit, 0, hitSurface);

			if (hitSurface[0])
			{
				G_GetHitLocFromSurfName(ent, hitSurface, &hitLoc, point, vec3_origin, vec3_origin, MOD_UNKNOWN);
			}
		}

		if (hitLoc == -1)
		{
			hitLoc = G_GetHitLocation( ent, point );
		}
	}

	switch(hitLoc)
	{
	case HL_FOOT_RT:
	case HL_LEG_RT:
		hitLocUse = G2_MODELPART_RLEG;
		break;
	case HL_FOOT_LT:
	case HL_LEG_LT:
		hitLocUse = G2_MODELPART_LLEG;
		break;
		
	case HL_WAIST:
		hitLocUse = G2_MODELPART_WAIST;
		break;
		/*
	case HL_BACK_RT:
	case HL_BACK_LT:
	case HL_BACK:
	case HL_CHEST_RT:
	case HL_CHEST_LT:
	case HL_CHEST:
		break;
		*/
	case HL_ARM_RT:
		hitLocUse = G2_MODELPART_RARM;
		break;
	case HL_HAND_RT:
		hitLocUse = G2_MODELPART_RHAND;
		break;
	case HL_ARM_LT:
	case HL_HAND_LT:
		hitLocUse = G2_MODELPART_LARM;
		break;
	case HL_HEAD:
		hitLocUse = G2_MODELPART_HEAD;
		break;
	default:
		hitLocUse = G_GetHitQuad(ent, point);
		break;
	}

	if (hitLocUse == -1)
	{
		return;
	}

	if (ent->client)
	{
		G_GetDismemberBolt(ent, boltPoint, hitLocUse);
	}
	else
	{
		G_GetDismemberLoc(ent, boltPoint, hitLocUse);
	}
	G_Dismember(ent, enemy, boltPoint, hitLocUse, 90, 0, deathAnim, postDeath);
}

//[FullDismemberment]
void G_CheckForBlowingUp(gentity_t *ent, gentity_t *enemy, vec3_t point, int damage, int deathAnim, qboolean postDeath)
{
	vec3_t boltPoint;
	int dismember = g_dismember.integer;

	if (ent->localAnimIndex >= NUM_RESERVED_ANIMSETS)
	{
		if (!ent->NPC)
		{
			return;
		}

		if (ent->client->NPC_class != CLASS_PROTOCOL)
		{ //this is the only non-humanoid allowed to do dismemberment.
			return;
		}
	}

	if (!dismember)
	{
		return;
	}

	// JKG - Dismemberment suppression
	if (ent->client && ent->client->noDismember) {
		return;
	}

	if (gGAvoidDismember == 1)
	{
		return;
	}

	if (gGAvoidDismember != 2)
	{ //this means do the dismemberment regardless of randomness and damage
		if (Q_irand(0, 100) > dismember)
		{
			return;
		}

		if (damage < 10)		// Insufficient damage to explain any physical damage
		{
			return;
		}
		if (damage < 50) {		// Insufficient damage to explain blowing apart
			G_CheckForDismemberment(ent, enemy, point, damage, deathAnim, postDeath);
			return;				// So use the regular dismemberment
		}
	}
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_HEAD);
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_HEAD, 90, 0, deathAnim, postDeath); 
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_LARM);
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_LARM, 90, 0, deathAnim, postDeath); 
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_RARM);
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_RARM, 90, 0, deathAnim, postDeath); 
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_LLEG);
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_LLEG, 90, 0, deathAnim, postDeath); 
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_RLEG);
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_RLEG, 90, 0, deathAnim, postDeath);
	G_GetDismemberBolt(ent, boltPoint, G2_MODELPART_WAIST); 
	G_Dismember(ent, enemy, boltPoint, G2_MODELPART_WAIST, 90, 0, deathAnim, postDeath); 
}
//[/FullDismemberment]

/*
 *	Modifies the damage based on the location where it hit and also the armor equipped at that location (if any)
    Returns true if a headshot, false if anything else.
 */
bool G_LocationBasedDamageModifier(gentity_t *ent, vec3_t point, int mod, int dflags, int *damage, meansOfDamage_t* means)
{
	int hitLoc = -1;
	int armorSlot = 0;
	qboolean headshot = false;

	if (!g_locationBasedDamage.integer)
	{ //then leave it alone
		return false;
	}

	if (means == nullptr) {
		return false;
	}

	if ( (dflags&DAMAGE_NO_HIT_LOC) )
	{ //then leave it alone
		return false;
	}

	if (mod == MOD_SABER && *damage <= 1)
	{ //don't bother for idle damage
		return false;
	}

	if (!point)
	{
		return false;
	}

	if ((ent->client && ent->client->g2LastSurfaceTime == level.time && mod == MOD_SABER) || //using ghoul2 collision? Then if the mod is a saber we should have surface data from the last hit (unless thrown).
		(d_projectileGhoul2Collision.integer && ent->client && ent->client->g2LastSurfaceTime == level.time)) //It's safe to assume we died from the projectile that just set our surface index. So, go ahead and use that as the surf I guess.
	{
		char hitSurface[MAX_QPATH];

		trap->G2API_GetSurfaceName(ent->ghoul2, ent->client->g2LastSurfaceHit, 0, hitSurface);

		if (hitSurface[0])
		{
			G_GetHitLocFromSurfName(ent, hitSurface, &hitLoc, point, vec3_origin, vec3_origin, MOD_UNKNOWN);
		}
	}

	if (hitLoc == -1)
	{
		hitLoc = G_GetHitLocation( ent, point );
	}

	switch (hitLoc)
	{
	case HL_FOOT_RT:
	case HL_FOOT_LT:
		armorSlot = ARMSLOT_BOOTS;
		*damage *= bgConstants.damageModifiers.footModifier;
		break;
	case HL_LEG_RT:
	case HL_LEG_LT:
		armorSlot = ARMSLOT_LEGS;
		*damage *= bgConstants.damageModifiers.legModifier;
		break;
	case HL_WAIST:
		armorSlot = ARMSLOT_LEGS;
		*damage *= bgConstants.damageModifiers.torsoModifier;
		break;
	case HL_BACK_RT:
	case HL_BACK_LT:
	case HL_BACK:
	case HL_CHEST_RT:
	case HL_CHEST_LT:
	case HL_CHEST:
		armorSlot = ARMSLOT_TORSO;
		*damage *= bgConstants.damageModifiers.torsoModifier;
		break; //normal damage
	case HL_ARM_RT:
	case HL_ARM_LT:
		armorSlot = ARMSLOT_SHOULDER;
		*damage *= bgConstants.damageModifiers.armModifier;
		break;
	case HL_HAND_RT:
	case HL_HAND_LT:
		armorSlot = ARMSLOT_GLOVES;
		*damage *= bgConstants.damageModifiers.handModifier;
		break;
	case HL_HEAD:
		armorSlot = ARMSLOT_HEAD;
		*damage *= bgConstants.damageModifiers.headModifier;
		headshot = true;
		break;
	default:
		break; //do nothing then
	}

	if (ent->client && (ent - g_entities) < MAX_CLIENTS && !means->modifiers.ignoreArmor) {
		// Remove damage based on armor
		int armor = ent->client->ps.armor[armorSlot];
		if (armor--) {
			armorData_t* pArm = &armorTable[armor];
			int armor = pArm->armor;
			float modifier = (ent->client->ps.stats[STAT_MAX_HEALTH] / (float)(ent->client->ps.stats[STAT_MAX_HEALTH] + armor));

			modifier *= means->modifiers.armor;

			*damage *= modifier;
		}
	}

	if (headshot)
		return true;
	else
		return false;
}
/*
===================================
rww - end dismemberment/lbd
===================================
*/

#define PLAYER_KNOCKDOWN_HOLD_EXTRA_TIME 0

void NPC_SetPainEvent( gentity_t *self );
qboolean PM_InKnockDown( playerState_t *ps );
qboolean BG_CrouchAnim( int anim );
qboolean PM_LockedAnim( int anim );

//[KnockdownSys] - Jedi Knight Galaxies - using knockdown system from OJP
void G_Knockdown( gentity_t *self, gentity_t *attacker, const vec3_t pushDir, float strength, qboolean breakSaberLock )
{
	if ( !self || !self->client )
	//removed the SP requirement for an attacker since I want G_Knockdown 
	//to work without an attacker.
	//if ( !self || !self->client || !attacker || !attacker->client ) 
	{
		return;
	}
	if ( (self->flags & FL_GODMODE) || (self->client->noclip)) {
		return;	// Dont knock down when havin godmode or noclip
	}
	
	//75% chance to knock jetpacks out of the air
	if (self->client->ps.eFlags & EF_JETPACK_ACTIVE)
	{
		if(Q_irand(1,4) > 1)
			Jetpack_Off(self);
		//return;  //jetpacks used to be immune to knockback
	}

	if ( PM_LockedAnim( self->client->ps.legsAnim ) )
	{ //stuck doing something else
		return;
	}

	//break out of a saberLock?
	if ( self->client->ps.saberLockTime > level.time )
	{
		if ( breakSaberLock )
		{
			self->client->ps.saberLockTime = 0;
			self->client->ps.saberLockEnemy = ENTITYNUM_NONE;
		}
		else
		{
			return;
		}
	}

	if ( self->health > 0 )
	{
		//racc - make a pain noise whenever you're knocked down.
		if ( self->s.number < MAX_CLIENTS )
		{
			NPC_SetPainEvent( self );
		}
		else
		{//npc
			NPC_Pain( self, attacker, 0 );
			//GEntity_PainFunc( self, attacker, attacker, self->currentOrigin, 0, MOD_MELEE ); SP Version
		}

		if ( !BG_RollingAnim( self->client->ps.legsAnim ) 
			&& !PM_InKnockDown( &self->client->ps ) )
		{
			int knockAnim = BOTH_KNOCKDOWN1;//default knockdown
			if ( BG_CrouchAnim( self->client->ps.legsAnim ) )
			{//crouched knockdown
				knockAnim = BOTH_KNOCKDOWN4;
			}
			else
			{//plain old knockdown
				vec3_t pLFwd, pLAngles;
				VectorSet(pLAngles, 0, self->client->ps.viewangles[YAW], 0);
				AngleVectors( pLAngles, pLFwd, NULL, NULL );
				if ( DotProduct( pLFwd, pushDir ) > 0.2f )
				{//pushing him from behind
					knockAnim = BOTH_KNOCKDOWN3;
				}
				else
				{//pushing him from front
					knockAnim = BOTH_KNOCKDOWN1;
				}
			}
			if ( knockAnim == BOTH_KNOCKDOWN1 && strength > 150 )
			{//push *hard*
				knockAnim = BOTH_KNOCKDOWN2;
			}
			NPC_SetAnim( self, SETANIM_BOTH, knockAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			if ( self->s.number >= MAX_CLIENTS )
			{//randomize getup times
				int addTime = Q_irand( -200, 200 );
				self->client->ps.legsTimer += addTime;
				self->client->ps.torsoTimer += addTime;
			}
			else
			{//player holds extra long so you have more time to decide to do the quick getup
				if ( BG_KnockdownAnim( self->client->ps.legsAnim ) )
				{
					self->client->ps.legsTimer += PLAYER_KNOCKDOWN_HOLD_EXTRA_TIME;
					self->client->ps.torsoTimer += PLAYER_KNOCKDOWN_HOLD_EXTRA_TIME;
				}
			}
		}
	}
}

/*
===================================
//Evasion Mechanics
JKG_EvaluateEvasion()
===================================
*/
int G_EvaluateEvasion(gentity_s* targ, gentity_s* attacker, int take)
{
	/*
			Note:
			Would like to make this sort of thing a skill (bounty hunter tree or something), can use rolling to evade attacks.
			Chances for successfully dodging with a roll would depend on player's skill level.  Some stuff shouldn't be evadeable.
			Probably should lower damage, instead of evading entirely as well.  Right now this is just an outline idea that is still fun.
			--Futuza
	*/

		//sample skill calculation:
		int dodgeSkill_level = 3;	//get player's skill level - we'll default to level 3 for now
		float dmgReduction;

		switch (dodgeSkill_level)
		{
		case 0:
			dmgReduction = 0;	//no skill, no dodge allowed
			break;
		case 1:
			dmgReduction = 0.1f;
			break;
		case 2:
			dmgReduction = 0.125f;
			break;
		case 3:
			dmgReduction = 0.15f;	//maximum reduction
			break;
		default:
			dmgReduction = 0;
		}


		bool rolled = false;
		if (dmgReduction > 0)
		{
			if( targ->client->ps.speed > 25 && BG_InRoll(targ->playerState, targ->client->ps.legsAnim)) //check if target is moving & in a roll anim)
			{
				int timing = bgAllAnims[targ->localAnimIndex].anims[targ->client->ps.legsAnim].numFrames * fabs((float)(bgHumanoidAnimations[targ->client->ps.legsAnim].frameLerp));	//get animation timing length
				timing *= 0.5f; //cut in two
				if ((timing + 300 > targ->playerState->legsTimer) && (targ->playerState->legsTimer > timing - 300))		//perfect timing
				{
					trap->SendServerCommand(targ - g_entities, va("notify 1 \"Flawless Dodge!\""));
					take > 2 ? take *= (1 - (dmgReduction * 2)) : take = 1;	//double , or set it to 1
					G_Sound(targ, CHAN_AUTO, G_SoundIndex("sound/weapons/melee/swing4.mp3"));		//play flawless dodge sound
				}
				else
				{
					take > 2 ? take *= (1 - dmgReduction) : take = 1;	//reduce damage by 1/4
					trap->SendServerCommand(targ - g_entities, va("notify 1 \"Dodge!\""));
				}
				rolled = true;
			}
		}
		if (rolled)
		{
			//do dodge efx/plum here

			if (attacker != targ || !attacker->client || attacker->s.number >= MAX_CLIENTS) //notify other players we dodged
				trap->SendServerCommand(attacker - g_entities, va("notify 1 \"%s ^7dodged!\"", targ->client->pers.netname));
		}
		return take;
}



/*
 *	Determines whether damage from attacker to target should trigger a hitmarker
 */
static QINLINE qboolean ShouldHitmarker( const gentity_t *attacker, const gentity_t *target, const int mod )
{
	meansOfDamage_t* means = JKG_GetMeansOfDamage(mod);

	if( !attacker->client )
		return qfalse;
	if( attacker->NPC )
		return qfalse;
	if ( target->health <= 0 )
		return qfalse;
	if( target->s.eType == ET_MISSILE )
		return qfalse;
	if( target->s.eType == ET_GENERAL )
	{
		qboolean explosive = means ? means->hitmarkerExplosives : qfalse;
		if( ( target->s.weapon == WP_TRIP_MINE || target->s.weapon == WP_DET_PACK ) && target->parent == attacker && explosive )
			return qfalse;
	}
	if( !target->client )
	{
		if ( target->s.eType == ET_GENERAL || target->s.eType == WP_TURRET )	//futuza: shouldn't this be || instead of &&?
			return qtrue;
		return qfalse;
	}
	return qtrue;
}

/*
============
G_Heal

Does the exact opposite of G_Damage, in that it heals the target instead of damaging them.
In fact, if G_Damage is called with negative damage, this function will be called instead.

targ		entity being healed
inflictor	entity that is healing the target
healer		entity that caused the inflictor to heal the target

Like G_Damage, we are supplied with dflags and a means of damage.
============
*/
void G_Heal(gentity_t* target, gentity_t* inflictor, gentity_t* healer,
	vec3_t dir, vec3_t point, int heal, int dflags, int mod, meansOfDamage_t* means)		//--Futuza: this function is unfinished, healing/negative values barely work
{
	if (!target)
	{
		return;	// nothing to heal
	}

	if (target->damageRedirect)
	{
		// We are redirecting damage taken, might as well redirect health healed as well
		G_Heal(&g_entities[target->damageRedirectTo], inflictor, healer, dir, point, heal, dflags, mod, means);
		return;
	}

	if (!target->takedamage)
	{
		return;	// target is not allowed to take damage, so it's not allowed to be healed either
	}

	if (!heal)
	{
		// no damage to heal
		return;
	}

	if (heal < 0)
	{
		// we meant to cause damage instead of healing
		G_Damage(target, inflictor, healer, dir, point, -heal, dflags, mod);
		return;
	}

	//if we have less than max health we can be healed!
	if (target->client->ps.stats[STAT_HEALTH] < target->client->ps.stats[STAT_MAX_HEALTH])
	{
		// modify it by the organic or structural modifier for this means
		if (means && heal > 0)
		{
			if (target->client)
			{
				heal *= means->modifiers.organic;
			}
			else
			{
				heal *= means->modifiers.droid;
			}
		}

		//do heal and display it
		int maxheal = target->client->ps.stats[STAT_MAX_HEALTH] - target->client->ps.stats[STAT_HEALTH];
		if (maxheal > heal)
		{
			HealingPlum(target, target->r.currentOrigin, heal);
			target->health += heal;
		}
		else
		{
			HealingPlum(target, target->r.currentOrigin, maxheal);
			target->health = target->client->ps.stats[STAT_MAX_HEALTH];
		}

		target->client->ps.stats[STAT_HEALTH] = target->health;		//update health
																	//if this is non-zero this guy should be updated his s.health to send to the client
		if (target->maxHealth)
			G_ScaleNetHealth(target);

		G_LogWeaponDamage(healer->s.number, mod, heal);

		/*if (target && target->s.eType == ET_NPC)		//healing is not bad
			target->enemy = NULL;*/
	}
}

/*
============
G_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_SHIELD		shield does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

int gPainMOD = 0;
int gPainHitLoc = -1;
vec3_t gPainPoint;
void GLua_NPCEV_OnPain(gentity_t *self, gentity_t *attacker, int damage);

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			ssave;
	int			knockback;
	qboolean	isHeadShot = false;
	meansOfDamage_t* means = JKG_GetMeansOfDamage(mod);


	if (!targ || !targ->inuse)
	{
		// No target to damage.
		return;
	}
	
	if ( !damage )
	{
	    // No damage to deal
	    return;
	}

	if (damage < 0)
	{
		// heal them instead of damaging them
		G_Heal(targ, inflictor, attacker, dir, point, -damage, dflags, mod, means);
		return;
	}

	if (targ->client
		&& targ->s.eType == ET_NPC 
		&& (attacker && (attacker->s.eType == ET_PLAYER || attacker->s.eType == ET_NPC)))
	{// UQ1: Civilians don't take damage from players or other NPCs.

		switch( targ->client->NPC_class)
		{// UQ1: Vendor types... Stand still for now...
		case CLASS_CIVILIAN:
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
		case CLASS_JKG_FAQ_IMP_DROID:
		case CLASS_JKG_FAQ_ALLIANCE_DROID:
		case CLASS_JKG_FAQ_SPY_DROID:
		case CLASS_JKG_FAQ_CRAFTER_DROID:
		case CLASS_JKG_FAQ_MERC_DROID:
		case CLASS_JKG_FAQ_JEDI_MENTOR:
		case CLASS_JKG_FAQ_SITH_MENTOR:
			{
				targ->enemy = NULL; // Make sure civilians never have an enemy... (no AI for it, no weapon for it)

				if (attacker && attacker->s.eType == ET_NPC)
					attacker->enemy = NULL; // Make sure this does not happen again...

				return;
			}
			break;
		default:
			break;
		}
	}

	if (targ->damageRedirect)
	{
		G_Damage(&g_entities[targ->damageRedirectTo], inflictor, attacker, dir, point, damage, dflags, mod);
		return;
	}

	if (!targ->takedamage)
	{
		return;
	}

	if ( (targ->flags&FL_SHIELDED) && mod != MOD_SABER  && !targ->client)
	{//magnetically protected, this thing can only be damaged by lightsabers
		return;
	}

	if ((targ->flags & FL_DMG_BY_SABER_ONLY) && mod != MOD_SABER)
	{ //saber-only damage
		return;
	}

	if (targ->client && targ->client->ps.duelInProgress)
	{
		if (attacker && attacker->client && attacker->s.number != targ->client->ps.duelIndex)
		{
			return;
		}
		else if (attacker && attacker->client && mod != MOD_SABER)
		{
			return;
		}
	}
	if (attacker && attacker->client && attacker->client->ps.duelInProgress)
	{
		if (targ->client && targ->s.number != attacker->client->ps.duelIndex)
		{
			return;
		}
		else if (targ->client && mod != MOD_SABER)
		{
			return;
		}
	}

	if ( !(dflags & DAMAGE_NO_PROTECTION) ) 
	{//rage overridden by no_protection
		if (targ->client && (targ->client->ps.fd.forcePowersActive & (1 << FP_RAGE)))
		{
			damage *= 0.5f;
		}
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}
	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health

	//if genericValue4 == 1 then it's glass or a breakable and those do have health
	if ( targ->s.eType == ET_MOVER && targ->genericValue4 != 1 ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			GlobalUse( targ, inflictor, attacker );
		}
		return;
	}
	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}

	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client && !targ->client->pmfreeze) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		if (mod == MOD_SABER)
		{
			float saberKnockbackScale = g_saberDmgVelocityScale.value;
			VectorScale (dir, (g_knockback.value * (float)knockback / mass)*saberKnockbackScale, kvel);
		}
		else
		{
			VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		}
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		if (attacker && attacker->client && attacker != targ)
		{
			float dur = 5000;
			float dur2 = 100;
			targ->client->ps.otherKiller = attacker->s.number;
			targ->client->ps.otherKillerTime = level.time + dur;
			targ->client->ps.otherKillerDebounceTime = level.time + dur2;
		}
		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time && (g_saberDmgVelocityScale.integer || mod != MOD_SABER ) ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( attacker && targ != attacker)
		{
			if (OnSameTeam (targ, attacker))
			{
				if ( !g_friendlyFire.integer )
				{
					return;
				}
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE) {
			return;
		}

		if (targ && targ->client && (targ->client->ps.eFlags & EF_INVULNERABLE) &&
			attacker && attacker->client && targ != attacker)
		{
			if (targ->client->invulnerableTimer <= level.time)
			{
				targ->client->ps.eFlags &= ~EF_INVULNERABLE;
			}
			else
			{
				return;
			}
		}
	}

	//check for teamnodmg
	//NOTE: non-client objects hitting clients (and clients hitting clients) purposely doesn't obey this teamnodmg (for emplaced guns)
	if ( attacker && !targ->client )
	{//attacker hit a non-client
		if ( level.gametype >= GT_TEAM )
		{
			if ( targ->teamnodmg )
			{//targ shouldn't take damage from a certain team
				if ( attacker->client )
				{//a client hit a non-client object
					if ( targ->teamnodmg == attacker->client->sess.sessionTeam )
					{
						return;
					}
				}
				else if ( attacker->teamnodmg )
				{//a non-client hit a non-client object
					//FIXME: maybe check alliedTeam instead?
					if ( targ->teamnodmg == attacker->teamnodmg )
					{
						if (attacker->activator &&
							attacker->activator->inuse &&
							attacker->activator->s.number < MAX_CLIENTS &&
							attacker->activator->client &&
							attacker->activator->client->sess.sessionTeam != targ->teamnodmg)
						{ //uh, let them damage it I guess.
						}
						else
						{
							return;
						}
					}
				}
			}
		}
	}

	// hitmarker should use same rules as base hit counter
	if( ShouldHitmarker( attacker, targ, mod ) )
	{
		if(attacker->client->lastHitmarkerTime < (level.time-100))
		{
			trap->SendServerCommand(attacker->client->ps.clientNum, "hitmarker");
			attacker->client->lastHitmarkerTime = level.time;
		}
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL
			&& client) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS]--;
		} else {
			attacker->client->ps.persistant[PERS_HITS]++;
		}
		attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health<<8)|(client->ps.stats[STAT_SHIELD]);
	}

	// always give half damage if hurting self - unless DAMAGE_NO_PROTECTION is set
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker  && !(dflags & DAMAGE_NO_PROTECTION)) {
		damage *= 0.5f;
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;  //from this point on want to know diff between damage received and what is actually taken (things like shields, armor, evasion all will reduce it)

	//if roll dodges are allowed (on by default)
	if (jkg_allowDodge.integer > 0 && take > 0 && means->modifiers.dodgeable && targ->client)
	{
		take = G_EvaluateEvasion(targ, attacker, take); //calculate roll dodge reduction
	}

	int directDmg = 0; //direct damage that bypasses shield from ACP weaponry
	if (mod == JKG_GetMeansOfDamageIndex("MOD_ACP") ) //handle division of ACP type damage
	{
		const weaponData_t* weaponData;
		weaponData = GetWeaponData(attacker->client->ps.weapon, attacker->client->ps.weaponVariation);
		float ratio = weaponData->firemodes[attacker->client->ps.firingMode].ACPRatio; //defaults to 0.5

		//clamp range
		if (ratio > 1)
			ratio = 1.0f;
		if (ratio < 0)
			ratio = 0.0f;

		directDmg = take * ratio; //% that is direct (bypasses shields), eg: 30dmg * 0.4 == 12 direct dmg
		take = (1 - ratio) * take; //% that is energy (blocked by shields), eg: 30dmg * 0.6 == 18 dmg to shield (or carry over to direct dmg if no shield)

		//set minimum dmg to 1
		if (take < 1)
			take = 1;
		if (directDmg < 1)
			directDmg = 1;
	}

	///////////////////////////////////////
	//
	//	1. Reduce damage by shield amount
	//	2. Modify damage by location based modifier
	//	3. Reduce damage by armor reduction

	// save some from shield
	if (targ->client && means->modifiers.shieldBlocks && targ->client->ps.stats[STAT_SHIELD] > 0)
	{
		// this damage is -completely avoided- because we're wearing a shield
		ssave = 1;	//setting to 1 for now, to indicate immunity since we can't display 0's or characters with plums yet
		take = 0;
		ShieldHitEffect(targ, dir, ssave);

	}
	else
	{
		ssave = CheckShield(targ, take, dflags, means);
		if (ssave)
		{
			if (targ->client) {
				if (targ->client->ps.stats[STAT_SHIELD] > ssave) {
					// absorb all damage by the shield, and don't take any
					targ->client->ps.stats[STAT_SHIELD] -= ssave;
					take = 0;
				}
				else if (targ->client->ps.stats[STAT_SHIELD] > 0)
				{
					//special exception for when damage ties shield charge
					if (static_cast<int>(take*means->modifiers.shield) == targ->client->ps.stats[STAT_SHIELD])
					{
						targ->client->ps.stats[STAT_SHIELD] = 0;
						take = 0;
					}

					// we have some shield charge but some damage will break through
					else
					{
						take -= targ->client->ps.stats[STAT_SHIELD];
						targ->client->ps.stats[STAT_SHIELD] = 0;

						if (take < 1)			//this isn't just safety checking, in the event that the means does extra damage to shields, ssave will be > take, so we'd end up a negative spill over.
							take = -take;	  //eg: 27 shield, 26 take (multipled to 41 shield dmg reduced to 27) == 26-27 == -1, uh oh!  no worries, math is still correct, just reverse the negative
					}
					
				}
			}
			ShieldHitEffect(targ, dir, ssave);
		}
	}

	take = take + directDmg;	//apply direct damage in case the shield was bypassed


	if (take > 0 && !(dflags&DAMAGE_NO_HIT_LOC))
	{//see if we should modify it by damage location
		if (targ->client &&
			attacker->inuse && attacker->client)
		{ //check for location based damage stuff.
			isHeadShot = G_LocationBasedDamageModifier(targ, point, mod, dflags, &take, means);
			if (targ->health < 0)
				isHeadShot = qfalse;	//don't notify about headshots on corpses
		}
	}

	// modify it by the organic or structural modifier for this means
	if (means && take > 0) {
		if (client)
		{
			take *= means->modifiers.organic;
		}
		else
		{
			take *= means->modifiers.droid;
		}
	}

	// save some from being resistant (eg: carbonite protects us)
	if (targ->client && take > 0 && JKG_HasResistanceBuff(targ->playerState))
	{
		take = take / 2; //reduce damage by 1/2
		if (take < 1)
			take = 1;
	}

	//apply EMP effects from electric damage
	if (take && targ->client && means->modifiers.isEMP)
	{
		//short out jetpacks
		if (targ->client->ps.eFlags & EF_JETPACK_ACTIVE)
			Jetpack_Off(targ);

		//--Futuza: note that a better version of EMP is available through the standard-emp debuff, this only shorts stuff out once
		//see how it interacts with jetpacks in jkg_equip.cpp ItemUse_Jetpack()
	}

#ifndef FINAL_BUILD
	if ( g_debugDamage.integer ) {
		trap->Print( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
			targ->health, take, ssave );
	}
#endif

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_shield += ssave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}

		if (attacker && attacker->client)
		{
			BotDamageNotification(client, attacker);
		}
		else if (inflictor && inflictor->client)
		{
			BotDamageNotification(client, inflictor);
		}
	}

	// See if it's the player hurting the emeny flag carrier
	if( level.gametype == GT_CTF ) {
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// note, Force Protect doesn't do anything (I removed the code)

	if (( ssave || take ) && targ->client && !targ->NPC )
	{
		targ->client->pers.partyUpdate = qtrue;
	}

	// do the damage
	if (take || ssave) 
	{
		// Display actual damage sustained
		if ( targ->health > 0 )
		{
			if ( !point || ( dflags & DAMAGE_RADIUS ))
			{
				DamagePlum(targ, targ->r.currentOrigin, take, mod, ssave, take <= (damage / 4));
			}
			else
			{
				DamagePlum(targ, point, take, mod, ssave, take <= (damage / 4));
			}
		}
		// -----------------------

		targ->health = targ->health - take;

		if ( (targ->flags&FL_UNDYING) )
		{//take damage down to 1, but never die
			if ( targ->health < 1 )
			{
				targ->health = 1;
			}
		}

		//don't bother with negative health  //--Futuza: for some reason if this isn't present disintegration sometimes doesn't happen for high damage?
		if (targ->health < 0)
		{
			targ->health = -10;	
		}

		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

		//We want to go ahead and set gPainHitLoc regardless of if we have a pain func,
		//so we can adjust the location damage too.
		if (targ->client && targ->ghoul2 && targ->client->g2LastSurfaceTime == level.time)
		{ //We updated the hit surface this frame, so it's valid.
			char hitSurface[MAX_QPATH];

			trap->G2API_GetSurfaceName(targ->ghoul2, targ->client->g2LastSurfaceHit, 0, hitSurface);

			if (hitSurface[0])
			{
				G_GetHitLocFromSurfName(targ, hitSurface, &gPainHitLoc, point, dir, vec3_origin, mod);
			}
			else
			{
				gPainHitLoc = -1;
			}

			if (gPainHitLoc < HL_MAX && gPainHitLoc >= 0 && targ->locationDamage[gPainHitLoc] < Q3_INFINITE &&
				(targ->s.eType == ET_PLAYER || targ->s.NPC_class != CLASS_VEHICLE))
			{
				targ->locationDamage[gPainHitLoc] += take;

				if (g_armBreakage.integer && !targ->client->ps.brokenLimbs &&
					targ->client->ps.stats[STAT_HEALTH] > 0 && targ->health > 0 &&
					!(targ->s.eFlags & EF_DEAD))
				{ //check for breakage
					if (targ->locationDamage[HL_ARM_RT]+targ->locationDamage[HL_HAND_RT] >= 80)
					{
						G_BreakArm(targ, BROKENLIMB_RARM);
					}
					else if (targ->locationDamage[HL_ARM_LT]+targ->locationDamage[HL_HAND_LT] >= 80)
					{
						G_BreakArm(targ, BROKENLIMB_LARM);
					}
				}
			}
		}
		else
		{
			gPainHitLoc = -1;
		}

		if (targ->maxHealth)
		{ //if this is non-zero this guy should be updated his s.health to send to the client
			G_ScaleNetHealth(targ);
		}

		if ( targ->health <= 0 ) {
			if ( client )
			{
				targ->flags |= FL_NO_KNOCKBACK;

				if (point)
				{
					VectorCopy( point, targ->pos1 );
				}
				else
				{
					VectorCopy(targ->client->ps.origin, targ->pos1);
				}

				//notify of headshot --futuza
				if (isHeadShot)
				{
					if (attacker == targ)
					{
						trap->SendServerCommand(targ - g_entities, va("chat 100 \"Killed by your own head blow!\""));
					}

					else
					{
						trap->SendServerCommand(targ - g_entities, va("chat 100 \"Killed by %s^7's head blow!\"", attacker->client->pers.netname));
						trap->SendServerCommand(attacker - g_entities, va("notify 1 \"Head blow!\""));
					}
				}
			}
			else if (targ->s.eType == ET_NPC)
			{ //g2animent
				VectorCopy(point, targ->pos1);
			}

			if (targ->health < -999)
				targ->health = -999;

			// If we are a breaking glass brush, store the damage point so we can do cool things with it.
			if ( targ->r.svFlags & SVF_GLASS_BRUSH )
			{
				VectorCopy( point, targ->pos1 );
				if (dir)
				{
					VectorCopy( dir, targ->pos2 );
				}
				else
				{
					VectorClear(targ->pos2);
				}
			}

			if (targ->s.eType == ET_NPC &&
				targ->client &&
				(targ->s.eFlags & EF_DEAD))
			{ //an NPC that's already dead. Maybe we can cut some more limbs off!
				if ( mod == MOD_SABER //saber or heavy melee (claws)
					&& take > 2
					&& !(dflags&DAMAGE_NO_DISMEMBER) )
				{
					G_CheckForDismemberment(targ, attacker, targ->pos1, take, targ->client->ps.torsoAnim, qtrue);
				}
			}

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			G_ActivateBehavior( targ, BSET_DEATH );
			return;
		} 
		else 
		{
			if ( g_debugMelee.integer )
			{//getting hurt makes you let go of the wall
				if ( targ->client && (targ->client->ps.pm_flags&PMF_STUCK_TO_WALL) )
				{
					G_LetGoOfWall( targ );
				}
			}
			if ( targ->pain ) 
			{
				if (targ->s.eType != ET_NPC || mod != MOD_SABER || take > 1)
				{ //don't even notify NPCs of pain if it's just idle saber damage
					gPainMOD = mod;
					if (point)
					{
						VectorCopy(point, gPainPoint);
					}
					else
					{
						VectorCopy(targ->r.currentOrigin, gPainPoint);
					}
					// If it a Lua NPC, do the OnPain event
					if (targ->NPC && targ->NPC->isLuaNPC) {
						GLua_NPCEV_OnPain(targ, attacker, damage);
					}
					targ->pain (targ, attacker, take);
				}
			}
		}

		G_LogWeaponDamage(attacker->s.number, mod, take);
	}
	if(attacker->client && !attacker->NPC)
	{
		if(targ->client && !targ->NPC && !OnSameTeam(attacker, targ))
		{
			// Add an assist to the records
			entityHitRecord_t hitrecord{ attacker, level.time, take };
			qboolean bAdded = qfalse;

			// If we have an assist record by this person already, then we need to add the damage
			for (auto it = targ->assists->begin(); it != targ->assists->end(); ++it) {
				if (it->entWhoHit == attacker) {
					it->damageDealt += take;
					bAdded = qtrue;
					break;
				}
			}

			// We didn't have an assist by this person already, go ahead and add an assist
			if (!bAdded) {
				targ->assists->push_back(hitrecord);
			}
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5f, midpoint);

	VectorCopy (midpoint, dest);
	trap->Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0);
	if (tr.fraction == 1.0f || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0f;
	dest[1] += 15.0f;
	trap->Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0);
	if (tr.fraction == 1.0f)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0f;
	dest[1] -= 15.0f;
	trap->Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0);
	if (tr.fraction == 1.0f)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0f;
	dest[1] += 15.0f;
	trap->Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0);
	if (tr.fraction == 1.0f)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0f;
	dest[1] -= 15.0f;
	trap->Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0);
	if (tr.fraction == 1.0f)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, gentity_t *missile, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap->EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0f - dist / radius );

		if( CanDamage (ent, origin) ) {
			hitClient = qtrue;

			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}

//Stoiss/Scooper add
void G_DelayedDisintegrate( gentity_t *ent )
{
	// Disintgrate the player	
	VectorClear( ent->client->ps.lastHitLoc );
	VectorClear( ent->client->ps.velocity );

	ent->client->ps.eFlags	|= EF_DISINTEGRATION;
	ent->r.contents = 0;

	ent->think = G_FreeEntity;
	ent->nextthink = level.time;
}
//Stoiss/Scooper end
