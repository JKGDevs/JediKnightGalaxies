// bg_weapons.h
// This has been modified beyond it's original purpose. In the past, weapons were hardcoded
// with a weaponData_t struct that barely contained any useful information about the weapon,
// not enough damages, which were in a define. However, most if not all of the weapon data
// has been appropriately moved over to weaponData_t, which is filled by the .wpn files.
// (c) 2013 Jedi Knight Galaxies
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


#ifndef BG_WEAPONS_H
#define BG_WEAPONS_H

#include "qcommon/q_shared.h"
#include "bg_ammo.h"

#define MAX_WEAPON_TABLE_SIZE (255)
#define MAX_FIREMODES (16)
#define MAX_STANCES	(16)
#define MAX_SABERHILTS	(64)
#define MAX_DEBUFFS_PRESENT		8

typedef enum 
{

	AMMO_NONE,
	AMMO_FORCE,
	AMMO_BLASTER,
	AMMO_POWERCELL,
	AMMO_METAL_BOLTS,
	AMMO_CONCUSSION,
	AMMO_ROCKETS,
	AMMO_EMPLACED,
	AMMO_THERMAL,
	AMMO_TRIPMINE,
	AMMO_DETPACK,
	AMMO_MAX

} ammoType_t;

typedef enum
{

	WP_NONE,
	WP_STUN_BATON,
	WP_MELEE,
	WP_SABER,
	WP_BRYAR_PISTOL,
	WP_BLASTER,
	WP_DISRUPTOR,
	WP_BOWCASTER,
	WP_REPEATER,
	WP_DEMP2,
	WP_FLECHETTE,
	WP_CONCUSSION,
	WP_ROCKET_LAUNCHER,
	WP_THERMAL,
	WP_TRIP_MINE,
	WP_DET_PACK,
	WP_BRYAR_OLD,
	WP_EMPLACED_GUN,
	WP_TURRET,
	WP_NUM_WEAPONS

} weapon_t;

typedef struct
{

	unsigned char ammoIndex;
	short ammoClipCost;
	short ammoClipSize;
	short ammoMax;

} weaponAmmo_t;

/*
The damage radius function describes how the damage radius changes over time.

RF_CONSTANT - Damage radius stays constant.
RF_LINEAR - Damage radius changes in a linear fashion.
RF_NONLINEAR - Damage radius stays at start value until some proportion of lifetime
has passed. Following this, the radius changes linearly until the end radius is reached.
RF_CLAMP - Damage radius changes in a linear fashion to its end radius, until some
proportion of lifetime has passed.
RF_WAVE - Damage radius oscillates over time, between the start and end radius at a
given frequency.
*/
typedef enum radiusFunc_e
{
	RF_CONSTANT,
	RF_LINEAR,
	RF_NONLINEAR,
	RF_CLAMP,
	RF_WAVE
} radiusFunc_t;

/*
Damage function describes how the damage changes over distance from the centre of the
damage origin, e.g. centre of explosion.
*/
typedef enum damageFunc_e
{
	DF_CONSTANT,
	DF_LINEAR,
	DF_GAUSSIAN
} damageFunc_t;

typedef enum penetrationType_e
{
	PT_NONE,
	PT_WALLS
} penetrationType_t;

typedef struct radiusParams_s
{
	int startRadius;
	int endRadius;
	int generic1;
	damageFunc_t damageFunc;
	radiusFunc_t radiusFunc;
} radiusParams_t;

typedef struct debuffData_s
{
	int					debuff;
	float				intensity;
	int					duration;
} debuffData_t;

typedef struct damageSettings_s
{
	qboolean            radial;
	radiusParams_t      radiusParams;
	int                 lifetime;
	int                 delay;
	int                 damage;
	int                 damageDelay;
	penetrationType_t   penetrationType;
	qboolean            planar;
	int					numberDebuffs;
	debuffData_t		debuffs[MAX_DEBUFFS_PRESENT];
	int					damageFlags;
} damageSettings_t;

