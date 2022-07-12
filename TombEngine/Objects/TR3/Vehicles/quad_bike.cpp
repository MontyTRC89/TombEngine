#include "framework.h"
#include "Objects/TR3/Vehicles/quad_bike.h"

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
#include "Objects/TR3/Vehicles/quad_bike_info.h"
#include "Objects/Utils/VehicleHelpers.h"
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
	BITE_INFO QuadBikeEffectsPositions[6] =
	{
		{ -56, -32, -380, 0	},
		{ 56, -32, -380, 0 },
		{ -8, 180, -48, 3 },
		{ 8, 180, -48, 4 },
		{ 90, 180, -32, 6 },
		{ -90, 180, -32, 7 }
	};
	const vector<VehicleMountType> QuadBikeMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	constexpr auto DAMAGE_START = 140;
	constexpr auto DAMAGE_LENGTH = 14;

	constexpr auto QBIKE_RADIUS = 500;
	constexpr auto QBIKE_HEIGHT = 512;
	constexpr auto QBIKE_FRONT = 550;
	constexpr auto QBIKE_BACK = -550;
	constexpr auto QBIKE_SIDE = 260;
	constexpr auto QBIKE_SLIP = 100;
	constexpr auto QBIKE_SLIP_SIDE = 50;
	constexpr auto QBIKE_MOUNT_DISTANCE = CLICK(2);
	constexpr auto QBIKE_DISMOUNT_DISTANCE = 385; // Precise offset derived from animation.

	constexpr int MAX_VELOCITY = 160 * VEHICLE_VELOCITY_SCALE;
	constexpr int MIN_DRIFT_VELOCITY = 48 * VEHICLE_VELOCITY_SCALE;
	constexpr int BRAKE = 2.5f * VEHICLE_VELOCITY_SCALE;
	constexpr int REVERSE_ACCELERATION = -3 * VEHICLE_VELOCITY_SCALE;
	constexpr int MAX_BACK = -48 * VEHICLE_VELOCITY_SCALE;
	constexpr int MAX_REVS = 160 * VEHICLE_VELOCITY_SCALE;
	constexpr int TERMINAL_VERTICAL_VELOCITY = 240;

	constexpr auto QBIKE_STEP_HEIGHT_MAX = CLICK(1); // Unused.
	constexpr auto QBIKE_MIN_BOUNCE = (MAX_VELOCITY / 2) / CLICK(1);

	// TODO
	constexpr auto QBIKE_HIT_LEFT = 11;
	constexpr auto QBIKE_HIT_RIGHT = 12;
	constexpr auto QBIKE_HIT_FRONT = 13;
	constexpr auto QBIKE_HIT_BACK = 14;

	#define QBIKE_TURN_RATE_ACCEL		  ANGLE(2.5f)
	#define QBIKE_TURN_RATE_DECEL		  ANGLE(2.0f)
	#define QBIKE_TURN_RATE_MAX			  ANGLE(5.0f)
	#define QBIKE_DRIFT_TURN_RATE_ACCEL	  ANGLE(2.75f)
	#define QBIKE_DRIFT_TURN_RATE_MAX	  ANGLE(8.0f)
	#define QBIKE_MOMENTUM_TURN_RATE_MIN  ANGLE(3.0f)
	#define QBIKE_MOMENTUM_TURN_RATE_MAX  ANGLE(1.5f)
	#define QBIKE_MOMENTUM_TURN_RATE_MAX2 ANGLE(150.0f) // TODO: Resolve this naming clash!

	enum QuadBikeState
	{
		QBIKE_STATE_DRIVE = 1,
		QBIKE_STATE_TURN_LEFT = 2,
		QBIKE_STATE_SLOW = 5,
		QBIKE_STATE_BRAKE = 6,
		QBIKE_STATE_BIKE_DEATH = 7,
		QBIKE_STATE_FALL = 8,
		QBIKE_STATE_MOUNT_RIGHT = 9,
		QBIKE_STATE_DISMOUNT_RIGHT = 10,
		QBIKE_STATE_HIT_BACK = 11,
		QBIKE_STATE_HIT_FRONT = 12,
		QBIKE_STATE_HIT_LEFT = 13,
		QBIKE_STATE_HIT_RIGHT = 14,
		QBIKE_STATE_IDLE = 15,
		QBIKE_STATE_LAND = 17,
		QBIKE_STATE_STOP_SLOWLY = 18,
		QBIKE_STATE_FALL_DEATH = 19,
		QBIKE_STATE_FALL_OFF = 20,
		QBIKE_STATE_WHEELIE = 21,	// Unused.
		QBIKE_STATE_TURN_RIGHT = 22,
		QBIKE_STATE_MOUNT_LEFT = 23,
		QBIKE_STATE_DISMOUNT_LEFT = 24,
	};

	enum QuadBikeAnim
	{
		QBIKE_ANIM_IDLE_DEATH = 0,
		QBIKE_ANIM_UNK_1 = 1,
		QBIKE_ANIM_DRIVE_BACK = 2,
		QBIKE_ANIM_TURN_LEFT_START = 3,
		QBIKE_ANIM_TURN_LEFT_CONTINUE = 4,
		QBIKE_ANIM_TURN_LEFT_END = 5,
		QBIKE_ANIM_LEAP_START = 6,
		QBIKE_ANIM_LEAP_CONTINUE = 7,
		QBIKE_ANIM_LEAP_END = 8,
		QBIKE_ANIM_MOUNT_RIGHT = 9,
		QBIKE_ANIM_DISMOUNT_RIGHT = 10,
		QBIKE_ANIM_HIT_FRONT = 11,
		QBIKE_ANIM_HIT_BACK = 12,
		QBIKE_ANIM_HIT_RIGHT = 13,
		QBIKE_ANIM_HIT_LEFT = 14,
		QBIKE_ANIM_UNK_2 = 15,
		QBIKE_ANIM_UNK_3 = 16,
		QBIKE_ANIM_UNK_4 = 17,
		QBIKE_ANIM_IDLE = 18,
		QBIKE_ANIM_FALL_OFF_DEATH = 19,
		QBIKE_ANIM_TURN_RIGHT_START = 20,
		QBIKE_ANIM_TURN_RIGHT_CONTINUE = 21,
		QBIKE_ANIM_TURN_RIGHT_END = 22,
		QBIKE_ANIM_MOUNT_LEFT = 23,
		QBIKE_ANIM_DISMOUNT_LEFT = 24,
		QBIKE_ANIM_LEAP_START2 = 25,
		QBIKE_ANIM_LEAP_CONTINUE2 = 26,
		QBIKE_ANIM_LEAP_END2 = 27,
		QBIKE_ANIM_LEAP_TO_FREEFALL = 28
	};

	enum QuadBikeFlags
	{
		QBIKE_FLAG_FALLING = (1 << 6),
		QBIKE_FLAG_DEAD = (1 << 7)
	};

	enum QuadBikeEffectPosition
	{
		EXHAUST_LEFT = 0,
		EXHAUST_RIGHT = 1,
		FRONT_LEFT_TYRE = 2,
		FRONT_RIGHT_TYRE = 3,
		BACK_LEFT_TYRE = 4,
		BACK_RIGHT_TYRE = 5
	};

	QuadBikeInfo* GetQuadBikeInfo(ItemInfo* quadBikeItem)
	{
		return (QuadBikeInfo*)quadBikeItem->Data;
	}

	void InitialiseQuadBike(short itemNumber)
	{
		auto* quadBikeItem = &g_Level.Items[itemNumber];
		quadBikeItem->Data = QuadBikeInfo();
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y;
	}

	void QuadBikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* quadBikeItem = &g_Level.Items[itemNumber];
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(quadBikeItem, laraItem, coll, QuadBikeMountTypes, QBIKE_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
			ObjectCollision(itemNumber, laraItem, coll);
		else
		{
			lara->Vehicle = itemNumber;
			DoQuadBikeMount(quadBikeItem, laraItem, mountType);
		}
	}

	void DoQuadBikeMount(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_IDLE;
			laraItem->Animation.ActiveState = QBIKE_STATE_IDLE;
			laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = QBIKE_STATE_MOUNT_LEFT;
			laraItem->Animation.TargetState = QBIKE_STATE_MOUNT_LEFT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = QBIKE_STATE_MOUNT_RIGHT;
			laraItem->Animation.TargetState = QBIKE_STATE_MOUNT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		ResetLaraFlex(laraItem);
		laraItem->Pose.Position = quadBikeItem->Pose.Position;
		laraItem->Pose.Orientation = Vector3Shrt(0, quadBikeItem->Pose.Orientation.y, 0);
		lara->Control.HandStatus = HandStatus::Busy;
		lara->HitDirection = -1;
		quadBikeItem->HitPoints = 1;
		quadBike->Revs = 0;

		AnimateItem(laraItem);
	}

	static int CanQuadbikeGetOff(ItemInfo* laraItem, int direction)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quadBikeItem = &g_Level.Items[lara->Vehicle];

		short angle = quadBikeItem->Pose.Orientation.y;
		angle += (direction < 0) ? -ANGLE(90.0f) : ANGLE(90.0f);

		int x = quadBikeItem->Pose.Position.x + CLICK(2) * phd_sin(angle);
		int y = quadBikeItem->Pose.Position.y;
		int z = quadBikeItem->Pose.Position.z + CLICK(2) * phd_cos(angle);

		auto collResult = GetCollision(x, y, z, quadBikeItem->RoomNumber);

		if (collResult.Position.FloorSlope ||
			collResult.Position.Floor == NO_HEIGHT)
		{
			return false;
		}

		if (abs(collResult.Position.Floor - quadBikeItem->Pose.Position.y) > CLICK(2))
			return false;

		if ((collResult.Position.Ceiling - quadBikeItem->Pose.Position.y) > -LARA_HEIGHT ||
			(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	static bool QuadBikeCheckGetOff(ItemInfo* quadBikeItem, ItemInfo* laraItem)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Vehicle == NO_ITEM)
			return true;

		if ((laraItem->Animation.ActiveState == QBIKE_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == QBIKE_STATE_DISMOUNT_LEFT) &&
			TestLastFrame(laraItem))
		{
			if (laraItem->Animation.ActiveState == QBIKE_STATE_DISMOUNT_LEFT)
				laraItem->Pose.Orientation.y += ANGLE(90.0f);
			else
				laraItem->Pose.Orientation.y -= ANGLE(90.0f);

			SetAnimation(laraItem, LA_STAND_IDLE);
			TranslateItem(laraItem, laraItem->Pose.Orientation.y, -QBIKE_DISMOUNT_DISTANCE);
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.z = 0;
			lara->Vehicle = NO_ITEM;
			lara->Control.HandStatus = HandStatus::Free;

			if (laraItem->Animation.ActiveState == QBIKE_STATE_FALL_OFF)
			{
				auto pos = Vector3Int();

				SetAnimation(laraItem, LA_FREEFALL);
				GetJointAbsPosition(laraItem, &pos, LM_HIPS);

				laraItem->Pose.Position = pos;
				laraItem->Animation.IsAirborne = true;
				laraItem->Animation.VerticalVelocity = quadBikeItem->Animation.VerticalVelocity;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				laraItem->HitPoints = 0;
				lara->Control.HandStatus = HandStatus::Free;
				quadBikeItem->Flags |= IFLAG_INVISIBLE;

				return false;
			}
			else if (laraItem->Animation.ActiveState == QBIKE_STATE_FALL_DEATH)
			{
				laraItem->Animation.TargetState = LS_DEATH;
				laraItem->Animation.Velocity = 0;
				laraItem->Animation.VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
				quadBike->Flags |= QBIKE_FLAG_DEAD;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	static int GetQuadCollisionAnim(ItemInfo* quadBikeItem, Vector3Int* pos)
	{
		pos->x = quadBikeItem->Pose.Position.x - pos->x;
		pos->z = quadBikeItem->Pose.Position.z - pos->z;

		if (pos->x || pos->z)
		{
			float c = phd_cos(quadBikeItem->Pose.Orientation.y);
			float s = phd_sin(quadBikeItem->Pose.Orientation.y);
			int front = pos->z * c + pos->x * s;
			int side = -pos->z * s + pos->x * c;

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return QBIKE_HIT_BACK;
				else
					return QBIKE_HIT_FRONT;
			}
			else
			{
				if (side > 0)
					return QBIKE_HIT_LEFT;
				else
					return QBIKE_HIT_RIGHT;
			}
		}

		return 0;
	}

	static int DoQuadShift(ItemInfo* quadBikeItem, Vector3Int* pos, Vector3Int* old)
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
				quadBikeItem->Pose.Position.z += (old->z - pos->z);
				quadBikeItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > oldZ)
			{
				quadBikeItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - quadBikeItem->Pose.Position.x);
			}
			else
			{
				quadBikeItem->Pose.Position.z += SECTOR(1) - shiftZ;
				return (quadBikeItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == oldZ)
		{
			if (x > oldX)
			{
				quadBikeItem->Pose.Position.x -= shiftX + 1;
				return (quadBikeItem->Pose.Position.z - pos->z);
			}
			else
			{
				quadBikeItem->Pose.Position.x += SECTOR(1) - shiftX;
				return (pos->z - quadBikeItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			probe = GetCollision(old->x, pos->y, pos->z, quadBikeItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = SECTOR(1) - shiftZ;
			}

			probe = GetCollision(pos->x, pos->y, old->z, quadBikeItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = SECTOR(1) - shiftX;
			}

			if (x && z)
			{
				quadBikeItem->Pose.Position.z += z;
				quadBikeItem->Pose.Position.x += x;
			}
			else if (z)
			{
				quadBikeItem->Pose.Position.z += z;

				if (z > 0)
					return (quadBikeItem->Pose.Position.x - pos->x);
				else
					return (pos->x - quadBikeItem->Pose.Position.x);
			}
			else if (x)
			{
				quadBikeItem->Pose.Position.x += x;

				if (x > 0)
					return (pos->z - quadBikeItem->Pose.Position.z);
				else
					return (quadBikeItem->Pose.Position.z - pos->z);
			}
			else
			{
				quadBikeItem->Pose.Position.z += (old->z - pos->z);
				quadBikeItem->Pose.Position.x += (old->x - pos->x);
			}
		}

		return 0;
	}

	static int DoQuadDynamics(int height, int verticalVelocity, int* y)
	{
		if (height > *y)
		{
			*y += verticalVelocity;
			if (*y > height - QBIKE_MIN_BOUNCE)
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

	static int QuadDynamics(ItemInfo* quadBikeItem, ItemInfo* laraItem)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);
		auto* lara = GetLaraInfo(laraItem);

		quadBike->NoDismount = false;

		Vector3Int oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight;
		int holdFrontLeft = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, -QBIKE_SIDE, true, &oldFrontLeft);
		int holdFrontRight = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, true, &oldFrontRight);
		int holdBottomLeft = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT, -QBIKE_SIDE, true, &oldBottomLeft);
		int holdBottomRight = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT, QBIKE_SIDE, true, &oldBottomRight);

		Vector3Int mtlOld, mtrOld, mmlOld, mmrOld;
		int hmml_old = GetVehicleHeight(quadBikeItem, 0, -QBIKE_SIDE, true, &mmlOld);
		int hmmr_old = GetVehicleHeight(quadBikeItem, 0, QBIKE_SIDE, true, &mmrOld);
		int hmtl_old = GetVehicleHeight(quadBikeItem, QBIKE_FRONT / 2, -QBIKE_SIDE, true, &mtlOld);
		int hmtr_old = GetVehicleHeight(quadBikeItem, QBIKE_FRONT / 2, QBIKE_SIDE, true, &mtrOld);

		Vector3Int moldBottomLeft, moldBottomRight;
		int hmoldBottomLeft = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT / 2, -QBIKE_SIDE, true, &moldBottomLeft);
		int hmoldBottomRight = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT / 2, QBIKE_SIDE, true, &moldBottomRight);

		Vector3Int old;
		old.x = quadBikeItem->Pose.Position.x;
		old.y = quadBikeItem->Pose.Position.y;
		old.z = quadBikeItem->Pose.Position.z;

		if (quadBikeItem->Pose.Position.y > (quadBikeItem->Floor - CLICK(1)))
		{
			if (quadBike->TurnRate < -QBIKE_TURN_RATE_DECEL)
				quadBike->TurnRate += QBIKE_TURN_RATE_DECEL;
			else if (quadBike->TurnRate > QBIKE_TURN_RATE_DECEL)
				quadBike->TurnRate -= QBIKE_TURN_RATE_DECEL;
			else
				quadBike->TurnRate = 0;

			quadBikeItem->Pose.Orientation.y += quadBike->TurnRate + quadBike->ExtraRotation;

			short momentum = QBIKE_MOMENTUM_TURN_RATE_MIN - (((((QBIKE_MOMENTUM_TURN_RATE_MIN - QBIKE_MOMENTUM_TURN_RATE_MAX) * 256) / MAX_VELOCITY) * quadBike->Velocity) / 256);
			if (!(TrInput & VEHICLE_IN_ACCELERATE) && quadBike->Velocity > 0)
				momentum += momentum / 4;

			short rot = quadBikeItem->Pose.Orientation.y - quadBike->MomentumAngle;
			if (rot < -QBIKE_MOMENTUM_TURN_RATE_MAX)
			{
				if (rot < -QBIKE_MOMENTUM_TURN_RATE_MAX2)
				{
					rot = -QBIKE_MOMENTUM_TURN_RATE_MAX2;
					quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y - rot;
				}
				else
					quadBike->MomentumAngle -= momentum;
			}
			else if (rot > QBIKE_MOMENTUM_TURN_RATE_MAX)
			{
				if (rot > QBIKE_MOMENTUM_TURN_RATE_MAX2)
				{
					rot = QBIKE_MOMENTUM_TURN_RATE_MAX2;
					quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y - rot;
				}
				else
					quadBike->MomentumAngle += momentum;
			}
			else
				quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y;
		}
		else
			quadBikeItem->Pose.Orientation.y += quadBike->TurnRate + quadBike->ExtraRotation;

		auto probe = GetCollision(quadBikeItem);
		int speed = 0;
		if (quadBikeItem->Pose.Position.y >= probe.Position.Floor)
			speed = quadBikeItem->Animation.Velocity * phd_cos(quadBikeItem->Pose.Orientation.x);
		else
			speed = quadBikeItem->Animation.Velocity;

		TranslateItem(quadBikeItem, quadBike->MomentumAngle, speed);

		int slip = QBIKE_SLIP * phd_sin(quadBikeItem->Pose.Orientation.x);
		if (abs(slip) > QBIKE_SLIP / 2)
		{
			if (slip > 0)
				slip -= 10;
			else
				slip += 10;
			quadBikeItem->Pose.Position.z -= slip * phd_cos(quadBikeItem->Pose.Orientation.y);
			quadBikeItem->Pose.Position.x -= slip * phd_sin(quadBikeItem->Pose.Orientation.y);
		}

		slip = QBIKE_SLIP_SIDE * phd_sin(quadBikeItem->Pose.Orientation.z);
		if (abs(slip) > QBIKE_SLIP_SIDE / 2)
		{
			quadBikeItem->Pose.Position.z -= slip * phd_sin(quadBikeItem->Pose.Orientation.y);
			quadBikeItem->Pose.Position.x += slip * phd_cos(quadBikeItem->Pose.Orientation.y);
		}

		Vector3Int moved;
		moved.x = quadBikeItem->Pose.Position.x;
		moved.z = quadBikeItem->Pose.Position.z;

		if (!(quadBikeItem->Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(quadBikeItem, QBIKE_RADIUS);

		short rot = 0;
		short rotAdd = 0;

		Vector3Int fl;
		int heightFrontLeft = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, -QBIKE_SIDE, false, &fl);
		if (heightFrontLeft < (oldFrontLeft.y - CLICK(1)))
			rot = DoQuadShift(quadBikeItem, &fl, &oldFrontLeft);

		Vector3Int mtl;
		int hmtl = GetVehicleHeight(quadBikeItem, QBIKE_FRONT / 2, -QBIKE_SIDE, false, &mtl);
		if (hmtl < (mtlOld.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mtl, &mtlOld);

		Vector3Int mml;
		int hmml = GetVehicleHeight(quadBikeItem, 0, -QBIKE_SIDE, false, &mml);
		if (hmml < (mmlOld.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mml, &mmlOld);

		Vector3Int mbl;
		int hmbl = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT / 2, -QBIKE_SIDE, false, &mbl);
		if (hmbl < (moldBottomLeft.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mbl, &moldBottomLeft);

		Vector3Int bl;
		int heightBackLeft = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT, -QBIKE_SIDE, false, &bl);
		if (heightBackLeft < (oldBottomLeft.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadBikeItem, &bl, &oldBottomLeft);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		Vector3Int fr;
		int heightFrontRight = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, false, &fr);
		if (heightFrontRight < (oldFrontRight.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadBikeItem, &fr, &oldFrontRight);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		Vector3Int mtr;
		int hmtr = GetVehicleHeight(quadBikeItem, QBIKE_FRONT / 2, QBIKE_SIDE, false, &mtr);
		if (hmtr < (mtrOld.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mtr, &mtrOld);

		Vector3Int mmr;
		int hmmr = GetVehicleHeight(quadBikeItem, 0, QBIKE_SIDE, false, &mmr);
		if (hmmr < (mmrOld.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mmr, &mmrOld);

		Vector3Int mbr;
		int hmbr = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT / 2, QBIKE_SIDE, false, &mbr);
		if (hmbr < (moldBottomRight.y - CLICK(1)))
			DoQuadShift(quadBikeItem, &mbr, &moldBottomRight);

		Vector3Int br;
		int heightBackRight = GetVehicleHeight(quadBikeItem, -QBIKE_FRONT, QBIKE_SIDE, false, &br);
		if (heightBackRight < (oldBottomRight.y - CLICK(1)))
		{
			rotAdd = DoQuadShift(quadBikeItem, &br, &oldBottomRight);
			if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
				rot += rotAdd;
		}

		probe = GetCollision(quadBikeItem);
		if (probe.Position.Floor < quadBikeItem->Pose.Position.y - CLICK(1))
			DoQuadShift(quadBikeItem, (Vector3Int*)&quadBikeItem->Pose, &old);

		quadBike->ExtraRotation = rot;

		int collide = GetQuadCollisionAnim(quadBikeItem, &moved);

		int newVelocity = 0;
		if (collide)
		{
			newVelocity = (quadBikeItem->Pose.Position.z - old.z) * phd_cos(quadBike->MomentumAngle) + (quadBikeItem->Pose.Position.x - old.x) * phd_sin(quadBike->MomentumAngle);
			newVelocity *= VEHICLE_VELOCITY_SCALE;

			if (&g_Level.Items[lara->Vehicle] == quadBikeItem &&
				quadBike->Velocity == MAX_VELOCITY &&
				newVelocity < (quadBike->Velocity - 10))
			{
				DoDamage(laraItem, (quadBike->Velocity - newVelocity) / 128);
			}

			if (quadBike->Velocity > 0 && newVelocity < quadBike->Velocity)
				quadBike->Velocity = (newVelocity < 0) ? 0 : newVelocity;

			else if (quadBike->Velocity < 0 && newVelocity > quadBike->Velocity)
				quadBike->Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (quadBike->Velocity < MAX_BACK)
				quadBike->Velocity = MAX_BACK;
		}

		return collide;
	}

	static void AnimateQuadBike(ItemInfo* quadBikeItem, ItemInfo* laraItem, int collide, bool dead)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		if (quadBikeItem->Pose.Position.y != quadBikeItem->Floor &&
			laraItem->Animation.ActiveState != QBIKE_STATE_FALL &&
			laraItem->Animation.ActiveState != QBIKE_STATE_LAND &&
			laraItem->Animation.ActiveState != QBIKE_STATE_FALL_OFF &&
			!dead)
		{
			if (quadBike->Velocity < 0)
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_LEAP_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_LEAP_START2;

			laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
			laraItem->Animation.ActiveState = QBIKE_STATE_FALL;
			laraItem->Animation.TargetState = QBIKE_STATE_FALL;
		}
		else if (collide &&
			laraItem->Animation.ActiveState != QBIKE_STATE_HIT_FRONT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_HIT_BACK &&
			laraItem->Animation.ActiveState != QBIKE_STATE_HIT_LEFT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_HIT_RIGHT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_FALL_OFF &&
			quadBike->Velocity > (MAX_VELOCITY / 3) &&
			!dead)
		{
			if (collide == QBIKE_HIT_FRONT)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_HIT_BACK;
				laraItem->Animation.ActiveState = QBIKE_STATE_HIT_FRONT;
				laraItem->Animation.TargetState = QBIKE_STATE_HIT_FRONT;
			}
			else if (collide == QBIKE_HIT_BACK)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_HIT_FRONT;
				laraItem->Animation.ActiveState = QBIKE_STATE_HIT_BACK;
				laraItem->Animation.TargetState = QBIKE_STATE_HIT_BACK;
			}
			else if (collide == QBIKE_HIT_LEFT)
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_HIT_RIGHT;
				laraItem->Animation.ActiveState = QBIKE_STATE_HIT_LEFT;
				laraItem->Animation.TargetState = QBIKE_STATE_HIT_LEFT;
			}
			else
			{
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_HIT_LEFT;
				laraItem->Animation.ActiveState = QBIKE_STATE_HIT_RIGHT;
				laraItem->Animation.TargetState = QBIKE_STATE_HIT_RIGHT;
			}

			laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
			SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_FRONT_IMPACT, &quadBikeItem->Pose);
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case QBIKE_STATE_IDLE:
				if (dead)
					laraItem->Animation.TargetState = QBIKE_STATE_BIKE_DEATH;
				else if (TrInput & VEHICLE_IN_DISMOUNT &&
					quadBike->Velocity == 0 &&
					!quadBike->NoDismount)
				{
					if (TrInput & VEHICLE_IN_LEFT && CanQuadbikeGetOff(laraItem, -1))
						laraItem->Animation.TargetState = QBIKE_STATE_DISMOUNT_LEFT;
					else if (TrInput & VEHICLE_IN_RIGHT && CanQuadbikeGetOff(laraItem, 1))
						laraItem->Animation.TargetState = QBIKE_STATE_DISMOUNT_RIGHT;
				}
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;

			case QBIKE_STATE_DRIVE:
				if (dead)
				{
					if (quadBike->Velocity > (MAX_VELOCITY / 2))
						laraItem->Animation.TargetState = QBIKE_STATE_FALL_DEATH;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_BIKE_DEATH;
				}
				else if (!(TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE)) &&
					(quadBike->Velocity / VEHICLE_VELOCITY_SCALE) == 0)
				{
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				}
				else if (TrInput & VEHICLE_IN_LEFT &&
					!quadBike->DriftStarting)
				{
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT &&
					!quadBike->DriftStarting)
				{
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else if (TrInput & (VEHICLE_IN_REVERSE | VEHICLE_IN_BRAKE))
				{
					if (quadBike->Velocity > (MAX_VELOCITY / 3 * 2))
						laraItem->Animation.TargetState = QBIKE_STATE_BRAKE;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_SLOW;
				}

				break;

			case QBIKE_STATE_BRAKE:
			case QBIKE_STATE_SLOW:
			case QBIKE_STATE_STOP_SLOWLY:
				if ((quadBike->Velocity / VEHICLE_VELOCITY_SCALE) == 0)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_LEFT)
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				else if (TrInput & VEHICLE_IN_RIGHT)
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;

				break;

			case QBIKE_STATE_TURN_LEFT:
				if ((quadBike->Velocity / VEHICLE_VELOCITY_SCALE) == 0)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_RIGHT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_RIGHT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else if (!(TrInput & VEHICLE_IN_LEFT))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;

			case QBIKE_STATE_TURN_RIGHT:
				if ((quadBike->Velocity / VEHICLE_VELOCITY_SCALE) == 0)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_LEFT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_LEFT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}
				else if (!(TrInput & VEHICLE_IN_RIGHT))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;

			case QBIKE_STATE_FALL:
				if (quadBikeItem->Pose.Position.y == quadBikeItem->Floor)
					laraItem->Animation.TargetState = QBIKE_STATE_LAND;
				else if (quadBikeItem->Animation.VerticalVelocity > TERMINAL_VERTICAL_VELOCITY)
					quadBike->Flags |= QBIKE_FLAG_FALLING;

				break;

			case QBIKE_STATE_FALL_OFF:
				break;

			case QBIKE_STATE_HIT_FRONT:
			case QBIKE_STATE_HIT_BACK:
			case QBIKE_STATE_HIT_LEFT:
			case QBIKE_STATE_HIT_RIGHT:
				if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;
			}
		}
	}

	static int QuadUserControl(ItemInfo* quadBikeItem, int height, int* pitch)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		bool drive = false; // Never changes?

		if (!(TrInput & VEHICLE_IN_SPEED) &&
			!quadBike->Velocity && !quadBike->CanStartDrift)
		{
			quadBike->CanStartDrift = true;
		}
		else if (quadBike->Velocity)
			quadBike->CanStartDrift = false;

		if (!(TrInput & VEHICLE_IN_SPEED))
			quadBike->DriftStarting = false;

		if (!quadBike->DriftStarting)
		{
			if (quadBike->Revs > 0x10)
			{
				quadBike->Velocity += (quadBike->Revs / 16);
				quadBike->Revs -= (quadBike->Revs / 8);
			}
			else
				quadBike->Revs = 0;
		}

		if (quadBikeItem->Pose.Position.y >= (height - CLICK(1)))
		{
			if (TrInput & IN_LOOK && !quadBike->Velocity)
				LookUpDown(LaraItem);

			// Driving forward.
			if (quadBike->Velocity > 0)
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					!quadBike->DriftStarting &&
					quadBike->Velocity > MIN_DRIFT_VELOCITY)
				{
					if (TrInput & VEHICLE_IN_LEFT)
					{
						quadBike->TurnRate -= QBIKE_DRIFT_TURN_RATE_ACCEL;
						if (quadBike->TurnRate < -QBIKE_DRIFT_TURN_RATE_MAX)
							quadBike->TurnRate = -QBIKE_DRIFT_TURN_RATE_MAX;
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
					{
						quadBike->TurnRate += QBIKE_DRIFT_TURN_RATE_ACCEL;
						if (quadBike->TurnRate > QBIKE_DRIFT_TURN_RATE_MAX)
							quadBike->TurnRate = QBIKE_DRIFT_TURN_RATE_MAX;
					}
				}
				else
				{
					if (TrInput & VEHICLE_IN_LEFT)
					{
						quadBike->TurnRate -= QBIKE_TURN_RATE_ACCEL;
						if (quadBike->TurnRate < -QBIKE_TURN_RATE_MAX)
							quadBike->TurnRate = -QBIKE_TURN_RATE_MAX;
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
					{
						quadBike->TurnRate += QBIKE_TURN_RATE_ACCEL;
						if (quadBike->TurnRate > QBIKE_TURN_RATE_MAX)
							quadBike->TurnRate = QBIKE_TURN_RATE_MAX;
					}
				}
			}
			// Driving back.
			else if (quadBike->Velocity < 0)
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					!quadBike->DriftStarting &&
					quadBike->Velocity < (-MIN_DRIFT_VELOCITY + 0x800))
				{
					if (TrInput & VEHICLE_IN_LEFT)
					{
						quadBike->TurnRate -= QBIKE_DRIFT_TURN_RATE_ACCEL;
						if (quadBike->TurnRate < -QBIKE_DRIFT_TURN_RATE_MAX)
							quadBike->TurnRate = -QBIKE_DRIFT_TURN_RATE_MAX;
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
					{
						quadBike->TurnRate += QBIKE_DRIFT_TURN_RATE_ACCEL;
						if (quadBike->TurnRate > QBIKE_DRIFT_TURN_RATE_MAX)
							quadBike->TurnRate = QBIKE_DRIFT_TURN_RATE_MAX;
					}
				}
				else
				{
					if (TrInput & VEHICLE_IN_RIGHT)
					{
						quadBike->TurnRate -= QBIKE_TURN_RATE_ACCEL;
						if (quadBike->TurnRate < -QBIKE_TURN_RATE_MAX)
							quadBike->TurnRate = -QBIKE_TURN_RATE_MAX;
					}
					else if (TrInput & VEHICLE_IN_LEFT)
					{
						quadBike->TurnRate += QBIKE_TURN_RATE_ACCEL;
						if (quadBike->TurnRate > QBIKE_TURN_RATE_MAX)
							quadBike->TurnRate = QBIKE_TURN_RATE_MAX;
					}
				}
			}

			// Driving back / braking.
			if (TrInput & (VEHICLE_IN_REVERSE | VEHICLE_IN_BRAKE))
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					(quadBike->CanStartDrift || quadBike->DriftStarting))
				{
					quadBike->DriftStarting = true;
					quadBike->Revs -= 0x200;
					if (quadBike->Revs < MAX_BACK)
						quadBike->Revs = MAX_BACK;
				}
				else if (quadBike->Velocity > 0)
					quadBike->Velocity -= BRAKE;
				else
				{
					if (quadBike->Velocity > MAX_BACK)
						quadBike->Velocity += REVERSE_ACCELERATION;
				}
			}
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					(quadBike->CanStartDrift || quadBike->DriftStarting))
				{
					quadBike->DriftStarting = true;
					quadBike->Revs += 0x200;
					if (quadBike->Revs >= MAX_VELOCITY)
						quadBike->Revs = MAX_VELOCITY;
				}
				else if (quadBike->Velocity < MAX_VELOCITY)
				{
					if (quadBike->Velocity < 0x4000)
						quadBike->Velocity += (8 + (0x4000 + 0x800 - quadBike->Velocity) / 8);
					else if (quadBike->Velocity < 0x7000)
						quadBike->Velocity += (4 + (0x7000 + 0x800 - quadBike->Velocity) / 16);
					else if (quadBike->Velocity < MAX_VELOCITY)
						quadBike->Velocity += (2 + (MAX_VELOCITY - quadBike->Velocity) / 8);
				}
				else
					quadBike->Velocity = MAX_VELOCITY;

				quadBike->Velocity -= abs(quadBikeItem->Pose.Orientation.y - quadBike->MomentumAngle) / 64;
			}

			else if (quadBike->Velocity > 0x0100)
				quadBike->Velocity -= 0x0100;
			else if (quadBike->Velocity < -0x0100)
				quadBike->Velocity += 0x0100;
			else
				quadBike->Velocity = 0;

			if (!(TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE)) &&
				quadBike->DriftStarting &&
				quadBike->Revs)
			{
				if (quadBike->Revs > 0x8)
					quadBike->Revs -= quadBike->Revs / 8;
				else
					quadBike->Revs = 0;
			}

			quadBikeItem->Animation.Velocity = quadBike->Velocity / VEHICLE_VELOCITY_SCALE;

			if (quadBike->EngineRevs > 0x7000)
				quadBike->EngineRevs = -0x2000;

			int revs = 0;
			if (quadBike->Velocity < 0)
				revs = abs(quadBike->Velocity / 2);
			else if (quadBike->Velocity < 0x7000)
				revs = -0x2000 + (quadBike->Velocity * (0x6800 - -0x2000)) / 0x7000;
			else if (quadBike->Velocity <= MAX_VELOCITY)
				revs = -0x2800 + ((quadBike->Velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000);

			revs += abs(quadBike->Revs);
			quadBike->EngineRevs += (revs - quadBike->EngineRevs) / 8;
		}
		else
		{
			if (quadBike->EngineRevs < 0xA000)
				quadBike->EngineRevs += (0xA000 - quadBike->EngineRevs) / 8;
		}

		*pitch = quadBike->EngineRevs;

		return drive;
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
		auto* quadBikeItem = &g_Level.Items[lara->Vehicle];
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		GameVector	oldPos;
		oldPos.x = quadBikeItem->Pose.Position.x;
		oldPos.y = quadBikeItem->Pose.Position.y;
		oldPos.z = quadBikeItem->Pose.Position.z;
		oldPos.roomNumber = quadBikeItem->RoomNumber;

		bool collide = QuadDynamics(quadBikeItem, laraItem);

		auto probe = GetCollision(quadBikeItem);

		Vector3Int frontLeft, frontRight;
		auto floorHeightLeft = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, -QBIKE_SIDE, false, &frontLeft);
		auto floorHeightRight = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, false, &frontRight);

		TestTriggers(quadBikeItem, false);

		bool dead = false;
		if (laraItem->HitPoints <= 0)
		{
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
			dead = true;
		}

		int drive = -1;
		int pitch = 0;
		if (quadBike->Flags)
			collide = false;
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case QBIKE_STATE_MOUNT_LEFT:
			case QBIKE_STATE_MOUNT_RIGHT:
			case QBIKE_STATE_DISMOUNT_LEFT:
			case QBIKE_STATE_DISMOUNT_RIGHT:
				drive = -1;
				collide = false;
				break;

			default:
				drive = QuadUserControl(quadBikeItem, probe.Position.Floor, &pitch);
				break;
			}
		}

		if (quadBike->Velocity || quadBike->Revs)
		{
			quadBike->Pitch = pitch;
			if (quadBike->Pitch < -0x8000)
				quadBike->Pitch = -0x8000;
			else if (quadBike->Pitch > 0xA000)
				quadBike->Pitch = 0xA000;

			SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_MOVE, &quadBikeItem->Pose, SoundEnvironment::Land, 0.5f + (float)abs(quadBike->Pitch) / (float)MAX_VELOCITY);
		}
		else
		{
			if (drive != -1)
				SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_IDLE, &quadBikeItem->Pose);

			quadBike->Pitch = 0;
		}

		quadBikeItem->Floor = probe.Position.Floor;

		short rotAdd = quadBike->Velocity / 4;
		quadBike->RearRot -= rotAdd;
		quadBike->RearRot -= (quadBike->Revs / 8);
		quadBike->FrontRot -= rotAdd;

		quadBike->LeftVerticalVelocity = DoQuadDynamics(floorHeightLeft, quadBike->LeftVerticalVelocity, (int*)&frontLeft.y);
		quadBike->RightVerticalVelocity = DoQuadDynamics(floorHeightRight, quadBike->RightVerticalVelocity, (int*)&frontRight.y);
		quadBikeItem->Animation.VerticalVelocity = DoQuadDynamics(probe.Position.Floor, quadBikeItem->Animation.VerticalVelocity, (int*)&quadBikeItem->Pose.Position.y);
		quadBike->Velocity = DoVehicleWaterMovement(quadBikeItem, laraItem, quadBike->Velocity, QBIKE_RADIUS, &quadBike->TurnRate);

		probe.Position.Floor = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(QBIKE_FRONT, quadBikeItem->Pose.Position.y - probe.Position.Floor);
		short zRot = phd_atan(QBIKE_SIDE, probe.Position.Floor - frontLeft.y);

		quadBikeItem->Pose.Orientation.x += ((xRot - quadBikeItem->Pose.Orientation.x) / 2);
		quadBikeItem->Pose.Orientation.z += ((zRot - quadBikeItem->Pose.Orientation.z) / 2);

		if (!(quadBike->Flags & QBIKE_FLAG_DEAD))
		{
			if (probe.RoomNumber != quadBikeItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose = quadBikeItem->Pose;

			AnimateQuadBike(quadBikeItem, laraItem, collide, dead);
			AnimateItem(laraItem);

			quadBikeItem->Animation.AnimNumber = Objects[ID_QUAD].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
			quadBikeItem->Animation.FrameNumber = g_Level.Anims[quadBikeItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			Camera.targetElevation = -ANGLE(30.0f);

			if (quadBike->Flags & QBIKE_FLAG_FALLING)
			{
				if (quadBikeItem->Pose.Position.y == quadBikeItem->Floor)
				{
					ExplodeVehicle(laraItem, quadBikeItem);
					return false;
				}
			}
		}

		if (laraItem->Animation.ActiveState != QBIKE_STATE_MOUNT_RIGHT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_MOUNT_LEFT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_DISMOUNT_RIGHT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_DISMOUNT_LEFT)
		{
			Vector3Int pos;
			int speed = 0;
			short angle = 0;

			for (int i = 0; i < 2; i++)
			{
				pos.x = QuadBikeEffectsPositions[i].x;
				pos.y = QuadBikeEffectsPositions[i].y;
				pos.z = QuadBikeEffectsPositions[i].z;
				GetJointAbsPosition(quadBikeItem, &pos, QuadBikeEffectsPositions[i].meshNum);
				angle = quadBikeItem->Pose.Orientation.y + ((i == 0) ? 0x9000 : 0x7000);
				if (quadBikeItem->Animation.Velocity > 32)
				{
					if (quadBikeItem->Animation.Velocity < 64)
					{
						speed = 64 - quadBikeItem->Animation.Velocity;
						TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
					}
				}
				else
				{
					if (quadBike->SmokeStart < 16)
					{
						speed = ((quadBike->SmokeStart * 2) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) * 128;
						quadBike->SmokeStart++;
					}
					else if (quadBike->DriftStarting)
						speed = (abs(quadBike->Revs) * 2) + ((GetRandomControl() & 7) * 128);
					else if ((GetRandomControl() & 3) == 0)
						speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) * 128;
					else
						speed = 0;

					TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
				}
			}
		}
		else
			quadBike->SmokeStart = 0;

		return QuadBikeCheckGetOff(quadBikeItem, laraItem);
	}
}
