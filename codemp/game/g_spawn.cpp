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

#include "g_local.h"
#include "jkg_gangwars.h"

qboolean	G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	if ( !level.spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		if ( !Q_stricmp( key, level.spawnVars[i][0] ) ) {
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	if ( sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] ) != 3 ) {
		trap->Print( "G_SpawnVector: Failed sscanf on %s (default: %s)\n", key, defaultString );
		VectorClear( out );
		return qfalse;
	}
	return present;
}

qboolean	G_SpawnBoolean( const char *key, const char *defaultString, qboolean *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );

	if ( !Q_stricmp( s, "qtrue" ) || !Q_stricmp( s, "true" ) || !Q_stricmp( s, "yes" ) || !Q_stricmp( s, "1" ) )
		*out = qtrue;
	else if ( !Q_stricmp( s, "qfalse" ) || !Q_stricmp( s, "false" ) || !Q_stricmp( s, "no" ) || !Q_stricmp( s, "0" ) )
		*out = qfalse;
	else
		*out = qfalse;

	return present;
}

//
// fields are needed for spawning from the entity string
//
typedef enum {
	F_INT, 
	F_FLOAT,
	F_STRING,			// string on disk, pointer in memory
	F_VECTOR,
	F_ANGLEHACK,
	F_PARM1,			// Special case for parms
	F_PARM2,			// Special case for parms
	F_PARM3,			// Special case for parms
	F_PARM4,			// Special case for parms
	F_PARM5,			// Special case for parms
	F_PARM6,			// Special case for parms
	F_PARM7,			// Special case for parms
	F_PARM8,			// Special case for parms
	F_PARM9,			// Special case for parms
	F_PARM10,			// Special case for parms
	F_PARM11,			// Special case for parms
	F_PARM12,			// Special case for parms
	F_PARM13,			// Special case for parms
	F_PARM14,			// Special case for parms
	F_PARM15,			// Special case for parms
	F_PARM16			// Special case for parms
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
} field_t;

/* This array MUST be sorted correctly by alphabetical name field */
/* for conformity, use lower-case names too */
field_t fields[] = {
	{ "alliedteam",				FOFS( alliedTeam ),						F_INT },//for misc_turrets
	{ "angerscript",			FOFS( behaviorSet[BSET_ANGER] ),		F_STRING },//name of script to run
	{ "angle",					FOFS( s.angles ),						F_ANGLEHACK },
	{ "angles",					FOFS( s.angles ),						F_VECTOR },
	{ "attackscript",			FOFS( behaviorSet[BSET_ATTACK] ),		F_STRING },//name of script to run
	{ "awakescript",			FOFS( behaviorSet[BSET_AWAKE] ),		F_STRING },//name of script to run
	{ "blockedscript",			FOFS( behaviorSet[BSET_BLOCKED] ),		F_STRING },//name of script to run
	{ "chunksize",				FOFS( mass ),							F_FLOAT },//for func_breakables
	{ "classname",				FOFS( classname ),						F_STRING },
	{ "closetarget",			FOFS( closetarget ),					F_STRING },//for doors
	{ "count",					FOFS( count ),							F_INT },
	{ "deathscript",			FOFS( behaviorSet[BSET_DEATH] ),		F_STRING },//name of script to run
	{ "delay",					FOFS( delay ),							F_INT },
	{ "delayscript",			FOFS( behaviorSet[BSET_DELAYED] ),		F_STRING },//name of script to run
	{ "delayscripttime",		FOFS( delayScriptTime ),				F_INT },//name of script to run
	{ "dmg",					FOFS( damage ),							F_INT },
	{ "ffdeathscript",			FOFS( behaviorSet[BSET_FFDEATH] ),		F_STRING },//name of script to run
	{ "ffirescript",			FOFS( behaviorSet[BSET_FFIRE] ),		F_STRING },//name of script to run
	{ "fleescript",				FOFS( behaviorSet[BSET_FLEE] ),			F_STRING },//name of script to run
	{ "fullname",				FOFS( fullName ),						F_STRING },
	{ "goaltarget",				FOFS( goaltarget ),						F_STRING },//for siege
	{ "healingclass",			FOFS( healingclass ),					F_STRING },
	{ "healingrate",			FOFS( healingrate ),					F_INT },
	{ "healingsound",			FOFS( healingsound ),					F_STRING },
	{ "health",					FOFS( health ),							F_INT },
	{ "idealclass",				FOFS( idealclass ),						F_STRING },//for siege spawnpoints
	{ "linear",					FOFS( alt_fire ),						F_INT },//for movers to use linear movement
	{ "lostenemyscript",		FOFS( behaviorSet[BSET_LOSTENEMY] ),	F_STRING },//name of script to run
	{ "maxs",					FOFS( r.maxs ),							F_VECTOR },
	{ "message",				FOFS( message ),						F_STRING },
	{ "mindtrickscript",		FOFS( behaviorSet[BSET_MINDTRICK] ),	F_STRING },//name of script to run
	{ "mins",					FOFS( r.mins ),							F_VECTOR },
	{ "model",					FOFS( model ),							F_STRING },
	{ "model2",					FOFS( model2 ),							F_STRING },
	{ "npc_target",				FOFS( NPC_target ),						F_STRING },
	{ "npc_target2",			FOFS( target2 ),						F_STRING },//NPC_spawner only
	{ "npc_target4",			FOFS( target4 ),						F_STRING },//NPC_spawner only
	{ "npc_targetname",			FOFS( NPC_targetname ),					F_STRING },
	{ "npc_type",				FOFS( NPC_type ),						F_STRING },
	{ "numchunks",				FOFS( radius ),							F_FLOAT },//for func_breakables
	{ "opentarget",				FOFS( opentarget ),						F_STRING },//for doors
	{ "origin",					FOFS( s.origin ),						F_VECTOR },
	{ "ownername",				FOFS( ownername ),						F_STRING },
	{ "painscript",				FOFS( behaviorSet[BSET_PAIN] ),			F_STRING },//name of script to run
	{ "paintarget",				FOFS( paintarget ),						F_STRING },//for doors
	{ "parm1",					0,										F_PARM1 },
	{ "parm10",					0,										F_PARM10 },
	{ "parm11",					0,										F_PARM11 },
	{ "parm12",					0,										F_PARM12 },
	{ "parm13",					0,										F_PARM13 },
	{ "parm14",					0,										F_PARM14 },
	{ "parm15",					0,										F_PARM15 },
	{ "parm16",					0,										F_PARM16 },
	{ "parm2",					0,										F_PARM2 },
	{ "parm3",					0,										F_PARM3 },
	{ "parm4",					0,										F_PARM4 },
	{ "parm5",					0,										F_PARM5 },
	{ "parm6",					0,										F_PARM6 },
	{ "parm7",					0,										F_PARM7 },
	{ "parm8",					0,										F_PARM8 },
	{ "parm9",					0,										F_PARM9 },
	{ "radius",					FOFS( radius ),							F_FLOAT },
	{ "random",					FOFS( random ),							F_FLOAT },
	{ "roffname",				FOFS( roffname ),						F_STRING },
	{ "rofftarget",				FOFS( rofftarget ),						F_STRING },
	{ "script_targetname",		FOFS( script_targetname ),				F_STRING },//scripts look for this when "affecting"
	{ "soundset",				FOFS( soundSet ),						F_STRING },
	{ "spawnflags",				FOFS( spawnflags ),						F_INT },
	{ "spawnscript",			FOFS( behaviorSet[BSET_SPAWN] ),		F_STRING },//name of script to run
	{ "speed",					FOFS( speed ),							F_FLOAT },
	{ "target",					FOFS( target ),							F_STRING },
	{ "target2",				FOFS( target2 ),						F_STRING },
	{ "target3",				FOFS( target3 ),						F_STRING },
	{ "target4",				FOFS( target4 ),						F_STRING },
	{ "target5",				FOFS( target5 ),						F_STRING },
	{ "target6",				FOFS( target6 ),						F_STRING },
	{ "targetname",				FOFS( targetname ),						F_STRING },
	{ "targetshadername",		FOFS( targetShaderName ),				F_STRING },
	{ "targetshadernewname",	FOFS( targetShaderNewName ),			F_STRING },
	{ "team",					FOFS( team ),							F_STRING },
	{ "teamnodmg",				FOFS( teamnodmg ),						F_INT },
	{ "teamowner",				FOFS( s.teamowner ),					F_INT },
	{ "teamuser",				FOFS( alliedTeam ),						F_INT },
	{ "usescript",				FOFS( behaviorSet[BSET_USE] ),			F_STRING },//name of script to run
	{ "victoryscript",			FOFS( behaviorSet[BSET_VICTORY] ),		F_STRING },//name of script to run
	{ "wait",					FOFS( wait ),							F_FLOAT },
};

