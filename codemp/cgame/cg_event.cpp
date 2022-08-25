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

// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"
#include "fx_local.h"
#include "../ui/ui_shared.h"
#include "../ui/ui_public.h"

// for the voice chats
#include "../../JKGalaxies/ui/menudef.h"

#include "ghoul2/G2.h"

#include "cg_weapons.h"
//==========================================================================

extern int cg_saberFlashTime;
extern vec3_t cg_saberFlashPos;
extern char *showPowersName[];

extern void JKG_SwapToSaber(int saberNum, clientInfo_t *ci, const char *newSaber, int weapon, int variation);

//vmCvar_t	jkg_nokillmessages;

//I know, not siege, but...
typedef enum
{
	TAUNT_TAUNT = 0,
	TAUNT_BOW,
	TAUNT_MEDITATE,
	TAUNT_FLOURISH,
	TAUNT_GLOAT
} tauntTypes_t ;
/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;
	// number extenstions, eg 1st, 2nd, 3rd, 4th etc.
	// note that the rules are different for french, but by changing the required strip strings they seem to work
	char sST[10];
	char sND[10];
	char sRD[10];
	char sTH[10];
	char sTiedFor[64];	// german is much longer, super safe...

	trap->SE_GetStringTextString("MP_INGAME_NUMBER_ST",sST, sizeof(sST) );
	trap->SE_GetStringTextString("MP_INGAME_NUMBER_ND",sND, sizeof(sND) );
	trap->SE_GetStringTextString("MP_INGAME_NUMBER_RD",sRD, sizeof(sRD) );
	trap->SE_GetStringTextString("MP_INGAME_NUMBER_TH",sTH, sizeof(sTH) );
	trap->SE_GetStringTextString("MP_INGAME_TIED_FOR" ,sTiedFor,sizeof(sTiedFor) );
	strcat(sTiedFor," ");	// save worrying about translators adding spaces or not

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = sTiedFor;//"Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = va("1%s",sST);//S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = va("2%s",sND);//S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = va("3%s",sRD);//S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = va("11%s",sTH);
	} else if ( rank == 12 ) {
		s = va("12%s",sTH);
	} else if ( rank == 13 ) {
		s = va("13%s",sTH);
	} else if ( rank % 10 == 1 ) {
		s = va("%i%s", rank,sST);
	} else if ( rank % 10 == 2 ) {
		s = va("%i%s", rank,sND);
	} else if ( rank % 10 == 3 ) {
		s = va("%i%s", rank,sRD);
	} else {
		s = va("%i%s", rank,sTH);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	const char	*attackerInfo;
	char		attackerName[MAX_QPATH];
	clientInfo_t	*ci;
	meansOfDamage_t* means;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;
	means = JKG_GetMeansOfDamage(mod);

	if (means == nullptr) {
		trap->Error(ERR_DROP, "CG_Obituary: mod out of range");
	}

	if ( target < 0 || target >= MAX_CLIENTS ) {
		trap->Error( ERR_DROP, "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	// Get info for attacker
	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
		Q_strncpyz(attackerName, Info_ValueForKey(attackerInfo, "n"), sizeof(attackerName));
	}

	// Attacker killed themselves.  Ridicule them for it.
	if (attacker == target) {
		
		if (mod == MOD_TEAM_CHANGE) //this doesn't count
			return;

		if (means->killfeed.genderedStringsPresent) {
			switch (ci->gender) { // don't assume, check!
				case GENDER_FEMALE:
					message = va("%s ^7%s\n", ci->name, CG_GetStringEdString2(means->killfeed.suicideMessage.gendered.female));
					break;
				case GENDER_MALE:
					message = va("%s ^7%s\n", ci->name, CG_GetStringEdString2(means->killfeed.suicideMessage.gendered.male));
					break;
				default:
				case GENDER_NEUTER:
					message = va("%s ^7%s\n", ci->name, CG_GetStringEdString2(means->killfeed.suicideMessage.gendered.neuter));
					break;
			}
		}
		else {
			message = va("%s ^7%s\n", ci->name, CG_GetStringEdString2(means->killfeed.suicideMessage.message));
		}
	}
	else if (attacker != ENTITYNUM_WORLD) {
		// Killed by another player
		message = va("%s ^7%s %s\n", ci->name, CG_GetStringEdString2(means->killfeed.killMessage), attackerName);
	}
	else {
		// Killed by world
		message = va("%s ^7%s\n", ci->name, CG_GetStringEdString2(means->killfeed.deathMessage));
	}

	// Print the actual message
	trap->Print(message);

	// If we're the attacker, display the "You Killed X" message.
	if (attacker == cg.snap->ps.clientNum)
	{
		char* s;

		if (cgs.gametype < GT_TEAM)
		{
			char sPlaceWith[256];
			char sKilledStr[256];
			trap->SE_GetStringTextString("MP_INGAME_PLACE_WITH", sPlaceWith, sizeof(sPlaceWith));
			trap->SE_GetStringTextString("MP_INGAME_KILLED_MESSAGE", sKilledStr, sizeof(sKilledStr));

			s = va("%s %s.\n%s %s %i.", sKilledStr, ci->name,
				CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1),
				sPlaceWith,
				cg.snap->ps.persistant[PERS_SCORE]);
		}
		else
		{
			char sKilledStr[256];
			trap->SE_GetStringTextString("MP_INGAME_KILLED_MESSAGE", sKilledStr, sizeof(sKilledStr));
			s = va("%s %s", sKilledStr, ci->name);
		}
		CG_CenterPrint(s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
	}
}

//set the local timing bar
extern int cg_genericTimerBar;
extern int cg_genericTimerDur;
extern vec4_t cg_genericTimerColor;
void CG_LocalTimingBar(int startTime, int duration)
{
    cg_genericTimerBar = startTime + duration;
	cg_genericTimerDur = duration;

	cg_genericTimerColor[0] = 1.0f;
	cg_genericTimerColor[1] = 1.0f;
	cg_genericTimerColor[2] = 0.0f;
	cg_genericTimerColor[3] = 1.0f;
}


/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;

	//rww - print pickup messages
	if (bg_itemlist[itemNum].classname && bg_itemlist[itemNum].classname[0] &&
		(bg_itemlist[itemNum].giType != IT_TEAM || (bg_itemlist[itemNum].giTag != PW_REDFLAG && bg_itemlist[itemNum].giTag != PW_BLUEFLAG)) )
	{ //don't print messages for flags, they have their own pickup event broadcasts
		char	text[1024];
		char	upperKey[1024];

		strcpy(upperKey, bg_itemlist[itemNum].classname);

		if ( trap->SE_GetStringTextString( va("SP_INGAME_%s",Q_strupr(upperKey)), text, sizeof( text )))
		{
			Com_Printf("%s %s\n", CG_GetStringEdString("MP_INGAME", "PICKUPLINE"), text);
		}
		else
		{
			Com_Printf("%s %s\n", CG_GetStringEdString("MP_INGAME", "PICKUPLINE"), bg_itemlist[itemNum].classname);
		}
	}
}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25.wav";
	} else if ( health < 50 ) {
		snd = "*pain50.wav";
	} else if ( health < 75 ) {
		snd = "*pain75.wav";
	} else {
		snd = "*pain100.wav";
	}
	trap->S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection	^= 1;
}

extern qboolean BG_GetRootSurfNameWithVariant( void *ghoul2, const char *rootSurfName, char *returnSurfName, int returnSize );
void CG_ReattachLimb(centity_t *source)
{
	clientInfo_t *ci = NULL;
	
	if ( source->currentState.number >= MAX_CLIENTS )
	{
		ci = source->npcClient;
	}
	else
	{
		ci = &cgs.clientinfo[source->currentState.number];
	}
	if ( ci )
	{//re-apply the skin
		if ( ci->torsoSkin > 0 )
		{
			trap->G2API_SetSkin(source->ghoul2,0,ci->torsoSkin,ci->torsoSkin);
		}
	}
		
	/*
	char *limbName;
	char *stubCapName;
	int i = G2_MODELPART_HEAD;

	//rww NOTE: Assumes G2_MODELPART_HEAD is first and G2_MODELPART_RLEG is last
	while (i <= G2_MODELPART_RLEG)
	{
		if (source->torsoBolt & (1 << (i-10)))
		{
			switch (i)
			{
			case G2_MODELPART_HEAD:
				limbName = "head";
				stubCapName = "torso_cap_head";
				break;
			case G2_MODELPART_WAIST:
				limbName = "torso";
				stubCapName = "hips_cap_torso";
				break;
			case G2_MODELPART_LARM:
				limbName = "l_arm";
				stubCapName = "torso_cap_l_arm";
				break;
			case G2_MODELPART_RARM:
				limbName = "r_arm";
				stubCapName = "torso_cap_r_arm";
				break;
			case G2_MODELPART_RHAND:
				limbName = "r_hand";
				stubCapName = "r_arm_cap_r_hand";
				break;
			case G2_MODELPART_LLEG:
				limbName = "l_leg";
				stubCapName = "hips_cap_l_leg";
				break;
			case G2_MODELPART_RLEG:
				limbName = "r_leg";
				stubCapName = "hips_cap_r_leg";
				break;
			default:
				source->torsoBolt = 0;
				source->ghoul2weapon = NULL;
				return;
			}

			trap->G2API_SetSurfaceOnOff(source->ghoul2, limbName, 0);
			trap->G2API_SetSurfaceOnOff(source->ghoul2, stubCapName, 0x00000100);
		}
		i++;
	}
	*/
	source->torsoBolt = 0;

	source->ghoul2weapon = NULL;
}

const char *CG_TeamName(int team)
{
	if (team==TEAM_RED)
		return "RED";
	else if (team==TEAM_BLUE)
		return "BLUE";
	else if (team==TEAM_SPECTATOR)
		return "SPECTATOR";
	return "FREE";
}

