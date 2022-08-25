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

// g_local.h -- local definitions for game module
#ifndef G_LOCAL_H
#define G_LOCAL_H

#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "g_public.h"
#include "bg_ammo.h"
#include "bg_items.h"
#include "bg_damage.h"
#include "bg_buffs.h"

#include "qcommon/game_version.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

typedef struct gentity_s gentity_t;
typedef struct gclient_s gclient_t;

//npc stuff
#include "b_public.h"

extern int gPainMOD;
extern int gPainHitLoc;
extern vec3_t gPainPoint;

//==================================================================

// the "gameversion" client command will print this plus compile date
#define SECURITY_LOG "security.log"

#define BODY_QUEUE_SIZE		64

#ifndef INFINITE
#define INFINITE			1000000
#endif

#define	FRAMETIME			100					// msec

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	5000

// Jedi Knight Galaxies - Team Party Slots
#define PARTY_SLOT_EMPTY	64
#define PARTY_SLOT_MEMBERS	 5
#define PARTY_SLOT_MAX		64
#define PARTY_SLOT_INVITES	 5

//primarily used by NPCs
#define	START_TIME_LINK_ENTS		FRAMETIME*1 // time-delay after map start at which all ents have been spawned, so can link them
#define	START_TIME_FIND_LINKS		FRAMETIME*2 // time-delay after map start at which you can find linked entities
#define	START_TIME_MOVERS_SPAWNED	FRAMETIME*2 // time-delay after map start at which all movers should be spawned
#define	START_TIME_REMOVE_ENTS		FRAMETIME*3 // time-delay after map start to remove temporary ents
#define	START_TIME_NAV_CALC			FRAMETIME*4 // time-delay after map start to connect waypoints and calc routes
#define	START_TIME_FIND_WAYPOINT	FRAMETIME*5 // time-delay after map start after which it's okay to try to find your best waypoint

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots
#define FL_FORCE_GESTURE		0x00008000	// force gesture on client
#define FL_INACTIVE				0x00010000	// inactive
#define FL_NAVGOAL				0x00020000	// for npc nav stuff
#define	FL_DONT_SHOOT			0x00040000
#define FL_SHIELDED				0x00080000
#define FL_UNDYING				0x00100000	// takes damage down to 1, but never dies
#define FL_NO_DEBUFF			0x00200000	// target is immune to being debuffed
#define FL_BUSYMODE				0x00400000	// target is busy with something else (eg: Pazaak, trading, etc.)

//ex-eFlags -rww
#define	FL_BOUNCE				0x00100000		// for missiles
#define	FL_BOUNCE_HALF			0x00200000		// for missiles
#define	FL_BOUNCE_SHRAPNEL		0x00400000		// special shrapnel flag

//breakable flags -rww
#define FL_DMG_BY_SABER_ONLY		0x01000000 //only take dmg from saber
#define FL_DMG_BY_HEAVY_WEAP_ONLY	0x02000000 //only take dmg from explosives

#define FL_BBRUSH					0x04000000 //I am a breakable brush

#ifndef FINAL_BUILD
#define DEBUG_SABER_BOX
#endif

// make sure this matches game/match.h for botlibs
#define EC "\x19"

#define	MAX_G_SHARED_BUFFER_SIZE		8192
// used for communication with the engine
typedef union sharedBuffer_u {
	char							raw[MAX_G_SHARED_BUFFER_SIZE];
	T_G_ICARUS_PLAYSOUND			playSound;
	T_G_ICARUS_SET					set;
	T_G_ICARUS_LERP2POS				lerp2Pos;
	T_G_ICARUS_LERP2ORIGIN			lerp2Origin;
	T_G_ICARUS_LERP2ANGLES			lerp2Angles;
	T_G_ICARUS_GETTAG				getTag;
	T_G_ICARUS_LERP2START			lerp2Start;
	T_G_ICARUS_LERP2END				lerp2End;
	T_G_ICARUS_USE					use;
	T_G_ICARUS_KILL					kill;
	T_G_ICARUS_REMOVE				remove;
	T_G_ICARUS_PLAY					play;
	T_G_ICARUS_GETFLOAT				getFloat;
	T_G_ICARUS_GETVECTOR			getVector;
	T_G_ICARUS_GETSTRING			getString;
	T_G_ICARUS_SOUNDINDEX			soundIndex;
	T_G_ICARUS_GETSETIDFORSTRING	getSetIDForString;
} sharedBuffer_t;
extern sharedBuffer_t gSharedBuffer;

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

#define SP_PODIUM_MODEL		"models/mapobjects/podium/podium4.md3"

typedef enum
{
	HL_NONE = 0,
	HL_FOOT_RT,
	HL_FOOT_LT,
	HL_LEG_RT,
	HL_LEG_LT,
	HL_WAIST,
	HL_BACK_RT,
	HL_BACK_LT,
	HL_BACK,
	HL_CHEST_RT,
	HL_CHEST_LT,
	HL_CHEST,
	HL_ARM_RT,
	HL_ARM_LT,
	HL_HAND_RT,
	HL_HAND_LT,
	HL_HEAD,
	HL_GENERIC1,
	HL_GENERIC2,
	HL_GENERIC3,
	HL_GENERIC4,
	HL_GENERIC5,
	HL_GENERIC6,
	HL_MAX
} hitLocation_t;

// The assist kill system is set up so that people can see who hit them, and when, within one life.

// Amount of time before a person who is given an assist is no longer considered
// able to be awarded the assist, due to the amount of time between damage
#define ASSIST_LAST_TIME	180000		// 3 minutes
// Duration of time to check for proper kill awarding when falling off of ledges
#define ASSIST_FALL_TIME	20000		// 20 seconds

typedef struct {
	gentity_t *entWhoHit;
	int timeHit;
	int damageDealt;
} entityHitRecord_t;

typedef struct {
	int endTime;
	int lastDamageTime;
	gentity_t* buffer;
} buffInfo_t;

typedef struct Dimension {
	int dNum { 0 };	//negative: isolated from all, 0: global/default, 1+: isolated instance
	std::vector<bool> properties{ 0, 0, 0, 0, 0, 0}; //controls which property to isolate
	/*
	  0 = damage,
	  1 = collision (knockback, movement, etc),
	  2 = interact (pazaak, trading, etc), 
	  3 = visual (draw, efx, etc),
	  4 = audio (footsteps, pain etc),
	  5 = chat (local chat)

	  usage example: 

	  for(int i = 0; i<5; i++)
		properties.at(i) = 1; //isolates all known properties

	  Note that vector<bool> is bitpacked and not really a vector of 'bools'.
	*/
} dimension_t;

#ifdef _GAME
class TreasureClass;
#endif

//============================================================================
extern void *precachedKyle;
extern void *g2SaberInstance;

extern qboolean gEscaping;
extern int gEscapeTime;

//============================================================================

struct gentity_s {
	//rww - entstate must be first, to correspond with the bg shared entity structure
	entityState_t	s;				// communicated by server to clients
	playerState_t	*playerState;	//ptr to playerstate if applicable (for bg ents)
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//From here up must be the same as centity_t/bgEntity_t

	entityShared_t	r;				// shared by both the server system and game

	//rww - these are shared icarus things. They must be in this order as well in relation to the entityshared structure.
	int				taskID[NUM_TIDS];
	parms_t			*parms;
	char			*behaviorSet[NUM_BSETS];
	char			*script_targetname;
	int				delayScriptTime;
	char			*fullName;

	//rww - targetname and classname are now shared as well. ICARUS needs access to them.
	char			*targetname;
	char			*classname;			// set in QuakeEd

