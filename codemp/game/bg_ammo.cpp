#include <json/cJSON.h>
#include "bg_ammo.h"

#define AMMO_NAMEBUFFER	8192
#define AMMO_FILEBUFFER	16386
#define AMMO_ERRBUFFER	1024

#if defined(_GAME)
	#include "g_local.h"
#elif defined(_CGAME)
	#include "cgame/cg_local.h"
#elif defined(IN_UI)
	#include "ui/ui_local.h"
#endif

#include "bg_buffs.h"

ammo_t ammoTable[MAX_AMMO_TYPES];
int numAmmoLoaded = 0;

/*
============================
BG_GetAmmo

Gets a specific ammo type by name
============================
*/
// Overload 1: Get by string reference
ammo_t *BG_GetAmmo(const char *ammoName) {
	for (int i = 0; i < numAmmoLoaded; i++) {
		if (!Q_stricmp(ammoName, ammoTable[i].name)) {
			return &ammoTable[i];
		}
	}
	return nullptr;
}

// Overload 2: Get by integer
ammo_t *BG_GetAmmo(const int ammoIndex) {
	if (ammoIndex < 0 || ammoIndex >= MAX_AMMO_TYPES) {
		return nullptr;
	}
	return &ammoTable[ammoIndex];
}

/*
============================
BG_GiveAmmo

Not technically a BG function, it's a utility to prevent overflow
============================
*/
#ifdef _GAME
void BG_GiveAmmo(gentity_t* ent, ammo_t* ammo, qboolean max, int amount) 
{
	if (ammo == nullptr)
	{
		Com_Printf(S_COLOR_RED "ammo is a nullptr! Check .ammo files for errors.\n");
		return;
	}

	if (max) {
		ent->client->ammoTable[ammo->ammoIndex] = ammo->ammoMax;
	}
	else
	{
		if (ent->client->ammoTable[ammo->ammoIndex] + amount < 0)
		{
			ent->client->ammoTable[ammo->ammoIndex] = 0;
		}

		else
		{
			ent->client->ammoTable[ammo->ammoIndex] += amount;

			if (ent->client->ammoTable[ammo->ammoIndex] > ammo->ammoMax)
			{
				ent->client->ammoTable[ammo->ammoIndex] = ammo->ammoMax;
			}
		}
	}
}
#endif

/*
==============================
BG_GetRefillAmmoCost

Gets the cost of refilling ammo, given an array of ammo and a weapon
==============================
*/
int BG_GetRefillAmmoCost(unsigned short* ammo, weaponData_t* wp)
{
	int totalCost = 0;
	std::vector<ammo_t*> uniqueAmmo;	//keeps track of which ammotypes are unique among the firing modes (eg: stun and primary are often the same ammo)
	//float ammoAdjust = 0.0;

		//if passiveUnderdogBonus is enabled and our team is losing, our ammo is discounted!  Awww, thanks vendor!  
		/*if (jkg_passiveUnderdogBonus.integer > 0)	//--Futuza: need to adjust BG_GetRefillAmmoCost() to include access to ent so we can see who's team we're on, logic should match Cmd_BuyAmmo_f() in g_cmds.cpp
		{
			//who is currently winning?
			auto my_team = ent->client->sess.sessionTeam;
			int curr_winner = -1;
			if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE])
				curr_winner = TEAM_RED;
			else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE])
				curr_winner = TEAM_BLUE;
			else
				curr_winner = -1;	//tie

			//we're losing - give us discount ammo please!
			if (my_team != curr_winner && curr_winner != -1)
			{
				ammoAdjust = 0.9;

				//evaluate how badly the losing team is losing by and reduce ammo accordingly
				int diff = level.teamScores[curr_winner] - level.teamScores[my_team];
				ammoAdjust = (1 - (float)diff / level.teamScores[curr_winner]);

				if (ammoAdjust > 0.9)	//minimum discount is 10% off
					ammoAdjust = 0.9;

				if (ammoAdjust < 0.25)
					ammoAdjust = 0.25;		//maximum discount is 75% off
			}
		}*/

	
	for (int i = 0; i < wp->numFiringModes; i++)
	{
		ammo_t* ammoType = wp->firemodes[i].ammoDefault;
		if (ammoType == nullptr)
		{
			continue;	// bad linkage - shouldn't happen
		}

		//add the first ammo to the vector
		if (i == 0)
		{
			uniqueAmmo.push_back(ammoType);
			continue;
		}

		//compare each ammo in the vector to our new ammo to ensure its unique
		for (int j = 0; j <= uniqueAmmo.size(); j++)
		{
			if (j == uniqueAmmo.size())
				uniqueAmmo.push_back(ammoType); //we made it to the end, its unique

			if (uniqueAmmo[j] != ammoType)
				continue; //unique so far

			else
				break; //found a samesy
		}
	}

	//add up all the unique ammo
	for (int i = 0; i < uniqueAmmo.size(); i++)
	{
		int ammoCount = uniqueAmmo[i]->ammoMax - ammo[uniqueAmmo[i]->ammoIndex];

		//int cost = (float)uniqueAmmo[i]->pricePerUnit * ammoAdjust;  //if we enable above underdogbonus, uncomment this and adjust below line
		totalCost += ammoCount * uniqueAmmo[i]->pricePerUnit;
	}

	return totalCost;
}

