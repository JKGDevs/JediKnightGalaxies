///////////////////////////////////////////////////////////////////
//
//  Cinematics module
//
//  Handles all cinematic stuff
//
//  By BobaFett
//
///////////////////////////////////////////////////////////////////

#include "cg_local.h"
#include <math.h>
#include "encoding/bitstream.h"
#include "encoding/base128.h"

#undef PI
#define PI (3.14159265358979323846)
#define RADIANS_PER_DEGREE (PI/180.0)

typedef enum {
	CAM_DEFAULT,
	CAM_STATIC,
	CAM_AIMED,
	CAM_LINEAR,
	CAM_SPLINE,
	CAM_HITCHCOCK,
	CAM_VIDEO,
} cammode_e;

typedef struct {
	int state;		// 0 = off, 1 = fading, 2 = black
	int from;		// in %
	int to;			// in %
	int startTime;	// cg.time when fade started
	int duration;	// ms fade time
} FadeInfo_t;

typedef struct {
	int state;		// 0 = off, 1 = fading, 2 = black
	int from;		// in %
	int to;			// in %
	int passes;		// Amount of blur passes
	int startTime;	// cg.time when fade started
	int duration;	// ms fade time
} BlurInfo_t;

typedef struct CmInfo_s{
	struct scale_s {
		int state;		// 0 = off, 1 = fading, 2 = static
		vec3_t from;
		vec3_t to;
		int startTime;	// cg.time when fade started
		int duration;	// ms fade time
	} scale;
	struct bias_s {
		int state;		// 0 = off, 1 = fading, 2 = static
		vec3_t from;
		vec3_t to;
		int startTime;	// cg.time when fade started
		int duration;	// ms fade time
	} bias;
	struct bc_s {
		int state;
		vec2_t from;	// [0] = brightness, [1] = contrast
		vec2_t to;
		int startTime;
		int duration;
	} bc;
	struct fx_s {
		int state;
		int fxID;
		vec2_t from;	// [0] = intensity, [1] = brightness
		vec2_t to;
		int startTime;
		int duration;
	} fx;
	struct inv_s {
		int state;
		float from;
		float to;
		int startTime;
		int duration;
	} inv;
} CMInfo_t;

typedef struct {
	vec3_t origin;
	vec3_t angles;
} CamData_Static_t;

typedef struct {
	vec3_t origin;
	int entID;
} CamData_Aimed_t;

typedef struct {
	int offset;
	vec3_t origin;
	vec3_t angles;
	vec3_t li_coeff[2];   //linear interpolation coefficients
} CamData_Linear_Point_t;

typedef struct {
	vec3_t origin;
	int targetType;
	vec3_t targetpos;
	int entID;
	int pointcount;
	int camStartTime;
	CamData_Linear_Point_t* pointData;
} CamData_Linear_t;

typedef enum {
	SPL_CUBIC,
	SPL_BSPLINE,
	SPL_CATMULLROM,
	SPL_LINEAR,
} SplineAlgorithm_e;

typedef struct {
	int offset;
	vec3_t origin;
	vec3_t angles;
	vec4_t csi_coeff[6];   //cubic spline interpolation coefficients
} CamData_Spline_Point_t;

typedef struct {
	vec3_t origin;
	int targetType;
	vec3_t targetpos;
	SplineAlgorithm_e Algorithms[6]; // Algorithms used for each axis
	int entID;
	int pointcount;
	int camStartTime;
	int lastpoint[6];		// Last used point (to optimize lookup time)
	CamData_Spline_Point_t* pointData;
} CamData_Spline_t;

typedef struct {
	vec3_t origin;
	vec3_t angles;
	int camStartTime;
	int startFov;
	int endFov;
	int duration;
	int width;
} CamData_Hitchcock_t;

typedef struct {
	char filename[64];
	int videoHandle;
	qboolean looping;
	// Backup of settings
	float volume;
	float voiceVolume;
	float musicVolume;
} CamData_Video_t;

typedef union {
	CamData_Static_t	CamStatic;
	CamData_Aimed_t		CamAimed;
	CamData_Linear_t	CamLinear;
	CamData_Spline_t	CamSpline;
	CamData_Hitchcock_t CamHitchcock;
	CamData_Video_t		CamVideo;
} CamData_Union_t;

typedef struct {
	int CinActive;
	FadeInfo_t Fade;
	FadeInfo_t Flash;
	FadeInfo_t Motionblur;
	FadeInfo_t Fov;
	BlurInfo_t Blur;
	CMInfo_t CM;
	int CamMode;
	void *CamData;
	int AwaitingData;
	int DataOffset;
} Cinematic_t;

static Cinematic_t cin;

void Cin_SetCamData(int CamMode) {
	switch (cin.CamMode) {
		case CAM_DEFAULT:
			break;
		case CAM_STATIC:
		case CAM_AIMED:
		case CAM_HITCHCOCK:
			free(cin.CamData);
			break;
		case CAM_LINEAR:
			free(((CamData_Linear_t *)cin.CamData)->pointData);
			free(cin.CamData);
			break;
		case CAM_SPLINE:
			free(((CamData_Spline_t *)cin.CamData)->pointData);
			free(cin.CamData);
			break;
		case CAM_VIDEO:
			trap->CIN_StopCinematic(((CamData_Video_t *)cin.CamData)->videoHandle);
			// Revert cvars
			trap->Cvar_Set("r_clear", "0");
			trap->Cvar_Set("r_drawworld", "1");
			trap->Cvar_Set("r_drawentities", "1");

			trap->Cvar_Set("s_volume", va("%f", ((CamData_Video_t *)cin.CamData)->volume));
			trap->Cvar_Set("s_volumevoice", va("%f", ((CamData_Video_t *)cin.CamData)->voiceVolume));
			trap->Cvar_Set("s_musicvolume", va("%f", ((CamData_Video_t *)cin.CamData)->musicVolume));
			cg.cinematicVideo = 0;
			free(cin.CamData);
		default:
			break;
	}
	cin.CamData = 0;

	switch (CamMode) {
		case CAM_DEFAULT:
			break;
		case CAM_STATIC:
			cin.CamData = malloc(sizeof(CamData_Static_t));
			memset(cin.CamData, 0, sizeof(CamData_Static_t));
			break;
		case CAM_AIMED:
			cin.CamData = malloc(sizeof(CamData_Aimed_t));
			memset(cin.CamData, 0, sizeof(CamData_Aimed_t));
			break;
		case CAM_LINEAR:
			cin.CamData = malloc(sizeof(CamData_Linear_t));
			memset(cin.CamData, 0, sizeof(CamData_Linear_t));
			break;
		case CAM_SPLINE:
			cin.CamData = malloc(sizeof(CamData_Spline_t));
			memset(cin.CamData, 0, sizeof(CamData_Spline_t));
			break;
		case CAM_HITCHCOCK:
			cin.CamData = malloc(sizeof(CamData_Hitchcock_t));
			memset(cin.CamData, 0, sizeof(CamData_Hitchcock_t));
			break;
		case CAM_VIDEO:
			cin.CamData = malloc(sizeof(CamData_Video_t));
			memset(cin.CamData, 0, sizeof(CamData_Video_t));
			break;
		default:
			break;
	}
	cin.CamMode = CamMode;
}

void Cin_InitParseBuff(parsebuff_t *pb) {
	memset(pb,0,sizeof(parsebuff_t));
	pb->arg = 1;
	pb->argc = trap->Cmd_Argc();
}

const char *Cin_NextToken(parsebuff_t *pb) {
	if (pb->arg > pb->argc) return NULL;
	trap->Cmd_Argv(pb->arg++,pb->buff, sizeof(pb->buff));
	return pb->buff;
}

qboolean Cin_TokensAvailable(parsebuff_t *pb) {
	if (pb->arg >= pb->argc) return qfalse;
	return qtrue;
}

