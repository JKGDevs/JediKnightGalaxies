// cg_local.h
// Copyright (C) 1999-2000 Id Software, Inc. (c) 2013 Jedi Knight Galaxies
#ifndef CG_LOCAL_H
#define CG_LOCAL_H
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


#include "qcommon/q_shared.h"
#include "rd-common/tr_types.h"
#include "game/bg_items.h"
#include "game/bg_public.h"
#include "cg_public.h"

#include "cg_weapons.h"

#include "jkg_cg_damagetypes.h"
#include "game/jkg_gangwars.h"
#include "game/bg_damage.h"
#include "game/bg_buffs.h"

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

// Jedi Knight Galaxies - Team Party Slots
#define PARTY_SLOT_EMPTY	64
#define PARTY_SLOT_MEMBERS	 5
#define PARTY_SLOT_MAX		64
#define PARTY_SLOT_INVITES	 5

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000

// Zoom vars
#define	ZOOM_TIME			150		// not currently used?
#define MAX_ZOOM_FOV		3.0f
#define ZOOM_IN_TIME		1500.0f
#define ZOOM_OUT_TIME		100.0f
#define ZOOM_START_PERCENT	0.3f

#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define NUM_FONT_BIG	1
#define NUM_FONT_SMALL	2
#define NUM_FONT_CHUNKY	3

#define	NUM_CROSSHAIRS		9

#define TEAM_OVERLAY_MAXNAME_WIDTH	32
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	64

#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

