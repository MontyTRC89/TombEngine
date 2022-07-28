#include "framework.h"
#include "Objects/TR2/Vehicles/skidoo.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/effects/simple_particle.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

using namespace TEN::Input;
using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Vehicles
{
	constexpr auto SKIDOO_RADIUS = 500;
	constexpr auto SKIDOO_FRONT = 550;
	constexpr auto SKIDOO_SIDE = 260;
	constexpr auto SKIDOO_SLIP = 100;
	constexpr auto SKIDOO_SLIP_SIDE = 50;
	constexpr auto SKIDOO_SNOW = 500; // Unused.
	constexpr auto SKIDOO_MOUNT_DISTANCE = CLICK(2);
	constexpr auto SKIDOO_DISMOUNT_DISTANCE = 295;

	constexpr auto SKIDOO_VELOCITY_ACCEL = 10;
	constexpr auto SKIDOO_VELOCITY_DECEL = 2;
	constexpr auto SKIDOO_VELOCITY_BRAKE_DECEL = 5;
	constexpr auto SKIDOO_REVERSE_VELOCITY_ACCEL = 5;
	constexpr auto SKIDOO_KICK_MAX = -80;

	constexpr auto SKIDOO_SLOW_VELOCITY_MAX = 50;
	constexpr auto SKIDOO_NORMAL_VELOCITY_MAX = 100;
	constexpr auto SKIDOO_FAST_VELOCITY_MAX = 150;
	constexpr auto SKIDOO_TURN_VELOCITY_MAX = 15;
	constexpr auto SKIDOO_REVERSE_VELOCITY_MAX = 30;

	constexpr auto SKIDOO_STEP_HEIGHT_MAX = CLICK(1); // Unused.
	constexpr auto SKIDOO_MIN_BOUNCE = (SKIDOO_NORMAL_VELOCITY_MAX / 2) / 256;

	constexpr auto SKIDOO_DAMAGE_START = 140;
	constexpr auto SKIDOO_DAMAGE_LENGTH = 14;

	#define SKIDOO_TURN_RATE_ACCEL			ANGLE(2.5f)
	#define SKIDOO_TURN_RATE_DECEL			ANGLE(2.0f)
	#define SKIDOO_TURN_RATE_MAX			ANGLE(6.0f)
	#define SKIDOO_MOMENTUM_TURN_RATE_ACCEL	ANGLE(3.0f)
	#define SKIDOO_MOMENTUM_TURN_RATE_MAX	ANGLE(150.0f)

	enum SkidooState
	{
		SKIDOO_STATE_DRIVE = 0,
		SKIDOO_STATE_MOUNT = 1,
		SKIDOO_STATE_LEFT = 2,
		SKIDOO_STATE_RIGHT = 3,
		SKIDOO_STATE_FALL = 4,
		SKIDOO_STATE_HIT = 5,
		SKIDOO_STATE_MOUNT_LEFT = 6,
		SKIDOO_STATE_DISMOUNT_LEFT = 7,
		SKIDOO_STATE_IDLE = 8,
		SKIDOO_STATE_DISMOUNT_RIGHT = 9,
		SKIDOO_STATE_JUMP_OFF = 10,
		SKIDOO_STATE_DEATH = 11,
		SKIDOO_STATE_FALLOFF = 12
	};

	enum SkidooAnim
	{
		SKIDOO_ANIM_DRIVE = 0,
		SKIDOO_ANIM_MOUNT_RIGHT = 1,
		SKIDOO_ANIM_TURN_LEFT_START = 2,
		SKIDOO_ANIM_TURN_LEFT_CONTINUE = 3,
		SKIDOO_ANIM_TURN_LEFT_END = 4,
		SKIDOO_ANIM_TURN_RIGHT_START = 5,
		SKIDOO_ANIM_TURN_RIGHT_CONTINUE = 6,
		SKIDOO_ANIM_TURN_RIGHT_END = 7,
		SKIDOO_ANIM_LEAP_START = 8,
		SKIDOO_ANIM_LEAP_END = 9,
		SKIDOO_ANIM_LEAP_CONTINUE = 10,
		SKIDOO_ANIM_HIT_LEFT = 11,
		SKIDOO_ANIM_HIT_RIGHT = 12,
		SKIDOO_ANIM_HIT_FRONT = 13,
		SKIDOO_ANIM_HIT_BACK = 14,
		SKIDOO_ANIM_IDLE = 15,
		SKIDOO_ANIM_DISMOUNT_RIGHT = 16,
		SKIDOO_ANIM_UNK = 17, // TODO
		SKIDOO_ANIM_MOUNT_LEFT = 18,
		SKIDOO_ANIM_DISMOUNT_LEFT = 19,
		SKIDOO_ANIM_FALL_OFF = 20,
		SKIDOO_ANIM_IDLE_DEATH = 21,
		SKIDOO_ANIM_FALL_DEATH = 22
	};

	const vector<VehicleMountType> SkidooMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	SkidooInfo* GetSkidooInfo(ItemInfo* skidooItem)
	{
		return (SkidooInfo*)skidooItem->Data;
	}

	void InitialiseSkidoo(short itemNumber)
	{
		auto* skidooItem = &g_Level.Items[itemNumber];
		skidooItem->Data = SkidooInfo();
		auto* skidoo = GetSkidooInfo(skidooItem);

		if (skidooItem->Status != ITEM_ACTIVE)
		{
			AddActiveItem(itemNumber);
			skidooItem->Status = ITEM_ACTIVE;
		}

		if (skidooItem->ObjectNumber == ID_SNOWMOBILE_GUN)
			skidoo->Armed = true;

		skidoo->MomentumAngle = skidooItem->Pose.Orientation.y;
	}

	void SkidooPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* skidooItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(skidooItem, laraItem, coll, SkidooMountTypes, SKIDOO_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
			ObjectCollision(itemNumber, laraItem, coll);
		else
		{
			lara->Vehicle = itemNumber;
			DoSkidooMount(skidooItem, laraItem, mountType);
		}
	}

	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IDLE;
			laraItem->Animation.ActiveState = SKIDOO_STATE_IDLE;
			laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = SKIDOO_STATE_MOUNT;
			laraItem->Animation.TargetState = SKIDOO_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = SKIDOO_STATE_MOUNT;
			laraItem->Animation.TargetState = SKIDOO_STATE_MOUNT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		laraItem->Pose.Position = skidooItem->Pose.Position;
		laraItem->Pose.Orientation = Vector3Shrt(0, skidooItem->Pose.Orientation.y, 0);
		lara->Control.HandStatus = HandStatus::Busy;
		skidooItem->Collidable = true;
	}

	bool TestSkidooDismountOK(ItemInfo* skidooItem, int direction)
	{
		short angle;
		if (direction == SKIDOO_STATE_DISMOUNT_LEFT)
			angle = skidooItem->Pose.Orientation.y + ANGLE(90.0f);
		else
			angle = skidooItem->Pose.Orientation.y - ANGLE(90.0f);

		auto probe = GetCollision(skidooItem, angle, -SKIDOO_DISMOUNT_DISTANCE);

		if ((probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT) ||
			abs(probe.Position.Floor - skidooItem->Pose.Position.y) > CLICK(2) ||
			((probe.Position.Ceiling - skidooItem->Pose.Position.y) > -LARA_HEIGHT ||
				(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
		{
			return false;
		}

		return true;
	}

	bool TestSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Vehicle != NO_ITEM)
		{
			if ((laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT) &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT)
					laraItem->Pose.Orientation.y += ANGLE(90.0f);
				else
					laraItem->Pose.Orientation.y -= ANGLE(90.0f);

				SetAnimation(laraItem, LA_STAND_IDLE);
				TranslateItem(laraItem, laraItem->Pose.Orientation.y, -SKIDOO_DISMOUNT_DISTANCE);
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
			}
			else if (laraItem->Animation.ActiveState == SKIDOO_STATE_JUMP_OFF &&
				(skidooItem->Pose.Position.y == skidooItem->Floor || TestLastFrame(laraItem)))
			{
				SetAnimation(laraItem, LA_FREEFALL);

				if (skidooItem->Pose.Position.y == skidooItem->Floor)
				{
					laraItem->Animation.TargetState = LS_DEATH;
					laraItem->Animation.Velocity = 0;
					laraItem->Animation.VerticalVelocity = SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH;
					ExplodeVehicle(laraItem, skidooItem);
				}
				else
				{
					laraItem->Animation.TargetState = LS_FREEFALL;
					laraItem->Pose.Position.y -= 200;
					laraItem->Animation.Velocity = skidooItem->Animation.Velocity;
					laraItem->Animation.VerticalVelocity = skidooItem->Animation.VerticalVelocity;
					SoundEffect(SFX_TR4_LARA_FALL, &laraItem->Pose);
				}

				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				laraItem->Animation.IsAirborne = true;
				lara->Control.MoveAngle = skidooItem->Pose.Orientation.y;
				lara->Control.HandStatus = HandStatus::Free;
				skidooItem->Collidable = false;
				skidooItem->Flags |= IFLAG_INVISIBLE;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* skidooItem = &g_Level.Items[lara->Vehicle];
		auto* skidoo = GetSkidooInfo(skidooItem);

		Vector3Int frontLeft, frontRight;
		auto collide = SkidooDynamics(skidooItem, laraItem);
		auto heightFrontLeft = GetVehicleHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true, &frontLeft);
		auto heightFrontRight = GetVehicleHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true, &frontRight);

		auto probe = GetCollision(skidooItem);

		TestTriggers(skidooItem, true);
		TestTriggers(skidooItem, false);

		bool dead = false;
		int drive = 0;

		if (laraItem->HitPoints <= 0)
		{
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
			dead = true;
		}
		else if (laraItem->Animation.ActiveState == SKIDOO_STATE_JUMP_OFF)
		{
			dead = true;
			collide = 0;
		}

		int height = probe.Position.Floor;
		int pitch = 0;

		if (skidooItem->Flags & IFLAG_INVISIBLE)
		{
			drive = 0;
			collide = 0;
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SKIDOO_STATE_MOUNT:
			case SKIDOO_STATE_DISMOUNT_RIGHT:
			case SKIDOO_STATE_DISMOUNT_LEFT:
			case SKIDOO_STATE_JUMP_OFF:
				drive = -1;
				collide = 0;

				break;

			default:
				drive = SkidooUserControl(skidooItem, laraItem, height, &pitch);
				break;
			}
		}

		bool banditSkidoo = skidoo->Armed;
		if (drive > 0)
		{
			skidoo->TrackMesh = ((skidoo->TrackMesh & 3) == 1) ? 2 : 1;
			skidoo->Pitch += (pitch - skidoo->Pitch) / 4;

			auto pitch = std::clamp(0.5f + (float)abs(skidoo->Pitch) / (float)SKIDOO_NORMAL_VELOCITY_MAX, 0.6f, 1.4f);
			SoundEffect(skidoo->Pitch ? SFX_TR2_VEHICLE_SNOWMOBILE_MOVING : SFX_TR2_VEHICLE_SNOWMOBILE_ACCELERATE, &skidooItem->Pose, SoundEnvironment::Land, pitch);
		}
		else
		{
			skidoo->TrackMesh = 0;
			if (!drive)
				SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidooItem->Pose);
			skidoo->Pitch = 0;
		}
		skidooItem->Floor = height;

		skidoo->LeftVerticalVelocity = DoSkidooDynamics(heightFrontLeft, skidoo->LeftVerticalVelocity, (int*)&frontLeft.y);
		skidoo->RightVerticalVelocity = DoSkidooDynamics(heightFrontRight, skidoo->RightVerticalVelocity, (int*)&frontRight.y);
		skidooItem->Animation.VerticalVelocity = DoSkidooDynamics(height, skidooItem->Animation.VerticalVelocity, (int*)&skidooItem->Pose.Position.y);
		skidooItem->Animation.Velocity = DoVehicleWaterMovement(skidooItem, laraItem, skidooItem->Animation.Velocity, SKIDOO_RADIUS, &skidoo->TurnRate);

		height = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(SKIDOO_FRONT, skidooItem->Pose.Position.y - height);
		short zRot = phd_atan(SKIDOO_SIDE, height - frontLeft.y);

		skidooItem->Pose.Orientation.x += (xRot - skidooItem->Pose.Orientation.x) / 2;
		skidooItem->Pose.Orientation.z += (zRot - skidooItem->Pose.Orientation.z) / 2;

		if (skidooItem->Flags & IFLAG_INVISIBLE)
		{
			if (probe.RoomNumber != skidooItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			AnimateItem(laraItem);

			if (skidooItem->Pose.Position.y == skidooItem->Floor)
				ExplodeVehicle(laraItem, skidooItem);

			return false;
		}

		SkidooAnimation(skidooItem, laraItem, collide, dead);

		if (probe.RoomNumber != skidooItem->RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALLOFF)
		{
			laraItem->Pose.Position = skidooItem->Pose.Position;
			laraItem->Pose.Orientation.y = skidooItem->Pose.Orientation.y;

			if (drive >= 0)
			{
				laraItem->Pose.Orientation.x = skidooItem->Pose.Orientation.x;
				laraItem->Pose.Orientation.z = skidooItem->Pose.Orientation.z;
			}
			else
				laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;
		}
		else
			laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;

		AnimateItem(laraItem);

		if (!dead && drive >= 0 && banditSkidoo)
			SkidooGuns(skidooItem, laraItem);

		if (!dead)
		{
			skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
			skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
		}
		else
		{
			skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_ANIM_IDLE;
			skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase;
		}

		if (skidooItem->Animation.Velocity && skidooItem->Floor == skidooItem->Pose.Position.y)
		{
			DoSnowEffect(skidooItem);

			if (skidooItem->Animation.Velocity < 50)
				DoSnowEffect(skidooItem);
		}

		return TestSkidooDismount(skidooItem, laraItem);
	}

	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		int maxVelocity = 0;
		bool drive = false;

		if (skidooItem->Pose.Position.y >= (height - CLICK(1)))
		{
			*pitch = skidooItem->Animation.Velocity + (height - skidooItem->Pose.Position.y);

			if (TrInput & IN_LOOK && skidooItem->Animation.Velocity == 0)
				LookUpDown(laraItem);

			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
				ModulateVehicleTurnRateY(&skidoo->TurnRate, SKIDOO_TURN_RATE_ACCEL, -SKIDOO_TURN_RATE_MAX, SKIDOO_TURN_RATE_MAX);

			if (TrInput & VEHICLE_IN_REVERSE)
			{
				if (skidooItem->Animation.Velocity > 0)
					skidooItem->Animation.Velocity -= SKIDOO_VELOCITY_BRAKE_DECEL;
				else
				{
					if (skidooItem->Animation.Velocity > -SKIDOO_REVERSE_VELOCITY_MAX)
						skidooItem->Animation.Velocity -= SKIDOO_REVERSE_VELOCITY_ACCEL;

					drive = true;
				}
			}
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_SPEED)
					maxVelocity = SKIDOO_FAST_VELOCITY_MAX;
				else if (TrInput & VEHICLE_IN_SLOW)
					maxVelocity = SKIDOO_SLOW_VELOCITY_MAX;
				else
					maxVelocity = SKIDOO_NORMAL_VELOCITY_MAX;

				if (skidooItem->Animation.Velocity < maxVelocity)
					skidooItem->Animation.Velocity += (SKIDOO_VELOCITY_ACCEL / 2) + (SKIDOO_VELOCITY_ACCEL * (skidooItem->Animation.Velocity / (2 * maxVelocity)));
				else if (skidooItem->Animation.Velocity > (maxVelocity + SKIDOO_VELOCITY_DECEL))
					skidooItem->Animation.Velocity -= SKIDOO_VELOCITY_DECEL;

				drive = true;
			}
			else if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
				skidooItem->Animation.Velocity >= 0 &&
				skidooItem->Animation.Velocity < SKIDOO_TURN_VELOCITY_MAX)
			{
				skidooItem->Animation.Velocity = SKIDOO_TURN_VELOCITY_MAX;
				drive = true;
			}
			else if (skidooItem->Animation.Velocity > SKIDOO_VELOCITY_DECEL)
			{
				skidooItem->Animation.Velocity -= SKIDOO_VELOCITY_DECEL;

				if ((GetRandomControl() & 0x7f) < skidooItem->Animation.Velocity)
					drive = true;
			}
			else
				skidooItem->Animation.Velocity = 0;
		}
		else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
		{
			*pitch = skidoo->Pitch + 50;
			drive = true;
		}

		return drive;
	}

	void SkidooAnimation(ItemInfo* skidooItem, ItemInfo* laraItem, int collide, bool dead)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
			skidooItem->Animation.VerticalVelocity > 0 &&
			skidooItem->Pose.Position.y != skidooItem->Floor &&
			!dead)
		{
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_LEAP_START;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = SKIDOO_STATE_FALL;
			laraItem->Animation.TargetState = SKIDOO_STATE_FALL;
		}
		else if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
			collide && !dead)
		{
			if (laraItem->Animation.ActiveState != SKIDOO_STATE_HIT)
			{
				if (collide == SKIDOO_ANIM_HIT_FRONT)
					SoundEffect(SFX_TR2_VEHICLE_IMPACT1, &skidooItem->Pose);
				else
					SoundEffect(SFX_TR2_VEHICLE_IMPACT2, &skidooItem->Pose);

				laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + collide;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SKIDOO_STATE_HIT;
			}
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SKIDOO_STATE_IDLE:

				if (dead)
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_DEATH;
					break;
				}

				laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;

				if (TrInput & VEHICLE_IN_DISMOUNT)
				{
					if (TrInput & VEHICLE_IN_RIGHT &&
						TestSkidooDismountOK(skidooItem, SKIDOO_STATE_DISMOUNT_RIGHT))
					{
						laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_RIGHT;
						skidooItem->Animation.Velocity = 0;
					}
					else if (TrInput & VEHICLE_IN_LEFT &&
						TestSkidooDismountOK(skidooItem, SKIDOO_STATE_DISMOUNT_LEFT))
					{
						laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_LEFT;
						skidooItem->Animation.Velocity = 0;
					}
				}
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					if (skidooItem->Animation.Velocity >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (skidooItem->Animation.Velocity >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
				}
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;

				break;

			case SKIDOO_STATE_DRIVE:
				if (skidooItem->Animation.Velocity == 0)
					laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;

				if (dead)
					laraItem->Animation.TargetState = SKIDOO_STATE_FALLOFF;
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					if (skidooItem->Animation.Velocity >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (skidooItem->Animation.Velocity >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
				}

				break;

			case SKIDOO_STATE_LEFT:
				if (skidooItem->Animation.Velocity >= 0)
				{
					if (!(TrInput & VEHICLE_IN_LEFT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!(TrInput & VEHICLE_IN_RIGHT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_RIGHT:
				if (skidooItem->Animation.Velocity >= 0)
				{
					if (!(TrInput & VEHICLE_IN_RIGHT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!(TrInput & VEHICLE_IN_LEFT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_FALL:
				if (skidooItem->Animation.VerticalVelocity <= 0 ||
					skidoo->LeftVerticalVelocity <= 0 ||
					skidoo->RightVerticalVelocity <= 0)
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
					SoundEffect(SFX_TR2_VEHICLE_IMPACT3, &skidooItem->Pose);
				}
				else if (skidooItem->Animation.VerticalVelocity > (SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH))
					laraItem->Animation.TargetState = SKIDOO_STATE_JUMP_OFF;

				break;
			}
		}
	}

	int GetSkidooCollisionAnim(ItemInfo* skidooItem, Vector3Int* moved)
	{
		moved->x = skidooItem->Pose.Position.x - moved->x;
		moved->z = skidooItem->Pose.Position.z - moved->z;

		if (moved->x || moved->z)
		{
			float sinY = phd_sin(skidooItem->Pose.Orientation.y);
			float cosY = phd_cos(skidooItem->Pose.Orientation.y);

			int front = (moved->z * cosY) + (moved->x * sinY);
			int side = (moved->z * -sinY) + (moved->x * cosY);

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return SKIDOO_ANIM_HIT_BACK;
				else
					return SKIDOO_ANIM_HIT_FRONT;
			}
			else
			{
				if (side > 0)
					return SKIDOO_ANIM_HIT_LEFT;
				else
					return SKIDOO_ANIM_HIT_RIGHT;
			}
		}

		return 0;
	}

	void SkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);
		auto* lara = GetLaraInfo(laraItem);
		auto* weapon = &Weapons[(int)LaraWeaponType::Snowmobile];

		LaraGetNewTarget(laraItem, weapon);
		AimWeapon(laraItem, weapon, &lara->RightArm);

		if (TrInput & VEHICLE_IN_FIRE && !skidooItem->ItemFlags[0])
		{
			auto angles = Vector3Shrt(
				lara->RightArm.Orientation.x,
				lara->RightArm.Orientation.y + laraItem->Pose.Orientation.y,
				0
			);

			if ((int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles) +
				(int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles))
			{
				skidoo->FlashTimer = 2;
				SoundEffect(weapon->SampleNum, &laraItem->Pose);
				skidooItem->ItemFlags[0] = 4;
			}
		}

		if (skidooItem->ItemFlags[0])
			skidooItem->ItemFlags[0]--;
	}

	void DoSnowEffect(ItemInfo* skidooItem)
	{
		auto material = GetCollision(skidooItem).BottomBlock->Material;
		if (material != FLOOR_MATERIAL::Ice && material != FLOOR_MATERIAL::Snow)
			return;

		TEN::Effects::TriggerSnowmobileSnow(skidooItem);
	}

	int DoSkidooDynamics(int height, int verticalVelocity, int* y)
	{
		// Grounded.
		if (height > *y)
		{
			*y += verticalVelocity;
			if (*y > height - SKIDOO_MIN_BOUNCE)
			{
				*y = height;
				verticalVelocity = 0;
			}
			else
				verticalVelocity += GRAVITY;
		}
		// Airborne.
		else
		{
			int kick = (height - *y) * 4;
			if (kick < SKIDOO_KICK_MAX)
				kick = SKIDOO_KICK_MAX;

			verticalVelocity += (kick - verticalVelocity) / 8;

			if (*y > height)
				*y = height;
		}

		return verticalVelocity;
	}

	short DoSkidooShift(ItemInfo* skidooItem, Vector3Int* pos, Vector3Int* old)
	{
		int	x = pos->x / SECTOR(1);
		int z = pos->z / SECTOR(1);
		int xOld = old->x / SECTOR(1);
		int zOld = old->z / SECTOR(1);
		int shiftX = pos->x & (SECTOR(1) - 1);
		int shiftZ = pos->z & (SECTOR(1) - 1);

		if (x == xOld)
		{
			if (z == zOld)
			{
				skidooItem->Pose.Position.z += (old->z - pos->z);
				skidooItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > zOld)
			{
				skidooItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - skidooItem->Pose.Position.x);
			}
			else
			{
				skidooItem->Pose.Position.z += SECTOR(1) - shiftZ;
				return (skidooItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == zOld)
		{
			if (x > xOld)
			{
				skidooItem->Pose.Position.x -= shiftX + 1;
				return (skidooItem->Pose.Position.z - pos->z);
			}
			else
			{
				skidooItem->Pose.Position.x += SECTOR(1) - shiftX;
				return (pos->z - skidooItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			auto probe = GetCollision(old->x, pos->y, pos->z, skidooItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = SECTOR(1) - shiftZ;
			}

			probe = GetCollision(pos->x, pos->y, old->z, skidooItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = SECTOR(1) - shiftX;
			}

			if (x && z)
			{
				skidooItem->Pose.Position.z += z;
				skidooItem->Pose.Position.x += x;
				skidooItem->Animation.Velocity -= 50;
			}
			else if (z)
			{
				skidooItem->Pose.Position.z += z;
				skidooItem->Animation.Velocity -= 50;

				if (z > 0)
					return (skidooItem->Pose.Position.x - pos->x);
				else
					return (pos->x - skidooItem->Pose.Position.x);
			}
			else if (x)
			{
				skidooItem->Pose.Position.x += x;
				skidooItem->Animation.Velocity -= 50;

				if (x > 0)
					return (pos->z - skidooItem->Pose.Position.z);
				else
					return (skidooItem->Pose.Position.z - pos->z);
			}
			else
			{
				skidooItem->Pose.Position.z += old->z - pos->z;
				skidooItem->Pose.Position.x += old->x - pos->x;
				skidooItem->Animation.Velocity -= 50;
			}
		}

		return 0;
	}

	int SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		Vector3Int frontLeftOld, frontRightOld, backLeftOld, backRightOld;
		auto heightFrontLeftOld = GetVehicleHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true, &frontLeftOld);
		auto heightFrontRightOld = GetVehicleHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true, &frontRightOld);
		auto heightBackLeftOld = GetVehicleHeight(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, true, &backLeftOld);
		auto heightBackRightOld = GetVehicleHeight(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, true, &backRightOld);

		auto oldPos = skidooItem->Pose.Position;

		short rotation;

		if (skidooItem->Pose.Position.y > (skidooItem->Floor - CLICK(1)))
		{
			if (skidoo->TurnRate < -SKIDOO_TURN_RATE_DECEL)
				skidoo->TurnRate += SKIDOO_TURN_RATE_DECEL;
			else if (skidoo->TurnRate > SKIDOO_TURN_RATE_DECEL)
				skidoo->TurnRate -= SKIDOO_TURN_RATE_DECEL;
			else
				skidoo->TurnRate = 0;
			skidooItem->Pose.Orientation.y += skidoo->TurnRate + skidoo->ExtraRotation;

			rotation = skidooItem->Pose.Orientation.y - skidoo->MomentumAngle;
			if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = -SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo->MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->MomentumAngle -= SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
			}
			else if (rotation > SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation > SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo->MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->MomentumAngle += SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
			}
			else
				skidoo->MomentumAngle = skidooItem->Pose.Orientation.y;
		}
		else
			skidooItem->Pose.Orientation.y += skidoo->TurnRate + skidoo->ExtraRotation;

		TranslateItem(skidooItem, skidoo->MomentumAngle, skidooItem->Animation.Velocity);

		int slip = SKIDOO_SLIP * phd_sin(skidooItem->Pose.Orientation.x);
		if (abs(slip) > (SKIDOO_SLIP / 2))
		{
			skidooItem->Pose.Position.x -= slip * phd_sin(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.z -= slip * phd_cos(skidooItem->Pose.Orientation.y);
		}

		slip = SKIDOO_SLIP_SIDE * phd_sin(skidooItem->Pose.Orientation.z);
		if (abs(slip) > (SKIDOO_SLIP_SIDE / 2))
		{
			skidooItem->Pose.Position.x += slip * phd_cos(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.z -= slip * phd_sin(skidooItem->Pose.Orientation.y);
		}

		Vector3Int moved;
		moved.x = skidooItem->Pose.Position.x;
		moved.z = skidooItem->Pose.Position.z;

		if (!(skidooItem->Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(skidooItem, SKIDOO_RADIUS);

		Vector3Int frontLeft, frontRight, backRight, backLeft;
		rotation = 0;
		auto heightBackLeft = GetVehicleHeight(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, false, &backLeft);
		if (heightBackLeft < (backLeftOld.y - CLICK(1)))
			rotation = DoSkidooShift(skidooItem, &backLeft, &backLeftOld);

		auto heightBackRight = GetVehicleHeight(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, false, &backRight);
		if (heightBackRight < (backRightOld.y - CLICK(1)))
			rotation += DoSkidooShift(skidooItem, &backRight, &backRightOld);

		auto heightFrontLeft = GetVehicleHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, false, &frontLeft);
		if (heightFrontLeft < (frontLeftOld.y - CLICK(1)))
			rotation += DoSkidooShift(skidooItem, &frontLeft, &frontLeftOld);

		auto heightFrontRight = GetVehicleHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, false, &frontRight);
		if (heightFrontRight < (frontRightOld.y - CLICK(1)))
			rotation += DoSkidooShift(skidooItem, &frontRight, &frontRightOld);

		auto probe = GetCollision(skidooItem);
		if (probe.Position.Floor < (skidooItem->Pose.Position.y - CLICK(1)))
			DoSkidooShift(skidooItem, (Vector3Int*)&skidooItem->Pose, &oldPos);

		skidoo->ExtraRotation = rotation;

		auto collide = GetSkidooCollisionAnim(skidooItem, &moved);
		if (collide)
		{
			int newVelocity = (skidooItem->Pose.Position.z - oldPos.z) * phd_cos(skidoo->MomentumAngle) + (skidooItem->Pose.Position.x - oldPos.x) * phd_sin(skidoo->MomentumAngle);
			if (skidooItem->Animation.Velocity > (SKIDOO_NORMAL_VELOCITY_MAX + SKIDOO_VELOCITY_ACCEL) &&
				newVelocity < (skidooItem->Animation.Velocity - 10))
			{
				DoDamage(laraItem, (skidooItem->Animation.Velocity - newVelocity) / 2);
			}

			if (skidooItem->Animation.Velocity > 0 && newVelocity < skidooItem->Animation.Velocity)
				skidooItem->Animation.Velocity = (newVelocity < 0) ? 0 : newVelocity;
			else if (skidooItem->Animation.Velocity < 0 && newVelocity > skidooItem->Animation.Velocity)
				skidooItem->Animation.Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (skidooItem->Animation.Velocity < SKIDOO_REVERSE_VELOCITY_MAX)
				skidooItem->Animation.Velocity = SKIDOO_REVERSE_VELOCITY_MAX;
		}

		return collide;
	}
}