typedef enum
{
    ZOOM_NONE,
    ZOOM_CONTINUOUS,
    ZOOM_TOGGLE
} zoomMode_t;

typedef enum firingType_e
{
    FT_AUTOMATIC,
    FT_SEMI,
    FT_BURST
} firingType_t;

typedef enum
{
	SPRINTSTYLE_CUSTOM = -1,
	SPRINTSTYLE_NONE,
	SPRINTSTYLE_LOWERED,
	SPRINTSTYLE_LOWERED_SLIGHT,
	SPRINTSTYLE_LOWERED_HEAVY,
	SPRINTSTYLE_SIDE,
	SPRINTSTYLE_SIDE_SLIGHT,
	SPRINTSTYLE_SIDE_HEAVY,
	SPRINTSTYLE_RAISED,
	SPRINTSTYLE_RAISED_SLIGHT,
	SPRINTSTYLE_RAISED_HEAVY,
	SPRINTSTYLE_ANGLED_DOWN,
	SPRINTSTYLE_ANGLED_DOWN_SLIGHT,			// copy bob from raised
	SPRINTSTYLE_ANGLED_DOWN_HEAVY,			// copy bob from raised
	SPRINTSTYLE_SIDE_UP,
	SPRINTSTYLE_SIDE_UP_SLIGHT,				// copy bob from side
	SPRINTSTYLE_SIDE_UP_HEAVY,				// copy bob from side
	SPRINTSTYLE_SIDE_MEDIUM,
	SPRINTSTYLE_SIDE_MEDIUM_SLIGHT,			// copy bob from side
	SPRINTSTYLE_SIDE_MEDIUM_HEAVY,			// copy bob from side
} sprintStyles_t;

typedef enum
{
	FMANIM_NONE,							// no change in animation
	FMANIM_RAISED,							// rasied, used for grenade launcher and flechette gun mines
	FMANIM_TILTED,							// tilted to the right a little, for the anti armor attachment on clone rifle
} firingModeAnim_t;

typedef enum
{
	FMTRANS_RAISED_NONE,					// had to fix this because this was causing issues --eez
	FMTRANS_NONE_NONE,
	FMTRANS_NONE_RAISED,
	FMTRANS_NONE_TILTED,
	FMTRANS_RAISED_RAISED,
	FMTRANS_RAISED_TILTED,
	FMTRANS_TILTED_NONE,
	FMTRANS_TILTED_RAISED,
	FMTRANS_TILTED_TILTED,
} firingModeTransitions_t;

// FIXME: Make this serverside?
// Although, if we make this serverside, we can't make the accuracy of the gun directly affect the crosshair size unless we keep
// that variable in the playerstate, which is just plain nasty imo.
typedef struct
{
	float accuracyRating;		// diameter of spread at 100 units away, in map units
	float crouchModifier;
	float runModifier;
	float walkModifier;		// this gets used in the case of crouch-walking, regardless of walking button being pressed
	float sightsModifier;	// is lerped during sights transition
	float inAirModifier;	// if we're in the air period

	// extra accuracy added on from weapon fire
	int accuracyRatingPerShot;		// add this much accuracy rating from one shot
	int msToDrainAccuracy;			// ms to drain 1 accuracy rating (NOTE: exploitable)
	int maxAccuracyAdd;				// maximum amount that accuracy can be dropped by
} weaponAccuracyDetails_t;

