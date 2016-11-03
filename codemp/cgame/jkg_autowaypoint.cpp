// FIXME: This needs to be in BG code, since it's shared between both games --eez

#include "qcommon/q_shared.h"
#include "cgame/cg_local.h"
#include "ui/ui_shared.h"
#include "game/surfaceflags.h"

#ifdef __AUTOWAYPOINT__

/*
=================================
Global Defines and Variables
=================================
*/

#define MAX_MAP_SIZE 16384

// FIXME: cvar these please, thanks. I had to clean up a bunch of recommented code around here --eez
float waypoint_distance_multiplier = 3.0f;
float area_distance_multiplier = 1.5f;

int waypoint_scatter_distance = 96;
int outdoor_waypoint_scatter_distance = 192;

qboolean optimize_again = qfalse;
qboolean DO_THOROUGH = qfalse;

// end FIXME

float		ENTITY_HEIGHTS[MAX_GENTITIES];
int			NUM_ENTITY_HEIGHTS = 0;
qboolean	ENTITY_HIGHTS_INITIALIZED = qfalse;

#define __TEST_CLEANER__ // Experimental New Cleaning Method...

qboolean AlreadyHaveEntityAtHeight( float height )
{
	int i = 0;

	for (i = 0; i < NUM_ENTITY_HEIGHTS; i++)
	{
		float heightDif = height - ENTITY_HEIGHTS[i];

		if (heightDif < 0) heightDif = ENTITY_HEIGHTS[i] - height;

		if (heightDif < 128) return qtrue;
	}

	return qfalse;
}

qboolean AIMOD_HaveEntityNearHeight( float height )
{
	int i = 0;

	for (i = 0; i < NUM_ENTITY_HEIGHTS; i++)
	{
		float heightDif = height - ENTITY_HEIGHTS[i];

		if (heightDif < 0) heightDif = ENTITY_HEIGHTS[i] - height;

		if (heightDif < 512) return qtrue;
	}

	return qfalse;
}

void AIMOD_MapEntityHeights()
{
	int i;

	if (ENTITY_HIGHTS_INITIALIZED) return; // Already made the list for this level...

	for (i = 0; i < MAX_GENTITIES; i++)
	{
		centity_t *cent = &cg_entities[i];

		if (!cent) continue;
		if (AlreadyHaveEntityAtHeight( cent->currentState.origin[2] )) continue;

		ENTITY_HEIGHTS[NUM_ENTITY_HEIGHTS] = cent->currentState.origin[2];
		NUM_ENTITY_HEIGHTS++;
	}

	trap->Print("^3*** ^3%s: ^5Mapped %i entity heights.\n", "AUTO-WAYPOINTER", NUM_ENTITY_HEIGHTS);
	trap->UpdateScreen();

	ENTITY_HIGHTS_INITIALIZED = qtrue;
}

float	BAD_HEIGHTS[1024];
int		NUM_BAD_HEIGHTS = 0;

qboolean AIMOD_IsWaypointHeightMarkedAsBad( vec3_t org )
{
	int i = 0;

	AIMOD_MapEntityHeights(); // Initialize the entity height list...

	if (!AIMOD_HaveEntityNearHeight(org[2]))
	{
		if (NUM_ENTITY_HEIGHTS > 0) // Only exit here if the level actually has entities!
			return qtrue;
	}

	for (i = 0; i < NUM_BAD_HEIGHTS; i++)
	{
		float heightDif = org[2] - BAD_HEIGHTS[i];

		if (heightDif < 0) heightDif = BAD_HEIGHTS[i] - org[2];

		if (heightDif < 96/*48*/) return qtrue;
	}

	return qfalse;
}

#define	G_MAX_SCRIPT_ACCUM_BUFFERS 10
#define MAX_NODELINKS       32              // Maximum Node Links (12)
#define MAX_NODES           48000//65536//19500//10000//8000//32000//16000	// Maximum Nodes (8000)
#define INVALID				-1

//vec3_t			botTraceMins = { -20, -20, -1 };
//vec3_t			botTraceMaxs = { 20, 20, 32 };
vec3_t			botTraceMins = { -15, -15, 0 };
vec3_t			botTraceMaxs = { 15, 15, 64 };

//  _  _         _       _____
// | \| |___  __| |___  |_   _|  _ _ __  ___ ___
// | .` / _ \/ _` / -_)   | || || | '_ \/ -_|_-<
// |_|\_\___/\__,_\___|   |_| \_, | .__/\___/__/
//                            |__/|_|
//------------------------------------------------------------------------------------

//===========================================================================
// Description  : Node flags + Link Flags...
#define NODE_INVALID				-1
#define NODE_MOVE					0       // Move Node
#define NODE_OBJECTIVE				1
#define NODE_TARGET					2
//#define NODE_TARGETSELECT			4
#define NODE_LAND_VEHICLE			4
#define NODE_FASTHOP				8
#define NODE_COVER					16
#define NODE_WATER					32
#define NODE_LADDER					64      // Ladder Node
#define	NODE_MG42					128		//node is at an mg42
#define	NODE_DYNAMITE				256
#define	NODE_BUILD					512
#define	NODE_JUMP					1024
#define	NODE_DUCK					2048
#define	NODE_ICE					4096	// Node is located on ice (slick)...
#define NODE_ALLY_UNREACHABLE		8192
#define NODE_AXIS_UNREACHABLE		16384
#define	NODE_AXIS_DELIVER			32768	//place axis should deliver stolen documents/objective
#define	NODE_ALLY_DELIVER			65536	//place allies should deliver stolen documents/objective

//===========================================================================
// Description  : NPC Node flags + Link Flags...
#define NPC_NODE_INVALID				-1
#define NPC_NODE_MOVE					0       // Move Node
#define NPC_NODE_OBJECTIVE				1		// Objective position... Do something... (Unused in 0.1 for npcs)
#define NPC_NODE_ROUTE_BEGIN			2		// The beginning of an NPC route...
#define NPC_NODE_ROUTE_END				4		// The end of an NPC route...
#define NPC_NODE_JUMP					8		// Need a jump here...
#define NPC_NODE_DUCK					16		// Need to duck here...
#define NPC_NODE_WATER					32		// Node is in/on water...
#define NPC_NODE_ICE					64      // Node is on ice...
#define	NPC_NODE_LADDER					128		// Ladder Node
#define	NPC_NODE_MG42					256		// Node is at an mg42
#define	NPC_NODE_BUILD					512		// Need to build something here... (Unused in 0.1 for npcs)
#define	NPC_NODE_ALLY_UNREACHABLE		1024	// Axis only...
#define	NPC_NODE_AXIS_UNREACHABLE		2048	// Allied only...
#define	NPC_NODE_COVER					4096	// Cover point...

// Node finding flags...
#define NODEFIND_BACKWARD			1      // For selecting nodes behind us.
#define NODEFIND_FORCED				2      // For manually selecting nodes (without using links)
#define NODEFIND_ALL				4      // For selecting all nodes

// Objective types...
#define	OBJECTIVE_DYNAMITE			0		//blow this objective up!
#define	OBJECTIVE_DYNOMITE			0		// if next objective is OBJ_DYNOMITE => engi's are important
#define	OBJECTIVE_STEAL				1		//steal the documents!
#define	OBJECTIVE_CAPTURE			2		//get the flag - not intented for checkpoint, but for spawn flags
#define	OBJECTIVE_BUILD				3		//get the flag - not intented for checkpoint, but for spawn flags
#define	OBJECTIVE_CARRY				4
#define	OBJECTIVE_FLAG				5
#define	OBJECTIVE_POPFLAG			6
#define	OBJECTIVE_AXISONLY			128
#define	OBJECTIVE_ALLYSONLY			256

// es_fix : added comments to flags
// This way the bot knows how to approach the next node
#define		PATH_NORMAL				0		// Bot moves normally to next node : ie equivalant to pressing the forward move key on your keyboard
#define		PATH_CROUCH				1		//bot should duck and walk toward next node
#define		PATH_SPRINT				2		//bot should sprint to next node
#define		PATH_JUMP				4		//bot should jump to next node
#define		PATH_WALK				8		//bot should walk to next node
#define		PATH_BLOCKED			16		//path to next node is blocked
#define		PATH_LADDER				32		//ladders!
#define		PATH_NOTANKS			64		//No Land Vehicles...
#define		PATH_DANGER				128		//path to next node is dangerous - Lava/Slime/Water

//------------------------------------------------------------------------------------

//  _  _         _       ___     _         _ _    _
// | \| |___  __| |___  | _ \___| |__ _  _(_) |__| |
// | .` / _ \/ _` / -_) |   / -_) '_ \ || | | / _` |
// |_|\_\___/\__,_\___| |_|_\___|_.__/\_,_|_|_\__,_|
//------------------------------------------------------------------------------------
qboolean    dorebuild;          // for rebuilding nodes
qboolean	shownodes;
qboolean	nodes_loaded;
//------------------------------------------------------------------------------------

//  _  _         _       _    _      _     ___ _               _
// | \| |___  __| |___  | |  (_)_ _ | |__ / __| |_ _ _ _  _ __| |_ _  _ _ _ ___
// | .` / _ \/ _` / -_) | |__| | ' \| / / \__ \  _| '_| || / _|  _| || | '_/ -_)
// |_|\_\___/\__,_\___| |____|_|_||_|_\_\ |___/\__|_|  \_,_\__|\__|\_,_|_| \___|
//------------------------------------------------------------------------------------
typedef struct nodelink_s       // Node Link Structure
{
    /*short*/ int       targetNode; // Target Node
    float           cost;       // Cost for PathSearch algorithm
	int				flags;
}nodelink_t;                    // Node Link Typedef
//------------------------------------------------------------------------------------
//  _  _         _       ___ _               _
// | \| |___  __| |___  / __| |_ _ _ _  _ __| |_ _  _ _ _ ___
// | .` / _ \/ _` / -_) \__ \  _| '_| || / _|  _| || | '_/ -_)
// |_|\_\___/\__,_\___| |___/\__|_|  \_,_\__|\__|\_,_|_| \___|
//------------------------------------------------------------------------------------
typedef struct node_s           // Node Structure
{
    vec3_t      origin;         // Node Origin
    int         type;           // Node Type
    short int   enodenum;		// Mostly just number of players around this node
	nodelink_t  links[MAX_NODELINKS];	// Store all links
	short int	objectNum[3];		//id numbers of any world objects associated with this node (used only with unreachable flags)
	int			objFlags;			//objective flags if this node is an objective node
	short int	objTeam;			//the team that should complete this objective
	short int	objType;			//what type of objective this is - see OBJECTIVE_XXX flags
	short int	objEntity;			//the entity number of what object to dynamite at a dynamite objective node
	//int			coverpointNum;		// Number of waypoints this node can be used as cover for...
	//int			coverpointFor[1024];	// List of all the waypoints this waypoint is cover for...
} node_t;                       // Node Typedef

typedef struct nodelink_convert_s       // Node Link Structure
{
    /*short*/ int       targetNode; // Target Node
    float           cost;       // Cost for PathSearch algorithm
}nodelink_convert_t;            // Node Link Typedef

typedef struct node_convert_s           // Node Structure
{
    vec3_t				origin;         // Node Origin
    int					type;           // Node Type
    short int			enodenum;		// Mostly just number of players around this node
	nodelink_convert_t  links[MAX_NODELINKS];	// Store all links
} node_convert_t;                       // Node Typedef
//------------------------------------------------------------------------------------

typedef struct enode_s
{
	int			link_node;
	int			num_routes;				// Number of alternate routes available, maximum 5
	int			routes[5];				// Possible alternate routes to reach the node
	team_t		team;
} enode_t;

int		number_of_nodes = 0;
int		optimized_number_of_nodes = 0;
int		aw_num_nodes = 0;
int		optimized_aw_num_nodes = 0;
node_t	*nodes;
node_t	*optimized_nodes;

#define BOT_MOD_NAME	"aimod"
//#define NOD_VERSION		1.1f
float NOD_VERSION = 1.1f;

float aw_percent_complete = 0.0f;
char task_string1[255];
char task_string2[255];
char task_string3[255];
char last_node_added_string[255];

int		aw_stage_start_time = 0;
float	aw_last_percent = 0;
int		aw_last_percent_time = 0;
int		aw_last_times[100];
int		aw_num_last_times = 0;

vec4_t	popBG			=	{0.f,0.f,0.f,0.3f};
vec4_t	popBorder		=	{0.28f,0.28f,0.28f,1.f};
vec4_t	popHover		=	{0.3f,0.3f,0.3f,1.f};
vec4_t	popText			=	{1.f,1.f,1.f,1.f};
vec4_t	popLime			=	{0.f,1.f,0.f,0.7f};
vec4_t	popCyan			=	{0.f,1.f,1.f,0.7f};
vec4_t	popRed			=	{1.f,0.f,0.f,0.7f};
vec4_t	popBlue			=	{0.f,0.f,1.f,0.7f};
vec4_t	popOrange		=	{1.f,0.63f,0.1f,0.7f};
vec4_t	popDefaultGrey	=	{0.38f,0.38f,0.38f,1.0f};
vec4_t	popLightGrey	=	{0.5f,0.5f,0.5f,1.0f};
vec4_t	popDarkGrey		=	{0.33f,0.33f,0.33f,1.0f};
vec4_t	popAlmostWhite	=	{0.83f,0.81f,0.71f,1.0f};
vec4_t	popAlmostBlack	=	{0.16f,0.16f,0.16f,1.0f};

#define POP_HUD_BORDERSIZE 1

/*
==============
CG_HorizontalPercentBar
	Generic routine for pretty much all status indicators that show a fractional
	value to the palyer by virtue of how full a drawn box is.

flags:
	left		- 1
	center		- 2		// direction is 'right' by default and orientation is 'horizontal'
	vert		- 4
	nohudalpha	- 8		// don't adjust bar's alpha value by the cg_hudalpha value
	bg			- 16	// background contrast box (bg set with bgColor of 'NULL' means use default bg color (1,1,1,0.25)
	spacing		- 32	// some bars use different sorts of spacing when drawing both an inner and outer box

	lerp color	- 256	// use an average of the start and end colors to set the fill color
==============
*/


// TODO: these flags will be shared, but it was easier to work on stuff if I wasn't changing header files a lot
#define BAR_LEFT		0x0001
#define BAR_CENTER		0x0002
#define BAR_VERT		0x0004
#define BAR_NOHUDALPHA	0x0008
#define BAR_BG			0x0010
// different spacing modes for use w/ BAR_BG
#define BAR_BGSPACING_X0Y5	0x0020
#define BAR_BGSPACING_X0Y0	0x0040

#define BAR_LERP_COLOR	0x0100

#define BAR_BORDERSIZE 2

void CG_FilledBar(float x, float y, float w, float h, float *startColor, float *endColor, const float *bgColor, float frac, int flags) {
	vec4_t	backgroundcolor = {1, 1, 1, 0.25f}, colorAtPos;	// colorAtPos is the lerped color if necessary
	int indent = BAR_BORDERSIZE;

	if( frac > 1 ) {
		frac = 1.f;
	}
	if( frac < 0 ) {
		frac = 0;
	}

	if((flags&BAR_BG) && bgColor) {	// BAR_BG set, and color specified, use specified bg color
		VectorCopy4(bgColor, backgroundcolor);
	}

	if(flags&BAR_LERP_COLOR) {
		colorAtPos[0] = (1.0f - frac) * startColor[0] + frac * endColor[0];
		colorAtPos[1] = (1.0f - frac) * startColor[1] + frac * endColor[1];
		colorAtPos[2] = (1.0f - frac) * startColor[2] + frac * endColor[2];
		colorAtPos[3] = (1.0f - frac) * startColor[3] + frac * endColor[3];
	}

	// background
	if((flags&BAR_BG)) {
		// draw background at full size and shrink the remaining box to fit inside with a border.  (alternate border may be specified by a BAR_BGSPACING_xx)
		CG_FillRect (	x,
						y,
						w,
						h,
						backgroundcolor );

		if(flags&BAR_BGSPACING_X0Y0) {			// fill the whole box (no border)

		} else if(flags&BAR_BGSPACING_X0Y5) {	// spacing created for weapon heat
			indent*=3;
			y+=indent;
			h-=(2*indent);

		} else {								// default spacing of 2 units on each side
			x+=indent;
			y+=indent;
			w-=(2*indent);
			h-=(2*indent);
		}
	}


	// adjust for horiz/vertical and draw the fractional box
	if(flags&BAR_VERT) {
		if(flags&BAR_LEFT) {	// TODO: remember to swap colors on the ends here
			y+=(h*(1-frac));
		} else if (flags&BAR_CENTER) {
			y+=(h*(1-frac)/2);
		}

		if(flags&BAR_LERP_COLOR) {
			CG_FillRect ( x, y, w, h * frac, colorAtPos );
		} else {
//			CG_FillRectGradient ( x, y, w, h * frac, startColor, endColor, 0 );
			CG_FillRect ( x, y, w, h * frac, startColor );
		}

	} else {

		if(flags&BAR_LEFT) {	// TODO: remember to swap colors on the ends here
			x+=(w*(1-frac));
		} else if (flags&BAR_CENTER) {
			x+=(w*(1-frac)/2);
		}

		if(flags&BAR_LERP_COLOR) {
			CG_FillRect ( x, y, w*frac, h, colorAtPos );
		} else {
//			CG_FillRectGradient ( x, y, w * frac, h, startColor, endColor, 0 );
			CG_FillRect ( x, y, w*frac, h, startColor );
		}
	}

}

void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
#if 0
	// adjust for wide screens
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		*x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
	}
#endif