void CG_PrintCTFMessage(clientInfo_t *ci, const char *teamName, int ctfMessage)
{
	char printMsg[1024];
	char *refName = NULL;
	const char *psStringEDString = NULL;

	switch (ctfMessage)
	{
	case CTFMESSAGE_FRAGGED_FLAG_CARRIER:
		refName = "FRAGGED_FLAG_CARRIER";
		break;
	case CTFMESSAGE_FLAG_RETURNED:
		refName = "FLAG_RETURNED";
		break;
	case CTFMESSAGE_PLAYER_RETURNED_FLAG:
		refName = "PLAYER_RETURNED_FLAG";
		break;
	case CTFMESSAGE_PLAYER_CAPTURED_FLAG:
		refName = "PLAYER_CAPTURED_FLAG";
		break;
	case CTFMESSAGE_PLAYER_GOT_FLAG:
		refName = "PLAYER_GOT_FLAG";
		break;
	default:
		return;
	}

	psStringEDString = CG_GetStringEdString("MP_INGAME", refName);

	if (!psStringEDString || !psStringEDString[0])
	{
		return;
	}

	if (teamName && teamName[0])
	{
		const char *f = strstr(psStringEDString, "%s");

		if (f)
		{
			int strLen = 0;
			int i = 0;

			if (ci)
			{
				//--futuza: possible ^xRGB fix, needs testing, but this concerns ctf
				Com_sprintf(printMsg, sizeof(printMsg), "%s^7 ", ci->name);
				strLen = strlen(printMsg);
			}

			while (psStringEDString[i] && i < 512)
			{
				if (psStringEDString[i] == '%' &&
					psStringEDString[i+1] == 's')
				{
					printMsg[strLen] = '\0';
					Q_strcat(printMsg, sizeof(printMsg), teamName);
					strLen = strlen(printMsg);

					i++;
				}
				else
				{
					printMsg[strLen] = psStringEDString[i];
					strLen++;
				}

				i++;
			}

			printMsg[strLen] = '\0';

			goto doPrint;
		}
	}

	if (ci)
	{
		Com_sprintf(printMsg, sizeof(printMsg), "%s^7 %s", ci->name, psStringEDString);
	}
	else
	{
		Com_sprintf(printMsg, sizeof(printMsg), "%s", psStringEDString);
	}

doPrint:
	Com_Printf("%s\n", printMsg);
}

void CG_GetCTFMessageEvent(entityState_t *es)
{
	int clIndex = es->trickedentindex;
	int teamIndex = es->trickedentindex2;
	clientInfo_t *ci = NULL;
	const char *teamName = NULL;

	if (clIndex < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[clIndex];
	}

	if (teamIndex < 50)
	{
		teamName = CG_TeamName(teamIndex);
	}

	if (!ci)
	{
		return;
	}

	CG_PrintCTFMessage(ci, teamName, es->eventParm);
}

qboolean BG_InKnockDownOnly( int anim );

void DoFall(centity_t *cent, entityState_t *es, int clientNum)
{
	int delta = es->eventParm;

	if (cent->currentState.eFlags & EF_DEAD)
	{ //corpses crack into the ground ^_^
		if (delta > 25)
		{
			trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.fallSound );
		}
		else
		{
			trap->S_StartSound (NULL, es->number, CHAN_AUTO, trap->S_RegisterSound( "sound/movers/objects/objectHit.wav" ) );
		}
	}
	else if (BG_InKnockDownOnly(es->legsAnim))
	{
		if (delta > 14)
		{
			trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.fallSound );
		}
		else
		{
			trap->S_StartSound (NULL, es->number, CHAN_AUTO, trap->S_RegisterSound( "sound/movers/objects/objectHit.wav" ) );
		}
	}
	else if (delta > 50)
	{
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.fallSound );
		trap->S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
			CG_CustomSound( cent->currentState.number, "*land1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
	}
	else if (delta > 44)
	{
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.fallSound );
		trap->S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
			CG_CustomSound( cent->currentState.number, "*land1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
	}
	else
	{
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
	}
	
	if ( clientNum == cg.predictedPlayerState.clientNum )
	{
		// smooth landing z changes
		cg.landChange = -delta;
		if (cg.landChange > 32)
		{
			cg.landChange = 32;
		}
		if (cg.landChange < -32)
		{
			cg.landChange = -32;
		}
		cg.landTime = cg.time;
	}
}

int CG_InClientBitflags(entityState_t *ent, int client)
{
	int checkIn;
	int sub = 0;

	if (client > 47)
	{
		checkIn = ent->trickedentindex4;
		sub = 48;
	}
	else if (client > 31)
	{
		checkIn = ent->trickedentindex3;
		sub = 32;
	}
	else if (client > 15)
	{
		checkIn = ent->trickedentindex2;
		sub = 16;
	}
	else
	{
		checkIn = ent->trickedentindex;
	}

	if (checkIn & (1 << (client-sub)))
	{
		return 1;
	}
	
	return 0;
}

void CG_PlayDoorLoopSound( centity_t *cent );
void CG_PlayDoorSound( centity_t *cent, int type );

void CG_TryPlayCustomSound( vec3_t origin, int entityNum, int channel, const char *soundName )
{
	sfxHandle_t cSound = CG_CustomSound(entityNum, soundName);

	if (cSound <= 0)
	{
		return;
	}

	trap->S_StartSound(origin, entityNum, channel, cSound);
}

void CG_G2MarkEvent(entityState_t *es)
{
	//es->origin should be the hit location of the projectile,
	//whereas es->origin2 is the predicted position of the
	//projectile. (based on the trajectory upon impact) -rww
	centity_t *pOwner = &cg_entities[es->otherEntityNum];
	vec3_t startPoint;
	float	size = 0.0f;
	qhandle_t shader = 0;

	if (!pOwner->ghoul2)
	{ //can't do anything then...
		return;
	}

	//es->eventParm being non-0 means to do a special trace check
	//first. This will give us an impact right at the surface to
	//project the mark on. Typically this is used for radius
	//explosions and such, where the source position could be
	//way outside of model space.
	if (es->eventParm)
	{
		trace_t tr;
		int ignore = ENTITYNUM_NONE;

		CG_G2Trace(&tr, es->origin, NULL, NULL, es->origin2, ignore, MASK_PLAYERSOLID);

		if (tr.entityNum != es->otherEntityNum)
		{ //try again if we hit an ent but not the one we wanted.
			//CG_TestLine(es->origin, es->origin2, 2000, 0x0000ff, 1);
			if (tr.entityNum < ENTITYNUM_WORLD)
			{
				ignore = tr.entityNum;
				CG_G2Trace(&tr, es->origin, NULL, NULL, es->origin2, ignore, MASK_PLAYERSOLID);
				if (tr.entityNum != es->otherEntityNum)
				{ //try extending the trace a bit.. or not
					/*
					vec3_t v;

					VectorSubtract(es->origin2, es->origin, v);
					VectorScale(v, 64.0f, v);
					VectorAdd(es->origin2, v, es->origin2);

					CG_G2Trace(&tr, es->origin, NULL, NULL, es->origin2, ignore, MASK_PLAYERSOLID);
					if (tr.entityNum != es->otherEntityNum)
					{
						return;
					}
					*/
					//didn't manage to collide with the desired person. No mark will be placed then.
					return;
				}
			}
		}

		//otherwise we now have a valid starting point.
		VectorCopy(tr.endpos, startPoint);
	}
	else
	{
		VectorCopy(es->origin, startPoint);
	}

	if ( (es->eFlags&EF_JETPACK_ACTIVE) )
	{// a vehicle weapon, make it a larger size mark
		//OR base this on the size of the thing you hit?
		size = flrand( 32.0f, 72.0f );
	}
	switch(es->weapon)
	{
	case WP_BRYAR_PISTOL:
	case WP_CONCUSSION:
	case WP_BRYAR_OLD:
	case WP_BLASTER:
	case WP_DISRUPTOR:
	case WP_BOWCASTER:
	case WP_REPEATER:
	case WP_TURRET:
		if ( !size )
		{
			size = 4.0f;
		}
		if ( !shader )
		{
			shader = cgs.media.bdecal_bodyburn1;
		}
		CG_AddGhoul2Mark(shader, size, 
			startPoint, es->origin2, es->owner, pOwner->lerpOrigin,
			pOwner->lerpAngles[YAW], pOwner->ghoul2,
			pOwner->modelScale, Q_irand(10000, 20000));
		break;
	case WP_ROCKET_LAUNCHER:
	case WP_THERMAL:
		if ( !size )
		{
			size = 24.0f;
		}
		if ( !shader )
		{
			shader = cgs.media.bdecal_burn1;
		}
		CG_AddGhoul2Mark(shader, size, 
			startPoint, es->origin2, es->owner, pOwner->lerpOrigin,
			pOwner->lerpAngles[YAW], pOwner->ghoul2,
			pOwner->modelScale, Q_irand(10000, 20000));
		break;
	default:
		break;
	}
}

