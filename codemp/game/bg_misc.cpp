/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// bg_misc.c -- both games misc functions, all completely stateless

#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "jkg_gangwars.h"
#include <json/cJSON.h>

#if defined(_GAME)
	#include "g_local.h"
#elif defined(IN_UI)
	#include "../ui/ui_local.h"
#elif defined(_CGAME)
	#include "../cgame/cg_local.h"
#endif

#ifdef _GAME
extern void Q3_SetParm (int entID, int parmNum, const char *parmValue);
#endif

const char *bgToggleableSurfaces[BG_NUM_TOGGLEABLE_SURFACES] = 
{
	"l_arm_key",					//0
	"torso_canister1",
	"torso_canister2",
	"torso_canister3",
	"torso_tube1",
	"torso_tube2",					//5
	"torso_tube3",
	"torso_tube4",
	"torso_tube5",
	"torso_tube6",
	"r_arm",						//10
	"l_arm",
	"torso_shield",
	"torso_galaktorso",
	"torso_collar",
//	"torso_eyes_mouth",				//15
//	"torso_galakhead",
//	"torso_galakface",
//	"torso_antenna_base_cap",
//	"torso_antenna",
//	"l_arm_augment",				//20
//	"l_arm_middle",
//	"l_arm_wrist",
//	"r_arm_middle", //yeah.. galak's surf stuff is no longer auto, sorry! need the space for vehicle surfs.
	"r_wing1",						//15
	"r_wing2",
	"l_wing1",
	"l_wing2",
	"r_gear",
	"l_gear",						//20
	"nose",
	"blah4",
	"blah5",
	"l_hand",
	"r_hand",						//25
	"helmet",
	"head",
	"head_concussion_charger",
	"head_light_blaster_cann",		//29
	NULL
};

const int bgToggleableSurfaceDebris[BG_NUM_TOGGLEABLE_SURFACES] =
{
	0,					//0
	0,
	0,
	0,
	0,
	0,					//5
	0,
	0,
	0,
	0,
	0,					//10
	0,
	0,
	0,
	0, //>= 2 means it should create a flame trail when destroyed (for vehicles)
	3,					//15
	5, //rwing2
	4,
	6, //lwing2
	0, //rgear
	0, //lgear			//20
	7, //nose
	0, //blah
	0, //blah
	0,
	0,					//25
	0,
	0,
	0,
	0,					//29
	-1
};

const char	*bg_customSiegeSoundNames[MAX_CUSTOM_SIEGE_SOUNDS] = 
{
	"*att_attack",
	"*att_primary",
	"*att_second",
	"*def_guns",
	"*def_position",
	"*def_primary",
	"*def_second",
	"*reply_coming",
	"*reply_go",
	"*reply_no",
	"*reply_stay",
	"*reply_yes",
	"*req_assist",
	"*req_demo",
	"*req_hvy",
	"*req_medic",
	"*req_sup",
	"*req_tech",
	"*spot_air",
	"*spot_defenses",
	"*spot_emplaced",
	"*spot_sniper",
	"*spot_troops",
	"*tac_cover",
	"*tac_fallback",
	"*tac_follow",
	"*tac_hold",
	"*tac_split",
	"*tac_together",
	NULL
};

//rww - not putting @ in front of these because
//we don't need them in a cgame StringEd lookup.
//Let me know if this causes problems, pat.
char *forceMasteryLevels[NUM_FORCE_MASTERY_LEVELS] = 
{
	"MASTERY0",	//"Uninitiated",	// FORCE_MASTERY_UNINITIATED,
	"MASTERY1",	//"Initiate",		// FORCE_MASTERY_INITIATE,
	"MASTERY2",	//"Padawan",		// FORCE_MASTERY_PADAWAN,
	"MASTERY3",	//"Jedi",			// FORCE_MASTERY_JEDI,
	"MASTERY4",	//"Jedi Adept",		// FORCE_MASTERY_JEDI_GUARDIAN,
	"MASTERY5",	//"Jedi Guardian",	// FORCE_MASTERY_JEDI_ADEPT,
	"MASTERY6",	//"Jedi Knight",	// FORCE_MASTERY_JEDI_KNIGHT,
	"MASTERY7",	//"Jedi Master"		// FORCE_MASTERY_JEDI_MASTER,
};

int forceMasteryPoints[NUM_FORCE_MASTERY_LEVELS] =
{
	0,		// FORCE_MASTERY_UNINITIATED,
	5,		// FORCE_MASTERY_INITIATE,
	10,		// FORCE_MASTERY_PADAWAN,
	20,		// FORCE_MASTERY_JEDI,
	30,		// FORCE_MASTERY_JEDI_GUARDIAN,
	50,		// FORCE_MASTERY_JEDI_ADEPT,
	75,		// FORCE_MASTERY_JEDI_KNIGHT,
	100		// FORCE_MASTERY_JEDI_MASTER,
};

int bgForcePowerCost[NUM_FORCE_POWERS][NUM_FORCE_POWER_LEVELS] = //0 == neutral
{
	{	0,	2,	4,	6	},	// Heal			// FP_HEAL
	{	0,	0,	2,	6	},	// Jump			//FP_LEVITATION,//hold/duration
	{	0,	2,	4,	6	},	// Speed		//FP_SPEED,//duration
	{	0,	1,	3,	6	},	// Push			//FP_PUSH,//hold/duration
	{	0,	1,	3,	6	},	// Pull			//FP_PULL,//hold/duration
	{	0,	4,	6,	8	},	// Mind Trick	//FP_TELEPATHY,//instant
	{	0,	1,	3,	6	},	// Grip			//FP_GRIP,//hold/duration
	{	0,	2,	5,	8	},	// Lightning	//FP_LIGHTNING,//hold/duration
	{	0,	4,	6,	8	},	// Dark Rage	//FP_RAGE,//duration
	{	0,	2,	5,	8	},	// Protection	//FP_PROTECT,//duration
	{	0,	1,	3,	6	},	// Absorb		//FP_ABSORB,//duration
	{	0,	1,	3,	6	},	// Team Heal	//FP_TEAM_HEAL,//instant
	{	0,	1,	3,	6	},	// Team Force	//FP_TEAM_FORCE,//instant
	{	0,	2,	4,	6	},	// Drain		//FP_DRAIN,//hold/duration
	{	0,	2,	5,	8	},	// Sight		//FP_SEE,//duration
	{	0,	1,	5,	8	},	// Saber Attack	//FP_SABER_OFFENSE,
	{	0,	1,	5,	8	},	// Saber Defend	//FP_SABER_DEFENSE,
	{	0,	4,	6,	8	}	// Saber Throw	//FP_SABERTHROW,
	//NUM_FORCE_POWERS
};

int forcePowerSorted[NUM_FORCE_POWERS] = 
{ //rww - always use this order when drawing force powers for any reason
	FP_TELEPATHY,
	FP_HEAL,
	FP_ABSORB,
	FP_PROTECT,
	FP_TEAM_HEAL,
	FP_LEVITATION,
	FP_SPEED,
	FP_PUSH,
	FP_PULL,
	FP_SEE,
	FP_LIGHTNING,
	FP_DRAIN,
	FP_RAGE,
	FP_GRIP,
	FP_TEAM_FORCE,
	FP_SABER_OFFENSE,
	FP_SABER_DEFENSE,
	FP_SABERTHROW
};

int forcePowerDarkLight[NUM_FORCE_POWERS] = //0 == neutral
{ //nothing should be usable at rank 0..
	FORCE_LIGHTSIDE,//FP_HEAL,//instant
	0,//FP_LEVITATION,//hold/duration
	0,//FP_SPEED,//duration
	0,//FP_PUSH,//hold/duration
	0,//FP_PULL,//hold/duration
	FORCE_LIGHTSIDE,//FP_TELEPATHY,//instant
	FORCE_DARKSIDE,//FP_GRIP,//hold/duration
	FORCE_DARKSIDE,//FP_LIGHTNING,//hold/duration
	FORCE_DARKSIDE,//FP_RAGE,//duration
	FORCE_LIGHTSIDE,//FP_PROTECT,//duration
	FORCE_LIGHTSIDE,//FP_ABSORB,//duration
	FORCE_LIGHTSIDE,//FP_TEAM_HEAL,//instant
	FORCE_DARKSIDE,//FP_TEAM_FORCE,//instant
	FORCE_DARKSIDE,//FP_DRAIN,//hold/duration
	0,//FP_SEE,//duration
	0,//FP_SABER_OFFENSE,
	0,//FP_SABER_DEFENSE,
	0//FP_SABERTHROW,
		//NUM_FORCE_POWERS
};

//Weapon and force power tables are also used in NPC parsing code and some other places.
stringID_table_t WPTable[] =
{
	{"NULL",WP_NONE},
	ENUM2STRING(WP_NONE),
	// Player weapons
	ENUM2STRING(WP_STUN_BATON),
	ENUM2STRING(WP_MELEE),
	ENUM2STRING(WP_SABER),
	ENUM2STRING(WP_BRYAR_PISTOL),
	{"WP_BLASTER_PISTOL", WP_BRYAR_PISTOL},
	ENUM2STRING(WP_BLASTER),
	ENUM2STRING(WP_DISRUPTOR),
	ENUM2STRING(WP_BOWCASTER),
	ENUM2STRING(WP_REPEATER),
	ENUM2STRING(WP_DEMP2),
	ENUM2STRING(WP_FLECHETTE),
	ENUM2STRING(WP_ROCKET_LAUNCHER),
	ENUM2STRING(WP_THERMAL),
	ENUM2STRING(WP_TRIP_MINE),
	ENUM2STRING(WP_DET_PACK),
	ENUM2STRING(WP_CONCUSSION),
	ENUM2STRING(WP_BRYAR_OLD),
	ENUM2STRING(WP_EMPLACED_GUN),
	ENUM2STRING(WP_TURRET),
	{"", 0}
};

stringID_table_t FPTable[] =
{
	ENUM2STRING(FP_HEAL),
	ENUM2STRING(FP_LEVITATION),
	ENUM2STRING(FP_SPEED),
	ENUM2STRING(FP_PUSH),
	ENUM2STRING(FP_PULL),
	ENUM2STRING(FP_TELEPATHY),
	ENUM2STRING(FP_GRIP),
	ENUM2STRING(FP_LIGHTNING),
	ENUM2STRING(FP_RAGE),
	ENUM2STRING(FP_PROTECT),
	ENUM2STRING(FP_ABSORB),
	ENUM2STRING(FP_TEAM_HEAL),
	ENUM2STRING(FP_TEAM_FORCE),
	ENUM2STRING(FP_DRAIN),
	ENUM2STRING(FP_SEE),
	ENUM2STRING(FP_SABER_OFFENSE),
	ENUM2STRING(FP_SABER_DEFENSE),
	ENUM2STRING(FP_SABERTHROW),
	{"",	-1}
};

