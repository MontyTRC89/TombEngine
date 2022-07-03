#include "framework.h"
#include "Objects/TR2/Vehicles/speedboat.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/effects/simple_particle.h"
#include "Objects/TR2/Vehicles/speedboat_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;
using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	const vector<VehicleMountType> SpeedboatMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right,
		VehicleMountType::Jump
	};

	constexpr auto SPEEDBOAT_RADIUS = 500;
	constexpr auto SPEEDBOAT_FRONT = 750;
	constexpr auto SPEEDBOAT_BACK = -700;
	constexpr auto SPEEDBOAT_SIDE = 300;
	constexpr auto SPEEDBOAT_SLIP = 10;
	constexpr auto SPEEDBOAT_SLIP_SIDE = 30;
	constexpr auto SPEEDBOAT_MOUNT_DISTANCE = CLICK(2.25f);
	constexpr auto SPEEDBOAT_DISMOUNT_DISTANCE = SECTOR(1);

	constexpr auto SPEEDBOAT_VELOCITY_ACCEL = 5;
	constexpr auto SPEEDBOAT_VELOCITY_DECEL = 1;
	constexpr auto SPEEDBOAT_VELOCITY_BRAKE_DECEL = 5;
	constexpr auto SPEEDBOAT_REVERSE_VELOCITY_DECEL = 5;

	constexpr auto SPEEDBOAT_VELOCITY_MIN = 20;
	constexpr auto SPEEDBOAT_SLOW_VELOCITY_MAX = 37;
	constexpr auto SPEEDBOAT_NORMAL_VELOCITY_MAX = 110;
	constexpr auto SPEEDBOAT_FAST_VELOCITY_MAX = 185;
	constexpr auto SPEEDBOAT_REVERSE_VELOCITY_MAX = 20;

	constexpr auto SPEEDBOAT_STEP_HEIGHT_MAX = CLICK(1); // Unused.
	constexpr auto SPEEDBOAT_SOUND_CEILING = SECTOR(5); // Unused.
	constexpr auto SPEEDBOAT_TIP = SPEEDBOAT_FRONT + 250;

	#define SPEEDBOAT_TURN_RATE_ACCEL (ANGLE(0.25f) / 2)
	#define SPEEDBOAT_TURN_RATE_DECEL ANGLE(0.25f)
	#define SPEEDBOAT_TURN_RATE_MAX	  ANGLE(4.0f)

	enum SpeedboatState
	{
		SPEEDBOAT_STATE_MOUNT = 0,
		SPEEDBOAT_STATE_IDLE = 1,
		SPEEDBOAT_STATE_MOVING = 2,
		SPEEDBOAT_STATE_DISMOUNT_RIGHT = 3,
		SPEEDBOAT_STATE_DISMOUNT_LEFT = 4,
		SPEEDBOAT_STATE_HIT = 5,
		SPEEDBOAT_STATE_FALL = 6,
		SPEEDBOAT_STATE_TURN_RIGHT = 7,
		SPEEDBOAT_STATE_DEATH = 8,
		SPEEDBOAT_STATE_TURN_LEFT = 9
	};

	enum SpeedboatAnim
	{
		SPEEDBOAT_ANIM_MOUNT_LEFT = 0,
		SPEEDBOAT_ANIM_IDLE = 1,	// ?
		SPEEDBOAT_ANIM_FORWARD = 2,	// ?

		SPEEDBOAT_ANIM_DISMOUNT_LEFT = 5,
		SPEEDBOAT_ANIM_MOUNT_JUMP = 6,
		SPEEDBOAT_ANIM_DISMOUNT_RIGHT = 7,
		SPEEDBOAT_ANIM_MOUNT_RIGHT = 8,

		SPEEDBOAT_ANIM_HIT_LEFT = 11,
		SPEEDBOAT_ANIM_HIT_RIGHT = 12,
		SPEEDBOAT_ANIM_HIT_FRONT = 13,
		SPEEDBOAT_ANIM_HIT_BACK = 14,
		SPEEDBOAT_ANIM_LEAP_START = 15,
		SPEEDBOAT_ANIM_LEAP = 16,
		SPEEDBOAT_ANIM_LEAP_END = 17,
		SPEEDBOAT_ANIM_DEATH = 18
	};

	SpeedboatInfo* GetSpeedboatInfo(ItemInfo* speedboatItem)
	{
		return (SpeedboatInfo*)speedboatItem->Data;
	}

	void InitialiseSpeedboat(short itemNumber)
	{
		auto* speedboatItem = &g_Level.Items[itemNumber];
		speedboatItem->Data = SpeedboatInfo();
		auto* speedboat = GetSpeedboatInfo(speedboatItem);
	}

	void SpeedboatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* speedboatItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(speedboatItem, laraItem, coll, SpeedboatMountTypes, SPEEDBOAT_MOUNT_DISTANCE, LARA_HEIGHT);
		if (mountType == VehicleMountType::None)
		{
			coll->Setup.EnableObjectPush = true;
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			lara->Vehicle = itemNumber;
			DoSpeedboatMount(speedboatItem, laraItem,mountType);

			if (g_Level.Items[itemNumber].Status != ITEM_ACTIVE)
			{
				AddActiveItem(itemNumber);
				g_Level.Items[itemNumber].Status = ITEM_ACTIVE;
			}
		}
	}

	void DoSpeedboatMount(ItemInfo* speedboatItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_IDLE;
			laraItem->Animation.ActiveState = SPEEDBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOUNT;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = SPEEDBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOUNT;
			break;

		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = SPEEDBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Jump:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_JUMP;
			laraItem->Animation.ActiveState = SPEEDBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOUNT;
			break;
		} 
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		if (laraItem->RoomNumber != speedboatItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, speedboatItem->RoomNumber);

		laraItem->Pose.Position = speedboatItem->Pose.Position;
		laraItem->Pose.Position.y -= 5;
		laraItem->Pose.Orientation = Vector3Shrt(0, speedboatItem->Pose.Orientation.y, 0);
		laraItem->Animation.IsAirborne = false;
		laraItem->Animation.Velocity = 0;
		laraItem->Animation.VerticalVelocity = 0;
		lara->Control.WaterStatus = WaterStatus::Dry;

		AnimateItem(laraItem);
	}

	bool TestSpeedboatDismount(ItemInfo* speedboatItem, int direction)
	{
		short angle;
		if (direction < 0)
			angle = speedboatItem->Pose.Orientation.y - ANGLE(90.0f);
		else
			angle = speedboatItem->Pose.Orientation.y + ANGLE(90.0f);

		int x = speedboatItem->Pose.Position.x +  SPEEDBOAT_DISMOUNT_DISTANCE * phd_sin(angle);
		int y = speedboatItem->Pose.Position.y;
		int z = speedboatItem->Pose.Position.z +  SPEEDBOAT_DISMOUNT_DISTANCE * phd_cos(angle);
		auto probe = GetCollision(x, y, z, speedboatItem->RoomNumber);

		if ((probe.Position.Floor - speedboatItem->Pose.Position.y) < -CLICK(2))
			return false;

		if (probe.Position.FloorSlope ||
			probe.Position.Floor == NO_HEIGHT)
		{
			return false;
		}

		if ((probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT ||
			(probe.Position.Ceiling - speedboatItem->Pose.Position.y) > -LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	void DoSpeedboatDismount(ItemInfo* speedboatItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if ((laraItem->Animation.ActiveState == SPEEDBOAT_STATE_DISMOUNT_LEFT ||
			laraItem->Animation.ActiveState == SPEEDBOAT_STATE_DISMOUNT_RIGHT) &&
			TestLastFrame(laraItem, laraItem->Animation.AnimNumber))
		{
			if (laraItem->Animation.ActiveState == SPEEDBOAT_STATE_DISMOUNT_LEFT)
				laraItem->Pose.Orientation.y -= ANGLE(90.0f);
			else if (laraItem->Animation.ActiveState == SPEEDBOAT_STATE_DISMOUNT_RIGHT)
				laraItem->Pose.Orientation.y += ANGLE(90.0f);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Animation.IsAirborne = true;
			laraItem->Animation.Velocity = 40;
			laraItem->Animation.VerticalVelocity = -50;
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.z = 0;
			lara->Vehicle = NO_ITEM;

			int x = laraItem->Pose.Position.x + 360 * phd_sin(laraItem->Pose.Orientation.y);
			int y = laraItem->Pose.Position.y - 90;
			int z = laraItem->Pose.Position.z + 360 * phd_cos(laraItem->Pose.Orientation.y);
			auto probe = GetCollision(x, y, z, laraItem->RoomNumber);

			if (probe.Position.Floor >= (y - CLICK(1)))
			{
				laraItem->Pose.Position.x = x;
				laraItem->Pose.Position.z = z;

				if (probe.RoomNumber != laraItem->RoomNumber)
					ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}
			laraItem->Pose.Position.y = y;

			speedboatItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT].animIndex;
			speedboatItem->Animation.FrameNumber = g_Level.Anims[speedboatItem->Animation.AnimNumber].frameBase;
		}
	}

	void SpeedboatDoBoatShift(ItemInfo* speedboatItem, int itemNumber)
	{
		short itemNumber2 = g_Level.Rooms[speedboatItem->RoomNumber].itemNumber;
		while (itemNumber2 != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNumber2];

			if (item->ObjectNumber == ID_SPEEDBOAT && itemNumber2 != itemNumber && Lara.Vehicle != itemNumber2)
			{
				int x = item->Pose.Position.x - speedboatItem->Pose.Position.x;
				int z = item->Pose.Position.z - speedboatItem->Pose.Position.z;

				int distance = pow(x, 2) + pow(z, 2);
				int radius = pow(SPEEDBOAT_RADIUS * 2, 2);
				if (distance < radius)
				{
					speedboatItem->Pose.Position.x = item->Pose.Position.x - x * radius / distance;
					speedboatItem->Pose.Position.z = item->Pose.Position.z - z * radius / distance;
				}

				return;
			}

			// TODO: mine and gondola

			itemNumber2 = item->NextItem;
		}
	}

	short SpeedboatDoShift(ItemInfo* speedboatItem, Vector3Int* pos, Vector3Int* old)
	{
		int x = pos->x / SECTOR(1);
		int z = pos->z / SECTOR(1);

		int xOld = old->x / SECTOR(1);
		int zOld = old->z / SECTOR(1);

		int shiftX = pos->x & (SECTOR(1) - 1);
		int shiftZ = pos->z & (SECTOR(1) - 1);

		if (x == xOld)
		{
			if (z == zOld)
			{
				speedboatItem->Pose.Position.z += (old->z - pos->z);
				speedboatItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > zOld)
			{
				speedboatItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - speedboatItem->Pose.Position.x);
			}
			else
			{
				speedboatItem->Pose.Position.z += SECTOR(1) - shiftZ;
				return (speedboatItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == zOld)
		{
			if (x > xOld)
			{
				speedboatItem->Pose.Position.x -= shiftX + 1;
				return (speedboatItem->Pose.Position.z - pos->z);
			}
			else
			{
				speedboatItem->Pose.Position.x += SECTOR(1) - shiftX;
				return (pos->z - speedboatItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			auto probe = GetCollision(old->x, pos->y, pos->z, speedboatItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = SECTOR(1) - shiftZ;
			}

			probe = GetCollision(pos->x, pos->y, old->z, speedboatItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = SECTOR(1) - shiftX;
			}

			if (x && z)
			{
				speedboatItem->Pose.Position.z += z;
				speedboatItem->Pose.Position.x += x;
			}
			else if (z)
			{
				speedboatItem->Pose.Position.z += z;
				if (z > 0)
					return (speedboatItem->Pose.Position.x - pos->x);
				else
					return (pos->x - speedboatItem->Pose.Position.x);
			}
			else if (x)
			{
				speedboatItem->Pose.Position.x += x;
				if (x > 0)
					return (pos->z - speedboatItem->Pose.Position.z);
				else
					return (speedboatItem->Pose.Position.z - pos->z);
			}
			else
			{
				speedboatItem->Pose.Position.z += (old->z - pos->z);
				speedboatItem->Pose.Position.x += (old->x - pos->x);
			}
		}

		return 0;
	}

	int GetSpeedboatHitAnim(ItemInfo* speedboatItem, Vector3Int* moved)
	{
		moved->x = speedboatItem->Pose.Position.x - moved->x;
		moved->z = speedboatItem->Pose.Position.z - moved->z;

		if (moved->x || moved->z)
		{
			float sinY = phd_sin(speedboatItem->Pose.Orientation.y);
			float cosY = phd_cos(speedboatItem->Pose.Orientation.y);

			int front = (moved->z * cosY) + (moved->x * sinY);
			int side = (moved->z * -sinY) + (moved->x * cosY);

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return SPEEDBOAT_ANIM_HIT_BACK;
				else
					return SPEEDBOAT_ANIM_HIT_FRONT;
			}
			else
			{
				if (side > 0)
					return SPEEDBOAT_ANIM_HIT_LEFT;
				else
					return SPEEDBOAT_ANIM_HIT_RIGHT;
			}
		}

		return 0;
	}

	int DoSpeedboatDynamics(int height, int verticalVelocity, int* y)
	{
		if (height > *y)
		{
			*y += verticalVelocity;
			if (*y > height)
			{
				*y = height;
				verticalVelocity = 0;
			}
			else
				verticalVelocity += GRAVITY;
		}
		else
		{
			verticalVelocity += ((height - *y - verticalVelocity) / 8);
			if (verticalVelocity < -SPEEDBOAT_REVERSE_VELOCITY_MAX)
				verticalVelocity = -SPEEDBOAT_REVERSE_VELOCITY_MAX;

			if (*y > height)
				*y = height;
		}

		return verticalVelocity;
	}

	int SpeedboatDynamics(short itemNumber, ItemInfo* laraItem)
	{
		auto* speedboatItem = &g_Level.Items[itemNumber];
		auto* speedboat = GetSpeedboatInfo(speedboatItem);
		auto* lara = GetLaraInfo(laraItem);

		speedboatItem->Pose.Orientation.z -= speedboat->LeanAngle;

		Vector3Int old, frontLeftOld, frontRightOld, backLeftOld, backRightOld, frontOld;
		int heightFrontLeftOld = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, true, &frontLeftOld);
		int heightFrontRightOld = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, true, &frontRightOld);
		int heightBackLeftOld = GetVehicleWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, true, &backLeftOld);
		int heightBackRightOld = GetVehicleWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, true, &backRightOld);
		int heightFrontOld = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_TIP, 0, true, &frontOld);

		old.x = speedboatItem->Pose.Position.x;
		old.y = speedboatItem->Pose.Position.y;
		old.z = speedboatItem->Pose.Position.z;

		speedboatItem->Pose.Orientation.y += speedboat->TurnRate + speedboat->ExtraRotation;
		speedboat->LeanAngle = speedboat->TurnRate * 6;

		speedboatItem->Pose.Position.x += speedboatItem->Animation.Velocity * phd_sin(speedboatItem->Pose.Orientation.y);
		speedboatItem->Pose.Position.z += speedboatItem->Animation.Velocity * phd_cos(speedboatItem->Pose.Orientation.y);

		int slip = SPEEDBOAT_SLIP_SIDE * phd_sin(speedboatItem->Pose.Orientation.z);
		if (!slip && speedboatItem->Pose.Orientation.z)
			slip = (speedboatItem->Pose.Orientation.z > 0) ? 1 : -1;
		speedboatItem->Pose.Position.x += slip * phd_sin(speedboatItem->Pose.Orientation.y);
		speedboatItem->Pose.Position.z -= slip * phd_cos(speedboatItem->Pose.Orientation.y);

		slip = SPEEDBOAT_SLIP * phd_sin(speedboatItem->Pose.Orientation.x);
		if (!slip && speedboatItem->Pose.Orientation.x)
			slip = (speedboatItem->Pose.Orientation.x > 0) ? 1 : -1;
		speedboatItem->Pose.Position.x -= slip * phd_sin(speedboatItem->Pose.Orientation.y);
		speedboatItem->Pose.Position.z -= slip * phd_cos(speedboatItem->Pose.Orientation.y);

		auto moved = Vector3Int(speedboatItem->Pose.Position.x, 0, speedboatItem->Pose.Position.z);

		SpeedboatDoBoatShift(speedboatItem, itemNumber);

		Vector3Int fl, fr, br, bl, f;
		short rotation = 0;
		auto heightBackLeft = GetVehicleWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, false, &bl);
		if (heightBackLeft < (backLeftOld.y - CLICK(0.5f)))
			rotation = SpeedboatDoShift(speedboatItem, &bl, &backLeftOld);

		auto heightBackRight = GetVehicleWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, false, &br);
		if (heightBackRight < (backRightOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &br, &backRightOld);

		auto heightFrontLeft = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, false, &fl);
		if (heightFrontLeft < (frontLeftOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &fl, &frontLeftOld);

		auto heightFrontRight = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, false, &fr);
		if (heightFrontRight < (frontRightOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &fr, &frontRightOld);

		int heightFront = 0;
		if (!slip)
		{
			heightFront = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_TIP, 0, false, &f);
			if (heightFront < (frontOld.y - CLICK(0.5f)))
				SpeedboatDoShift(speedboatItem, &f, &frontOld);
		}

		auto probe = GetCollision(speedboatItem);
		auto height = GetWaterHeight(speedboatItem->Pose.Position.x, speedboatItem->Pose.Position.y - 5, speedboatItem->Pose.Position.z, probe.RoomNumber);

		if (height == NO_HEIGHT)
			height = GetFloorHeight(probe.Block, speedboatItem->Pose.Position.x, speedboatItem->Pose.Position.y - 5, speedboatItem->Pose.Position.z);

		if (height < (speedboatItem->Pose.Position.y - CLICK(0.5f)))
			SpeedboatDoShift(speedboatItem, (Vector3Int*)&speedboatItem->Pose, &old);

		speedboat->ExtraRotation = rotation;

		DoVehicleCollision(speedboatItem, SPEEDBOAT_RADIUS);

		auto collide = GetSpeedboatHitAnim(speedboatItem, &moved);

		int newVelocity = 0;
		if (slip || collide)
		{
			newVelocity = (speedboatItem->Pose.Position.z - old.z) * phd_cos(speedboatItem->Pose.Orientation.y) + (speedboatItem->Pose.Position.x - old.x) * phd_sin(speedboatItem->Pose.Orientation.y);

			if (lara->Vehicle == itemNumber && speedboatItem->Animation.Velocity > SPEEDBOAT_NORMAL_VELOCITY_MAX + SPEEDBOAT_VELOCITY_ACCEL && newVelocity < speedboatItem->Animation.Velocity - 10)
			{
				DoDamage(laraItem, speedboatItem->Animation.Velocity);
				SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Pose);
				newVelocity /= 2;
				speedboatItem->Animation.Velocity /= 2;
			}

			if (slip)
			{
				if (speedboatItem->Animation.Velocity <= SPEEDBOAT_NORMAL_VELOCITY_MAX + 10)
					speedboatItem->Animation.Velocity = newVelocity;
			}
			else
			{
				if (speedboatItem->Animation.Velocity > 0 && newVelocity < speedboatItem->Animation.Velocity)
					speedboatItem->Animation.Velocity = newVelocity;
				else if (speedboatItem->Animation.Velocity < 0 && newVelocity > speedboatItem->Animation.Velocity)
					speedboatItem->Animation.Velocity = newVelocity;
			}

			if (speedboatItem->Animation.Velocity < -SPEEDBOAT_REVERSE_VELOCITY_MAX)
				speedboatItem->Animation.Velocity = -SPEEDBOAT_REVERSE_VELOCITY_MAX;
		}

		return collide;
	}

	bool SpeedboatUserControl(ItemInfo* speedboatItem, ItemInfo* laraItem)
	{
		auto* speedboat = GetSpeedboatInfo(speedboatItem);

		bool noTurn = true;
		int maxVelocity;

		if (speedboatItem->Pose.Position.y >= speedboat->Water - CLICK(0.5f) && speedboat->Water != NO_HEIGHT)
		{
			if (!(TrInput & VEHICLE_IN_DISMOUNT) && !(TrInput & IN_LOOK) ||
				speedboatItem->Animation.Velocity)
			{
				if (TrInput & VEHICLE_IN_LEFT && !(TrInput & VEHICLE_IN_REVERSE) ||
					TrInput & VEHICLE_IN_RIGHT && TrInput & VEHICLE_IN_REVERSE)
				{
					if (speedboat->TurnRate > 0)
						speedboat->TurnRate -= SPEEDBOAT_TURN_RATE_DECEL;
					else
					{
						speedboat->TurnRate -= SPEEDBOAT_TURN_RATE_ACCEL;
						if (speedboat->TurnRate < -SPEEDBOAT_TURN_RATE_MAX)
							speedboat->TurnRate = -SPEEDBOAT_TURN_RATE_MAX;
					}

					noTurn = false;
				}
				else if (TrInput & VEHICLE_IN_RIGHT && !(TrInput & VEHICLE_IN_REVERSE) ||
					TrInput & VEHICLE_IN_LEFT && TrInput & VEHICLE_IN_REVERSE)
				{
					if (speedboat->TurnRate < 0)
						speedboat->TurnRate += SPEEDBOAT_TURN_RATE_DECEL;
					else
					{
						speedboat->TurnRate += SPEEDBOAT_TURN_RATE_ACCEL;
						if (speedboat->TurnRate > SPEEDBOAT_TURN_RATE_MAX)
							speedboat->TurnRate = SPEEDBOAT_TURN_RATE_MAX;
					}

					noTurn = false;
				}

				if (TrInput & VEHICLE_IN_REVERSE)
				{
					if (speedboatItem->Animation.Velocity > 0)
						speedboatItem->Animation.Velocity -= SPEEDBOAT_VELOCITY_BRAKE_DECEL;
					else if (speedboatItem->Animation.Velocity > -SPEEDBOAT_REVERSE_VELOCITY_MAX)
						speedboatItem->Animation.Velocity -= SPEEDBOAT_REVERSE_VELOCITY_DECEL;
				}
				else if (TrInput & VEHICLE_IN_ACCELERATE)
				{
					if (TrInput & VEHICLE_IN_SPEED)
						maxVelocity = SPEEDBOAT_FAST_VELOCITY_MAX;
					else
						maxVelocity = (TrInput & VEHICLE_IN_SLOW) ? SPEEDBOAT_SLOW_VELOCITY_MAX : SPEEDBOAT_NORMAL_VELOCITY_MAX;

					if (speedboatItem->Animation.Velocity < maxVelocity)
						speedboatItem->Animation.Velocity += (SPEEDBOAT_VELOCITY_ACCEL / 2) + (SPEEDBOAT_VELOCITY_ACCEL * (speedboatItem->Animation.Velocity / (maxVelocity * 2)));
					else if (speedboatItem->Animation.Velocity > (maxVelocity + SPEEDBOAT_VELOCITY_DECEL))
						speedboatItem->Animation.Velocity -= SPEEDBOAT_VELOCITY_DECEL;
				}
				else if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
					speedboatItem->Animation.Velocity >= 0 &&
					speedboatItem->Animation.Velocity < SPEEDBOAT_VELOCITY_MIN)
				{
					if (!(TrInput & VEHICLE_IN_DISMOUNT) &&
						speedboatItem->Animation.Velocity == 0)
						speedboatItem->Animation.Velocity = SPEEDBOAT_VELOCITY_MIN;
				}
				else if (speedboatItem->Animation.Velocity > SPEEDBOAT_VELOCITY_DECEL)
					speedboatItem->Animation.Velocity -= SPEEDBOAT_VELOCITY_DECEL;
				else
					speedboatItem->Animation.Velocity = 0;
			}
			else
			{
				if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
					speedboatItem->Animation.Velocity >= 0 &&
					speedboatItem->Animation.Velocity < SPEEDBOAT_VELOCITY_MIN)
				{
					if (speedboatItem->Animation.Velocity == 0 && !(TrInput & VEHICLE_IN_DISMOUNT))
						speedboatItem->Animation.Velocity = SPEEDBOAT_VELOCITY_MIN;
				}
				else if (speedboatItem->Animation.Velocity > SPEEDBOAT_VELOCITY_DECEL)
					speedboatItem->Animation.Velocity -= SPEEDBOAT_VELOCITY_DECEL;
				else
					speedboatItem->Animation.Velocity = 0;

				if (TrInput & IN_LOOK && speedboatItem->Animation.Velocity == 0)
					LookUpDown(laraItem);
			}
		}

		return noTurn;
	}

	void SpeedboatAnimation(ItemInfo* speedboatItem, ItemInfo* laraItem, int collide)
	{
		auto* speedboat = GetSpeedboatInfo(speedboatItem);

		if (laraItem->HitPoints <= 0)
		{
			if (laraItem->Animation.ActiveState != SPEEDBOAT_STATE_DEATH)
			{
				laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SPEEDBOAT_STATE_DEATH;
			}
		}
		else if (speedboatItem->Pose.Position.y < speedboat->Water - CLICK(0.5f) && speedboatItem->Animation.VerticalVelocity > 0)
		{
			if (laraItem->Animation.ActiveState != SPEEDBOAT_STATE_FALL)
			{
				laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_LEAP_START;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SPEEDBOAT_STATE_FALL;
			}
		}
		else if (collide)
		{
			if (laraItem->Animation.ActiveState != SPEEDBOAT_STATE_HIT)
			{
				laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SPEEDBOAT_STATE_HIT;
			}
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SPEEDBOAT_STATE_IDLE:
				if (TrInput & VEHICLE_IN_DISMOUNT)
				{
					if (speedboatItem->Animation.Velocity == 0)
					{
						if (TrInput & VEHICLE_IN_RIGHT && TestSpeedboatDismount(speedboatItem, speedboatItem->Pose.Orientation.y + ANGLE(90.0f)))
							laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_RIGHT;
						else if (TrInput & VEHICLE_IN_LEFT && TestSpeedboatDismount(speedboatItem, speedboatItem->Pose.Orientation.y - ANGLE(90.0f)))
							laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_LEFT;
					}
				}

				if (speedboatItem->Animation.Velocity > 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;

			case SPEEDBOAT_STATE_MOVING:
				if (TrInput & VEHICLE_IN_DISMOUNT)
				{
					if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_RIGHT;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_LEFT;
				}
				else if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;

				break;

			case SPEEDBOAT_STATE_FALL:
				laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;
				break;

				//case SPEEDBOAT_TURN_RATE_ACCELR:
				if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;
				else if (!(TrInput & VEHICLE_IN_RIGHT))
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;

			case SPEEDBOAT_STATE_TURN_LEFT:
				if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;
				else if (!(TrInput & VEHICLE_IN_LEFT))
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;
			}
		}
	}

	void SpeedboatSplash(ItemInfo* speedboatItem, long verticalVelocity, long water)
	{
		//OLD SPLASH
		/*
		splash_setup.x = speedboatItem->pos.x_pos;
		splash_setup.y = water;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerXZoff = 16 << 2;
		splash_setup.InnerXZsize = 12 << 2;
		splash_setup.InnerYsize = -96 << 2;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -fallspeed << 7;
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24 << 2;
		splash_setup.MiddleXZsize = 24 << 2;
		splash_setup.MiddleYsize = -64 << 2;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -fallspeed << 6;
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32 << 2;
		splash_setup.OuterXZsize = 32 << 2;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;
		SetupSplash(&splash_setup);
		SplashCount = 16;
		*/
	}

	void SpeedboatControl(short itemNumber)
	{
		auto* speedboatItem = &g_Level.Items[itemNumber];
		auto* speedboat = GetSpeedboatInfo(speedboatItem);
		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);

		int collide = SpeedboatDynamics(itemNumber, laraItem);

		Vector3Int frontLeft, frontRight;
		int heightFrontLeft = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, true, &frontLeft);
		int heightFrontRight = GetVehicleWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, true, &frontRight);

		auto probe = GetCollision(speedboatItem);

		if (lara->Vehicle == itemNumber)
		{
			TestTriggers(speedboatItem, true);
			TestTriggers(speedboatItem, false);
		}

		auto water = GetWaterHeight(speedboatItem->Pose.Position.x, speedboatItem->Pose.Position.y, speedboatItem->Pose.Position.z, probe.RoomNumber);
		speedboat->Water = water;

		bool noTurn = true;
		bool drive = false;
		bool idle = !speedboatItem->Animation.Velocity;

		if (lara->Vehicle == itemNumber && laraItem->HitPoints > 0)
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SPEEDBOAT_STATE_MOUNT:
			case SPEEDBOAT_STATE_DISMOUNT_RIGHT:
			case SPEEDBOAT_STATE_DISMOUNT_LEFT:
				break;

			default:
				drive = true;
				noTurn = SpeedboatUserControl(speedboatItem, laraItem);
				break;
			}
		}
		else
		{
			if (speedboatItem->Animation.Velocity > SPEEDBOAT_VELOCITY_DECEL)
				speedboatItem->Animation.Velocity -= SPEEDBOAT_VELOCITY_DECEL;
			else
				speedboatItem->Animation.Velocity = 0;
		}

		if (noTurn)
		{
			if (speedboat->TurnRate < -SPEEDBOAT_TURN_RATE_DECEL)
				speedboat->TurnRate += SPEEDBOAT_TURN_RATE_DECEL;
			else if (speedboat->TurnRate > SPEEDBOAT_TURN_RATE_DECEL)
				speedboat->TurnRate -= SPEEDBOAT_TURN_RATE_DECEL;
			else
				speedboat->TurnRate = 0;
		}

		speedboatItem->Floor = probe.Position.Floor - 5;
		if (speedboat->Water == NO_HEIGHT)
			speedboat->Water = probe.Position.Floor;
		else
			speedboat->Water -= 5;

		speedboat->LeftVerticalVelocity = DoSpeedboatDynamics(heightFrontLeft, speedboat->LeftVerticalVelocity, (int*)&frontLeft.y);
		speedboat->RightVerticalVelocity = DoSpeedboatDynamics(heightFrontRight, speedboat->RightVerticalVelocity, (int*)&frontRight.y);
		speedboatItem->Animation.VerticalVelocity = DoSpeedboatDynamics(speedboat->Water, speedboatItem->Animation.VerticalVelocity, (int*)&speedboatItem->Pose.Position.y);

		auto ofs = speedboatItem->Animation.VerticalVelocity;
		if (ofs - speedboatItem->Animation.VerticalVelocity > 32 && speedboatItem->Animation.VerticalVelocity == 0 && water != NO_HEIGHT)
			SpeedboatSplash(speedboatItem, ofs - speedboatItem->Animation.VerticalVelocity, water);

		probe.Position.Floor = (frontLeft.y + frontRight.y);
		if (probe.Position.Floor < 0)
			probe.Position.Floor = -(abs(probe.Position.Floor) / 2);
		else
			probe.Position.Floor /= 2;

		short xRot = phd_atan(SPEEDBOAT_FRONT, speedboatItem->Pose.Position.y - probe.Position.Floor);
		short zRot = phd_atan(SPEEDBOAT_SIDE, probe.Position.Floor - frontLeft.y);

		speedboatItem->Pose.Orientation.x += ((xRot - speedboatItem->Pose.Orientation.x) / 2);
		speedboatItem->Pose.Orientation.z += ((zRot - speedboatItem->Pose.Orientation.z) / 2);

		if (!xRot && abs(speedboatItem->Pose.Orientation.x) < 4)
			speedboatItem->Pose.Orientation.x = 0;
		if (!zRot && abs(speedboatItem->Pose.Orientation.z) < 4)
			speedboatItem->Pose.Orientation.z = 0;

		if (lara->Vehicle == itemNumber)
		{
			SpeedboatAnimation(speedboatItem, laraItem, collide);

			if (probe.RoomNumber != speedboatItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose = speedboatItem->Pose;
			speedboatItem->Pose.Orientation.z += speedboat->LeanAngle;

			AnimateItem(laraItem);

			if (laraItem->HitPoints > 0)
			{
				speedboatItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
				speedboatItem->Animation.FrameNumber = g_Level.Anims[speedboatItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
			}

			Camera.targetElevation = -ANGLE(20.0f);
			Camera.targetDistance = SECTOR(2);

			auto pitch = speedboatItem->Animation.Velocity;
			speedboat->Pitch += (pitch - speedboat->Pitch) / 4;

			if (drive)
			{
				bool accelerating = idle && abs(speedboatItem->Animation.Velocity) > 4;
				bool moving = (abs(speedboatItem->Animation.Velocity) > 8 || speedboat->TurnRate);
				int fx = accelerating ? SFX_TR2_VEHICLE_SPEEDBOAT_ACCELERATE : (moving ? SFX_TR2_VEHICLE_SPEEDBOAT_MOVING : SFX_TR2_VEHICLE_SPEEDBOAT_IDLE);
				float pitch  = idle ? 1.0f : 1.0f + speedboat->Pitch / (float)SPEEDBOAT_NORMAL_VELOCITY_MAX / 4.0f;
				SoundEffect(fx, &speedboatItem->Pose, SoundEnvironment::Land, pitch);
			}
		}
		else
		{
			if (probe.RoomNumber != speedboatItem->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			speedboatItem->Pose.Orientation.z += speedboat->LeanAngle;
		}

		if (speedboatItem->Animation.Velocity && (water - 5) == speedboatItem->Pose.Position.y)
		{
			auto room = probe.Block->RoomBelow(speedboatItem->Pose.Position.x, speedboatItem->Pose.Position.z).value_or(NO_ROOM);
			if (room != NO_ROOM && (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, room) || TestEnvironment(RoomEnvFlags::ENV_FLAG_SWAMP, room)))
				TEN::Effects::TriggerSpeedboatFoam(speedboatItem, Vector3(0.0f, 0.0f, SPEEDBOAT_BACK));
		}

		if (lara->Vehicle != itemNumber)
			return;

		DoSpeedboatDismount(speedboatItem, laraItem);
	}
}
