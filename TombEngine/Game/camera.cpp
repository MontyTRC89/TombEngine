#include "framework.h"
#include "Game/camera.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;

using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;

constexpr auto COLL_CHECK_THRESHOLD   = SECTOR(4);
constexpr auto COLL_CANCEL_THRESHOLD  = SECTOR(2);
constexpr auto COLL_DISCARD_THRESHOLD = CLICK(0.5f);
constexpr auto CAMERA_RADIUS          = CLICK(1);

#define LOOKCAM_VERTICAL_CONSTRAINT_ANGLE   ANGLE(70.0f)
#define LOOKCAM_HORIZONTAL_CONSTRAINT_ANGLE ANGLE(90.0f)
#define LOOKCAM_TURN_RATE_ACCEL			    ANGLE(1.0f)
#define LOOKCAM_TURN_RATE_MAX			    ANGLE(4.0f)

#define THUMBCAM_VERTICAL_CONSTRAINT_ANGLE   ANGLE(120.0f)
#define THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE ANGLE(80.0f)

struct OLD_CAMERA
{
	short ActiveState;
	short TargetState;
	int targetDistance;
	short actualElevation;
	short targetElevation;
	short actualAngle;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;
	Vector3Int target;
};

GameVector LastTarget;

GameVector LastIdeal;
GameVector Ideals[5];
OLD_CAMERA OldCam;
int CameraSnaps = 0;
int TargetSnaps = 0;
GameVector LookCamPosition;
GameVector LookCamTarget;
Vector3Int CamOldPos;
CAMERA_INFO Camera;
GameVector ForcedFixedCamera;
int UseForcedFixedCamera;

int BinocularRange;
bool BinocularOn;
CameraType BinocularOldCamera;
bool LaserSight;

int LSHKTimer = 0;
int LSHKShotsFired = 0;
int WeaponDelay = 0;

int PhdPerspective;
short CurrentFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

bool  ScreenFadedOut = false;
bool  ScreenFading = false;
float ScreenFadeSpeed = 0;
float ScreenFadeStart = 0;
float ScreenFadeEnd = 0;
float ScreenFadeCurrent = 0;

float CinematicBarsHeight = 0;
float CinematicBarsDestinationHeight = 0;
float CinematicBarsSpeed = 0;

void DoLookAround(ItemInfo* item, bool invertVerticalAxis)
{
	auto* lara = GetLaraInfo(item);

	Camera.type = CameraType::Look;

	// Determine vertical axis coefficient.
	float vAxisCoeff = 0.0f;
	if (TrInput & (IN_FORWARD | IN_BACK) &&
		(lara->Control.Look.Mode == LookMode::Vertical || lara->Control.Look.Mode == LookMode::Free))
	{
		vAxisCoeff = AxisMap[InputAxis::MoveVertical];
	}

	// Determine horizontal axis coefficient.
	float hAxisCoeff = 0.0f;
	if (TrInput & (IN_LEFT | IN_RIGHT) &&
		(lara->Control.Look.Mode == LookMode::Horizontal || lara->Control.Look.Mode == LookMode::Free))
	{
		hAxisCoeff = AxisMap[InputAxis::MoveHorizontal];
	}

	// Modulate turn rates.
	short turnRateMax = LOOKCAM_TURN_RATE_MAX;
	if (BinocularRange)
		turnRateMax *= (BinocularRange - ANGLE(10.0f)) / ANGLE(17.0f);

	lara->Control.Look.TurnRate.x = ModulateLaraTurnRate(lara->Control.Look.TurnRate.x, LOOKCAM_TURN_RATE_ACCEL, 0, turnRateMax, vAxisCoeff, invertVerticalAxis);
	lara->Control.Look.TurnRate.y = ModulateLaraTurnRate(lara->Control.Look.TurnRate.y, LOOKCAM_TURN_RATE_ACCEL, 0, turnRateMax, hAxisCoeff, false);

	// Apply and constrain turn rates.
	lara->Control.Look.Orientation += lara->Control.Look.TurnRate;
	lara->Control.Look.Orientation.x = std::clamp<short>(lara->Control.Look.Orientation.x, -LOOKCAM_VERTICAL_CONSTRAINT_ANGLE, LOOKCAM_VERTICAL_CONSTRAINT_ANGLE);
	lara->Control.Look.Orientation.y = std::clamp<short>(lara->Control.Look.Orientation.y, -LOOKCAM_HORIZONTAL_CONSTRAINT_ANGLE, LOOKCAM_HORIZONTAL_CONSTRAINT_ANGLE);

	// Visually adapt head and torso orientations.
	lara->ExtraHeadRot = lara->Control.Look.Orientation / 2;

	if (lara->Control.HandStatus != HandStatus::Busy &&
		!lara->LeftArm.Locked && !lara->RightArm.Locked &&
		lara->Vehicle == NO_ITEM)
	{
		lara->ExtraTorsoRot = lara->ExtraHeadRot;
	}

	// Clear directional inputs.
	if (lara->Control.Look.Mode == LookMode::Vertical ||
		lara->Control.Look.Mode == LookMode::Free)
	{
		TrInput &= ~IN_DIRECTION;
	}
	else
		TrInput &= ~(IN_LEFT | IN_RIGHT);

	// Debug
	g_Renderer.PrintDebugMessage("LookMode: %d", (int)lara->Control.Look.Mode);
	g_Renderer.PrintDebugMessage("LookCam.x: %.3f", TO_DEGREES(lara->Control.Look.Orientation.x));
	g_Renderer.PrintDebugMessage("LookCam.y: %.3f", TO_DEGREES(lara->Control.Look.Orientation.y));
	g_Renderer.PrintDebugMessage("hAxisCoeff: %f", hAxisCoeff);
	g_Renderer.PrintDebugMessage("vAxisCoeff: %f", vAxisCoeff);
}

void DoThumbstickCamera()
{
	if (!g_Configuration.EnableThumbstickCameraControl)
		return;

	if (Camera.laraNode == -1 && (Camera.target.x == OldCam.target.x &&
		Camera.target.y == OldCam.target.y &&
		Camera.target.z == OldCam.target.z))
	{
		float hAxisCoeff = AxisMap[InputAxis::CameraHorizontal];
		float vAxisCoeff = AxisMap[InputAxis::CameraVertical];

		Camera.targetAngle = THUMBCAM_VERTICAL_CONSTRAINT_ANGLE * hAxisCoeff;
		Camera.targetElevation = ANGLE(-10.0f) + (THUMBCAM_HORIZONTAL_CONSTRAINT_ANGLE * hAxisCoeff);
	}
}

void LookCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	// TODO:
	// - Room probing.
	// - Swamp collision.

	// Define camera orientation.
	auto orient = lara->Control.Look.Orientation + Vector3Shrt(0, item->Pose.Orientation.y, 0);

	// Define landmarks.
	auto pivot = TranslateVector(item->Pose.Position, item->Pose.Orientation.y, CLICK(0.25f), -LaraCollision.Setup.Height);
	auto cameraPos = TranslateVector(pivot, orient, -std::max(Camera.targetDistance * 0.5f, SECTOR(0.5f)));
	auto lookAtPos = TranslateVector(pivot, orient, CLICK(0.75f));

	// Determine best position.
	auto origin = GameVector(pivot, GetCollision(item, item->Pose.Orientation.y, CLICK(0.25f), -LaraCollision.Setup.Height).RoomNumber);
	auto target = GameVector(cameraPos, GetCollision(origin.ToVector3Int(), origin.roomNumber, orient, -std::max(Camera.targetDistance * 0.5f, SECTOR(0.5f))).RoomNumber);

	LOSAndReturnTarget(&origin, &target, 0);
	cameraPos = target.ToVector3Int();

	// Handle room and object collisions.
	auto temp = target;
	CameraCollisionBounds(&temp, CLICK(1) - CLICK(0.25f) / 2, true);
	cameraPos = temp.ToVector3Int();
	ItemsCollideCamera();

	// Smoothly update camera position.
	//MoveCamera(&GameVector(Camera.target.ToVector3Int() + (lookAtPos - Camera.target.ToVector3Int()) * 0.25f, item->RoomNumber), Camera.speed);
	Camera.pos = GameVector(Camera.pos.ToVector3Int() + (cameraPos - Camera.pos.ToVector3Int()) * 0.25f, target.roomNumber);
	Camera.target = GameVector(Camera.target.ToVector3Int() + (lookAtPos - Camera.target.ToVector3Int()) * 0.25f, item->RoomNumber);

	LookAt(&Camera, 0);

	// Set mike position.
	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = orient.y;
		Camera.mikePos = item->Pose.Position;
	}
	else
	{
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos = Vector3Int(
			Camera.pos.x + (PhdPerspective * phd_sin(Camera.actualAngle)),
			Camera.pos.y,
			Camera.pos.z + (PhdPerspective * phd_cos(Camera.actualAngle))
		);
	}

	Camera.oldType = Camera.type;
}

void LookAt(CAMERA_INFO* cam, short roll)
{
	auto pos = cam->pos.ToVector3();
	auto target = cam->target.ToVector3();
	auto up = Vector3::Down;
	float fov = TO_RAD(CurrentFOV / 1.333333f);
	float r = TO_RAD(roll);

	float levelFarView = g_GameFlow->GetLevel(CurrentLevel)->GetFarView() * float(SECTOR(1));

	g_Renderer.UpdateCameraMatrices(cam, r, fov, levelFarView);
}

void AlterFOV(int value)
{
	CurrentFOV = value;
	PhdPerspective = g_Configuration.Width / 2 * phd_cos(CurrentFOV / 2) / phd_sin(CurrentFOV / 2);
}

short GetCurrentFOV()
{
	return CurrentFOV;
}

inline void RumbleFromBounce()
{
	Rumble(std::clamp((float)abs(Camera.bounce) / 70.0f, 0.0f, 0.8f), 0.2f);
}

void InitialiseCamera()
{
	Camera.shift = LaraItem->Pose.Position.y - SECTOR(1);

	LastTarget = GameVector(
		LaraItem->Pose.Position.x,
		Camera.shift,
		LaraItem->Pose.Position.z,
		LaraItem->RoomNumber
	);

	Camera.target = GameVector(
		LastTarget.x,
		Camera.shift,
		LastTarget.z,
		LaraItem->RoomNumber
	);

	Camera.pos = GameVector(
		LastTarget.x,
		Camera.shift,
		LastTarget.z - 100,
		LaraItem->RoomNumber
	);

	Camera.targetDistance = SECTOR(1.5f);
	Camera.item = nullptr;
	Camera.numberFrames = 1;
	Camera.type = CameraType::Chase;
	Camera.speed = 1;
	Camera.flags = CF_NONE;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = false;

	AlterFOV(ANGLE(80.0f));

	UseForcedFixedCamera = 0;
	CalculateCamera();
}

