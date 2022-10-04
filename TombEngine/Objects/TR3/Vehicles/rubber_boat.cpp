#include "framework.h"
#include "Objects/TR3/Vehicles/rubber_boat.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/bubble.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/rubber_boat_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11Enums.h"

using namespace TEN::Input;
using std::vector;

namespace TEN::Entities::Vehicles
{
	constexpr auto RBOAT_RADIUS	   = 500;
	constexpr auto RBOAT_FRONT	   = 750;
	constexpr auto RBOAT_SIDE	   = 300;
	constexpr auto RBOAT_SLIP	   = 10;
	constexpr auto RBOAT_SIDE_SLIP = 30;

	constexpr auto RBOAT_VELOCITY_ACCEL			= 5;
	constexpr auto RBOAT_VELOCITY_DECEL			= 1;
	constexpr auto RBOAT_VELOCITY_BRAKE_DECEL	= 5;
	constexpr auto RBOAT_REVERSE_VELOCITY_DECEL = 2;

	constexpr auto RBOAT_VELOCITY_MIN		  = 20;
	constexpr auto RBOAT_SLOW_VELOCITY_MAX	  = 37;
	constexpr auto RBOAT_NORMAL_VELOCITY_MAX  = 110;
	constexpr auto RBOAT_FAST_VELOCITY_MAX	  = 185;
	constexpr auto RBOAT_REVERSE_VELOCITY_MAX = 20;

	constexpr auto RBOAT_MOUNT_DISTANCE = CLICK(2.25f);
	constexpr auto RBOAT_DISMOUNT_DISTANCE = SECTOR(1);
	constexpr auto RBOAT_BOUNCE_MIN = 0;
	constexpr auto RBOAT_KICK_MAX = -80;

	#define RBOAT_TURN_RATE_ACCEL ANGLE(0.5f)
	#define RBOAT_TURN_RATE_DECEL ANGLE(0.25f)
	#define RBOAT_TURN_RATE_MAX	  ANGLE(4.0f)

	enum RubberBoatState
	{
		RBOAT_STATE_MOUNT = 0,
		RBOAT_STATE_IDLE = 1,
		RBOAT_STATE_MOVING = 2,
		RBOAT_STATE_JUMP_RIGHT = 3,
		RBOAT_STATE_JUMP_LEFT = 4,
		RBOAT_STATE_IMPACT = 5,
		RBOAT_STATE_FALL = 6,
		RBOAT_STATE_TURN_RIGHT = 7,
		RBOAT_STATE_DEATH = 8,
		RBOAT_STATE_TURN_LEFT = 9
	};

	enum RubberBoatAnim
	{
		RBOAT_ANIM_MOUNT_LEFT = 0,
		RBOAT_ANIM_IDLE = 1,
		RBOAT_ANIM_UNK_1 = 2,
		RBOAT_ANIM_UNK_2 = 3,
		RBOAT_ANIM_UNK_3 = 4,
		RBOAT_ANIM_DISMOUNT_LEFT = 5,
		RBOAT_ANIM_MOUNT_JUMP = 6,
		RBOAT_ANIM_DISMOUNT_RIGHT = 7,
		RBOAT_ANIM_MOUNT_RIGHT = 8,
		RBOAT_ANIM_TURN_LEFT_CONTINUE = 9,
		RBOAT_ANIM_TURN_LEFT_END = 10,
		RBOAT_ANIM_IMPACT_RIGHT = 11,
		RBOAT_ANIM_IMPACT_LEFT = 12,
		RBOAT_ANIM_IMPACT_FRONT = 13,
		RBOAT_ANIM_IMPACT_BACK = 14,
		RBOAT_ANIM_LEAP_START = 15,
		RBOAT_ANIM_LEAP_CONTINUE = 16,
		RBOAT_ANIM_LEAP_END = 17,
		RBOAT_ANIM_IDLE_DEATH = 18,
		RBOAT_ANIM_TURN_RIGHT_CONTINUE = 19,
		RBOAT_ANIM_TURN_RIGHT_END = 20,
		RBOAT_ANIM_TURN_LEFT_START = 21,
		RBOAT_ANIM_TURN_RIGHT_START = 22
	};

