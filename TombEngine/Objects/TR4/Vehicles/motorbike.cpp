#include "framework.h"
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Objects/TR4/Vehicles/motorbike.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/gui.h"
#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/effects/simple_particle.h"
#include "Game/health.h"
#include "Game/camera.h"
#include "Game/animation.h"
#include "Specific/prng.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

using std::vector;
using namespace TEN::Math::Random;

static char ExhaustStart = 0;
static bool DisableDismount = false;

constexpr auto MOTORBIKE_RADIUS = 500;
constexpr auto MOTORBIKE_FRICTION = 384;
constexpr auto MOTORBIKE_FRONT = 500;
constexpr auto MOTORBIKE_SIDE = 350;
constexpr auto MOTORBIKE_SLIP = 100;

constexpr auto MOTORBIKE_ACCEL_1 = 64 * 256;
constexpr auto MOTORBIKE_ACCEL_2 = 112 * 256;
constexpr auto MOTORBIKE_ACCEL_MAX = 192 * 256;
constexpr auto MOTORBIKE_ACCEL = 128 * 256;
constexpr auto MOTORBIKE_BACKING_VEL = 8 * 256;
constexpr auto MOTORBIKE_BIG_SLOWDOWN = 48 * 256;
constexpr auto MOTORBIKE_SLOWDOWN1 = 1088; // 4.25f * 256; // TODO: Float velocities. @Sezz 2022.06.16
constexpr auto MOTORBIKE_SLOWDOWN2 = 6 * 256;

constexpr auto MOTORBIKE_PITCH_SLOWDOWN = 0x8000;
constexpr auto MOTORBIKE_BITCH_MAX = 0xA000;

#define MOTORBIKE_MOMENTUM_TURN_ANGLE_MIN ANGLE(4.0f)
#define MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX ANGLE(1.5f)
#define MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX2 ANGLE(150.0f)
#define MOTORBIKE_DEFAULT_HTURN ANGLE(1.5f)
#define MOTORBIKE_HTURN ANGLE(0.5f)
#define MOTORBIKE_MAX_HTURN ANGLE(5.0f)

#define MOTORBIKE_IN_ACCELERATE IN_ACTION
#define MOTORBIKE_IN_REVERSE	IN_BACK
#define MOTORBIKE_IN_SPEED		IN_SPRINT
#define MOTORBIKE_IN_BRAKE		IN_JUMP
#define	MOTORBIKE_IN_LEFT		(IN_LEFT | IN_LSTEP)
#define	MOTORBIKE_IN_RIGHT		(IN_RIGHT | IN_RSTEP)

enum MotorbikeState
{
	BIKE_EMPTY,
	BIKE_MOVING_FRONT,
	BIKE_MOVING_LEFT,
	BIKE_MOVING_BACK,
	BIKE_MOVING_BACK_LOOP,
	BIKE_EMPTY3,
	BIKE_STOP,
	BIKE_DEATH,
	BIKE_FALLING,
	BIKE_ENTER, // include unlocking state
	BIKE_EXIT,
	BIKE_HITFRONT,
	BIKE_HITBACK,
	BIKE_HITRIGHT,
	BIKE_HITLEFT,
	BIKE_IDLE,
	BIKE_LOADING_BOOST, // not used !
	BIKE_LANDING,
	BIKE_ACCELERATE,
	BIKE_EMPTY5,
	BIKE_EMPTY6,
	BIKE_NOTUSED,
	BIKE_MOVING_RIGHT
};

enum MotorbikeAnim
{
	MOTORBIKE_ANIM_DIE = 0,
	MOTORBIKE_ANIM_BRAKE = 1,
	MOTORBIKE_ANIM_MOVE_FORWARD = 2,
	MOTORBIKE_ANIM_START_LEFT = 3,
	MOTORBIKE_ANIM_LEFT = 4,
	MOTORBIKE_ANIM_END_LEFT = 5,
	MOTORBIKE_ANIM_START_FALL = 6,
	MOTORBIKE_ANIM_FALLING = 7,
	MOTORBIKE_ANIM_FALL_LAND = 8,
	MOTORBIKE_ANIM_ENTER = 9,
	MOTORBIKE_ANIM_EXIT = 10,
	MOTORBIKE_ANIM_FRONT_HIT = 11,
	MOTORBIKE_ANIM_BACK_HIT = 12,
	MOTORBIKE_ANIM_LEFT_HIT = 13,
	MOTORBIKE_ANIM_RIGHT_HIT = 14,
	MOTORBIKE_ANIM_REV = 15, //unused? it looks like she's revving the engine but I've never seen it before
	MOTORBIKE_ANIM_SLOWDOWN = 16,
	MOTORBIKE_ANIM_UNUSED = 17,
	MOTORBIKE_ANIM_IDLE = 18,
	MOTORBIKE_ANIM_START_RIGHT = 19,
	MOTORBIKE_ANIM_RIGHT = 20,
	MOTORBIKE_ANIM_END_RIGHT = 21,
	MOTORBIKE_ANIM_START_JUMP = 22,
	MOTORBIKE_ANIM_JUMPING = 23,
	MOTORBIKE_ANIM_JUMP_LAND = 24,
	MOTORBIKE_ANIM_KICKSTART = 25,
	MOTORBIKE_ANIM_BACK_START = 26,
	MOTORBIKE_ANIM_BACK_LOOP = 27,
	MOTORBIKE_ANIM_UNLOCK = 28
};

enum MotorbikeFlags
{
	MOTORBIKE_FLAG_BOOST = (1 << 0),
	MOTORBIKE_FLAG_FALLING = (1 << 6),
	MOTORBIKE_FLAG_DEATH = (1 << 7)
};

static MotorbikeInfo* GetMotorbikeInfo(ItemInfo* motorbikeItem)
{
	return (MotorbikeInfo*)motorbikeItem->Data;
}

void InitialiseMotorbike(short itemNumber)
{
	auto* motorbikeItem = &g_Level.Items[itemNumber];
	motorbikeItem->Data = MotorbikeInfo();
	auto* motorbike = GetMotorbikeInfo(motorbikeItem);

	motorbike = motorbikeItem->Data;
	motorbike->Velocity = 0;
	motorbike->TurnRate = 0;
	motorbike->Pitch = 0;
	motorbike->MomentumAngle = motorbikeItem->Pose.Orientation.y;
	motorbike->WallShiftRotation = 0;
	motorbike->ExtraRotation = 0;
	motorbike->Flags = NULL;
	motorbike->LightPower = 0;
	motorbike->LeftWheelRotation = 0; // left wheel
	motorbike->RightWheelsRotation = 0; // two wheel in the principal body
	motorbikeItem->MeshBits = 0x3F7;
}