void MoveCamera(GameVector* ideal, int speed)
{
	if (BinocularOn)
		speed = 1;

	if (OldCam.pos.Orientation != LaraItem->Pose.Orientation ||
		OldCam.pos2.Orientation.x != Lara.ExtraHeadRot.x ||
		OldCam.pos2.Orientation.y != Lara.ExtraHeadRot.y ||
		OldCam.pos2.Position.x != Lara.ExtraTorsoRot.x ||
		OldCam.pos2.Position.y != Lara.ExtraTorsoRot.y ||
		OldCam.pos.Position != LaraItem->Pose.Position ||
		OldCam.ActiveState != LaraItem->Animation.ActiveState ||
		OldCam.TargetState != LaraItem->Animation.TargetState ||
		OldCam.targetDistance != Camera.targetDistance ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target.x != Camera.target.x ||
		OldCam.target.y != Camera.target.y ||
		OldCam.target.z != Camera.target.z ||
		Camera.oldType != Camera.type ||
		BinocularOn)
	{
		OldCam.pos.Orientation = LaraItem->Pose.Orientation;
		OldCam.pos2.Orientation.x = Lara.ExtraHeadRot.x;
		OldCam.pos2.Orientation.y = Lara.ExtraHeadRot.y;
		OldCam.pos2.Position.x = Lara.ExtraTorsoRot.x;
		OldCam.pos2.Position.y = Lara.ExtraTorsoRot.y;
		OldCam.pos.Position = LaraItem->Pose.Position;
		OldCam.ActiveState = LaraItem->Animation.ActiveState;
		OldCam.TargetState = LaraItem->Animation.TargetState;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target.x = Camera.target.x;
		OldCam.target.y = Camera.target.y;
		OldCam.target.z = Camera.target.z;
		LastIdeal.x = ideal->x;
		LastIdeal.y = ideal->y;
		LastIdeal.z = ideal->z;
		LastIdeal.roomNumber = ideal->roomNumber;
	}
	else
	{
		ideal->x = LastIdeal.x;
		ideal->y = LastIdeal.y;
		ideal->z = LastIdeal.z;
		ideal->roomNumber = LastIdeal.roomNumber;
	}

	Camera.pos.x += (ideal->x - Camera.pos.x) / speed;
	Camera.pos.y += (ideal->y - Camera.pos.y) / speed;
	Camera.pos.z += (ideal->z - Camera.pos.z) / speed;
	Camera.pos.roomNumber = ideal->roomNumber;

	if (Camera.bounce)
	{
		if (Camera.bounce <= 0)
		{
			int bounce = -Camera.bounce;
			int bounce2 = -Camera.bounce >> 2;
			Camera.target.x += GetRandomControl() % bounce - bounce2;
			Camera.target.y += GetRandomControl() % bounce - bounce2;
			Camera.target.z += GetRandomControl() % bounce - bounce2;
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.pos.y += Camera.bounce;
			Camera.target.y += Camera.bounce;
			Camera.bounce = 0;
		}
	}

	int y = Camera.pos.y;
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.roomNumber))
		y = g_Level.Rooms[Camera.pos.roomNumber].y - CLICK(1);

	auto probe = GetCollision(Camera.pos.x, y, Camera.pos.z, Camera.pos.roomNumber);
	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor)
	{
		LOSAndReturnTarget(&Camera.target, &Camera.pos, 0);

		if (abs(Camera.pos.x - ideal->x) < SECTOR(0.5f) &&
			abs(Camera.pos.y - ideal->y) < SECTOR(0.5f) &&
			abs(Camera.pos.z - ideal->z) < SECTOR(0.5f))
		{
			auto origin = *ideal;
			auto target = Camera.pos;
			if (!LOSAndReturnTarget(&origin, &target, 0) &&
				++CameraSnaps >= 8)
			{
				Camera.pos = *ideal;
				CameraSnaps = 0;
			}
		}
	}

	probe = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);

	int buffer = CLICK(1) - 1;
	if ((Camera.pos.y - buffer) < probe.Position.Ceiling &&
		(Camera.pos.y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
	}
	else if ((Camera.pos.y + buffer) > probe.Position.Floor &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Floor - buffer;
	}
	else if ((Camera.pos.y - buffer) < probe.Position.Ceiling &&
		probe.Position.Ceiling < probe.Position.Floor &&
		probe.Position.Ceiling != NO_HEIGHT &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.pos.y = probe.Position.Ceiling + buffer;
	}
	else if (probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		Camera.pos = *ideal;
	}

	ItemsCollideCamera();

	Camera.pos.roomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.mikePos.x = LaraItem->Pose.Position.x;
		Camera.mikePos.y = LaraItem->Pose.Position.y;
		Camera.mikePos.z = LaraItem->Pose.Position.z;
		Camera.oldType = Camera.type;
	}
	else
	{
		short angle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + (PhdPerspective * phd_sin(angle));
		Camera.mikePos.y = Camera.pos.y;
		Camera.mikePos.z = Camera.pos.z + (PhdPerspective * phd_cos(angle));
		Camera.oldType = Camera.type;
	}
}

void ChaseCamera(ItemInfo* item)
{
	static const unsigned int maxSwivelSteps = 5;

	if (!Camera.targetElevation)
		Camera.targetElevation = -ANGLE(10.0f);

	Camera.targetElevation += item->Pose.Orientation.x;
	UpdateCameraElevation();

	// Clamp X orientation.
	if (Camera.actualElevation > ANGLE(85.0f))
		Camera.actualElevation = ANGLE(85.0f);
	else if (Camera.actualElevation < ANGLE(-85.0f))
		Camera.actualElevation = ANGLE(-85.0f);

	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);

	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
	if (((y < probe.Position.Ceiling || probe.Position.Floor < y) || probe.Position.Floor <= probe.Position.Ceiling) ||
		(probe.Position.Floor == NO_HEIGHT || probe.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
		TargetSnaps = 0;

	for (int i = 0; i < maxSwivelSteps; i++)
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

	// Determine best player viewing angle.
	float farthestDist = FLT_MAX;
	int indexOfFarthestIdeal = 0;
	GameVector temp[2];
	for (int i = 0; i < maxSwivelSteps; i++)
	{
		// Incrementally swivel camera position.
		short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

		// Record ideal position at default distance for given swivel.
		Ideals[i].x = Camera.target.x - (distance * phd_sin(angle));
		Ideals[i].z = Camera.target.z - (distance * phd_cos(angle));
		Ideals[i].roomNumber = Camera.target.roomNumber;

		// Assess LOS.
		if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
		{
			temp[0] = Ideals[i];
			temp[1] = Camera.pos;

			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					indexOfFarthestIdeal = 0;
					break;
				}

				float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
				dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
				if (dx < farthestDist)
				{
					farthestDist = dx;
					indexOfFarthestIdeal = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0] = Ideals[i];
			temp[1] = Camera.pos;

			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				float dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
				float dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);

				if ((dx + dz) > SQUARE(SECTOR(0.75f)))
				{
					indexOfFarthestIdeal = 0;
					break;
				}
			}
		}
	}

	auto ideal = Ideals[indexOfFarthestIdeal];
	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);
	MoveCamera(&ideal, Camera.speed);
}

void UpdateCameraElevation()
{
	DoThumbstickCamera();

	if (Camera.laraNode != -1)
	{
		auto pos = Vector3Int::Zero;
		GetLaraJointPosition(&pos, Camera.laraNode);

		auto pos1 = Vector3Int(0, -CLICK(1), SECTOR(2));
		GetLaraJointPosition(&pos1, Camera.laraNode);

		pos.x = pos1.x - pos.x;
		pos.z = pos1.z - pos.z;
		Camera.actualAngle = Camera.targetAngle + phd_atan(pos.z, pos.x);
	}
	else
		Camera.actualAngle = LaraItem->Pose.Orientation.y + Camera.targetAngle;

	Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) / 8;
}

