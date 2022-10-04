#include "framework.h"
#include "Objects/TR3/Vehicles/quad_bike.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
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
using std::vector;

namespace TEN::Entities::Vehicles
{
	constexpr auto QBIKE_RADIUS	   = 500;
	constexpr auto QBIKE_HEIGHT	   = 512;
	constexpr auto QBIKE_FRONT	   = 550;
	constexpr auto QBIKE_BACK	   = -550;
	constexpr auto QBIKE_SIDE	   = 260;
	constexpr auto QBIKE_SLIP	   = 100;
	constexpr auto QBIKE_SLIP_SIDE = 50;
	constexpr auto DAMAGE_START	   = 140;
	constexpr auto DAMAGE_LENGTH   = 14;
	
	constexpr auto QBIKE_REVERSE_VELOCITY_ACCEL = -3 * VEHICLE_VELOCITY_SCALE;
	constexpr auto QBIKE_VELOCITY_BRAKE_DECEL   = int(2.5f * VEHICLE_VELOCITY_SCALE);

	constexpr auto QBIKE_DRIFT_VELOCITY_MIN	   = 48 * VEHICLE_VELOCITY_SCALE;
	constexpr auto QBIKE_REVERSE_VELOCITY_MAX  = -48 * VEHICLE_VELOCITY_SCALE;
	constexpr auto QBIKE_REV_VELOCITY_MAX	   = 160 * VEHICLE_VELOCITY_SCALE;
	constexpr auto QBIKE_VELOCITY_MAX		   = 160 * VEHICLE_VELOCITY_SCALE;
	constexpr auto QBIKE_DEATH_VERTICAL_VELOCITY_MIN = 240;