	//rww - and yet more things to share. This is because the nav code is in the exe because it's all C++.
	int				waypoint;			//Set once per frame, if you've moved, and if someone asks
	int				lastWaypoint;		//To make sure you don't double-back
	int				lastValidWaypoint;	//ALWAYS valid -used for tracking someone you lost
	int				noWaypointTime;		//Debouncer - so don't keep checking every waypoint in existance every frame that you can't find one
	int				combatPoint;
	int				failedWaypoints[MAX_FAILED_NODES];
	int				failedWaypointCheckTime;

	int				next_roff_time; //rww - npc's need to know when they're getting roff'd

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client

	gNPC_t		*NPC;//Only allocated if the entity becomes an NPC
	int			cantHitEnemyCounter;//HACK - Makes them look for another enemy on the same team if the one they're after can't be hit

	qboolean	noLumbar; //see note in cg_local.h

	qboolean	inuse;

	int			lockCount; //used by NPCs

	int			spawnflags;			// set in QuakeEd

	int			teamnodmg;			// damage will be ignored if it comes from this team

	char		*roffname;			// set in QuakeEd
	char		*rofftarget;		// set in QuakeEd

	char		*healingclass; //set in quakeed
	char		*healingsound; //set in quakeed
	int			healingrate; //set in quakeed
	int			healingDebounce; //debounce for generic object healing shiz

	char		*ownername;

	int			objective;
	int			side;

	int			passThroughNum;		// set to index to pass through (+1) for missiles

	int			aimDebounceTime;
	int			painDebounceTime;
	int			attackDebounceTime;
	int			alliedTeam;			// only useable by this team, never target this team

	int			roffid;				// if roffname != NULL then set on spawn

	qboolean	neverFree;			// if true, FreeEntity will only unlink
									// bodyque uses this

	int			flags;				// FL_* variables

	char		*model;
	char		*model2;
	int			freetime;			// level.time when the object was freed

	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
	qboolean	freeAfterEvent;
	qboolean	unlinkAfterEvent;
	qboolean	tempEntity;

	qboolean	physicsObject;		// if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
	int			clipmask;			// brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

//Only used by NPC_spawners
	char		*NPC_type;
	char		*NPC_targetname;
	char		*NPC_target;

	// movers
	moverState_t moverState;
	int			soundPos1;
	int			sound1to2;
	int			sound2to1;
	int			soundPos2;
	int			soundLoop;
	gentity_t	*parent;
	gentity_t	*nextTrain;
	gentity_t	*prevTrain;
	vec3_t		pos1, pos2;

	//for npc's
	vec3_t		pos3;

	char		*message;

	int			timestamp;		// body queue sinking, etc

	float		angle;			// set in editor, -1 = up, -2 = down
	char		*target;
	char		*target2;
	char		*target3;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target4;		//For multiple targets, not used for firing/triggering/using, though, only for path branches
	char		*target5;		//mainly added for siege items
	char		*target6;		//mainly added for siege items

	char		*team;
	char		*targetShaderName;
	char		*targetShaderNewName;
	gentity_t	*target_ent;

	char		*closetarget;
	char		*opentarget;
	char		*paintarget;

	char		*goaltarget;
	char		*idealclass;

	float		radius;

	int			maxHealth; //used as a base for crosshair health display

	float		speed;
	vec3_t		movedir;
	float		mass;
	int			setTime;

//Think Functions
	int			nextthink;
	// Jedi Knight Galaxies
	void		(*remove)(gentity_t *self);
	// ------------
	void		(*think)(gentity_t *self);
	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
	void		(*blocked)(gentity_t *self, gentity_t *other);
	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

	int			pain_debounce_time;
	int			fly_sound_debounce_time;	// wind tunnel
	int			last_move_time;

//Health and damage fields
	int			health;
	qboolean	takedamage;
	material_t	material;

	int			damage;
	int			dflags;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	int			locationDamage[HL_MAX];		// Damage accumulated on different body locations

	int			count;
	int			bounceCount;
	qboolean	alt_fire;
	trType_t	teamMoveType;			// JKG: ET_MOVER only, if non-0, use this trtype for movement of linked ents

	gentity_t	*chain;
	gentity_t	*enemy;
	gentity_t	*lastEnemy;
	gentity_t	*activator;
	gentity_t	*teamchain;		// next entity in team
	gentity_t	*teammaster;	// master of the team

	int			watertype;
	int			waterlevel;

	int			noise_index;

	// timing variables
	float		wait;
	float		random;
	int			delay;

	//generic values used by various entities for different purposes.
	int			genericValue1;
	int			genericValue2;
	int			genericValue3;
	int			genericValue4;
	int			genericValue5;
	int			genericValue6;
	int			genericValue7;
	int			genericValue8;
	int			genericValue9;
	int			genericValue10;
	int			genericValue11;
	int			genericValue12;
	int			genericValue13;
	int			genericValue14;
	int			genericValue15;

	char		*soundSet;

	qboolean	isSaberEntity;

	int			damageRedirect; //if entity takes damage, redirect to..
	int			damageRedirectTo; //this entity number

	vec3_t		epVelocity;
	float		epGravFactor;

	gitem_t		*item;			// for bonus items

	// OpenJK add
	int			useDebounceTime;	// for cultist_destroyer
	
	// Jedi Knight Galaxies
	int			IDCode;			// Used for GLua to identify old ent references
	int			UsesELS;		// Whether or not the ent uses entity local storage, set to 1 once any access to the table has been made
	int			LuaUsable;		// Whether lua can use this (when inuse is false)
	qboolean	isLogical;		// Determines if this ent is logical or not
	gentity_t   *damagePlum;
	int			damagePlumTime;
	int			lastHealTime;
	buffInfo_t	buffData[PLAYERBUFF_BITS];
	dimension_t dimension;		// Used by player isolation to determine if player's can interact with each other (see jkg_playerisolation.cpp)

	int			grenadeCookTime;	// For cookable grenades.
	int			grenadeWeapon;		// The cookable grenade type that has been set (it can explode in your pocket).
	int			grenadeVariation;	// The cookable grenade variation that has been set (it can explode in your pocket).

	char		treasureclass[MAX_QPATH];

	float		decayRate;			// Helps determine how much dmg should be lost when firing beyond the weapon's range.
	float		maxRange;			// The weapon's absolute maximum range (weapon does 0 dmg at this range). 0 == no decayRate
	float		range;				// The weapon's recommended range (firing beyond this reduces damage)

	// For scripted NPCs
	char		*npcscript;

	// For NPC waypoint following..
	int			wpCurrent;
	int			wpNext;
	int			wpLast;
	int			wpSeenTime;
	int			wpTravelTime;
	int			longTermGoal; // For ASTAR pathing NPCs...
	int			coverpointGoal; // Coverpoint Waypoint...
	int			coverpointOFC; // Coverpoint Out-From-Cover Waypoint...
	int			coverpointOFCtime; // Coverpoint Out-From-Cover timer...
	int			coverpointHIDEtime; // Coverpoint HIDE timer...
	int			pathsize; // For ASTAR pathing NPCs...
	int			pathlist[MAX_WPARRAY_SIZE]; // For ASTAR pathing NPCs...
	float		patrol_range;
	qboolean	return_home;
	vec3_t		spawn_pos;
	vec3_t		move_vector;
	int			bot_strafe_left_timer;
	int			bot_strafe_right_timer;
	int			bot_strafe_crouch_timer;
	int			bot_strafe_jump_timer;
	int			bot_strafe_target_timer;
	vec3_t		bot_strafe_target_position;
	int			npc_mover_start_pos;
	int			npc_dumb_route_time;

	int			next_weapon_switch;
	int			next_rifle_butt_time;
	int			next_flamer_time;
	int			next_kick_time;