static int TestMotorbikeHeight(ItemInfo* motorbikeItem, int dz, int dx, Vector3Int* pos)
{
	float sinX = phd_sin(motorbikeItem->Pose.Orientation.x);
	float sinY = phd_sin(motorbikeItem->Pose.Orientation.y);
	float cosY = phd_cos(motorbikeItem->Pose.Orientation.y);
	float sinZ = phd_sin(motorbikeItem->Pose.Orientation.z);

	pos->x = motorbikeItem->Pose.Position.x + (dz * sinY) + (dx * cosY);
	pos->y = motorbikeItem->Pose.Position.y - (dz * sinX) + (dx * sinZ);
	pos->z = motorbikeItem->Pose.Position.z + (dz * cosY) - (dx * sinY);

	auto probe = GetCollision(pos->x, pos->y, pos->z, motorbikeItem->RoomNumber);

	if (pos->y < probe.Position.Ceiling || probe.Position.Ceiling == NO_HEIGHT)
		return NO_HEIGHT;

	return probe.Position.Floor;
}

static int DoMotorbikeShift(ItemInfo* motorbikeItem, Vector3Int* pos, Vector3Int* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);
	int oldX = old->x / SECTOR(1);
	int oldZ = old->z / SECTOR(1);
	int shiftX = pos->x & (SECTOR(1) - 1);
	int shiftZ = pos->z & (SECTOR(1) - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			motorbikeItem->Pose.Position.z += old->z - pos->z;
			motorbikeItem->Pose.Position.x += old->x - pos->x;
		}
		else if (z > oldZ)
		{
			motorbikeItem->Pose.Position.z -= shiftZ + 1;
			return (pos->x - motorbikeItem->Pose.Position.x);
		}
		else
		{
			motorbikeItem->Pose.Position.z += SECTOR(1) - shiftZ;
			return (motorbikeItem->Pose.Position.x - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			motorbikeItem->Pose.Position.x -= shiftX + 1;
			return (motorbikeItem->Pose.Position.z - pos->z);
		}
		else
		{
			motorbikeItem->Pose.Position.x += SECTOR(1) - shiftX;
			return (pos->z - motorbikeItem->Pose.Position.z);
		}
	}
	else
	{
		x = 0;
		z = 0;

		int floorHeight = GetCollision(old->x, pos->y, pos->z, motorbikeItem->RoomNumber).Position.Floor;
		if (floorHeight < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = SECTOR(1) - shiftZ;
		}

		floorHeight = GetCollision(pos->x, pos->y, old->z, motorbikeItem->RoomNumber).Position.Floor;
		if (floorHeight < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = SECTOR(1) - shiftX;
		}

		if (x && z)
		{
			motorbikeItem->Pose.Position.z += z;
			motorbikeItem->Pose.Position.x += x;
		}
		else if (z)
		{
			motorbikeItem->Pose.Position.z += z;

			if (z > 0)
				return (motorbikeItem->Pose.Position.x - pos->x);
			else
				return (pos->x - motorbikeItem->Pose.Position.x);
		}
		else if (x)
		{
			motorbikeItem->Pose.Position.x += x;

			if (x > 0)
				return (pos->z - motorbikeItem->Pose.Position.z);
			else
				return (motorbikeItem->Pose.Position.z - pos->z);
		}
		else
		{
			motorbikeItem->Pose.Position.z += old->z - pos->z;
			motorbikeItem->Pose.Position.x += old->x - pos->x;
		}
	}

	return 0;
}

static void DrawMotorbikeLight(ItemInfo* motorbikeItem)
{
	auto* motorbike = GetMotorbikeInfo(motorbikeItem);

	if (motorbike->LightPower <= 0)
		return;

	auto start = Vector3Int(0, -470, 1836);
	GetJointAbsPosition(motorbikeItem, &start, 0);

	auto target = Vector3Int(0, -470, 20780);
	GetJointAbsPosition(motorbikeItem, &target, 0);

	int random = (motorbike->LightPower * 2) - (GetRandomControl() & 0xF);

	// TODO: Use target as direction vector for spotlight.
	TriggerDynamicLight(start.x, start.y, start.z, 8, random, random / 2, 0);
}

static bool TestMotorbikeMount(short itemNumber)
{
	auto* motorbikeItem = &g_Level.Items[itemNumber];

	if (motorbikeItem->Flags & ONESHOT || Lara.Control.HandStatus != HandStatus::Free || LaraItem->Animation.Airborne)
		return false;

	if ((abs(motorbikeItem->Pose.Position.y - LaraItem->Pose.Position.y) >= CLICK(1) ||
		!(TrInput & IN_ACTION)) && g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM1)
	{
		return false;
	}

	int dx = LaraItem->Pose.Position.x - motorbikeItem->Pose.Position.x;
	int dz = LaraItem->Pose.Position.z - motorbikeItem->Pose.Position.z;
	int distance = pow(dx, 2) + pow(dz, 2);
	if (distance > SECTOR(166))
		return false;

	int floorHeight = GetCollision(motorbikeItem).Position.Floor;
	if (floorHeight < -SECTOR(31.25f))
		return false;

	short angle = phd_atan(motorbikeItem->Pose.Position.z - LaraItem->Pose.Position.z, motorbikeItem->Pose.Position.x - LaraItem->Pose.Position.x) - motorbikeItem->Pose.Orientation.y;
	unsigned short deltaAngle = angle - motorbikeItem->Pose.Orientation.y;

	// Left.
	if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
	{
		if (deltaAngle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
			return false;
	}
	// Right.
	else
	{
		if (deltaAngle > ANGLE(225.0f) && deltaAngle < ANGLE(315.0f))
			return false;
	}

	return true;
}

void MotorbikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	if (laraItem->HitPoints >= 0 && Lara.Vehicle == NO_ITEM)
	{
		auto* motorbikeItem = &g_Level.Items[itemNumber];
		auto* motorbike = GetMotorbikeInfo(motorbikeItem);

        if (TestMotorbikeMount(itemNumber))
        {
            Lara.Vehicle = itemNumber;

			if (Lara.Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				CreateFlare(LaraItem, ID_FLARE_ITEM, false);
				UndrawFlareMeshes(laraItem);
				Lara.Control.Weapon.GunType = LaraWeaponType::None;
				Lara.Control.Weapon.RequestGunType = LaraWeaponType::None;
				Lara.Flare.Life = 0;
				Lara.Flare.ControlLeft = false;
			}

			Lara.Control.HandStatus = HandStatus::Free;

			short angle = phd_atan(motorbikeItem->Pose.Position.z - laraItem->Pose.Position.z, motorbikeItem->Pose.Position.x - laraItem->Pose.Position.x) - motorbikeItem->Pose.Orientation.y;
			if (angle <= -ANGLE(45.0f) || angle >= ANGLE(135.0f))
			{
				if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
				{
					laraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_UNLOCK;
					g_Gui.SetInventoryItemChosen(NO_ITEM);
				}
				else
					laraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_ENTER;

				laraItem->Animation.ActiveState = BIKE_ENTER;
				laraItem->Animation.TargetState = BIKE_ENTER;
			}

			motorbikeItem->HitPoints = 1;
            laraItem->Pose.Position = motorbikeItem->Pose.Position;
            laraItem->Pose.Orientation.y = motorbikeItem->Pose.Orientation.y;
            ResetLaraFlex(laraItem);
            Lara.HitDirection = -1;
            AnimateItem(laraItem);
            motorbike->Revs = 0;
            motorbike->LightPower = 0;
            motorbikeItem->Collidable = true;
        }
        else
            ObjectCollision(itemNumber, laraItem, coll);
    }
}

