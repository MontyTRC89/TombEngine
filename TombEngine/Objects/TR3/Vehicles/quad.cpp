#include "framework.h"
#include "Objects/TR3/Vehicles/quad.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Objects/TR3/Vehicles/quad_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"
#include "Specific/prng.h"
#include "Game/effects/simple_particle.h"

using namespace TEN::Input;
using namespace TEN::Math::Random;

namespace TEN::Entities::Vehicles
{
	#define MAX_VELOCITY				0xA000
	#define MIN_DRIFT_VELOCITY			0x3000
	#define BRAKE						0x0280
	#define REVERSE_ACCELERATION		-0x0300
	#define MAX_BACK					-0x3000
	#define MAX_REVS					0xa000
	#define TERMINAL_VERTICAL_VELOCITY	240
	#define QUAD_SLIP					100
	#define QUAD_SLIP_SIDE				50

	#define QUAD_FRONT	550
	#define QUAD_BACK  -550
	#define QUAD_SIDE	260
	#define QUAD_RADIUS	500
	#define QUAD_HEIGHT	512

	// TODO
	#define QUAD_HIT_LEFT  11
	#define QUAD_HIT_RIGHT 12
	#define QUAD_HIT_FRONT 13
	#define QUAD_HIT_BACK  14

	#define DAMAGE_START  140
	#define DAMAGE_LENGTH 14

	#define DISMOUNT_DISTANCE 385	// Precise root bone offset derived from final frame of animation.

	#define QUAD_UNDO_TURN			ANGLE(2.0f)
	#define QUAD_TURN_RATE			(ANGLE(0.5f) + QUAD_UNDO_TURN)
	#define QUAD_TURN_MAX			ANGLE(5.0f)
	#define QUAD_DRIFT_TURN_RATE	(ANGLE(0.75f) + QUAD_UNDO_TURN)
	#define QUAD_DRIFT_TURN_MAX		ANGLE(8.0f)

	#define MIN_MOMENTUM_TURN ANGLE(3.0f)
	#define MAX_MOMENTUM_TURN ANGLE(1.5f)
	#define QUAD_MAX_MOM_TURN ANGLE(150.0f)

	#define QUAD_MAX_HEIGHT CLICK(1)
	#define QUAD_MIN_BOUNCE ((MAX_VELOCITY / 2) / CLICK(1))

	// TODO: Common controls for all vehicles + unique settings page to set them. @Sezz 2021.11.14
	#define QUAD_IN_ACCELERATE	IN_ACTION
	#define QUAD_IN_BRAKE		IN_JUMP
	#define QUAD_IN_DRIFT		(IN_CROUCH | IN_SPRINT)
	#define QUAD_IN_DISMOUNT	IN_ROLL
	#define QUAD_IN_LEFT		IN_LEFT
	#define QUAD_IN_RIGHT		IN_RIGHT

	enum QuadState
	{
		QUAD_STATE_DRIVE = 1,
		QUAD_STATE_TURN_LEFT = 2,
		QUAD_STATE_SLOW = 5,
		QUAD_STATE_BRAKE = 6,
		QUAD_STATE_BIKE_DEATH = 7,
		QUAD_STATE_FALL = 8,
		QUAD_STATE_MOUNT_RIGHT = 9,
		QUAD_STATE_DISMOUNT_RIGHT = 10,
		QUAD_STATE_HIT_BACK = 11,
		QUAD_STATE_HIT_FRONT = 12,
		QUAD_STATE_HIT_LEFT = 13,
		QUAD_STATE_HIT_RIGHT = 14,
		QUAD_STATE_IDLE = 15,
		QUAD_STATE_LAND = 17,
		QUAD_STATE_STOP_SLOWLY = 18,
		QUAD_STATE_FALL_DEATH = 19,
		QUAD_STATE_FALL_OFF = 20,
		QUAD_STATE_WHEELIE = 21,	// Unused.
		QUAD_STATE_TURN_RIGHT = 22,
		QUAD_STATE_MOUNT_LEFT = 23,
		QUAD_STATE_DISMOUNT_LEFT = 24,
	};

	enum QuadAnim
	{
		QUAD_ANIM_IDLE_DEATH = 0,
		QUAD_ANIM_UNK_1 = 1,
		QUAD_ANIM_DRIVE_BACK = 2,
		QUAD_ANIM_TURN_LEFT_START = 3,
		QUAD_ANIM_TURN_LEFT_CONTINUE = 4,
		QUAD_ANIM_TURN_LEFT_END = 5,
		QUAD_ANIM_LEAP_START = 6,
		QUAD_ANIM_LEAP_CONTINUE = 7,
		QUAD_ANIM_LEAP_END = 8,
		QUAD_ANIM_MOUNT_RIGHT = 9,
		QUAD_ANIM_DISMOUNT_RIGHT = 10,
		QUAD_ANIM_HIT_FRONT = 11,
		QUAD_ANIM_HIT_BACK = 12,
		QUAD_ANIM_HIT_RIGHT = 13,
		QUAD_ANIM_HIT_LEFT = 14,
		QUAD_ANIM_UNK_2 = 15,
		QUAD_ANIM_UNK_3 = 16,
		QUAD_ANIM_UNK_4 = 17,
		QUAD_ANIM_IDLE = 18,
		QUAD_ANIM_FALL_OFF_DEATH = 19,
		QUAD_ANIM_TURN_RIGHT_START = 20,
		QUAD_ANIM_TURN_RIGHT_CONTINUE = 21,
		QUAD_ANIM_TURN_RIGHT_END = 22,
		QUAD_ANIM_MOUNT_LEFT = 23,
		QUAD_ANIM_DISMOUNT_LEFT = 24,
		QUAD_ANIM_LEAP_START2 = 25,
		QUAD_ANIM_LEAP_CONTINUE2 = 26,
		QUAD_ANIM_LEAP_END2 = 27,
		QUAD_ANIM_LEAP_TO_FREEFALL = 28
	};