typedef enum {
	FOOTSTEP_STONEWALK,
	FOOTSTEP_STONERUN,
	FOOTSTEP_METALWALK,
	FOOTSTEP_METALRUN,
	FOOTSTEP_PIPEWALK,
	FOOTSTEP_PIPERUN,
	FOOTSTEP_SPLASH,
	FOOTSTEP_WADE,
	FOOTSTEP_SWIM,
	FOOTSTEP_SNOWWALK,
	FOOTSTEP_SNOWRUN,
	FOOTSTEP_SANDWALK,
	FOOTSTEP_SANDRUN,
	FOOTSTEP_GRASSWALK,
	FOOTSTEP_GRASSRUN,
	FOOTSTEP_DIRTWALK,
	FOOTSTEP_DIRTRUN,
	FOOTSTEP_MUDWALK,
	FOOTSTEP_MUDRUN,
	FOOTSTEP_GRAVELWALK,
	FOOTSTEP_GRAVELRUN,
	FOOTSTEP_RUGWALK,
	FOOTSTEP_RUGRUN,
	FOOTSTEP_WOODWALK,
	FOOTSTEP_WOODRUN,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

typedef struct trRefEntity_s
{
	refEntity_t	e;

	float		axisLength;		// compensate for non-normalized axis

	qboolean	needDlights;	// true for bmodels that touch a dlight
	qboolean	lightingCalculated;
	vec3_t		lightDir;		// normalized direction towards light
	vec3_t		ambientLight;	// color normalized to 0-255
	int			ambientLightInt;	// 32 bit rgba packed
	vec3_t		directedLight;
	int         unknown_field;
} trRefEntity_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation

typedef struct playerEntity_s {
	lerpFrame_t		legs, torso, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_COMBAT_SOUNDS	40
#define	MAX_CUSTOM_EXTRA_SOUNDS	40
#define	MAX_CUSTOM_JEDI_SOUNDS	40
// MAX_CUSTOM_SIEGE_SOUNDS defined in bg_public.h
#define MAX_CUSTOM_DUEL_SOUNDS	40

#define	MAX_CUSTOM_SOUNDS	40 //rww - Note that for now these must all be the same, because of the way I am
							   //cycling through them and comparing for custom sounds.

typedef struct clientInfo_s {
	qboolean		infoValid;

	float			colorOverride[3];

	saberInfo_t		saber[MAX_SABERS];
	void			*ghoul2Weapons[MAX_SABERS];

	char			saberName[64];
	char			saber2Name[64];

	char			name[MAX_QPATH];
	char			cleanname[MAX_QPATH];
	team_t			team;
	// JKG
	int				customteam;		// For mini-games like the arena

	int				duelTeam;

	int				botSkill;		// -1 = not bot, 0-5 = bot

	int				frame;

	vec3_t			color1;
	vec3_t			color2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				maxhealth;		// you only get this info about your teammates
	int				armor;			// you only get this info about your teammates
	int				maxarmor;		// you only get this info about your teammates
	int				curForcePower;	// you only get this info about your teammates
	int				maxForcePower;	// you only get this info about your teammates
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;

	int				breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
//	char			headModelName[MAX_QPATH];
//	char			headSkinName[MAX_QPATH];
	char			forcePowers[MAX_QPATH];
//	char			redTeam[MAX_TEAMNAME];
//	char			blueTeam[MAX_TEAMNAME];

	char			teamName[MAX_TEAMNAME];

	int				corrTime;

	vec3_t			lastHeadAngles;
	int				lookTime;

	int				brokenLimbs;

	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	//footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	//qhandle_t		headModel;
	//qhandle_t		headSkin;

	void			*ghoul2Model;

	qhandle_t		modelIcon;

	qhandle_t		bolt_rhand;
	qhandle_t		bolt_lhand;

	qhandle_t		bolt_head;

	qhandle_t		bolt_motion;

	qhandle_t		bolt_llumbar;

	int				siegeIndex;
	int				siegeDesiredTeam;

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
	sfxHandle_t		combatSounds[MAX_CUSTOM_COMBAT_SOUNDS];
	sfxHandle_t		extraSounds[MAX_CUSTOM_EXTRA_SOUNDS];
	sfxHandle_t		jediSounds[MAX_CUSTOM_JEDI_SOUNDS];
	sfxHandle_t		siegeSounds[MAX_CUSTOM_SIEGE_SOUNDS];
	sfxHandle_t		duelSounds[MAX_CUSTOM_DUEL_SOUNDS];

	int				legsAnim;
	int				torsoAnim;

	float		facial_blink;		// time before next blink. If a minus value, we are in blink mode
	float		facial_frown;		// time before next frown. If a minus value, we are in frown mode
	float		facial_aux;			// time before next aux. If a minus value, we are in aux mode

	int			superSmoothTime; //do crazy amount of smoothing

} clientInfo_t;

//rww - cheap looping sound struct
#define MAX_CG_LOOPSOUNDS 8

typedef struct cgLoopSound_s {
	int entityNum;
	vec3_t origin;
	vec3_t velocity;
	sfxHandle_t sfx;
} cgLoopSound_t;

typedef struct debuffVisualsData_s
{
	int lastEFXTime;
	int debuffStartTime;
} debuffVisualsData_t;

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	//rww - entstate must be first, to correspond with the bg shared entity structure
	entityState_t	currentState;	// from cg.frame
	playerState_t	*playerState;	//ptr to playerstate if applicable (for bg ents)
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//from here up must be unified with bgEntity_t -rww

	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

    unsigned int    shotCount;
    qboolean        shotCountOverflowed;
    unsigned int    muzzleFlashCount;
	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
//	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	vec3_t			damageAngles;
	int				damageTime;

	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

	vec3_t			ragLastOrigin;
	int				ragLastOriginTime;

	qboolean		noLumbar; //if true only do anims and things on model_root instead of lower_lumbar, this will be the case for some NPCs.
	qboolean		noFace;

	//For keeping track of the current surface status in relation to the entitystate surface fields.
	int				npcLocalSurfOn;
	int				npcLocalSurfOff;

	int				eventAnimIndex;

	clientInfo_t	*npcClient; //dynamically allocated - always free it, and never stomp over it.

	int				weapon;

	void			*ghoul2weapon; //rww - pointer to ghoul2 instance of the current 3rd person weapon

	float			radius;
	int				boltInfo;

	//sometimes used as a bolt index, but these values are also used as generic values for clientside entities
	//at times
	int				bolt1;
	int				bolt2;
	int				bolt3;
	int				bolt4;

	float			bodyHeight;

	int				torsoBolt;

	vec3_t			turAngles;

	vec3_t			frame_minus1;
	vec3_t			frame_minus2;

	int				frame_minus1_refreshed;
	int				frame_minus2_refreshed;

	void			*frame_hold; //pointer to a ghoul2 instance

	int				frame_hold_time;
	int				frame_hold_refreshed;

	void			*grip_arm; //pointer to a ghoul2 instance

	int				trickAlpha;
	int				trickAlphaTime;

	int				teamPowerEffectTime;
	int				teamPowerType; //0 regen, 1 heal, 2 drain, 3 absorb

	qboolean		isRagging;
	qboolean		ownerRagging;
	int				overridingBones;

	int				bodyFadeTime;
	vec3_t			pushEffectOrigin;

	cgLoopSound_t	loopingSound[MAX_CG_LOOPSOUNDS];
	int				numLoopingSounds;

	int				serverSaberHitIndex;
	int				serverSaberHitTime;
	qboolean		serverSaberFleshImpact; //true if flesh, false if anything else.

	qboolean		ikStatus;

	qboolean		saberWasInFlight;

	float			smoothYaw;

	int				uncloaking;
	qboolean		cloaked;

	int				vChatTime;

	// JKG - Extra Values
	int				fadeAlpha;			// Current alpha being used for the fade
	int				fadeState;			// 0 = Normal, 1 = Fading-In, 2 = Fading-Out, 3 = Don't Render
	int				visibilityTime;		// Time of the last visibility check.
	qboolean		visibilityState;	// 0 = No, 1 = Yes..
	debuffVisualsData_t debuffVisuals[PLAYERBUFF_BITS];
	int				shieldHitTime;
	int				shieldRechargeTime;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independently from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_FRAGMENT,
	LE_PUFF,
	LE_SCOREPLUM,
	LE_DAMAGEPLUM,
	LE_LINE
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE = 0x0001,			// do not scale size over time
	LEF_TUMBLE			= 0x0002,			// tumble over time, used for ejecting shells
} leFlag_t;

typedef enum {
	LEMT_NONE,
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_METAL,
	LEBS_ROCK
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect
	int				bounceSound;		// optional sound index to play upon bounce

	float			alpha;
	float			dalpha;

	int				forceAlpha;

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	union {
		struct {
			float radius;
			float dradius;
			vec3_t startRGB;
			vec3_t dRGB;
		} sprite;
		struct {
			float width;
			float dwidth;
			float length;
			float dlength;
			vec3_t startRGB;
			vec3_t dRGB;
		} trail;
		struct {
			float width;
			float dwidth;
			// Below are bezier specific.
			vec3_t			control1;				// initial position of control points
			vec3_t			control2;
			vec3_t			control1_velocity;		// initial velocity of control points
			vec3_t			control2_velocity;
			vec3_t			control1_acceleration;	// constant acceleration of control points
			vec3_t			control2_acceleration;
		} line;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			vec3_t startRGB;
			vec3_t dRGB;
		} line2;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			float height;
			float dheight;
		} cylinder;
		struct {
			float width;
			float dwidth;
		} electricity;
		struct
		{
			// fight the power! open and close brackets in the same column!
			float radius;
			float dradius;
			qboolean (*thinkFn)(struct localEntity_s *le);
			vec3_t	dir;	// magnitude is 1, but this is oldpos - newpos right before the
							//particle is sent to the renderer
			// may want to add something like particle::localEntity_s *le (for the particle's think fn)
		} particle;
		struct
		{
			qboolean	dontDie;
			vec3_t		dir;
			float		variance;
			int			delay;
			int			nextthink;
			qboolean	(*thinkFn)(struct localEntity_s *le);
			int			data1;
			int			data2;
		} spawner;
		struct
		{
			float radius;
		} fragment;
	} data;

	refEntity_t		refEntity;
} localEntity_t;