/*
============================
BG_GetAllAmmoSubstitutions

Gets all substitutions for a specifc ammo type
Also includes the original ammo type in the list
============================
*/
void BG_GetAllAmmoSubstitutions(int ammoIndex, std::vector<ammo_t*>& outSubs) {
	if (ammoIndex < 0 || ammoIndex >= numAmmoLoaded) {
		return;
	}

	outSubs.clear();

	ammo_t* find = &ammoTable[ammoIndex]; //which ammo we're looking for

	//--futuza: note that doing things this way might be more accurate to what we're searching for, but would probably require a rewrite of JKG.ammo and most weapon files.
	//			I would probably do it this way if instead of cycling to the next ammo you wanted to generate a list of ammo that worked with the weapon.
	/*if (find->pSub->ammoIndex != ammoIndex) //if the ammo we're searching for is a substitution itself, recursive time
	{
		//find = &ammoTable[find->pSub->ammoIndex];
		BG_GetAllAmmoSubstitutions(find->pSub->ammoIndex, outSubs);
		return;
	}*/

	for (int i = 0; i < numAmmoLoaded; i++) {
		ammo_t* thisAmmo = &ammoTable[i];
		if (thisAmmo->pSub == find || thisAmmo == find) {
			outSubs.push_back(thisAmmo);
		}
	}
}

/*
============================
BG_WeaponAcceptsAlternateAmmo

Returns true if the weapon accepts alternate ammo, false otherwise
============================
*/
qboolean BG_WeaponAcceptsAlternateAmmo(int weapon, int variation) {
	weaponData_t* wp = GetWeaponData(weapon, variation);

	switch (weapon) {
		case WP_SABER:
		case WP_NONE:
		case WP_MELEE:
			return qfalse;
		default:
			break;
	}

	if (wp->firemodes[0].clipSize <= 0) {
		return qfalse;
	}

	if (wp->firemodes[0].useQuantity) {
		return qfalse;
	}

	return qtrue;
}