static void TriggerMotorbikeExhaustSmoke(int x, int y, int z, short angle, short speed, bool moving)
{
	int rnd = 0;
	BYTE trans, size;

	auto* sptr = GetFreeParticle();

	sptr->dR = 96;
	sptr->dG = 96;
	sptr->on = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;
	sptr->dB = 128;

	if (moving)
	{
		trans = speed * 4;
		sptr->dR = trans;
		sptr->dG = trans;
		sptr->dB = trans;
	}

	sptr->colFadeSpeed = 4;
	sptr->fadeToBlack = 4;
	rnd = (GetRandomControl() & 3) - (speed / 4096) + 20;
	if (rnd < 9)
	{
		sptr->life = 9;
		sptr->sLife = 9;
	}
	else
	{
		sptr->life = rnd;
		sptr->sLife = rnd;
	}

	sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	sptr->x = x + (GetRandomControl() & 0xF) - 8;
	sptr->y = y + (GetRandomControl() & 0xF) - 8;
	sptr->z = z + (GetRandomControl() & 0xF) - 8;
	sptr->xVel = speed * phd_sin(angle) / 4;
	sptr->yVel = (GetRandomControl() & 7) - 8;
	sptr->zVel = speed * phd_cos(angle) / 4;
	sptr->friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
		sptr->rotAng = (GetRandomControl() & 0xFFF);
		if (GetRandomControl() & 1)
			sptr->rotAdd = (GetRandomControl() & 7) - 24;
		else
			sptr->rotAdd = (GetRandomControl() & 7) + 24;
	}
	else
		sptr->flags = SP_EXPDEF | SP_DEF | SP_SCALE;

	sptr->scalar = 1;
	sptr->spriteIndex = (unsigned char)Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->gravity = (GetRandomControl() & 3) - 4;
	sptr->maxYvel = (GetRandomControl() & 7) - 8;
	size = (GetRandomControl() & 7) + (speed / 128) + 32;
	sptr->dSize = size;
	sptr->sSize = size / 2;
	sptr->size = size / 2;
}

static void DrawMotorBikeSmoke(ItemInfo* motorbikeItem)
{
	if (Lara.Vehicle == NO_ITEM)
		return;

	if (LaraItem->Animation.ActiveState != BIKE_ENTER && LaraItem->Animation.ActiveState != BIKE_EXIT)
	{
		Vector3Int pos;
		int speed;

		pos.x = 56;
		pos.y = -144;
		pos.z = -500;
		GetJointAbsPosition(motorbikeItem, &pos, 0);

		speed = motorbikeItem->Animation.Velocity;
		if (speed > 32 && speed < 64)
		{
			TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, motorbikeItem->Pose.Orientation.y - ANGLE(180), 64 - speed, TRUE);
			return;
		}

		if (ExhaustStart >= 16)
		{
			if (GetRandomControl() & 3)
				speed = 0;
			else
				speed = ((GetRandomControl() & 0xF) + (GetRandomControl() & 0x10)) * 64;
		}
		else
		{
			speed = ((GetRandomControl() & 0xF) + (GetRandomControl() & 0x10) + 2 * ExhaustStart) * 64;
		}

        TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, motorbikeItem->Pose.Orientation.y - ANGLE(180), speed, false);
    }
}