	constexpr auto QBIKE_STEP_HEIGHT	   = CLICK(1);
	constexpr auto QBIKE_BOUNCE			   = (QBIKE_VELOCITY_MAX / 2) / CLICK(1);
	constexpr auto QBIKE_KICK			   = -80;
	constexpr auto QBIKE_MOUNT_DISTANCE	   = CLICK(2);
	constexpr auto QBIKE_DISMOUNT_DISTANCE = 385; // Precise offset derived from animation.

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
		QBIKE_STATE_IMPACT_BACK = 11,
		QBIKE_STATE_IMPACT_FRONT = 12,
		QBIKE_STATE_IMPACT_LEFT = 13,
		QBIKE_STATE_IMPACT_RIGHT = 14,
		QBIKE_STATE_IDLE = 15,
		QBIKE_STATE_LAND = 17,
		QBIKE_STATE_STOP_SLOWLY = 18,
		QBIKE_STATE_FALL_DEATH = 19,
		QBIKE_STATE_DISMOUNT_FALL = 20,
		QBIKE_STATE_WHEELIE = 21, // Unused.
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
		QBIKE_ANIM_IMPACT_BACK = 11,
		QBIKE_ANIM_IMPACT_FRONT = 12,
		QBIKE_ANIM_IMPACT_LEFT = 13,
		QBIKE_ANIM_IMPACT_RIGHT = 14,
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
		QBIKE_ANIM_LEAP_START_2 = 25,
		QBIKE_ANIM_LEAP_CONTINUE_2 = 26,
		QBIKE_ANIM_LEAP_END_2 = 27,
		QBIKE_ANIM_LEAP_TO_FREEFALL = 28
	};

	enum QuadBikeFlags
	{
		QBIKE_FLAG_FALLING = (1 << 6),
		QBIKE_FLAG_DEAD	   = (1 << 7)
	};

	enum QuadBikeBiteIndex
	{
		QBIKE_BITE_EXHAUST_LEFT		= 0,
		QBIKE_BITE_EXHAUST_RIGHT	= 1,
		QBIKE_BITE_TYRE_FRONT_LEFT	= 2,
		QBIKE_BITE_TYRE_FRONT_RIGHT = 3,
		QBIKE_BITE_TYRE_BACK_LEFT	= 4,
		QBIKE_BITE_TYRE_BACK_RIGHT	= 5
	};

	BiteInfo QuadBikeBites[6] =
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
	const vector<VehicleDismountType> QuadBikeDismountTypes =
	{
		VehicleDismountType::Left,
		VehicleDismountType::Right,
		VehicleDismountType::Fall
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

	bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* quadBikeItem = &g_Level.Items[lara->Vehicle];
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		auto oldPos = GameVector(
			quadBikeItem->Pose.Position.x,
			quadBikeItem->Pose.Position.y,
			quadBikeItem->Pose.Position.z,
			quadBikeItem->RoomNumber
		);

		auto impactDirection = QuadDynamics(quadBikeItem, laraItem);

		auto probe = GetCollision(quadBikeItem);

		Vector3Int frontLeft, frontRight;
		auto floorHeightLeft = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, -QBIKE_SIDE, false, frontLeft);
		auto floorHeightRight = GetVehicleHeight(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, false, frontRight);

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
			impactDirection = VehicleImpactDirection::None;
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case QBIKE_STATE_MOUNT_LEFT:
			case QBIKE_STATE_MOUNT_RIGHT:
			case QBIKE_STATE_DISMOUNT_LEFT:
			case QBIKE_STATE_DISMOUNT_RIGHT:
				drive = -1;
				impactDirection = VehicleImpactDirection::None;
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

			SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_MOVE, &quadBikeItem->Pose, SoundEnvironment::Land, 0.5f + (float)abs(quadBike->Pitch) / (float)QBIKE_VELOCITY_MAX);
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

		quadBike->LeftVerticalVelocity = DoVehicleDynamics(floorHeightLeft, quadBike->LeftVerticalVelocity, QBIKE_BOUNCE, QBIKE_KICK, frontLeft.y);
		quadBike->RightVerticalVelocity = DoVehicleDynamics(floorHeightRight, quadBike->RightVerticalVelocity, QBIKE_BOUNCE, QBIKE_KICK, frontRight.y);
		quadBikeItem->Animation.Velocity.y = DoVehicleDynamics(probe.Position.Floor, quadBikeItem->Animation.Velocity.y, QBIKE_BOUNCE, QBIKE_KICK, quadBikeItem->Pose.Position.y);
		quadBike->Velocity = DoVehicleWaterMovement(quadBikeItem, laraItem, quadBike->Velocity, QBIKE_RADIUS, quadBike->TurnRate);

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

			AnimateQuadBike(quadBikeItem, laraItem, impactDirection, dead);
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
				pos = QuadBikeBites[i].Position;
				GetJointAbsPosition(quadBikeItem, &pos, QuadBikeBites[i].meshNum);
				angle = quadBikeItem->Pose.Orientation.y + ((i == 0) ? ANGLE(202.5f) : ANGLE(157.5f));
				if (quadBikeItem->Animation.Velocity.z > 32.0f)
				{
					if (quadBikeItem->Animation.Velocity.z < 64.0f)
					{
						speed = 64 - quadBikeItem->Animation.Velocity.z;
						TriggerQuadBikeExhaustSmokeEffect(pos.x, pos.y, pos.z, angle, speed, 1);
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

					TriggerQuadBikeExhaustSmokeEffect(pos.x, pos.y, pos.z, angle, speed, 0);
				}
			}
		}
		else
			quadBike->SmokeStart = 0;

		return DoQuadBikeDismount(quadBikeItem, laraItem);
	}

	int QuadUserControl(ItemInfo* quadBikeItem, int height, int* pitch)
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

			if (quadBike->Velocity)
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					!quadBike->DriftStarting &&
					(quadBike->Velocity > QBIKE_DRIFT_VELOCITY_MIN ||
						quadBike->Velocity < (-QBIKE_DRIFT_VELOCITY_MIN + (8 * VEHICLE_VELOCITY_SCALE))))
				{
					ModulateVehicleTurnRateY(quadBike->TurnRate, QBIKE_DRIFT_TURN_RATE_ACCEL, -QBIKE_DRIFT_TURN_RATE_MAX, QBIKE_DRIFT_TURN_RATE_MAX);
				}
				else
					ModulateVehicleTurnRateY(quadBike->TurnRate, QBIKE_TURN_RATE_ACCEL, -QBIKE_TURN_RATE_MAX, QBIKE_TURN_RATE_MAX);
			}

			// Reversing / braking.
			if (TrInput & (VEHICLE_IN_REVERSE | VEHICLE_IN_BRAKE))
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					(quadBike->CanStartDrift || quadBike->DriftStarting))
				{
					quadBike->DriftStarting = true;
					quadBike->Revs -= 0x200;
					if (quadBike->Revs < QBIKE_REVERSE_VELOCITY_MAX)
						quadBike->Revs = QBIKE_REVERSE_VELOCITY_MAX;
				}
				else if (quadBike->Velocity > 0)
					quadBike->Velocity -= QBIKE_VELOCITY_BRAKE_DECEL;
				else
				{
					if (TrInput & VEHICLE_IN_BRAKE)
					{
						int sign = std::copysign(1, quadBike->Velocity);
						if (abs(quadBikeItem->Animation.Velocity.z) <= (QBIKE_VELOCITY_BRAKE_DECEL / 256))
							quadBike->Velocity = 0;
						else
							quadBike->Velocity -= QBIKE_VELOCITY_BRAKE_DECEL * sign;
					}
					else if (quadBike->Velocity > QBIKE_REVERSE_VELOCITY_MAX)
						quadBike->Velocity += QBIKE_REVERSE_VELOCITY_ACCEL;
				}
			}
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_SPEED &&
					(quadBike->CanStartDrift || quadBike->DriftStarting))
				{
					quadBike->DriftStarting = true;
					quadBike->Revs += 0x200;
					if (quadBike->Revs >= QBIKE_VELOCITY_MAX)
						quadBike->Revs = QBIKE_VELOCITY_MAX;
				}
				else if (quadBike->Velocity < QBIKE_VELOCITY_MAX)
				{
					if (quadBike->Velocity < (64 * VEHICLE_VELOCITY_SCALE))
						quadBike->Velocity += (8 + ((64 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - quadBike->Velocity) / 8);
					else if (quadBike->Velocity < (112 * VEHICLE_VELOCITY_SCALE))
						quadBike->Velocity += (4 + ((112 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - quadBike->Velocity) / 16);
					else if (quadBike->Velocity < QBIKE_VELOCITY_MAX)
						quadBike->Velocity += (2 + (QBIKE_VELOCITY_MAX - quadBike->Velocity) / 8);
				}
				else
					quadBike->Velocity = QBIKE_VELOCITY_MAX;

				quadBike->Velocity -= abs(quadBikeItem->Pose.Orientation.y - quadBike->MomentumAngle) / 64;
			}

			else if (quadBike->Velocity > (1 * VEHICLE_VELOCITY_SCALE))
				quadBike->Velocity -= (1 * VEHICLE_VELOCITY_SCALE);
			else if (quadBike->Velocity < -(1 * VEHICLE_VELOCITY_SCALE))
				quadBike->Velocity += (1 * VEHICLE_VELOCITY_SCALE);
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

			quadBikeItem->Animation.Velocity.z = quadBike->Velocity / VEHICLE_VELOCITY_SCALE;

			if (quadBike->EngineRevs > 0x7000)
				quadBike->EngineRevs = -0x2000;

			int revs = 0;
			if (quadBike->Velocity < 0)
				revs = abs(quadBike->Velocity / 2);
			else if (quadBike->Velocity < (112 * VEHICLE_VELOCITY_SCALE))
				revs = -(112 * VEHICLE_VELOCITY_SCALE) + (quadBike->Velocity * ((104 * VEHICLE_VELOCITY_SCALE) - -(112 * VEHICLE_VELOCITY_SCALE))) / (112 * VEHICLE_VELOCITY_SCALE);
			else if (quadBike->Velocity <= QBIKE_VELOCITY_MAX)
				revs = -(40 * VEHICLE_VELOCITY_SCALE) + ((quadBike->Velocity - (112 * VEHICLE_VELOCITY_SCALE)) * ((112 * VEHICLE_VELOCITY_SCALE) - -(40 * VEHICLE_VELOCITY_SCALE))) / (QBIKE_VELOCITY_MAX - (112 * VEHICLE_VELOCITY_SCALE));

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

	void AnimateQuadBike(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool dead)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);

		if (quadBikeItem->Pose.Position.y != quadBikeItem->Floor &&
			laraItem->Animation.ActiveState != QBIKE_STATE_FALL &&
			laraItem->Animation.ActiveState != QBIKE_STATE_LAND &&
			laraItem->Animation.ActiveState != QBIKE_STATE_DISMOUNT_FALL &&
			!dead)
		{
			if (quadBike->Velocity < 0)
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_LEAP_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_LEAP_START_2;

			laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
			laraItem->Animation.ActiveState = QBIKE_STATE_FALL;
			laraItem->Animation.TargetState = QBIKE_STATE_FALL;
		}
		else if (impactDirection != VehicleImpactDirection::None &&
			laraItem->Animation.ActiveState != QBIKE_STATE_IMPACT_FRONT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_IMPACT_BACK &&
			laraItem->Animation.ActiveState != QBIKE_STATE_IMPACT_LEFT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_IMPACT_RIGHT &&
			laraItem->Animation.ActiveState != QBIKE_STATE_DISMOUNT_FALL &&
			quadBike->Velocity > (QBIKE_VELOCITY_MAX / 3) &&
			!dead)
		{
			DoQuadBikeImpact(quadBikeItem, laraItem, impactDirection);
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
					if (TrInput & VEHICLE_IN_LEFT && TestQuadBikeDismount(laraItem, -1))
						laraItem->Animation.TargetState = QBIKE_STATE_DISMOUNT_LEFT;
					else if (TrInput & VEHICLE_IN_RIGHT && TestQuadBikeDismount(laraItem, 1))
						laraItem->Animation.TargetState = QBIKE_STATE_DISMOUNT_RIGHT;
				}
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;

			case QBIKE_STATE_DRIVE:
				if (dead)
				{
					if (quadBike->Velocity > (QBIKE_VELOCITY_MAX / 2))
						laraItem->Animation.TargetState = QBIKE_STATE_FALL_DEATH;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_BIKE_DEATH;
				}
				else if (!(TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE)) &&
					!(quadBike->Velocity / VEHICLE_VELOCITY_SCALE))
				{
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				}
				else if (TrInput & VEHICLE_IN_LEFT &&
					!quadBike->DriftStarting)
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT &&
					!quadBike->DriftStarting)
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}
				else if (TrInput & (VEHICLE_IN_REVERSE | VEHICLE_IN_BRAKE))
				{
					if (quadBike->Velocity > (QBIKE_VELOCITY_MAX / 3 * 2))
						laraItem->Animation.TargetState = QBIKE_STATE_BRAKE;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_SLOW;
				}

				break;

			case QBIKE_STATE_BRAKE:
			case QBIKE_STATE_SLOW:
			case QBIKE_STATE_STOP_SLOWLY:
				if (!quadBikeItem->Animation.Velocity.z)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
					else
						laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}

				break;

			case QBIKE_STATE_TURN_LEFT:
				if (!quadBikeItem->Animation.Velocity.z)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_RIGHT &&
					quadBikeItem->Animation.Velocity.z > 0)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_RIGHT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_RIGHT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_LEFT &&
					quadBikeItem->Animation.Velocity.z < 0)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_LEFT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_LEFT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}
				else
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
					{
						if (!(TrInput & VEHICLE_IN_LEFT))
							laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;
					}
					else
					{
						if (!(TrInput & VEHICLE_IN_RIGHT))
							laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;
					}
				}

				break;

			case QBIKE_STATE_TURN_RIGHT:
				if ((quadBike->Velocity / VEHICLE_VELOCITY_SCALE) == 0)
					laraItem->Animation.TargetState = QBIKE_STATE_IDLE;
				else if (TrInput & VEHICLE_IN_LEFT &&
					quadBikeItem->Animation.Velocity.z > 0)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_LEFT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_LEFT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_LEFT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT &&
					quadBikeItem->Animation.Velocity.z < 0)
				{
					laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_TURN_RIGHT_START;
					laraItem->Animation.FrameNumber = GetFrameNumber(laraItem, laraItem->Animation.AnimNumber);
					laraItem->Animation.ActiveState = QBIKE_STATE_TURN_RIGHT;
					laraItem->Animation.TargetState = QBIKE_STATE_TURN_RIGHT;
				}
				else
				{
					if (quadBikeItem->Animation.Velocity.z >= 0)
					{
						if (!(TrInput & VEHICLE_IN_RIGHT))
							laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;
					}
					else
					{
						if (!(TrInput & VEHICLE_IN_LEFT))
							laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;
					}
				}

				break;

			case QBIKE_STATE_FALL:
				if (quadBikeItem->Pose.Position.y == quadBikeItem->Floor)
					laraItem->Animation.TargetState = QBIKE_STATE_LAND;
				else if (quadBikeItem->Animation.Velocity.y > QBIKE_DEATH_VERTICAL_VELOCITY_MIN)
					quadBike->Flags |= QBIKE_FLAG_FALLING;

				break;

			case QBIKE_STATE_DISMOUNT_FALL:
				break;

			case QBIKE_STATE_IMPACT_FRONT:
			case QBIKE_STATE_IMPACT_BACK:
			case QBIKE_STATE_IMPACT_LEFT:
			case QBIKE_STATE_IMPACT_RIGHT:
				if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = QBIKE_STATE_DRIVE;

				break;
			}
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

	int TestQuadBikeDismount(ItemInfo* laraItem, int direction)
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

	bool DoQuadBikeDismount(ItemInfo* quadBikeItem, ItemInfo* laraItem)
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

			if (laraItem->Animation.ActiveState == QBIKE_STATE_DISMOUNT_FALL)
			{
				auto pos = Vector3Int();

				SetAnimation(laraItem, LA_FREEFALL);
				GetJointAbsPosition(laraItem, &pos, LM_HIPS);

				laraItem->Pose.Position = pos;
				laraItem->Animation.IsAirborne = true;
				laraItem->Animation.Velocity.y = quadBikeItem->Animation.Velocity.y;
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
				laraItem->Animation.Velocity.z = 0;
				laraItem->Animation.Velocity.y = DAMAGE_START + DAMAGE_LENGTH;
				quadBike->Flags |= QBIKE_FLAG_DEAD;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	void DoQuadBikeImpact(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection)
	{
		switch (impactDirection)
		{
		default:
		case VehicleImpactDirection::Front:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_IMPACT_FRONT;
			laraItem->Animation.ActiveState = QBIKE_STATE_IMPACT_FRONT;
			laraItem->Animation.TargetState = QBIKE_STATE_IMPACT_FRONT;
			break;

		case VehicleImpactDirection::Back:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_IMPACT_BACK;
			laraItem->Animation.ActiveState = QBIKE_STATE_IMPACT_BACK;
			laraItem->Animation.TargetState = QBIKE_STATE_IMPACT_BACK;
			break;

		case VehicleImpactDirection::Left:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_IMPACT_LEFT;
			laraItem->Animation.ActiveState = QBIKE_STATE_IMPACT_LEFT;
			laraItem->Animation.TargetState = QBIKE_STATE_IMPACT_LEFT;
			break;

		case VehicleImpactDirection::Right:
			laraItem->Animation.AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QBIKE_ANIM_IMPACT_RIGHT;
			laraItem->Animation.ActiveState = QBIKE_STATE_IMPACT_RIGHT;
			laraItem->Animation.TargetState = QBIKE_STATE_IMPACT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_FRONT_IMPACT, &quadBikeItem->Pose);
	}

	VehicleImpactDirection QuadDynamics(ItemInfo* quadBikeItem, ItemInfo* laraItem)
	{
		auto* quadBike = GetQuadBikeInfo(quadBikeItem);
		auto* lara = GetLaraInfo(laraItem);

		// Enable dismounts.
		quadBike->NoDismount = false;

		// Get point/room collision at vehicle corners.
		auto prevPointFrontLeft = GetVehicleCollision(quadBikeItem, QBIKE_FRONT, -QBIKE_SIDE, true);
		auto prevPointFrontRight = GetVehicleCollision(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, true);
		auto prevPointBackLeft = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT, -QBIKE_SIDE, true);
		auto prevPointBackRight = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT, QBIKE_SIDE, true);

		// Get point/room collision defining base of vehicle chassis (substitute for rigid body).
		auto prevPointLeft = GetVehicleCollision(quadBikeItem, 0, -QBIKE_SIDE, true);
		auto prevPointRight = GetVehicleCollision(quadBikeItem, 0, QBIKE_SIDE, true);
		auto prevPointCenterFrontLeft = GetVehicleCollision(quadBikeItem, QBIKE_FRONT / 2, -QBIKE_SIDE, true);
		auto prevPointCenterFrontRight = GetVehicleCollision(quadBikeItem, QBIKE_FRONT / 2, QBIKE_SIDE, true);
		auto prevPointCenterBackLeft = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT / 2, -QBIKE_SIDE, true);
		auto prevPointCenterBackRight = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT / 2, QBIKE_SIDE, true);

		auto prevPos = quadBikeItem->Pose.Position;

		// Apply rotations and determine angle of momentum.
		if (quadBikeItem->Pose.Position.y > (quadBikeItem->Floor - CLICK(1)))
		{
			ResetVehicleTurnRateY(quadBike->TurnRate, QBIKE_TURN_RATE_DECEL);
			quadBikeItem->Pose.Orientation.y += quadBike->TurnRate + quadBike->ExtraRotation;

			short momentum = QBIKE_MOMENTUM_TURN_RATE_MIN - (((((QBIKE_MOMENTUM_TURN_RATE_MIN - QBIKE_MOMENTUM_TURN_RATE_MAX) * 256) / QBIKE_VELOCITY_MAX) * quadBike->Velocity) / 256);
			if (!(TrInput & VEHICLE_IN_ACCELERATE) && quadBike->Velocity > 0)
				momentum += momentum / 4;

			short rotation = quadBikeItem->Pose.Orientation.y - quadBike->MomentumAngle;
			if (rotation < -QBIKE_MOMENTUM_TURN_RATE_MAX)
			{
				if (rotation < -QBIKE_MOMENTUM_TURN_RATE_MAX2)
				{
					rotation = -QBIKE_MOMENTUM_TURN_RATE_MAX2;
					quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y - rotation;
				}
				else
					quadBike->MomentumAngle -= momentum;
			}
			else if (rotation > QBIKE_MOMENTUM_TURN_RATE_MAX)
			{
				if (rotation > QBIKE_MOMENTUM_TURN_RATE_MAX2)
				{
					rotation = QBIKE_MOMENTUM_TURN_RATE_MAX2;
					quadBike->MomentumAngle = quadBikeItem->Pose.Orientation.y - rotation;
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
			speed = quadBikeItem->Animation.Velocity.z * phd_cos(quadBikeItem->Pose.Orientation.x);
		else
			speed = quadBikeItem->Animation.Velocity.z;

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

		// Store old 2D position to determine movement delta later.
		auto moved = Vector3Int(quadBikeItem->Pose.Position.x, 0, quadBikeItem->Pose.Position.z);

		// Process entity collision.
		if (!(quadBikeItem->Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(quadBikeItem, QBIKE_RADIUS);

		// Apply shifts.
		short extraRot = 0;
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointFrontLeft, QBIKE_HEIGHT, QBIKE_FRONT, -QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointCenterFrontLeft, QBIKE_HEIGHT, QBIKE_FRONT / 2, -QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointLeft, QBIKE_HEIGHT, 0, -QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointCenterBackLeft, QBIKE_HEIGHT, -QBIKE_FRONT / 2, -QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);

		short rotAdd = 0;
		auto pointBackLeft = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT, -QBIKE_SIDE, false);
		if (pointBackLeft.FloorHeight < (prevPointBackLeft.Position.y - CLICK(1)))
		{
			rotAdd = DoVehicleShift(quadBikeItem, pointBackLeft.Position, prevPointBackLeft.Position);
			if ((rotAdd > 0 && extraRot >= 0) || (rotAdd < 0 && extraRot <= 0))
				extraRot += rotAdd;
		}

		auto pointFrontRight = GetVehicleCollision(quadBikeItem, QBIKE_FRONT, QBIKE_SIDE, false);
		if (pointFrontRight.FloorHeight < (prevPointFrontRight.Position.y - CLICK(1)))
		{
			rotAdd = DoVehicleShift(quadBikeItem, pointFrontRight.Position, prevPointFrontRight.Position);
			if ((rotAdd > 0 && extraRot >= 0) || (rotAdd < 0 && extraRot <= 0))
				extraRot += rotAdd;
		}

		CalculateVehicleShift(quadBikeItem, extraRot, prevPointCenterFrontRight, QBIKE_HEIGHT, QBIKE_FRONT / 2, QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointRight, QBIKE_HEIGHT, 0, QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointCenterBackRight, QBIKE_HEIGHT, -QBIKE_FRONT / 2, QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);
		CalculateVehicleShift(quadBikeItem, extraRot, prevPointCenterBackRight, QBIKE_HEIGHT, -QBIKE_FRONT / 2, QBIKE_SIDE, QBIKE_STEP_HEIGHT, false);

		auto pointBackRight = GetVehicleCollision(quadBikeItem, -QBIKE_FRONT, QBIKE_SIDE, false);
		if (pointBackRight.FloorHeight < (prevPointBackRight.Position.y - CLICK(1)))
		{
			rotAdd = DoVehicleShift(quadBikeItem, pointBackRight.Position, prevPointBackRight.Position);
			if ((rotAdd > 0 && extraRot >= 0) || (rotAdd < 0 && extraRot <= 0))
				extraRot += rotAdd;
		}

		probe = GetCollision(quadBikeItem);
		if (probe.Position.Floor < quadBikeItem->Pose.Position.y - CLICK(1))
			DoVehicleShift(quadBikeItem, quadBikeItem->Pose.Position, prevPos);

		quadBike->ExtraRotation = extraRot;

		// Determine whether wall impact occurred and affect vehicle accordingly.
		auto impactDirection = GetVehicleImpactDirection(quadBikeItem, moved);
		if (impactDirection != VehicleImpactDirection::None)
		{
			int newVelocity = (quadBikeItem->Pose.Position.z - prevPos.z) * phd_cos(quadBike->MomentumAngle) + (quadBikeItem->Pose.Position.x - prevPos.x) * phd_sin(quadBike->MomentumAngle);
			newVelocity *= VEHICLE_VELOCITY_SCALE;

			if (&g_Level.Items[lara->Vehicle] == quadBikeItem &&
				quadBike->Velocity == QBIKE_VELOCITY_MAX &&
				newVelocity < (quadBike->Velocity - 10))
			{
				DoDamage(laraItem, (quadBike->Velocity - newVelocity) / 128);
			}

			if (quadBike->Velocity > 0 && newVelocity < quadBike->Velocity)
				quadBike->Velocity = (newVelocity < 0) ? 0 : newVelocity;

			else if (quadBike->Velocity < 0 && newVelocity > quadBike->Velocity)
				quadBike->Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (quadBike->Velocity < QBIKE_REVERSE_VELOCITY_MAX)
				quadBike->Velocity = QBIKE_REVERSE_VELOCITY_MAX;
		}

		return impactDirection;
	}

	void TriggerQuadBikeExhaustSmokeEffect(int x, int y, int z, short angle, int speed, int moving)
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
}
