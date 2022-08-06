#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

constexpr auto MAX_SPOTCAMS = 256;
constexpr auto SPOTCAM_CINEMATIC_BARS_HEIGHT = 1.0f / 16.0f;
constexpr auto SPOTCAM_CINEMATIC_BARS_SPEED = 1.0f / FPS;

struct SPOTCAM
{
	int x;
	int y;
	int z;
	int tx;
	int ty;
	int tz;
	unsigned char sequence;
	unsigned char camera;
	short fov;
	short roll;
	short timer;
	short speed;
	short flags;
	short roomNumber;
	short pad;
};

enum SPOTCAM_FLAGS
{
	SCF_CUT_PAN = (1 << 0),					 // 0x0001   cut without panning smoothly
	SCF_OVERLAY = (1 << 1),				     // 0x0002	 TODO: add vignette
	SCF_LOOP_SEQUENCE = (1 << 2),			 // 0x0004
	SCF_TRACKING_CAM = (1 << 3),			 // 0x0008
	SCF_HIDE_LARA = (1 << 4),				 // 0x0010
	SCF_FOCUS_LARA_HEAD = (1 << 5),			 // 0x0020
	SCF_PAN_TO_LARA_CAM = (1 << 6),			 // 0x0040
	SCF_CUT_TO_CAM = (1 << 7),				 // 0x0080
	SCF_STOP_MOVEMENT = (1 << 8),			 // 0x0100   stops movement for a given time (cf. `Timer` field)
	SCF_DISABLE_BREAKOUT = (1 << 9),		 // 0x0200   disables breaking out from cutscene using Look key
	SCF_DISABLE_LARA_CONTROLS = (1 << 10),	 // 0x0400   also adds widescreen bars
	SCF_REENABLE_LARA_CONTROLS = (1 << 11),	 // 0x0800   use with 0x0400, keeps widescreen bars
	SCF_SCREEN_FADE_IN = (1 << 12),			 // 0x1000
	SCF_SCREEN_FADE_OUT = (1 << 13),		 // 0x2000
	SCF_ACTIVATE_HEAVY_TRIGGERS = (1 << 14), // 0x4000   when camera is moving above heavy trigger sector, it'll be	activated
	SCF_CAMERA_ONE_SHOT = (1 << 15),		 // 0x8000
};

extern SPOTCAM SpotCam[MAX_SPOTCAMS];
extern int SpotCamRemap[MAX_SPOTCAMS];
extern int CameraCnt[MAX_SPOTCAMS];
extern int LastSpotCamSequence;
extern int NumberSpotcams;
extern bool UseSpotCam;
extern bool SpotcamDontDrawLara;
extern bool SpotcamOverlay;
extern bool TrackCameraInit;

void ClearSpotCamSequences();
void InitSpotCamSequences();
void InitialiseSpotCam(short sequence);
void CalculateSpotCameras();
int Spline(int x, int* knots, int nk);