/*	if ( (cg.showGameView) && cg.refdef_current->width ) {
		float xscale = ( ( cg.refdef_current->width / cgs.screenXScale ) / 640.f );
		float yscale = ( ( cg.refdef_current->height / cgs.screenYScale ) / 480.f );

		(*x) = (*x) * xscale + ( cg.refdef_current->x / cgs.screenXScale );
		(*y) = (*y) * yscale + ( cg.refdef_current->y / cgs.screenYScale );
		(*w) *= xscale;
		(*h) *= yscale;
	}*/

	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides2( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenXScale;
	trap->R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap->R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawTopBottom2( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenYScale;
	trap->R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	trap->R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawSides_NoScale( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	trap->R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap->R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawTopBottom_NoScale( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	trap->R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	trap->R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}

// CHRUKER: b076 - Scoreboard background had black lines drawn twice
void CG_DrawBottom_NoScale( float x, float y, float w, float h, float size ) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	trap->R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawRect_FixedBorder( float x, float y, float width, float height, int border, const float *color ) {
	trap->R_SetColor( color );

	CG_DrawTopBottom_NoScale( x, y, width, height, border );
	CG_DrawSides_NoScale( x, y, width, height, border );

	trap->R_SetColor( NULL );
}

void AIMod_AutoWaypoint_DrawProgress ( void )
{
	int				flags = 64|128;
	float			frac;//, time_frac;
	rectDef_t		rect;
	//int				time_taken = 0;
	int				total_seconds_left = 0;
	int				seconds_left = 0;
	int				minutes_left = 0;
	qboolean		estimating = qfalse;
	//int				avg_time = 0;

	if (aw_percent_complete == 0.0f)
	{// Init timer...
		aw_stage_start_time = trap->Milliseconds();
		aw_last_percent_time = aw_stage_start_time;
		aw_last_percent = 0;
		aw_num_last_times = 0;
	}

	if (aw_percent_complete > 100.0f)
	{// UQ: It can happen... Somehow... LOL!
		aw_percent_complete = 100.0f;
		aw_last_percent = 0;
		aw_num_last_times = 0;
	}

	if (aw_percent_complete != 0.0f)
	{
		/*int total_time;
		float time_frac2, done;

		time_frac = (aw_percent_complete/100);
		time_frac2 = 1-time_frac;
		
		time_taken = ( (trap->Milliseconds() - aw_stage_start_time) / 1000 );
		done = (float)(time_frac * (float)time_taken);

		// Ok, i have done % in x time...

		if (done > 0)
		{
			total_time = ( (time_taken*time_frac)+(time_taken*time_frac2) * time_taken);
			total_seconds_left = ((total_time / done) * time_frac2) * (time_frac/time_frac2);
			minutes_left = total_seconds_left/60;
			seconds_left = total_seconds_left-(minutes_left*60);
		}
		else
		{
			estimating = qtrue;
		}*/

		if (aw_percent_complete >= 1)
		{
			float percent_average_time = 0;
			int i;

			for (i = 0; i < aw_num_last_times; i++)
			{
				percent_average_time += aw_last_times[i];
			}

			if (aw_num_last_times >= 1)
			{
				percent_average_time /= aw_num_last_times;

				//avg_time = percent_average_time;

				//total_seconds_left = ((percent_average_time * 100) - (percent_average_time * aw_percent_complete)) / 1000;
				total_seconds_left = ((100 - aw_percent_complete) * percent_average_time) / 1000;
				minutes_left = total_seconds_left/60;
				seconds_left = total_seconds_left-(minutes_left*60);
			}
			else
			{
				estimating = qtrue;
			}

			if (aw_last_percent <= aw_percent_complete - 1 /*&& current_time - aw_last_percent_time > 1000*/)
			{
				int current_time = trap->Milliseconds();
				float percent_time = (current_time-aw_last_percent_time);

				if (percent_time < 1)
					percent_time = 1;

				aw_last_percent = aw_percent_complete;
				aw_last_percent_time = current_time;

				if (aw_num_last_times >= 100)
				{
					for (i = 1; i < aw_num_last_times; i++)
					{
						aw_last_times[i-1] = aw_last_times[i];
					}

					aw_last_times[99] = percent_time;
				}
				else
				{
					aw_last_times[aw_num_last_times] = percent_time;
					aw_num_last_times++;
				}
			}
		}
		else
		{
			estimating = qtrue;
		}
	}
	else
	{
		estimating = qtrue;
	}

	// Draw the bar!
	frac = (float)((aw_percent_complete)*0.01);

	rect.w = 500;
	rect.h = 30;
	
	rect.x = 69;
	rect.y = 369;

	// draw the background, then filled bar, then border
	CG_FillRect( rect.x, rect.y, rect.w, rect.h, popBG );
	CG_FilledBar( rect.x, rect.y, rect.w, rect.h, popRed, NULL, NULL, frac, flags );
	CG_DrawRect_FixedBorder( rect.x, rect.y, rect.w, rect.h, POP_HUD_BORDERSIZE, popBorder );

	CG_Text_Paint((rect.x), (rect.y + (rect.h*0.5) - 120), 0.5f, colorWhite, va("^3AUTO-WAYPOINTING^5 - Please wait..."), 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_MEDIUM );
	CG_Text_Paint((rect.x), (rect.y + (rect.h*0.5) - 80), 0.5f, colorWhite, task_string1, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_MEDIUM );
	CG_Text_Paint((rect.x), (rect.y + (rect.h*0.5) - 60), 0.5f, colorWhite, task_string2, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_MEDIUM );
	CG_Text_Paint((rect.x), (rect.y + (rect.h*0.5) - 40), 0.5f, colorWhite, task_string3, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_MEDIUM );
	CG_Text_Paint((rect.x + (rect.w*0.5) - 35), (rect.y + (rect.h*0.5) - 18/*+ 8*/), 1.0f, colorWhite, va("^7%.2f%%", aw_percent_complete), 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_LARGE );
	
	if (!estimating && aw_percent_complete > 1.0f)
		CG_Text_Paint((rect.x + 160), (rect.y + (rect.h*0.5) + 16), 0.5f, colorWhite, va("^3%i ^5minutes ^3%i ^5seconds remaining (estimated)", minutes_left, seconds_left), 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL );
	else
		CG_Text_Paint((rect.x + 160), (rect.y + (rect.h*0.5) + 16), 0.5f, colorWhite, va("^5    ... estimating time remaining ..."), 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL );

	CG_Text_Paint((rect.x + 100/*80*/), (rect.y + (rect.h*0.5) + 30), 0.5f, colorWhite, last_node_added_string, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_MEDIUM );

	trap->R_SetColor( NULL );
}

qboolean AW_Map_Has_Waypoints ( void )
{
	fileHandle_t	f;
	char			filename[60];
	int				len = 0;
	vmCvar_t		mapname;

	strcpy( filename, "nodes/" );

	////////////////////
	trap->Cvar_VariableStringBuffer( "g_scriptName", filename, sizeof(filename) );
	if ( strlen( filename) > 0 )
	{
		trap->Cvar_Register( &mapname, "g_scriptName", "", CVAR_ROM );
	}
	else
	{
		trap->Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
	}

	Q_strcat( filename, sizeof(filename), mapname.string );

	///////////////////
	//open the node file for reading, return false on error
	len = trap->FS_Open( va( "nodes/%s.bwp", filename), &f, FS_READ );
	trap->FS_Close(f);
		
	if( len <= 0 )
	{
		Com_Printf("^1ERROR: Could not load AWP node file nodes/%s.bwp\n", filename);
		return qfalse;
	}

	return qtrue;
}


/* */
float
VectorDistanceNoHeight ( vec3_t v1, vec3_t v2 )
{
	vec3_t	dir;
	vec3_t	v1a, v2a;
	VectorCopy( v1, v1a );
	VectorCopy( v2, v2a );
	v2a[2] = v1a[2];
	VectorSubtract( v2a, v1a, dir );
	return ( VectorLength( dir) );
}


/* */
float
HeightDistance ( vec3_t v1, vec3_t v2 )
{
	vec3_t	dir;
	vec3_t	v1a, v2a;
	VectorCopy( v1, v1a );
	VectorCopy( v2, v2a );
	v2a[0] = v1a[0];
	v2a[1] = v1a[1];
	VectorSubtract( v2a, v1a, dir );
	return ( VectorLength( dir) );
}

qboolean
NodeIsOnMover ( vec3_t org1 )
{
	trace_t tr;
	vec3_t	newOrg, newOrgDown;

	VectorCopy( org1, newOrg );

	newOrg[2] += 8; // Look from up a little...

	VectorCopy( org1, newOrgDown );
	newOrgDown[2] = -64000.0f;

	CG_Trace( &tr, newOrg, NULL, NULL, newOrgDown, -1, MASK_PLAYERSOLID );
	
	if ( tr.fraction == 1 )
	{
		return ( qfalse );
	}

	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (cg_entities[tr.entityNum].currentState.eType == ET_MOVER)
			return ( qtrue );
	}

	return ( qfalse );
}

/* */
qboolean
BAD_WP_Height ( vec3_t start, vec3_t end )
{
	/*float distance = VectorDistanceNoHeight(start, end);
	float height_diff = HeightDistance(start, end);

	if (distance > 8)
	{// < 8 is probebly a ladder...
		if ((start[2] + 32 < end[2]) 
			&& (height_diff*1.5) > distance 
			&& distance > 48)
			return qtrue;

		if ((start[2] < end[2] + 48) 
			&& (height_diff*1.5) > distance 
			&& distance > 48)
			return qtrue;
	}*/

	if (HeightDistance(start, end) > 64 && start[2] < end[2])
	{
		if (!NodeIsOnMover(start))
			return qtrue;
	}

	return ( qfalse );
}

/* */
qboolean
BAD_WP_Distance ( vec3_t start, vec3_t end )
{
	qboolean hitsmover = qfalse;
	float distance = Distance( start, end );
	float height_diff = HeightDistance(start, end);
	float length_diff = VectorDistanceNoHeight(start, end);

	if (distance > waypoint_scatter_distance*waypoint_distance_multiplier)
	{
		return ( qtrue );
	}

	if (NodeIsOnMover(start))
	{
		// Too far, even for mover node...
		hitsmover = qtrue;
	}

	if (!hitsmover && length_diff * 0.8 < height_diff)
	{
		// This slope looks too sharp...
		return ( qtrue );
	}

	// Looks good...
	return ( qfalse );
}

qboolean HasPortalFlags ( int surfaceFlags, int contents )
{
	if ( ( (surfaceFlags & SURF_NOMARKS) && (surfaceFlags & SURF_NOIMPACT) && (surfaceFlags & SURF_NODRAW) && (contents & CONTENTS_PLAYERCLIP) && (contents & CONTENTS_TRANSLUCENT) ) )
		return qtrue;

	return qfalse;
}

qboolean VisibleAllowEntType ( int type, int flags )
{
	switch (type)
	{
	case ET_GENERAL:
	case ET_PLAYER:
	case ET_ITEM:
	//case ET_MISSILE:
	//case ET_SPECIAL:				// rww - force fields
	case ET_BEAM:
	case ET_PORTAL:
	case ET_SPEAKER:
	//case ET_PUSH_TRIGGER:
	//case ET_TELEPORT_TRIGGER:
	case ET_INVISIBLE:
	case ET_NPC:					// ghoul2 player-like entity
	case ET_TEAM:
	case ET_BODY:
	//case ET_TERRAIN:
	//case ET_FX:
#ifdef __DOMINANCE__
	case ET_FLAG:
#endif //__DOMINANCE__
		return ( qtrue );
	case ET_MOVER:
		//if (flags & EF_JETPACK_ACTIVE)
			return ( qtrue );
	default:
		break;
	}

	return ( qfalse );
}

qboolean AIMod_AutoWaypoint_Check_PlayerWidth ( vec3_t origin ) 
{
	trace_t		trace;
	vec3_t		org, destorg;

	//Offset the step height
	//vec3_t	mins = {-18, -18, 0};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	mins = {-24, -24, 0};
	vec3_t	maxs = {24, 24, 48};
	//vec3_t	mins = {-48, -48, 0};
	//vec3_t	maxs = {48, 48, 48};

	VectorCopy(origin, org);
	org[2]+=18;
	VectorCopy(origin, destorg);
	destorg[2]+=18;

	CG_Trace( &trace, org, mins, maxs, destorg, -1, MASK_PLAYERSOLID );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		return qfalse;
	}

	//if ( trace.contents == 0 && trace.surfaceFlags == 0 )
	//{// Dont know what to do about these... Gonna try disabling them and see what happens...
	//	return qfalse;
	//}

	if ( trace.contents == CONTENTS_WATER || trace.contents == CONTENTS_LAVA )
	{
		return qfalse;
	}

	return qtrue;
}

float FloorHeightAt ( vec3_t org ); // below

int CheckHeightsBetween( vec3_t from, vec3_t dest )
{
	vec3_t	from_point, to_point, current_point;
	vec3_t	dir, forward;
	float	distance;

	VectorCopy(from, from_point);
	from_point[2]+=32;
	VectorCopy(dest, to_point);
	to_point[2]+=32;

	distance = Distance(from_point, to_point);

	VectorSubtract(to_point, from_point, dir);
	vectoangles(dir, dir);

	AngleVectors( from_point, forward, NULL, NULL );

	VectorCopy(from_point, current_point);

	while (distance > -15)
	{
		float floor = 0;

		VectorMA( current_point, 16, forward, current_point );

		floor = FloorHeightAt(current_point);

		if (floor > 65000.0f || floor < -65000.0f || floor < from_point[2] - 128.0f)
		{// Found a bad drop!
			return 0;
		}

		distance -= 16.0f; // Subtract this move from the distance...
	}

	//we made it!
	return 1;
}

int NodeVisible_WithExtraHeightAtTraceEnd( vec3_t from, vec3_t dest, int ignore )
{
	trace_t		trace;
	vec3_t		org, destorg;
	int			j = 0;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, 48-STEPSIZE};

	VectorCopy(from, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	CG_Trace( &trace, org, mins, maxs, destorg, ignore, MASK_PLAYERSOLID );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		return 0;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if (trace.entityNum != ENTITYNUM_NONE 
			&& trace.entityNum < ENTITYNUM_MAX_NORMAL )
		{
			if (VisibleAllowEntType(cg_entities[trace.entityNum].currentState.eType, cg_entities[trace.entityNum].currentState.eFlags))
				if (CheckHeightsBetween(from, dest) != 0)
					return ( 1 );
		}

		return 0;
	}

	// Doors... Meh!
	for (j = MAX_CLIENTS; j < MAX_GENTITIES; j++)
	{
		centity_t *ent = &cg_entities[j];

		if (!ent) continue;

		// Too far???
		if (Distance(from, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier
			|| Distance(from, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier)
			continue;

		if (ent->currentState.eType == ET_GENERAL || ent->currentState.eType == ET_SPEAKER)
		{
			if (CheckHeightsBetween(from, dest) != 0)
				return ( 2 ); // Doors???
		}
	}

	// Check heights along this route for falls...
	if (CheckHeightsBetween(from, dest) == 0)
		return 0; // Bad height (drop) in between these points...

	//we made it!
	return 1;
}

//0 = wall in way
//1 = player or no obstruction
//2 = useable door in the way.
//3 = team door entity in the way.
int NodeVisible( vec3_t from, vec3_t dest, int ignore )
{
	trace_t		trace;
	vec3_t		org, destorg;
	int			j = 0;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, 48-STEPSIZE};

	VectorCopy(from, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	CG_Trace( &trace, org, mins, maxs, destorg, ignore, MASK_PLAYERSOLID );

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		return 0;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if (trace.entityNum != ENTITYNUM_NONE 
			&& trace.entityNum < ENTITYNUM_MAX_NORMAL )
		{
			if (VisibleAllowEntType(cg_entities[trace.entityNum].currentState.eType, cg_entities[trace.entityNum].currentState.eFlags))
				//if (CheckHeightsBetween(from, dest) != 0)
					return ( 1 );
		}

		// Instead of simply failing, first check if looking from above the trace end a little would see over a bump (steps)...
		//if (NodeVisible_WithExtraHeightAtTraceEnd( trace.endpos, dest, ignore ) == 0)
			return 0;
	}

	// Doors... Meh!
	for (j = MAX_CLIENTS; j < MAX_GENTITIES; j++)
	{
		centity_t *ent = &cg_entities[j];

		if (!ent) continue;

		// Too far???
		if (Distance(from, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier
			|| Distance(from, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier)
			continue;

		if (ent->currentState.eType == ET_GENERAL || ent->currentState.eType == ET_SPEAKER)
		{
			//if (CheckHeightsBetween(from, dest) != 0)
				return ( 2 ); // Doors???
		}
	}

	// Check heights along this route for falls...
	//if (CheckHeightsBetween(from, dest) == 0)
	//	return 0; // Bad height (drop) in between these points...

	//we made it!
	return 1;
}

//0 = wall in way
//1 = player or no obstruction
//2 = useable door in the way.
//3 = team door entity in the way.
int
NodeVisible_OLD ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	vec3_t	newOrg, newOrg2;
	vec3_t	mins, maxs;

//	vec3_t			traceMins = { -10, -10, -1 };
//	vec3_t			traceMaxs = { 10, 10, 48/*32*/ };
	vec3_t			traceMins = { -15, -15, 0 };
	vec3_t			traceMaxs = { 15, 15, 32 };

	VectorCopy( traceMins, mins );
	VectorCopy( traceMaxs, maxs );
	VectorCopy( org2, newOrg );
	VectorCopy( org1, newOrg2 );

	newOrg[2] += 16/*8*/; // Look from up a little...
	newOrg2[2] += 16/*8*/; // Look from up a little...

	CG_Trace( &tr, newOrg2, NULL/*mins*/, NULL/*maxs*/, newOrg, ignore, MASK_PLAYERSOLID /*| CONTENTS_LAVA*//*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA*/ /*MASK_WATER*/ );

	if ( tr.startsolid )
	{
		//trap->Print("START SOLID!\n");
		return ( 0 );
	}

	if ( tr.fraction == 1 /*&& !(tr.contents & CONTENTS_LAVA)*/ /*|| HasPortalFlags(tr.surfaceFlags, tr.contents)*/ )
	{
		return ( 1 );
	}

	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (VisibleAllowEntType(cg_entities[tr.entityNum].currentState.eType, cg_entities[tr.entityNum].currentState.eFlags))
			return ( 1 );
	}

	// Search for door triggers...
	//if ( tr.fraction != 1 )
	{
		//if ((tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE))
		{
			int j = 0;

			for (j = MAX_CLIENTS; j < MAX_GENTITIES; j++)
			{
				centity_t *ent = &cg_entities[j];

				if (!ent) continue;

				// Too far???
				if (Distance(org1, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier
					|| Distance(org2, ent->currentState.pos.trBase) > waypoint_scatter_distance*waypoint_distance_multiplier)
					continue;
				
				if (ent->currentState.eType == ET_GENERAL || ent->currentState.eType == ET_SPEAKER)
				{
					return ( 2 ); // Doors???
				}
			}
		}
	}

	//trap->Print("NO VIS!\n");
	return ( 0 );
}

int
NodeVisibleJump ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	vec3_t	newOrg, newOrg2;
	vec3_t	mins, maxs;

	//vec3_t			botTraceMins = { -20, -20, -1 };
	//vec3_t			botTraceMaxs = { 20, 20, 32 };
	//vec3_t			traceMins = { -20, -20, -1 };
	//vec3_t			traceMaxs = { 20, 20, 32 };

//	vec3_t			traceMins = { -10, -10, -1 };
//	vec3_t			traceMaxs = { 10, 10, 48/*32*/ };
	vec3_t			traceMins = { -15, -15, 0 };
	vec3_t			traceMaxs = { 15, 15, 64 };

	//vec3_t	traceMins = { -10, -10, -1 };
	//vec3_t	traceMaxs = { 10, 10, 15 };

	VectorCopy( traceMins, mins );
	VectorCopy( traceMaxs, maxs );
	VectorCopy( org1, newOrg );
	VectorCopy( org1, newOrg2 );

	// UQ1: First check the up position is reachable (for inward sloped walls)
	newOrg[2] += 2; // Look from up a little...
	newOrg2[2] += 18; // Look from up a little...

	CG_Trace( &tr, newOrg2, mins, maxs, newOrg, ignore, MASK_PLAYERSOLID );

	if ( tr.startsolid || tr.fraction != 1.0f )
	{
		return ( 0 );
	}

	VectorCopy( org2, newOrg );
	VectorCopy( org2, newOrg2 );

	newOrg[2] += 2; // Look from up a little...
	newOrg2[2] += 18; // Look from up a little...

	CG_Trace( &tr, newOrg2, mins, maxs, newOrg, ignore, MASK_PLAYERSOLID );

	if ( tr.startsolid || tr.fraction != 1.0f )
	{
		return ( 0 );
	}

	// Init the variables for the actual (real) vis check...
	VectorCopy( org2, newOrg );
	VectorCopy( org1, newOrg2 );

	newOrg[2] += 16; // Look from up a little...
	newOrg2[2] += 16; // Look from up a little...

	//CG_Trace(&tr, newOrg, mins, maxs, org2, ignore, MASK_SHOT);
	CG_Trace( &tr, newOrg2, mins, maxs, newOrg, ignore, MASK_PLAYERSOLID /*| CONTENTS_LAVA*//*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA*/ /*MASK_WATER*/ );

	if ( tr.startsolid )
	{
		return ( 0 );
	}

	if ( tr.fraction == 1 /*&& !(tr.contents & CONTENTS_LAVA)*/ /*|| HasPortalFlags(tr.surfaceFlags, tr.contents)*/ )
	{
		return ( 1 );
	}

	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (VisibleAllowEntType(cg_entities[tr.entityNum].currentState.eType, cg_entities[tr.entityNum].currentState.eFlags))
			return ( 1 );
	}

	return ( 0 );
}

int
NodeVisibleCrouch ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	vec3_t	newOrg, newOrg2;
	vec3_t	mins, maxs;

	//vec3_t			botTraceMins = { -20, -20, -1 };
	//vec3_t			botTraceMaxs = { 20, 20, 32 };
	//vec3_t			traceMins = { -20, -20, -1 };
	//vec3_t			traceMaxs = { 20, 20, 32 };

//	vec3_t			traceMins = { -10, -10, -1 };
//	vec3_t			traceMaxs = { 10, 10, 32/*16*/ };
	vec3_t			traceMins = { -15, -15, 0 };
	vec3_t			traceMaxs = { 15, 15, 64 };

	//vec3_t	traceMins = { -10, -10, -1 };
	//vec3_t	traceMaxs = { 10, 10, 15 };

	VectorCopy( traceMins, mins );
	VectorCopy( traceMaxs, maxs );
	VectorCopy( org2, newOrg );
	VectorCopy( org1, newOrg2 );

	newOrg[2] += 1; // Look from up a little...
	newOrg2[2] += 1; // Look from up a little...

	//CG_Trace(&tr, newOrg, mins, maxs, org2, ignore, MASK_SHOT);
	CG_Trace( &tr, newOrg2, mins, maxs, newOrg, ignore, MASK_PLAYERSOLID /*| CONTENTS_LAVA*//*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA*/ /*MASK_WATER*/ );

	if ( tr.startsolid )
	{
		return ( 0 );
	}

	if ( tr.fraction == 1 /*&& !(tr.contents & CONTENTS_LAVA)*/ /*|| HasPortalFlags(tr.surfaceFlags, tr.contents)*/ )
	{
		return ( 1 );
	}

/*	if (tr.surfaceFlags & SURF_LADDER)
	{
		return ( 1 );
	}
*/
	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (VisibleAllowEntType(cg_entities[tr.entityNum].currentState.eType, cg_entities[tr.entityNum].currentState.eFlags))
			return ( 1 );
	}

	return ( 0 );
}

//extern int OrgVisibleBox(vec3_t org1, vec3_t mins, vec3_t maxs, vec3_t org2, int ignore);
//special node visibility check for bot linkages..
//0 = wall in way
//1 = player or no obstruction
//2 = useable door in the way.

//3 = team door entity in the way.
int TankNodeVisible ( vec3_t org1, vec3_t org2, vec3_t mins, vec3_t maxs, int ignore )
{
	trace_t tr;
	vec3_t	newOrg, newOrg2;
	VectorCopy( org1, newOrg2 );
	VectorCopy( org2, newOrg );

	newOrg[2] += 32;		// Look from up a little...
	newOrg2[2] += 32;	// Look from up a little...

	CG_Trace( &tr, newOrg2, mins, maxs, newOrg, ignore, MASK_PLAYERSOLID /*| CONTENTS_LAVA*//*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA*/ /*MASK_WATER*/ );
	
	if ( tr.startsolid )
	{
		return 0;
	}

	if ( tr.fraction == 1 /*&& !(tr.contents & CONTENTS_LAVA)*/ || HasPortalFlags(tr.surfaceFlags, tr.contents) )
	{
		return 1;
	}

	return 0;
}

int			num_cover_spots = 0;
int			cover_nodes[MAX_NODES];

//standard visibility check
int
OrgVisible ( vec3_t org1, vec3_t org2, int ignore )
{
	trace_t tr;
	CG_Trace( &tr, org1, NULL, NULL, org2, ignore, MASK_SOLID | MASK_OPAQUE | MASK_WATER );
	if ( tr.fraction == 1 )
	{
		return ( 1 );
	}

	return ( 0 );
}

void AIMOD_SaveCoverPoints ( void )
{
	fileHandle_t	f;
	int				i;

	trap->Print( "AWP: Saving cover-point file\n", "AUTO-WAYPOINTER" );

	///////////////////
	//try to open the output file, return if it failed
	trap->FS_Open( va( "nodes/%s.cpw", cgs.rawmapname), &f, FS_WRITE );
	if ( !f )
	{
		trap->Print( "^1AWP ERROR: Error opening cover point file /nodes/%s.cpw for writing\n", "AUTO-WAYPOINTER", cgs.rawmapname );
		return;
	}

	trap->FS_Write( &number_of_nodes, sizeof(int), f );							//write the number of nodes in the map
									//write the map name
	trap->FS_Write( &num_cover_spots, sizeof(int), f );							//write the number of nodes in the map

	for ( i = 0; i < num_cover_spots; i++ )											//loop through all the nodes
	{
		//int j = 0;

		trap->FS_Write( &(cover_nodes[i]), sizeof(int), f );

		/*
		// UQ1: Now write the spots this is a coverpoint for...
		trap->FS_Write( &(nodes[cover_nodes[i]].coverpointNum), sizeof(int), f );

		// And then save each one...
		for ( j = 0; j < nodes[cover_nodes[i]].coverpointNum; j++)
		{
			trap->FS_Write( &(nodes[cover_nodes[i]].coverpointFor[j]), sizeof(int), f );
		}
		*/
	}

	trap->FS_Close( f );		

	trap->Print( "^3*** ^3%s: ^5Cover point table saved to file ^7/nodes/%s.cpw^5.\n", "AUTO-WAYPOINTER", cgs.rawmapname );
}

qboolean AIMOD_LoadCoverPoints ( void )
{
	//FILE			*pIn;
	int				i = 0;
	fileHandle_t	f;
	int				num_map_waypoints = 0;

	// Init...
	num_cover_spots = 0;

	trap->FS_Open( va( "nodes/%s.cpw", cgs.rawmapname), &f, FS_READ );

	if (!f)
	{
		trap->Print( "^1ERROR: Failed to load coverpoint file /nodes/%s.cpw\n", cgs.rawmapname );
		return qfalse;
	}

	trap->FS_Read( &num_map_waypoints, sizeof(int), f );

	if (num_map_waypoints != number_of_nodes)
	{// Is an old file! We need to make a new one!
		trap->Print( "^1ERROR: /nodes/%s.cpw uses incorrect file version, suggest recreate using /awp..\n", cgs.rawmapname );
		trap->FS_Close( f );
		return qfalse;
	}

	trap->FS_Read( &num_cover_spots, sizeof(int), f );	

	for ( i = 0; i < num_cover_spots; i++ )
	{
		trap->FS_Read( &(cover_nodes[i]), sizeof(int), f );

		if (!(nodes[cover_nodes[i]].type & NODE_COVER))
			nodes[cover_nodes[i]].type |= NODE_COVER;

		//trap->Print("Cover spot #%i (node %i) is at %f %f %f.\n", i, cover_nodes[i], nodes[cover_nodes[i]].origin[0], nodes[cover_nodes[i]].origin[1], nodes[cover_nodes[i]].origin[2]);
	}

	trap->FS_Close( f );

	trap->Print( "^1*** ^3%s^5: Successfully loaded %i cover points from file ^7/nodes/%s.cpw^5.\n", GAME_VERSION, num_cover_spots, cgs.rawmapname);

	return qtrue;
}

void AIMOD_Generate_Cover_Spots ( void )
{
	{// Need to make some from waypoint list if we can!
		if (number_of_nodes > 32000)
		{
			trap->Print("^3*** ^3%s: ^5Too many waypoints to make cover spots. Use ^3/awo^5 (auto-waypoint optimizer) to reduce the numbers!\n", "AUTO-WAYPOINTER");
			return;
		}

		num_cover_spots = 0;

		if (number_of_nodes > 0)
		{
			int i = 0;
			int update_timer = 0;

			trap->Print( "^1*** ^3%s^1: ^5Generating and saving coverspot waypoints list.\n", "AUTO-WAYPOINTER" );
			strcpy( task_string3, va("^5Generating and saving coverspot waypoints list...") );
			trap->UpdateScreen();

			aw_percent_complete = 0.0f;

			for (i = 0; i < number_of_nodes; i++)
			{
				int			j = 0;
				vec3_t		up_org2;
				//qboolean	IS_GOOD_COVERPOINT = qfalse;

				//nodes[i].coverpointNum = 0;

				// Draw a nice little progress bar ;)
				aw_percent_complete = (float)((float)i/(float)number_of_nodes)*100.0f;

				update_timer++;

				if (update_timer >= 100)
				{
					trap->UpdateScreen();
					update_timer = 0;
				}

				for (j = 0; j < number_of_nodes; j++)
				{
					if (Distance(nodes[i].origin, nodes[j].origin) < 256.0f)
					{
						vec3_t up_org;

						VectorCopy(nodes[j].origin, up_org);
						up_org[2]+=DEFAULT_VIEWHEIGHT; // Standing height!

						VectorCopy(nodes[i].origin, up_org2);
						up_org2[2]+=DEFAULT_VIEWHEIGHT; // Standing height!

						if (!OrgVisible(up_org, up_org2, -1))
						{
							/*
							IS_GOOD_COVERPOINT = qtrue;
							nodes[i].coverpointFor[nodes[i].coverpointNum] = j;
							nodes[i].coverpointNum++;

							if (nodes[i].coverpointNum > 1023)
								break; // Maxed them out...
							*/
							nodes[i].type |= NODE_COVER;
							cover_nodes[num_cover_spots] = i;
							num_cover_spots++;

							strcpy( last_node_added_string, va("^5Waypoint ^3%i ^5set as a cover waypoint.", i) );
							break;
						}
					}
				}

				/*
				if (IS_GOOD_COVERPOINT)
				{
					nodes[i].type |= NODE_COVER;
					cover_nodes[num_cover_spots] = i;
					num_cover_spots++;

					strcpy( last_node_added_string, va("^5Waypoint ^3%i ^5set as a cover waypoint for %i other waypoints.", i, nodes[i].coverpointNum) );
				}
				*/
			}

			aw_percent_complete = 0.0f;
			trap->UpdateScreen();
			update_timer = 0;

			//if (bot_debug.integer)
			{
				trap->Print( "^1*** ^3%s^1: ^5 Generated ^7%i^5 coverspot waypoints.\n", "AUTO-WAYPOINTER", num_cover_spots );
			}

			// Save them for fast loading next time!
			AIMOD_SaveCoverPoints();
		}
	}
}


qboolean AIMod_Check_Slope_Between ( vec3_t org1, vec3_t org2 ) {  
	/*int i;
	vec3_t	orgA, orgB;
	trace_t tr;
	vec3_t	forward, right, up, start, end, dir;
	vec3_t	angles;
	vec3_t	testangles;
	vec3_t	boxMins= {-8, -8, -8}; // @fixme , tune this to be more smooth on delailed terrian (eg. railroad )
	vec3_t	boxMaxs= {8, 8, 8};
	float	pitch, roll, yaw, roof;
	vec3_t	slopeangles;

	VectorCopy(org1, orgA);
	VectorSubtract(org2, org1, dir);
	vectoangles(dir, testangles);
	AngleVectors( testangles, forward, right, up );
	VectorMA(orgA, (VectorDistanceNoHeight(org1, org2)*0.5), forward, orgA);

	roof = RoofHeightAt( orgA );
	roof -= 16;
	orgA[2] = roof;

	VectorCopy(orgA, orgB);
	orgB[2] = -65000;
	
	CG_Trace( &tr, orgA, boxMins, boxMaxs, orgB, -1, MASK_PLAYERSOLID );
	
	if (tr.endpos[2]+64 < org1[2])
		return qfalse;

	if (tr.endpos[2]-48 > org1[2])
		return qfalse;

	if ( tr.fraction == 1 || (tr.contents & CONTENTS_LAVA) )
		return qfalse;*/

	return qtrue;
}

/* */
int AIMOD_MAPPING_CreateNodeLinks ( int node )
{
	vec3_t	tmp;
	int		loop = 0;
	int		linknum = 0;
	//vec3_t	tankMaxsSize = {96, 96, 0};
	//vec3_t	tankMinsSize = {-96, -96, 0};

	VectorCopy( nodes[node].origin, tmp );
	tmp[2]+=8;

	for ( loop = 0; loop < number_of_nodes; loop++ )
	{
		if (loop == node)
			continue;

		if ( linknum >= MAX_NODELINKS )
		{
			break;
		}

		if ( !BAD_WP_Distance( nodes[node].origin, nodes[loop].origin) )
		//if (Distance(nodes[node].origin, nodes[loop].origin) < 512)
		{
			int visCheck = NodeVisible( nodes[loop].origin, tmp, -1 );

			//0 = wall in way
			//1 = player or no obstruction
			//2 = useable door in the way.
			//3 = door entity in the way.
			if ( visCheck == 1 || visCheck == 2 || visCheck == 3 /*|| loop == node - 1*/ )
			{
				//if (AIMod_Check_Slope_Between(nodes[node].origin, nodes[loop].origin))
				{
					nodes[node].links[linknum].targetNode = loop;
					nodes[node].links[linknum].cost = Distance(nodes[loop].origin, nodes[node].origin) + (HeightDistance(nodes[loop].origin, nodes[node].origin)*HeightDistance(nodes[loop].origin, nodes[node].origin));
					nodes[node].links[linknum].flags = 0;

					linknum++;
				}
			}
/*			else
			{// Look for jump node links...
				visCheck = NodeVisibleJump( nodes[loop].origin, tmp, -1 );

				//0 = wall in way
				//1 = player or no obstruction
				//2 = useable door in the way.
				//3 = door entity in the way.
				if ( visCheck == 1 || visCheck == 2 || visCheck == 3 )
				{
					if (AIMod_Check_Slope_Between(nodes[node].origin, nodes[loop].origin))
					{
						nodes[node].links[linknum].targetNode = loop;
						nodes[node].links[linknum].cost = Distance( nodes[loop].origin, nodes[node].origin ) + (HeightDistance( nodes[loop].origin, nodes[node].origin )*16);
						nodes[node].links[linknum].flags |= PATH_JUMP;

						linknum++;
					}
				}
				else
				{// Look for crouch node links...
					visCheck = NodeVisibleCrouch( nodes[loop].origin, tmp, -1 );

					//0 = wall in way
					//1 = player or no obstruction
					//2 = useable door in the way.
					//3 = door entity in the way.
					if ( visCheck == 1 || visCheck == 2 || visCheck == 3 )
					{
						if (AIMod_Check_Slope_Between(nodes[node].origin, nodes[loop].origin))
						{
							nodes[node].links[linknum].targetNode = loop;
							nodes[node].links[linknum].cost = Distance( nodes[loop].origin, nodes[node].origin ) + (HeightDistance( nodes[loop].origin, nodes[node].origin )*16);
							nodes[node].links[linknum].flags |= PATH_CROUCH;

							linknum++;
						}
					}
				}
			}*/
		}
	}

	/*
#ifdef __VEHICLES__	// way too generic of a name, just remove the code for now --eez
	// UQ1: If not too many links, add extra vehicle links...
	if (linknum < MAX_NODELINKS && (nodes[node].type & NODE_LAND_VEHICLE) )
	for ( loop = 0; loop < number_of_nodes; loop++ )
	{
		vec3_t	tmp;
		VectorCopy( nodes[node].origin, tmp );

		if ( linknum >= MAX_NODELINKS )
		{
			break;
		}

		if (!(nodes[loop].type & NODE_LAND_VEHICLE))
		{
			continue;
		}

		if ( BAD_WP_Height( nodes[node].origin, nodes[loop].origin) )
		{
			continue;
		}

		if ( Distance( nodes[loop].origin, nodes[node].origin) <= (waypoint_scatter_distance*waypoint_distance_multiplier)*4 )
		{
			int visCheck = TankNodeVisible( nodes[loop].origin, tmp, tankMinsSize, tankMaxsSize, -1 );

			//0 = wall in way
			//1 = player or no obstruction
			//2 = useable door in the way.
			//3 = door entity in the way.
			if ( visCheck == 1 )
			{
				if (AIMod_Check_Slope_Between(nodes[node].origin, nodes[loop].origin))
				{
					nodes[node].links[linknum].targetNode = loop;
					nodes[node].links[linknum].cost = Distance( nodes[loop].origin, nodes[node].origin );

					linknum++;
				}
			}
		}
	}

	if (linknum < MAX_NODELINKS && (nodes[node].type & NODE_LAND_VEHICLE) )
	for ( loop = 0; loop < number_of_nodes; loop++ )
	{
		vec3_t	tmp;
		VectorCopy( nodes[node].origin, tmp );

		if ( linknum >= MAX_NODELINKS )
		{
			break;
		}

		if (!(nodes[loop].type & NODE_LAND_VEHICLE))
		{
			continue;
		}

		if ( BAD_WP_Height( nodes[node].origin, nodes[loop].origin) )
		{
			continue;
		}

		if ( Distance( nodes[loop].origin, nodes[node].origin) <= (waypoint_scatter_distance*waypoint_distance_multiplier)*8 )
		{
			int visCheck = TankNodeVisible( nodes[loop].origin, tmp, tankMinsSize, tankMaxsSize, -1 );

			//0 = wall in way
			//1 = player or no obstruction
			//2 = useable door in the way.
			//3 = door entity in the way.
			if ( visCheck == 1 )
			{
				if (AIMod_Check_Slope_Between(nodes[node].origin, nodes[loop].origin))
				{
					nodes[node].links[linknum].targetNode = loop;
					nodes[node].links[linknum].cost = Distance( nodes[loop].origin, nodes[node].origin );

					linknum++;
				}
			}
		}
	}
#endif //__VEHICLES__
	*/

	nodes[node].enodenum = linknum;

	//if (nodes[node].enodenum > 0)
	//	trap->Print("Node %i has %i links. ", node, nodes[node].enodenum);

	return ( linknum );
}

