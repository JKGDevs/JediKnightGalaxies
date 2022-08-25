#include "bg_public.h"
#include <json/cJSON.h>

#if defined(_GAME)
	#include "g_local.h"
#elif defined(_CGAME)
	#include "cgame/cg_local.h"
#elif defined(IN_UI)
	#include "ui/ui_local.h"
#endif

bgConstants_t bgConstants;

#define MAX_CONSTANTS_FILE_SIZE (16834) // 16kb

static void DefineBaselineConstants(void)
{
	bgConstants.baseForceJumpLevel = 0;
	bgConstants.baseJumpHeight = 32;
	bgConstants.baseJumpTapHeight = 32;
	bgConstants.baseJumpVelocity = 225;
	bgConstants.baseJumpTapVelocity = 225;
	//Stoiss add, missing code from saber merge 
	bgConstants.walkingSpeed = 64.0f;
	bgConstants.ironsightsMoveSpeed = 64.0f;
	bgConstants.blockingModeMoveSpeed = 64.0f;

	bgConstants.backwardsSpeedModifier = -0.45f;
	bgConstants.strafeSpeedModifier = -0.25f;
	bgConstants.backwardsDiagonalSpeedModifier = 0.21;
	bgConstants.baseSpeedModifier = -0.1f;
	bgConstants.walkSpeedModifier = -0.2f;
	bgConstants.minimumSpeedModifier = 0.5f;
	bgConstants.sprintSpeedModifier = 1.3f;
	//Stoiss end

	bgConstants.consumableTime = 3000;

	bgConstants.staminaDrains.lossFromJumping = 10;
	bgConstants.staminaDrains.lossFromKicking = 5;
	bgConstants.staminaDrains.lossFromPunching = 5;
	bgConstants.staminaDrains.lossFromRolling = 10;
	bgConstants.staminaDrains.minJumpThreshold = 20;
	bgConstants.staminaDrains.minKickThreshold = 0;
	bgConstants.staminaDrains.minPunchThreshold = 0;
	bgConstants.staminaDrains.minRollThreshold = 20;
	bgConstants.staminaDrains.minSprintThreshold = 25;

	bgConstants.damageModifiers.headModifier = 1.3f;
	bgConstants.damageModifiers.torsoModifier = 1.0f;
	bgConstants.damageModifiers.armModifier = 0.85f;
	bgConstants.damageModifiers.handModifier = 0.6f;
	bgConstants.damageModifiers.legModifier = 0.7f;
	bgConstants.damageModifiers.footModifier = 0.5f;
}

