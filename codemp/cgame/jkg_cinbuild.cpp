////////////////////////////////////
//
// Jedi Knight Galaxies Cinematic Builder
//
////////////////////////////////////

#include "cg_local.h"

static qhandle_t orc_a_font;

typedef struct {
	int offset;
	int time;
	vec3_t origin;
	vec3_t angles;
	vec3_t aimvec;
	vec4_t csi_coeff[6];   //cubic spline interpolation coefficients
} campoint_t;

typedef struct {
	vec3_t org;
	vec3_t ang;
	vec3_t angpos;
} traj_t;

typedef struct {
	int active;
	int type;	// 1 = linear, 2 = spline, 0 = undefined (dont process coords)
	int algos[6];
	int lastpoint[6];
	int interval;
	int campoints;
	int visible;
	int usetarget;
	vec3_t target;
	int suppressTrajectory;
	campoint_t * points;
	int trajpoints;
	traj_t * trajectory;
} cinbuild_t;

static cinbuild_t CinBuildData;
static void Cin_DoCubicSplineInterpolation();
static float CinBuild_GetCamPos(int offset, int axis);

void CinBuild_Init() {
	trap->AddCommand("cinbuild");
	memset(&CinBuildData, 0, sizeof(CinBuildData));
	orc_a_font = trap->R_RegisterFont("ocr_a");

	// Some additional stuff I thought I could chuck here to fix the Pazaak blur bug --eez
	if(ui_blurbackground.integer)
	{
		trap->Cvar_Set("ui_blurbackground", "0");
		cg.turnOnBlurCvar = qtrue;
	}
	trap->Cvar_Set("ui_hidehud", "0");
}

static void CinBuild_RecalculateOffsets() {
	// Recalculates all offsets based on the time entry
	// The time specifies the time to take to head to that point (!), so the time on point 0 has no effect
	int i;
	int totaloffset = 0;
	if (CinBuildData.campoints == 0) {
		return;
	} else {
		CinBuildData.points[0].offset = 0;
		for (i=1; i < CinBuildData.campoints; i++) {
			totaloffset += CinBuildData.points[i].time;
			CinBuildData.points[i].offset = totaloffset;
		}
	}
}

static void CinBuild_CreateTrajectory() {
	// Goal of this function is to fill in CinBuildData.trajectory with points the game has to draw lines inbetween
	// Remove the preview trajectory if defined
	int i;
	if (CinBuildData.trajectory) {
		free(CinBuildData.trajectory);
		CinBuildData.trajectory = 0;
		CinBuildData.trajpoints = 0;
	}
	if (CinBuildData.suppressTrajectory) {
		return;
	}
	CinBuild_RecalculateOffsets();
	if (!CinBuildData.type) {
		for (i=0; i < CinBuildData.campoints; i++) {
			vec3_t temp;
			AngleVectors(CinBuildData.points[i].angles, temp, NULL, NULL);
			VectorMA(CinBuildData.points[i].origin, 50, temp, temp);
			VectorCopy(temp, CinBuildData.points[i].aimvec);
		}
	} else if (CinBuildData.type == 1) {
		// Easy one here, its just linear ^_^, so just copy the points into the trajectory and be done with it
		if (CinBuildData.campoints < 2) return;
		CinBuildData.trajpoints = CinBuildData.campoints;
		CinBuildData.trajectory = (traj_t *)malloc(sizeof(traj_t) * CinBuildData.trajpoints);
		for (i=0; i < CinBuildData.campoints; i++) {
			vec3_t temp;
			VectorCopy(CinBuildData.points[i].origin, CinBuildData.trajectory[i].org);
			VectorCopy(CinBuildData.points[i].angles, CinBuildData.trajectory[i].ang);
			AngleVectors(CinBuildData.points[i].angles, temp, NULL, NULL);
			VectorMA(CinBuildData.points[i].origin, 50, temp, temp);
			VectorCopy(temp, CinBuildData.trajectory[i].angpos);
			VectorCopy(temp, CinBuildData.points[i].aimvec);
		}
	} else if (CinBuildData.type == 2) {
		// First off, if ANY of the tracks uses cubic spline interp, then do the interpolation
		int totaltime, time;
		float tmp;
		int points;
		// Dont even THINK of processing this with less than 2 points, as this will crash the interpolators
		if (CinBuildData.campoints < 2) {
			return;
		}

		for (i=0; i<6; i++) {
			if (CinBuildData.algos[i] == 0) {
				Cin_DoCubicSplineInterpolation();
				break;
			}
		}

		for (i=0; i<6; i++) {
			CinBuildData.lastpoint[i] = 0;	// Clear the last points
		}
		// Before we proceed, we have to determine the amount of trajectory points we'll be creating
		totaltime = CinBuildData.points[CinBuildData.campoints-1].offset;
		tmp = ceil((float)totaltime / (float)CinBuildData.interval);
		points = tmp + 1;
		CinBuildData.trajpoints = points;
		CinBuildData.trajectory = (traj_t *)malloc(sizeof(traj_t) * CinBuildData.trajpoints);
		// Time to interpolate! booya!
		time = 0;
		for (i=0; i < points; i++) {
			vec3_t temp;
			if (time > totaltime) {
				time = totaltime;
			}
			CinBuildData.trajectory[i].org[0] = CinBuild_GetCamPos(time, 0);
			CinBuildData.trajectory[i].org[1] = CinBuild_GetCamPos(time, 1);
			CinBuildData.trajectory[i].org[2] = CinBuild_GetCamPos(time, 2);
			if (CinBuildData.usetarget) {
				VectorSubtract(CinBuildData.target, CinBuildData.trajectory[i].org, temp);
				vectoangles(temp, CinBuildData.trajectory[i].ang);
			} else {
				CinBuildData.trajectory[i].ang[0] = CinBuild_GetCamPos(time, 3);
				CinBuildData.trajectory[i].ang[1] = CinBuild_GetCamPos(time, 4);
				CinBuildData.trajectory[i].ang[2] = CinBuild_GetCamPos(time, 5);
			}

			AngleVectors(CinBuildData.trajectory[i].ang, temp, NULL, NULL);
			VectorMA(CinBuildData.trajectory[i].org, 50, temp, temp);
			VectorCopy(temp, CinBuildData.trajectory[i].angpos);
			
			time += CinBuildData.interval;
		}
		for (i=0; i < CinBuildData.campoints; i++) {
			vec3_t temp;
			AngleVectors(CinBuildData.points[i].angles, temp, NULL, NULL);
			VectorMA(CinBuildData.points[i].origin, 50, temp, temp);
			VectorCopy(temp, CinBuildData.points[i].aimvec);
		}
	}
}