//======================================================================


typedef struct score_s {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				team;
} score_t;


#define MAX_WEAPON_ANIMATIONS (16)

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
    unsigned int    weaponNum;
    unsigned int    variation;

	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	void            *g2WorldModel;      // Used to be in g2WeaponInstances
	qhandle_t		weaponModel;		// this is the pickup model
	qhandle_t		viewModel;			// this is the in-view model used by the player
	qhandle_t		barrelModels[4];
	qhandle_t		flashModel;
	
	void            *g2ViewModel;
	qhandle_t       viewModelSkin;
	animation_t     viewModelAnims[MAX_WEAPON_ANIMATIONS];

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;

	qhandle_t		hudIcon;
	qhandle_t		hudNAIcon;

	qhandle_t		ammoModel;
	
	qhandle_t       fireModeIndicator;
	qhandle_t       groupedIndicators[3];
	indicatorType_t indicatorType;
	
	vec3_t          gunPosition;
	vec3_t          ironsightsPosition;
	float           ironsightsFov;
	
    qhandle_t       scopeShader;
    sfxHandle_t     scopeStartSound;
    sfxHandle_t     scopeStopSound;
    int             scopeSoundLoopTime;
    sfxHandle_t     scopeLoopSound;
	
	//const weaponEventsHandler_t *primaryEventsHandler;
	//const weaponEventsHandler_t *altEventsHandler;

	const weaponEventsHandler_t *eventsHandler[MAX_FIREMODES];
	
	//weaponDrawData_t primDrawData;
	//weaponDrawData_t altDrawData;
	weaponDrawData_t drawData[MAX_FIREMODES];

	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose
	sfxHandle_t		firingSound;
	sfxHandle_t		chargeSound;
	fxHandle_t		muzzleEffect;
	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;
	sfxHandle_t		missileHitSound;

	sfxHandle_t		altFlashSound[4];
	sfxHandle_t		altFiringSound;
	sfxHandle_t		altChargeSound;
	fxHandle_t		altMuzzleEffect;
	qhandle_t		altMissileModel;
	sfxHandle_t		altMissileSound;
	void			(*altMissileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			altMissileDlight;
	vec3_t			altMissileDlightColor;
	int				altMissileRenderfx;
	sfxHandle_t		altMissileHitSound;

	sfxHandle_t		selectSound;

	sfxHandle_t		readySound;
	float			trailRadius;
	float			wiTrailTime;

} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct itemInfo_s {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
/*
Ghoul2 Insert Start
*/
	void			*g2Models[MAX_ITEM_MODELS];
	float			radius[MAX_ITEM_MODELS];
/*
Ghoul2 Insert End
*/
} itemInfo_t;


typedef struct powerupInfo_s {
	int				itemNum;
} powerupInfo_t;

#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16


// Chat system modified for JKG
#define	MAX_CHATBOX_ITEMS				12	// 5
#define MAX_MESSAGE_NOTIFICATIONS		4
#define MESSAGE_NOTIFICATION_TIME		10000	// each message lasts this amount of time before it begins to fade
#define MESSAGE_NOTIFICATION_FADE_TIME	1000	// message takes this long to fade out
#define MAX_NOTIFICATION_CHARS			350
typedef struct chatBoxItem_s
{
	char	string[MAX_SAY_TEXT + 50];		// extra bytes for names, newlines 'n timestamp
	int		active; //time;
	int		lines;
	float	alpha;
} chatBoxItem_t;

typedef struct notificationItem_s
{
	char	string[MAX_NOTIFICATION_CHARS];
	int		active;
	int		lines;
	float	alpha;
	qboolean weaponNotification;
} notificationItem_t;