	qboolean	bVendor;

	/////////////////////////////////////////
	// 
	// EVERYTHING ABOVE THIS POINT MUST BE POD
	//
	/////////////////////////////////////////
	std::vector<itemInstance_t>* inventory;
	std::vector<entityHitRecord_t>* assists;
};

//used for objective dependancy stuff
#define		MAX_OBJECTIVES			6

//max allowed objective dependancy
#define		MAX_OBJECTIVEDEPENDANCY	6

//TAB bot orders/tactical options
typedef enum botOrder_e {
	BOTORDER_NONE,  //no order
	BOTORDER_KNEELBEFOREZOD,  //Kneel before the ordered person
	BOTORDER_SEARCHANDDESTROY,	//Attack mode.  If given an entity the bot will search for
							  //and then attack that entity.  If NULL, the bot will just
							  //hunt around and attack enemies.
	BOTORDER_OBJECTIVE,	//Do objective play for seige.  Bot will defend or attack objective
						//based on who's objective it is.
	BOTORDER_SIEGECLASS_INFANTRY,
	BOTORDER_SIEGECLASS_VANGUARD,
	BOTORDER_SIEGECLASS_SUPPORT,
	BOTORDER_SIEGECLASS_JEDI,
	BOTORDER_SIEGECLASS_DEMOLITIONIST,
	BOTORDER_SIEGECLASS_HEAVY_WEAPONS,
	BOTORDER_MAX
} botOrder_t;

typedef enum objectiveType_e {
	OT_NONE,	//no OT selected or bad OT
	OT_ATTACK,	//Attack this objective, for destroyable stationary objectives
	OT_DEFEND,  //Defend this objective, for destroyable stationary objectives 
	//or touch objectives
	OT_CAPTURE,  //Capture this objective
	OT_DEFENDCAPTURE,  //prevent capture of this objective
	OT_TOUCH,
	OT_VEHICLE,  //get this vehicle to the related trigger_once.
	OT_WAIT		//This is used by the bots to while they are waiting for a vehicle to respawn
	
} objectiveType_t;

#define DAMAGEREDIRECT_HEAD		1
#define DAMAGEREDIRECT_RLEG		2
#define DAMAGEREDIRECT_LLEG		3

typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

typedef enum {
	SPECTATOR_NOT,
	SPECTATOR_FREE,
	SPECTATOR_FOLLOW,
	SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
	TEAM_BEGIN,		// Beginning a team game, spawn at base
	TEAM_ACTIVE		// Now actively playing
} playerTeamStateState_t;

