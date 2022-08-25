// bg_armor.h
// Everything related to armor and .arm files
// (c) 2016 Jedi Knight Galaxies

#pragma once

#include "qcommon/q_shared.h"
#include <vector>
#include <string>

#define MAX_ARMOR_ITEMS		128		// Max amount of armor items
#define MAX_ARMOR_G2		64		// Max amount of armor G2 instances

// Loading optimization: To prevent having multiple unnecessary GHOUL2
// instances for the same set of armor, we create a reference lookup for
// similar sets. ie, all "stormtrooper" sets will use the same instance.
// All armor with the same reference lookup will use the same instance.
typedef struct {
	char ref[MAX_QPATH];	// The GHOUL2 reference
	void* instance;			// The GHOUL2 instance
	int numSurfaces;		// Total number of surfaces on the model
	int skin;				// The skin to use
} armorG2Reference_t;

typedef struct {
	char ref[MAX_QPATH];	// The name of this jetpack (internal name, for referencing)
	int slot;				// Which armor slot this item equips to - this corresponds to the GHOUL2 model index
	int armor;				// Effective hit points - 25 armor is the same as having 25% extra health on the limb
	int hp;					// Health to add by equipping this piece of armor
	int stamina;			// Stamina/fp to add by equipping this piece of armor
	qboolean filter;		// If true, the armor has a filter that protects from toxins

	float movemodifier;		// Affects how fast you can move with this piece of equipment
	// More movement modifiers? (jump height? stamina usage?)

	struct {
		armorG2Reference_t* pGHOUL2;					// Pointer to GHOUL2 data
		std::vector<std::string> armorOnSurfs;			// Which surfaces to show on the armor (if any)
		std::vector<std::string> bodyOffSurfs;			// Which surfaces to hide on the player (if any)
		char motionBone[MAX_QPATH];						// Which bone to check for motion
		// More model info? (bolting armor onto bones instead of replacing body parts?)

		char equipSound[MAX_QPATH];						// A sound to play when the armor is equipped
	} visuals;
} armorData_t;

extern armorData_t armorTable[MAX_ARMOR_ITEMS];
extern int numLoadedArmor;

void JKG_LoadArmor();
void JKG_UnloadArmor(); // Only really needed on the client (to clean up ghoul2), but probably a good idea to call on the server too
armorData_t* JKG_FindArmorByName(const char* ref);