	enum QuadEffectPosition
	{
		EXHAUST_LEFT = 0,
		EXHAUST_RIGHT = 1,
		FRONT_LEFT_TYRE = 2,
		FRONT_RIGHT_TYRE = 3,
		BACK_LEFT_TYRE = 4,
		BACK_RIGHT_TYRE = 5
	};

	enum QuadFlags
	{
		QUAD_FLAG_DEAD = 0x80,
		QUAD_FLAG_IS_FALLING = 0x40
	};

	BITE_INFO quadEffectsPositions[6] =
	{
		{ -56, -32, -380, 0	},
		{ 56, -32, -380, 0 },
		{ -8, 180, -48, 3 },
		{ 8, 180, -48, 4 },
		{ 90, 180, -32, 6 },
		{ -90, 180, -32, 7 }
	};

	void InitialiseQuadBike(short itemNumber)
	{
		auto* quadItem = &g_Level.Items[itemNumber];
		quadItem->Data = QuadInfo();
		auto* quad = (QuadInfo*)quadItem->Data;

		quad->TurnRate = 0;
		quad->MomentumAngle = quadItem->Pose.Orientation.y;
		quad->ExtraRotation = 0;
		quad->Velocity = 0;
		quad->LeftVerticalVelocity = 0;
		quad->RightVerticalVelocity = 0;
		quad->Pitch = 0;
		quad->Flags = 0;
	}