/* */
qboolean AI_PM_SlickTrace ( vec3_t point, int clientNum )
{
	/*trace_t trace;
	vec3_t	point2;
	VectorCopy( point, point2 );
	point2[2] = point2[2] - 0.25f;

	CG_Trace( &trace, point, botTraceMins, botTraceMaxs, point2, clientNum, MASK_SHOT );

	if ( trace.surfaceFlags & SURF_SLICK )
	{
		return ( qtrue );
	}
	else
	{*/
		return ( qfalse );
	//}
}

/* */
void
AIMOD_MAPPING_CreateSpecialNodeFlags ( int node )
{	// Need to check for duck (etc) nodes and mark them...
	trace_t tr;
	vec3_t	up, temp, uporg;
	//vec3_t	tankMaxsSize = {96, 96, 0};
	//vec3_t	tankMinsSize = {-96, -96, 0};

	VectorCopy( nodes[node].origin, temp );
	temp[2] += 1;
	nodes[node].type &= ~NODE_DUCK;
	VectorCopy( nodes[node].origin, up );
	up[2] += 16550;
	CG_Trace( &tr, nodes[node].origin, NULL, NULL, up, -1, MASK_SHOT | MASK_OPAQUE | MASK_WATER /*MASK_ALL*/ );
	
	if ( Distance( nodes[node].origin, tr.endpos) <= 72 )
	{	// Could not see the up pos.. Need to duck to go here!
		nodes[node].type |= NODE_DUCK;
		//trap->Print( "^4*** ^3%s^5: Node ^7%i^5 marked as a duck node.\n", GAME_VERSION, node );
	}

	if ( AI_PM_SlickTrace( nodes[node].origin, -1) )
	{	// This node is on slippery ice... Mark it...
		nodes[node].type |= NODE_ICE;
		//trap->Print( "^4*** ^3%s^5: Node ^7%i^5 marked as an ice (slick) node.\n", GAME_VERSION, node );
	}

	VectorCopy(nodes[node].origin, uporg);
	uporg[2]+=104;

	//if ( TankNodeVisible( nodes[node].origin, uporg, tankMinsSize, tankMaxsSize, -1) == 1 )
	//{
	//	nodes[node].type |= NODE_LAND_VEHICLE;
	//}
}

//#define __BOT_AUTOWAYPOINT_OPTIMIZE__

/* */
void
AIMOD_MAPPING_MakeLinks ( void )
{
	int		loop = 0;
//	node_t	*good_nodes;
//	int		upto = 0;
//	int		total_good_count = 0;
//	int		bad_nodes = 0;
	int		final_tests = 0;
	int		update_timer = 0;

#ifdef __BOT_AUTOWAYPOINT_OPTIMIZE__
	good_nodes = malloc( (sizeof(node_t)+1)*MAX_NODES );
#endif //__BOT_AUTOWAYPOINT_OPTIMIZE__

	if ( aw_num_nodes > 0 )
	{
		number_of_nodes = aw_num_nodes;
	}

	aw_percent_complete = 0.0f;
 	strcpy( last_node_added_string, va("") );

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Creating waypoint linkages and flags...\n" );
	strcpy( task_string3, va("^5Creating waypoint linkages and flags...") );
	trap->UpdateScreen();

	final_tests = 0;
	update_timer = 0;
	aw_percent_complete = 0.0f;
	trap->UpdateScreen();
	
	for ( loop = 0; loop < number_of_nodes; loop++ )
	{// Do links...
		nodes[loop].enodenum = 0;

		// Draw a nice little progress bar ;)
		aw_percent_complete = (float)((float)((float)loop/(float)number_of_nodes)*100.0f);

		update_timer++;

		if (update_timer >= 100)
		{
			trap->UpdateScreen();
			update_timer = 0;
		}

		// Also check if the node needs special flags...
		AIMOD_MAPPING_CreateSpecialNodeFlags( loop );

		AIMOD_MAPPING_CreateNodeLinks( loop );

		strcpy( last_node_added_string, va("^5Created ^3%i ^5links for waypoint ^7%i^5.", nodes[loop].enodenum, loop) );
	}

	aw_percent_complete = 0.0f;
	strcpy( last_node_added_string, va("") );
	aw_num_nodes = number_of_nodes;
	trap->UpdateScreen();
}

/* */
void
AIMOD_NODES_SetObjectiveFlags ( int node )
{										// Find objects near this node.
	// Init them first...
	nodes[node].objectNum[0] = nodes[node].objectNum[1] = nodes[node].objectNum[2] = ENTITYNUM_NONE;
	nodes[node].objEntity = -1;
	nodes[node].objFlags = -1;
	nodes[node].objTeam = -1;
	nodes[node].objType = -1;
}

/*////////////////////////////////////////////////
ConnectNodes
Connects 2 nodes and sets the flags for the path between them
/*/


///////////////////////////////////////////////
qboolean
ConnectNodes ( int from, int to, int flags )
{

	//check that we don't have too many connections from the 'from' node already
	if ( nodes[from].enodenum + 1 > MAX_NODELINKS )
	{
		return ( qfalse );
	}

	//check that we are creating a path between 2 valid nodes
	if ( (nodes[from].type == NODE_INVALID) || (to == NODE_INVALID) || from > MAX_NODES || from < 0 || to > MAX_NODES || to < 0)
	{	//nodes[to].type is invalid on LoadNodes()
		return ( qfalse );
	}

	//update the individual nodes
	nodes[from].links[nodes[from].enodenum].targetNode = to;
	nodes[from].links[nodes[from].enodenum].flags = flags;
	nodes[from].enodenum++;
	return ( qtrue );
}

/*//////////////////////////////////////////////
AddNode
creates a new waypoint/node with the specified values
*/

/////////////////////////////////////////////
int numAxisOnlyNodes = 0;
int AxisOnlyFirstNode = -1;
int numAlliedOnlyNodes = 0;
int AlliedOnlyFirstNode = -1;


/* */
qboolean
Load_AddNode ( vec3_t origin, int fl, short int *ents, int objFl )
{
	if ( number_of_nodes + 1 > MAX_NODES )
	{
		return ( qfalse );
	}

	VectorCopy( origin, nodes[number_of_nodes].origin );	//set the node's position

	nodes[number_of_nodes].type = fl;						//set the flags (NODE_OBJECTIVE, for example)
	nodes[number_of_nodes].objectNum[0] = ents[0];			//set any map objects associated with this node
	nodes[number_of_nodes].objectNum[1] = ents[1];			//only applies to objects linked to the unreachable flag
	nodes[number_of_nodes].objectNum[2] = ents[2];
	nodes[number_of_nodes].objFlags = objFl;				//set the objective flags
	if ( nodes[number_of_nodes].type & NODE_AXIS_UNREACHABLE )
	{
		if ( AlliedOnlyFirstNode < 0 )
		{
			AlliedOnlyFirstNode = number_of_nodes;
		}
		nodes[number_of_nodes].objTeam |= TEAM_BLUE;
		numAlliedOnlyNodes++;
	}

	if ( nodes[number_of_nodes].type & NODE_ALLY_UNREACHABLE )
	{
		if ( AxisOnlyFirstNode < 0 )
		{
			AxisOnlyFirstNode = number_of_nodes;
		}
		nodes[number_of_nodes].objTeam |= TEAM_RED;
		numAxisOnlyNodes++;
	}

	number_of_nodes++;
	return ( qtrue );
}

void AIMOD_NODES_LoadNodes ( void )
{
	fileHandle_t	f;
	int				i, j;
	char			filename[60];
//	vmCvar_t		mapname;
	short int		objNum[3] = { 0, 0, 0 },
	objFlags, numLinks;
	int				flags;
	vec3_t			vec;
	short int		fl2;
	int				target;
	char			name[] = BOT_MOD_NAME;
	char			nm[64] = "";
	float			version;
	char			map[64] = "";
	char			mp[64] = "";
	/*short*/ int		numberNodes;
	short int		temp, fix_aas_nodes;

	i = 0;

	///////////////////
	//open the node file for reading, return false on error
	trap->FS_Open( va( "nodes/%s.bwp", cgs.rawmapname), &f, FS_READ );
	if ( !f )
	{
		Com_Printf("^1AWP ERROR: Failed to load nodes/%s.bwp\n", cgs.rawmapname);
		return;
	}

	strcpy( mp, cgs.rawmapname );
	trap->FS_Read( &nm, strlen( name) + 1, f );									//read in a string the size of the mod name (+1 is because all strings end in hex '00')
	trap->FS_Read( &version, sizeof(float), f );			//read and make sure the version is the same

	if ( version != NOD_VERSION && version != 1.0f )
	{
		trap->Print( "^3WARNING: Reading from nodes/%s.bwp failed:\n", filename );
		trap->Print( "^3         Old node file detected.\n" );
		trap->FS_Close( f );
		return;
	}

	trap->FS_Read( &map, strlen( mp) + 1, f );			//make sure the file is for the current map
	if ( Q_stricmp( map, mp) != 0 )
	{
		trap->Print( "^3WARNING: Reading from nodes/%s.bwp failed:\n", filename );
		trap->Print( "^3         Node file is not for this map!\n" );
		trap->FS_Close( f );
		return;
	}

	if (version == NOD_VERSION)
	{
		trap->FS_Read( &numberNodes, sizeof(/*short*/ int), f ); //read in the number of nodes in the map
	}
	else
	{
		trap->FS_Read( &temp, sizeof(short int), f ); //read in the number of nodes in the map
		numberNodes = temp;
	}

	for ( i = 0; i < numberNodes; i++ )					//loop through all the nodes
	{
		nodes[i].enodenum = 0;

		//read in all the node info stored in the file
		trap->FS_Read( &vec, sizeof(vec3_t), f );
		trap->FS_Read( &flags, sizeof(int), f );
		trap->FS_Read( objNum, sizeof(short int) * 3, f );
		trap->FS_Read( &objFlags, sizeof(short int), f );
		trap->FS_Read( &numLinks, sizeof(short int), f );

		Load_AddNode( vec, flags, objNum, objFlags );	//add the node

		//loop through all of the links and read the data
		for ( j = 0; j < numLinks; j++ )
		{
			if (version == NOD_VERSION)
			{
				trap->FS_Read( &target, sizeof(/*short*/ int), f );
			}
			else
			{
				trap->FS_Read( &temp, sizeof(short int), f );
				target = temp;
			}

			trap->FS_Read( &fl2, sizeof(short int), f );
			ConnectNodes( i, target, fl2 );				//add any links
		}

		// Set node objective flags..
		AIMOD_NODES_SetObjectiveFlags( i );
	}

	trap->FS_Read( &fix_aas_nodes, sizeof(short int), f );
	trap->FS_Close( f );							//close the file
	trap->Print( "AWP: Successfully loaded %i waypoints from waypoint file nodes/%s.bwp.\n",
			  number_of_nodes, filename );
	nodes_loaded = qtrue;

	return;
}

/* */
void AIMOD_NODES_SaveNodes_Autowaypointed ( void )
{
	fileHandle_t	f;
	int				i;
	/*short*/ int		j;
	float			version = NOD_VERSION;										//version is 1.0 for now
	char			name[] = BOT_MOD_NAME;
	//vmCvar_t		mapname;
	char			map[64] = "";
	char			filename[60];
	/*short*/ int		num_nodes = number_of_nodes;
	
	aw_num_nodes = number_of_nodes;
	strcpy( filename, "nodes/" );

	///////////////////
	//try to open the output file, return if it failed
	trap->FS_Open( va( "nodes/%s.bwp", cgs.rawmapname), &f, FS_WRITE );
	if ( !f )
	{
		trap->Print( "^1*** ^3ERROR^5: Error opening node file ^7nodes/%s.bwp^5!!!\n", cgs.rawmapname/*filename*/ );
		return;
	}

//#pragma omp parallel for
	for ( i = 0; i < aw_num_nodes/*num_nodes*/; i++ )
	{
		nodes[i].enodenum = 0;

//#pragma omp parallel for
		for ( j = 0; j < MAX_NODELINKS; j++ )
		{
			nodes[i].links[j].targetNode = INVALID;
			nodes[i].links[j].cost = 999999;
			nodes[i].links[j].flags = 0;
			nodes[i].objectNum[0] = nodes[i].objectNum[1] = nodes[i].objectNum[2] = ENTITYNUM_NONE;
		}
	}

//	RemoveDoorsAndDestroyablesForSave();
//	num_nodes = number_of_nodes;

	// Resolve paths
	//-------------------------------------------------------
	AIMOD_MAPPING_MakeLinks();

	num_nodes = aw_num_nodes;

//#pragma omp parallel for
	for ( i = 0; i < aw_num_nodes/*num_nodes*/; i++ )
	{
		// Set node objective flags..
		AIMOD_NODES_SetObjectiveFlags( i );
	}
	
	//trap->Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );	//get the map name
	strcpy( map, cgs.rawmapname );
	trap->FS_Write( &name, strlen( name) + 1, f );								//write the mod name to the file
	trap->FS_Write( &version, sizeof(float), f );								//write the version of this file
	trap->FS_Write( &map, strlen( map) + 1, f );									//write the map name
	trap->FS_Write( &num_nodes, sizeof(/*short*/ int), f );							//write the number of nodes in the map
	
	for ( i = 0; i < aw_num_nodes/*num_nodes*/; i++ )											//loop through all the nodes
	{
		//write all the node data to the file
		trap->FS_Write( &(nodes[i].origin), sizeof(vec3_t), f );
		trap->FS_Write( &(nodes[i].type), sizeof(int), f );
		trap->FS_Write( &(nodes[i].objectNum), sizeof(short int) * 3, f );
		trap->FS_Write( &(nodes[i].objFlags), sizeof(short int), f );
		trap->FS_Write( &(nodes[i].enodenum), sizeof(short int), f );
		for ( j = 0; j < nodes[i].enodenum; j++ )
		{
			trap->FS_Write( &(nodes[i].links[j].targetNode), sizeof(/*short*/ int), f );
			trap->FS_Write( &(nodes[i].links[j].flags), sizeof(short int), f );
		}
	}
	{
		//short int	fix = 1;
		short int	fix = 0;
		trap->FS_Write( &fix, sizeof(short int), f );
	}

	trap->FS_Close( f );													//close the file
	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Successfully saved node file ^7nodes/%s.bwp^5.\n", cgs.rawmapname/*filename*/ );
}

//
// The utilities for faster (then vector/float) integer maths...
//
typedef long int	intvec_t;
typedef intvec_t	intvec3_t[3];


/* */
void
intToVectorCopy ( const intvec3_t in, vec3_t out )
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}


/* */
void
intVectorCopy ( const intvec3_t in, intvec3_t out )
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}


/* */
intvec_t
intVectorLength ( const intvec3_t v )
{
	return ( sqrt( (double)(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])) );
}


/* */
void
intVectorSubtract ( const intvec3_t veca, const intvec3_t vecb, intvec3_t out )
{
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}


/* */
long int
intVectorDistance ( intvec3_t v1, intvec3_t v2 )
{
	intvec3_t	dir;
	intVectorSubtract( v2, v1, dir );
	return ( intVectorLength( dir) );
}

//
// Now the actual number crunching and visualizations...

extern float BG_GetGroundHeightAtPoint( vec3_t pos );
extern qboolean BG_TraceMapLoaded ( void );
extern void CG_GenerateTracemap( void );

qboolean Waypoint_FloorSurfaceOK ( int surfaceFlags )
{
	if (surfaceFlags == 0)
		return qtrue;

/*
#define	SURF_SKY				0x00002000	// lighting from environment map
#define	SURF_SLICK				0x00004000	// affects game physics
#define	SURF_METALSTEPS			0x00008000	// CHC needs this since we use same tools (though this flag is temp?)
#define SURF_FORCEFIELD			0x00010000	// CHC ""			(but not temp)
#define	SURF_NODAMAGE			0x00040000	// never give falling damage
#define	SURF_NOIMPACT			0x00080000	// don't make missile explosions
#define	SURF_NOMARKS			0x00100000	// don't leave missile marks
#define	SURF_NODRAW				0x00200000	// don't generate a drawsurface at all
#define	SURF_NOSTEPS			0x00400000	// no footstep sounds
#define	SURF_NODLIGHT			0x00800000	// don't dlight even if solid (solid lava, skies)
#define	SURF_NOMISCENTS			0x01000000	// no client models allowed on this surface
*/
	if (surfaceFlags & SURF_SLICK)
		return qtrue;

	if (surfaceFlags & SURF_METALSTEPS)
		return qtrue;

	if (surfaceFlags & SURF_FORCEFIELD)
		return qtrue;

	if (surfaceFlags & SURF_NOSTEPS)
		return qtrue;

	//if (surfaceFlags & SURF_SKY)
	//	return qfalse;

	return qfalse;	
}

float GroundHeightAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2;
//	float height = 0;

	VectorCopy(org, org1);
	org1[2]+=48;

	VectorCopy(org, org2);
	org2[2]= -65536.0f;

	CG_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID);//CONTENTS_PLAYERCLIP | MASK_SHOT /*| MASK_OPAQUE*/ | MASK_WATER );
	
	if ( tr.startsolid || tr.allsolid )
	{
		return -65536.0f;
	}

	if ( tr.surfaceFlags & SURF_SKY )
	{// Sky...
		return -65536.0f;
	}

	if ( tr.contents & CONTENTS_TRIGGER )
	{// Trigger hurt???
		return -65536.0f;
	}

	if ( tr.contents & CONTENTS_WATER )
	{// Water. Bad m'kay...
		return -65536.0f;
	}

//	if ( (tr.surfaceFlags & SURF_NODRAW) 
//		&& (tr.surfaceFlags & SURF_NOMARKS) 
//		/*&& !Waypoint_FloorSurfaceOK(tr.surfaceFlags) 
//		&& !HasPortalFlags(tr.surfaceFlags, tr.contents)*/ )
//	{// Sky...
//		return -65536.0f;
//	}

	if (tr.endpos[2] < -65000)
		return -65536.0f;

	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (cg_entities[tr.entityNum].currentState.eType == ET_MOVER 
			|| cg_entities[tr.entityNum].currentState.eType == ET_PUSH_TRIGGER
			|| cg_entities[tr.entityNum].currentState.eType == ET_PORTAL
			|| cg_entities[tr.entityNum].currentState.eType == ET_TELEPORT_TRIGGER
			|| cg_entities[tr.entityNum].currentState.eType == ET_TEAM
			|| cg_entities[tr.entityNum].currentState.eType == ET_TERRAIN
			|| cg_entities[tr.entityNum].currentState.eType == ET_FX
			|| HasPortalFlags(tr.surfaceFlags, tr.contents))
		{// Hit a mover... Add waypoints at all of them!
			return tr.endpos[2];
		}
	}

//	if ( (tr.surfaceFlags & SURF_NODRAW) && (tr.surfaceFlags & SURF_NOMARKS) 
//		/*&& !Waypoint_FloorSurfaceOK(tr.surfaceFlags) 
//		&& !HasPortalFlags(tr.surfaceFlags, tr.contents)*/)
//	{// Sky...
//		//trap->Print("(tr.surfaceFlags & SURF_NODRAW) && (tr.surfaceFlags & SURF_NOMARKS)\n");
//		return 65536.0f;
//	}

	return tr.endpos[2];
}

qboolean BadHeightNearby( vec3_t org )
{
	vec3_t org1, org2, angles, forward, right;

	VectorSet(angles, 0, 0, 0);
	VectorCopy(org, org1);
	org1[2] += 18;

	AngleVectors( angles, forward, right, NULL );
	
	// Check forward...
	VectorMA( org1, 192, forward, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check back...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, -192, forward, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check right...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, 192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check left...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, -192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check forward right...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, 192, forward, org2 );
	VectorMA( org2, 192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check forward left...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, 192, forward, org2 );
	VectorMA( org2, -192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check back right...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, -192, forward, org2 );
	VectorMA( org2, 192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	// Check back left...
	VectorCopy(org, org1);
	org1[2] += 18;
	VectorMA( org1, -192, forward, org2 );
	VectorMA( org2, -192, right, org2 );

	if (OrgVisible(org1, org2, -1))
		if (GroundHeightAt( org2 ) < org[2]-100)
			return qtrue;

	return qfalse;
}

qboolean aw_floor_trace_hit_mover = qfalse;

float FloorHeightAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2, slopeangles;
	float pitch = 0;

	aw_floor_trace_hit_mover = qfalse;

	if (AIMOD_IsWaypointHeightMarkedAsBad( org ))
	{
		return 65536.0f;
	}

	VectorCopy(org, org1);
	org1[2]+=48;

	VectorCopy(org, org2);
	org2[2]= -65536.0f;

	CG_Trace( &tr, org1, NULL, NULL, org2, -1, MASK_PLAYERSOLID );//CONTENTS_PLAYERCLIP | MASK_SHOT /*| MASK_OPAQUE*/ | MASK_WATER );
	
	if (tr.endpos[2] < -65000)
		return -65536.0f;

	if ( tr.surfaceFlags & SURF_SKY )
	{// Sky...
		//trap->Print("SURF_SKY\n");
		return 65536.0f;
	}

//	if ( (tr.surfaceFlags & SURF_NODRAW) && (tr.surfaceFlags & SURF_NOMARKS) 
//		/*&& !Waypoint_FloorSurfaceOK(tr.surfaceFlags) 
//		&& !HasPortalFlags(tr.surfaceFlags, tr.contents)*/)
//	{// Sky...
//		//trap->Print("(tr.surfaceFlags & SURF_NODRAW) && (tr.surfaceFlags & SURF_NOMARKS)\n");
//		return 65536.0f;
//	}

/*

#define	CONTENTS_SOLID			0x00000001	// Default setting. An eye is never valid in a solid
#define	CONTENTS_LAVA			0x00000002
#define	CONTENTS_WATER			0x00000004
#define	CONTENTS_FOG			0x00000008
#define	CONTENTS_PLAYERCLIP		0x00000010
#define	CONTENTS_MONSTERCLIP	0x00000020	// Physically block bots
#define CONTENTS_BOTCLIP		0x00000040	// A hint for bots - do not enter this brush by navigation (if possible)
#define CONTENTS_SHOTCLIP		0x00000080
#define	CONTENTS_BODY			0x00000100	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x00000200	// should never be on a brush, only in game
#define	CONTENTS_TRIGGER		0x00000400
#define	CONTENTS_NODROP			0x00000800	// don't leave bodies or items (death fog, lava)
#define CONTENTS_TERRAIN		0x00001000	// volume contains terrain data
#define CONTENTS_LADDER			0x00002000
#define CONTENTS_ABSEIL			0x00004000  // (SOF2) used like ladder to define where an NPC can abseil
#define CONTENTS_OPAQUE			0x00008000	// defaults to on, when off, solid can be seen through
#define CONTENTS_OUTSIDE		0x00010000	// volume is considered to be in the outside (i.e. not indoors)

#define	CONTENTS_INSIDE			0x10000000	// volume is considered to be inside (i.e. indoors)

#define CONTENTS_SLIME			0x00020000	// CHC needs this since we use same tools
#define CONTENTS_LIGHTSABER		0x00040000	// ""
#define CONTENTS_TELEPORTER		0x00080000	// ""
#define CONTENTS_ITEM			0x00100000	// ""
#define CONTENTS_NOSHOT			0x00200000	// shots pass through me
#define	CONTENTS_DETAIL			0x08000000	// brushes not used for the bsp
#define	CONTENTS_TRANSLUCENT	0x80000000	// don't consume surface fragments inside

#define	SURF_SKY				0x00002000	// lighting from environment map
#define	SURF_SLICK				0x00004000	// affects game physics
#define	SURF_METALSTEPS			0x00008000	// CHC needs this since we use same tools (though this flag is temp?)
#define SURF_FORCEFIELD			0x00010000	// CHC ""			(but not temp)
#define	SURF_NODAMAGE			0x00040000	// never give falling damage
#define	SURF_NOIMPACT			0x00080000	// don't make missile explosions
#define	SURF_NOMARKS			0x00100000	// don't leave missile marks
#define	SURF_NODRAW				0x00200000	// don't generate a drawsurface at all
#define	SURF_NOSTEPS			0x00400000	// no footstep sounds
#define	SURF_NODLIGHT			0x00800000	// don't dlight even if solid (solid lava, skies)
#define	SURF_NOMISCENTS			0x01000000	// no client models allowed on this surface


			*surfaceparm     nodraw
            *surfaceparm     nonsolid
            *surfaceparm     nonopaque
            *surfaceparm     trans
            *surfaceparm     abseil
            *surfaceparm     ladder
    // Useless commands, AWP uses this to check intention of
    // the brush as Notrace bounds.
            *surfaceparm     monsterclip
            *surfaceparm     playerclip
            *surfaceparm     botclip
            *surfaceparm     nodamage
            *surfaceparm     noimpact
            *surfaceparm     nomarks
            *surfaceparm     nosteps
            *surfaceparm     nodlight
            *surfaceparm     nomiscents
            surfaceparm     nodrop
*/
	if ( tr.surfaceFlags & SURF_NODRAW
		&& tr.surfaceFlags & CONTENTS_ABSEIL
		&& tr.surfaceFlags & SURF_NODAMAGE
		&& tr.surfaceFlags & SURF_NOIMPACT
		&& tr.surfaceFlags & SURF_NOMARKS
		&& tr.surfaceFlags & SURF_NOSTEPS
		&& tr.surfaceFlags & SURF_NODLIGHT
		&& tr.surfaceFlags & SURF_NOMISCENTS
		//&& !(tr.contents & CONTENTS_SOLID)
		&& tr.contents & CONTENTS_OPAQUE
		&& tr.contents & CONTENTS_TRANSLUCENT
		&& tr.contents & CONTENTS_LADDER
		&& tr.contents & CONTENTS_PLAYERCLIP
		&& tr.contents & CONTENTS_MONSTERCLIP
		&& tr.contents & CONTENTS_BOTCLIP
		&& tr.contents & CONTENTS_NODROP)
	{// Special flags - Mapped out Z axis here...
		//trap->Print("Ignore Area Surface\n");
		return -65536.0f;
	}
	
	if ( tr.surfaceFlags & SURF_NOMISCENTS )
	{// Sky...
		//trap->Print("SURF_NOMISCENTS\n");
		return 65536.0f;
	}

	if ( tr.contents & CONTENTS_LAVA )
	{// Sky...
		//trap->Print("CONTENTS_LAVA\n");
		return 65536.0f;
	}

	if ( tr.contents & CONTENTS_WATER )
	{// Water... I'm just gonna ignore these!
		//trap->Print("CONTENTS_LAVA\n");
		return 65536.0f;
	}

	/*
	if ( tr.contents == 0 && tr.surfaceFlags == 0 )
	{// Dont know what to do about these... Gonna try disabling them and see what happens...
		//trap->Print("Hit noflag zone.\n");
		return 65536.0f;
	}
	*/

	/*if (tr.surfaceFlags & SURF_NODRAW)
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS))\n");
		return 65536.0f;
	}*/

	if ( ((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS) && !(tr.contents & CONTENTS_PLAYERCLIP)) )
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS))\n");
		return 65536.0f;
	}

	if ( ((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS) && (tr.contents & CONTENTS_DETAIL)) )
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS))\n");
		if (AIMOD_IsWaypointHeightMarkedAsBad( tr.endpos ))
			return 65536.0f;
	}

	/*if ( ((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NOIMPACT)) )
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS))\n");
		return 65536.0f;
	}*/

	if ((tr.surfaceFlags & SURF_NOMARKS) && (tr.surfaceFlags & SURF_NODRAW) && (tr.contents & CONTENTS_SOLID) && (tr.contents & CONTENTS_OPAQUE))
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_TRANSLUCENT) && (tr.surfaceFlags & SURF_NOMARKS))\n");
		if (AIMOD_IsWaypointHeightMarkedAsBad( tr.endpos ))
			return 65536.0f;
	}

	/*if ( !((tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_SOLID)) )
	{// Sky...
		//trap->Print("((tr.contents & CONTENTS_OPAQUE) && (tr.contents & CONTENTS_SOLID))\n");
		return 65536.0f;
	}*/

	if ( tr.fraction != 1 
		&& tr.entityNum != ENTITYNUM_NONE 
		&& tr.entityNum < ENTITYNUM_MAX_NORMAL )
	{
		if (cg_entities[tr.entityNum].currentState.eType == ET_MOVER 
			|| cg_entities[tr.entityNum].currentState.eType == ET_PUSH_TRIGGER
			|| cg_entities[tr.entityNum].currentState.eType == ET_PORTAL
			|| cg_entities[tr.entityNum].currentState.eType == ET_TELEPORT_TRIGGER
			|| cg_entities[tr.entityNum].currentState.eType == ET_TEAM
			|| cg_entities[tr.entityNum].currentState.eType == ET_TERRAIN
			|| cg_entities[tr.entityNum].currentState.eType == ET_FX
			|| HasPortalFlags(tr.surfaceFlags, tr.contents))
		{// Hit a mover... Add waypoints at all of them!
			aw_floor_trace_hit_mover = qtrue;
			return tr.endpos[2];
		}
	}

	aw_floor_trace_hit_mover = qfalse;

	// Added -- Check slope...
	vectoangles( tr.plane.normal, slopeangles );

	pitch = slopeangles[0];
	
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	if (pitch > 46.0f || pitch < -46.0f)
		return 65536.0f; // bad slope...

	if ( tr.startsolid || tr.allsolid /*|| tr.contents & CONTENTS_WATER*/ )
	{
		return 65536.0f;
	}

	if (DO_THOROUGH && BadHeightNearby( org ))
	{
		return 65536.0f;
	}

	return tr.endpos[2];
}