typedef struct playerTeamState_s {
	playerTeamStateState_t	state;
	int			location;
	float		lasthurtcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct clientSession_s {
	team_t		sessionTeam;
	int			spectatorNum;		// for determining next-in-line to play
	spectatorState_t	spectatorState;
	int			spectatorClient;	// for chasecam and follow mode
	int			wins, losses;		// tournament stats
	int			selectedFP;			// check against this, if doesn't match value in playerstate then update userinfo
	int			saberLevel;			// similar to above method, but for current saber attack level
	qboolean	setForce;			// set to true once player is given the chance to set force powers
	int			updateUITime;		// only update userinfo for FP/SL if < level.time
	char		saberType[64];
	char		saber2Type[64];
	int			duelTeam;
	int			siegeDesiredTeam;

	char		IP[NET_ADDRSTRMAXLEN];
	
	// JKGalaxies additions
	int			connTime;		// Time of connection
	int			noq3fill;		// Set to 1 after connection activity check
	int			validated;		// 1 if the client-side validation was received
	/* Admin system */
	int			adminRank;

	bool		canUseCheats;
} clientSession_t;

// playerstate mGameFlags
#define	PSG_VOTED				(1<<0)		// already cast a vote
#define PSG_TEAMVOTED			(1<<1)		// already cast a team vote

//
#define MAX_NETNAME			36				//--futuza:  making this bigger Max_QPATH size?
#define	MAX_VOTE_COUNT		3

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct clientPersistant_s {
	clientConnected_t	connected;
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	char		netname_nocolor[MAX_NETNAME];
	int			netnameTime;				// Last time the name was changed
	int			maxHealth;			// for handicapping
	int			maxStamina;
	int			enterTime;			// level.time the client entered the game
	playerTeamState_t teamState;	// status in teamplay games
	int			voteCount;			// to prevent people from constantly calling votes
	int			teamVoteCount;		// to prevent people from constantly calling votes
	qboolean	teamInfo;			// send team overlay updates?

	int			connectTime;	
	int			lastCreditTime;		// last time we got passive credit reward

	// Jedi Knight Galaxies
	int			partyNumber;						// Your party index in the level.party struct.
	int			partyIndex;							// Your index in the level.party[party] struct.
	int			partyInvite[PARTY_SLOT_INVITES];	// Your pending invites, you may accept/decline these.
	int			partyManagement;					// Set to a time when party management panel is on!
	qboolean	partyUpdate;						// Set to true when in need of an update in the next second (health/armor/etc)!

	// Admin flags
	qboolean	silenced;

	char		saber1[MAX_QPATH], saber2[MAX_QPATH];

	char		account[MAX_QPATH];

	// Custom team
	int			customteam;					// Enables icons above the player's heads, for games like the arena

	int			vote, teamvote; // 0 = none, 1 = yes, 2 = no

	char		guid[33];
} clientPersistant_t;

typedef struct renderInfo_s
{
	//In whole degrees, How far to let the different model parts yaw and pitch
	int		headYawRangeLeft;
	int		headYawRangeRight;
	int		headPitchRangeUp;
	int		headPitchRangeDown;

	int		torsoYawRangeLeft;
	int		torsoYawRangeRight;
	int		torsoPitchRangeUp;
	int		torsoPitchRangeDown;

	int		legsFrame;
	int		torsoFrame;

	float	legsFpsMod;
	float	torsoFpsMod;

	//Fields to apply to entire model set, individual model's equivalents will modify this value
	vec3_t	customRGB;//Red Green Blue, 0 = don't apply
	int		customAlpha;//Alpha to apply, 0 = none?

	//RF?
	int			renderFlags;

	//
	vec3_t		muzzlePoint;
	vec3_t		muzzleDir;
	vec3_t		muzzlePointOld;
	vec3_t		muzzleDirOld;
	//vec3_t		muzzlePointNext;	// Muzzle point one server frame in the future!
	//vec3_t		muzzleDirNext;
	int			mPCalcTime;//Last time muzzle point was calced

	//
	float		lockYaw;//

	//
	vec3_t		headPoint;//Where your tag_head is
	vec3_t		headAngles;//where the tag_head in the torso is pointing
	vec3_t		handRPoint;//where your right hand is
	vec3_t		handLPoint;//where your left hand is
	vec3_t		crotchPoint;//Where your crotch is
	vec3_t		footRPoint;//where your right hand is
	vec3_t		footLPoint;//where your left hand is
	vec3_t		torsoPoint;//Where your chest is
	vec3_t		torsoAngles;//Where the chest is pointing
	vec3_t		eyePoint;//Where your eyes are
	vec3_t		eyeAngles;//Where your eyes face
	int			lookTarget;//Which ent to look at with lookAngles
	lookMode_t	lookMode;
	int			lookTargetClearTime;//Time to clear the lookTarget
	int			lastVoiceVolume;//Last frame's voice volume
	vec3_t		lastHeadAngles;//Last headAngles, NOT actual facing of head model
	vec3_t		headBobAngles;//headAngle offsets
	vec3_t		targetHeadBobAngles;//head bob angles will try to get to targetHeadBobAngles
	int			lookingDebounceTime;//When we can stop using head looking angle behavior
	float		legsYaw;//yaw angle your legs are actually rendering at

	//for tracking legitimate bolt indecies
	void		*lastG2; //if it doesn't match ent->ghoul2, the bolts are considered invalid.
	int			headBolt;
	int			handRBolt;
	int			handLBolt;
	int			torsoBolt;
	int			crotchBolt;
	int			footRBolt;
	int			footLBolt;
	int			motionBolt;

	int			boltValidityTime;
} renderInfo_t;

//EEZSTREET EDIT: STAT STRUCTURE
struct statData_s
{
	//"Real" stats
	int			weight;
};

//Stoiss add
typedef struct 
{
	int EntityNum;
	int Debounce;
	int SaberNum;
	int BladeNum;
}  sabimpact_t;
//Stois end

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;
	clientSession_t		sess;

	saberInfo_t	saber[MAX_SABERS];
	void		*weaponGhoul2[MAX_SABERS];

	int			bodyGrabTime;
	int			bodyGrabIndex;

	int			pushEffectTime;

	int			invulnerableTimer;

	int			saberCycleQueue;

	int			legsAnimExecute;
	int			torsoAnimExecute;
	qboolean	legsLastFlip;
	qboolean	torsoLastFlip;

	qboolean	readyToExit;		// wishes to leave the intermission

	qboolean	noclip;

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	vec3_t		oldOrigin;

	// debounce time for accuracy
	int			accuracyDebounce;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_shield;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			damageBoxHandle_Head; //entity number of head damage box
	int			damageBoxHandle_RLeg; //entity number of right leg damage box
	int			damageBoxHandle_LLeg; //entity number of left leg damage box

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	// JKG
	int			deathcamTime;
	vec3_t		deathcamCenter;
	int			deathcamRadius;
	int			deathcamForceRespawn;

	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams

	int			switchDuelTeamTime;		// time the player switched duel teams

	int			switchClassTime;	// class changed debounce timer

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	char		*areabits;

	int			g2LastSurfaceHit; //index of surface hit during the most recent ghoul2 collision performed on this client.
	int			g2LastSurfaceTime; //time when the surface index was set (to make sure it's up to date)
	//[BUGFIX12]
	int			g2LastSurfaceModel; //the index of the model on the ghoul2 that was hit during the lastest hit.
	//[BUGFIX12]

	int			corrTime;

	vec3_t		lastHeadAngles;
	int			lookTime;

	int			brokenLimbs;

	qboolean	noCorpse; //don't leave a corpse on respawn this time.

	int			jetPackTime;
	int			jetPackToggleTime;
	int			jetPackDebRecharge;
	int			jetPackDebReduce;

	int			cloakToggleTime;
	int			cloakDebRecharge;
	int			cloakDebReduce;

	int			saberStoredIndex; //stores saberEntityNum from playerstate for when it's set to 0 (indicating saber was knocked out of the air)

	int			saberKnockedTime; //if saber gets knocked away, can't pull it back until this value is < level.time

	vec3_t		olderSaberBase; //Set before lastSaberBase_Always, to whatever lastSaberBase_Always was previously
	qboolean	olderIsValid;	//is it valid?

	vec3_t		lastSaberDir_Always; //every getboltmatrix, set to saber dir
	vec3_t		lastSaberBase_Always; //every getboltmatrix, set to saber base
	int			lastSaberStorageTime; //server time that the above two values were updated (for making sure they aren't out of date)

	qboolean	hasCurrentPosition;	//are lastSaberTip and lastSaberBase valid?

	int			dangerTime;		// level.time when last attack occured

	int			idleTime;		//keep track of when to play an idle anim on the client.

	int			idleHealth;		//stop idling if health decreases
	vec3_t		idleViewAngles;	//stop idling if viewangles change

	int			forcePowerSoundDebounce; //if > level.time, don't do certain sound events again (drain sound, absorb sound, etc)

	char		modelname[MAX_QPATH];

	qboolean	fjDidJump;

	qboolean	ikStatus;

	int			throwingIndex;
	int			beingThrown;
	int			doingThrow;

	float		hiddenDist;//How close ents have to be to pick you up as an enemy
	vec3_t		hiddenDir;//Normalized direction in which NPCs can't see you (you are hidden)

	renderInfo_t	renderInfo;

	//mostly NPC stuff:
	npcteam_t	playerTeam;
	npcteam_t	enemyTeam;
	char		*squadname;
	gentity_t	*team_leader;
	gentity_t	*leader;
	gentity_t	*follower;
	int			numFollowers;
	gentity_t	*formationGoal;
	int			nextFormGoal;
	class_t		NPC_class;

	vec3_t		pushVec;
	int			pushVecTime;

	//used in conjunction with ps.hackingTime
	int			isHacking;
	vec3_t		hackingAngles;

	//debounce time for sending extended siege data to certain classes
	int			siegeEDataSend;

	int			ewebIndex; //index of e-web gun if spawned
	int			ewebTime; //e-web use debounce
	int			ewebHealth; //health of e-web (to keep track between deployments)

	int			tempSpectate; //time to force spectator mode

	//keep track of last person kicked and the time so we don't hit multiple times per kick
	int			jediKickIndex;
	int			jediKickTime;

	//special moves (designed for kyle boss npc, but useable by players in mp)
	int			grappleIndex;
	int			grappleState;

	int			solidHack;

	int			noLightningTime;

	unsigned	mGameFlags;

	//fallen duelist
	qboolean	iAmALoser;

	int			lastGenCmd;
	int			lastGenCmdTime;

	struct force {
		int		regenDebounce;
		int		drainDebounce;
		int		lightningDebounce;
	} force;
	
	//Stoiss add
	//the SaberNum of the last enemy blade that you hit.
	int			lastSaberCollided;
	//the BladeNum of the last enemy blade that you hit.
	int			lastBladeCollided;

	sabimpact_t	sabimpact[MAX_SABERS][MAX_BLADES];
	//Stoiss end

	// Jedi Knight Galaxies
	int			IDCode;
	int			InCinematic;
	unsigned short			clipammo[256][MAX_FIREMODES];		// Ammo in current clip of specific weapon
	unsigned short			firingModes[256];					// Different firing modes for each gun so it automagically remembers
	unsigned short			ammoTypes[256][MAX_FIREMODES];		// Different ammo types for each gun so it automagically remembers
	qboolean	customGravity;
	qboolean	pmfreeze;
	qboolean	pmlock;
	qboolean	pmnomove;
	qboolean	noDismember;
	qboolean	noDisintegrate;
	qboolean	noDrops;					// Supress item drops
	
	// Custom disco messages
	int			customDisconnectMsg;
	
	// Chat flood protection
	int			lastChatMessage;

	struct statData_s	coreStats;
	qboolean	shieldEquipped;
	int			shieldRechargeTime;
	int			shieldRegenTime;
	int			shieldRechargeLast;
	int			shieldRegenLast;
	qboolean	shieldRecharging;	// to make sure that the shield sound doesn't play twice
	qboolean	jetpackEquipped;	//is there a jetpack equipped?
	itemJetpackData_t* pItemJetpack;

	unsigned short ammoTable[MAX_AMMO_TYPES];		// Max ammo indices increased to JKG_MAX_AMMO_INDICES

	unsigned int numDroneShotsInBurst;
	unsigned int lastHitmarkerTime;			// Timer to ensure that we don't get ear-raped whenever we hit someone with a scattergun --eez
	unsigned int storedCredits;				// hack a doodle doo to prevent the credits from spectators from carrying over to people that join
	unsigned int saberStanceDebounce;		// prevent people from spamming style change and causing issues

	int				saberBPDebRecharge;
	unsigned int	saberProjBlockTime;
	unsigned int	saberBlockTime;

	int			weaponHeatDebounceTime;		// last time that we lost heat

	// all of the below was migrated from the playerState. None of it belonged there. --eez
	int saberBlockDebounce;
	int saberAttackWound;
	int saberIdleWound;
	int saberAttackSequence;				// FIXME: not used? --eez
	int saberSaberBlockDebounce;

	unsigned int numKillsThisLife;			// Killstreaks!

	char		botSoundDir[MAX_QPATH];
	float		blockingLightningAccumulation;//Stoiss add
	qboolean	didSaberOffSound;				// eez add
	float		ironsightsBlend;			// only used in ~1 place, but it's used to prevent noscoping

	gentity_t	*currentTrader;				// who we are currently trading with
};