stringID_table_t PowerupTable[] =
{
	ENUM2STRING(PW_NONE),
	ENUM2STRING(PW_PULL),
	ENUM2STRING(PW_REDFLAG),
	ENUM2STRING(PW_BLUEFLAG),
	ENUM2STRING(PW_SHIELDHIT),
	ENUM2STRING(PW_SPEEDBURST),
	ENUM2STRING(PW_DISINT_4),
	ENUM2STRING(PW_SPEED),
	ENUM2STRING(PW_CLOAKED),

	{"", -1}
};

//======================================
//Parsing functions
//======================================
void BG_StripTabs(char *buf)
{
	int i = 0;
	int i_r = 0;

	while (buf[i])
	{
		if (buf[i] != /*SIEGECHAR_TAB*/ '\t')		// REALLY, RAVEN, REALLY?
													// YOU SET '\t' as a MACRO??!
													// FFFFFFFFFFFF
		{ //not a tab, just stick it in
			buf[i_r] = buf[i];
		}
		else
		{ //If it's a tab, convert it to a space.
			buf[i_r] = ' ';
		}

		i_r++;
		i++;
	}

	buf[i_r] = '\0';
}

int BG_GetValueGroup(char *buf, char *group, char *outbuf)
{
	int i = 0;
	int j;
	char checkGroup[4096];
	qboolean isGroup;
	int parseGroups = 0;

	while (buf[i])
	{
		if (buf[i] != ' ' && buf[i] != '{' && buf[i] != '}' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t')
		{ //we're on a valid character
			if (buf[i] == '/' &&
				buf[i+1] == '/')
			{ //this is a comment, so skip over it
				while (buf[i] && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t')
				{
					i++;
				}
			}
			else
			{ //parse to the next space/endline/eos and check this value against our group value.
				j = 0;

				while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t' && buf[i] != '{' && buf[i])
				{
					if (buf[i] == '/' && buf[i+1] == '/')
					{ //hit a comment, break out.
						break;
					}

					checkGroup[j] = buf[i];
					j++;
					i++;
				}
				checkGroup[j] = 0;

				//Make sure this is a group as opposed to a globally defined value.
				if (buf[i] == '/' && buf[i+1] == '/')
				{ //stopped on a comment, so first parse to the end of it.
                    while (buf[i] && buf[i] != '\n' && buf[i] != '\r')
					{
						i++;
					}
					while (buf[i] == '\n' || buf[i] == '\r')
					{
						i++;
					}
				}

				if (!buf[i])
				{
					Com_Error(ERR_DROP, "Unexpected EOF while looking for group '%s'", group);
				}

				isGroup = qfalse;

				while (buf[i] && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r'))
				{ //parse to the next valid character
					i++;
				}

				if (buf[i] == '{')
				{ //if the next valid character is an opening bracket, then this is indeed a group
					isGroup = qtrue;
				}

				//Is this the one we want?
				if (isGroup && !Q_stricmp(checkGroup, group))
				{ //guess so. Parse until we hit the { indicating the beginning of the group.
					while (buf[i] != '{' && buf[i])
					{
						i++;
					}

					if (buf[i])
					{ //We're at the start of the group now, so parse to the closing bracket.
						j = 0;

						parseGroups = 0;

						while ((buf[i] != '}' || parseGroups) && buf[i])
						{
							if (buf[i] == '{')
							{ //increment for the opening bracket.
								parseGroups++;
							}
							else if (buf[i] == '}')
							{ //decrement for the closing bracket
								parseGroups--;
							}

							if (parseGroups < 0)
							{ //Syntax error, I guess.
								Com_Error(ERR_DROP, "Found a closing bracket without an opening bracket while looking for group '%s'", group);
							}

							if ((buf[i] != '{' || parseGroups > 1) &&
								(buf[i] != '}' || parseGroups > 0))
							{ //don't put the start and end brackets for this group into the output buffer
								outbuf[j] = buf[i];
								j++;
							}

							if (buf[i] == '}' && !parseGroups)
							{ //Alright, we can break out now.
								break;
							}

							i++;
						}
						outbuf[j] = 0;

						//Verify that we ended up on the closing bracket.
						if (buf[i] != '}')
						{
							Com_Error(ERR_DROP, "Group '%s' is missing a closing bracket", group);
						}

						//Strip the tabs so we're friendly for value parsing.
						BG_StripTabs(outbuf);

						return 1; //we got it, so return 1.
					}
					else
					{
						Com_Error(ERR_DROP, "Error parsing group in file, unexpected EOF before opening bracket while looking for group '%s'", group);
					}
				}
				else if (!isGroup)
				{ //if it wasn't a group, parse to the end of the line
					while (buf[i] && buf[i] != '\n' && buf[i] != '\r')
					{
						i++;
					}
				}
				else
				{ //this was a group but we not the one we wanted to find, so parse by it.
					parseGroups = 0;

					while (buf[i] && (buf[i] != '}' || parseGroups))
					{
						if (buf[i] == '{')
						{
							parseGroups++;
						}
						else if (buf[i] == '}')
						{
							parseGroups--;
						}

						if (parseGroups < 0)
						{ //Syntax error, I guess.
							Com_Error(ERR_DROP, "Found a closing bracket without an opening bracket while looking for group '%s'", group);
						}

						if (buf[i] == '}' && !parseGroups)
						{ //Alright, we can break out now.
							break;
						}

						i++;
					}

					if (buf[i] != '}')
					{
						Com_Error(ERR_DROP, "Found an opening bracket without a matching closing bracket while looking for group '%s'", group);
					}

					i++;
				}
			}
		}
		else if (buf[i] == '{')
		{ //we're in a group that isn't the one we want, so parse to the end.
			parseGroups = 0;

			while (buf[i] && (buf[i] != '}' || parseGroups))
			{
				if (buf[i] == '{')
				{
					parseGroups++;
				}
				else if (buf[i] == '}')
				{
					parseGroups--;
				}

				if (parseGroups < 0)
				{ //Syntax error, I guess.
					Com_Error(ERR_DROP, "Found a closing bracket without an opening bracket while looking for group '%s'", group);
				}

				if (buf[i] == '}' && !parseGroups)
				{ //Alright, we can break out now.
					break;
				}

				i++;
			}

			if (buf[i] != '}')
			{
				Com_Error(ERR_DROP, "Found an opening bracket without a matching closing bracket while looking for group '%s'", group);
			}
		}

		if (!buf[i])
		{
			break;
		}
		i++;
	}

	return 0; //guess we never found it.
}

int BG_GetPairedValue(char *buf, char *key, char *outbuf)
{
	int i = 0;
	int j;
	int k;
	char checkKey[4096];

	while (buf[i])
	{
		if (buf[i] != ' ' && buf[i] != '{' && buf[i] != '}' && buf[i] != '\n' && buf[i] != '\r')
		{ //we're on a valid character
			if (buf[i] == '/' &&
				buf[i+1] == '/')
			{ //this is a comment, so skip over it
				while (buf[i] && buf[i] != '\n' && buf[i] != '\r')
				{
					i++;
				}
			}
			else
			{ //parse to the next space/endline/eos and check this value against our key value.
				j = 0;

				while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t' && buf[i])
				{
					if (buf[i] == '/' && buf[i+1] == '/')
					{ //hit a comment, break out.
						break;
					}

					checkKey[j] = buf[i];
					j++;
					i++;
				}
				checkKey[j] = 0;

				k = i;

				while (buf[k] && (buf[k] == ' ' || buf[k] == '\n' || buf[k] == '\r'))
				{
					k++;
				}

				if (buf[k] == '{')
				{ //this is not the start of a value but rather of a group. We don't want to look in subgroups so skip over the whole thing.
					int openB = 0;

					while (buf[i] && (buf[i] != '}' || openB))
					{
						if (buf[i] == '{')
						{
							openB++;
						}
						else if (buf[i] == '}')
						{
							openB--;
						}

						if (openB < 0)
						{
							Com_Error(ERR_DROP, "Unexpected closing bracket (too many) while parsing to end of group '%s'", checkKey);
						}

						if (buf[i] == '}' && !openB)
						{ //this is the end of the group
							break;
						}
						i++;
					}

					if (buf[i] == '}')
					{
						i++;
					}
				}
				else
				{
					//Is this the one we want?
					if (buf[i] != '/' || buf[i+1] != '/')
					{ //make sure we didn't stop on a comment, if we did then this is considered an error in the file.
						if (!Q_stricmp(checkKey, key))
						{ //guess so. Parse along to the next valid character, then put that into the output buffer and return 1.
							while ((buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r' || buf[i] == '\t') && buf[i])
							{
								i++;
							}

							if (buf[i])
							{ //We're at the start of the value now.
								qboolean parseToQuote = qfalse;

								if (buf[i] == '\"')
								{ //if the value is in quotes, then stop at the next quote instead of ' '
									i++;
									parseToQuote = qtrue;
								}

								j = 0;
								while ( ((!parseToQuote && buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\r') || (parseToQuote && buf[i] != '\"')) )
								{
									if (buf[i] == '/' &&
										buf[i+1] == '/')
									{ //hit a comment after the value? This isn't an ideal way to be writing things, but we'll support it anyway.
										break;
									}
									outbuf[j] = buf[i];
									j++;
									i++;

									if (!buf[i])
									{
										if (parseToQuote)
										{
											Com_Error(ERR_DROP, "Unexpected EOF while looking for endquote, error finding paired value for '%s'", key);
										}
										else
										{
											Com_Error(ERR_DROP, "Unexpected EOF while looking for space or endline, error finding paired value for '%s'", key);
										}
									}
								}
								outbuf[j] = 0;

								return 1; //we got it, so return 1.
							}
							else
							{
								Com_Error(ERR_DROP, "Error parsing file, unexpected EOF while looking for valud '%s'", key);
							}
						}
						else
						{ //if that wasn't the desired key, then make sure we parse to the end of the line, so we don't mistake a value for a key
							while (buf[i] && buf[i] != '\n')
							{
								i++;
							}
						}
					}
					else
					{
						Com_Error(ERR_DROP, "Error parsing file, found comment, expected value for '%s'", key);
					}
				}
			}
		}

		if (!buf[i])
		{
			break;
		}
		i++;
	}

	return 0; //guess we never found it.
}

qboolean BG_FileExists(const char *fileName)
{
	if (fileName && fileName[0])
	{
		int fh = 0;
		trap->FS_Open(fileName, &fh, FS_READ);
		if (fh > 0)
		{
			trap->FS_Close(fh);
			return qtrue;
		}
	}

	return qfalse;
}

// given a boltmatrix, return in vec a normalised vector for the axis requested in flags
void BG_GiveMeVectorFromMatrix(mdxaBone_t *boltMatrix, int flags, vec3_t vec)
{
	switch (flags)
	{
	case ORIGIN:
		vec[0] = boltMatrix->matrix[0][3];
		vec[1] = boltMatrix->matrix[1][3];
		vec[2] = boltMatrix->matrix[2][3];
		break;
	case POSITIVE_Y:
		vec[0] = boltMatrix->matrix[0][1];
		vec[1] = boltMatrix->matrix[1][1];
		vec[2] = boltMatrix->matrix[2][1];
 		break;
	case POSITIVE_X:
		vec[0] = boltMatrix->matrix[0][0];
		vec[1] = boltMatrix->matrix[1][0];
		vec[2] = boltMatrix->matrix[2][0];
		break;
	case POSITIVE_Z:
		vec[0] = boltMatrix->matrix[0][2];
		vec[1] = boltMatrix->matrix[1][2];
		vec[2] = boltMatrix->matrix[2][2];
		break;
	case NEGATIVE_Y:
		vec[0] = -boltMatrix->matrix[0][1];
		vec[1] = -boltMatrix->matrix[1][1];
		vec[2] = -boltMatrix->matrix[2][1];
		break;
	case NEGATIVE_X:
		vec[0] = -boltMatrix->matrix[0][0];
		vec[1] = -boltMatrix->matrix[1][0];
		vec[2] = -boltMatrix->matrix[2][0];
		break;
	case NEGATIVE_Z:
		vec[0] = -boltMatrix->matrix[0][2];
		vec[1] = -boltMatrix->matrix[1][2];
		vec[2] = -boltMatrix->matrix[2][2];
		break;
	}
}

/*
================
BG_LegalizedForcePowers

The magical function to end all functions.
This will take the force power string in powerOut and parse through it, then legalize
it based on the supposed rank and spit it into powerOut, returning true if it was legal
to begin with and false if not.
fpDisabled is actually only expected (needed) from the server, because the ui disables
force power selection anyway when force powers are disabled on the server.
================
*/
qboolean BG_LegalizedForcePowers(char *powerOut, size_t powerOutSize, int maxRank, qboolean freeSaber, int teamForce, int gametype, int fpDisabled)
{
	char powerBuf[128];
	char readBuf[128];
	qboolean maintainsValidity = qtrue;
	int powerLen = strlen(powerOut);
	int i = 0;
	int c = 0;
	int allowedPoints = 0;
	int usedPoints = 0;
	int countDown = 0;
	
	int final_Side;
	int final_Powers[NUM_FORCE_POWERS];

	if (powerLen != 22)
	{ //This should not happen. If it does, this is obviously a bogus string.
		//They can have this string. Because I said so.
		//strcpy(powerBuf, "7-1-032330000000001333");
		strcpy(powerBuf, "7-1-000000000000000000");
		maintainsValidity = qfalse;
	}
	else
	{
		strcpy(powerBuf, powerOut); //copy it as the original
	}

	//first of all, print the max rank into the string as the rank
	Q_strncpyz( powerOut, va( "%i-", maxRank ), powerOutSize );

	while (i < 128 && powerBuf[i] && powerBuf[i] != '-')
	{
		i++;
	}
	i++;
	while (i < 128 && powerBuf[i] && powerBuf[i] != '-')
	{
		readBuf[c] = powerBuf[i];
		c++;
		i++;
	}
	readBuf[c] = 0;
	i++;
	//at this point, readBuf contains the intended side
	final_Side = atoi(readBuf);

	if (final_Side != FORCE_LIGHTSIDE &&
		final_Side != FORCE_DARKSIDE)
	{ //Not a valid side. You will be dark. Because I said so. (this is something that should never actually happen unless you purposely feed in an invalid config)
		final_Side = FORCE_DARKSIDE;
		maintainsValidity = qfalse;
	}

	if (teamForce)
	{ //If we are under force-aligned teams, make sure we're on the right side.
		if (final_Side != teamForce)
		{
			final_Side = teamForce;
			//maintainsValidity = qfalse;
			//Not doing this, for now. Let them join the team with their filtered powers.
		}
	}

	//Now we have established a valid rank, and a valid side.
	//Read the force powers in, and cut them down based on the various rules supplied.
	c = 0;
	while (i < 128 && powerBuf[i] && powerBuf[i] != '\n' && c < NUM_FORCE_POWERS)
	{
		readBuf[0] = powerBuf[i];
		readBuf[1] = 0;
		final_Powers[c] = atoi(readBuf);
		c++;
		i++;
	}

	//final_Powers now contains all the stuff from the string
	//Set the maximum allowed points used based on the max rank level, and count the points actually used.
	allowedPoints = forceMasteryPoints[maxRank];

	i = 0;
	while (i < NUM_FORCE_POWERS)
	{ //if this power doesn't match the side we're on, then 0 it now.
		if (final_Powers[i] &&
			forcePowerDarkLight[i] &&
			forcePowerDarkLight[i] != final_Side)
		{
			final_Powers[i] = 0;
			//This is only likely to happen with g_forceBasedTeams. Let it slide.
		}

		if ( final_Powers[i] &&
			(fpDisabled & (1 << i)) )
		{ //if this power is disabled on the server via said server option, then we don't get it.
			final_Powers[i] = 0;
		}

		i++;
	}

	if (gametype < GT_TEAM)
	{ //don't bother with team powers then
		final_Powers[FP_TEAM_HEAL] = 0;
		final_Powers[FP_TEAM_FORCE] = 0;
	}

	usedPoints = 0;
	i = 0;
	while (i < NUM_FORCE_POWERS)
	{
		countDown = 0;

		countDown = final_Powers[i];

		while (countDown > 0)
		{
			usedPoints += bgForcePowerCost[i][countDown]; //[fp index][fp level]
			//if this is jump, or we have a free saber and it's offense or defense, take the level back down on level 1
			if ( countDown == 1 &&
				((i == FP_LEVITATION) ||
				 (i == FP_SABER_OFFENSE && freeSaber) ||
				 (i == FP_SABER_DEFENSE && freeSaber)) )
			{
				usedPoints -= bgForcePowerCost[i][countDown];
			}
			countDown--;
		}

		i++;
	}

	if (usedPoints > allowedPoints)
	{ //Time to do the fancy stuff. (meaning, slowly cut parts off while taking a guess at what is most or least important in the config)
		int attemptedCycles = 0;
		int powerCycle = 2;
		int minPow = 0;
		
		if (freeSaber)
		{
			minPow = 1;
		}

		maintainsValidity = qfalse;

		while (usedPoints > allowedPoints)
		{
			c = 0;

			while (c < NUM_FORCE_POWERS && usedPoints > allowedPoints)
			{
				if (final_Powers[c] && final_Powers[c] < powerCycle)
				{ //kill in order of lowest powers, because the higher powers are probably more important
					if (c == FP_SABER_OFFENSE &&
						(final_Powers[FP_SABER_DEFENSE] > minPow || final_Powers[FP_SABERTHROW] > 0))
					{ //if we're on saber attack, only suck it down if we have no def or throw either
						int whichOne = FP_SABERTHROW; //first try throw

						if (!final_Powers[whichOne])
						{
							whichOne = FP_SABER_DEFENSE; //if no throw, drain defense
						}

						while (final_Powers[whichOne] > 0 && usedPoints > allowedPoints)
						{
							if ( final_Powers[whichOne] > 1 ||
								( (whichOne != FP_SABER_OFFENSE || !freeSaber) &&
								  (whichOne != FP_SABER_DEFENSE || !freeSaber) ) )
							{ //don't take attack or defend down on level 1 still, if it's free
								usedPoints -= bgForcePowerCost[whichOne][final_Powers[whichOne]];
								final_Powers[whichOne]--;
							}
							else
							{
								break;
							}
						}
					}
					else
					{
						while (final_Powers[c] > 0 && usedPoints > allowedPoints)
						{
							if ( final_Powers[c] > 1 ||
								((c != FP_LEVITATION) &&
								(c != FP_SABER_OFFENSE || !freeSaber) &&
								(c != FP_SABER_DEFENSE || !freeSaber)) )
							{
								usedPoints -= bgForcePowerCost[c][final_Powers[c]];
								final_Powers[c]--;
							}
							else
							{
								break;
							}
						}
					}
				}

				c++;
			}

			powerCycle++;
			attemptedCycles++;

			if (attemptedCycles > NUM_FORCE_POWERS)
			{ //I think this should be impossible. But just in case.
				break;
			}
		}

		if (usedPoints > allowedPoints)
		{ //Still? Fine then.. we will kill all of your powers, except the freebies.
			i = 0;

			while (i < NUM_FORCE_POWERS)
			{
				final_Powers[i] = 0;
				if (i == FP_LEVITATION ||
					(i == FP_SABER_OFFENSE && freeSaber) ||
					(i == FP_SABER_DEFENSE && freeSaber))
				{
					final_Powers[i] = 1;
				}
				i++;
			}
			usedPoints = 0;
		}
	}

	if (freeSaber)
	{
		if (final_Powers[FP_SABER_OFFENSE] < 1)
		{
			final_Powers[FP_SABER_OFFENSE] = 1;
		}
		if (final_Powers[FP_SABER_DEFENSE] < 1)
		{
			final_Powers[FP_SABER_DEFENSE] = 1;
		}
	}
	if (final_Powers[FP_LEVITATION] < bgConstants.baseForceJumpLevel)
	{
		final_Powers[FP_LEVITATION] = bgConstants.baseForceJumpLevel;
	}

	i = 0;
	while (i < NUM_FORCE_POWERS)
	{
		if (final_Powers[i] > FORCE_LEVEL_3)
		{
			final_Powers[i] = FORCE_LEVEL_3;
		}
		i++;
	}

	if (fpDisabled)
	{ //If we specifically have attack or def disabled, force them up to level 3. It's the way
	  //things work for the case of all powers disabled.
	  //If jump is disabled, down-cap it to level 1. Otherwise don't do a thing.
		if (fpDisabled & (1 << FP_LEVITATION))
		{
			final_Powers[FP_LEVITATION] = bgConstants.baseForceJumpLevel;
		}
		if (fpDisabled & (1 << FP_SABER_OFFENSE))
		{
			final_Powers[FP_SABER_OFFENSE] = 3;
		}
		if (fpDisabled & (1 << FP_SABER_DEFENSE))
		{
			final_Powers[FP_SABER_DEFENSE] = 3;
		}
	}

	if (final_Powers[FP_SABER_OFFENSE] < 1)
	{
		final_Powers[FP_SABER_DEFENSE] = 0;
		final_Powers[FP_SABERTHROW] = 0;
	}

	//We finally have all the force powers legalized and stored locally.
	//Put them all into the string and return the result. We already have
	//the rank there, so print the side and the powers now.
	Q_strcat(powerOut, powerOutSize, va("%i-", final_Side));

	i = strlen(powerOut);
	c = 0;
	while (c < NUM_FORCE_POWERS)
	{
		strcpy(readBuf, va("%i", final_Powers[c]));
		powerOut[i] = readBuf[0];
		c++;
		i++;
	}
	powerOut[i] = 0;

	return maintainsValidity;
}

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t	bg_itemlist[] = 
{
	{
		NULL,				// classname	
		NULL,				// pickup_sound
		{	NULL,			// world_model[0]
			NULL,			// world_model[1]
			0, 0} ,			// world_model[2],[3]
		NULL,				// view_model
/* icon */		NULL,		// icon
/* pickup */	//NULL,		// pickup_name
		0,					// quantity
		IT_BAD,				// giType (IT_*)
		0,					// giTag
/* precache */ "",			// precaches
/* sounds */ "",			// sounds
		""					// description
	},	// leave index 0 alone

	//
	// Pickups
	//

/*QUAKED item_shield_sm_instant (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Instant shield pickup, restores 25
*/
	{
		"item_shield_sm_instant", 
		"sound/player/pickupshield.wav",
        { "models/map_objects/mp/psd_sm.md3",
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/small_shield",
/* pickup *///	"Shield Small",
		25,
		IT_ARMOR,
		1, //special for shield - max on pickup is maxhealth*tag, thus small shield goes up to 100 shield
/* precache */ "",
/* sounds */ ""
		""					// description
	},

/*QUAKED item_shield_lrg_instant (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Instant shield pickup, restores 100
*/
	{
		"item_shield_lrg_instant", 
		"sound/player/pickupshield.wav",
        { "models/map_objects/mp/psd.md3",
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/large_shield",
/* pickup *///	"Shield Large",
		100,
		IT_ARMOR,
		2, //special for shield - max on pickup is maxhealth*tag, thus large shield goes up to 200 shield
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED item_medpak_instant (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Instant medpack pickup, heals 25
*/
	{
		"item_medpak_instant",
		"sound/player/pickuphealth.wav",
        { "models/map_objects/mp/medpac.md3", 
		0, 0, 0 },
/* view */		NULL,			
/* icon */		"gfx/hud/i_icon_medkit",
/* pickup *///	"Medpack",
		25,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

	//
	// WEAPONS 
	//

/*QUAKED weapon_stun_baton (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		"weapon_stun_baton", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/stun_baton/baton_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/stun_baton/baton.md3", 
/* icon */		"gfx/hud/w_icon_stunbaton",
/* pickup *///	"Stun Baton",
		100,
		IT_WEAPON,
		WP_STUN_BATON,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED weapon_melee (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		"weapon_melee", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/stun_baton/baton_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/stun_baton/baton.md3", 
/* icon */		"gfx/hud/w_icon_melee",
/* pickup *///	"Stun Baton",
		100,
		IT_WEAPON,
		WP_MELEE,
/* precache */ "",
/* sounds */ "",
		"@MENUS_MELEE_DESC"					// description
	},

/*QUAKED weapon_saber (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		"weapon_saber", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/saber/saber_w.glm",
		0, 0, 0},
/* view */		"models/weapons2/saber/saber_w.md3",
/* icon */		"gfx/hud/w_icon_lightsaber",
/* pickup *///	"Lightsaber",
		100,
		IT_WEAPON,
		WP_SABER,
/* precache */ "",
/* sounds */ "",
		"@MENUS_AN_ELEGANT_WEAPON_FOR"				// description
	},

/*QUAKED weapon_bryar_pistol (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		//"weapon_bryar_pistol", 
		"weapon_blaster_pistol", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/blaster_pistol/blaster_pistol_w.glm",//"models/weapons2/briar_pistol/briar_pistol_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/blaster_pistol/blaster_pistol.md3",//"models/weapons2/briar_pistol/briar_pistol.md3", 
/* icon */		"gfx/hud/w_icon_blaster_pistol",//"gfx/hud/w_icon_rifle",
/* pickup *///	"Bryar Pistol",
		100,
		IT_WEAPON,
		WP_BRYAR_PISTOL,
/* precache */ "",
/* sounds */ "",
		"@MENUS_BLASTER_PISTOL_DESC"					// description
	},

/*QUAKED weapon_concussion_rifle (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_concussion_rifle", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/concussion/c_rifle_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/concussion/c_rifle.md3", 
/* icon */		"gfx/hud/w_icon_c_rifle",//"gfx/hud/w_icon_rifle",
/* pickup *///	"Concussion Rifle",
		50,
		IT_WEAPON,
		WP_CONCUSSION,
/* precache */ "",
/* sounds */ "",
		"@MENUS_CONC_RIFLE_DESC"					// description
	},

/*QUAKED weapon_bryar_pistol_old (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		"weapon_bryar_pistol", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/briar_pistol/briar_pistol_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/briar_pistol/briar_pistol.md3", 
/* icon */		"gfx/hud/w_icon_briar",//"gfx/hud/w_icon_rifle",
/* pickup *///	"Bryar Pistol",
		100,
		IT_WEAPON,
		WP_BRYAR_OLD,
/* precache */ "",
/* sounds */ "",
		"@SP_INGAME_BLASTER_PISTOL"					// description
	},

/*QUAKED weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_blaster", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/blaster_r/blaster_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/blaster_r/blaster.md3", 
/* icon */		"gfx/hud/w_icon_blaster",
/* pickup *///	"E11 Blaster Rifle",
		100,
		IT_WEAPON,
		WP_BLASTER,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THE_PRIMARY_WEAPON_OF"				// description
	},

/*QUAKED weapon_disruptor (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_disruptor",
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/disruptor/disruptor_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/disruptor/disruptor.md3", 
/* icon */		"gfx/hud/w_icon_disruptor",
/* pickup *///	"Tenloss Disruptor Rifle",
		100,
		IT_WEAPON,
		WP_DISRUPTOR,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THIS_NEFARIOUS_WEAPON"					// description
	},

/*QUAKED weapon_bowcaster (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_bowcaster",
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/bowcaster/bowcaster_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/bowcaster/bowcaster.md3", 
/* icon */		"gfx/hud/w_icon_bowcaster",
/* pickup *///	"Wookiee Bowcaster",
		100,
		IT_WEAPON,
		WP_BOWCASTER,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THIS_ARCHAIC_LOOKING"					// description
	},

/*QUAKED weapon_repeater (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_repeater", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/heavy_repeater/heavy_repeater_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/heavy_repeater/heavy_repeater.md3", 
/* icon */		"gfx/hud/w_icon_repeater",
/* pickup *///	"Imperial Heavy Repeater",
		100,
		IT_WEAPON,
		WP_REPEATER,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THIS_DESTRUCTIVE_PROJECTILE"					// description
	},

/*QUAKED weapon_demp2 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
NOTENOTE This weapon is not yet complete.  Don't place it.
*/
	{
		"weapon_demp2", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/demp2/demp2_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/demp2/demp2.md3", 
/* icon */		"gfx/hud/w_icon_demp2",
/* pickup *///	"DEMP2",
		100,
		IT_WEAPON,
		WP_DEMP2,
/* precache */ "",
/* sounds */ "",
		"@MENUS_COMMONLY_REFERRED_TO"					// description
	},

/*QUAKED weapon_flechette (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_flechette", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/golan_arms/golan_arms_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/golan_arms/golan_arms.md3", 
/* icon */		"gfx/hud/w_icon_flechette",
/* pickup *///	"Golan Arms Flechette",
		100,
		IT_WEAPON,
		WP_FLECHETTE,
/* precache */ "",
/* sounds */ "",
		"@MENUS_WIDELY_USED_BY_THE_CORPORATE"					// description
	},

/*QUAKED weapon_rocket_launcher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_rocket_launcher",
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/merr_sonn/merr_sonn_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/merr_sonn/merr_sonn.md3", 
/* icon */		"gfx/hud/w_icon_merrsonn",
/* pickup *///	"Merr-Sonn Missile System",
		3,
		IT_WEAPON,
		WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THE_PLX_2M_IS_AN_EXTREMELY"					// description
	},

/*QUAKED ammo_thermal (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_thermal",
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/thermal/thermal_pu.md3", 
		"models/weapons2/thermal/thermal_w.glm", 0, 0},
/* view */		"models/weapons2/thermal/thermal.md3", 
/* icon */		"gfx/hud/w_icon_thermal",
/* pickup *///	"Thermal Detonators",
		4,
		IT_AMMO,
		AMMO_THERMAL,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THE_THERMAL_DETONATOR"					// description
	},

/*QUAKED ammo_tripmine (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_tripmine", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/laser_trap/laser_trap_pu.md3", 
		"models/weapons2/laser_trap/laser_trap_w.glm", 0, 0},
/* view */		"models/weapons2/laser_trap/laser_trap.md3", 
/* icon */		"gfx/hud/w_icon_tripmine",
/* pickup *///	"Trip Mines",
		3,
		IT_AMMO,
		AMMO_TRIPMINE,
/* precache */ "",
/* sounds */ "",
		"@MENUS_TRIP_MINES_CONSIST_OF"					// description
	},

/*QUAKED ammo_detpack (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_detpack", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/detpack/det_pack_pu.md3", "models/weapons2/detpack/det_pack_proj.glm", "models/weapons2/detpack/det_pack_w.glm", 0},
/* view */		"models/weapons2/detpack/det_pack.md3", 
/* icon */		"gfx/hud/w_icon_detpack",
/* pickup *///	"Det Packs",
		3,
		IT_AMMO,
		AMMO_DETPACK,
/* precache */ "",
/* sounds */ "",
		"@MENUS_A_DETONATION_PACK_IS"					// description
	},

/*QUAKED weapon_thermal (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_thermal",
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/thermal/thermal_w.glm", "models/weapons2/thermal/thermal_pu.md3",
		0, 0 },
/* view */		"models/weapons2/thermal/thermal.md3", 
/* icon */		"gfx/hud/w_icon_thermal",
/* pickup *///	"Thermal Detonator",
		4,
		IT_WEAPON,
		WP_THERMAL,
/* precache */ "",
/* sounds */ "",
		"@MENUS_THE_THERMAL_DETONATOR"					// description
	},

/*QUAKED weapon_trip_mine (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_trip_mine", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/laser_trap/laser_trap_w.glm", "models/weapons2/laser_trap/laser_trap_pu.md3",
		0, 0},
/* view */		"models/weapons2/laser_trap/laser_trap.md3", 
/* icon */		"gfx/hud/w_icon_tripmine",
/* pickup *///	"Trip Mine",
		3,
		IT_WEAPON,
		WP_TRIP_MINE,
/* precache */ "",
/* sounds */ "",
		"@MENUS_TRIP_MINES_CONSIST_OF"					// description
	},

