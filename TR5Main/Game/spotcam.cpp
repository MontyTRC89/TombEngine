#include "framework.h"
#include "Game/spotcam.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/input.h"

using namespace TEN::Renderer;
using namespace TEN::Control::Volumes;

int TrackCameraInit;
int SpotcamTimer;
int SpotcamPaused;
int SpotcamLoopCnt;
int CameraFade;
PHD_VECTOR LaraFixedPosition;
int InitialCameraRoom;
int LastFOV;
PHD_VECTOR InitialCameraPosition;
PHD_VECTOR InitialCameraTarget;
int CurrentSplinePosition;
int SplineToCamera;
int FirstCamera;
int LastCamera;
int CurrentCameraCnt;
int CameraXposition[MAX_CAMERA];
int CameraYposition[MAX_CAMERA];
int CameraZposition[MAX_CAMERA];
int CameraXtarget[MAX_CAMERA];
int CameraYtarget[MAX_CAMERA];
int CameraZtarget[MAX_CAMERA];
int CameraRoll[MAX_CAMERA];
int CameraFOV[MAX_CAMERA];
int CameraSpeed[MAX_CAMERA];
QUAKE_CAMERA QuakeCam;
int SplineFromCamera;
bool SpotCamFirstLook;
short CurrentSplineCamera;
int LastSpotCamSequence;
int LaraHealth;
int LaraAir;
int CurrentSpotcamSequence;
SPOTCAM SpotCam[MAX_SPOTCAMS];
byte SpotCamRemap[MAX_SPOTCAMS];
byte CameraCnt[MAX_SPOTCAMS];
int NumberSpotcams;
int CheckTrigger = 0;
int UseSpotCam = 0;
int SpotcamDontDrawLara;
int SpotcamOverlay;

void ClearSpotCamSequences()
{
	UseSpotCam = false;

	for (int i = 0; i < MAX_SPOTCAMS; i++)
		SpotCam[i] = {};
}

void InitSpotCamSequences() 
{
	int s, cc, n, ce;

	n = NumberSpotcams;
	TrackCameraInit = 0;
	cc = 1;

	if (n != 0)
	{
		ce = 0;
		s = SpotCam[0].sequence;

		if (cc < n)
		{
			for (n = 1; n < NumberSpotcams; n++)
			{
				//Same sequence
				if (SpotCam[n].sequence == s)
				{
					cc++;
				}
				else
				{
					//New sequence
					CameraCnt[ce] = cc;
					cc = 1;
					SpotCamRemap[s] = ce;
					ce++;
					s = SpotCam[n].sequence;
				}
			}
		}

		CameraCnt[ce] = cc;
		SpotCamRemap[s] = ce;
	}

	return;
}