static void CinBuild_InitParseBuff(parsebuff_t *pb) {
	memset(pb,0,sizeof(parsebuff_t));
	pb->arg = 1;
	pb->argc = trap->Cmd_Argc();
}

static const char *CinBuild_NextToken(parsebuff_t *pb) {
	if (pb->arg > pb->argc) return NULL;
	trap->Cmd_Argv(pb->arg++,pb->buff, sizeof(pb->buff));
	return pb->buff;
}

static qboolean CinBuild_TokensAvailable(parsebuff_t *pb) {
	if (pb->arg >= pb->argc) return qfalse;
	return qtrue;
}

static int CinBuild_ParseVector(parsebuff_t *pb, vec3_t *vec) {
	const char *token;
	int i;
	for (i=0; i<3; i++) {
		token = CinBuild_NextToken(pb);
		if (!token) {
			trap->Print("WARNING: ^3Error processing cinematic builder info: Could not parse vector\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

static int CinBuild_ParseVector2(parsebuff_t *pb, vec2_t *vec) {
	const char *token;
	int i;
	for (i=0; i<2; i++) {
		token = CinBuild_NextToken(pb);
		if (!token) {
			trap->Print("WARNING: ^3Error processing cinematic builder info: Could not parse vector2\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

static int CinBuild_ParseInt(parsebuff_t *pb, int *num) {
	const char *token;
	token = CinBuild_NextToken(pb);
	if (!token) {
		trap->Print("WARNING: ^3Error processing cinematic builder info: Could not parse int\n");
		return 1;
	}
	*num = atoi(token);

	return 0;
}

static int CinBuild_ParseFloat(parsebuff_t *pb, float *num) {
	const char *token;
	token = CinBuild_NextToken(pb);
	
	if (!token) {
		trap->Print("WARNING: ^3Error processing cinematic builder info: Could not parse float\n");
		return 1;
	}
	*num = atof(token);

	return 0;
}

qboolean CG_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y);

// Code from UU
static float* MakeColor(float r, float g, float b, float a) {
	static float RGBA[4];
	RGBA[0]=r;
	RGBA[1]=g;
	RGBA[2]=b;
	RGBA[3]=a;
	return &RGBA[0];
}

// Code from UU
static void RE_Font_DrawCenterString(int ox, int oy, const char *text, const float *rgba, const int setIndex, int iCharLimit, const float scale) {
	int len = trap->R_Font_StrLenPixels(text,(setIndex & 0xFF),scale);
	int newx = ox - (len/2);
	trap->R_Font_DrawString(newx,oy,text,rgba,setIndex,iCharLimit,scale);
}

#define	FX_ALPHA_LINEAR		0x00000001
#define	FX_SIZE_LINEAR		0x00000100

void CinBuild_Visualize() {
	int i;
	refEntity_t ent;
	if (!CinBuildData.active || !CinBuildData.visible) {
		return;
	}
	
	// First, visualize the camera points (and display their info if you're close to em)
	memset( &ent, 0, sizeof( ent ) );
	ent.nonNormalizedAxes = qtrue;

	ent.hModel = trap->R_RegisterModel ( "models/weaphits/testboom.md3" );
	ent.customShader = trap->R_RegisterShader( "powerups/invulnerabilityshell");

	for (i=0; i < CinBuildData.campoints; i++) {
		vec3_t ang;
		const float scale=0.5f;
		
		VectorCopy( CinBuildData.points[i].origin, ent.origin );

		VectorSubtract(ent.origin, cg.refdef.vieworg, ent.axis[0]);
		
		vectoangles(ent.axis[0], ang);
		ang[ROLL] += 180.0f;
		ang[PITCH] += 180.0f;
		AnglesToAxis(ang, ent.axis);
		
		VectorScale(ent.axis[0], scale, ent.axis[0]);
		VectorScale(ent.axis[1], scale, ent.axis[1]);
		VectorScale(ent.axis[2], scale, ent.axis[2]);
		
		if (i == 0) {	// First point is red
			if (i == CinBuildData.campoints-1) {		// If its the first AND last point, then its the only point, so mix colors :P
				ent.shaderRGBA[0] = 255;
				ent.shaderRGBA[1] = 50;
				ent.shaderRGBA[2] = 255;
				ent.shaderRGBA[3] = 100;
				ent.renderfx = RF_RGB_TINT;
			} else {
				ent.shaderRGBA[0] = 255;
				ent.shaderRGBA[1] = 50;
				ent.shaderRGBA[2] = 50;
				ent.shaderRGBA[3] = 100;
				ent.renderfx = RF_RGB_TINT;
			}
		} else if (i == CinBuildData.campoints-1) {		// Last point is blue
			ent.shaderRGBA[0] = 50;
			ent.shaderRGBA[1] = 50;
			ent.shaderRGBA[2] = 255;
			ent.shaderRGBA[3] = 100;
			ent.renderfx = RF_RGB_TINT;
		} else {
			ent.renderfx = 0;
		}

		if (trap->R_InPVS(cg.refdef.vieworg, ent.origin, cg.snap->areamask)) {
			static vec3_t YELLOW = {1.0f, 1.0f, 0.0f };
			trap->R_AddRefEntityToScene( &ent );


			trap->FX_AddLine( CinBuildData.points[i].aimvec, CinBuildData.points[i].origin , 0.1f, 2.0f, 0.0f, 
				1.0f, 0.0f, 0.0f,
				YELLOW, YELLOW, 0.0f,
				150, trap->R_RegisterShader( "gfx/effects/redLine" ), 
				FX_SIZE_LINEAR | FX_ALPHA_LINEAR );
		}
	}
	if (CinBuildData.usetarget) {
		vec3_t ang;
		const float scale=0.5f;
		
		VectorCopy( CinBuildData.target, ent.origin );

		VectorSubtract(ent.origin, cg.refdef.vieworg, ent.axis[0]);
		
		vectoangles(ent.axis[0], ang);
		ang[ROLL] += 180.0f;
		ang[PITCH] += 180.0f;
		AnglesToAxis(ang, ent.axis);
		
		VectorScale(ent.axis[0], scale, ent.axis[0]);
		VectorScale(ent.axis[1], scale, ent.axis[1]);
		VectorScale(ent.axis[2], scale, ent.axis[2]);
		
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 50;
		ent.shaderRGBA[3] = 100;
		ent.renderfx = RF_RGB_TINT;


		if (trap->R_InPVS(cg.refdef.vieworg, ent.origin, cg.snap->areamask)) {
			trap->R_AddRefEntityToScene( &ent );
		}

	}
	for (i=1; i < CinBuildData.trajpoints; i++) {
		static vec3_t WHITE = {1.0f, 1.0f, 1.0f };
		static vec3_t BLUE = {0.0f, 0.0f, 1.0f };
		if (trap->R_InPVS(cg.refdef.vieworg, CinBuildData.trajectory[i].org, cg.snap->areamask)) {
			trap->FX_AddLine( CinBuildData.trajectory[i].org, CinBuildData.trajectory[i-1].org, 0.1f, 6.0f, 0.0f, 
				1.0f, 0.0f, 0.0f,
				WHITE, WHITE, 0.0f,
				150, trap->R_RegisterShader( "gfx/effects/redLine" ), 
				FX_SIZE_LINEAR | FX_ALPHA_LINEAR );


			trap->FX_AddLine( CinBuildData.trajectory[i].angpos, CinBuildData.trajectory[i-1].angpos, 0.1f, 2.0f, 0.0f, 
				1.0f, 0.0f, 0.0f,
				BLUE, BLUE, 0.0f,
				150, trap->R_RegisterShader( "gfx/effects/redLine" ), 
				FX_SIZE_LINEAR | FX_ALPHA_LINEAR );
		}

	}
}

void CinBuild_Visualize2D() {
	int i;
	if (!CinBuildData.active || !CinBuildData.visible) {
		return;
	}

	for (i=0; i < CinBuildData.campoints; i++) {
		vec3_t tmppos, dist;
		float alpha, distance, x, y;		
		
		if (trap->R_InPVS(cg.refdef.vieworg, CinBuildData.points[i].origin, cg.snap->areamask)) {

			VectorSubtract(CinBuildData.points[i].origin, cg.refdef.vieworg, dist);
			distance = sqrt(dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2]);
			if (distance > 300)
				continue;
			if (distance < 150) {
				alpha = 1.0f;
			} else {
				alpha = (150.0f - (distance - 150.0f)) / 150.0f;
			}
			VectorCopy(CinBuildData.points[i].origin,tmppos);
			tmppos[2] += 40;
			if (CG_WorldCoordToScreenCoordFloat(tmppos, &x, &y)) {
				RE_Font_DrawCenterString(x, y, va("Cam point #%i", i+1), MakeColor(1,0,0,alpha), orc_a_font, -1, 0.6f);
				RE_Font_DrawCenterString(x, y+10,va("Position: (%.1f %.1f %.1f) (%.1f %.1f %.1f)",CinBuildData.points[i].origin[0], CinBuildData.points[i].origin[1], CinBuildData.points[i].origin[2], CinBuildData.points[i].angles[0], CinBuildData.points[i].angles[1],CinBuildData.points[i].angles[2]),  MakeColor(1,1,1,alpha), orc_a_font, -1, 0.6f);
				if (i != 0) {
					RE_Font_DrawCenterString(x, y+20,va("Move time: %i ms",CinBuildData.points[i].time), MakeColor(1,1,1,alpha), orc_a_font, -1, 0.6f);
				}
			}
		}
	}
	if (CinBuildData.usetarget) {
		vec3_t tmppos, dist;
		float alpha, distance, x, y;		
		
		if (trap->R_InPVS(cg.refdef.vieworg, CinBuildData.target, cg.snap->areamask)) {

			VectorSubtract(CinBuildData.target, cg.refdef.vieworg, dist);
			distance = sqrt(dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2]);
			if (distance > 300)
				return;
			if (distance < 150) {
				alpha = 1.0f;
			} else {
				alpha = (150.0f - (distance - 150.0f)) / 150.0f;
			}
			VectorCopy(CinBuildData.target,tmppos);
			tmppos[2] += 40;
			if (CG_WorldCoordToScreenCoordFloat(tmppos, &x, &y)) {
				RE_Font_DrawCenterString(x, y, "View Target", MakeColor(1,0,0,alpha), orc_a_font, -1, 0.6f);
				RE_Font_DrawCenterString(x, y+10,va("Position: (%.1f %.1f %.1f)", CinBuildData.target[0], CinBuildData.target[1], CinBuildData.target[2]),  MakeColor(1,1,1,alpha), orc_a_font, -1, 0.6f);
			}
		}

	}
}

void CinBuild_Cmd_f() {
	// Process cinematic builder commands
	parsebuff_t pb;
	const char *token;
	int i;
	CinBuild_InitParseBuff(&pb);

	while (1) {
		token = CinBuild_NextToken(&pb);
		if (!token) break;

		if (!Q_stricmp(token,"on")) {
			// Initiate cinematic builder mode
			if (CinBuildData.points) {
				free(CinBuildData.points);
			}
			if (CinBuildData.trajectory) {
				free(CinBuildData.trajectory);
			}
			memset(&CinBuildData, 0, sizeof(CinBuildData));
			CinBuildData.active = 1;
			CinBuildData.interval = 250;
			CinBuildData.visible = 1;
			continue;
		}
		if (!Q_stricmp(token,"off")) {
			// Terminate cinematic builder mode
			if (CinBuildData.points) {
				free(CinBuildData.points);
			}
			if (CinBuildData.trajectory) {
				free(CinBuildData.trajectory);
			}
			memset(&CinBuildData, 0, sizeof(CinBuildData));
			continue;
		}
		if (!CinBuildData.active) {
			// Dont process commands beyond this point if the builder is off
			break;
		}
		if (!Q_stricmp(token,"ap")) {
			// Add new camera point
			CinBuildData.campoints++;
			CinBuildData.points = (campoint_t *)realloc(CinBuildData.points, sizeof(campoint_t) * CinBuildData.campoints);
			i = CinBuildData.campoints-1;
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[i].origin)) return;
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[i].angles)) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.points[i].time)) return;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"ep")) {
			// Edit camera point
			if (CinBuild_ParseInt(&pb, &i)) return;
			i--;	// Lua starts at 1, so decrement by 1 to get our array index
			if (i < 0 || i >= CinBuildData.campoints) {
				trap->Print("WARNING: ^3Error processing cinematic builder info: Edit Point - Invalid campoint index specified");
				return;
			}
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[i].origin)) return;
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[i].angles)) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.points[i].time)) return;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"ip")) {
			int pt;
			// Insert camera point
			if (CinBuild_ParseInt(&pb, &i)) return;
			i--;	// Lua starts at 1, so decrement by 1 to get our array index
			pt = i;
			if (i < 0 || i >= CinBuildData.campoints) {
				trap->Print("WARNING: ^3Error processing cinematic builder info: Insert Point - Invalid campoint index specified");
				return;
			}
			// Shift all entries around it upward
			CinBuildData.campoints++;
			CinBuildData.points = (campoint_t *)realloc(CinBuildData.points, sizeof(campoint_t) * CinBuildData.campoints);
			for (i = CinBuildData.campoints-2; i >= pt; i--) {
				CinBuildData.points[i+1] = CinBuildData.points[i];
			}
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[pt].origin)) return;
			if (CinBuild_ParseVector(&pb, &CinBuildData.points[pt].angles)) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.points[pt].time)) return;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"type")) {
			// Edit camera point
			if (CinBuild_ParseInt(&pb, &CinBuildData.type)) return;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"int")) {
			// Edit camera point
			if (CinBuild_ParseInt(&pb, &CinBuildData.interval)) return;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"alg")) {
			// Edit camera point
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[0])) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[1])) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[2])) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[3])) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[4])) return;
			if (CinBuild_ParseInt(&pb, &CinBuildData.algos[5])) return;

			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"rp")) {
			// Remove point
			if (CinBuild_ParseInt(&pb, &i)) return;
			// i = array index +1 (!)
			if (i < 1 || i > CinBuildData.campoints) {
				trap->Print("WARNING: ^3Error processing cinematic builder info: Remove Point - Invalid campoint index specified");
				return;
			}
			// Shift all the points down
			for (; i < CinBuildData.campoints; i++) {
				CinBuildData.points[i-1] = CinBuildData.points[i];
			}
			CinBuildData.campoints--;
			CinBuildData.points = (campoint_t *)realloc(CinBuildData.points, sizeof(campoint_t) * CinBuildData.campoints);

			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"clr")) {
			// Clear all points
			if (CinBuildData.points) {
				free(CinBuildData.points);
				CinBuildData.points = 0;
			}
			CinBuildData.campoints = 0;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"fixang")) {
			// Fix angles
			// Exactly the same code as used in lua server-side
			if (CinBuildData.campoints < 2) {
				continue;
			}
			for (i=0; i < CinBuildData.campoints-1; i++) {
				float delta = CinBuildData.points[i+1].angles[YAW] - CinBuildData.points[i].angles[YAW];
				if (delta > 180) {
					// Check for sign changes
					if (CinBuildData.points[i+1].angles[YAW] < 0 && CinBuildData.points[i].angles[YAW] > 0) {
						CinBuildData.points[i+1].angles[YAW] = 360 + CinBuildData.points[i+1].angles[YAW];
					} else if ( CinBuildData.points[i+1].angles[YAW] > 0 && CinBuildData.points[i].angles[YAW] < 0 ) {
						CinBuildData.points[i+1].angles[YAW] = CinBuildData.points[i+1].angles[YAW] - 360;
					} else {
						CinBuildData.points[i+1].angles[YAW] = CinBuildData.points[i+1].angles[YAW] - 360;
					}
				} else if (delta < -180) {
					if (CinBuildData.points[i+1].angles[YAW] < 0 && CinBuildData.points[i].angles[YAW] > 0) {
						CinBuildData.points[i+1].angles[YAW] = 360 + CinBuildData.points[i+1].angles[YAW];
					} else if ( CinBuildData.points[i+1].angles[YAW] > 0 && CinBuildData.points[i].angles[YAW] < 0 ) {
						CinBuildData.points[i+1].angles[YAW] = -360 +CinBuildData.points[i+1].angles[YAW];
					} else {
						CinBuildData.points[i+1].angles[YAW] = 360 + CinBuildData.points[i+1].angles[YAW];
					}
				}
			}
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"vis")) {
			// Toggle point visibility (used during previews)
			if (CinBuild_ParseInt(&pb, &CinBuildData.visible)) return;
			continue;
		}
		if (!Q_stricmp(token,"supr")) {
			// Suppress trajectory update (used when loading cinematics)
			if (CinBuild_ParseInt(&pb, &CinBuildData.suppressTrajectory)) return;
			if (!CinBuildData.suppressTrajectory) {
				// If it got enabled again, create the trajectory
				CinBuild_CreateTrajectory();
			}
			continue;
		}
		if (!Q_stricmp(token,"st")) {
			// Set view target
			if (CinBuild_ParseVector(&pb, &CinBuildData.target)) return;
			CinBuildData.usetarget = 1;
			CinBuild_CreateTrajectory();
			continue;
		}
		if (!Q_stricmp(token,"ct")) {
			// Clear view target
			CinBuildData.usetarget = 0;
			CinBuild_CreateTrajectory();
			continue;
		}
	}
}

