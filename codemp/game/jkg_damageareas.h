#ifndef JKG_DAMAGETYPES_H
#define JKG_DAMAGETYPES_H

#include "g_local.h"
#include "bg_public.h"
#include "game/bg_weapons.h"

typedef struct damageInstance_s
{
	gentity_t *attacker; // e.g. a player
	gentity_t *inflictor; // e.g. the player's rocket
	gentity_t *ignoreEnt;
	vec3_t direction;
	int methodOfDeath;
	int damageFlags;
	int ammoType;

	// Charge related
	int damageOverride;
} damageInstance_t;

typedef struct damageArea_s
{
	int         startTime;
	int         lastDamageTime;
	vec3_t      origin;
	damageInstance_t context;

	const damageSettings_t *data;
} damageArea_t;

struct damageDecay_t
{
	float recommendedRange;
	float maxRange;
	float distanceToDamageOrigin;
	float decayRate;
};

void JKG_DoDamage(
	weaponFireModeStats_t *fireMode,
	gentity_t *targ,
	gentity_t *inflictor,
	gentity_t *attacker,
	vec3_t dir,
	vec3_t origin,
	int dflags,
	int mod,
	const damageDecay_t *decay = nullptr);

void JKG_DoSplashDamage(
	damageSettings_t *data,
	const vec3_t origin,
	gentity_t *inflictor,
	gentity_t *attacker,
	gentity_t *ignoreEnt,
	int mod);

void JKG_DoDirectDamage(
	weaponFireModeStats_t *fireMode,
	gentity_t *targ,
	gentity_t *inflictor,
	gentity_t *attacker,
	vec3_t dir,
	vec3_t origin,
	int dflags,
	int mod,
	const damageDecay_t *decay = nullptr);

void JKG_DamagePlayers(void);

void G_TickBuffs(gentity_t *ent);

int JKG_CalculateDamageDecay(int damage, float distance, float range, float decayRate);
int JKG_AdjustAmbientHeatDamage(bool heat, int damage);	//adjust debuff fire/cold damage based on ambient tempature

constexpr float MAX_RANGE_MULTIPLIER = 10.0;	//how many times should we bother calculating beyond a weapon's range

#endif