	static int CanQuadbikeGetOff(int direction)
	{
		auto* item = &g_Level.Items[Lara.Vehicle];
		short angle;

		if (direction < 0)
			angle = item->Pose.Orientation.y - ANGLE(90.0f);
		else
			angle = item->Pose.Orientation.y + ANGLE(90.0f);

		int x = item->Pose.Position.x + CLICK(2) * phd_sin(angle);
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z + CLICK(2) * phd_cos(angle);

		auto collResult = GetCollision(x, y, z, item->RoomNumber);

		if (collResult.Position.FloorSlope ||
			collResult.Position.Floor == NO_HEIGHT)
		{
			return false;
		}

		if (abs(collResult.Position.Floor - item->Pose.Position.y) > CLICK(2))
			return false;

		if ((collResult.Position.Ceiling - item->Pose.Position.y) > -LARA_HEIGHT ||
			(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	static bool QuadCheckGetOff(ItemInfo* laraItem, ItemInfo* quadItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quad = (QuadInfo*)quadItem->Data;

		if (lara->Vehicle == NO_ITEM)
			return true;

		if ((laraItem->Animation.ActiveState == QUAD_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == QUAD_STATE_DISMOUNT_LEFT) &&
			TestLastFrame(laraItem))
		{
			if (laraItem->Animation.ActiveState == QUAD_STATE_DISMOUNT_LEFT)
				laraItem->Pose.Orientation.y += ANGLE(90.0f);
			else
				laraItem->Pose.Orientation.y -= ANGLE(90.0f);

			SetAnimation(laraItem, LA_STAND_IDLE);
			TranslateItem(laraItem, laraItem->Pose.Orientation.y, -DISMOUNT_DISTANCE);
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.z = 0;
			lara->Vehicle = NO_ITEM;
			lara->Control.HandStatus = HandStatus::Free;

			if (laraItem->Animation.ActiveState == QUAD_STATE_FALL_OFF)
			{
				auto pos = Vector3Int();

				SetAnimation(laraItem, LA_FREEFALL);
				GetJointAbsPosition(laraItem, &pos, LM_HIPS);

				laraItem->Pose.Position = pos;
				laraItem->Animation.Airborne = true;
				laraItem->Animation.VerticalVelocity = quadItem->Animation.VerticalVelocity;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				laraItem->HitPoints = 0;
				lara->Control.HandStatus = HandStatus::Free;
				quadItem->Flags |= ONESHOT;

				return false;
			}
			else if (laraItem->Animation.ActiveState == QUAD_STATE_FALL_DEATH)
			{
				laraItem->Animation.TargetState = LS_DEATH;
				laraItem->Animation.Velocity = 0;
				laraItem->Animation.VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
				quad->Flags |= QUAD_FLAG_DEAD;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	static int GetOnQuadBike(ItemInfo* laraItem, ItemInfo* quadItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (!(TrInput & IN_ACTION) ||
			laraItem->Animation.Airborne ||
			lara->Control.HandStatus != HandStatus::Free ||
			quadItem->Flags & ONESHOT ||
			abs(quadItem->Pose.Position.y - laraItem->Pose.Position.y) > CLICK(1))
		{
			return false;
		}

		auto dist = pow(laraItem->Pose.Position.x - quadItem->Pose.Position.x, 2) + pow(laraItem->Pose.Position.z - quadItem->Pose.Position.z, 2);
		if (dist > 170000)
			return false;

		auto probe = GetCollision(quadItem);
		if (probe.Position.Floor < -32000)
			return false;
		else
		{
			short angle = phd_atan(quadItem->Pose.Position.z - laraItem->Pose.Position.z, quadItem->Pose.Position.x - laraItem->Pose.Position.x);
			angle -= quadItem->Pose.Orientation.y;

			if ((angle > -ANGLE(45.0f)) && (angle < ANGLE(135.0f)))
			{
				short tempAngle = laraItem->Pose.Orientation.y - quadItem->Pose.Orientation.y;
				if (tempAngle > ANGLE(45.0f) && tempAngle < ANGLE(135.0f))
					return true;
				else
					return false;
			}
			else
			{
				short tempAngle = laraItem->Pose.Orientation.y - quadItem->Pose.Orientation.y;
				if (tempAngle > ANGLE(225.0f) && tempAngle < ANGLE(315.0f))
					return true;
				else
					return false;
			}
		}

		return true;
	}

	static int GetQuadCollisionAnim(ItemInfo* quadItem, Vector3Int* p)
	{
		p->x = quadItem->Pose.Position.x - p->x;
		p->z = quadItem->Pose.Position.z - p->z;

		if (p->x || p->z)
		{
			float c = phd_cos(quadItem->Pose.Orientation.y);
			float s = phd_sin(quadItem->Pose.Orientation.y);
			int front = p->z * c + p->x * s;
			int side = -p->z * s + p->x * c;

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return QUAD_HIT_BACK;
				else
					return QUAD_HIT_FRONT;
			}
			else
			{
				if (side > 0)
					return QUAD_HIT_LEFT;
				else
					return QUAD_HIT_RIGHT;
			}
		}

		return 0;
	}

	static int TestQuadHeight(ItemInfo* quadItem, int dz, int dx, Vector3Int* pos)
	{
		pos->y = quadItem->Pose.Position.y - dz * phd_sin(quadItem->Pose.Orientation.x) + dx * phd_sin(quadItem->Pose.Orientation.z);

		float c = phd_cos(quadItem->Pose.Orientation.y);
		float s = phd_sin(quadItem->Pose.Orientation.y);

		pos->z = quadItem->Pose.Position.z + dz * c - dx * s;
		pos->x = quadItem->Pose.Position.x + dz * s + dx * c;

		auto probe = GetCollision(pos->x, pos->y, pos->z, quadItem->RoomNumber);
		if (probe.Position.Ceiling > pos->y ||
			probe.Position.Ceiling == NO_HEIGHT)
		{
			return NO_HEIGHT;
		}

		return probe.Position.Floor;
	}

	static int DoQuadShift(ItemInfo* quadItem, Vector3Int* pos, Vector3Int* old)
	{
		CollisionResult probe;
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
				quadItem->Pose.Position.z += (old->z - pos->z);
				quadItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > oldZ)
			{
				quadItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - quadItem->Pose.Position.x);
			}
			else
			{
				quadItem->Pose.Position.z += SECTOR(1) - shiftZ;
				return (quadItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == oldZ)
		{
			if (x > oldX)
			{
				quadItem->Pose.Position.x -= shiftX + 1;
				return (quadItem->Pose.Position.z - pos->z);
			}
			else
			{
				quadItem->Pose.Position.x += SECTOR(1) - shiftX;
				return (pos->z - quadItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			probe = GetCollision(old->x, pos->y, pos->z, quadItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = SECTOR(1) - shiftZ;
			}

			probe = GetCollision(pos->x, pos->y, old->z, quadItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = SECTOR(1) - shiftX;
			}

			if (x && z)
			{
				quadItem->Pose.Position.z += z;
				quadItem->Pose.Position.x += x;
			}
			else if (z)
			{
				quadItem->Pose.Position.z += z;

				if (z > 0)
					return (quadItem->Pose.Position.x - pos->x);
				else
					return (pos->x - quadItem->Pose.Position.x);
			}
			else if (x)
			{
				quadItem->Pose.Position.x += x;

				if (x > 0)
					return (pos->z - quadItem->Pose.Position.z);
				else
					return (quadItem->Pose.Position.z - pos->z);
			}
			else
			{
				quadItem->Pose.Position.z += (old->z - pos->z);
				quadItem->Pose.Position.x += (old->x - pos->x);
			}
		}

		return 0;
	}

	static int DoQuadDynamics(int height, int verticalVelocity, int* y)
	{
		if (height > *y)
		{
			*y += verticalVelocity;
			if (*y > height - QUAD_MIN_BOUNCE)
			{
				*y = height;
				verticalVelocity = 0;
			}
			else
				verticalVelocity += 6;
		}
		else
		{
			int kick = (height - *y) * 4;
			if (kick < -80)
				kick = -80;

			verticalVelocity += ((kick - verticalVelocity) / 8);

			if (*y > height)
				*y = height;
		}

		return verticalVelocity;
	}

	static int QuadDynamics(ItemInfo* laraItem, ItemInfo* quadItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quad = (QuadInfo*)quadItem->Data;

		quad->NoDismount = false;

		Vector3Int oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight;
		int holdFrontLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &oldFrontLeft);
		int holdFrontRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &oldFrontRight);
		int holdBottomLeft = TestQuadHeight(quadItem, -QUAD_FRONT, -QUAD_SIDE, &oldBottomLeft);
		int holdBottomRight = TestQuadHeight(quadItem, -QUAD_FRONT, QUAD_SIDE, &oldBottomRight);

		Vector3Int mtlOld, mtrOld, mmlOld, mmrOld;
		int hmml_old = TestQuadHeight(quadItem, 0, -QUAD_SIDE, &mmlOld);
		int hmmr_old = TestQuadHeight(quadItem, 0, QUAD_SIDE, &mmrOld);
		int hmtl_old = TestQuadHeight(quadItem, QUAD_FRONT / 2, -QUAD_SIDE, &mtlOld);
		int hmtr_old = TestQuadHeight(quadItem, QUAD_FRONT / 2, QUAD_SIDE, &mtrOld);

		Vector3Int moldBottomLeft, moldBottomRight;
		int hmoldBottomLeft = TestQuadHeight(quadItem, -QUAD_FRONT / 2, -QUAD_SIDE, &moldBottomLeft);
		int hmoldBottomRight = TestQuadHeight(quadItem, -QUAD_FRONT / 2, QUAD_SIDE, &moldBottomRight);

		Vector3Int old;
		old.x = quadItem->Pose.Position.x;
		old.y = quadItem->Pose.Position.y;
		old.z = quadItem->Pose.Position.z;

		if (oldBottomLeft.y > holdBottomLeft)
			oldBottomLeft.y = holdBottomLeft;

		if (oldBottomRight.y > holdBottomRight)
			oldBottomRight.y = holdBottomRight;

		if (oldFrontLeft.y > holdFrontLeft)
			oldFrontLeft.y = holdFrontLeft;

		if (oldFrontRight.y > holdFrontRight)
			oldFrontRight.y = holdFrontRight;

		if (moldBottomLeft.y > hmoldBottomLeft)
			moldBottomLeft.y = hmoldBottomLeft;

		if (moldBottomRight.y > hmoldBottomRight)
			moldBottomRight.y = hmoldBottomRight;

		if (mtlOld.y > hmtl_old)
			mtlOld.y = hmtl_old;

		if (mtrOld.y > hmtr_old)
			mtrOld.y = hmtr_old;

		if (mmlOld.y > hmml_old)
			mmlOld.y = hmml_old;

		if (mmrOld.y > hmmr_old)
			mmrOld.y = hmmr_old;

		if (quadItem->Pose.Position.y > (quadItem->Floor - CLICK(1)))
		{
			if (quad->TurnRate < -QUAD_UNDO_TURN)
				quad->TurnRate += QUAD_UNDO_TURN;
			else if (quad->TurnRate > QUAD_UNDO_TURN)
				quad->TurnRate -= QUAD_UNDO_TURN;
			else
				quad->TurnRate = 0;

			quadItem->Pose.Orientation.y += quad->TurnRate + quad->ExtraRotation;

			short momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) * 256) / MAX_VELOCITY) * quad->Velocity) / 256);
			if (!(TrInput & QUAD_IN_ACCELERATE) && quad->Velocity > 0)
				momentum += momentum / 4;

			short rot = quadItem->Pose.Orientation.y - quad->MomentumAngle;
			if (rot < -MAX_MOMENTUM_TURN)
			{
				if (rot < -QUAD_MAX_MOM_TURN)
				{
					rot = -QUAD_MAX_MOM_TURN;
					quad->MomentumAngle = quadItem->Pose.Orientation.y - rot;
				}
				else
					quad->MomentumAngle -= momentum;
			}
			else if (rot > MAX_MOMENTUM_TURN)
			{
				if (rot > QUAD_MAX_MOM_TURN)
				{
					rot = QUAD_MAX_MOM_TURN;
					quad->MomentumAngle = quadItem->Pose.Orientation.y - rot;
				}
				else
					quad->MomentumAngle += momentum;
			}
			else
				quad->MomentumAngle = quadItem->Pose.Orientation.y;
		}
		else
			quadItem->Pose.Orientation.y += quad->TurnRate + quad->ExtraRotation;

