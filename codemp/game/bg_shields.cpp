#include "bg_shields.h"
#include <json/cJSON.h>

#if defined(_GAME)
	#include "g_local.h"
#elif defined(_CGAME)
	#include "../cgame/cg_local.h"
#endif

#define SHIELD_NAMEBUFFER	8192
#define SHIELD_FILEBUFFER	4096
#define SHIELD_ERRBUFFER	1024

shieldData_t shieldTable[MAX_SHIELDS];
int numLoadedShields;

/*
===========================
JKG_ParseShieldBarrierTypes

===========================
*/
qboolean JKG_ParseShieldBarrierTypes(cJSON* json, shieldData_t &shieldData, std::vector<int>& list)
{
	qboolean status = qtrue; //if no errors encountered
	if (json)
	{
		int arraySize = cJSON_GetArraySize(json);
		for (int i = 0; i < arraySize; i++)
		{
			int mod; cJSON* jsonNode;
			jsonNode = cJSON_GetArrayItem(json, i);
			std::string s = cJSON_ToString(jsonNode);

			//skip empty string
			if (s == "")
			{
				Com_Printf(S_COLOR_YELLOW "Empty string in shield barrier type: %s\n", cJSON_ToString(jsonNode));
				continue;
			}

			mod = JKG_GetMeansOfDamageIndex(s);
			if (mod == MOD_UNKNOWN)
			{
				Com_Printf(S_COLOR_ORANGE "Unrecognized means of damage specified for shield barrier type: %s\n", cJSON_ToString(jsonNode));
				status = qfalse;
				continue;
			}

			if (list.data() == shieldData.blockedMODs.data())
				shieldData.blockedMODs.push_back(mod);
			else if (list.data() == shieldData.allowedMODs.data())
				shieldData.allowedMODs.push_back(mod);
			else
			{
				Com_Printf(S_COLOR_RED "Passed an illegal vector to JKG_ParseShieldBarrierTypes() in %s\n", cJSON_ToString(jsonNode));
				status = qfalse;
				break;
			}
		}
	}

	return status;
}

/*
============================
JKG_ParseShieldData

Parses data of shield.
============================
*/
static qboolean JKG_ParseShieldData(char* buffer, const char* fileName, shieldData_t& shieldData) {
	char errorBuffer[SHIELD_ERRBUFFER]{ 0 };
	cJSON* json;
	cJSON* jsonNode;

	json = cJSON_ParsePooled(buffer, errorBuffer, sizeof(errorBuffer));
	if (json == nullptr) {
		Com_Printf(S_COLOR_RED "%s: %s\n", fileName, errorBuffer);
		return qfalse;
	}

	// Basic information
	jsonNode = cJSON_GetObjectItem(json, "ref");
	if (!jsonNode) {
		Com_Printf(S_COLOR_RED "%s doesn't contain a valid ref name!\n", fileName);
		cJSON_Delete(json);
		return qfalse;
	}
	Q_strncpyz(shieldData.ref, cJSON_ToString(jsonNode), sizeof(shieldData.ref));

	jsonNode = cJSON_GetObjectItem(json, "capacity");
	shieldData.capacity = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_CAPACITY);

	jsonNode = cJSON_GetObjectItem(json, "cooldown");
	shieldData.cooldown = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_COOLDOWN);

	jsonNode = cJSON_GetObjectItem(json, "regenrate");
	shieldData.regenrate = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_REGEN);

	jsonNode = cJSON_GetObjectItem(json, "rechargeSoundEffect");
	if (jsonNode) {
		Q_strncpyz(shieldData.rechargeSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
	}

	jsonNode = cJSON_GetObjectItem(json, "brokenSoundEffect");
	if (jsonNode) {
		Q_strncpyz(shieldData.brokenSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
	}

	jsonNode = cJSON_GetObjectItem(json, "equippedSoundEffect");
	if (jsonNode) {
		Q_strncpyz(shieldData.equippedSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
	}

	jsonNode = cJSON_GetObjectItem(json, "chargedSoundEffect");
	if (jsonNode) {
		Q_strncpyz(shieldData.chargedSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
	}

	jsonNode = cJSON_GetObjectItem(json, "malfunctionSoundEffect");
	if (jsonNode) {
		Q_strncpyz(shieldData.malfunctionSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
	}

	jsonNode = cJSON_GetObjectItem(json, "blockedMODs");
	if (jsonNode) {
		if (!JKG_ParseShieldBarrierTypes(jsonNode, shieldData, shieldData.blockedMODs))
			Com_Printf(S_COLOR_ORANGE "%s has invalid blockedMODs data that has been ignored.\n", fileName);
	}

	jsonNode = cJSON_GetObjectItem(json, "allowedMODs");
	if (jsonNode) {
		if (!JKG_ParseShieldBarrierTypes(jsonNode, shieldData, shieldData.allowedMODs))
			Com_Printf(S_COLOR_ORANGE "%s has invalid allowedMODs data that has been ignored.\n", fileName);
	}

	cJSON_Delete(json);
	return qtrue;
}

/*
============================
JKG_LoadShield

Loads an individual shield (.shd) file.
Called on both the client and the server.
============================
*/
static qboolean JKG_LoadShield(const char* fileName, const char* dir, shieldData_t& shieldData) {
	int fileLen;
	fileHandle_t f;
	char fileBuffer[SHIELD_FILEBUFFER];

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

	if ((fileLen + 1) >= SHIELD_FILEBUFFER) {
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s is too big - max file size %d bytes (%.2f kb)\n", fileName, SHIELD_FILEBUFFER, SHIELD_FILEBUFFER / 1024.0f);
		return qfalse;
	}

	trap->FS_Read(&fileBuffer, fileLen, f);
	fileBuffer[fileLen] = '\0';
	trap->FS_Close(f);

	return JKG_ParseShieldData(fileBuffer, fileName, shieldData);
}

/*
============================
JKG_LoadShields

Loads all of the shield items.
Called on both the client and the server.
============================
*/
void JKG_LoadShields() {
	char shieldFiles[SHIELD_NAMEBUFFER]{ 0 };
	const char* shield;
	const char* shieldDir = "ext_data/shields/";
	int failed = 0;
	int numFiles;

	numFiles = Q_FSGetFileListSorted(shieldDir, ".shd", shieldFiles, sizeof(shieldFiles));
	shield = shieldFiles;

	Com_Printf("------- Shields -------\n");

	for (int i = 0; i < numFiles; i++) {
		if (!JKG_LoadShield(shield, shieldDir, shieldTable[numLoadedShields])) {
			failed++;
			shield += strlen(shield) + 1;
			continue;
		}

		numLoadedShields++;
		shield += strlen(shield) + 1;
	}

	Com_Printf("Shields: %d successful, %d failed.\n", numLoadedShields, failed);
	Com_Printf("-------------------------------------\n");
}

/*
============================
JKG_FindShieldByName

Should be used sparingly.
============================
*/
shieldData_t* JKG_FindShieldByName(const char* shieldName) {
	for (int i = 0; i < numLoadedShields; i++) {
		shieldData_t* shieldData = &shieldTable[i];
		if (!Q_stricmp(shieldData->ref, shieldName)) {
			return shieldData;
		}
	}
	return nullptr;
}