void InitialiseSpotCam(short Sequence)
{
	SPOTCAM* s;
	int cn;
	int sp;
	int i;

	if (TrackCameraInit != 0 && LastSpotCamSequence == Sequence)
	{
		TrackCameraInit = 0;
		return;
	}

	BinocularRange = 0;
	LaserSight = false;

	AlterFOV(16380);

	LaraItem->MeshBits = -1;

	ResetLaraFlex(LaraItem);

	Camera.bounce = 0;

	Lara.Control.IsBusy = 0;

	CameraFade = -1;
	LastSpotCamSequence = Sequence;
	TrackCameraInit = 0;
	SpotcamTimer = 0;
	SpotcamPaused = 0;
	SpotcamLoopCnt = 0;
	Lara.Control.Uncontrollable = false;

	LastFOV = CurrentFOV;
	LaraAir = Lara.air;

	InitialCameraPosition.x = Camera.pos.x;
	InitialCameraPosition.y = Camera.pos.y;
	InitialCameraPosition.z = Camera.pos.z;

	InitialCameraTarget.x = Camera.target.x;
	InitialCameraTarget.y = Camera.target.y;
	InitialCameraTarget.z = Camera.target.z;

	LaraHealth = LaraItem->HitPoints;
	InitialCameraRoom = Camera.pos.roomNumber;

	LaraFixedPosition.x = LaraItem->Position.xPos;
	LaraFixedPosition.y = LaraItem->Position.yPos;
	LaraFixedPosition.z = LaraItem->Position.zPos;

	CurrentSpotcamSequence = Sequence;
	CurrentSplineCamera = 0;

	for (i = 0; i < SpotCamRemap[Sequence]; i++)
	{
		CurrentSplineCamera += CameraCnt[i];
	}

	CurrentSplinePosition = 0;
	SplineToCamera = 0;

	FirstCamera = CurrentSplineCamera;

	s = &SpotCam[CurrentSplineCamera];

	LastCamera = CurrentSplineCamera + (CameraCnt[SpotCamRemap[Sequence]] - 1);
	CurrentCameraCnt = CameraCnt[SpotCamRemap[Sequence]];

	if ((s->flags & SCF_DISABLE_LARA_CONTROLS))
	{
		Lara.Control.Uncontrollable = true;
		SetCinematicBars(SPOTCAM_CINEMATIC_BARS_HEIGHT, SPOTCAM_CINEMATIC_BARS_SPEED);
	}

	if (s->flags & SCF_TRACKING_CAM)
	{
		CameraXposition[1] = SpotCam[FirstCamera].x;
		CameraYposition[1] = SpotCam[FirstCamera].y;
		CameraZposition[1] = SpotCam[FirstCamera].z;
		CameraXtarget[1] = SpotCam[FirstCamera].tx;
		CameraYtarget[1] = SpotCam[FirstCamera].ty;
		CameraZtarget[1] = SpotCam[FirstCamera].tz;
		CameraRoll[1] = SpotCam[FirstCamera].roll;
		CameraFOV[1] = SpotCam[FirstCamera].fov;
		CameraSpeed[1] = SpotCam[FirstCamera].speed;

		SplineFromCamera = 0;

		if (CurrentCameraCnt > 0)
		{
			s = &SpotCam[FirstCamera];

			for (i = 0; i < CurrentCameraCnt; i++, s++)
			{
				CameraXposition[i + 2] = s->x;
				CameraYposition[i + 2] = s->y;
				CameraZposition[i + 2] = s->z;
				CameraXtarget[i + 2] = s->tx;
				CameraYtarget[i + 2] = s->ty;
				CameraZtarget[i + 2] = s->tz;
				CameraRoll[i + 2] = s->roll;
				CameraFOV[i + 2] = s->fov;
				CameraSpeed[i + 2] = s->speed;
			}
		}

		//loc_379F8
		CameraXposition[CurrentCameraCnt + 2] = SpotCam[LastCamera].x;
		CameraYposition[CurrentCameraCnt + 2] = SpotCam[LastCamera].y;
		CameraZposition[CurrentCameraCnt + 2] = SpotCam[LastCamera].z;
		CameraXtarget[CurrentCameraCnt + 2] = SpotCam[LastCamera].tx;
		CameraYtarget[CurrentCameraCnt + 2] = SpotCam[LastCamera].ty;
		CameraZtarget[CurrentCameraCnt + 2] = SpotCam[LastCamera].tz;
		CameraFOV[CurrentCameraCnt + 2] = SpotCam[LastCamera].fov;
		CameraRoll[CurrentCameraCnt + 2] = SpotCam[LastCamera].roll;
		CameraSpeed[CurrentCameraCnt + 2] = SpotCam[LastCamera].speed;
	}
	else
	{
		//loc_37AA8
		sp = 0;
		if ((s->flags & SCF_CUT_PAN))
		{
			CameraXposition[1] = SpotCam[CurrentSplineCamera].x;
			CameraYposition[1] = SpotCam[CurrentSplineCamera].y;
			CameraZposition[1] = SpotCam[CurrentSplineCamera].z;
			CameraXtarget[1] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[1] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[1] = SpotCam[CurrentSplineCamera].tz;
			CameraRoll[1] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[1] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[1] = SpotCam[CurrentSplineCamera].speed;

			SplineFromCamera = 0;

			cn = CurrentSplineCamera;
			while (sp < 4)
			{
				if (LastCamera < CurrentSplineCamera)
				{
					cn = FirstCamera;
				}

				//loc_37B74
				CameraXposition[sp + 2] = SpotCam[cn].x;
				CameraYposition[sp + 2] = SpotCam[cn].y;
				CameraZposition[sp + 2] = SpotCam[cn].z;
				CameraXtarget[sp + 2] = SpotCam[cn].tx;
				CameraYtarget[sp + 2] = SpotCam[cn].ty;
				CameraZtarget[sp + 2] = SpotCam[cn].tz;
				CameraRoll[sp + 2] = SpotCam[cn].roll;
				CameraFOV[sp + 2] = SpotCam[cn].fov;
				CameraSpeed[sp + 2] = SpotCam[cn].speed;
				cn++;
				sp++;
			}

			CurrentSplineCamera++;

			if (CurrentSplineCamera > LastCamera)
			{
				CurrentSplineCamera = FirstCamera;
			}

			if (s->flags & SCF_ACTIVATE_HEAVY_TRIGGERS)
			{
				CheckTrigger = true;
			}

			if (s->flags & SCF_HIDE_LARA)
			{
				SpotcamDontDrawLara = true;
			}
			else
			{
				QuakeCam.spos.boxNumber = 0;
				return;
			}

		}
		else
		{
			cn = CurrentSplineCamera;

			CameraXposition[1] = InitialCameraPosition.x;
			CameraYposition[1] = InitialCameraPosition.y;
			CameraZposition[1] = InitialCameraPosition.z;
			CameraXtarget[1] = InitialCameraTarget.x;
			CameraYtarget[1] = InitialCameraTarget.y;
			CameraZtarget[1] = InitialCameraTarget.z;
			CameraFOV[1] = CurrentFOV;
			CameraRoll[1] = 0;
			CameraSpeed[1] = s->speed;

			CameraXposition[2] = InitialCameraPosition.x;
			CameraYposition[2] = InitialCameraPosition.y;
			CameraZposition[2] = InitialCameraPosition.z;
			CameraXtarget[2] = InitialCameraTarget.x;
			CameraYtarget[2] = InitialCameraTarget.y;
			CameraZtarget[2] = InitialCameraTarget.z;
			CameraFOV[2] = CurrentFOV;
			CameraRoll[2] = 0;
			CameraSpeed[2] = s->speed;

			CameraXposition[3] = SpotCam[CurrentSplineCamera].x;
			CameraYposition[3] = SpotCam[CurrentSplineCamera].y;
			CameraZposition[3] = SpotCam[CurrentSplineCamera].z;
			CameraXtarget[3] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[3] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[3] = SpotCam[CurrentSplineCamera].tz;
			CameraRoll[3] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[3] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[3] = SpotCam[CurrentSplineCamera].speed;

			SplineFromCamera = 1;

			cn++;

			if (LastCamera < cn)
			{
				cn = FirstCamera;
			}

			CameraXposition[4] = SpotCam[cn].x;
			CameraYposition[4] = SpotCam[cn].y;
			CameraZposition[4] = SpotCam[cn].z;

			CameraXtarget[4] = SpotCam[cn].tx;
			CameraYtarget[4] = SpotCam[cn].ty;
			CameraZtarget[4] = SpotCam[cn].tz;

			CameraRoll[4] = SpotCam[cn].roll;
			CameraFOV[4] = SpotCam[cn].fov;
			CameraSpeed[4] = SpotCam[cn].speed;
		}
	}

	if (s->flags & SCF_HIDE_LARA)
	{
		SpotcamDontDrawLara = true;
	}

	QuakeCam.spos.boxNumber = 0;
}

