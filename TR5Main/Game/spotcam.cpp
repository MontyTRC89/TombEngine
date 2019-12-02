#include "spotcam.h"

#include "..\Global\global.h"
#include "..\Game\Camera.h"
#include "..\Game\control.h"
#include "..\Game\draw.h"


void __cdecl InitSpotCamSequences() 
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

void __cdecl InitialiseSpotCam(short Sequence)
{
	SPOTCAM* s;
	int cn;
	int sp;
	int i;

	if (TrackCameraInit != 0 && LastSequence == Sequence)
	{
		TrackCameraInit = 0;
		return;
	}

	BinocularRange = 0;
	LaserSight = 0;

	AlterFOV(16380);

	LaraItem->meshBits = -1;

	Lara.headYrot = 0;
	Lara.headXrot = 0;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	Camera.bounce = 0;

	Lara.busy = 0;

	CameraFade = -1;
	LastSequence = Sequence;
	TrackCameraInit = 0;
	SpotcamTimer = 0;
	SpotcamLoopCnt = 0;
	DisableLaraControl = 0;

	LastFOV = CurrentFOV;
	LaraAir = Lara.air;

	InitialCameraPosition.x = Camera.pos.x;
	InitialCameraPosition.y = Camera.pos.y;
	InitialCameraPosition.z = Camera.pos.z;

	InitialCameraTarget.x = Camera.target.x;
	InitialCameraTarget.y = Camera.target.y;
	InitialCameraTarget.z = Camera.target.z;

	LaraHealth = LaraItem->hitPoints;
	InitialCameraRoom = Camera.pos.roomNumber;

	LaraFixedPosition.x = LaraItem->pos.xPos;
	LaraFixedPosition.y = LaraItem->pos.yPos;
	LaraFixedPosition.z = LaraItem->pos.zPos;

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

	if ((s->flags & SCF_DISABLE_LARA_CONTROLS) /*|| gfGameMode == 1*/)
	{
		DisableLaraControl = 1;
		/*if (gfGameMode != 1)
		{
			//SetFadeClip(16, 1);
		}*/
	}

	if (s->flags & SCF_TRACKING_CAM)
	{
		CameraXposition[0] = SpotCam[FirstCamera].x;
		CameraYposition[0] = SpotCam[FirstCamera].y;
		CameraZposition[0] = SpotCam[FirstCamera].z;

		CameraXtarget[0] = SpotCam[FirstCamera].tx;
		CameraYtarget[0] = SpotCam[FirstCamera].ty;
		CameraZtarget[0] = SpotCam[FirstCamera].tz;

		CameraXtarget[0] = SpotCam[FirstCamera].tx;
		CameraYtarget[0] = SpotCam[FirstCamera].ty;
		CameraZtarget[0] = SpotCam[FirstCamera].tz;

		CameraRoll[0] = SpotCam[FirstCamera].roll;
		CameraFOV[0] = SpotCam[FirstCamera].fov;

		SplineFromCamera = 0;
		CameraSpeed[0] = SpotCam[FirstCamera].speed;

		if (CurrentCameraCnt > 0) 
		{
			s = &SpotCam[FirstCamera];

			for (i = 1; i < CurrentCameraCnt + 1; i++, s++)
			{
				CameraXposition[i] = s->x;
				CameraYposition[i] = s->y;
				CameraZposition[i] = s->z;

				CameraXtarget[i] = s->tx;
				CameraYtarget[i] = s->ty;
				CameraZtarget[i] = s->tz;

				CameraXtarget[i] = s->tx;
				CameraYtarget[i] = s->ty;
				CameraZtarget[i] = s->tz;

				CameraRoll[i] = s->roll;
				CameraFOV[i] = s->fov;
				CameraSpeed[i] = s->speed;
			}
		}

		//loc_379F8
		CameraXposition[1] = SpotCam[LastCamera].x;
		CameraYposition[1] = SpotCam[LastCamera].y;
		CameraZposition[1] = SpotCam[LastCamera].z;

		CameraXtarget[1] = SpotCam[LastCamera].tx;
		CameraYtarget[1] = SpotCam[LastCamera].ty;
		CameraZtarget[1] = SpotCam[LastCamera].tz;

		CameraFOV[1] = SpotCam[LastCamera].fov;
		CameraRoll[1] = SpotCam[LastCamera].roll;
		CameraSpeed[1] = SpotCam[LastCamera].speed;
	}
	else
	{
		//loc_37AA8
		sp = 1;
		if ((s->flags & SCF_CUT_PAN))
		{
			CameraXposition[0] = SpotCam[CurrentSplineCamera].x;
			CameraYposition[0] = SpotCam[CurrentSplineCamera].y;
			CameraZposition[0] = SpotCam[CurrentSplineCamera].z;

			CameraXtarget[0] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[0] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[0] = SpotCam[CurrentSplineCamera].tz;

			CameraRoll[0] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[0] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[0] = SpotCam[CurrentSplineCamera].speed;

			SplineFromCamera = 0;

			cn = CurrentSplineCamera;
			while (sp < 4)
			{
				if (LastCamera < CurrentSplineCamera)
				{
					cn = FirstCamera;
				}

				//loc_37B74
				CameraXposition[sp] = SpotCam[cn].x;
				CameraYposition[sp] = SpotCam[cn].y;
				CameraZposition[sp] = SpotCam[cn].z;

				CameraXtarget[sp] = SpotCam[cn].tx;
				CameraYtarget[sp] = SpotCam[cn].ty;
				CameraZtarget[sp] = SpotCam[cn].tz;

				CameraRoll[sp] = SpotCam[cn].roll;
				CameraFOV[sp] = SpotCam[cn].fov;
				CameraSpeed[sp] = SpotCam[cn].speed;
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

			if (s->flags & SCF_VIGNETTE)
			{
				/*if (s->timer < 0)
				{
					SCOverlay = 1;
				}//loc_37C8C
				else if (SlowMotion == 0)
				{
					SlowMotion = s->timer;
				}*/
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

			CameraRoll[0] = 0;
			CameraXposition[0] = InitialCameraPosition.x;
			CameraYposition[0] = InitialCameraPosition.y;
			CameraZposition[0] = InitialCameraPosition.z;
			CameraXtarget[0] = InitialCameraTarget.x;
			CameraYtarget[0] = InitialCameraTarget.y;
			CameraZtarget[0] = InitialCameraTarget.z;
			CameraFOV[0] = CurrentFOV;

			CameraXposition[1] = InitialCameraPosition.x;
			CameraXtarget[1] = InitialCameraTarget.x;
			CameraYposition[1] = InitialCameraPosition.y;
			CameraZposition[1] = InitialCameraPosition.z;
			CameraYtarget[1] = InitialCameraTarget.y;
			CameraZtarget[1] = InitialCameraTarget.z;
			CameraRoll[1] = 0;
			CameraFOV[1] = 0;

			CameraSpeed[0] = s->speed;
			CameraSpeed[1] = s->speed;

			CameraXposition[2] = SpotCam[CurrentSplineCamera].x;
			CameraYposition[2] = SpotCam[CurrentSplineCamera].y;
			CameraZposition[2] = SpotCam[CurrentSplineCamera].z;

			SplineFromCamera = 1;

			CameraXtarget[2] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[2] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[2] = SpotCam[CurrentSplineCamera].tz;

			CameraRoll[2] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[2] = SpotCam[CurrentSplineCamera].fov;


			CameraSpeed[2] = SpotCam[CurrentSplineCamera].speed;

			cn++;

			if (LastCamera < cn)
			{
				cn = FirstCamera;
			}

			CameraXposition[3] = SpotCam[cn].x;
			CameraYposition[3] = SpotCam[cn].y;
			CameraZposition[3] = SpotCam[cn].z;

			CameraXtarget[3] = SpotCam[cn].tx;
			CameraYtarget[3] = SpotCam[cn].ty;
			CameraZtarget[3] = SpotCam[cn].tz;

			CameraRoll[3] = SpotCam[cn].roll;
			CameraFOV[3] = SpotCam[cn].fov;
			CameraSpeed[3] = SpotCam[cn].speed;
		}
	}

	if (s->flags & SCF_HIDE_LARA)
	{
		SpotcamDontDrawLara = true;
	}

	QuakeCam.spos.boxNumber = 0;
}

#ifdef OLD_CODE
void __cdecl CalculateSpotCameras()
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
	static int bFirstLook; // offset 0x18 dword_A0AC4?
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

	if (DisableLaraControl)
	{
		LaraItem->hitPoints = LaraHealth;
		Lara.air = LaraAir;
	}

	SPOTCAM* s = &SpotCam[FirstCamera];
	short spline_cnt = 4;

	if ((s->flags & SCF_TRACKING_CAM))
	{
		spline_cnt = CurrentCameraCnt + 2;
	}

	//loc_37F64
	int cpx = Spline(CurrentSplinePosition, &CameraXposition[0], spline_cnt);
	int cpy = Spline(CurrentSplinePosition, &CameraYposition[0], spline_cnt);
	int cpz = Spline(CurrentSplinePosition, &CameraZposition[0], spline_cnt);

	int ctx = Spline(CurrentSplinePosition, &CameraXtarget[0], spline_cnt);
	int cty = Spline(CurrentSplinePosition, &CameraYtarget[0], spline_cnt);
	int ctz = Spline(CurrentSplinePosition, &CameraZtarget[0], spline_cnt);

	int cspeed = Spline(CurrentSplinePosition, &CameraSpeed[0], spline_cnt);
	int croll = Spline(CurrentSplinePosition, &CameraRoll[0], spline_cnt);
	int cfov = Spline(CurrentSplinePosition, &CameraFOV[0], spline_cnt);

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_IN) && CameraFade != CurrentSplineCamera)
	{
		CameraFade = CurrentSplineCamera;

		/*if (gfCurrentLevel != LVL5_TITLE)
		{
			ScreenFadedOut = 0;
			ScreenFade = 255;
			dScreenFade = 0;
			SetScreenFadeIn(16);
		}*/
	}//loc_38084

	if ((SpotCam[CurrentSplineCamera].flags & SCF_SCREEN_FADE_OUT) && CameraFade != CurrentSplineCamera)
	{
		CameraFade = CurrentSplineCamera;

		/*if (gfCurrentLevel != LVL5_TITLE)
		{
			ScreenFadedOut = 0;
			ScreenFade = 0;
			dScreenFade = 255;
			SetScreenFadeOut(16, 0);
		}*/
	}

	int sp = 0;
	int tlen = 0;
	int clen = 0;
	int cp = 0;
	int temp = 0;

	if ((s->flags & SCF_TRACKING_CAM))
	{
		lx = LaraItem->pos.xPos;
		ly = LaraItem->pos.yPos;
		lz = LaraItem->pos.zPos;

		for (int i = 0; i < 8; i++)
		{
			clen = 0x10000;

			for (int j = 0; j < 8; j++)
			{
				cx = Spline(sp, &CameraXposition[0], spline_cnt);
				cy = Spline(sp, &CameraYposition[0], spline_cnt);
				cz = Spline(sp, &CameraZposition[0], spline_cnt);

				dx = SQUARE(cx - lx);
				dz = SQUARE(cz - lz);
				dy = SQUARE(cy - ly);

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
			sp = cp - 2 * (temp & 0xFFFE); // << 2 ?

			if (sp < 0)
				sp = 0;
		}

		CurrentSplinePosition += (cp - CurrentSplinePosition) >> 5;
		if ((s->flags & SCF_CUT_PAN))
		{
			temp = cp - CurrentSplinePosition;
			if (temp < 0)
			{
				temp = CurrentSplinePosition - cp;
			}
			if (temp > 0x8000)
				CurrentSplinePosition = cp;
		}
		if (CurrentSplinePosition >= 0)
		{
			if (CurrentSplinePosition >= 0x8000)
			{
				CurrentSplinePosition = 0x10000;
			}
		}
		else
		{
			CurrentSplinePosition = 0;
		}
	}
	else if (!SpotcamTimer)
	{
		CurrentSplinePosition += cspeed;
	}

	if (!(TrInput & IN_LOOK))
	{
		Unk_0051D024 = 0;
	}

	if (s->flags & SCF_DISABLE_BREAKOUT || !(TrInput & IN_LOOK) /*&& gfGameMode != 1*/)
	{
		Camera.pos.x = cpx;
		Camera.pos.y = cpy;
		Camera.pos.z = cpz;

		if ((s->flags & SCF_FOCUS_LARA_HEAD || s->flags & SCF_TRACKING_CAM))
		{
			ctx = LaraItem->pos.xPos;
			cty = LaraItem->pos.yPos;
			ctz = LaraItem->pos.zPos;
		}

		Camera.target.x = ctx;
		Camera.target.y = cty;
		Camera.target.z = ctz;

		IsRoomOutsideNo = -1;
		IsRoomOutside(cpx, cpy, cpz);
		if (IsRoomOutsideNo == -1)
		{
			Camera.pos.roomNumber = SpotCam[CurrentSplineCamera].roomNumber;
			GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
		}
		else
		{
			Camera.pos.roomNumber = IsRoomOutsideNo;
		}

		AlterFOV(cfov);

		if (QuakeCam.spos.boxNumber != 0)
		{
			dx = (Camera.pos.x - QuakeCam.epos.x);
			dy = (Camera.pos.y - QuakeCam.epos.y);
			dz = (Camera.pos.z - QuakeCam.epos.z);

			if (SQRT_ASM(SQUARE(dx) * SQUARE(dy) * SQUARE(dz)) < QuakeCam.epos.boxNumber)
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

		LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, 0);

		if (CheckTrigger)
		{
			CAMERA_TYPE oldType = Camera.type;
			Camera.type = HEAVY_CAMERA;
			if (CurrentLevel != 0)
			{
				TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
			}
			else
			{
				TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 0, 0);
				TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
			}
			Camera.type = oldType;
			CheckTrigger = false;
		}

		if (s->flags & SCF_TRACKING_CAM)
		{
			TrackCameraInit = true;
		}
		else
		{
			if (CurrentSplinePosition > 0x10000 - cspeed)
			{
				/*v25 = current_spline_camera;
		v13 = 40 * current_spline_camera;
		v26 = *(short *)((char *)&SpotCam[0].flags + v13);
		if ( v26 & 2 )
		{
		  if ( *(short *)((char *)&SpotCam[0].timer + v13) >= 0 )
		  {
			if ( !SlowMotion )
			  SlowMotion = *(short *)((char *)&SpotCam[0].timer + v13);
		  }
		  else
		  {
			SCOverlay = 1;
		  }
		}*/
				if (SpotCam[CurrentSplineCamera].flags & SCF_HIDE_LARA)
					SpotcamDontDrawLara = true;
				if (SpotCam[CurrentSplineCamera].flags & SCF_ACTIVATE_HEAVY_TRIGGERS)
					CheckTrigger = true;

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

						QuakeCam.epos.boxNumber = SQRT_ASM(((QuakeCam.spos.x - QuakeCam.epos.x) * (QuakeCam.spos.x - QuakeCam.epos.x)) + ((QuakeCam.spos.y - QuakeCam.epos.y) * (QuakeCam.spos.y - QuakeCam.epos.y) + ((QuakeCam.spos.z - QuakeCam.epos.z) * (QuakeCam.spos.z - QuakeCam.epos.z))));
					}
					else
					{
						QuakeCam.spos.boxNumber = 0;
					}
				}

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

					if (SplineFromCamera != 0)
					{
						SplineFromCamera = 0;
						cn = FirstCamera - 1;
					}
					else
					{
						if ((SpotCam[CurrentSplineCamera].flags & SCF_REENABLE_LARA_CONTROLS))
						{
							DisableLaraControl = false;
						}

						if ((SpotCam[CurrentSplineCamera].flags & SCF_DISABLE_LARA_CONTROLS))
						{
							//SetFadeClip(16, 1);
							DisableLaraControl = true;
						}

						sp = 0;

						if ((SpotCam[CurrentSplineCamera].flags & SCF_CUT_TO_CAM))
						{
							cn = (SpotCam[CurrentSplineCamera].timer & 0xF) + FirstCamera;

							CameraXposition[0] = SpotCam[cn].x;
							CameraYposition[0] = SpotCam[cn].y;
							CameraZposition[0] = SpotCam[cn].z;
							CameraXtarget[0] = SpotCam[cn].tx;
							CameraYtarget[0] = SpotCam[cn].ty;
							CameraZtarget[0] = SpotCam[cn].tz;
							CameraXtarget[0] = SpotCam[cn].tx;
							CameraYtarget[0] = SpotCam[cn].ty;
							CameraZtarget[0] = SpotCam[cn].tz;
							CameraRoll[0] = SpotCam[cn].roll;
							CameraFOV[0] = SpotCam[cn].fov;
							sp = 1;
							CurrentSplineCamera = cn;
							CameraSpeed[0] = SpotCam[cn].speed;
						}

						sp++;

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


				}
			}
		}
	}
	else if (s->flags & SCF_TRACKING_CAM)
	{
		if (!Unk_0051D024)
		{
			Camera.oldType = FIXED_CAMERA;
			Unk_0051D024 = true;
		}

		CalculateCamera();
	}
	else
	{
		UseSpotCam = false;
		DisableLaraControl = false;
		Camera.speed = 1;
		AlterFOV(LastFOV);
		CalculateCamera();
		CheckTrigger = false;
	}



















	if (s->flags & SCF_DISABLE_BREAKOUT || !(TrInput & IN_LOOK) /*&& gfGameMode != 1*/)
	{
		if ((s->flags & SCF_TRACKING_CAM))
		{
			if (Unk_0051D024 == 0)
			{
				Camera.oldType = FIXED_CAMERA;
				Unk_0051D024 = 1;
			}//loc_3836C

			CalculateCamera();

			return;
		}
		else
		{
			//loc_3837C
			//SetFadeClip(0, 1);
			UseSpotCam = 0;
			//assert(0);
			DisableLaraControl = 0;
			Camera.speed = 1;

			AlterFOV(LastFOV);
			CalculateCamera();
			CheckTrigger = 0;

			return;
		}
	}
	//loc_383B4
	Camera.pos.x = cpx;
	Camera.pos.y = cpy;
	Camera.pos.z = cpz;

	if ((s->speed & 0x280000))
	{
		ctx = LaraItem->pos.xPos;
		cty = LaraItem->pos.yPos;
		ctz = LaraItem->pos.zPos;
	}

	//loc_38420
	Camera.target.x = ctx;
	Camera.target.y = cty;
	Camera.target.z = ctz;

	IsRoomOutsideNo = -1;
	IsRoomOutside(cpx, cpy, cpz);

	if (IsRoomOutsideNo != -1)///@FIXME IsRoomOutsideNo bad value
	{
		Camera.pos.roomNumber = IsRoomOutsideNo;
	}
	else
	{
		//loc_38490
		Camera.pos.roomNumber = SpotCam[CurrentSplineCamera].roomNumber;
		GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &Camera.pos.roomNumber);
	}

	//loc_384DC
	AlterFOV(cfov);

	if (QuakeCam.spos.boxNumber != 0)
	{
		dx = (Camera.pos.x - QuakeCam.epos.x);
		dy = (Camera.pos.y - QuakeCam.epos.y);
		dz = (Camera.pos.z - QuakeCam.epos.z);


		if (SQRT_ASM(dx * dy * dz) < QuakeCam.epos.boxNumber)
		{
			dz = QuakeCam.spos.roomNumber + (((QuakeCam.epos.roomNumber - QuakeCam.spos.roomNumber) * -QuakeCam.epos.boxNumber) / QuakeCam.epos.boxNumber) >> 1;//s1
			dy = QuakeCam.spos.roomNumber + (((QuakeCam.epos.roomNumber - QuakeCam.spos.roomNumber) * -QuakeCam.epos.boxNumber) / QuakeCam.epos.boxNumber);//s0
			if (dy > 0)
			{
				Camera.pos.x += (GetRandomControl() / dy) - dz;
				Camera.pos.y += (GetRandomControl() / dy) - dz;
				Camera.pos.z += (GetRandomControl() / dy) - dz;
			}//loc_38650
		}//loc_38650
	}//loc_38650