/*
============================
BG_AmmoIsBasedOn

Returns true if the ammo in arg 1 is based on (can be substituted for) the ammo in arg 2
============================
*/
qboolean BG_AmmoIsBasedOn(int ammoTypeIndex, int basedOnIndex)
{
	std::vector<ammo_t*> ammoTypes;
	BG_GetAllAmmoSubstitutions(basedOnIndex, ammoTypes);

	for (auto it = ammoTypes.begin(); it != ammoTypes.end(); ++it)
	{
		if ((*it)->ammoIndex == ammoTypeIndex)
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
============================
JKG_ApplyAmmoOverride

Applies an ammo override (if present)
============================
*/
void JKG_ApplyAmmoOverride(int& value, const complexAmmoOverride<int>& field) {
	if (!field.bIsPresent) {
		return;
	}

	if (field.bSet) {
		value = field.set;
	}
	if (field.bAdd) {
		value += field.add;
	}
	if (field.multiply) {
		value *= field.multiply;
	}
}

void JKG_ApplyAmmoOverride(double& value, const complexAmmoOverride<double>& field) {
	if (!field.bIsPresent) {
		return;
	}

	if (field.bSet) {
		value = field.set;
	}
	if (field.bAdd) {
		value += field.add;
	}
	if (field.multiply) {
		value *= field.multiply;
	}
}

void JKG_ApplySimpleAmmoOverride(int& value, const std::pair<qboolean, int>& field)
{
	if (!field.first)
	{
		return;
	}
	value = field.second;
}

void JKG_ApplySimpleAmmoOverride(double& value, const std::pair<qboolean, double>& field)
{
	if (!field.first)
	{
		return;
	}
	value = field.second;
}

/*
============================

============================
*/
qboolean JKG_SimpleAmmoOverridePresent(const std::pair<qboolean, int>& field)
{
	return field.first;
}

qboolean JKG_SimpleAmmoOverridePresent(const std::pair<qboolean, double>& field)
{
	return field.first;
}

qboolean JKG_SimpleAmmoOverridePresent(const std::pair<qboolean, std::string>& field)
{
	return field.first;
}

qboolean JKG_SimpleAmmoOverridePresent(const std::pair<qboolean, std::vector<std::string>>& field)
{
	return field.first;
}

qboolean JKG_SimpleAmmoOverridePresent(const std::pair<qboolean, vec3_t>& field)
{
	return field.first;
}

/*
============================
JKG_ParseAmmoOverride_Int

Parses a single ammo override
============================
*/
static void JKG_ParseAmmoOverride_Int(cJSON* json, const char* name, complexAmmoOverride<int>& field) {
	cJSON* node = cJSON_GetObjectItem(json, name);
	if (!node) {
		field.bIsPresent = qfalse;
	}
	else {
		cJSON* child;

		field.bIsPresent = qtrue;
		field.bAdd = qfalse;
		field.bSet = qfalse;
		field.bMultiply = qfalse;

		child = cJSON_GetObjectItem(node, "set");
		if (child) {
			field.bSet = qtrue;
			field.set = cJSON_ToInteger(child);
		}

		child = cJSON_GetObjectItem(node, "add");
		if (child) {
			field.bAdd = qtrue;
			field.add = cJSON_ToInteger(child);
		}

		child = cJSON_GetObjectItem(node, "multiply");
		if (child) {
			field.bMultiply = qtrue;
			field.multiply = cJSON_ToNumber(child);
		}
	}
}

/*
============================
JKG_ParseAmmoOverride_Float

Parses a single ammo override
============================
*/
static void JKG_ParseAmmoOverride_Float(cJSON* json, const char* name, complexAmmoOverride<double>& field) {
	cJSON* node = cJSON_GetObjectItem(json, name);
	if (!node) {
		field.bIsPresent = qfalse;
	}
	else {
		cJSON* child;

		field.bIsPresent = qtrue;
		field.bAdd = qfalse;
		field.bSet = qfalse;
		field.bMultiply = qfalse;

		child = cJSON_GetObjectItem(node, "set");
		if (child) {
			field.bSet = qtrue;
			field.set = cJSON_ToNumber(child);
		}

		child = cJSON_GetObjectItem(node, "add");
		if (child) {
			field.bAdd = qtrue;
			field.add = cJSON_ToNumber(child);
		}

		child = cJSON_GetObjectItem(node, "multiply");
		if (child) {
			field.bMultiply = qtrue;
			field.multiply = cJSON_ToNumber(child);
		}
	}
}

/*
============================
JKG_ParseAmmoOverride_Means

Parses a means of death override
============================
*/
static void JKG_ParseAmmoOverride_Means(cJSON* json, const char* name, std::pair<qboolean, int>& field) {
	cJSON* node = cJSON_GetObjectItem(json, name);
	if (!node) {
		field.first = qfalse;
	}
	else {
		field.first = qtrue;
		field.second = JKG_GetMeansOfDamageIndex(cJSON_ToString(node));
	}
}

/*
============================
JKG_ParseSimpleOverrideString

============================
*/
static void JKG_ParseSimpleOverrideString(std::pair<qboolean, std::string>& field, const char* nodeName, cJSON* json)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	if (child)
	{
		field.first = qtrue;
		field.second = cJSON_ToString(child);
	}
	else
	{
		field.first = qfalse;
	}
}

/*
============================
JKG_ParseSimpleOverrideFloat

============================
*/
static void JKG_ParseSimpleOverrideFloat(std::pair<qboolean, float>& field, const char* nodeName, cJSON* json)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	if (child)
	{
		field.first = qtrue;
		field.second = cJSON_ToNumber(child);
	}
	else
	{
		field.first = qfalse;
	}
}

