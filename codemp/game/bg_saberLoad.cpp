/*
===========================================================================
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

// bg_saberLoad.c
#include "cgame/animtable.h"
#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "w_saber.h"

#include <json/cJSON.h>
#include <cctype>
#include <string>
#include <unordered_map>

#if defined(_GAME)
	#include "g_local.h"
#elif defined(IN_UI)
	#include "../ui/ui_local.h"
#elif defined(_CGAME)
	#include "../cgame/cg_local.h"
#endif

saberStanceExternal_t SaberStances[MAX_STANCES];

class CaseInsensitiveHash
{
public:
	std::size_t operator()( const std::string& s ) const
	{
		std::size_t hash = 0;
		for ( std::size_t i = 0; i < s.size(); i++ )
		{
			int c = std::tolower( s[i] );
			hash += c * (i + 119);
		}

		return hash ^ (hash >> 10) ^ (hash >> 20);
	}
};

std::unordered_map<std::string, saberInfo_t, CaseInsensitiveHash> *hiltLookupTable;

#ifdef _GAME
extern int G_SoundIndex( const char *name );
#endif

int BG_SoundIndex(char *sound)
{
#if defined(_GAME)
	return G_SoundIndex(sound);
#elif defined(_CGAME) || defined(IN_UI)
	return trap->S_RegisterSound(sound);
#endif
}

extern stringID_table_t FPTable[];

//#define MAX_SABER_DATA_SIZE 0x80000
//static char SaberParms[MAX_SABER_DATA_SIZE];

stringID_table_t SaberTable[] =
{
	ENUM2STRING(SABER_NONE),
	ENUM2STRING(SABER_SINGLE),
	ENUM2STRING(SABER_STAFF),
	ENUM2STRING(SABER_BROAD),
	ENUM2STRING(SABER_PRONG),
	ENUM2STRING(SABER_DAGGER),
	ENUM2STRING(SABER_ARC),
	ENUM2STRING(SABER_SAI),
	ENUM2STRING(SABER_CLAW),
	ENUM2STRING(SABER_LANCE),
	ENUM2STRING(SABER_STAR),
	ENUM2STRING(SABER_TRIDENT),
	{"",	-1}
};

stringID_table_t SaberMoveTable[] =
{
	ENUM2STRING(LS_NONE),
	// Attacks
	ENUM2STRING(LS_A_TL2BR),
	ENUM2STRING(LS_A_L2R),
	ENUM2STRING(LS_A_BL2TR),
	ENUM2STRING(LS_A_BR2TL),
	ENUM2STRING(LS_A_R2L),
	ENUM2STRING(LS_A_TR2BL),
	ENUM2STRING(LS_A_T2B),
	ENUM2STRING(LS_A_BACKSTAB),
	ENUM2STRING(LS_A_BACK),
	ENUM2STRING(LS_A_BACK_CR),
	ENUM2STRING(LS_ROLL_STAB),
	ENUM2STRING(LS_A_LUNGE),
	ENUM2STRING(LS_A_JUMP_T__B_),
	ENUM2STRING(LS_A_FLIP_STAB),
	ENUM2STRING(LS_A_FLIP_SLASH),
	ENUM2STRING(LS_JUMPATTACK_DUAL),
	ENUM2STRING(LS_JUMPATTACK_ARIAL_LEFT),
	ENUM2STRING(LS_JUMPATTACK_ARIAL_RIGHT),
	ENUM2STRING(LS_JUMPATTACK_CART_LEFT),
	ENUM2STRING(LS_JUMPATTACK_CART_RIGHT),
	ENUM2STRING(LS_JUMPATTACK_STAFF_LEFT),
	ENUM2STRING(LS_JUMPATTACK_STAFF_RIGHT),
	ENUM2STRING(LS_BUTTERFLY_LEFT),
	ENUM2STRING(LS_BUTTERFLY_RIGHT),
	ENUM2STRING(LS_A_BACKFLIP_ATK),
	ENUM2STRING(LS_SPINATTACK_DUAL),
	ENUM2STRING(LS_SPINATTACK),
	ENUM2STRING(LS_LEAP_ATTACK),
	ENUM2STRING(LS_SWOOP_ATTACK_RIGHT),
	ENUM2STRING(LS_SWOOP_ATTACK_LEFT),
	ENUM2STRING(LS_TAUNTAUN_ATTACK_RIGHT),
	ENUM2STRING(LS_TAUNTAUN_ATTACK_LEFT),
	ENUM2STRING(LS_KICK_F),
	ENUM2STRING(LS_KICK_B),
	ENUM2STRING(LS_KICK_R),
	ENUM2STRING(LS_KICK_L),
	ENUM2STRING(LS_KICK_S),
	ENUM2STRING(LS_KICK_BF),
	ENUM2STRING(LS_KICK_RL),
	ENUM2STRING(LS_KICK_F_AIR),
	ENUM2STRING(LS_KICK_B_AIR),
	ENUM2STRING(LS_KICK_R_AIR),
	ENUM2STRING(LS_KICK_L_AIR),
	ENUM2STRING(LS_STABDOWN),
	ENUM2STRING(LS_STABDOWN_STAFF),
	ENUM2STRING(LS_STABDOWN_DUAL),
	ENUM2STRING(LS_DUAL_SPIN_PROTECT),
	ENUM2STRING(LS_STAFF_SOULCAL),
	ENUM2STRING(LS_A1_SPECIAL),
	ENUM2STRING(LS_A2_SPECIAL),
	ENUM2STRING(LS_A3_SPECIAL),
	ENUM2STRING(LS_UPSIDE_DOWN_ATTACK),
	ENUM2STRING(LS_PULL_ATTACK_STAB),
	ENUM2STRING(LS_PULL_ATTACK_SWING),
	ENUM2STRING(LS_SPINATTACK_ALORA),
	ENUM2STRING(LS_DUAL_FB),
	ENUM2STRING(LS_DUAL_LR),
	ENUM2STRING(LS_HILT_BASH),
	{"",	-1}
};

saber_colors_t TranslateSaberColor( const char *name ) 
{
	if ( !Q_stricmp( name, "red" ) ) 
	{
		return SABER_RED;
	}
	if ( !Q_stricmp( name, "orange" ) ) 
	{
		return SABER_ORANGE;
	}
	if ( !Q_stricmp( name, "yellow" ) ) 
	{
		return SABER_YELLOW;
	}
	if ( !Q_stricmp( name, "green" ) ) 
	{
		return SABER_GREEN;
	}
	if ( !Q_stricmp( name, "blue" ) ) 
	{
		return SABER_BLUE;
	}
	if ( !Q_stricmp( name, "purple" ) ) 
	{
		return SABER_PURPLE;
	}
	if ( !Q_stricmp( name, "random" ) ) 
	{
		return ((saber_colors_t)(Q_irand( SABER_ORANGE, SABER_PURPLE )));
	}
	return SABER_BLUE;
}

const char *SaberColorToString( saber_colors_t color ) {
	if ( color == SABER_RED )		return "red";
	if ( color == SABER_ORANGE )	return "orange";
	if ( color == SABER_YELLOW )	return "yellow";
	if ( color == SABER_GREEN )		return "green";
	if ( color == SABER_BLUE )		return "blue";
	if ( color == SABER_PURPLE )	return "purple";

	return NULL;
}

saber_styles_t TranslateSaberStyle( const char *name ) 
{
	if ( !Q_stricmp( name, "fast" ) ) 
	{
		return SS_MAKASHI;
	}
	if ( !Q_stricmp( name, "medium" ) ) 
	{
		return SS_SHII_CHO;
	}
	if ( !Q_stricmp( name, "strong" ) ) 
	{
		return SS_SORESU;
	}
	if ( !Q_stricmp( name, "desann" ) ) 
	{
		return SS_ATARU;
	}
	if ( !Q_stricmp( name, "tavion" ) ) 
	{
		return SS_JUYO;
	}
	if ( !Q_stricmp( name, "dual" ) ) 
	{
		return SS_DUAL;
	}
	if ( !Q_stricmp( name, "staff" ) ) 
	{
		return SS_STAFF;
	}
	return SS_NONE;
}

saberType_t TranslateSaberType( const char *name ) {
	if ( !Q_stricmp( name, "SABER_SINGLE" ) )		return SABER_SINGLE;
	if ( !Q_stricmp( name, "SABER_STAFF" ) ) 		return SABER_STAFF;
	if ( !Q_stricmp( name, "SABER_DAGGER" ) ) 		return SABER_DAGGER;
	if ( !Q_stricmp( name, "SABER_BROAD" ) ) 		return SABER_BROAD;
	if ( !Q_stricmp( name, "SABER_PRONG" ) ) 		return SABER_PRONG;
	if ( !Q_stricmp( name, "SABER_ARC" ) )			return SABER_ARC;
	if ( !Q_stricmp( name, "SABER_SAI" ) )			return SABER_SAI;
	if ( !Q_stricmp( name, "SABER_CLAW" ) )			return SABER_CLAW;
	if ( !Q_stricmp( name, "SABER_LANCE" ) )		return SABER_LANCE;
	if ( !Q_stricmp( name, "SABER_STAR" ) )			return SABER_STAR;
	if ( !Q_stricmp( name, "SABER_TRIDENT" ) )		return SABER_TRIDENT;
	if ( !Q_stricmp( name, "SABER_SITH_SWORD" ) )	return SABER_SITH_SWORD;

	return SABER_SINGLE;
}

bool WP_SaberBladeDoTransitionDamage( saberInfo_t *saber, int bladeNum )
{
	if ( saber->saberFlags2&SFL2_TRANSITION_DAMAGE )
	{//use first blade style for this blade
		return true;
	}
	return false;
}

qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel )
{
	qboolean styleInvalid = qfalse;
	qboolean saber1Active;
	qboolean saber2Active;
	qboolean dualSabers = qfalse;
	int	validStyles = 0, styleNum;

	if ( saber2 && saber2->model[0] )
	{
		dualSabers = qtrue;
	}

	if ( dualSabers )
	{//dual
		if ( saberHolstered > 1 )
		{
			saber1Active = saber2Active = qfalse;
		}
		else if ( saberHolstered > 0 )
		{
			saber1Active = qtrue;
			saber2Active = qfalse;
		}
		else
		{
			saber1Active = saber2Active = qtrue;
		}
	}
	else
	{
		saber2Active = qfalse;
		if ( !saber1
			|| !saber1->model[0] )
		{
			saber1Active = qfalse;
		}
		else if ( saber1->numBlades > 1 )
		{//staff
			if ( saberHolstered > 1 )
			{
				saber1Active = qfalse;
			}
			else
			{
				saber1Active = qtrue;
			}
		}
		else
		{//single
			if ( saberHolstered )
			{
				saber1Active = qfalse;
			}
			else
			{
				saber1Active = qtrue;
			}
		}
	}

	//initially, all styles are valid
	for ( styleNum = 0; styleNum < MAX_STANCES; styleNum++ )
	{
		validStyles |= (1<<styleNum);
	}

	if ( saber1Active
		&& saber1
		&& saber1->model[0]
		&& saber1->stylesForbidden )
	{
		if ( (saber1->stylesForbidden&(1<<*saberAnimLevel)) )
		{//not a valid style for first saber!
			styleInvalid = qtrue;
			validStyles &= ~saber1->stylesForbidden;
		}
	}
	if ( dualSabers )
	{//check second saber, too
		if ( saber2Active
			&& saber2->stylesForbidden )
		{
			if ( (saber2->stylesForbidden&(1<<*saberAnimLevel)) )
			{//not a valid style for second saber!
				styleInvalid = qtrue;
				//only the ones both sabers allow is valid
				validStyles &= ~saber2->stylesForbidden;
			}
		}
	}
	if ( styleInvalid && validStyles )
	{//using an invalid style and have at least one valid style to use, so switch to it
		int styleNum;
		for ( styleNum = 0; styleNum < MAX_STANCES; styleNum++ )
		{
			if ( (validStyles&(1<<styleNum)) )
			{
				*saberAnimLevel = styleNum;
				return qtrue;
			}
		}
	}
	return qfalse;
}

qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel )
{
	qboolean saber1Active;
	qboolean saber2Active;
	qboolean dualSabers = qfalse;

	if ( saber2 && saber2->model[0] )
	{
		dualSabers = qtrue;
	}

	if ( dualSabers )
	{//dual
		if ( saberHolstered > 1 )
		{
			saber1Active = saber2Active = qfalse;
		}
		else if ( saberHolstered > 0 )
		{
			saber1Active = qtrue;
			saber2Active = qfalse;
		}
		else
		{
			saber1Active = saber2Active = qtrue;
		}
	}
	else
	{
		saber2Active = qfalse;
		if ( !saber1
			|| !saber1->model[0] )
		{
			saber1Active = qfalse;
		}
		else if ( saber1->numBlades > 1 )
		{//staff
			if ( saberHolstered > 1 )
			{
				saber1Active = qfalse;
			}
			else
			{
				saber1Active = qtrue;
			}
		}
		else
		{//single
			if ( saberHolstered )
			{
				saber1Active = qfalse;
			}
			else
			{
				saber1Active = qtrue;
			}
		}
	}

	if ( saber1Active
		&& saber1
		&& saber1->model[0]
		&& saber1->stylesForbidden )
	{
		if ( (saber1->stylesForbidden&(1<<saberAnimLevel)) )
		{//not a valid style for first saber!
			return qfalse;
		}
	}
	if ( dualSabers
		&& saber2Active
		&& saber2
		&& saber2->model[0] )
	{
		if ( saber2->stylesForbidden )
		{//check second saber, too
			if ( (saber2->stylesForbidden&(1<<saberAnimLevel)) )
			{//not a valid style for second saber!
				return qfalse;
			}
		}
	}
	return qtrue;
}

qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber )
{
	if ( saber->bladeStyle2Start > 0
		&& saber->numBlades > saber->bladeStyle2Start )
	{
		if ( (saber->saberFlags2&SFL2_NO_MANUAL_DEACTIVATE)
			&& (saber->saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
		{//all blades are always on
			return qfalse;
		}
	}
	else
	{
		if ( (saber->saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
		{//all blades are always on
			return qfalse;
		}
	}
	//you can turn some off
	return qtrue;
}

void WP_SaberSetDefaults( saberInfo_t *saber )
{
	int i;

	//Set defaults so that, if it fails, there's at least something there
	for ( i = 0; i < MAX_BLADES; i++ )
	{
		saber->blade[i].color = SABER_RED;
		saber->blade[i].radius = SABER_RADIUS_STANDARD;
		saber->blade[i].lengthMax = 32;
	}

	Q_strncpyz( saber->name, DEFAULT_SABER, sizeof( saber->name ) );
	Q_strncpyz( saber->fullName, "lightsaber", sizeof( saber->fullName ) );
	Q_strncpyz( saber->model, DEFAULT_SABER_MODEL, sizeof( saber->model ) );
	saber->skin = 0;
	saber->soundOn = BG_SoundIndex( "sound/weapons/saber/enemy_saber_on.wav" );
	saber->soundLoop = BG_SoundIndex( "sound/weapons/saber/saberhum3.wav" );
	saber->soundOff = BG_SoundIndex( "sound/weapons/saber/enemy_saber_off.wav" );
	saber->numBlades = 1;
	saber->type = SABER_SINGLE;
	saber->stylesLearned = 0;
	saber->stylesForbidden = 0;//allow all styles
	saber->maxChain = 0;//0 = use default behavior
	saber->forceRestrictions = 0;
	saber->lockBonus = 0;
	saber->parryBonus = 0;
	saber->breakParryBonus = 0;
	saber->breakParryBonus2 = 0;
	saber->disarmBonus = 0;
	saber->disarmBonus2 = 0;
	saber->singleBladeStyle = SS_NONE;//makes it so that you use a different style if you only have the first blade active
//	saber->brokenSaber1 = NULL;//if saber is actually hit by another saber, it can be cut in half/broken and will be replaced with this saber in your right hand
//	saber->brokenSaber2 = NULL;//if saber is actually hit by another saber, it can be cut in half/broken and will be replaced with this saber in your left hand
//===NEW========================================================================================
	//done in cgame (client-side code)
	saber->saberFlags = 0;					//see all the SFL_ flags
	saber->saberFlags2 = 0;					//see all the SFL2_ flags

	saber->spinSound = 0;					//none - if set, plays this sound as it spins when thrown
	saber->swingSound[0] = 0;				//none - if set, plays one of these 3 sounds when swung during an attack - NOTE: must provide all 3!!!
	saber->swingSound[1] = 0;				//none - if set, plays one of these 3 sounds when swung during an attack - NOTE: must provide all 3!!!
	saber->swingSound[2] = 0;				//none - if set, plays one of these 3 sounds when swung during an attack - NOTE: must provide all 3!!!

	//done in game (server-side code)
	saber->moveSpeedScale = 1.0f;				//1.0 - you move faster/slower when using this saber
	saber->animSpeedScale = 1.0f;				//1.0 - plays normal attack animations faster/slower

	saber->kataMove = LS_INVALID;				//LS_INVALID - if set, player will execute this move when they press both attack buttons at the same time 
	saber->lungeAtkMove = LS_INVALID;			//LS_INVALID - if set, player will execute this move when they crouch+fwd+attack 
	saber->jumpAtkUpMove = LS_INVALID;			//LS_INVALID - if set, player will execute this move when they jump+attack 
	saber->jumpAtkFwdMove = LS_INVALID;			//LS_INVALID - if set, player will execute this move when they jump+fwd+attack 
	saber->jumpAtkBackMove = LS_INVALID;		//LS_INVALID - if set, player will execute this move when they jump+back+attack
	saber->jumpAtkRightMove = LS_INVALID;		//LS_INVALID - if set, player will execute this move when they jump+rightattack
	saber->jumpAtkLeftMove = LS_INVALID;		//LS_INVALID - if set, player will execute this move when they jump+left+attack
	saber->readyAnim = -1;						//-1 - anim to use when standing idle
	saber->drawAnim = -1;						//-1 - anim to use when drawing weapon
	saber->putawayAnim = -1;					//-1 - anim to use when putting weapon away
	saber->tauntAnim = -1;						//-1 - anim to use when hit "taunt"
	saber->bowAnim = -1;						//-1 - anim to use when hit "bow"
	saber->meditateAnim = -1;					//-1 - anim to use when hit "meditate"
	saber->flourishAnim = -1;					//-1 - anim to use when hit "flourish"
	saber->gloatAnim = -1;						//-1 - anim to use when hit "gloat"

	//***NOTE: you can only have a maximum of 2 "styles" of blades, so this next value, "bladeStyle2Start" is the number of the first blade to use these value on... all blades before this use the normal values above, all blades at and after this number use the secondary values below***
	saber->bladeStyle2Start = 0;			//0 - if set, blades from this number and higher use the following values (otherwise, they use the normal values already set)
	
	//***The following can be different for the extra blades - not setting them individually defaults them to the value for the whole saber (and first blade)***

	//===PRIMARY BLADES=====================
	//done in cgame (client-side code)
	saber->trailStyle = 0;					//0 - default (0) is normal, 1 is a motion blur and 2 is no trail at all (good for real-sword type mods)
	saber->g2MarksShader = 0;				//none - if set, the game will use this shader for marks on enemies instead of the default "gfx/damage/saberglowmark"
	saber->g2WeaponMarkShader = 0;			//none - if set, the game will use this shader for marks on enemies instead of the default "gfx/damage/saberglowmark"
	//saber->bladeShader = 0;				//none - if set, overrides the shader used for the saber blade?
	//saber->trailShader = 0;				//none - if set, overrides the shader used for the saber trail?
	saber->hitSound[0] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->hitSound[1] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->hitSound[2] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->blockSound[0] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->blockSound[1] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->blockSound[2] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->bounceSound[0] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->bounceSound[1] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->bounceSound[2] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->blockEffect = 0;					//none - if set, plays this effect when the saber/sword hits another saber/sword (instead of "saber/saber_block.efx")
	saber->hitPersonEffect = 0;				//none - if set, plays this effect when the saber/sword hits a person (instead of "saber/blood_sparks_mp.efx")
	saber->hitOtherEffect = 0;				//none - if set, plays this effect when the saber/sword hits something else damagable (instead of "saber/saber_cut.efx")
	saber->bladeEffect = 0;					//none - if set, plays this effect at the blade tag

	//done in game (server-side code)
	saber->knockbackScale = 0;				//0 - if non-zero, uses damage done to calculate an appropriate amount of knockback
	saber->damageScale = 1.0f;				//1 - scale up or down the damage done by the saber
	saber->splashRadius = 0.0f;				//0 - radius of splashDamage
	saber->splashDamage = 0;				//0 - amount of splashDamage, 100% at a distance of 0, 0% at a distance = splashRadius
	saber->splashKnockback = 0.0f;			//0 - amount of splashKnockback, 100% at a distance of 0, 0% at a distance = splashRadius

	//===SECONDARY BLADES===================
	//done in cgame (client-side code)
	saber->trailStyle2 = 0;					//0 - default (0) is normal, 1 is a motion blur and 2 is no trail at all (good for real-sword type mods)
	saber->g2MarksShader2 = 0;				//none - if set, the game will use this shader for marks on enemies instead of the default "gfx/damage/saberglowmark"
	saber->g2WeaponMarkShader2 = 0;			//none - if set, the game will use this shader for marks on enemies instead of the default "gfx/damage/saberglowmark"
	//saber->bladeShader = 0;				//none - if set, overrides the shader used for the saber blade?
	//saber->trailShader = 0;				//none - if set, overrides the shader used for the saber trail?
	saber->hit2Sound[0] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->hit2Sound[1] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->hit2Sound[2] = 0;					//none - if set, plays one of these 3 sounds when saber hits a person - NOTE: must provide all 3!!!
	saber->block2Sound[0] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->block2Sound[1] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->block2Sound[2] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits another saber/sword - NOTE: must provide all 3!!!
	saber->bounce2Sound[0] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->bounce2Sound[1] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->bounce2Sound[2] = 0;				//none - if set, plays one of these 3 sounds when saber/sword hits a wall and bounces off (must set bounceOnWall to 1 to use these sounds) - NOTE: must provide all 3!!!
	saber->blockEffect2 = 0;					//none - if set, plays this effect when the saber/sword hits another saber/sword (instead of "saber/saber_block.efx")
	saber->hitPersonEffect2 = 0;				//none - if set, plays this effect when the saber/sword hits a person (instead of "saber/blood_sparks_mp.efx")
	saber->hitOtherEffect2 = 0;				//none - if set, plays this effect when the saber/sword hits something else damagable (instead of "saber/saber_cut.efx")
	saber->bladeEffect2 = 0;				//none - if set, plays this effect at the blade tag

	//done in game (server-side code)
	saber->knockbackScale2 = 0;				//0 - if non-zero, uses damage done to calculate an appropriate amount of knockback
	saber->damageScale2 = 1.0f;				//1 - scale up or down the damage done by the saber
	saber->splashRadius2 = 0.0f;				//0 - radius of splashDamage
	saber->splashDamage2 = 0;				//0 - amount of splashDamage, 100% at a distance of 0, 0% at a distance = splashRadius
	saber->splashKnockback2 = 0.0f;			//0 - amount of splashKnockback, 100% at a distance of 0, 0% at a distance = splashRadius
//=========================================================================================================================================
	saber->extraDisarmChance = 11;
	saber->extraDamage = 1.0f;
	saber->BPregenRate = 300;
	saber->FPregenRate = 200;

}

// NOTES:
// * saberColor no longer exists (not needed/used)
// * anything that sets flags (ie "lockable") is totally different under this new system
// * forceRestrict = GONE (didn't feel like bothering with it)
// * most stuff involving restricting to specific stances is broken or nonfunctional
// * kataMove = GONE (katas don't exist anymore...)
// * lungeAtkMove = GONE (dependent on stance)
// * jumpAtkUpMove = GONE (dependent on stance)
// * jumpAtkFwdMove = GONE (dependent on stance)
// * jumpAtkBackMove = GONE (dependent on stance)
// * jumpAtkRightMove = GONE (dependent on stance)
// * jumpAtkLeftMove = GONE (dependent on stance)
// * readyAnim = GONE (dependent on stance)
// * drawAnim = GONE (dependent on stance)
// * putawayAnim = GONE (dependent on stance)
// * noRollStab, noPullAttack, noBackAttack, noStabDown, noWallRuns, noWallFlips, 
// noWallGrab, noRolls, noFlips, noCartwheels, noKicks, noMirrorAttacks, notInMP = GONE
// *noWallMarks2, noDlight2, noBlade2, trailStyle2, g2MarksShader2, g2WeaponMarkShader2,
// knockbackScale2, damageScale2, noDismemberment2, noIdleEffect2, alwaysBlock2,
// noManualDeactivate2, transitionDamage2, splashRadius2, splashDamage2, splashKnockback2,
// hit2Sound1/2/3, block2Sound1/2/3, bounce2Sound1/2/3, blockEffect2, hitPersonEffect2,
// hitOtherEffect2, bladeEffect2, noClashFlare2 = GONEEEEEEE
// these were stupid, pointless, and retarded, not to mention the vast majority of these weren't
// used or they were used incorrectly.
// * extraDisarmChance: new field
// * extraDamage: new field
// * bpRegenRate: new field
// * fpRegenRate: new field

bool JKG_ParseHiltFile( const char *filename )
{
	saberInfo_t theHilt;
	cJSON *json = NULL;
    cJSON *jsonNode = NULL;
	//cJSON *jsonChild = NULL;
	char error[MAX_STRING_CHARS];
	char hiltFileData[32967];		// TODO: change
	fileHandle_t f;
	int fileLen = trap->FS_Open (filename, &f, FS_READ);
	char name[MAX_QPATH];

	if ( !f || fileLen == -1 )
    {
        Com_Printf (S_COLOR_RED "%s: failed to read the hilt file. File is unreadable or is empty.\n", filename);
        return false;
    }

	if ( (fileLen + 1) >= 32967 )
    {
        trap->FS_Close (f);
        Com_Printf (S_COLOR_RED "%s: file too big (%d bytes, maximum is %d).\n", filename, fileLen, 32966);
        return false;
    }
    
    trap->FS_Read (&hiltFileData, fileLen, f);
    hiltFileData[fileLen] = '\0';
    
    trap->FS_Close (f);

	json = cJSON_ParsePooled (hiltFileData, error, sizeof (error));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "%s: %s\n", filename, error);
        return false;
    }

	jsonNode = cJSON_GetObjectItem(json, "name");
	if(!jsonNode)
	{
		Com_Printf (S_COLOR_RED "%s: No \"name\" field.\n", filename);
		return false;
	}
	WP_SaberSetDefaults( &theHilt );
	Q_strncpyz(name, cJSON_ToString(jsonNode), sizeof(name));

#define JSONPARSE(x)		\
	jsonNode = cJSON_GetObjectItem(json, x); \
	if(jsonNode != NULL)

	/*
	Begin parsing for all the .hilt parameters
	*/

	JSONPARSE("fullName")
		Q_strncpyz(theHilt.fullName, cJSON_ToString(jsonNode), MAX_QPATH);

	JSONPARSE("saberType")
	{
		saberType_t theType = (saberType_t)GetIDForString( SaberTable, cJSON_ToString(jsonNode) );
		theHilt.type = (theType >= SABER_SINGLE && theType <= NUM_SABERS) ? theType : SABER_NONE;
	}

	JSONPARSE("saberModel")
		Q_strncpyz(theHilt.model, cJSON_ToString(jsonNode), MAX_QPATH);

	JSONPARSE("customSkin")
		theHilt.skin = trap->R_RegisterSkin(cJSON_ToString(jsonNode));

	JSONPARSE("soundOn")
		theHilt.soundOn = BG_SoundIndex( const_cast<char *>(cJSON_ToString(jsonNode)) );

	JSONPARSE("soundLoop")
		theHilt.soundLoop = BG_SoundIndex( const_cast<char *>(cJSON_ToString(jsonNode)) );

	JSONPARSE("soundOff")
		theHilt.soundOff = BG_SoundIndex( const_cast<char *>(cJSON_ToString(jsonNode)) );

	JSONPARSE("numBlades")
	{
		int numBlades = cJSON_ToInteger(jsonNode);
		if( numBlades < 1 || numBlades > MAX_BLADES )
		{
			Com_Error(ERR_DROP, "JKG_ParseHiltFile: hilt %s has illegal number of blades (%d) max: %d", 
				name, numBlades, MAX_BLADES);
			return false;
		}
		theHilt.numBlades = numBlades;
	}

	JSONPARSE("blades")
	{
		for(int i = 0; i < theHilt.numBlades; i++)
		{
			cJSON *child = cJSON_GetObjectItem(jsonNode, va("blade%i", i+1));
			if(child)
			{
				cJSON *childNode;
#define JSONCHILDPARSE(x)	\
childNode = cJSON_GetObjectItem(child, x); \
if(childNode)

				JSONCHILDPARSE("length")
					theHilt.blade[i].lengthMax = cJSON_ToNumber(childNode);

				JSONCHILDPARSE("radius")
					theHilt.blade[i].radius = cJSON_ToNumber(childNode);
#undef JSONCHILDPARSE
			}
		}
	}

	// FIXME: this is heavily broken due to new stance construction
	JSONPARSE("saberStyle")
	{
		int style = TranslateSaberStyle( cJSON_ToString( jsonNode ) );
		theHilt.stylesLearned = (1<<style);
		theHilt.stylesForbidden = 0;
		for( int i = SS_NONE+1; i < SS_NUM_SABER_STYLES; i++ )
		{
			if( i != style )
				theHilt.stylesForbidden |= (1<<style);
		}
	}

	JSONPARSE("saberStyleLearned")
		theHilt.stylesLearned |= (1<<TranslateSaberStyle( cJSON_ToString( jsonNode ) ));

	JSONPARSE("saberStyleForbidden")
		theHilt.stylesForbidden |= (1<<TranslateSaberStyle( cJSON_ToString( jsonNode ) ));
	// end FIXME

	JSONPARSE("maxChain")
		theHilt.maxChain = cJSON_ToInteger(jsonNode);

	JSONPARSE("flags")
	{
		// TODO: parse flags
		const char *flags[4];
		int numFlags = cJSON_ReadStringArray(jsonNode, 27, flags);

		for(int i = 0; i < numFlags; i++)
		{
			if(!Q_stricmp(flags[i], "notLockable"))
				theHilt.saberFlags |= SFL_NOT_LOCKABLE;
			else if(!Q_stricmp(flags[i], "notThrowable"))
				theHilt.saberFlags |= SFL_NOT_THROWABLE;
			else if(!Q_stricmp(flags[i], "notDisarmable"))
				theHilt.saberFlags |= SFL_NOT_DISARMABLE;
			else if(!Q_stricmp(flags[i], "notActiveBlocking"))
				theHilt.saberFlags |= SFL_NOT_ACTIVE_BLOCKING;
			else if(!Q_stricmp(flags[i], "twoHanded"))
				theHilt.saberFlags |= SFL_TWO_HANDED;
			else if(!Q_stricmp(flags[i], "singleBladeThrowable"))
				theHilt.saberFlags |= SFL_SINGLE_BLADE_THROWABLE;
			else if(!Q_stricmp(flags[i], "returnDamage"))
				theHilt.saberFlags |= SFL_RETURN_DAMAGE;
			else if(!Q_stricmp(flags[i], "bounceOnWalls"))
				theHilt.saberFlags |= SFL_BOUNCE_ON_WALLS;
			else if(!Q_stricmp(flags[i], "boltToWrist"))
				theHilt.saberFlags |= SFL_BOLT_TO_WRIST;
			else if(!Q_stricmp(flags[i], "noWallMarks"))
				theHilt.saberFlags2 |= SFL2_NO_WALL_MARKS;
			else if(!Q_stricmp(flags[i], "noDlight"))
				theHilt.saberFlags2 |= SFL2_NO_DLIGHT;
			else if(!Q_stricmp(flags[i], "noBlade"))
				theHilt.saberFlags2 |= SFL2_NO_BLADE;
			else if(!Q_stricmp(flags[i], "noDismemberment"))
				theHilt.saberFlags2 |= SFL2_NO_DISMEMBERMENT;
			else if(!Q_stricmp(flags[i], "noIdleEffect"))
				theHilt.saberFlags2 |= SFL2_NO_IDLE_EFFECT;
			else if(!Q_stricmp(flags[i], "alwaysBlock"))
				theHilt.saberFlags2 |= SFL2_ALWAYS_BLOCK;
			else if(!Q_stricmp(flags[i], "noManualDeactivate"))
				theHilt.saberFlags2 |= SFL2_NO_MANUAL_DEACTIVATE;
			else if(!Q_stricmp(flags[i], "transitionDamage"))
				theHilt.saberFlags2 |= SFL2_TRANSITION_DAMAGE;
			else if(!Q_stricmp(flags[i], "noClashFlare"))
				theHilt.saberFlags2 |= SFL2_NO_CLASH_FLARE;
		}
	}

	JSONPARSE("lockBonus")
		theHilt.lockBonus = cJSON_ToInteger(jsonNode);

	JSONPARSE("parryBonus")
		theHilt.parryBonus = cJSON_ToInteger(jsonNode);

	JSONPARSE("breakParryBonus")
		theHilt.breakParryBonus = cJSON_ToInteger(jsonNode);

	JSONPARSE("breakParryBonus2")
		theHilt.breakParryBonus2 = cJSON_ToInteger(jsonNode);

	JSONPARSE("disarmBonus")
		theHilt.disarmBonus = cJSON_ToInteger(jsonNode);

	JSONPARSE("disarmBonus2")
		theHilt.disarmBonus2 = cJSON_ToInteger(jsonNode);

	JSONPARSE("singleBladeStyle")
		(void)0;
		//int i = 0; // does not work

	JSONPARSE("brokenSaber1")
		(void)0;
		//int i = 0; // BROKEN in base (ironic)

	JSONPARSE("brokenSaber2")
		(void)0;
		//int i = 0; // BROKEN in base (ironic)

	JSONPARSE("moveSpeedScale")
		theHilt.moveSpeedScale = cJSON_ToNumber(jsonNode);

	JSONPARSE("animSpeedScale")
		theHilt.animSpeedScale = cJSON_ToNumber(jsonNode);

	JSONPARSE("tauntAnim")
		theHilt.tauntAnim = GetIDForString( animTable, cJSON_ToString( jsonNode ) );

	JSONPARSE("bowAnim")
		theHilt.bowAnim = GetIDForString( animTable, cJSON_ToString( jsonNode ) );

	JSONPARSE("meditateAnim")
		theHilt.tauntAnim = GetIDForString( animTable, cJSON_ToString( jsonNode ) );

	JSONPARSE("flourishAnim")
		theHilt.tauntAnim = GetIDForString( animTable, cJSON_ToString( jsonNode ) );

	JSONPARSE("gloatAnim")
		theHilt.tauntAnim = GetIDForString( animTable, cJSON_ToString( jsonNode ) );

	JSONPARSE("trailStyle")
		theHilt.trailStyle = cJSON_ToInteger( jsonNode );

