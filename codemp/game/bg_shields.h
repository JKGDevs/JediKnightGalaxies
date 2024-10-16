// bg_shields.h
// Everything related to shields and .shd files
// (c) 2024 Jedi Knight Galaxies

#pragma once

#include "qcommon/q_shared.h"
#include <vector>
#include <string>

#define SHIELD_DEFAULT_CAPACITY	25
#define SHIELD_DEFAULT_COOLDOWN	10000
#define SHIELD_DEFAULT_REGEN	100

#define MAX_SHIELDS	64

struct shieldData_t {
	char	ref[MAX_QPATH];		// The name of this shield (internal name, for referencing)

	int capacity;	// Total amount that the shield can absorb
	int cooldown;	// Time between when we received a hit and when the shield will start recharging
	int regenrate;	// Time (ms) it takes to recharge one shield unit

	char rechargeSoundEffect[MAX_QPATH];	// Sound that plays once we start charging
	char brokenSoundEffect[MAX_QPATH];		// Sound that plays once the shield is broken
	char equippedSoundEffect[MAX_QPATH];	// Sound that plays once the shield is equipped
	char chargedSoundEffect[MAX_QPATH];		// Sound that plays once a shield is finished charging
	char malfunctionSoundEffect[MAX_QPATH];	// Sound that plays if the shield has malfunctioned --futuza: to be added later

	std::vector<int> blockedMODs;			// what MODs the shield is a barrier to, if .size() < 1 then it has default types
	std::vector<int> allowedMODs;			// what MODs pass through the shield, if .size() < 1 then it has the default types
};

extern shieldData_t shieldTable[MAX_SHIELDS];
extern int numLoadedShields;

void JKG_LoadShields();
shieldData_t* JKG_FindShieldByName(const char* shieldName);