typedef struct cg_s {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
//	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	mMapChange;

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	playerState_t	predictedPlayerState;

	int lastPurchasedItem;

	//centity_t		predictedPlayerEntity;
	//rww - I removed this and made it use cg_entities[clnum] directly.

	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	int			forceSelect;
	int			itemSelect;

	// auto rotating items
	vec3_t		autoAngles;
	matrix3_t	autoAxis;
	vec3_t		autoAnglesFast;
	matrix3_t	autoAxisFast;

	// view rendering
	refdef_t	refdef;

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NETNAME];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	int			kick_time;
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera
	int			loadLCARSStage;

	int			forceHUDTotalFlashTime;
	int			forceHUDNextFlashTime;
	qboolean	forceHUDActive;				// Flag to show force hud is off/on

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	int			VHUDFlashTime;
	qboolean	VHUDTurboFlag;

	vec3_t			lastFPFlashPoint;

/*
Ghoul2 Insert Start
*/
	int				testModel;
	// had to be moved so we wouldn't wipe these out with the memset - these have STL in them and shouldn't be cleared that way
	snapshot_t	activeSnapshots[2];
/*
Ghoul2 Insert End
*/

	// used for communication with the engine
	union {
		char						raw[MAX_CG_SHARED_BUFFER_SIZE];
		TCGPointContents			pointContents;
		TCGVectorData				vectorData;
		TCGGetBoltData				getBoltData;
		TCGTrace					trace;
		TCGG2Mark					g2Mark;
		TCGImpactMark				impactMark;
		ragCallbackDebugBox_t		rcbDebugBox;
		ragCallbackDebugLine_t		rcbDebugLine;
		ragCallbackBoneSnap_t		rcbBoneSnap;
		ragCallbackBoneInSolid_t	rcbBoneInSolid;
		ragCallbackTraceLine_t		rcbTraceLine;
		TCGMiscEnt					miscEnt;
		autoMapInput_t				autoMapInput;
		TCGCameraShake				cameraShake;
	} sharedBuffer;

	short				radarEntityCount;
	short				radarEntities[MAX_CLIENTS+16];

	float				distanceCull;

	// Jedi Knight Galaxies

	// JKG: Modified to act as a cyclic buffer
	chatBoxItem_t		chatItems[MAX_CHATBOX_ITEMS];
						// chatItemNext is the next item in queue
						// and is therefore the current oldest item
						// The newest is located at chatItemNext - 1 (wraparound accounted for)
	int					chatItemNext;		
	qboolean			isChatting;

	qboolean            framebufferActive;
	int					motionBlurTime;
	float				blurLevel;
	int					blurPasses;
	float				noise_cintensity;
	float				noise_dintensity;
	int					deathTime;
	int					cinematicState; // 0 = off, 1 = coming up, 2 = on, 3 = going down
	int					cinematicVideo;
	int					cinematicTime;
	int					trapEscape;
	qboolean			turnOnBlurCvar;

	int					deathcamFadeStart;
	int					deathcamTime;
	vec3_t				deathcamBackupPos;
	vec3_t				deathcamCenter;
	int					deathcamRadius;

	int					hitmarkerLastTime;

	int					showMapLoadProgress;
	
	int					chatItemActive;

#if 0
	int					snapshotTimeoutTime;
#endif
	int					frameDelta;		// Delta time for this frame in milliseconds
	int					reloadTimeStart;
	int					reloadTimeDuration;
	float				sprintTime;
	
	int					i360CameraForce;	// The value to set if you started the camera from first person.
	int					i360CameraTime;		// The time the button was pressed.
	int					i360CameraOffset;	// The offset to add to the view angles.
	int					i360CameraOriginal;	// The original view angles which is to be restored when stopping.
	int					i360CameraUserCmd;	// The last user command, we use this for changing the offset.

	float				jkg_HUDOpacity;
	float				jkg_WHUDOpacity;	// Inherits the value of the HUD opacity, but deals with ammo display and stuffs.
											// The ammo display fades when changing weapons.
	int					jkg_grenadeCookTimer;
	
	// view weapon animation
	animation_t         *viewWeaponAnimation;
	float               ironsightsBlend;
	float				sprintBlend;

	// inventory
	std::vector<itemInstance_t>* playerInventory;
	int					playerACI[MAX_ACI_SLOTS];

	// trade
	std::vector<itemInstance_t>* ourTradeItems;			// Items that we have up to trade - only used for PvP trade
	std::vector<itemInstance_t>* otherTradeItems;		// Items that the other player/NPC/container has up to trade
	int							currentlyTradingWith;	// Entity number of the other ent that we are trading with
	int							otherTradeCredits;		// How many credits the other player is offering us

	// assist data / VERSUS ONLY / kinda anyway
	notificationItem_t		notificationBox[MAX_MESSAGE_NOTIFICATIONS];
	int					notifyNext;	




#ifdef __AUTOWAYPOINT__
	vec3_t		mapcoordsMins;
	vec3_t		mapcoordsMaxs;
	qboolean	mapcoordsValid;
#endif //__AUTOWAYPOINT__		// Used to detect changes in the firing mode stuff. If a change is detected, it plays the sound
#ifndef NO_SP_STYLE_AMMO
	int					lastFiringMode;
	int					lastFiringModeTime;
	int					lastAmmo;
	int					lastAmmoTime;
	int					lastAmmoGun;
	int					lastAmmoTypeTime;