float RoofHeightAt ( vec3_t org )
{
	trace_t tr;
	vec3_t org1, org2;
//	float height = 0;

	VectorCopy(org, org1);
	org1[2]+=4;

	VectorCopy(org, org2);
	org2[2]= 65536.0f;

	CG_Trace( &tr, org1, NULL, NULL, org2, cg.clientNum, MASK_PLAYERSOLID);//CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER );
	
	if ( tr.startsolid || tr.allsolid )
	{
		return -65536.0f;
	}

	return tr.endpos[2];
}

#define POS_FW 0
#define POS_BC 1
#define POS_L 2
#define POS_R 3
#define POS_MAX 4

/*
	 _____
	|  0  |
    |     |
    |2   3|
	|     |
	|__1__|
*/

void CG_ShowSlope ( void ) {  
	vec3_t	org;
	trace_t	 trace;
	vec3_t	forward, right, up, start, end;
	vec3_t	testangles;
	vec3_t	boxMins= {-1, -1, -1};
	vec3_t	boxMaxs= {1, 1, 1};
	float	pitch, roll, yaw, roof;
	vec3_t	slopeangles;

	VectorCopy(cg_entities[cg.clientNum].lerpOrigin, org);

	testangles[0] = testangles[1] = testangles[2] = 0;

	AngleVectors( testangles, forward, right, up );

	roof = RoofHeightAt( org );
	roof -= 16;

	VectorCopy(org, start);
	VectorMA ( start, -65000 , up , end);
	start[2] = roof;

	CG_Trace( &trace, start, NULL, NULL, end, cg.clientNum, MASK_PLAYERSOLID );

	vectoangles( trace.plane.normal, slopeangles );

	pitch = slopeangles[0];
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	yaw = slopeangles[1];
	if (yaw > 180)
		yaw -= 360;

	if (yaw < -180)
		yaw += 360;

	roll = slopeangles[2];
	if (roll > 180)
		roll -= 360;

	if (roll < -180)
		roll += 360;

	trap->Print("Slope is %f %f %f\n", pitch, yaw, roll);
}

void CG_ShowSurface ( void )
{
	vec3_t org, down_org;
	trace_t tr;

	VectorCopy(cg_entities[cg.clientNum].lerpOrigin, org);
	VectorCopy(cg_entities[cg.clientNum].lerpOrigin, down_org);
	//down_org[2]-=48;
	down_org[2] = -65000;

	// Do forward test...
	CG_Trace( &tr, org, NULL, NULL, down_org, -1, MASK_PLAYERSOLID/*MASK_ALL*/ );

	//
	// Surface
	//

	trap->Print("Current surface flags (%i):\n", tr.surfaceFlags);

/*
#define	SURF_SKY				0x00002000	// lighting from environment map
#define	SURF_SLICK				0x00004000	// affects game physics
#define	SURF_METALSTEPS			0x00008000	// CHC needs this since we use same tools (though this flag is temp?)
#define SURF_FORCEFIELD			0x00010000	// CHC ""			(but not temp)
#define	SURF_NODAMAGE			0x00040000	// never give falling damage
#define	SURF_NOIMPACT			0x00080000	// don't make missile explosions
#define	SURF_NOMARKS			0x00100000	// don't leave missile marks
#define	SURF_NODRAW				0x00200000	// don't generate a drawsurface at all
#define	SURF_NOSTEPS			0x00400000	// no footstep sounds
#define	SURF_NODLIGHT			0x00800000	// don't dlight even if solid (solid lava, skies)
#define	SURF_NOMISCENTS			0x01000000	// no client models allowed on this surface
*/

	if (tr.surfaceFlags & SURF_NODAMAGE)
		trap->Print("SURF_NODAMAGE ");

	if (tr.surfaceFlags & SURF_SLICK)
		trap->Print("SURF_SLICK ");

	if (tr.surfaceFlags & SURF_SKY)
		trap->Print("SURF_SKY ");

	if (tr.surfaceFlags & SURF_METALSTEPS)
		trap->Print("SURF_METALSTEPS ");

	if (tr.surfaceFlags & SURF_FORCEFIELD)
		trap->Print("SURF_FORCEFIELD ");

	if (tr.surfaceFlags & SURF_NOMARKS)
		trap->Print("SURF_NOMARKS ");

	if (tr.surfaceFlags & SURF_NOIMPACT)
		trap->Print("SURF_NOIMPACT ");

	if (tr.surfaceFlags & SURF_NODRAW)
		trap->Print("SURF_NODRAW ");

	if (tr.surfaceFlags & SURF_NOSTEPS)
		trap->Print("SURF_NOSTEPS ");

	if (tr.surfaceFlags & SURF_NODLIGHT)
		trap->Print("SURF_NODLIGHT ");

	if (tr.surfaceFlags & SURF_NOSTEPS)
		trap->Print("SURF_NOSTEPS ");

	if (tr.surfaceFlags & SURF_NOMISCENTS)
		trap->Print("SURF_NOMISCENTS ");

	trap->Print("\n");

	//
	// Contents...
	//

	trap->Print("Current contents flags (%i):\n", tr.surfaceFlags);

	if (tr.contents & CONTENTS_SOLID)
		trap->Print("CONTENTS_SOLID ");

	if (tr.contents & CONTENTS_LAVA)
		trap->Print("CONTENTS_LAVA ");

	if (tr.contents & CONTENTS_WATER)
		trap->Print("CONTENTS_WATER ");

	if (tr.contents & CONTENTS_FOG)
		trap->Print("CONTENTS_FOG ");

	if (tr.contents & CONTENTS_PLAYERCLIP)
		trap->Print("CONTENTS_PLAYERCLIP ");

	if (tr.contents & CONTENTS_BOTCLIP)
		trap->Print("CONTENTS_BOTCLIP ");

	if (tr.contents & CONTENTS_SHOTCLIP)
		trap->Print("CONTENTS_SHOTCLIP ");

	if (tr.contents & CONTENTS_BODY)
		trap->Print("CONTENTS_BODY ");

	if (tr.contents & CONTENTS_CORPSE)
		trap->Print("CONTENTS_CORPSE ");

	if (tr.contents & CONTENTS_TRIGGER)
		trap->Print("CONTENTS_TRIGGER ");

	if (tr.contents & CONTENTS_NODROP)
		trap->Print("CONTENTS_NODROP ");

	if (tr.contents & CONTENTS_TERRAIN)
		trap->Print("CONTENTS_TERRAIN ");

	if (tr.contents & CONTENTS_LADDER)
		trap->Print("CONTENTS_LADDER ");

	if (tr.contents & CONTENTS_ABSEIL)
		trap->Print("CONTENTS_ABSEIL ");

	if (tr.contents & CONTENTS_OPAQUE)
		trap->Print("CONTENTS_OPAQUE ");

	if (tr.contents & CONTENTS_OUTSIDE)
		trap->Print("CONTENTS_OUTSIDE ");

	if (tr.contents & CONTENTS_INSIDE)
		trap->Print("CONTENTS_INSIDE ");

	if (tr.contents & CONTENTS_SLIME)
		trap->Print("CONTENTS_SLIME ");

	if (tr.contents & CONTENTS_LIGHTSABER)
		trap->Print("CONTENTS_LIGHTSABER ");

	if (tr.contents & CONTENTS_TELEPORTER)
		trap->Print("CONTENTS_TELEPORTER ");

	if (tr.contents & CONTENTS_ITEM)
		trap->Print("CONTENTS_ITEM ");

	if (tr.contents & CONTENTS_NOSHOT)
		trap->Print("CONTENTS_NOSHOT ");

	if (tr.contents & CONTENTS_DETAIL)
		trap->Print("CONTENTS_DETAIL ");

	if (tr.contents & CONTENTS_TRANSLUCENT)
		trap->Print("CONTENTS_TRANSLUCENT ");

	trap->Print("\n");

	// UQ1: May as well show the slope as well...
	CG_ShowSlope();
}

qboolean AIMod_AutoWaypoint_Check_Stepps ( vec3_t org )
{// return qtrue if NOT stepps...
	return qtrue;
/*
	trace_t		trace;
	vec3_t		testangles, forward, right, up, start, end;
	float		roof = 0.0f, last_height = 0.0f, last_diff = 0.0f, this_diff = 0.0f;
	int			i = 0, num_same_diffs = 0, num_zero_diffs = 0;

	testangles[0] = testangles[1] = testangles[2] = 0;
	AngleVectors( testangles, forward, right, up );

	roof = RoofHeightAt( org );
	roof -= 16;

	last_height = FloorHeightAt( org );

	for (i = 0; i < 64; i+=4)
	{
		VectorCopy(org, start);
		VectorMA ( start, i , forward , start);
		VectorMA ( start, -64000 , up , end);
		start[2] = roof;//RoofHeightAt( start );
		//start[2] -= 16;

		trap->CM_BoxTrace( &trace, start, end, NULL, NULL,  0, MASK_PLAYERSOLID );

		if (num_zero_diffs > 4 && num_same_diffs <= 0)
		{// Flat in this direction. Skip!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (trace.endpos[2] == last_height)
		{
			num_zero_diffs++;
			continue;
		}

		if (last_height > trace.endpos[2])
			this_diff = last_height - trace.endpos[2];
		else
			this_diff = trace.endpos[2] - last_height;

		if (this_diff > 24)
		{
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (last_diff == this_diff)
			num_same_diffs++;

		last_diff = this_diff;
		last_height = trace.endpos[2];

		if (num_same_diffs > 1 && num_zero_diffs <= 0)
		{// Save thinking.. It's obviously a slope!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			return qtrue;
		}

		if (num_same_diffs > 1 && num_zero_diffs > 1)
			break; // We seem to have found stepps...
	}

	if (num_same_diffs > 1 && num_zero_diffs > 1)
		return qfalse; // We seem to have found stepps...

	num_same_diffs = 0;
	num_zero_diffs = 0;
	last_height = FloorHeightAt( org );
	last_diff = 0;
	this_diff = 0;

	for (i = 0; i < 64; i+=4)
	{
		VectorCopy(org, start);
		VectorMA ( start, i , right , start);
		VectorMA ( start, -64000 , up , end);
		start[2] = roof;//RoofHeightAt( start );
		//start[2] -= 16;

		trap->CM_BoxTrace( &trace, start, end, NULL, NULL,  0, MASK_PLAYERSOLID );

		if (num_zero_diffs > 4 && num_same_diffs <= 0)
		{// Flat in this direction. Skip!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (trace.endpos[2] == last_height)
		{
			num_zero_diffs++;
			continue;
		}

		if (last_height > trace.endpos[2])
			this_diff = last_height - trace.endpos[2];
		else
			this_diff = trace.endpos[2] - last_height;

		if (this_diff > 24)
		{
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (last_diff == this_diff)
			num_same_diffs++;

		last_diff = this_diff;
		last_height = trace.endpos[2];

		if (num_same_diffs > 1 && num_zero_diffs <= 0)
		{// Save thinking.. It's obviously a slope!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			return qtrue;
		}

		if (num_same_diffs > 1 && num_zero_diffs > 1)
			break; // We seem to have found stepps...
	}

	if (num_same_diffs > 1 && num_zero_diffs > 1)
		return qfalse; // We seem to have found stepps...

	num_same_diffs = 0;
	num_zero_diffs = 0;
	last_height = FloorHeightAt( org );
	last_diff = 0;
	this_diff = 0;

	for (i = 0; i < 64; i+=4)
	{
		VectorCopy(org, start);
		VectorMA ( start, 0-i , forward , start);
		VectorMA ( start, -64000 , up , end);
		start[2] = roof;//RoofHeightAt( start );
		//start[2] -= 16;

		trap->CM_BoxTrace( &trace, start, end, NULL, NULL,  0, MASK_PLAYERSOLID );

		if (num_zero_diffs > 4 && num_same_diffs <= 0)
		{// Flat in this direction. Skip!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (trace.endpos[2] == last_height)
		{
			num_zero_diffs++;
			continue;
		}

		if (last_height > trace.endpos[2])
			this_diff = last_height - trace.endpos[2];
		else
			this_diff = trace.endpos[2] - last_height;

		if (this_diff > 24)
		{
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (last_diff == this_diff)
			num_same_diffs++;

		last_diff = this_diff;
		last_height = trace.endpos[2];

		if (num_same_diffs > 1 && num_zero_diffs <= 0)
		{// Save thinking.. It's obviously a slope!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			return qtrue;
		}

		if (num_same_diffs > 1 && num_zero_diffs > 1)
			break; // We seem to have found stepps...
	}

	if (num_same_diffs > 1 && num_zero_diffs > 1)
		return qfalse; // We seem to have found stepps...

	num_same_diffs = 0;
	num_zero_diffs = 0;
	last_height = FloorHeightAt( org );
	last_diff = 0;
	this_diff = 0;

	for (i = 0; i < 64; i+=4)
	{
		VectorCopy(org, start);
		VectorMA ( start, 0-i , right , start);
		VectorMA ( start, -64000 , up , end);
		start[2] = roof;//RoofHeightAt( start );
		//start[2] -= 16;

		trap->CM_BoxTrace( &trace, start, end, NULL, NULL,  0, MASK_PLAYERSOLID );

		if (num_zero_diffs > 4 && num_same_diffs <= 0)
		{// Flat in this direction. Skip!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (trace.endpos[2] == last_height)
		{
			num_zero_diffs++;
			continue;
		}

		if (last_height > trace.endpos[2])
			this_diff = last_height - trace.endpos[2];
		else
			this_diff = trace.endpos[2] - last_height;

		if (this_diff > 24)
		{
			num_same_diffs = 0;
			num_zero_diffs = 0;
			break;
		}

		if (last_diff == this_diff)
			num_same_diffs++;

		last_diff = this_diff;
		last_height = trace.endpos[2];

		if (num_same_diffs > 1 && num_zero_diffs <= 0)
		{// Save thinking.. It's obviously a slope!
			num_same_diffs = 0;
			num_zero_diffs = 0;
			return qtrue;
		}

		if (num_same_diffs > 1 && num_zero_diffs > 1)
			break; // We seem to have found stepps...
	}

	if (num_same_diffs > 1 && num_zero_diffs > 1)
		return qfalse; // We seem to have found stepps...

	return qtrue;
*/
}

qboolean AIMod_AutoWaypoint_Check_Slope ( vec3_t org ) { 
	// UQ1: Now down in the floor check code, saves a trace per spot check...
/*	trace_t	 trace;
	vec3_t	forward, right, up, start, end;
	vec3_t	testangles;
	vec3_t	boxMins= {-1, -1, -1};
	vec3_t	boxMaxs= {1, 1, 1};
	float	pitch, roof;
	vec3_t	slopeangles;

	testangles[0] = testangles[1] = testangles[2] = 0;
	AngleVectors( testangles, forward, right, up );
	roof = org[2] + 127;

	VectorCopy(org, start);
	VectorMA ( start, -65000 , up , end);
	start[2] = roof;

	CG_Trace( &trace, start, NULL, NULL, end, cg.clientNum, MASK_PLAYERSOLID );

	vectoangles( trace.plane.normal, slopeangles );

	pitch = slopeangles[0];
	
	if (pitch > 180)
		pitch -= 360;

	if (pitch < -180)
		pitch += 360;

	pitch += 90.0f;

	if (pitch > 46.0f || pitch < -46.0f)
		return qtrue;
*/
	return qfalse;
}

vec3_t fixed_position;

void RepairPosition ( intvec3_t org1 )
{
#ifdef __AW_UNUSED__
//	trace_t tr;
	vec3_t	/*newOrg, newOrg2,*/ forward, right, up;
	vec3_t	angles = { 0, 0, 0 };

	AngleVectors(angles, forward, right, up);

	// Init fixed_position
	fixed_position[0] = org1[0];
	fixed_position[1] = org1[1];
	fixed_position[2] = org1[2];

	// Prepare for forward test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, 64, forward, newOrg2);

	// Do forward test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, MASK_PLAYERSOLID/*MASK_SOLID|MASK_WATER*//*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER*/ );

	if ( tr.fraction != 1 )
	{// Repair the position...
		float move_ammount = Distance(tr.endpos, newOrg2);

		VectorMA(newOrg, 0-(move_ammount+1), forward, newOrg);
	}
	
	VectorCopy(newOrg, fixed_position);

	// Prepare for back test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, -64, forward, newOrg2);

	// Do back test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, MASK_PLAYERSOLID/*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER*/ );

	if ( tr.fraction != 1 )
	{// Repair the position...
		float move_ammount = Distance(tr.endpos, newOrg2);

		VectorMA(newOrg, move_ammount+1, forward, newOrg);
	}
	
	VectorCopy(newOrg, fixed_position);

	// Prepare for right test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, 64, right, newOrg2);

	// Do right test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, MASK_PLAYERSOLID/*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER*/ );

	if ( tr.fraction != 1 )
	{// Repair the position...
		float move_ammount = Distance(tr.endpos, newOrg2);

		VectorMA(newOrg, 0-(move_ammount+1), right, newOrg);
	}
	
	VectorCopy(newOrg, fixed_position);

	// Prepare for left test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, -64, right, newOrg2);

	// Do left test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, MASK_PLAYERSOLID/*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER*/ );

	if ( tr.fraction != 1 )
	{// Repair the position...
		float move_ammount = Distance(tr.endpos, newOrg2);

		VectorMA(newOrg, move_ammount+1, right, newOrg);
	}
	
	VectorCopy(newOrg, fixed_position);

	// Prepare for solid test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, 16, up, newOrg2);

	// Do start-solid test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, MASK_PLAYERSOLID /*CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER*/ );

	if ( tr.fraction != 1 || tr.startsolid || tr.contents & CONTENTS_WATER)
	{// Bad waypoint. Remove it!
		fixed_position[0] = 0.0f;
		fixed_position[1] = 0.0f;
		fixed_position[2] = -65536.0f;
		return;
	}

	// New floor test...
	/*fixed_position[2]=FloorHeightAt(fixed_position)+16;

	if (fixed_position[2] == -65536.0f || fixed_position[2] == 65536.0f)
	{// Bad waypoint. Remove it!
		fixed_position[0] = 0.0f;
		fixed_position[1] = 0.0f;
		fixed_position[2] = -65536.0f;
		return;
	}*/

	//
	// Let's try also centralizing the points...
	//
/*
	// Prepare for forward test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, 65536, forward, newOrg2);

	// Do forward test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER );
	
	if ( Distance(newOrg, tr.endpos) < 256 )
	{// Possibly a hallway.. Can we centralize it?
		float move_ammount = Distance(newOrg, tr.endpos);

		// Prepare for back test...
		VectorCopy( fixed_position, newOrg );
		VectorCopy( fixed_position, newOrg2 );
		VectorMA(newOrg2, -65536, forward, newOrg2);

		// Do back test...
		CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER );

		if ( Distance(newOrg, tr.endpos) < 256 )
		{
			move_ammount -= Distance(newOrg, tr.endpos);
			VectorMA(newOrg, move_ammount, forward, newOrg);
			VectorCopy(newOrg, fixed_position);
		}
	}

	// Prepare for right test...
	VectorCopy( fixed_position, newOrg );
	VectorCopy( fixed_position, newOrg2 );
	VectorMA(newOrg2, 64, right, newOrg2);

	// Do right test...
	CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER );

	if ( Distance(newOrg, tr.endpos) < 256 )
	{// Possibly a hallway.. Can we centralize it?
		float move_ammount = Distance(newOrg, tr.endpos);

		// Prepare for left test...
		VectorCopy( fixed_position, newOrg );
		VectorCopy( fixed_position, newOrg2 );
		VectorMA(newOrg2, -65536, right, newOrg2);

		// Do back test...
		CG_Trace( &tr, newOrg, NULL, NULL, newOrg2, -1, CONTENTS_PLAYERCLIP | MASK_SHOT | MASK_OPAQUE | CONTENTS_LAVA | MASK_WATER );

		if ( Distance(newOrg, tr.endpos) < 256 )
		{
			move_ammount -= Distance(newOrg, tr.endpos);
			VectorMA(newOrg, move_ammount, right, newOrg);
			VectorCopy(newOrg, fixed_position);
		}
	}*/
#endif //__AW_UNUSED__
}

vec3_t	aw_ladder_positions[MAX_NODES];
int		aw_num_ladder_positions = 0;
int		aw_total_waypoints = 0;

void AIMod_AutoWaypoint_Check_Create_Ladder_Waypoints ( vec3_t original_org, vec3_t angles )
{// UQ1: JKA -- Needed ???
	/*vec3_t			org1, org2, forward, right, up, last_endpos, ladder_pos;
	qboolean	complete = qfalse;
	vec3_t			traceMins = { -20, -20, -1 };
	vec3_t			traceMaxs = { 20, 20, 32 };

	VectorCopy(original_org, org1);
	//org1[2] = FloorHeightAt(org1);

	VectorCopy(original_org, org2);
	AngleVectors( angles, forward, right, up );
	VectorMA ( org2, waypoint_scatter_distance*3 , forward , org2);
	//org2[2] = FloorHeightAt(org2);

	while (!complete)
	{
		trace_t		trace;

		trap->CM_BoxTrace( &trace, org1, org2, traceMins, traceMaxs,  0, MASK_PLAYERSOLID );

		if (trace.surfaceFlags & SURF_LADDER)
		{// Ladder found here! Add a new waypoint!
			short int	objNum[3] = { 0, 0, 0 };

			VectorCopy(trace.endpos, ladder_pos);
			Load_AddNode( ladder_pos, 0, objNum, 0 );	//add the node
			aw_total_waypoints++;
			VectorCopy(ladder_pos, last_endpos);
		}
		else
		{// We found the top of the ladder... Add 1 more waypoint and we are done! :)
			short int	objNum[3] = { 0, 0, 0 };

			last_endpos[2]+=16;

			Load_AddNode( last_endpos, 0, objNum, 0 );	//add the node
			aw_total_waypoints++;

			complete = qtrue;
		}

		org1[2]+=16;
		org2[2]+=16;
	}*/
}
/*
vec3_t aw_ladder_origin;
vec3_t aw_ladder_angles;

void AIMod_AutoWaypoint_Best_Ladder_Side ( vec3_t org )
{
	vec3_t			org1, org2, forward, right, up, testangles, best_dir;
	int				i = 0, j = 0;
	vec3_t			traceMins = { -20, -20, -1 };
	vec3_t			traceMaxs = { 20, 20, 32 };
	float			best_height = -64000.0f;
	vec3_t			best_positions[MAX_NODES];
	int				num_best_positions = 0;

	VectorCopy(org, org1);
	VectorCopy(org, org2);

	for (i = 0; i <= 360; i+=1)
	{
		trace_t		trace;

		VectorCopy(org, org1);

		testangles[0] = testangles[1] = testangles[2] = 0;
		testangles[YAW] = (float)i;
		AngleVectors( testangles, forward, right, up );
		VectorMA ( org1, 40 , forward , org1);

		VectorCopy(org1, org2);
		org2[2]+=8192;

		trap->CM_BoxTrace( &trace, org1, org2, traceMins, traceMaxs,  0, MASK_PLAYERSOLID );

		if (trace.endpos[2] > best_height)
		{
			best_height = trace.endpos[2];
			VectorCopy(org1, best_positions[0]);
			num_best_positions = 1;
		}
		else if (trace.endpos[2] == best_height)
		{
			VectorCopy(org1, best_positions[num_best_positions]);
			num_best_positions++;
		}
	}

	if (num_best_positions == 1)
	{
		VectorCopy(best_positions[0], aw_ladder_origin);
	}
	else
	{// Use most central one...
		int choice = (num_best_positions-1)*0.5;
		VectorCopy(best_positions[choice], aw_ladder_origin);
	}

	VectorSubtract(org, aw_ladder_origin, best_dir);
	vectoangles(best_dir, aw_ladder_angles);
}*/