void CalculateSpotCameras()
{
	int cpx; // stack offset -96
	int cpy; // stack offset -92
	int cpz; // stack offset -88
	int ctx; // stack offset -84
	int cty; // stack offset -80
	int ctz; // stack offset -76
	int cspeed; // stack offset -72
	int cfov; // stack offset -68
	int croll; // stack offset -64
	SPOTCAM* s; // stack offset -60
	short spline_cnt; // $s3
	int next_spline_camera; // $s0
	int n; // $s5
	int dx; // $v1
	int dy; // $s0
	int dz; // $s1

	//{ // line 76, offset 0x38114
	int cs; // $s6
	int sp; // $s2
	int cp; // $fp
	int clen; // $s4
	int tlen; // $v1
	int cx; // $s1
	int cy; // $s0
	int cz; // $v0
	int lx; // stack offset -56
	int lz; // stack offset -52
	int ly; // stack offset -48
	int i; // $v1
	int var_2C;
	int ctype; // $s0
	int cn; // $s0


	CAMERA_INFO Backup;

	if (Lara.Control.Uncontrollable)
	{
		LaraItem->HitPoints = LaraHealth;
		Lara.air = LaraAir;
	}

	s = &SpotCam[FirstCamera];
	spline_cnt = 4;

	if ((s->flags & SCF_TRACKING_CAM))
	{
		spline_cnt = CurrentCameraCnt + 2;
	}

	//loc_37F64
	cpx = Spline(CurrentSplinePosition, &CameraXposition[1], spline_cnt);
	cpy = Spline(CurrentSplinePosition, &CameraYposition[1], spline_cnt);
	cpz = Spline(CurrentSplinePosition, &CameraZposition[1], spline_cnt);
	ctx = Spline(CurrentSplinePosition, &CameraXtarget[1], spline_cnt);
	cty = Spline(CurrentSplinePosition, &CameraYtarget[1], spline_cnt);
	ctz = Spline(CurrentSplinePosition, &CameraZtarget[1], spline_cnt);
	cspeed = Spline(CurrentSplinePosition, &CameraSpeed[1], spline_cnt);
	croll = Spline(CurrentSplinePosition, &CameraRoll[1], spline_cnt);
	cfov = Spline(CurrentSplinePosition, &CameraFOV[1], spline_cnt);

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_IN)
		&& CameraFade != CurrentSplineCamera)
	{
		CameraFade = CurrentSplineCamera;
	}

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_OUT)
		&& CameraFade != CurrentSplineCamera)
	{
		CameraFade = CurrentSplineCamera;
	}

	sp = 0;
	tlen = 0;
	clen = 0;
	cp = 0;
	int temp = 0x2000;

	if ((s->flags & SCF_TRACKING_CAM))
	{
		lx = LaraItem->Position.xPos;
		ly = LaraItem->Position.yPos;
		lz = LaraItem->Position.zPos;

		for (int i = 0; i < 8; i++)
		{
			clen = 0x10000;

			for (int j = 0; j < 8; j++)
			{
				cx = Spline(sp, &CameraXposition[1], spline_cnt);
				cy = Spline(sp, &CameraYposition[1], spline_cnt);
				cz = Spline(sp, &CameraZposition[1], spline_cnt);

				dx = SQUARE(cx - lx);
				dy = SQUARE(cy - ly);
				dz = SQUARE(cz - lz);

				tlen = sqrt(dx + dy + dz);

				if (tlen <= clen)
				{
					cp = sp;
					clen = tlen;
				}

				sp += temp;

				if (sp > 0x10000)
					break;
			}

			temp >>= 1;
			sp = cp - 2 * (temp & 0xFE); // << 2 ?

			if (sp < 0)
				sp = 0;
		}

		CurrentSplinePosition += (cp - CurrentSplinePosition) >> 5;

		if ((s->flags & SCF_CUT_PAN))
		{
			if (abs(cp - CurrentSplinePosition) > 0x8000)
				CurrentSplinePosition = cp;
		}

		if (CurrentSplinePosition > 0x10000)
			CurrentSplinePosition = 0x10000;
		else if (CurrentSplinePosition < 0)
			CurrentSplinePosition = 0;
	}
	else if (!SpotcamTimer)
	{
		CurrentSplinePosition += cspeed;
	}

	if (!(TrInput & IN_LOOK))
	{
		SpotCamFirstLook = false;
	}

	if (s->flags & SCF_DISABLE_BREAKOUT
		|| !(TrInput & IN_LOOK))
	{
		Camera.pos.x = cpx;
		Camera.pos.y = cpy;
		Camera.pos.z = cpz;

		if ((s->flags & SCF_FOCUS_LARA_HEAD) || (s->flags & SCF_TRACKING_CAM))
		{
			Camera.target.x = LaraItem->Position.xPos;
			Camera.target.y = LaraItem->Position.yPos;
			Camera.target.z = LaraItem->Position.zPos;
		}
		else
		{
			Camera.target.x = ctx;
			Camera.target.y = cty;
			Camera.target.z = ctz;
		}

		auto outsideRoom = IsRoomOutside(cpx, cpy, cpz);
		if (outsideRoom == NO_ROOM)
		{
			Camera.pos.roomNumber = SpotCam[CurrentSplineCamera].roomNumber;
			GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
		}
		else
		{
			Camera.pos.roomNumber = outsideRoom;
		}

		AlterFOV(cfov);

		// WTF?
		if (QuakeCam.spos.boxNumber != 0)
		{
			dx = (Camera.pos.x - QuakeCam.epos.x);
			dy = (Camera.pos.y - QuakeCam.epos.y);
			dz = (Camera.pos.z - QuakeCam.epos.z);

			if (sqrt(SQUARE(dx) * SQUARE(dy) * SQUARE(dz)) < QuakeCam.epos.boxNumber)
			{
				dz = QuakeCam.spos.roomNumber + (((QuakeCam.epos.roomNumber - QuakeCam.spos.roomNumber) * -QuakeCam.epos.boxNumber) / QuakeCam.epos.boxNumber) >> 1;
				dy = QuakeCam.spos.roomNumber + (((QuakeCam.epos.roomNumber - QuakeCam.spos.roomNumber) * -QuakeCam.epos.boxNumber) / QuakeCam.epos.boxNumber);
				if (dy > 0)
				{
					Camera.pos.x += (GetRandomControl() / dy) - dz;
					Camera.pos.y += (GetRandomControl() / dy) - dz;
					Camera.pos.z += (GetRandomControl() / dy) - dz;
				}
			}
		}

		LookAt(&Camera, 0);

		if (CheckTrigger)
		{
			CAMERA_TYPE oldType = Camera.type;
			Camera.type = CAMERA_TYPE::HEAVY_CAMERA;
			if (CurrentLevel != 0)
			{
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, true);
				TestVolumes(&Camera);
			}
			else
			{
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, false);
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, true);
				TestVolumes(&Camera);
			}
			Camera.type = oldType;
			CheckTrigger = false;
		}

		if (s->flags & SCF_TRACKING_CAM)
		{
			TrackCameraInit = true;
		}
		else if (CurrentSplinePosition > 0x10000 - cspeed)
		{
			if (SpotCam[CurrentSplineCamera].timer > 0 &&
				SpotCam[CurrentSplineCamera].flags & SCF_STOP_MOVEMENT)
			{
				if (!SpotcamTimer && !SpotcamPaused)
					SpotcamTimer = SpotCam[CurrentSplineCamera].timer >> 3;
			}
			else if (SpotCam[CurrentSplineCamera].timer < 0)
			{
				SpotcamOverlay = 1; // Negative timer = sniper mode?
			}

			if (SpotCam[CurrentSplineCamera].flags & SCF_HIDE_LARA)
				SpotcamDontDrawLara = true;
			if (SpotCam[CurrentSplineCamera].flags & SCF_ACTIVATE_HEAVY_TRIGGERS)
				CheckTrigger = true;

			/* // Weird code which possibly did some shaking over the course of camera
			if (SpotCam[CurrentSplineCamera].flags & SCF_STOP_MOVEMENT)
			{
				if (QuakeCam.spos.boxNumber == 0 || SpotCam[CurrentSplineCamera].timer != -1)
				{
					QuakeCam.spos.x = SpotCam[CurrentSplineCamera].x;
					QuakeCam.spos.y = SpotCam[CurrentSplineCamera].y;
					QuakeCam.spos.z = SpotCam[CurrentSplineCamera].z;

					if (SpotCam[CurrentSplineCamera].timer != -1)
					{
						QuakeCam.spos.roomNumber = SpotCam[CurrentSplineCamera].timer << 3;
					}
					else
					{
						QuakeCam.spos.roomNumber = 0;
					}
					QuakeCam.spos.boxNumber = 1;
					QuakeCam.epos.x = SpotCam[CurrentSplineCamera + 1].x;
					QuakeCam.epos.y = SpotCam[CurrentSplineCamera + 1].y;
					QuakeCam.epos.z = SpotCam[CurrentSplineCamera + 1].z;

					if (SpotCam[CurrentSplineCamera + 1].timer != -1)
					{
						QuakeCam.epos.roomNumber = SpotCam[CurrentSplineCamera + 1].timer << 3;
					}
					else
					{
						QuakeCam.epos.roomNumber = 0;
					}

					QuakeCam.epos.boxNumber = sqrt(((QuakeCam.spos.x - QuakeCam.epos.x) * (QuakeCam.spos.x - QuakeCam.epos.x)) + ((QuakeCam.spos.y - QuakeCam.epos.y) * (QuakeCam.spos.y - QuakeCam.epos.y) + ((QuakeCam.spos.z - QuakeCam.epos.z) * (QuakeCam.spos.z - QuakeCam.epos.z))));
				}
				else
				{
					QuakeCam.spos.boxNumber = 0;
				}
			}
			*/

			if (!SpotcamTimer)
			{
				CurrentSplinePosition = 0;

				if (CurrentSplineCamera != FirstCamera)
				{
					cn = CurrentSplineCamera - 1;
				}
				else
				{
					cn = LastCamera;
				}

				sp = 1;

				if (SplineFromCamera != 0)
				{
					SplineFromCamera = 0;
					cn = FirstCamera - 1;
				}
				else
				{
					if ((SpotCam[CurrentSplineCamera].flags & SCF_REENABLE_LARA_CONTROLS))
					{
						Lara.Control.Uncontrollable = false;
					}

					if ((SpotCam[CurrentSplineCamera].flags & SCF_DISABLE_LARA_CONTROLS))
					{						
						if (CurrentLevel)
							SetCinematicBars(SPOTCAM_CINEMATIC_BARS_HEIGHT, SPOTCAM_CINEMATIC_BARS_SPEED);
						Lara.Control.Uncontrollable = true;
					}

					int sp2 = 0;
					if ((SpotCam[CurrentSplineCamera].flags & SCF_CUT_TO_CAM))
					{
						cn = (SpotCam[CurrentSplineCamera].timer & 0xF) + FirstCamera;

						CameraXposition[1] = SpotCam[cn].x;
						CameraYposition[1] = SpotCam[cn].y;
						CameraZposition[1] = SpotCam[cn].z;
						CameraXtarget[1] = SpotCam[cn].tx;
						CameraYtarget[1] = SpotCam[cn].ty;
						CameraZtarget[1] = SpotCam[cn].tz;
						CameraRoll[1] = SpotCam[cn].roll;
						CameraFOV[1] = SpotCam[cn].fov;
						CameraSpeed[1] = SpotCam[cn].speed;
						sp2 = 1;
						CurrentSplineCamera = cn;
					}

					sp = sp2 + 1;

					CameraXposition[sp] = SpotCam[cn].x;
					CameraYposition[sp] = SpotCam[cn].y;
					CameraZposition[sp] = SpotCam[cn].z;
					CameraXtarget[sp] = SpotCam[cn].tx;
					CameraYtarget[sp] = SpotCam[cn].ty;
					CameraZtarget[sp] = SpotCam[cn].tz;
					CameraRoll[sp] = SpotCam[cn].roll;
					CameraFOV[sp] = SpotCam[cn].fov;
					CameraSpeed[sp] = SpotCam[cn].speed;
				}

				cn++;
				if (sp < 4)
				{
					while (sp < 4)
					{
						if ((s->flags & SCF_LOOP_SEQUENCE))
						{
							if (LastCamera < cn)
							{
								cn = FirstCamera;
							}
						}
						else
						{
							if (LastCamera < cn)
							{
								cn = LastCamera;
							}
						}

						CameraXposition[sp + 1] = SpotCam[cn].x;
						CameraYposition[sp + 1] = SpotCam[cn].y;
						CameraZposition[sp + 1] = SpotCam[cn].z;
						CameraXtarget[sp + 1] = SpotCam[cn].tx;
						CameraYtarget[sp + 1] = SpotCam[cn].ty;
						CameraZtarget[sp + 1] = SpotCam[cn].tz;
						CameraRoll[sp + 1] = SpotCam[cn].roll;
						CameraFOV[sp + 1] = SpotCam[cn].fov;
						CameraSpeed[sp + 1] = SpotCam[cn].speed;
						cn++;
						sp++;
					}
				}

				CurrentSplineCamera++;
				SpotcamPaused = 0;

				if (LastCamera >= CurrentSplineCamera)
				{
					return;
				}

				if ((s->flags & SCF_LOOP_SEQUENCE))
				{
					CurrentSplineCamera = FirstCamera;
					SpotcamLoopCnt++;
				}
				else if (s->flags & SCF_PAN_TO_LARA_CAM || SplineToCamera)
				{
					if (CheckTrigger)
					{
						CAMERA_TYPE oldType = Camera.type;
						Camera.type = CAMERA_TYPE::HEAVY_CAMERA;
						if (CurrentLevel)
						{
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, true);
							TestVolumes(&Camera);
						}
						else
						{
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, false);
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, true);
							TestVolumes(&Camera);
						}
						Camera.type = oldType;
						CheckTrigger = false;
					}

					SetCinematicBars(0.0f, SPOTCAM_CINEMATIC_BARS_SPEED);

					UseSpotCam = 0;
					Lara.Control.Uncontrollable = false;
					CheckTrigger = 0;
					Camera.oldType = CAMERA_TYPE::FIXED_CAMERA;
					Camera.type = CAMERA_TYPE::CHASE_CAMERA;
					Camera.speed = 1;

					if (s->flags & SCF_PAN_TO_LARA_CAM)
					{
						Camera.pos.x = InitialCameraPosition.x;
						Camera.pos.y = InitialCameraPosition.y;
						Camera.pos.z = InitialCameraPosition.z;
						Camera.target.x = InitialCameraTarget.x;
						Camera.target.y = InitialCameraTarget.y;
						Camera.target.z = InitialCameraTarget.z;
						Camera.pos.roomNumber = InitialCameraRoom;
					}

					SpotcamOverlay = false;
					SpotcamDontDrawLara = false;
					AlterFOV(LastFOV);
				}
				else
				{
					CameraXposition[1] = SpotCam[CurrentSplineCamera - 1].x;
					CameraYposition[1] = SpotCam[CurrentSplineCamera - 1].y;
					CameraZposition[1] = SpotCam[CurrentSplineCamera - 1].z;
					CameraXtarget[1] = SpotCam[CurrentSplineCamera - 1].tx;
					CameraYtarget[1] = SpotCam[CurrentSplineCamera - 1].ty;
					CameraZtarget[1] = SpotCam[CurrentSplineCamera - 1].tz;
					CameraRoll[1] = SpotCam[CurrentSplineCamera - 1].roll;
					CameraFOV[1] = SpotCam[CurrentSplineCamera - 1].fov;
					CameraSpeed[1] = SpotCam[CurrentSplineCamera - 1].speed;

					CameraXposition[2] = SpotCam[CurrentSplineCamera - 1].x;
					CameraYposition[2] = SpotCam[CurrentSplineCamera - 1].y;
					CameraZposition[2] = SpotCam[CurrentSplineCamera - 1].z;
					CameraXtarget[2] = SpotCam[CurrentSplineCamera - 1].tx;
					CameraYtarget[2] = SpotCam[CurrentSplineCamera - 1].ty;
					CameraZtarget[2] = SpotCam[CurrentSplineCamera - 1].tz;
					CameraRoll[2] = SpotCam[CurrentSplineCamera - 1].roll;
					CameraFOV[2] = SpotCam[CurrentSplineCamera - 1].fov;
					CameraSpeed[2] = SpotCam[CurrentSplineCamera - 1].speed;

					memcpy((char*)& Backup, (char*)& Camera, sizeof(CAMERA_INFO));
					Camera.oldType = CAMERA_TYPE::FIXED_CAMERA;
					Camera.type = CAMERA_TYPE::CHASE_CAMERA;
					Camera.speed = 1;

					int elevation = Camera.targetElevation;

					CalculateCamera();

					CameraRoll[2] = 0;
					CameraRoll[3] = 0;
					CameraSpeed[2] = CameraSpeed[1];

					InitialCameraPosition.x = Camera.pos.x;
					InitialCameraPosition.y = Camera.pos.y;
					InitialCameraPosition.z = Camera.pos.z;

					InitialCameraTarget.x = Camera.target.x;
					InitialCameraTarget.y = Camera.target.y;
					InitialCameraTarget.z = Camera.target.z;

					CameraXposition[3] = Camera.pos.x;
					CameraYposition[3] = Camera.pos.y;
					CameraZposition[3] = Camera.pos.z;
					CameraXtarget[3] = Camera.target.x;
					CameraYtarget[3] = Camera.target.y;
					CameraZtarget[3] = Camera.target.z;
					CameraFOV[3] = CurrentFOV;
					CameraSpeed[3] = CameraSpeed[2];
					CameraRoll[3] = 0;

					CameraXposition[4] = Camera.pos.x;
					CameraYposition[4] = Camera.pos.y;
					CameraZposition[4] = Camera.pos.z;
					CameraXtarget[4] = Camera.target.x;
					CameraYtarget[4] = Camera.target.y;
					CameraZtarget[4] = Camera.target.z;
					CameraFOV[4] = CurrentFOV;
					CameraSpeed[4] = CameraSpeed[2] >> 1;
					CameraRoll[4] = 0;

					memcpy((char*)& Camera, (char*)& Backup, sizeof(CAMERA_INFO));

					Camera.targetElevation = elevation;

					LookAt(&Camera, croll);

					SplineToCamera = 1;
				}
			}
			else
			{
				SpotcamTimer--;
				if (!SpotcamTimer)
					SpotcamPaused = 1;
			}
		}
	}
	else if (s->flags & SCF_TRACKING_CAM)
	{
		if (!SpotCamFirstLook)
		{
			Camera.oldType = CAMERA_TYPE::FIXED_CAMERA;
			SpotCamFirstLook = true;
		}

		CalculateCamera();
	}
	else
	{
		SetCinematicBars(0.0f, SPOTCAM_CINEMATIC_BARS_SPEED);
		UseSpotCam = false;
		Lara.Control.Uncontrollable = false;
		Camera.speed = 1;
		AlterFOV(LastFOV);
		CalculateCamera();
		CheckTrigger = false;
	}
}