#endif
	int					lastAmmoType;
	int					lastFiringModeGun;
	int					fireModeChangeTime;
	int					fireModeTransition;

	int					last_joy;

	// Warzone Gametype...
	int					captureFlagPercent;
	qboolean			capturingFlag;
	qboolean			recaptureingFlag;

	qboolean spawning;
	int	numSpawnVars;
	char *spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int numSpawnVarChars;
	char spawnVarChars[MAX_SPAWN_VARS_CHARS];

} cg_t;

#define MAX_TICS	14

typedef struct forceTicPos_s
{
	int				x;
	int				y;
	int				width;
	int				height;
	char			*file;
	qhandle_t		tic;
} forceTicPos_t;
extern forceTicPos_t forceTicPos[];
extern forceTicPos_t ammoTicPos[];

typedef struct cgscreffects_s
{
	float		FOV;
	float		FOV2;

	float		shake_intensity;
	int			shake_duration;
	int			shake_start;

	float		music_volume_multiplier;
	int			music_volume_time;
	qboolean	music_volume_set;
} cgscreffects_t;

extern cgscreffects_t cgScreenEffects;

void CGCam_Shake( float intensity, int duration );
void CGCam_SetMusicMult( float multiplier, int duration );

enum
{
	CHUNK_METAL1 = 0,
	CHUNK_METAL2,
	CHUNK_ROCK1,
	CHUNK_ROCK2,
	CHUNK_ROCK3,
	CHUNK_CRATE1,
	CHUNK_CRATE2,
	CHUNK_WHITE_METAL,
	NUM_CHUNK_TYPES
};
#define NUM_CHUNK_MODELS	4

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct cgMedia_s {
	qhandle_t	charsetShader;
	qhandle_t	whiteShader;

	//Jedi Knight Galaxies: more charsets
	qhandle_t	charset_Arial;
	qhandle_t	charset_Courier;
	qhandle_t	charset_Segoeui;
	qhandle_t	charset_Fixedsys;

	qhandle_t	loadBarLED;
	qhandle_t	loadBarLEDCap;
	qhandle_t	loadBarLEDSurround;

	qhandle_t	itemHoloModel;
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel; // Warzone...

	qhandle_t	redFlagRadarShader;
	qhandle_t	blueFlagRadarShader;
	qhandle_t	neutralFlagRadarShader; // Warzone...

	qhandle_t	teamStatusBar;

	qhandle_t	siegeItemShader;
	qhandle_t	mAutomapPlayerIcon;
	qhandle_t	mAutomapRocketIcon;

	qhandle_t	wireframeAutomapFrame_left;
	qhandle_t	wireframeAutomapFrame_right;
	qhandle_t	wireframeAutomapFrame_top;
	qhandle_t	wireframeAutomapFrame_bottom;

//Chunks
	qhandle_t	chunkModels[NUM_CHUNK_TYPES][4];
	sfxHandle_t	chunkSound;
	sfxHandle_t	grateSound;
	sfxHandle_t	rockBreakSound;
	sfxHandle_t	rockBounceSound[2];
	sfxHandle_t	metalBounceSound[2];
	sfxHandle_t	glassChunkSound;
	sfxHandle_t	crateBreakSound[2];

	qhandle_t	hackerIconShader;

	// Saber shaders
	//-----------------------------
	qhandle_t	forceCoronaShader;

	qhandle_t	redSaberGlowShader;
	qhandle_t	redSaberCoreShader;
	qhandle_t	orangeSaberGlowShader;
	qhandle_t	orangeSaberCoreShader;
	qhandle_t	yellowSaberGlowShader;
	qhandle_t	yellowSaberCoreShader;
	qhandle_t	greenSaberGlowShader;
	qhandle_t	greenSaberCoreShader;
	qhandle_t	blueSaberGlowShader;
	qhandle_t	blueSaberCoreShader;
	qhandle_t	purpleSaberGlowShader;
	qhandle_t	purpleSaberCoreShader;
	qhandle_t	saberBlurShader;
	qhandle_t	swordTrailShader;

	qhandle_t	yellowDroppedSaberShader;

	qhandle_t	rivetMarkShader;

	qhandle_t	teamRedShader;
	qhandle_t	teamBlueShader;

	qhandle_t	powerDuelAllyShader;

	qhandle_t	balloonShader;
	qhandle_t	vchatShader;
	qhandle_t	connectionShader;

	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;

	qhandle_t	plumShaders[11];
	qhandle_t	numberShaders[11];
	qhandle_t	smallnumberShaders[11];
	qhandle_t	chunkyNumberShaders[11];

	qhandle_t	electricBodyShader;
	qhandle_t	electricBody2Shader;

	qhandle_t	fsrMarkShader;
	qhandle_t	fslMarkShader;
	qhandle_t	fshrMarkShader;
	qhandle_t	fshlMarkShader;

	qhandle_t	refractionShader;

	qhandle_t	cloakedShader;

	qhandle_t	boltShader;

	qhandle_t	shadowMarkShader;

	//glass shard shader
	qhandle_t	glassShardShader;

	// wall mark shaders
	qhandle_t	wakeMarkShader;

	qhandle_t	itemRespawningPlaceholder;
	qhandle_t	itemRespawningRezOut;

	qhandle_t	playerShieldDamage;
	qhandle_t	protectShader;
	qhandle_t	forceSightBubble;
	qhandle_t	forceShell;
	qhandle_t	sightShell;

	// explosion assets
	qhandle_t	explosionModel;
	qhandle_t	surfaceExplosionShader;

	qhandle_t	disruptorShader;

	qhandle_t	solidWhite;

	// sounds
	sfxHandle_t	selectSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];

	sfxHandle_t crackleSound;

	sfxHandle_t teamHealSound;
	sfxHandle_t teamRegenSound;

	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t rollSound;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t noforceSound;

	sfxHandle_t	drainSound;

	sfxHandle_t hitmarkerSound;
	qhandle_t	hitmarkerGraphic;

	//music blips
	sfxHandle_t dramaticFailure;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;

	qhandle_t halfShieldModel;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;

	// Jedi Knight Galaxies
	qhandle_t lowHealthAura; //	trap_R_RegisterShader("gfx/jkag/lowhealthaura")

	//force power icons
	qhandle_t forcePowerIcons[NUM_FORCE_POWERS];

	qhandle_t rageRecShader;

	//other HUD parts
	int			currentBackground;
	qhandle_t	weaponIconBackground;
	qhandle_t	forceIconBackground;
	qhandle_t	inventoryIconBackground;

	qhandle_t	bdecal_bodyburn1;
	qhandle_t	bdecal_saberglow;
	qhandle_t	bdecal_burn1;
	qhandle_t	mSaberDamageGlow;

	sfxHandle_t	noAmmoSound;

	// JKG
	qhandle_t	deathfont;
	qhandle_t	hudfont1;
	qhandle_t	hudfont2;
	qhandle_t	horizgradient;
	qhandle_t	avatar_placeholder;
	
	qhandle_t   bboxShader;

} cgMedia_t;