void CombatCamera(ItemInfo* item)
{
	static const unsigned int maxSwivelSteps = 5;

	auto* lara = GetLaraInfo(item);

	Camera.target.x = item->Pose.Position.x;
	Camera.target.z = item->Pose.Position.z;

	if (lara->TargetEntity)
	{
		Camera.targetAngle = lara->TargetArmOrient.y;
		Camera.targetElevation = lara->TargetArmOrient.x + item->Pose.Orientation.x;
	}
	else
	{
		Camera.targetAngle = lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y;
		Camera.targetElevation = lara->ExtraHeadRot.x + lara->ExtraTorsoRot.x + item->Pose.Orientation.x - ANGLE(15.0f);
	}

	auto probe = GetCollision(Camera.target.x, Camera.target.y + CLICK(1), Camera.target.z, Camera.target.roomNumber);
	if (TestEnvironment(ENV_FLAG_SWAMP, probe.RoomNumber))
		Camera.target.y = g_Level.Rooms[probe.RoomNumber].y - CLICK(1);

	probe = GetCollision(Camera.target.x, Camera.target.y, Camera.target.z, Camera.target.roomNumber);
	Camera.target.roomNumber = probe.RoomNumber;

	int buffer = CLICK(0.25f);
	if ((probe.Position.Ceiling + buffer) > (probe.Position.Floor - buffer) &&
		probe.Position.Floor != NO_HEIGHT &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = (probe.Position.Ceiling + probe.Position.Floor) / 2;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y > (probe.Position.Floor - buffer) &&
		probe.Position.Floor != NO_HEIGHT)
	{
		Camera.target.y = probe.Position.Floor - buffer;
		Camera.targetElevation = 0;
	}
	else if (Camera.target.y < (probe.Position.Ceiling + buffer) &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		Camera.target.y = probe.Position.Ceiling + buffer;
		Camera.targetElevation = 0;
	}

	int y = Camera.target.y;
	probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
	Camera.target.roomNumber = probe.RoomNumber;

	if (y < probe.Position.Ceiling ||
		y > probe.Position.Floor ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.target = LastTarget;
	}
	else
		TargetSnaps = 0;

	UpdateCameraElevation();

	Camera.targetDistance = SECTOR(1.5f);
	int distance = Camera.targetDistance * phd_cos(Camera.actualElevation);

	for (int i = 0; i < maxSwivelSteps; i++)
		Ideals[i].y = Camera.target.y + (Camera.targetDistance * phd_sin(Camera.actualElevation));

	// Determine best player viewing angle.
	float farthestDist = FLT_MAX;
	int indexOfFarthestIdeal = 0;
	GameVector temp[2];
	for (int i = 0; i < maxSwivelSteps; i++)
	{
		// Incrementally swivel camera position.
		short angle = (i == 0) ? Camera.actualAngle : ((i - 1) * ANGLE(90.0f));

		// Record ideal position at default distance for given swivel.
		Ideals[i].x = Camera.target.x - (distance * phd_sin(angle));
		Ideals[i].z = Camera.target.z - (distance * phd_cos(angle));
		Ideals[i].roomNumber = Camera.target.roomNumber;

		// Assess LOS.
		if (LOSAndReturnTarget(&Camera.target, &Ideals[i], 200))
		{
			temp[0] = Ideals[i];
			temp[1] = Camera.pos;
			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				if (i == 0)
				{
					indexOfFarthestIdeal = 0;
					break;
				}

				float dx = (Camera.pos.x - Ideals[i].x) * (Camera.pos.x - Ideals[i].x);
				dx += (Camera.pos.z - Ideals[i].z) * (Camera.pos.z - Ideals[i].z);
				if (dx < farthestDist)
				{
					farthestDist = dx;
					indexOfFarthestIdeal = i;
				}
			}
		}
		else if (i == 0)
		{
			temp[0] = Ideals[i];
			temp[1] = Camera.pos;
			if (i == 0 || LOSAndReturnTarget(&temp[0], &temp[1], 0))
			{
				float dx = (Camera.target.x - Ideals[i].x) * (Camera.target.x - Ideals[i].x);
				float dz = (Camera.target.z - Ideals[i].z) * (Camera.target.z - Ideals[i].z);
				if ((dx + dz) > SQUARE(SECTOR(0.75f)))
				{
					indexOfFarthestIdeal = 0;
					break;
				}
			}
		}
	}

	// Handle room collision.
	auto ideal = Ideals[indexOfFarthestIdeal];
	CameraCollisionBounds(&ideal, CLICK(1.5f), 1);

	// Snap position of fixed camera type.
	if (Camera.oldType == CameraType::Fixed)
		Camera.speed = 1;

	MoveCamera(&ideal, Camera.speed);
}

bool CameraCollisionBounds(GameVector* ideal, int push, bool yFirst)
{
	int x = ideal->x;
	int y = ideal->y;
	int z = ideal->z;

	CollisionResult probe = {};
	if (yFirst)
	{
		probe = GetCollision(x, y, z, ideal->roomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < probe.Position.Ceiling &&
			(y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Floor - buffer;
		}
		else if ((y - buffer) < probe.Position.Ceiling &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Ceiling + buffer;
		}
	}

	probe = GetCollision(x - push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x & (~1023)) + push;
	}

	probe = GetCollision(x, y, z - push, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z & (~1023)) + push;
	}

	probe = GetCollision(x + push, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		x = (x | 1023) - push;
	}

	probe = GetCollision(x, y, z + push, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor ||
		y < probe.Position.Ceiling)
	{
		z = (z | 1023) - push;
	}

	if (!yFirst)
	{
		probe = GetCollision(x, y, z, ideal->roomNumber);

		int buffer = CLICK(1) - 1;
		if ((y - buffer) < probe.Position.Ceiling &&
			(y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = (probe.Position.Floor + probe.Position.Ceiling) / 2;
		}
		else if ((y + buffer) > probe.Position.Floor &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Floor - buffer;
		}
		else if ((y - buffer) < probe.Position.Ceiling &&
			probe.Position.Ceiling < probe.Position.Floor &&
			probe.Position.Ceiling != NO_HEIGHT &&
			probe.Position.Floor != NO_HEIGHT)
		{
			y = probe.Position.Ceiling + buffer;
		}
	}

	probe = GetCollision(x, y, z, ideal->roomNumber);
	if (y > probe.Position.Floor ||
		y < probe.Position.Ceiling ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT ||
		probe.Position.Ceiling >= probe.Position.Floor)
	{
		return true;
	}

	ideal->roomNumber = probe.RoomNumber;
	ideal->x = x;
	ideal->y = y;
	ideal->z = z;

	return false;
}

void FixedCamera(ItemInfo* item)
{
	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, we move speed value out of legacy
	// floordata trigger to camera itself and make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	int moveSpeed = 1;

	GameVector origin, target;
	if (UseForcedFixedCamera)
		origin = ForcedFixedCamera;
	else
	{
		auto* camera = &g_Level.Cameras[Camera.number];

		origin = GameVector(camera->x, camera->y, camera->z, camera->roomNumber);

		// Multiply original speed by 8 to comply with original bitshifted speed from TR1-2.
		moveSpeed = camera->speed * 8 + 1;
	}

	Camera.fixedCamera = true;

	MoveCamera(&origin, moveSpeed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = -1;
	}
}