int Cin_ParseVector(parsebuff_t *pb, vec3_t *vec) {
	const char *token;
	int i;
	for (i=0; i<3; i++) {
		token = Cin_NextToken(pb);
		if (!token) {
			trap->Print("WARNING: ^3Error processing cinematic info: Could not parse vector\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

int Cin_ParseVector2(parsebuff_t *pb, vec2_t *vec) {
	const char *token;
	int i;
	for (i=0; i<2; i++) {
		token = Cin_NextToken(pb);
		if (!token) {
			trap->Print("WARNING: ^3Error processing cinematic info: Could not parse vector2\n");
			return 1;
		}
		(*vec)[i] = atof(token);
	}
	return 0;
}

int Cin_ParseInt(parsebuff_t *pb, int *num) {
	const char *token;
	token = Cin_NextToken(pb);
	if (!token) {
		trap->Print("WARNING: ^3Error processing cinematic info: Could not parse int\n");
		return 1;
	}
	*num = atoi(token);

	return 0;
}

int Cin_ParseFloat(parsebuff_t *pb, float *num) {
	const char *token;
	token = Cin_NextToken(pb);
	
	if (!token) {
		trap->Print("WARNING: ^3Error processing cinematic info: Could not parse float\n");
		return 1;
	}
	*num = atof(token);

	return 0;
}

//#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

int Cin_GetBSplinePoint(int index, int count) {
	if (index < 0)
		return 0;
	if (index > count)
		return count;
	return index;
}

float BSpF[21][5] = { //B-Spline Factors, used for fast subdivision lookup
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

float CMRF[21][5] = { // Catmull-rom Factors, used for fast subdivision lookup
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

float Cin_BSplinePhaseLookup(float x, float p1, float p2, float p3, float p4) {
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

float Cin_CatMullPhaseLookup(float x, float p1, float p2, float p3, float p4) {
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

static float Cin_GetSCamPos(CamData_Spline_t *camdata, int offset, int axis) {
	// Axis: 0 1 2 = x y z, 3 4 5 = pitch yaw roll
	int i;
	int pt;
	float val;
	if (axis < 0 || axis > 5) return 0;
	switch (camdata->Algorithms[axis]) {
		case SPL_CUBIC:
			{
				float ltime;
				// Do selective lookup (saves time ;) )
				i = camdata->lastpoint[axis];
				while (1) {
					if (camdata->pointData[i+1].offset >= offset) {
						if (camdata->pointData[i].offset <= offset) {
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
						if (i == camdata->pointcount-1) {
							// Last point
							pt = i;
							break;
						}
						i++;
					}
				}
				camdata->lastpoint[axis] = pt; // Store this point, so we start looking there next time
				
				ltime = (float)(offset - camdata->pointData[pt].offset)/1000.0f;
				val = camdata->pointData[pt].csi_coeff[axis][0]*ltime*ltime*ltime+camdata->pointData[pt].csi_coeff[axis][1]*ltime*ltime+camdata->pointData[pt].csi_coeff[axis][2]*ltime+camdata->pointData[pt].csi_coeff[axis][3];
				if (axis < 3) {
					val += camdata->pointData[pt].origin[axis];
				} else {
					val += camdata->pointData[pt].angles[axis-3];
				}
				return val;
			}
		case SPL_BSPLINE:
			{
				float phase;
				float mintime;
				float maxtime;
				float t, it, b0, b1, b2, b3;
				// Do selective lookup (saves time ;) )
				i = camdata->lastpoint[axis]; // (-3 to pointcount-1)! (clamped)
				while (1) {
					mintime = BSpF[0][0] * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset +
							  BSpF[0][1] * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset +
							  BSpF[0][2] * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset +
							  BSpF[0][3] * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset;


					maxtime = BSpF[20][0] * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset +
							  BSpF[20][1] * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset +
							  BSpF[20][2] * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset +
							  BSpF[20][3] * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset;
					
					if (offset < mintime) {
						// Go back one part, or clamp to this one if its -3
						if (i == -3) {
							pt = -3;
							break;
						}
						i--;
					} else if ( offset > maxtime) {
						// Go forward another point, or clamp if its pointcount-1
						if (i == camdata->pointcount-1) {
							pt = i;
							break;
						}
						i++;
					} else {
						pt = i;
						break;
					}
				}
				camdata->lastpoint[axis] = pt;
				// Got the point, calculate the phase		
				// We still have the min and max times :p so yeah
				phase = Cin_BSplinePhaseLookup(offset, camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset, 
													   camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset,
													   camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset,
													   camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset);
				t = phase;
			
				// the t value inverted
				it = 1.0f-t;

				// calculate blending functions for cubic bspline
				b0 = it*it*it/6.0f;
				b1 = (3*t*t*t - 6*t*t +4)/6.0f;
				b2 = (-3*t*t*t +3*t*t + 3*t + 1)/6.0f;
				b3 =  t*t*t/6.0f;

				if (axis < 3) {
					val = b0 * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].origin[axis] +
						  b1 * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].origin[axis] +
						  b2 * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].origin[axis] +
						  b3 * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].origin[axis];
				} else {
					val = b0 * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].angles[axis-3] +
						  b1 * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].angles[axis-3] +
						  b2 * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].angles[axis-3] +
						  b3 * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].angles[axis-3];
				}
				return val;
				
			}
		case SPL_CATMULLROM:
			{
				float phase;
				float mintime;
				float maxtime;
				float t, t2, t3, b0, b1, b2, b3;
				// Do selective lookup (saves time ;) )
				i = camdata->lastpoint[axis]; // (-3 to pointcount-1)! (clamped)
				while (1) {
					mintime = CMRF[0][0] * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset +
							  CMRF[0][1] * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset +
							  CMRF[0][2] * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset +
							  CMRF[0][3] * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset;


					maxtime = CMRF[20][0] * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset +
							  CMRF[20][1] * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset +
							  CMRF[20][2] * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset +
							  CMRF[20][3] * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset;
					
					if (offset < mintime) {
						// Go back one part, or clamp to this one if its -2
						if (i == -2) {
							pt = -2;
							break;
						}
						i--;
					} else if ( offset > maxtime) {
						// Go forward another point, or clamp if its pointcount-1
						if (i == camdata->pointcount-1) {
							pt = i;
							break;
						}
						i++;
					} else {
						pt = i;
						break;
					}
				}
				camdata->lastpoint[axis] = pt;
				// Got the point, calculate the phase		
				// We still have the min and max times :p so yeah
				phase = Cin_CatMullPhaseLookup(offset, camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].offset, 
													   camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].offset,
													   camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].offset,
													   camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].offset);
				t = phase;
				t2 = t * t;
				t3 = t2 * t;

				b0 = .5 * (  -t3 + 2*t2 - t);
				b1 = .5 * ( 3*t3 - 5*t2 + 2);
				b2 = .5 * (-3*t3 + 4*t2 + t);
				b3 = .5 * (   t3 -   t2    );

				if (axis < 3) {
					val = b0 * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].origin[axis] +
						  b1 * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].origin[axis] +
						  b2 * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].origin[axis] +
						  b3 * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].origin[axis];
				} else {
					val = b0 * camdata->pointData[Cin_GetBSplinePoint(i,camdata->pointcount-1)].angles[axis-3] +
						  b1 * camdata->pointData[Cin_GetBSplinePoint(i+1,camdata->pointcount-1)].angles[axis-3] +
						  b2 * camdata->pointData[Cin_GetBSplinePoint(i+2,camdata->pointcount-1)].angles[axis-3] +
						  b3 * camdata->pointData[Cin_GetBSplinePoint(i+3,camdata->pointcount-1)].angles[axis-3];
				}
				return val;	
			}
			break;
		case SPL_LINEAR:
			{
				float phase;
				i = camdata->lastpoint[axis];
				while (1) {
					if (camdata->pointData[i+1].offset >= offset) {
						if (camdata->pointData[i].offset <= offset) {
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
						if (i == camdata->pointcount-1) {
							// Last point
							pt = i;
							break;
						}
						i++;
					}
				}
				camdata->lastpoint[axis] = pt; // Store this point, so we start looking there next time
				if (pt == camdata->pointcount - 1) {
					// Last point, dont bother to interpolate
					if (axis < 3) {
						val = camdata->pointData[pt].origin[axis];
					} else {
						val = camdata->pointData[pt].angles[axis-3];
					}
					return val;
				}
				// Interpolate it nicely then :)
				if (camdata->pointData[pt].offset == camdata->pointData[pt+1].offset) {
					phase = 0;
				} else {
					phase = (float)(offset - camdata->pointData[pt].offset) / (float)(camdata->pointData[pt+1].offset - camdata->pointData[pt].offset);
				}
				if (axis < 3) {
					val = (camdata->pointData[pt].origin[axis] * (1-phase)) + (camdata->pointData[pt+1].origin[axis] * phase);
				} else {
					val = (camdata->pointData[pt].angles[axis-3] * (1-phase)) + (camdata->pointData[pt+1].angles[axis-3] * phase);
				}
				return val;
			}
			break;
		default:
			return 0;
	}
}

float Cin_ProcessFOV();