/*
============================
JKG_ParseSimpleOverrideInt

============================
*/
static void JKG_ParseSimpleOverrideInt(std::pair<qboolean, int>& field, const char* nodeName, cJSON* json)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	if (child)
	{
		field.first = qtrue;
		field.second = cJSON_ToInteger(json);
	}
	else
	{
		field.first = qfalse;
	}
}

/*
============================
JKG_ParseSimpleOverrideVector

============================
*/
static void JKG_ParseSimpleOverrideVector(std::pair<qboolean, std::vector<std::string>>& field, const char* nodeName, cJSON* json)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	if (child)
	{
		field.first = qtrue;
		for (int i = 0; i < cJSON_GetArraySize(child); i++)
		{
			field.second.push_back(cJSON_ToString(cJSON_GetArrayItem(child, i)));
		}
	}
	else
	{
		field.first = qfalse;
	}
}

/*
============================
JKG_ParseSimpleOverrideVec3

============================
*/
static void JKG_ParseSimpleOverrideVec3(std::pair<qboolean, vec3_t>& field, const char* nodeName, cJSON* json)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	int max;

	if (child)
	{
		field.first = qtrue;
		max = cJSON_GetArraySize(child);
		if (max != 3)
		{
			field.second[0] = field.second[1] = field.second[2] = 1.0f;
		}
		else
		{
			for (int i = 0; i < max; i++)
			{
				field.second[i] = cJSON_ToNumber(cJSON_GetArrayItem(child, i));
			}
		}
	}
	else
	{
		field.first = qfalse;
	}
}

/*
============================
JKG_ParseBuffOverrides

This parses all of the buff overrides for an ammo type
============================
*/
#ifndef UI_EXPORTS	// UI doesn't have a friggin clue what buffs are
static void JKG_ParseBuffOverrides(ammo_t* ammo, cJSON* json, const char* nodeName)
{
	cJSON* child = cJSON_GetObjectItem(json, nodeName);
	if (child == nullptr)
	{
		return;
	}

	int elemCount = cJSON_GetArraySize(child);
	if (elemCount <= 0)
	{
		return; // not actually an array
	}

	// Iterate through each buff array item
	for (int i = 0; i < elemCount; i++)
	{
		cJSON* buff = cJSON_GetArrayItem(child, i);
		complexAmmoBuffOverride buffOverride = { 0 };

		cJSON* buffChild = cJSON_GetObjectItem(buff, "buff");
		if (buffChild == nullptr)
		{	// JSON doesn't contain a "buff" field. Continue.
			continue;
		}

		buffOverride.buff = JKG_ResolveBuffName(cJSON_ToStringOpt(buffChild, ""));
		if (buffOverride.buff < 0)
		{	// A buff by this name doesn't exist, continue.
			continue;
		}

		buffChild = cJSON_GetObjectItem(buff, "remove");
		buffOverride.bRemove = cJSON_ToBooleanOpt(buffChild, qfalse);

		buffChild = cJSON_GetObjectItem(buff, "addbuff");
		buffOverride.bAddBuff = cJSON_ToBooleanOpt(buffChild, qfalse);

		buffChild = cJSON_GetObjectItem(buff, "duration");
		if (buffChild != nullptr)
		{
			cJSON* childProps;

			childProps = cJSON_GetObjectItem(buffChild, "add");
			if (childProps != nullptr)
			{
				buffOverride.bAddDuration = qtrue;
				buffOverride.addDuration = cJSON_ToInteger(childProps);
			}

			childProps = cJSON_GetObjectItem(buffChild, "multiply");
			if (childProps != nullptr)
			{
				buffOverride.bMultiplyDuration = qtrue;
				buffOverride.multiplyDuration = cJSON_ToNumber(childProps);
			}

			childProps = cJSON_GetObjectItem(buffChild, "set");
			if (childProps != nullptr)
			{
				buffOverride.bSetDuration = qtrue;
				buffOverride.setDuration = cJSON_ToInteger(childProps);
			}
		}

		buffChild = cJSON_GetObjectItem(buff, "intensity");
		if (buffChild != nullptr)
		{
			cJSON* childProps;

			childProps = cJSON_GetObjectItem(buffChild, "add");
			if (childProps != nullptr)
			{
				buffOverride.bAddIntensity = qtrue;
				buffOverride.addIntensity = cJSON_ToNumber(childProps);
			}

			childProps = cJSON_GetObjectItem(buffChild, "multiply");
			if (childProps != nullptr)
			{
				buffOverride.bMultiplyIntensity = qtrue;
				buffOverride.multiplyIntensity = cJSON_ToNumber(childProps);
			}

			childProps = cJSON_GetObjectItem(buffChild, "set");
			if (childProps != nullptr)
			{
				buffOverride.bSetIntensity = qtrue;
				buffOverride.setIntensity = cJSON_ToNumber(childProps);
			}
		}

		// Add the override to the list of buff overrides
		ammo->overrides.buffs.emplace_back(buffOverride);
	}
}
#endif