//Interest points

#define MAX_INTEREST_POINTS		64

typedef struct
{
	vec3_t		origin;
	char		*target;
} interestPoint_t;

//Combat points

#define MAX_COMBAT_POINTS		512

typedef struct
{
	vec3_t		origin;
	int			flags;
//	char		*NPC_targetname;
//	team_t		team;
	qboolean	occupied;
	int			waypoint;
	int			dangerTime;
} combatPoint_t;

// Alert events

#define	MAX_ALERT_EVENTS	32

typedef enum
{
	AET_SIGHT,
	AET_SOUND,
} alertEventType_e;

typedef enum
{
	AEL_MINOR,			//Enemy responds to the sound, but only by looking
	AEL_SUSPICIOUS,		//Enemy looks at the sound, and will also investigate it
	AEL_DISCOVERED,		//Enemy knows the player is around, and will actively hunt
	AEL_DANGER,			//Enemy should try to find cover
	AEL_DANGER_GREAT,	//Enemy should run like hell!
} alertEventLevel_e;

typedef struct alertEvent_s
{
	vec3_t				position;	//Where the event is located
	float				radius;		//Consideration radius
	alertEventLevel_e	level;		//Priority level of the event
	alertEventType_e	type;		//Event type (sound,sight)
	gentity_t			*owner;		//Who made the sound
	float				light;		//ambient light level at point
	float				addLight;	//additional light- makes it more noticable, even in darkness
	int					ID;			//unique... if get a ridiculous number, this will repeat, but should not be a problem as it's just comparing it to your lastAlertID
	int					timestamp;	//when it was created
} alertEvent_t;

//
// this structure is cleared as each map is entered
//
typedef struct waypointData_s {
	char	targetname[MAX_QPATH];
	char	target[MAX_QPATH];
	char	target2[MAX_QPATH];
	char	target3[MAX_QPATH];
	char	target4[MAX_QPATH];
	int		nodeID;
} waypointData_t;


typedef struct
{

	unsigned int	 time;
	int				 id;
	char			 message[64];

} teamPartyList_t;

typedef struct {
	char	message[MAX_SPAWN_VARS_CHARS];
	int		count;
	int		cs_index;
	vec3_t	origin;
} locationData_t;

typedef struct level_locals_s {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// current number, <= MAX_GENTITIES
	int			num_logicalents;	// current numner of logical ents, > MAX_GENTIIES, <= MAX_LOGICALENTS

	int			warmupTime;			// restart match at this time

	fileHandle_t	logFile;

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started

	int			teamScores[TEAM_NUM_TEAMS];
	int			lastTeamLocationTime;		// last time of client team location update

	qboolean	newSession;				// don't use any old session data, because
										// we changed gametype

	qboolean	restarted;				// waiting for a map_restart to fire

	int			numConnectedClients;
	int			numNonSpectatorClients;	// includes connecting clients
	int			numPlayingClients;		// connected, non-spectators
	int			sortedClients[MAX_CLIENTS];		// sorted by score
	int			follow1, follow2;		// clientNums for auto-follow spectators

	int			snd_fry;				// sound index for standing in lava

	int			snd_hack;				//hacking loop sound
    int			snd_medHealed;			//being healed by supply class
	int			snd_medSupplied;		//being supplied by supply class

	// voting state
	char		voteString[MAX_STRING_CHARS];
	char		voteStringClean[MAX_STRING_CHARS];
	char		voteDisplayString[MAX_STRING_CHARS];
	int			voteTime;				// level.time vote was called
	int			voteExecuteTime;		// time the vote is executed
	int			voteExecuteDelay;		// set per-vote
	int			voteYes;
	int			voteNo;
	int			numVotingClients;		// set by CalculateRanks

	qboolean	votingGametype;
	int			votingGametypeTo;

	// team voting state
	char		teamVoteString[2][MAX_STRING_CHARS];
	char		teamVoteStringClean[2][MAX_STRING_CHARS];
	char		teamVoteDisplayString[2][MAX_STRING_CHARS];
	int			teamVoteTime[2];		// level.time vote was called
	int			teamVoteExecuteTime[2];		// time the vote is executed
	int			teamVoteYes[2];
	int			teamVoteNo[2];
	int			numteamVotingClients[2];// set by CalculateRanks

	// spawn variables
	qboolean	spawning;				// the G_Spawn*() functions are valid
	int			numSpawnVars;
	char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int			numSpawnVarChars;
	char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

	// intermission state
	int			intermissionQueued;		// intermission was qualified, but
										// wait INTERMISSION_DELAY_TIME before
										// actually going there so the last
										// frag can be watched.  Disable future
										// kills during this delay
	int			intermissiontime;		// time the intermission was started
	char		*changemap;
	qboolean	readyToExit;			// at least one client wants to exit
	int			exitTime;
	vec3_t		intermission_origin;	// also used for spectator spawns
	vec3_t		intermission_angle;

	int			bodyQueIndex;			// dead bodies
	gentity_t	*bodyQue[BODY_QUEUE_SIZE];
	int			portalSequence;

	alertEvent_t	alertEvents[ MAX_ALERT_EVENTS ];
	int				numAlertEvents;
	int				curAlertID;

	AIGroupInfo_t	groups[MAX_FRAME_GROUPS];

	//Interest points- squadmates automatically look at these if standing around and close to them
	interestPoint_t	interestPoints[MAX_INTEREST_POINTS];
	int			numInterestPoints;

	//Combat points- NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point
	combatPoint_t	combatPoints[MAX_COMBAT_POINTS];
	int			numCombatPoints;

	//rwwRMG - added:
	int			mNumBSPInstances;
	int			mBSPInstanceDepth;
	vec3_t		mOriginAdjust;
	float		mRotationAdjust;
	char		*mTargetAdjust;

	char		mTeamFilter[MAX_QPATH];
	// Jedi Knight Galaxies
	int				serverInit;
	char			party[MAX_CLIENTS][5];
	teamPartyList_t	partyList[MAX_CLIENTS];

	struct {
		fileHandle_t	log;
	} security;
	
	int redTeam;								// Red team index, should be equivalent to cgs.redTeam
	int blueTeam;								// Blue team index, should be equivalent to cgs.blueTeam'

	int queriedVendors[32];						// Nothing fancy here, just a hack to restore balance in the universe
	int lastVendorCheck;						// Last time we checked to replenish vendor stock
	int lastVendorUpdateTime;
	int	lastUpdatedVendor;

	char	startingWeapon[MAX_QPATH];			// mega hax here.
												// gives you one of these suckers, and auto equips it to the first slot.

#ifdef __JKG_NINELIVES__
	int		gamestartTime;						// After a five minute or so period of time, disallows joining from outside players
#elif defined __JKG_TICKETING__
	int		gamestartTime;						// After a five minute or so period of time, disallows joining from outside players
#elif defined __JKG_ROUNDBASED__
	int		gamestartTime;						// After a five minute or so period of time, disallows joining from outside players
#endif

	struct {
		int num;
		char *infos[MAX_BOTS];
	} bots;

	struct {
		int num;
		char *infos[MAX_ARENAS];
	} arenas;

	struct {
		int num;
		qboolean linked;
		locationData_t data[MAX_LOCATIONS];
	} locations;

	gametype_t	gametype;
} level_locals_t;