#if PSXENGINE
	LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, croll);
#endif

	///sp = s0
	if (CheckTrigger)
	{
		ctype = Camera.type;
		Camera.type = HEAVY_CAMERA;

		/*if (gfCurrentLevel != LVL5_TITLE)
		{
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
		}
		else
		{*/
			//loc_38704
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 0, 0);
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
		//}

			Camera.type = (enum camera_type)ctype;
		CheckTrigger = 0;
	}//loc_3876C

	if ((s->flags & SCF_TRACKING_CAM))
	{
		TrackCameraInit = 1;
		return;
	}

	if (0x10000 - cspeed >= CurrentSplinePosition)
	{
		return;
	}

	if ((SpotCam[CurrentSplineCamera].flags & SCF_VIGNETTE))
	{
		if (SpotCam[CurrentSplineCamera].timer < 0)
		{
			//SCOverlay = 1;
		}
		else
		{
			//loc_387F8
			/*if (SlowMotion == 0)
			{
				SlowMotion = 1;
			}*/
		}
	}//loc_38814

	if ((SpotCam[CurrentSplineCamera].flags & SCF_HIDE_LARA))
	{
		SpotcamDontDrawLara = 1;
	}
	//loc_38844

	if ((SpotCam[CurrentSplineCamera].flags & SCF_ACTIVATE_HEAVY_TRIGGERS))
	{
		CheckTrigger = 1;
	}

	//loc_3885C
	if ((SpotCam[CurrentSplineCamera].flags & SCF_STOP_MOVEMENT))
	{
		if (QuakeCam.spos.boxNumber == 0 || SpotCam[CurrentSplineCamera].timer != -1)
		{
			//loc_38888
			QuakeCam.spos.x = SpotCam[CurrentSplineCamera].x;
			QuakeCam.spos.y = SpotCam[CurrentSplineCamera].y;
			QuakeCam.spos.z = SpotCam[CurrentSplineCamera].z;

			if (SpotCam[CurrentSplineCamera].timer != -1)
			{
				QuakeCam.spos.roomNumber = SpotCam[CurrentSplineCamera].timer << 3;
			}
			else
			{
				//loc_388C8
				QuakeCam.spos.roomNumber = 0;
			}
			//loc_388CC
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
				//loc_3892C
				QuakeCam.epos.roomNumber = 0;
			}

			//loc_38930
			QuakeCam.epos.boxNumber = SQRT_ASM(((QuakeCam.spos.x - QuakeCam.epos.x) * (QuakeCam.spos.x - QuakeCam.epos.x)) + ((QuakeCam.spos.y - QuakeCam.epos.y) * (QuakeCam.spos.y - QuakeCam.epos.y) + ((QuakeCam.spos.z - QuakeCam.epos.z) * (QuakeCam.spos.z - QuakeCam.epos.z))));

		}
		else
		{
			//loc_38990
			QuakeCam.spos.boxNumber = 0;
		}
	}

	//loc_38994
	if (SpotcamTimer != 0)
	{
		return;
	}

	CurrentSplinePosition = 0;

	if (CurrentSplineCamera != FirstCamera)///@FIXME CurrentSplineCamera bad value when switching
	{
		cn = CurrentSplineCamera - 1;
	}
	else
	{
		cn = LastCamera;
	}

	//loc_389BC
	if (SplineFromCamera != 0)
	{
		SplineFromCamera = 0;
		cn = FirstCamera - 1;
	}
	else
	{
		if ((SpotCam[CurrentSplineCamera].flags & SCF_REENABLE_LARA_CONTROLS))
		{
			DisableLaraControl = 0;
		}//loc_38A0C

		if ((SpotCam[CurrentSplineCamera].flags & SCF_DISABLE_LARA_CONTROLS))
		{
			//SetFadeClip(16, 1);
			DisableLaraControl = 1;
		}
		//loc_38A24

		sp = 0;
		if ((SpotCam[CurrentSplineCamera].flags & SCF_CUT_TO_CAM))
		{
			cn = (SpotCam[CurrentSplineCamera].timer & 0xF) + FirstCamera;
			//v0 = SpotCam[cn]
			CameraXposition[0] = SpotCam[cn].x;
			CameraYposition[0] = SpotCam[cn].y;
			CameraZposition[0] = SpotCam[cn].z;
			CameraXtarget[0] = SpotCam[cn].tx;
			CameraYtarget[0] = SpotCam[cn].ty;
			CameraZtarget[0] = SpotCam[cn].tz;
			CameraXtarget[0] = SpotCam[cn].tx;
			CameraYtarget[0] = SpotCam[cn].ty;
			CameraZtarget[0] = SpotCam[cn].tz;
			CameraRoll[0] = SpotCam[cn].roll;
			CameraFOV[0] = SpotCam[cn].fov;
			sp = 1;
			CurrentSplineCamera = cn;
			CameraSpeed[0] = SpotCam[cn].speed;
		}//loc_38B04

		CameraXposition[sp] = SpotCam[cn].x;
		CameraYposition[sp] = SpotCam[cn].y;
		CameraZposition[sp] = SpotCam[cn].z;
		CameraXtarget[sp] = SpotCam[cn].tx;
		CameraYtarget[sp] = SpotCam[cn].ty;
		CameraZtarget[sp] = SpotCam[cn].tz;
		sp++;
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
		//loc_38BEC
		while (sp < 4)
		{
			if ((s->flags & SCF_LOOP_SEQUENCE))
			{
				if (LastCamera < cn)
				{
					cn = FirstCamera;
				}//loc_38C2C
			}//loc_38C18
			else
			{
				//loc_38C18
				if (LastCamera < cn)
				{
					cn = LastCamera;
				}//loc_38C2C
			}

			//loc_38C2C
			CameraXposition[sp] = SpotCam[cn].x;
			CameraYposition[sp] = SpotCam[cn].y;
			CameraZposition[sp] = SpotCam[cn].z;
			CameraXtarget[sp] = SpotCam[cn].tx;
			CameraYtarget[sp] = SpotCam[cn].ty;
			CameraZtarget[sp] = SpotCam[cn].tz;
			CameraXtarget[sp] = SpotCam[cn].tx;
			CameraYtarget[sp] = SpotCam[cn].ty;
			CameraZtarget[sp] = SpotCam[cn].tz;
			CameraRoll[sp] = SpotCam[cn].roll;
			CameraFOV[sp] = SpotCam[cn].fov;
			CameraSpeed[sp] = SpotCam[cn].speed;
			cn++;
			sp++;
		}
	}//loc_38CF4

	printf("First: %d, Last: %d, Current: %d\n", FirstCamera, LastCamera, CurrentSplineCamera);

	if (LastCamera > ++CurrentSplineCamera)
	{
		return;
	}

	if ((s->flags & SCF_LOOP_SEQUENCE))
	{
		CurrentSplineCamera = FirstCamera;
		SpotcamLoopCnt++;
		return;
	}//loc_38D50

	if (!(s->flags & SCF_PAN_TO_LARA_CAM))
	{
		//loc_38D50
		if (SplineToCamera == 0)
		{
			CameraXposition[0] = SpotCam[CurrentSplineCamera - 1].x;
			CameraYposition[0] = SpotCam[CurrentSplineCamera - 1].y;
			CameraZposition[0] = SpotCam[CurrentSplineCamera - 1].z;
			CameraXtarget[0] = SpotCam[CurrentSplineCamera - 1].tx;
			CameraYtarget[0] = SpotCam[CurrentSplineCamera - 1].ty;
			CameraZtarget[0] = SpotCam[CurrentSplineCamera - 1].tz;
			CameraXtarget[0] = SpotCam[CurrentSplineCamera - 1].tx;
			CameraYtarget[0] = SpotCam[CurrentSplineCamera - 1].ty;
			CameraZtarget[0] = SpotCam[CurrentSplineCamera - 1].tz;
			CameraRoll[0] = SpotCam[CurrentSplineCamera - 1].roll;
			CameraFOV[0] = SpotCam[CurrentSplineCamera - 1].fov;
			CameraSpeed[0] = SpotCam[CurrentSplineCamera - 1].speed;

			CameraXposition[1] = SpotCam[CurrentSplineCamera].x;
			CameraYposition[1] = SpotCam[CurrentSplineCamera].y;
			CameraZposition[1] = SpotCam[CurrentSplineCamera].z;
			CurrentSplineCamera = CurrentSplineCamera;///?
			CameraXtarget[1] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[1] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[1] = SpotCam[CurrentSplineCamera].tz;
			CameraXtarget[1] = SpotCam[CurrentSplineCamera].tx;
			CameraYtarget[1] = SpotCam[CurrentSplineCamera].ty;
			CameraZtarget[1] = SpotCam[CurrentSplineCamera].tz;
			CameraRoll[1] = SpotCam[CurrentSplineCamera].roll;
			CameraFOV[1] = SpotCam[CurrentSplineCamera].fov;
			CameraSpeed[1] = SpotCam[CurrentSplineCamera].speed;

			memcpy((char*)&Backup, (char*)&Camera, sizeof(CAMERA_INFO));
			Camera.oldType = FIXED_CAMERA;
			Camera.type = CHASE_CAMERA;
			Camera.speed = 1;

			CalculateCamera();
			//a0 = &camera;
			//a1 = &Backup;

			CameraRoll[2] = 0;
			CameraRoll[3] = 0;
			CameraSpeed[2] = CameraSpeed[1];

			InitialCameraPosition.x = Camera.pos.x;
			InitialCameraPosition.y = Camera.pos.y;
			InitialCameraPosition.z = Camera.pos.z;

			InitialCameraTarget.x = Camera.target.x;
			InitialCameraTarget.y = Camera.target.y;
			InitialCameraTarget.z = Camera.target.z;

			CameraXposition[2] = Camera.pos.x;
			CameraYposition[2] = Camera.pos.y;
			CameraZposition[2] = Camera.pos.z;
			CameraXtarget[2] = Camera.target.x;
			CameraYtarget[2] = Camera.target.y;
			CameraZtarget[2] = Camera.target.z;
			CameraFOV[2] = CurrentFOV;

			CameraXposition[3] = Camera.pos.x;
			CameraYposition[3] = Camera.pos.y;
			CameraZposition[3] = Camera.pos.z;
			CameraXtarget[3] = Camera.target.x;
			CameraYtarget[3] = Camera.target.y;
			CameraZtarget[3] = Camera.target.z;
			CameraFOV[3] = CurrentFOV;
			CameraSpeed[3] = CameraSpeed[1] >> 1;

			memcpy((char*)&Camera, (char*)&Backup, sizeof(CAMERA_INFO));
#if PSXENGINE
			LookAt(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.x, Camera.target.y, Camera.target.z, 0);
#endif
			SplineToCamera = 1;
			return;
		}
	}//loc_38FE0

	if (CheckTrigger)
	{
		ctype = Camera.type;
		Camera.type = HEAVY_CAMERA;

		/*if (gfCurrentLevel != LVL5_TITLE)
		{
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
		}//loc_39044
		else
		{*/
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 0, 0);
			TestTriggersAtXYZ(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber, 1, 0);
		//}

		//loc_390A0
			Camera.type = (enum camera_type)ctype;

		CheckTrigger = 0;
	}

	//loc_390AC
	//SetFadeClip(0, 1);

	Camera.oldType = FIXED_CAMERA;
	Camera.type = CHASE_CAMERA;
	Camera.speed = 1;
	UseSpotCam = 0;
	DisableLaraControl = 0;
	CheckTrigger = 0;

	if ((s->flags & SCF_PAN_TO_LARA_CAM))
	{
		Camera.pos.x = InitialCameraPosition.x;
		Camera.pos.y = InitialCameraPosition.y;
		Camera.pos.z = InitialCameraPosition.z;

		Camera.target.x = InitialCameraTarget.x;
		Camera.target.y = InitialCameraTarget.y;
		Camera.target.z = InitialCameraTarget.z;

		Camera.pos.roomNumber = InitialCameraRoom;
	}

	//loc_39148
	//SCOverlay = 0;
	SpotcamDontDrawLara = 0;
	cfov = LastFOV;
	AlterFOV(LastFOV);
}
#endif

/*void __cdecl CalculateSpotCameras()
{

}*/

void __cdecl Inject_Spotcam()
{
	INJECT(0x0047A800, InitSpotCamSequences);
	//INJECT(0x0047A9D0, InitialiseSpotCam);
	//INJECT(0x0047B280, CalculateSpotCameras);
}