/*
============================
JKG_ParseAmmoOverrideVisuals

Processes the visual aspect of an ammo override
============================
*/

static void JKG_ParseAmmoOverrideVisuals(ammo_t* ammo, cJSON* json)
{
	cJSON* child;

	if (json == nullptr)
	{
		ammo->visualOverrides.crosshairShader.first = qfalse;
		goto dead1;
	}
	JKG_ParseSimpleOverrideString(ammo->visualOverrides.crosshairShader, "crosshairShader", json);

	// weaponRender/weaponFire
	child = cJSON_GetObjectItem(json, "fire");
	if (child)
	{
		JKG_ParseAmmoOverride_Float(child, "muzzleLightIntensity", ammo->visualOverrides.muzzleLightIntensity);
		JKG_ParseSimpleOverrideVec3(ammo->visualOverrides.muzzleLightColor, "muzzleLightColor", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.chargingEffect, "chargingEffect", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.muzzleEffect, "muzzleEffect", child);
		JKG_ParseSimpleOverrideVector(ammo->visualOverrides.fireSound, "fireSound", child);
	}
	else
	{
dead1:
		ammo->visualOverrides.muzzleLightIntensity.bIsPresent = qfalse;
		ammo->visualOverrides.muzzleLightColor.first = qfalse;
		ammo->visualOverrides.chargingEffect.first = qfalse;
		ammo->visualOverrides.muzzleEffect.first = qfalse;
		ammo->visualOverrides.fireSound.first = qfalse;
		if (json == nullptr)
		{
			goto dead2;
		}
	}

	// traceline
	child = cJSON_GetObjectItem(json, "traceline");
	if (child)
	{
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.traceline.shader, "shader", child);
		JKG_ParseAmmoOverride_Float(child, "minSize", ammo->visualOverrides.traceline.minSize);
		JKG_ParseAmmoOverride_Float(child, "maxSize", ammo->visualOverrides.traceline.maxSize);
		JKG_ParseAmmoOverride_Int(child, "lifeTime", ammo->visualOverrides.traceline.lifeTime);
	}
	else
	{
dead2:
		ammo->visualOverrides.traceline.shader.first = qfalse;
		ammo->visualOverrides.traceline.minSize.bIsPresent = qfalse;
		ammo->visualOverrides.traceline.maxSize.bIsPresent = qfalse;
		ammo->visualOverrides.traceline.lifeTime.bIsPresent = qfalse;
		if (json == nullptr)
		{
			goto dead3;
		}
	}

	// projectile
	child = cJSON_GetObjectItem(json, "projectile");
	if (child)
	{
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.projectileModel, "model", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.projectileEffect, "effect", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.runSound, "runSound", child);
		JKG_ParseAmmoOverride_Float(child, "lightIntensity", ammo->visualOverrides.projectile.lightIntensity);
		JKG_ParseSimpleOverrideVec3(ammo->visualOverrides.projectile.lightColor, "lightColor", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.deathEffect, "deathfx", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.missEffect, "miss", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.impactEffect, "hit", child);
		JKG_ParseSimpleOverrideString(ammo->visualOverrides.projectile.deflectEffect, "deflect", child);
	}
	else
	{
dead3:
		ammo->visualOverrides.projectile.projectileModel.first = qfalse;
		ammo->visualOverrides.projectile.projectileEffect.first = qfalse;
		ammo->visualOverrides.projectile.runSound.first = qfalse;;
		ammo->visualOverrides.projectile.lightIntensity.bIsPresent = qfalse;
		ammo->visualOverrides.projectile.lightColor.first = qfalse;
		ammo->visualOverrides.projectile.deathEffect.first = qfalse;
		ammo->visualOverrides.projectile.missEffect.first = qfalse;
		ammo->visualOverrides.projectile.impactEffect.first = qfalse;
		ammo->visualOverrides.projectile.deflectEffect.first = qfalse;
	}
}