int Cin_ProcessCamera() {
	// Returns 1 if we're using a non-default cam
	CamData_Union_t *CamData = (CamData_Union_t *)cin.CamData;
	centity_t *ent;
	vec3_t temp;
	int i;
	int pt = 0;
	float ltime;

	if (!cin.CamMode) return 0;
	switch (cin.CamMode) {
		case CAM_STATIC:
			VectorCopy(CamData->CamStatic.origin, cg.refdef.vieworg);
			VectorCopy(CamData->CamStatic.angles, cg.refdef.viewangles);
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
		case CAM_AIMED:
			VectorCopy(CamData->CamAimed.origin, cg.refdef.vieworg);
			if (CamData->CamAimed.entID == cg.predictedPlayerState.clientNum) {
				VectorSubtract(cg.predictedPlayerState.origin, cg.refdef.vieworg, temp);
				vectoangles(temp, cg.refdef.viewangles);
			} else {
				ent = &cg_entities[CamData->CamAimed.entID];
				if (ent->currentValid) {
					VectorSubtract(ent->lerpOrigin, cg.refdef.vieworg, temp);
					vectoangles(temp, cg.refdef.viewangles);				
				}
			}
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
		case CAM_LINEAR:
			// Oki this one's a bit more tricky
			// First determine which point we're on
			if (CamData->CamLinear.camStartTime + CamData->CamLinear.pointData[CamData->CamLinear.pointcount-1].offset <= cg.time) {
				pt = CamData->CamLinear.pointcount-1;
			} else {
				for (i=0; i<CamData->CamLinear.pointcount-1; i++) {
					if (CamData->CamLinear.camStartTime + CamData->CamLinear.pointData[i+1].offset > cg.time) {
						pt = i;
						break;
					}
				}
			}
			ltime = (float)(cg.time - (CamData->CamLinear.camStartTime + CamData->CamLinear.pointData[pt].offset))/1000.0f;
			VectorScale(CamData->CamLinear.pointData[pt].li_coeff[0], ltime, cg.refdef.vieworg);
			VectorAdd(CamData->CamLinear.pointData[pt].origin, cg.refdef.vieworg, cg.refdef.vieworg);
			if (CamData->CamLinear.targetType == 0) {
				VectorScale(CamData->CamLinear.pointData[pt].li_coeff[1], ltime, cg.refdef.viewangles);
				VectorAdd(CamData->CamLinear.pointData[pt].angles, cg.refdef.viewangles, cg.refdef.viewangles);
			} else if (CamData->CamLinear.targetType == 1) {
				if (CamData->CamLinear.entID == cg.predictedPlayerState.clientNum) {
					VectorSubtract(cg.predictedPlayerState.origin, cg.refdef.vieworg, temp);
					vectoangles(temp, cg.refdef.viewangles);
				} else {
					ent = &cg_entities[CamData->CamLinear.entID];
					if (ent->currentValid) {
						VectorSubtract(ent->lerpOrigin, cg.refdef.vieworg, temp);
						vectoangles(temp, cg.refdef.viewangles);				
					}
				}
			} else if (CamData->CamLinear.targetType == 2) {
				VectorSubtract(CamData->CamLinear.targetpos, cg.refdef.vieworg, temp);
				vectoangles(temp, cg.refdef.viewangles);
			} else {
				// Bad type.. dont error for now..
			}
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
		case CAM_SPLINE:
			// Even more tricky..
			// First determine which point we're on
			/*
			if (CamData->CamSpline.camStartTime + CamData->CamSpline.pointData[CamData->CamSpline.pointcount-1].offset <= cg.time) {
				pt = CamData->CamSpline.pointcount-1;
			} else {
				for (i=0; i<CamData->CamSpline.pointcount-1; i++) {
					if (CamData->CamSpline.camStartTime + CamData->CamSpline.pointData[i+1].offset >= cg.time) {
						pt = i;
						break;
					}
				}
			}
			ltime = (float)(cg.time - (CamData->CamSpline.camStartTime + CamData->CamSpline.pointData[pt].offset))/1000.0f;
			
			cg.refdef.vieworg[0] = CamData->CamSpline.pointData[pt].csi_coeff[0][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[0][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[0][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[0][3];
			cg.refdef.vieworg[1] = CamData->CamSpline.pointData[pt].csi_coeff[1][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[1][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[1][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[1][3];
			cg.refdef.vieworg[2] = CamData->CamSpline.pointData[pt].csi_coeff[2][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[2][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[2][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[2][3];
			VectorAdd(CamData->CamSpline.pointData[pt].origin, cg.refdef.vieworg, cg.refdef.vieworg);
			if (CamData->CamSpline.targetType == 0) {
				cg.refdef.viewangles[0] = CamData->CamSpline.pointData[pt].csi_coeff[3][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[3][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[3][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[3][3];
				cg.refdef.viewangles[1] = CamData->CamSpline.pointData[pt].csi_coeff[4][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[4][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[4][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[4][3];
				cg.refdef.viewangles[2] = CamData->CamSpline.pointData[pt].csi_coeff[5][0]*ltime*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[5][1]*ltime*ltime+CamData->CamSpline.pointData[pt].csi_coeff[5][2]*ltime+CamData->CamSpline.pointData[pt].csi_coeff[5][3];
				VectorAdd(CamData->CamSpline.pointData[pt].angles, cg.refdef.viewangles, cg.refdef.viewangles);
			} else if (CamData->CamSpline.targetType == 1) {
				if (CamData->CamLinear.entID == cg.predictedPlayerState.clientNum) {
					VectorSubtract(cg.predictedPlayerState.origin, cg.refdef.vieworg, temp);
					vectoangles(temp, cg.refdef.viewangles);
				} else {
					ent = &cg_entities[CamData->CamSpline.entID];
					if (ent->currentValid) {
						VectorSubtract(ent->lerpOrigin, cg.refdef.vieworg, temp);
						vectoangles(temp, cg.refdef.viewangles);				
					}
				}
			} else if (CamData->CamSpline.targetType == 2) {
				VectorSubtract(CamData->CamSpline.targetpos, cg.refdef.vieworg, temp);
				vectoangles(temp, cg.refdef.viewangles);
			} else {
				// Bad type.. dont error for now..
			}
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
			*/
			cg.refdef.vieworg[0] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 0);
			cg.refdef.vieworg[1] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 1);
			cg.refdef.vieworg[2] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 2);
			if (CamData->CamSpline.targetType == 0) {
				cg.refdef.viewangles[0] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 3);
				cg.refdef.viewangles[1] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 4);
				cg.refdef.viewangles[2] = Cin_GetSCamPos(&CamData->CamSpline, cg.time - CamData->CamSpline.camStartTime, 5);
			} else if (CamData->CamSpline.targetType == 1) {
				if (CamData->CamLinear.entID == cg.predictedPlayerState.clientNum) {
					VectorSubtract(cg.predictedPlayerState.origin, cg.refdef.vieworg, temp);
					vectoangles(temp, cg.refdef.viewangles);
				} else {
					ent = &cg_entities[CamData->CamSpline.entID];
					if (ent->currentValid) {
						VectorSubtract(ent->lerpOrigin, cg.refdef.vieworg, temp);
						vectoangles(temp, cg.refdef.viewangles);				
					}
				}
			} else if (CamData->CamSpline.targetType == 2) {
				VectorSubtract(CamData->CamSpline.targetpos, cg.refdef.vieworg, temp);
				vectoangles(temp, cg.refdef.viewangles);
			} else {
				// Bad type.. dont error for now..
			}
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
		case CAM_HITCHCOCK:
			{
				float cfov = Cin_ProcessFOV(); // Lil cheat to get the fov quickly
				// What we do now, is check the difference in the c(urrent)fov and startfov
				// and reposition the camera accordingly
				float dist1 = (CamData->CamHitchcock.width/(2*tan(PI * ((float)CamData->CamHitchcock.startFov / 400.0f))));
				float dist2 = (CamData->CamHitchcock.width/(2*tan(PI * (cfov / 400.0f))));
				float movedist = dist1-dist2;
				vec3_t vec, org2, dummy;
				AngleVectors(CamData->CamHitchcock.angles, vec, dummy, dummy);
				VectorScale(vec, movedist, org2);
				VectorAdd(CamData->CamHitchcock.origin, org2, org2);
				VectorCopy(org2, cg.refdef.vieworg);
				VectorCopy(CamData->CamHitchcock.angles, cg.refdef.viewangles);
				AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			}
			break;
		case CAM_VIDEO:
			// Place the camera in the void when playing back video
			cg.refdef.vieworg[0] = cg.refdef.vieworg[1] = cg.refdef.vieworg[2] = 70000;
			cg.refdef.viewangles[0] = cg.refdef.viewangles[1] = cg.refdef.viewangles[2] = 0;
			AnglesToAxis(cg.refdef.viewangles, cg.refdef.viewaxis);
			break;
		default:
			break;
	}
	return 1;
}

void Cin_ProcessFlash() {
	// Processes flash ins and flash outs
	vec4_t cincolor;
	cincolor[0] = cincolor[1] = cincolor[2] = 1;
	if (cin.Flash.state == 0) return; // Off
	if (cin.Flash.state == 2) { // Faded white
		cincolor[3] = 1;
		trap->R_SetColor(&cincolor[0]);
		trap->R_DrawStretchPic(0,0,640,480,0,0,0,0,cgs.media.whiteShader);
		trap->R_SetColor(NULL);
		return;
	}
	if (cin.Flash.state == 1) {
		float alpha;
		if (cg.time > cin.Flash.startTime + cin.Flash.duration) {
			if (cin.Flash.to == 100) {
				cin.Flash.state = 2;
				alpha = 1;
			} else {
				cin.Flash.state = 0;
				alpha = 0;
			}
		} else if (cg.time < cin.Flash.startTime) {
			alpha = (float)cin.Flash.from / 100.f;
		} else {
			alpha = ((float)(cg.time - cin.Flash.startTime) / cin.Flash.duration);
			alpha = ((cin.Flash.to * alpha) + (cin.Flash.from * (1-alpha))) / 100;
		}
		cincolor[3] = alpha;
		trap->R_SetColor(&cincolor[0]);
		trap->R_DrawStretchPic(0,0,640,480,0,0,0,0,cgs.media.whiteShader);
		trap->R_SetColor(NULL);
		return;
	}

	trap->Error(ERR_DROP, va("ERROR: Cin_ProcessFlash: Invalid state %i", cin.Flash.state));
}

void Cin_ProcessFade() {
	// Processes fade ins and fade outs
	vec4_t cincolor;
	cincolor[0] = cincolor[1] = cincolor[2] = 0;
	if (cin.Fade.state == 0) return; // Off
	if (cin.Fade.state == 2) { // Faded black
		cincolor[3] = 1;
		trap->R_SetColor(&cincolor[0]);
		trap->R_DrawStretchPic(0,0,640,480,0,0,0,0,cgs.media.whiteShader);
		trap->R_SetColor(NULL);
		return;
	}
	if (cin.Fade.state == 1) {
		float alpha;
		if (cg.time > cin.Fade.startTime + cin.Fade.duration) {
			if (cin.Fade.to == 100) {
				cin.Fade.state = 2;
				alpha = 1;
			} else {
				cin.Fade.state = 0;
				alpha = 0;
			}
		} else if (cg.time < cin.Fade.startTime) {
			alpha = (float)cin.Fade.from / 100.f;
		} else {
			alpha = ((float)(cg.time - cin.Fade.startTime) / cin.Fade.duration);
			alpha = ((cin.Fade.to * alpha) + (cin.Fade.from * (1-alpha))) / 100;
		}
		cincolor[3] = alpha;
		trap->R_SetColor(&cincolor[0]);
		trap->R_DrawStretchPic(0,0,640,480,0,0,0,0,cgs.media.whiteShader);
		trap->R_SetColor(NULL);
		return;
	}

	trap->Error(ERR_DROP, va("ERROR: Cin_ProcessFade: Invalid state %i", cin.Fade.state));
}

int Cin_ProcessMB() {
	// Returns the motion blur level to use
	if (!cin.Motionblur.state) return 0;
	if (cin.Motionblur.state == 2) {
		// Static
		return cin.Motionblur.from;
	}
	if (cin.Motionblur.state == 1) {
		// Fading
		float fade;
		if (cg.time > cin.Motionblur.startTime + cin.Motionblur.duration) {
			if (cin.Motionblur.to == 0) {
				cin.Motionblur.state = 0;
				return 0;
			} else {
				cin.Motionblur.state = 2;
				cin.Motionblur.from = cin.Motionblur.to;
				return cin.Motionblur.to;
			}
		} else if (cg.time < cin.Motionblur.startTime) {
			fade = (float)cin.Motionblur.from;
		} else {
			fade = ((float)(cg.time - cin.Motionblur.startTime) / cin.Motionblur.duration);
			fade = ((cin.Motionblur.to * fade) + (cin.Motionblur.from * (1-fade)));
		}
		return (int)fade;
	}
	trap->Error(ERR_DROP, va("ERROR: Cin_ProcessMB: Invalid state %i", cin.Motionblur.state));
	return 0;
}

void Cin_ProcessVideo() {
	CamData_Video_t *data;

	if (cin.CamMode != CAM_VIDEO) {
		return;
	}
	data = (CamData_Video_t *)cin.CamData;

	trap->CIN_RunCinematic(data->videoHandle);
	trap->CIN_DrawCinematic(data->videoHandle);
}