#if 0
int Spline(int x, int* knots, int nk)//Monty's version?
{
	/*int num = nk - 1;

	float gamma[100];
	float delta[100];
	float D[100]; 

	gamma[0] = 1.0f / 2.0f;
	for (int i = 1; i < num; i++)
		gamma[i] = 1.0f / (4.0f - gamma[i - 1]);

	gamma[num] = 1.0f / (2.0f - gamma[num - 1]);

	float p0 = knots[0];
	float p1 = knots[1];

	delta[0] = 3.0f * (p1 - p0) * gamma[0];

	for (int i = 1; i < num; i++)
	{
		p0 = knots[i - 1];
		p1 = knots[i + 1];
		delta[i] = (3.0f * (p1 - p0) - delta[i - 1]) * gamma[i];
	}

	p0 = knots[num - 1];
	p1 = knots[num];

	delta[num] = (3.0f * (p1 - p0) - delta[num - 1]) * gamma[num];

	D[num] = delta[num];
	for (int i = num - 1; i >= 0; i--)
		D[i] = delta[i] - gamma[i] * D[i + 1];

	p0 = knots[0];
	p1 = knots[1];

	float a = p0;
	float b = D[0];
	float c = 3 * (p1 - p0) - 2 * D[0] - D[1];
	float d = 2 * (p0 - p1) + D[0] + D[1];

	return ((((d * x) + c) * x + b) * x + a);

	/*int num = nk - 1;

	float gamma = 0;
	float delta = 0;
	float D = 0;

	gamma = 1.0f / 2.0f;
	for (int i = 1; i < num; i++)
		gamma[i] = 1.0f / (4.0f - gamma[i - 1]);

	gamma[num] = 1.0f / (2.0f - gamma[num - 1]);

	float p0 = knots[0];
	float p1 = knots[1];

	delta[0] = 3.0f * (p1 - p0) * gamma[0];

	for (int i = 1; i < num; i++)
	{
		p0 = knots[i - 1];
		p1 = knots[i + 1];
		delta[i] = (3.0f * (p1 - p0) - delta[i - 1]) * gamma[i];
	}

	p0 = points[num - 1];
	p1 = points[num];

	delta[num] = (3.0f * (p1 - p0) - delta[num - 1]) * gamma[num];

	D[num] = delta[num];
	for (int i = num - 1; i >= 0; i--)
		D[i] = delta[i] - gamma[i] * D[i + 1];

	float D1 = 
	float D0 = delta - gamma * D1;

	float a = p0;
	float b = D;
	float c = 3 * (p1 - p0) - 2 * D0 - D1;
	float d = 2 * (p0 - p1) + D0 + D1;

	return ((((d * x) + c) * x + b) * x + a);
	*/
}
#else
int Spline(int x, int* knots, int nk)//Core's version, *proper* decompilation by ChocolateFan
{
	int* k;
	int span, c1, c2;

	span = x * (nk - 3) >> 16;

	if (span >= nk - 3)
		span = nk - 4;

	k = &knots[span];
	x = x * (nk - 3) - span * 65536;
	c1 = (k[1] >> 1) - (k[2] >> 1) - k[2] + k[1] + (k[3] >> 1) + ((-k[0] - 1) >> 1);
	c2 = 2 * k[2] - 2 * k[1] - (k[1] >> 1) - (k[3] >> 1) + k[0];
	return ((__int64)x * (((__int64)x * (((__int64)x * c1 >> 16) + c2) >> 16) + (k[2] >> 1) + ((-k[0] - 1) >> 1)) >> 16) + k[1];
}
#endif