void AIMod_AutoWaypoint_Check_For_Ladders ( vec3_t org )
{// UQ1: JKA -- Needed ???
/*	vec3_t			org1, org2, forward, right, up, testangles;
	int				i = 0;//, j = 0;
//	vec3_t			traceMins = { -20, -20, -1 };
//	vec3_t			traceMaxs = { 20, 20, 32 };

	for ( i = 0; i < aw_num_ladder_positions; i++)
	{// Do a quick check to make sure we do not do the same ladder twice!
		if (VectorDistanceNoHeight(org, aw_ladder_positions[i]) < 128)
			return;
	}

	VectorCopy(org, org1);
	org1[2]+=16;

	for (i = 0; i < 360; i++)
	{
		trace_t		trace;

		VectorCopy(org, org2);

		testangles[0] = testangles[1] = testangles[2] = 0;
		testangles[YAW] = (float)i;
		AngleVectors( testangles, forward, right, up );
		VectorMA ( org2, waypoint_scatter_distance*3 , forward , org2);
		org2[2]+=16;

		trap->CM_BoxTrace( &trace, org1, org2, NULL, NULL,  0, MASK_PLAYERSOLID );
		
		if (trace.surfaceFlags & SURF_LADDER)
		{// Ladder found here! Make the waypoints!
			strcpy( last_node_added_string, va("^5Adding ladder (^3%i^5) waypoints at ^7%f %f %f^5.", aw_num_ladder_positions+1, trace.endpos[0], trace.endpos[1], trace.endpos[2]) );
			AIMod_AutoWaypoint_Check_Create_Ladder_Waypoints(org1, testangles);
			VectorCopy(trace.endpos, aw_ladder_positions[aw_num_ladder_positions]);
			aw_num_ladder_positions++;
			return; // Not likely more ladders here!
		}
	}*/
}

void AIMod_AutoWaypoint_Init_Memory ( void ); // below...
void AIMod_AutoWaypoint_Free_Memory ( void ); // below...

qboolean ContentsOK ( int contents )
{
	if (contents & CONTENTS_DETAIL || contents & CONTENTS_NODROP /*|| contents & CONTENTS_MONSTERCLIP || contents & CONTENTS_PLAYERCLIP || contents & CONTENTS_BOTCLIP || contents & CONTENTS_SLIME || contents & CONTENTS_LAVA || contents & CONTENTS_WATER*/ )
		return qfalse;

	return qtrue;
}

void
AIMod_GetMapBounts ( void )
{
	int		i;
	float	startx = -MAX_MAP_SIZE, starty = -MAX_MAP_SIZE, startz = -MAX_MAP_SIZE;
	float	highest_z_point = -MAX_MAP_SIZE;
	trace_t tr;
	vec3_t	org1;
	vec3_t	org2;
	vec3_t	mapMins, mapMaxs;

	VectorSet(mapMins, MAX_MAP_SIZE, MAX_MAP_SIZE, MAX_MAP_SIZE);
	VectorSet(mapMaxs, -MAX_MAP_SIZE, -MAX_MAP_SIZE, -MAX_MAP_SIZE);

	//
	// Z
	//
	i = 0;
	while ( startx < MAX_MAP_SIZE )
	{
		while ( starty < MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[2] += (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			
			if ( tr.endpos[2] < mapMins[2] )
			{
				mapMins[2] = tr.endpos[2];
				starty += 256;
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				starty += 256;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				starty += 256;
				continue;
			}

			starty += 256;
		}

		startx += 256;
		starty = -MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 1 completed.\n");

	mapMins[2] += 16;
	startx = MAX_MAP_SIZE;
	starty = MAX_MAP_SIZE;
	startz = MAX_MAP_SIZE;
	i = 0;
	while ( startx > -MAX_MAP_SIZE )
	{
		while ( starty > -MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[2] -= (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.endpos[2] > mapMaxs[2] )
			{
				mapMaxs[2] = tr.endpos[2];
				starty -= 256;
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				starty -= 256;
				continue;
			}

			starty -= 256;
		}

		startx -= 256;
		starty = MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 2 completed.\n");

	mapMaxs[2] -= 16;

	//
	// X
	//
	startx = -MAX_MAP_SIZE;
	starty = -MAX_MAP_SIZE;
	startz = mapMins[2];
	i = 0;
	while ( startz < mapMaxs[2] )
	{
		while ( starty < MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[0] += (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.endpos[0] < mapMins[0] )
			{
				starty += 256;
				mapMins[0] = tr.endpos[0];
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				starty += 256;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				starty += 256;
				continue;
			}

			starty += 256;
		}

		startz += 256;
		starty = -MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 3 completed.\n");

	mapMins[0] += 16;
	startx = MAX_MAP_SIZE;
	starty = MAX_MAP_SIZE;
	startz = mapMaxs[2];
	i = 0;
	while ( startz > mapMins[2] )
	{
		while ( starty > -MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[0] -= (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.endpos[0] > mapMaxs[0] )
			{
				mapMaxs[0] = tr.endpos[0];
				starty -= 256;
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				starty -= 256;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				starty -= 256;
				continue;
			}

			starty -= 256;
		}

		startz -= 256;
		starty = MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 4 completed.\n");

	mapMaxs[0] -= 16;

	//
	// Y
	//
	startx = -MAX_MAP_SIZE;
	starty = -MAX_MAP_SIZE;
	startz = mapMins[2];
	i = 0;
	while ( startz < mapMaxs[2] )
	{
		while ( startx < MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[1] += (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.endpos[1] < mapMins[1] )
			{
				mapMins[1] = tr.endpos[1];
				startx += 256;
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				startx += 256;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				startx += 256;
				continue;
			}

			startx += 256;
		}

		startz += 256;
		startx = -MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 5 completed.\n");

	mapMins[1] += 16;
	startx = MAX_MAP_SIZE;
	starty = MAX_MAP_SIZE;
	startz = mapMaxs[2];
	i = 0;
	while ( startz > mapMins[2] )
	{
		while ( startx > -MAX_MAP_SIZE )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[1] -= (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.endpos[1] > mapMaxs[1] )
			{
				mapMaxs[1] = tr.endpos[1];
				startx -= 256;
				continue;
			}

			if ( tr.startsolid || tr.allsolid )
			{
				startx -= 256;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				startx -= 256;
				continue;
			}

			startx -= 256;
		}

		startz -= 256;
		startx = MAX_MAP_SIZE;
	}

	//Com_Printf("Stage 6 completed.\n");

	mapMaxs[1] -= 16;

	//
	// Refine Z Top Point to highest ground height!
	//
	startx = mapMaxs[0] - 32;
	starty = mapMaxs[1] - 32;
	startz = mapMaxs[2] - 32;
	highest_z_point = mapMins[2];
	i = 0;
	while ( startx > mapMins[2] )
	{
		while ( starty > mapMins[1] )
		{
			VectorSet( org1, startx, starty, startz );
			VectorSet( org2, startx, starty, startz );
			org2[2] -= (MAX_MAP_SIZE*2);
			CG_Trace( &tr, org1, NULL, NULL, org2, ENTITYNUM_NONE, MASK_SHOT | MASK_WATER );
			if ( tr.startsolid || tr.allsolid )
			{
				starty -= 128;
				continue;
			}

			if (!ContentsOK(tr.contents) && !HasPortalFlags(tr.surfaceFlags, tr.contents))
			{
				starty -= 128;
				continue;
			}

			if ( tr.endpos[2] > highest_z_point )
			{
				highest_z_point = tr.endpos[2];
				starty -= 128;
				continue;
			}

			starty -= 64 /*128*/ ;
		}

		startx -= 64 /*128*/ ;
		starty = mapMaxs[1];
	}

	//Com_Printf("Stage 7 completed.\n");

	if ( highest_z_point <= mapMins[2] )
	{
		highest_z_point = mapMaxs[2] - 32;
	}

	if ( highest_z_point <= mapMins[2] + 128 )
	{
		highest_z_point = mapMaxs[2] - 32;
	}

	mapMaxs[2] = highest_z_point;

	if (mapMaxs[0] < mapMins[0])
	{
		float temp = mapMins[0];
		mapMins[0] = mapMaxs[0];
		mapMaxs[0] = temp;
	}

	if (mapMaxs[1] < mapMins[1])
	{
		float temp = mapMins[1];
		mapMins[1] = mapMaxs[1];
		mapMaxs[1] = temp;
	}

	if (mapMaxs[2] < mapMins[2])
	{
		float temp = mapMins[2];
		mapMins[2] = mapMaxs[2];
		mapMaxs[2] = temp;
	}

	//trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Map mins %f %f %f.\n", mapMins[0], mapMins[1], mapMins[2]);
	//trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Map maxs %f %f %f.\n", mapMaxs[0], mapMaxs[1], mapMaxs[2]);

	VectorCopy(mapMins, cg.mapcoordsMins);
	VectorCopy(mapMaxs, cg.mapcoordsMaxs);
	cg.mapcoordsValid = qtrue;
}

void AIMod_AutoWaypoint_StandardMethod( void )
{// Advanced method for multi-level maps...
	int			i;
	float		startx = -MAX_MAP_SIZE, starty = -MAX_MAP_SIZE, startz = -MAX_MAP_SIZE;
	float		orig_startx, orig_starty, orig_startz;
	int			areas = 0, total_waypoints = 0, total_areas = 0;
	intvec3_t	*arealist;
	float		map_size, temp, original_waypoint_scatter_distance = waypoint_scatter_distance;
	vec3_t		mapMins, mapMaxs;
	int			total_tests = 0, final_tests = 0;
	int			start_time = trap->Milliseconds();
	int			update_timer = 0;
	float		waypoint_scatter_realtime_modifier = 1.0f;
	float		waypoint_scatter_realtime_modifier_alt = 0.5f; // 0.3f;
	int			wp_loop = 0;

	trap->Cvar_Set("jkg_waypoint_render", "0");
	trap->UpdateScreen();
	trap->UpdateScreen();
	trap->UpdateScreen();

	AIMod_AutoWaypoint_Init_Memory();
	AIMod_GetMapBounts();

	if (!cg.mapcoordsValid)
	{
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^7Map Coordinates are invalid. Can not use auto-waypointer!\n");
		return;
	}

	arealist = (intvec3_t *)malloc((sizeof(intvec3_t)+1)*512000);

	VectorCopy(cg.mapcoordsMins, mapMins);
	VectorCopy(cg.mapcoordsMaxs, mapMaxs);

	if (mapMaxs[0] < mapMins[0])
	{
		temp = mapMins[0];
		mapMins[0] = mapMaxs[0];
		mapMaxs[0] = temp;
	}

	if (mapMaxs[1] < mapMins[1])
	{
		temp = mapMins[1];
		mapMins[1] = mapMaxs[1];
		mapMaxs[1] = temp;
	}

	if (mapMaxs[2] < mapMins[2])
	{
		temp = mapMins[2];
		mapMins[2] = mapMaxs[2];
		mapMaxs[2] = temp;
	}

	map_size = Distance(mapMins, mapMaxs);

	// Work out the best scatter distance to use for this map size...
	/*if (map_size > 96000)
	{
		waypoint_scatter_distance *= 5;
	}
	else if (map_size > 32768)
	{
		waypoint_scatter_distance *= 4;
	}
	else if (map_size > 24000)
	{
		waypoint_scatter_distance *= 3;
	}
	else if (map_size > 20000)
	{
		waypoint_scatter_distance *= 2;
	}
	else if (map_size > 16550)
	{
		waypoint_scatter_distance *= 1.5;
	}
	else if (map_size > 8192)
	{
		waypoint_scatter_distance *= 1.12;
	}*/

	orig_startx = mapMaxs[0]+2048/*-16*/;
	orig_starty = mapMaxs[1]+2048/*-16*/;
	orig_startz = mapMaxs[2]+2048/*-16*/;

	startx = orig_startx;
	starty = orig_starty;
	startz = orig_startz;

//#pragma omp parallel
	while ( startx > mapMins[0]-2048 )
	{
		while ( starty > mapMins[1]-2048 )
		{
			while ( startz > mapMins[2]-2048 )
			{
				startz -= 70;//waypoint_scatter_distance;
				total_tests++;
			}

			startz = orig_startz;
			starty -= waypoint_scatter_distance;
		}

		starty = orig_starty;
		startx -= waypoint_scatter_distance;
	}

	startx = orig_startx;
	starty = orig_starty;
	startz = orig_startz;

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Map bounds are ^3%.2f %.2f %.2f ^5to ^3%.2f %.2f %.2f^5.\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2] );
	strcpy( task_string1, va("^5Map bounds are ^3%.2f %.2f %.2f ^7to ^3%.2f %.2f %.2f^5.", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2]) );
	trap->UpdateScreen();

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Generating AI waypoints. This could take a while... (Map size ^3%.2f^5)\n", map_size );
	strcpy( task_string2, va("^5Generating AI waypoints. This could take a while... (Map size ^3%.2f^5)", map_size) );
	trap->UpdateScreen();

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5First pass. Finding temporary waypoints...\n" );
	strcpy( task_string3, va("^5First pass. Finding temporary waypoints...") );
	trap->UpdateScreen();

	//#pragma omp parallel
	while ( startx > mapMins[0]-2048 )
	{
		//#pragma omp parallel
		while ( starty > mapMins[1]-2048 )
		{
			vec3_t last_org, new_org;
			float floor, current_floor = startz;

			VectorSet(last_org, 0, 0, 0);
			VectorSet(new_org, startx, starty, startz);

			floor = FloorHeightAt(new_org);

			if (floor == -65536.0f || floor <= mapMins[2]-2048)
			{// Can skip this one!
				//#pragma omp parallel
				while (startz > floor && startz > mapMins[2]-2048)
				{// We can skip some checks, until we get to our hit position...
					startz -= waypoint_scatter_distance;
					final_tests++;

					// Draw a nice little progress bar ;)
					aw_percent_complete = (float)((float)((float)final_tests/(float)total_tests)*100.0f);

					update_timer++;

					if (update_timer >= 15000)
					{
						trap->UpdateScreen();
						update_timer = 0;
					}
				}

				// since we failed, decrease modifier...
				//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
				{// Distance Variation...
					int p;
					qboolean got_close = qfalse;
					int startspot = areas - 100;

					if (startspot < 0) startspot = 0;

					for (p = startspot; p < areas-1; p++)
					{
						intvec3_t org2;
						org2[0] = startx;
						org2[1] = starty;
						org2[2] = floor;

						if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
						{
							got_close = qtrue;
							break;
						}
					}

					if (got_close)
						waypoint_scatter_realtime_modifier = 2.0f;
					else
						waypoint_scatter_realtime_modifier = 0.33f;
				}

				startz = orig_startz;
				starty -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
				continue;
			}

			if (floor >= 65536.0f)
			{// Marks a start-solid or on top of the sky... Skip...
				// since we failed, decrease modifier...
				//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
				{// Distance Variation...
					int p;
					qboolean got_close = qfalse;
					int startspot = areas - 100;

					if (startspot < 0) startspot = 0;

					for (p = startspot; p < areas-1; p++)
					{
						intvec3_t org2;
						org2[0] = startx;
						org2[1] = starty;
						org2[2] = floor;

						if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
						{
							got_close = qtrue;
							break;
						}
					}

					if (got_close)
						waypoint_scatter_realtime_modifier = 2.0f;
					else
						waypoint_scatter_realtime_modifier = 0.33f;
				}

				startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;

				VectorSet(new_org, startx, starty, startz);

				floor = FloorHeightAt(new_org);
			}

			//#pragma omp parallel
			while ( startz > mapMins[2]-2048 )
			{
				vec3_t org;
				float orig_floor;
				//				qboolean bad = qfalse;

				if (current_floor == -65536.0f || current_floor <= mapMins[2]-2048)
				{// Can skip this one!
					//#pragma omp parallel
					while (startz > floor && startz > mapMins[2]-2048)
					{// We can skip some checks, until we get to our hit position...
						startz -= waypoint_scatter_distance;
						final_tests++;

						// Draw a nice little progress bar ;)
						aw_percent_complete = (float)((float)((float)final_tests/(float)total_tests)*100.0f);

						update_timer++;

						if (update_timer >= 15000)
						{
							trap->UpdateScreen();
							update_timer = 0;
						}
					}

					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz = orig_startz;
					starty -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					break;
				}

				update_timer++;

				if (update_timer >= 15000)
				{
					trap->UpdateScreen();
					update_timer = 0;
				}

				VectorSet(org, startx, starty, startz);

				org[2]=FloorHeightAt(org);

				if (org[2] <= -65536.0f)
				{
					//#pragma omp parallel
					while (startz > floor && startz > mapMins[2]-2048)
					{// We can skip some checks, until we get to our hit position...
						startz -= waypoint_scatter_distance;
						final_tests++;

						// Draw a nice little progress bar ;)
						aw_percent_complete = (float)((float)((float)final_tests/(float)total_tests)*100.0f);

						update_timer++;

						if (update_timer >= 15000)
						{
							trap->UpdateScreen();
							update_timer = 0;
						}
					}

					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz = orig_startz;
					starty -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					break;
				}

				final_tests++;

				// Draw a nice little progress bar ;)
				aw_percent_complete = (float)((float)((float)final_tests/(float)total_tests)*100.0f);

				if (org[2] >= 65536.0f)
				{// Marks a start-solid or on top of the sky... Skip...
					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					continue;
				}

				if (Distance(org, last_org) < (waypoint_scatter_distance))
				{
					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					continue;
				}

				if (AIMod_AutoWaypoint_Check_Slope(org))
				{// Bad slope angles here!
					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					continue;
				}

				if (!AIMod_AutoWaypoint_Check_PlayerWidth(org))
				{// Not wide enough for a player to fit!
					// since we failed, decrease modifier...
					//waypoint_scatter_realtime_modifier = waypoint_scatter_realtime_modifier_alt;
					{// Distance Variation...
						int p;
						qboolean got_close = qfalse;
						int startspot = areas - 100;

						if (startspot < 0) startspot = 0;

						for (p = startspot; p < areas-1; p++)
						{
							intvec3_t org2;
							org2[0] = startx;
							org2[1] = starty;
							org2[2] = floor;

							if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
							{
								got_close = qtrue;
								break;
							}
						}

						if (got_close)
							waypoint_scatter_realtime_modifier = 2.0f;
						else
							waypoint_scatter_realtime_modifier = 0.33f;
					}

					startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					continue;
				}

				// Raise node a little to add it...(for visibility)
				if (org[2] >= -65536.0f && org[2] <= 65536.0f)
				{
					orig_floor = org[2];
					org[2] += 8;

					//trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Adding temp waypoint ^3%i ^7at ^7%f %f %f^5. (^3%.2f^5%% complete)\n", areas, org[0], org[1], org[2], (float)((float)((float)final_tests/(float)total_tests)*100.0f));
					strcpy( last_node_added_string, va("^5Adding temp waypoint ^3%i ^5at ^7%f %f %f^5.", areas/*+areas2+areas3*/, org[0], org[1], org[2]) );

					VectorCopy( org, (float *)arealist[areas] );
					areas++;
				}
				else
				{// Don't add this one to the temp list...
					orig_floor = org[2];
					org[2] += 8;
				}

				if (org[2] >= -65536.0f && org[2] <= 65536.0f && aw_floor_trace_hit_mover)
				{// Need to generate waypoints to the top/bottom of the mover...
					float temp_roof, temp_ground;
					vec3_t temp_org, temp_org2;

					VectorCopy(org, temp_org2);
					VectorCopy(org, temp_org);
					temp_org[2] += 24; // Start above the lift...
					temp_roof = RoofHeightAt(temp_org);

					VectorCopy(org, temp_org);
					temp_org[2] -= 24; // Start below the lift...
					temp_ground = GroundHeightAt(temp_org);

					temp_org2[2] = temp_roof;

					if (temp_roof <= mapMaxs[2]/*< 64000*/ && HeightDistance(temp_org, temp_org2) >= 128)
					{// Looks like it goes up!
						int z = 0;

						VectorCopy(org, temp_org);
						temp_org[2] += 24;

						while (temp_org[2] <= temp_org2[2])
						{// Add waypoints all the way up!
							VectorCopy( temp_org, (float *)arealist[areas] );
							areas++;
							temp_org[2] += 24;
						}
					}

					temp_org2[2] = temp_ground;

					if (temp_roof >= mapMins[2]/*> -64000*/ && HeightDistance(temp_org, temp_org2) >= 128)
					{// Looks like it goes up!
						int z = 0;

						VectorCopy(org, temp_org);
						temp_org[2] -= 24;

						while (temp_org[2] >= temp_org2[2])
						{// Add waypoints all the way up!
							VectorCopy( temp_org, (float *)arealist[areas] );
							areas++;
							temp_org[2] -= 24;
						}
					}
				}

				// Lower current org a back to where it was before raising it above...
				org[2] = orig_floor;

				VectorCopy(org, last_org);

				floor = org[2];
				current_floor = floor;

				// since we passed, increase modifier...
				/*
				if (waypoint_scatter_realtime_modifier == 1.25f)
				waypoint_scatter_realtime_modifier = 1.0f;
				else if (waypoint_scatter_realtime_modifier == 1.0f)
				waypoint_scatter_realtime_modifier = 0.5f;
				else
				waypoint_scatter_realtime_modifier = 1.25f;
				*/

				{// Distance Variation...
					int p;
					qboolean got_close = qfalse;
					int startspot = areas - 100;

					if (startspot < 0) startspot = 0;

					for (p = startspot; p < areas-1; p++)
					{
						intvec3_t org2;
						org2[0] = org[0];
						org2[1] = org[1];
						org2[2] = org[2];

						if (intVectorDistance(arealist[p], org2) <= waypoint_scatter_distance)
						{
							got_close = qtrue;
							break;
						}
					}

					if (got_close)
						waypoint_scatter_realtime_modifier = 2.0f;
					else
						waypoint_scatter_realtime_modifier = 0.33f;
				}


				startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;

//#pragma omp parallel
				while (startz > floor && startz > mapMins[2]-2048)
				{// We can skip some checks, until we get to our current floor height...
					startz -= waypoint_scatter_distance * waypoint_scatter_realtime_modifier;
					final_tests++;

					// Draw a nice little progress bar ;)
					aw_percent_complete = (float)((float)((float)final_tests/(float)total_tests)*100.0f);

					update_timer++;

					if (update_timer >= 15000)
					{
						trap->UpdateScreen();
						update_timer = 0;
					}
				}
			}

			startz = orig_startz;
			starty -= waypoint_scatter_distance;
		}

		starty = orig_starty;
		startx -= waypoint_scatter_distance;
	}

	if (areas < 32000)
	{// UQ1: Can use them all!
		aw_percent_complete = 0.0f;
		strcpy( task_string3, va("^5Final (cleanup) pass. Building final waypoints...") );
		trap->UpdateScreen();

		total_areas = areas;

//#pragma omp parallel for
		for ( i = 0; i < areas; i++ )
		{
			vec3_t		area_org;
			short int	objNum[3] = { 0, 0, 0 };

			// Draw a nice little progress bar ;)
			aw_percent_complete = (float)((float)((float)i/(float)total_areas)*100.0f);
				
			update_timer++;

			if (update_timer >= 500)
			{
				trap->UpdateScreen();
				update_timer = 0;
			}

			area_org[0] = arealist[i][0];
			area_org[1] = arealist[i][1];
			area_org[2] = arealist[i][2];

			if (area_org[2] <= -65536.0f)
			{// This is a bad height!
				continue;
			}

			strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", total_waypoints, area_org[0], area_org[1], area_org[2]) );
			Load_AddNode( area_org, 0, objNum, 0 );	//add the node
			total_waypoints++;
		}

		// Ladders...
//		aw_total_waypoints = total_waypoints;
//		aw_percent_complete = 0.0f;
//		strcpy( task_string3, va("^5Looking for ladders...") );
//		trap->UpdateScreen();

//		for ( i = 0; i < total_waypoints; i++ )
//		{
//			// Draw a nice little progress bar ;)
//			aw_percent_complete = (float)((float)((float)i/(float)total_waypoints)*100.0f);
				
//			update_timer++;

//			if (update_timer >= 100)
//			{
//				trap->UpdateScreen();
//				update_timer = 0;
//			}

//			AIMod_AutoWaypoint_Check_For_Ladders( nodes[i].origin );
//		}

//		total_waypoints = aw_total_waypoints;
//		// End Ladders...

		strcpy( task_string3, va("^5Saving %i generated waypoints.", total_waypoints) );
		trap->UpdateScreen();

		number_of_nodes = total_waypoints;
		AIMOD_NODES_SaveNodes_Autowaypointed();

		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint database created successfully in ^3%.2f ^5seconds with ^7%i ^5waypoints.\n",
				 (float) ((trap->Milliseconds() - start_time) / 1000), total_waypoints );

		strcpy( task_string3, va("^5Waypoint database created successfully in ^3%.2f ^5seconds with ^7%i ^5waypoints.\n", (float) ((trap->Milliseconds() - start_time) / 1000), total_waypoints) );
		trap->UpdateScreen();

		aw_percent_complete = 0.0f;
		strcpy( task_string3, va("^5Informing the server to load & test the new waypoints...") );
		trap->UpdateScreen();

		//trap->SendConsoleCommand( "set bot_wp_visconnect 1\n" );
		//trap->SendConsoleCommand( "bot_wp_convert_awp\n" );

		aw_percent_complete = 0.0f;
		strcpy( task_string3, va("^5Waypoint auto-generation is complete...") );
		trap->UpdateScreen();

		waypoint_scatter_distance = original_waypoint_scatter_distance;
		
		AIMOD_Generate_Cover_Spots(); // UQ1: Want to add these to JKA???

		free(arealist);

		AIMod_AutoWaypoint_Free_Memory();

		aw_percent_complete = 0.0f;
		trap->UpdateScreen();

		return;
	}

	total_areas = areas;

	/*if (total_areas < 40000)
	{
		waypoint_distance_multiplier = 1.5f;
	}
	else if (total_areas < 64000)
	{
		waypoint_distance_multiplier = 2.0f;
	}
	else
	{
		waypoint_distance_multiplier = 2.5f;
	}*/

#ifdef __AW_UNUSED__
	aw_percent_complete = 0.0f;
	strcpy( task_string3, va("^5Second (repair) pass. Adjusting waypoint positions...") );
	trap->UpdateScreen();

//#pragma omp parallel for
	for ( i = 0; i < areas; i++ )
	{
		vec3_t original_position = { arealist[i][0], arealist[i][1], arealist[i][2] };

		// Draw a nice little progress bar ;)
		aw_percent_complete = (float)((float)((float)i/(float)total_areas)*100.0f);
		
		update_timer++;

		if (update_timer >= 500)
		{
			trap->UpdateScreen();
			update_timer = 0;
		}

		RepairPosition( arealist[i] );

		if (Distance(fixed_position, original_position) > 0)
		{// New position.. Fix it!
			strcpy( last_node_added_string, va("^5Temp waypoint ^3%i ^5moved to ^7%f %f %f^5.", i, fixed_position[0], fixed_position[1], fixed_position[2]) );
			arealist[i][0] = fixed_position[0];
			arealist[i][1] = fixed_position[1];
			arealist[i][2] = fixed_position[2];
		}
	}
#endif //__AW_UNUSED__

	aw_percent_complete = 0.0f;
	strcpy( task_string3, va("^5Final (cleanup) pass. Building final waypoints...") );
	trap->UpdateScreen();

//#pragma omp parallel for
	for ( i = 0; i < areas; i++ )
	{
		vec3_t		area_org;
		short int	objNum[3] = { 0, 0, 0 };
		qboolean	bad = qfalse;
		int			j;

		// Draw a nice little progress bar ;)
		aw_percent_complete = (float)((float)((float)i/(float)total_areas)*100.0f);
				
		update_timer++;

		if (update_timer >= 100)
		{
			trap->UpdateScreen();
			update_timer = 0;
		}

		area_org[0] = arealist[i][0];
		area_org[1] = arealist[i][1];
		area_org[2] = arealist[i][2];

		if (area_org[2] <= -65536.0f)
		{// This is a bad height!
			continue;
		}

//#pragma omp parallel
		for (j = 0; j < number_of_nodes; j++)
		{
			vec3_t area_org2;

			area_org2[0] = nodes[j].origin[0];
			area_org2[1] = nodes[j].origin[1];
			area_org2[2] = nodes[j].origin[2];

			if (Distance(area_org, area_org2) < waypoint_scatter_distance*area_distance_multiplier)
			{
				bad = qtrue;
				break;
			}
		}

		if (bad)
		{
			continue;
		}

		strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", total_waypoints, area_org[0], area_org[1], area_org[2]) );
		Load_AddNode( area_org, 0, objNum, 0 );	//add the node
		total_waypoints++;
	}

	// Ladders...
	/*aw_total_waypoints = total_waypoints;
	aw_percent_complete = 0.0f;
	strcpy( task_string3, va("^5Looking for ladders...") );
	trap->UpdateScreen();

	for ( i = 0; i < total_waypoints; i++ )
	{
		// Draw a nice little progress bar ;)
		aw_percent_complete = (float)((float)((float)i/(float)total_waypoints)*100.0f);
				
		update_timer++;

		if (update_timer >= 100)
		{
			trap->UpdateScreen();
			update_timer = 0;
		}

		AIMod_AutoWaypoint_Check_For_Ladders( nodes[i].origin );
	}

	total_waypoints = aw_total_waypoints;*/
	// End Ladders...

	strcpy( task_string3, va("^5Saving the generated waypoints.\n") );
	trap->UpdateScreen();

	number_of_nodes = total_waypoints;
	AIMOD_NODES_SaveNodes_Autowaypointed();

	AIMOD_Generate_Cover_Spots(); // UQ1: Want to add these to JKA???

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint database created successfully in ^3%.2f ^5seconds with ^7%i ^5waypoints.\n",
				 (float) ((trap->Milliseconds() - start_time) / 1000), total_waypoints);

	strcpy( task_string3, va("^5Waypoint database created successfully in ^3%.2f ^5seconds with ^7%i ^5waypoints.\n", (float) ((trap->Milliseconds() - start_time) / 1000), total_waypoints) );
	trap->UpdateScreen();

	aw_percent_complete = 0.0f;
	strcpy( task_string3, va("^5Informing the server to load & test the new waypoints...") );
	trap->UpdateScreen();

	//trap->SendConsoleCommand( "set bot_wp_visconnect 1\n" );
	//trap->SendConsoleCommand( "bot_wp_convert_awp\n" );

	aw_percent_complete = 0.0f;
	strcpy( task_string3, va("^5Waypoint auto-generation is complete...") );
	trap->UpdateScreen();

	waypoint_scatter_distance = original_waypoint_scatter_distance;

	free(arealist);

	AIMod_AutoWaypoint_Free_Memory();

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();
}