typedef struct weaponFireModeStats_s
{
									// Charge Base Damage: baseDamage * (( chargeTime / chargeDrain ) * chargeMuliplier )
	short       baseDamage;		    // The amount of base damage this weapon does, goes through modifiers for skills.
	damageSettings_t   primary;		// For use with the more complex damage type system.
	damageSettings_t   secondary;	// For secondary damage..
	qboolean	secondaryDmgPresent;
	char		applyGravity;		// If true, bolt is affected by gravity and will act accordingly. Speed will decide the forward thrust.
	char		bounceCount;		// The amount of bounces this weapon has, if any.
	char		hitscan;        	// Is this weapon hitscan?
	char		shotCount;			// The number of shots this weapon makes per fire (Flechette shoots more for instance)
	float		boxSize;			// The box size for a shot. By default 1.0 (like a blaster) and can increase for charged weapons.
	short		chargeMaximum;		// The maximum amount of time charged.
	float		chargeMultiplier;	// The multiplier to apply on charged shot damage calculation.
	short		chargeTime;		    // The time before substracting a drain.
	int			chargeSubtract;		// The amount of ammo subtracted from a drain
	char		cost;				// The ammo cost to shoot the weapon.
	short		delay;				// The delay between each shot/throw/burstfire/whatever.
	float		range;				// The maximum amount of range this weapon/mode can reach.
	float		decayRate;			// The rate the weapons damage decays when fired beyond the maximum range (default 0.25).
	float		rangeSplash;		// The possible splash damage range. Damage calculation is done based on point of impact to end of radius.
	float		recoil;			    // The weapon recoil to smash into the camera, repeaters for instance have low but much recoil.
	float		speed;				// The speed override, set different then 0 to avoid using global speed.
	firingType_t    firingType;     // Firing type (auto, semi, burst)
	char        shotsPerBurst;      // Shots per burst
	short       burstFireDelay;     // Delay between firing in a burst.
	char	    weaponClass[32];	// The projectile class name for server reference and information.
	int		    weaponMOD;			// The MOD (Means of Death) for this mode and weapon for direct contact.
	int			weaponSplashMOD;	// The MOD (Means of Death) for this mode and weapon for splash damage.
	float		ACPRatio;			// If MOD is MOD_ACP, the ratio that is energy vs physical applied to the target
	qboolean    isGrenade;          // Is this firemode a grenade?
	qboolean	grenadeBounces;		// Does this grenade bounce off of enemies, or does it explode on impact? (true for bounces on people)
	int			grenadeBounceDMG;	// Determines the amount of damage to do when bouncing off of an enemy.
	qboolean	useQuantity;		// Subtracts from quantity instead of ammo index when firing.
									// This is separate because some fire modes (ie sequencer charge detonate) don't consume ammo
	ammo_t*		ammoBase;			// The base ammo type that this weapon accepts
	ammo_t*		ammoDefault;		// The default ammo type (BUY AMMO) that is used
	int			clipSize;			// The size of one clip
	float		heatGenerated;		// How much heat is generated by firing this mode
	int			maxHeat;			// If current heat is greater than this number, we overheat
	int			heatThreshold;		// After overheating, weapon needs too cool to this amount before it can fire again (also warns players they're about to overheat)
	int			reloadClearHeat;	// Percent of how much heat to flush if you reload a weapon while it has heat.

	weaponAccuracyDetails_t weaponAccuracy; // replaces spread
} weaponFireModeStats_t;

typedef enum indicatorType_e
{
    IND_NONE,
    IND_NORMAL,
    IND_GRENADE
} indicatorType_t;