typedef struct {
	char	*name;
	qboolean	logical;
	void	(*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start (gentity_t *ent);
void SP_info_player_duel( gentity_t *ent );
void SP_info_player_duel1( gentity_t *ent );
void SP_info_player_duel2( gentity_t *ent );
void SP_info_player_deathmatch (gentity_t *ent);
void SP_info_player_intermission (gentity_t *ent);
void SP_info_player_intermission_red (gentity_t *ent);
void SP_info_player_intermission_blue (gentity_t *ent);
void SP_info_player_start_red (gentity_t *ent);
void SP_info_player_start_blue (gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);
void SP_info_podium(gentity_t *ent);

void SP_func_plat (gentity_t *ent);
void SP_func_static (gentity_t *ent);
void SP_func_rotating (gentity_t *ent);
void SP_func_bobbing (gentity_t *ent);
void SP_func_pendulum( gentity_t *ent );
void SP_func_button (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_train (gentity_t *ent);
void SP_func_timer (gentity_t *self);
void SP_func_breakable (gentity_t *ent);
void SP_func_glass (gentity_t *ent);
void SP_func_usable( gentity_t *ent);
void SP_func_wall( gentity_t *ent );

void SP_trigger_lightningstrike( gentity_t *ent );

void SP_trigger_always (gentity_t *ent);
void SP_trigger_multiple (gentity_t *ent);
void SP_trigger_once( gentity_t *ent );
void SP_trigger_push (gentity_t *ent);
void SP_trigger_teleport (gentity_t *ent);
void SP_trigger_hurt (gentity_t *ent);

void SP_target_remove_powerups( gentity_t *ent );
void SP_target_give (gentity_t *ent);
void SP_target_delay (gentity_t *ent);
void SP_target_speaker (gentity_t *ent);
void SP_target_print (gentity_t *ent);
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay (gentity_t *ent);
void SP_target_kill (gentity_t *ent);
void SP_target_position (gentity_t *ent);
void SP_target_location (gentity_t *ent);
void SP_target_counter (gentity_t *self);
void SP_target_random (gentity_t *self);
void SP_target_scriptrunner( gentity_t *self );
void SP_target_interest (gentity_t *self);
void SP_target_activate (gentity_t *self);
void SP_target_deactivate (gentity_t *self);
void SP_target_level_change( gentity_t *self );
void SP_target_play_music( gentity_t *self );
void SP_target_push (gentity_t *ent);

void SP_light (gentity_t *self);
void SP_info_null (gentity_t *self);
void SP_info_notnull (gentity_t *self);
void SP_info_camp (gentity_t *self);
void SP_path_corner (gentity_t *self);

void SP_misc_teleporter_dest (gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_model_static(gentity_t *ent);
void SP_misc_model_breakable( gentity_t *ent ) ;
void SP_misc_G2model(gentity_t *ent);
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);
void SP_misc_weather_zone( gentity_t *ent );

void SP_misc_bsp (gentity_t *ent);
void SP_terrain (gentity_t *ent);
void SP_misc_skyportal_orient (gentity_t *ent);
void SP_misc_skyportal (gentity_t *ent);

void SP_misc_ammo_floor_unit(gentity_t *ent);
void SP_misc_shield_floor_unit( gentity_t *ent );
void SP_misc_model_shield_power_converter( gentity_t *ent );
void SP_misc_model_ammo_power_converter( gentity_t *ent );
void SP_misc_model_health_power_converter( gentity_t *ent );

void SP_fx_runner( gentity_t *ent );

void SP_target_screenshake(gentity_t *ent);
void SP_target_escapetrig(gentity_t *ent);

void SP_misc_maglock ( gentity_t *self );

void SP_misc_faller(gentity_t *ent);

void SP_reference_tag ( gentity_t *ent );

void SP_misc_weapon_shooter( gentity_t *self );

void SP_NPC_spawner( gentity_t *self );

void SP_NPC_Kyle( gentity_t *self );
void SP_NPC_Lando( gentity_t *self );
void SP_NPC_Jan( gentity_t *self );
void SP_NPC_Luke( gentity_t *self );
void SP_NPC_MonMothma( gentity_t *self );
void SP_NPC_Tavion( gentity_t *self );
void SP_NPC_Tavion_New( gentity_t *self );
void SP_NPC_Alora( gentity_t *self );
void SP_NPC_Reelo( gentity_t *self );
void SP_NPC_Galak( gentity_t *self );
void SP_NPC_Desann( gentity_t *self );
void SP_NPC_Bartender( gentity_t *self );
void SP_NPC_MorganKatarn( gentity_t *self );
void SP_NPC_Jedi( gentity_t *self );
void SP_NPC_Prisoner( gentity_t *self );
void SP_NPC_Rebel( gentity_t *self );
void SP_NPC_Stormtrooper( gentity_t *self );
void SP_NPC_StormtrooperOfficer( gentity_t *self );
void SP_NPC_Snowtrooper( gentity_t *self);
void SP_NPC_Tie_Pilot( gentity_t *self );
void SP_NPC_Ugnaught( gentity_t *self );
void SP_NPC_Jawa( gentity_t *self );
void SP_NPC_Gran( gentity_t *self );
void SP_NPC_Rodian( gentity_t *self );
void SP_NPC_Weequay( gentity_t *self );
void SP_NPC_Trandoshan( gentity_t *self );
void SP_NPC_Tusken( gentity_t *self );
void SP_NPC_Noghri( gentity_t *self );
void SP_NPC_SwampTrooper( gentity_t *self );
void SP_NPC_Imperial( gentity_t *self );
void SP_NPC_ImpWorker( gentity_t *self );
void SP_NPC_BespinCop( gentity_t *self );
void SP_NPC_Reborn( gentity_t *self );
void SP_NPC_ShadowTrooper( gentity_t *self );
void SP_NPC_Monster_Murjj( gentity_t *self );
void SP_NPC_Monster_Swamp( gentity_t *self );
void SP_NPC_Monster_Howler( gentity_t *self );
void SP_NPC_Monster_Claw( gentity_t *self );
void SP_NPC_Monster_Glider( gentity_t *self );
void SP_NPC_Monster_Flier2( gentity_t *self );
void SP_NPC_Monster_Lizard( gentity_t *self );
void SP_NPC_Monster_Fish( gentity_t *self );
void SP_NPC_Monster_Wampa( gentity_t *self );
void SP_NPC_Monster_Rancor( gentity_t *self );
void SP_NPC_MineMonster( gentity_t *self );
void SP_NPC_Droid_Interrogator( gentity_t *self );
void SP_NPC_Droid_Probe( gentity_t *self );
void SP_NPC_Droid_Mark1( gentity_t *self );
void SP_NPC_Droid_Mark2( gentity_t *self );
void SP_NPC_Droid_ATST( gentity_t *self );
void SP_NPC_Droid_Seeker( gentity_t *self );
void SP_NPC_Droid_Remote( gentity_t *self );
void SP_NPC_Droid_Sentry( gentity_t *self );
void SP_NPC_Droid_Gonk( gentity_t *self );
void SP_NPC_Droid_Mouse( gentity_t *self );
void SP_NPC_Droid_R2D2( gentity_t *self );
void SP_NPC_Droid_R5D2( gentity_t *self );
void SP_NPC_Droid_Protocol( gentity_t *self );

void SP_NPC_Reborn_New( gentity_t *self);
void SP_NPC_Cultist( gentity_t *self );
void SP_NPC_Cultist_Saber( gentity_t *self );
void SP_NPC_Cultist_Saber_Powers( gentity_t *self );
void SP_NPC_Cultist_Destroyer( gentity_t *self );
void SP_NPC_Cultist_Commando( gentity_t *self );

void SP_waypoint (gentity_t *ent);
void SP_waypoint_small (gentity_t *ent);
void SP_waypoint_navgoal (gentity_t *ent);
void SP_waypoint_navgoal_8 (gentity_t *ent);
void SP_waypoint_navgoal_4 (gentity_t *ent);
void SP_waypoint_navgoal_2 (gentity_t *ent);
void SP_waypoint_navgoal_1 (gentity_t *ent);

void SP_CreateSpaceDust( gentity_t *ent );
void SP_CreateSnow( gentity_t *ent );
void SP_CreateRain( gentity_t *ent );

void SP_point_combat( gentity_t *self );

void SP_shooter_blaster( gentity_t *ent );

void SP_team_CTF_redplayer( gentity_t *ent );
void SP_team_CTF_blueplayer( gentity_t *ent );

void SP_team_CTF_redspawn( gentity_t *ent );
void SP_team_CTF_bluespawn( gentity_t *ent );

void SP_misc_turret( gentity_t *ent );
void SP_misc_turretG2( gentity_t *base );

// Warzone...
void SP_misc_control_point (gentity_t* ent);
void SP_ammo_crate_spawn ( gentity_t* ent );
void SP_health_crate_spawn ( gentity_t* ent );

void SP_item_botroam( gentity_t *ent )
{
}

void SP_gametype_item ( gentity_t* ent )
{
	gitem_t *item = NULL;
	char *value;
	int team = -1;

	G_SpawnString("teamfilter", "", &value);

	G_SetOrigin( ent, ent->s.origin );

	// If a team filter is set then override any team settings for the spawns
	if ( level.mTeamFilter[0] )
	{
		if ( Q_stricmp ( level.mTeamFilter, "red") == 0 )
		{
			team = TEAM_RED;
		}
		else if ( Q_stricmp ( level.mTeamFilter, "blue") == 0 )
		{
			team = TEAM_BLUE;
		}
	}

	if (ent->targetname && ent->targetname[0])
	{
		if (team != -1)
		{
			if (strstr(ent->targetname, "flag"))
			{
				if (team == TEAM_RED)
				{
					item = BG_FindItem("team_CTF_redflag");
				}
				else
				{ //blue
					item = BG_FindItem("team_CTF_blueflag");
				}
			}
		}
		else if (strstr(ent->targetname, "red_flag"))
		{
			item = BG_FindItem("team_CTF_redflag");
		}
		else if (strstr(ent->targetname, "blue_flag"))
		{
			item = BG_FindItem("team_CTF_blueflag");
		}
		else
		{
			item = NULL;
		}

		if (item)
		{
			ent->targetname = NULL;
			ent->classname = item->classname;
			G_SpawnItem( ent, item );
		}
	}
}

void SP_emplaced_gun( gentity_t *ent );

/* This array MUST be sorted correctly by alphabetical name field */
/* for conformity, use lower-case names too */
spawn_t	spawns[] = {
	{ "emplaced_gun", qfalse,						SP_emplaced_gun },
	{ "func_bobbing", qfalse,						SP_func_bobbing },
	{ "func_breakable", qfalse,						SP_func_breakable },
	{ "func_button", qfalse,						SP_func_button },
	{ "func_door", qfalse,							SP_func_door },
	{ "func_glass", qfalse,							SP_func_glass },
	{ "func_group",	 qfalse,						SP_info_null },
	{ "func_pendulum", qfalse,						SP_func_pendulum },
	{ "func_plat", qfalse,							SP_func_plat },
	{ "func_rotating", qfalse,						SP_func_rotating },
	{ "func_static", qfalse,						SP_func_static },
	{ "func_timer", qfalse,							SP_func_timer }, // rename trigger_timer?
	{ "func_train", qfalse,							SP_func_train },
	{ "func_usable", qfalse,						SP_func_usable },
	{ "func_wall", qfalse,							SP_func_wall },
	{ "fx_rain", qtrue,							SP_CreateRain },
	{ "fx_runner", qfalse,							SP_fx_runner },
	{ "fx_snow", qtrue,							SP_CreateSnow },
	{ "fx_spacedust", qtrue,						SP_CreateSpaceDust },
	{ "gametype_item", qfalse,						SP_gametype_item },
	{ "info_camp", qtrue,							SP_info_camp },
	{ "info_notnull", qtrue,						SP_info_notnull }, // use target_position instead
	{ "info_null", qtrue,							SP_info_null },
	{ "info_player_deathmatch", qtrue,					SP_info_player_deathmatch },
	{ "info_player_duel", qtrue,						SP_info_player_duel },
	{ "info_player_duel1", qtrue,						SP_info_player_duel1 },
	{ "info_player_duel2", qtrue,						SP_info_player_duel2 },
	{ "info_player_intermission", qtrue,					SP_info_player_intermission },
	{ "info_player_intermission_blue", qtrue,				SP_info_player_intermission_blue },
	{ "info_player_intermission_red", qtrue,				SP_info_player_intermission_red },
	{ "info_player_start", qtrue,						SP_info_player_start },
	{ "info_player_start_blue", qtrue,					SP_info_player_start_blue },
	{ "info_player_start_red", qtrue,					SP_info_player_start_red },
	{ "item_botroam", qtrue,						SP_item_botroam },
	{ "jkg_target_vendor", qtrue,						JKG_SP_target_vendor },
	{ "light", qfalse,							SP_light },
	{ "misc_ammo_crate", qfalse,						SP_ammo_crate_spawn },
	{ "misc_ammo_floor_unit", qfalse,					SP_misc_ammo_floor_unit },
	{ "misc_bsp", qfalse,							SP_misc_bsp },
	{ "misc_control_point", qfalse,						SP_misc_control_point },
	{ "misc_faller", qfalse,						SP_misc_faller },
	{ "misc_G2model", qfalse,						SP_misc_G2model },
	{ "misc_health_crate", qfalse,						SP_health_crate_spawn },
	{ "misc_maglock", qfalse,						SP_misc_maglock },
	{ "misc_model", qtrue,							SP_misc_model },
	{ "misc_model_ammo_power_converter", qfalse,				SP_misc_model_ammo_power_converter },
	{ "misc_model_breakable", qfalse,					SP_misc_model_breakable },
	{ "misc_model_health_power_converter", qfalse,				SP_misc_model_health_power_converter },
	{ "misc_model_shield_power_converter", qfalse,				SP_misc_model_shield_power_converter },
	{ "misc_model_static", qtrue,						SP_misc_model_static },
	{ "misc_portal_camera", qfalse,						SP_misc_portal_camera },
	{ "misc_portal_surface", qfalse,					SP_misc_portal_surface },
	{ "misc_shield_floor_unit", qfalse,					SP_misc_shield_floor_unit },
	{ "misc_skyportal", qfalse,						SP_misc_skyportal },
	{ "misc_skyportal_orient", qfalse,					SP_misc_skyportal_orient },
	{ "misc_teleporter_dest", qfalse,					SP_misc_teleporter_dest },
	{ "misc_turret", qfalse,						SP_misc_turret },
	{ "misc_turretG2", qfalse,						SP_misc_turretG2 },
	{ "misc_weapon_shooter", qfalse,					SP_misc_weapon_shooter },
	{ "misc_weather_zone", qfalse,						SP_misc_weather_zone },
	{ "npc_alora", qtrue,							SP_NPC_Alora },
	{ "npc_bartender", qtrue,						SP_NPC_Bartender },
	{ "npc_bespincop", qtrue,						SP_NPC_BespinCop },
	{ "npc_colombian_emplacedgunner", qtrue,				SP_NPC_ShadowTrooper },
	{ "npc_colombian_rebel", qtrue,						SP_NPC_Reborn },
	{ "npc_colombian_soldier", qtrue,					SP_NPC_Reborn },
	{ "npc_cultist", qtrue,							SP_NPC_Cultist },
	{ "npc_cultist_commando", qtrue,					SP_NPC_Cultist_Commando },
	{ "npc_cultist_destroyer", qtrue,					SP_NPC_Cultist_Destroyer },
	{ "npc_cultist_saber", qtrue,						SP_NPC_Cultist_Saber },
	{ "npc_cultist_saber_powers", qtrue,					SP_NPC_Cultist_Saber_Powers },
	{ "npc_desann", qtrue,							SP_NPC_Desann },
	{ "npc_droid_atst", qtrue,						SP_NPC_Droid_ATST },
	{ "npc_droid_gonk", qtrue,						SP_NPC_Droid_Gonk },
	{ "npc_droid_interrogator", qtrue,					SP_NPC_Droid_Interrogator },
	{ "npc_droid_mark1", qtrue,						SP_NPC_Droid_Mark1 },
	{ "npc_droid_mark2", qtrue,						SP_NPC_Droid_Mark2 },
	{ "npc_droid_mouse", qtrue,						SP_NPC_Droid_Mouse },
	{ "npc_droid_probe", qtrue,						SP_NPC_Droid_Probe },
	{ "npc_droid_protocol", qtrue,						SP_NPC_Droid_Protocol },
	{ "npc_droid_r2d2", qtrue,						SP_NPC_Droid_R2D2 },
	{ "npc_droid_r5d2", qtrue,						SP_NPC_Droid_R5D2 },
	{ "npc_droid_remote", qtrue,						SP_NPC_Droid_Remote },
	{ "npc_droid_seeker", qtrue,						SP_NPC_Droid_Seeker },
	{ "npc_droid_sentry", qtrue,						SP_NPC_Droid_Sentry },
	{ "npc_galak", qtrue,							SP_NPC_Galak },
	{ "npc_gran", qtrue,							SP_NPC_Gran },
	{ "npc_imperial", qtrue,						SP_NPC_Imperial },
	{ "npc_impworker", qtrue,						SP_NPC_ImpWorker },
	{ "npc_jan", qtrue,							SP_NPC_Jan },
	{ "npc_jawa", qtrue,							SP_NPC_Jawa },
	{ "npc_jedi", qtrue,							SP_NPC_Jedi },
	{ "npc_kyle", qtrue,							SP_NPC_Kyle },
	{ "npc_lando", qtrue,							SP_NPC_Lando },
	{ "npc_luke", qtrue,							SP_NPC_Luke },
	{ "npc_manuel_vergara_rmg", qtrue,					SP_NPC_Desann },
	{ "npc_minemonster", qtrue,						SP_NPC_MineMonster },
	{ "npc_monmothma", qtrue,						SP_NPC_MonMothma },
	{ "npc_monster_claw", qtrue,						SP_NPC_Monster_Claw },
	{ "npc_monster_fish", qtrue,						SP_NPC_Monster_Fish },
	{ "npc_monster_flier2", qtrue,						SP_NPC_Monster_Flier2 },
	{ "npc_monster_glider", qtrue,						SP_NPC_Monster_Glider },
	{ "npc_monster_howler", qtrue,						SP_NPC_Monster_Howler },
	{ "npc_monster_lizard", qtrue,						SP_NPC_Monster_Lizard },
	{ "npc_monster_murjj", qtrue,						SP_NPC_Monster_Murjj },
	{ "npc_monster_rancor",	 qtrue,						SP_NPC_Monster_Rancor },
	{ "npc_monster_swamp", qtrue,						SP_NPC_Monster_Swamp },
	{ "npc_monster_wampa", qtrue,						SP_NPC_Monster_Wampa },
	{ "npc_morgankatarn", qtrue,						SP_NPC_MorganKatarn },
	{ "npc_noghri", qtrue,							SP_NPC_Noghri },
	{ "npc_prisoner", qtrue,						SP_NPC_Prisoner },
	{ "npc_rebel", qtrue,							SP_NPC_Rebel },
	{ "npc_reborn", qtrue,							SP_NPC_Reborn },
	{ "npc_reborn_new", qtrue,						SP_NPC_Reborn_New },
	{ "npc_reelo", qtrue,							SP_NPC_Reelo },
	{ "npc_rodian", qtrue,							SP_NPC_Rodian },
	{ "npc_shadowtrooper", qtrue,						SP_NPC_ShadowTrooper },
	{ "npc_snowtrooper", qtrue,						SP_NPC_Snowtrooper },
	{ "npc_spawner", qtrue,							SP_NPC_spawner },
	{ "npc_stormtrooper", qtrue,						SP_NPC_Stormtrooper },
	{ "npc_stormtrooperofficer", qtrue,					SP_NPC_StormtrooperOfficer },
	{ "npc_swamptrooper", qtrue,						SP_NPC_SwampTrooper },
	{ "npc_tavion", qtrue,							SP_NPC_Tavion },
	{ "npc_tavion_new", qtrue,						SP_NPC_Tavion_New },
	{ "npc_tie_pilot", qtrue,						SP_NPC_Tie_Pilot },
	{ "npc_trandoshan", qtrue,						SP_NPC_Trandoshan },
	{ "npc_tusken", qtrue,							SP_NPC_Tusken },
	{ "npc_ugnaught", qtrue,						SP_NPC_Ugnaught },
	{ "npc_weequay", qtrue,							SP_NPC_Weequay },
	{ "path_corner", qtrue,							SP_path_corner },
	{ "point_combat", qtrue,						SP_point_combat },
	{ "ref_tag", qtrue,							SP_reference_tag },
	{ "ref_tag_huge", qtrue,						SP_reference_tag },
	{ "shooter_blaster", qtrue,						SP_shooter_blaster },
	{ "target_activate", qtrue,						SP_target_activate },
	{ "target_counter", qtrue,						SP_target_counter },
	{ "target_deactivate", qtrue,						SP_target_deactivate },
	{ "target_delay", qtrue,						SP_target_delay },
	{ "target_escapetrig", qtrue,						SP_target_escapetrig },
	{ "target_give", qtrue,							SP_target_give },
	{ "target_interest", qtrue,						SP_target_interest },
	{ "target_kill", qtrue,							SP_target_kill },
	{ "target_level_change", qtrue,						SP_target_level_change },
	{ "target_location", qtrue,						SP_target_location },
	{ "target_play_music", qtrue,						SP_target_play_music },
	{ "target_position", qtrue,						SP_target_position },
	{ "target_print", qtrue,						SP_target_print },
	{ "target_push", qtrue,							SP_target_push },
	{ "target_random", qtrue,						SP_target_random },
	{ "target_relay", qtrue,						SP_target_relay },
	{ "target_remove_powerups", qtrue,					SP_target_remove_powerups },
	{ "target_score", qtrue,						SP_target_score },
	{ "target_screenshake", qtrue,						SP_target_screenshake },
	{ "target_scriptrunner", qtrue,						SP_target_scriptrunner },
	{ "target_speaker", qfalse,						SP_target_speaker },
	{ "target_teleporter", qtrue,						SP_target_teleporter },
	{ "team_CTF_blueplayer", qtrue,						SP_team_CTF_blueplayer },
	{ "team_CTF_bluespawn", qtrue,						SP_team_CTF_bluespawn },
	{ "team_CTF_redplayer", qtrue,						SP_team_CTF_redplayer },
	{ "team_CTF_redspawn", qtrue,						SP_team_CTF_redspawn },
	{ "terrain", qfalse,							SP_terrain },
	{ "trigger_always", qfalse,						SP_trigger_always },
	{ "trigger_hurt", qfalse,						SP_trigger_hurt },
	{ "trigger_lightningstrike", qfalse,					SP_trigger_lightningstrike },
	{ "trigger_multiple", qfalse,						SP_trigger_multiple },
	{ "trigger_once", qfalse,						SP_trigger_once },
	{ "trigger_push", qfalse,						SP_trigger_push },
	{ "trigger_teleport", qfalse,						SP_trigger_teleport },
	{ "waypoint", qtrue,							SP_waypoint },
	{ "waypoint_navgoal", qtrue,						SP_waypoint_navgoal },
	{ "waypoint_navgoal_1", qtrue,						SP_waypoint_navgoal_1 },
	{ "waypoint_navgoal_2", qtrue,						SP_waypoint_navgoal_2 },
	{ "waypoint_navgoal_4", qtrue,						SP_waypoint_navgoal_4 },
	{ "waypoint_navgoal_8", qtrue,						SP_waypoint_navgoal_8 },
	{ "waypoint_small", qtrue,						SP_waypoint_small },
	{NULL, NULL, NULL}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
static int spawncmp( const void *a, const void *b )
{
	return Q_stricmp( (const char *)a, ((spawn_t*)b)->name );
}

int GLua_Spawn_Entity(gentity_t* ent);
int GLua_GetEntityTypeID(const char* classname);

qboolean G_IsLogicalEntity(const char *classname) {
	spawn_t	*s;

	if (!classname) {
		return qfalse;
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !Q_stricmp(s->name, classname) ) {
			// found it
			if (s->logical) {
				return qtrue;
			} else {
				return qfalse;
			}
		}
	}
	// Try Lua entities next
	if (GLua_GetEntityTypeID(classname) == 2 /* Logical ent */) {
		return qtrue;
	}

	return qfalse;
}


qboolean G_CallSpawn( gentity_t *ent ) {
	spawn_t	*s;
	gitem_t	*item;

	if ( !ent->classname ) {
		trap->Print( "G_CallSpawn: NULL classname\n" );
		return qfalse;
	}

	// check item spawn functions
	//TODO: cant reorder items because compat so....?
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
		if ( !strcmp(item->classname, ent->classname) ) {
#ifdef __DISABLE_UNUSED_SPAWNS__
			if ( !Q_strncmp(ent->classname, "weapon_", 7) )
			{// UQ1: Since we can't pick them up anyway, don't even spawn weapons...
				trap->Print("%s spawn disabled.\n", ent->classname);
				G_FreeEntity(ent);
				return qfalse;
			}
			else if ( !Q_strncmp(ent->classname, "ammo_", 5) )
			{// UQ1: Since we can't pick them up anyway, don't even spawn ammo...
				trap->Print("%s spawn disabled.\n", ent->classname);
				G_FreeEntity(ent);
				return qfalse;
			}
#endif //__DISABLE_UNUSED_SPAWNS__

			G_SpawnItem( ent, item );
			return qtrue;
		}
	}

	// check normal spawn functions
	s = (spawn_t *)bsearch( ent->classname, spawns, ARRAY_LEN( spawns ), sizeof( spawn_t ), spawncmp );
	if ( s )
	{// found it
		if ( VALIDSTRING( ent->healingsound ) )
			G_SoundIndex( ent->healingsound );

		s->spawn( ent );
		return qtrue;
	}
	// Jedi Knight Galaxies
	// Try Lua entities next
	if (GLua_Spawn_Entity(ent)) {
		return qtrue;
	}

	trap->Print( "%s doesn't have a spawn function\n", ent->classname );
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string )
{
	char *newb=NULL, *new_p=NULL;
	int i=0, len=0;
	
	len = strlen( string )+1;
	new_p = newb = (char *)G_Alloc( len );

	for ( i=0; i<len; i++ )
	{// turn \n into a real linefeed
		if ( string[i] == '\\' && i < len-1 )
		{
			if ( string[i+1] == 'n' )
			{
				*new_p++ = '\n';
				i++;
			}
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}

	return newb;
}

char *G_NewString_Safe( const char *string )
{//JAC: This version uses malloc() and is safe to free() to avoid memory leaks or internal memory pool overflow
	char *newb=NULL, *new_p=NULL;
	int i=0, len=0;
	
	len = strlen( string )+1;
	new_p = newb = (char *)malloc( len );

	if ( !new_p )
		return NULL;

	for ( i=0; i<len; i++ )
	{// turn \n into a real linefeed
		if ( string[i] == '\\' && i < len-1 )
		{
			if ( string[i+1] == 'n' )
			{
				*new_p++ = '\n';
				i++;
			}
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}

	return newb;
}

/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/

static int fieldcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((field_t*)b)->name );
}

void Q3_SetParm ( int entID, int parmNum, const char *parmValue );
void G_ParseField( const char *key, const char *value, gentity_t *ent )
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	f = (field_t *)bsearch( key, fields, ARRAY_LEN( fields ), sizeof( field_t ), fieldcmp );
	if ( f )
	{// found it
		b = (byte *)ent;

		switch( f->type ) {
		case F_STRING:
			*(char **)(b+f->ofs) = G_NewString (value);
			break;
		case F_VECTOR:
			if ( sscanf( value, "%f %f %f", &vec[0], &vec[1], &vec[2] ) == 3 ) {
				// HACK for mins/maxs
				if( !Q_stricmp( key, "mins" ) )
				{
					ent->r.contents = CONTENTS_SOLID;
					ent->clipmask = MASK_SOLID;
					VectorCopy(vec, ent->r.mins);
				}
				else if( !Q_stricmp( key, "maxs" ) )
				{
					ent->r.contents = CONTENTS_SOLID;
					ent->clipmask = MASK_SOLID;
					VectorCopy(vec, ent->r.maxs);
				}
				else
				{
					((float *)(b+f->ofs))[0] = vec[0];
					((float *)(b+f->ofs))[1] = vec[1];
					((float *)(b+f->ofs))[2] = vec[2];
				}
			}
			else {
				trap->Print( "G_ParseField: Failed sscanf on F_VECTOR (key/value: %s/%s)\n", key, value );
				((float *)(b+f->ofs))[0] = ((float *)(b+f->ofs))[1] = ((float *)(b+f->ofs))[2] = 0.0f;
			}
			break;
		case F_INT:
			*(int *)(b+f->ofs) = atoi(value);
			break;
		case F_FLOAT:
			*(float *)(b+f->ofs) = atof(value);
			break;
		case F_ANGLEHACK:
			v = atof(value);
			((float *)(b+f->ofs))[0] = 0;
			((float *)(b+f->ofs))[1] = v;
			((float *)(b+f->ofs))[2] = 0;
			break;
		case F_PARM1:
		case F_PARM2:
		case F_PARM3:
		case F_PARM4:
		case F_PARM5:
		case F_PARM6:
		case F_PARM7:
		case F_PARM8:
		case F_PARM9:
		case F_PARM10:
		case F_PARM11:
		case F_PARM12:
		case F_PARM13:
		case F_PARM14:
		case F_PARM15:
		case F_PARM16:
			Q3_SetParm( ent->s.number, (f->type - F_PARM1), (char *) value );
			break;
		}
		return;
	}
}

#define ADJUST_AREAPORTAL() \
	if(ent->s.eType == ET_MOVER) \
	{ \
		trap->LinkEntity((sharedEntity_t *)ent); \
		trap->AdjustAreaPortalState((sharedEntity_t *)ent, qtrue); \
	}

/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars( qboolean inSubBSP ) 
{
	int			i;
	gentity_t	*ent;
	char		*s, *value, *gametypeName;
	KeyPairSet_t *spv;
	static char *gametypeNames[] = {"ffa", "duel", "powerduel", "single", "team", "ctf", "cty"};

	// get the next free entity
	ent = G_Spawn();

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
	}

	// check for "notsingle" flag
	if ( level.gametype == GT_SINGLE_PLAYER ) {
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) {
			ADJUST_AREAPORTAL();
			G_FreeEntity( ent );
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_DUEL, GT_SINGLE_PLAYER)
	if ( level.gametype >= GT_TEAM ) {
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) {
			ADJUST_AREAPORTAL();
			G_FreeEntity( ent );
			return;
		}
	} else {
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) {
			ADJUST_AREAPORTAL();
			G_FreeEntity( ent );
			return;
		}
	}

	if( G_SpawnString( "gametype", NULL, &value ) ) {
		if( level.gametype >= GT_FFA && level.gametype < GT_MAX_GAME_TYPE ) {
			gametypeName = gametypeNames[level.gametype];

			s = strstr( value, gametypeName );
			if( !s ) {
				ADJUST_AREAPORTAL();
				G_FreeEntity( ent );
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	
	
	// Store the spawnvars for later use
	spv = &g_spawnvars[ent->s.number];
	for (i=0; i < level.numSpawnVars; i++) {
		JKG_Pairs_Add(spv, level.spawnVars[i][0], level.spawnVars[i][1]);
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}

	//Tag on the ICARUS scripting information only to valid recipients
	if ( trap->ICARUS_ValidEnt( (sharedEntity_t *)ent ) )
	{
		trap->ICARUS_InitEnt( (sharedEntity_t *)ent );

		if ( ent->classname && ent->classname[0] )
		{
			if ( Q_strncmp( "NPC_", ent->classname, 4 ) != 0 )
			{//Not an NPC_spawner (rww - probably don't even care for MP, but whatever)
				G_ActivateBehavior( ent, BSET_SPAWN );
			}
		}
	}
}

// Used by the lua entity factory
void G_SpawnEntity(gentity_t **outent) {
	int			i;
	gentity_t	*ent;
	char		*s, *value, *gametypeName;
	KeyPairSet_t *spv;
	static char *gametypeNames[] = {"ffa", "duel", "powerduel", "single", "team", "ctf", "cty"};

	G_SpawnString( "classname", NULL, &value );
	if (!value) {
		return;	// Dont even bother spawning an ent without a classname
	}
	if (G_IsLogicalEntity(value)) {
		// Check if the entity wants to be nonlogical anyway
		G_SpawnInt("nological", "0", &i);
		if (i) {				// Despite it being a logical entity, it wants to be nonlogical
			ent = G_Spawn();	// possibly because it wants to use icarus for example
		} else {
			// Get the next free logical entity
			ent = G_SpawnLogical();
		}
	} else {
		// Get the next free normal entity
		ent = G_Spawn();
	}

	*outent = ent;

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
	}

	// check for "notsingle" flag
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_DUEL, GT_SINGLE_PLAYER)
	if ( g_gametype.integer >= GT_TEAM ) {
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	} else {
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}

	G_SpawnInt( "notta", "0", &i );
	if ( i ) {
		G_FreeEntity( ent );
		return;
	}

	if( G_SpawnString( "gametype", NULL, &value ) ) {
		if( g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE ) {
			gametypeName = gametypeNames[g_gametype.integer];

			s = strstr( value, gametypeName );
			if( !s ) {
				G_FreeEntity( ent );
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );



	// Store the spawnvars for later use
	spv = &g_spawnvars[ent->s.number];
	for (i=0; i < level.numSpawnVars; i++) {
		JKG_Pairs_Add(spv, level.spawnVars[i][0], level.spawnVars[i][1]);
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}

	//Tag on the ICARUS scripting information only to valid recipients
	if ( trap->ICARUS_ValidEnt( (sharedEntity_t *)ent ) )
	{
		trap->ICARUS_InitEnt( (sharedEntity_t *)ent );

		if ( ent->classname && ent->classname[0] )
		{
			if ( Q_strncmp( "NPC_", ent->classname, 4 ) != 0 )
			{//Not an NPC_spawner (rww - probably don't even care for MP, but whatever)
				G_ActivateBehavior( ent, BSET_SPAWN );
			}
		}
	}
}

/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		trap->Error( ERR_DROP, "G_AddSpawnVarToken: MAX_SPAWN_VARS_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l+1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}

void AddSpawnField(char *field, char *value)
{
	int	i;

	for(i=0;i<level.numSpawnVars;i++)
	{
		if (Q_stricmp(level.spawnVars[i][0], field) == 0)
		{
			level.spawnVars[ i ][1] = G_AddSpawnVarToken( value );
			return;
		}
	}

	level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( field );
	level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( value );
	level.numSpawnVars++;
}

#define NOVALUE "novalue"

static void HandleEntityAdjustment(void)
{
	char		*value;
	vec3_t		origin, newOrigin, angles;
	char		temp[MAX_QPATH];
	float		rotation;

	G_SpawnString("origin", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		if ( sscanf( value, "%f %f %f", &origin[0], &origin[1], &origin[2] ) != 3 ) {
			trap->Print( "HandleEntityAdjustment: failed sscanf on 'origin' (%s)\n", value );
			VectorClear( origin );
		}
	}
	else
	{
		origin[0] = origin[1] = origin[2] = 0.0;
	}

	rotation = DEG2RAD(level.mRotationAdjust);
	newOrigin[0] = origin[0]*cos(rotation) - origin[1]*sin(rotation);
	newOrigin[1] = origin[0]*sin(rotation) + origin[1]*cos(rotation);
	newOrigin[2] = origin[2];
	VectorAdd(newOrigin, level.mOriginAdjust, newOrigin);
	// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
	Com_sprintf(temp, sizeof( temp ), "%0.0f %0.0f %0.0f", newOrigin[0], newOrigin[1], newOrigin[2]);
	AddSpawnField("origin", temp);

	G_SpawnString("angles", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		if ( sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] ) != 3 ) {
			trap->Print( "HandleEntityAdjustment: failed sscanf on 'angles' (%s)\n", value );
			VectorClear( angles );
		}

		angles[YAW] = fmod(angles[YAW] + level.mRotationAdjust, 360.0f);
		// damn VMs don't handle outputing a float that is compatible with sscanf in all cases
		Com_sprintf(temp, sizeof( temp ), "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
		AddSpawnField("angles", temp);
	}
	else
	{
		G_SpawnString("angle", NOVALUE, &value);
		if (Q_stricmp(value, NOVALUE) != 0)
		{
			angles[YAW] = atof( value );
		}
		else
		{
			angles[YAW] = 0.0;
		}
		angles[YAW] = fmod(angles[YAW] + level.mRotationAdjust, 360.0f);
		Com_sprintf(temp, sizeof( temp ), "%0.0f", angles[YAW]);
		AddSpawnField("angle", temp);
	}

	// RJR experimental code for handling "direction" field of breakable brushes
	// though direction is rarely ever used.
	G_SpawnString("direction", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		if ( sscanf( value, "%f %f %f", &angles[0], &angles[1], &angles[2] ) != 3 ) {
			trap->Print( "HandleEntityAdjustment: failed sscanf on 'direction' (%s)\n", value );
			VectorClear( angles );
		}
	}
	else
	{
		angles[0] = angles[1] = angles[2] = 0.0;
	}
	angles[YAW] = fmod(angles[YAW] + level.mRotationAdjust, 360.0f);
	Com_sprintf(temp, sizeof( temp ), "%0.0f %0.0f %0.0f", angles[0], angles[1], angles[2]);
	AddSpawnField("direction", temp);


	AddSpawnField("BSPInstanceID", level.mTargetAdjust);

	G_SpawnString("targetname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("targetname", temp);
	}

	G_SpawnString("target", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("target", temp);
	}

	G_SpawnString("killtarget", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("killtarget", temp);
	}

	G_SpawnString("brushparent", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushparent", temp);
	}

	G_SpawnString("brushchild", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("brushchild", temp);
	}

	G_SpawnString("enemy", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("enemy", temp);
	}

	G_SpawnString("ICARUSname", NOVALUE, &value);
	if (Q_stricmp(value, NOVALUE) != 0)
	{
		Com_sprintf(temp, sizeof( temp ), "%s%s", level.mTargetAdjust, value);
		AddSpawnField("ICARUSname", temp);
	}
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( qboolean inSubBSP ) {
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if ( !trap->GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		trap->Error( ERR_DROP, "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if ( !trap->GetEntityToken( keyname, sizeof( keyname ) ) ) {
			trap->Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) {
			break;
		}
		
		// parse value	
		if ( !trap->GetEntityToken( com_token, sizeof( com_token ) ) ) {
			trap->Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) {
			trap->Error( ERR_DROP, "G_ParseSpawnVars: closing brace without data" );
		}
		if ( level.numSpawnVars == MAX_SPAWN_VARS ) {
			trap->Error( ERR_DROP, "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		level.numSpawnVars++;
	}

	if (inSubBSP)
	{
		HandleEntityAdjustment();
	}

	return qtrue;
}

#ifdef __ENTITY_OVERRIDES__
/*
====================
G_ParseSpawnVarsEx

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVarsEx( int handle ) {
	pc_token_t	token;
	char		keyname[MAX_TOKEN_CHARS];
	
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if (trap->PC_ReadToken(handle, &token) == 0)
		// end of spawn string
		return qfalse;

	if (Q_stricmp(token.string, "{") != 0)
		trap->Error( ERR_DROP, "G_ParseSpawnVarsEx: found %s when expecting {", token.string );

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if (trap->PC_ReadToken( handle, &token) == 0)
			trap->Error( ERR_DROP, "G_ParseSpawnVarsEx: EOF without closing brace" );

		if (Q_stricmp(token.string, "}") == 0)
			break;
		
		strcpy(keyname, token.string);

		// parse value	
		if (trap->PC_ReadToken( handle, &token) == 0)
			trap->Error( ERR_DROP, "G_ParseSpawnVarsEx: EOF without closing brace" );

		if (Q_stricmp(token.string, "}") == 0)
			trap->Error( ERR_DROP,"G_ParseSpawnVarsEx: closing brace without data");

		if (level.numSpawnVars == MAX_SPAWN_VARS)
			trap->Error( ERR_DROP,"G_ParseSpawnVarsEx: MAX_SPAWN_VARS");

		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken(keyname);
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken(token.string);
		level.numSpawnVars++;
	}

	return qtrue;
}
#endif //__ENTITY_OVERRIDES__

static	char *defaultStyles[32][3] = 
{
	{	// 0 normal
		"z",
		"z",
		"z"
	},
	{	// 1 FLICKER (first variety)
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo",
		"mmnmmommommnonmmonqnmmo"
	},
	{	// 2 SLOW STRONG PULSE
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcb"
	},
	{	// 3 CANDLE (first variety)
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg"
	},
	{	// 4 FAST STROBE
		"mamamamamama",
		"mamamamamama",
		"mamamamamama"
	},
	{	// 5 GENTLE PULSE 1
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj"
	},
	{	// 6 FLICKER (second variety)
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno",
		"nmonqnmomnmomomno"
	},
	{	// 7 CANDLE (second variety)
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaaabcdefgmmmmaaaammmaamm"
	},
	{	// 8 CANDLE (third variety)
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa"
	},
	{	// 9 SLOW STROBE (fourth variety)
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz",
		"aaaaaaaazzzzzzzz"
	},
	{	// 10 FLUORESCENT FLICKER
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma",
		"mmamammmmammamamaaamammma"
	},
	{	// 11 SLOW PULSE NOT FADE TO BLACK
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"abcdefghijklmnopqrrqponmlkjihgfedcba"
	},
	{	// 12 FAST PULSE FOR JEREMY
		"mkigegik",
		"mkigegik",
		"mkigegik"
	},
	{	// 13 Test Blending
		"abcdefghijklmqrstuvwxyz",
		"zyxwvutsrqmlkjihgfedcba",
		"aammbbzzccllcckkffyyggp"
	},
	{	// 14
		"",
		"",
		""
	},
	{	// 15
		"",
		"",
		""
	},
	{	// 16
		"",
		"",
		""
	},
	{	// 17
		"",
		"",
		""
	},
	{	// 18
		"",
		"",
		""
	},
	{	// 19
		"",
		"",
		""
	},
	{	// 20
		"",
		"",
		""
	},
	{	// 21
		"",
		"",
		""
	},
	{	// 22
		"",
		"",
		""
	},
	{	// 23
		"",
		"",
		""
	},
	{	// 24
		"",
		"",
		""
	},
	{	// 25
		"",
		"",
		""
	},
	{	// 26
		"",
		"",
		""
	},
	{	// 27
		"",
		"",
		""
	},
	{	// 28
		"",
		"",
		""
	},
	{	// 29
		"",
		"",
		""
	},
	{	// 30
		"",
		"",
		""
	},
	{	// 31
		"",
		"",
		""
	}
};

void *precachedKyle = 0;
void scriptrunner_run (gentity_t *self);

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process

BSP Options
"gridsize"     size of lighting grid to "X Y Z". default="64 64 128"
"ambient"      scale of global light (from _color)
"fog"          shader name of the global fog texture - must include the full path, such as "textures/rj/fog1"
"distancecull" value for vis for the maximum viewing distance
"chopsize"     value for bsp on the maximum polygon / portal size
"ls_Xr"	override lightstyle X with this pattern for Red.
"ls_Xg"	green (valid patterns are "a-z")
"ls_Xb"	blue (a is OFF, z is ON)

"fogstart"		override fog start distance and force linear
*/
extern void EWebPrecache(void); //g_items.c
float g_cullDistance;

void SP_worldspawn( void ) 
{
	char		*text, temp[32];
	int			i;
	int			lengthRed, lengthBlue, lengthGreen;

	//I want to "cull" entities out of net sends to clients to reduce
	//net traffic on our larger open maps -rww
	G_SpawnFloat("distanceCull", "6000.0", &g_cullDistance);
	trap->SetServerCull(g_cullDistance);

	G_SpawnString( "classname", "", &text );
	if ( Q_stricmp( text, "worldspawn" ) ) {
		trap->Error( ERR_DROP, "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) 
	{
		if ( Q_stricmp( "spawnscript", level.spawnVars[i][0] ) == 0 )
		{//ONly let them set spawnscript, we don't want them setting an angle or something on the world.
			G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], &g_entities[ENTITYNUM_WORLD] );
		}
	}
	//The server will precache the standard model and animations, so that there is no hit
	//when the first client connnects.
	if (!BGPAFtextLoaded)
	{
		BG_ParseAnimationFile("models/players/_humanoid/animation.cfg", bgHumanoidAnimations, qtrue);
	}

	if (!precachedKyle)
	{
		int defSkin;

		trap->G2API_InitGhoul2Model(&precachedKyle, "models/players/kyle/model.glm", 0, 0, -20, 0, 0);

		if (precachedKyle && trap->G2_HaveWeGhoul2Models(precachedKyle)) // better safe than sorry --eez
		{
			defSkin = trap->R_RegisterSkin("models/players/kyle/model_default.skin");
			trap->G2API_SetSkin(precachedKyle, 0, defSkin, defSkin);
		}
	}

	if (!g2SaberInstance)
	{
		trap->G2API_InitGhoul2Model(&g2SaberInstance, "models/weapons2/saber/saber_w.glm", 0, 0, -20, 0, 0);

		if (g2SaberInstance && trap->G2_HaveWeGhoul2Models(g2SaberInstance))	// better safe than sorry --eez
		{
			// indicate we will be bolted to model 0 (ie the player) on bolt 0 (always the right hand) when we get copied
			trap->G2API_SetBoltInfo(g2SaberInstance, 0, 0);
			// now set up the gun bolt on it
			trap->G2API_AddBolt(g2SaberInstance, 0, "*blade1");
		}
	}

	// make some data visible to connecting client
	trap->SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	trap->SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	G_SpawnInt("serverinit", "0", &level.serverInit);

	G_SpawnString( "music", "", &text );
	trap->SetConfigstring( CS_MUSIC, text );

	G_SpawnString( "message", "", &text );
	trap->SetConfigstring( CS_MESSAGE, text );				// map specific message

	trap->SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	G_SpawnString( "gravity", "800", &text );
	trap->Cvar_Set( "g_gravity", text );
	trap->Cvar_Update( &g_gravity );

	G_SpawnString( "enableBreath", "0", &text );

	G_SpawnString( "soundSet", "default", &text );
	trap->SetConfigstring( CS_GLOBAL_AMBIENT_SET, text );

	G_SpawnString( "ambientHeatRate", "100", &text );		//how fast weapon overheating should cool (lower is faster cooling, default: 100)
	trap->Cvar_Set( "jkg_heatDissipateTime", text );
	trap->Cvar_Update( &jkg_heatDissipateTime );

	switch( level.gametype ) {
		case GT_DUEL:
		case GT_POWERDUEL:
			{
				G_SpawnString( "defaultWeapon", jkg_startingSaberDuel.string, &text );
				if(text && *text)
				{
					Q_strncpyz(level.startingWeapon, text, sizeof(level.startingWeapon));
					trap->Cvar_Set( "jkg_startingSaberDuel", text );
				}
				else
					Q_strncpyz(level.startingWeapon, jkg_startingSaberDuel.string, sizeof(level.startingWeapon));
			}
			break;
		default:
			{
				G_SpawnString( "defaultWeapon", "pistol_DL-18", &text );//pistol_DL-18
				if(text && *text)
				{
					Q_strncpyz(level.startingWeapon, text, sizeof(level.startingWeapon));
					trap->Cvar_Set( "jkg_startingGun", text );
				}
				else
					Q_strncpyz(level.startingWeapon, jkg_startingGun.string, sizeof(level.startingWeapon));
			}
			break;
	}
	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].r.ownerNum = ENTITYNUM_NONE;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	g_entities[ENTITYNUM_NONE].s.number = ENTITYNUM_NONE;
	g_entities[ENTITYNUM_NONE].r.ownerNum = ENTITYNUM_NONE;
	g_entities[ENTITYNUM_NONE].classname = "nothing";


	// see if we want a warmup time
	trap->SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) {
		trap->Cvar_Set( "g_restarted", "0" );
		trap->Cvar_Update( &g_restarted );
	} 

	// Gang Wars
	if( g_gametype.integer >= GT_TEAM )
	{
		char *redString, *blueString;
		char teamInfo[MAX_INFO_STRING];

		G_SpawnString("gwTeamRed", "red", &redString);
		G_SpawnString("gwTeamBlue", "blue", &blueString);
		
		teamInfo[0] = '\0';

		Info_SetValueForKey(teamInfo, "redTeam", redString);
		Info_SetValueForKey(teamInfo, "blueTeam", blueString);

		if(!Info_Validate(teamInfo))
		{
			Com_Error(ERR_FATAL, "GW: Info_Validate returned qfalse.\n");
			return;
		}

		trap->SetConfigstring(CS_TEAMS, teamInfo);

		level.redTeam = JKG_GetTeamByReference( redString );
		level.blueTeam = JKG_GetTeamByReference( blueString );

		if( level.redTeam < 0 || level.blueTeam < 0 )
		{
			Com_Printf("^3WARNING: Improper team for team %s\n", (level.redTeam < 0) ? "Red" : "Blue");
		}
	}

	trap->SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+0, defaultStyles[0][0]);
	trap->SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+1, defaultStyles[0][1]);
	trap->SetConfigstring(CS_LIGHT_STYLES+(LS_STYLES_START*3)+2, defaultStyles[0][2]);
	
	for(i=1;i<LS_NUM_STYLES;i++)
	{
		Com_sprintf(temp, sizeof(temp), "ls_%dr", i);
		G_SpawnString(temp, defaultStyles[i][0], &text);
		lengthRed = strlen(text);
		trap->SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+0, text);

		Com_sprintf(temp, sizeof(temp), "ls_%dg", i);
		G_SpawnString(temp, defaultStyles[i][1], &text);
		lengthGreen = strlen(text);
		trap->SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+1, text);

		Com_sprintf(temp, sizeof(temp), "ls_%db", i);
		G_SpawnString(temp, defaultStyles[i][2], &text);
		lengthBlue = strlen(text);
		trap->SetConfigstring(CS_LIGHT_STYLES+((i+LS_STYLES_START)*3)+2, text);

		if (lengthRed != lengthGreen || lengthGreen != lengthBlue)
		{
			Com_Error(ERR_DROP, "Style %d has inconsistent lengths: R %d, G %d, B %d", 
				i, lengthRed, lengthGreen, lengthBlue);
		}
	}		
}