float Cin_ProcessFOV() {
	// Returns the motion blur level to use
	if (!cin.Fov.state) return cg_fov.value;
	if (cin.Fov.state == 2) {
		// Static
		return cin.Fov.from;
	}
	if (cin.Fov.state == 1) {
		// Fading
		float fade;
		if (cg.time > cin.Fov.startTime + cin.Fov.duration) {
			if (cin.Fov.to == cg_fov.integer) {
				cin.Fov.state = 0;
				return cg_fov.integer;
			} else {
				cin.Fov.state = 2;
				cin.Fov.from = cin.Fov.to;
				return cin.Fov.to;
			}
		} else if (cg.time < cin.Fov.startTime) {
			fade = (float)cin.Fov.from;
		} else {
			fade = ((float)(cg.time - cin.Fov.startTime) / cin.Fov.duration);
			fade = ((cin.Fov.to * fade) + (cin.Fov.from * (1-fade)));
		}
		return fade;
	}

	trap->Error(ERR_DROP, va("ERROR: Cin_ProcessFOV: Invalid state %i", cin.Fov.state));

	return cg_fov.integer;
}

#define Vector2Copy(a,b)					((b)[0]=(a)[0],(b)[1]=(a)[1])

static void Cin_DoLinearInterpolation();
static void Cin_DoCubicSplineInterpolation();

static void Cin_Reset() {
	cin.CinActive = 0;
	cin.AwaitingData = 0;
	Cin_SetCamData(CAM_DEFAULT);

	cin.Motionblur.state = 0;
	cin.Fov.state = 0;
	memset(&cin.CM, 0, sizeof(cin.CM));
	cin.Fade.state = 0;
	cin.Flash.state = 0;
	if (cg.trapEscape) {
		cg.trapEscape = 0;
		uiImports->SetEscapeTrap(qfalse);
	}
}

enum {
	CIN_ACT_FADEIN = 1,
	CIN_ACT_FADEOUT = 2,
	CIN_ACT_STATICCAM = 3,
	CIN_ACT_AIMEDCAM = 4,
	CIN_ACT_VIDEO = 5,
	CIN_ACT_LINEARCAM = 6,
	CIN_ACT_SPLINECAM = 7,
	CIN_ACT_MOTIONBLUR = 8,
	CIN_ACT_FOV = 9,

	CIN_ACT_RESTORE = 11,
	

	CIN_ACT_HITCHCOCKEFFECT = 14,
	CIN_ACT_CAMSHAKE = 15,
	
	CIN_ACT_BLUR = 17,
	CIN_ACT_COLORMOD_SCALE = 18,
	CIN_ACT_COLORMOD_BIAS = 19,
	CIN_ACT_COLORMOD_BC = 20,
	CIN_ACT_COLORMOD_FX = 21,
	CIN_ACT_COLORMOD_INV = 22,

	CIN_ACT_CAPTION = 24,
	CIN_ACT_FLASHIN = 25,
	CIN_ACT_FLASHOUT = 26,

	CIN_ACT_CONTDATA = 31,
	CIN_ACT_END = 0,
};

int BitStream_ReadTime(bitstream_t *stream) {	// Time is normally sent in 10 bits, with a resolution of 50 msecs
	return (int)BitStream_ReadBits(stream, 10) * 50;
}

int BitStream_ReadTimeHR(bitstream_t *stream) {	// HR (Higher Resolution) Time is sent in 11 bits, giving it a resolution of 25 msec
	return (int)BitStream_ReadBits(stream, 11) * 25;
}

float BitStream_ReadCoord(bitstream_t *stream) {
	int temp = (int)BitStream_ReadBits(stream, -19);
	return (float)temp / 4.0f;
}

void BitStream_ReadVector(bitstream_t *stream, vec3_t *vector) {
	(*vector)[0] = BitStream_ReadCoord(stream);
	(*vector)[1] = BitStream_ReadCoord(stream);
	(*vector)[2] = BitStream_ReadCoord(stream);
}

static float Cin_GetCvarValue(const char *cvar_name) 
{
	char buffer[1024] = {0};
	trap->Cvar_VariableStringBuffer(cvar_name, buffer, sizeof(buffer));
	return atof(buffer);
}

