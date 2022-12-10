#include "framework.h"
#include "Objects/TR3/Vehicles/upv.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/effects/bubble.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Objects/Sink.h"
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/Input/Input.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using namespace TEN::Math;

// TODO:
// Redo water surface dismount.
// Calibrate rotation control to work well on both keyboard and gamepad.
// Improve deflection.
// Try to improve room collision.

namespace TEN::Entities::Vehicles
{
	constexpr auto UPV_RADIUS = 300;
	constexpr auto UPV_HEIGHT = 400;
	constexpr auto UPV_LENGTH = SECTOR(1);

	constexpr auto UPV_MOUNT_DISTANCE		  = CLICK(2);
	constexpr auto UPV_DISMOUNT_DISTANCE	  = SECTOR(1);
	constexpr auto UPV_WATER_SURFACE_DISTANCE = 210;
	constexpr auto UPV_SHIFT				  = 128;
	constexpr auto UPV_HARPOON_RELOAD_TIME	  = 15;
	constexpr auto UPV_HARPOON_VELOCITY		  = CLICK(1);

	constexpr auto UPV_VELOCITY_ACCEL		   = 4.0f;
	constexpr auto UPV_VELOCITY_FRICTION_DECEL = 1.5f;

	constexpr auto UPV_VELOCITY_MAX = 64.0f;

	// TODO: These should be done in the wad. @Sezz 2022.06.24
	constexpr auto UPV_DEATH_FRAME_1 = 16;
	constexpr auto UPV_DEATH_FRAME_2 = 17;
	constexpr auto UPV_MOUNT_WATER_SURFACE_SOUND_FRAME = 30;
	constexpr auto UPV_MOUNT_WATER_SURFACE_CONTROL_FRAME = 50;
	constexpr auto UPV_DISMOUNT_WATER_SURFACE_FRAME = 51;
	constexpr auto UPV_MOUNT_UNDERWATER_SOUND_FRAME = 30;
	constexpr auto UPV_MOUNT_UNDERWATER_CONTROL_FRAME = 42;
	constexpr auto UPV_DISMOUNT_UNDERWATER_FRAME = 42;

	#define UPV_X_TURN_RATE_DIVE_ACCEL	   ANGLE(5.0f)
	#define UPV_X_TURN_RATE_ACCEL		   ANGLE(0.6f)
	#define UPV_X_TURN_RATE_FRICTION_DECEL ANGLE(0.3f)
	#define UPV_X_TURN_RATE_MAX			   ANGLE(3.25f)

	#define UPV_Y_TURN_RATE_ACCEL		   ANGLE(0.6f)
	#define UPV_Y_TURN_RATE_FRICTION_DECEL ANGLE(0.3f)
	#define UPV_Y_TURN_RATE_MAX			   ANGLE(3.75f)

	#define UPV_X_ORIENT_WATER_SURFACE_MAX ANGLE(30.0f)
	#define UPV_X_ORIENT_DIVE_MAX		   ANGLE(15.0f)
	#define UPV_X_ORIENT_MAX			   ANGLE(85.0f)

	#define UPV_DEFLECT_ANGLE		 ANGLE(45.0f)
	#define UPV_DEFLCT_TURN_RATE_MAX ANGLE(2.0f)

	#define UPV_LEAN_RATE ANGLE(0.6f)
	#define UPV_LEAN_MAX  ANGLE(10.0f)

	enum UpvState
	{
		UPV_STATE_DEATH = 0,
		UPV_STATE_IMPACT = 1,
		UPV_STATE_DISMOUNT_WATER_SURFACE = 2,
		UPV_STATE_UNUSED_1 = 3, // Unused.
		UPV_STATE_MOVE = 4,
		UPV_STATE_IDLE = 5,
		UPV_STATE_UNUSED_2 = 6, // Unused.
		UPV_STATE_UNUSED_3 = 7, // Unused.
		UPV_STATE_MOUNT = 8,
		UPV_STATE_DISMOUNT_UNDERWATER = 9
	};

	enum UpvAnim
	{
		UPV_ANIM_MOVING_DEATH = 0,
		UPV_ANIM_IDLE_DEATH = 1,
		UPV_ANIM_IMPACT_FRONT = 2,
		UPV_ANIM_MOVE = 3,
		UPV_ANIM_IMPACT_FRONT_2 = 4, // Unused.
		UPV_ANIM_IDLE = 5,
		UPV_ANIM_IDLE_TO_MOVE = 6,
		UPV_ANIM_MOVE_TO_IDLE = 7,
		UPV_ANIM_DISMOUNT_WATER_SURFACE_START = 8,
		UPV_ANIM_DISMOUNT_WATER_SURFACE_END = 9,
		UPV_ANIM_MOUNT_SURFACE_START = 10,
		UPV_ANIM_MOUNT_SURFACE_END = 11,
		UPV_ANIM_DISMOUNT_UNDERWATER = 12,
		UPV_ANIM_MOUNT_UNDERWATER = 13,
	};