#ifdef _CGAME
	JSONPARSE("g2MarksShader")
		theHilt.g2MarksShader = trap->R_RegisterShader( cJSON_ToString( jsonNode ) );

	JSONPARSE("g2WeaponMarkShader")
		theHilt.g2WeaponMarkShader = trap->R_RegisterShader( cJSON_ToString( jsonNode ) );

	JSONPARSE("blockEffect")
		theHilt.blockEffect = trap->FX_RegisterEffect( const_cast<char *>( cJSON_ToString( jsonNode ) ) );

	JSONPARSE("hitPersonEffect")
		theHilt.hitPersonEffect = trap->FX_RegisterEffect( const_cast<char *>( cJSON_ToString( jsonNode ) ) );

	JSONPARSE("hitOtherEffect")
		theHilt.hitOtherEffect = trap->FX_RegisterEffect( const_cast<char *>( cJSON_ToString( jsonNode ) ) );

	JSONPARSE("bladeEffect")
		theHilt.bladeEffect = trap->FX_RegisterEffect( const_cast<char *>( cJSON_ToString( jsonNode ) ) );
#endif

	JSONPARSE("knockbackScale")
		theHilt.knockbackScale = cJSON_ToNumber( jsonNode );

	JSONPARSE("damageScale")
		theHilt.damageScale = cJSON_ToNumber( jsonNode );

	JSONPARSE("splash")
	{
		cJSON *splashNode;

		splashNode = cJSON_GetObjectItem(jsonNode, "splashDamage");
		if(splashNode)
			theHilt.splashDamage = cJSON_ToInteger(splashNode);

		splashNode = cJSON_GetObjectItem(jsonNode, "splashKnockback");
		if(splashNode)
			theHilt.splashKnockback = cJSON_ToNumber(splashNode);

		splashNode = cJSON_GetObjectItem(jsonNode, "splashRadius");
		if(splashNode)
			theHilt.splashRadius = cJSON_ToNumber(splashNode);
	}

	JSONPARSE("spinSound")
		theHilt.spinSound = BG_SoundIndex(const_cast<char*>(cJSON_ToString(jsonNode)));

	JSONPARSE("swingSounds")
	{
		const char *sounds[3];
		int numSounds = cJSON_ReadStringArray(jsonNode, 3, sounds);

		for(int i = 0; i < numSounds; i++)
		{
			theHilt.swingSound[i] = BG_SoundIndex( const_cast<char *>( sounds[i] ) );
		}
	}

	JSONPARSE("hitSounds")
	{
		const char *sounds[3];
		int numSounds = cJSON_ReadStringArray(jsonNode, 3, sounds);

		for(int i = 0; i < numSounds; i++)
		{
			theHilt.hitSound[i] = BG_SoundIndex( const_cast<char *>( sounds[i] ) );
		}
	}

	JSONPARSE("blockSounds")
	{
		const char *sounds[3];
		int numSounds = cJSON_ReadStringArray(jsonNode, 3, sounds);

		for(int i = 0; i < numSounds; i++)
		{
			theHilt.blockSound[i] = BG_SoundIndex( const_cast<char *>( sounds[i] ) );
		}
	}

	JSONPARSE("bounceSounds")
	{
		const char *sounds[3];
		int numSounds = cJSON_ReadStringArray(jsonNode, 3, sounds);

		for(int i = 0; i < numSounds; i++)
		{
			theHilt.bounceSound[i] = BG_SoundIndex( const_cast<char *>( sounds[i] ) );
		}
	}

	JSONPARSE("extraDisarmChance")
		theHilt.extraDisarmChance = cJSON_ToInteger(jsonNode);

	JSONPARSE("extraDamage")
		theHilt.extraDamage = cJSON_ToNumber(jsonNode);

	JSONPARSE("bpRegenRate")
		theHilt.BPregenRate = cJSON_ToInteger(jsonNode);

	JSONPARSE("fpRegenRate")
		theHilt.FPregenRate = cJSON_ToInteger(jsonNode);

	/*
	End parsing
	*/

