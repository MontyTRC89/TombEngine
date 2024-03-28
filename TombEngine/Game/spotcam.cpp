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
#include "Math/Math.h"
#include "Specific/Input/Input.h"

using namespace TEN::Control::Volumes;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

constexpr auto MAX_CAMERA = 18;

// Globals
bool TrackCameraInit;
int SpotcamTimer;
bool SpotcamPaused;
int SpotcamLoopCnt;
int CameraFade;
Vector3i LaraFixedPosition;
int InitialCameraRoom;
Vector3i InitialCameraPosition;
Vector3i InitialCameraTarget;
int CurrentSplinePosition;
int SplineToCamera;
int FirstCamera;
int LastCamera;
int CurrentCameraCnt;
Vector3i CameraPosition[MAX_CAMERA];
Vector3i CameraPositionTarget[MAX_CAMERA];
int CameraRoll[MAX_CAMERA];
int CameraFOV[MAX_CAMERA];
int CameraSpeed[MAX_CAMERA];
int SplineFromCamera;
bool SpotCamFirstLook;
short CurrentSplineCamera;
int LastSpotCamSequence;
int LaraHealth;
int LaraAir;
int CurrentSpotcamSequence;
SPOTCAM SpotCam[MAX_SPOTCAMS];
int SpotCamRemap[MAX_SPOTCAMS];
int CameraCnt[MAX_SPOTCAMS];
int NumberSpotcams;

bool CheckTrigger = false;
bool UseSpotCam = false;
bool SpotcamDontDrawLara = false;
bool SpotcamOverlay = false;

void ClearSpotCamSequences()
{
	UseSpotCam = false;
	SpotcamDontDrawLara = false;
	SpotcamOverlay = false;

	for (int i = 0; i < MAX_SPOTCAMS; i++)
		SpotCam[i] = {};
}

void InitializeSpotCamSequences(bool startFirstSequence)
{
	TrackCameraInit = false;

	int spotCamCount = NumberSpotcams;
	int cc = 1;

	if (spotCamCount != 0)
	{
		int ce = 0;
		int sequenceID = SpotCam[0].sequence;

		if (cc < spotCamCount)
		{
			for (spotCamCount = 1; spotCamCount < NumberSpotcams; spotCamCount++)
			{
				// Same sequence.
				if (SpotCam[spotCamCount].sequence == sequenceID)
				{
					cc++;
				}
				// New sequence.
				else
				{
					CameraCnt[ce] = cc;
					cc = 1;
					SpotCamRemap[sequenceID] = ce;
					ce++;
					sequenceID = SpotCam[spotCamCount].sequence;
				}
			}
		}

		CameraCnt[ce] = cc;
		SpotCamRemap[sequenceID] = ce;
	}

	if (startFirstSequence)
	{
		InitializeSpotCam(0);
		UseSpotCam = true;
	}
}