// Warzone Gametype...
typedef struct {
	int			closeWaypoints[256];
	int			num_waypoints;
	gentity_t	*flagentity;
	gentity_t	*redNPCs[32];
	gentity_t	*blueNPCs[32];
	vec3_t		spawnpoints[32];
	vec3_t		spawnangles[32];
	int			num_spawnpoints;
} flag_list_t;

extern flag_list_t flag_list[1024]; 

extern int bluetickets;
extern int redtickets;

//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnBoolean( const char *key, const char *defaultString, qboolean *out );
void		G_SpawnEntitiesFromString( qboolean inSubBSP );
char *G_NewString( const char *string );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);
char	*ConcatArgs( int start );
void StopFollowing( gentity_t *ent );
void BroadcastTeamChange( gclient_t *client, int oldTeam );
void SetTeam( gentity_t *ent, char *s );
void Cmd_FollowCycle_f( gentity_t *ent, int dir );
void Cmd_SaberAttackCycle_f(gentity_t *ent);
void Cmd_ToggleSaber_f(gentity_t *ent);
void Cmd_EngageDuel_f(gentity_t *ent);
void Cmd_Reload_f(gentity_t *ent);

void JKG_BindChatCommands( void );
void CCmd_Cleanup();

//
// g_items.c
//

void Jetpack_Off(gentity_t *ent);
void Jetpack_On(gentity_t *ent);
void ItemUse_Jetpack(gentity_t *ent);

void G_CheckTeamItems( void );
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle );
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity );
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void	Add_Ammo (gentity_t *ent, int weapon, int count);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int		G_ModelIndex( const char *name );
int		G_SoundIndex( const char *name );
int		G_SoundSetIndex(const char *name);
int		G_EffectIndex( const char *name );
int		G_BSPIndex( const char *name );
int		G_IconIndex( const char* name );

qboolean	G_PlayerHasCustomSkeleton(gentity_t *ent);

void	G_TeamCommand( team_t team, char *cmd );
void	G_ScaleNetHealth(gentity_t *self);
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, const char *match);
int		G_RadiusList ( vec3_t origin, float radius,	gentity_t *ignore, qboolean takeDamage, gentity_t *ent_list[MAX_GENTITIES]);

void	G_Throw( gentity_t *targ, vec3_t newDir, float push );

void	G_CreateFakeClient(int entNum, gclient_t **cl);
void	G_CleanAllFakeClients(void);

void	G_SetAnim(gentity_t *ent, usercmd_t *ucmd, int setAnimParts, int anim, int setAnimFlags);
gentity_t *G_PickTarget (char *targetname);
void	GlobalUse(gentity_t *self, gentity_t *other, gentity_t *activator);
void	G_UseTargets2( gentity_t *ent, gentity_t *activator, const char *string );
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);
void	G_SetAngles( gentity_t *ent, vec3_t angles );

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_SpawnLogical(void);
gentity_t *G_TempEntity( const vec3_t origin, int event );
gentity_t	*G_PlayEffect(int fxID, vec3_t org, vec3_t ang);
gentity_t	*G_PlayEffectID(const int fxID, vec3_t org, vec3_t ang);
gentity_t *G_ScreenShake(vec3_t org, gentity_t *target, float intensity, int duration, qboolean global);
void	G_MuteSound( int entnum, int channel );
void	G_Sound( gentity_t *ent, int channel, int soundIndex );
void	G_SoundAtLoc( vec3_t loc, int channel, int soundIndex );
void	G_EntitySound( gentity_t *ent, int channel, int soundIndex );
void	TryUse( gentity_t *ent );
void	G_SendG2KillQueue(void);
void	G_KillG2Queue(int entNum);
void	G_FreeEntity( gentity_t *e );
qboolean	G_EntitiesFree( void );

qboolean G_ActivateBehavior (gentity_t *self, int bset );

void	G_TouchTriggers (gentity_t *ent);
void	GetAnglesForDirection( const vec3_t p1, const vec3_t p2, vec3_t out );

//
// g_object.c
//

extern void G_RunObject			( gentity_t *ent );


float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

void G_AddPredictableEvent( gentity_t *ent, int event, int eventParm );
void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
qboolean G_CheckInSolid (gentity_t *self, qboolean fix);
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig(void);
/*
Ghoul2 Insert Start
*/
int G_BoneIndex( const char *name );

/*
Ghoul2 Insert End
*/

//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, vec3_t origin);
void G_Knockdown( gentity_t *self, gentity_t *attacker, const vec3_t pushDir, float strength, qboolean breakSaberLock );
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
qboolean G_RadiusDamage (vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, gentity_t *missile, int mod);
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void TossClientItems( gentity_t *self );
void G_CheckForDismemberment(gentity_t *ent, gentity_t *enemy, vec3_t point, int damage, int deathAnim, qboolean postDeath);
extern int gGAvoidDismember;

//
// g_exphysics.c
//
void G_RunExPhys(gentity_t *ent, float gravity, float mass, float bounce, qboolean autoKill, int *g2Bolts, int numG2Bolts);

//
// g_missile.c
//
void G_ReflectMissile( gentity_t *ent, gentity_t *missile, vec3_t forward );

void G_RunMissile( gentity_t *ent );

gentity_t *CreateMissile( vec3_t org, vec3_t dir, float vel, int life,
							gentity_t *owner, qboolean altFire);
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout );


//
// g_mover.c
//
extern int	BMS_START;
extern int	BMS_MID;
extern int	BMS_END;

#define SPF_BUTTON_USABLE		1
#define SPF_BUTTON_FPUSHABLE	2
void G_PlayDoorLoopSound( gentity_t *ent );
void G_PlayDoorSound( gentity_t *ent, int type );
void G_RunMover( gentity_t *ent );
void Touch_DoorTrigger( gentity_t *ent, gentity_t *other, trace_t *trace );

//
// g_trigger.c
//
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace );


//
// g_misc.c
//
#define MAX_REFNAME	32
#define	START_TIME_LINK_ENTS		FRAMETIME*1

#define	RTF_NONE	0
#define	RTF_NAVGOAL	0x00000001

typedef struct reference_tag_s
{
	char		name[MAX_REFNAME];
	vec3_t		origin;
	vec3_t		angles;
	int			flags;	//Just in case
	int			radius;	//For nav goals
	qboolean	inuse;
} reference_tag_t;

void TAG_Init( void );
reference_tag_t	*TAG_Find( const char *owner, const char *name );
reference_tag_t	*TAG_Add( const char *name, const char *owner, vec3_t origin, vec3_t angles, int radius, int flags );
int	TAG_GetOrigin( const char *owner, const char *name, vec3_t origin );
int	TAG_GetOrigin2( const char *owner, const char *name, vec3_t origin );
int	TAG_GetAngles( const char *owner, const char *name, vec3_t angles );
int TAG_GetRadius( const char *owner, const char *name );
int TAG_GetFlags( const char *owner, const char *name );

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles );
void TeleportPlayer2( gentity_t *player, vec3_t origin, vec3_t angles );

//
// g_weapon.c
//