void BounceCamera(ItemInfo* item, short bounce, short maxDistance)
{
	float distance = Vector3Int::Distance(item->Pose.Position, Camera.pos.ToVector3Int());
	if (distance < maxDistance)
	{
		if (maxDistance == -1)
			Camera.bounce = bounce;
		else
			Camera.bounce = -(bounce * (maxDistance - distance) / maxDistance);
	}
	else if (maxDistance == -1)
		Camera.bounce = bounce;
}

void LaserSightCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (WeaponDelay)
		WeaponDelay--;

	if (LSHKTimer)
		LSHKTimer--;

	bool isFiring = false;
	auto& ammo = GetAmmo(item, lara->Control.Weapon.GunType);

	if (!(RawInput & IN_ACTION) || WeaponDelay || !ammo)
	{
		if (!(RawInput & IN_ACTION))
		{
			LSHKShotsFired = 0;
			Camera.bounce = 0;
		}
	}
	else
	{
		if (lara->Control.Weapon.GunType == LaraWeaponType::Revolver)
		{
			isFiring = true;
			WeaponDelay = 16;
			Statistics.Game.AmmoUsed++;

			if (!ammo.hasInfinite())
				(ammo)--;

			Camera.bounce = -16 - (GetRandomControl() & 0x1F);
		}
		else if (lara->Control.Weapon.GunType == LaraWeaponType::Crossbow)
		{
			isFiring = true;
			WeaponDelay = 32;
		}
		else
		{
			if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				WeaponDelay = 12;
				isFiring = true;

				if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
					SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
				else
				{
					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
				}

				Camera.bounce = -16 - (GetRandomControl() & 0x1F);
			}
			else if (lara->Weapons[(int)LaraWeaponType::HK].SelectedAmmo == WeaponAmmoType::Ammo2)
			{
				if (!LSHKTimer)
				{
					if (++LSHKShotsFired == 5)
					{
						LSHKShotsFired = 0;
						WeaponDelay = 12;
					}

					LSHKTimer = 4;
					isFiring = true;

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}

					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					Camera.bounce = -16 - (GetRandomControl() & 0x1F);

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}

					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
			}
			else
			{
				if (LSHKTimer)
				{
					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}
				}
				else
				{
					LSHKTimer = 4;
					isFiring = true;

					if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
						SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
					else
					{
						SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
						SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					}
				}
			}

			if (!ammo.hasInfinite())
				(ammo)--;
		}
	}

	GetTargetOnLOS(&Camera.pos, &Camera.target, true, isFiring);
}

void BinocularCamera(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (!LaserSight)
	{
		// TODO: Some of these inputs should ideally be blocked. @Sezz 2022.05.19
		if (RawInput & (IN_DESELECT | IN_OPTION | IN_LOOK | IN_DRAW | IN_FLARE | IN_WALK | IN_JUMP))
		{
			item->MeshBits = ALL_JOINT_BITS;
			lara->Inventory.IsBusy = false;
			lara->ExtraHeadRot = Vector3Shrt::Zero;
			lara->ExtraTorsoRot = Vector3Shrt::Zero;
			Camera.type = BinocularOldCamera;
			BinocularOn = false;
			BinocularRange = 0;
			AlterFOV(ANGLE(80.0f));
			return;
		}
	}

	item->MeshBits = NO_JOINT_BITS;
	AlterFOV(7 * (ANGLE(11.5f) - BinocularRange));

	short headXRot = lara->ExtraHeadRot.x * 2;
	short headYRot = lara->ExtraHeadRot.y;

	if (headXRot > ANGLE(75.0f))
		headXRot = ANGLE(75.0f);
	else if (headXRot < -ANGLE(75.0f))
		headXRot = -ANGLE(75.0f);

	if (headYRot > ANGLE(80.0f))
		headYRot = ANGLE(80.0f);
	else if (headYRot < -ANGLE(80.0f))
		headYRot = -ANGLE(80.0f);

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y - CLICK(2);
	int z = item->Pose.Position.z;

	auto probe = GetCollision(x, y, z, item->RoomNumber);
	if (probe.Position.Ceiling <= (y - CLICK(1)))
		y -= CLICK(1);
	else
		y = probe.Position.Ceiling + CLICK(0.25f);

	Camera.pos.x = x;
	Camera.pos.y = y;
	Camera.pos.z = z;
	Camera.pos.roomNumber = probe.RoomNumber;

	int l = SECTOR(20.25f) * phd_cos(headXRot);

	int tx = x + l * phd_sin(item->Pose.Orientation.y + headYRot);
	int ty = y - SECTOR(20.25f) * phd_sin(headXRot);
	int tz = z + l * phd_cos(item->Pose.Orientation.y + headYRot);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.target.x = tx;
		Camera.target.y = ty;
		Camera.target.z = tz;
		Camera.target.roomNumber = item->RoomNumber;
	}
	else
	{
		Camera.target.x += (tx - Camera.target.x) / 4;
		Camera.target.y += (ty - Camera.target.y) / 4;
		Camera.target.z += (tz - Camera.target.z) / 4;
		Camera.target.roomNumber = item->RoomNumber;
	}

	if (Camera.bounce &&
		Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.target.x += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.target.y += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.target.z += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.bounce = 0;
			Camera.target.y += Camera.bounce;
		}
	}

	Camera.target.roomNumber = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.target.roomNumber).RoomNumber;
	LookAt(&Camera, 0);

	if (Camera.mikeAtLara)
	{
		Camera.actualAngle = item->Pose.Orientation.y + lara->ExtraHeadRot.y + lara->ExtraTorsoRot.y;
		Camera.mikePos = item->Pose.Position;
	}
	else
	{
		Camera.actualAngle = phd_atan(Camera.target.z - Camera.pos.z, Camera.target.x - Camera.pos.x);
		Camera.mikePos.x = Camera.pos.x + PhdPerspective * phd_sin(Camera.actualAngle);
		Camera.mikePos.z = Camera.pos.z + PhdPerspective * phd_cos(Camera.actualAngle);
		Camera.mikePos.y = Camera.pos.y;
	}

	Camera.oldType = Camera.type;

	int range = 0;
	int flags = 0;

	if (!(RawInput & IN_WALK))
	{
		range = 64;
		flags = 0x10000;
	}
	else
	{
		range = 32;
		flags = 0x8000;
	}

	if (RawInput & IN_SPRINT)
	{
		BinocularRange -= range;
		if (BinocularRange < ANGLE(0.7f))
			BinocularRange = ANGLE(0.7f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 0.9f);
	}
	else if (RawInput & IN_CROUCH)
	{
		BinocularRange += range;
		if (BinocularRange > ANGLE(8.5f))
			BinocularRange = ANGLE(8.5f);
		else
			SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 1.0f);
	}

	if (!LaserSight)
	{
		auto origin = Camera.pos.ToVector3Int();
		auto target = Camera.target.ToVector3Int();
		GetTargetOnLOS(&Camera.pos, &Camera.target, false, false);

		if (RawInput & IN_ACTION)
			LaraTorch(&origin, &target, lara->ExtraHeadRot.y, 192);
	}
	else
		LaserSightCamera(item);
}