#undef JSONPARSE

	cJSON_Delete(json);

	hiltLookupTable->insert(std::make_pair(name, theHilt));

	return true;
}

bool JKG_ParseHiltFiles( void )
{
	char hiltFiles[32967];
	int numFiles = Q_FSGetFileListSorted ("ext_data/hilts", ".hilt", hiltFiles, sizeof (hiltFiles));
	const char *hiltFile = hiltFiles;
	int successful = 0;
    int failed = 0;

	int t = trap->Milliseconds();
	int i;

	hiltLookupTable = new std::unordered_map<std::string, saberInfo_t, CaseInsensitiveHash>();

	Com_Printf ("------- Hilt data -------\n");

	for ( i = 0; i < numFiles; i++ )
    {
        if ( JKG_ParseHiltFile (va ("ext_data/hilts/%s", hiltFile)) )
            successful++;
        else
            failed++;
            
        hiltFile += strlen (hiltFile) + 1;
    }
    
    Com_Printf ("Successfully loaded %d hilts, failed to load %d hilts.\n", successful, failed);
    Com_Printf ("Took %d milliseconds.\n", trap->Milliseconds() - t);
    Com_Printf ("-------------------------------------\n");
    
    return (successful > 0);
}

bool JKG_GetSaberHilt( const char *hiltName, saberInfo_t *saber )
{
	std::unordered_map<std::string, saberInfo_t>::iterator it;
	if(!hiltLookupTable || hiltLookupTable->size() <= 0)
		return false;	// occasionally gets set, incorrectly.

	it = hiltLookupTable->find(hiltName);
	if(it == hiltLookupTable->end())
	{
		Com_Printf(S_COLOR_YELLOW "WARNING: Couldn't find hilt \"%s\" reference\n", hiltName);
		return false;
	}

	*saber = it->second;

	return true;
}