// Stored FX handles
//--------------------
typedef struct cgEffects_s {

	// TRIPMINE
	fxHandle_t	tripmineLaserFX;
	fxHandle_t	tripmineGlowFX;

	//FORCE
	fxHandle_t forceLightning;
	fxHandle_t forceLightningWide;

	fxHandle_t forceDrain;
	fxHandle_t forceDrainWide;
	fxHandle_t forceDrained;

	//TURRET
	fxHandle_t turretShotEffect;

	//Whatever
	fxHandle_t itemCone;

	fxHandle_t	mSparks;
	fxHandle_t	mSaberCut;
	fxHandle_t	mTurretMuzzleFlash;
	fxHandle_t	mSaberBlock;
	fxHandle_t	mSaberBloodSparks;
	fxHandle_t	mSaberBloodSparksSmall;
	fxHandle_t	mSaberBloodSparksMid;
	fxHandle_t	mBlasterDeflect;
	fxHandle_t	mBlasterSmoke;
	fxHandle_t	mForceConfusionOld;
	fxHandle_t	mDisruptorDeathSmoke;
	fxHandle_t	mSparkExplosion;
	fxHandle_t	mTurretExplode;
	fxHandle_t	mEmplacedExplode;
	fxHandle_t	mEmplacedDeadSmoke;
	fxHandle_t	mDetpackExplosion;
	fxHandle_t	mStunBatonFleshImpact;
	fxHandle_t	mSparksExplodeNoSound;
	fxHandle_t	mTripMineLaser;
	fxHandle_t	mEmplacedMuzzleFlash;
	fxHandle_t	mHeatSteam;
	fxHandle_t	mOverheatSteam;
	fxHandle_t	mHyperspaceStars;
	fxHandle_t	mBlackSmoke;
	fxHandle_t	mShipDestDestroyed;
	fxHandle_t	mShipDestBurning;
	// jetpack
	fxHandle_t	mJetpack;	//effects/rockettrooper/flamenew.efx

	//footstep effects
	fxHandle_t footstepMud;
	fxHandle_t footstepSand;
	fxHandle_t footstepSnow;
	fxHandle_t footstepGravel;
	//landing effects
	fxHandle_t landingMud;
	fxHandle_t landingSand;
	fxHandle_t landingDirt;
	fxHandle_t landingSnow;
	fxHandle_t landingGravel;
	//splashes
	fxHandle_t waterSplash;
	fxHandle_t lavaSplash;
	fxHandle_t acidSplash;
} cgEffects_t;

#define MAX_STATIC_MODELS 4000

typedef struct cg_staticmodel_s {
	qhandle_t		model;
	vec3_t			org;
	matrix3_t		axes;
	float			radius;
	float			zoffset;
} cg_staticmodel_t;

typedef struct {
	char				id;								// The party identifier (reference only).
	char				leaderId;						// The leader identifier, used to lookup the name.
	char				memberCount;					// The current amount of members of this party.

} teamPartyInvites_t;

typedef struct
{

	char				id;								// This is the member identifier, used to lookup the name
	char				classId;						// This is the member class identifier so we can identify the class
	char				status;							// This is the status: -1 (Pending), 0 (Member) or 1 (Leader)

} teamPartyMember_t;

typedef struct
{

	char				active;							// Set to true when actively in a party.
	char				number;							// Party number we are in (or have been in last).
	teamPartyInvites_t	invites[PARTY_SLOT_INVITES];	// The pending invites to be displayed.
	teamPartyMember_t	members[PARTY_SLOT_MEMBERS];	// The members of the current (or the last) party.

} teamParty_t;

