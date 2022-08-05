#pragma once
#include "qcommon/q_shared.h"

#define BUFF_CATEGORY_LEN	32
#define BUFF_NAME_LEN		64
#define	BUFF_BOLT_LEN		32
#define BUFF_EFX_LEN		MAX_QPATH

/*
 *	Buffs can also cancel buffs in other categories, as long as the cancel priority is higher
 */
struct jkgOtherBuffCancel_t
{
	char	category[BUFF_CATEGORY_LEN];
	int		priority;
};

/*
 *	Buffs can cancel other buffs that are present, as long as their priority is higher.
 */
struct jkgBuffCanceling_t
{
	int			priority;
	qboolean	noBuffStack;		// if true, don't allow buffs with the same ID to stack
	qboolean	noCategoryStack;	// if true, don't allow buffs in the same category to stack
	qboolean	waterRemoval;		// if true, swimming around in water removes the buff
	qboolean	rollRemoval;		// if true, rolling removes the buff
	qboolean	shieldRemoval;		// if true, if an active shield is equipped (with at least 1 charge) will remove the buff
	qboolean	filterRemoval;		// if true, if armor with the filter property is equipped will remove the buff

	std::vector<jkgOtherBuffCancel_t> other;
};

/*
 *	Buffs can deal damage or heal the player.
 */
struct jkgBuffDamage_t
{
	int			damage;
	qboolean	percentage;		//if true, interpret damage as a % based on current health (eg: 5% of 100 = 5dmg), instead of a hard value
	qboolean	deadly;			//if the buff should be allowed to do last hits, true by default
	int			meansOfDeath;
	int			damageRate;		// how often, in ms, to damage the player
};

/*
 *	Buffs can visually affect the player by applying a wholebody shader over them, or drawing an EFX over them.
 */
struct jkgBuffVisuals_t
{
	qboolean	useEFX;
	char		efx[BUFF_EFX_LEN];
	char		efxBolt[BUFF_BOLT_LEN];
	int			efxDebounce;
	qboolean	useShader;
	char		shader[BUFF_EFX_LEN];
	int			shaderRGBA[4];
	int			shaderLen;
};

/*
 *	Buffs can affect various passive stats of the player, like movement speed, resistances, etc
 */
struct jkgBuffPassive_t
{
	std::pair<qboolean, int>	overridePmoveType;

	float movemodifier;			// Affects how your movement is affected by the debuff (1.0 is standard, 0.5 is half speed, 2.0 is double speed)
	float movemodifier_cur;		// Increases/decreases with additional stacks
	unsigned int maxstacks;		// How many times the movemodifier effect can stack (0 is the same as disabling the movement modifier, set to 1 if you want 1 stack)
	unsigned int stacks;		// How many stacks we currently have

	qboolean empstaggered;		//are your electronics shorted out? (prevents jetpack activation/other things not yet implemented, like maybe hud shutoff?)
	qboolean resistant;			//does the buff give you resistance? (reduces incoming damage by 50%)
	qboolean knockdown;			//knock the player over if true

};

/*
 *	Buffs can be applied to clients.
 *	Currently they just do damage or alter movement or remove other debuffs.
 */
struct jkgBuff_t
{
	char				name[BUFF_NAME_LEN];
	char				category[BUFF_CATEGORY_LEN];

	jkgBuffCanceling_t	cancel;
	jkgBuffDamage_t		damage;
	jkgBuffVisuals_t	visuals;
	jkgBuffPassive_t	passive;
	qboolean			remove_f{ false };	//if true the buff has been marked for removal
};

extern jkgBuff_t buffTable[MAX_BUFFS];

void JKG_InitializeBuffs();
qboolean JKG_HasFreezingBuff(entityState_t* es);
qboolean JKG_HasFreezingBuff(playerState_t* ps);
qboolean JKG_HasFreezingBuff(playerState_t& ps);
qboolean JKG_HasResistanceBuff(playerState_t* ps);
void JKG_RemoveBuffCategory(const char* buffCategory, playerState_t* ps);
void JKG_CheckWaterRemoval(playerState_t* ps);
void JKG_CheckRollRemoval(playerState_t* ps);
void JKG_CheckFilterRemoval(playerState_t* ps);
int  JKG_ResolveBuffName(const char* szBuffName);
void JKG_GetBuffNames(std::vector<std::string>& outBuffNames);