void JKG_CleanSaberHilts( void )
{
	delete hiltLookupTable;
	hiltLookupTable = NULL;
}

void WP_RemoveSaber( saberInfo_t *sabers, int saberNum )
{
	if ( !sabers )
	{
		return;
	}
	//reset everything for this saber just in case
	WP_SaberSetDefaults( &sabers[saberNum] );

	strcpy(sabers[saberNum].name, "none");
	sabers[saberNum].model[0] = 0;

	//ent->client->ps.dualSabers = qfalse;
	sabers[saberNum].Deactivate();
	sabers[saberNum].SetLength(0.0f);
//	if ( ent->weaponModel[saberNum] > 0 )
//	{
//		gi.G2API_RemoveGhoul2Model( ent->ghoul2, ent->weaponModel[saberNum] );
//		ent->weaponModel[saberNum] = -1;
//	}
//	if ( saberNum == 1 )
//	{
//		ent->client->ps.dualSabers = qfalse;
//	}
}

void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName )
{
	if ( !sabers )
	{
		return;
	}
	if ( Q_stricmp( "none", saberName ) == 0 || Q_stricmp( "remove", saberName ) == 0 )
	{
		if (saberNum != 0)
		{ //can't remove saber 0 ever
			WP_RemoveSaber( sabers, saberNum );
		}
		return;
	}

	JKG_GetSaberHilt( saberName, &sabers[saberNum] );

	if ((sabers[1].saberFlags&SFL_TWO_HANDED))
	{//not allowed to use a 2-handed saber as second saber
		WP_RemoveSaber( sabers, 1 );
		return;
	}
	else if ((sabers[0].saberFlags&SFL_TWO_HANDED) &&
		sabers[1].model[0])
	{ //you can't use a two-handed saber with a second saber, so remove saber 2
		WP_RemoveSaber( sabers, 1 );
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//						JEDI KNIGHT GALAXIES							//
//					SABER SYSTEM CODE MODIFICATION						//
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////

#include <json/cJSON.h>

// Saber system aesthetics stuff
// The data about crystals is stored in a table which is accessed elsewhere

unsigned int numLoadedCrystals = 0;
saberCrystalData_t saberCrystalsLookup[MAX_SABER_CRYSTALS];

const saberCrystalData_t *JKG_GetSaberCrystal( const char *crystalName )
{
	int i;

	if(atoi(crystalName))
	{
		int number = atoi(crystalName);
		if(number < 0 || number >= numLoadedCrystals) return NULL;
		return &saberCrystalsLookup[number];
	}

	for(i = 0; i < numLoadedCrystals; i++)
	{
		if(!Q_stricmp(crystalName, saberCrystalsLookup[i].crystalName))
		{
			return &saberCrystalsLookup[i];
		}
	}

	return NULL;
}

static void JKG_ParseCrystalFile( const char *fileText )
{
	int i = 0;
    cJSON *json = NULL;
    char jsonError[MAX_STRING_CHARS] = { 0 };

    json = cJSON_ParsePooled (fileText, jsonError, sizeof (jsonError));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "Error: %s\n", jsonError);
    }
    else
    {
        saberCrystalData_t *crystal = &saberCrystalsLookup[0];
        cJSON *jsonNode;
        cJSON *field;
        const char *string = NULL;
        
        for ( jsonNode = cJSON_GetFirstItem (json); jsonNode; jsonNode = cJSON_GetNextItem (jsonNode), crystal++, numLoadedCrystals++, i++ )
        {
            field = cJSON_GetObjectItem (jsonNode, "name");
            string = cJSON_ToString (field);
			if(string && string[0])
				Q_strncpyz (crystal->crystalName, string, sizeof (crystal->crystalName));
			else
				strcpy( crystal->crystalName, "noname" );

			crystal->crystalID = i;
            
            #ifdef _CGAME
            {
				cJSON *rgb;
				field = cJSON_GetObjectItem (jsonNode, "bladeEffect");
				string = cJSON_ToString (field);

				if(string && string[0])
					Q_strncpyz(crystal->bladeEffect, string, sizeof(crystal->bladeEffect));
				else
					strcpy(crystal->bladeEffect, "NULL_BLADE");

				field = cJSON_GetObjectItem (jsonNode, "glowEffect");
				string = cJSON_ToString (field);

				if(string && string[0])
					Q_strncpyz(crystal->glowEffect, string, sizeof(crystal->glowEffect));
				else
					strcpy(crystal->glowEffect, "NULL_GLOW");

				field = cJSON_GetObjectItem (jsonNode, "trailEffect");
				string = cJSON_ToString (field);

				if(string && string[0])
					Q_strncpyz(crystal->trailEffect, string, sizeof(crystal->trailEffect));
				else
					strcpy(crystal->trailEffect, "NULL_TRAIL");

                rgb = cJSON_GetObjectItem (jsonNode, "rgb");
                if ( rgb )
                {       
                    field = cJSON_GetObjectItem (rgb, "r");
					if(field)
					{
						crystal->vRGB[0] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->vRGB[0] = 1.0f;
					}

					field = cJSON_GetObjectItem (rgb, "g");
					if(field)
					{
						crystal->vRGB[1] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->vRGB[1] = 1.0f;
					}

					field = cJSON_GetObjectItem (rgb, "b");
					if(field)
					{
						crystal->vRGB[2] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->vRGB[2] = 1.0f;
					}
                }
				else
				{
					crystal->vRGB[0] = crystal->vRGB[1] = crystal->vRGB[2] = 1.0f;
				}

				rgb = cJSON_GetObjectItem (jsonNode, "bladergb");
                if ( rgb )
                {       
                    field = cJSON_GetObjectItem (rgb, "r");
					if(field)
					{
						crystal->bRGB[0] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->bRGB[0] = 1.0f;
					}

					field = cJSON_GetObjectItem (rgb, "g");
					if(field)
					{
						crystal->bRGB[1] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->bRGB[1] = 1.0f;
					}

					field = cJSON_GetObjectItem (rgb, "b");
					if(field)
					{
						crystal->bRGB[2] = cJSON_ToInteger(field)/255.0f;
					}
					else
					{
						crystal->bRGB[2] = 1.0f;
					}
                }
				else
				{
					crystal->bRGB[0] = crystal->bRGB[1] = crystal->bRGB[2] = 1.0f;
				}
            }
            #endif
        }
    }
}