/*QUAKED weapon_det_pack (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_det_pack", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/detpack/det_pack_proj.glm", "models/weapons2/detpack/det_pack_pu.md3", "models/weapons2/detpack/det_pack_w.glm", 0},
/* view */		"models/weapons2/detpack/det_pack.md3", 
/* icon */		"gfx/hud/w_icon_detpack",
/* pickup *///	"Det Pack",
		3,
		IT_WEAPON,
		WP_DET_PACK,
/* precache */ "",
/* sounds */ "",
		"@MENUS_A_DETONATION_PACK_IS"					// description
	},

/*QUAKED weapon_emplaced (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_emplaced", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/blaster_r/blaster_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/blaster_r/blaster.md3", 
/* icon */		"gfx/hud/w_icon_blaster",
/* pickup *///	"Emplaced Gun",
		50,
		IT_WEAPON,
		WP_EMPLACED_GUN,
/* precache */ "",
/* sounds */ "",
		""					// description
	},


//NOTE: This is to keep things from messing up because the turret weapon type isn't real
	{
		"weapon_turretwp", 
		"sound/weapons/w_pkup.wav",
        { "models/weapons2/blaster_r/blaster_w.glm", 
		0, 0, 0},
/* view */		"models/weapons2/blaster_r/blaster.md3", 
/* icon */		"gfx/hud/w_icon_blaster",
/* pickup *///	"Turret Gun",
		50,
		IT_WEAPON,
		WP_TURRET,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_force (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Don't place this
*/
	{
		"ammo_force",
		"sound/player/pickupenergy.wav",
        { "models/items/energy_cell.md3", 
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/hud/w_icon_blaster",
/* pickup *///	"Force??",
		100,
		IT_AMMO,
		AMMO_FORCE,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED ammo_blaster (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Ammo for the Bryar and Blaster pistols.
*/
	{
		"ammo_blaster",
		"sound/player/pickupenergy.wav",
        { "models/items/energy_cell.md3", 
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/hud/i_icon_battery",
/* pickup *///	"Blaster Pack",
		100,
		IT_AMMO,
		AMMO_BLASTER,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED ammo_powercell (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Ammo for Tenloss Disruptor, Wookie Bowcaster, and the Destructive Electro Magnetic Pulse (demp2 ) guns
*/
	{
		"ammo_powercell",
		"sound/player/pickupenergy.wav",
        { "models/items/power_cell.md3", 
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/ammo_power_cell",
/* pickup *///	"Power Cell",
		100,
		IT_AMMO,
		AMMO_POWERCELL,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED ammo_metallic_bolts (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Ammo for Imperial Heavy Repeater and the Golan Arms Flechette
*/
	{
		"ammo_metallic_bolts",
		"sound/player/pickupenergy.wav",
        { "models/items/metallic_bolts.md3", 
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/ammo_metallic_bolts",
/* pickup *///	"Metallic Bolts",
		100,
		IT_AMMO,
		AMMO_METAL_BOLTS,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
Ammo for Merr-Sonn portable missile launcher
*/
	{
		"ammo_rockets",
		"sound/player/pickupenergy.wav",
        { "models/items/rockets.md3", 
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/ammo_rockets",
/* pickup *///	"Rockets",
		3,
		IT_AMMO,
		AMMO_ROCKETS,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED ammo_all (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
DO NOT PLACE in a map, this is only for siege classes that have ammo
dispensing ability
*/
	{
		"ammo_all",
		"sound/player/pickupenergy.wav",
        { "models/items/battery.md3",  //replace me
		0, 0, 0},
/* view */		NULL,			
/* icon */		"gfx/mp/ammo_rockets", //replace me
/* pickup *///	"Rockets",
		0,
		IT_AMMO,
		-1,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED team_CTF_redflag (1 0 0) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
		"team_CTF_redflag",
		NULL,
        { "models/flags/r_flag.md3",
		"models/flags/r_flag_ysal.md3", 0, 0 },
/* view */		NULL,			
/* icon */		"gfx/hud/mpi_rflag",
/* pickup *///	"Red Flag",
		0,
		IT_TEAM,
		PW_REDFLAG,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

/*QUAKED team_CTF_blueflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
		"team_CTF_blueflag",
		NULL,
        { "models/flags/b_flag.md3",
		"models/flags/b_flag_ysal.md3", 0, 0 },
/* view */		NULL,			
/* icon */		"gfx/hud/mpi_bflag",
/* pickup *///	"Blue Flag",
		0,
		IT_TEAM,
		PW_BLUEFLAG,
/* precache */ "",
/* sounds */ "",
		""					// description
	},

	// end of list marker
	{NULL}
};

int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;

float vectoyaw( const vec3_t vec ) {
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	} else {
		if (vec[PITCH]) {
			yaw = ( atan2( vec[YAW], vec[PITCH]) * 180 / M_PI );
		} else if (vec[YAW] > 0) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}

qboolean BG_CanUseFPNow(int gametype, playerState_t *ps, int time, forcePowers_t power)
{
	if ( ps->forceRestricted )
	{
		return qfalse;
	}

	if (ps->weapon == WP_EMPLACED_GUN)
	{ //can't use any of your powers while on an emplaced weapon
		return qfalse;
	}

	if (ps->m_iVehicleNum)
	{ //can't use powers while riding a vehicle (this may change, I don't know)
		return qfalse;
	}

	if (ps->duelInProgress)
	{
		if (power != FP_SABER_OFFENSE && power != FP_SABER_DEFENSE && /*power != FP_SABERTHROW &&*/
			power != FP_LEVITATION)
		{
			if (!ps->saberLockFrame || power != FP_PUSH)
			{
				return qfalse;
			}
		}
	}

	if (ps->saberLockFrame || ps->saberLockTime > time)
	{
		if (power != FP_PUSH)
		{
			return qfalse;
		}
	}

	if (ps->fallingToDeath)
	{
		return qfalse;
	}

	if ((ps->brokenLimbs & (1 << BROKENLIMB_RARM)) ||
		(ps->brokenLimbs & (1 << BROKENLIMB_LARM)))
	{ //powers we can't use with a broken arm
        switch (power)
		{
		case FP_PUSH:
		case FP_PULL:
		case FP_GRIP:
		case FP_LIGHTNING:
		case FP_DRAIN:
			return qfalse;
		default:
			break;
		}
	}

	return qtrue;
}

/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t	*BG_FindItemForPowerup( powerup_t pw ) {
	int		i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( (bg_itemlist[i].giType == IT_POWERUP || 
					bg_itemlist[i].giType == IT_TEAM) && 
			bg_itemlist[i].giTag == pw ) {
			return &bg_itemlist[i];
		}
	}

	return NULL;
}


/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t	*BG_FindItemForWeapon( weapon_t weapon ) {
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++) {
		if ( it->giType == IT_WEAPON && it->giTag == weapon ) {
			return it;
		}
	}

	Com_Error( ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindItemForAmmo

===============
*/
gitem_t	*BG_FindItemForAmmo( ammoType_t ammo ) {
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++) {
		if ( it->giType == IT_AMMO && it->giTag == ammo ) {
			return it;
		}
	}

	Com_Error( ERR_DROP, "Couldn't find item for ammo %i", ammo);
	return NULL;
}

/*
===============
BG_FindItem

===============
*/
gitem_t	*BG_FindItem( const char *classname ) {
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
		if ( !Q_stricmp( it->classname, classname) )
			return it;
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t		origin;

	BG_EvaluateTrajectory( &item->pos, atTime, origin );

	// we are ignoring ducked differences here
	if ( ps->origin[0] - origin[0] > 44
		|| ps->origin[0] - origin[0] < -50
		|| ps->origin[1] - origin[1] > 36
		|| ps->origin[1] - origin[1] < -36
		|| ps->origin[2] - origin[2] > 36
		|| ps->origin[2] - origin[2] < -36 ) {
		return qfalse;
	}

	return qtrue;
}

int BG_ProperForceIndex(int power)
{
	int i = 0;

	if (power < 0) {
		return -1;
	}

	while (i < NUM_FORCE_POWERS)
	{
		if (forcePowerSorted[i] == power)
		{
			return i;
		}

		i++;
	}

	return -1;
}

void BG_CycleForce(playerState_t *ps, int direction)
{
	int i = ps->fd.forcePowerSelected;
	int x = i;
	int presel = i;
	int foundnext = -1;

	if (!(ps->fd.forcePowersKnown & (1 << x)) ||
		x >= NUM_FORCE_POWERS ||
		x == -1)
	{ //apparently we have no valid force powers
		return;
	}

	x = BG_ProperForceIndex(x);
	presel = x;

	if (direction == 1)
	{ //get the next power
		x++;
	}
	else
	{ //get the previous power
		x--;
	}

	if (x >= NUM_FORCE_POWERS)
	{ //cycled off the end.. cycle around to the first
		x = 0;
	}
	if (x < 0)
	{ //cycled off the beginning.. cycle around to the last
		x = NUM_FORCE_POWERS-1;
	}

	i = forcePowerSorted[x]; //the "sorted" value of this power

	while (x != presel)
	{ //loop around to the current force power
		if (ps->fd.forcePowersKnown & (1 << i) && i != ps->fd.forcePowerSelected)
		{ //we have the force power
			if (i != FP_LEVITATION &&
				i != FP_SABER_OFFENSE &&
				i != FP_SABER_DEFENSE &&
				i != FP_SABERTHROW)
			{ //it's selectable
				foundnext = i;
				break;
			}
		}

		if (direction == 1)
		{ //next
			x++;
		}
		else
		{ //previous
			x--;
		}
	
		if (x >= NUM_FORCE_POWERS)
		{ //loop around
			x = 0;
		}
		if (x < 0)
		{ //loop around
			x = NUM_FORCE_POWERS-1;
		}

		i = forcePowerSorted[x]; //set to the sorted value again
	}

	if (foundnext != -1)
	{ //found one, select it
		ps->fd.forcePowerSelected = foundnext;
	}
}

int BG_GetItemIndexByTag(int tag, int type)
{ //Get the itemlist index from the tag and type
	int i = 0;

	while (i < bg_numItems)
	{
		if (bg_itemlist[i].giTag == tag &&
			bg_itemlist[i].giType == type)
		{
			return i;
		}

		i++;
	}

	return 0;
}

/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps ) {
	gitem_t	*item;

	if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
		Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
	}

	item = &bg_itemlist[ent->modelindex];

	if ( ps )
	{
		if ( ps->duelInProgress )
		{ //no picking stuff up while in a duel, no matter what the type is
			return qfalse;
		}
	}
	else
	{//safety return since below code assumes a non-null ps
		return qfalse;
	}

	switch( item->giType ) {
	case IT_WEAPON:
		// TODO: move this into inventory stuff
		if (ent->generic1 == ps->clientNum && ent->powerups)
		{
			return qfalse;
		}
		if (!(ent->eFlags & EF_DROPPEDWEAPON) && (ps->stats[STAT_WEAPONS] & (1 << item->giTag)) &&
			item->giTag != WP_THERMAL && item->giTag != WP_TRIP_MINE && item->giTag != WP_DET_PACK)
		{ //weaponstay stuff.. if this isn't dropped, and you already have it, you don't get it.
			return qfalse;
		}
		return qtrue;	// weapons are always picked up

	case IT_AMMO:
		return qtrue;

	case IT_ARMOR:
		if ( ps->stats[STAT_SHIELD] >= ps->stats[STAT_MAX_SHIELD] ) {
			return qfalse;
		}
		return qtrue;

	case IT_HEALTH:
		// small and mega healths will go over the max, otherwise
		// don't pick up if already at max
		if ((ps->fd.forcePowersActive & (1 << FP_RAGE)))
		{
			return qfalse;
		}

		if ( item->quantity == 5 || item->quantity == 100 ) {
			if ( ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] * 2 ) {
				return qfalse;
			}
			return qtrue;
		}

		if ( ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] ) {
			return qfalse;
		}
		return qtrue;

	case IT_POWERUP:
		return qtrue;	// powerups are always picked up

	case IT_TEAM: // team items, such as flags
		if( gametype == GT_CTF ) {
			// ent->modelindex2 is non-zero on items if they are dropped
			// we need to know this because we can pick up our dropped flag (and return it)
			// but we can't pick up our flag at base
			if (ps->persistant[PERS_TEAM] == TEAM_RED) {
				if (item->giTag == PW_BLUEFLAG ||
					(item->giTag == PW_REDFLAG && ent->modelindex2) ||
					(item->giTag == PW_REDFLAG && ps->powerups[PW_BLUEFLAG]) )
					return qtrue;
			} else if (ps->persistant[PERS_TEAM] == TEAM_BLUE) {
				if (item->giTag == PW_REDFLAG ||
					(item->giTag == PW_BLUEFLAG && ent->modelindex2) ||
					(item->giTag == PW_BLUEFLAG && ps->powerups[PW_REDFLAG]) )
					return qtrue;
			}
		}

		return qfalse;

	default:
		break;
	}

	return qfalse;
}

//======================================================================


/*
================
Cublic Bezier Curve interpolation

================
*/

double QuadraticBezierInterpolate(double phase, double p0, double p1, double p2) 
{
	double t = phase;
	double it = 1-phase;
	return ( it*it * p0 ) + ( 2*it*t * p1 ) + ( t*t * p2 );
}

double CubicBezierInterpolate(double phase, double p0, double p1, double p2, double p3) 
{
	double t = phase;
	double it = 1-phase;
	return ( it*it*it * p0 ) + ( 3*it*it*t * p1 ) + ( 3*it*t*t * p2 ) + ( t*t*t * p3 );
}

double QuarticBezierInterpolate(double phase, double p0, double p1, double p2, double p3, double p4) 
{
	double t = phase;
	double it = 1-phase;
	return ( it*it*it*it * p0 ) + ( 4*it*it*it*t * p1 ) + ( 6*it*it*t*t * p2 ) + ( 4*it*t*t*t * p3 ) + ( t*t*t*t * p4 );
}

double QuinticBezierInterpolate(double phase, double p0, double p1, double p2, double p3, double p4, double p5) 
{
	double t = phase;
	double it = 1-phase;
	return ( it*it*it*it*it * p0 ) + ( 5*it*it*it*it*t * p1 ) + ( 10*it*it*it*t*t * p2 ) + ( 10*it*it*t*t*t * p3 ) + ( 5*it*t*t*t*t * p4 ) + ( t*t*t*t*t * p5 );
}

//=========================================================
// JKG_CalculateIronsightsPhase
//---------------------------------------------------------
// Returns a value between 0 and 1, where 0 represents
// normal gun position, and 1 represents ironsights in
// their correct position. All values in between are some
// position between normal position and ironsights
// position.
//=========================================================

float JKG_CalculateIronsightsPhase ( const playerState_t *ps, int levelTime, float *blend )
{
    double phase;
    unsigned int time = ps->ironsightsTime & ~IRONSIGHTS_MSB;
	weaponData_t *wp = BG_GetWeaponDataByIndex( ps->weaponId );
    if ( ps->ironsightsTime & IRONSIGHTS_MSB )
    {
		phase = CubicBezierInterpolate (Q_min (levelTime - time, wp->ironsightsTime) / (double)wp->ironsightsTime, 0.0, 0.0, 1.0, 1.0);
        *blend = Q_min (1.0f, Q_max (0.0f, phase));
    }
    else
    {
		phase = (*blend) - CubicBezierInterpolate (Q_min (levelTime - time, wp->ironsightsTime * (*blend)) / (double)(wp->ironsightsTime * (*blend)), 0.0, 0.0, 1.0, 1.0);
    }
    
    return Q_min (1.0f, Q_max (0.0f, phase));
}

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float		deltaTime;
	float		phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_NONLINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		//new slow-down at end
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{//FIXME: maybe scale this somehow?  So that it starts out faster and stops faster?
			deltaTime = tr->trDuration*0.001f*((float)cos( DEG2RAD(90.0f - (90.0f*((float)atTime-tr->trTime)/(float)tr->trDuration)) ));
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	case TR_NONLINEAR_ACCEL_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		// Inverse of the NONLINEAR_STOP, this one accelerates rather than decelerating
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*(1-((float)sin( DEG2RAD(90.0f - (90.0f*((float)atTime-tr->trTime)/(float)tr->trDuration)))));
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_EASEINOUT:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*CubicBezierInterpolate(((float)atTime-tr->trTime)/(float)tr->trDuration, 0, 0, 1, 1);
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_HARDEASEIN:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*CubicBezierInterpolate(((float)atTime-tr->trTime)/(float)tr->trDuration, 0, 0, 0, 1);
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SOFTEASEIN:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*CubicBezierInterpolate(((float)atTime-tr->trTime)/(float)tr->trDuration, 0, 0, 0.5, 1);
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_HARDEASEOUT:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*CubicBezierInterpolate(((float)atTime-tr->trTime)/(float)tr->trDuration, 0, 1, 1, 1);
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SOFTEASEOUT:
		if ( atTime > tr->trTime + tr->trDuration ) 
		{
			atTime = tr->trTime + tr->trDuration;
		}
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			deltaTime = 0;
		}
		else
		{
			deltaTime = tr->trDuration*0.001f*CubicBezierInterpolate(((float)atTime-tr->trTime)/(float)tr->trDuration, 0, 0.5, 1, 1);
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	default:
#ifdef _GAME
		Com_Error( ERR_DROP, "BG_EvaluateTrajectory: [GAME SIDE] unknown trType: %i", tr->trType );
#else
		Com_Error( ERR_DROP, "BG_EvaluateTrajectory: [CLIENTGAME SIDE] unknown trType: %i", tr->trType );
#endif
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float	deltaTime;
	float	phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = cos( deltaTime * M_PI * 2 );	// derivative of sin = cos
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_NONLINEAR_STOP:
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			VectorClear( result );
			return;
		}
		deltaTime = tr->trDuration*0.001f*((float)cos( DEG2RAD(90.0f - (90.0f*((float)atTime-tr->trTime)/(float)tr->trDuration)) ));
		VectorScale( tr->trDelta, deltaTime, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	case TR_NONLINEAR_ACCEL_STOP:
		if ( atTime - tr->trTime > tr->trDuration || atTime - tr->trTime <= 0  )
		{
			VectorClear( result );
			return;
		}
		deltaTime = tr->trDuration*0.001f*(1-((float)sin( DEG2RAD(90.0f - (90.0f*((float)atTime-tr->trTime)/(float)tr->trDuration)))));
		VectorScale( tr->trDelta, deltaTime, result );
		break;		
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

const char *eventnames[] = {
	"EV_NONE",

	"EV_CLIENTJOIN",

	"EV_FOOTSTEP",
	"EV_FOOTSTEP_METAL",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",

	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",

	"EV_FALL",

	"EV_JUMP_PAD",			// boing sound at origin", jump sound on player

	"EV_GHOUL2_MARK",			//create a projectile impact mark on something with a client-side g2 instance.

	"EV_GLOBAL_DUEL",
	"EV_PRIVATE_DUEL",

	"EV_JUMP",
	"EV_ROLL",
	"EV_WATER_TOUCH",	// foot touches
	"EV_WATER_LEAVE",	// foot leaves
	"EV_WATER_UNDER",	// head touches
	"EV_WATER_CLEAR",	// head leaves

	"EV_ITEM_PICKUP",			// normal item pickups are predictable
	"EV_GLOBAL_ITEM_PICKUP",	// powerup / team sounds are broadcast to everyone

	"EV_VEH_FIRE",

	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_FIRE_WEAPON",
	"EV_ALT_FIRE",
	"EV_HEATCRIT",
	"EV_OVERHEATED",
	"EV_HEATCOOLED",
	"EV_SABER_ATTACK",
	"EV_SABER_HIT",
	"EV_SABER_BLOCK",
	"EV_SABER_CLASHFLARE",
	"EV_SABER_UNHOLSTER",
	"EV_DISRUPTOR_MAIN_SHOT",
	"EV_DISRUPTOR_SNIPER_SHOT",
	"EV_DISRUPTOR_SNIPER_MISS",
	"EV_DISRUPTOR_HIT",
	"EV_DISRUPTOR_ZOOMSOUND",

	"EV_PREDEFSOUND",

	"EV_TEAM_POWER",

	"EV_SCREENSHAKE",
	
	"EV_LOCALTIMER",

	"EV_USE",			// +Use key

	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_USE_ITEM4",
	"EV_USE_ITEM5",
	"EV_USE_ITEM6",
	"EV_USE_ITEM7",
	"EV_USE_ITEM8",
	"EV_USE_ITEM9",
	"EV_USE_ITEM10",
	"EV_USE_ITEM11",
	"EV_USE_ITEM12",
	"EV_USE_ITEM13",
	"EV_USE_ITEM14",
	"EV_USE_ITEM15",

	"EV_ITEMUSEFAIL",

	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",

	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex
	"EV_MISSILE_STICK",

	"EV_PLAY_EFFECT",
	"EV_PLAY_EFFECT_ID", //finally gave in and added it..
	"EV_PLAY_PORTAL_EFFECT_ID",

	"EV_PLAYDOORSOUND",
	"EV_PLAYDOORLOOPSOUND",
	"EV_BMODEL_SOUND",

	"EV_MUTE_SOUND",
	"EV_VOICECMD_SOUND",
	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",		// no attenuation
	"EV_GLOBAL_TEAM_SOUND",
	"EV_ENTITY_SOUND",

	"EV_PLAY_ROFF",

	"EV_GLASS_SHATTER",
	"EV_DEBRIS",
	"EV_MISC_MODEL_EXP",

	"EV_CONC_ALT_IMPACT",

	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_MISSILE_MISS_METAL",
	"EV_BULLET",				// otherEntity is the shooter

	"EV_PAIN",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_OBITUARY",

	"EV_POWERUP_QUAD",
	"EV_POWERUP_BATTLESUIT",
	//"EV_POWERUP_REGEN",

	"EV_FORCE_DRAINED",

	"EV_GIB_PLAYER",			// gib a previously living player
	"EV_SCOREPLUM",			// score plum

	"EV_CTFMESSAGE",

	"EV_BODYFADE",

	"EV_SIEGE_ROUNDOVER",
	"EV_SIEGE_OBJECTIVECOMPLETE",

	"EV_DESTROY_GHOUL2_INSTANCE",

	"EV_DESTROY_WEAPON_MODEL",

	"EV_GIVE_NEW_RANK",
	"EV_SET_FREE_SABER",
	"EV_SET_FORCE_DISABLE",

	"EV_WEAPON_CHARGE",
	"EV_WEAPON_CHARGE_ALT",

	"EV_SHIELD_HIT",

	"EV_DEBUG_LINE",
	"EV_TESTLINE",
	"EV_STOPLOOPINGSOUND",
	"EV_STARTLOOPINGSOUND",
	"EV_TAUNT",
//fixme, added a bunch that aren't here!
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

//void	trap->Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {

#ifdef _DEBUG
	{
		static vmCvar_t		showEvents;
		static qboolean		isRegistered = qfalse;

		if (!isRegistered)
		{
			trap->Cvar_Register(&showEvents, "showevents", "0", 0);
			isRegistered = qtrue;
		}

		if ( showEvents.integer != 0 ) {
#ifdef _GAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_PS_EVENTS-1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS-1)] = eventParm;
	ps->eventSequence++;
}

/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad ) {
	// spectators don't use jump pads
	if ( ps->pm_type != PM_NORMAL && ps->pm_type != PM_JETPACK && ps->pm_type != PM_FLOAT ) {
		return;
	}

	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	VectorCopy( jumppad->origin2, ps->velocity );
}

/*
=================
BG_EmplacedView

Shared code for emplaced angle gun constriction
=================
*/
int BG_EmplacedView(vec3_t baseAngles, vec3_t angles, float *newYaw, float constraint)
{
	float dif = AngleSubtract(baseAngles[YAW], angles[YAW]);

	if (dif > constraint ||
		dif < -constraint)
	{
		float amt;

		if (dif > constraint)
		{
			amt = (dif-constraint);
			dif = constraint;
		}
		else if (dif < -constraint)
		{
			amt = (dif+constraint);
			dif = -constraint;
		}
		else
		{
			amt = 0.0f;
		}

		*newYaw = AngleSubtract(angles[YAW], -dif);

		if (amt > 1.0f || amt < -1.0f)
		{ //significant, force the view
			return 2;
		}
		else
		{ //just a little out of range
			return 1;
		}
	}

	return 0;
}

//To see if the client is trying to use one of the included skins not meant for MP.
//I don't much care for hardcoded strings, but this seems the best way to go.
qboolean BG_IsValidCharacterModel(const char *modelName, const char *skinName)
{
	if (!Q_stricmp(skinName, "menu"))
	{
		return qfalse;
	}
	else if (!Q_stricmp(modelName, "kyle"))
	{
		if (!Q_stricmp(skinName, "fpls"))
		{
			return qfalse;
		}
		else if (!Q_stricmp(skinName, "fpls2"))
		{
			return qfalse;
		}
		else if (!Q_stricmp(skinName, "fpls3"))
		{
			return qfalse;
		}
	}
	return qtrue;
}
#ifdef _CGAME
extern cg_t				cg;
#endif

qboolean BG_ValidateSkinForTeam( char *modelName, char *skinName, int team, float *colors, int redTeam, int blueTeam, int clientNum )
{
	int teamNum;
	if (Q_stricmp(bgGangWarsTeams[teamNum=((team == TEAM_RED) ? redTeam : blueTeam)].modelStore[0], "NULL"))
	{
		// First entry is not NULL, therefore we need to check the skin
		int i = 0;
		char buffer[MAX_QPATH];
#ifdef _CGAME
		if(cg.clientNum == clientNum)
		{
			// Eh. Why bother. Let custom skin nerds have their fun.
			return qtrue;
		}
#endif
		if(strchr(modelName, '/') != NULL) {
			// Model name has a slash, just use that.
			Q_strncpyz(buffer, modelName, sizeof(buffer));
		}
		else {
			sprintf(buffer, "%s/%s", modelName, skinName);
		}
		for(; i < bgGangWarsTeams[teamNum].numModelsInStore; i++)
		{
			if(!Q_stricmp(buffer, bgGangWarsTeams[teamNum].modelStore[i]))
			{
				// We're off scott-free.
				return qtrue;
			}
		}
		// NOPE
		Q_strncpyz( modelName, bgGangWarsTeams[teamNum].defaultModel, MAX_QPATH );
		Q_strncpyz( skinName, ((bgGangWarsTeams[teamNum].useTeamColors) ? ((team == TEAM_RED) ? "red" : "blue") : "default"), MAX_QPATH ); 
		return qtrue;
	}
	if ( team == TEAM_RED )
	{
		if( !bgGangWarsTeams[redTeam].useTeamColors )
		{
			return qtrue;	// Let 'em use whatever colors they want. Honey badger doesn't care.
		}
	}
	else if ( team == TEAM_BLUE )
	{
		if( !bgGangWarsTeams[blueTeam].useTeamColors )
		{
			return qtrue;	// Let 'em use whatever colors they want.
		}
	}
	if (!Q_stricmpn(modelName, "jedi_",5))
	{ //argh, it's a custom player skin!
		if (team == TEAM_RED && colors)
		{
			colors[0] = 1.0f;
			colors[1] = 0.0f;
			colors[2] = 0.0f;
		}
		else if (team == TEAM_BLUE && colors)
		{
			colors[0] = 0.0f;
			colors[1] = 0.0f;
			colors[2] = 1.0f;
		}
		return qtrue;
	}

	if (team == TEAM_RED)
	{
		if ( Q_stricmp( "red", skinName ) != 0 )
		{//not "red"
			if ( Q_stricmp( "blue", skinName ) == 0
				|| Q_stricmp( "default", skinName ) == 0
				|| strchr(skinName, '|')//a multi-skin playerModel
				|| !BG_IsValidCharacterModel(modelName, skinName) )
			{
				Q_strncpyz(skinName, "red", MAX_QPATH);
				return qfalse;
			}
			else
			{//need to set it to red
				int len = strlen( skinName );
				if ( len < 3 )
				{//too short to be "red"
					Q_strcat(skinName, MAX_QPATH, "_red");
				}
				else
				{
					char	*start = &skinName[len-3];
					if ( Q_strncmp( "red", start, 3 ) != 0 )
					{//doesn't already end in "red"
						if ( len+4 >= MAX_QPATH )
						{//too big to append "_red"
							Q_strncpyz(skinName, "red", MAX_QPATH);
							return qfalse;
						}
						else
						{
							Q_strcat(skinName, MAX_QPATH, "_red");
						}
					}
				}
				//if file does not exist, set to "red"
				if ( !BG_FileExists( va( "models/players/%s/model_%s.skin", modelName, skinName ) ) )
				{
					Q_strncpyz(skinName, "red", MAX_QPATH);
				}
				return qfalse;
			}
		}

	}
	else if (team == TEAM_BLUE)
	{
		if ( Q_stricmp( "blue", skinName ) != 0 )
		{
			if ( Q_stricmp( "red", skinName ) == 0
				|| Q_stricmp( "default", skinName ) == 0
				|| strchr(skinName, '|')//a multi-skin playerModel
				|| !BG_IsValidCharacterModel(modelName, skinName) )
			{
				Q_strncpyz(skinName, "blue", MAX_QPATH);
				return qfalse;
			}
			else
			{//need to set it to blue
				int len = strlen( skinName );
				if ( len < 4 )
				{//too short to be "blue"
					Q_strcat(skinName, MAX_QPATH, "_blue");
				}
				else 
				{
					char	*start = &skinName[len-4];
					if ( Q_strncmp( "blue", start, 4 ) != 0 )
					{//doesn't already end in "blue"
						if ( len+5 >= MAX_QPATH )
						{//too big to append "_blue"
							Q_strncpyz(skinName, "blue", MAX_QPATH);
							return qfalse;
						}
						else
						{
							Q_strcat(skinName, MAX_QPATH, "_blue");
						}
					}
				}
				//if file does not exist, set to "blue"
				if ( !BG_FileExists( va( "models/players/%s/model_%s.skin", modelName, skinName ) ) )
				{
					Q_strncpyz(skinName, "blue", MAX_QPATH);
				}
				return qfalse;
			}
		}
	}
	return qtrue;
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) {
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR ) {
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	for (i = 0; i < MAX_ARMOR; i++) {
		s->armor[i] = ps->armor[i];
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}
	// set the trDelta for flag direction
	VectorCopy( ps->velocity, s->pos.trDelta );

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	s->trickedentindex = ps->fd.forceMindtrickTargetIndex;
	s->trickedentindex2 = ps->fd.forceMindtrickTargetIndex2;
	s->trickedentindex3 = ps->fd.forceMindtrickTargetIndex3;
	s->trickedentindex4 = ps->fd.forceMindtrickTargetIndex4;

	s->forceFrame = ps->saberLockFrame;

	s->emplacedOwner = ps->electrifyTime;

	s->speed = ps->speed;

	s->genericenemyindex = ps->genericEnemyIndex;

	s->activeForcePass = ps->activeForcePass;

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;

	s->legsFlip = ps->legsFlip;
	s->torsoFlip = ps->torsoFlip;

	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	s->eFlags2 = ps->eFlags2;

	s->saberInFlight = ps->saberInFlight;
	s->saberEntityNum = ps->saberEntityNum;
	s->saberMove = ps->saberMove;
	s->forcePowersActive = ps->fd.forcePowersActive;
	s->jetpack = ps->jetpack;

	if (ps->duelInProgress)
	{
		s->bolt1 = 1;
	}
	else
	{
		s->bolt1 = 0;
	}

	s->otherEntityNum2 = ps->emplacedIndex;

	s->saberHolstered = ps->saberHolstered;

	if (ps->genericEnemyIndex != -1)
	{
		s->eFlags |= EF_SEEKERDRONE;
	}

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	if ( ps->externalEvent ) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if ( ps->entityEventSequence < ps->eventSequence ) {
		int		seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}


	s->weapon = ps->weapon;
	s->weaponVariation = ps->weaponVariation;
	s->groundEntityNum = ps->groundEntityNum;
	s->saberActionFlags = ps->saberActionFlags;

	s->saberSwingSpeed = ps->saberSwingSpeed;
	s->firingMode = ps->firingMode;
	s->weaponstate = ps->weaponstate;
	s->ammoType = ps->ammoType;
	s->sightsTransition = ps->sightsTransition;

	s->saberCrystal[0] = ps->saberCrystal[0];
	s->saberCrystal[1] = ps->saberCrystal[1];

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;

	//NOT INCLUDED IN ENTITYSTATETOPLAYERSTATE:
	s->modelindex2 = ps->weaponstate;
	s->constantLight = ps->weaponChargeTime;

	VectorCopy(ps->lastHitLoc, s->origin2);

	s->fireflag = ps->fd.saberAnimLevel;

	s->heldByClient = ps->heldByClient;
	s->ragAttach = ps->ragAttach;

	s->iModelScale = ps->iModelScale;

	s->brokenLimbs = ps->brokenLimbs;

	s->hasLookTarget = ps->hasLookTarget;
	s->lookTarget = ps->lookTarget;

	s->customRGBA[0] = ps->customRGBA[0];
	s->customRGBA[1] = ps->customRGBA[1];
	s->customRGBA[2] = ps->customRGBA[2];
	s->customRGBA[3] = ps->customRGBA[3];
	
	//s->ironsightsTime = ps->ironsightsTime;
	//s->sprintTime = ps->sprintTime;
	
	s->buffsActive = ps->buffsActive;
	for (i = 0; i < PLAYERBUFF_BITS; i++)
	{
		s->buffs[i].buffID = ps->buffs[i].buffID;
		s->buffs[i].intensity = ps->buffs[i].intensity;
	}
	s->freezeLegsAnim = ps->freezeLegsAnim;
	s->freezeTorsoAnim = ps->freezeTorsoAnim;
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) {
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR ) {
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	for (i = 0; i < MAX_ARMOR; i++)
	{
		s->armor[i] = ps->armor[i];
	}

	s->pos.trType = TR_LINEAR_STOP;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy( ps->velocity, s->pos.trDelta );
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	s->trickedentindex = ps->fd.forceMindtrickTargetIndex;
	s->trickedentindex2 = ps->fd.forceMindtrickTargetIndex2;
	s->trickedentindex3 = ps->fd.forceMindtrickTargetIndex3;
	s->trickedentindex4 = ps->fd.forceMindtrickTargetIndex4;

	s->forceFrame = ps->saberLockFrame;

	s->emplacedOwner = ps->electrifyTime;

	s->speed = ps->speed;

	s->genericenemyindex = ps->genericEnemyIndex;

	s->activeForcePass = ps->activeForcePass;

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;

	s->legsFlip = ps->legsFlip;
	s->torsoFlip = ps->torsoFlip;

	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	s->eFlags2 = ps->eFlags2;

	s->saberInFlight = ps->saberInFlight;
	s->saberEntityNum = ps->saberEntityNum;
	s->saberMove = ps->saberMove;
	s->forcePowersActive = ps->fd.forcePowersActive;

	if (ps->duelInProgress)
	{
		s->bolt1 = 1;
	}
	else
	{
		s->bolt1 = 0;
	}

	s->otherEntityNum2 = ps->emplacedIndex;

	s->saberHolstered = ps->saberHolstered;

	if (ps->genericEnemyIndex != -1)
	{
		s->eFlags |= EF_SEEKERDRONE;
	}

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	if ( ps->externalEvent ) {
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	} else if ( ps->entityEventSequence < ps->eventSequence ) {
		int		seq;

		if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		s->eventParm = ps->eventParms[ seq ];
		ps->entityEventSequence++;
	}
	s->weapon = ps->weapon;
	s->weaponVariation = ps->weaponVariation;
	s->groundEntityNum = ps->groundEntityNum;

	s->saberActionFlags = ps->saberActionFlags;
	s->ammoType = ps->ammoType;

	s->saberSwingSpeed = ps->saberSwingSpeed;
	s->firingMode = ps->firingMode;
	s->weaponstate = ps->weaponstate;
	s->sightsTransition = ps->sightsTransition;
	s->saberCrystal[0] = ps->saberCrystal[0];
	s->saberCrystal[1] = ps->saberCrystal[1];

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;

	//NOT INCLUDED IN ENTITYSTATETOPLAYERSTATE:
	s->modelindex2 = ps->weaponstate;
	s->constantLight = ps->weaponChargeTime;

	VectorCopy(ps->lastHitLoc, s->origin2);

	s->fireflag = ps->fd.saberAnimLevel;

	s->heldByClient = ps->heldByClient;
	s->ragAttach = ps->ragAttach;

	s->iModelScale = ps->iModelScale;

	s->brokenLimbs = ps->brokenLimbs;

	s->hasLookTarget = ps->hasLookTarget;
	s->lookTarget = ps->lookTarget;

	s->customRGBA[0] = ps->customRGBA[0];
	s->customRGBA[1] = ps->customRGBA[1];
	s->customRGBA[2] = ps->customRGBA[2];
	s->customRGBA[3] = ps->customRGBA[3];
	
//	s->ironsightsTime = ps->ironsightsTime;
	s->buffsActive = ps->buffsActive;
	for (i = 0; i < PLAYERBUFF_BITS; i++)
	{
		s->buffs[i].buffID = ps->buffs[i].buffID;
		s->buffs[i].intensity = ps->buffs[i].intensity;
	}
	s->freezeLegsAnim = ps->freezeLegsAnim;
	s->freezeTorsoAnim = ps->freezeTorsoAnim;
	s->jetpack = ps->jetpack;

//	s->sprintTime = ps->sprintTime;
}



/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

//perform the appropriate model precache routine
int BG_ModelCache(const char *modelName, const char *skinName)
{
#ifdef _GAME
	void *g2 = NULL;

	if (skinName && skinName[0])
	{
		trap->R_RegisterSkin(skinName);
	}

	//I could hook up a precache ghoul2 function, but oh well, this works
	trap->G2API_InitGhoul2Model(&g2, modelName, 0, 0, 0, 0, 0);
	if (g2)
	{ //now get rid of it
		trap->G2API_CleanGhoul2Models(&g2);
	}
	return 0;
#else
	if (skinName && skinName[0])
	{
		trap->R_RegisterSkin(skinName);
	}
	return trap->R_RegisterModel(modelName);
#endif
}

#include "../cgame/animtable.h"
int BG_ParseGenericAnimationFile ( animation_t *animset, size_t maxAnimations, const stringID_table_t *animTableOut, const char *filename, const char *fileText )
{
    int i;
    int animNum;
    float fps;
    const char *token;
    const char *text_p = fileText;

    //initialize anim array so that from 0 to MAX_ANIMATIONS, set default values of 0 1 0 100
	for ( i = 0; i < maxAnimations; i++ )
	{
		animset[i].firstFrame = 0;
		animset[i].numFrames = 0;
		animset[i].loopFrames = -1;
		animset[i].frameLerp = 100;
	}

    i = 0;
	// read information for each frame
	while ( 1 ) 
	{
		token = COM_Parse (&text_p);

		if ( !token || !token[0] ) 
		{
			break;
		}

		animNum = GetIDForString (const_cast<stringID_table_t *>(animTableOut), token);
		if ( animNum == -1 )
		{
//#ifndef FINAL_BUILD
#ifdef _DEBUG
			Com_Printf (S_COLOR_RED "WARNING: Unknown token %s in %s\n", token, filename);
			while ( token[0] )
			{
				token = COM_ParseExt (&text_p, qfalse);	//returns empty string when next token is EOL
			}
#endif
			continue;
		}
		
		if ( i >= maxAnimations )
		{
		    Com_Printf (S_COLOR_RED "WARNING: Max number of animations reached. Discontinuing animation file parsing.\n");
		    break;
		}

		token = COM_Parse (&text_p);
		if ( !token ) 
		{
			break;
		}
		animset[animNum].firstFrame = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) 
		{
			break;
		}
		animset[animNum].numFrames = atoi( token );

		token = COM_Parse(&text_p);
		if ( !token ) 
		{
			break;
		}
		animset[animNum].loopFrames = atoi( token );

		token = COM_Parse(&text_p);
		if ( !token ) 
		{
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) 
		{
			fps = 1;//Don't allow divide by zero error
		}
		if ( fps < 0 )
		{//backwards
			animset[animNum].frameLerp = floor (1000.0f / fps);
		}
		else
		{
			animset[animNum].frameLerp = ceil (1000.0f / fps);
		}
		
		i++;
	}
	
	return i;
}

const char *gametypeStringShort[GT_MAX_GAME_TYPE] = {
	"FFA",
	"1v1",
	"2v1",
	"SP",
	"TDM",
	"CTF",
	"WZ",
#ifdef __JKG_NINELIVES__
	"9L",
#endif
#ifdef __JKG_TICKETING__
	"LMS-T",
#endif
#ifdef __JKG_ROUNDBASED__
	"LMS-R",
#endif
};

const char *BG_GetGametypeString( int gametype )
{
	switch ( gametype )
	{
	case GT_FFA:
		return "Free-For-All";
	case GT_DUEL:
		return "Duel";
	case GT_POWERDUEL:
		return "Power Duel";
	case GT_SINGLE_PLAYER:
		return "Cooperative";
	case GT_TEAM:
		return "Team Deathmatch";
	case GT_CTF:
		return "Capture the Flag";
	case GT_WARZONE:
		return "Warzone";
#ifdef __JKG_NINELIVES__
	case GT_LMS_NINELIVES:
		return "Ninelives";
#endif
#ifdef __JKG_TICKETING__
	case GT_LMS_TICKETED:
		return "Ticketed LMS";
#endif
#ifdef __JKG_ROUNDBASED__
	case GT_LMS_ROUNDS:
		return "Round-based LMS";
#endif
	default:
		return "Unknown Gametype";
	}
}

int BG_GetGametypeForString( const char *gametype )
{
	if(Q_stricmp( gametype, "dm" ) == 0 ||
		Q_stricmp( gametype, "ffa" ) == 0 )
		return GT_FFA;
	else if( Q_stricmp( gametype, "duel" ) == 0 ||
		Q_stricmp( gametype, "1v1" ) == 0 )
		return GT_DUEL;
	else if( Q_stricmp( gametype, "powerduel" ) == 0 ||
		Q_stricmp( gametype, "power duel" ) == 0 ||
		Q_stricmp( gametype, "2v1" ) == 0 )
		return GT_POWERDUEL;
	else if( Q_stricmp( gametype, "sp" ) == 0 ||
		Q_stricmp( gametype, "coop" ) == 0 ||
		Q_stricmp( gametype, "co-op" ) == 0 ||
		Q_stricmp( gametype, "co op" ) == 0 )
		return GT_SINGLE_PLAYER;
	else if( Q_stricmp( gametype, "tdm" ) == 0 ||
		Q_stricmp( gametype, "tffa" ) == 0 )
		return GT_TEAM;
	else if( Q_stricmp( gametype, "ctf" ) == 0 )
		return GT_CTF;
	else if( Q_stricmp( gametype, "warzone" ) == 0 ||
		Q_stricmp( gametype, "wz" ) == 0 )
		return GT_WARZONE;
	else
		return GT_TEAM;
}

void Q_FSBinaryDump( const char *filename, const void *buffer, size_t len ) {
	fileHandle_t f = NULL_FILE;
	trap->FS_Open( filename, &f, FS_WRITE );
	trap->FS_Write( buffer, (int)len, f );
	trap->FS_Close( f );
	f = NULL_FILE;
}

// serialise a JSON object and write it to the specified file
void Q_FSWriteJSON( void *root, fileHandle_t f ) {
	const char *serialised = NULL;

	serialised = cJSON_Serialize( (cJSON *)root, 1 );
	trap->FS_Write( serialised, strlen( serialised ), f );
	trap->FS_Close( f );

	free( (void *)serialised );
	cJSON_Delete( (cJSON *)root );
}

void Q_FSWriteString( fileHandle_t f, const char *msg ) {
	if ( f != NULL_FILE ) {
		trap->FS_Write( msg, strlen( msg ), f );
	}
}

int Q_FSGetFileListSorted( const char *path, const char *extension, char *listbuf, int bufsize )
{
	int numFiles = trap->FS_GetFileList( path, extension, listbuf, bufsize );
	sortStrings( &numFiles, listbuf, bufsize );
	return numFiles;
}