	enum UpvJoint
	{
		UPV_JOINT_LEFT_RUDDER  = 1,
		UPV_JOINT_RIGHT_RUDDER = 2,
		UPV_JOINT_TURBINE	   = 3
	};
	
	enum UpvFlags
	{
		UPV_FLAG_CONTROL = (1 << 0),
		UPV_FLAG_SURFACE = (1 << 1),
		UPV_FLAG_DIVE	 = (1 << 2),
		UPV_FLAG_DEAD	 = (1 << 3)
	};

	enum UpvBiteIndex
	{
		UPV_BITE_TURBINE			= 0,
		UPV_BITE_FRONT_LIGHT		= 1,
		UPV_BITE_LEFT_RUDDER_LEFT   = 2, // Unused. Perhaps something like a trailing stream effect behind rudders was intended?
		UPV_BITE_LEFT_RUDDER_RIGHT  = 3, // Unused.
		UPV_BITE_RIGHT_RUDDER_RIGHT = 4, // Unused.
		UPV_BITE_RIGHT_RUDDER_LEFT  = 5	 // Unused.
	};

	const BiteInfo UpvBites[6] =
	{
		{ 0, 0, 0, 3 },
		{ 0, 96, 256, 0 },
		{ -128, 0, -64, 1 },
		{ 0, 0, -64, 1 },
		{ 128, 0, -64, 2 },
		{ 0, 0, -64, 2 }
	};
	const auto UpvMountTypes = std::vector<VehicleMountType>
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Back
	};

	UpvInfo& GetUpvInfo(ItemInfo* upvItem)
	{
		return *(UpvInfo*)upvItem->Data;
	}

	void UpvInitialise(short itemNumber)
	{
		auto* upvItem = &g_Level.Items[itemNumber];
		upvItem->Data = UpvInfo();
		auto& upv = GetUpvInfo(upvItem);

		upv.Flags = UPV_FLAG_SURFACE;
	}

	bool UpvControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* upvItem = &g_Level.Items[lara->Vehicle];
		auto& upv = GetUpvInfo(upvItem);

		auto oldPos = upvItem->Pose;
		auto probe = GetCollision(upvItem);

		if (!(upv.Flags & UPV_FLAG_DEAD))
		{
			UpvUserControl(upvItem, laraItem);

			upvItem->Animation.Velocity.z = (int)round(upv.Velocity);
			upvItem->Pose.Orientation += upv.TurnRate;

			if (upvItem->Pose.Orientation.x > UPV_X_ORIENT_MAX)
				upvItem->Pose.Orientation.x = UPV_X_ORIENT_MAX;
			else if (upvItem->Pose.Orientation.x < -UPV_X_ORIENT_MAX)
				upvItem->Pose.Orientation.x = -UPV_X_ORIENT_MAX;

			if (!(TrInput & IN_LEFT) && !(TrInput & IN_RIGHT))
				ResetVehicleLean(upvItem, 0.9f);

			TranslateItem(upvItem, upvItem->Pose.Orientation, upvItem->Animation.Velocity.z);
		}

		int newHeight = GetCollision(upvItem).Position.Floor;
		int waterHeight = GetWaterHeight(upvItem);

		if ((newHeight - waterHeight) < UPV_HEIGHT || (newHeight < upvItem->Pose.Position.y - UPV_HEIGHT / 2) ||
			(newHeight == NO_HEIGHT) || (waterHeight == NO_HEIGHT))
		{
			upvItem->Pose.Position = oldPos.Position;
			upvItem->Animation.Velocity.z = 0;
		}

		upvItem->Floor = probe.Position.Floor;

		if (upv.Flags & UPV_FLAG_CONTROL && !(upv.Flags & UPV_FLAG_DEAD))
		{
			if (!TestEnvironment(ENV_FLAG_WATER, upvItem->RoomNumber) &&
				waterHeight != NO_HEIGHT)
			{
				if ((waterHeight - upvItem->Pose.Position.y) >= -UPV_WATER_SURFACE_DISTANCE)
					upvItem->Pose.Position.y = waterHeight + UPV_WATER_SURFACE_DISTANCE;

				if (!(upv.Flags & UPV_FLAG_SURFACE))
				{
					SoundEffect(SFX_TR4_LARA_BREATH, &laraItem->Pose, SoundEnvironment::Always);
					upv.Flags &= ~UPV_FLAG_DIVE;
				}

				upv.Flags |= UPV_FLAG_SURFACE;
			}
			else if ((waterHeight - upvItem->Pose.Position.y) >= -UPV_WATER_SURFACE_DISTANCE && waterHeight != NO_HEIGHT &&
				(laraItem->Pose.Position.y - probe.Position.Ceiling) >= CLICK(1))
			{
				upvItem->Pose.Position.y = waterHeight + UPV_WATER_SURFACE_DISTANCE;

				if (!(upv.Flags & UPV_FLAG_SURFACE))
				{
					SoundEffect(SFX_TR4_LARA_BREATH, &laraItem->Pose, SoundEnvironment::Always);
					upv.Flags &= ~UPV_FLAG_DIVE;
				}

				upv.Flags |= UPV_FLAG_SURFACE;
			}
			else
				upv.Flags &= ~UPV_FLAG_SURFACE;

			if (!(upv.Flags & UPV_FLAG_SURFACE))
			{
				if (laraItem->HitPoints > 0)
				{
					lara->Air--;
					if (lara->Air < 0)
					{
						lara->Air = -1;
						DoDamage(laraItem, 5);
					}
				}
			}
			else
			{
				if (laraItem->HitPoints >= 0)
				{
					lara->Air += 10;
					if (lara->Air > LARA_AIR_MAX)
						lara->Air = LARA_AIR_MAX;
				}
			}
		}

		TestTriggers(upvItem, false);
		TriggerUpvEffects(lara->Vehicle);

		if (!(upv.Flags & UPV_FLAG_DEAD) &&
			lara->Vehicle != NO_ITEM)
		{
			DoUpvCurrent(upvItem, laraItem);

			if (TrInput & VEHICLE_IN_FIRE &&
				upv.Flags & UPV_FLAG_CONTROL &&
				!upv.HarpoonTimer)
			{
				if (laraItem->Animation.ActiveState != UPV_STATE_DISMOUNT_UNDERWATER &&
					laraItem->Animation.ActiveState != UPV_STATE_DISMOUNT_WATER_SURFACE &&
					laraItem->Animation.ActiveState != UPV_STATE_MOUNT)
				{
					FireUpvHarpoon(upvItem, laraItem);
					upv.HarpoonTimer = UPV_HARPOON_RELOAD_TIME;
				}
			}

			if (probe.RoomNumber != upvItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose = upvItem->Pose;

			AnimateItem(laraItem);
			UpvBackgroundCollision(upvItem, laraItem);
			DoVehicleCollision(upvItem, UPV_RADIUS);

			if (upv.Flags & UPV_FLAG_CONTROL)
				SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, &upvItem->Pose, SoundEnvironment::Always, 1.0f + (float)upvItem->Animation.Velocity.z / 96.0f);

			upvItem->Animation.AnimNumber = Objects[ID_UPV].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_UPV_LARA_ANIMS].animIndex);
			upvItem->Animation.FrameNumber = g_Level.Anims[upvItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			if (upv.Flags & UPV_FLAG_SURFACE)
				Camera.targetElevation = -ANGLE(60.0f);
			else
				Camera.targetElevation = 0;

			return true;
		}
		else if (upv.Flags & UPV_FLAG_DEAD)
		{
			AnimateItem(laraItem);

			if (probe.RoomNumber != upvItem->RoomNumber)
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);

			UpvBackgroundCollision(upvItem, laraItem);

			upv.TurnRate.x = 0;

			SetAnimation(upvItem, UPV_ANIM_IDLE);
			upvItem->Animation.Velocity.y = 0;
			upvItem->Animation.Velocity.z = 0;
			upvItem->Animation.IsAirborne = true;
			AnimateItem(upvItem);

			return true;
		}
		else
			return false;
	}

	void UpvUserControl(ItemInfo* upvItem, ItemInfo* laraItem)
	{
		auto& upv = GetUpvInfo(upvItem);
		auto* lara = GetLaraInfo(laraItem);

		TestUpvDismount(upvItem, laraItem);

		int anim = laraItem->Animation.AnimNumber - Objects[ID_UPV_LARA_ANIMS].animIndex;
		int frame = laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		switch (laraItem->Animation.ActiveState)
		{
		case UPV_STATE_MOVE:
			if (laraItem->HitPoints <= 0)
			{
				laraItem->Animation.TargetState = UPV_STATE_DEATH;
				break;
			}

			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
			{
				ModulateVehicleTurnRateY(upv.TurnRate.y, UPV_Y_TURN_RATE_ACCEL, -UPV_Y_TURN_RATE_MAX, UPV_Y_TURN_RATE_MAX);
				ModulateVehicleLean(upvItem, UPV_LEAN_RATE, UPV_LEAN_MAX);
			}

			if (upv.Flags & UPV_FLAG_SURFACE)
			{
				int xa = upvItem->Pose.Orientation.x - UPV_X_ORIENT_WATER_SURFACE_MAX;
				int ax = UPV_X_ORIENT_WATER_SURFACE_MAX - upvItem->Pose.Orientation.x;

				if (xa > 0)
				{
					if (xa > ANGLE(1.0f))
						upvItem->Pose.Orientation.x -= ANGLE(1.0f);
					else
						upvItem->Pose.Orientation.x -= ANGLE(0.1f);
				}
				else if (ax)
				{
					if (ax > ANGLE(1.0f))
						upvItem->Pose.Orientation.x += ANGLE(1.0f);
					else
						upvItem->Pose.Orientation.x += ANGLE(0.1f);
				}
				else
					upvItem->Pose.Orientation.x = UPV_X_ORIENT_WATER_SURFACE_MAX;
			}
			else
			{
				if (TrInput & (VEHICLE_IN_UP | VEHICLE_IN_DOWN))
					ModulateVehicleTurnRateX(upv.TurnRate.x, UPV_X_TURN_RATE_ACCEL, -UPV_X_TURN_RATE_MAX, UPV_X_TURN_RATE_MAX);
			}

			if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_UP &&
					upv.Flags & UPV_FLAG_SURFACE &&
					upvItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX)
				{
					upv.Flags |= UPV_FLAG_DIVE;
				}

				upv.Velocity += UPV_VELOCITY_ACCEL;
			}
			else
				laraItem->Animation.TargetState = UPV_STATE_IDLE;

			break;

		case UPV_STATE_IDLE:
			if (laraItem->HitPoints <= 0)
			{
				laraItem->Animation.TargetState = UPV_STATE_DEATH;
				break;
			}

			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
			{
				ModulateVehicleTurnRateY(upv.TurnRate.y, UPV_Y_TURN_RATE_ACCEL, -UPV_Y_TURN_RATE_MAX, UPV_Y_TURN_RATE_MAX);
				ModulateVehicleLean(upvItem, UPV_LEAN_RATE, UPV_LEAN_MAX);
			}

			if (upv.Flags & UPV_FLAG_SURFACE)
			{
				int xa = upvItem->Pose.Orientation.x - UPV_X_ORIENT_WATER_SURFACE_MAX;
				int ax = UPV_X_ORIENT_WATER_SURFACE_MAX - upvItem->Pose.Orientation.x;
				if (xa > 0)
				{
					if (xa > ANGLE(1.0f))
						upvItem->Pose.Orientation.x -= ANGLE(1.0f);
					else
						upvItem->Pose.Orientation.x -= ANGLE(0.1f);
				}
				else if (ax)
				{
					if (ax > ANGLE(1.0f))
						upvItem->Pose.Orientation.x += ANGLE(1.0f);
					else
						upvItem->Pose.Orientation.x += ANGLE(0.1f);
				}
				else
					upvItem->Pose.Orientation.x = UPV_X_ORIENT_WATER_SURFACE_MAX;
			}
			else
			{
				if (TrInput & (VEHICLE_IN_UP | VEHICLE_IN_DOWN))
					ModulateVehicleTurnRateX(upv.TurnRate.x, UPV_X_TURN_RATE_ACCEL, -UPV_X_TURN_RATE_MAX, UPV_X_TURN_RATE_MAX);
			}

			if (TrInput & VEHICLE_IN_DISMOUNT && TestUpvDismount(upvItem, laraItem))
			{
				if (upv.Velocity > 0.0f)
					upv.Velocity -= UPV_VELOCITY_ACCEL;
				else
				{
					if (upv.Flags & UPV_FLAG_SURFACE)
						laraItem->Animation.TargetState = UPV_STATE_DISMOUNT_WATER_SURFACE;
					else
						laraItem->Animation.TargetState = UPV_STATE_DISMOUNT_UNDERWATER;

					//sub->Flags &= ~UPV_FLAG_CONTROL; having this here causes the upv glitch, moving it directly to the states' code is better

					StopSoundEffect(SFX_TR3_VEHICLE_UPV_LOOP);
					SoundEffect(SFX_TR3_VEHICLE_UPV_STOP, (Pose*)&upvItem->Pose.Position.x, SoundEnvironment::Always);
				}
			}

			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (TrInput & VEHICLE_IN_UP &&
					upvItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX &&
					upv.Flags & UPV_FLAG_SURFACE)
				{
					upv.Flags |= UPV_FLAG_DIVE;
				}

				laraItem->Animation.TargetState = UPV_STATE_MOVE;
			}

			break;

		case UPV_STATE_MOUNT:
			if (anim == UPV_ANIM_MOUNT_SURFACE_END)
			{
				upvItem->Pose.Position.y += 4;
				upvItem->Pose.Orientation.x += ANGLE(1.0f);

				if (frame == UPV_MOUNT_WATER_SURFACE_SOUND_FRAME)
					SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, (Pose*)&upvItem->Pose.Position.x, SoundEnvironment::Always);

				if (frame == UPV_MOUNT_WATER_SURFACE_CONTROL_FRAME)
					upv.Flags |= UPV_FLAG_CONTROL;
			}

			else if (anim == UPV_ANIM_MOUNT_UNDERWATER)
			{
				if (frame == UPV_MOUNT_UNDERWATER_SOUND_FRAME)
					SoundEffect(SFX_TR3_VEHICLE_UPV_LOOP, (Pose*)&upvItem->Pose.Position.x, SoundEnvironment::Always);

				if (frame == UPV_MOUNT_UNDERWATER_CONTROL_FRAME)
					upv.Flags |= UPV_FLAG_CONTROL;
			}

			break;

		case UPV_STATE_DISMOUNT_UNDERWATER:
			if (anim == UPV_ANIM_DISMOUNT_UNDERWATER && frame == UPV_DISMOUNT_UNDERWATER_FRAME)
			{
				upv.Flags &= ~UPV_FLAG_CONTROL;

				auto vec = GetJointPosition(laraItem, LM_HIPS);

				auto LPos = GameVector(
					vec.x,
					vec.y,
					vec.z,
					upvItem->RoomNumber
				);

				auto VPos = GameVector(
					upvItem->Pose.Position.x,
					upvItem->Pose.Position.y,
					upvItem->Pose.Position.z,
					upvItem->RoomNumber
				);
				LOSAndReturnTarget(&VPos, &LPos, 0);

				laraItem->Pose.Position = Vector3i(LPos.x, LPos.y, LPos.z);

				SetAnimation(laraItem, LA_UNDERWATER_IDLE);
				laraItem->Animation.Velocity.y = 0;
				laraItem->Animation.IsAirborne = false;
				laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;

				UpdateLaraRoom(laraItem, 0);

				lara->Control.WaterStatus = WaterStatus::Underwater;
				lara->Control.HandStatus = HandStatus::Free;
				SetLaraVehicle(laraItem, nullptr);

				upvItem->HitPoints = 0;
			}

			break;

		case UPV_STATE_DISMOUNT_WATER_SURFACE:
			if (anim == UPV_ANIM_DISMOUNT_WATER_SURFACE_END && frame == UPV_DISMOUNT_WATER_SURFACE_FRAME)
			{
				upv.Flags &= ~UPV_FLAG_CONTROL;
				int waterDepth, waterHeight, heightFromWater;

				waterDepth = GetWaterSurface(laraItem);
				waterHeight = GetWaterHeight(laraItem);

				if (waterHeight != NO_HEIGHT)
					heightFromWater = laraItem->Pose.Position.y - waterHeight;
				else
					heightFromWater = NO_HEIGHT;

				auto vec = GetJointPosition(laraItem, LM_HIPS);

				laraItem->Pose.Position.x = vec.x;
				//laraItem->Pose.Position.y += -heightFromWater + 1; // Doesn't work as intended.
				laraItem->Pose.Position.y = vec.y;
				laraItem->Pose.Position.z = vec.z;

				SetAnimation(laraItem, LA_ONWATER_IDLE);
				laraItem->Animation.Velocity.y = 0;
				laraItem->Animation.IsAirborne = false;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;

				UpdateLaraRoom(laraItem, -LARA_HEIGHT / 2);

				ResetLaraFlex(laraItem);
				lara->Control.WaterStatus = WaterStatus::TreadWater;
				lara->WaterSurfaceDist = -heightFromWater;
				SetLaraVehicle(laraItem, nullptr);

				upvItem->HitPoints = 0;
			}
			else
			{
				upv.TurnRate.x -= UPV_X_TURN_RATE_ACCEL;
				if (upvItem->Pose.Orientation.x < 0)
					upvItem->Pose.Orientation.x = 0;
			}

			break;

		case UPV_STATE_DEATH:
			if ((anim == UPV_ANIM_IDLE_DEATH || anim == UPV_ANIM_MOVING_DEATH) &&
				(frame == UPV_DEATH_FRAME_1 || frame == UPV_DEATH_FRAME_2))
			{
				auto vec = GetJointPosition(laraItem, LM_HIPS);
				laraItem->Pose.Position = vec;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;

				SetAnimation(laraItem, LA_UNDERWATER_DEATH, 17);
				laraItem->Animation.IsAirborne = false;
				laraItem->Animation.Velocity.y = 0;

				upv.Flags |= UPV_FLAG_DEAD;
			}

			upvItem->Animation.Velocity.z = 0;
			break;
		}

		if (upv.Flags & UPV_FLAG_DIVE)
		{
			if (upvItem->Pose.Orientation.x > -UPV_X_ORIENT_DIVE_MAX)
				upvItem->Pose.Orientation.x -= UPV_X_TURN_RATE_DIVE_ACCEL;
			else
				upv.Flags &= ~UPV_FLAG_DIVE;
		}

		if (upv.Velocity > 0.0f)
		{
			upv.Velocity -= UPV_VELOCITY_FRICTION_DECEL;
			if (upv.Velocity < 0.0f)
				upv.Velocity = 0.0f;
		}
		else if (upv.Velocity < 0.0f)
		{
			upv.Velocity += UPV_VELOCITY_FRICTION_DECEL;
			if (upv.Velocity > 0.0f)
				upv.Velocity = 0.0f;
		}

		if (upv.Velocity > UPV_VELOCITY_MAX)
			upv.Velocity = UPV_VELOCITY_MAX;
		else if (upv.Velocity < -UPV_VELOCITY_MAX)
			upv.Velocity = -UPV_VELOCITY_MAX;

		ResetVehicleTurnRateX(upv.TurnRate.x, UPV_X_TURN_RATE_FRICTION_DECEL);
		ResetVehicleTurnRateY(upv.TurnRate.y, UPV_Y_TURN_RATE_FRICTION_DECEL);
	}

	void UpvPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* upvItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints <= 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(upvItem, laraItem, coll, UpvMountTypes, UPV_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
		{
			// HACK: Collision in water behaves differently? @Sezz 2022.06.28
			if (TestBoundsCollide(upvItem, laraItem, coll->Setup.Radius) && TestCollision(upvItem, laraItem))
				ItemPushItem(upvItem, laraItem, coll, false, false);
		}
		else
		{
			SetLaraVehicle(laraItem, UPVItem);
			DoUpvMount(upvItem, laraItem, mountType);
		}
	}

	void DoUpvMount(ItemInfo* upvItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_IDLE;
			laraItem->Animation.ActiveState = UPV_STATE_IDLE;
			laraItem->Animation.TargetState = UPV_STATE_IDLE;
			break;

		default:
		case VehicleMountType::Back:
			if (lara->Control.WaterStatus == WaterStatus::TreadWater)
				laraItem->Animation.AnimNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_SURFACE_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_UNDERWATER;

			laraItem->Animation.ActiveState = UPV_STATE_MOUNT;
			laraItem->Animation.TargetState = UPV_STATE_MOUNT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		laraItem->Pose = upvItem->Pose;
		lara->Control.HandStatus = HandStatus::Busy;
		lara->Control.WaterStatus = WaterStatus::Dry;
		upvItem->HitPoints = 1;

		AnimateItem(laraItem);
	}

	bool TestUpvDismount(ItemInfo* upvItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (lara->WaterCurrentPull.x || lara->WaterCurrentPull.z)
			return false;

		short moveAngle = upvItem->Pose.Orientation.y + ANGLE(180.0f);
		int velocity = UPV_DISMOUNT_DISTANCE * phd_cos(upvItem->Pose.Orientation.x);
		int x = upvItem->Pose.Position.x + velocity * phd_sin(moveAngle);
		int z = upvItem->Pose.Position.z + velocity * phd_cos(moveAngle);
		int y = upvItem->Pose.Position.y - UPV_DISMOUNT_DISTANCE * phd_sin(-upvItem->Pose.Orientation.x);

		auto probe = GetCollision(x, y, z, upvItem->RoomNumber);
		if ((probe.Position.Floor - probe.Position.Ceiling) < CLICK(1) ||
			probe.Position.Floor < y ||
			probe.Position.Ceiling > y ||
			probe.Position.Floor == NO_HEIGHT ||
			probe.Position.Ceiling == NO_HEIGHT)
		{
			return false;
		}

		return true;
	}

	void FireUpvHarpoon(ItemInfo* upvItem, ItemInfo* laraItem)
	{
		auto upv = GetUpvInfo(upvItem);
		auto* lara = GetLaraInfo(laraItem);

		auto& ammo = GetAmmo(*lara, LaraWeaponType::HarpoonGun);
		if (ammo.GetCount() == 0 && !ammo.HasInfinite())
			return;
		else if (!ammo.HasInfinite())
			ammo--;

		short itemNumber = CreateItem();

		if (itemNumber != NO_ITEM)
		{
			auto* harpoonItem = &g_Level.Items[itemNumber];
			harpoonItem->ObjectNumber = ID_HARPOON;
			harpoonItem->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			harpoonItem->RoomNumber = upvItem->RoomNumber;

			auto pos = GetJointPosition(upvItem, UPV_JOINT_TURBINE, Vector3i((upv.HarpoonLeft ? 22 : -22), 24, 230));
			harpoonItem->Pose.Position = pos;

			InitialiseItem(itemNumber);

			harpoonItem->Pose.Orientation = EulerAngles(upvItem->Pose.Orientation.x, upvItem->Pose.Orientation.y, 0);

			// TODO: Huh?
			harpoonItem->Animation.Velocity.y = -UPV_HARPOON_VELOCITY * phd_sin(harpoonItem->Pose.Orientation.x);
			harpoonItem->Animation.Velocity.z = UPV_HARPOON_VELOCITY * phd_cos(harpoonItem->Pose.Orientation.x);
			harpoonItem->HitPoints = HARPOON_TIME;
			harpoonItem->ItemFlags[0] = 1;

			AddActiveItem(itemNumber);

			SoundEffect(SFX_TR4_HARPOON_FIRE_UNDERWATER, &laraItem->Pose, SoundEnvironment::Always);

			Statistics.Game.AmmoUsed++;
			upv.HarpoonLeft = !upv.HarpoonLeft;
		}
	}

	void UpvBackgroundCollision(ItemInfo* upvItem, ItemInfo* laraItem)
	{
		auto& upv = GetUpvInfo(upvItem);
		auto* lara = GetLaraInfo(laraItem);
		CollisionInfo cinfo, * coll = &cinfo; // ??

		coll->Setup.Mode = CollisionProbeMode::Quadrants;
		coll->Setup.Radius = UPV_RADIUS;

		coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
		coll->Setup.UpperFloorBound = -UPV_HEIGHT;
		coll->Setup.LowerCeilingBound = UPV_HEIGHT;
		coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;

		coll->Setup.BlockFloorSlopeUp = false;
		coll->Setup.BlockFloorSlopeDown = false;
		coll->Setup.BlockDeathFloorDown = false;
		coll->Setup.BlockMonkeySwingEdge = false;
		coll->Setup.EnableObjectPush = true;
		coll->Setup.EnableSpasm = false;

		coll->Setup.OldPosition = upvItem->Pose.Position;

		if ((upvItem->Pose.Orientation.x >= -(SHRT_MAX / 2 + 1)) && (upvItem->Pose.Orientation.x <= (SHRT_MAX / 2 + 1)))
		{
			lara->Control.MoveAngle = upvItem->Pose.Orientation.y;
			coll->Setup.ForwardAngle = lara->Control.MoveAngle;
		}
		else
		{
			lara->Control.MoveAngle = upvItem->Pose.Orientation.y - ANGLE(180.0f);
			coll->Setup.ForwardAngle = lara->Control.MoveAngle;
		}

		int height = phd_sin(upvItem->Pose.Orientation.x) * UPV_LENGTH;
		if (height < 0)
			height = -height;
		if (height < 200)
			height = 200;

		coll->Setup.UpperFloorBound = -height;
		coll->Setup.Height = height;

		GetCollisionInfo(coll, upvItem, Vector3i(0, height / 2, 0));
		ShiftItem(upvItem, coll);

		if (coll->CollisionType == CT_FRONT)
		{
			if (upv.TurnRate.x > UPV_DEFLECT_ANGLE)
				upv.TurnRate.x += UPV_DEFLCT_TURN_RATE_MAX;
			else if (upv.TurnRate.x < -UPV_DEFLECT_ANGLE)
				upv.TurnRate.x -= UPV_DEFLCT_TURN_RATE_MAX;
			else
			{
				if (abs(upv.Velocity) >= UPV_VELOCITY_MAX)
				{
					laraItem->Animation.TargetState = UPV_STATE_IMPACT;
					upv.Velocity = -upv.Velocity / 2.0f;
				}
				else
					upv.Velocity = 0.0f;
			}
		}
		else if (coll->CollisionType == CT_TOP)
		{
			if (upv.TurnRate.x >= -UPV_DEFLECT_ANGLE)
				upv.TurnRate.x -= UPV_DEFLCT_TURN_RATE_MAX;
		}
		else if (coll->CollisionType == CT_TOP_FRONT)
			upv.Velocity = 0.0f;
		else if (coll->CollisionType == CT_LEFT)
			upvItem->Pose.Orientation.y += ANGLE(5.0f);
		else if (coll->CollisionType == CT_RIGHT)
			upvItem->Pose.Orientation.y -= ANGLE(5.0f);
		else if (coll->CollisionType == CT_CLAMP)
		{
			upvItem->Pose.Position = coll->Setup.OldPosition;
			upv.Velocity = 0.0f;
			return;
		}

		if (coll->Middle.Floor < 0)
		{
			upvItem->Pose.Position.y += coll->Middle.Floor;
			upv.TurnRate.x += UPV_DEFLCT_TURN_RATE_MAX;
		}
	}

	void DoUpvCurrent(ItemInfo* upvItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		Vector3i target;

		if (!lara->WaterCurrentActive)
		{
			int absVel = abs(lara->WaterCurrentPull.x);
			int shift;
			if (absVel > 16)
				shift = 4;
			else if (absVel > 8)
				shift = 3;
			else
				shift = 2;

			lara->WaterCurrentPull.x -= lara->WaterCurrentPull.x >> shift;

			if (abs(lara->WaterCurrentPull.x) < 4)
				lara->WaterCurrentPull.x = 0;

			absVel = abs(lara->WaterCurrentPull.z);
			if (absVel > 16)
				shift = 4;
			else if (absVel > 8)
				shift = 3;
			else
				shift = 2;

			lara->WaterCurrentPull.z -= lara->WaterCurrentPull.z >> shift;
			if (abs(lara->WaterCurrentPull.z) < 4)
				lara->WaterCurrentPull.z = 0;

			if (lara->WaterCurrentPull.x == 0 && lara->WaterCurrentPull.z == 0)
				return;
		}
		else
		{
			int sinkVal = lara->WaterCurrentActive - 1;
			target = g_Level.Sinks[sinkVal].Position;
		
			int angle = ((Geometry::GetOrientToPoint(laraItem->Pose.Position.ToVector3(), target.ToVector3()).y) / 16) & 4095;

			int dx = target.x - laraItem->Pose.Position.x;
			int dz = target.z - laraItem->Pose.Position.z;

			int velocity = g_Level.Sinks[sinkVal].Strength;
			dx = phd_sin(angle * 16) * velocity * SECTOR(1);
			dz = phd_cos(angle * 16) * velocity * SECTOR(1);

			lara->WaterCurrentPull.x += ((dx - lara->WaterCurrentPull.x) / 16);
			lara->WaterCurrentPull.z += ((dz - lara->WaterCurrentPull.z) / 16);
		}

		lara->WaterCurrentActive = 0;
		upvItem->Pose.Position.x += lara->WaterCurrentPull.x / CLICK(1);
		upvItem->Pose.Position.z += lara->WaterCurrentPull.z / CLICK(1);
	}

	void TriggerUpvEffects(short itemNumber)
	{
		if (itemNumber == NO_ITEM)
			return;

		auto* upvItem = &g_Level.Items[itemNumber];
		auto& upv = GetUpvInfo(upvItem);
		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);

		Vector3i pos;

		if (lara->Vehicle == itemNumber)
		{
			upv.TurbineRotation += (upv.Velocity * VEHICLE_VELOCITY_SCALE) ? ((upv.Velocity * VEHICLE_VELOCITY_SCALE) / 8.0f) : ANGLE(2.0f);
			upv.LeftRudderRotation = (upv.TurnRate.x + upv.TurnRate.y) * 8;
			upv.RightRudderRotation = (upv.TurnRate.x + -upv.TurnRate.y) * 8;

			if (upv.Velocity)
			{
				pos = GetJointPosition(upvItem, UpvBites[UPV_BITE_TURBINE].meshNum, UpvBites[UPV_BITE_TURBINE].Position);

				TriggerUpvMistEffect(pos.x, pos.y + UPV_SHIFT, pos.z, abs((int)round(upv.Velocity)), upvItem->Pose.Orientation.y + ANGLE(180.0f));

				if ((GetRandomControl() & 1) == 0)
				{
					auto pos2 = Vector3i(
						pos.x + (GetRandomControl() & 63) - 32,
						pos.y + UPV_SHIFT,
						pos.z + (GetRandomControl() & 63) - 32
					);
					short probedRoomNumber = GetCollision(pos2.x, pos2.y, pos2.z, upvItem->RoomNumber).RoomNumber;
				
					CreateBubble(&pos2, probedRoomNumber, 4, 8, BUBBLE_FLAG_CLUMP, 0, 0, 0);
				}
			}
		}
	
		for (int lp = 0; lp < 2; lp++)
		{
			int random = 31 - (GetRandomControl() & 3);
			pos = GetJointPosition(upvItem, UpvBites[UPV_BITE_FRONT_LIGHT].meshNum, UpvBites[UPV_BITE_FRONT_LIGHT].Position);

			GameVector source;
			if (lp == 1)
			{
				auto target = GameVector(pos.x, pos.y, pos.z, upvItem->RoomNumber);
				LOS(&source, &target);
				pos = Vector3i(target.x, target.y, target.z);
			}
			else
				source = GameVector(pos.x, pos.y, pos.z, upvItem->RoomNumber);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 16 + (lp << 3), random, random, random);
		}

		if (upv.HarpoonTimer)
			upv.HarpoonTimer--;
	}

	void TriggerUpvMistEffect(long x, long y, long z, long velocity, short angle)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;

		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;

		sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 12;
		sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = x + ((GetRandomControl() & 15) - 8);
		sptr->y = y + ((GetRandomControl() & 15) - 8);
		sptr->z = z + ((GetRandomControl() & 15) - 8);
		long zv = velocity * phd_cos(angle) / 4;
		long xv = velocity * phd_sin(angle) / 4;
		sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
		sptr->yVel = 0;
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

		sptr->scalar = 3;
		sptr->gravity = sptr->maxYvel = 0;
		long size = (GetRandomControl() & 7) + (velocity / 2) + 16;
		sptr->size = sptr->sSize = size / 4;
		sptr->dSize = size;
	}
}