static qboolean JKG_LoadSaberCrystals( void )
{
	fileHandle_t f;
    char buffer[MAX_CRYSTAL_FILE_SIZE + 1];
    int fileLength;
    
    Com_Printf ("------- Crystals -------\n");
    
    fileLength = trap->FS_Open ("ext_data/tables/crystals.json", &f, FS_READ);
    if ( fileLength == -1 || !f )
    {
        Com_Printf (S_COLOR_RED "Error: Failed to read the crystals.json file. File is unreadable or does not exist.\n");
        return qfalse;
    }
    
    if ( fileLength == 0 )
    {
        Com_Printf (S_COLOR_RED "Error: crystals.json file is empty.\n");
        trap->FS_Close (f);
        return qfalse;
    }
    
    if ( fileLength >= MAX_CRYSTAL_FILE_SIZE )
    {
		Com_Printf (S_COLOR_RED "Error: crystals.json file is too large (max file size is %d kb)\n", MAX_CRYSTAL_FILE_SIZE/1024);
		Com_Error(ERR_FATAL, "Crystals file is too big.");
        trap->FS_Close (f);
        return qfalse;
    }
    
    trap->FS_Read (buffer, fileLength, f);
    buffer[fileLength] = '\0';
    trap->FS_Close (f);
    
    JKG_ParseCrystalFile (buffer);
    
    Com_Printf ("-----------------------------------\n");
    
    return qtrue;
}

void JKG_InitializeSaberCrystalData( void )
{
	if(!JKG_LoadSaberCrystals())
	{
		Com_Error(ERR_FATAL, "Could not read saber crystal data! (See console for errors)");
		return;
	}
}


/////////////////////////////////////////////////////////////////
//
// SABER STANCE DATA
// By default, a blank .stance file will just be a base "blue" stance
// with no special moves enabled. You'll need a completed .stance
// file if you want more moves enabled.
//
/////////////////////////////////////////////////////////////////