/*
============================
JKG_ParseAmmoOverrides

Overrides are things such as changing damage, ...
============================
*/
static void JKG_ParseAmmoOverrides(ammo_t* ammo, cJSON* json) {
	
	JKG_ParseAmmoOverride_Means(json, "meansofdeath", ammo->overrides.means);
	JKG_ParseAmmoOverride_Means(json, "splashmeansofdeath", ammo->overrides.splashmeans);


	JKG_ParseAmmoOverride_Int(json, "damage", ammo->overrides.damage);
	JKG_ParseAmmoOverride_Int(json, "projectiles", ammo->overrides.projectiles);
	JKG_ParseAmmoOverride_Float(json, "splashRange", ammo->overrides.splashRange);
	JKG_ParseAmmoOverride_Float(json, "collisionSize", ammo->overrides.collisionSize);
	JKG_ParseAmmoOverride_Float(json, "recoil", ammo->overrides.recoil);
	JKG_ParseAmmoOverride_Int(json, "ammocost", ammo->overrides.ammocost);
	JKG_ParseAmmoOverride_Int(json, "fireDelay", ammo->overrides.fireDelay);
	JKG_ParseAmmoOverride_Int(json, "bounces", ammo->overrides.bounces);
	JKG_ParseAmmoOverride_Int(json, "accuracyRatingBase", ammo->overrides.accuracyRatingBase);
	JKG_ParseAmmoOverride_Int(json, "accuracyRatingPerShot", ammo->overrides.accuracyRatingPerShot);
	JKG_ParseAmmoOverride_Float(json, "knockback", ammo->overrides.knockback);
	JKG_ParseAmmoOverride_Float(json, "speed", ammo->overrides.speed);

#ifndef UI_EXPORTS	// UI hasn't got a clue what buffs are
	JKG_ParseBuffOverrides(ammo, json, "buffs");
#endif

	JKG_ParseSimpleOverrideInt(ammo->overrides.useGravity, "useGravity", json);

	JKG_ParseSimpleOverrideInt(ammo->overrides.hitscan, "hitscan", json);

	JKG_ParseAmmoOverrideVisuals(ammo, cJSON_GetObjectItem(json, "visuals"));
}