// Following code originally from pugmod
// Modified to improve readability, and to make it compatible with JKG
// start cubic spline interpolation code (parts are converted from javascript at http://people.hofstra.edu/faculty/Stefan_Waner/realworld/tutorialsf1/scriptpivot2.html)

double csplMatrix[50*4][50*4+1];

static void Cin_Spl_pivot( int rows, int cols, int theRow, int theCol ) {
	double thePivot = csplMatrix[theRow][theCol];
	int i, j;
	for( i=0; i<cols; i++ )
		csplMatrix[theRow][i] = csplMatrix[theRow][i]/thePivot;

	for( i=0; i<rows; i++ )
		if( ( i!=theRow ) && ( csplMatrix[i][theCol]!=0 ) )
		{
			double factr = csplMatrix[i][theCol];
			for( j=0; j<cols; j++ )
				csplMatrix[i][j] = csplMatrix[i][j] - factr*csplMatrix[theRow][j];	
		}
}

static void Cin_Spl_swapRows( int p, int q, int numCols ) {
	double rowHold = 0;
	int j;
	for( j=0; j<numCols; j++ )
	{
		rowHold = csplMatrix[p][j];
		csplMatrix[p][j] = csplMatrix[q][j];
		csplMatrix[q][j] = rowHold;
	}
}