// This should roughly match up with weaponDrawData_t in
// cg_weapons.h.
typedef struct weaponVisualFireMode_s
{
    char type[16];
	char				displayName[128];	// what to display as the name of it ("Burst", "Grenade Launcher", etc)
	firingModeAnim_t	animType;			// which special anim to use
	char				switchToSound[MAX_QPATH];
	char				crosshairShader[MAX_QPATH];	// what to change the crosshair to (if applicable)
	short				overrideIndicatorFrame;	 // frame to override the firing mode shader with (since these aren't always in order)
	qboolean			displayExplosive;	// display both Blast Damage and Fallout Damage in the UI?

    union
    {
        struct
        {
            float muzzleLightIntensity;
            char muzzleLightColor[16];
            char chargingEffect[MAX_QPATH];
            char muzzleEffect[MAX_QPATH];
        } generic;
    } weaponRender;
    
    union
    {
        struct
        {
            char fireSound[8][MAX_QPATH];
        } generic;
    } weaponFire;
    
    union
    {
        struct
        {
            char tracelineShader[MAX_QPATH];
            float minSize;
            float maxSize;
            int lifeTime;
        } generic;
    } tracelineRender;
    
    union
    {
        char chargingSound[MAX_QPATH];
    } weaponCharge;
    
    union
    {
        struct
        {
            char projectileModel[MAX_QPATH];
            char projectileEffect[MAX_QPATH];
            char runSound[MAX_QPATH];
            
            float lightIntensity;
            char lightColor[16];
            
            char deathEffect[MAX_QPATH];
        } generic;
    } projectileRender;
    
    union
    {
        struct
        {
            char impactEffect[MAX_QPATH];
        } generic;
        
        struct
        {
            char stickSound[MAX_QPATH];
        } explosive;
        
        struct
        {
            char impactEffect[MAX_QPATH];
            char shockwaveEffect[MAX_QPATH];
        } grenade;
    } projectileMiss;
    
    union
    {
        struct
        {
            char impactEffect[MAX_QPATH];
			char impactShieldEffect[MAX_QPATH];
        } generic;
        
        struct
        {
            char impactEffect[MAX_QPATH];
            char shockwaveEffect[MAX_QPATH];
        } grenade;
    } projectileHitPlayer;
    
    union
    {
        struct
        {
            char deflectEffect[MAX_QPATH];
        } generic;
    } projectileDeflected;
    
    union
    {
        struct
        {
            char bounceSound[2][MAX_QPATH];
        } grenade;
    } grenadeBounce;
    
    union
    {
        struct
        {
            char g2Model[MAX_QPATH];
            float g2Radius;
        } detpack;
        
        struct
        {
            char g2Model[MAX_QPATH];
            float g2Radius;
            char lineEffect[MAX_QPATH];
        } tripmine;
    } explosiveRender;
    
    union
    {
        struct
        {
            char explodeEffect[MAX_QPATH];
        } generic;
    } explosiveBlow;
    
    union
    {
        char armSound[MAX_QPATH];
    } explosiveArm;
} weaponVisualFireMode_t;

typedef struct weaponVisual_s
{
#if defined (_CGAME) || (IN_UI)
	char description[512];			// The description of this weapon to display in UI.
#endif	
	// Server needs to know the world model for its Ghoul 2 instances.
	char world_model[MAX_QPATH];	// The model used for 3D rendering.
#if defined (_CGAME) || (IN_UI)
	char view_model[MAX_QPATH];		// The model used when in first person mode.
	
	char icon[MAX_QPATH];		    // The icon of this weapon to be used in the HUD.
	char icon_na[MAX_QPATH];        // Not available icon
	
	char selectSound[MAX_QPATH];
	
	char firemodeIndicatorShader[MAX_QPATH];
	char groupedIndicatorShaders[3][MAX_QPATH];
	indicatorType_t indicatorType;
	
	char gunPosition[16];
	char ironsightsPosition[16];
	float ironsightsFov;
	
	char scopeShader[MAX_QPATH];
	char scopeStartSound[MAX_QPATH];
    char scopeStopSound[MAX_QPATH];
    int scopeSoundLoopTime;
    char scopeLoopSound[MAX_QPATH];

	// eez: variable number of barrels, default is 0
	int barrelCount;

	weaponVisualFireMode_t visualFireModes[MAX_FIREMODES];
#endif
} weaponVisual_t;

// create one instance of all the weapons we are going to use so we can just copy this info into each clients gun ghoul2 object in fast way
struct g2WeaponInstance_s {
    unsigned int weaponNum;
    unsigned int weaponVariation;
    
    void *ghoul2;
};
extern g2WeaponInstance_s g2WeaponInstances[MAX_WEAPON_TABLE_SIZE];

