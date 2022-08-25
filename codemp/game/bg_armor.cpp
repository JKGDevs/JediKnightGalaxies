#include "bg_armor.h"
#include <json/cJSON.h>

#if defined(_GAME)
#include "g_local.h"
#elif defined(_CGAME)
#include "../cgame/cg_local.h"
#endif

#define ARMOR_NAMEBUFFER	8192
#define ARMOR_FILEBUFFER	4096
#define ARMOR_ERRBUFFER		1024

armorData_t armorTable[MAX_ARMOR_ITEMS];
int numLoadedArmor;

static armorG2Reference_t g2Table[MAX_ARMOR_G2];
static int numUsedG2;

#ifndef _GAME
// FIXME: move into bg_g2_utils
int JKG_G2_GetNumberOfSurfaces_(const char *modelPath)
{
	mdxmHeader_t header;
	fileHandle_t f;
	int fileLen = trap->FS_Open(modelPath, &f, FS_READ);
	if (fileLen == -1 || !f)
	{
#ifdef _DEBUG
		trap->Print("Failed to open the model %s.\n", modelPath);
#endif
		return 0;
	}

	if (fileLen < sizeof(mdxmHeader_t))
	{
#ifdef _DEBUG
		trap->Print("Invalid model file %s.\n", modelPath);
#endif
		return 0;
	}

	trap->FS_Read(&header, sizeof(mdxmHeader_t), f);
	trap->FS_Close(f);

	return header.numSurfaces;
}
#endif
/*
=====================
JKG_RegisterArmorGhoul2

=====================
*/
#ifndef _GAME
extern int CG_HandleAppendedSkin(char *modelName);
#endif
armorG2Reference_t* JKG_RegisterArmorGhoul2(const char* ref, char* model) {
	// Check and see if we've already registered a GHOUL2 instance with this refname
	for (int i = 0; i < numUsedG2; i++) {
		if (!Q_stricmp(g2Table[i].ref, ref)) {
			return &g2Table[i];
		}
	}

	if (numUsedG2 >= MAX_ARMOR_G2) {
		Com_Printf(S_COLOR_RED "Too many armor GHOUL2 instances. Ask the programmers for more space!\n");
		return nullptr; // too many
	}

	// If not, try and register it
	// TODO: skins
	Q_strncpyz(g2Table[numUsedG2].ref, ref, MAX_QPATH);
#ifdef _GAME
	// Strip out the stuff after the *
	char* find = Q_strrchr(model, '*');
	if (find != nullptr)
		*find = 0; //Q_strncpyz(model, model, find - model); instead, just set to null to end the string.
#else
	g2Table[numUsedG2].skin = CG_HandleAppendedSkin(model);
#endif
	trap->G2API_InitGhoul2Model(&g2Table[numUsedG2].instance, model, 0, 0, 0, 0, 0);
#ifndef _GAME
	trap->G2API_SetSkin(g2Table[numUsedG2].instance, 0, g2Table[numUsedG2].skin, g2Table[numUsedG2].skin);
	g2Table[numUsedG2].numSurfaces = JKG_G2_GetNumberOfSurfaces_(model);
#endif
	return &g2Table[numUsedG2++];
}

/*
=====================
JKG_ParseArmorVisuals

=====================
*/
void JKG_ParseArmorVisuals(cJSON* json, armorData_t& armor) {
	char ghoul2Ref[MAX_QPATH] {0};
	char ghoul2Model[MAX_QPATH]{0};
	std::string ghoul2Surface;
	cJSON* jsonNode;
	cJSON* child;
	int count;

	// FIXME: is this really needed on the server?
	jsonNode = cJSON_GetObjectItem(json, "model");
	Q_strncpyz(ghoul2Model, cJSON_ToStringOpt(jsonNode, ""), MAX_QPATH);
	jsonNode = cJSON_GetObjectItem(json, "modelGroup");
	Q_strncpyz(ghoul2Ref, cJSON_ToStringOpt(jsonNode, ""), MAX_QPATH);
	if (ghoul2Ref[0]) {
		armor.visuals.pGHOUL2 = JKG_RegisterArmorGhoul2(ghoul2Ref, ghoul2Model);
	}

	jsonNode = cJSON_GetObjectItem(json, "motionBone");
	Q_strncpyz(armor.visuals.motionBone, cJSON_ToStringOpt(jsonNode, ""), MAX_QPATH);

	// Handle surfaces to turn on/off.
	// These are handled as an array in the JSON file
	jsonNode = cJSON_GetObjectItem(json, "armorOnSurfaces");
	if (jsonNode) {
		count = cJSON_GetArraySize(jsonNode);
		for (int i = 0; i < count; i++) {
			child = cJSON_GetArrayItem(jsonNode, i);
			ghoul2Surface = cJSON_ToStringOpt(child, "");
			armor.visuals.armorOnSurfs.push_back(ghoul2Surface);
		}
	}

	jsonNode = cJSON_GetObjectItem(json, "bodyOffSurfaces");
	if (jsonNode) {
		count = cJSON_GetArraySize(jsonNode);
		for (int i = 0; i < count; i++) {
			child = cJSON_GetArrayItem(jsonNode, i);
			ghoul2Surface = cJSON_ToStringOpt(child, "");
			armor.visuals.bodyOffSurfs.push_back(ghoul2Surface);
		}
	}

	// The sound for equipping the piece
	jsonNode = cJSON_GetObjectItem(json, "equippedSound");
	Q_strncpyz(armor.visuals.equipSound, cJSON_ToStringOpt(jsonNode, ""), MAX_QPATH);
}