void Cin_ProcessCinematicBinary_f() {
	// Big one, this processes ALL cinematic commands (bitstream format)
	// Since this is a binary stream, the stream is assumed to be valid, so no error checks are performed.
	char arg[1024] = {0};
	char data[840];
	unsigned int temp;
	unsigned int len;

	int i;
	CamData_Union_t *CamData;
	bitstream_t stream;

	trap->Cmd_Argv(1, arg, 1024);

	len = Base128_DecodeLength((unsigned int)strlen(arg));
	Base128_Decode((const char *)arg, (unsigned int)strlen(arg), (void *)data, (unsigned int)840);


	BitStream_Init(&stream, (unsigned char *)data, len);
	BitStream_BeginReading(&stream);

	for(;;) {
		switch (BitStream_ReadBits(&stream, 5)) // Get the next act
		{
		case CIN_ACT_END:
			// End of message
			return;
		case CIN_ACT_FADEIN:
			cin.Fade.startTime = cg.time;
			cin.Fade.from = 100;
			cin.Fade.to = 0; // in percentage scale (alpha)
			cin.Fade.duration = BitStream_ReadTime(&stream);
			cin.Fade.state = 1; // Fading
			break;
		case CIN_ACT_FADEOUT:
			cin.Fade.startTime = cg.time;
			cin.Fade.from = 0;
			cin.Fade.to = 100; // in percentage scale (alpha)
			cin.Fade.duration = BitStream_ReadTime(&stream);
			cin.Fade.state = 1; // Fading
			break;
		case CIN_ACT_STATICCAM:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_STATIC);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			BitStream_ReadVector(&stream, &CamData->CamStatic.origin);
			BitStream_ReadVector(&stream, &CamData->CamStatic.angles);
			break;
		case CIN_ACT_AIMEDCAM:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_AIMED);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamAimed.entID = BitStream_ReadBits(&stream, 10);
			BitStream_ReadVector(&stream, &CamData->CamAimed.origin);
			break;
		case CIN_ACT_LINEARCAM:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_LINEAR);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamLinear.camStartTime = cg.time;
			if (BitStream_ReadBool(&stream)) { // Targetted?
				CamData->CamLinear.targetType = 1 + BitStream_ReadBool(&stream); // Check target type (explicit vs entity)
			} else {
				CamData->CamLinear.targetType = 0;
			}

			// Read additional parameters for targetted cameras
			switch (CamData->CamLinear.targetType)
			{
			case 1:
				CamData->CamLinear.entID = BitStream_ReadBits(&stream, 10);
				break;
			case 2:
				BitStream_ReadVector(&stream, &CamData->CamLinear.targetpos);
				break;
			}

			// Get amount of points
			CamData->CamLinear.pointcount = BitStream_ReadByte(&stream);

			// Alloc a buffer for the points
			CamData->CamLinear.pointData = (CamData_Linear_Point_t *)malloc(sizeof(CamData_Linear_Point_t) * CamData->CamLinear.pointcount);
			memset(CamData->CamLinear.pointData, 0, sizeof(CamData_Linear_Point_t) * CamData->CamLinear.pointcount);
			
			// Check for fragmentation
			if (BitStream_ReadBool(&stream)) {
				// Message is fragmented, check how many points we can read from this message
				len = BitStream_ReadByte(&stream);
			} else {
				len = CamData->CamLinear.pointcount;
			}

			temp = 0;
			// Start parsing the cam points
			for (i=0; i< len; i++) {
				CamData->CamLinear.pointData[i].offset = BitStream_ReadTimeHR(&stream) + temp;
				temp = CamData->CamLinear.pointData[i].offset;

				BitStream_ReadVector(&stream, &CamData->CamLinear.pointData[i].origin);
				
				if (!CamData->CamLinear.targetType) {
					BitStream_ReadVector(&stream, &CamData->CamLinear.pointData[i].angles);
				}
			}
			if (len != CamData->CamLinear.pointcount) { // This message was fragmented, stop processing and wait for the rest
				cin.AwaitingData = 1;
				cin.DataOffset = i;
				return;
			}
			Cin_DoLinearInterpolation();
			break;
		case CIN_ACT_SPLINECAM:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_SPLINE);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamSpline.camStartTime = cg.time;

			if (BitStream_ReadBool(&stream)) { // Targetted?
				CamData->CamSpline.targetType = 1 + BitStream_ReadBool(&stream); // Check target type (explicit vs entity)
			} else {
				CamData->CamSpline.targetType = 0;
			}
			// Read additional parameters for targetted cameras
			switch (CamData->CamSpline.targetType)
			{
			case 1:
				CamData->CamSpline.entID = BitStream_ReadBits(&stream, 10);
				break;
			case 2:
				BitStream_ReadVector(&stream, &CamData->CamSpline.targetpos);
				break;
			}

			// Get amount of points
			CamData->CamSpline.pointcount = BitStream_ReadByte(&stream);

			CamData->CamSpline.Algorithms[0] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);
			CamData->CamSpline.Algorithms[1] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);
			CamData->CamSpline.Algorithms[2] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);
			CamData->CamSpline.Algorithms[3] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);
			CamData->CamSpline.Algorithms[4] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);
			CamData->CamSpline.Algorithms[5] = (SplineAlgorithm_e)BitStream_ReadBits(&stream, 2);

			// Alloc a buffer for the points
			CamData->CamSpline.pointData = (CamData_Spline_Point_t *)malloc(sizeof(CamData_Spline_Point_t) * CamData->CamSpline.pointcount);
			memset(CamData->CamSpline.pointData, 0, sizeof(CamData_Spline_Point_t) * CamData->CamSpline.pointcount);
			
			// Check for fragmentation
			if (BitStream_ReadBool(&stream)) {
				// Message is fragmented, check how many points we can read from this message
				len = BitStream_ReadByte(&stream);
			} else {
				len = CamData->CamSpline.pointcount;
			}

			temp = 0;
			// Start parsing the cam points
			for (i=0; i< len; i++) {
				CamData->CamSpline.pointData[i].offset = BitStream_ReadTimeHR(&stream) + temp;
				temp = CamData->CamSpline.pointData[i].offset;

				BitStream_ReadVector(&stream, &CamData->CamSpline.pointData[i].origin);
				
				if (!CamData->CamSpline.targetType) {
					BitStream_ReadVector(&stream, &CamData->CamSpline.pointData[i].angles);
				}
			}
			if (len != CamData->CamSpline.pointcount) { // This message was fragmented, stop processing and wait for the rest
				cin.AwaitingData = 1;
				cin.DataOffset = i;
				return;
			}
			
			for (i=0; i<6; i++) {
				if (CamData->CamSpline.Algorithms[i] == 0) {
					Cin_DoCubicSplineInterpolation();
					break;
				}
			}
			break;
		case CIN_ACT_MOTIONBLUR:
			if (BitStream_ReadBool(&stream)) { // Are we fading?
				cin.Motionblur.state = 1;
				cin.Motionblur.startTime = cg.time;
				cin.Motionblur.from = BitStream_ReadTimeHR(&stream);	// From and to values are sent as HR time values
				cin.Motionblur.to = BitStream_ReadTimeHR(&stream);
				cin.Motionblur.duration = BitStream_ReadTime(&stream);
			} else {
				cin.Motionblur.state = 2;
				cin.Motionblur.from = BitStream_ReadTimeHR(&stream);
				if (cin.Motionblur.from == 0) { // turn it off
					cin.Motionblur.state = 0;
				}
			}
			break;
		case CIN_ACT_FOV:
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.Fov.state = 1;
				cin.Fov.startTime = cg.time;
				cin.Fov.from = (int)BitStream_ReadBits(&stream, -9);
				if (cin.Fov.from < 0) cin.Fov.from = cg_fov.integer;

				cin.Fov.to = (int)BitStream_ReadBits(&stream, -9);
				if (cin.Fov.to < 0) cin.Fov.to = cg_fov.integer;

				cin.Fov.duration = BitStream_ReadTime(&stream);

			} else {
				cin.Fov.state = 2;
				cin.Fov.from = (int)BitStream_ReadBits(&stream, -9);
				if (cin.Fov.from < 0) { // turn it off
					cin.Fov.state = 0;
				}
			}

			break;

		case CIN_ACT_RESTORE:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_DEFAULT);
			break;
		case CIN_ACT_HITCHCOCKEFFECT:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_HITCHCOCK);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamHitchcock.camStartTime = cg.time;
			BitStream_ReadVector(&stream, &CamData->CamHitchcock.origin);
			BitStream_ReadVector(&stream, &CamData->CamHitchcock.angles);

			CamData->CamHitchcock.width = BitStream_ReadUShort(&stream);
			CamData->CamHitchcock.startFov = (int)BitStream_ReadBits(&stream, -9);
			if (CamData->CamHitchcock.startFov == -1) CamData->CamHitchcock.startFov = cg_fov.integer;
			CamData->CamHitchcock.endFov = (int)BitStream_ReadBits(&stream, -9);
			if (CamData->CamHitchcock.endFov == -1) CamData->CamHitchcock.endFov = cg_fov.integer;
			
			CamData->CamHitchcock.duration = BitStream_ReadTime(&stream);
			// enable FOV while we're at it
			cin.Fov.state = 1;
			cin.Fov.startTime = cg.time;
			cin.Fov.from = CamData->CamHitchcock.startFov;
			cin.Fov.to = CamData->CamHitchcock.endFov;
			cin.Fov.duration = CamData->CamHitchcock.duration;
			break;
		case CIN_ACT_VIDEO:
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_VIDEO);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamVideo.looping = BitStream_ReadBool(&stream);
			BitStream_ReadString(&stream, CamData->CamVideo.filename, 64);

			CamData->CamVideo.videoHandle = trap->CIN_PlayCinematic(CamData->CamVideo.filename, 0, 0, 640, 480, (CamData->CamVideo.looping ? CIN_loop : CIN_hold) | CIN_aspect);

			// Adjust cvars to prepare the game for video rendering
			trap->Cvar_Set("r_clear", "1");
			trap->Cvar_Set("r_drawworld", "0");
			trap->Cvar_Set("r_drawentities", "0");

			CamData->CamVideo.volume = Cin_GetCvarValue("s_volume");
			CamData->CamVideo.voiceVolume = Cin_GetCvarValue("s_volumevoice");
			CamData->CamVideo.musicVolume = Cin_GetCvarValue("s_musicvolume");
			trap->Cvar_Set("s_volume", "0");
			trap->Cvar_Set("s_volumevoice", "0");
			trap->Cvar_Set("s_musicvolume", "0");
			cg.cinematicVideo = 1;

			break;
		case CIN_ACT_CAMSHAKE:
		case CIN_ACT_BLUR:
			break; // NOT IMPLEMENTED YET
		case CIN_ACT_COLORMOD_SCALE:
			if (BitStream_ReadBool(&stream)) { // Disable?
				cin.CM.scale.state = 0;
				break;
			}
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.CM.scale.state = 1;
				cin.CM.scale.startTime = cg.time;
				cin.CM.scale.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.from[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.scale.to[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.to[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.to[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.scale.duration = BitStream_ReadTime(&stream);

			} else {
				cin.CM.scale.state = 2;

				cin.CM.scale.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.scale.from[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				if (cin.CM.scale.from[0] == 1 && cin.CM.scale.from[1] == 1 && cin.CM.scale.from[2] == 1) {
					cin.CM.scale.state = 0;
				}
			}
			break;
		case CIN_ACT_COLORMOD_BIAS:
			if (BitStream_ReadBool(&stream)) { // Disable?
				cin.CM.bias.state = 0;
				break;
			}
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.CM.bias.state = 1;
				cin.CM.bias.startTime = cg.time;
				cin.CM.bias.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.from[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.bias.to[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.to[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.to[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.bias.duration = BitStream_ReadTime(&stream);

			} else {
				cin.CM.bias.state = 2;

				cin.CM.bias.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.bias.from[2] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				if (cin.CM.bias.from[0] == 1 && cin.CM.bias.from[1] == 1 && cin.CM.bias.from[2] == 1) {
					cin.CM.bias.state = 0;
				}
			}
			break;
		case CIN_ACT_COLORMOD_BC:
			if (BitStream_ReadBool(&stream)) { // Disable?
				cin.CM.bc.state = 0;
				break;
			}
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.CM.bc.state = 1;
				cin.CM.bc.startTime = cg.time;
				cin.CM.bc.from[0] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;
				cin.CM.bc.from[1] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;

				cin.CM.bc.to[0] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;
				cin.CM.bc.to[1] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;

				cin.CM.bc.duration = BitStream_ReadTime(&stream);

			} else {
				cin.CM.bc.state = 2;

				cin.CM.bc.from[0] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;
				cin.CM.bc.from[1] = (float)((int)BitStream_ReadBits(&stream, -13)) / 100.0;

				if (cin.CM.bc.from[0] == 0 && cin.CM.bc.from[1] == 1) {
					cin.CM.bc.state = 0;
				}
			}
			break;
		case CIN_ACT_COLORMOD_FX:
			if (BitStream_ReadBool(&stream)) { // Disable?
				cin.CM.fx.state = 0;
				break;
			}
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.CM.fx.state = 1;
				cin.CM.fx.startTime = cg.time;

				cin.CM.fx.fxID = BitStream_ReadBits(&stream, 2);

				cin.CM.fx.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.fx.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.fx.to[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.fx.to[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				cin.CM.fx.duration = BitStream_ReadTime(&stream);

			} else {
				cin.CM.fx.state = 2;

				cin.CM.fx.fxID = BitStream_ReadBits(&stream, 2);

				cin.CM.fx.from[0] = (float)BitStream_ReadBits(&stream, 12) / 100.0;
				cin.CM.fx.from[1] = (float)BitStream_ReadBits(&stream, 12) / 100.0;

				if (cin.CM.fx.fxID == 0 || cin.CM.fx.from[0] == 0) {
					cin.CM.fx.state = 0;
				}
			}
			break;
		case CIN_ACT_COLORMOD_INV:
			if (BitStream_ReadBool(&stream)) { // Disable?
				cin.CM.inv.state = 0;
				break;
			}
			if (BitStream_ReadBool(&stream)) { // Fading?
				cin.CM.inv.state = 1;
				cin.CM.inv.startTime = cg.time;

				cin.CM.inv.from = (float)BitStream_ReadBits(&stream, 7) / 100.0;
				
				cin.CM.inv.to = (float)BitStream_ReadBits(&stream, 7) / 100.0;

				cin.CM.inv.duration = BitStream_ReadTime(&stream);

			} else {
				cin.CM.inv.state = 2;

				cin.CM.inv.from = (float)BitStream_ReadBits(&stream, 7) / 100.0;

				if (cin.CM.inv.from == 0) {
					cin.CM.inv.state = 0;
				}
			}
			break;
		case CIN_ACT_CAPTION:
			break; // NOT IMPLEMENTED YET
		case CIN_ACT_FLASHIN:
			cin.Flash.startTime = cg.time;
			cin.Flash.from = 100;
			cin.Flash.to = 0; // in percentage scale (alpha)
			cin.Flash.duration = BitStream_ReadTime(&stream);
			cin.Flash.state = 1; // Fading
			break;
		case CIN_ACT_FLASHOUT:
			cin.Flash.startTime = cg.time;
			cin.Flash.from = 0;
			cin.Flash.to = 100; // in percentage scale (alpha)
			cin.Flash.duration = BitStream_ReadTime(&stream);
			cin.Flash.state = 1; // Fading
			break;
		case CIN_ACT_CONTDATA:
			if (!cin.AwaitingData) {
				trap->Print("WARNING: ^3Error processing cinematic info: Cont received without awaiting further data\n");
				return;
			}
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			cin.AwaitingData = 0;

			// Check for fragmentation
			if (BitStream_ReadBool(&stream)) {
				// Message is fragmented, check how many points we can read from this message
				len = cin.DataOffset + BitStream_ReadByte(&stream);
			} else {
				len = -1;
			}

			if (cin.CamMode == CAM_LINEAR) {
				if (len == -1) { len = CamData->CamLinear.pointcount; }
				temp = CamData->CamLinear.pointData[cin.DataOffset - 1].offset;

				for (i=cin.DataOffset; i < len; i++) {
					CamData->CamLinear.pointData[i].offset = BitStream_ReadTimeHR(&stream) + temp;
					temp = CamData->CamLinear.pointData[i].offset;

					BitStream_ReadVector(&stream, &CamData->CamLinear.pointData[i].origin);
					
					if (!CamData->CamLinear.targetType) {
						BitStream_ReadVector(&stream, &CamData->CamLinear.pointData[i].angles);
					}
				}
				if (len != CamData->CamLinear.pointcount) { // This message was fragmented, stop processing and wait for the rest
					cin.AwaitingData = 1;
					cin.DataOffset = i;
					return;
				}
				Cin_DoLinearInterpolation();
			} else if (cin.CamMode == CAM_SPLINE) {
				if (len == -1) { len = CamData->CamSpline.pointcount; }
				temp = CamData->CamSpline.pointData[cin.DataOffset - 1].offset;

				for (i=cin.DataOffset; i < len; i++) {
					CamData->CamSpline.pointData[i].offset = BitStream_ReadTimeHR(&stream) + temp;
					temp = CamData->CamSpline.pointData[i].offset;

					BitStream_ReadVector(&stream, &CamData->CamSpline.pointData[i].origin);
					
					if (!CamData->CamSpline.targetType) {
						BitStream_ReadVector(&stream, &CamData->CamSpline.pointData[i].angles);
					}
				}
				if (len != CamData->CamSpline.pointcount) { // This message was fragmented, stop processing and wait for the rest
					cin.AwaitingData = 1;
					cin.DataOffset = i;
					return;
				}
				for (i=0; i<6; i++) {
					if (CamData->CamSpline.Algorithms[i] == 0) {
						Cin_DoCubicSplineInterpolation();
						break;
					}
				}
			}
			break;

		default:
			// Should never happen
			trap->Print("WARNING: ^3Error processing cinematic info: Invalid act code\n");
			return;
		}
	}
}

void Cin_ProcessCinematic_f() {
	// Big one, this processes ALL cinematic commands
	parsebuff_t pb;
	const char *token;
	int i;
	CamData_Union_t *CamData;
	Cin_InitParseBuff(&pb);
	

	while (1) {
		token = Cin_NextToken(&pb);
		if (!token) break;

		if (!Q_stricmp(token,"start")) {
			if (!cin.CinActive) {
				Cin_Reset();
				// Start cinematic mode
				cg.cinematicState = 1;
				cg.cinematicTime = cg.time;
				cin.CinActive = 1;
			}
			continue;
		}
		if (!Q_stricmp(token,"stop")) {
			// Stop cinematic mode
			cg.cinematicState = 3;
			cg.cinematicTime = cg.time;
			Cin_Reset();
			continue;
		}
		if (!Q_stricmp(token,"ae")) {
			// Allow use of escape key
			cg.trapEscape = 1;
			uiImports->SetEscapeTrap( qfalse );
			continue;
		}
		if (!Q_stricmp(token,"de")) {
			// Disallow use of escape key
			cg.trapEscape = 0;
			uiImports->SetEscapeTrap( qfalse );
			continue;
		}
		if (!Q_stricmp(token,"rdc")) {
			// Stop cinematic mode
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_DEFAULT);
			continue;
		}
		
		if (!Q_stricmp(token,"fi")) {
			// Fade in
			token = Cin_NextToken(&pb);
			if (!token) {
				trap->Print("WARNING: ^3Error processing cinematic info: No fadetime after fi instruction\n");
				return;
			}
			cin.Fade.startTime = cg.time;
			cin.Fade.from = 100;
			cin.Fade.to = 0; // in percentage scale (alpha)
			cin.Fade.duration = atoi(token);
			cin.Fade.state = 1; // Fading
			continue;
		}
		if (!Q_stricmp(token,"fo")) {
			// Fade out
			token = Cin_NextToken(&pb);
			if (!token) {
				trap->Print("WARNING: ^3Error processing cinematic info: No fadetime after fo instruction\n");
				return;
			}
			cin.Fade.startTime = cg.time;
			cin.Fade.from = 0;
			cin.Fade.to = 100; // in percentage scale (alpha)
			cin.Fade.duration = atoi(token);
			cin.Fade.state = 1; // Fading
			continue;
		}
		if (!Q_stricmp(token,"fli")) {
			// Flash in
			token = Cin_NextToken(&pb);
			if (!token) {
				trap->Print("WARNING: ^3Error processing cinematic info: No fadetime after fli instruction\n");
				return;
			}
			cin.Flash.startTime = cg.time;
			cin.Flash.from = 100;
			cin.Flash.to = 0; // in percentage scale (alpha)
			cin.Flash.duration = atoi(token);
			cin.Flash.state = 1; // Fading
			continue;
		}
		if (!Q_stricmp(token,"flo")) {
			// Flash out
			token = Cin_NextToken(&pb);
			if (!token) {
				trap->Print("WARNING: ^3Error processing cinematic info: No fadetime after flo instruction\n");
				return;
			}
			cin.Flash.startTime = cg.time;
			cin.Flash.from = 0;
			cin.Flash.to = 100; // in percentage scale (alpha)
			cin.Flash.duration = atoi(token);
			cin.Flash.state = 1; // Fading
			continue;
		}
		if (!Q_stricmp(token, "sc")) {
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_STATIC);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			if (Cin_ParseVector(&pb, &CamData->CamStatic.origin)) return;
			if (Cin_ParseVector(&pb, &CamData->CamStatic.angles)) return;
			continue;
		}
		if (!Q_stricmp(token, "ac")) {
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_AIMED);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			if (Cin_ParseInt(&pb, &CamData->CamAimed.entID)) return;
			if (Cin_ParseVector(&pb, &CamData->CamAimed.origin)) return;
			continue;
		}
		if (!Q_stricmp(token, "lc")) {
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_LINEAR);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamLinear.camStartTime = cg.time;
			if (Cin_ParseInt(&pb, &CamData->CamLinear.targetType)) return;
			if (CamData->CamLinear.targetType == 0) {
				// All data explicit
				if (Cin_ParseInt(&pb, &CamData->CamLinear.pointcount)) return;
			} else if (CamData->CamLinear.targetType == 1) {
				// Target an ent
				if (Cin_ParseInt(&pb, &CamData->CamLinear.entID)) return;
				if (Cin_ParseInt(&pb, &CamData->CamLinear.pointcount)) return;
			} else if (CamData->CamLinear.targetType == 2) {
				// Target an origin
				if (Cin_ParseVector(&pb, &CamData->CamLinear.targetpos)) return;
				if (Cin_ParseInt(&pb, &CamData->CamLinear.pointcount)) return;
			} else {
				trap->Print("WARNING: ^3Error processing cinematic info: Invalid linear cam trajectory type\n");
			}
			// Alloc a buffer for the points
			CamData->CamLinear.pointData = (CamData_Linear_Point_t *)malloc(sizeof(CamData_Linear_Point_t) * CamData->CamLinear.pointcount);
			memset(CamData->CamLinear.pointData, 0, sizeof(CamData_Linear_Point_t) * CamData->CamLinear.pointcount);
			// Start parsing the cam points
			for (i=0; i<CamData->CamLinear.pointcount; i++) {
				if (!Cin_TokensAvailable(&pb)) {
					// Ok we're out of data, mark this position and bail out
					cin.AwaitingData = 1;
					cin.DataOffset = i;
					return;
				}
				if (Cin_ParseInt(&pb, &CamData->CamLinear.pointData[i].offset)) return;
				if (Cin_ParseVector(&pb, &CamData->CamLinear.pointData[i].origin)) return;
				if (!CamData->CamLinear.targetType) {
					if (Cin_ParseVector(&pb, &CamData->CamLinear.pointData[i].angles)) return;
				}
			}
			Cin_DoLinearInterpolation();
			continue;
		}
		if (!Q_stricmp(token, "spc")) {
			int algos;
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_SPLINE);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamSpline.camStartTime = cg.time;
			if (Cin_ParseInt(&pb, &CamData->CamSpline.targetType)) return;
			if (CamData->CamSpline.targetType == 0) {
				// All data explicit
				if (Cin_ParseInt(&pb, &CamData->CamSpline.pointcount)) return;
			} else if (CamData->CamSpline.targetType == 1) {
				// Target an ent
				if (Cin_ParseInt(&pb, &CamData->CamSpline.entID)) return;
				if (Cin_ParseInt(&pb, &CamData->CamSpline.pointcount)) return;
			} else if (CamData->CamSpline.targetType == 2) {
				// Target an origin
				if (Cin_ParseVector(&pb, &CamData->CamSpline.targetpos)) return;
				if (Cin_ParseInt(&pb, &CamData->CamSpline.pointcount)) return;
			} else {
				trap->Print("WARNING: ^3Error processing cinematic info: Invalid spline cam trajectory type\n");
			}
			// Get algoritms
			// Format:
			//  xx  xx  xx  xx  xx  xx  (x = bit)
			// <O1><O2><O3><A1><A2><A3>
			Cin_ParseInt(&pb, &algos);
			CamData->CamSpline.Algorithms[0] = (SplineAlgorithm_e)((algos & 3072) >> 10);
			CamData->CamSpline.Algorithms[1] = (SplineAlgorithm_e)((algos & 768) >> 8);
			CamData->CamSpline.Algorithms[2] = (SplineAlgorithm_e)((algos & 192) >> 6);
			CamData->CamSpline.Algorithms[3] = (SplineAlgorithm_e)((algos & 48) >> 4);
			CamData->CamSpline.Algorithms[4] = (SplineAlgorithm_e)((algos & 12) >> 2);
			CamData->CamSpline.Algorithms[5] = (SplineAlgorithm_e)((algos & 3));

			// Alloc a buffer for the points
			CamData->CamSpline.pointData = (CamData_Spline_Point_t *)malloc(sizeof(CamData_Spline_Point_t) * CamData->CamSpline.pointcount);
			memset(CamData->CamSpline.pointData, 0, sizeof(CamData_Spline_Point_t) * CamData->CamSpline.pointcount);
			// Start parsing the cam points
			for (i=0; i<CamData->CamSpline.pointcount; i++) {
				if (!Cin_TokensAvailable(&pb)) {
					// Ok we're out of data, mark this position and bail out
					cin.AwaitingData = 1;
					cin.DataOffset = i;
					return;
				}
				if (Cin_ParseInt(&pb, &CamData->CamSpline.pointData[i].offset)) return;
				if (Cin_ParseVector(&pb, &CamData->CamSpline.pointData[i].origin)) return;
				if (!CamData->CamSpline.targetType) {
					if (Cin_ParseVector(&pb, &CamData->CamSpline.pointData[i].angles)) return;
				}
			}
			
			for (i=0; i<6; i++) {
				if (CamData->CamSpline.Algorithms[i] == 0) {
					Cin_DoCubicSplineInterpolation();
					break;
				}
			}
			continue;
		}
		if (!Q_stricmp(token, "cont")) {
			// Continuation of a previous coord set
			if (!cin.AwaitingData) {
				trap->Print("WARNING: ^3Error processing cinematic info: Cont received without awaiting further data\n");
				return;
			}
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			cin.AwaitingData = 0;
			if (cin.CamMode == CAM_LINEAR) {
				for (i=cin.DataOffset; i<CamData->CamLinear.pointcount; i++) {
					if (!Cin_TokensAvailable(&pb)) {
						// Ok we're out of data, mark this position and bail out
						cin.AwaitingData = 1;
						cin.DataOffset = i;
						return;
					}
					if (Cin_ParseInt(&pb, &CamData->CamLinear.pointData[i].offset)) return;
					if (Cin_ParseVector(&pb, &CamData->CamLinear.pointData[i].origin)) return;
					if (!CamData->CamLinear.targetType) {
						if (Cin_ParseVector(&pb, &CamData->CamLinear.pointData[i].angles)) return;
					}
				}
				Cin_DoLinearInterpolation();
			} else if (cin.CamMode == CAM_SPLINE) {
				for (i=cin.DataOffset; i<CamData->CamSpline.pointcount; i++) {
					if (!Cin_TokensAvailable(&pb)) {
						// Ok we're out of data, mark this position and bail out
						cin.AwaitingData = 1;
						cin.DataOffset = i;
						return;
					}
					if (Cin_ParseInt(&pb, &CamData->CamSpline.pointData[i].offset)) return;
					if (Cin_ParseVector(&pb, &CamData->CamSpline.pointData[i].origin)) return;
					if (!CamData->CamSpline.targetType) {
						if (Cin_ParseVector(&pb, &CamData->CamSpline.pointData[i].angles)) return;
					}
				}
				for (i=0; i<6; i++) {
					if (CamData->CamSpline.Algorithms[i] == 0) {
						Cin_DoCubicSplineInterpolation();
						break;
					}
				}
			}
			continue;
		}
		if (!Q_stricmp(token, "mb")) {
			cin.Motionblur.state = 2;
			if (Cin_ParseInt(&pb, &cin.Motionblur.from)) return;
			if (cin.Motionblur.from < 0) { // turn it off
				cin.Motionblur.state = 0;
			}
			continue;
		}

		if (!Q_stricmp(token, "mbf")) {
			cin.Motionblur.state = 1;
			cin.Motionblur.startTime = cg.time;
			if (Cin_ParseInt(&pb, &cin.Motionblur.from)) return;
			if (cin.Motionblur.from < 0) cin.Motionblur.from = 0;
			if (Cin_ParseInt(&pb, &cin.Motionblur.to)) return;
			if (cin.Motionblur.to < 0) cin.Motionblur.to = 0;
			if (Cin_ParseInt(&pb, &cin.Motionblur.duration)) return;
			continue;
		}

		if (!Q_stricmp(token, "fov")) {
			cin.Fov.state = 2;
			if (Cin_ParseInt(&pb, &cin.Fov.from)) return;
			if (cin.Fov.from < 0) { // turn it off
				cin.Fov.state = 0;
			}
			continue;
		}

		if (!Q_stricmp(token, "fovf")) {
			cin.Fov.state = 1;
			cin.Fov.startTime = cg.time;
			if (Cin_ParseInt(&pb, &cin.Fov.from)) return;
			if (cin.Fov.from < 0) cin.Fov.from = cg_fov.integer;
			if (Cin_ParseInt(&pb, &cin.Fov.to)) return;
			if (cin.Fov.to < 0) cin.Fov.to = cg_fov.integer;
			if (Cin_ParseInt(&pb, &cin.Fov.duration)) return;
			continue;
		}
		// Color mod - Scale
		if (!Q_stricmp(token, "cmso")) {
			cin.CM.scale.state = 0;
			continue;
		}

		if (!Q_stricmp(token, "cmsf")) {
			cin.CM.scale.state = 1;
			cin.CM.scale.startTime = cg.time;
			if (Cin_ParseVector(&pb, &cin.CM.scale.from)) return;
			if (Cin_ParseVector(&pb, &cin.CM.scale.to)) return;
			if (Cin_ParseInt(&pb, &cin.CM.scale.duration)) return;
			continue;
		}
		if (!Q_stricmp(token, "cms")) {
			cin.CM.scale.state = 2;
			if (Cin_ParseVector(&pb, &cin.CM.scale.from)) return;
			if (cin.CM.scale.from[0] == 1 && cin.CM.scale.from[1] == 1 && cin.CM.scale.from[2] == 1) {
				cin.CM.scale.state = 0;
			}
			continue;
		}

		// Color mod - Bias
		if (!Q_stricmp(token, "cmbo")) {
			cin.CM.bias.state = 0;
			continue;
		}

		if (!Q_stricmp(token, "cmbf")) {
			cin.CM.bias.state = 1;
			cin.CM.bias.startTime = cg.time;
			if (Cin_ParseVector(&pb, &cin.CM.bias.from)) return;
			if (Cin_ParseVector(&pb, &cin.CM.bias.to)) return;
			if (Cin_ParseInt(&pb, &cin.CM.bias.duration)) return;
			continue;
		}
		if (!Q_stricmp(token, "cmb")) {
			cin.CM.bias.state = 2;
			if (Cin_ParseVector(&pb, &cin.CM.bias.from)) return;
			if (cin.CM.bias.from[0] == 0 && cin.CM.bias.from[1] == 0 && cin.CM.bias.from[2] == 0) {
				cin.CM.bias.state = 0;
			}
			continue;
		}

		// Color mod - Brightness/Contrast
		if (!Q_stricmp(token, "cmco")) {
			cin.CM.bc.state = 0;
			continue;
		}

		if (!Q_stricmp(token, "cmcf")) {
			cin.CM.bc.state = 1;
			cin.CM.bc.startTime = cg.time;
			if (Cin_ParseVector2(&pb, &cin.CM.bc.from)) return;
			if (Cin_ParseVector2(&pb, &cin.CM.bc.to)) return;
			if (Cin_ParseInt(&pb, &cin.CM.bc.duration)) return;
			continue;
		}
		if (!Q_stricmp(token, "cmc")) {
			cin.CM.bc.state = 2;
			if (Cin_ParseVector2(&pb, &cin.CM.bc.from)) return;
			if (cin.CM.bc.from[0] == 0 && cin.CM.bc.from[1] == 1) {
				cin.CM.bc.state = 0;
			}
			continue;
		}

		// Color mod - FX
		if (!Q_stricmp(token, "cmfo")) {
			cin.CM.fx.state = 0;
			continue;
		}

		if (!Q_stricmp(token, "cmff")) {
			cin.CM.fx.state = 1;
			cin.CM.fx.startTime = cg.time;
			if (Cin_ParseInt(&pb, &cin.CM.fx.fxID)) return;
			if (Cin_ParseVector2(&pb, &cin.CM.fx.from)) return;
			if (Cin_ParseVector2(&pb, &cin.CM.fx.to)) return;
			if (Cin_ParseInt(&pb, &cin.CM.fx.duration)) return;
			continue;
		}
		if (!Q_stricmp(token, "cmf")) {
			cin.CM.fx.state = 2;
			if (Cin_ParseInt(&pb, &cin.CM.fx.fxID)) return;
			if (Cin_ParseVector2(&pb, &cin.CM.fx.from)) return;
			if (cin.CM.fx.fxID == 0 || cin.CM.fx.from[0] == 0) {
				cin.CM.fx.state = 0;
			}
			continue;
		}

		// Color mod - Inversion
		if (!Q_stricmp(token, "cmio")) {
			cin.CM.inv.state = 0;
			continue;
		}

		if (!Q_stricmp(token, "cmif")) {
			cin.CM.inv.state = 1;
			cin.CM.inv.startTime = cg.time;
			if (Cin_ParseFloat(&pb, &cin.CM.inv.from)) return;
			if (Cin_ParseFloat(&pb, &cin.CM.inv.to)) return;
			if (Cin_ParseInt(&pb, &cin.CM.inv.duration)) return;
			continue;
		}
		if (!Q_stricmp(token, "cmi")) {
			cin.CM.inv.state = 2;
			if (Cin_ParseFloat(&pb, &cin.CM.inv.from)) return;
			if (cin.CM.inv.from == 0) {
				cin.CM.inv.state = 0;
			}
			continue;
		}

		if (!Q_stricmp(token, "he")) { // Hitchcock effect
			// This one is converted into two acts: a linear cam act and an fov act
			cin.AwaitingData = 0;
			Cin_SetCamData(CAM_HITCHCOCK);
			CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
			CamData->CamHitchcock.camStartTime = cg.time;
			if (Cin_ParseVector(&pb, &CamData->CamHitchcock.origin)) return;
			if (Cin_ParseVector(&pb, &CamData->CamHitchcock.angles)) return;
			if (Cin_ParseInt(&pb, &CamData->CamHitchcock.width)) return;
			if (Cin_ParseInt(&pb, &CamData->CamHitchcock.startFov)) return;
			if (CamData->CamHitchcock.startFov == -1) CamData->CamHitchcock.startFov = cg_fov.integer;
			if (Cin_ParseInt(&pb, &CamData->CamHitchcock.endFov)) return;
			if (CamData->CamHitchcock.endFov == -1) CamData->CamHitchcock.endFov = cg_fov.integer;
			if (Cin_ParseInt(&pb, &CamData->CamHitchcock.duration)) return;
			// enable FOV while we're at it
			cin.Fov.state = 1;
			cin.Fov.startTime = cg.time;
			cin.Fov.from = CamData->CamHitchcock.startFov;
			cin.Fov.to = CamData->CamHitchcock.endFov;
			cin.Fov.duration = CamData->CamHitchcock.duration;
		}
	}
}