void BG_InitWeaponG2Instances(void);
void *BG_GetWeaponGhoul2 ( int weaponNum, int weaponVariation );
void BG_ShutdownWeaponG2Instances(void);

typedef struct
{
	int torsoAnim;
	int legsAnim;
} weaponAnimationSet_t;

typedef struct
{
	weaponAnimationSet_t firing;
	weaponAnimationSet_t ready;
	weaponAnimationSet_t reload;
	weaponAnimationSet_t forwardWalk;
	weaponAnimationSet_t backwardWalk;
	weaponAnimationSet_t crouchWalk;
	weaponAnimationSet_t crouchWalkBack;
	weaponAnimationSet_t jump;
	weaponAnimationSet_t land;
	weaponAnimationSet_t run;
	weaponAnimationSet_t sprint;
	weaponAnimationSet_t sights;
	weaponAnimationSet_t sightsFiring;
} weaponAnimationReplacements_t;

// Sabers are a special breed.
// The hilt parameters are defined in .sab files, literally no difference from how Raven handles it.
// However, should we be on variation 0, we use a special, AWESOME hilt customization system.
// Of course, due to modeling and manpower constraints, we aren't doing a whole lot of the awesome hilt customization stuff yet.
// Right, so every other variation uses a "saberData" block, this defines the hilt and the default crystal that the item
// happens to be stocked with on spawn.
// So for example, Desann's saber always starts with a red crystal in it, etc

typedef struct
{
	char			hiltname[MAX_QPATH];		// We get this from the .sab files.
	char			defaultcrystal[MAX_QPATH];	// Crystal that gets chucked into this puppy by default...
} saberWeaponData_t;

typedef struct
{
    char            classname[MAX_QPATH];   // Class name..

	unsigned char	weaponBaseIndex;		// Base index, determines the type of weapon this is.
	unsigned char	weaponModIndex;			// Mod index, determines which alternate version of the base weapon this is.
	unsigned short	weaponReloadTime;		// The time required to reload the weapon (or the time till she blows, for grenades).

	unsigned char	numFiringModes;			// Number of firing modes on the gun.

	unsigned char	hasCookAbility;			// Determines whether or not this weapon can be cooked (grenades only).
	unsigned char	hasKnockBack;			// The amount of damage determines the knockback of the weapon.
	unsigned char	hasRollAbility;			// Determines whether or not you can roll with this weapon.
	
	unsigned char	zoomType;		    	// Determines whether or not the weapon has the ability to zoom.
	float           startZoomFov;           // Starting FOV when zooming
	float           endZoomFov;             // Max FOV when zooming
	unsigned int    zoomTime;               // Time in milliseconds it takes to zoom all the way in
	
	float           speedModifier;          // Modifier to apply to player's speed when weapon in use. 1.0f for default speed.
	float           reloadModifier;         // Modifier to apply to player's speed when reloading. 1.0f for no change.
	unsigned int	swapTime;				// Base time in milliseconds it takes to swap to a different weapon.

	unsigned char	ironsightsTime;			// How long it takes to bring the weapon up to ironsights

	weaponAnimationReplacements_t anims;

	int firstPersonSprintStyle;				// Handles the way sprinting is done.
											// -1 = custom animation defined in MD3
											// 0 = no animation
											// 1 = lowered weapon
											// 2 = lowered weapon, slight bob
											// 3 = lowered weapon, heavy bob
											// 4 = side weapon
											// 5 = side weapon, slight bob
											// 6 = side weapon, heavy bob
											// 7 = lowered, angled weapon
											// 8 = lowered, angled weapon, slight bob
											// 9 = lowered, angled weapon, heavy bob
											// 10 = raised
											// 11 = raised, slight bob
											// 12 = raised, heavy bob

	weaponFireModeStats_t firemodes[MAX_FIREMODES];

	saberWeaponData_t sab;
    
    char displayName[64];			// The name which is to be displayed on the HUD.
    
    weaponVisual_t visuals;

} weaponData_t;

