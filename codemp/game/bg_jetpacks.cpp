#include "bg_jetpacks.h"
#include <json/cJSON.h>

#if defined(_GAME)
	#include "g_local.h"
#elif defined(_CGAME)
	#include "../cgame/cg_local.h"
#endif

#define JETPACK_NAMEBUFFER	8192
#define JETPACK_FILEBUFFER	4096
#define JETPACK_ERRBUFFER	1024

jetpackData_t jetpackTable[MAX_JETPACKS];
int numLoadedJetpacks;

/*
============================
JKG_ParseJetpackMovement

Parses the "movement" block of a jetpack.
============================
*/
static void JKG_ParseJetpackMovement(cJSON* jsonNode, jetpackData_t& jetpackData) {
	cJSON* child;

	child = cJSON_GetObjectItem(jsonNode, "thrustAllowed");
	jetpackData.move.thrustAllowed = cJSON_ToBooleanOpt(child, true);

	child = cJSON_GetObjectItem(jsonNode, "fwdThrustAllowed");
	jetpackData.move.fwdThrustAllowed = cJSON_ToBooleanOpt(child, true);

	child = cJSON_GetObjectItem(jsonNode, "loadBearingAllowed");
	jetpackData.move.loadBearingAllowed = cJSON_ToBooleanOpt(child, false);
	
	child = cJSON_GetObjectItem(jsonNode, "hoverGravity");
	jetpackData.move.hoverGravity = cJSON_ToNumberOpt(child, -1.0);

	child = cJSON_GetObjectItem(jsonNode, "forwardMove");
	jetpackData.move.forwardMove = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "backMove");
	jetpackData.move.backMove = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "sideMove");
	jetpackData.move.sideMove = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "downMove");
	jetpackData.move.downMove = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "thrustFwd");
	jetpackData.move.thrustFwd = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "thrustBack");
	jetpackData.move.thrustBack = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "thrustSide");
	jetpackData.move.thrustSide = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "thrustUp");
	jetpackData.move.thrustUp = cJSON_ToNumberOpt(child, 1.0);

	child = cJSON_GetObjectItem(jsonNode, "fwdThrustAmt");
	jetpackData.move.fwdThrustAmt = cJSON_ToNumberOpt(child, 2.0);
}