void		 WP_CalculateAngles( gentity_t *ent );
void		 WP_CalculateMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
void		 WP_RecalculateTheFreakingMuzzleCrap( gentity_t *ent );
gentity_t	*WP_FireGenericMissile( gentity_t *ent, int firemode, vec3_t origin, vec3_t dir );
void		 WP_FireGenericWeapon( gentity_t *ent, int firemode );
float		 WP_GetWeaponBoxSize( gentity_t *ent, int firemode );
int			 WP_GetWeaponBounce( gentity_t *ent, int firemode );
int			 WP_GetWeaponCharge( gentity_t *ent, int firemode );
char		*WP_GetWeaponClassname( gentity_t *ent, int firemode );
int			 WP_GetWeaponDamage( gentity_t *ent, int firemode );
qboolean	 WP_GetWeaponGravity( gentity_t *ent, int firemode );
int			 WP_GetWeaponMOD( gentity_t *ent, int firemode );
int			 WP_GetWeaponSplashMOD( gentity_t *ent, int firemode );
float		 WP_GetWeaponRange( gentity_t *ent, int firemode );
float		 WP_GetWeaponDecayRate(gentity_t* ent, int firemode);
float		 WP_GetWeaponSpeed( gentity_t *ent, int firemode );
double		 WP_GetWeaponSplashRange( gentity_t *ent, int firemode );

void SnapVectorTowards( vec3_t v, vec3_t to );

//
// g_client.c
//
team_t TeamCount( int ignoreClientNum, int team );
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, team_t team, qboolean isbot );
void MaintainBodyQueue(gentity_t *ent);
void JKG_PermaSpectate(gentity_t *ent);
void respawn (gentity_t *ent);
qboolean JKG_ClientAlive(gentity_t* ent);
void BeginIntermission (void);
void InitBodyQue (void);
void ClientSpawn( gentity_t *ent, qboolean respawn );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, vec3_t origin, int score );
void CalculateRanks( void );
qboolean SpotWouldTelefrag( gentity_t *spot );

//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );
void G_ProcessIPBans(void);
qboolean G_FilterPacket (char *from);

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent, int firingMode );
void BlowDetpacks(gentity_t *ent);

void MoveClientToIntermission (gentity_t *client);
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_main.c
//
extern qboolean gDoSlowMoDuel;
extern int gSlowMoDuelTime;

void G_PowerDuelCount(int *loners, int *doubles, qboolean countSpec);

void FindIntermissionPoint( void );
void G_RunThink (gentity_t *ent);
void AddTournamentQueue(gclient_t *client);
void QDECL G_LogPrintf( const char *fmt, ... );
void QDECL G_SecurityLogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
const char *G_GetStringEdString(char *refSection, char *refName);
const char *G_GetStringEdString2(char *refName);

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
qboolean ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum, qboolean allowTeamReset );
void G_BreakArm(gentity_t *ent, int arm);
void G_UpdateClientAnims(gentity_t *self, float animSpeedScale);
void ClientCommand( int clientNum );
void G_ClearVote( gentity_t *ent );
void G_ClearTeamVote( gentity_t *ent, int team );

//
// g_active.c
//
void G_CheckClientTimeouts	( gentity_t *ent );
void ClientThink			( int clientNum, usercmd_t *ucmd );
void ClientEndFrame			( gentity_t *ent );
void G_RunClient			( gentity_t *ent );

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );
void Team_CheckDroppedItem( gentity_t *dropped );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadClientSessionData( gclient_t *client );
void G_WriteClientSessionData( const gclient_t *client );
void G_InitClientSessionData( gclient_t *client, char *userinfo, qboolean isBot );
void G_ReadSessionData( void );
void G_WriteSessionData( void );

// NPC_spawn.cpp
void NPC_Cleanup();

//
// NPC_senses.cpp
//
extern void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel, float addLight ); //addLight = 0.0f
extern void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel, qboolean needLOS ); //needLOS = qfalse
extern qboolean G_CheckForDanger( gentity_t *self, int alertEvent );
extern int G_CheckAlertEvents( gentity_t *self, qboolean checkSight, qboolean checkSound, float maxSeeDist, float maxHearDist, int ignoreAlert, qboolean mustHaveOwner, int minAlertLevel ); //ignoreAlert = -1, mustHaveOwner = qfalse, minAlertLevel = AEL_MINOR
extern qboolean G_CheckForDanger( gentity_t *self, int alertEvent );
extern qboolean G_ClearLOS( gentity_t *self, const vec3_t start, const vec3_t end );
extern qboolean G_ClearLOS2( gentity_t *self, gentity_t *ent, const vec3_t end );
extern qboolean G_ClearLOS3( gentity_t *self, const vec3_t start, gentity_t *ent );
extern qboolean G_ClearLOS4( gentity_t *self, gentity_t *ent );
extern qboolean G_ClearLOS5( gentity_t *self, const vec3_t end );

//
// g_bot.c
//
void G_InitBots( void );
void G_CleanupBots();
char *G_GetBotInfoByNumber( int num );
char *G_GetBotInfoByName( const char *name );
void G_CheckBotSpawn( void );
void G_RemoveQueuedBotBegin( int clientNum );
qboolean G_BotConnect( int clientNum, qboolean restart );
void Svcmd_AddBot_f( void );
void Svcmd_BotList_f( void );
qboolean G_DoesMapSupportGametype(const char *mapname, int gametype);
const char *G_RefreshNextMap(int gametype, qboolean forced);

extern int ASTAR_FindPath(int from, int to, int *pathlist);
qboolean JKG_CheckRoutingFrom( int wp );
qboolean JKG_CheckBelowWaypoint( int wp );
// w_force.c / w_saber.c
gentity_t *G_PreDefSound(vec3_t org, int pdSound);
void WP_ForcePowerStop( gentity_t *self, forcePowers_t forcePower );
void WP_SaberPositionUpdate( gentity_t *self, usercmd_t *ucmd );
qboolean WP_SaberCanBlock(gentity_t *self, vec3_t point, int dflags, int mod, qboolean projectile, int attackStr);
void WP_SaberInitBladeData( gentity_t *ent );
void WP_InitForcePowers( gentity_t *ent );
void WP_SpawnInitForcePowers( gentity_t *ent );
void WP_ForcePowersUpdate( gentity_t *self, usercmd_t *ucmd );
int ForcePowerUsableOn(gentity_t *attacker, gentity_t *other, forcePowers_t forcePower);
void ForceHeal( gentity_t *self );
void ForceSpeed( gentity_t *self, int forceDuration );
void ForceRage( gentity_t *self );
void ForceGrip( gentity_t *self );
void ForceProtect( gentity_t *self );
void ForceAbsorb( gentity_t *self );
void ForceTeamHeal( gentity_t *self );
void ForceTeamForceReplenish( gentity_t *self );
void ForceSeeing( gentity_t *self );
void ForceThrow( gentity_t *self, qboolean pull );
void ForceDrain( gentity_t *self );
void ForceTelepathy(gentity_t *self);
qboolean NPC_Humanoid_DodgeEvasion( gentity_t *self, gentity_t *shooter, trace_t *tr, int hitLoc );
void WP_DeactivateSaber( gentity_t *self, qboolean clearLength );
void WP_ActivateSaber( gentity_t *self );
void JKG_NetworkSaberCrystals( playerState_t *ps, int invId, int weaponId );
void JKG_DoubleCheckWeaponChange( usercmd_t *cmd, playerState_t *ps );

// wp_melee.cpp
qboolean G_CanBeEnemy( gentity_t *self, gentity_t *enemy );
qboolean G_KickDownable(gentity_t *ent);
void G_KickSomeMofos(gentity_t *ent);
void G_GrabSomeMofos(gentity_t *self);
void JKG_GrappleUpdate( gentity_t *self );

// bg_ammo.cpp
void BG_GiveAmmo(gentity_t* ent, ammo_t* ammo, qboolean max = qtrue, int amount = 0);