const char	*cg_stringEdVoiceChatTable[MAX_CUSTOM_SIEGE_SOUNDS] = 
{
	"VC_ATT",//"*att_attack",
	"VC_ATT_PRIMARY",//"*att_primary",
	"VC_ATT_SECONDARY",//"*att_second",
	"VC_DEF_GUNS",//"*def_guns",
	"VC_DEF_POSITION",//"*def_position",
	"VC_DEF_PRIMARY",//"*def_primary",
	"VC_DEF_SECONDARY",//"*def_second",
	"VC_REPLY_COMING",//"*reply_coming",
	"VC_REPLY_GO",//"*reply_go",
	"VC_REPLY_NO",//"*reply_no",
	"VC_REPLY_STAY",//"*reply_stay",
	"VC_REPLY_YES",//"*reply_yes",
	"VC_REQ_ASSIST",//"*req_assist",
	"VC_REQ_DEMO",//"*req_demo",
	"VC_REQ_HVY",//"*req_hvy",
	"VC_REQ_MEDIC",//"*req_medic",
	"VC_REQ_SUPPLY",//"*req_sup",
	"VC_REQ_TECH",//"*req_tech",
	"VC_SPOT_AIR",//"*spot_air",
	"VC_SPOT_DEF",//"*spot_defenses",
	"VC_SPOT_EMPLACED",//"*spot_emplaced",
	"VC_SPOT_SNIPER",//"*spot_sniper",
	"VC_SPOT_TROOP",//"*spot_troops",
	"VC_TAC_COVER",//"*tac_cover",
	"VC_TAC_FALLBACK",//"*tac_fallback",
	"VC_TAC_FOLLOW",//"*tac_follow",
	"VC_TAC_HOLD",//"*tac_hold",
	"VC_TAC_SPLIT",//"*tac_split",
	"VC_TAC_TOGETHER",//"*tac_together",
	NULL
};

//stupid way of figuring out what string to use for voice chats
const char *CG_GetStringForVoiceSound(const char *s)
{
	int i = 0;
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (bg_customSiegeSoundNames[i] &&
			!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //get the matching reference name
			assert(cg_stringEdVoiceChatTable[i]);
			return CG_GetStringEdString("MENUS", (char *)cg_stringEdVoiceChatTable[i]);
		}
		i++;
	}

	return "voice chat";
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/

