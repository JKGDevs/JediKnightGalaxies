// bg_damage.h
// Contains all of the code pertaining to custom damage types (meansOfDamage.json)
// (c) 2016 Jedi Knight Galaxies

#pragma once

#include "qcommon/q_shared.h"
#include <vector>
#include <string>

#define MAX_OBIT_LENGTH			64
#define MAX_MEANSOFDAMAGE_NAME	64

struct meansOfDamage_t {
	char ref[MAX_MEANSOFDAMAGE_NAME];
	char inventoryName[MAX_QPATH];

	qboolean hitmarkerExplosives;
	qboolean disintegrate;

	struct {
		qboolean noDamagePlums;

		qboolean overrideDamagePlum;
		vec4_t overrideDamagePlumColor;

		qboolean overrideShieldDamagePlum;
		vec4_t overrideShieldDamagePlumColor;

		qboolean overrideLowDamagePlum;
		vec4_t overrideLowDamagePlumColor;
	} plums;

	struct {
		qboolean ignoreArmor;	// if true, this damage type goes through armor
		qboolean ignoreShield;	// if true, this damage types goes through shields
		qboolean dodgeable;		// if true, this damage type can be dodged
		qboolean shieldBlocks;	// if true, this damage type is blocked entirely by shields
		qboolean shieldProtects;// if true, this damage type cannot be applied as a debuff while protected by a shield (can still damage shields tho)
		qboolean isEMP;			// if true, this damage is electric based (can effect electric equipment)
		qboolean isCC;			// if true, this damage is considered crowd control
		qboolean canRevive;		// if true, this damage can bring back ents from death

		//multiply damage to specific types (eg: 1.0 == normal damage, 1.25 shield == +25% extra damage to shields)
		float armor;
		float shield;
		float organic;
		float droid;

		float armorPenetration;	//how much % of total armor is ignored
	} modifiers;

	struct {
		qboolean canDismember;
		qboolean blowChunks;
	} dismemberment;

	struct {
		char killMessage[MAX_OBIT_LENGTH];
		char deathMessage[MAX_OBIT_LENGTH];

		union {
			char message[MAX_OBIT_LENGTH];
			struct {
				char male[MAX_OBIT_LENGTH];
				char female[MAX_OBIT_LENGTH];
				char neuter[MAX_OBIT_LENGTH];
			} gendered;
		} suicideMessage;
		qboolean genderedStringsPresent;
	} killfeed;
};

void JKG_LoadMeansOfDamage();
meansOfDamage_t* JKG_GetMeansOfDamage(const std::string& ref);
meansOfDamage_t* JKG_GetMeansOfDamage(int num);
int JKG_GetMeansOfDamageIndex(const std::string& ref);

extern std::vector<meansOfDamage_t> allMeansOfDamage;