// This needs to be initialized again, because I don't trust Raven's setup --eez
stringID_table_t SaberMoves[] =
{
	ENUM2STRING(LS_NONE),
	ENUM2STRING(LS_READY),
	ENUM2STRING(LS_DRAW),
	ENUM2STRING(LS_PUTAWAY),

	// attacks -- base
	ENUM2STRING(LS_A_TL2BR),		// Top left to bottom right
	ENUM2STRING(LS_A_L2R),			// Left to right
	ENUM2STRING(LS_A_BL2TR),		// Bottom left to top right
	ENUM2STRING(LS_A_BR2TL),		// Bottom right to top left
	ENUM2STRING(LS_A_R2L),			// Right to left
	ENUM2STRING(LS_A_TR2BL),		// Top right to bottom left
	ENUM2STRING(LS_A_T2B),			// Top to bottom

	// attacks -- special
	ENUM2STRING(LS_A_BACKSTAB),
	ENUM2STRING(LS_A_BACK),
	ENUM2STRING(LS_A_BACK_CR),
	ENUM2STRING(LS_ROLL_STAB),
	ENUM2STRING(LS_A_LUNGE),
	ENUM2STRING(LS_A_JUMP_T__B_),
	ENUM2STRING(LS_A_FLIP_STAB),
	ENUM2STRING(LS_A_FLIP_SLASH),
	ENUM2STRING(LS_JUMPATTACK_DUAL),
	ENUM2STRING(LS_JUMPATTACK_ARIAL_LEFT),
	ENUM2STRING(LS_JUMPATTACK_ARIAL_RIGHT),
	ENUM2STRING(LS_JUMPATTACK_CART_LEFT),
	ENUM2STRING(LS_JUMPATTACK_CART_RIGHT),
	ENUM2STRING(LS_JUMPATTACK_STAFF_LEFT),
	ENUM2STRING(LS_JUMPATTACK_STAFF_RIGHT),
	ENUM2STRING(LS_BUTTERFLY_LEFT),
	ENUM2STRING(LS_BUTTERFLY_RIGHT),
	ENUM2STRING(LS_A_BACKFLIP_ATK),
	ENUM2STRING(LS_SPINATTACK_DUAL),
	ENUM2STRING(LS_SPINATTACK),
	ENUM2STRING(LS_LEAP_ATTACK),

	// Vehicles
	ENUM2STRING(LS_SWOOP_ATTACK_RIGHT),
	ENUM2STRING(LS_SWOOP_ATTACK_LEFT),
	ENUM2STRING(LS_TAUNTAUN_ATTACK_RIGHT),
	ENUM2STRING(LS_TAUNTAUN_ATTACK_LEFT),

	// Kicks
	ENUM2STRING(LS_KICK_F),
	ENUM2STRING(LS_KICK_B),
	ENUM2STRING(LS_KICK_R),
	ENUM2STRING(LS_KICK_L),
	ENUM2STRING(LS_KICK_S),
	ENUM2STRING(LS_KICK_BF),
	ENUM2STRING(LS_KICK_RL),
	ENUM2STRING(LS_KICK_F_AIR),
	ENUM2STRING(LS_KICK_B_AIR),
	ENUM2STRING(LS_KICK_R_AIR),
	ENUM2STRING(LS_KICK_L_AIR),

	// More special moves u_u
	ENUM2STRING(LS_STABDOWN),
	ENUM2STRING(LS_STABDOWN_STAFF),
	ENUM2STRING(LS_STABDOWN_DUAL),
	ENUM2STRING(LS_DUAL_SPIN_PROTECT),
	ENUM2STRING(LS_STAFF_SOULCAL),

	// katas -- KILL THEM WITH FIRE!
	ENUM2STRING(LS_A1_SPECIAL),
	ENUM2STRING(LS_A2_SPECIAL),
	ENUM2STRING(LS_A3_SPECIAL),

	// even more special moves
	ENUM2STRING(LS_UPSIDE_DOWN_ATTACK),
	ENUM2STRING(LS_PULL_ATTACK_STAB),
	ENUM2STRING(LS_PULL_ATTACK_SWING),
	ENUM2STRING(LS_SPINATTACK_ALORA),
	ENUM2STRING(LS_DUAL_FB),
	ENUM2STRING(LS_DUAL_LR),
	ENUM2STRING(LS_HILT_BASH),

	// starts
	ENUM2STRING(LS_S_TL2BR),
	ENUM2STRING(LS_S_L2R),
	ENUM2STRING(LS_S_BL2TR),
	ENUM2STRING(LS_S_BR2TL),
	ENUM2STRING(LS_S_R2L),
	ENUM2STRING(LS_S_TR2BL),
	ENUM2STRING(LS_S_T2B),

	// returns
	ENUM2STRING(LS_R_TL2BR),
	ENUM2STRING(LS_R_L2R),
	ENUM2STRING(LS_R_BL2TR),
	ENUM2STRING(LS_R_BR2TL),
	ENUM2STRING(LS_R_R2L),
	ENUM2STRING(LS_R_TR2BL),
	ENUM2STRING(LS_R_T2B),

	// transitions
	ENUM2STRING(LS_T1_BR__R),
	ENUM2STRING(LS_T1_BR_TR),
	ENUM2STRING(LS_T1_BR_T_),
	ENUM2STRING(LS_T1_BR_TL),
	ENUM2STRING(LS_T1_BR__L),
	ENUM2STRING(LS_T1_BR_BL),
	ENUM2STRING(LS_T1__R_BR),
	ENUM2STRING(LS_T1__R_TR),
	ENUM2STRING(LS_T1__R_T_),
	ENUM2STRING(LS_T1__R_TL),
	ENUM2STRING(LS_T1__R__L),
	ENUM2STRING(LS_T1__R_BL),
	ENUM2STRING(LS_T1_TR_BR),
	ENUM2STRING(LS_T1_TR__R),
	ENUM2STRING(LS_T1_TR_T_),
	ENUM2STRING(LS_T1_TR_TL),
	ENUM2STRING(LS_T1_TR__L),
	ENUM2STRING(LS_T1_TR_BL),
	ENUM2STRING(LS_T1_T__BR),
	ENUM2STRING(LS_T1_T___R),
	ENUM2STRING(LS_T1_T__TR),
	ENUM2STRING(LS_T1_T__TL),
	ENUM2STRING(LS_T1_T___L),
	ENUM2STRING(LS_T1_T__BL),
	ENUM2STRING(LS_T1_TL_BR),
	ENUM2STRING(LS_T1_TL__R),
	ENUM2STRING(LS_T1_TL_TR),
	ENUM2STRING(LS_T1_TL_T_),
	ENUM2STRING(LS_T1_TL__L),
	ENUM2STRING(LS_T1_TL_BL),
	ENUM2STRING(LS_T1__L_BR),
	ENUM2STRING(LS_T1__L__R),
	ENUM2STRING(LS_T1__L_TR),
	ENUM2STRING(LS_T1__L_T_),
	ENUM2STRING(LS_T1__L_TL),
	ENUM2STRING(LS_T1__L_BL),
	ENUM2STRING(LS_T1_BL_BR),
	ENUM2STRING(LS_T1_BL__R),
	ENUM2STRING(LS_T1_BL_TR),
	ENUM2STRING(LS_T1_BL_T_),
	ENUM2STRING(LS_T1_BL_TL),
	ENUM2STRING(LS_T1_BL__L),

	// bounces
	ENUM2STRING(LS_B1_BR),
	ENUM2STRING(LS_B1__R),
	ENUM2STRING(LS_B1_TR),
	ENUM2STRING(LS_B1_T_),
	ENUM2STRING(LS_B1_TL),
	ENUM2STRING(LS_B1__L),
	ENUM2STRING(LS_B1_BL),

	// deflected attack
	ENUM2STRING(LS_D1_BR),
	ENUM2STRING(LS_D1__R),
	ENUM2STRING(LS_D1_TR),
	ENUM2STRING(LS_D1_T_),
	ENUM2STRING(LS_D1_TL),
	ENUM2STRING(LS_D1__L),
	ENUM2STRING(LS_D1_BL),
	ENUM2STRING(LS_D1_B_),

	// reflected attacks
	ENUM2STRING(LS_V1_BR),
	ENUM2STRING(LS_V1__R),
	ENUM2STRING(LS_V1_TR),
	ENUM2STRING(LS_V1_T_),
	ENUM2STRING(LS_V1_TL),
	ENUM2STRING(LS_V1__L),
	ENUM2STRING(LS_V1_BL),
	ENUM2STRING(LS_V1_B_),

	// broken parries
	ENUM2STRING(LS_H1_T_),
	ENUM2STRING(LS_H1_TR),
	ENUM2STRING(LS_H1_TL),
	ENUM2STRING(LS_H1_BR),
	ENUM2STRING(LS_H1_B_),
	ENUM2STRING(LS_H1_BL),

	// knockaways / staggers
	ENUM2STRING(LS_K1_T_),
	ENUM2STRING(LS_K1_TR),
	ENUM2STRING(LS_K1_TL),
	ENUM2STRING(LS_K1_BR),
	ENUM2STRING(LS_K1_BL),

	// parries
	ENUM2STRING(LS_PARRY_UP),
	ENUM2STRING(LS_PARRY_UR),
	ENUM2STRING(LS_PARRY_UL),
	ENUM2STRING(LS_PARRY_LR),
	ENUM2STRING(LS_PARRY_LL),

	// projectile blocks / unused
	ENUM2STRING(LS_REFLECT_UP),
	ENUM2STRING(LS_REFLECT_UR),
	ENUM2STRING(LS_REFLECT_UL),
	ENUM2STRING(LS_REFLECT_LR),
	ENUM2STRING(LS_REFLECT_LL),
};

stringID_table_t SaberQuadrants[] = {
	ENUM2STRING(Q_BR),
	ENUM2STRING(Q_R),
	ENUM2STRING(Q_TR),
	ENUM2STRING(Q_T),
	ENUM2STRING(Q_TL),
	ENUM2STRING(Q_L),
	ENUM2STRING(Q_BL),
	ENUM2STRING(Q_B),
};