static void Cin_Spl_rowReduce(int numRows, int numCols) {
	int i, j, theCol;
	double theSmallestNumber = 0.00000000001;
	int theRow = 0;
	for(  i=0; i<numRows; i++ )
	{ 
		theCol = -1;
		for( j=0; j<numCols; j++ )
		{
			double theNum = csplMatrix[i][j];
			if( theNum * ( theNum > 0 ? 1 : -1 ) <= theSmallestNumber )
				csplMatrix[i][j] = 0;
			else { 
				theCol = j;
				break; 
			}
		}
		if( theCol!=-1 )
			Cin_Spl_pivot(numRows,numCols,i,theCol);
	}
	
	for( j=0; j<numCols; j++ ) {
		for( i=theRow; i<numRows; i++ ) {
			if( csplMatrix[i][j] != 0 )
			{
				if( i==theRow ) {
					theRow++;
					break;
				} else {
					Cin_Spl_swapRows( theRow, i, numCols);
					theRow++; 
					break;
				}
			}
		}
	}
}

#define TIMEDIV 1000
// recalculate cubic spline interpolation coefficients for cam paths
static void Cin_DoCubicSplineInterpolation() {
	int numRows = 0, numCols = 0;
	int i, j, k, curRow;
	campoint_t *pd;
	double deltaT;
	double deltaD;
	int CamCount;

	pd = CinBuildData.points; // faster access

	CamCount = CinBuildData.campoints-1;
	for( k=0; k<3; k++ )
	{
		curRow = 0;
		memset( csplMatrix[0], 0, sizeof( csplMatrix ) );
		for( i=0; i<CamCount-1; i++ )
		{
			deltaT = (double)(pd[i+1].offset - pd[i].offset);
			deltaD = (double)(pd[i+1].origin[k] - pd[i].origin[k]);
			j=0;
			//runs through first point
			csplMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;//cam[i].origin[k];
			
			j=0;
			//runs through second point
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow++][(CamCount-1)*4+4] = deltaD;
			
			j=0;
			//first derivitives are equal
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*2;
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*2;
			csplMatrix[curRow][i*4+j++] = -1;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
			
			j=0;
			//second derivitives are equal
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*6;
			csplMatrix[curRow][i*4+j++] = 2;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*6;
			csplMatrix[curRow][i*4+j++] = -2;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		}
		//4 more equations are needed to have a solvable matrix (there should be n-1 splines)
		
		j=0;
		//second derivitive at the end is zero
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)pd[CamCount].offset-pd[CamCount-1].offset)/TIMEDIV)*6;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 2;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//second derivitive at the beginning is zero
		csplMatrix[curRow][j++] = 0;//(((double)cam[0].time)/TIMEDIV)*6;
		csplMatrix[curRow][j++] = 2;
		csplMatrix[curRow][j++] = 0;
		csplMatrix[curRow][j++] = 0;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//runs through first point
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 1;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;//cam[(CamCount-1)].origin[k];
		
		deltaT = (double)(pd[CamCount].offset - pd[CamCount-1].offset);
		j=0;
		//runs through second point
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 1;
		csplMatrix[curRow++][(CamCount-1)*4+4] = pd[CamCount].origin[k] - pd[CamCount-1].origin[k];
		
		numRows = curRow;
		numCols = CamCount*4+1;
		Cin_Spl_rowReduce(numRows, numCols);
		
		for( i=0; i<CamCount; i++ )
		{
			pd[i].csi_coeff[k][0] = csplMatrix[i*4][CamCount*4];
			pd[i].csi_coeff[k][1] = csplMatrix[i*4+1][CamCount*4];
			pd[i].csi_coeff[k][2] = csplMatrix[i*4+2][CamCount*4];
			pd[i].csi_coeff[k][3] = csplMatrix[i*4+3][CamCount*4];
		}
	}
	
	for( k=0; k<3; k++ )
	{
		curRow = 0;
		memset( csplMatrix[0], 0, sizeof( csplMatrix ) );
		for( i=0; i<CamCount-1; i++ )
		{
			deltaT = (double)(pd[i+1].offset - pd[i].offset);
			deltaD = (double)(pd[i+1].angles[k] - pd[i].angles[k]);
			j=0;
			//runs through first point
			//guess what? ive decided it's (0,0) kthx.
			csplMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;//pd[i].origin[k];
			
			j=0;
			//runs through second point
			//doodz we're makin dese thangs unit doodz so it b leeter :>
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV);
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow++][(CamCount-1)*4+4] = deltaD;
			
			j=0;
			//first derivitives are equal
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*2;
			csplMatrix[curRow][i*4+j++] = 1;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*2;
			csplMatrix[curRow][i*4+j++] = -1;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
			
			j=0;
			//second derivitives are equal
			csplMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*6;
			csplMatrix[curRow][i*4+j++] = 2;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*6;
			csplMatrix[curRow][i*4+j++] = -2;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow][i*4+j++] = 0;
			csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		}
		//4 more equations are needed to have a solvable matrix (there should be n-1 splines)
		
		j=0;
		//second derivitive at the end is zero
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)pd[CamCount].offset-pd[CamCount-1].offset)/TIMEDIV)*6;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 2;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//second derivitive at the beginning is zero
		csplMatrix[curRow][j++] = 0;//(((double)pd[0].offset)/TIMEDIV)*6;
		csplMatrix[curRow][j++] = 2;
		csplMatrix[curRow][j++] = 0;
		csplMatrix[curRow][j++] = 0;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//runs through first point
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 1;
		csplMatrix[curRow++][(CamCount-1)*4+4] = 0;//pd[(CamCount-1)].origin[k];
		
		deltaT = (double)(pd[CamCount].offset - pd[CamCount-1].offset);
		j=0;
		//runs through second point
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV);
		csplMatrix[curRow][(CamCount-1)*4+j++] = 1;
		csplMatrix[curRow++][(CamCount-1)*4+4] = pd[CamCount].angles[k] - pd[CamCount-1].angles[k];
		
		numRows = curRow;
		numCols = CamCount*4+1;
		Cin_Spl_rowReduce(numRows, numCols);
		
		for( i=0; i<CamCount; i++ )
		{
			pd[i].csi_coeff[k+3][0] = csplMatrix[i*4][CamCount*4];
			pd[i].csi_coeff[k+3][1] = csplMatrix[i*4+1][CamCount*4];
			pd[i].csi_coeff[k+3][2] = csplMatrix[i*4+2][CamCount*4];
			pd[i].csi_coeff[k+3][3] = csplMatrix[i*4+3][CamCount*4];
		}
	}
}

