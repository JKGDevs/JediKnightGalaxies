#include "bg_damage.h"
#ifdef _GAME
#include "g_local.h"
#elif defined (IN_UI)
#include "ui/ui_local.h"
#else
#include "cgame/cg_local.h"
#endif
#include "json/cJSON.h"

#define MAX_MEANSOFDAMAGE_BUFFER	32768
#define MAX_MEANSOFDAMAGE_ERROR		1024

std::vector<meansOfDamage_t> allMeansOfDamage;

/*
=========================
JKG_ParseJSONColor
=========================
*/
static qboolean JKG_ParseJSONColor(cJSON* json, vec4_t& color)
{
	if (!cJSON_IsArray(json))
	{
		return qfalse;
	}

	for (int i = 0; i < cJSON_GetArraySize(json) && i < 4; i++)
	{
		color[i] = cJSON_ToNumber(cJSON_GetArrayItem(json, i));
	}

	return qtrue;
}

/*
=========================
JKG_ParseSingleMeansOfDamage

=========================
*/
static void JKG_ParseSingleMeansOfDamage(const char* name, cJSON* json) {
	meansOfDamage_t mod = {};
	cJSON* jsonNode;
	cJSON* child;
	
	Q_strncpyz(mod.ref, name, sizeof(mod.ref));

	jsonNode = cJSON_GetObjectItem(json, "hitmarkerExplosives");
	mod.hitmarkerExplosives = cJSON_ToBooleanOpt(jsonNode, qfalse);

	jsonNode = cJSON_GetObjectItem(json, "inventoryName");
	Q_strncpyz(mod.inventoryName, cJSON_ToStringOpt(jsonNode, ""), MAX_QPATH);

	jsonNode = cJSON_GetObjectItem(json, "disintegrate");
	mod.disintegrate = cJSON_ToBooleanOpt(jsonNode, qfalse);

	jsonNode = cJSON_GetObjectItem(json, "modifiers");
	if (jsonNode) {
		child = cJSON_GetObjectItem(jsonNode, "ignoreArmor");
		mod.modifiers.ignoreArmor = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "ignoreShield");
		mod.modifiers.ignoreShield = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "shieldBlocks");
		mod.modifiers.shieldBlocks = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "dodgeable");
		mod.modifiers.dodgeable = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "isEMP");
		mod.modifiers.isEMP = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "armor");
		mod.modifiers.armor = cJSON_ToNumberOpt(child, 1.0);

		child = cJSON_GetObjectItem(jsonNode, "shield");
		mod.modifiers.shield = cJSON_ToNumberOpt(child, 1.0);

		child = cJSON_GetObjectItem(jsonNode, "organic");
		mod.modifiers.organic = cJSON_ToNumberOpt(child, 1.0);

		child = cJSON_GetObjectItem(jsonNode, "droid");
		mod.modifiers.droid = cJSON_ToNumberOpt(child, 1.0);
	}
	else {
		// defaults
		mod.modifiers.armor = 1.0f;
		mod.modifiers.droid = 1.0f;
		mod.modifiers.organic = 1.0f;
		mod.modifiers.shield = 1.0f;
	}

	jsonNode = cJSON_GetObjectItem(json, "dismemberment");
	if (jsonNode) {
		child = cJSON_GetObjectItem(json, "canDismember");
		mod.dismemberment.canDismember = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(json, "blowChunks");
		mod.dismemberment.blowChunks = cJSON_ToBooleanOpt(child, qfalse);
	}

	jsonNode = cJSON_GetObjectItem(json, "killfeed");
	if (jsonNode) {
		child = cJSON_GetObjectItem(jsonNode, "killMessage");
		Q_strncpyz(mod.killfeed.killMessage, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);

		child = cJSON_GetObjectItem(jsonNode, "deathMessage");
		Q_strncpyz(mod.killfeed.deathMessage, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);

		child = cJSON_GetObjectItem(jsonNode, "suicideMessageMale");
		if (child) {
			// we are using gendered suicide messages and not generic ones
			mod.killfeed.genderedStringsPresent = qtrue;

			Q_strncpyz(mod.killfeed.suicideMessage.gendered.male, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);

			child = cJSON_GetObjectItem(jsonNode, "suicideMessageFemale");
			Q_strncpyz(mod.killfeed.suicideMessage.gendered.female, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);

			child = cJSON_GetObjectItem(jsonNode, "suicideMessageNeuter");
			Q_strncpyz(mod.killfeed.suicideMessage.gendered.neuter, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);
		}
		else {
			child = cJSON_GetObjectItem(jsonNode, "suicideMessage");
			Q_strncpyz(mod.killfeed.suicideMessage.message, cJSON_ToStringOpt(child, ""), MAX_OBIT_LENGTH);
		}
	}

	jsonNode = cJSON_GetObjectItem(json, "plums");
	if (jsonNode) {
		child = cJSON_GetObjectItem(jsonNode, "hide");
		mod.plums.noDamagePlums = cJSON_ToBooleanOpt(child, 0);

		child = cJSON_GetObjectItem(jsonNode, "damageColor");
		if (child) {
			if (JKG_ParseJSONColor(child, mod.plums.overrideDamagePlumColor)) {
				mod.plums.overrideDamagePlum = qtrue;
			}
		}

		child = cJSON_GetObjectItem(jsonNode, "lowDamageColor");
		if (child) {
			if (JKG_ParseJSONColor(child, mod.plums.overrideLowDamagePlumColor)) {
				mod.plums.overrideLowDamagePlum = qtrue;
			}
		}

		child = cJSON_GetObjectItem(jsonNode, "shieldDamageColor");
		if (child) {
			if (JKG_ParseJSONColor(child, mod.plums.overrideShieldDamagePlumColor)) {
				mod.plums.overrideShieldDamagePlum = qtrue;
			}
		}
	}

	allMeansOfDamage.push_back(mod);
}