qboolean wp_memory_initialized = qfalse;

void AIMod_AutoWaypoint_Init_Memory ( void )
{
	number_of_nodes = 0;

	if (wp_memory_initialized == qfalse)
	{
		nodes = (node_t *)malloc( (sizeof(node_t)+1)*MAX_NODES );
		wp_memory_initialized = qtrue;
	}
}

void AIMod_AutoWaypoint_Free_Memory ( void )
{
	number_of_nodes = 0;

	if (wp_memory_initialized == qtrue)
	{
		free(nodes);
		wp_memory_initialized = qfalse;
	}
}

void AIMod_AutoWaypoint_Init_Memory ( void ); // below...
void AIMod_AutoWaypoint_Optimizer ( void ); // below...
//void AIMod_AutoWaypoint_Cleaner ( qboolean quiet, qboolean null_links_only, qboolean relink_only ); // below...
void AIMod_AutoWaypoint_Cleaner ( qboolean quiet, qboolean null_links_only, qboolean relink_only, qboolean multipass, qboolean initial_pass, qboolean extra, qboolean marked_locations, qboolean extra_reach );

void AIMod_AutoWaypoint_Clean ( void )
{
	char	str[MAX_TOKEN_CHARS];

	if ( trap->Cmd_Argc() < 2 )
	{
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Usage:\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3/awc <method>^5.\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Available methods are:\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"relink\" ^5- Just do relinking.\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"clean\" ^5- Do a full clean.\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"multipass\" ^5- Do a multi-pass full clean (max optimize).\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"extra\" ^5- Do a full clean (but remove more - good if the number is still too high after optimization).\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"markedlocations\" ^5- Remove waypoints nearby your marked locations (awc_addremovalspot & awc_addbadheight).\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"extrareach\" ^5- Remove waypoints nearby your marked locations (awc_addremovalspot & awc_addbadheight) and add extra reachability (wp link ranges).\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"cover\" ^5- Just generate coverpoints.\n");
		trap->UpdateScreen();
		return;
	}

	trap->Cmd_Argv( 1, str, sizeof(str) );
	
	if ( Q_stricmp( str, "relink") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qtrue, qfalse, qfalse, qfalse, qfalse, qfalse);
	}
	else if ( Q_stricmp( str, "clean") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qfalse, qfalse, qfalse, qfalse, qfalse);
	}
	else if ( Q_stricmp( str, "multipass") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qtrue, qfalse, qfalse, qfalse, qfalse);
	}
	else if ( Q_stricmp( str, "extra") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qtrue, qfalse, qtrue, qfalse, qfalse);
	}
	else if ( Q_stricmp( str, "markedlocations") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qfalse, qfalse, qfalse, qtrue, qfalse);
	}
	else if ( Q_stricmp( str, "extrareach") == 0 )
	{
		AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qfalse, qfalse, qfalse, qtrue, qtrue);
	}
	else if ( Q_stricmp( str, "cover") == 0 )
	{
		AIMod_AutoWaypoint_Init_Memory();

		if (number_of_nodes > 0)
		{// UQ1: Init nodes list!
			number_of_nodes = 0; 
			optimized_number_of_nodes = 0;

			memset( nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
			memset( optimized_nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
		}

		AIMOD_NODES_LoadNodes();

		if (number_of_nodes <= 0)
		{
			AIMod_AutoWaypoint_Free_Memory();
			return;
		}

		AIMOD_Generate_Cover_Spots();

		AIMod_AutoWaypoint_Free_Memory();
	}
}

/* */
void
AIMod_AutoWaypoint ( void )
{
	char	str[MAX_TOKEN_CHARS];
	int		original_wp_scatter_dist = waypoint_scatter_distance;

	if ( trap->Cmd_Argc() < 2 )
	{
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Usage:\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3/autowaypoint <method> <scatter_distance> ^5or ^3/awp <method> <scatter_distance>^5. Distance is optional.\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Available methods are:\n" );
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"standard\" ^5- For standard multi-level maps.\n");
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"thorough\" ^5- Use extensive fall waypoint checking.\n");
		//trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"noclean\" ^5- For standard multi-level maps (with no cleaning passes).\n");
		//trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^3\"outdoor_only\" ^5- For standard single level maps.\n");
		trap->UpdateScreen();
		return;
	}

	DO_THOROUGH = qfalse;

	trap->Cmd_Argv( 1, str, sizeof(str) );
	
	if ( Q_stricmp( str, "standard") == 0 )
	{
		if ( trap->Cmd_Argc() >= 2 )
		{
			// Override normal scatter distance...
			int dist = waypoint_scatter_distance;

			trap->Cmd_Argv( 2, str, sizeof(str) );
			dist = atoi(str);

			if (dist <= 20)
			{
				// Fallback and warning...
				dist = original_wp_scatter_dist;

				trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Warning: ^5Invalid scatter distance set (%i). Using default (%i)...\n", atoi(str), original_wp_scatter_dist );
			}

			waypoint_scatter_distance = dist;
			AIMod_AutoWaypoint_StandardMethod();

			waypoint_scatter_distance = original_wp_scatter_dist;
		}
		else
		{
			AIMod_AutoWaypoint_StandardMethod();
		}
	}
	else if ( Q_stricmp( str, "thorough") == 0 )
	{
		if ( trap->Cmd_Argc() >= 2 )
		{
			// Override normal scatter distance...
			int dist = waypoint_scatter_distance;

			trap->Cmd_Argv( 2, str, sizeof(str) );
			dist = atoi(str);

			if (dist <= 20)
			{
				// Fallback and warning...
				dist = original_wp_scatter_dist;

				trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7Warning: ^5Invalid scatter distance set (%i). Using default (%i)...\n", atoi(str), original_wp_scatter_dist );
			}

			waypoint_scatter_distance = dist;

			DO_THOROUGH = qtrue;

			AIMod_AutoWaypoint_StandardMethod();

			waypoint_scatter_distance = original_wp_scatter_dist;
		}
		else
		{
			DO_THOROUGH = qtrue;

			AIMod_AutoWaypoint_StandardMethod();
		}
	}
	/*else if ( Q_stricmp( str, "noclean") == 0 )
	{
		AIMod_AutoWaypoint_StandardMethod();
		return;
	}
	else if ( Q_stricmp( str, "outdoor_only") == 0 )
	{
		//AIMod_AutoWaypoint_OutdoorMethod();
	}*/

	//AIMod_AutoWaypoint_Cleaner(qtrue, qtrue, qfalse, qtrue, qtrue, qfalse);
}

//
// Autowaypoint Optimizer...
//

/*
===========================================================================
GetFCost
Utility function used by A* pathfinding to calculate the
cost to move between nodes towards a goal.  Using the A*
algorithm F = G + H, G here is the distance along the node
paths the bot must travel, and H is the straight-line distance
to the goal node.
Returned as an int because more precision is unnecessary and it
will slightly speed up heap access
===========================================================================
*/
int
GetFCost ( centity_t *bot, int to, int num, int parentNum, float *gcost )
{
	float	gc = 0;
	float	hc = 0;
	vec3_t	v;
	
	if ( gcost[num] == -1 )
	{
		if ( parentNum != -1 )
		{
			gc = gcost[parentNum];
			VectorSubtract( nodes[num].origin, nodes[parentNum].origin, v );
			gc += VectorLength( v );

			// UQ1: Make WATER, BARB WIRE, and ICE nodes cost more!
			if (nodes[num].type & NODE_WATER)
			{// Huge cost for water nodes!
				//gc+=(gc*100);
				gc = 65000.0f;
			}
			else if (nodes[num].type & NODE_ICE)
			{// Huge cost for ice(slick) nodes!
				//gc+=(gc*100);
				gc = 65000.0f;
			}

			if (gc > 65000)
				gc = 65000.0f;

			/*if (nodes[num].type & NODE_COVER)
			{// Encorage the use of cover spots!
				gc = 0.0f;
			}*/
		}

		gcost[num] = gc;
	}
	else
	{
		gc = gcost[num];
		//G_Printf("gcost for num %i is %f\n", num, gc);
	}

	VectorSubtract( nodes[to].origin, nodes[num].origin, v );
	hc = VectorLength( v );

	/*if (nodes[num].type & NODE_WATER)
	{// Huge cost for water nodes!
		hc*=2;
	}
	else if (nodes[num].type & NODE_ICE)
	{// Huge cost for ice(slick) nodes!
		hc*=4;
	}*/

	/*if (nodes[num].type & NODE_COVER)
	{// Encorage the use of cover spots!
		hc *= 0.5;
	}*/

	return (int) ( gc + hc );
}

#define MAX_PATHLIST_NODES  4096/*8192*//*MAX_NODES*///1024	// MAX_NODES??

int
CreatePathAStar ( centity_t *bot, int from, int to, int *pathlist )
{
	int	openlist[MAX_NODES*2 + 1];												//add 1 because it's a binary heap, and they don't use 0 - 1 is the first used index
	float		gcost[MAX_NODES];
	int			fcost[MAX_NODES];
	char			list[MAX_NODES];														//0 is neither, 1 is open, 2 is closed - char because it's the smallest data type
	int	parent[MAX_NODES];
	int	numOpen = 0;
	int	atNode, temp, newnode = -1;
	qboolean	found = qfalse;
	int			count = -1;
	float		gc;
	int			i, j, u, v, m;
	vec3_t		vec;

	/*for (i = 0; i < MAX_NODES; i++)
	{
		gcost[i] = -1.0f;
		fcost[i] = 0;
		openlist[i] = 0;
		parent[i] = 0;
		list[i] = 0;
	}*/

	//clear out all the arrays - UQ1: Added - only allocate total nodes for map for speed...
	memset( openlist, 0, ((sizeof(int)) * (number_of_nodes + 1)) );
	memset( fcost, 0, ((sizeof(int)) * number_of_nodes) );
	memset( list, 0, ((sizeof(char)) * number_of_nodes) );
	memset( parent, 0, ((sizeof(int)) * number_of_nodes) );
	memset( gcost, 0, ((sizeof(float)) * number_of_nodes) );

	openlist[MAX_NODES+1] = 0;

	if ( (from == NODE_INVALID) || (to == NODE_INVALID) || (from >= MAX_NODES) || (to >= MAX_NODES) || (from == to) )
	{
		return ( -1 );
	}

	openlist[1] = from;																	//add the starting node to the open list
	numOpen++;
	gcost[from] = 0;																	//its f and g costs are obviously 0
	fcost[from] = 0;

	while ( 1 )
	{
		if ( numOpen != 0 )																//if there are still items in the open list
		{
			//pop the top item off of the list
			atNode = openlist[1];
			list[atNode] = 2;															//put the node on the closed list so we don't check it again
			numOpen--;
			openlist[1] = openlist[numOpen + 1];										//move the last item in the list to the top position
			v = 1;

			//this while loop reorders the list so that the new lowest fcost is at the top again
			while ( 1 )
			{
				u = v;
				if ( (2 * u + 1) < numOpen )											//if both children exist
				{
					if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
					{
						v = 2 * u;
					}

					if ( fcost[openlist[v]] >= fcost[openlist[2 * u + 1]] )
					{
						v = 2 * u + 1;
					}
				}
				else
				{
					if ( (2 * u) < numOpen )											//if only one child exists
					{
						if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
						{
							v = 2 * u;
						}
					}
				}

				if ( u != v )															//if they're out of order, swap this item with its parent
				{
					temp = openlist[u];
					openlist[u] = openlist[v];
					openlist[v] = temp;
				}
				else
				{
					break;
				}
			}

			if (nodes[atNode].enodenum <= MAX_NODELINKS)
			for ( i = 0; i < nodes[atNode].enodenum; i++ )								//loop through all the links for this node
			{
				newnode = nodes[atNode].links[i].targetNode;

				if (newnode > number_of_nodes)
					continue;

				if (newnode < 0)
					continue;

				if (nodes[newnode].objectNum[0] == 1)
					continue; // Skip water/ice disabled node!

				if ( list[newnode] == 2 )
				{																		//if this node is on the closed list, skip it
					continue;
				}

				if ( list[newnode] != 1 )												//if this node is not already on the open list
				{
					openlist[++numOpen] = newnode;										//add the new node to the open list
					list[newnode] = 1;
					parent[newnode] = atNode;											//record the node's parent
					if ( newnode == to )
					{																	//if we've found the goal, don't keep computing paths!
						break;															//this will break the 'for' and go all the way to 'if (list[to] == 1)'
					}

					fcost[newnode] = GetFCost( bot, to, newnode, parent[newnode], gcost );	//store it's f cost value

					//this loop re-orders the heap so that the lowest fcost is at the top
					m = numOpen;

					while ( m != 1 )													//while this item isn't at the top of the heap already
					{
						if ( fcost[openlist[m]] <= fcost[openlist[m / 2]] )				//if it has a lower fcost than its parent
						{
							temp = openlist[m / 2];
							openlist[m / 2] = openlist[m];
							openlist[m] = temp;											//swap them
							m /= 2;
						}
						else
						{
							break;
						}
					}
				}
				else										//if this node is already on the open list
				{
					gc = gcost[atNode];
					//VectorSubtract( nodes[newnode].origin, nodes[atNode].origin, vec );
					//gc += VectorLength( vec );				//calculate what the gcost would be if we reached this node along the current path
					if (nodes[atNode].links[i].cost)
					{// UQ1: Already have a cost value, skip the calculations!
						gc += nodes[atNode].links[i].cost;
					}
					else
					{
						VectorSubtract( nodes[newnode].origin, nodes[atNode].origin, vec );
						gc += VectorLength( vec );				//calculate what the gcost would be if we reached this node along the current path
						nodes[atNode].links[i].cost = gc;
					}

					if ( gc < gcost[newnode] )				//if the new gcost is less (ie, this path is shorter than what we had before)
					{
						parent[newnode] = atNode;			//set the new parent for this node
						gcost[newnode] = gc;				//and the new g cost

						for ( j = 1; j < numOpen; j++ )		//loop through all the items on the open list
						{
							if ( openlist[j] == newnode )	//find this node in the list
							{

								//calculate the new fcost and store it
								fcost[newnode] = GetFCost( bot, to, newnode, parent[newnode], gcost );

								//reorder the list again, with the lowest fcost item on top
								m = j;

								while ( m != 1 )
								{
									if ( fcost[openlist[m]] < fcost[openlist[m / 2]] )	//if the item has a lower fcost than it's parent
									{
										temp = openlist[m / 2];
										openlist[m / 2] = openlist[m];
										openlist[m] = temp;								//swap them
										m /= 2;
									}
									else
									{
										break;
									}
								}
								break;													//exit the 'for' loop because we already changed this node
							}															//if
						}																//for
					}											//if (gc < gcost[newnode])
				}												//if (list[newnode] != 1) --> else
			}													//for (loop through links)
		}														//if (numOpen != 0)
		else
		{
			found = qfalse;										//there is no path between these nodes
			break;
		}

		if ( list[to] == 1 )									//if the destination node is on the open list, we're done
		{
			found = qtrue;
			break;
		}
	}															//while (1)

	if ( found == qtrue )							//if we found a path, and are trying to store the pathlist...
	{
		count = 0;
		temp = to;												//start at the end point
		while ( temp != from )									//travel along the path (backwards) until we reach the starting point
		{
			if (count+1 >= MAX_PATHLIST_NODES)
				return -1; // UQ1: Added to stop crash if path is too long for the memory allocation...

			pathlist[count++] = temp;							//add the node to the pathlist and increment the count
			temp = parent[temp];								//move to the parent of this node to continue the path
		}

		pathlist[count++] = from;								//add the beginning node to the end of the pathlist
		return ( count );
	}

	return ( count );											//return the number of nodes in the path, -1 if not found
}

qboolean wp_optimize_memory_initialized = qfalse;

void AIMod_AutoWaypoint_Optimize_Init_Memory ( void )
{
	//if (wp_memory_initialized == qfalse)
	{
		optimized_nodes = (node_t *)malloc( (sizeof(node_t)+1)*MAX_NODES );
		wp_optimize_memory_initialized = qtrue;
	}
}

void AIMod_AutoWaypoint_Optimize_Free_Memory ( void )
{
	//if (wp_memory_initialized == qtrue)
	{
		free(optimized_nodes);
		wp_optimize_memory_initialized = qfalse;
	}
}

/* */
qboolean
Optimize_AddNode ( vec3_t origin, int fl, short int *ents, int objFl )
{
	if ( optimized_number_of_nodes + 1 > MAX_NODES )
	{
		return ( qfalse );
	}

	VectorCopy( origin, optimized_nodes[optimized_number_of_nodes].origin );	//set the node's position

	nodes[optimized_number_of_nodes].type = fl;						//set the flags (NODE_OBJECTIVE, for example)
	nodes[optimized_number_of_nodes].objectNum[0] = ents[0];			//set any map objects associated with this node
	nodes[optimized_number_of_nodes].objectNum[1] = ents[1];			//only applies to objects linked to the unreachable flag
	nodes[optimized_number_of_nodes].objectNum[2] = ents[2];
	nodes[optimized_number_of_nodes].objFlags = objFl;				//set the objective flags
	optimized_number_of_nodes++;
	return ( qtrue );
}

qboolean Is_Waypoint_Entity ( int eType )
{
	switch (eType)
	{
	//case ET_GENERAL:
	//case ET_PLAYER:
	case ET_ITEM:
	//case ET_MISSILE:
	//case ET_SPECIAL:				// rww - force fields
	//case ET_MOVER:
	//case ET_BEAM:
	case ET_PORTAL:
	//case ET_SPEAKER:
	case ET_PUSH_TRIGGER:
	case ET_TELEPORT_TRIGGER:
	//case ET_INVISIBLE:
	case ET_NPC:					// ghoul2 player-like entity
	case ET_TEAM:
	//case ET_BODY:
	//case ET_TERRAIN:
	//case ET_FX:
#ifdef __DOMINANCE__
	case ET_FLAG:
#endif //__DOMINANCE__
		return qtrue;
		break;
	default:
		break;
	}

	return qfalse;
}

//#define DEFAULT_AREA_SEPERATION 512
#define DEFAULT_AREA_SEPERATION 340
#define MAP_BOUNDS_OFFSET 2048

int ClosestNodeTo(vec3_t origin, qboolean isEntity)
{
	int		i;
	float	AREA_SEPERATION = DEFAULT_AREA_SEPERATION;
	float	closest_dist = AREA_SEPERATION*1.5;
	int		closest_node = -1;

	for (i = 0; i < number_of_nodes; i++)
	{
		float dist = Distance(nodes[i].origin, origin);

		if (dist >= closest_dist)
			continue;

		if (nodes[i].objectNum[0] == 1)
			continue; // Skip water/ice disabled node!

		if (isEntity)
		{
			if (HeightDistance(origin, nodes[i].origin) > 64)
				continue;
		}

		closest_dist = dist;
		closest_node = i;
	}

	return closest_node;
}

extern void AIMOD_SaveMapCoordFile ( void );
extern qboolean AIMOD_LoadMapCoordFile ( void );
extern void AIMod_GetMapBounts ( void );
extern qboolean CG_SpawnVector2D( const char *key, const char *defaultString, float *out );

qboolean bad_surface = qfalse;
int		aw_num_bad_surfaces = 0;

/* */
int
AI_PM_GroundTrace ( vec3_t point, int clientNum )
{
	vec3_t	playerMins = {-18, -18, -24};
	vec3_t	playerMaxs = {18, 18, 48};
	trace_t trace;
	vec3_t	point2;
	VectorCopy( point, point2 );
	point2[2] -= 128.0f;

	bad_surface = qfalse;

	CG_Trace( &trace, point, playerMins, playerMaxs, point2, clientNum, (MASK_PLAYERSOLID | MASK_WATER) & ~CONTENTS_BODY/*MASK_SHOT*/ );

	//if ( (trace.surfaceFlags & SURF_NODRAW) && (trace.surfaceFlags & SURF_NOMARKS) && !HasPortalFlags(trace.surfaceFlags, trace.contents) )
	//	bad_surface = qtrue;

	/* TESTING THIS */
	if ( (trace.surfaceFlags & SURF_NODRAW) && (trace.surfaceFlags & SURF_NOMARKS) && !((trace.contents & CONTENTS_PLAYERCLIP) && (trace.contents & CONTENTS_TRANSLUCENT)) )
		bad_surface = qtrue;

	if ( (trace.surfaceFlags & SURF_SKY) )
		bad_surface = qtrue;

	return (trace.contents);
}

void AIMOD_AI_InitNodeContentsFlags ( void )
{
	int i;

	for (i = 0; i < number_of_nodes; i++)
	{
		int contents = 0;
		vec3_t up_pos;

		VectorCopy(nodes[i].origin, up_pos);
		up_pos[2]+= 32;

		contents = AI_PM_GroundTrace(nodes[i].origin, -1);

		if (contents & CONTENTS_LAVA)
		{// Not ICE, but still avoid if possible!
			nodes[i].type |= NODE_ICE;
		}
			
		if (contents & CONTENTS_SLIME)
		{
			nodes[i].type |= NODE_ICE;
		}
			
		if (contents & CONTENTS_WATER)
		{
			nodes[i].type |= NODE_WATER;
		}

		if (bad_surface)
		{// Made code also check for sky surfaces, should be able to remove a lot of crappy waypoints on the skybox!
			nodes[i].objectNum[0] = 1;
			aw_num_bad_surfaces++;
		}
	}
}

#define MAX_ROUTE_FILE_LENGTH (1024 * 1024 * 1.6) // 1.6mb