// end cubic spline interpolation code

static int Cin_GetBSplinePoint(int index, int count) {
	if (index < 0)
		return 0;
	if (index > count)
		return count;
	return index;
}

static float BSpF[21][5] = { //B-Spline Factors, used for fast subdivision lookup
	{ 0.166667f, 0.666667f, 0.166667f, 0.000000f, 0.000f }, // 0
	{ 0.142896f, 0.664229f, 0.192854f, 0.000021f, 0.050f }, // 1
	{ 0.121500f, 0.657167f, 0.221167f, 0.000167f, 0.100f }, // 2
	{ 0.102354f, 0.645854f, 0.251229f, 0.000563f, 0.150f }, // 3
	{ 0.085333f, 0.630667f, 0.282667f, 0.001333f, 0.200f }, // 4
	{ 0.070313f, 0.611979f, 0.315104f, 0.002604f, 0.250f }, // 5
	{ 0.057167f, 0.590167f, 0.348167f, 0.004500f, 0.300f }, // 6
	{ 0.045771f, 0.565604f, 0.381479f, 0.007146f, 0.350f }, // 7
	{ 0.036000f, 0.538667f, 0.414667f, 0.010667f, 0.400f }, // 8
	{ 0.027729f, 0.509729f, 0.447354f, 0.015187f, 0.450f }, // 9
	{ 0.020833f, 0.479167f, 0.479167f, 0.020833f, 0.500f }, // 10
	{ 0.015187f, 0.447354f, 0.509729f, 0.027729f, 0.550f }, // 11
	{ 0.010667f, 0.414667f, 0.538667f, 0.036000f, 0.600f }, // 12
	{ 0.007146f, 0.381479f, 0.565604f, 0.045771f, 0.650f }, // 13
	{ 0.004500f, 0.348167f, 0.590167f, 0.057167f, 0.700f }, // 14
	{ 0.002604f, 0.315104f, 0.611979f, 0.070313f, 0.750f }, // 15
	{ 0.001333f, 0.282667f, 0.630667f, 0.085333f, 0.800f }, // 16
	{ 0.000562f, 0.251229f, 0.645854f, 0.102354f, 0.850f }, // 17
	{ 0.000167f, 0.221167f, 0.657167f, 0.121500f, 0.900f }, // 18
	{ 0.000021f, 0.192854f, 0.664229f, 0.142896f, 0.950f }, // 19
	{ 0.000000f, 0.166667f, 0.666667f, 0.166667f, 1.000f }, // 20
};