static void ParseConstantsFile ( const char *fileText )
{
	//int i = 0;
    cJSON *json = NULL;
    char jsonError[MAX_STRING_CHARS] = { 0 };
	cJSON *jsonNode;
    //const char *string = NULL;

    json = cJSON_ParsePooled (fileText, jsonError, sizeof (jsonError));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "Error: %s\n", jsonError);
    }
    else
    {
		jsonNode = cJSON_GetObjectItem (json, "baseJumpHeight");
		bgConstants.baseJumpHeight = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseForceJumpLevel");
		bgConstants.baseForceJumpLevel = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpVelocity");
		bgConstants.baseJumpVelocity = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpTapHeight");
		bgConstants.baseJumpTapHeight = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpTapVelocity");
		bgConstants.baseJumpTapVelocity = cJSON_ToNumber(jsonNode);
		//Stoiss add missing merge code from saber code
		jsonNode = cJSON_GetObjectItem (json, "walkingSpeed");
		bgConstants.walkingSpeed = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "ironsightsMoveSpeed");
		bgConstants.ironsightsMoveSpeed = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "blockingModeMoveSpeed");
		bgConstants.blockingModeMoveSpeed = cJSON_ToNumber(jsonNode);


		jsonNode = cJSON_GetObjectItem (json, "backwardsSpeedModifier");
		bgConstants.backwardsSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "strafeSpeedModifier");
		bgConstants.strafeSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "backwardsDiagonalSpeedModifier");
		bgConstants.backwardsDiagonalSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseSpeedModifier");
		bgConstants.baseSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "walkSpeedModifier");
		bgConstants.walkSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "minimumSpeedModifier");
		bgConstants.minimumSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "sprintSpeedModifier");
		bgConstants.sprintSpeedModifier = cJSON_ToNumber(jsonNode);
		//Stoiss end

		jsonNode = cJSON_GetObjectItem(json, "consumableTime");
		bgConstants.consumableTime = cJSON_ToIntegerOpt(jsonNode, 3000);

		jsonNode = cJSON_GetObjectItem(json, "stamina");
		if (jsonNode) {
			cJSON* childNode = cJSON_GetObjectItem(jsonNode, "lossFromRolling");
			bgConstants.staminaDrains.lossFromRolling = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "lossFromPunching");
			bgConstants.staminaDrains.lossFromPunching = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "lossFromJumping");
			bgConstants.staminaDrains.lossFromJumping = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "lossFromKicking");
			bgConstants.staminaDrains.lossFromKicking = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "minSprintThreshold");
			bgConstants.staminaDrains.minSprintThreshold = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "minRollThreshold");
			bgConstants.staminaDrains.minRollThreshold = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "minJumpThreshold");
			bgConstants.staminaDrains.minJumpThreshold = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "minPunchThreshold");
			bgConstants.staminaDrains.minPunchThreshold = cJSON_ToInteger(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "minKickThreshold");
			bgConstants.staminaDrains.minKickThreshold = cJSON_ToInteger(childNode);
		}

		jsonNode = cJSON_GetObjectItem(json, "damage");
		if (jsonNode)
		{
			cJSON* childNode = cJSON_GetObjectItem(jsonNode, "headModifier");
			bgConstants.damageModifiers.headModifier = cJSON_ToNumber(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "torsoModifier");
			bgConstants.damageModifiers.torsoModifier = cJSON_ToNumber(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "armModifier");
			bgConstants.damageModifiers.armModifier = cJSON_ToNumber(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "legModifier");
			bgConstants.damageModifiers.legModifier = cJSON_ToNumber(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "handModifier");
			bgConstants.damageModifiers.handModifier = cJSON_ToNumber(childNode);

			childNode = cJSON_GetObjectItem(jsonNode, "footModifier");
			bgConstants.damageModifiers.footModifier = cJSON_ToNumber(childNode);
		}
    }
    
    cJSON_Delete (json);
}

qboolean ReadConstantsFile(void)
{
	fileHandle_t f;
    char buffer[MAX_CONSTANTS_FILE_SIZE + 1];
    int fileLength;
    
    fileLength = trap->FS_Open("ext_data/tables/constants.json", &f, FS_READ);
    if ( fileLength == -1 || !f )
    {
        Com_Printf (S_COLOR_RED "Error: Failed to read the constants.json file. File is unreadable or does not exist.\n");
        return qfalse;
    }
    
    if ( fileLength == 0 )
    {
        Com_Printf (S_COLOR_RED "Error: constants.json file is empty.\n");
        trap->FS_Close (f);
        return qfalse;
    }
    
    if ( fileLength > MAX_CONSTANTS_FILE_SIZE )
    {
        Com_Printf (S_COLOR_RED "Error: constants.json file is too large (max file size is %d bytes)\n", MAX_CONSTANTS_FILE_SIZE);
        trap->FS_Close (f);
        return qfalse;
    }
    
    trap->FS_Read (buffer, fileLength, f);
    buffer[fileLength] = '\0';
    trap->FS_Close (f);
    
    ParseConstantsFile (buffer);
    
    Com_Printf ("-----------------------------------\n");
    
    return qtrue;
}

void JKG_InitializeConstants(void)
{
	DefineBaselineConstants();
	if(!ReadConstantsFile())
	{
		//Com_Error(ERR_DROP, "Unable to parse the constants file.");
	}
}