		auto probe = GetCollision(quadItem);
		int speed = 0;
		if (quadItem->Pose.Position.y >= probe.Position.Floor)
			speed = quadItem->Animation.Velocity * phd_cos(quadItem->Pose.Orientation.x);
		else
			speed = quadItem->Animation.Velocity;

		TranslateItem(quadItem, quad->MomentumAngle, speed);

		int slip = QUAD_SLIP * phd_sin(quadItem->Pose.Orientation.x);
		if (abs(slip) > QUAD_SLIP / 2)
		{
			if (slip > 0)
				slip -= 10;
			else
				slip += 10;
			quadItem->Pose.Position.z -= slip * phd_cos(quadItem->Pose.Orientation.y);
			quadItem->Pose.Position.x -= slip * phd_sin(quadItem->Pose.Orientation.y);
		}

		slip = QUAD_SLIP_SIDE * phd_sin(quadItem->Pose.Orientation.z);
		if (abs(slip) > QUAD_SLIP_SIDE / 2)
		{
			quadItem->Pose.Position.z -= slip * phd_sin(quadItem->Pose.Orientation.y);
			quadItem->Pose.Position.x += slip * phd_cos(quadItem->Pose.Orientation.y);
		}

		Vector3Int moved;
		moved.x = quadItem->Pose.Position.x;
		moved.z = quadItem->Pose.Position.z;

		if (!(quadItem->Flags & ONESHOT))
			DoVehicleCollision(quadItem, QUAD_RADIUS);

		short rot = 0;
		short rotAdd = 0;