static int MotorBikeCheckGetOff(void)
{
	ItemInfo* item;

	if (Lara.Vehicle != NO_ITEM)
	{
		item = &g_Level.Items[Lara.Vehicle];
		if (LaraItem->Animation.ActiveState == BIKE_EXIT && LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
		{
			LaraItem->Pose.Orientation.y -= 0x4000;
			LaraItem->Animation.AnimNumber = LA_STAND_SOLID;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
			LaraItem->Animation.TargetState = LS_IDLE;
			LaraItem->Animation.ActiveState = LS_IDLE;
			LaraItem->Pose.Position.x -= 2 * phd_sin(item->Pose.Orientation.y);
			LaraItem->Pose.Position.z -= 2 * phd_cos(item->Pose.Orientation.y);
			LaraItem->Pose.Orientation.x = 0;
			LaraItem->Pose.Orientation.z = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.Control.HandStatus = HandStatus::Free;
			Lara.SprintEnergy = 120;
			return true;
		}

		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
			return true;

		// exit when falling
		// if (LaraItem->state_current == BIKE_EMPTY6) {

		// }
		// else if (LaraItem->state_current == BIKE_EMPTY5) {
		// lara death when falling too much
		// }

		return false;
	}
	else
		return false;
}

static int DoMotorBikeDynamics(int height, int fallspeed, int* y, int flags)
{
	int kick;

	if (height <= *y)
	{
		if (flags)
			return fallspeed;
		else
		{
			// On ground
			kick = (height - *y);

			if (kick < -80)
				kick = -80;

			fallspeed += ((kick - fallspeed) / 16);

			if (*y > height)
				*y = height;
		}
	}
	else
	{
		// In air
		*y += fallspeed;
		if (*y > height - 32)
		{
			*y = height;
			fallspeed = 0;
		}
		else
		{
			if (flags)
				fallspeed += flags;
			else
				fallspeed += GRAVITY;
		}
	}

	return fallspeed;
}

static int GetMotorbikeCollisionAnim(ItemInfo* motorbikeItem, Vector3Int* pos)
{
	pos->x = motorbikeItem->Pose.Position.x - pos->x;
	pos->z = motorbikeItem->Pose.Position.z - pos->z;

	if (pos->x || pos->z)
	{
		float sinY = phd_sin(motorbikeItem->Pose.Orientation.y);
		float cosY = phd_cos(motorbikeItem->Pose.Orientation.y);
		int front = (pos->z * cosY) + (pos->x * sinY);
		int side = (pos->z * -sinY) + (pos->x * cosY);

		if (abs(front) > abs(side))
			return (front > 0) + 13;
		else
			return (side <= 0) + 11;
	}

	return 0;
}

static int MotorBikeDynamics(ItemInfo* motorbikeItem)
{
	MotorbikeInfo* motorbike;
	Vector3Int bl_old, mtb_old, br_old, mtf_old, fl_old;
	Vector3Int fl, bl, mtf, mtb, br;
	Vector3Int oldpos, moved;
	FloorInfo* floor;
	int hmf_old, hbl_old, hbr_old, hmtb_old, hfl_old;
	int height, collide, speed, newspeed;
	short momentum = 0, rot, room_number;

	DisableDismount = false;
	motorbike = GetMotorbikeInfo(motorbikeItem);

	hfl_old = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, -MOTORBIKE_SIDE, &fl_old);
	hmf_old = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, STEP_SIZE / 2, &mtf_old);
	hbl_old = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, -MOTORBIKE_SIDE, &bl_old);
	hbr_old = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, STEP_SIZE / 2, &br_old);
	hmtb_old = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, 0, &mtb_old);

	oldpos.x = motorbikeItem->Pose.Position.x;
	oldpos.y = motorbikeItem->Pose.Position.y;
	oldpos.z = motorbikeItem->Pose.Position.z;

	if (bl_old.y > hbl_old)
		bl_old.y = hbl_old;
	if (br_old.y > hbr_old)
		br_old.y = hbr_old;
	if (fl_old.y > hfl_old)
		fl_old.y = hfl_old;
	if (mtf_old.y > hmf_old)
		mtf_old.y = hmf_old;
	if (mtb_old.y > hmtb_old)
		mtb_old.y = hmtb_old;

	if (motorbikeItem->Pose.Position.y <= (motorbikeItem->Floor - 8))
	{
		if (motorbike->TurnRate < -91)
			motorbike->TurnRate += 91;
		else if (motorbike->TurnRate > 91)
			motorbike->TurnRate -= 91;
		else
			motorbike->TurnRate = 0;

		motorbikeItem->Pose.Orientation.y += motorbike->TurnRate + motorbike->ExtraRotation;
		motorbike->MomentumAngle += ((motorbikeItem->Pose.Orientation.y - motorbike->MomentumAngle) / 32);
	}
	else
	{
		if (motorbike->TurnRate >= -182)
		{
			if (motorbike->TurnRate <= 182)
				motorbike->TurnRate = 0;
			else
				motorbike->TurnRate -= 182;
		}
		else
		{
			motorbike->TurnRate += 182;
		}

		motorbikeItem->Pose.Orientation.y += motorbike->TurnRate + motorbike->ExtraRotation;
		rot = motorbikeItem->Pose.Orientation.y - motorbike->MomentumAngle;
		momentum = MOTORBIKE_MOMENTUM_TURN_ANGLE_MIN - ((2 * motorbike->Velocity) / SECTOR(1));

		if (!(TrInput & MOTORBIKE_IN_ACCELERATE) && motorbike->Velocity > 0)
			momentum += (momentum / 2);

		if (rot < -MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX)
		{
			if (rot < -MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX2)
			{
				rot = -MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX2;
				motorbike->MomentumAngle = motorbikeItem->Pose.Orientation.y - rot;
			}
			else
			{
				motorbike->MomentumAngle -= momentum;
			}
		}
		else if (rot > MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX)
		{
			if (rot > MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX2)
			{
				rot = MOTORBIKE_MOMENTUM_TURN_ANGLE_MAX2;
				motorbike->MomentumAngle = motorbikeItem->Pose.Orientation.y - rot;
			}
			else
			{
				motorbike->MomentumAngle += momentum;
			}
		}
		else
		{
			motorbike->MomentumAngle = motorbikeItem->Pose.Orientation.y;
		}
	}

	room_number = motorbikeItem->RoomNumber;
	floor = GetFloor(motorbikeItem->Pose.Position.x, motorbikeItem->Pose.Position.y, motorbikeItem->Pose.Position.z, &room_number);
	height = GetFloorHeight(floor, motorbikeItem->Pose.Position.x, motorbikeItem->Pose.Position.y, motorbikeItem->Pose.Position.z);
	if (motorbikeItem->Pose.Position.y >= height)
		speed = motorbikeItem->Animation.Velocity * phd_cos(motorbikeItem->Pose.Orientation.x);
	else
		speed = motorbikeItem->Animation.Velocity;

	motorbikeItem->Pose.Position.z += speed * phd_cos(motorbike->MomentumAngle);
	motorbikeItem->Pose.Position.x += speed * phd_sin(motorbike->MomentumAngle);

	if (motorbikeItem->Pose.Position.y >= height)
	{
		short anglex = MOTORBIKE_SLIP * phd_sin(motorbikeItem->Pose.Orientation.x);
		if (abs(anglex) > 16)
		{
			short anglex2 = MOTORBIKE_SLIP * phd_sin(motorbikeItem->Pose.Orientation.x);
			if (anglex < 0)
				anglex2 = -anglex;
			if (anglex2 > 24)
				DisableDismount = true;
			anglex *= 16;
			motorbike->Velocity -= anglex;
		}

		short anglez = MOTORBIKE_SLIP * phd_sin(motorbikeItem->Pose.Orientation.z);
		if (abs(anglez) > 32)
		{
			short ang, angabs;
			DisableDismount = true;
			if (anglez >= 0)
				ang = motorbikeItem->Pose.Orientation.y + 0x4000;
			else
				ang = motorbikeItem->Pose.Orientation.y - 0x4000;
			angabs = abs(anglez) - 24;
			motorbikeItem->Pose.Position.x += angabs * phd_sin(ang);
			motorbikeItem->Pose.Position.z += angabs * phd_cos(ang);
		}
	}

	if (motorbike->Velocity <= MOTORBIKE_ACCEL || motorbike->Flags & MOTORBIKE_FLAG_BOOST)
	{
		if (motorbike->Velocity <= MOTORBIKE_ACCEL_MAX)
		{
			if (motorbike->Velocity < -MOTORBIKE_BIG_SLOWDOWN)
				motorbike->Velocity = -MOTORBIKE_BIG_SLOWDOWN;
		}
		else
			motorbike->Velocity = MOTORBIKE_ACCEL_MAX;
	}
	else
		motorbike->Velocity -= MOTORBIKE_SLOWDOWN1;

	moved.x = motorbikeItem->Pose.Position.x;
	moved.z = motorbikeItem->Pose.Position.z;

    if (!(motorbikeItem->Flags & ONESHOT))
        DoVehicleCollision(motorbikeItem, MOTORBIKE_RADIUS);

	int rot1 = 0;
	int rot2 = 0;

	int hfl = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, -MOTORBIKE_SIDE, &fl);
	if (hfl < fl_old.y - STEP_SIZE)
	{
		rot1 = abs(4 * DoMotorbikeShift(motorbikeItem, &fl, &fl_old));
	}

	int hbl = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, -MOTORBIKE_SIDE, &bl);
	if (hbl < bl_old.y - STEP_SIZE)
	{
		if (rot1)
			rot1 += abs(4 * DoMotorbikeShift(motorbikeItem, &bl, &bl_old));
		else
			rot1 -= abs(4 * DoMotorbikeShift(motorbikeItem, &bl, &bl_old));
	}

	int hmtf = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, STEP_SIZE / 2, &mtf);
	if (hmtf < mtf_old.y - STEP_SIZE)
		rot2 -= abs(4 * DoMotorbikeShift(motorbikeItem, &bl, &bl_old));

	int hmtb = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, 0, &mtb);
	if (hmtb < mtb_old.y - STEP_SIZE)
		DoMotorbikeShift(motorbikeItem, &mtb, &mtb_old);

	int hbr = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, STEP_SIZE / 2, &br);
	if (hbr < br_old.y - STEP_SIZE)
	{
		if (rot2)
			rot2 -= abs(4 * DoMotorbikeShift(motorbikeItem, &bl, &bl_old));
		else
			rot2 += abs(4 * DoMotorbikeShift(motorbikeItem, &bl, &bl_old));
	}

	if (rot1)
		rot2 = rot1;

	room_number = motorbikeItem->RoomNumber;
	floor = GetFloor(motorbikeItem->Pose.Position.x, motorbikeItem->Pose.Position.y, motorbikeItem->Pose.Position.z, &room_number);
	height = GetFloorHeight(floor, motorbikeItem->Pose.Position.x, motorbikeItem->Pose.Position.y, motorbikeItem->Pose.Position.z);
	if (height < (motorbikeItem->Pose.Position.y - STEP_SIZE))
		DoMotorbikeShift(motorbikeItem, (Vector3Int*)&motorbikeItem->Pose, &oldpos);

	if (!motorbike->Velocity)
		rot2 = 0;

	motorbike->WallShiftRotation = (motorbike->WallShiftRotation + rot2) / 2;
	if (abs(motorbike->WallShiftRotation) < 2)
		motorbike->WallShiftRotation = 0;

	if (abs(motorbike->WallShiftRotation - motorbike->ExtraRotation) >= 4)
		motorbike->ExtraRotation += ((motorbike->WallShiftRotation - motorbike->ExtraRotation) / 4);
	else
		motorbike->ExtraRotation = motorbike->WallShiftRotation;

	collide = GetMotorbikeCollisionAnim(motorbikeItem, &moved);
	if (collide)
	{
		newspeed = ((motorbikeItem->Pose.Position.z - oldpos.z) * phd_cos(motorbike->MomentumAngle) + (motorbikeItem->Pose.Position.x - oldpos.x) * phd_sin(motorbike->MomentumAngle)) * 256;
		if (&g_Level.Items[Lara.Vehicle] == motorbikeItem && motorbike->Velocity >= MOTORBIKE_ACCEL && newspeed < (motorbike->Velocity - 10))
		{
			LaraItem->HitPoints -= ((motorbike->Velocity - newspeed) / 128);
			LaraItem->HitStatus = true;
		}

		if (motorbike->Velocity > 0 && newspeed < motorbike->Velocity)
			motorbike->Velocity = (newspeed < 0) ? 0 : newspeed;
		else if (motorbike->Velocity < 0 && newspeed > motorbike->Velocity)
			motorbike->Velocity = (newspeed > 0) ? 0 : newspeed;

		if (motorbike->Velocity < -MOTORBIKE_BIG_SLOWDOWN)
			motorbike->Velocity = -MOTORBIKE_BIG_SLOWDOWN;
	}

	return collide;
}