int FileLength(FILE *fp)
{
	int pos;
	int end;

	pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	end = ftell(fp);
	fseek(fp, pos, SEEK_SET);

	return end;
} //end of the function FileLength
/*
}
*/
/* */
void
AIMod_AutoWaypoint_Optimizer ( void )
{
	qboolean quiet = qfalse;
	qboolean null_links_only = qfalse;
	int i = 0, j = 0, k = 0, l = 0;//, m = 0;
	int	total_calculations = 0;
	int	calculations_complete = 0;
	int	*areas;//[16550];
	int num_areas = 0;
	float map_size;
	vec3_t mapMins, mapMaxs;
	float temp;
	float AREA_SEPERATION = DEFAULT_AREA_SEPERATION;
	int screen_update_timer = 0;
	int entities_start = 0;
	char	str[MAX_TOKEN_CHARS];
	int	node_disable_ticker = 0;
	int	num_disabled_nodes = 0;
	int num_nolink_nodes = 0;
	int	node_disable_ratio = 2;
	qboolean	bad_surfaces_only = qfalse;
	qboolean	noiceremove = qfalse;
	qboolean	nowaterremove = qfalse;
//	vmCvar_t	mapname;

	trap->Cvar_Set("jkg_waypoint_render", "0");

	aw_num_bad_surfaces = 0;

	// UQ1: start - First handle the command line options...
	trap->Cmd_Argv( 1, str, sizeof(str) );

	if (!quiet && str && str[0])
	{// Use player specified area seperation...
		float area_sep = 0.0f;

		if (!Q_stricmp(str, "help") || !Q_stricmp(str, "commands"))
		{
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^7NOTE: The following command line options are available...\n");
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3badsurfsonly  ^4- ^5Remove waypoints on bad surfaces only (includes sky, ice, water).\n");
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3nulllinksonly ^4- ^5Remove waypoints with no links to nearby waypoints (can be used with badsurfsonly).\n");
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3noiceremove   ^4- ^5Do not automaticly remove some waypoints on ice (can be used with badsurfsonly).\n");
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3nowaterremove ^4- ^5Do not automaticly remove some waypoints on/in water (can be used with badsurfsonly).\n");
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3#             ^4- ^5Use specified area scatter distance (default is ^7%i^5).\n", (int)DEFAULT_AREA_SEPERATION);
			return;
		}

		if (!Q_stricmp(str, "badsurfsonly"))
			bad_surfaces_only = qtrue;
		else if (!Q_stricmp(str, "noiceremove"))
			noiceremove = qtrue;
		else if (!Q_stricmp(str, "nowaterremove"))
			nowaterremove = qtrue;
		else if (!Q_stricmp(str, "nulllinksonly"))
			null_links_only = qtrue;
		else
			area_sep = atof(str);

		trap->Cmd_Argv( 2, str, sizeof(str) );

		if (str && str[0])
		{// Use player specified area seperation...
			if (!Q_stricmp(str, "badsurfsonly"))
				bad_surfaces_only = qtrue;
			else if (!Q_stricmp(str, "noiceremove"))
				noiceremove = qtrue;
			else if (!Q_stricmp(str, "nowaterremove"))
				nowaterremove = qtrue;
			else if (!Q_stricmp(str, "nulllinksonly"))
				null_links_only = qtrue;
			else
				area_sep = atof(str);

			trap->Cmd_Argv( 3, str, sizeof(str) );

			if (str && str[0])
			{// Use player specified area seperation...
				if (!Q_stricmp(str, "badsurfsonly"))
					bad_surfaces_only = qtrue;
				else if (!Q_stricmp(str, "noiceremove"))
					noiceremove = qtrue;
				else if (!Q_stricmp(str, "nowaterremove"))
					nowaterremove = qtrue;
				else if (!Q_stricmp(str, "nulllinksonly"))
					null_links_only = qtrue;
				else
					area_sep = atof(str);

				trap->Cmd_Argv( 4, str, sizeof(str) );

				if (str && str[0])
				{// Use player specified area seperation...
					if (!Q_stricmp(str, "badsurfsonly"))
						bad_surfaces_only = qtrue;
					else if (!Q_stricmp(str, "noiceremove"))
						noiceremove = qtrue;
					else if (!Q_stricmp(str, "nowaterremove"))
						nowaterremove = qtrue;
					else if (!Q_stricmp(str, "nulllinksonly"))
						null_links_only = qtrue;
					else
						area_sep = atof(str);

					trap->Cmd_Argv( 5, str, sizeof(str) );

					if (str && str[0])
					{// Use player specified area seperation...
						if (!Q_stricmp(str, "badsurfsonly"))
							bad_surfaces_only = qtrue;
						else if (!Q_stricmp(str, "noiceremove"))
							noiceremove = qtrue;
						else if (!Q_stricmp(str, "nowaterremove"))
							nowaterremove = qtrue;
						else if (!Q_stricmp(str, "nulllinksonly"))
							null_links_only = qtrue;
						else
							area_sep = atof(str);
					}
				}
			}
		}

		if (area_sep != 0)
			AREA_SEPERATION = area_sep;
	}
	else if (!quiet)
	{
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^7NOTE: The following command line options are available...\n");
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3badsurfsonly  ^4- ^5Remove waypoints on bad surfaces only (includes sky, ice, water).\n");
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3nulllinksonly ^4- ^5Remove waypoints with no links to nearby waypoints (can be used with badsurfsonly).\n");
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3noiceremove   ^4- ^5Do not automaticly remove some waypoints on ice (can be used with badsurfsonly).\n");
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3nowaterremove ^4- ^5Do not automaticly remove some waypoints on/in water (can be used with badsurfsonly).\n");
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^3#             ^4- ^5Use specified area scatter distance (default is ^7%i^5).\n", (int)DEFAULT_AREA_SEPERATION);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^7No command line specified, using defaults...\n");
	}
	// UQ1: end - First handle the command line options...

	AIMod_AutoWaypoint_Init_Memory();
	AIMod_AutoWaypoint_Optimize_Init_Memory();

	if (number_of_nodes > 0)
	{// UQ1: Init nodes list!
		number_of_nodes = 0; 
		optimized_number_of_nodes = 0;
		
		memset( nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
		memset( optimized_nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
	}

	AIMOD_NODES_LoadNodes();

	if (number_of_nodes <= 0)
	{
		AIMod_AutoWaypoint_Free_Memory();
		AIMod_AutoWaypoint_Optimize_Free_Memory();
		return;
	}

	areas = (int *)malloc( (sizeof(int)+1)*512000 );

	//AIMod_GetMapBounts( mapMins, mapMaxs );
	AIMod_GetMapBounts();
	VectorCopy(cg.mapcoordsMins, mapMins);
	VectorCopy(cg.mapcoordsMaxs, mapMaxs);

	if (mapMaxs[0] < mapMins[0])
	{
		temp = mapMins[0];
		mapMins[0] = mapMaxs[0];
		mapMaxs[0] = temp;
	}

	if (mapMaxs[1] < mapMins[1])
	{
		temp = mapMins[1];
		mapMins[1] = mapMaxs[1];
		mapMaxs[1] = temp;
	}

	if (mapMaxs[2] < mapMins[2])
	{
		temp = mapMins[2];
		mapMins[2] = mapMaxs[2];
		mapMaxs[2] = temp;
	}

	map_size = Distance(mapMins, mapMaxs);

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Map bounds (^7%f %f %f ^5by ^7%f %f %f^5).\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2] );

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Optimizing waypoint list...\n" );
	strcpy( task_string1, va("^7Optimizing waypoint list....") );
	trap->UpdateScreen();

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7This should not take too long...\n" );
	strcpy( task_string2, va("^7This should not take too long...") );
	trap->UpdateScreen();

	if (!bad_surfaces_only)
	{
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5First pass. Creating area lists... Please wait...\n" );
		strcpy( task_string3, va("^5First pass. Creating area lists... Please wait...") );
		trap->UpdateScreen();
	}

	// UQ1: Set node types..
	AIMOD_AI_InitNodeContentsFlags();

	/*if (number_of_nodes > 128000)
		node_disable_ratio = 24;
	else if (number_of_nodes > 96000)
		node_disable_ratio = 22;
	else if (number_of_nodes > 72000)
		node_disable_ratio = 20;
	else if (number_of_nodes > 64000)
		node_disable_ratio = 18;
	else if (number_of_nodes > 48000)
		node_disable_ratio = 16;
	else*/ if (number_of_nodes > 32000)
		node_disable_ratio = 8;
	else if (number_of_nodes > 24000)
		node_disable_ratio = 6;
	else if (number_of_nodes > 16000)
		node_disable_ratio = 4;
	else if (number_of_nodes > 8000)
		node_disable_ratio = 2;

	// Disable some ice/water ndoes...
	for (i = 0; i < number_of_nodes; i++)
	{
		if (!nowaterremove && (nodes[i].type & NODE_WATER))
		{
			node_disable_ticker++;

			if (node_disable_ticker < node_disable_ratio)
			{// Remove 2 out of every (node_disable_ratio)..
				nodes[i].objectNum[0] = 1;
				num_disabled_nodes++;
			}
			else
			{
				node_disable_ticker = 0;
			}
		}
		
		if (!noiceremove && (nodes[i].type & NODE_ICE))
		{
			node_disable_ticker++;

			if (node_disable_ticker < node_disable_ratio)
			{// Remove 2 out of every (node_disable_ratio)..
				nodes[i].objectNum[0] = 1;
				num_disabled_nodes++;
			}
			else
			{
				node_disable_ticker = 0;
			}
		}
		
		if (nodes[i].enodenum <= 0)
		{// Remove all waypoints without any links...
			nodes[i].objectNum[0] = 1;
			num_nolink_nodes++;
		}
	}

	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 water/ice waypoints.\n", num_disabled_nodes);
	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 bad surfaces.\n", aw_num_bad_surfaces);
	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints without links.\n", num_nolink_nodes);

	if ((null_links_only || bad_surfaces_only) && num_disabled_nodes == 0 && aw_num_bad_surfaces == 0 && num_nolink_nodes == 0)
	{// No point relinking and saving...
		free(areas);
		AIMod_AutoWaypoint_Free_Memory();
		AIMod_AutoWaypoint_Optimize_Free_Memory();
		return;
	}

	if (!bad_surfaces_only && !null_links_only)
	{
		i = mapMins[0]-MAP_BOUNDS_OFFSET;
		j = mapMins[1]-MAP_BOUNDS_OFFSET;
		k = mapMins[2]-MAP_BOUNDS_OFFSET;

		while (i < mapMaxs[0]+MAP_BOUNDS_OFFSET)
		{
			while (j < mapMaxs[1]+MAP_BOUNDS_OFFSET)
			{
				while (k < mapMaxs[2]+MAP_BOUNDS_OFFSET)
				{
					total_calculations++;

					k += AREA_SEPERATION;
				}

				j += AREA_SEPERATION/**1.5*/;
				k = mapMins[2]-MAP_BOUNDS_OFFSET;
			}

			i += AREA_SEPERATION/**1.5*/;
			j = mapMins[1]-MAP_BOUNDS_OFFSET;
		}

		//trap->Print("Total calcs is %i\n", total_calculations);

		i = mapMins[0]-MAP_BOUNDS_OFFSET;
		j = mapMins[1]-MAP_BOUNDS_OFFSET;
		k = mapMins[2]-MAP_BOUNDS_OFFSET;
	
		calculations_complete = 0;

		while (i < mapMaxs[0]+MAP_BOUNDS_OFFSET)
		{
			int num_nodes_added = 0;

			while (j < mapMaxs[1]+MAP_BOUNDS_OFFSET)
			{
				while (k < mapMaxs[2]+MAP_BOUNDS_OFFSET)
				{
					vec3_t		area_org;
					int			closest_node = -1;
					qboolean	skip = qfalse;

					calculations_complete++;

					// Draw a nice little progress bar ;)
					aw_percent_complete = (float)((float)((float)(calculations_complete)/(float)(total_calculations))*100.0f);

					num_nodes_added++;
				
					if (num_nodes_added > 20)
					{
						trap->UpdateScreen();
						num_nodes_added = 0;
					}

					area_org[0] = i;
					area_org[1] = j;
					area_org[2] = k;

					closest_node = ClosestNodeTo(area_org, qfalse);

					if (closest_node == -1)
					{
						//trap->Print("No closest node at %f %f %f.\n", area_org[0], area_org[1], area_org[2]);
						k += AREA_SEPERATION;
						continue;
					}

					for (l = 0; l < num_areas; l++)
					{
						if (areas[l] == closest_node)
						{
							//trap->Print("area %i skipped\n", l);
							skip = qtrue;
							break;
						}
					}

					if (!skip)
					{
						strcpy( last_node_added_string, va("^5Adding area ^3%i ^5at ^7%f %f %f^5.", num_areas, nodes[closest_node].origin[0], nodes[closest_node].origin[1], nodes[closest_node].origin[2]) );
						areas[num_areas] = closest_node;
						num_areas++;
					}

					k += AREA_SEPERATION;
				}

				j += AREA_SEPERATION/**1.5*/;
				k = mapMins[2]-MAP_BOUNDS_OFFSET;
			}

			i += AREA_SEPERATION/**1.5*/;
			j = mapMins[1]-MAP_BOUNDS_OFFSET;
		}

		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Second pass. Adding goal area entities... Please wait...\n" );
		strcpy( task_string3, va("^5Second pass. Adding goal area entities... Please wait...") );
		trap->UpdateScreen();

		entities_start = num_areas;

		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			centity_t	*cent = &cg_entities[i];
			int				closest_node = -1;
			qboolean	skip = qfalse;

			// Draw a nice little progress bar ;)
			aw_percent_complete = (float)((float)((float)(i-MAX_CLIENTS)/(float)(MAX_GENTITIES-MAX_CLIENTS))*100.0f);

			screen_update_timer++;
				
			if (screen_update_timer > 10)
			{
				trap->UpdateScreen();
				screen_update_timer = 0;
			}

			if (!cent)
				continue;

			if (!Is_Waypoint_Entity(cent->currentState.eType))
				continue;

			closest_node = ClosestNodeTo(cent->currentState.origin, qtrue);

			if (closest_node == -1)
			{
				continue;
			}

			for (l = 0; l < num_areas; l++)
			{
				if (areas[l] == closest_node)
				{
					skip = qtrue;
					break;
				}
			}

			if (!skip)
			{
				strcpy( last_node_added_string, va("^5Adding (entity) area ^3%i ^5at ^7%f %f %f^5.", num_areas, nodes[closest_node].origin[0], nodes[closest_node].origin[1], nodes[closest_node].origin[2]) );
				areas[num_areas] = closest_node;
				num_areas++;
			}
		}

		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Generated ^3%i^5 areas.\n", num_areas);

		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Final pass. Creating waypoints... Please wait...\n" );
		strcpy( task_string3, va("^5Final pass. Creating waypoints... Please wait...") );
		trap->UpdateScreen();

		total_calculations = num_areas*num_areas;
		calculations_complete = 0;

		for (i = 0; i < num_areas; i++)
		{
			vec3_t		org;
			int				num_nodes_added = 0;

			VectorCopy(nodes[areas[i]].origin, org);
		
			for (j = 0; j < num_areas; j++)
			{
				vec3_t		org2;
				int				pathlist[MAX_NODES];
				int				pathsize = 0;
				int				node = -1;

				calculations_complete++;

				// Draw a nice little progress bar ;)
				aw_percent_complete = (float)((float)((float)(calculations_complete)/(float)(total_calculations))*100.0f);

				num_nodes_added++;
				
				if (num_nodes_added > 50)
				{
					trap->UpdateScreen();
					num_nodes_added = 0;
				}

				if (i == j)
					continue;

				VectorCopy(nodes[areas[j]].origin, org2);

				if (i >= entities_start || j >= entities_start)
				{// Extra checking for entities!
					if (Distance(org, org2) > AREA_SEPERATION*8/*6*//*2*//*8*/)
						continue;
				}
				else
				{
					if (Distance(org, org2) > AREA_SEPERATION*1.6/*2*//*8*/)
						continue;
				}

				pathsize = CreatePathAStar( NULL, areas[i], areas[j], pathlist );
						
				if (pathsize > 0)
				{
					node = areas[i];	//pathlist is in reverse order

					while (node != -1 && pathsize > 0)
					{
						short int		objNum[3] = { 0, 0, 0 };
						qboolean	skip = qfalse;

						if (!skip && nodes[node].objEntity != 1)
						{
							strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", optimized_number_of_nodes, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]) );
							num_nodes_added++;

							if (num_nodes_added > 100)
							{
								trap->UpdateScreen();
								num_nodes_added = 0;
							}

							Optimize_AddNode( nodes[node].origin, 0, objNum, 0 );	//add the node
							nodes[node].objEntity = 1;
						}

						node = pathlist[pathsize - 1];	//pathlist is in reverse order
						pathsize--;
					}
				}
			}
		}
	}
	else
	{// bad_surfaces_only
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Final pass. Creating waypoints... Please wait...\n" );
		strcpy( task_string3, va("^5Creating waypoints... Please wait...") );
		trap->UpdateScreen();

		total_calculations = number_of_nodes;
		calculations_complete = 0;

		for (i = 0; i < number_of_nodes; i++)
		{
			int				num_nodes_added = 0;
			short int		objNum[3] = { 0, 0, 0 };

			if (nodes[i].objEntity != 1 && nodes[i].objectNum[0] != 1)
			{
				strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", optimized_number_of_nodes, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]) );
				num_nodes_added++;

				if (num_nodes_added > 100)
				{
					trap->UpdateScreen();
					num_nodes_added = 0;
				}

				Optimize_AddNode( nodes[i].origin, 0, objNum, 0 );	//add the node
				nodes[i].objEntity = 1;
			}
		}

		aw_percent_complete = 0.0f;
		trap->UpdateScreen();
	}

	free(areas);

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint list reduced from ^3%i^5 waypoints to ^3%i^5 waypoints.\n", number_of_nodes, optimized_number_of_nodes );

	// Work out the best scatter distance to use for this map size...
	if (map_size > 96000)
	{
		waypoint_scatter_distance *= 5;
	}
	else if (map_size > 32768)
	{
		waypoint_scatter_distance *= 4;
	}
	else if (map_size > 24000)
	{
		waypoint_scatter_distance *= 3;
	}
	else if (map_size > 20000)
	{
		waypoint_scatter_distance *= 2;
	}
	else if (map_size > 16550)
	{
		waypoint_scatter_distance *= 1.5;
	}
	else if (map_size > 8192)
	{
		waypoint_scatter_distance *= 1.12;
	}

	number_of_nodes = 0;

	// Copy over the new list!
	memcpy(nodes, optimized_nodes, (sizeof(node_t)*MAX_NODES)+1);
	number_of_nodes = optimized_number_of_nodes;

	/*for (i = 0; i < optimized_number_of_nodes; i++)
	{
		VectorCopy(optimized_nodes[i].origin, nodes[number_of_nodes].origin);
		optimized_nodes[i].enodenum = 0;
		number_of_nodes++;
	}*/

	aw_num_nodes = number_of_nodes;

	// Save the new list...
	AIMOD_NODES_SaveNodes_Autowaypointed();

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	// Remake cover spots...
	AIMOD_Generate_Cover_Spots(); // UQ1: Want to add these to JKA???

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	AIMod_AutoWaypoint_Free_Memory();
	AIMod_AutoWaypoint_Optimize_Free_Memory();

	//trap->SendConsoleCommand( "set bot_wp_visconnect 1\n" );
	//trap->SendConsoleCommand( "bot_wp_convert_awp\n" );

	//trap->Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	trap->SendConsoleCommand( "!loadAWPnodes\n" );
}

extern int Q_TrueRand ( int low, int high );

qboolean AIMod_AutoWaypoint_Cleaner_NodeHasLinkWithFewLinks ( int node )
{// Check the links of the current node for any nodes with very few links (so we dont remove an important node for reachability)
	int i = 0;
	int removed_neighbours_count = 0;
	
	for (i = 0; i < nodes[node].enodenum; i++)
	{
		if (nodes[node].enodenum > 64)
			continue; // Corrupt?

		if (nodes[nodes[node].links[i].targetNode].objectNum[0] == 1)
			removed_neighbours_count++;
	}

	if (nodes[node].enodenum - removed_neighbours_count <= 8/*4*/)
		return qtrue; // lready removed too many neighbours here! Remove no more!

	for (i = 0; i < nodes[node].enodenum; i++)
	{
		if (nodes[node].enodenum > 64)
			continue; // Corrupt?

		if (nodes[nodes[node].links[i].targetNode].enodenum <= 8/*4*/)
			return qtrue;
	}

	return qfalse;
}

/* */
void
AIMod_AutoWaypoint_Cleaner_OLD ( qboolean quiet, qboolean null_links_only, qboolean relink_only )
{
	int i = 0;//, j = 0;//, k = 0, l = 0;//, m = 0;
	int	total_calculations = 0;
	int	calculations_complete = 0;
	int	*areas;//[16550];
//	int num_areas = 0;
	float map_size;
	vec3_t mapMins, mapMaxs;
	float temp;
//	float AREA_SEPERATION = DEFAULT_AREA_SEPERATION;
//	int screen_update_timer = 0;
//	int entities_start = 0;
	int	node_disable_ticker = 0;
	int	num_disabled_nodes = 0;
	int num_nolink_nodes = 0;
	int num_trigger_hurt_nodes = 0;
	int num_skiped_nodes = 0;
	int	node_disable_ratio = 2;
//	qboolean	bad_surfaces_only = qfalse;
	qboolean	noiceremove = qfalse;
	qboolean	nowaterremove = qfalse;
	int			skip = 0;
//	int			skip_threshold = 2;
	qboolean	reducecount = qtrue;
	int			start_wp_total = 0;

	aw_num_bad_surfaces = 0;

	trap->Cvar_Set("jkg_waypoint_render", "0");

	AIMod_AutoWaypoint_Init_Memory();
	AIMod_AutoWaypoint_Optimize_Init_Memory();

	if (number_of_nodes > 0)
	{// UQ1: Init nodes list!
		number_of_nodes = 0; 
		optimized_number_of_nodes = 0;
		
		memset( nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
		memset( optimized_nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
	}

	AIMOD_NODES_LoadNodes();

	if (number_of_nodes <= 0)
	{
		AIMod_AutoWaypoint_Free_Memory();
		AIMod_AutoWaypoint_Optimize_Free_Memory();
		return;
	}

	start_wp_total = number_of_nodes;

	areas = ( int * )malloc( (sizeof(int)+1)*512000 );

	//AIMod_GetMapBounts( mapMins, mapMaxs );
	AIMod_GetMapBounts();
	VectorCopy(cg.mapcoordsMins, mapMins);
	VectorCopy(cg.mapcoordsMaxs, mapMaxs);

	if (mapMaxs[0] < mapMins[0])
	{
		temp = mapMins[0];
		mapMins[0] = mapMaxs[0];
		mapMaxs[0] = temp;
	}

	if (mapMaxs[1] < mapMins[1])
	{
		temp = mapMins[1];
		mapMins[1] = mapMaxs[1];
		mapMaxs[1] = temp;
	}

	if (mapMaxs[2] < mapMins[2])
	{
		temp = mapMins[2];
		mapMins[2] = mapMaxs[2];
		mapMaxs[2] = temp;
	}

	map_size = Distance(mapMins, mapMaxs);

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Map bounds (^7%f %f %f ^5by ^7%f %f %f^5).\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2] );

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Cleaning waypoint list...\n" );
	strcpy( task_string1, va("^7Cleaning waypoint list....") );
	trap->UpdateScreen();

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7This should not take too long...\n" );
	strcpy( task_string2, va("^7This should not take too long...") );
	trap->UpdateScreen();

	// UQ1: Set node types..
	AIMOD_AI_InitNodeContentsFlags();
//	AIMod_AutoWaypoint_Trigger_Hurt_List_Setup();

	if (number_of_nodes > 32000)
		node_disable_ratio = 8;
	else if (number_of_nodes > 24000)
		node_disable_ratio = 6;
	else if (number_of_nodes > 16000)
		node_disable_ratio = 4;
	else if (number_of_nodes > 8000)
		node_disable_ratio = 2;

/*	if (number_of_nodes > 64000)
		skip_threshold = 4;
	else if (number_of_nodes > 48000)
		skip_threshold = 3;
	else if (number_of_nodes > 32000)
		skip_threshold = 2;
	else if (number_of_nodes > 10000)
		skip_threshold = 2;
	else
	{
		skip_threshold = 0;
		reducecount = qfalse;
	}*/

	// Disable some ice/water ndoes...
	if (!relink_only)
	for (i = 0; i < number_of_nodes; i++)
	{
		if (nodes[i].enodenum <= 0)
		{// Remove all waypoints without any links...
			nodes[i].objectNum[0] = 1;
			num_nolink_nodes++;
			continue;
		}

		if (relink_only)
			continue;

/*		if (nodes[i].enodenum > 2 && LocationIsNearTriggerHurt(nodes[i].origin))
		{// Lots of links, but also seems to be located close to barb wire (or other damage entity). Remove it!
			nodes[i].objectNum[0] = 1;
			num_trigger_hurt_nodes++;
			continue;
		}*/

		if (nodes[i].enodenum <= 8/*4*/ || AIMod_AutoWaypoint_Cleaner_NodeHasLinkWithFewLinks(i))
			continue;

		if (reducecount)
		{
			if (skip == 0)
			{
				skip = 1;
			}
			else
			{
				nodes[i].objectNum[0] = 1;
				num_skiped_nodes++;

				skip = 0;
			}
		}

		if ((!nowaterremove && (nodes[i].type & NODE_WATER)) || (!noiceremove && (nodes[i].type & NODE_ICE)))
		{
			if (node_disable_ticker < node_disable_ratio)
			{// Remove 2 out of every (node_disable_ratio)..
				nodes[i].objectNum[0] = 1;
				num_disabled_nodes++;
				node_disable_ticker++;
			}
			else
			{
				node_disable_ticker = 0;
			}
		}
	}

	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 water/ice waypoints.\n", num_disabled_nodes);
	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 bad surfaces.\n", aw_num_bad_surfaces);
	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 trigger hurt waypoints.\n", num_trigger_hurt_nodes);
	trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints without links.\n", num_nolink_nodes);

	if (reducecount)
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 random waypoints to reduce count.\n", num_skiped_nodes);

	if (num_disabled_nodes == 0 && aw_num_bad_surfaces == 0 && num_nolink_nodes == 0 && num_trigger_hurt_nodes == 0 && num_skiped_nodes == 0 && !relink_only)
	{// No point relinking and saving...
		free(areas);
		AIMod_AutoWaypoint_Free_Memory();
		AIMod_AutoWaypoint_Optimize_Free_Memory();
		return;
	}

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Creating waypoints... Please wait...\n" );
	strcpy( task_string3, va("^5Creating waypoints... Please wait...") );
	trap->UpdateScreen();

	total_calculations = number_of_nodes;
	calculations_complete = 0;

	for (i = 0; i < number_of_nodes; i++)
	{
		int				num_nodes_added = 0;
		short int		objNum[3] = { 0, 0, 0 };

		if (nodes[i].objEntity != 1 && nodes[i].objectNum[0] != 1)
		{
			strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", optimized_number_of_nodes, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]) );
			num_nodes_added++;

			if (num_nodes_added > 100)
			{
				trap->UpdateScreen();
				num_nodes_added = 0;
			}

			Optimize_AddNode( nodes[i].origin, 0, objNum, 0 );	//add the node
			nodes[i].objEntity = 1;
		}
	}

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	free(areas);

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint list reduced from ^3%i^5 waypoints to ^3%i^5 waypoints.\n", number_of_nodes, optimized_number_of_nodes );

	// Work out the best scatter distance to use for this map size...
	/*if (map_size > 96000)
	{
		waypoint_scatter_distance *= 5;
	}
	else if (map_size > 32768)
	{
		waypoint_scatter_distance *= 4;
	}
	else if (map_size > 24000)
	{
		waypoint_scatter_distance *= 3;
	}
	else if (map_size > 20000)
	{
		waypoint_scatter_distance *= 2;
	}
	else if (map_size > 16550)
	{
		waypoint_scatter_distance *= 1.5;
	}
	else if (map_size > 8192)
	{
		waypoint_scatter_distance *= 1.12;
	}*/
	
	/*if (number_of_nodes < 20000)
	{
		waypoint_distance_multiplier = 1.5f;
	}
	else if (number_of_nodes < 32000)
	{
		waypoint_distance_multiplier = 2.0f;
	}
	else
	{
		waypoint_distance_multiplier = 2.5f;
	}*/

	number_of_nodes = 0;

	// Copy over the new list!
	memcpy(nodes, optimized_nodes, (sizeof(node_t)*MAX_NODES)+1);
	number_of_nodes = optimized_number_of_nodes;

	aw_num_nodes = number_of_nodes;

	// Save the new list...
	AIMOD_NODES_SaveNodes_Autowaypointed();

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	// Remake cover spots...
	if (start_wp_total != number_of_nodes)
		AIMOD_Generate_Cover_Spots();

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	AIMod_AutoWaypoint_Free_Memory();
	AIMod_AutoWaypoint_Optimize_Free_Memory();

	trap->SendConsoleCommand( "!loadAWPnodes\n" );
}

//
//
//
//
//

int			trigger_hurt_counter = 0;
centity_t	*trigger_hurt_list[MAX_GENTITIES];

void AIMod_AutoWaypoint_Trigger_Hurt_List_Setup ( void )
{// Generate a list for the current map to speed up processing...
	/*int i = 0;

	trigger_hurt_counter = 0;

	for (i = 0; i < MAX_GENTITIES; i++)
	{
		centity_t *cent = &cg_entities[i];

		if (cent->currentState.eType == ET_TRIGGER_HURT)
		{
			trigger_hurt_list[trigger_hurt_counter] = cent;
			trigger_hurt_counter++;
		}
	}*/
}

qboolean LocationIsNearTriggerHurt ( vec3_t origin )
{
	/*int i = 0;

	for (i = 0; i < trigger_hurt_counter; i++)
	{
		if (Distance(trigger_hurt_list[i]->currentState.origin, origin) < 64)
			return qtrue;
	}
	*/

	return qfalse;
}

int num_dupe_nodes = 0;