		Vector3Int fl;
		int heightFrontLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &fl);
		if (heightFrontLeft < (oldFrontLeft.y - CLICK(1)))
			rot = DoQuadShift(quadItem, &fl, &oldFrontLeft);

		Vector3Int mtl;
		int hmtl = TestQuadHeight(quadItem, QUAD_FRONT / 2, -QUAD_SIDE, &mtl);
		if (hmtl < (mtlOld.y - CLICK(1)))
			DoQuadShift(quadItem, &mtl, &mtlOld);

		Vector3Int mml;
		int hmml = TestQuadHeight(quadItem, 0, -QUAD_SIDE, &mml);
		if (hmml < (mmlOld.y - CLICK(1)))
			DoQuadShift(quadItem, &mml, &mmlOld);

		Vector3Int mbl;
		int hmbl = TestQuadHeight(quadItem, -QUAD_FRONT / 2, -QUAD_SIDE, &mbl);
		if (hmbl < (moldBottomLeft.y - CLICK(1)))
			DoQuadShift(quadItem, &mbl, &moldBottomLeft);

		Vector3Int bl;
		int heightBackLeft = TestQuadHeight(quadItem, -QUAD_FRONT, -QUAD_SIDE, &bl);
		if (heightBackLeft < (oldBottomLeft.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadItem, &bl, &oldBottomLeft);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		Vector3Int fr;
		int heightFrontRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &fr);
		if (heightFrontRight < (oldFrontRight.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadItem, &fr, &oldFrontRight);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		Vector3Int mtr;
		int hmtr = TestQuadHeight(quadItem, QUAD_FRONT / 2, QUAD_SIDE, &mtr);
		if (hmtr < (mtrOld.y - CLICK(1)))
			DoQuadShift(quadItem, &mtr, &mtrOld);

		Vector3Int mmr;
		int hmmr = TestQuadHeight(quadItem, 0, QUAD_SIDE, &mmr);
		if (hmmr < (mmrOld.y - CLICK(1)))
			DoQuadShift(quadItem, &mmr, &mmrOld);

		Vector3Int mbr;
		int hmbr = TestQuadHeight(quadItem, -QUAD_FRONT / 2, QUAD_SIDE, &mbr);
		if (hmbr < (moldBottomRight.y - CLICK(1)))
			DoQuadShift(quadItem, &mbr, &moldBottomRight);

		Vector3Int br;
		int heightBackRight = TestQuadHeight(quadItem, -QUAD_FRONT, QUAD_SIDE, &br);
		if (heightBackRight < (oldBottomRight.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadItem, &br, &oldBottomRight);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		probe = GetCollision(quadItem);
		if (probe.Position.Floor < quadItem->Pose.Position.y - CLICK(1))
			DoQuadShift(quadItem, (Vector3Int*)&quadItem->Pose, &old);

		quad->ExtraRotation = rot;

		int collide = GetQuadCollisionAnim(quadItem, &moved);

		int newVelocity = 0;
		if (collide)
		{
			newVelocity = (quadItem->Pose.Position.z - old.z) * phd_cos(quad->MomentumAngle) + (quadItem->Pose.Position.x - old.x) * phd_sin(quad->MomentumAngle);
			newVelocity *= 256;

			if (&g_Level.Items[lara->Vehicle] == quadItem &&
				quad->Velocity == MAX_VELOCITY &&
				newVelocity < (quad->Velocity - 10))
			{
				DoDamage(laraItem, (quad->Velocity - newVelocity) / 128);
			}

			if (quad->Velocity > 0 && newVelocity < quad->Velocity)
				quad->Velocity = (newVelocity < 0) ? 0 : newVelocity;

			else if (quad->Velocity < 0 && newVelocity > quad->Velocity)
				quad->Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (quad->Velocity < MAX_BACK)
				quad->Velocity = MAX_BACK;
		}

		return collide;
	}

	static void AnimateQuadBike(ItemInfo* laraItem, ItemInfo* quadItem, int collide, bool dead)
	{
		auto* quad = (QuadInfo*)quadItem->Data;

		if (quadItem->Pose.Position.y != quadItem->Floor &&
			laraItem->Animation.ActiveState != QUAD_STATE_FALL &&
			laraItem->Animation.ActiveState != QUAD_STATE_LAND &&
			laraItem->Animation.ActiveState != QUAD_STATE_FALL_OFF &&
			!dead)
		{
			if (quad->Velocity < 0)
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START2;

			laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
			laraItem->Animation.ActiveState = QUAD_STATE_FALL;
			laraItem->Animation.TargetState = QUAD_STATE_FALL;
		}
		else if (collide &&
			laraItem->Animation.ActiveState != QUAD_STATE_HIT_FRONT &&
			laraItem->Animation.ActiveState != QUAD_STATE_HIT_BACK &&
			laraItem->Animation.ActiveState != QUAD_STATE_HIT_LEFT &&
			laraItem->Animation.ActiveState != QUAD_STATE_HIT_RIGHT &&
			laraItem->Animation.ActiveState != QUAD_STATE_FALL_OFF &&
			quad->Velocity > (MAX_VELOCITY / 3) &&
			!dead)
		{
			if (collide == QUAD_HIT_FRONT)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_BACK;
				laraItem->Animation.ActiveState = QUAD_STATE_HIT_FRONT;
				laraItem->Animation.TargetState = QUAD_STATE_HIT_FRONT;
			}
			else if (collide == QUAD_HIT_BACK)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_FRONT;
				laraItem->Animation.ActiveState = QUAD_STATE_HIT_BACK;
				laraItem->Animation.TargetState = QUAD_STATE_HIT_BACK;
			}
			else if (collide == QUAD_HIT_LEFT)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_RIGHT;
				laraItem->Animation.ActiveState = QUAD_STATE_HIT_LEFT;
				laraItem->Animation.TargetState = QUAD_STATE_HIT_LEFT;
			}
			else
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_LEFT;
				laraItem->Animation.ActiveState = QUAD_STATE_HIT_RIGHT;
				laraItem->Animation.TargetState = QUAD_STATE_HIT_RIGHT;
			}

			laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
			SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_FRONT_IMPACT, &quadItem->Pose);
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case QUAD_STATE_IDLE:
				if (dead)
					laraItem->Animation.TargetState = QUAD_STATE_BIKE_DEATH;
				else if (TrInput & QUAD_IN_DISMOUNT &&
					quad->Velocity == 0 &&
					!quad->NoDismount)
				{
					if (TrInput & QUAD_IN_LEFT && CanQuadbikeGetOff(-1))
						laraItem->Animation.TargetState = QUAD_STATE_DISMOUNT_LEFT;
					else if (TrInput & QUAD_IN_RIGHT && CanQuadbikeGetOff(1))
						laraItem->Animation.TargetState = QUAD_STATE_DISMOUNT_RIGHT;
				}
				else if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
					laraItem->Animation.TargetState = QUAD_STATE_DRIVE;

				break;

			case QUAD_STATE_DRIVE:
				if (dead)
				{
					if (quad->Velocity > (MAX_VELOCITY / 2))
						laraItem->Animation.TargetState = QUAD_STATE_FALL_DEATH;
					else
						laraItem->Animation.TargetState = QUAD_STATE_BIKE_DEATH;
				}
				else if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
					(quad->Velocity / 256) == 0)
				{
					laraItem->Animation.TargetState = QUAD_STATE_IDLE;
				}
				else if (TrInput & QUAD_IN_LEFT &&
					!quad->DriftStarting)
				{
					laraItem->Animation.TargetState = QUAD_STATE_TURN_LEFT;
				}
				else if (TrInput & QUAD_IN_RIGHT &&
					!quad->DriftStarting)
				{
					laraItem->Animation.TargetState = QUAD_STATE_TURN_RIGHT;
				}
				else if (TrInput & QUAD_IN_BRAKE)
				{
					if (quad->Velocity > (MAX_VELOCITY / 3 * 2))
						laraItem->Animation.TargetState = QUAD_STATE_BRAKE;
					else
						laraItem->Animation.TargetState = QUAD_STATE_SLOW;
				}

				break;

			case QUAD_STATE_BRAKE:
			case QUAD_STATE_SLOW:
			case QUAD_STATE_STOP_SLOWLY:
				if ((quad->Velocity / 256) == 0)
					laraItem->Animation.TargetState = QUAD_STATE_IDLE;
				else if (TrInput & QUAD_IN_LEFT)
					laraItem->Animation.TargetState = QUAD_STATE_TURN_LEFT;
				else if (TrInput & QUAD_IN_RIGHT)
					laraItem->Animation.TargetState = QUAD_STATE_TURN_RIGHT;

				break;

			case QUAD_STATE_TURN_LEFT:
				if ((quad->Velocity / 256) == 0)
					laraItem->Animation.TargetState = QUAD_STATE_IDLE;
				else if (TrInput & QUAD_IN_RIGHT)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_RIGHT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QUAD_STATE_TURN_RIGHT;
					laraItem->Animation.TargetState = QUAD_STATE_TURN_RIGHT;
				}
				else if (!(TrInput & QUAD_IN_LEFT))
					laraItem->Animation.TargetState = QUAD_STATE_DRIVE;

				break;

			case QUAD_STATE_TURN_RIGHT:
				if ((quad->Velocity / 256) == 0)
					laraItem->Animation.TargetState = QUAD_STATE_IDLE;
				else if (TrInput & QUAD_IN_LEFT)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_LEFT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QUAD_STATE_TURN_LEFT;
					laraItem->Animation.TargetState = QUAD_STATE_TURN_LEFT;
				}
				else if (!(TrInput & QUAD_IN_RIGHT))
					laraItem->Animation.TargetState = QUAD_STATE_DRIVE;

				break;

			case QUAD_STATE_FALL:
				if (quadItem->Pose.Position.y == quadItem->Floor)
					laraItem->Animation.TargetState = QUAD_STATE_LAND;
				else if (quadItem->Animation.VerticalVelocity > TERMINAL_VERTICAL_VELOCITY)
					quad->Flags |= QUAD_FLAG_IS_FALLING;

				break;

			case QUAD_STATE_FALL_OFF:
				break;

			case QUAD_STATE_HIT_FRONT:
			case QUAD_STATE_HIT_BACK:
			case QUAD_STATE_HIT_LEFT:
			case QUAD_STATE_HIT_RIGHT:
				if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
					laraItem->Animation.TargetState = QUAD_STATE_DRIVE;

				break;
			}
		}
	}

	static int QuadUserControl(ItemInfo* quadItem, int height, int* pitch)
	{
		auto* quad = (QuadInfo*)quadItem->Data;

		bool drive = false; // Never changes?

		if (!(TrInput & QUAD_IN_DRIFT) &&
			!quad->Velocity && !quad->CanStartDrift)
		{
			quad->CanStartDrift = true;
		}
		else if (quad->Velocity)
			quad->CanStartDrift = false;

		if (!(TrInput & QUAD_IN_DRIFT))
			quad->DriftStarting = false;

		if (!quad->DriftStarting)
		{
			if (quad->Revs > 0x10)
			{
				quad->Velocity += (quad->Revs / 16);
				quad->Revs -= (quad->Revs / 8);
			}
			else
				quad->Revs = 0;
		}

		if (quadItem->Pose.Position.y >= (height - CLICK(1)))
		{
			if (TrInput & IN_LOOK && !quad->Velocity)
				LookUpDown(LaraItem);

			// Driving forward.
			if (quad->Velocity > 0)
			{
				if (TrInput & QUAD_IN_DRIFT &&
					!quad->DriftStarting &&
					quad->Velocity > MIN_DRIFT_VELOCITY)
				{
					if (TrInput & QUAD_IN_LEFT)
					{
						quad->TurnRate -= QUAD_DRIFT_TURN_RATE;
						if (quad->TurnRate < -QUAD_DRIFT_TURN_MAX)
							quad->TurnRate = -QUAD_DRIFT_TURN_MAX;
					}
					else if (TrInput & QUAD_IN_RIGHT)
					{
						quad->TurnRate += QUAD_DRIFT_TURN_RATE;
						if (quad->TurnRate > QUAD_DRIFT_TURN_MAX)
							quad->TurnRate = QUAD_DRIFT_TURN_MAX;
					}
				}
				else
				{
					if (TrInput & QUAD_IN_LEFT)
					{
						quad->TurnRate -= QUAD_TURN_RATE;
						if (quad->TurnRate < -QUAD_TURN_MAX)
							quad->TurnRate = -QUAD_TURN_MAX;
					}
					else if (TrInput & QUAD_IN_RIGHT)
					{
						quad->TurnRate += QUAD_TURN_RATE;
						if (quad->TurnRate > QUAD_TURN_MAX)
							quad->TurnRate = QUAD_TURN_MAX;
					}
				}
			}
			// Driving back.
			else if (quad->Velocity < 0)
			{
				if (TrInput & QUAD_IN_DRIFT &&
					!quad->DriftStarting &&
					quad->Velocity < (-MIN_DRIFT_VELOCITY + 0x800))
				{
					if (TrInput & QUAD_IN_LEFT)
					{
						quad->TurnRate -= QUAD_DRIFT_TURN_RATE;
						if (quad->TurnRate < -QUAD_DRIFT_TURN_MAX)
							quad->TurnRate = -QUAD_DRIFT_TURN_MAX;
					}
					else if (TrInput & QUAD_IN_RIGHT)
					{
						quad->TurnRate += QUAD_DRIFT_TURN_RATE;
						if (quad->TurnRate > QUAD_DRIFT_TURN_MAX)
							quad->TurnRate = QUAD_DRIFT_TURN_MAX;
					}
				}
				else
				{
					if (TrInput & QUAD_IN_RIGHT)
					{
						quad->TurnRate -= QUAD_TURN_RATE;
						if (quad->TurnRate < -QUAD_TURN_MAX)
							quad->TurnRate = -QUAD_TURN_MAX;
					}
					else if (TrInput & QUAD_IN_LEFT)
					{
						quad->TurnRate += QUAD_TURN_RATE;
						if (quad->TurnRate > QUAD_TURN_MAX)
							quad->TurnRate = QUAD_TURN_MAX;
					}
				}
			}

			// Driving back / braking.
			if (TrInput & QUAD_IN_BRAKE)
			{
				if (TrInput & QUAD_IN_DRIFT &&
					(quad->CanStartDrift || quad->DriftStarting))
				{
					quad->DriftStarting = true;
					quad->Revs -= 0x200;
					if (quad->Revs < MAX_BACK)
						quad->Revs = MAX_BACK;
				}
				else if (quad->Velocity > 0)
					quad->Velocity -= BRAKE;
				else
				{
					if (quad->Velocity > MAX_BACK)
						quad->Velocity += REVERSE_ACCELERATION;
				}
			}
			else if (TrInput & QUAD_IN_ACCELERATE)
			{
				if (TrInput & QUAD_IN_DRIFT &&
					(quad->CanStartDrift || quad->DriftStarting))
				{
					quad->DriftStarting = true;
					quad->Revs += 0x200;
					if (quad->Revs >= MAX_VELOCITY)
						quad->Revs = MAX_VELOCITY;
				}
				else if (quad->Velocity < MAX_VELOCITY)
				{
					if (quad->Velocity < 0x4000)
						quad->Velocity += (8 + (0x4000 + 0x800 - quad->Velocity) / 8);
					else if (quad->Velocity < 0x7000)
						quad->Velocity += (4 + (0x7000 + 0x800 - quad->Velocity) / 16);
					else if (quad->Velocity < MAX_VELOCITY)
						quad->Velocity += (2 + (MAX_VELOCITY - quad->Velocity) / 8);
				}
				else
					quad->Velocity = MAX_VELOCITY;

				quad->Velocity -= abs(quadItem->Pose.Orientation.y - quad->MomentumAngle) / 64;
			}

			else if (quad->Velocity > 0x0100)
				quad->Velocity -= 0x0100;
			else if (quad->Velocity < -0x0100)
				quad->Velocity += 0x0100;
			else
				quad->Velocity = 0;

			if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
				quad->DriftStarting &&
				quad->Revs)
			{
				if (quad->Revs > 0x8)
					quad->Revs -= quad->Revs / 8;
				else
					quad->Revs = 0;
			}

			quadItem->Animation.Velocity = quad->Velocity / 256;

			if (quad->EngineRevs > 0x7000)
				quad->EngineRevs = -0x2000;

			int revs = 0;
			if (quad->Velocity < 0)
				revs = abs(quad->Velocity / 2);
			else if (quad->Velocity < 0x7000)
				revs = -0x2000 + (quad->Velocity * (0x6800 - -0x2000)) / 0x7000;
			else if (quad->Velocity <= MAX_VELOCITY)
				revs = -0x2800 + ((quad->Velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000);

			revs += abs(quad->Revs);
			quad->EngineRevs += (revs - quad->EngineRevs) / 8;
		}
		else
		{
			if (quad->EngineRevs < 0xA000)
				quad->EngineRevs += (0xA000 - quad->EngineRevs) / 8;
		}

		*pitch = quad->EngineRevs;

		return drive;
	}

	void QuadBikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quadItem = &g_Level.Items[itemNumber];
		auto* quad = (QuadInfo*)quadItem->Data;

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		if (GetOnQuadBike(laraItem, &g_Level.Items[itemNumber], coll))
		{
			lara->Vehicle = itemNumber;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				CreateFlare(laraItem, ID_FLARE_ITEM, 0);
				UndrawFlareMeshes(laraItem);
				lara->Flare.ControlLeft = false;
				lara->Control.Weapon.RequestGunType = lara->Control.Weapon.GunType = LaraWeaponType::None;
			}

			lara->Control.HandStatus = HandStatus::Busy;

			short angle = phd_atan(quadItem->Pose.Position.z - laraItem->Pose.Position.z, quadItem->Pose.Position.x - laraItem->Pose.Position.x);
			angle -= quadItem->Pose.Orientation.y;

			if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_LEFT;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = QUAD_STATE_MOUNT_LEFT;
			}
			else
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_RIGHT;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = QUAD_STATE_MOUNT_RIGHT;
			}

			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Pose.Position.x = quadItem->Pose.Position.x;
			laraItem->Pose.Position.y = quadItem->Pose.Position.y;
			laraItem->Pose.Position.z = quadItem->Pose.Position.z;
			laraItem->Pose.Orientation.y = quadItem->Pose.Orientation.y;
			ResetLaraFlex(laraItem);
			lara->HitDirection = -1;
			quadItem->HitPoints = 1;

			AnimateItem(laraItem);

			quad->Revs = 0;
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}

	static void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
	{
		auto* spark = GetFreeParticle();

		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;

		spark->dR = 96;
		spark->dG = 96;
		spark->dB = 128;

		if (moving)
		{
			spark->dR = (spark->dR * speed) / 32;
			spark->dG = (spark->dG * speed) / 32;
			spark->dB = (spark->dB * speed) / 32;
		}

		spark->sLife = spark->life = (GetRandomControl() & 3) + 20 - (speed / 4096);
		if (spark->sLife < 9)
			spark->sLife = spark->life = 9;

		spark->blendMode = BLEND_MODES::BLENDMODE_SCREEN;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 4;
		spark->extras = 0;
		spark->dynamic = -1;
		spark->x = x + ((GetRandomControl() & 15) - 8);
		spark->y = y + ((GetRandomControl() & 15) - 8);
		spark->z = z + ((GetRandomControl() & 15) - 8);
		int zv = speed * phd_cos(angle) / 4;
		int xv = speed * phd_sin(angle) / 4;
		spark->xVel = xv + ((GetRandomControl() & 255) - 128);
		spark->yVel = -(GetRandomControl() & 7) - 8;
		spark->zVel = zv + ((GetRandomControl() & 255) - 128);
		spark->friction = 4;

		if (GetRandomControl() & 1)
		{
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
			spark->rotAng = GetRandomControl() & 4095;
			if (GetRandomControl() & 1)
				spark->rotAdd = -(GetRandomControl() & 7) - 24;
			else
				spark->rotAdd = (GetRandomControl() & 7) + 24;
		}
		else
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

		spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		spark->scalar = 2;
		spark->gravity = -(GetRandomControl() & 3) - 4;
		spark->maxYvel = -(GetRandomControl() & 7) - 8;
		int size = (GetRandomControl() & 7) + 64 + (speed / 128);
		spark->dSize = size;
		spark->size = spark->sSize = size / 2;
	}

	bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quadItem = &g_Level.Items[lara->Vehicle];
		auto* quad = (QuadInfo*)quadItem->Data;

		GameVector	oldPos;
		oldPos.x = quadItem->Pose.Position.x;
		oldPos.y = quadItem->Pose.Position.y;
		oldPos.z = quadItem->Pose.Position.z;
		oldPos.roomNumber = quadItem->RoomNumber;

		bool collide = QuadDynamics(laraItem, quadItem);

		auto probe = GetCollision(quadItem);

		Vector3Int frontLeft, frontRight;
		auto floorHeightLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &frontLeft);
		auto floorHeightRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &frontRight);

		TestTriggers(quadItem, false);

		bool dead = false;
		if (laraItem->HitPoints <= 0)
		{
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
			dead = true;
		}

		int drive = -1;
		int pitch = 0;
		if (quad->Flags)
			collide = false;
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case QUAD_STATE_MOUNT_LEFT:
			case QUAD_STATE_MOUNT_RIGHT:
			case QUAD_STATE_DISMOUNT_LEFT:
			case QUAD_STATE_DISMOUNT_RIGHT:
				drive = -1;
				collide = false;
				break;

			default:
				drive = QuadUserControl(quadItem, probe.Position.Floor, &pitch);
				break;
			}
		}

		if (quad->Velocity || quad->Revs)
		{
			quad->Pitch = pitch;
			if (quad->Pitch < -0x8000)
				quad->Pitch = -0x8000;
			else if (quad->Pitch > 0xA000)
				quad->Pitch = 0xA000;

			SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_MOVE, &quadItem->Pose, SoundEnvironment::Land, 0.5f + (float)abs(quad->Pitch) / (float)MAX_VELOCITY);
		}
		else
		{
			if (drive != -1)
				SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_IDLE, &quadItem->Pose);

			quad->Pitch = 0;
		}

		quadItem->Floor = probe.Position.Floor;

		short rotAdd = quad->Velocity / 4;
		quad->RearRot -= rotAdd;
		quad->RearRot -= (quad->Revs / 8);
		quad->FrontRot -= rotAdd;

		quad->LeftVerticalVelocity = DoQuadDynamics(floorHeightLeft, quad->LeftVerticalVelocity, (int*)&frontLeft.y);
		quad->RightVerticalVelocity = DoQuadDynamics(floorHeightRight, quad->RightVerticalVelocity, (int*)&frontRight.y);
		quadItem->Animation.VerticalVelocity = DoQuadDynamics(probe.Position.Floor, quadItem->Animation.VerticalVelocity, (int*)&quadItem->Pose.Position.y);
		quad->Velocity = DoVehicleWaterMovement(quadItem, laraItem, quad->Velocity, QUAD_RADIUS, &quad->TurnRate);

		probe.Position.Floor = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(QUAD_FRONT, quadItem->Pose.Position.y - probe.Position.Floor);
		short zRot = phd_atan(QUAD_SIDE, probe.Position.Floor - frontLeft.y);

		quadItem->Pose.Orientation.x += ((xRot - quadItem->Pose.Orientation.x) / 2);
		quadItem->Pose.Orientation.z += ((zRot - quadItem->Pose.Orientation.z) / 2);

		if (!(quad->Flags & QUAD_FLAG_DEAD))
		{
			if (probe.RoomNumber != quadItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose = quadItem->Pose;

			AnimateQuadBike(laraItem, quadItem, collide, dead);
			AnimateItem(laraItem);

			quadItem->Animation.AnimNumber = Objects[ID_QUAD].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
			quadItem->Animation.FrameNumber = g_Level.Anims[quadItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			Camera.targetElevation = -ANGLE(30.0f);

			if (quad->Flags & QUAD_FLAG_IS_FALLING)
			{
				if (quadItem->Pose.Position.y == quadItem->Floor)
				{
					ExplodeVehicle(laraItem, quadItem);
					return false;
				}
			}
		}

		if (laraItem->Animation.ActiveState != QUAD_STATE_MOUNT_RIGHT &&
			laraItem->Animation.ActiveState != QUAD_STATE_MOUNT_LEFT &&
			laraItem->Animation.ActiveState != QUAD_STATE_DISMOUNT_RIGHT &&
			laraItem->Animation.ActiveState != QUAD_STATE_DISMOUNT_LEFT)
		{
			Vector3Int pos;
			int speed = 0;
			short angle = 0;

			for (int i = 0; i < 2; i++)
			{
				pos.x = quadEffectsPositions[i].x;
				pos.y = quadEffectsPositions[i].y;
				pos.z = quadEffectsPositions[i].z;
				GetJointAbsPosition(quadItem, &pos, quadEffectsPositions[i].meshNum);
				angle = quadItem->Pose.Orientation.y + ((i == 0) ? 0x9000 : 0x7000);
				if (quadItem->Animation.Velocity > 32)
				{
					if (quadItem->Animation.Velocity < 64)
					{
						speed = 64 - quadItem->Animation.Velocity;
						TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
					}
				}
				else
				{
					if (quad->SmokeStart < 16)
					{
						speed = ((quad->SmokeStart * 2) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) * 128;
						quad->SmokeStart++;
					}
					else if (quad->DriftStarting)
						speed = (abs(quad->Revs) * 2) + ((GetRandomControl() & 7) * 128);
					else if ((GetRandomControl() & 3) == 0)
						speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) * 128;
					else
						speed = 0;

					TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
				}
			}
		}
		else
			quad->SmokeStart = 0;

		return QuadCheckGetOff(laraItem, quadItem);
	}
}