static float CMRF[21][5] = { // Catmull-rom Factors, used for fast subdivision lookup
	{ 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000f }, // 0
	{ -0.022563f, 0.993937f, 0.029812f, -0.001188f, 0.050f }, // 1
	{ -0.040500f, 0.976500f, 0.068500f, -0.004500f, 0.100f }, // 2
	{ -0.054188f, 0.948812f, 0.114938f, -0.009562f, 0.150f }, // 3
	{ -0.064000f, 0.912000f, 0.168000f, -0.016000f, 0.200f }, // 4
	{ -0.070313f, 0.867188f, 0.226563f, -0.023438f, 0.250f }, // 5
	{ -0.073500f, 0.815500f, 0.289500f, -0.031500f, 0.300f }, // 6
	{ -0.073938f, 0.758062f, 0.355687f, -0.039812f, 0.350f }, // 7
	{ -0.072000f, 0.696000f, 0.424000f, -0.048000f, 0.400f }, // 8
	{ -0.068062f, 0.630437f, 0.493312f, -0.055687f, 0.450f }, // 9
	{ -0.062500f, 0.562500f, 0.562500f, -0.062500f, 0.500f }, // 10
	{ -0.055688f, 0.493312f, 0.630437f, -0.068062f, 0.550f }, // 11
	{ -0.048000f, 0.424000f, 0.696000f, -0.072000f, 0.600f }, // 12
	{ -0.039813f, 0.355688f, 0.758062f, -0.073937f, 0.650f }, // 13
	{ -0.031500f, 0.289500f, 0.815500f, -0.073500f, 0.700f }, // 14
	{ -0.023438f, 0.226563f, 0.867188f, -0.070313f, 0.750f }, // 15
	{ -0.016000f, 0.168000f, 0.912000f, -0.064000f, 0.800f }, // 16
	{ -0.009562f, 0.114937f, 0.948813f, -0.054188f, 0.850f }, // 17
	{ -0.004500f, 0.068500f, 0.976500f, -0.040500f, 0.900f }, // 18
	{ -0.001188f, 0.029813f, 0.993937f, -0.022563f, 0.950f }, // 19
	{ 0.000000f, 0.000000f, 1.000000f, 0.000000f, 1.000f }, // 20
};