/*stringID_table_t SaberAnimFlags[] = {
	ENUM2STRING
};*/
// uh wait...those aren't enum members 8D

stringID_table_t SaberBlocking[] = {
	ENUM2STRING(BLK_NO),
	ENUM2STRING(BLK_TIGHT),
	ENUM2STRING(BLK_WIDE),
};

#define MAX_STANCE_FILE_LENGTH		65535

// todo: merge all these similar funcs into a better one that adds a default number as an arg, and a string table as an arg too
int JKG_AnimationForJSON( cJSON *node )
{
	if( node )
	{
		const char *anim = cJSON_ToString (node);
		if ( anim && anim[0] )
		{
			if( atoi( anim ) )
			{
				return cJSON_ToInteger(node);
			}
			else
			{
				return GetIDForString (animTable, anim);
			}
		}
	}
	return 0;
}

int JKG_LSForJSON( cJSON *node )
{
	if( node )
	{
		const char *move = cJSON_ToString (node);
		if ( move && move[0] )
		{
			if( atoi( move ) )
			{
				return cJSON_ToInteger(node);
			}
			else
			{
				return GetIDForString (SaberMoves, move);
			}
		}
	}
	return LS_NONE;
}

int JKG_QForJSON( cJSON *node )
{
	if( node )
	{
		const char *quad = cJSON_ToString (node);
		if ( quad && quad[0] )
		{
			if( atoi( quad ) )
			{
				return cJSON_ToInteger(node);
			}
			else
			{
				return GetIDForString (SaberQuadrants, quad);
			}
		}
	}
	return Q_NUM_QUADS;
}

int JKG_BLKForJSON( cJSON *node )
{
	if( node )
	{
		const char *blk = cJSON_ToString (node);
		if ( blk && blk[0] )
		{
			if( atoi( blk ) )
			{
				return cJSON_ToInteger(node);
			}
			else
			{
				return GetIDForString (SaberBlocking, blk);
			}
		}
	}
	return BLK_NO;
}