typedef enum
{

	WPS_NONE = 0,
	WPS_GRENADE,
	WPS_SECONDARY,
	WPS_PRIMARY

} weaponSlot_t;

/* This is the main function to get weapon data for each variation */
void            BG_InitializeWeapons ( void );
void            BG_InitializeWeaponData ( weaponData_t *weaponData );
void			BG_ShutdownWeapons();
qboolean        BG_WeaponVariationExists ( unsigned int weaponId, unsigned int variation );
int             BG_GetWeaponIndex ( unsigned int weapon, unsigned int variation );
qboolean        BG_GetWeaponByIndex ( int index, int *weapon, int *variation );
weaponData_t   *BG_GetWeaponByClassName ( const char *className );
int             BG_GetWeaponIndexFromClass ( int weapon, int variation );
qboolean		BG_WeaponCanUseSpecialAmmo ( weaponData_t* wp );

unsigned int BG_NumberOfLoadedWeapons ( void );
unsigned int BG_NumberOfWeaponVariations ( unsigned char weaponId );
weaponData_t *BG_GetWeaponDataByIndex( int index );

//void			GetWeaponInitialization( void );
weaponData_t   *GetWeaponData( unsigned char baseIndex, unsigned char modIndex );
weaponData_t   *GetWeaponDataUnsafe ( unsigned char weapon, unsigned char variation );
short           GetAmmoMax ( unsigned char ammoIndex );

qboolean BG_DumpWeaponList ( const char *filename );
void BG_PrintWeaponList( void );

/* Original definitions used for weapon switching and such, we won't use it eventually */
#define LAST_USEABLE_WEAPON			WP_BRYAR_OLD		// anything > this will be considered not player useable
#define FIRST_WEAPON				WP_BRYAR_PISTOL		// this is the first weapon for next and prev weapon switching
#define MAX_PLAYER_WEAPONS			WP_NUM_WEAPONS - 1	// this is the max you can switch to and get with the give all.

/* These are the weapon ranges to be used by any weapon. Excepts are possible, as always */
#define WPR_S						3072.0f		/* Short Range */
#define WPR_M						3584.0f		/* Medium Range */
#define WPR_L						4096.0f		/* Long Range */
#define WPR_I						  -1.0f		/* Infinite Range */

/* These externals contain all the data we want and need! */
extern	weaponAmmo_t				xweaponAmmo[];

typedef struct
{
	union {
		char *a;
		int i;
		float f;
		unsigned char uc;
	} data;
	//void *data;
	qboolean isFloat;
	qboolean isString;
	int byteCount;
} weaponDataGrab_t;

//======================================
// Here be sabers
//======================================

typedef struct
{
	unsigned short crystalID;
	vec3_t vRGB;					// glow RGB
	vec3_t bRGB;					// blade RGB
	char crystalName[64];			// can be referenced via files
	char bladeEffect[64];			// "none" for defaults
	char trailEffect[64];			// "none" for defaults
	char glowEffect[64];			// "none" for defaults
} saberCrystalData_t;

#define MAX_SABER_CRYSTALS			256
#define MAX_CRYSTAL_FILE_SIZE		(262144) // 64kb

extern saberCrystalData_t saberCrystalsLookup[MAX_SABER_CRYSTALS];

void JKG_InitializeSaberCrystalData( void );
const saberCrystalData_t *JKG_GetSaberCrystal( const char *crystalName );

typedef struct
{
	char baseName[64];						// Not sure how base uses these. just replicate them to be on the safe side --eez

	int FPdrain;						// drain this much FP on using this move

	int chainIdle;						// What move to call if the attack button is not pressed at the end of this anim
	int chainAttack;					// What move to call if the attack button (and nothing else) is pressed

	int startingQuadrant;				// starts at this quadrant
	int endQuadrant;					// and ends at this one
	
	int blockType;						// BLK_NO, BLK_TIGHT, BLK_WIDE

	int trailLen;						// trail length in move. unknown unit of measure

	int blendTime;						// blend time for the animation
	int setanimflag;					// set anim flag		
	int anim;							// use this anim
	float animspeedscale;				// alters the speed of this move
} saberMoveExternal_t;