void InitializeSpotCam(short sequenceID)
{
	if (TrackCameraInit != 0 && LastSpotCamSequence == sequenceID)
	{
		TrackCameraInit = false;
		return;
	}

	Lara.Control.Look.OpticRange = 0;
	Lara.Control.Look.IsUsingLasersight = false;

	AlterFOV(ANGLE(DEFAULT_FOV), false);

	LaraItem->MeshBits = ALL_JOINT_BITS;

	ResetPlayerFlex(LaraItem);

	Camera.bounce = 0;

	Lara.Inventory.IsBusy = 0;

	CameraFade = NO_VALUE;
	LastSpotCamSequence = sequenceID;
	TrackCameraInit = false;
	SpotcamTimer = 0;
	SpotcamPaused = false;
	SpotcamLoopCnt = 0;
	Lara.Control.IsLocked = false;

	LaraAir = Lara.Status.Air;

	InitialCameraPosition.x = Camera.pos.x;
	InitialCameraPosition.y = Camera.pos.y;
	InitialCameraPosition.z = Camera.pos.z;

	InitialCameraTarget.x = Camera.target.x;
	InitialCameraTarget.y = Camera.target.y;
	InitialCameraTarget.z = Camera.target.z;

	LaraHealth = LaraItem->HitPoints;
	InitialCameraRoom = Camera.pos.RoomNumber;

	LaraFixedPosition.x = LaraItem->Pose.Position.x;
	LaraFixedPosition.y = LaraItem->Pose.Position.y;
	LaraFixedPosition.z = LaraItem->Pose.Position.z;

	CurrentSpotcamSequence = sequenceID;
	CurrentSplineCamera = 0;

	for (int i = 0; i < SpotCamRemap[sequenceID]; i++)
		CurrentSplineCamera += CameraCnt[i];

	CurrentSplinePosition = 0;
	SplineToCamera = 0;

	FirstCamera = CurrentSplineCamera;

	const auto* spotCamPtr = &SpotCam[CurrentSplineCamera];

	LastCamera = CurrentSplineCamera + (CameraCnt[SpotCamRemap[sequenceID]] - 1);
	CurrentCameraCnt = CameraCnt[SpotCamRemap[sequenceID]];

	if ((spotCamPtr->flags & SCF_DISABLE_LARA_CONTROLS))
	{
		Lara.Control.IsLocked = true;
		SetCinematicBars(1.0f, SPOTCAM_CINEMATIC_BARS_SPEED);
	}

	if (spotCamPtr->flags & SCF_TRACKING_CAM)
	{
		CameraPosition[1] = SpotCam[FirstCamera].Position;
		CameraPositionTarget[1] = SpotCam[FirstCamera].PositionTarget;
		CameraRoll[1] = SpotCam[FirstCamera].roll;
		CameraFOV[1] = SpotCam[FirstCamera].fov;
		CameraSpeed[1] = SpotCam[FirstCamera].speed;

		SplineFromCamera = 0;

		if (CurrentCameraCnt > 0)
		{
			spotCamPtr = &SpotCam[FirstCamera];

			for (int i = 0; i < CurrentCameraCnt; i++, spotCamPtr++)
			{
				CameraPosition[i + 2] = spotCamPtr->Position;
				CameraPositionTarget[i + 2] = spotCamPtr->PositionTarget;
				CameraRoll[i + 2] = spotCamPtr->roll;
				CameraFOV[i + 2] = spotCamPtr->fov;
				CameraSpeed[i + 2] = spotCamPtr->speed;
			}
		}

		CameraPosition[CurrentCameraCnt + 2] = SpotCam[LastCamera].Position;
		CameraPositionTarget[CurrentCameraCnt + 2] = SpotCam[LastCamera].PositionTarget;
		CameraFOV[CurrentCameraCnt + 2] = SpotCam[LastCamera].fov;
		CameraRoll[CurrentCameraCnt + 2] = SpotCam[LastCamera].roll;
		CameraSpeed[CurrentCameraCnt + 2] = SpotCam[LastCamera].speed;
	}
	else
	{
		int sp = 0;
		if ((spotCamPtr->flags & SCF_CUT_PAN))
		{
			CameraPosition[1] = SpotCam[CurrentSplineCamera].Position;
			CameraPositionTarget[1] = SpotCam[CurrentSplineCamera].PositionTarget;
			CameraRoll[1] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[1] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[1] = SpotCam[CurrentSplineCamera].speed;

			SplineFromCamera = 0;

			int cn = CurrentSplineCamera;
			while (sp < 4)
			{
				if (LastCamera < CurrentSplineCamera)
					cn = FirstCamera;

				CameraPosition[sp + 2] = SpotCam[cn].Position;
				CameraPositionTarget[sp + 2] = SpotCam[cn].PositionTarget;
				CameraRoll[sp + 2] = SpotCam[cn].roll;
				CameraFOV[sp + 2] = SpotCam[cn].fov;
				CameraSpeed[sp + 2] = SpotCam[cn].speed;
				cn++;
				sp++;
			}

			CurrentSplineCamera++;

			if (CurrentSplineCamera > LastCamera)
				CurrentSplineCamera = FirstCamera;

			if (spotCamPtr->flags & SCF_ACTIVATE_HEAVY_TRIGGERS)
				CheckTrigger = true;

			if (spotCamPtr->flags & SCF_HIDE_LARA)
				SpotcamDontDrawLara = true;
		}
		else
		{
			int cn = CurrentSplineCamera;

			CameraPosition[1] = InitialCameraPosition;
			CameraPositionTarget[1] = InitialCameraTarget;
			CameraFOV[1] = CurrentFOV;
			CameraRoll[1] = 0;
			CameraSpeed[1] = spotCamPtr->speed;

			CameraPosition[2] = InitialCameraPosition;
			CameraPositionTarget[2] = InitialCameraTarget;
			CameraFOV[2] = CurrentFOV;
			CameraRoll[2] = 0;
			CameraSpeed[2] = spotCamPtr->speed;

			CameraPosition[3] = SpotCam[CurrentSplineCamera].Position;
			CameraPositionTarget[3] = SpotCam[CurrentSplineCamera].PositionTarget;
			CameraRoll[3] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[3] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[3] = SpotCam[CurrentSplineCamera].speed;

			SplineFromCamera = 1;

			cn++;

			if (LastCamera < cn)
				cn = FirstCamera;

			CameraPosition[4] = SpotCam[cn].Position;
			CameraPositionTarget[4] = SpotCam[cn].PositionTarget;

			CameraRoll[4] = SpotCam[cn].roll;
			CameraFOV[4] = SpotCam[cn].fov;
			CameraSpeed[4] = SpotCam[cn].speed;
		}
	}

	if (spotCamPtr->flags & SCF_HIDE_LARA)
		SpotcamDontDrawLara = true;
}

