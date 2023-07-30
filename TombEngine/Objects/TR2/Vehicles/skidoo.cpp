#include "framework.h"
#include "Objects/TR2/Vehicles/Skidoo.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/effects/simple_particle.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/TR2/Vehicles/SkidooInfo.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Sound/sound.h"

using namespace TEN::Input;
using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	constexpr auto SKIDOO_HEIGHT	= LARA_HEIGHT;
	constexpr auto SKIDOO_RADIUS	= 500;
	constexpr auto SKIDOO_FRONT		= 550;
	constexpr auto SKIDOO_SIDE		= 260;
	constexpr auto SKIDOO_SLIP		= 100;
	constexpr auto SKIDOO_SLIP_SIDE = 50;
	
	constexpr auto SKIDOO_VELOCITY_ACCEL		 = 10.0f;
	constexpr auto SKIDOO_VELOCITY_DECEL		 = 2.0f;
	constexpr auto SKIDOO_VELOCITY_BRAKE_DECEL	 = 5.0f;
	constexpr auto SKIDOO_REVERSE_VELOCITY_ACCEL = 5.0f;

	constexpr auto SKIDOO_SLOW_VELOCITY_MAX	   = 50.0f;
	constexpr auto SKIDOO_NORMAL_VELOCITY_MAX  = 100.0f;
	constexpr auto SKIDOO_FAST_VELOCITY_MAX    = 150.0f;
	constexpr auto SKIDOO_TURN_VELOCITY_MAX    = 15.0f;
	constexpr auto SKIDOO_REVERSE_VELOCITY_MAX = 30.0f;
	
	constexpr auto SKIDOO_STEP_HEIGHT		= CLICK(1);
	constexpr auto SKIDOO_BOUNCE			= (SKIDOO_NORMAL_VELOCITY_MAX / 2) / 256;
	constexpr auto SKIDOO_KICK				= -80;
	constexpr auto SKIDOO_MOUNT_DISTANCE	= CLICK(2);
	constexpr auto SKIDOO_DISMOUNT_DISTANCE = 295;
	constexpr auto SKIDOO_DAMAGE_START		= 140;
	constexpr auto SKIDOO_DAMAGE_LENGTH		= 14;
	constexpr auto SKIDOO_WAKE_OFFSET = Vector3(SKIDOO_SIDE, 0, SKIDOO_FRONT / 2);

	constexpr auto SKIDOO_TURN_RATE_ACCEL		   = ANGLE(2.5f);
	constexpr auto SKIDOO_TURN_RATE_DECEL		   = ANGLE(2.0f);
	constexpr auto SKIDOO_TURN_RATE_MAX			   = ANGLE(6.0f);
	constexpr auto SKIDOO_MOMENTUM_TURN_RATE_ACCEL = ANGLE(3.0f);
	constexpr auto SKIDOO_MOMENTUM_TURN_RATE_MAX   = ANGLE(150.0f);

	const auto SkidooMountTypes = std::vector<VehicleMountType>
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};
	const auto SkidooDismountTypes = std::vector<VehicleDismountType>
	{
		VehicleDismountType::Left,
		VehicleDismountType::Right,
		VehicleDismountType::Fall
	};

	const std::vector<VehicleMountType> SkidooMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	enum SkidooState
	{
		SKIDOO_STATE_DRIVE = 0,
		SKIDOO_STATE_MOUNT_RIGHT = 1,
		SKIDOO_STATE_LEFT = 2,
		SKIDOO_STATE_RIGHT = 3,
		SKIDOO_STATE_FALL = 4,
		SKIDOO_STATE_IMPACT = 5,
		SKIDOO_STATE_MOUNT_LEFT = 6,
		SKIDOO_STATE_DISMOUNT_LEFT = 7,
		SKIDOO_STATE_IDLE = 8,
		SKIDOO_STATE_DISMOUNT_RIGHT = 9,
		SKIDOO_STATE_DISMOUNT_FALL = 10,
		SKIDOO_STATE_IDLE_DEATH = 11,
		SKIDOO_STATE_DRIVE_DEATH = 12
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
		SKIDOO_ANIM_IMPACT_LEFT = 11,
		SKIDOO_ANIM_IMPACT_RIGHT = 12,
		SKIDOO_ANIM_IMPACT_FRONT = 13,
		SKIDOO_ANIM_IMPACT_BACK = 14,
		SKIDOO_ANIM_IDLE = 15,
		SKIDOO_ANIM_DISMOUNT_RIGHT = 16,
		SKIDOO_ANIM_UNK = 17,				// TODO
		SKIDOO_ANIM_MOUNT_LEFT = 18,
		SKIDOO_ANIM_DISMOUNT_LEFT = 19,
		SKIDOO_ANIM_DISMOUNT_FALL = 20,
		SKIDOO_ANIM_IDLE_DEATH = 21,
		SKIDOO_ANIM_FALL_DEATH = 22
	};

	SkidooInfo& GetSkidooInfo(ItemInfo& skidooItem)
	{
		return (SkidooInfo&)skidooItem.Data;
	}

	void InitializeSkidoo(short itemNumber)
	{
		auto& skidooItem = g_Level.Items[itemNumber];
		skidooItem.Data = SkidooInfo();
		auto& skidoo = GetSkidooInfo(skidooItem);

		if (skidooItem.Status != ITEM_ACTIVE)
		{
			AddActiveItem(itemNumber);
			skidooItem.Status = ITEM_ACTIVE;
		}

		if (skidooItem.ObjectNumber == ID_SNOWMOBILE_GUN)
			skidoo.IsArmed = true;

		skidoo.MomentumAngle = skidooItem.Pose.Orientation.y;
	}

	bool SkidooControl(ItemInfo& laraItem, CollisionInfo& coll)
	{
		auto& player = GetLaraInfo(laraItem);
		auto& skidooItem = g_Level.Items[player.Context.Vehicle];
		auto& skidoo = GetSkidooInfo(skidooItem);

		Vector3i frontLeft, frontRight;
		auto collide = SkidooDynamics(skidooItem, laraItem);
		auto heightFrontLeft = GetVehicleHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true, frontLeft);
		auto heightFrontRight = GetVehicleHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true, frontRight);

		auto probe = GetCollision(&skidooItem);

		TestTriggers(&skidooItem, true);
		TestTriggers(&skidooItem, false);

		bool isDead = false;
		int drive = 0;

		if (laraItem.HitPoints <= 0)
		{
			isDead = true;
			ClearAction(In::Forward);
			ClearAction(In::Back);
			ClearAction(In::Left);
			ClearAction(In::Right);
		}
		else if (laraItem.Animation.ActiveState == SKIDOO_STATE_DISMOUNT_FALL)
		{
			isDead = true;
			collide = VehicleImpactType::None;
		}

		int height = probe.Position.Floor;
		int pitch = 0;

		if (skidooItem.Flags & IFLAG_INVISIBLE)
		{
			drive = 0;
			collide = VehicleImpactType::None;
		}
		else
		{
			switch (laraItem.Animation.ActiveState)
			{
			case SKIDOO_STATE_MOUNT_LEFT:  // Unused?
			case SKIDOO_STATE_MOUNT_RIGHT: // Used for both?
			case SKIDOO_STATE_DISMOUNT_LEFT:
			case SKIDOO_STATE_DISMOUNT_RIGHT:
			case SKIDOO_STATE_DISMOUNT_FALL:
				drive = -1;
				collide = VehicleImpactType::None;

				break;

			default:
				drive = SkidooUserControl(skidooItem, laraItem, height, &pitch);
				break;
			}
		}

		if (drive > 0)
		{
			skidoo.TrackMesh = ((skidoo.TrackMesh & 3) == 1) ? 2 : 1;
			skidoo.Pitch += (pitch - skidoo.Pitch) / 4;

			auto pitch = std::clamp(0.5f + (float)abs(skidoo.Pitch) / (float)SKIDOO_NORMAL_VELOCITY_MAX, 0.6f, 1.4f);
			SoundEffect(skidoo.Pitch ? SFX_TR2_VEHICLE_SNOWMOBILE_MOVING : SFX_TR2_VEHICLE_SNOWMOBILE_ACCELERATE, &skidooItem.Pose, SoundEnvironment::Land, pitch);
		}
		else
		{
			skidoo.TrackMesh = 0;
			if (!drive)
				SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidooItem.Pose);
			skidoo.Pitch = 0;
		}

		skidooItem.Floor = height;

		skidoo.LeftVerticalVelocity = DoVehicleDynamics(heightFrontLeft, skidoo.LeftVerticalVelocity, SKIDOO_BOUNCE, SKIDOO_KICK, frontLeft.y);
		skidoo.RightVerticalVelocity = DoVehicleDynamics(heightFrontRight, skidoo.RightVerticalVelocity, SKIDOO_BOUNCE, SKIDOO_KICK, frontRight.y);
		skidooItem.Animation.Velocity.y = DoVehicleDynamics(height, skidooItem.Animation.Velocity.y, SKIDOO_BOUNCE, SKIDOO_KICK, skidooItem.Pose.Position.y);
		skidooItem.Animation.Velocity.z = DoVehicleWaterMovement(skidooItem, laraItem, skidooItem.Animation.Velocity.z, SKIDOO_RADIUS, skidoo.TurnRate, SKIDOO_WAKE_OFFSET);

		height = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(SKIDOO_FRONT, skidooItem.Pose.Position.y - height);
		short zRot = phd_atan(SKIDOO_SIDE, height - frontLeft.y);

		skidooItem.Pose.Orientation.x += (xRot - skidooItem.Pose.Orientation.x) / 2;
		skidooItem.Pose.Orientation.z += (zRot - skidooItem.Pose.Orientation.z) / 2;

		if (skidooItem.Flags & IFLAG_INVISIBLE)
		{
			if (probe.RoomNumber != skidooItem.RoomNumber)
			{
				ItemNewRoom(player.Context.Vehicle, probe.RoomNumber);
				ItemNewRoom(laraItem.Index, probe.RoomNumber);
			}

			AnimateItem(&laraItem);

			if (skidooItem.Pose.Position.y == skidooItem.Floor)
				ExplodeVehicle(laraItem, skidooItem);

			return false;
		}

		AnimateSkidoo(skidooItem, laraItem, collide, isDead);

		if (probe.RoomNumber != skidooItem.RoomNumber)
		{
			ItemNewRoom(player.Context.Vehicle, probe.RoomNumber);
			ItemNewRoom(laraItem.Index, probe.RoomNumber);
		}

		if (laraItem.Animation.ActiveState != SKIDOO_STATE_DRIVE_DEATH)
		{
			laraItem.Pose.Position = skidooItem.Pose.Position;
			laraItem.Pose.Orientation.y = skidooItem.Pose.Orientation.y;

			if (drive >= 0)
			{
				laraItem.Pose.Orientation.x = skidooItem.Pose.Orientation.x;
				laraItem.Pose.Orientation.z = skidooItem.Pose.Orientation.z;
			}
			else
			{
				laraItem.Pose.Orientation.x = laraItem.Pose.Orientation.z = 0;
			}
		}
		else
		{
			laraItem.Pose.Orientation.x = laraItem.Pose.Orientation.z = 0;
		}

		AnimateItem(&laraItem);

		if (!isDead && drive >= 0 && skidoo.IsArmed)
			HandleSkidooGuns(skidooItem, laraItem);

		if (!isDead)
		{
			SyncVehicleAnimation(skidooItem, laraItem);
		}
		else
		{
			SetAnimation(skidooItem, ID_SNOWMOBILE, SKIDOO_ANIM_IDLE);
		}

		if (skidooItem.Animation.Velocity.z && skidooItem.Floor == skidooItem.Pose.Position.y)
		{
			SpawnSkidooSnow(skidooItem);

			if (skidooItem.Animation.Velocity.z < 50)
				SpawnSkidooSnow(skidooItem);
		}

		return DoSkidooDismount(skidooItem, laraItem);
	}

	VehicleImpactType SkidooDynamics(ItemInfo& skidooItem, ItemInfo& laraItem)
	{
		auto& skidoo = GetSkidooInfo(skidooItem);

		// Get point collision at vehicle corners.
		auto prevPointFrontLeft = GetVehicleCollision(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true);
		auto prevPointFrontRight = GetVehicleCollision(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true);
		auto prevPointBackLeft = GetVehicleCollision(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, true);
		auto prevPointBackRight = GetVehicleCollision(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, true);
		auto prevPos = skidooItem.Pose.Position;
			
		// Apply rotations and determine angle of momentum.
		if (skidooItem.Pose.Position.y > (skidooItem.Floor - SKIDOO_STEP_HEIGHT))
		{
			ResetVehicleTurnRateY(skidoo.TurnRate, SKIDOO_TURN_RATE_DECEL);
			skidooItem.Pose.Orientation.y += skidoo.TurnRate + skidoo.ExtraRotation;

			short rotation = skidooItem.Pose.Orientation.y - skidoo.MomentumAngle;
			if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = -SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo.MomentumAngle = skidooItem.Pose.Orientation.y - rotation;
				}
				else
				{
					skidoo.MomentumAngle -= SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
				}
			}
			else if (rotation > SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation > SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo.MomentumAngle = skidooItem.Pose.Orientation.y - rotation;
				}
				else
				{
					skidoo.MomentumAngle += SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
				}
			}
			else
			{
				skidoo.MomentumAngle = skidooItem.Pose.Orientation.y;
			}
		}
		else
		{
			skidooItem.Pose.Orientation.y += skidoo.TurnRate + skidoo.ExtraRotation;
		}

		// Translate vehicle according to momentum angle.
		TranslateItem(&skidooItem, skidoo.MomentumAngle, skidooItem.Animation.Velocity.z);

		// Apply slip. TODO: Determine what "slip" is exactly.
		int slip = SKIDOO_SLIP * phd_sin(skidooItem.Pose.Orientation.x);
		if (abs(slip) > (SKIDOO_SLIP / 2))
		{
			skidooItem.Pose.Position.x -= slip * phd_sin(skidooItem.Pose.Orientation.y);
			skidooItem.Pose.Position.z -= slip * phd_cos(skidooItem.Pose.Orientation.y);
		}

		slip = SKIDOO_SLIP_SIDE * phd_sin(skidooItem.Pose.Orientation.z);
		if (abs(slip) > (SKIDOO_SLIP_SIDE / 2))
		{
			skidooItem.Pose.Position.x += slip * phd_cos(skidooItem.Pose.Orientation.y);
			skidooItem.Pose.Position.z -= slip * phd_sin(skidooItem.Pose.Orientation.y);
		}

		// Store previous 2D position to determine movement delta later.
		auto moved = Vector3i(skidooItem.Pose.Position.x, 0, skidooItem.Pose.Position.z);

		// Process entity collision.
		if (!(skidooItem.Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(skidooItem, SKIDOO_RADIUS);

		if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		{
			auto leftProbe = GetCollision(&skidooItem, skidooItem.Pose.Position.y, SKIDOO_FRONT, -SKIDOO_SIDE, -SKIDOO_HEIGHT);
			auto rightProbe = GetCollision(&skidooItem, skidooItem.Pose.Position.y, SKIDOO_FRONT, SKIDOO_SIDE, -SKIDOO_HEIGHT);

			if (leftProbe.Position.Floor < (skidooItem.Pose.Position.y - SKIDOO_HEIGHT))
			{
				skidooItem.Pose.Orientation.y -= ANGLE(2.0f);
			}
			else if (rightProbe.Position.Floor < (skidooItem.Pose.Position.y - SKIDOO_HEIGHT))
			{
				skidooItem.Pose.Orientation.y += ANGLE(2.0f);
			}
		}

		// Apply shifts.
		short extraRot = 0;
		CalculateVehicleShift(skidooItem, extraRot, prevPointBackLeft, SKIDOO_HEIGHT, -SKIDOO_FRONT, -SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, extraRot, prevPointBackRight, SKIDOO_HEIGHT, -SKIDOO_FRONT, SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, extraRot, prevPointFrontLeft, SKIDOO_HEIGHT, SKIDOO_FRONT, -SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, extraRot, prevPointFrontRight, SKIDOO_HEIGHT, SKIDOO_FRONT, SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);

		auto probe = GetCollision(&skidooItem);
		if (probe.Position.Floor < (skidooItem.Pose.Position.y - SKIDOO_STEP_HEIGHT) ||
			abs(probe.Position.Ceiling - probe.Position.Floor) <= SKIDOO_HEIGHT)
		{
			DoVehicleShift(skidooItem, skidooItem.Pose.Position, prevPos);
		}

		skidoo.ExtraRotation = extraRot;

		// Determine whether wall impact occurred and affect vehicle accordingly.
		auto impactDirection = GetVehicleImpactType(skidooItem, moved);
		if (impactDirection != VehicleImpactType::None)
		{
			skidooItem.Animation.Velocity.z = 0;

			/*int newVelocity = (skidooItem.Pose.Position.z - prevPos.z) * phd_cos(skidoo.MomentumAngle) + (skidooItem.Pose.Position.x - prevPos.x) * phd_sin(skidoo.MomentumAngle);
			if (skidooItem.Animation.Velocity > (SKIDOO_NORMAL_VELOCITY_MAX + SKIDOO_VELOCITY_ACCEL) &&
				newVelocity < (skidooItem.Animation.Velocity - 10))
			{
				DoDamage(laraItem, (skidooItem.Animation.Velocity - newVelocity) / 2);
			}

			if (skidooItem.Animation.Velocity > 0 && newVelocity < skidooItem.Animation.Velocity)
				skidooItem.Animation.Velocity = (newVelocity < 0) ? 0 : newVelocity;
			else if (skidooItem.Animation.Velocity < 0 && newVelocity > skidooItem.Animation.Velocity)
				skidooItem.Animation.Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (skidooItem.Animation.Velocity < SKIDOO_REVERSE_VELOCITY_MAX)
				skidooItem.Animation.Velocity = SKIDOO_REVERSE_VELOCITY_MAX;*/
		}

		return impactDirection;
	}

	bool SkidooUserControl(ItemInfo& skidooItem, ItemInfo& laraItem, int height, int* pitch)
	{
		auto& skidoo = GetSkidooInfo(skidooItem);

		bool drive = false;

		if (skidooItem.Pose.Position.y >= (height - SKIDOO_STEP_HEIGHT))
		{
			*pitch = skidooItem.Animation.Velocity.z + (height - skidooItem.Pose.Position.y);

			if (TrInput & IN_LOOK && skidooItem.Animation.Velocity.z == 0)
				LookUpDown(&laraItem);

			if (IsHeld(In::Left) || IsHeld(In::Right))
				ModulateVehicleTurnRateY(skidoo.TurnRate, SKIDOO_TURN_RATE_ACCEL, -SKIDOO_TURN_RATE_MAX, SKIDOO_TURN_RATE_MAX);

			if (IsHeld(In::Reverse))
			{
				if (skidooItem.Animation.Velocity.z > 0)
					skidooItem.Animation.Velocity.z -= SKIDOO_VELOCITY_BRAKE_DECEL;
				else
				{
					drive = true;

					if (skidooItem.Animation.Velocity.z > -SKIDOO_REVERSE_VELOCITY_MAX)
						skidooItem.Animation.Velocity.z -= SKIDOO_REVERSE_VELOCITY_ACCEL;
				}
			}
			else if (IsHeld(In::Accelerate))
			{
				drive = true;

				float maxVelocity = 0.0f;
				if (IsHeld(In::Slow))
				{
					maxVelocity = SKIDOO_SLOW_VELOCITY_MAX;
				}
				else if (IsHeld(In::Speed))
				{
					maxVelocity = SKIDOO_FAST_VELOCITY_MAX;
				}
				else
				{
					maxVelocity = SKIDOO_NORMAL_VELOCITY_MAX;
				}

				if (skidooItem.Animation.Velocity.z < maxVelocity)
				{
					skidooItem.Animation.Velocity.z += (SKIDOO_VELOCITY_ACCEL / 2) + (SKIDOO_VELOCITY_ACCEL * (skidooItem.Animation.Velocity.z / (2 * maxVelocity)));
				}
				else if (skidooItem.Animation.Velocity.z > (maxVelocity + SKIDOO_VELOCITY_DECEL))
				{
					skidooItem.Animation.Velocity.z -= SKIDOO_VELOCITY_DECEL;
				}
			}
			else if ((IsHeld(In::Left) || IsHeld(In::Right)) &&
				skidooItem.Animation.Velocity.z >= 0.0f &&
				skidooItem.Animation.Velocity.z < SKIDOO_TURN_VELOCITY_MAX)
			{
				drive = true;
				skidooItem.Animation.Velocity.z = SKIDOO_TURN_VELOCITY_MAX;
			}
			else if (skidooItem.Animation.Velocity.z > SKIDOO_VELOCITY_DECEL)
			{
				skidooItem.Animation.Velocity.z -= SKIDOO_VELOCITY_DECEL;

				if ((GetRandomControl() & 0x7f) < skidooItem.Animation.Velocity.z)
					drive = true;
			}
			else
			{
				skidooItem.Animation.Velocity.z = 0.0f;
			}
		}
		else if (IsHeld(In::Accelerate) || IsHeld(In::Reverse))
		{
			drive = true;
			*pitch = skidoo.Pitch + 50;
		}

		return drive;
	}

	void AnimateSkidoo(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleImpactType impactDirection, bool isDead)
	{
		const auto& skidoo = GetSkidooInfo(skidooItem);

		if (!isDead &&
			laraItem.Animation.ActiveState != SKIDOO_STATE_FALL &&
			skidooItem.Animation.Velocity.y > 0 &&
			skidooItem.Pose.Position.y != skidooItem.Floor)
		{
			laraItem.Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_LEAP_START;
			laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
			laraItem.Animation.ActiveState = SKIDOO_STATE_FALL;
			laraItem.Animation.TargetState = SKIDOO_STATE_FALL;
		}
		else if (!isDead && impactDirection != VehicleImpactType::None &&
			laraItem.Animation.ActiveState != SKIDOO_STATE_FALL &&
			abs(skidooItem.Animation.Velocity.z) >= SKIDOO_REVERSE_VELOCITY_MAX)
		{
			DoSkidooImpact(skidooItem, laraItem, impactDirection);
		}
		else
		{
			switch (laraItem.Animation.ActiveState)
			{
			case SKIDOO_STATE_IDLE:

				if (isDead)
				{
					laraItem.Animation.TargetState = SKIDOO_STATE_IDLE_DEATH;
					break;
				}

				laraItem.Animation.TargetState = SKIDOO_STATE_IDLE;

				if (IsHeld(In::Brake))
				{
					if (IsHeld(In::Right) &&
						TestSkidooDismount(skidooItem, SKIDOO_STATE_DISMOUNT_RIGHT))
					{
						laraItem.Animation.TargetState = SKIDOO_STATE_DISMOUNT_RIGHT;
						skidooItem.Animation.Velocity.z = 0;
					}
					else if (IsHeld(In::Left) &&
						TestSkidooDismount(skidooItem, SKIDOO_STATE_DISMOUNT_LEFT))
					{
						laraItem.Animation.TargetState = SKIDOO_STATE_DISMOUNT_LEFT;
						skidooItem.Animation.Velocity.z = 0;
					}
				}
				else if (IsHeld(In::Left))
				{
					if (skidooItem.Animation.Velocity.z >= 0.0f)
						laraItem.Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem.Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (IsHeld(In::Right))
				{
					if (skidooItem.Animation.Velocity.z >= 0.0f)
						laraItem.Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem.Animation.TargetState = SKIDOO_STATE_LEFT;
				}
				else if (IsHeld(In::Accelerate) || IsHeld(In::Reverse))
				{
					laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_DRIVE:
				if (!skidooItem.Animation.Velocity.z)
					laraItem.Animation.TargetState = SKIDOO_STATE_IDLE;

				if (isDead)
				{
					laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE_DEATH;
				}
				else if (IsHeld(In::Left))
				{
					if (skidooItem.Animation.Velocity.z >= 0)
						laraItem.Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem.Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (IsHeld(In::Right))
				{
					if (skidooItem.Animation.Velocity.z >= 0)
						laraItem.Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem.Animation.TargetState = SKIDOO_STATE_LEFT;
				}

				break;

			case SKIDOO_STATE_LEFT:
				if (skidooItem.Animation.Velocity.z >= 0)
				{
					if (!IsHeld(In::Left))
						laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!IsHeld(In::Right))
						laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_RIGHT:
				if (skidooItem.Animation.Velocity.z >= 0)
				{
					if (!IsHeld(In::Right))
						laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!IsHeld(In::Left))
						laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_FALL:
				if (skidooItem.Animation.Velocity.y <= 0 ||
					skidoo.LeftVerticalVelocity <= 0 ||
					skidoo.RightVerticalVelocity <= 0)
				{
					laraItem.Animation.TargetState = SKIDOO_STATE_DRIVE;
					SoundEffect(SFX_TR2_VEHICLE_IMPACT3, &skidooItem.Pose);
				}
				else if (skidooItem.Animation.Velocity.y > (SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH))
					laraItem.Animation.TargetState = SKIDOO_STATE_DISMOUNT_FALL;

				break;
			}
		}
	}

	void SkidooPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& skidooItem = g_Level.Items[itemNumber];
		auto& player = GetLaraInfo(*laraItem);

		if (laraItem->HitPoints < 0)
			return;

		if (player.Context.Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(skidooItem, *laraItem, *coll, SkidooMountTypes, SKIDOO_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
		{
			ObjectCollision(itemNumber, laraItem, coll);
			return;
		}

		SetLaraVehicle(*laraItem, &skidooItem);
		DoSkidooMount(skidooItem, *laraItem, mountType);
	}

	void DoSkidooMount(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleMountType mountType)
	{
		auto& player = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			SetAnimation(laraItem, ID_SNOWMOBILE_LARA_ANIMS, SKIDOO_ANIM_IDLE);
			break;

		case VehicleMountType::Left:
			SetAnimation(laraItem, ID_SNOWMOBILE_LARA_ANIMS, SKIDOO_ANIM_MOUNT_LEFT);
			break;

		default:
		case VehicleMountType::Right:
			SetAnimation(laraItem, ID_SNOWMOBILE_LARA_ANIMS, SKIDOO_ANIM_MOUNT_RIGHT);
			break;
		}

		DoVehicleFlareDiscard(laraItem);
		laraItem.Pose.Position = skidooItem.Pose.Position;
		laraItem.Pose.Orientation = EulerAngles(0, skidooItem.Pose.Orientation.y, 0);
		player.Control.HandStatus = HandStatus::Busy;
		skidooItem.Collidable = true;
	}

	bool TestSkidooDismount(ItemInfo& skidooItem, int direction)
	{
		short angle = skidooItem.Pose.Orientation.y + ((direction == SKIDOO_STATE_DISMOUNT_LEFT) ? ANGLE(90.0f) : ANGLE(-90.0f));

		auto probe = GetCollision(&skidooItem, angle, -SKIDOO_DISMOUNT_DISTANCE);

		if ((probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT) ||
			abs(probe.Position.Floor - skidooItem.Pose.Position.y) > CLICK(2) ||
			((probe.Position.Ceiling - skidooItem.Pose.Position.y) > -LARA_HEIGHT ||
				(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
		{
			return false;
		}

		return true;
	}

	bool DoSkidooDismount(ItemInfo& skidooItem, ItemInfo& laraItem)
	{
		auto& player = GetLaraInfo(laraItem);

		if (player.Context.Vehicle != NO_ITEM)
		{
			if ((laraItem.Animation.ActiveState == SKIDOO_STATE_DISMOUNT_RIGHT || laraItem.Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT) &&
				laraItem.Animation.FrameNumber == g_Level.Anims[laraItem.Animation.AnimNumber].frameEnd)
			{
				if (laraItem.Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT)
					laraItem.Pose.Orientation.y += ANGLE(90.0f);
				else
					laraItem.Pose.Orientation.y -= ANGLE(90.0f);

				SetAnimation(&laraItem, LA_STAND_IDLE);
				TranslateItem(&laraItem, laraItem.Pose.Orientation.y, -SKIDOO_DISMOUNT_DISTANCE);
				laraItem.Pose.Orientation.x = 0;
				laraItem.Pose.Orientation.z = 0;
				player.Control.HandStatus = HandStatus::Free;
				player.Context.Vehicle = NO_ITEM;
			}
			else if (laraItem.Animation.ActiveState == SKIDOO_STATE_DISMOUNT_FALL &&
				(skidooItem.Pose.Position.y == skidooItem.Floor || TestLastFrame(&laraItem)))
			{
				SetAnimation(&laraItem, LA_FREEFALL);

				if (skidooItem.Pose.Position.y == skidooItem.Floor)
				{
					laraItem.Animation.TargetState = LS_DEATH;
					laraItem.Animation.Velocity.y = SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH;
					laraItem.Animation.Velocity.z = 0.0f;
					ExplodeVehicle(laraItem, skidooItem);
				}
				else
				{
					laraItem.Animation.TargetState = LS_FREEFALL;
					laraItem.Pose.Position.y -= 200;
					laraItem.Animation.Velocity = skidooItem.Animation.Velocity;
					laraItem.Animation.Velocity.y = skidooItem.Animation.Velocity.y;
					SoundEffect(SFX_TR4_LARA_FALL, &laraItem.Pose);
				}

				laraItem.Pose.Orientation.x = 0;
				laraItem.Pose.Orientation.z = 0;
				laraItem.Animation.IsAirborne = true;
				player.Control.MoveAngle = skidooItem.Pose.Orientation.y;
				player.Control.HandStatus = HandStatus::Free;
				skidooItem.Collidable = false;
				skidooItem.Flags |= IFLAG_INVISIBLE;

				return false;
			}

			return true;
		}
		else
		{
			return true;
		}
	}

	void DoSkidooImpact(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleImpactType impact)
	{
		if (laraItem.Animation.ActiveState == SKIDOO_STATE_IMPACT)
			return;

		int animNumber = 0;
		switch (impact)
		{
		default:
		case VehicleImpactType::Front:
			animNumber = SKIDOO_ANIM_IMPACT_FRONT;
			break;

		case VehicleImpactType::Back:
			animNumber = SKIDOO_ANIM_IMPACT_BACK;
			break;

		case VehicleImpactType::Left:
			animNumber = SKIDOO_ANIM_IMPACT_LEFT;
			break;

		case VehicleImpactType::Right:
			animNumber = SKIDOO_ANIM_IMPACT_RIGHT;
			break;
		}
		SetAnimation(laraItem, ID_SNOWMOBILE_LARA_ANIMS, animNumber);

		auto sfx = (impact == VehicleImpactType::Front) ? SFX_TR2_VEHICLE_IMPACT1 : SFX_TR2_VEHICLE_IMPACT1;
		SoundEffect(sfx, &skidooItem.Pose);
	}

	void HandleSkidooGuns(ItemInfo& skidooItem, ItemInfo& laraItem)
	{
		auto& skidoo = GetSkidooInfo(skidooItem);
		auto& player = GetLaraInfo(laraItem);
		const auto& weapon = Weapons[(int)LaraWeaponType::Snowmobile];

		FindNewTarget(laraItem, weapon);
		AimWeapon(laraItem, player.RightArm, weapon);

		if (IsHeld(In::Fire) && !skidooItem.ItemFlags[0])
		{
			auto angles = EulerAngles(
				player.RightArm.Orientation.x,
				player.RightArm.Orientation.y + laraItem.Pose.Orientation.y,
				0);

			if ((int)FireWeapon(LaraWeaponType::Pistol, *player.TargetEntity, laraItem, angles) +
				(int)FireWeapon(LaraWeaponType::Pistol, *player.TargetEntity, laraItem, angles))
			{
				skidoo.FlashTimer = 2;
				SoundEffect(weapon.SampleNum, &laraItem.Pose);
				skidooItem.ItemFlags[0] = 4;
			}
		}

		if (skidooItem.ItemFlags[0])
			skidooItem.ItemFlags[0]--;
	}

	void SpawnSkidooSnow(ItemInfo& skidooItem)
	{
		auto material = GetCollision(&skidooItem).BottomBlock->Material;
		if (material == MaterialType::Ice || material == MaterialType::Snow)
			TEN::Effects::TriggerSnowmobileSnow(&skidooItem);
	}
}