static bool TestMotorbikeDismount(void)
{
	auto item = &g_Level.Items[Lara.Vehicle];
	auto angle = item->Pose.Orientation.y + 0x4000;
	auto x = item->Pose.Position.x + MOTORBIKE_RADIUS * phd_sin(angle);
	auto y = item->Pose.Position.y;
	auto z = item->Pose.Position.z + MOTORBIKE_RADIUS * phd_cos(angle);

	auto collResult = GetCollision(x, y, z, item->RoomNumber);

	if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT) // Was previously set to -NO_HEIGHT by TokyoSU -- Lwmte 23.08.21
		return false;

	if (abs(collResult.Position.Floor - item->Pose.Position.y) > STEP_SIZE)
		return false;

	if ((collResult.Position.Ceiling - item->Pose.Position.y) > -LARA_HEIGHT)
		return false;

	if ((collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
		return false;

	return true;
}

static void AnimateMotorbike(ItemInfo* item, int collide, BOOL dead)
{
	MotorbikeInfo* motorbike;
	motorbike = GetMotorbikeInfo(item);

	if (item->Pose.Position.y == item->Floor
		|| LaraItem->Animation.ActiveState == BIKE_FALLING
		|| LaraItem->Animation.ActiveState == BIKE_LANDING
		|| LaraItem->Animation.ActiveState == BIKE_EMPTY6
		|| dead)
	{
		if (!collide
			|| LaraItem->Animation.ActiveState == BIKE_HITBACK
			|| LaraItem->Animation.ActiveState == BIKE_HITFRONT
			|| LaraItem->Animation.ActiveState == BIKE_HITLEFT
			|| LaraItem->Animation.ActiveState == BIKE_EMPTY6
			|| motorbike->Velocity <= 10922
			|| dead)
		{
			switch (LaraItem->Animation.ActiveState)
			{
			case BIKE_IDLE:
				if (dead)
					LaraItem->Animation.TargetState = BIKE_DEATH;
				else 
				{

					bool dismount;
					if ((TrInput & MOTORBIKE_IN_RIGHT) && (TrInput & MOTORBIKE_IN_BRAKE))
						dismount = true;
					else if (!((TrInput & MOTORBIKE_IN_RIGHT) && (TrInput & MOTORBIKE_IN_BRAKE)))
						dismount = false;

					if (!dismount || motorbike->Velocity || DisableDismount)
					{
						if (TrInput & MOTORBIKE_IN_ACCELERATE && !(TrInput & MOTORBIKE_IN_BRAKE))
							LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
						else if (TrInput & MOTORBIKE_IN_REVERSE)
							LaraItem->Animation.TargetState = BIKE_MOVING_BACK;
					}
					else if (dismount && TestMotorbikeDismount())
					{
						LaraItem->Animation.TargetState = BIKE_EXIT;
					}
					else
					{
						LaraItem->Animation.TargetState = BIKE_IDLE;
					}
				}
				break;

			case BIKE_MOVING_FRONT:
				if (dead)
				{
					if (motorbike->Velocity <= MOTORBIKE_ACCEL_1)
						LaraItem->Animation.TargetState = BIKE_DEATH;
					else
						LaraItem->Animation.TargetState = BIKE_EMPTY5;
				}
				else if (motorbike->Velocity & -256 || TrInput & (MOTORBIKE_IN_ACCELERATE | MOTORBIKE_IN_BRAKE))
				{
					if (TrInput & MOTORBIKE_IN_LEFT)
						LaraItem->Animation.TargetState = BIKE_MOVING_LEFT;
					else if (TrInput & MOTORBIKE_IN_RIGHT)
						LaraItem->Animation.TargetState = BIKE_MOVING_RIGHT;
					else if (TrInput & MOTORBIKE_IN_BRAKE)
					{
						if (motorbike->Velocity <= 0x5554)
							LaraItem->Animation.TargetState = BIKE_EMPTY3;
						else
							LaraItem->Animation.TargetState = BIKE_STOP;
					}
					else if (TrInput & MOTORBIKE_IN_REVERSE && motorbike->Velocity <= MOTORBIKE_BACKING_VEL)
						LaraItem->Animation.TargetState = BIKE_MOVING_BACK;
					else if (motorbike->Velocity == 0)
						LaraItem->Animation.TargetState = BIKE_IDLE;
				}
				else
					LaraItem->Animation.TargetState = BIKE_IDLE;
				break;

			case BIKE_MOVING_LEFT:
				if (motorbike->Velocity & -256)
				{
					if (TrInput & MOTORBIKE_IN_RIGHT || !(TrInput & MOTORBIKE_IN_LEFT))
						LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
				}
				else
					LaraItem->Animation.TargetState = BIKE_IDLE;
				if (motorbike->Velocity == 0)
					LaraItem->Animation.TargetState = BIKE_IDLE;
				break;

			case BIKE_MOVING_BACK:
				if (TrInput & MOTORBIKE_IN_REVERSE)
					LaraItem->Animation.TargetState = BIKE_MOVING_BACK_LOOP;
				else
					LaraItem->Animation.TargetState = BIKE_IDLE;
				break;

			case BIKE_MOVING_RIGHT:
				if (motorbike->Velocity & -256)
				{
					if (TrInput & MOTORBIKE_IN_LEFT || !(TrInput & MOTORBIKE_IN_RIGHT))
						LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
				}
				else
					LaraItem->Animation.TargetState = BIKE_IDLE;
				if (motorbike->Velocity == 0)
					LaraItem->Animation.TargetState = BIKE_IDLE;
				break;

			case BIKE_EMPTY3:
			case BIKE_STOP:
			case BIKE_ACCELERATE:
				if (motorbike->Velocity & -256)
				{
					if (TrInput & MOTORBIKE_IN_LEFT)
						LaraItem->Animation.TargetState = BIKE_MOVING_LEFT;
					if (TrInput & MOTORBIKE_IN_RIGHT)
						LaraItem->Animation.TargetState = BIKE_MOVING_RIGHT;
				}
				else
				{
					LaraItem->Animation.TargetState = BIKE_IDLE;
				}
				break;

			case BIKE_FALLING:
				if (item->Pose.Position.y == item->Floor)
				{
					LaraItem->Animation.TargetState = BIKE_LANDING;

					int fallspeed_damage = item->Animation.VerticalVelocity - 140;
					if (fallspeed_damage > 0)
					{
						if (fallspeed_damage <= 100)
							LaraItem->HitPoints -= (-1000 * fallspeed_damage * fallspeed_damage) / 10000;
						else
							LaraItem->HitPoints = 0;
					}
				}
				else if (item->Animation.VerticalVelocity > 220)
					motorbike->Flags |= MOTORBIKE_FLAG_FALLING;
				break;

			case BIKE_HITFRONT:
			case BIKE_HITBACK:
			case BIKE_HITRIGHT:
			case BIKE_HITLEFT:
				if (TrInput & (MOTORBIKE_IN_ACCELERATE | MOTORBIKE_IN_BRAKE))
					LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
				break;

			}
		}
		else
		{
			switch (collide)
			{

			case 13:
				LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_BACK_HIT;
				LaraItem->Animation.ActiveState = BIKE_HITBACK;
				LaraItem->Animation.TargetState = BIKE_HITBACK;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				break;

			case 14:
				LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_FRONT_HIT;
				LaraItem->Animation.ActiveState = BIKE_HITFRONT;
				LaraItem->Animation.TargetState = BIKE_HITFRONT;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				break;

			case 11:
				LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_RIGHT_HIT;
				LaraItem->Animation.ActiveState = BIKE_HITRIGHT;
				LaraItem->Animation.TargetState = BIKE_HITRIGHT;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				break;

			default:
			case 12:
				LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_LEFT_HIT;
				LaraItem->Animation.ActiveState = BIKE_HITLEFT;
				LaraItem->Animation.TargetState = BIKE_HITLEFT;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				break;
			}
		}
	}
	else
	{
		if (motorbike->Velocity >= 0)
			LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_START_JUMP;
		else
			LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_START_FALL;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
		LaraItem->Animation.ActiveState = BIKE_FALLING;
		LaraItem->Animation.TargetState = BIKE_FALLING;
	}
}

static int MotorbikeUserControl(ItemInfo* motorbikeItem, ItemInfo* laraItem, int height, int* pitch)
{
	auto* motorbike = GetMotorbikeInfo(motorbikeItem);
	auto* lara = GetLaraInfo(laraItem);

	if (motorbike->LightPower < 127)
	{
		motorbike->LightPower += (GetRandomControl() & 7) + 3;
		if (motorbike->LightPower > 127)
			motorbike->LightPower = 127;
	}

	if (motorbike->Revs > 0x10)
	{
		motorbike->Velocity += (motorbike->Revs / 16);
		motorbike->Revs -= (motorbike->Revs / 80);
	}
	else
		motorbike->Revs = 0;

	if ((TrInput & MOTORBIKE_IN_SPEED) && (TrInput & MOTORBIKE_IN_ACCELERATE) && lara->SprintEnergy)
	{
		motorbike->Flags |= MOTORBIKE_FLAG_BOOST;

		lara->SprintEnergy -= 2;
		if (lara->SprintEnergy > MOTORBIKE_ACCEL)//hmm
		{
			motorbike->Flags &= ~MOTORBIKE_FLAG_BOOST;
			lara->SprintEnergy = 0;
		}
	}
	else
		motorbike->Flags &= ~MOTORBIKE_FLAG_BOOST;

	if (motorbikeItem->Pose.Position.y >= (height - CLICK(1)))
	{
		if (!motorbike->Velocity && (TrInput & IN_LOOK))
			LookUpDown(laraItem);

		// Moving forward.
		if (motorbike->Velocity > 0)
		{
			if (TrInput & MOTORBIKE_IN_LEFT)
			{
				if (motorbike->Velocity > MOTORBIKE_ACCEL_1)
					motorbike->TurnRate -= MOTORBIKE_DEFAULT_HTURN;
				else
					motorbike->TurnRate -= (MOTORBIKE_DEFAULT_HTURN * ((float)motorbike->Velocity / 16384.0f));

				if (motorbike->TurnRate < -MOTORBIKE_MAX_HTURN)
					motorbike->TurnRate = -MOTORBIKE_MAX_HTURN;
			}
			else if (TrInput & MOTORBIKE_IN_RIGHT)
			{
				if (motorbike->Velocity > MOTORBIKE_ACCEL_1)
					motorbike->TurnRate += MOTORBIKE_DEFAULT_HTURN;
				else
					motorbike->TurnRate += (MOTORBIKE_DEFAULT_HTURN * ((float)motorbike->Velocity / 16384.0f));

				if (motorbike->TurnRate > MOTORBIKE_MAX_HTURN)
					motorbike->TurnRate = MOTORBIKE_MAX_HTURN;
			}
		}
		// Moving back.
		else if (motorbike->Velocity < 0)
		{
			if (TrInput & MOTORBIKE_IN_LEFT)
			{
				motorbike->TurnRate += MOTORBIKE_HTURN;
				if (motorbike->TurnRate > MOTORBIKE_MAX_HTURN)
					motorbike->TurnRate = MOTORBIKE_MAX_HTURN;
			}
			else if (TrInput & MOTORBIKE_IN_RIGHT)
			{
				motorbike->TurnRate -= MOTORBIKE_HTURN;
				if (motorbike->TurnRate < -MOTORBIKE_MAX_HTURN)
					motorbike->TurnRate = -MOTORBIKE_MAX_HTURN;
			}
		}

		if (TrInput & MOTORBIKE_IN_BRAKE)
		{
			auto pos = Vector3Int(0, -144, -1024);
			GetJointAbsPosition(motorbikeItem, &pos, NULL);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 0x40, 0, 0);
			motorbikeItem->MeshBits = 0x5F7;
		}
		else
			motorbikeItem->MeshBits = 0x3F7;

		if (TrInput & MOTORBIKE_IN_BRAKE)
		{
			if (motorbike->Velocity < 0)
			{
				motorbike->Velocity += 3 * 256;
				if (motorbike->Velocity > 0)
					motorbike->Velocity = 0;
			}
			else
			{
				motorbike->Velocity -= 3 * 256;
				if (motorbike->Velocity < 0)
					motorbike->Velocity = 0;
			}
		}
		else if (TrInput & MOTORBIKE_IN_ACCELERATE)
		{
			if (motorbike->Velocity < MOTORBIKE_ACCEL_MAX)
			{
				if (motorbike->Velocity < MOTORBIKE_ACCEL_1)
					motorbike->Velocity += ((MOTORBIKE_ACCEL_1 + MOTORBIKE_BACKING_VEL - motorbike->Velocity) / 8) + 8;
				else if (motorbike->Velocity < MOTORBIKE_ACCEL_2)
					motorbike->Velocity += ((MOTORBIKE_ACCEL_2 + MOTORBIKE_BACKING_VEL - motorbike->Velocity) / 16) + 4;
				else if (motorbike->Velocity < MOTORBIKE_ACCEL_MAX)
					motorbike->Velocity += ((MOTORBIKE_ACCEL_MAX - motorbike->Velocity) / 16) + 2;

				if (motorbike->Flags & MOTORBIKE_FLAG_BOOST)
					motorbike->Velocity += 256;
			}
			else
				motorbike->Velocity = MOTORBIKE_ACCEL_MAX;

			// Apply friction according to turn.
			motorbike->Velocity -= abs(motorbikeItem->Pose.Orientation.y - motorbike->MomentumAngle) / 64;
		}
		else if (motorbike->Velocity > MOTORBIKE_FRICTION)
		{
			motorbike->Velocity -= MOTORBIKE_FRICTION;
			if (motorbike->Velocity < 0)
				motorbike->Velocity = 0;
		}
		else if (motorbike->Velocity < MOTORBIKE_FRICTION)
		{
			motorbike->Velocity += MOTORBIKE_FRICTION;
			if (motorbike->Velocity > 0)
				motorbike->Velocity = 0;
		}
		else
			motorbike->Velocity = 0;

		if (laraItem->Animation.ActiveState == BIKE_MOVING_BACK)
		{
			int currentFrame = laraItem->Animation.FrameNumber;
			int frameBase = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

			if (currentFrame >= frameBase + 24 &&
				currentFrame <= frameBase + 29)
			{
				if (motorbike->Velocity > -MOTORBIKE_BIG_SLOWDOWN)
					motorbike->Velocity -= MOTORBIKE_SLOWDOWN2;
			}
		}

		motorbikeItem->Animation.Velocity = motorbike->Velocity / 256;

		if (motorbike->EngineRevs > MOTORBIKE_ACCEL_MAX)
			motorbike->EngineRevs = (GetRandomControl() & 0x1FF) + 0xBF00;

		int newPitch = motorbike->Velocity;
		if (motorbike->Velocity < 0)
			newPitch /= 2;

		motorbike->EngineRevs += (abs(newPitch) - 0x2000 - motorbike->EngineRevs) / 8;
		*pitch = motorbike->EngineRevs;

	}
	else
	{
		if (motorbike->EngineRevs < 0xFFFF)
			motorbike->EngineRevs += (motorbike->EngineRevs - 0xFFFF) / 8;

		*pitch = motorbike->EngineRevs;
	}

	return 0;
}