typedef struct
{

	unsigned int		 time;							// Time of the last update (even unregister gets this to send delta's!)
	int					 id;							// The player identifier (PARTY_SLOT_EMPTY when empty).
	int					 classId;						// The player class identifier.
	char				 message[64];					// The message that has been set.

} teamPartyList_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournament restart is done, allowing
// all clients to begin playing instantly
typedef struct cgs_s {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	int				showDuelHealths;
	gametype_t		gametype;
	// Gang Wars stuff -- Jedi Knight Galaxies
	int				redTeam;
	int				blueTeam;
	int				debugMelee;
	int				stepSlideFix;
	int				noSpecMove;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				duel_fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	qboolean		needpass;
	int				wDisable;
	int				fDisable;

	char			mapname[MAX_QPATH];
	char			rawmapname[MAX_QPATH];
//	char			redTeam[MAX_QPATH];
//	char			blueTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				duelWinner;
	int				duelist1;
	int				duelist2;
	int				duelist3;
// nmckenzie: DUEL_HEALTH.  hmm.
	int				duelist1health;
	int				duelist2health;
	int				duelist3health;

	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];
	fxHandle_t		gameEffects[MAX_FX];
	qhandle_t		gameIcons[MAX_ICONS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// media
	cgMedia_t		media;

	// effects
	cgEffects_t		effects;

	int					numMiscStaticModels;
	cg_staticmodel_t	miscStaticModels[MAX_STATIC_MODELS];

	// Jedi Knight Galaxies - New persistant values..
	teamParty_t		party;
	teamPartyList_t	partyList[MAX_CLIENTS];
	int				partyListTime;	// Highest time, used to send.

	//eezstreet add: armor
	int				armorInformation[MAX_CLIENTS][ARMSLOT_MAX];

	// UQ1: Warzone Gametype...
	int redtickets;
	int bluetickets;
} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];

extern	centity_t		*cg_permanents[MAX_GENTITIES];
extern	int				cg_numpermanents;

extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

// cg_cvar.c
#define XCVAR_PROTO
	#include "cg_xcvar.h"
#undef XCVAR_PROTO
void CG_RegisterCvars( void );
void CG_UpdateCvars( void );

//
// cg_main.c
//
void C_Trace( void );
void C_G2Trace( void );
void C_G2Mark( void );

const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void CG_StartMusic( qboolean bForceStart );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_SetScoreSelection(void *menu);
void CG_BuildSpectatorString(void);

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

float CG_ClampFov ( float fov );
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
qboolean CG_CullPointAndRadius (const vec3_t pt, float radius);

//
// cg_drawtools.c
//
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawRotatePic( float x, float y, float width, float height,float angle, qhandle_t hShader );
void CG_DrawRotatePic2( float x, float y, float width, float height,float angle, qhandle_t hShader );

void CG_DrawNumField (int x, int y, int width, int value,int charWidth,int charHeight,int style,qboolean zeroFill);

void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars, qhandle_t textshader );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, const vec4_t color, int font );
void UI_DrawScaledProportionalString( int x, int y, const char* str, int style, vec4_t color, float scale);
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

// UQ1: Added...
void CG_FilledBar(float x, float y, float w, float h, float *startColor, float *endColor, const float *bgColor, float frac, int flags);
void CG_HorizontalPercentBar( float x, float y, float width, float height, float percent );
void CG_VerticalPercentBar( float x, float y, float width, float height, float percent );
void CG_VerticalPercentBarNoBorder( float x, float y, float width, float height, float percent );

//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern  char systemChat[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_OwnerDraw(void *alwaysNull, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle,int font, int ownerDrawID);
void CG_Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style, int iMenuFont);
int CG_Text_Width(const char *text, float scale, int iMenuFont);
int CG_Text_Height(const char *text, float scale, int iMenuFont);
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
qboolean CG_DeferMenuScript(char **args);
void CG_ShowResponseHead(void);
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText(void);
const char *CG_GetKillerText(void);
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, void *ghoul2, int g2radius, qhandle_t skin, vec3_t origin, vec3_t angles );
qboolean CG_YourTeamHasFlag(void);
qboolean CG_OtherTeamHasFlag(void);
qhandle_t CG_StatusHandle(int task);

// Gets the performance analysis data from the engine and displays it in a friendly manner.
#define	LAG_SAMPLES		128
struct performanceSampleData_t {

	uint64_t rendererSamples[LAG_SAMPLES]{ 0 };
	uint64_t cgameSamples[LAG_SAMPLES]{ 0 };
	uint64_t gameSamples[LAG_SAMPLES]{ 0 };
	uint64_t serverSamples[LAG_SAMPLES]{ 0 };
	uint64_t frameSamples[LAG_SAMPLES]{ 0 };
	uint64_t clientSamples[LAG_SAMPLES]{ 0 };
};
static performanceSampleData_t cg_performanceData;


//
// cg_player.c
//
qboolean CG_RagDoll(centity_t *cent, vec3_t forcedAngles);
qboolean CG_G2TraceCollide(trace_t *tr, const vec3_t mins, const vec3_t maxs, const vec3_t lastValidStart, const vec3_t lastValidEnd);
void CG_AddGhoul2Mark(int shader, float size, vec3_t start, vec3_t end, int entnum,
					  vec3_t entposition, float entangle, void *ghoul2, vec3_t scale, int lifeTime);