void CheackForNearbyDupeNodes( int node )
{// Removes any nodes with the same links as this node to reduce numbers...
	int i = 0;

	if (nodes[node].objectNum[0] == 1)
		return; // This node is disabled already...

	for (i = 0; i < number_of_nodes; i++)
	{
		int j = 0;
		//int num_same = 0;
		qboolean REMOVE_NODE = qtrue;

		if (i == node)
			continue;

		if (nodes[i].enodenum <= 0)
			continue;

		if (nodes[i].objectNum[0] == 1)
			continue;

		if (Distance(nodes[node].origin, nodes[i].origin) > waypoint_scatter_distance*waypoint_distance_multiplier)
			continue;

		if (nodes[i].enodenum > nodes[node].enodenum)
			continue; // Never remove a better node...

		for (j = 0; j < nodes[node].enodenum; j++)
		{
			int k = 0;
			qboolean found = qfalse;

			for (k = 0; k < nodes[i].enodenum; k++)
			{
				if (nodes[node].links[j].targetNode == nodes[i].links[k].targetNode)
				{
					found = qtrue;
					break;
				}
			}

			if (!found)
			{
				REMOVE_NODE = qfalse;
				break;
			}
		}

		if (REMOVE_NODE)
		{// Node i is a dupe node! Disable it!
			if ((nodes[i].type & NODE_LAND_VEHICLE) && !(nodes[node].type & NODE_LAND_VEHICLE))
			{// Never remove a vehicle node and replace it with a non vehicle one!

			}
			else
			{
				nodes[i].objectNum[0] = 1;
				num_dupe_nodes++;
			}
		}
	}
}

qboolean CheckIfTooManyLinksRemoved ( int node, qboolean extra )
{
	int i = 0;
	int num_found = 0;

	for (i = 0; i < nodes[node].enodenum; i++)
	{
		int target = nodes[node].links[i].targetNode;

		if (target < 0)
			continue;

		if (target > number_of_nodes)
			continue;

		if (nodes[target].objectNum[0] == 1)
			continue;

		num_found++;

		if (extra)
		{
			if (num_found > nodes[node].enodenum*0.3)//4)
				return qfalse;
		}
		else
		{
			if (num_found > nodes[node].enodenum*0.6)//8)
				return qfalse;
		}
	}

	return qtrue;
}

vec3_t	REMOVAL_POINTS[MAX_NODES];
int		NUM_REMOVAL_POINTS = 0;

void AIMod_AddRemovalPoint ( void )
{
	VectorCopy(cg.refdef.vieworg, REMOVAL_POINTS[NUM_REMOVAL_POINTS]);
	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Removal Point added at %f %f %f...\n", REMOVAL_POINTS[NUM_REMOVAL_POINTS][0], REMOVAL_POINTS[NUM_REMOVAL_POINTS][1], REMOVAL_POINTS[NUM_REMOVAL_POINTS][2] );
	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoints within 128 of this location will be removed on awc run...\n" );
	NUM_REMOVAL_POINTS++;
}

void AIMod_AWC_MarkBadHeight ( void )
{// Mark this height on the map as bad for waypoints...
	BAD_HEIGHTS[NUM_BAD_HEIGHTS] = cg.refdef.vieworg[2];
	NUM_BAD_HEIGHTS++;
	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Height %f marked as bad for waypoints.\n", cg.refdef.vieworg[2] );
}

/* */
void
AIMod_AutoWaypoint_Cleaner ( qboolean quiet, qboolean null_links_only, qboolean relink_only, qboolean multipass, qboolean initial_pass, qboolean extra, qboolean marked_locations, qboolean extra_reach )
{
	int i = 0;//, j = 0;//, k = 0, l = 0;//, m = 0;
	int	total_calculations = 0;
	int	calculations_complete = 0;
	//int	*areas;//[16550];
//	int num_areas = 0;
	float map_size;
	vec3_t mapMins, mapMaxs;
	float temp;
//	float AREA_SEPERATION = DEFAULT_AREA_SEPERATION;
//	int screen_update_timer = 0;
//	int entities_start = 0;
//	int	node_disable_ticker = 0;
	int	num_disabled_nodes = 0;
	int num_nolink_nodes = 0;
	int num_trigger_hurt_nodes = 0;
	int num_this_location_nodes = 0;
	int num_marked_height_nodes = 0;
	int num_skiped_nodes = 0;
	int	node_disable_ratio = 2;
	int total_removed = 0;
//	qboolean	bad_surfaces_only = qfalse;
//	qboolean	noiceremove = qfalse;
//	qboolean	nowaterremove = qfalse;
//	int			skip = 0;
//	int			skip_threshold = 2;
//	qboolean	reducecount = qtrue;
	int			start_wp_total = 0;
	int			node_clean_ticker = 0;
	int			num_passes_completed = 0;
	float		original_wp_max_distance = 0;
	float		original_wp_scatter_multiplier = 0;

	trap->Cvar_Set("jkg_waypoint_render", "0");

	aw_num_bad_surfaces = 0;
	num_dupe_nodes = 0;

	AIMod_AutoWaypoint_Init_Memory();
	AIMod_AutoWaypoint_Optimize_Init_Memory();

	if (number_of_nodes > 0)
	{// UQ1: Init nodes list!
		number_of_nodes = 0; 
		optimized_number_of_nodes = 0;
		
		memset( nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
		memset( optimized_nodes, 0, ((sizeof(node_t)+1)*MAX_NODES) );
	}

	AIMOD_NODES_LoadNodes();

	if (number_of_nodes <= 0)
	{
		AIMod_AutoWaypoint_Free_Memory();
		AIMod_AutoWaypoint_Optimize_Free_Memory();
		return;
	}

	start_wp_total = number_of_nodes;

	//areas = malloc( (sizeof(int)+1)*512000 );

	//AIMod_GetMapBounts( mapMins, mapMaxs );
	AIMod_GetMapBounts();

	VectorCopy(cg.mapcoordsMins, mapMins);
	VectorCopy(cg.mapcoordsMaxs, mapMaxs);

	if (mapMaxs[0] < mapMins[0])
	{
		temp = mapMins[0];
		mapMins[0] = mapMaxs[0];
		mapMaxs[0] = temp;
	}

	if (mapMaxs[1] < mapMins[1])
	{
		temp = mapMins[1];
		mapMins[1] = mapMaxs[1];
		mapMaxs[1] = temp;
	}

	if (mapMaxs[2] < mapMins[2])
	{
		temp = mapMins[2];
		mapMins[2] = mapMaxs[2];
		mapMaxs[2] = temp;
	}

	map_size = Distance(mapMins, mapMaxs);

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Map bounds (^7%f %f %f ^5by ^7%f %f %f^5).\n", mapMins[0], mapMins[1], mapMins[2], mapMaxs[0], mapMaxs[1], mapMaxs[2] );

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Cleaning waypoint list...\n" );
	strcpy( task_string1, va("^7Cleaning waypoint list....") );
	trap->UpdateScreen();

	trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^7This could take a while...\n" );
	strcpy( task_string2, va("^7This could take a while...") );
	trap->UpdateScreen();

	// UQ1: Set node types..
	AIMOD_AI_InitNodeContentsFlags();
	AIMod_AutoWaypoint_Trigger_Hurt_List_Setup();

	if (number_of_nodes > 40000)
		node_disable_ratio = 10;
	else if (number_of_nodes > 36000)
		node_disable_ratio = 9;
	else if (number_of_nodes > 32000)
		node_disable_ratio = 8;
	else if (number_of_nodes > 30000)
		node_disable_ratio = 7;
	else if (number_of_nodes > 24000)
		node_disable_ratio = 6;
	else if (number_of_nodes > 22000)
		node_disable_ratio = 5;
	else if (number_of_nodes > 18000)
		node_disable_ratio = 4;
	else if (number_of_nodes > 14000)
		node_disable_ratio = 3;
	else if (number_of_nodes > 8000)
		node_disable_ratio = 2;

/*	if (number_of_nodes > 64000)
		skip_threshold = 4;
	else if (number_of_nodes > 48000)
		skip_threshold = 3;
	else if (number_of_nodes > 32000)
		skip_threshold = 2;
	else if (number_of_nodes > 10000)
		skip_threshold = 2;
	else
	{
		skip_threshold = 0;
		reducecount = qfalse;
	}*/

	// waypoint_scatter_distance*waypoint_distance_multiplier
	// Get original awp's distance multiplier...
	for (i = 0; i < number_of_nodes; i++)
	{
		int j;

		for (j = 0; j < nodes[i].enodenum; j++)
		{
			float dist = Distance(nodes[i].origin, nodes[nodes[i].links[j].targetNode].origin);

			if (dist > original_wp_max_distance)
				original_wp_max_distance = dist;
		}
	}

	if (extra_reach)
	{// 1.5x?
		original_wp_max_distance *= 1.5f;
	}

	// Set out distance multiplier...
	original_wp_scatter_multiplier = waypoint_distance_multiplier;
	waypoint_distance_multiplier = original_wp_max_distance/waypoint_scatter_distance;

	while (1)
	{
		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Cleaning waypoints list... Please wait...\n" );
		strcpy( task_string3, va("^5Cleaning waypoints list... Please wait...") );
		trap->UpdateScreen();

		total_calculations = number_of_nodes;
		calculations_complete = 0;
		node_clean_ticker = 0;
		num_nolink_nodes = 0;
		num_disabled_nodes = 0;
		num_trigger_hurt_nodes = 0;
		num_dupe_nodes = 0;
		aw_num_bad_surfaces = 0;
		num_skiped_nodes = 0;
		num_this_location_nodes = 0;
		num_marked_height_nodes = 0;
		total_removed = 0;
		
		num_passes_completed++;

		// Work out the best scatter distance to use for this map size...
		/*if (map_size > 96000)
		{
			waypoint_scatter_distance *= 1.5;
		}
		else if (map_size > 32768)
		{
			waypoint_scatter_distance *= 1.35;
		}
		else if (map_size > 24000)
		{
			waypoint_scatter_distance *= 1.25;
		}
		else if (map_size > 16550)
		{
			waypoint_scatter_distance *= 1.10;
		}
		else if (map_size > 8192)
		{
			waypoint_scatter_distance *= 1.02;
		}*/
	
		/*if (number_of_nodes < 20000)
		{
			waypoint_distance_multiplier = 1.5f;
		}
		else if (number_of_nodes < 32000)
		{
			waypoint_distance_multiplier = 2.0f;
		}
		else
		{
			waypoint_distance_multiplier = 2.5f;
		}*/

		for (i = 0; i < number_of_nodes; i++)
		{// Initialize...
			nodes[i].objectNum[0] = 0;
			nodes[i].objEntity = 0;
		}

		// Disable some ice/water ndoes...
		if (!relink_only)
		for (i = 0; i < number_of_nodes; i++)
		{
			node_clean_ticker++;
			calculations_complete++;

			/*
			if (number_of_nodes <= 5000 )
			{// No point relinking and saving...
				AIMod_AutoWaypoint_Free_Memory();
				AIMod_AutoWaypoint_Optimize_Free_Memory();
				trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint cleaner exited. The waypoint file has been optimized enough.\n");
				
				// Restore the original multiplier...
				waypoint_distance_multiplier = original_wp_scatter_multiplier;
				return;
			}
			*/

			// Draw a nice little progress bar ;)
			aw_percent_complete = (float)((float)((float)(calculations_complete)/(float)(total_calculations))*100.0f);

			if (node_clean_ticker > 100)
			{
				strcpy( last_node_added_string, va("^5Checking waypoint ^3%i ^5at ^7%f %f %f^5.", i, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]) );
				trap->UpdateScreen();
				node_clean_ticker = 0;
			}

			if (nodes[i].objectNum[0] == 1)
				continue;

			if (nodes[i].enodenum <= 1/*0*/)
			{// Remove all waypoints without any links...
				nodes[i].objectNum[0] = 1;
				num_nolink_nodes++;
				continue;
			}

			if (marked_locations)
			{
				int z = 0;

				if (AIMOD_IsWaypointHeightMarkedAsBad( nodes[i].origin ))
				{
					nodes[i].objectNum[0] = 1;
					num_marked_height_nodes++;
					continue;
				}

				for (z = 0; z < NUM_REMOVAL_POINTS; z++)
				{
					if (Distance(nodes[i].origin, REMOVAL_POINTS[z]) <= 128)
					{
						nodes[i].objectNum[0] = 1;
						num_this_location_nodes++;
					}
				}

				// Skip cleaner...
				continue;
			}

			if (relink_only)
				continue;

			/*
			if (nodes[i].enodenum > 2 && LocationIsNearTriggerHurt(nodes[i].origin))
			{// Lots of links, but also seems to be located close to barb wire (or other damage entity). Remove it!
				nodes[i].objectNum[0] = 1;
				num_trigger_hurt_nodes++;
				continue;
			}
			*/

#ifndef __TEST_CLEANER__
			if (extra)
			{
				if (nodes[i].enodenum >= 12/*16*/)
				{
					/*if ((nodes[i].type & NODE_LAND_VEHICLE) && !CheckIfAnotherVehicleNodeIsNearby(i))
					{// We need to never remove vehicle nodes when there is not enough others around...
					
					}
					else*/
					//if (!CheckIfTooManyLinksRemoved(i, extra))
					{
						nodes[i].objectNum[0] = 1;
						num_skiped_nodes++;
						continue;
					}
				}
			}
			else if (nodes[i].enodenum >= 16/*16*/)
			{
				/*if ((nodes[i].type & NODE_LAND_VEHICLE) && !CheckIfAnotherVehicleNodeIsNearby(i))
				{// We need to never remove vehicle nodes when there is not enough others around...
					
				}
				else*/
				if (!CheckIfTooManyLinksRemoved(i, extra))
				{
					nodes[i].objectNum[0] = 1;
					num_skiped_nodes++;
					continue;
				}
			}

			CheackForNearbyDupeNodes(i);
#else //__TEST_CLEANER__
			if (nodes[i].enodenum >= 1)
			{
				int num_links_alt_reachable = 0;
				int current_link = 0;
				int	reach_nodes[64];
				int	num_reach_nodes = 0;

				for (current_link = 0; current_link < nodes[i].enodenum; current_link++)
				{
					int			j = 0;
					int			current_target_node = nodes[i].links[current_link].targetNode;
					qboolean	return_to_start = qfalse;

					num_reach_nodes = 0;

					for (j = 0; j < nodes[i].enodenum; j++)
					{
						int k = 0;
						int target_node1 = nodes[i].links[j].targetNode;

						if (nodes[target_node1].objectNum[0] == 1)
							continue;

						for (k = 0; k < nodes[target_node1].enodenum; k++)
						{
							int target_node2 = nodes[target_node1].links[k].targetNode;

							if (target_node2 == i)
								continue;

							if (target_node2 == current_target_node)
							{
								num_links_alt_reachable++;
								reach_nodes[num_reach_nodes] = target_node2;
								num_reach_nodes++;
								return_to_start = qtrue;
								break;
							}
						}

						//if (return_to_start)
						//	break;
					}
				}

				if (num_links_alt_reachable >= nodes[i].enodenum*4)
				{
					int k = 0;
					int num_alt_links = 0;

					for (k = 0; k < num_reach_nodes; k++)
					{
						int j = 0;
						int target_node1 = reach_nodes[k];

						for (j = 0; j < nodes[i].enodenum; j++)
						{
							qboolean found = qfalse;
							int l = 0;
							int target_node2 = nodes[i].links[j].targetNode;
						
							for (l = 0; l < nodes[target_node2].enodenum; l++)
							{
								int target_node3 = nodes[target_node2].links[l].targetNode;

								if (target_node3 == target_node1)
								{
									num_alt_links++;
									found = qtrue;
									break;
								}
							}

							//if (found)
							//	break;
						}
					}

					if (num_alt_links >= nodes[i].enodenum*3 && num_alt_links >= num_reach_nodes*4)
					{
						nodes[i].objectNum[0] = 1;
						num_skiped_nodes++;
						continue;
					}
				}

				//CheackForNearbyDupeNodes(i);
			}
#endif //__TEST_CLEANER__

			//if (nodes[i].enodenum <= 8/*4*/ || AIMod_AutoWaypoint_Cleaner_NodeHasLinkWithFewLinks(i))
			//	continue;

			/*if (initial_pass && reducecount)
			{
				if (skip == 0)
				{
					skip = 1;
				}
				else
				{
					nodes[i].objectNum[0] = 1;
					num_skiped_nodes++;

					skip = 0;
				}
			}*/

			/*if ((!nowaterremove && (nodes[i].type & NODE_WATER)) || (!noiceremove && (nodes[i].type & NODE_ICE)))
			{
				if (node_disable_ticker < node_disable_ratio)
				{// Remove 2 out of every (node_disable_ratio)..
					nodes[i].objectNum[0] = 1;
					num_disabled_nodes++;
					node_disable_ticker++;
				}
				else
				{
					node_disable_ticker = 0;
				}
			}*/
		}

		aw_percent_complete = 0.0f;
		trap->UpdateScreen();

		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 water/ice waypoints.\n", num_disabled_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 bad surfaces.\n", aw_num_bad_surfaces);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 trigger hurt waypoints.\n", num_trigger_hurt_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints without links.\n", num_nolink_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints with duplicate links to a neighbor.\n", num_dupe_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints in over-waypointed areas.\n", num_skiped_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints at your marked locations (removal spots).\n", num_this_location_nodes);
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 waypoints at your given bad heights (bad height spots).\n", num_marked_height_nodes);

		total_removed = num_skiped_nodes + num_dupe_nodes + num_disabled_nodes + aw_num_bad_surfaces + num_nolink_nodes + num_trigger_hurt_nodes + num_this_location_nodes + num_marked_height_nodes;

		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Disabled ^3%i^5 total waypoints in this run.\n", total_removed);

		if (total_removed <= 100 && multipass && !relink_only )
		{
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Multipass waypoint cleaner completed in %i passes. Nothing left to remove.\n", num_passes_completed);
			AIMod_AutoWaypoint_Free_Memory();
			AIMod_AutoWaypoint_Optimize_Free_Memory();

			// Restore the original multiplier...
			waypoint_distance_multiplier = original_wp_scatter_multiplier;
			return;
		}
		else if (total_removed == 0 && !relink_only )
		{// No point relinking and saving...
//			free(areas);
			AIMod_AutoWaypoint_Free_Memory();
			AIMod_AutoWaypoint_Optimize_Free_Memory();
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint cleaner completed. Nothing left to remove.\n");

			// Restore the original multiplier...
			waypoint_distance_multiplier = original_wp_scatter_multiplier;
			return;
		}

		if (multipass)
		{
			trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Creating new waypoints list (pass# ^7%i^5)... Please wait...\n", num_passes_completed );
			strcpy( task_string3, va("^5Creating new waypoints list (pass# ^7%i^5)... Please wait...", num_passes_completed) );
		}
		else
		{
			trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Creating new waypoints list... Please wait...\n" );
			strcpy( task_string3, va("^5Creating new waypoints list... Please wait...") );
		}
		trap->UpdateScreen();

		total_calculations = number_of_nodes;
		calculations_complete = 0;

		for (i = 0; i < number_of_nodes; i++)
		{
			int				num_nodes_added = 0;
			short int		objNum[3] = { 0, 0, 0 };

			if (nodes[i].objEntity != 1 && nodes[i].objectNum[0] != 1)
			{
				num_nodes_added++;

				calculations_complete++;

				// Draw a nice little progress bar ;)
				aw_percent_complete = (float)((float)((float)(calculations_complete)/(float)(total_calculations))*100.0f);

				if (num_nodes_added > 100)
				{
					strcpy( last_node_added_string, va("^5Adding waypoint ^3%i ^5at ^7%f %f %f^5.", optimized_number_of_nodes, nodes[i].origin[0], nodes[i].origin[1], nodes[i].origin[2]) );
					trap->UpdateScreen();
					num_nodes_added = 0;
				}

				Optimize_AddNode( nodes[i].origin, 0, objNum, 0 );	//add the node
				nodes[i].objEntity = 1;
			}
		}

		calculations_complete = 0;
		aw_percent_complete = 0.0f;
		trap->UpdateScreen();

//		free(areas);

		trap->Print( "^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint list reduced from ^3%i^5 waypoints to ^3%i^5 waypoints.\n", number_of_nodes, optimized_number_of_nodes );

		number_of_nodes = 0;

		// Copy over the new list!
		memcpy(nodes, optimized_nodes, (sizeof(node_t)*MAX_NODES)+1);
		number_of_nodes = optimized_number_of_nodes;
		optimized_number_of_nodes = 0;

		aw_num_nodes = number_of_nodes;

		// Save the new list...
		AIMOD_NODES_SaveNodes_Autowaypointed();

		aw_percent_complete = 0.0f;
		trap->UpdateScreen();
	
		trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Multipass waypoint cleaner pass #%i completed. Beginning next pass.\n", num_passes_completed);
		
		initial_pass = qfalse;

		if (!multipass)
		{// No point relinking and saving...
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Waypoint cleaner completed.\n");
			break;
		}

		if (!extra && number_of_nodes > 10000 && total_removed < 1000)
		{// Too many, we need an "extra" pass...
			extra = qtrue;
			trap->Print("^4*** ^3AUTO-WAYPOINTER^4: ^5Not enough removed in this pass, forcing next pass to be an \"extra\" pass.\n");
		}
		else
		{
			extra = qfalse;
		}

		calculations_complete = 0;
		node_clean_ticker = 0;
		num_nolink_nodes = 0;
		num_disabled_nodes = 0;
		num_trigger_hurt_nodes = 0;
		num_dupe_nodes = 0;
		aw_num_bad_surfaces = 0;
		num_skiped_nodes = 0;
		num_this_location_nodes = 0;
		num_marked_height_nodes = 0;
		total_removed = 0;
	}

	// Remake cover spots...
	if (start_wp_total != number_of_nodes)
		AIMOD_Generate_Cover_Spots();

	aw_percent_complete = 0.0f;
	trap->UpdateScreen();

	AIMod_AutoWaypoint_Free_Memory();
	AIMod_AutoWaypoint_Optimize_Free_Memory();

	// Restore the original multiplier...
	waypoint_distance_multiplier = original_wp_scatter_multiplier;

	trap->SendConsoleCommand( "!loadnodes\n" );
}

#define NUMBER_SIZE		8

void CG_Waypoint( int wp_num ) {
	refEntity_t		re;
	vec3_t			angles, vec, dir, up;
	int				i, numdigits, digits[10], temp_num;

	memset( &re, 0, sizeof( re ) );

	VectorCopy(nodes[wp_num].origin, re.origin);
	re.origin[2]+=16;

	re.reType = RT_SPRITE;
	re.radius = 16;

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0x11;
	re.shaderRGBA[2] = 0x11;

	re.radius = NUMBER_SIZE / 2;

	VectorClear(angles);
	AnglesToAxis( angles, re.axis );

	VectorSubtract(cg.refdef.vieworg, re.origin, dir);
	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	temp_num = wp_num;

	for (numdigits = 0; !(numdigits && !temp_num); numdigits++) {
		digits[numdigits] = temp_num % 10;
		temp_num = temp_num / 10;
	}

	for (i = 0; i < numdigits; i++) {
		//VectorMA(nodes[wp_num].origin, (float) (((float) numdigits / 2) - i) * NUMBER_SIZE, vec, re.origin);
		re.customShader = cgs.media.numberShaders[digits[numdigits-1-i]];
		trap->R_AddRefEntityToScene( &re );
	}
}

qboolean LinkCanReachMe ( int wp_from, int wp_to )
{
	int i = 0;

	for (i = 0; i < nodes[wp_to].enodenum; i ++)
	{
		if (nodes[wp_to].links[i].targetNode == wp_from)
			return qtrue;
	}

	return qfalse;
}

void CG_AddWaypointLinkLine( int wp_from, int wp_to )
{
	refEntity_t		re;

	memset( &re, 0, sizeof( re ) );

	re.reType = RT_LINE;
	re.radius = 1;

	if (nodes[wp_from].type & NODE_COVER)
	{// Cover spots show as yellow...
		re.shaderRGBA[0] = 0xff;
		re.shaderRGBA[1] = 0xff;
		re.shaderRGBA[2] = 0x00;
		re.shaderRGBA[3] = 0xff;
	}
	else if (LinkCanReachMe( wp_from, wp_to ))
	{// Link is bi-directional.. Display in white..
		re.shaderRGBA[0] = re.shaderRGBA[1] = re.shaderRGBA[2] = re.shaderRGBA[3] = 0xff;
	}
	else
	{// Link is uni-directional.. Display in red..
		re.shaderRGBA[0] = 0xff;
		re.shaderRGBA[1] = 0x00;
		re.shaderRGBA[2] = 0x00;
		re.shaderRGBA[3] = 0xff;
	}

	re.customShader = cgs.media.whiteShader;

	VectorCopy(nodes[wp_from].origin, re.origin);
	re.origin[2]+=16;
	VectorCopy(nodes[wp_to].origin, re.oldorigin);
	re.oldorigin[2]+=16;

	trap->R_AddRefEntityToScene( &re );
}

extern vmCvar_t jkg_waypoint_render;

qboolean CURRENTLY_RENDERRING = qfalse;

qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV )
{
	vec3_t	deltaVector, angles, deltaAngles;

	VectorSubtract ( spot, from, deltaVector );
	vectoangles ( deltaVector, angles );
	
	deltaAngles[PITCH]	= AngleDelta ( fromAngles[PITCH], angles[PITCH] );
	deltaAngles[YAW]	= AngleDelta ( fromAngles[YAW], angles[YAW] );

	if ( fabs ( deltaAngles[PITCH] ) <= vFOV && fabs ( deltaAngles[YAW] ) <= hFOV ) 
	{
		return qtrue;
	}

	return qfalse;
}

void DrawWaypoints()
{
	int node = 0;
	
	if (jkg_waypoint_render.integer <= 0)
	{
		if (CURRENTLY_RENDERRING)
		{
			AIMod_AutoWaypoint_Free_Memory();
		}

		CURRENTLY_RENDERRING = qfalse;
		return;
	}

	if (!CURRENTLY_RENDERRING)
	{
		number_of_nodes = 0;

		AIMod_AutoWaypoint_Init_Memory();

		AIMOD_NODES_LoadNodes(); // Load node file on first check...
		AIMOD_LoadCoverPoints();

		if (number_of_nodes <= 0)
		{
			AIMod_AutoWaypoint_Free_Memory();
			CURRENTLY_RENDERRING = qfalse;
			return;
		}

		CURRENTLY_RENDERRING = qtrue;
	}

	if (number_of_nodes <= 0) return; // If still no nodes, exit early...

	for (node = 0; node < number_of_nodes; node++)
	{
		// Draw anything closeish to us...
		int		len = 0;
		int		link = 0;
		vec3_t	delta;

		if (!InFOV( nodes[node].origin, cg.refdef.vieworg, cg.refdef.viewangles, 120, 120 ))
			continue;

		VectorSubtract( nodes[node].origin, cg.refdef.vieworg, delta );
		len = VectorLength( delta );
		
		if ( len < 20 ) continue;
		if ( len > 512 ) continue;

		//if (Distance(cg_entities[cg.clientNum].lerpOrigin, nodes[node].origin) > 2048) continue;

		//CG_Waypoint( node );

		for (link = 0; link < nodes[node].enodenum; link++)
		{
			CG_AddWaypointLinkLine( node, nodes[node].links[link].targetNode );
		}
	}
}

//float	BAD_HEIGHTS[1024];
//int		NUM_BAD_HEIGHTS = 0;

void AIMod_MarkBadHeight ( void )
{// Mark this height on the map as bad for waypoints...
	BAD_HEIGHTS[NUM_BAD_HEIGHTS] = cg.refdef.vieworg[2];
	NUM_BAD_HEIGHTS++;
	trap->Print("Height %f marked as bad for waypoints.\n", cg.refdef.vieworg[2]);
}

#endif //__AUTOWAYPOINT__