// TODO: Unused function?
void SetLaraOnMotorBike(ItemInfo* motorbikeItem, ItemInfo* laraItem)
{
	auto* motorbike = GetMotorbikeInfo(motorbikeItem);
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + MOTORBIKE_ANIM_IDLE;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	laraItem->Animation.ActiveState = BIKE_IDLE;
	laraItem->Animation.TargetState = BIKE_IDLE;
	laraItem->Animation.Airborne = false;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->HitDirection = -1;

	motorbikeItem->Animation.AnimNumber = laraItem->Animation.AnimNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
	motorbikeItem->Animation.FrameNumber = laraItem->Animation.FrameNumber + (g_Level.Anims[ID_MOTORBIKE].frameBase - g_Level.Anims[ID_MOTORBIKE_LARA_ANIMS].frameBase);
	motorbikeItem->HitPoints = 1;
	motorbikeItem->Flags = short(IFLAG_KILLED); // hmm... maybe wrong name (it can be IFLAG_CODEBITS) ?
	motorbike->Revs = 0;
}

bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* motorbikeItem = &g_Level.Items[Lara.Vehicle];
	auto* motorbike = GetMotorbikeInfo(motorbikeItem);

	int collide = MotorBikeDynamics(motorbikeItem);
	int drive = -1;

	auto oldPos = motorbikeItem->Pose.Position;

	Vector3Int frontLeft, frontRight, frontMiddle;
	int heightFrontLeft = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, -MOTORBIKE_SIDE, &frontLeft);
	int heightFrontRight = TestMotorbikeHeight(motorbikeItem, MOTORBIKE_FRONT, CLICK(0.5f), &frontRight);
	int heightFrontMiddle = TestMotorbikeHeight(motorbikeItem, -MOTORBIKE_FRONT, 0, &frontMiddle);

	auto probe = GetCollision(motorbikeItem);

	TestTriggers(motorbikeItem, true);
	TestTriggers(motorbikeItem, false);

	bool isDead;
	if (laraItem->HitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		isDead = true;
	}
	else
		isDead = false;

	int pitch = 0;

	if (motorbike->Flags)
		collide = 0;
	else
	{
		DrawMotorbikeLight(motorbikeItem);
		if (laraItem->Animation.ActiveState < BIKE_ENTER ||
			laraItem->Animation.ActiveState > BIKE_EXIT)
		{
			drive = MotorbikeUserControl(motorbikeItem, laraItem, probe.Position.Floor, &pitch);
		}
		else
		{
			drive = -1;
			collide = 0;
		}
	}

	if (motorbike->Velocity > 0 || motorbike->Revs)
	{
		motorbike->Pitch = pitch;

	if (motorbike->Pitch < -MOTORBIKE_PITCH_SLOWDOWN) 
		motorbike->Pitch = -MOTORBIKE_PITCH_SLOWDOWN; 
	else 
	if (motorbike->Pitch > MOTORBIKE_BITCH_MAX)
		motorbike->Pitch = MOTORBIKE_BITCH_MAX;

		SoundEffect(SFX_TR4_VEHICLE_MOTORBIKE_MOVING, &motorbikeItem->Pose, SoundEnvironment::Land, 0.7f + motorbike->Pitch / 24756.0f);
	}
	else
	{
		if (drive != -1)
		{
			SoundEffect(SFX_TR4_VEHICLE_MOTORBIKE_IDLE, &motorbikeItem->Pose);
			SoundEffect(SFX_TR4_VEHICLE_MOTORBIKE_MOVING, &motorbikeItem->Pose, SoundEnvironment::Land, 0.7f + motorbike->Pitch / 24756.0f, 0.5f);
		}

		motorbike->Pitch = 0;
	}

	if (motorbike->Velocity < MOTORBIKE_ACCEL_1)
		DrawMotorBikeSmoke(motorbikeItem);

	motorbikeItem->Floor = probe.Position.Floor;

	int rotation = motorbike->Velocity / 4;
	motorbike->LeftWheelRotation -= rotation;
	motorbike->RightWheelsRotation -= rotation;

	int newY = motorbikeItem->Pose.Position.y;
	motorbikeItem->Animation.VerticalVelocity = DoMotorBikeDynamics(probe.Position.Floor, motorbikeItem->Animation.VerticalVelocity, &motorbikeItem->Pose.Position.y, 0);
	motorbike->Velocity = DoVehicleWaterMovement(motorbikeItem, laraItem, motorbike->Velocity, MOTORBIKE_RADIUS, &motorbike->TurnRate);

	int r1 = (frontRight.y + frontLeft.y) / 2;
	int r2 = (frontRight.y + frontLeft.y) / 2;
	short xRot = 0;
	short zRot = 0;
	if (frontMiddle.y >= heightFrontMiddle)
	{
		if (r1 >= ((heightFrontLeft + heightFrontRight) / 2))
		{
			xRot = phd_atan(1000, heightFrontMiddle - r1);
			zRot = phd_atan(MOTORBIKE_SIDE, r2 - frontLeft.y);
		}
		else
		{
			xRot = phd_atan(MOTORBIKE_FRONT, heightFrontMiddle - motorbikeItem->Pose.Position.y);
			zRot = phd_atan(MOTORBIKE_SIDE, r2 - frontLeft.y);
		}
	}
	else if (r1 >= ((heightFrontLeft + heightFrontRight) / 2))
	{
		xRot = phd_atan(MOTORBIKE_FRONT, motorbikeItem->Pose.Position.y - r1);
		zRot = phd_atan(MOTORBIKE_SIDE, r2 - frontLeft.y);
	}
	else
	{
		xRot = phd_atan(125, newY - motorbikeItem->Pose.Position.y);
		zRot = phd_atan(MOTORBIKE_SIDE, r2 - frontLeft.y);
	}

	motorbikeItem->Pose.Orientation.x += (xRot - motorbikeItem->Pose.Orientation.x) / 4;
	motorbikeItem->Pose.Orientation.z += (zRot - motorbikeItem->Pose.Orientation.z) / 4;

	if (motorbike->Flags >= 0)
	{
		if (probe.RoomNumber != motorbikeItem->RoomNumber)
		{
			ItemNewRoom(Lara.Vehicle, probe.RoomNumber);
			ItemNewRoom(Lara.ItemNumber, probe.RoomNumber);
		}

		laraItem->Pose = motorbikeItem->Pose;

		AnimateMotorbike(motorbikeItem, collide, isDead);
		AnimateItem(laraItem);

		motorbikeItem->Animation.AnimNumber = laraItem->Animation.AnimNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
		motorbikeItem->Animation.FrameNumber = laraItem->Animation.FrameNumber + (g_Level.Anims[motorbikeItem->Animation.AnimNumber].frameBase - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);

        if (motorbike->Flags & MOTORBIKE_FLAG_FALLING)
        {
            if (motorbikeItem->Pose.Position.y == motorbikeItem->Floor)
            {
                ExplodeVehicle(laraItem, motorbikeItem);
                return 0;
            }
        }
    }

	if (laraItem->Animation.ActiveState == BIKE_EXIT)
	{
		ExhaustStart = false;
		MotorBikeCheckGetOff();
		return true;
	}

	MotorBikeCheckGetOff();
	return true;
}