/*
=====================
JKG_ArmorSlotFromText

=====================
*/
int JKG_ArmorSlotFromText(const char* text) {
	if (!Q_stricmp("head", text)) {
		return ARMSLOT_HEAD;
	}
	else if (!Q_stricmp("neck", text)) {
		return ARMSLOT_NECK;
	}
	else if (!Q_stricmp("torso", text)) {
		return ARMSLOT_TORSO;
	}
	else if (!Q_stricmp("robe", text)) {
		return ARMSLOT_ROBE;
	}
	else if (!Q_stricmp("legs", text)) {
		return ARMSLOT_LEGS;
	}
	else if (!Q_stricmp("gloves", text)) {
		return ARMSLOT_GLOVES;
	}
	else if (!Q_stricmp("boots", text)) {
		return ARMSLOT_BOOTS;
	}
	else if (!Q_stricmp("shoulder", text)) {
		return ARMSLOT_SHOULDER;
	}
	else if (!Q_stricmp("implant", text)) {
		return ARMSLOT_IMPLANTS;
	}
	else {
		return ARMSLOT_MAX;
	}
}

/*
=====================
JKG_ParseArmorFile

=====================
*/
static qboolean JKG_ParseArmorFile(char* buffer, const char* fileName, armorData_t& armorData) {
	char errorBuffer[ARMOR_ERRBUFFER] {0};
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
	Q_strncpyz(armorData.ref, cJSON_ToString(jsonNode), sizeof(armorData.ref));

	jsonNode = cJSON_GetObjectItem(json, "slot");
	armorData.slot = JKG_ArmorSlotFromText(cJSON_ToStringOpt(jsonNode, ""));

	jsonNode = cJSON_GetObjectItem(json, "armor");
	armorData.armor = cJSON_ToIntegerOpt(jsonNode, 0);

	jsonNode = cJSON_GetObjectItem(json, "hp");
	armorData.hp = cJSON_ToIntegerOpt(jsonNode, 0);

	jsonNode = cJSON_GetObjectItem(json, "stamina");
	armorData.stamina = cJSON_ToIntegerOpt(jsonNode, 0);

	jsonNode = cJSON_GetObjectItem(json, "filter");
	armorData.filter = cJSON_ToBooleanOpt(jsonNode, false);

	jsonNode = cJSON_GetObjectItem(json, "movemodifier");
	armorData.movemodifier = cJSON_ToNumberOpt(jsonNode, 1.0);

	jsonNode = cJSON_GetObjectItem(json, "visuals");
	JKG_ParseArmorVisuals(jsonNode, armorData);

	cJSON_Delete(json);
	return qtrue;
}

/*
=====================
JKG_LoadArmorFile


=====================
*/
static qboolean JKG_LoadArmorFile(const char* fileName, const char* dir, armorData_t& armorData) {
	int fileLen;
	fileHandle_t f;
	char fileBuffer[ARMOR_FILEBUFFER];

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

	if ((fileLen + 1) >= ARMOR_FILEBUFFER) {
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s is too big - max file size %d bytes (%.2f kb)\n", fileName, ARMOR_FILEBUFFER, ARMOR_FILEBUFFER / 1024.0f);
		return qfalse;
	}

	trap->FS_Read(&fileBuffer, fileLen, f);
	fileBuffer[fileLen] = '\0';
	trap->FS_Close(f);

	return JKG_ParseArmorFile(fileBuffer, fileName, armorData);
}

/*
=====================
JKG_LoadArmor

=====================
*/
void JKG_LoadArmor() {
	char armorFiles[ARMOR_NAMEBUFFER] {0};
	const char* armor;
	const char* armorDir = "ext_data/armor/";
	int failed = 0;
	int numFiles;

	numFiles = Q_FSGetFileListSorted(armorDir, ".arm", armorFiles, sizeof(armorFiles));
	armor = armorFiles;

	Com_Printf("------- Armor -------\n");

	for (int i = 0; i < numFiles; i++) {
		if (!JKG_LoadArmorFile(armor, armorDir, armorTable[numLoadedArmor])) {
			failed++;
			armor += strlen(armor) + 1;
			continue;
		}

		numLoadedArmor++;
		armor += strlen(armor) + 1;
	}

	Com_Printf("Armor: %d successful, %d failed.\n", numLoadedArmor, failed);
	Com_Printf("-------------------------------------\n");
}

/*
=====================
JKG_UnloadArmor

=====================
*/
void JKG_UnloadArmor() {
	for (int i = 0; i < numUsedG2; i++) {
		armorG2Reference_t* g2 = &g2Table[i];
		if (g2->instance && trap->G2_HaveWeGhoul2Models(g2->instance)) {
			trap->G2API_CleanGhoul2Models(&g2->instance);
			g2->instance = nullptr;
		}
	}
}

/*
=====================
JKG_FindArmorByName

=====================
*/
armorData_t* JKG_FindArmorByName(const char* ref) {
	for (int i = 0; i < numLoadedArmor && i < MAX_ARMOR_ITEMS; i++) {
		if (!Q_stricmp(armorTable[i].ref, ref)) {
			return &armorTable[i];
		}
	}
	return nullptr;
}