static void Cin_DoLinearInterpolation() {
	CamData_Union_t *CamData;
	CamData_Linear_Point_t *pd;
	int i;
	if (cin.CamMode != CAM_LINEAR) return;
	if (cin.AwaitingData) return;
	// First things first, check if the first cam is at offset 0
	// If not, add the current refdef as first point, so we can interpolate that too
	
	CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
	if (CamData->CamLinear.pointData[0].offset != 0) {
		// Alright we gotta include the current refdef as first point, so lets do it!
		CamData_Linear_Point_t *newarr;
		newarr = (CamData_Linear_Point_t *)malloc(sizeof(CamData_Linear_Point_t) * (CamData->CamLinear.pointcount+1));
		memcpy(&newarr[1], CamData->CamLinear.pointData, sizeof(CamData_Linear_Point_t) * CamData->CamLinear.pointcount);
		CamData->CamLinear.pointcount++;
		memset(newarr, 0, sizeof(CamData_Linear_Point_t));
		free(CamData->CamLinear.pointData);
		CamData->CamLinear.pointData = newarr;
		CamData->CamLinear.pointData[0].offset = 0;
		VectorCopy(cg.refdef.vieworg, CamData->CamLinear.pointData[0].origin);
		VectorCopy(cg.refdef.viewangles, CamData->CamLinear.pointData[0].angles);
	}
	pd = CamData->CamLinear.pointData; // faster access
	// Start the linear interpolation and fill in the coeffs ([0] = origin, [1] = angles)
	for (i=0; i< CamData->CamLinear.pointcount -1; i++) {
		vec3_t deltaV;
		float deltaT;
		
		// Origin
		VectorSubtract(pd[i+1].origin, pd[i].origin, deltaV);
		deltaT = ((float)pd[i+1].offset - (float)pd[i].offset) / 1000; // in seconds
		VectorScale(deltaV, 1/deltaT, pd[i].li_coeff[0]);

		// Angles (targetted cams simply ignore this)
		VectorSubtract(pd[i+1].angles, pd[i].angles, deltaV);
		deltaT = ((float)pd[i+1].offset - (float)pd[i].offset) / 1000; // in seconds
		VectorScale(deltaV, 1/deltaT, pd[i].li_coeff[1]);
	}
}