// g_log.c
void QDECL G_LogPrintf( const char *fmt, ... );
void QDECL G_LogWeaponPickup(int client, int weaponid);
void QDECL G_LogWeaponFire(int client, int weaponid);
void QDECL G_LogWeaponDamage(int client, int mod, int amount);
void QDECL G_LogWeaponKill(int client, int mod);
void QDECL G_LogWeaponDeath(int client, int weaponid);
void QDECL G_LogWeaponFrag(int attacker, int deadguy);
void QDECL G_LogWeaponPowerup(int client, int powerupid);
void QDECL G_LogWeaponItem(int client, int itemid);
void QDECL G_LogWeaponInit(void);
void QDECL G_LogWeaponOutput(void);
void QDECL G_ClearClientLog(int client);

// g_timer
//Timing information
void		TIMER_Clear( void );
void		TIMER_Clear2( gentity_t *ent );
void		TIMER_Set( gentity_t *ent, const char *identifier, int duration );
int			TIMER_Get( gentity_t *ent, const char *identifier );
qboolean	TIMER_Done( gentity_t *ent, const char *identifier );
qboolean	TIMER_Start( gentity_t *self, const char *identifier, int duration );
qboolean	TIMER_Done2( gentity_t *ent, const char *identifier, qboolean remove );
qboolean	TIMER_Exists( gentity_t *ent, const char *identifier );
void		TIMER_Remove( gentity_t *ent, const char *identifier );

float NPC_GetHFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float hFOV );
float NPC_GetVFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float vFOV );


extern void G_SetEnemy (gentity_t *self, gentity_t *enemy);
qboolean InFront( vec3_t spot, vec3_t from, vec3_t fromAngles, float threshHold );

// ai_main.c
#define MAX_FILEPATH			144

int		OrgVisible		( vec3_t org1, vec3_t org2, int ignore);
int		InFieldOfVision	( vec3_t viewangles, float fov, vec3_t angles);

//bot settings
typedef struct bot_settings_s
{
	char personalityfile[MAX_FILEPATH];
	float skill;
	char team[MAX_FILEPATH];
} bot_settings_t;

int BotAISetup( int restart );
int BotAIShutdown( qboolean restart );
int BotAILoadMap( int restart );
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int BotAIShutdownClient( int client, qboolean restart );
int BotAIStartFrame( int time );

#include "g_team.h" // teamplay specific stuff
#include "jkg_keypairs.h"

extern	level_locals_t	level;
extern gentity_t		g_entities[MAX_ENTITIESTOTAL];
extern gentity_t		*g_logicalents;
extern KeyPairSet_t		g_spawnvars[MAX_ENTITIESTOTAL];

#define	FOFS(x) offsetof(gentity_t, x)

// userinfo validation bitflags
// default is all except extended ascii
// numUserinfoFields + USERINFO_VALIDATION_MAX should not exceed 31
typedef enum userinfoValidationBits_e {
	// validation & (1<<(numUserinfoFields+USERINFO_VALIDATION_BLAH))
	USERINFO_VALIDATION_SIZE=0,
	USERINFO_VALIDATION_SLASH,
	USERINFO_VALIDATION_EXTASCII,
	USERINFO_VALIDATION_CONTROLCHARS,
	USERINFO_VALIDATION_MAX
} userinfoValidationBits_t;

/**************************************************
* jkg_equip.cpp
**************************************************/

void JKG_ShieldEquipped(gentity_t* ent, int shieldItemNumber, qboolean playSound);
void Cmd_ShieldUnequipped(gentity_t* ent);
void Cmd_ShieldUnequipped(gentity_t* ent, unsigned int index);
void JKG_JetpackEquipped(gentity_t* ent, int jetpackItemNumber);
void Cmd_JetpackUnequipped(gentity_t* ent);
void Cmd_JetpackUnequipped(gentity_t* ent, unsigned int index);
void JKG_ArmorChanged(gentity_t* ent);
void JKG_EquipItem(gentity_t* ent, int itemNumber);

/**************************************************
* jkg_team.c
**************************************************/

void Svcmd_ToggleUserinfoValidation_f( void );
void Svcmd_ToggleAllowVote_f( void );
qboolean	TeamCommand( int clientNum, char *cmd, char *param );							// A client is issueing a command, check for team commands here.
void		TeamDisconnect( int clientNum );												// This client is disconnecting without dealing with his team.
qboolean	TeamFriendly( int p1, int p2 );													// Returns wether or not these players are friendly.
void		TeamInitialize( int clientNum );												// Initializes the party identifiers for this player.
void		TeamInitializeServer( void );													// Initializes the party identifiers for the server.
void		TeamPartyCommandAccept( int clientNum, int iID );								// Accepts the party invitation in the provided slot.
void		TeamPartyCommandChangeLeader( int clientNum, int iID, qboolean forceClient );	// Changes the authority of the party leader to another member. The identifier is the slot!
void		TeamPartyCommandCreate( int clientNum );										// Creates a new party. Find the first slot and set the client as leader.
void		TeamPartyCommandDisband( int clientNum );										// Disbands the party, removes all members and the leader.
void		TeamPartyCommandDismiss( int clientNum, int iID );								// Dismisses the target member from the party. The identifier is the slot!
void		TeamPartyCommandInvite( int clientNum, char *parm );							// Invites a player to join your party. The parameter can be a identifier or name.
void		TeamPartyCommandLeave( int clientNum, qboolean forceClient );					// Leave your current party. This may not be performed by the leader.
void		TeamPartyListRefresh( int clientNum, int iTime );								// Refreshes the 'seeking party list', where people register to look for a party.
void		TeamPartyListRegister( int clientNum, char *message );							// Register yourself on the seeking list!
void		TeamPartyListUnregister( int clientNum, qboolean forcedUpdate );				// Unregister yourself from the seeking list!
void		TeamPartyCommandReject( int clientNum, int iID );								// Rejects the party invitation in the provided slot.
int			TeamPartyCount( int partyNum );													// Counts the number of players that are in the party.
void		TeamPartyUpdate( int partyNum, int forceClient );								// Updates the team and individual status. This sends a full incremental update.
int			TeamTarget( gentity_t *ent, char *cmd );										// Finds a target based on the client data and partial string.

// g_cvar.c
#define XCVAR_PROTO
	#include "g_xcvar.h"
#undef XCVAR_PROTO
void G_RegisterCvars( void );
void G_UpdateCvars( void );

/**************************************************
* jkg_vendor.cpp
**************************************************/
void JKG_SP_target_vendor(gentity_t *ent);
void JKG_target_vendor_use(gentity_t* self, gentity_t* other, gentity_t* activator);
void JKG_MakeNPCVendor(gentity_t* ent, char* szTreasureClassName);
void JKG_UnmakeNPCVendor(gentity_t* ent);
void JKG_GenericVendorUse(gentity_t* self, gentity_t* other, gentity_t* activator);
void JKG_RegenerateStock(gentity_t* ent);


/**************************************************
* jkg_treasureclass.cpp
**************************************************/
void JKG_TC_Init(const char* szTCDirectory);
void JKG_TC_Shutdown();

/**************************************************
* jkg_astar.cpp - New A* Routing Implementation.
**************************************************/
extern qboolean PATHING_IGNORE_FRAME_TIME;

int ASTAR_FindPath(int from, int to, int *pathlist);
int ASTAR_FindPathWithTimeLimit(int from, int to, int *pathlist);
int ASTAR_FindPathFast(int from, int to, int *pathlist, qboolean shorten);

void NPC_ClearLookTarget( gentity_t *self );

// Refactored included functions
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);
void JKG_CBB_SendAll(int client);
void JKG_PlayerIsolationClear(int client);
#endif

extern gameImport_t *trap;