//rww - Planning on having something here?
qboolean SP_bsp_worldspawn ( void )
{
	return qtrue;
}

void G_PrecacheSoundsets( void )
{
	gentity_t	*ent = NULL;
	int i;
	int countedSets = 0;

	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->soundSet && ent->soundSet[0])
		{
			if (countedSets >= MAX_AMBIENT_SETS)
			{
				Com_Error(ERR_DROP, "MAX_AMBIENT_SETS was exceeded! (too many soundsets)\n");
			}

			ent->s.soundSetIndex = G_SoundSetIndex(ent->soundSet);
			countedSets++;
		}
	}
}

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( qboolean inSubBSP ) {
#ifdef __ENTITY_OVERRIDES__
	int			handle;
	vmCvar_t	mapname;

	trap->Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
#endif //__ENTITY_OVERRIDES__

	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
#ifdef __ENTITY_OVERRIDES__
	//
	// UQ1: Allows different entities defintions to be loaded per gametype for a map with a .ovrents file...
	// This version overrides the bsp's spawns completely.
	//
	if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		handle = trap->PC_LoadSource(va("maps/%s_scenario.ovrents", mapname.string));
	else if (g_gametype.integer == GT_SINGLE_PLAYER)
		handle = trap->PC_LoadSource(va("maps/%s_coop.ovrents", mapname.string));
	else
		handle = trap->PC_LoadSource(va("maps/%s.ovrents", mapname.string));

	if (handle)
	{
		if ( !G_ParseSpawnVarsEx(handle) )
		{
			trap->Error( ERR_DROP, "SpawnEntities: no entities" );
			trap->PC_FreeSource(handle); // UQ added!
		}
	}
	else
#endif //__ENTITY_OVERRIDES__
	if ( !G_ParseSpawnVars(qfalse) ) {
		trap->Error( ERR_DROP, "SpawnEntities: no entities" );
	}

	if (!inSubBSP)
	{
		SP_worldspawn();
	}
	else
	{
		// Skip this guy if its worldspawn fails
		if ( !SP_bsp_worldspawn() )
		{
			return;
		}
	}

#ifdef __ENTITY_OVERRIDES__ // UQ added.. We need to pass all the new entity list, not just worldspawn...
	if (handle)
	{// Pass all the new .ovrents file's entities one at a time...
		while( G_ParseSpawnVarsEx(handle) )
			G_SpawnGEntityFromSpawnVars(inSubBSP);

		trap->PC_FreeSource(handle); // UQ: Release the handle here instead of above...
	}
	else
#endif //__ENTITY_OVERRIDES__
	{// Parse ents from the actual map bsp.
		while( G_ParseSpawnVars(inSubBSP) ) {
			G_SpawnGEntityFromSpawnVars(inSubBSP);
		}	
	}

#ifdef __ENTITY_OVERRIDES__
	//
	// UQ1: This version loads only 'extra' entitiyes from a .entities file...
	//
	if (!handle) 
	{
		// parse possible external entities map files
		// it's used to add new ents to existing pure ET map
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
			handle = trap->PC_LoadSource(va("maps/%s_scenario.entities", mapname.string));
		else if (g_gametype.integer == GT_SINGLE_PLAYER)
			handle = trap->PC_LoadSource(va("maps/%s_coop.entities", mapname.string));
		else
			handle = trap->PC_LoadSource(va("maps/%s.entities", mapname.string));

		if (handle)
		{
			if (G_ParseSpawnVarsEx(handle) == qfalse)
				trap->Error( ERR_DROP, "SpawnEntities: no entities" );

			// parse ents
			while (G_ParseSpawnVarsEx(handle))
				G_SpawnGEntityFromSpawnVars(inSubBSP);

			trap->PC_FreeSource(handle);
		}
	}
#endif //__ENTITY_OVERRIDES__

	if( g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN] && g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN][0] )
	{//World has a spawn script, but we don't want the world in ICARUS and running scripts,
		//so make a scriptrunner and start it going.
		gentity_t *script_runner = G_Spawn();
		if ( script_runner )
		{
			script_runner->behaviorSet[BSET_USE] = g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN];
			script_runner->count = 1;
			script_runner->think = scriptrunner_run;
			script_runner->nextthink = level.time + 100;

			if ( script_runner->inuse )
			{
				trap->ICARUS_InitEnt( (sharedEntity_t *)script_runner );
			}
		}
	}

	if (!inSubBSP)
	{
		level.spawning = qfalse;			// any future calls to G_Spawn*() will be errors
	}

	G_PrecacheSoundsets();
}