	const vector<VehicleMountType> RubberBoatMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right,
		VehicleMountType::Jump
	};

	RubberBoatInfo* GetRubberBoatInfo(ItemInfo* rBoatItem)
	{
		return (RubberBoatInfo*)rBoatItem->Data;
	}

	void InitialiseRubberBoat(short itemNumber)
	{
		auto* rBoatItem = &g_Level.Items[itemNumber];
		rBoatItem->Data = RubberBoatInfo();
		auto* rBoat = GetRubberBoatInfo(rBoatItem);
	}

	void RubberBoatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* rBoatItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(rBoatItem, laraItem, coll, RubberBoatMountTypes, RBOAT_MOUNT_DISTANCE, LARA_HEIGHT);
		if (mountType == VehicleMountType::None)
		{
			coll->Setup.EnableObjectPush = true;
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			lara->Vehicle = itemNumber;
			DoRubberBoatMount(rBoatItem, laraItem, mountType);

			if (g_Level.Items[itemNumber].Status != ITEM_ACTIVE)
			{
				AddActiveItem(itemNumber);
				g_Level.Items[itemNumber].Status = ITEM_ACTIVE;
			}
		}
	}

	void RubberBoatControl(short itemNumber)
	{
		auto* rBoatItem = &g_Level.Items[itemNumber];
		auto* rBoat = GetRubberBoatInfo(rBoatItem);
		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);

		bool drive = false;

		int pitch, height, ofs;

		Vector3Int frontLeft, frontRight;
		auto impactDirection = RubberBoatDynamics(itemNumber, laraItem);
		int heightFrontLeft = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, true, frontLeft);
		int heightFrontRight = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, true, frontRight);

		if (lara->Vehicle == itemNumber)
		{
			TestTriggers(rBoatItem, false);
			TestTriggers(rBoatItem, true);
		}

		auto probe = GetCollision(rBoatItem);
		int water = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, probe.RoomNumber);
		rBoat->Water = water;

		if (lara->Vehicle == itemNumber && laraItem->HitPoints > 0)
		{
			switch (laraItem->Animation.ActiveState)
			{
			case RBOAT_STATE_MOUNT:
			case RBOAT_STATE_JUMP_RIGHT:
			case RBOAT_STATE_JUMP_LEFT:
				break;

			default:
				drive = true;
				RubberBoatUserControl(rBoatItem, laraItem);
				break;
			}
		}
		else
		{
			if (rBoatItem->Animation.Velocity.z > RBOAT_VELOCITY_DECEL)
				rBoatItem->Animation.Velocity.z -= RBOAT_VELOCITY_DECEL;
			else
				rBoatItem->Animation.Velocity.z = 0;
		}

		ResetVehicleTurnRateY(&rBoat->TurnRate, RBOAT_TURN_RATE_DECEL);

		height = probe.Position.Floor;

		rBoatItem->Floor = height - 5;
		if (rBoat->Water == NO_HEIGHT)
			rBoat->Water = height;
		else
			rBoat->Water -= 5;

		rBoat->LeftVerticalVelocity = DoVehicleDynamics(heightFrontLeft, rBoat->LeftVerticalVelocity, RBOAT_BOUNCE_MIN, RBOAT_KICK_MAX, (int*)&frontLeft.y);
		rBoat->RightVerticalVelocity = DoVehicleDynamics(heightFrontRight, rBoat->RightVerticalVelocity, RBOAT_BOUNCE_MIN, RBOAT_KICK_MAX, (int*)&frontRight.y);
		ofs = rBoatItem->Animation.Velocity.y;
		rBoatItem->Animation.Velocity.y = DoVehicleDynamics(rBoat->Water, rBoatItem->Animation.Velocity.y, RBOAT_BOUNCE_MIN, RBOAT_KICK_MAX, (int*)&rBoatItem->Pose.Position.y);

		height = frontLeft.y + frontRight.y;
		if (height < 0)
			height = -(abs(height) / 2);
		else
			height = height / 2;

		short xRot = phd_atan(RBOAT_FRONT, rBoatItem->Pose.Position.y - height);
		short rRot = phd_atan(RBOAT_SIDE, height - frontLeft.y);

		rBoatItem->Pose.Orientation.x += ((xRot - rBoatItem->Pose.Orientation.x) / 2);
		rBoatItem->Pose.Orientation.z += ((rRot - rBoatItem->Pose.Orientation.z) / 2);

		if (!xRot && abs(rBoatItem->Pose.Orientation.x) < 4)
			rBoatItem->Pose.Orientation.x = 0;
		if (!rRot && abs(rBoatItem->Pose.Orientation.z) < 4)
			rBoatItem->Pose.Orientation.z = 0;

		if (lara->Vehicle == itemNumber)
		{
			RubberBoatAnimation(rBoatItem, laraItem, impactDirection);

			if (probe.RoomNumber != rBoatItem->RoomNumber)
			{
				ItemNewRoom(itemNumber, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			rBoatItem->Pose.Orientation.z += rBoat->LeanAngle;
			laraItem->Pose = rBoatItem->Pose;

			AnimateItem(laraItem);

			if (laraItem->HitPoints > 0)
			{
				rBoatItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex);
				rBoatItem->Animation.FrameNumber = g_Level.Anims[rBoatItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
			}

			Camera.targetElevation = -ANGLE(20.0f);
			Camera.targetDistance = SECTOR(2);
		}
		else
		{
			if (probe.RoomNumber != rBoatItem->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			rBoatItem->Pose.Orientation.z += rBoat->LeanAngle;
		}

		pitch = rBoatItem->Animation.Velocity.z;
		rBoat->Pitch += ((pitch - rBoat->Pitch) / 4);

		if (rBoatItem->Animation.Velocity.z > 8)
			SoundEffect(SFX_TR3_VEHICLE_RUBBERBOAT_MOVING, &rBoatItem->Pose, SoundEnvironment::Land, 0.5f + (float)abs(rBoat->Pitch) / (float)RBOAT_NORMAL_VELOCITY_MAX);
		else if (drive)
			SoundEffect(SFX_TR3_VEHICLE_RUBBERBOAT_IDLE, &rBoatItem->Pose, SoundEnvironment::Land, 0.5f + (float)abs(rBoat->Pitch) / (float)RBOAT_NORMAL_VELOCITY_MAX);

		if (lara->Vehicle != itemNumber)
			return;

		DoRubberBoatDismount(rBoatItem, laraItem);

		short probedRoomNumber = GetCollision(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y + 128, rBoatItem->Pose.Position.z, rBoatItem->RoomNumber).RoomNumber;
		height = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y + 128, rBoatItem->Pose.Position.z, probedRoomNumber);
		if (height > rBoatItem->Pose.Position.y + 32 || height == NO_HEIGHT)
			height = 0;
		else
			height = 1;

		auto prop = Vector3Int(0, 0, -80);
		GetJointAbsPosition(rBoatItem, &prop, 2);

		probedRoomNumber = GetCollision(prop.x, prop.y, prop.z, rBoatItem->RoomNumber).RoomNumber;

		if (rBoatItem->Animation.Velocity.z &&
			height < prop.y &&
			height != NO_HEIGHT)
		{
			TriggerRubberBoatMistEffect(prop.x, prop.y, prop.z, abs(rBoatItem->Animation.Velocity.z), rBoatItem->Pose.Orientation.y + 0x8000, 0);
			if ((GetRandomControl() & 1) == 0)
			{
				PHD_3DPOS pos;
				pos.Position.x = prop.x + (GetRandomControl() & 63) - 32;
				pos.Position.y = prop.y + (GetRandomControl() & 15);
				pos.Position.z = prop.z + (GetRandomControl() & 63) - 32;

				short roomNumber = rBoatItem->RoomNumber;
				GetFloor(pos.Position.x, pos.Position.y, pos.Position.z, &roomNumber);
				CreateBubble((Vector3Int*)&pos, roomNumber, 16, 8, 0, 0, 0, 0);
			}
		}
		else
		{
			height = GetCollision(prop.x, prop.y, prop.z, rBoatItem->RoomNumber).Position.Floor;
			if (prop.y > height &&
				!TestEnvironment(ENV_FLAG_WATER, probedRoomNumber))
			{
				GameVector pos;
				pos.x = prop.x;
				pos.y = prop.y;
				pos.z = prop.z;

				long cnt = (GetRandomControl() & 3) + 3;
				for (; cnt > 0; cnt--)
					TriggerRubberBoatMistEffect(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) * 16, rBoatItem->Pose.Orientation.y + 0x4000 + GetRandomControl(), 1);
			}
		}
	}

	void RubberBoatUserControl(ItemInfo* rBoatItem, ItemInfo* laraItem)
	{
		auto* rBoat = GetRubberBoatInfo(rBoatItem);

		float velocity = rBoatItem->Animation.Velocity.z;

		if (rBoatItem->Pose.Position.y < (rBoat->Water - CLICK(0.5f)) ||
			rBoat->Water == NO_HEIGHT)
		{
			return;
		}

		if (TrInput & (VEHICLE_IN_DISMOUNT | IN_LOOK) && velocity)
		{
			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
				velocity >= 0 &&
				velocity < RBOAT_VELOCITY_MIN)
			{
				if (!(TrInput & VEHICLE_IN_DISMOUNT) && !velocity)
					rBoatItem->Animation.Velocity.z = RBOAT_VELOCITY_MIN;
			}
			else if (velocity > RBOAT_VELOCITY_DECEL)
				rBoatItem->Animation.Velocity.z -= RBOAT_VELOCITY_DECEL;
			else
				rBoatItem->Animation.Velocity.z = 0;

			if (TrInput & IN_LOOK && !velocity)
				LookUpDown(laraItem);

			return;
		}

		if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
			ModulateVehicleTurnRateY(&rBoat->TurnRate, RBOAT_TURN_RATE_ACCEL, -RBOAT_TURN_RATE_MAX, RBOAT_TURN_RATE_MAX);
		
		if (TrInput & VEHICLE_IN_ACCELERATE)
		{
			float maxVelocity;
			if (TrInput & VEHICLE_IN_SLOW)
				maxVelocity = RBOAT_SLOW_VELOCITY_MAX;
			else if (TrInput & VEHICLE_IN_SPEED)
				maxVelocity = RBOAT_FAST_VELOCITY_MAX;
			else
				maxVelocity = RBOAT_NORMAL_VELOCITY_MAX;

			if (velocity < maxVelocity)
				rBoatItem->Animation.Velocity.z += (RBOAT_VELOCITY_ACCEL / 2) + (RBOAT_VELOCITY_ACCEL * (velocity / (maxVelocity * 2)));
			else if (velocity > (maxVelocity + RBOAT_VELOCITY_DECEL))
				rBoatItem->Animation.Velocity.z -= RBOAT_VELOCITY_DECEL;
		}
		else if (TrInput & VEHICLE_IN_REVERSE)
		{
			if (velocity > 0)
				rBoatItem->Animation.Velocity.z -= RBOAT_VELOCITY_BRAKE_DECEL;
			else if (velocity > -RBOAT_REVERSE_VELOCITY_MAX)
				rBoatItem->Animation.Velocity.z -= RBOAT_REVERSE_VELOCITY_DECEL;
		}
		else if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
			velocity >= 0 &&
			velocity < RBOAT_VELOCITY_MIN)
		{
			if (!(TrInput & VEHICLE_IN_DISMOUNT) && !velocity)
				rBoatItem->Animation.Velocity.z = RBOAT_VELOCITY_MIN;
		}
		else if (velocity > RBOAT_VELOCITY_DECEL)
			rBoatItem->Animation.Velocity.z -= RBOAT_VELOCITY_DECEL;
		else
			rBoatItem->Animation.Velocity.z = 0;
	}

	void RubberBoatAnimation(ItemInfo* rBoatItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection)
	{
		auto* rBoat = GetRubberBoatInfo(rBoatItem);

		if (laraItem->HitPoints <= 0)
		{
			if (laraItem->Animation.ActiveState != RBOAT_STATE_DEATH)
			{
				laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.TargetState = RBOAT_STATE_DEATH;
				laraItem->Animation.ActiveState = RBOAT_STATE_DEATH;
			}
		}
		else if (rBoatItem->Pose.Position.y < (rBoat->Water - CLICK(0.5f)) &&
			rBoatItem->Animation.Velocity.y > 0)
		{
			if (laraItem->Animation.ActiveState != RBOAT_STATE_FALL)
			{
				laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_LEAP_START;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = RBOAT_STATE_FALL;
				laraItem->Animation.TargetState = RBOAT_STATE_FALL;
			}
		}
		else if (impactDirection != VehicleImpactDirection::None)
			DoRubberBoatImpact(rBoatItem, laraItem, impactDirection);
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case RBOAT_STATE_IDLE:
				if (TrInput & VEHICLE_IN_DISMOUNT)
				{
					if (rBoatItem->Animation.Velocity.z == 0)
					{
						if (TrInput & IN_RIGHT && TestRubberBoatDismount(laraItem, rBoatItem->Pose.Orientation.y + ANGLE(90.0f)))
							laraItem->Animation.TargetState = RBOAT_STATE_JUMP_RIGHT;
						else if (TrInput & IN_LEFT && TestRubberBoatDismount(laraItem, rBoatItem->Pose.Orientation.y - ANGLE(90.0f)))
							laraItem->Animation.TargetState = RBOAT_STATE_JUMP_LEFT;
					}
				}

				if (rBoatItem->Animation.Velocity.z > 0)
					laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

				break;

			case RBOAT_STATE_MOVING:
				if (rBoatItem->Animation.Velocity.z <= 0)
					laraItem->Animation.TargetState = RBOAT_STATE_IDLE;

				if (TrInput & VEHICLE_IN_RIGHT)
					laraItem->Animation.TargetState = RBOAT_STATE_TURN_RIGHT;
				else if (TrInput & VEHICLE_IN_LEFT)
					laraItem->Animation.TargetState = RBOAT_STATE_TURN_LEFT;

				break;

			case RBOAT_STATE_FALL:
				laraItem->Animation.TargetState = RBOAT_STATE_MOVING;
				break;

			case RBOAT_STATE_TURN_RIGHT:
				if (rBoatItem->Animation.Velocity.z <= 0)
					laraItem->Animation.TargetState = RBOAT_STATE_IDLE;
				else if (!(TrInput & VEHICLE_IN_RIGHT))
					laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

				break;

			case RBOAT_STATE_TURN_LEFT:
				if (rBoatItem->Animation.Velocity.z <= 0)
					laraItem->Animation.TargetState = RBOAT_STATE_IDLE;
				else if (!(TrInput & VEHICLE_IN_LEFT))
					laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

				break;
			}
		}
	}

	void DoRubberBoatMount(ItemInfo* rBoatItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE;
			laraItem->Animation.ActiveState = RBOAT_STATE_IDLE;
			laraItem->Animation.TargetState = RBOAT_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = RBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = RBOAT_STATE_MOUNT;
			break;

		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = RBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = RBOAT_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Jump:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_JUMP;
			laraItem->Animation.ActiveState = RBOAT_STATE_MOUNT;
			laraItem->Animation.TargetState = RBOAT_STATE_MOUNT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		if (laraItem->RoomNumber != rBoatItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, rBoatItem->RoomNumber);

		laraItem->Pose.Position = rBoatItem->Pose.Position;
		laraItem->Pose.Position.y -= 5;
		laraItem->Pose.Orientation = Vector3Shrt(0, rBoatItem->Pose.Orientation.y, 0);
		laraItem->Animation.IsAirborne = false;
		laraItem->Animation.Velocity.z = 0;
		laraItem->Animation.Velocity.y = 0;
		lara->Control.WaterStatus = WaterStatus::Dry;

		AnimateItem(laraItem);
	}

	bool TestRubberBoatDismount(ItemInfo* laraItem, int direction)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* sBoatItem = &g_Level.Items[lara->Vehicle];

		short angle;
		if (direction < 0)
			angle = sBoatItem->Pose.Orientation.y - ANGLE(90.0f);
		else
			angle = sBoatItem->Pose.Orientation.y + ANGLE(90.0f);

		int x = sBoatItem->Pose.Position.x + SECTOR(1) * phd_sin(angle);
		int y = sBoatItem->Pose.Position.y;
		int z = sBoatItem->Pose.Position.z + SECTOR(1) * phd_cos(angle);

		auto collResult = GetCollision(x, y, z, sBoatItem->RoomNumber);

		if ((collResult.Position.Floor - sBoatItem->Pose.Position.y) < -512)
			return false;

		if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT)
			return false;

		if ((collResult.Position.Ceiling - sBoatItem->Pose.Position.y) > -LARA_HEIGHT ||
			(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	void DoRubberBoatDismount(ItemInfo* rBoatItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if ((laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_RIGHT || laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_LEFT) &&
			laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
		{
			if (laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_LEFT)
				laraItem->Pose.Orientation.y -= ANGLE(90.0f);
			else
				laraItem->Pose.Orientation.y += ANGLE(90.0f);

			laraItem->Animation.AnimNumber = LA_JUMP_FORWARD;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = LS_JUMP_FORWARD;
			laraItem->Animation.TargetState = LS_JUMP_FORWARD;
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.z = 0;
			laraItem->Animation.IsAirborne = true;
			laraItem->Animation.Velocity.z = 20;
			laraItem->Animation.Velocity.y = -40;
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

			rBoatItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT].animIndex;
			rBoatItem->Animation.FrameNumber = g_Level.Anims[rBoatItem->Animation.AnimNumber].frameBase;
		}
	}

	void DoRubberBoatImpact(ItemInfo* rBoatItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection)
	{
		if (laraItem->Animation.ActiveState == RBOAT_STATE_IMPACT)
			return;

		switch (impactDirection)
		{
		default:
		case VehicleImpactDirection::Front:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IMPACT_FRONT;
			break;

		case VehicleImpactDirection::Back:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IMPACT_BACK;
			break;

		case VehicleImpactDirection::Left:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IMPACT_LEFT;
			break;

		case VehicleImpactDirection::Right:
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IMPACT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = RBOAT_STATE_IMPACT;
		laraItem->Animation.TargetState = RBOAT_STATE_IMPACT;

		// TODO: Impact sound?
	}

	VehicleImpactDirection RubberBoatDynamics(short itemNumber, ItemInfo* laraItem)
	{
		auto* rBoatItem = &g_Level.Items[itemNumber];
		auto* rBoat = GetRubberBoatInfo(rBoatItem);
		auto* lara = GetLaraInfo(laraItem);

		rBoatItem->Pose.Orientation.z -= rBoat->LeanAngle;

		// Get point/room collision at vehicle front and corners.
		Vector3Int frontLeftOld, frontRightOld, backLeftOld, backRightOld, frontOld;
		int heightFrontLeftOld = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, true, frontLeftOld);
		int heightFrontRightOld = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, true, frontRightOld);
		int heightBackLeftOld = GetVehicleWaterHeight(rBoatItem, -RBOAT_FRONT, -RBOAT_SIDE, true, backLeftOld);
		int heightBackRightOld = GetVehicleWaterHeight(rBoatItem, -RBOAT_FRONT, RBOAT_SIDE, true, backRightOld);
		int heightFrontOld = GetVehicleWaterHeight(rBoatItem, 1000, 0, true, frontOld);

		auto prevPos = rBoatItem->Pose.Position;

		rBoatItem->Pose.Orientation.y += rBoat->TurnRate + rBoat->ExtraRotation;
		rBoat->LeanAngle = rBoat->TurnRate * 6;

		rBoatItem->Pose.Position.z += rBoatItem->Animation.Velocity.z * phd_cos(rBoatItem->Pose.Orientation.y);
		rBoatItem->Pose.Position.x += rBoatItem->Animation.Velocity.z * phd_sin(rBoatItem->Pose.Orientation.y);
		if (rBoatItem->Animation.Velocity.z >= 0)
			rBoat->PropellerRotation += (rBoatItem->Animation.Velocity.z * ANGLE(3.0f)) + ANGLE(2.0f);
		else
			rBoat->PropellerRotation += ANGLE(33.0f);

		int slip = RBOAT_SIDE_SLIP * phd_sin(rBoatItem->Pose.Orientation.z);
		if (!slip && rBoatItem->Pose.Orientation.z)
			slip = (rBoatItem->Pose.Orientation.z > 0) ? 1 : -1;

		rBoatItem->Pose.Position.z -= slip * phd_sin(rBoatItem->Pose.Orientation.y);
		rBoatItem->Pose.Position.x += slip * phd_cos(rBoatItem->Pose.Orientation.y);

		slip = RBOAT_SLIP * phd_sin(rBoatItem->Pose.Orientation.x);
		if (!slip && rBoatItem->Pose.Orientation.x)
			slip = (rBoatItem->Pose.Orientation.x > 0) ? 1 : -1;

		rBoatItem->Pose.Position.z -= slip * phd_cos(rBoatItem->Pose.Orientation.y);
		rBoatItem->Pose.Position.x -= slip * phd_sin(rBoatItem->Pose.Orientation.y);

		// Store old 2D position to determine movement delta later.
		auto moved = Vector3Int(rBoatItem->Pose.Position.x, 0, rBoatItem->Pose.Position.z);

		// Apply shifts.

		DoRubberBoatShift(itemNumber, laraItem);

		Vector3Int frontLeft, frontRight, backRight, backLeft, front;
		short extraRot = 0;
		int heightBackLeft = GetVehicleWaterHeight(rBoatItem, -RBOAT_FRONT, -RBOAT_SIDE, false, backLeft);
		if (heightBackLeft < (backLeftOld.y - CLICK(0.5f)))
			extraRot = DoVehicleShift(rBoatItem, backLeft,backLeftOld);

		int heightBackRight = GetVehicleWaterHeight(rBoatItem, -RBOAT_FRONT, RBOAT_SIDE, false, backRight);
		if (heightBackRight < (backRightOld.y - CLICK(0.5f)))
			extraRot += DoVehicleShift(rBoatItem, backRight, backRightOld);

		int heightFrontLeft = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, false, frontLeft);
		if (heightFrontLeft < (frontLeftOld.y - CLICK(0.5f)))
			extraRot += DoVehicleShift(rBoatItem, frontLeft, frontLeftOld);

		int heightFrontRight = GetVehicleWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, false, frontRight);
		if (heightFrontRight < (frontRightOld.y - CLICK(0.5f)))
			extraRot += DoVehicleShift(rBoatItem, frontRight, frontRightOld);

		if (!slip)
		{
			int heightFront = GetVehicleWaterHeight(rBoatItem, 1000, 0, false, front);
			if (heightFront < (frontOld.y - CLICK(0.5f)))
				DoVehicleShift(rBoatItem, front, frontOld);
		}

		short roomNumber = rBoatItem->RoomNumber;
		auto floor = GetFloor(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, &roomNumber);
		int height = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, roomNumber);

		if (height == NO_HEIGHT)
			height = GetFloorHeight(floor, rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z);

		if (height < (rBoatItem->Pose.Position.y - CLICK(0.5f)))
			DoVehicleShift(rBoatItem, rBoatItem->Pose.Position, prevPos);

		DoVehicleCollision(rBoatItem, RBOAT_RADIUS);

		rBoat->ExtraRotation = extraRot;
		auto impactDirection = GetVehicleImpactDirection(rBoatItem, moved);

		if (slip || impactDirection != VehicleImpactDirection::None)
		{
			int newVelocity = (rBoatItem->Pose.Position.z - prevPos.z) * phd_cos(rBoatItem->Pose.Orientation.y) + (rBoatItem->Pose.Position.x - prevPos.x) * phd_sin(rBoatItem->Pose.Orientation.y);

			if (lara->Vehicle == itemNumber &&
				rBoatItem->Animation.Velocity.z > (RBOAT_NORMAL_VELOCITY_MAX + RBOAT_VELOCITY_ACCEL) &&
				newVelocity < rBoatItem->Animation.Velocity.z - 10)
			{
				DoDamage(laraItem, rBoatItem->Animation.Velocity.z);
				SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Pose);
				newVelocity /= 2;
				rBoatItem->Animation.Velocity /= 2;
			}

			if (slip)
			{
				if (rBoatItem->Animation.Velocity.z <= RBOAT_NORMAL_VELOCITY_MAX + 10)
					rBoatItem->Animation.Velocity.z = newVelocity;
			}
			else
			{
				if (rBoatItem->Animation.Velocity.z > 0 && newVelocity < rBoatItem->Animation.Velocity.z)
					rBoatItem->Animation.Velocity.z = newVelocity;
				else if (rBoatItem->Animation.Velocity.z < 0 && newVelocity > rBoatItem->Animation.Velocity.z)
					rBoatItem->Animation.Velocity.z = newVelocity;
			}

			if (rBoatItem->Animation.Velocity.z < -RBOAT_REVERSE_VELOCITY_MAX)
				rBoatItem->Animation.Velocity.z = -RBOAT_REVERSE_VELOCITY_MAX;
		}

		return impactDirection;
	}

	void DoRubberBoatShift(int itemNumber, ItemInfo* laraItem)
	{
		auto* boatItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		int itemNumber2 = g_Level.Rooms[boatItem->RoomNumber].itemNumber;
		while (itemNumber2 != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNumber2];

			if (item->ObjectNumber == ID_RUBBER_BOAT && itemNumber2 != itemNumber && lara->Vehicle != itemNumber2)
			{
				int x = item->Pose.Position.x - boatItem->Pose.Position.x;
				int z = item->Pose.Position.z - boatItem->Pose.Position.z;

				int distance = pow(x, 2) + pow(z, 2);
				if (distance < 1000000)
				{
					boatItem->Pose.Position.x = item->Pose.Position.x - x * 1000000 / distance;
					boatItem->Pose.Position.z = item->Pose.Position.z - z * 1000000 / distance;
				}

				return;
			}

			itemNumber2 = item->NextItem;
		}
	}

	void TriggerRubberBoatMistEffect(long x, long y, long z, long velocity, short angle, long snow)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;

		if (snow)
		{
			sptr->dR = 255;
			sptr->dG = 255;
			sptr->dB = 255;
		}
		else
		{
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}

		sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 12 - (snow * 8);
		sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = x * ((GetRandomControl() & 15) - 8);
		sptr->y = y * ((GetRandomControl() & 15) - 8);
		sptr->z = z * ((GetRandomControl() & 15) - 8);
		long zv = velocity * phd_cos(angle) / 4;
		long xv = velocity * phd_sin(angle) / 4;
		sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
		sptr->yVel = (velocity * 8) + (velocity * 4);
		sptr->zVel = zv + ((GetRandomControl() & 127) - 64);
		sptr->friction = 3;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
			sptr->rotAng = GetRandomControl() & 4095;

			if (GetRandomControl() & 1)
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			else
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

		sptr->spriteIndex = Objects[ID_EXPLOSION_SPRITES].meshIndex;

		if (!snow)
		{
			sptr->scalar = 4;
			sptr->gravity = 0;
			sptr->maxYvel = 0;
			long size = (GetRandomControl() & 7) + (velocity / 2) + 16;
		}
	}

	// DEPRECATED:
	void DrawRubberBoat(ItemInfo* rBoatItem)
	{
		/*RUBBER_BOAT_INFO *b;

		b = item->data;
		item->data = &b->propRot;
		DrawAnimatingItem(item);
		item->data = b;*/
	}
}