static float Cin_BSplinePhaseLookup(float x, float p1, float p2, float p3, float p4) {
	// Alright, since the distribution of the x values isnt always linear
	// We'll use this lookup method to find it with quite good precision

	// To do this, the spline is split up in 20 sections
	// All of these segments have their boundary factors hardcoded in the BSpF table
	

	float min_x = BSpF[0][0] * p1 + BSpF[0][1] * p2 + BSpF[0][2] * p3 + BSpF[0][3] * p4;
	float max_x = BSpF[20][0] * p1 + BSpF[20][1] * p2 + BSpF[20][2] * p3 + BSpF[20][3] * p4;
	float seg_min;
	float seg_max;
	// To begin the check, try to get an estimate of the phase
	float phase;
	int segment;

	if (min_x == max_x) {
		phase = 0;
	} else {
		phase = ((float)x - min_x) / (max_x-min_x);
	}
	segment = (int)(phase/20); // Get the estimated segment

	// Alright, now we do the lookup
	while (1) {
		seg_min = BSpF[segment][0] * p1 + BSpF[segment][1] * p2 + BSpF[segment][2] * p3 + BSpF[segment][3] * p4;
		seg_max = BSpF[segment+1][0] * p1 + BSpF[segment+1][1] * p2 + BSpF[segment+1][2] * p3 + BSpF[segment+1][3] * p4;
		if (x < seg_min) {
			segment--;
			if (segment < 0) {
				phase = 0;
				break;
			}
		} else if (x > seg_max) {
			segment++;
			if (segment > 20) {
				phase = 1;
				break;
			}
		} else {
			// Alright!, we got a nice one, lets make the interp a lil more accurate now
			if (seg_min == seg_max) { // Rare.. but check just in case
				phase = BSpF[segment][4];
			} else {
				phase = ((float)x - seg_min) / (seg_max-seg_min);
				phase = BSpF[segment][4] + (phase * 0.05f);
			}
			break;
		}
	}
	return phase;
}

static float Cin_CatMullPhaseLookup(float x, float p1, float p2, float p3, float p4) {
	// Alright, since the distribution of the x values isnt always linear
	// We'll use this lookup method to find it with quite good precision

	// To do this, the spline is split up in 20 sections
	// All of these segments have their boundary factors hardcoded in the BSpF table
	

	float min_x = CMRF[0][0] * p1 + CMRF[0][1] * p2 + CMRF[0][2] * p3 + CMRF[0][3] * p4;
	float max_x = CMRF[20][0] * p1 + CMRF[20][1] * p2 + CMRF[20][2] * p3 + CMRF[20][3] * p4;
	float seg_min;
	float seg_max;
	// To begin the check, try to get an estimate of the phase
	float phase;
	int segment;

	if (min_x == max_x) {
		phase = 0;
	} else {
		phase = ((float)x - min_x) / (max_x-min_x);
	}
	segment = (int)(phase/20); // Get the estimated segment

	// Alright, now we do the lookup
	while (1) {
		seg_min = CMRF[segment][0] * p1 + CMRF[segment][1] * p2 + CMRF[segment][2] * p3 + CMRF[segment][3] * p4;
		seg_max = CMRF[segment+1][0] * p1 + CMRF[segment+1][1] * p2 + CMRF[segment+1][2] * p3 + CMRF[segment+1][3] * p4;
		if (x < seg_min) {
			segment--;
			if (segment < 0) {
				phase = 0;
				break;
			}
		} else if (x > seg_max) {
			segment++;
			if (segment > 20) {
				phase = 1;
				break;
			}
		} else {
			// Alright!, we got a nice one, lets make the interp a lil more accurate now
			if (seg_min == seg_max) { // Rare.. but check just in case
				phase = CMRF[segment][4];
			} else {
				phase = ((float)x - seg_min) / (seg_max-seg_min);
				phase = CMRF[segment][4] + (phase * 0.05f);
			}
			break;
		}
	}
	return phase;
}