void CalculateSpotCameras()
{
	auto backup = CAMERA_INFO{};

	if (Lara.Control.IsLocked)
	{
		LaraItem->HitPoints = LaraHealth;
		Lara.Status.Air = LaraAir;
	}

	auto* s = &SpotCam[FirstCamera];
	int splineCount = 4;

	if (s->flags & SCF_TRACKING_CAM)
		splineCount = CurrentCameraCnt + 2;

	int cpx = Spline(CurrentSplinePosition, &CameraPosition[1].x, splineCount);
	int cpy = Spline(CurrentSplinePosition, &CameraPosition[1].y, splineCount);
	int cpz = Spline(CurrentSplinePosition, &CameraPosition[1].z, splineCount);
	int ctx = Spline(CurrentSplinePosition, &CameraPositionTarget[1].x, splineCount);
	int cty = Spline(CurrentSplinePosition, &CameraPositionTarget[1].y, splineCount);
	int ctz = Spline(CurrentSplinePosition, &CameraPositionTarget[1].z, splineCount);
	int cspeed = Spline(CurrentSplinePosition, &CameraSpeed[1], splineCount);
	int croll = Spline(CurrentSplinePosition, &CameraRoll[1], splineCount);
	int cfov = Spline(CurrentSplinePosition, &CameraFOV[1], splineCount);

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_IN) &&
		CameraFade != CurrentSplineCamera)
	{
		SetScreenFadeIn(FADE_SCREEN_SPEED);
		CameraFade = CurrentSplineCamera;
	}

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_OUT) &&
		CameraFade != CurrentSplineCamera)
	{
		SetScreenFadeOut(FADE_SCREEN_SPEED);
		CameraFade = CurrentSplineCamera;
	}

	int sp = 0;
	int tlen = 0;
	int clen = 0;
	int cp = 0;
	int temp = 0x2000;

	if (s->flags & SCF_TRACKING_CAM)
	{
		int lx = LaraItem->Pose.Position.x;
		int ly = LaraItem->Pose.Position.y;
		int lz = LaraItem->Pose.Position.z;

		for (int i = 0; i < 8; i++)
		{
			clen = 0x10000;

			for (int j = 0; j < 8; j++)
			{
				int cx = Spline(sp, &CameraPosition[1].x, splineCount);
				int cy = Spline(sp, &CameraPosition[1].y, splineCount);
				int cz = Spline(sp, &CameraPosition[1].z, splineCount);

				int dx = SQUARE(cx - lx);
				int dy = SQUARE(cy - ly);
				int dz = SQUARE(cz - lz);

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
		{
			CurrentSplinePosition = 0x10000;
		}
		else if (CurrentSplinePosition < 0)
		{
			CurrentSplinePosition = 0;
		}
	}
	else if (!SpotcamTimer)
	{
		CurrentSplinePosition += cspeed;
	}

	if (!IsHeld(In::Look))
		SpotCamFirstLook = false;

	if ((s->flags & SCF_DISABLE_BREAKOUT) || !IsHeld(In::Look))
	{
		Camera.pos.x = cpx;
		Camera.pos.y = cpy;
		Camera.pos.z = cpz;

		if ((s->flags & SCF_FOCUS_LARA_HEAD) || (s->flags & SCF_TRACKING_CAM))
		{
			Camera.target.x = LaraItem->Pose.Position.x;
			Camera.target.y = LaraItem->Pose.Position.y;
			Camera.target.z = LaraItem->Pose.Position.z;
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
			Camera.pos.RoomNumber = SpotCam[CurrentSplineCamera].roomNumber;
			GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.RoomNumber);
		}
		else
		{
			Camera.pos.RoomNumber = outsideRoom;
		}

		AlterFOV(cfov, false);

		LookAt(&Camera, croll);
		UpdateMikePos(*LaraItem);

		if (SpotCam[CurrentSplineCamera].flags & SCF_OVERLAY)
			SpotcamOverlay = true;

		if (SpotCam[CurrentSplineCamera].flags & SCF_HIDE_LARA)
			SpotcamDontDrawLara = true;

		if (SpotCam[CurrentSplineCamera].flags & SCF_ACTIVATE_HEAVY_TRIGGERS)
			CheckTrigger = true;

		if (CheckTrigger)
		{
			CameraType oldType = Camera.type;
			Camera.type = CameraType::Heavy;
			if (CurrentLevel != 0)
			{
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, true);
				TestVolumes(&Camera);
			}
			else
			{
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, false);
				TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, true);
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
			
			if (!SpotcamTimer)
			{
				CurrentSplinePosition = 0;

				int cn = 0;
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
					if (SpotCam[CurrentSplineCamera].flags & SCF_REENABLE_LARA_CONTROLS)
						Lara.Control.IsLocked = false;

					if (SpotCam[CurrentSplineCamera].flags & SCF_DISABLE_LARA_CONTROLS)
					{						
						if (CurrentLevel)
							SetCinematicBars(1.0f, SPOTCAM_CINEMATIC_BARS_SPEED);

						Lara.Control.IsLocked = true;
					}

					int sp2 = 0;
					if (SpotCam[CurrentSplineCamera].flags & SCF_CUT_TO_CAM)
					{
						cn = FirstCamera + SpotCam[CurrentSplineCamera].timer;

						CameraPosition[1] = SpotCam[cn].Position;
						CameraPositionTarget[1] = SpotCam[cn].PositionTarget;
						CameraRoll[1] = SpotCam[cn].roll;
						CameraFOV[1] = SpotCam[cn].fov;
						CameraSpeed[1] = SpotCam[cn].speed;
						sp2 = 1;
						CurrentSplineCamera = cn;
					}

					sp = sp2 + 1;

					CameraPosition[sp] = SpotCam[cn].Position;
					CameraPositionTarget[sp] = SpotCam[cn].PositionTarget;
					CameraRoll[sp] = SpotCam[cn].roll;
					CameraFOV[sp] = SpotCam[cn].fov;
					CameraSpeed[sp] = SpotCam[cn].speed;
				}

				cn++;
				if (sp < 4)
				{
					while (sp < 4)
					{
						if (s->flags & SCF_LOOP_SEQUENCE)
						{
							if (LastCamera < cn)
								cn = FirstCamera;
						}
						else
						{
							if (LastCamera < cn)
								cn = LastCamera;
						}

						CameraPosition[sp + 1] = SpotCam[cn].Position;
						CameraPositionTarget[sp + 1] = SpotCam[cn].PositionTarget;
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
					return;

				if (s->flags & SCF_LOOP_SEQUENCE)
				{
					CurrentSplineCamera = FirstCamera;
					SpotcamLoopCnt++;
				}
				else if (s->flags & SCF_CUT_TO_LARA_CAM || SplineToCamera)
				{
					if (CheckTrigger)
					{
						CameraType oldType = Camera.type;
						Camera.type = CameraType::Heavy;
						if (CurrentLevel)
						{
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, true);
							TestVolumes(&Camera);
						}
						else
						{
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, false);
							TestTriggers(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.RoomNumber, true);
							TestVolumes(&Camera);
						}

						Camera.type = oldType;
						CheckTrigger = false;
					}

					SetCinematicBars(0.0f, SPOTCAM_CINEMATIC_BARS_SPEED);

					UseSpotCam = false;
					Lara.Control.IsLocked = false;
					CheckTrigger = false;
					Camera.oldType = CameraType::Fixed;
					Camera.type = CameraType::Chase;
					Camera.speed = 1;

					if (s->flags & SCF_CUT_TO_LARA_CAM)
					{
						Camera.pos.x = InitialCameraPosition.x;
						Camera.pos.y = InitialCameraPosition.y;
						Camera.pos.z = InitialCameraPosition.z;
						Camera.target.x = InitialCameraTarget.x;
						Camera.target.y = InitialCameraTarget.y;
						Camera.target.z = InitialCameraTarget.z;
						Camera.pos.RoomNumber = InitialCameraRoom;
					}

					SpotcamOverlay = false;
					SpotcamDontDrawLara = false;
					AlterFOV(LastFOV);
				}
				else
				{
					CameraPosition[1] = SpotCam[CurrentSplineCamera - 1].Position;
					CameraPositionTarget[1] = SpotCam[CurrentSplineCamera - 1].PositionTarget;
					CameraRoll[1] = SpotCam[CurrentSplineCamera - 1].roll;
					CameraFOV[1] = SpotCam[CurrentSplineCamera - 1].fov;
					CameraSpeed[1] = SpotCam[CurrentSplineCamera - 1].speed;

					CameraPosition[2] = SpotCam[CurrentSplineCamera - 1].Position;
					CameraPositionTarget[2] = SpotCam[CurrentSplineCamera - 1].PositionTarget;
					CameraRoll[2] = SpotCam[CurrentSplineCamera - 1].roll;
					CameraFOV[2] = SpotCam[CurrentSplineCamera - 1].fov;
					CameraSpeed[2] = SpotCam[CurrentSplineCamera - 1].speed;

					memcpy((char*)& backup, (char*)& Camera, sizeof(CAMERA_INFO));
					Camera.oldType = CameraType::Fixed;
					Camera.type = CameraType::Chase;
					Camera.speed = 1;

					int elevation = Camera.targetElevation;

					CalculateCamera(LaraCollision);

					CameraRoll[2] = 0;
					CameraRoll[3] = 0;
					CameraSpeed[2] = CameraSpeed[1];

					InitialCameraPosition = Camera.pos.ToVector3i();
					InitialCameraTarget = Camera.target.ToVector3i();

					CameraPosition[3] = Camera.pos.ToVector3i();
					CameraPositionTarget[3] = Camera.target.ToVector3i();
					CameraFOV[3] = LastFOV;
					CameraSpeed[3] = CameraSpeed[2];
					CameraRoll[3] = 0;

					CameraPosition[4] = Camera.pos.ToVector3i();
					CameraPositionTarget[4] = Camera.target.ToVector3i();
					CameraFOV[4] = LastFOV;
					CameraSpeed[4] = CameraSpeed[2] >> 1;
					CameraRoll[4] = 0;

					memcpy((char*)& Camera, (char*)& backup, sizeof(CAMERA_INFO));

					Camera.targetElevation = elevation;

					LookAt(&Camera, croll);
					UpdateMikePos(*LaraItem);

					SplineToCamera = 1;
				}

				if (CurrentSplineCamera > LastCamera)
					CurrentSplineCamera = LastCamera;
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
			Camera.oldType = CameraType::Fixed;
			SpotCamFirstLook = true;
		}

		CalculateCamera(LaraCollision);
	}
	else
	{
		SetScreenFadeIn(FADE_SCREEN_SPEED);
		SetCinematicBars(0.0f, SPOTCAM_CINEMATIC_BARS_SPEED);
		UseSpotCam = false;
		Lara.Control.IsLocked = false;
		Camera.speed = 1;
		AlterFOV(LastFOV);
		CalculateCamera(LaraCollision);
		CheckTrigger = false;
	}
}

// Core's version. Proper decompilation by ChocolateFan
int Spline(int x, int* knots, int nk)
{
	int span = x * (nk - 3) >> 16;
	if (span >= nk - 3)
		span = nk - 4;

	int* k = &knots[span];
	x = x * (nk - 3) - span * 65536;

	int c1 = (k[1] >> 1) - (k[2] >> 1) - k[2] + k[1] + (k[3] >> 1) + ((-k[0] - 1) >> 1);
	int c2 = 2 * k[2] - 2 * k[1] - (k[1] >> 1) - (k[3] >> 1) + k[0];

	return ((__int64)x * (((__int64)x * (((__int64)x * c1 >> 16) + c2) >> 16) + (k[2] >> 1) + ((-k[0] - 1) >> 1)) >> 16) + k[1];
}