void ConfirmCameraTargetPos()
{
	auto pos = Vector3Int::Zero;
	GetLaraJointPosition(&pos, LM_TORSO);

	if (Camera.laraNode != -1)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
	else
	{
		Camera.target.x = LaraItem->Pose.Position.x;
		Camera.target.y = (Camera.target.y + pos.y) / 2;
		Camera.target.z = LaraItem->Pose.Position.z;
	}

	int y = Camera.target.y;
	auto probe = GetCollision(Camera.target.x, y, Camera.target.z, Camera.target.roomNumber);
	if (y < probe.Position.Ceiling ||
		probe.Position.Floor < y ||
		probe.Position.Floor <= probe.Position.Ceiling ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		Camera.target.x = pos.x;
		Camera.target.y = pos.y;
		Camera.target.z = pos.z;
	}
}

void CalculateCamera()
{
	CamOldPos.x = Camera.pos.x;
	CamOldPos.y = Camera.pos.y;
	CamOldPos.z = Camera.pos.z;

	if (BinocularOn)
	{
		BinocularCamera(LaraItem);
		return;
	}

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1;
	}

	// Camera is in a water room, play water sound effect.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber))
	{
		SoundEffect(SFX_TR4_UNDERWATER, nullptr, SoundEnvironment::Always);
		if (Camera.underwater == false)
			Camera.underwater = true;
	}
	else
	{
		if (Camera.underwater == true)
			Camera.underwater = false;
	}

	ItemInfo* item = nullptr;
	bool isFixedCamera = false;
	if (Camera.item != nullptr &&
		(Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
	{
		item = Camera.item;
		isFixedCamera = true;
	}
	else
	{
		item = LaraItem;
		isFixedCamera = false;
	}

	auto* bounds = GetBoundsAccurate(item);

	int x;
	int y = item->Pose.Position.y + bounds->Y2 + (3 * (bounds->Y1 - bounds->Y2) / 4);
	int z;

	if (Camera.item)
	{
		if (!isFixedCamera)
		{
			auto dx = Camera.item->Pose.Position.x - item->Pose.Position.x;
			auto dz = Camera.item->Pose.Position.z - item->Pose.Position.z;
			int shift = sqrt(pow(dx, 2) + pow(dz, 2));
			short angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			short tilt = phd_atan(shift, y - (bounds->Y1 + bounds->Y2) / 2 - Camera.item->Pose.Position.y);
			bounds = GetBoundsAccurate(Camera.item);
			angle /= 2;
			tilt /= 2;

			if (angle > -ANGLE(50.0f) && angle < ANGLE(50.0f) && tilt > -ANGLE(85.0f) && tilt < ANGLE(85.0f))
			{
				short change = angle - Lara.ExtraHeadRot.y;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.y += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.y -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.y += change;
				Lara.ExtraTorsoRot.y = Lara.ExtraHeadRot.y;

				change = tilt - Lara.ExtraHeadRot.x;
				if (change > ANGLE(4.0f))
					Lara.ExtraHeadRot.x += ANGLE(4.0f);
				else if (change < -ANGLE(4.0f))
					Lara.ExtraHeadRot.x -= ANGLE(4.0f);
				else
					Lara.ExtraHeadRot.x += change;
				Lara.ExtraTorsoRot.x = Lara.ExtraHeadRot.x;

				Camera.type = CameraType::Look;
				Camera.item->LookedAt = 1;
			}
		}
	}

	if (Camera.type == CameraType::Look ||
		Camera.type == CameraType::Combat)
	{
		if (Camera.type == CameraType::Combat)
		{
			LastTarget.x = Camera.target.x;
			LastTarget.y = Camera.target.y;
			LastTarget.z = Camera.target.z;
			LastTarget.roomNumber = Camera.target.roomNumber;
		}

		Camera.target.roomNumber = item->RoomNumber;

		if (Camera.fixedCamera || BinocularOn)
		{
			Camera.target.y = y;
			Camera.speed = 1;
		}
		else
		{
			Camera.target.y += (y - Camera.target.y) >> 2;
			Camera.speed = Camera.type != CameraType::Look ? 8 : 4;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CameraType::Look)
			LookCamera(item);
		else
			CombatCamera(item);
	}
	else
	{
		LastTarget.x = Camera.target.x;
		LastTarget.y = Camera.target.y;
		LastTarget.z = Camera.target.z;
		LastTarget.roomNumber = Camera.target.roomNumber;

		Camera.target.roomNumber = item->RoomNumber;
		Camera.target.y = y;

		x = item->Pose.Position.x;
		z = item->Pose.Position.z;

		if (Camera.flags == CF_FOLLOW_CENTER)	//Troye Aug. 7th 2022
		{
			auto shift = (bounds->Z1 + bounds->Z2) / 2;
			x += shift * phd_sin(item->Pose.Orientation.y);
			z += shift * phd_cos(item->Pose.Orientation.y);
		}

		Camera.target.x = x;
		Camera.target.z = z;

		// CF_FOLLOW_CENTER sets target on the item, ConfirmCameraTargetPos overrides this target, 
		// hence the flag check. Troye Aug. 7th 2022
		
		if (item->IsLara() && Camera.flags != CF_FOLLOW_CENTER)
			ConfirmCameraTargetPos();

		if (isFixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1 &&
				Camera.oldType != CameraType::Look &&
				!BinocularOn)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) >> 2);
					Camera.target.x = x;
					y = LastTarget.y + ((y - LastTarget.y) >> 2);
					Camera.target.y = y;
					z = LastTarget.z + ((z - LastTarget.z) >> 2);
					Camera.target.z = z;
				}
				else
					TargetSnaps = 0;
			}
		}
		else
		{
			Camera.fixedCamera = true;
			Camera.speed = 1;
		}

		Camera.target.roomNumber = GetCollision(x, y, z, Camera.target.roomNumber).RoomNumber;

		if (abs(LastTarget.x - Camera.target.x) < 4 &&
			abs(LastTarget.y - Camera.target.y) < 4 &&
			abs(LastTarget.z - Camera.target.z) < 4)
		{
			Camera.target.x = LastTarget.x;
			Camera.target.y = LastTarget.y;
			Camera.target.z = LastTarget.z;
		}

		if (Camera.type != CameraType::Chase && Camera.flags != CF_CHASE_OBJECT)
			FixedCamera(item);
		else
			ChaseCamera(item);
	}

	Camera.fixedCamera = isFixedCamera;
	Camera.last = Camera.number;

	if (Camera.type != CameraType::Heavy ||
		Camera.timer == -1)
	{
		Camera.type = CameraType::Chase;
		Camera.speed = 10;
		Camera.number = -1;
		Camera.lastItem = Camera.item;
		Camera.item = nullptr;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = SECTOR(1.5f);
		Camera.flags = 0;
		Camera.laraNode = -1;
	}
}