static float CinBuild_GetCamPos(int offset, int axis) {
	// Axis: 0 1 2 = x y z, 3 4 5 = pitch yaw roll
	int i;
	int pt;
	float val;
	if (axis < 0 || axis > 5) return 0;
	switch (CinBuildData.algos[axis]) {
		case 0:
			{
				float ltime;
				// Do selective lookup (saves time ;) )
				i = CinBuildData.lastpoint[axis];
				while (1) {
					if (CinBuildData.points[i+1].offset >= offset) {
						if (CinBuildData.points[i].offset <= offset) {
							pt = i;
							break;
						} else {
							// Go back one point (or bail if this is the first point
							if (i==0) {
								pt = 0;
								break;
							}
							i--;
						}
					} else {
						// Go to the next point, or bail if this is the last (should never happen)
						if (i == CinBuildData.campoints-1) {
							// Last point
							pt = i;
							break;
						}
						i++;
					}
				}

				CinBuildData.lastpoint[axis] = pt;

				ltime = (float)(offset - CinBuildData.points[pt].offset)/1000.0f;
				val = CinBuildData.points[pt].csi_coeff[axis][0]*ltime*ltime*ltime + CinBuildData.points[pt].csi_coeff[axis][1]*ltime*ltime + CinBuildData.points[pt].csi_coeff[axis][2]*ltime + CinBuildData.points[pt].csi_coeff[axis][3];
				if (axis < 3) {
					val += CinBuildData.points[pt].origin[axis];
				} else {
					val += CinBuildData.points[pt].angles[axis-3];
				}
				return val;
			}
		case 1:
			{
				float phase;
				float mintime;
				float maxtime;
				float t, it, b0, b1, b2, b3;
				// Do selective lookup (saves time ;) )
				i = CinBuildData.lastpoint[axis]; // (-3 to pointcount-1)! (clamped)
				while (1) {
					mintime = BSpF[0][0] * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset +
							  BSpF[0][1] * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset +
							  BSpF[0][2] * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset +
							  BSpF[0][3] * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset;


					maxtime = BSpF[20][0] * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset +
							  BSpF[20][1] * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset +
							  BSpF[20][2] * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset +
							  BSpF[20][3] * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset;
					
					if (offset < mintime) {
						// Go back one part, or clamp to this one if its -3
						if (i == -3) {
							pt = -3;
							break;
						}
						i--;
					} else if ( offset > maxtime) {
						// Go forward another point, or clamp if its pointcount-1
						if (i == CinBuildData.campoints-1) {
							pt = i;
							break;
						}
						i++;
					} else {
						pt = i;
						break;
					}
				}
				CinBuildData.lastpoint[axis] = pt;
				// Got the point, calculate the phase		
				// We still have the min and max times :p so yeah
				phase = Cin_BSplinePhaseLookup(offset, CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset, 
													   CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset,
													   CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset,
													   CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset);
				t = phase;
			
				// the t value inverted
				it = 1.0f-t;

				// calculate blending functions for cubic bspline
				b0 = it*it*it/6.0f;
				b1 = (3*t*t*t - 6*t*t +4)/6.0f;
				b2 = (-3*t*t*t +3*t*t + 3*t + 1)/6.0f;
				b3 =  t*t*t/6.0f;

				if (axis < 3) {
					val = b0 * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].origin[axis] +
						  b1 * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].origin[axis] +
						  b2 * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].origin[axis] +
						  b3 * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].origin[axis];
				} else {
					val = b0 * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].angles[axis-3] +
						  b1 * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].angles[axis-3] +
						  b2 * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].angles[axis-3] +
						  b3 * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].angles[axis-3];
				}
				return val;
				
			}
		case 2:
			{
				float phase;
				float mintime;
				float maxtime;
				float t, t2, t3, b0, b1, b2, b3;
				// Do selective lookup (saves time ;) )
				i = CinBuildData.lastpoint[axis]; // (-3 to pointcount-1)! (clamped)
				while (1) {
					mintime = CMRF[0][0] * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset +
							  CMRF[0][1] * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset +
							  CMRF[0][2] * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset +
							  CMRF[0][3] * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset;


					maxtime = CMRF[20][0] * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset +
							  CMRF[20][1] * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset +
							  CMRF[20][2] * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset +
							  CMRF[20][3] * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset;
					
					if (offset < mintime) {
						// Go back one part, or clamp to this one if its -2
						if (i == -2) {
							pt = -2;
							break;
						}
						i--;
					} else if ( offset > maxtime) {
						// Go forward another point, or clamp if its pointcount-1
						if (i == CinBuildData.campoints-1) {
							pt = i;
							break;
						}
						i++;
					} else {
						pt = i;
						break;
					}
				}
				CinBuildData.lastpoint[axis] = pt;
				// Got the point, calculate the phase		
				// We still have the min and max times :p so yeah
				phase = Cin_CatMullPhaseLookup(offset, CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].offset, 
													   CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].offset,
													   CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].offset,
													   CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].offset);
				t = phase;
				t2 = t * t;
				t3 = t2 * t;

				b0 = .5 * (  -t3 + 2*t2 - t);
				b1 = .5 * ( 3*t3 - 5*t2 + 2);
				b2 = .5 * (-3*t3 + 4*t2 + t);
				b3 = .5 * (   t3 -   t2    );

				if (axis < 3) {
					val = b0 * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].origin[axis] +
						  b1 * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].origin[axis] +
						  b2 * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].origin[axis] +
						  b3 * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].origin[axis];
				} else {
					val = b0 * CinBuildData.points[Cin_GetBSplinePoint(i,CinBuildData.campoints-1)].angles[axis-3] +
						  b1 * CinBuildData.points[Cin_GetBSplinePoint(i+1,CinBuildData.campoints-1)].angles[axis-3] +
						  b2 * CinBuildData.points[Cin_GetBSplinePoint(i+2,CinBuildData.campoints-1)].angles[axis-3] +
						  b3 * CinBuildData.points[Cin_GetBSplinePoint(i+3,CinBuildData.campoints-1)].angles[axis-3];
				}
				return val;	
			}
			break;
		case 3:
			{
				float phase;
				i = CinBuildData.lastpoint[axis];
				while (1) {
					if (CinBuildData.points[i+1].offset >= offset) {
						if (CinBuildData.points[i].offset <= offset) {
							pt = i;
							break;
						} else {
							// Go back one point (or bail if this is the first point
							if (i==0) {
								pt = 0;
								break;
							}
							i--;
						}
					} else {
						// Go to the next point, or bail if this is the last (should never happen)
						if (i == CinBuildData.campoints-1) {
							// Last point
							pt = i;
							break;
						}
						i++;
					}
				}
				CinBuildData.lastpoint[axis] = pt; // Store this point, so we start looking there next time
				if (pt == CinBuildData.campoints - 1) {
					// Last point, dont bother to interpolate
					if (axis < 3) {
						val = CinBuildData.points[pt].origin[axis];
					} else {
						val = CinBuildData.points[pt].angles[axis-3];
					}
					return val;
				}
				// Interpolate it nicely then :)
				if (CinBuildData.points[pt].offset == CinBuildData.points[pt+1].offset) {
					phase = 0;
				} else {
					phase = (float)(offset - CinBuildData.points[pt].offset) / (float)(CinBuildData.points[pt+1].offset - CinBuildData.points[pt].offset);
				}
				if (axis < 3) {
					val = (CinBuildData.points[pt].origin[axis] * (1-phase)) + (CinBuildData.points[pt+1].origin[axis] * phase);
				} else {
					val = (CinBuildData.points[pt].angles[axis-3] * (1-phase)) + (CinBuildData.points[pt+1].angles[axis-3] * phase);
				}
				return val;
			}
			break;
		default:
			return 0;
	}
}