// Following code originally from pugmod
// Modified to improve readability, and to make it compatible with JKG
// start cubic spline interpolation code (parts are converted from javascript at http://people.hofstra.edu/faculty/Stefan_Waner/realworld/tutorialsf1/scriptpivot2.html)

double splMatrix[50*4][50*4+1];

static void Cin_Spl_pivot( int rows, int cols, int theRow, int theCol ) {
	double thePivot = splMatrix[theRow][theCol];
	int i, j;
	for( i=0; i<cols; i++ )
		splMatrix[theRow][i] = splMatrix[theRow][i]/thePivot;

	for( i=0; i<rows; i++ )
		if( ( i!=theRow ) && ( splMatrix[i][theCol]!=0 ) )
		{
			double factr = splMatrix[i][theCol];
			for( j=0; j<cols; j++ )
				splMatrix[i][j] = splMatrix[i][j] - factr*splMatrix[theRow][j];	
		}
}

static void Cin_Spl_swapRows( int p, int q, int numCols ) {
	double rowHold = 0;
	int j;
	for( j=0; j<numCols; j++ )
	{
		rowHold = splMatrix[p][j];
		splMatrix[p][j] = splMatrix[q][j];
		splMatrix[q][j] = rowHold;
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
			double theNum = splMatrix[i][j];
			if( theNum * ( theNum > 0 ? 1 : -1 ) <= theSmallestNumber )
				splMatrix[i][j] = 0;
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
			if( splMatrix[i][j] != 0 )
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
	CamData_Union_t *CamData;
	CamData_Spline_Point_t *pd;
	double deltaT;
	double deltaD;
	int CamCount;

	if (cin.CamMode != CAM_SPLINE) return;
	if (cin.AwaitingData) return;
	// First things first, check if the first cam is at offset 0
	// If not, add the current refdef as first point, so we can interpolate that too
	CamData = reinterpret_cast<CamData_Union_t *>(cin.CamData);
	if (CamData->CamSpline.pointData[0].offset != 0) {
		// Alright we gotta include the current refdef as first point, so lets do it!
		CamData_Spline_Point_t *newarr;
		newarr = (CamData_Spline_Point_t *)malloc(sizeof(CamData_Spline_Point_t) * (CamData->CamSpline.pointcount+1));
		memcpy(&newarr[1], CamData->CamLinear.pointData, sizeof(CamData_Spline_Point_t) * CamData->CamLinear.pointcount);
		CamData->CamSpline.pointcount++;
		memset(newarr, 0, sizeof(CamData_Spline_Point_t));
		free(CamData->CamSpline.pointData);
		CamData->CamSpline.pointData = newarr;
		CamData->CamSpline.pointData[0].offset = 0;
		VectorCopy(cg.refdef.vieworg, CamData->CamSpline.pointData[0].origin);
		VectorCopy(cg.refdef.viewangles, CamData->CamSpline.pointData[0].angles);
	}
	pd = CamData->CamSpline.pointData; // faster access

	CamCount = CamData->CamSpline.pointcount-1;
	for( k=0; k<3; k++ )
	{
		curRow = 0;
		memset( splMatrix[0], 0, sizeof( splMatrix ) );
		for( i=0; i<CamCount-1; i++ )
		{
			deltaT = (double)(pd[i+1].offset - pd[i].offset);
			deltaD = (double)(pd[i+1].origin[k] - pd[i].origin[k]);
			j=0;
			//runs through first point
			splMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV)*(((double)cam[i].time)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 0;//(((double)cam[i].time)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;//cam[i].origin[k];
			
			j=0;
			//runs through second point
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow++][(CamCount-1)*4+4] = deltaD;
			
			j=0;
			//first derivitives are equal
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*2;
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*2;
			splMatrix[curRow][i*4+j++] = -1;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;
			
			j=0;
			//second derivitives are equal
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*6;
			splMatrix[curRow][i*4+j++] = 2;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*6;
			splMatrix[curRow][i*4+j++] = -2;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		}
		//4 more equations are needed to have a solvable matrix (there should be n-1 splines)
		
		j=0;
		//second derivitive at the end is zero
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)pd[CamCount].offset-pd[CamCount-1].offset)/TIMEDIV)*6;
		splMatrix[curRow][(CamCount-1)*4+j++] = 2;
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//second derivitive at the beginning is zero
		splMatrix[curRow][j++] = 0;//(((double)cam[0].time)/TIMEDIV)*6;
		splMatrix[curRow][j++] = 2;
		splMatrix[curRow][j++] = 0;
		splMatrix[curRow][j++] = 0;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//runs through first point
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV)*(((double)cam[CamCount-1].time)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)cam[CamCount-1].time)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 1;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;//cam[(CamCount-1)].origin[k];
		
		deltaT = (double)(pd[CamCount].offset - pd[CamCount-1].offset);
		j=0;
		//runs through second point
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 1;
		splMatrix[curRow++][(CamCount-1)*4+4] = pd[CamCount].origin[k] - pd[CamCount-1].origin[k];
		
		numRows = curRow;
		numCols = CamCount*4+1;
		Cin_Spl_rowReduce(numRows, numCols);
		
		for( i=0; i<CamCount; i++ )
		{
			pd[i].csi_coeff[k][0] = splMatrix[i*4][CamCount*4];
			pd[i].csi_coeff[k][1] = splMatrix[i*4+1][CamCount*4];
			pd[i].csi_coeff[k][2] = splMatrix[i*4+2][CamCount*4];
			pd[i].csi_coeff[k][3] = splMatrix[i*4+3][CamCount*4];
		}
	}
	
	for( k=0; k<3; k++ )
	{
		curRow = 0;
		memset( splMatrix[0], 0, sizeof( splMatrix ) );
		for( i=0; i<CamCount-1; i++ )
		{
			deltaT = (double)(pd[i+1].offset - pd[i].offset);
			deltaD = (double)(pd[i+1].angles[k] - pd[i].angles[k]);
			j=0;
			//runs through first point
			//guess what? ive decided it's (0,0) kthx.
			splMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV)*(((double)pd[i].offset)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 0;//(((double)pd[i].offset)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;//pd[i].origin[k];
			
			j=0;
			//runs through second point
			//doodz we're makin dese thangs unit doodz so it b leeter :>
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV);
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow++][(CamCount-1)*4+4] = deltaD;
			
			j=0;
			//first derivitives are equal
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*2;
			splMatrix[curRow][i*4+j++] = 1;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*3;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*2;
			splMatrix[curRow][i*4+j++] = -1;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;
			
			j=0;
			//second derivitives are equal
			splMatrix[curRow][i*4+j++] = (((double)deltaT)/TIMEDIV)*6;
			splMatrix[curRow][i*4+j++] = 2;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;//-(((double)deltaT)/TIMEDIV)*6;
			splMatrix[curRow][i*4+j++] = -2;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow][i*4+j++] = 0;
			splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		}
		//4 more equations are needed to have a solvable matrix (there should be n-1 splines)
		
		j=0;
		//second derivitive at the end is zero
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)pd[CamCount].offset-pd[CamCount-1].offset)/TIMEDIV)*6;
		splMatrix[curRow][(CamCount-1)*4+j++] = 2;
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//second derivitive at the beginning is zero
		splMatrix[curRow][j++] = 0;//(((double)pd[0].offset)/TIMEDIV)*6;
		splMatrix[curRow][j++] = 2;
		splMatrix[curRow][j++] = 0;
		splMatrix[curRow][j++] = 0;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;
		
		j=0;
		//runs through first point
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV)*(((double)pd[CamCount-1].offset)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 0;//(((double)pd[CamCount-1].offset)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 1;
		splMatrix[curRow++][(CamCount-1)*4+4] = 0;//pd[(CamCount-1)].origin[k];
		
		deltaT = (double)(pd[CamCount].offset - pd[CamCount-1].offset);
		j=0;
		//runs through second point
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV)*(((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = (((double)deltaT)/TIMEDIV);
		splMatrix[curRow][(CamCount-1)*4+j++] = 1;
		splMatrix[curRow++][(CamCount-1)*4+4] = pd[CamCount].angles[k] - pd[CamCount-1].angles[k];
		
		numRows = curRow;
		numCols = CamCount*4+1;
		Cin_Spl_rowReduce(numRows, numCols);
		
		for( i=0; i<CamCount; i++ )
		{
			pd[i].csi_coeff[k+3][0] = splMatrix[i*4][CamCount*4];
			pd[i].csi_coeff[k+3][1] = splMatrix[i*4+1][CamCount*4];
			pd[i].csi_coeff[k+3][2] = splMatrix[i*4+2][CamCount*4];
			pd[i].csi_coeff[k+3][3] = splMatrix[i*4+3][CamCount*4];
		}
	}
}

// end cubic spline interpolation code