void ResetLook(ItemInfo* item, float alpha)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Look.Orientation = Vector3Shrt::Zero;

	if (Camera.type != CameraType::Look)
	{
		if (abs(lara->ExtraHeadRot.x) > ANGLE(0.1f))
			lara->ExtraHeadRot.x *= alpha;
		else
			lara->ExtraHeadRot.x = 0;

		if (abs(lara->ExtraHeadRot.y) > ANGLE(0.1f))
			lara->ExtraHeadRot.y *= alpha;
		else
			lara->ExtraHeadRot.y = 0;

		if (abs(lara->ExtraHeadRot.z) > ANGLE(0.1f))
			lara->ExtraHeadRot.z *= alpha;
		else
			lara->ExtraHeadRot.z = 0;

		if (lara->Control.HandStatus != HandStatus::Busy &&
			!lara->LeftArm.Locked && !lara->RightArm.Locked &&
			lara->Vehicle == NO_ITEM)
		{
			lara->ExtraTorsoRot = lara->ExtraHeadRot;
		}
		else
		{
			if (!lara->ExtraHeadRot.x)
				lara->ExtraTorsoRot.x = 0;
			if (!lara->ExtraHeadRot.y)
				lara->ExtraTorsoRot.y = 0;
			if (!lara->ExtraHeadRot.z)
				lara->ExtraTorsoRot.z = 0;
		}
	}
}

bool TestBoundsCollideCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius)
{
	auto dxBox = TO_DX_BBOX(*pos, bounds);
	auto sphere = BoundingSphere(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), radius);
	return sphere.Intersects(dxBox);
}

void ItemPushCamera(BOUNDING_BOX* bounds, PHD_3DPOS* pos, short radius)
{
	int dx = Camera.pos.x - pos->Position.x;
	int dz = Camera.pos.z - pos->Position.z;
	auto sinY = phd_sin(pos->Orientation.y);
	auto cosY = phd_cos(pos->Orientation.y);
	auto x = (dx * cosY) - (dz * sinY);
	auto z = (dx * sinY) + (dz * cosY);

	int xMin = bounds->X1 - radius;
	int xMax = bounds->X2 + radius;
	int zMin = bounds->Z1 - radius;
	int zMax = bounds->Z2 + radius;

	if (x <= xMin || x >= xMax || z <= zMin || z >= zMax)
		return;

	auto left = x - xMin;
	auto right = xMax - x;
	auto top = zMax - z;
	auto bottom = z - zMin;

	// Left is closest.
	if (left <= right && left <= top && left <= bottom)
		x -= left;
	// Right is closest.
	else if (right <= left && right <= top && right <= bottom)
		x += right;
	// Top is closest.
	else if (top <= left && top <= right && top <= bottom)
		z += top;
	// Bottom is closest.
	else
		z -= bottom;

	Camera.pos.x = pos->Position.x + ((x * cosY) + (z * sinY));
	Camera.pos.z = pos->Position.z + ((z * cosY) - (x * sinY));

	auto pointColl = GetCollision(Camera.pos.x, Camera.pos.y, Camera.pos.z, Camera.pos.roomNumber);
	if (pointColl.Position.Floor == NO_HEIGHT || Camera.pos.y > pointColl.Position.Floor || Camera.pos.y < pointColl.Position.Ceiling)
		Camera.pos = GameVector(CamOldPos, pointColl.RoomNumber);
}

bool CheckItemCollideCamera(ItemInfo* item)
{
	bool isCloseEnough = Vector3Int::Distance(item->Pose.Position, Camera.pos.ToVector3Int()) <= COLL_CHECK_THRESHOLD;

	if (!isCloseEnough || !item->Collidable || !Objects[item->ObjectNumber].usingDrawAnimatingItem)
		return false;

	// TODO: Find a better way to define objects which are collidable with camera.
	auto* object = &Objects[item->ObjectNumber];
	if (object->intelligent || object->isPickup || object->isPuzzleHole || object->collision == nullptr)
		return false;

	// Check extents, if any 2 bounds are smaller than threshold, discard.
	auto extents = TO_DX_BBOX(item->Pose, GetBoundsAccurate(item)).Extents;
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

std::vector<short> FillCollideableItemList()
{
	std::vector<short> itemList;
	auto& roomList = g_Level.Rooms[Camera.pos.roomNumber].neighbors;

	for (short i = 0; i < g_Level.NumItems; i++)
	{
		auto item = &g_Level.Items[i];

		if (std::find(roomList.begin(), roomList.end(), item->RoomNumber) == roomList.end())
			continue;

		if (!CheckItemCollideCamera(&g_Level.Items[i]))
			continue;

		itemList.push_back(i);
	}

	return itemList;
}

bool CheckStaticCollideCamera(MESH_INFO* mesh)
{
	bool isCloseEnough = Vector3Int::Distance(mesh->pos.Position, Camera.pos.ToVector3Int()) <= COLL_CHECK_THRESHOLD;
	if (!isCloseEnough)
		return false;

	if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	auto bounds = GetBoundsAccurate(mesh, false);
	auto extents = Vector3(
		abs(bounds->X1 - bounds->X2),
		abs(bounds->Y1 - bounds->Y2),
		abs(bounds->Z1 - bounds->Z2)
	);

	// Check extents; if any two bounds are smaller than threshold, discard.
	if ((abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.y) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.x) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD) ||
		(abs(extents.y) < COLL_DISCARD_THRESHOLD && abs(extents.z) < COLL_DISCARD_THRESHOLD))
	{
		return false;
	}

	return true;
}