/*
============================
JKG_ParseJetpackVisuals

Parses the visual elements of a jetpack.
FIXME: Should precache?
============================
*/
static void JKG_ParseJetpackVisuals(cJSON* jsonNode, jetpackData_t& jetpackData) {
	cJSON* child;
	int boltSize;

	child = cJSON_GetObjectItem(jsonNode, "modelName");
	Q_strncpyz(jetpackData.visuals.modelName, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "effectBolts");
	boltSize = cJSON_GetArraySize(child);
	for (int i = 0; i < boltSize; i++) {
		cJSON* arrayObj = cJSON_GetArrayItem(child, i);
		std::string bolt = cJSON_ToString(arrayObj);
		jetpackData.visuals.effectBolts.push_back(bolt);
	}

	child = cJSON_GetObjectItem(jsonNode, "hoverEffect");
	Q_strncpyz(jetpackData.visuals.hoverEffect, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "thrustEffect");
	Q_strncpyz(jetpackData.visuals.thrustEffect, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "jetEffect");
	Q_strncpyz(jetpackData.visuals.jetEffect, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "idleSound");
	Q_strncpyz(jetpackData.visuals.idleSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "thrustSound");
	Q_strncpyz(jetpackData.visuals.thrustSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "activateSound");
	Q_strncpyz(jetpackData.visuals.activateSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "deactivateSound");
	Q_strncpyz(jetpackData.visuals.deactivateSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

	child = cJSON_GetObjectItem(jsonNode, "sputterSound");
	Q_strncpyz(jetpackData.visuals.sputterSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);
}

/*
============================
JKG_ParseJetpack

Parses a jetpack file, from memory.
============================
*/
static qboolean JKG_ParseJetpack(char* buffer, const char* fileName, jetpackData_t& jetpackData) {
	char errorBuffer[JETPACK_ERRBUFFER] {0};
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
	Q_strncpyz(jetpackData.ref, cJSON_ToString(jsonNode), sizeof(jetpackData.ref));

	jsonNode = cJSON_GetObjectItem(json, "fuelCapacity");
	jetpackData.fuelCapacity = cJSON_ToIntegerOpt(jsonNode, 100);

	jsonNode = cJSON_GetObjectItem(json, "fuelConsumption");
	jetpackData.fuelConsumption = cJSON_ToNumberOpt(jsonNode, 1.0);

	jsonNode = cJSON_GetObjectItem(json, "thrustConsumption");
	jetpackData.thrustConsumption = cJSON_ToNumberOpt(jsonNode, 2.0);

	jsonNode = cJSON_GetObjectItem(json, "fuelRegeneration");
	jetpackData.fuelRegeneration = cJSON_ToNumberOpt(jsonNode, 1.0);

	// Movement related fields
	jsonNode = cJSON_GetObjectItem(json, "movement");
	JKG_ParseJetpackMovement(jsonNode, jetpackData);

	// Visuals
	jsonNode = cJSON_GetObjectItem(json, "visuals");
	JKG_ParseJetpackVisuals(jsonNode, jetpackData);

	cJSON_Delete(json);
	return qtrue;
}

/*
============================
JKG_LoadJetpack

Loads an individual jetpack (.jet) file.
Called on both the client and the server.
============================
*/
static qboolean JKG_LoadJetpack(const char* fileName, const char* dir, jetpackData_t& jetpackData) {
	int fileLen;
	fileHandle_t f;
	char fileBuffer[JETPACK_FILEBUFFER];

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

	if ((fileLen + 1) >= JETPACK_FILEBUFFER) {
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s is too big - max file size %d bytes (%.2f kb)\n", fileName, JETPACK_FILEBUFFER, JETPACK_FILEBUFFER / 1024.0f);
		return qfalse;
	}

	trap->FS_Read(&fileBuffer, fileLen, f);
	fileBuffer[fileLen] = '\0';
	trap->FS_Close(f);

	return JKG_ParseJetpack(fileBuffer, fileName, jetpackData);
}

/*
============================
JKG_LoadJetpacks

Loads all of the jetpack items.
Called on both the client and the server.
============================
*/
void JKG_LoadJetpacks() {
	char jetpackFiles[JETPACK_NAMEBUFFER] {0}; 
	const char* jetpack;
	const char* jetpackDir = "ext_data/jetpacks/";
	int failed = 0;
	int numFiles;

	numFiles = Q_FSGetFileListSorted(jetpackDir, ".jet", jetpackFiles, sizeof(jetpackFiles));
	jetpack = jetpackFiles;

	Com_Printf("------- Jetpacks -------\n");

	for (int i = 0; i < numFiles; i++) {
		if (!JKG_LoadJetpack(jetpack, jetpackDir, jetpackTable[numLoadedJetpacks])) {
			failed++;
			jetpack += strlen(jetpack) + 1;
			continue;
		}

		numLoadedJetpacks++;
		jetpack += strlen(jetpack) + 1;
	}

	Com_Printf("Jetpacks: %d successful, %d failed.\n", numLoadedJetpacks, failed);
	Com_Printf("-------------------------------------\n");
}

/*
============================
JKG_FindJetpackByName

Should be used sparingly.
============================
*/
jetpackData_t* JKG_FindJetpackByName(const char* jetpackName) {
	for (int i = 0; i < numLoadedJetpacks; i++) {
		jetpackData_t* jetData = &jetpackTable[i];
		if (!Q_stricmp(jetData->ref, jetpackName)) {
			return jetData;
		}
	}
	return nullptr;
}