#define	DEBUGNAME(x) if(cg_debugEvents.integer){trap->Print(x"\n");}
#define	DEBUGNAME2(x, y) if(cg_debugEvents.integer){trap->Print(x"\n", y);}
extern void CG_ChatBox_AddString(char *chatStr, int fadeLevel); //cg_draw.c
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;
	int				eID = 0;
	int				isnd = 0;
	centity_t		*cl_ent;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		trap->Print( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}

	if (es->eType == ET_NPC)
	{
		clientNum = es->number;

		if (!cent->npcClient)
		{
			CG_CreateNPCClient(&cent->npcClient); //allocate memory for it

			if (!cent->npcClient)
			{
				assert(0);
				return;
			}

			memset(cent->npcClient, 0, sizeof(clientInfo_t));
			cent->npcClient->ghoul2Model = NULL;
		}

		ci = cent->npcClient;

		assert(ci);
	}
	else
	{
		ci = &cgs.clientinfo[ clientNum ];
	}

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_CLIENTJOIN:
		DEBUGNAME("EV_CLIENTJOIN");

		//Slight hack to force a local reinit of client entity on join.
		cl_ent = &cg_entities[es->eventParm];

		if (cl_ent)
		{
			//cl_ent->torsoBolt = 0;
			cl_ent->bolt1 = 0;
			cl_ent->bolt2 = 0;
			cl_ent->bolt3 = 0;
			cl_ent->bolt4 = 0;
			cl_ent->bodyHeight = 0;//SABER_LENGTH_MAX;
			//cl_ent->saberExtendTime = 0;
			cl_ent->boltInfo = 0;
			cl_ent->frame_minus1_refreshed = 0;
			cl_ent->frame_minus2_refreshed = 0;
			cl_ent->frame_hold_time = 0;
			cl_ent->frame_hold_refreshed = 0;
			cl_ent->trickAlpha = 0;
			cl_ent->trickAlphaTime = 0;
			cl_ent->ghoul2weapon = NULL;
			cl_ent->weapon = WP_NONE;
			cl_ent->teamPowerEffectTime = 0;
			cl_ent->teamPowerType = 0;
			cl_ent->numLoopingSounds = 0;
			//cl_ent->localAnimIndex = 0;
		}
		break;

	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			footstep_t	soundType;
			switch( es->eventParm )
			{
			case MATERIAL_MUD:
				soundType = FOOTSTEP_MUDWALK;
				break;
			case MATERIAL_DIRT:			
				soundType = FOOTSTEP_DIRTWALK;
				break;
			case MATERIAL_SAND:			
				soundType = FOOTSTEP_SANDWALK;
				break;
			case MATERIAL_SNOW:			
				soundType = FOOTSTEP_SNOWWALK;
				break;
			case MATERIAL_SHORTGRASS:		
			case MATERIAL_LONGGRASS:		
				soundType = FOOTSTEP_GRASSWALK;
				break;
			case MATERIAL_SOLIDMETAL:		
				soundType = FOOTSTEP_METALWALK;
				break;
			case MATERIAL_HOLLOWMETAL:	
				soundType = FOOTSTEP_PIPEWALK;
				break;
			case MATERIAL_GRAVEL:
				soundType = FOOTSTEP_GRAVELWALK;
				break;
			case MATERIAL_CARPET:
			case MATERIAL_FABRIC:
			case MATERIAL_CANVAS:
			case MATERIAL_RUBBER:
			case MATERIAL_PLASTIC:
				soundType = FOOTSTEP_RUGWALK;
				break;
			case MATERIAL_SOLIDWOOD:
			case MATERIAL_HOLLOWWOOD:
				soundType = FOOTSTEP_WOODWALK;
				break;

			default:
				soundType = FOOTSTEP_STONEWALK;
				break;
			}

			trap->S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.footsteps[ soundType ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap->S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METALWALK ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap->S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap->S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap->S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL:
		DEBUGNAME("EV_FALL");
		if (es->number == cg.snap->ps.clientNum && cg.snap->ps.fallingToDeath)
		{
			break;
		}
		DoFall(cent, es, clientNum);
		break;
	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_noPredict.integer || g_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
		break;

	case EV_GHOUL2_MARK:
		DEBUGNAME("EV_GHOUL2_MARK");

		if (cg_ghoul2Marks.integer)
		{ //Can we put a burn mark on him?
			CG_G2MarkEvent(es);
		}
		break;

	case EV_GLOBAL_DUEL:
		DEBUGNAME("EV_GLOBAL_DUEL");
		//used for beginning of power duels
		//if (cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_SPECTATOR)
		if (es->otherEntityNum == cg.predictedPlayerState.clientNum ||
			es->otherEntityNum2 == cg.predictedPlayerState.clientNum ||
			es->groundEntityNum == cg.predictedPlayerState.clientNum)
		{
			CG_CenterPrint( CG_GetStringEdString("MP_SVGAME", "BEGIN_DUEL"), 120, GIANTCHAR_WIDTH*2 );				
			trap->S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		}
		break;

	case EV_PRIVATE_DUEL:
		DEBUGNAME("EV_PRIVATE_DUEL");

		if (cg.snap->ps.clientNum != es->number)
		{
			break;
		}

		if (es->eventParm)
		{ //starting the duel
			if (es->eventParm == 2)
			{
				CG_CenterPrint( CG_GetStringEdString("MP_SVGAME", "BEGIN_DUEL"), 120, GIANTCHAR_WIDTH*2 );				
				trap->S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
			}
			else
			{
				trap->S_StartBackgroundTrack( "music/mp/duel.mp3", "music/mp/duel.mp3", qfalse );
			}
		}
		else
		{ //ending the duel
			CG_StartMusic(qtrue);
		}
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		if (cg_jumpSounds.integer)
		{
			trap->S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		}
		break;
	case EV_ROLL:
		DEBUGNAME("EV_ROLL");
		if (es->number == cg.snap->ps.clientNum && cg.snap->ps.fallingToDeath)
		{
			break;
		}
		if (es->eventParm)
		{ //fall-roll-in-one event
			DoFall(cent, es, clientNum);
		}

		trap->S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		trap->S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.rollSound  );

		//FIXME: need some sort of body impact on ground sound and maybe kick up some dust?
		break;

	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		{
			int soundIndex = 0;
			if ( cgs.gametype != GT_DUEL
				&& cgs.gametype != GT_POWERDUEL
				&& es->eventParm == TAUNT_TAUNT )
			{//normal taunt
				soundIndex = CG_CustomSound( es->number, "*taunt.wav" );
			}
			else
			{
				switch ( es->eventParm )
				{
				case TAUNT_TAUNT:
				default:
					if ( Q_irand( 0, 1 ) )
					{
						soundIndex = CG_CustomSound( es->number, va("*anger%d.wav", Q_irand(1,3)) );
					}
					else
					{
						soundIndex = CG_CustomSound( es->number, va("*taunt%d.wav", Q_irand(1,3)) );
						if ( !soundIndex )
						{
							soundIndex = CG_CustomSound( es->number, va("*anger%d.wav", Q_irand(1,3)) );
						}
					}
					break;
				case TAUNT_BOW:
					//soundIndex = CG_CustomSound( es->number, va("*respect%d.wav", Q_irand(1,3)) );
					break;
				case TAUNT_MEDITATE:
					//soundIndex = CG_CustomSound( es->number, va("*meditate%d.wav", Q_irand(1,3)) );
					break;
				case TAUNT_FLOURISH:
					if ( Q_irand( 0, 1 ) )
					{
						soundIndex = CG_CustomSound( es->number, va("*deflect%d.wav", Q_irand(1,3)) );
						if ( !soundIndex )
						{
							soundIndex = CG_CustomSound( es->number, va("*gloat%d.wav", Q_irand(1,3)) );
							if ( !soundIndex )
							{
								soundIndex = CG_CustomSound( es->number, va("*anger%d.wav", Q_irand(1,3)) );
							}
						}
					}
					else
					{
						soundIndex = CG_CustomSound( es->number, va("*gloat%d.wav", Q_irand(1,3)) );
						if ( !soundIndex )
						{
							soundIndex = CG_CustomSound( es->number, va("*deflect%d.wav", Q_irand(1,3)) );
							if ( !soundIndex )
							{
								soundIndex = CG_CustomSound( es->number, va("*anger%d.wav", Q_irand(1,3)) );
							}
						}
					}
					break;
				case TAUNT_GLOAT:
					soundIndex = CG_CustomSound( es->number, va("*victory%d.wav", Q_irand(1,3)) );
					break;
				}
			}
			if ( !soundIndex )
			{
				soundIndex = CG_CustomSound( es->number, "*taunt.wav" );
			}
			if ( soundIndex )
			{
				trap->S_StartSound (NULL, es->number, CHAN_VOICE, soundIndex );
			}
		}
		break;

		//Begin NPC sounds
	case EV_ANGER1:	//Say when acquire an enemy when didn't have one before
	case EV_ANGER2:
	case EV_ANGER3:
		DEBUGNAME("EV_ANGERx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*anger%i.wav", event - EV_ANGER1 + 1) );
		break;

	case EV_VICTORY1:	//Say when killed an enemy
	case EV_VICTORY2:
	case EV_VICTORY3:
		DEBUGNAME("EV_VICTORYx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*victory%i.wav", event - EV_VICTORY1 + 1) );
		break;

	case EV_CONFUSE1:	//Say when confused
	case EV_CONFUSE2:
	case EV_CONFUSE3:
		DEBUGNAME("EV_CONFUSEDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*confuse%i.wav", event - EV_CONFUSE1 + 1) );
		break;

	case EV_PUSHED1:	//Say when pushed
	case EV_PUSHED2:
	case EV_PUSHED3:
		DEBUGNAME("EV_PUSHEDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*pushed%i.wav", event - EV_PUSHED1 + 1) );
		break;

	case EV_CHOKE1:	//Say when choking
	case EV_CHOKE2:
	case EV_CHOKE3:
		DEBUGNAME("EV_CHOKEx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*choke%i.wav", event - EV_CHOKE1 + 1) );
		break;

	case EV_FFWARN:	//Warn ally to stop shooting you
		DEBUGNAME("EV_FFWARN");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, "*ffwarn.wav" );
		break;

	case EV_FFTURN:	//Turn on ally after being shot by them
		DEBUGNAME("EV_FFTURN");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, "*ffturn.wav" );
		break;

	//extra sounds for ST
	case EV_CHASE1:
	case EV_CHASE2:
	case EV_CHASE3:
		DEBUGNAME("EV_CHASEx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*chase%i.wav", event - EV_CHASE1 + 1) );
		break;
	case EV_COVER1:
	case EV_COVER2:
	case EV_COVER3:
	case EV_COVER4:
	case EV_COVER5:
		DEBUGNAME("EV_COVERx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*cover%i.wav", event - EV_COVER1 + 1) );
		break;
	case EV_DETECTED1:
	case EV_DETECTED2:
	case EV_DETECTED3:
	case EV_DETECTED4:
	case EV_DETECTED5:
		DEBUGNAME("EV_DETECTEDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*detected%i.wav", event - EV_DETECTED1 + 1) );
		break;
	case EV_GIVEUP1:
	case EV_GIVEUP2:
	case EV_GIVEUP3:
	case EV_GIVEUP4:
		DEBUGNAME("EV_GIVEUPx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*giveup%i.wav", event - EV_GIVEUP1 + 1) );
		break;
	case EV_LOOK1:
	case EV_LOOK2:
		DEBUGNAME("EV_LOOKx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*look%i.wav", event - EV_LOOK1 + 1) );
		break;
	case EV_LOST1:
		DEBUGNAME("EV_LOST1");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, "*lost1.wav" );
		break;
	case EV_OUTFLANK1:
	case EV_OUTFLANK2:
		DEBUGNAME("EV_OUTFLANKx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*outflank%i.wav", event - EV_OUTFLANK1 + 1) );
		break;
	case EV_ESCAPING1:
	case EV_ESCAPING2:
	case EV_ESCAPING3:
		DEBUGNAME("EV_ESCAPINGx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*escaping%i.wav", event - EV_ESCAPING1 + 1) );
		break;
	case EV_SIGHT1:
	case EV_SIGHT2:
	case EV_SIGHT3:
		DEBUGNAME("EV_SIGHTx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*sight%i.wav", event - EV_SIGHT1 + 1) );
		break;
	case EV_SOUND1:
	case EV_SOUND2:
	case EV_SOUND3:
		DEBUGNAME("EV_SOUNDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*sound%i.wav", event - EV_SOUND1 + 1) );
		break;
	case EV_SUSPICIOUS1:
	case EV_SUSPICIOUS2:
	case EV_SUSPICIOUS3:
	case EV_SUSPICIOUS4:
	case EV_SUSPICIOUS5:
		DEBUGNAME("EV_SUSPICIOUSx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*suspicious%i.wav", event - EV_SUSPICIOUS1 + 1) );
		break;
	//extra sounds for Jedi
	case EV_COMBAT1:
	case EV_COMBAT2:
	case EV_COMBAT3:
		DEBUGNAME("EV_COMBATx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*combat%i.wav", event - EV_COMBAT1 + 1) );
		break;
	case EV_JDETECTED1:
	case EV_JDETECTED2:
	case EV_JDETECTED3:
		DEBUGNAME("EV_JDETECTEDx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*jdetected%i.wav", event - EV_JDETECTED1 + 1) );
		break;
	case EV_TAUNT1:
	case EV_TAUNT2:
	case EV_TAUNT3:
		DEBUGNAME("EV_TAUNTx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*taunt%i.wav", event - EV_TAUNT1 + 1) );
		break;
	case EV_JCHASE1:
	case EV_JCHASE2:
	case EV_JCHASE3:
		DEBUGNAME("EV_JCHASEx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*jchase%i.wav", event - EV_JCHASE1 + 1) );
		break;
	case EV_JLOST1:
	case EV_JLOST2:
	case EV_JLOST3:
		DEBUGNAME("EV_JLOSTx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*jlost%i.wav", event - EV_JLOST1 + 1) );
		break;
	case EV_DEFLECT1:
	case EV_DEFLECT2:
	case EV_DEFLECT3:
		DEBUGNAME("EV_DEFLECTx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*deflect%i.wav", event - EV_DEFLECT1 + 1) );
		break;
	case EV_GLOAT1:
	case EV_GLOAT2:
	case EV_GLOAT3:
		DEBUGNAME("EV_GLOATx");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, va("*gloat%i.wav", event - EV_GLOAT1 + 1) );
		break;
	case EV_PUSHFAIL:
		DEBUGNAME("EV_PUSHFAIL");
		CG_TryPlayCustomSound( NULL, es->number, CHAN_VOICE, "*pushfail.wav" );
		break;
		//End NPC sounds
		
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			if ( /*item->giType != IT_POWERUP && */item->giType != IT_TEAM) {
				if (item->pickup_sound && item->pickup_sound[0])
				{
					trap->S_StartSound (NULL, es->number, CHAN_AUTO, trap->S_RegisterSound( item->pickup_sound ) );
				}
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound && item->pickup_sound[0] ) {
				trap->S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap->S_RegisterSound( item->pickup_sound) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum )
		{
			if ( cg.snap->ps.weapon == WP_SABER )
			{
				cg.forceHUDTotalFlashTime = cg.time + 1000;
			}
			else
			{
				trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
			}
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		{
		    int param = es->eventParm;
			int weapon;
			int variation;
			weaponInfo_t *weaponInfo;
			weaponData_t *weaponData;
			qboolean result;

			assert(param >= 0 && param < MAX_WEAPON_TABLE_SIZE);
			
 			result = BG_GetWeaponByIndex (param, &weapon, &variation);
			assert (result);

			weaponData = GetWeaponData( weapon, variation );

			assert( weaponData );
			
			#ifdef _DEBUG
			//trap->Print("Client-side received change weapon event: weapon %d, variation %d\n", weapon, variation);
			#endif

			weaponInfo = CG_WeaponInfo (weapon, variation);

			assert(weaponInfo);

			if(weaponData->visuals.selectSound[0])
			{
				trap->S_StartSound( NULL, es->number, CHAN_AUTO, trap->S_RegisterSound(weaponData->visuals.selectSound) );
			}
			else if (weapon != WP_SABER)
			{ //not sure what SP is doing for this but I don't want a select sound for saber (it has the saber-turn-on)
				trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
			}

			// If we're changing to a saber, we need to make sure that we change our client's saber to match.
			if( weapon == WP_SABER )
			{
				WP_SetSaber( es->number, cgs.clientinfo[es->number].saber, 0, weaponData->sab.hiltname );
				JKG_SwapToSaber( 0, &cgs.clientinfo[es->number], weaponData->sab.hiltname, weapon, variation );

				const saberCrystalData_t* crystal = JKG_GetSaberCrystal(weaponData->sab.defaultcrystal);
				if(crystal)
				{
					cg.predictedPlayerState.saberCrystal[0] = crystal->crystalID;
					cg_entities[es->number].currentState.saberCrystal[0] = crystal->crystalID;

					//--futuza todo: handle additional sabers eg: cg.predictedPlayerState.saberCrystal[1]
				}


				//CG_InitG2SaberData( 0, &cgs.clientinfo[es->number] );
				cgs.clientinfo[es->number].saber[0].SetDesiredLength(0, -1);
				cgs.clientinfo[es->number].saber[1].SetDesiredLength(0, -1);

				if( cg_entities[es->number].currentState.saberHolstered == 2 || 
					cg_entities[es->number].currentState.weapon == WP_NONE ||
					cg_entities[es->number].currentState.weapon == WP_MELEE )
				{
					trap->S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
				}
			}
		}
		break;
	case EV_RELOAD:
		DEBUGNAME("EV_RELOAD");
		{
			int weapon = es->eventParm;
			weaponInfo_t *weaponInfo;
			
			assert(weapon >= 0 && weapon < MAX_WEAPONS);

			weaponInfo = CG_WeaponInfo (weapon, 0);

			assert(weaponInfo);

			if (weaponInfo->selectSound)
			{
				trap->S_StartSound (NULL, es->time, CHAN_AUTO, weaponInfo->selectSound );
			}
			else
			{ //not sure what SP is doing for this but I don't want a select sound for saber (it has the saber-turn-on)
				trap->S_StartSound (NULL, es->time, CHAN_AUTO, cgs.media.selectSound );
			}
			// Check if its us that's reloading, so we can show the reload bar :o
			if (es->time == cg.snap->ps.clientNum) {
				cg.reloadTimeStart = cg.time;
				cg.reloadTimeDuration = GetWeaponData( weapon, es->weaponVariation )->weaponReloadTime;
				
				//cg.viewWeaponAnimation = &weaponInfo->viewModelAnims[E11_IDLE];
			}
		}
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");

		if ( cg.jkg_grenadeCookTimer && cent->currentState.number == cg.clientNum )
		{
			cg.jkg_grenadeCookTimer = 0;
		}

		if (cent->currentState.number >= MAX_CLIENTS && cent->currentState.eType != ET_NPC)
		{ //special case for turret firing
			vec3_t gunpoint, gunangle;
			mdxaBone_t matrix;

			//weaponInfo_t *weaponInfo = CG_WeaponInfo (WP_TURRET, 0);

			if (cent->ghoul2)
			{
				if (!cent->bolt1)
				{
					cent->bolt1 = trap->G2API_AddBolt(cent->ghoul2, 0, "*flash01");
				}
				if (!cent->bolt2)
				{
					cent->bolt2 = trap->G2API_AddBolt(cent->ghoul2, 0, "*flash02");
				}
				trap->G2API_SetBoneAnim(cent->ghoul2, 0, "Bone02", 1, 4, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND,
					1.0f, cg.time, -1, 300);
			}
			else
			{
				break;
			}

			if (cent->currentState.eventParm)
			{
				trap->G2API_GetBoltMatrix(cent->ghoul2, 0, cent->bolt2, &matrix, cent->currentState.angles, cent->currentState.origin, cg.time, cgs.gameModels, cent->modelScale);
			}
			else
			{
				trap->G2API_GetBoltMatrix(cent->ghoul2, 0, cent->bolt1, &matrix, cent->currentState.angles, cent->currentState.origin, cg.time, cgs.gameModels, cent->modelScale);
			}

			gunpoint[0] = matrix.matrix[0][3];
			gunpoint[1] = matrix.matrix[1][3] + 10.0f;
			gunpoint[2] = matrix.matrix[2][3];

			gunangle[0] = -matrix.matrix[0][0];
			gunangle[1] = -matrix.matrix[1][0];
			gunangle[2] = -matrix.matrix[2][0];

			trap->FX_PlayEffectID(cgs.effects.mEmplacedMuzzleFlash, gunpoint, gunangle, -1, -1, false);
		}
		else if (cent->currentState.weapon != WP_EMPLACED_GUN || cent->currentState.eType == ET_NPC)
		{
			JKG_FireWeapon (cent, qfalse);
		}
		break;

	case EV_ALT_FIRE:
		DEBUGNAME("EV_ALT_FIRE");

		if ( cg.jkg_grenadeCookTimer && cent->currentState.number == cg.clientNum )
		{
			cg.jkg_grenadeCookTimer = 0;
		}

		if (cent->currentState.weapon == WP_EMPLACED_GUN)
		{ //don't do anything for emplaced stuff
			break;
		}

        cent->currentState.origin[1] += 10.0f;
		JKG_FireWeapon (cent, qtrue);
		cent->currentState.origin[1] -= 10.0f;
		break;

	case EV_HEATCRIT:
		DEBUGNAME("EV_HEATCRIT")

		if (es->number == cg.snap->ps.clientNum)
		{
			qhandle_t warning = trap->S_RegisterSound(va("sound/weapons/common/heatWarning.wav"));
			trap->S_StartSound(es->pos.trBase, es->number, CHAN_WEAPON, warning);

		}
		break;

	case EV_OVERHEATED:
		DEBUGNAME("EV_OVERHEATED");

		if (es->number == cg.snap->ps.clientNum)
		{

			qhandle_t heatSound = trap->S_RegisterSound(va("sound/weapons/common/overheat0%i.wav", Q_irand(0, 3)));
			trap->S_StartSound(es->pos.trBase, es->number, CHAN_WEAPON, heatSound);
			//also steam comes off gun once in an overheated state see cg_weapons JKG_RenderOverheatEffect() function
		}
		break;

	case EV_HEATCOOLED:
		DEBUGNAME("EV_HEATCOOLED");
		//for when a weapon has cooled sufficiently (less than heatThreshold), 
		//this event is currently unused, but might be useful later (see w_force.cpp for current use of snd)
		if (es->number == cg.snap->ps.clientNum)
		{
			qhandle_t notify = trap->S_RegisterSound(va("sound/weapons/common/heatClear.wav"));
			trap->S_StartSound(es->pos.trBase, es->number, CHAN_WEAPON, notify);
		}
		break;

	case EV_SABER_ATTACK:
		DEBUGNAME("EV_SABER_ATTACK");
		{
			qhandle_t swingSound = trap->S_RegisterSound(va("sound/weapons/saber/saberhup%i.wav", Q_irand(1, 8)));
			clientInfo_t *client = NULL;
			if ( cg_entities[es->number].currentState.eType == ET_NPC )
			{
				client = cg_entities[es->number].npcClient;
			}
			else if ( es->number < MAX_CLIENTS )
			{
				client = &cgs.clientinfo[es->number];
			}
			if ( client && client->infoValid && client->saber[0].swingSound[0] )
			{//custom swing sound
				swingSound = client->saber[0].swingSound[Q_irand(0,2)];
			}
            trap->S_StartSound(es->pos.trBase, es->number, CHAN_WEAPON, swingSound );
		}
		break;

	case EV_SABER_HIT:
		DEBUGNAME("EV_SABER_HIT");
		{
			int hitPersonFxID = cgs.effects.mSaberBloodSparks;
			int hitPersonSmallFxID = cgs.effects.mSaberBloodSparksSmall;
			int hitPersonMidFxID = cgs.effects.mSaberBloodSparksMid;
			int hitOtherFxID = cgs.effects.mSaberCut;
			int hitSound = trap->S_RegisterSound(va("sound/weapons/saber/saberhit%i.wav", Q_irand(1, 3)));
			
			if ( es->otherEntityNum2 >= 0
				&& es->otherEntityNum2 < ENTITYNUM_NONE )
			{//we have a specific person who is causing this effect, see if we should override it with any custom saber effects/sounds
				clientInfo_t *client = NULL;
				if ( cg_entities[es->otherEntityNum2].currentState.eType == ET_NPC )
				{
					client = cg_entities[es->otherEntityNum2].npcClient;
				}
				else if ( es->otherEntityNum2 < MAX_CLIENTS )
				{
					client = &cgs.clientinfo[es->otherEntityNum2];
				}
				if ( client && client->infoValid )
				{
					int saberNum = es->weapon;

					if ( client->saber[saberNum].hitPersonEffect )
					{//custom hit person effect
						hitPersonFxID = hitPersonSmallFxID = hitPersonMidFxID = client->saber[saberNum].hitPersonEffect;
					}
					if ( client->saber[saberNum].hitOtherEffect )
					{//custom hit other effect
						hitOtherFxID = client->saber[0].hitOtherEffect;
					}
					if ( client->saber[saberNum].hitSound[0] )
					{//custom hit sound
						hitSound = client->saber[saberNum].hitSound[Q_irand(0,2)];
					}
				}
			}

			if (es->eventParm == 16)
			{ //Make lots of sparks, something special happened
				vec3_t fxDir;
				VectorCopy(es->angles, fxDir);
				if (!fxDir[0] && !fxDir[1] && !fxDir[2])
				{
					fxDir[1] = 1;
				}
				trap->S_StartSound(es->origin, es->number, CHAN_AUTO, hitSound );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
			}
			else if (es->eventParm)
			{ //hit a person
				vec3_t fxDir;
				VectorCopy(es->angles, fxDir);
				if (!fxDir[0] && !fxDir[1] && !fxDir[2])
				{
					fxDir[1] = 1;
				}
				trap->S_StartSound(es->origin, es->number, CHAN_AUTO, hitSound );
				if ( es->eventParm == 3 )
				{	// moderate or big hits.
					trap->FX_PlayEffectID( hitPersonSmallFxID, es->origin, fxDir, -1, -1, false );
				}
				else if ( es->eventParm == 2 )
				{	// this is for really big hits.
					trap->FX_PlayEffectID( hitPersonMidFxID, es->origin, fxDir, -1, -1, false );
				}
				else
				{	// this should really just be done in the effect itself, no?
					trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
					trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
					trap->FX_PlayEffectID( hitPersonFxID, es->origin, fxDir, -1, -1, false );
				}
			}
			else
			{ //hit something else
				vec3_t fxDir;
				VectorCopy(es->angles, fxDir);
				if (!fxDir[0] && !fxDir[1] && !fxDir[2])
				{
					fxDir[1] = 1;
				}
				//old jk2mp method
				/*
				trap->S_StartSound(es->origin, es->number, CHAN_AUTO, trap->S_RegisterSound("sound/weapons/saber/saberhit.wav"));
				trap->FX_PlayEffectID( trap->FX_RegisterEffect("saber/spark.efx"), es->origin, fxDir, -1, -1 );
				*/

				trap->FX_PlayEffectID( hitOtherFxID, es->origin, fxDir, -1, -1, false );
			}

			//rww - this means we have the number of the ent being hit and the ent that owns the saber doing
			//the hit. This being the case, we can store these indecies and the current time in order to do
			//some visual tricks on the client between frames to make it look like we're actually continuing
			//to hit between server frames.
			if (es->otherEntityNum != ENTITYNUM_NONE && es->otherEntityNum2 != ENTITYNUM_NONE)
			{
				centity_t *saberOwner;

				saberOwner = &cg_entities[es->otherEntityNum2];

				saberOwner->serverSaberHitIndex = es->otherEntityNum;
				saberOwner->serverSaberHitTime = cg.time;

				if (es->eventParm)
				{
					saberOwner->serverSaberFleshImpact = qtrue;
				}
				else
				{
					saberOwner->serverSaberFleshImpact = qfalse;
				}
			}
		}
		break;

	case EV_SABER_BLOCK:
		DEBUGNAME("EV_SABER_BLOCK");
		{
			if (es->eventParm)
			{ //saber block
				int			blockFXID = cgs.effects.mSaberBlock;
				qhandle_t	blockSound = trap->S_RegisterSound(va( "sound/weapons/saber/saberblock%d.wav", Q_irand(1, 9) ));
				qboolean	noFlare = qfalse;

				if ( es->otherEntityNum2 >= 0
					&& es->otherEntityNum2 < ENTITYNUM_NONE )
				{//we have a specific person who is causing this effect, see if we should override it with any custom saber effects/sounds
					clientInfo_t *client = NULL;
					if ( cg_entities[es->otherEntityNum2].currentState.eType == ET_NPC )
					{
						client = cg_entities[es->otherEntityNum2].npcClient;
					}
					else if ( es->otherEntityNum2 < MAX_CLIENTS )
					{
						client = &cgs.clientinfo[es->otherEntityNum2];
					}
					if ( client && client->infoValid )
					{
						int saberNum = es->weapon;

						if ( client->saber[saberNum].blockEffect )
						{//custom saber block effect
							blockFXID = client->saber[saberNum].blockEffect;
						}
						if ( client->saber[saberNum].blockSound[0] )
						{//custom hit sound
							blockSound = client->saber[saberNum].blockSound[Q_irand(0,2)];
						}
						if ( (client->saber[saberNum].saberFlags2&SFL2_NO_CLASH_FLARE) )
						{
							noFlare = qtrue;
						}
					}
				}

				vec3_t fxDir;

				VectorCopy(es->angles, fxDir);
				if (!fxDir[0] && !fxDir[1] && !fxDir[2])
				{
					fxDir[1] = 1;
				}
				trap->S_StartSound(es->origin, es->number, CHAN_AUTO, blockSound );
				trap->FX_PlayEffectID( blockFXID, es->origin, fxDir, -1, -1, false );

				if ( !noFlare )
				{
					cg_saberFlashTime = cg.time-50;
					VectorCopy( es->origin, cg_saberFlashPos );
				}
			}
			else
			{ //projectile block
				vec3_t fxDir;
				VectorCopy(es->angles, fxDir);
				if (!fxDir[0] && !fxDir[1] && !fxDir[2])
				{
					fxDir[1] = 1;
				}
				trap->FX_PlayEffectID(cgs.effects.mBlasterDeflect, es->origin, fxDir, -1, -1, false);
			}
		}
		break;

	case EV_SABER_CLASHFLARE:
		DEBUGNAME("EV_SABER_CLASHFLARE");
		{
			cg_saberFlashTime = cg.time-50;
			VectorCopy( es->origin, cg_saberFlashPos );
			trap->S_StartSound ( es->origin, -1, CHAN_WEAPON, trap->S_RegisterSound( va("sound/weapons/saber/saberhitwall%i", Q_irand(1, 3)) ) );
		}
		break;

	case EV_SABER_UNHOLSTER:
		DEBUGNAME("EV_SABER_UNHOLSTER");
		{
			clientInfo_t *ci = NULL;

			if (es->eType == ET_NPC)
			{
				ci = cg_entities[es->number].npcClient;
			}
			else if (es->number < MAX_CLIENTS)
			{
				ci = &cgs.clientinfo[es->number];
			}

			if (ci)
			{
				if (ci->saber[0].soundOn)
				{
					trap->S_StartSound (NULL, es->number, CHAN_AUTO, ci->saber[0].soundOn );
				}
				if (ci->saber[1].soundOn)
				{
					trap->S_StartSound (NULL, es->number, CHAN_AUTO, ci->saber[1].soundOn );
				}
			}
		}
		break;

	case EV_SABER_HOLSTER:
		DEBUGNAME("EV_SABER_HOLSTER");
		{
			clientInfo_t *ci = NULL;

			if (es->eType == ET_NPC)
			{
				ci = cg_entities[es->eventParm].npcClient;
			}
			else if (es->eventParm < MAX_CLIENTS)
			{
				ci = &cgs.clientinfo[es->eventParm];
			}

			if (ci)
			{
				if ( es->eFlags == 1 )
				{
					// Quick sound effect
					trap->S_StartSound( NULL, es->eventParm, CHAN_AUTO, trap->S_RegisterSound("sound/weapons/saber/saberoffquick.wav") );
				}
				else
				{
					if (ci->saber[0].soundOff)
					{
						trap->S_StartSound (NULL, es->eventParm, CHAN_AUTO, ci->saber[0].soundOff );
					}
					if (ci->saber[1].soundOff)
					{
						trap->S_StartSound (NULL, es->eventParm, CHAN_AUTO, ci->saber[1].soundOff );
					}
				}
			}
		}
		break;

	case EV_WEAPON_TRACELINE:
		DEBUGNAME("EV_WEAPON_TRACELINE");
		if (cent->currentState.owner != cg.snap->ps.clientNum ||
			//[TrueView]
			cg.renderingThirdPerson || cg_trueguns.integer 
			|| cg.predictedPlayerState.weapon == WP_SABER || cg.predictedPlayerState.weapon == WP_MELEE)
			//cg.renderingThirdPerson)
			//[/TrueView]
		{ //h4q3ry
			CG_GetClientWeaponMuzzleBoltPoint(cent->currentState.owner, cent->currentState.origin2);
		}
		else
		{
			if (cg.lastFPFlashPoint[0] ||cg.lastFPFlashPoint[1] || cg.lastFPFlashPoint[2])
			{ //get the position of the muzzle flash for the first person weapon model from the last frame
				VectorCopy(cg.lastFPFlashPoint, cent->currentState.origin2);
			}
		}
		JKG_RenderTraceline (cent, cent->currentState.origin2, cent->lerpOrigin, cent->currentState.firingMode);
		
		ByteToDir (cent->currentState.eventParm, dir);
		if ( cent->currentState.otherEntityNum < MAX_CLIENTS ||
		    cg_entities[cent->currentState.otherEntityNum].currentState.eType == ET_NPC )
		{
			JKG_RenderProjectileHitPlayer (cent, cent->lerpOrigin, dir, cent->currentState.firingMode);
		}
		else if ( cent->currentState.otherEntityNum != ENTITYNUM_NONE )
		{
			JKG_RenderProjectileMiss (cent, cent->lerpOrigin, dir, cent->currentState.firingMode);
		}
		break;

	case EV_ZOOM:
		DEBUGNAME("EV_ZOOM");
		if (es->number == cg.snap->ps.clientNum)
		{
			JKG_ToggleScope (cent);
		}
		break;
	case EV_PREDEFSOUND:
		DEBUGNAME("EV_PREDEFSOUND");
		{
			int sID = -1;

			switch (es->eventParm)
			{
			case PDSOUND_PROTECTHIT:
				sID = trap->S_RegisterSound("sound/weapons/force/protecthit.mp3");
				break;
			case PDSOUND_PROTECT:
				sID = trap->S_RegisterSound("sound/weapons/force/protect.mp3");
				break;
			case PDSOUND_ABSORBHIT:
				sID = trap->S_RegisterSound("sound/weapons/force/absorbhit.mp3");
				if (es->trickedentindex >= 0 && es->trickedentindex < MAX_CLIENTS)
				{
					int clnum = es->trickedentindex;

					cg_entities[clnum].teamPowerEffectTime = cg.time + 1000;
					cg_entities[clnum].teamPowerType = 3;
				}
				break;
			case PDSOUND_ABSORB:
				sID = trap->S_RegisterSound("sound/weapons/force/absorb.mp3");
				break;
			case PDSOUND_FORCEJUMP:
				sID = trap->S_RegisterSound("sound/weapons/force/jump.mp3");
				break;
			case PDSOUND_FORCEGRIP:
				sID = trap->S_RegisterSound("sound/weapons/force/grip.mp3");
				break;

			case PDSOUND_VENDORPURCHASE:
				sID = trap->S_RegisterSound("sound/vendor/generic/purchase00.mp3");	
				break;

			case PDSOUND_TRADE:
				sID = trap->S_RegisterSound("sound/vendor/generic/purchase01.mp3");		
				break;

			case PDSOUND_PAY:
				sID = trap->S_RegisterSound("sound/vendor/generic/purchase02.mp3");
				break;

			default:
				break;
			}

			if (sID != 1)
			{
				trap->S_StartSound(es->origin, es->number, CHAN_AUTO, sID);
			}
		}
		break;

	case EV_TEAM_POWER:
		DEBUGNAME("EV_TEAM_POWER");
		{
			int clnum = 0;

			while (clnum < MAX_CLIENTS)
			{
				if (CG_InClientBitflags(es, clnum))
				{
					if (es->eventParm == 1)
					{ //eventParm 1 is heal
						trap->S_StartSound (NULL, clnum, CHAN_AUTO, cgs.media.teamHealSound );
						cg_entities[clnum].teamPowerEffectTime = cg.time + 1000;
						cg_entities[clnum].teamPowerType = 1;
					}
					else
					{ //eventParm 2 is force regen
						trap->S_StartSound (NULL, clnum, CHAN_AUTO, cgs.media.teamRegenSound );
						cg_entities[clnum].teamPowerEffectTime = cg.time + 1000;
						cg_entities[clnum].teamPowerType = 0;
					}
				}
				clnum++;
			}
		}
		break;

	case EV_SCREENSHAKE:
		DEBUGNAME("EV_SCREENSHAKE");
		if (!es->modelindex || cg.predictedPlayerState.clientNum == es->modelindex-1)
		{
			CGCam_Shake(es->angles[0], es->time);
		}
		break;
	case EV_LOCALTIMER:
		DEBUGNAME("EV_LOCALTIMER");
		if (es->owner == cg.predictedPlayerState.clientNum)
		{
			CG_LocalTimingBar(es->time, es->time2);
		}
		break;

	//=================================================================

	//
	// other events
	//
	//Player teleporting
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		// Pande: is annoying, doesn't fit aesthetically, and the effect relies on gfx from Mace's stargate stuff.
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		// Pande: is annoying, doesn't fit aesthetically, and the effect relies on gfx from Mace's stargate stuff.
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		JKG_BounceGrenade (cent, es->firingMode);
		break;

	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	case EV_DAMAGEPLUM:
		DEBUGNAME("EV_DAMAGEPLUM");
		CG_DamagePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time, 
			cent->currentState.eventParm, cent->currentState.generic1, cent->currentState.groundEntityNum );
		break;

	case EV_CTFMESSAGE:
		DEBUGNAME("EV_CTFMESSAGE");
		CG_GetCTFMessageEvent(es);
		break;

	case EV_BODYFADE:
		if (es->eType != ET_BODY)
		{
			assert(!"EV_BODYFADE event from a non-corpse");
			break;
		}
		
		if (cent->ghoul2 && trap->G2_HaveWeGhoul2Models(cent->ghoul2))
		{
			//turn the inside of the face off, to avoid showing the mouth when we start alpha fading the corpse
			trap->G2API_SetSurfaceOnOff( cent->ghoul2, "head_eyes_mouth", 0x00000002/*G2SURFACEFLAG_OFF*/ );
		}

		cent->bodyFadeTime = cg.time + 60000;
		break;

	case EV_DESTROY_GHOUL2_INSTANCE:
		DEBUGNAME("EV_DESTROY_GHOUL2_INSTANCE");
		if (cg_entities[es->eventParm].ghoul2 && trap->G2_HaveWeGhoul2Models(cg_entities[es->eventParm].ghoul2))
		{
			if (es->eventParm < MAX_CLIENTS)
			{ //You try to do very bad thing!
#ifdef _DEBUG
				Com_Printf("WARNING: Tried to kill a client ghoul2 instance with a server event!\n");
#endif
				break;
			}
			trap->G2API_CleanGhoul2Models(&(cg_entities[es->eventParm].ghoul2));
		}
		break;

	case EV_DESTROY_WEAPON_MODEL:
		DEBUGNAME("EV_DESTROY_WEAPON_MODEL");
		if (cg_entities[es->eventParm].ghoul2 && trap->G2_HaveWeGhoul2Models(cg_entities[es->eventParm].ghoul2) &&
			trap->G2API_HasGhoul2ModelOnIndex(&(cg_entities[es->eventParm].ghoul2), 1))
		{
			trap->G2API_RemoveGhoul2Model(&(cg_entities[es->eventParm].ghoul2), 1);
		}
		break;

	case EV_GIVE_NEW_RANK:
		DEBUGNAME("EV_GIVE_NEW_RANK");
		if (es->trickedentindex == cg.snap->ps.clientNum)
		{
			trap->Cvar_Set("ui_rankChange", va("%i", es->eventParm));

			trap->Cvar_Set("ui_myteam", va("%i", es->bolt2));

			if (!( trap->Key_GetCatcher() & KEYCATCH_UI ) && !es->bolt1)
			{
				trap->OpenUIMenu(UIMENU_PLAYERCONFIG);
			}
		}
		break;

	case EV_SET_FREE_SABER:
		DEBUGNAME("EV_SET_FREE_SABER");

		trap->Cvar_Set("ui_freeSaber", va("%i", es->eventParm));
		break;

	case EV_SET_FORCE_DISABLE:
		DEBUGNAME("EV_SET_FORCE_DISABLE");

		trap->Cvar_Set("ui_forcePowerDisable", va("%i", es->eventParm));
		break;

	//
	// missile impacts
	//
	case EV_EXPLOSIVE_BLOW:
		DEBUGNAME("EV_EXPLOSIVE_BLOW");
		JKG_BlowExplosive (cent, qtrue);
		break;

	case EV_MISSILE_STICK:
		DEBUGNAME("EV_MISSILE_STICK");