/*
=========================
JKG_ParseAllMeansOfDamage

=========================
*/
static qboolean JKG_ParseAllMeansOfDamage(const char* fileName, char* buffer) {
	char error[MAX_MEANSOFDAMAGE_ERROR] = { 0 };
	cJSON* json = cJSON_ParsePooled(buffer, error, MAX_MEANSOFDAMAGE_ERROR);
	cJSON* it;

	if (!json) {
		trap->Print("Couldn't parse %s - %s\n", fileName, error);
		return qfalse;
	}

	for (it = cJSON_GetFirstItem(json); it; it = cJSON_GetNextItem(it)) {
		const char* name = cJSON_GetItemKey(it);
		JKG_ParseSingleMeansOfDamage(name, it);
	}

	return qtrue;
}

/*
=========================
JKG_LoadMeansOfDamage

=========================
*/
#define MEANSOFDAMAGE_FILE	"ext_data/tables/meansOfDamage.json"
void JKG_LoadMeansOfDamage() {
	fileHandle_t f;
	int fileLen = trap->FS_Open(MEANSOFDAMAGE_FILE, &f, FS_READ);
	char buffer[MAX_MEANSOFDAMAGE_BUFFER] = { 0 };

	if (!f || fileLen == -1 || f == -1) {
		trap->Print("Could not open %s for reading\n", MEANSOFDAMAGE_FILE);
		return;
	}
	
	if (fileLen <= 0) {
		trap->FS_Close(f);
		trap->Print("%s is blank\n", MEANSOFDAMAGE_FILE);
		return;
	}

	if (fileLen >= MAX_MEANSOFDAMAGE_BUFFER) {
		trap->FS_Close(f);
		trap->Print("%s is too big [%i >= %i (%i/%i KB)]\n", MEANSOFDAMAGE_FILE, fileLen, MAX_MEANSOFDAMAGE_BUFFER, fileLen / 1024, MAX_MEANSOFDAMAGE_BUFFER/1024);
		return;
	}

	trap->FS_Read(buffer, fileLen, f);
	buffer[fileLen] = '\0';
	trap->FS_Close(f);

	if (!JKG_ParseAllMeansOfDamage(MEANSOFDAMAGE_FILE, buffer)) {
		trap->Print("Couldn't parse %s\n", MEANSOFDAMAGE_FILE);
	}
}

/*
=========================
JKG_GetMeansOfDamage

=========================
*/

/*### Overload 1 ###*/
meansOfDamage_t* JKG_GetMeansOfDamage(const std::string& ref) {
	for (int i = 0; i < allMeansOfDamage.size(); i++)
	{
		auto it = allMeansOfDamage.at(i);
		if (!Q_stricmp(it.ref, ref.c_str()))
		{
			return &(allMeansOfDamage[i]);
		}
	}
	return nullptr;
}

/*### Overload 2 ###*/
meansOfDamage_t* JKG_GetMeansOfDamage(int num) {
	if (num < 0 || num >= allMeansOfDamage.size()) {
		return nullptr;
	}

	return &(allMeansOfDamage[num]);
}

/*
=========================
JKG_GetMeansOfDamageIndex

=========================
*/
int JKG_GetMeansOfDamageIndex(const std::string& ref) {
	for (int i = 0; i < allMeansOfDamage.size(); i++) {
		if (!Q_stricmp(allMeansOfDamage[i].ref, ref.c_str())) {
			return i;
		}
	}
	return MOD_UNKNOWN;
}