std::vector<MESH_INFO*> FillCollideableStaticsList()
{
	std::vector<MESH_INFO*> staticList;
	auto& roomList = g_Level.Rooms[Camera.pos.roomNumber].neighbors;

	for (int i : roomList)
	{
		auto* room = &g_Level.Rooms[i];

		for (short j = 0; j < room->mesh.size(); j++)
		{
			if (!CheckStaticCollideCamera(&room->mesh[j]))
				continue;

			staticList.push_back(&room->mesh[j]);
		}
	}

	return staticList;
}

void ItemsCollideCamera()
{
	float radius = CLICK(0.5f);
	auto itemList = FillCollideableItemList();

	// Collide with items in the items list.
	for (int i = 0; i < itemList.size(); i++)
	{
		auto* item = &g_Level.Items[itemList[i]];
		if (!item)
			return;

		// Break off if camera is stuck behind an object and the player runs off.
		auto distance = Vector3Int::Distance(item->Pose.Position, LaraItem->Pose.Position);
		if (distance > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(item);
		if (TestBoundsCollideCamera(bounds, &item->Pose, CAMERA_RADIUS))
			ItemPushCamera(bounds, &item->Pose, radius);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.AddDebugBox(TO_DX_BBOX(item->Pose, bounds),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::DIMENSION_STATS);
#endif
	}

	// Done.
	itemList.clear();

	// Collide with static meshes.
	auto staticList = FillCollideableStaticsList();
	for (auto* mesh : staticList)
	{
		if (!mesh)
			return;

		auto distance = Vector3Int::Distance(mesh->pos.Position, LaraItem->Pose.Position);
		if (distance > COLL_CANCEL_THRESHOLD)
			continue;

		auto bounds = GetBoundsAccurate(mesh, false);
		if (TestBoundsCollideCamera(bounds, &mesh->pos, CAMERA_RADIUS))
			ItemPushCamera(bounds, &mesh->pos, radius);

#ifdef _DEBUG
		TEN::Renderer::g_Renderer.AddDebugBox(TO_DX_BBOX(mesh->pos, bounds),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::DIMENSION_STATS);
#endif
	}

	// Done.
	staticList.clear();
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, nullptr, SoundEnvironment::Land, 0.25f);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(GetRandomControl() & 0x1FF))
		{
			RumbleCounter = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (RumbleCounter >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			RumbleCounter++;
			Camera.bounce = -(GetRandomControl() % RumbleCounter);
		}
	}
}

void SetScreenFadeOut(float speed)
{
	if (!ScreenFading)
	{
		ScreenFading = true;
		ScreenFadeStart = 1.0f;
		ScreenFadeEnd = 0;
		ScreenFadeSpeed = speed;
		ScreenFadeCurrent = ScreenFadeStart;
	}
}

void SetScreenFadeIn(float speed)
{
	if (!ScreenFading)
	{
		ScreenFading = true;
		ScreenFadeStart = 0;
		ScreenFadeEnd = 1.0f;
		ScreenFadeSpeed = speed;
		ScreenFadeCurrent = ScreenFadeStart;
	}
}

void SetCinematicBars(float height, float speed)
{
	CinematicBarsDestinationHeight = height;
	CinematicBarsSpeed = speed;
}

void ClearCinematicBars()
{
	CinematicBarsHeight = 0;
	CinematicBarsDestinationHeight = 0;
	CinematicBarsSpeed = 0;
}

void UpdateFadeScreenAndCinematicBars()
{
	if (CinematicBarsDestinationHeight < CinematicBarsHeight)
	{
		CinematicBarsHeight -= CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight > CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}
	else if (CinematicBarsDestinationHeight > CinematicBarsHeight)
	{
		CinematicBarsHeight += CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight < CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}

	int oldScreenFadeCurrent = ScreenFadeCurrent;

	if (ScreenFadeEnd != 0 && ScreenFadeEnd >= ScreenFadeCurrent)
	{
		ScreenFadeCurrent += ScreenFadeSpeed;
		if (ScreenFadeCurrent > ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			if (oldScreenFadeCurrent >= ScreenFadeCurrent)
			{
				ScreenFadedOut = true;
				ScreenFading = false;
			}

		}
	}
	else if (ScreenFadeEnd < ScreenFadeCurrent)
	{
		ScreenFadeCurrent -= ScreenFadeSpeed;
		if (ScreenFadeCurrent < ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			ScreenFading = false;
		}
	}
}

void HandleOptics()
{
	bool breakOptics = true;

	if (!LaserSight && BinocularOn) // Imitate pushing look key in binocular mode
	{
		TrInput |= IN_LOOK;
		DbInput = 0;
	}

	// We are standing, can use optics.
	if (LaraItem->Animation.ActiveState == LS_IDLE || LaraItem->Animation.AnimNumber == LA_STAND_IDLE)
		breakOptics = false;

	// We are crouching, can use optics.
	if ((Lara.Control.IsLow || TrInput & IN_CROUCH) &&
		(LaraItem->Animation.TargetState == LS_CROUCH_IDLE || LaraItem->Animation.AnimNumber == LA_CROUCH_IDLE))
		breakOptics = false;

	// If any input but optic controls (directions + action), immediately exit optics.
	if ((TrInput & ~IN_OPTIC_CONTROLS) != IN_NONE)
		breakOptics = true;

	// If lasersight, and no look is pressed, exit optics.
	if (LaserSight && !(TrInput & IN_LOOK))
		breakOptics = true;

	if (!LaserSight && !breakOptics && (TrInput == IN_LOOK)) // Engage lasersight, if available.
	{
		if (Lara.Control.HandStatus == HandStatus::WeaponReady &&
			(Lara.Control.Weapon.GunType == LaraWeaponType::HK ||
				(Lara.Control.Weapon.GunType == LaraWeaponType::Revolver && Lara.Weapons[(int)LaraWeaponType::Revolver].HasLasersight) ||
				(Lara.Control.Weapon.GunType == LaraWeaponType::Crossbow && Lara.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)))
		{
			BinocularRange = 128;
			BinocularOldCamera = Camera.oldType;
			BinocularOn = true;
			LaserSight = true;
			Lara.Inventory.IsBusy = true;
			return;
		}
	}

	if (!breakOptics)
		return;

	// Nothing to process, exit.
	if (!BinocularOn && !LaserSight)
		return;

	BinocularRange = 0;
	BinocularOn = false;
	LaserSight = false;
	Camera.type = BinocularOldCamera;
	Camera.bounce = 0;
	AlterFOV(ANGLE(80.0f));

	LaraItem->MeshBits = ALL_JOINT_BITS;
	Lara.Inventory.IsBusy = false;
	ResetLaraFlex(LaraItem);

	TrInput &= ~IN_LOOK;
}