typedef struct
{
	qboolean allowBackStab;
	qboolean allowBackAttack;
	qboolean allowCrouchedBackAttack;
	qboolean allowRollStab;
	qboolean allowLunge; // aka "blue DFA"
	qboolean allowLeapAttack; // aka "red DFA"
	qboolean allowFlipStab;	// unused in MP
	qboolean allowFlipSlash; // aka "yellow DFA" 
	qboolean allowButterfly;
	qboolean allowCartwheel;
	qboolean allowBackflipAttack; // unused in MP
	qboolean allowDualSpinAttack;	// i think this is Dual Kata, but unsure --eez
	qboolean allowSpeedLunge;
	qboolean allowStabDown;
	qboolean allowSpinAttack;	// possibly unused
	qboolean allowSoulCal; // was staff kata, but could be reusued
	qboolean allowBlueKata; // different controls?
	qboolean allowYellowKata;
	qboolean allowRedKata;
	qboolean allowUpsideDown; // unused in MP
	qboolean allowPullStab; // unused in MP
	qboolean allowPullSlash; // unused in MP
	qboolean allowAloraSpin; // unused in MP
	qboolean allowDualFrontBack;
	qboolean allowDualSides;
	qboolean allowHiltBash; // unused in MP
} saberSpecial_t;

typedef struct
{
	// Moves
	saberMoveExternal_t moves[162];		// FIXME: INCORRECT MAX!! this should be LS_MOVE_MAX but this file doesn't have access to that enum
	saberSpecial_t specialMoves;
	int chainStyle;						// 0 = chain like blue, 1 = chain like red
	int maxChainCount;					// maximum number of moves in a chain
	float attackSpeedModifier;			// Modify movement speed while running with each attack by this amount
	float transitionSpeedModifier;		// Modify movement speed while running while in a transition by this amount
	float attackBackSpeedModifier;		// same as attackSpeedModifier, but when running backwards
	int baseDamageDefault;
	int baseDamageMin;
	int baseDamageMax;
	int BPdrain;						// how much to drain BP (based on NUMBER of PLANES)


	int defensePenetration;				// higher numbers for faster stances.
										// baseJA values:
										// Blue - 24
										// Yellow - 16
										// Red - 8
										// All others - 16 (this is why duals are so OP)
	int defense;						// Base defense level

	// Basic data
	char saberName_technical[64];			// "Makashi", "Soresu", "Ataru" et al
	char saberName_simple[64];				// "Fast", "Medium", "Strong" et al

	int offensiveStanceAnim;			// anim while standing still
	int projectileBlockAnim;			// use this anim in projectile block mode

	// Accessiblity
	qboolean isStaffFriendly;			// can be used with staffs and single?
	qboolean isDualsFriendly;			// can be used with duals and single?
	qboolean isStaffOnly;				// Staff only? (no use in singles period)
	qboolean isDualsOnly;				// Duals only? (no use in singles period)
	qboolean sabSpecific;				// only allowed for use in specific vibroblade weapons (unused as of 12/7/12)

	// Saber throw
	float saberThrowSpeed;
	float saberThrowPitch;
	float saberThrowYaw;
	float saberThrowRoll;
	char saberThrowSound[MAX_QPATH];
} saberStanceExternal_t;

extern saberStanceExternal_t SaberStances[MAX_STANCES];

void JKG_InitializeStanceData( void );

// Is actually defined in bg_ammo.cpp, just here to shut the compiler up
int BG_GetRefillAmmoCost(unsigned short* ammo, weaponData_t* wp);

#endif