void CG_CreateNPCClient(clientInfo_t **ci);
void CG_DestroyNPCClient(clientInfo_t **ci);

void CG_ArmorChanged(centity_t* cent, entityState_t* to, entityState_t* from);

void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( const refEntity_t *ent, const entityState_t *state, int team );
void CG_NewClientInfo( int clientNum, qboolean entitiesInitialized );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );
void CG_PlayerShieldHit(int entitynum, vec3_t angles, int amount);
void CG_PlayerShieldRecharging(int entitynum);
qboolean TeamFriendly( int player );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_G2Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );
void CG_ReattachLimb(centity_t *source);


//
// cg_ents.c
//

void CG_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void CG_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void CG_S_StopLoopingSound(int entityNum, sfxHandle_t sfx);
void CG_S_UpdateLoopingSounds(int entityNum);

void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( qboolean isPortal );
void CG_ManualEntityRender(centity_t *cent);
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, const char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, const char *tagName );

/*
Ghoul2 Insert Start
*/
void ScaleModelAxis(refEntity_t	*ent);
/*
Ghoul2 Insert End
*/

//
// cg_turret.c
//
void TurretClientRun(centity_t *ent);

//
// cg_weapons.c
//
void CG_GetClientWeaponMuzzleBoltPoint(int clIndex, vec3_t to);

void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum, int variation );
void CG_RegisterItemVisuals( int itemNum );

void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, vec3_t newAngles, qboolean thirdPerson );
void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups );
void CG_AnimateViewWeapon ( const playerState_t *ps );
void JKG_CG_FillACISlot(int itemNum, int slot);
void JKG_CG_ClearACISlot(int slot);
void JKG_CG_ACICheckRemoval(int itemNumber);
void JKG_CG_EquipItem(int newItem, int oldItem);
void JKG_CG_UnequipItem(int inventorySlot);
//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir,
					float orientation,
				    float r, float g, float b, float a,
					qboolean alphaFade,
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
void CG_GlassShatter(int entnum, vec3_t dmgPt, vec3_t dmgDir, float dmgRadius, int maxShards);
void CG_ScorePlum( int client, vec3_t org, int score );
void CG_DamagePlum( int client, vec3_t org, int damage, int means, int shield, qboolean max );

void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, const vec3_t mins, const vec3_t maxs,
						float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );
void CG_MiscModelExplosion( vec3_t mins, vec3_t maxs, int size, material_t chunkType );

void CG_TestLine( vec3_t start, vec3_t end, int time, unsigned int color, int radius);

void CG_InitGlass( void );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );
void CG_ParseLoadingScreenTips( void );

//
// cg_spawn.c
//
qboolean	CG_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	CG_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	CG_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	CG_SpawnBoolean( const char *key, const char *defaultString, qboolean *out );
qboolean	CG_SpawnVector( const char *key, const char *defaultString, float *out );
void		CG_ParseEntitiesFromString( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);

//
// cg_playerstate.c
//
int CG_IsMindTricked(int trickIndex1, int trickIndex2, int trickIndex3, int trickIndex4, int client);
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );

//[TrueView]
void CG_TrueViewInit( void );
void CG_AdjustEyePos (const char *modelName);
//[/TrueView]


//===============================================

int			BG_ProperForceIndex(int power);
void		BG_CycleForce(playerState_t *ps, int direction);

const char *CG_GetStringEdString(char *refSection, char *refName);
const char *CG_GetStringEdString2(char *refName);


void FX_TurretProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon );


void		CG_Init_CG(void);
void		CG_Init_CGents(void);


void CG_SetGhoul2Info( refEntity_t *ent, const centity_t *cent);

void CG_ShutDownG2Weapons(void);
void CG_CopyG2WeaponInstance(centity_t *cent, int weaponNum, int weaponVariation, void *toGhoul2);
void *CG_G2WeaponInstance(centity_t *cent, int weapon, int variation);
void CG_CheckPlayerG2Weapons(playerState_t *ps, centity_t *cent);

void	CG_ClearLightStyles (void);
void	CG_RunLightStyles (void);
void	CG_SetLightstyle (int i);

float JKG_CalculateSprintPhase( const playerState_t *ps );

void CG_Notifications_Add(char *string, qboolean weapon);

/*
Ghoul2 Insert End
*/

#ifdef __WEAPON_HOLSTER__
// jkg_cg_weaponholster.c
extern qboolean CG_SnapRefEntToBone(centity_t *cent, refEntity_t *refEnt, const char *bone_name);
extern void JKG_DrawWeaponHolsters( centity_t *cent, refEntity_t legs, float shadowPlane );
#endif //__WEAPON_HOLSTER__

// jkg_cg_damagetypes.c -- UQ1: Moved to stop cg_local.h recursive #include error...
void JKG_PlayerDebuffVisuals ( centity_t *cent, refEntity_t *refEntity );
void JKG_WeaponDebuffVisuals ( centity_t *cent, refEntity_t *refEntity );

// -------------
// Crossover API
// -------------

#ifdef _CGAME
#include "../ui/ui_shared.h"
extern uiCrossoverExports_t *uiImports;
#endif

extern cgameImport_t *trap;

#include "ghoul2/ghoul2_shared.h"
#include "client/FxSystem.h"

#endif