//		trap->S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.missileStick );
		break;

	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );

		if ( es->emplacedOwner )
		{//hack: this is an index to a custom effect to use
			trap->FX_PlayEffectID(cgs.gameEffects[es->emplacedOwner], position, dir, -1, -1, false);
		}
		else
		{
			JKG_RenderProjectileHitPlayer (cent, position, dir, es->firingMode);
		}

		if (cg_ghoul2Marks.integer &&
			es->trickedentindex)
		{ //flag to place a ghoul2 mark
			CG_G2MarkEvent(es);
		}
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		if ( es->emplacedOwner )
		{//hack: this is an index to a custom effect to use
			trap->FX_PlayEffectID(cgs.gameEffects[es->emplacedOwner], position, dir, -1, -1, false);
		}
		else
		{
			JKG_RenderProjectileMiss (cent, position, dir, es->firingMode);
		}

		if (cg_ghoul2Marks.integer &&
			es->trickedentindex)
		{ //flag to place a ghoul2 mark
			CG_G2MarkEvent(es);
		}
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
		if ( es->emplacedOwner )
		{//hack: this is an index to a custom effect to use
			trap->FX_PlayEffectID(cgs.gameEffects[es->emplacedOwner], position, dir, -1, -1, false);
		}
		else
		{
		    JKG_RenderProjectileMiss (cent, position, dir, (qboolean)(es->eFlags & EF_ALT_FIRING));
		}
		break;

	case EV_PLAY_EFFECT:
		DEBUGNAME("EV_PLAY_EFFECT");
		switch(es->eventParm)
		{ //it isn't a hack, it's ingenuity!
		case EFFECT_SMOKE:
			eID = cgs.effects.mEmplacedDeadSmoke;
			break;
		case EFFECT_EXPLOSION:
			eID = cgs.effects.mEmplacedExplode;
			break;
		case EFFECT_EXPLOSION_PAS:
			eID = cgs.effects.mTurretExplode;
			break;
		case EFFECT_SPARK_EXPLOSION:
			eID = cgs.effects.mSparkExplosion;
			break;
		case EFFECT_EXPLOSION_DETPACK:
			eID = cgs.effects.mDetpackExplosion;
			break;
		case EFFECT_STUNHIT:
			eID = cgs.effects.mStunBatonFleshImpact;
			break;
		case EFFECT_EXPLOSION_TURRET:
			eID = cgs.effects.mTurretExplode;
			break;
		case EFFECT_SPARKS:
			eID = cgs.effects.mSparksExplodeNoSound;
			break;
		case EFFECT_WATER_SPLASH:
			eID = cgs.effects.waterSplash;
			break;
		case EFFECT_ACID_SPLASH:
			eID = cgs.effects.acidSplash;
			break;
		case EFFECT_LAVA_SPLASH:
			eID = cgs.effects.lavaSplash;
			break;
		case EFFECT_LANDING_MUD:
			eID = cgs.effects.landingMud;
			break;
		case EFFECT_LANDING_SAND:
			eID = cgs.effects.landingSand;
			break;
		case EFFECT_LANDING_DIRT:
			eID = cgs.effects.landingDirt;
			break;
		case EFFECT_LANDING_SNOW:
			eID = cgs.effects.landingSnow;
			break;
		case EFFECT_LANDING_GRAVEL:
			eID = cgs.effects.landingGravel;
			break;
		default:
			eID = -1;
			break;
		}

		if (eID != -1)
		{
			vec3_t fxDir;

			VectorCopy(es->angles, fxDir);

			if (!fxDir[0] && !fxDir[1] && !fxDir[2])
			{
				fxDir[1] = 1;
			}

			trap->FX_PlayEffectID(eID, es->origin, fxDir, -1, -1, false);
		}
		break;

	case EV_PLAY_EFFECT_ID:
	case EV_PLAY_PORTAL_EFFECT_ID:
		DEBUGNAME("EV_PLAY_EFFECT_ID");
		{
			vec3_t fxDir;
			qboolean portalEffect = qfalse;
			int efxIndex = 0;

			if (event == EV_PLAY_PORTAL_EFFECT_ID)
			{ //This effect should only be played inside sky portals.
				portalEffect = qtrue;
			}

			AngleVectors(es->angles, fxDir, 0, 0);
			
			if (!fxDir[0] && !fxDir[1] && !fxDir[2])
			{
				fxDir[1] = 1;
			}

			if ( cgs.gameEffects[ es->eventParm ] )
			{
				efxIndex = cgs.gameEffects[es->eventParm];
			}
			else
			{
				s = CG_ConfigString( CS_EFFECTS + es->eventParm );
				if (s && s[0])
				{
					efxIndex = trap->FX_RegisterEffect(s);
				}
			}

			if (efxIndex)
			{
				if (portalEffect)
				{
					trap->FX_PlayEffectID(efxIndex, position, fxDir, -1, -1, true );
				}
				else
				{
					trap->FX_PlayEffectID(efxIndex, position, fxDir, -1, -1, false );
				}
			}
		}
		break;

	case EV_PLAYDOORSOUND:
		CG_PlayDoorSound(cent, es->eventParm);
		break;
	case EV_PLAYDOORLOOPSOUND:
		CG_PlayDoorLoopSound(cent);
		break;
	case EV_BMODEL_SOUND:
		DEBUGNAME("EV_BMODEL_SOUND");
		{
			sfxHandle_t sfx;
			const char *soundSet;
			
			soundSet = CG_ConfigString( CS_AMBIENT_SET + es->soundSetIndex );

			if (!soundSet || !soundSet[0])
			{
				break;
			}

			sfx = trap->AS_GetBModelSound(soundSet, es->eventParm);

			if (sfx == -1)
			{
				break;
			}

			trap->S_StartSound( NULL, es->number, CHAN_AUTO, sfx );
		}
		break;


	case EV_MUTE_SOUND:
		DEBUGNAME("EV_MUTE_SOUND");
		if (cg_entities[es->trickedentindex2].currentState.eFlags & EF_SOUNDTRACKER)
		{
			cg_entities[es->trickedentindex2].currentState.eFlags -= EF_SOUNDTRACKER;
		}
		trap->S_MuteSound(es->trickedentindex2, es->trickedentindex);
		CG_S_StopLoopingSound(es->trickedentindex2, -1);
		break;

	case EV_VOICECMD_SOUND:
		DEBUGNAME("EV_VOICECMD_SOUND");
		if (es->groundEntityNum < MAX_CLIENTS && es->groundEntityNum >= 0)
		{
			int clientNum = es->groundEntityNum;
			sfxHandle_t sfx = cgs.gameSounds[ es->eventParm ];
			clientInfo_t *ci = &cgs.clientinfo[clientNum];
			centity_t *vChatEnt = &cg_entities[clientNum];
			char descr[1024] = {0};

			Q_strncpyz(descr, CG_GetStringForVoiceSound(CG_ConfigString( CS_SOUNDS + es->eventParm )), sizeof( descr ) );

			if (!sfx)
			{
				s = CG_ConfigString( CS_SOUNDS + es->eventParm );
				sfx = CG_CustomSound( clientNum, s );
			}

			if (sfx)
			{
				if (clientNum != cg.predictedPlayerState.clientNum)
				{ //play on the head as well to simulate hearing in radio and in world
					if (ci->team == cg.predictedPlayerState.persistant[PERS_TEAM])
					{ //don't hear it if this person is on the other team, but they can still
						//hear it in the world spot.
						trap->S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_MENU1, sfx);
					}
				}
				if (ci->team == cg.predictedPlayerState.persistant[PERS_TEAM])
				{ //add to the chat box
					//hear it in the world spot.
					char vchatstr[1024];
					Q_strncpyz(vchatstr, va("<%s: %s>\n", ci->name, descr), sizeof(vchatstr));
					trap->Print("*%s", vchatstr);
					CG_ChatBox_AddString(vchatstr, 100);
				}

				//and play in world for everyone
				trap->S_StartSound (NULL, clientNum, CHAN_VOICE, sfx);
				vChatEnt->vChatTime = cg.time + 1000;
			}
		}
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if (es->saberEntityNum == TRACK_CHANNEL_2 || es->saberEntityNum == TRACK_CHANNEL_3 ||
			es->saberEntityNum == TRACK_CHANNEL_5)
		{ //channels 2 and 3 are for speed and rage, 5 for sight
			if ( cgs.gameSounds[ es->eventParm ] )
			{
				CG_S_AddRealLoopingSound(es->number, es->pos.trBase, vec3_origin, cgs.gameSounds[ es->eventParm ] );
			}
		}
		else
		{
			if ( cgs.gameSounds[ es->eventParm ] ) {
				trap->S_StartSound (NULL, es->number, es->saberEntityNum, cgs.gameSounds[ es->eventParm ] );
			} else {
				s = CG_ConfigString( CS_SOUNDS + es->eventParm );
				trap->S_StartSound (NULL, es->number, es->saberEntityNum, CG_CustomSound( es->number, s ) );
			}
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap->S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_MENU1, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap->S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_MENU1, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_ENTITY_SOUND:
		DEBUGNAME("EV_ENTITY_SOUND");
		//somewhat of a hack - weapon is the caller entity's index, trickedentindex is the proper sound channel
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap->S_StartSound (NULL, es->clientNum, es->trickedentindex, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap->S_StartSound (NULL, es->clientNum, es->trickedentindex, CG_CustomSound( es->clientNum, s ) );
		}
		break;

	case EV_PLAY_ROFF:
		DEBUGNAME("EV_PLAY_ROFF");
		trap->ROFF_Play(es->weapon, es->eventParm, es->trickedentindex);
		break;

	case EV_GLASS_SHATTER:
		DEBUGNAME("EV_GLASS_SHATTER");
		CG_GlassShatter(es->genericenemyindex, es->origin, es->angles, es->trickedentindex, es->pos.trTime);
		break;

	case EV_DEBRIS:
		DEBUGNAME("EV_DEBRIS");
		CG_Chunks(es->owner, es->origin, es->angles, es->origin2, es->angles2, es->speed,
			es->eventParm, static_cast<material_t>(es->trickedentindex), es->modelindex, es->apos.trBase[0]);
		break;

	case EV_MISC_MODEL_EXP:
		DEBUGNAME("EV_MISC_MODEL_EXP");
		CG_MiscModelExplosion(es->origin2, es->angles2, es->time, static_cast<material_t>(es->eventParm));
		break;

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");

		if ( !cg_oldPainSounds.integer || (cent->currentState.number != cg.snap->ps.clientNum) )
		{
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		trap->S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		if (es->eventParm && es->number == cg.snap->ps.clientNum)
		{
			trap->S_StartLocalSound(cgs.media.dramaticFailure, CHAN_LOCAL);
			CGCam_SetMusicMult(0.3f, 5000);
		}
		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	case EV_FORCE_DRAINED:
		DEBUGNAME("EV_FORCE_DRAINED");
		ByteToDir( es->eventParm, dir );
		//FX_ForceDrained(position, dir);
		trap->S_StartSound (NULL, es->owner, CHAN_AUTO, cgs.media.drainSound );
		cg_entities[es->owner].teamPowerEffectTime = cg.time + 1000;
		cg_entities[es->owner].teamPowerType = 2;
		break;

	case EV_STARTLOOPINGSOUND:
		DEBUGNAME("EV_STARTLOOPINGSOUND");
		if ( cgs.gameSounds[ es->eventParm ] )
		{
			isnd = cgs.gameSounds[es->eventParm];
		}
		else
		{
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			isnd = CG_CustomSound(es->number, s);
		}

		CG_S_AddRealLoopingSound( es->number, es->pos.trBase, vec3_origin, isnd );
		es->loopSound = isnd;
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		CG_S_StopLoopingSound( es->number, -1 );
		es->loopSound = 0;
		break;

	case EV_WEAPON_CHARGE:
		DEBUGNAME("EV_WEAPON_CHARGE");
		//assert(es->eventParm > WP_NONE && es->eventParm < WP_NUM_WEAPONS);
		
		JKG_ChargeWeapon (cent, qfalse);
		
		/*weapon = CG_WeaponInfo (es->eventParm, 0);
		
		if (weapon->chargeSound)
		{
			trap->S_StartSound(NULL, es->number, CHAN_WEAPON, weapon->chargeSound);
		}
		else if (es->eventParm == WP_DISRUPTOR)
		{
			trap->S_StartSound(NULL, es->number, CHAN_WEAPON, cgs.media.disruptorZoomLoop);
		}*/
		break;

	case EV_WEAPON_CHARGE_ALT:
		DEBUGNAME("EV_WEAPON_CHARGE_ALT");
		//assert(es->eventParm > WP_NONE && es->eventParm < WP_NUM_WEAPONS);
		
		JKG_ChargeWeapon (cent, qtrue);
		
		/*weapon = CG_WeaponInfo (es->eventParm, 0);
		
		if (weapon->altChargeSound)
		{
			trap->S_StartSound(NULL, es->number, CHAN_WEAPON, weapon->altChargeSound);
		}*/
		break;

	case EV_SHIELD_HIT:
		DEBUGNAME("EV_SHIELD_HIT");
		ByteToDir(es->eventParm, dir);
		CG_PlayerShieldHit(es->otherEntityNum, dir, es->time2);
		break;

	case EV_SHIELD_BROKEN:
		DEBUGNAME("EV_SHIELD_BROKEN")
		// Doesn't do anything atm
		break;

	case EV_SHIELD_RECHARGE:
		DEBUGNAME("EV_SHIELD_RECHARGE")
		CG_PlayerShieldRecharging(es->otherEntityNum);
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	case EV_TESTLINE:
		DEBUGNAME("EV_TESTLINE");
		CG_TestLine(es->origin, es->origin2, es->time2, es->weapon, 1);
		break;

	// JKG - Start displaying a grenade indicator
	case EV_GRENADE_COOK:
	{
		if ( es->number == cg.clientNum && cg.snap )
		{
			cg.jkg_grenadeCookTimer = cg.time + GetWeaponData( cg.snap->ps.weapon, cg.snap->ps.weaponVariation )->weaponReloadTime;
		}
		
		break;
	}
	
	case EV_EXPLOSIVE_ARM:
	    DEBUGNAME ("EV_EXPLOSIVE_ARM");
	    JKG_ArmExplosive (cent, (qboolean)(es->eFlags & EF_ALT_FIRING));
	    break;
	    
	case EV_MISSILE_DIE:
		JKG_RenderProjectileDeath (cent, cent->currentState.origin, cent->currentState.angles, cent->currentState.firingMode);
	    break;
	default:
		DEBUGNAME("UNKNOWN");
		trap->Error( ERR_DROP, "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