/*
============================
JKG_ParseSingleAmmo

Reads a single ammo entry from a .ammo file
============================
*/
static qboolean JKG_ParseSingleAmmo(cJSON* json) {
	const char* name = cJSON_GetItemKey(json);
	ammo_t* ammo = &ammoTable[numAmmoLoaded];
	cJSON* child;

	ammo->ammoIndex = numAmmoLoaded;
	Q_strncpyz(ammo->name, name, sizeof(ammo->name));

	Q_strncpyz(ammo->shortname, cJSON_ToStringOpt(cJSON_GetObjectItem(json, "shortname"), ""), sizeof(ammo->shortname));

	// Parse substitutes (we need to establish the link later)
	ammo->pSub = nullptr;
	child = cJSON_GetObjectItem(json, "basedOn");
	if (child) {
		Q_strncpyz(ammo->substitute, cJSON_ToString(child), sizeof(ammo->substitute));
	}
	else {
		Q_strncpyz(ammo->substitute, ammo->name, sizeof(ammo->substitute));
	}

	child = cJSON_GetObjectItem(json, "max");
	ammo->ammoMax = cJSON_ToIntegerOpt(child, 999);

	child = cJSON_GetObjectItem(json, "pricePerUnit");
	ammo->pricePerUnit = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(json, "overrides");
	if (child) {
		JKG_ParseAmmoOverrides(ammo, child);
	}

	return qtrue;
}

/*
============================
JKG_LoadAmmo

Loads an individual ammo (.ammo) file.
Called on both the client and the server.
============================
*/
static qboolean JKG_LoadAmmo(const char* fileName, const char* dir) {
	int fileLen;
	fileHandle_t f;
	char fileBuffer[AMMO_FILEBUFFER];
	char error[AMMO_ERRBUFFER];

	fileLen = trap->FS_Open(va("%s%s", dir, fileName), &f, FS_READ);

	if (!f || fileLen == -1) {
		Com_Printf(S_COLOR_RED "Could not read %s\n", fileName);
		return qfalse;
	}

	if (fileLen == 0) {
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s is blank\n", fileName);
		return qfalse;
	}

	if ((fileLen + 1) >= AMMO_FILEBUFFER) {
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s is too big - max file size %d bytes (%.2f kb)\n", fileName, AMMO_FILEBUFFER, AMMO_FILEBUFFER / 1024.0f);
		return qfalse;
	}

	trap->FS_Read(&fileBuffer, fileLen, f);
	fileBuffer[fileLen] = '\0';
	trap->FS_Close(f);

	cJSON* json = cJSON_ParsePooled(fileBuffer, error, AMMO_ERRBUFFER);
	if (!json) {
		Com_Printf(S_COLOR_RED "%s: %s\n", fileName, error);
		return qfalse;
	}

	// Read each node in the json file from top level as an ammo_t
	cJSON* child;
	for (child = cJSON_GetFirstItem(json); child; child = cJSON_GetNextItem(child)) {
		if (numAmmoLoaded >= MAX_AMMO_TYPES) {
			Com_Printf("MAX_AMMO_TYPES exceeded\n");
			break;
		}
		if (JKG_ParseSingleAmmo(child)) {
			numAmmoLoaded++;
		}
	}

	cJSON_Delete(json);
	return qtrue;
}

/*
========================
BG_InitializeAmmo

Initializes the ammo subsystem.
========================
*/
void BG_InitializeAmmo ( void )
{
	char ammoFiles[AMMO_NAMEBUFFER] {0};
	const char* ammo;
	const char* ammoDir = "ext_data/ammo/";
	int failed = 0;
	int numFiles;

	numFiles = Q_FSGetFileListSorted(ammoDir, ".ammo", ammoFiles, sizeof(ammoFiles));
	ammo = ammoFiles;

	Com_Printf("------- Ammo -------\n");

	for (int i = 0; i < numFiles; i++) {
		if (!JKG_LoadAmmo(ammo, ammoDir)) {
			failed++;
			ammo += strlen(ammo) + 1;
			continue;
		}

		ammo += strlen(ammo) + 1;
	}

	Com_Printf("Ammo: %d successful, %d failed.\n", numAmmoLoaded, failed);
	Com_Printf("-------------------------------------\n");

	// Link up all of the substitutes
	for (int i = 0; i < numAmmoLoaded; i++) {
		ammoTable[i].pSub = BG_GetAmmo(ammoTable[i].substitute);
	}
}