// This func does not comply to the cleanup note posted before the bulk of these stated functions, since it cannot be done via ENUM2STRING --eez
int JKG_SAFForJSON( cJSON *node )
{
	const char *text = cJSON_ToString( node );

	if( !text || !text[0] )
	{
		return 0;
	}

	if( !Q_stricmp( "AFLAG_IDLE", text ) )
	{
		return (SETANIM_FLAG_NORMAL);
	}
	else if( !Q_stricmp( "AFLAG_ACTIVE", text ) )
	{
		return (SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
	} 
	else if( !Q_stricmp( "AFLAG_WAIT", text ) )
	{
		return (SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
	}
	else if( !Q_stricmp( "AFLAG_FINISH", text ) )
	{
		return (SETANIM_FLAG_HOLD);
	}
	else
	{
		return SETANIM_FLAG_NORMAL;
	}
}

static void JKG_CopyMoveFromDefaultInfo( int moveNum, saberMoveExternal_t *saberMove )
{
	saberMove->anim = saberMoveData[moveNum].animToUse;
	saberMove->animspeedscale = 1.0f;

	strcpy(saberMove->baseName, saberMoveData[moveNum].name);

	saberMove->FPdrain = 0;

	saberMove->startingQuadrant = saberMoveData[moveNum].startQuad;
	saberMove->endQuadrant = saberMoveData[moveNum].endQuad;

	saberMove->chainIdle = saberMoveData[moveNum].chain_idle;
	saberMove->chainAttack = saberMoveData[moveNum].chain_attack;

	saberMove->blockType = saberMoveData[moveNum].blocking;
	saberMove->trailLen = saberMoveData[moveNum].trailLength;

	saberMove->blendTime = saberMoveData[moveNum].blendTime;
	saberMove->setanimflag = saberMoveData[moveNum].animSetFlags;
}

saberMoveExternal_t defaultMove = { 
	// It's just LS_IDLE's default, basically
	"LS_IDLE",				// baseName
	0,						// fpDrain


	LS_READY,				// chainIdle
	LS_S_R2L,				// chainAttack

	Q_R,					// startingQuadrant
	Q_R,					// endQuadrant
	BLK_WIDE,				// blockType

	100,					// trailLen

	350,					// blendTime
	SETANIM_FLAG_NORMAL,
	BOTH_STAND2,
	1.0f,					// animspeedscale
};

// NO special moves by default --eez
saberSpecial_t defaultSpecialMoves = {
	qfalse,				// qboolean allowBackStab;
	qfalse,				// qboolean allowBackAttack;
	qfalse,				// qboolean allowCrouchedBackAttack;
	qfalse,				// qboolean allowRollStab;
	qfalse,				// qboolean allowLunge;
	qfalse,				// qboolean allowLeapAttack;
	qfalse,				// qboolean allowFlipStab;
	qfalse,				// qboolean allowFlipSlash;
	qfalse,				// qboolean allowButterfly;
	qfalse,				// qboolean allowCartwheel;
	qfalse,				// qboolean allowBackflipAttack;
	qfalse,				// qboolean allowSpinAttack;
	qfalse,				// qboolean allowSpeedLunge;
	qfalse,				// qboolean allowStabDown;
	qfalse,				// qboolean allowSpinAttack;
	qfalse,				// qboolean allowSoulCal;
	qfalse,				// qboolean allowBlueKata;
	qfalse,				// qboolean allowYellowKata;
	qfalse,				// qboolean allowRedKata;
	qfalse,				// qboolean allowUpsideDown;
	qfalse,				// qboolean allowPullStab;
	qfalse,				// qboolean allowPullSlash;
	qfalse,				// qboolean allowAloraSpin;
	qfalse,				// qboolean allowDualFrontBack;
	qfalse,				// qboolean allowDualSides;
	qfalse,				// qboolean allowHiltBash;
};

static unsigned short numStancesParsed = 0;

qboolean JKG_ParseSaberStanceFile( char *filename )
{
	cJSON *json = NULL;
    cJSON *jsonNode = NULL;
	cJSON *jsonChild = NULL;
    char error[MAX_STRING_CHARS];
    const char *str = NULL;
    int stance, i;

	char stanceFileData[MAX_STANCE_FILE_LENGTH];		// TODO: change
	fileHandle_t f;
	int fileLen = trap->FS_Open (filename, &f, FS_READ);

	saberStanceExternal_t theStance;
	saberMoveExternal_t theMove;

	if ( !f || fileLen == -1 )
    {
        Com_Printf (S_COLOR_RED "%s: failed to read the stance file. File is unreadable or is empty.\n", filename);
        return qfalse;
    }
    
    if ( (fileLen + 1) >= MAX_STANCE_FILE_LENGTH )
    {
        trap->FS_Close (f);
        Com_Printf (S_COLOR_RED "%s: file too big (%d bytes, maximum is %d).\n", filename, fileLen, MAX_STANCE_FILE_LENGTH - 1);
        
        return qfalse;
    }
    
    trap->FS_Read (&stanceFileData, fileLen, f);
    stanceFileData[fileLen] = '\0';
    
    trap->FS_Close (f);

	json = cJSON_ParsePooled (stanceFileData, error, sizeof (error));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "%s: %s\n", filename, error);
        
        return qfalse;
    }

	jsonNode = cJSON_GetObjectItem (json, "maxChainCount");
    stance = cJSON_ToInteger (jsonNode);
    theStance.maxChainCount = stance;

	jsonNode = cJSON_GetObjectItem (json, "attackSpeedModifier");
	if(jsonNode)
	{
		theStance.attackSpeedModifier = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.attackSpeedModifier = 0.55f;
	}

	jsonNode = cJSON_GetObjectItem (json, "transitionSpeedModifier");
	if(jsonNode)
	{
		theStance.transitionSpeedModifier = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.transitionSpeedModifier = 0.55f;
	}

	jsonNode = cJSON_GetObjectItem (json, "attackBackSpeedModifier");
	if(jsonNode)
	{
		theStance.attackBackSpeedModifier = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.attackBackSpeedModifier = 0.75f;
	}

	jsonNode = cJSON_GetObjectItem (json, "chainStyle");
    stance = cJSON_ToInteger (jsonNode);
	theStance.chainStyle = stance;

	jsonNode = cJSON_GetObjectItem (json, "baseDamageDefault");
	if(jsonNode)
	{
		theStance.baseDamageDefault = cJSON_ToInteger(jsonNode);
	}
	else
	{
		theStance.baseDamageDefault = 35;
	}

	jsonNode = cJSON_GetObjectItem (json, "baseDamageMin");
	if(jsonNode)
	{
		theStance.baseDamageMin = cJSON_ToInteger(jsonNode);
	}
	else
	{
		theStance.baseDamageMin = 2;
	}

	jsonNode = cJSON_GetObjectItem (json, "baseDamageMax");
	if(jsonNode)
	{
		theStance.baseDamageMax = cJSON_ToInteger(jsonNode);
	}
	else
	{
		theStance.baseDamageMax = 30;
	}

	jsonNode = cJSON_GetObjectItem( json, "BPDrain" );
	if( jsonNode )
	{
		stance = cJSON_ToInteger ( jsonNode );
		theStance.BPdrain = stance;
	}
	else
	{
		theStance.BPdrain = 16;
	}

	jsonNode = cJSON_GetObjectItem (json, "defensePenetration");
    if(jsonNode)
	{
		theStance.defensePenetration = cJSON_ToInteger(jsonNode);
	}
	else
	{
		theStance.defensePenetration = 1;
	}

	jsonNode = cJSON_GetObjectItem (json, "name_technical");
    str = cJSON_ToString (jsonNode);
	Q_strncpyz(theStance.saberName_technical, str, 64);

	jsonNode = cJSON_GetObjectItem (json, "name_simple");
    str = cJSON_ToString (jsonNode);
	Q_strncpyz(theStance.saberName_simple, str, 64);

	// Saber throw data
	jsonNode = cJSON_GetObjectItem (json, "saberThrowSound");
	if(jsonNode)
	{
		str = cJSON_ToString (jsonNode);
		Q_strncpyz(theStance.saberThrowSound, str, MAX_QPATH);
	}
	else
	{
		strcpy(theStance.saberThrowSound, "sound/weapons/saber/saberspin.wav");
	}

	jsonNode = cJSON_GetObjectItem (json, "saberThrowSpeed");
	if(jsonNode)
	{
		theStance.saberThrowSpeed = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.saberThrowSpeed = 500.0f;
	}

	jsonNode = cJSON_GetObjectItem (json, "saberThrowPitch");
	if(jsonNode)
	{
		theStance.saberThrowPitch = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.saberThrowPitch = 0;
	}

	jsonNode = cJSON_GetObjectItem (json, "saberThrowYaw");
	if(jsonNode)
	{
		theStance.saberThrowSpeed = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.saberThrowYaw= 800.0f;
	}

	jsonNode = cJSON_GetObjectItem (json, "saberThrowRoll");
	if(jsonNode)
	{
		theStance.saberThrowRoll = cJSON_ToNumber(jsonNode);
	}
	else
	{
		theStance.saberThrowRoll = 0;
	}


	jsonNode = cJSON_GetObjectItem (json, "offensiveStanceAnim");
    stance = JKG_AnimationForJSON (jsonNode);
    theStance.offensiveStanceAnim = stance;
	if(stance == -1)
	{
		Com_Printf("^3WARNING: %s: invalid Offensive Stance animation (%s)\n", 
			theStance.saberName_technical,
			cJSON_ToString(jsonNode));
	}

	jsonNode = cJSON_GetObjectItem (json, "projectileBlockAnim");
    stance = JKG_AnimationForJSON (jsonNode);
    theStance.projectileBlockAnim = stance;
	if(stance == -1)
	{
		Com_Printf("^3WARNING: %s: invalid Projectile Block Stance animation (%s)\n", 
			theStance.saberName_technical,
			cJSON_ToString(jsonNode));
	}



	jsonNode = cJSON_GetObjectItem (json, "isStaffFriendly");
    stance = cJSON_ToInteger (jsonNode);
    theStance.isStaffFriendly = stance;

	jsonNode = cJSON_GetObjectItem (json, "isDualsFriendly");
    stance = cJSON_ToInteger (jsonNode);
    theStance.isDualsFriendly = stance;

	jsonNode = cJSON_GetObjectItem (json, "isStaffOnly");
    stance = cJSON_ToInteger (jsonNode);
    theStance.isStaffOnly = stance;

	jsonNode = cJSON_GetObjectItem (json, "isDualsOnly");
    stance = cJSON_ToInteger (jsonNode);
    theStance.isDualsOnly = stance;

	jsonNode = cJSON_GetObjectItem (json, "sabSpecific");
    stance = cJSON_ToInteger (jsonNode);
    theStance.sabSpecific = stance;

	// kk, handle all of the move stuff now
	for(i = 0; i < LS_MOVE_MAX; i++)
	{
		JKG_CopyMoveFromDefaultInfo( i, &theMove );
		jsonNode = cJSON_GetObjectItem( json, GetStringForID(SaberMoves, i) );
		if( jsonNode )
		{
			// valid move i guess
			jsonChild = cJSON_GetObjectItem( jsonNode, "FPDrain" );
			if( jsonChild )
			{
				stance = cJSON_ToInteger ( jsonChild );
				theMove.FPdrain = stance;
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "chainIdle" );
			if( jsonChild )
			{
				theMove.chainIdle = JKG_LSForJSON(jsonChild);
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "chainAttack" );
			if( jsonChild )
			{
				theMove.chainAttack = JKG_LSForJSON(jsonChild);
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "startingQuadrant" );
			if( jsonChild )
			{
				theMove.startingQuadrant = JKG_QForJSON(jsonChild);
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "endQuadrant" );
			if( jsonChild )
			{
				theMove.endQuadrant = JKG_QForJSON(jsonChild);
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "blockType" );
			if( jsonChild )
			{
				theMove.blockType = JKG_BLKForJSON(jsonChild);
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "trailLen" );
			if( jsonChild )
			{
				stance = cJSON_ToInteger ( jsonChild );
				theMove.trailLen = stance;
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "blendTime" );
			if( jsonChild )
			{
				stance = cJSON_ToNumber ( jsonChild );
				theMove.blendTime = stance;
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "setanimflag" );
			if( jsonChild )
			{
				theMove.setanimflag = JKG_SAFForJSON( jsonChild );
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "anim" );
			if( jsonChild )
			{
				theMove.anim = JKG_AnimationForJSON( jsonChild );
				if(theMove.anim == -1 && i < 60)	// slight hack
				{
					// badness, warn that this is invalid
					Com_Printf("^3WARNING: %s: invalid anim (%s) for move (%s)\n", 
						theStance.saberName_technical, 
						cJSON_ToString(jsonChild), SaberMoveTable[i].name);
				}
			}

			jsonChild = cJSON_GetObjectItem( jsonNode, "animspeedscale" );
			if( jsonChild )
			{
				theMove.animspeedscale = cJSON_ToNumber(jsonChild);
			}
		}

		memcpy(&theStance.moves[i], &theMove, sizeof(theMove));
	}

	// Get the special move info
	memcpy(&theStance.specialMoves, &defaultSpecialMoves, sizeof(SaberStances[numStancesParsed].specialMoves));
	jsonNode = cJSON_GetObjectItem( json, "SpecialMoves" );
	//if( jsonNode )
	{
		jsonChild = cJSON_GetObjectItem( jsonNode, "allowBackStab" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowBackStab = stance;
		}
		else
		{
			theStance.specialMoves.allowBackStab = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowBackAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowBackAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowBackAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowCrouchedBackAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowCrouchedBackAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowCrouchedBackAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowRollStab" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowRollStab = stance;
		}
		else
		{
			theStance.specialMoves.allowRollStab = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowLunge" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowLunge = stance;
		}
		else
		{
			theStance.specialMoves.allowLunge = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowLeapAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowLeapAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowLeapAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowFlipStab" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowFlipStab = stance;
		}
		else
		{
			theStance.specialMoves.allowFlipStab = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowFlipSlash" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowFlipSlash = stance;
		}
		else
		{
			theStance.specialMoves.allowFlipSlash = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowButterfly" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowButterfly = stance;
		}
		else
		{
			theStance.specialMoves.allowButterfly = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowCartwheel" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowCartwheel = stance;
		}
		else
		{
			theStance.specialMoves.allowCartwheel = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowBackflipAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowBackflipAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowBackflipAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowDualSpinAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowDualSpinAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowDualSpinAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowSpeedLunge" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowSpeedLunge = stance;
		}
		else
		{
			theStance.specialMoves.allowSpeedLunge = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowStabDown" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowStabDown = stance;
		}
		else
		{
			theStance.specialMoves.allowStabDown = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowSpinAttack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowSpinAttack = stance;
		}
		else
		{
			theStance.specialMoves.allowSpinAttack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowSoulCal" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowSoulCal = stance;
		}
		else
		{
			theStance.specialMoves.allowSoulCal = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowBlueKata" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowBlueKata = stance;
		}
		else
		{
			theStance.specialMoves.allowBlueKata = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowYellowKata" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowYellowKata = stance;
		}
		else
		{
			theStance.specialMoves.allowYellowKata = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowRedKata" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowRedKata = stance;
		}
		else
		{
			theStance.specialMoves.allowRedKata = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowUpsideDown" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowUpsideDown = stance;
		}
		else
		{
			theStance.specialMoves.allowUpsideDown = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowPullStab" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowPullStab = stance;
		}
		else
		{
			theStance.specialMoves.allowPullStab = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowPullSlash" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowPullSlash = stance;
		}
		else
		{
			theStance.specialMoves.allowPullSlash = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowAloraSpin" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowAloraSpin = stance;
		}
		else
		{
			theStance.specialMoves.allowAloraSpin = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowDualFrontBack" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowDualFrontBack = stance;
		}
		else
		{
			theStance.specialMoves.allowDualFrontBack = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowDualSides" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowDualSides = stance;
		}
		else
		{
			theStance.specialMoves.allowDualSides = qfalse;
		}

		jsonChild = cJSON_GetObjectItem( jsonNode, "allowHiltBash" );
		if( jsonChild )
		{
			stance = cJSON_ToInteger (jsonChild);
			theStance.specialMoves.allowHiltBash = stance;
		}
		else
		{
			theStance.specialMoves.allowHiltBash = qfalse;
		}
	}

	memcpy(&SaberStances[numStancesParsed], &theStance, sizeof(theStance));
	return qtrue;
}

qboolean JKG_LoadStanceData(void)
{
	char stanceFiles[16384];
	int numFiles = Q_FSGetFileListSorted ("ext_data/stances", ".stance", stanceFiles, sizeof (stanceFiles));
	const char *stanceFile = stanceFiles;
	int successful = 0;
    int failed = 0;

	int t = trap->Milliseconds();
	int i;

	Com_Printf ("------- Stance data -------\n");

    for ( i = 0; i < numFiles; i++ )
    {
        if ( JKG_ParseSaberStanceFile (va ("ext_data/stances/%s", stanceFile)) )
        {
            successful++;
			numStancesParsed++;
        }
        else
        {
            failed++;
        }
            
        stanceFile += strlen (stanceFile) + 1;
    }
    
    Com_Printf ("Successfully loaded %d stances, failed to load %d stances.\n", successful, failed);
    Com_Printf ("Took %d milliseconds.\n", trap->Milliseconds() - t);
    Com_Printf ("-------------------------------------\n");
    
    return (qboolean)(successful > 0);
}
	
void JKG_InitializeStanceData( void )
{

	if(!JKG_LoadStanceData())
	{
		Com_Error(ERR_FATAL, "Could not read saber stance data!");
		return;
	}
}
