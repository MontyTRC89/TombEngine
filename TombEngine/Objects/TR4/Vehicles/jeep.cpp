#include "framework.h"
#include "Objects/TR4/Vehicles/jeep.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using namespace TEN::Math;

namespace TEN::Entities::Vehicles
{
	constexpr auto JEEP_FRONT	  = 550;
	constexpr auto JEEP_SIDE	  = 280;
	constexpr auto JEEP_SLIP	  = 100;
	constexpr auto JEEP_SLIP_SIDE = 128;

	constexpr auto JEEP_VELOCITY_BRAKE_DECEL = 3 * VEHICLE_VELOCITY_SCALE;

	constexpr auto JEEP_IMPACT_VELOCITY_MIN  = 42 * VEHICLE_VELOCITY_SCALE;
	constexpr auto JEEP_VELOCITY_MAX		 = 128 * VEHICLE_VELOCITY_SCALE;
	constexpr auto JEEP_REVERSE_VELOCITY_MAX = 64 * VEHICLE_VELOCITY_SCALE;
	constexpr auto JEEP_DEATH_VERTICAL_VELOCITY_MIN = 300;
	
	constexpr auto JEEP_MOUNT_DISTANCE	  = CLICK(2);
	constexpr auto JEEP_DISMOUNT_DISTANCE = 512;
	constexpr auto JEEP_BOUNCE_MIN		  = 32;
	constexpr auto JEEP_KICK_MAX		  = -80;

	const auto JEEP_TURN_RATE_DECEL = ANGLE(0.5f);

	// TODO: Simpler toggle.
	constexpr auto JEEP_IN_TOGGLE_REVERSE = IN_SPRINT;
	constexpr auto JEEP_IN_TOGGLE_FORWARD = IN_WALK;

	const auto JeepMountTypes = std::vector<VehicleMountType>
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};
	const auto JeepJoints			= std::vector<unsigned int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16 };
	const auto JeepBrakeLightJoints = std::vector<unsigned int>{ 15, 16 };

	enum JeepState
	{
		JEEP_STATE_IDLE = 0,
		JEEP_STATE_DRIVE_FORWARD = 1,
		JEEP_STATE_IMPACT_RIGHT = 2,
		JEEP_STATE_IMPACT_BACK = 3,
		JEEP_STATE_IMPACT_LEFT = 4,
		JEEP_STATE_IMPACT_FRONT = 5,
		JEEP_STATE_BRAKE = 6, // ?
		JEEP_STATE_FORWARD_LEFT = 7,
		JEEP_STATE_FORWARD_RIGHT = 8,
		JEEP_STATE_MOUNT = 9,
		JEEP_STATE_DISMOUNT = 10,
		JEEP_STATE_LEAP = 11,
		JEEP_STATE_LAND = 12,
		JEEP_STATE_REVERSE = 13,
		JEEP_STATE_REVERSE_LEFT = 14,
		JEEP_STATE_REVERSE_RIGHT = 15,
		JEEP_STATE_DEATH = 16,
		JEEP_STATE_DRIVE_BACK = 17
	};

	enum JeepAnim
	{
		JEEP_ANIM_DEATH = 0,
		JEEP_ANIM_BRAKE = 1,
		JEEP_ANIM_DRIVE_FORWARD = 2,
		JEEP_ANIM_FORWARD_LEFT_START = 3,
		JEEP_ANIM_FORWARD_LEFT = 4,
		JEEP_ANIM_FORWARD_LEFT_END = 5,
		JEEP_ANIM_FORWARD_JUMP_START = 6,
		JEEP_ANIM_JUMP = 7,
		JEEP_ANIM_FORWARD_JUMP_LAND = 8,
		JEEP_ANIM_GETIN_RIGHT = 9,
		JEEP_ANIM_IMPACT_FRONT = 10,
		JEEP_ANIM_IMPACT_LEFT = 11,
		JEEP_ANIM_IMPACT_RIGHT = 12,
		JEEP_ANIM_IMPACT_BACK = 13,
		JEEP_ANIM_IDLE = 14,
		JEEP_ANIM_FORWARD_RIGHT_START = 15,
		JEEP_ANIM_FORWARD_RIGHT = 16,
		JEEP_ANIM_FORWARD_RIGHT_END = 17,
		JEEP_ANIM_MOUNT_LEFT = 18,
		JEEP_ANIM_DISMOUNT_LEFT = 19,
		JEEP_ANIM_BACK_JUMP_START = 20,
		JEEP_ANIM_BACK_JUMP = 21,
		JEEP_ANIM_BACK_JUMP_LAND = 22,
		JEEP_ANIM_REVERSE_START = 23,
		JEEP_ANIM_REVERSE = 24,
		JEEP_ANIM_REVERSE_END = 25, //aka turn head back forwards
		JEEP_ANIM_BACK_RIGHT_START = 26,
		JEEP_ANIM_BACK_RIGHT = 27,
		JEEP_ANIM_BACK_RIGHT_END = 28,
		JEEP_ANIM_BACK_LEFT_START = 29,
		JEEP_ANIM_BACK_LEFT = 30,
		JEEP_ANIM_BACK_LEFT_END = 31,
		JEEP_ANIM_IDLE_RIGHT_START = 32,//turn steering whel right while idle
		JEEP_ANIM_IDLE_LEFT_START = 33,// blah blah left while idle
		JEEP_ANIM_IDLE_RIGHT_END = 34,//turn steering wheel straight from right while idle
		JEEP_ANIM_IDLE_LEFT_END = 35,//blah blah straight from left while idle
		JEEP_ANIM_IDLE_RIGHT_BACK_START = 36,//same as 32 but in reverse
		JEEP_ANIM_IDLE_LEFT_BACK_START = 37,//same as 33 but in reverse
		JEEP_ANIM_IDLE_BACK_RIGHT_END = 38,//same as 34 but in reverse
		JEEP_ANIM_IDLE_BACK_LEFT_END = 39,//same as 35 but in reverse
		JEEP_ANIM_IDLE_REVERSE_RIGHT = 40,//"change to reverse gear with the wheels still turned left"
		JEEP_ANIM_IDLE_REVERSE_LEFT = 41,//"change to reverse gear with the wheels still turned right"
		JEEP_ANIM_BACK_IDLE = 42,
		JEEP_ANIM_IDLE_FORWARD_LEFT = 43,//aka the opposite of 41. "change to forward gear with the wheels still turned left"
		JEEP_ANIM_IDLE_FORWARD_RIGHT = 44,//aka the opposite of 42. "change to forward gear with the wheels still turned right"
	};

	enum JeepFlags
	{
		JEEP_FLAG_FALLING = (1 << 6),
		JEEP_FLAG_DEAD	  = (1 << 7)
	};

	JeepInfo& GetJeepInfo(ItemInfo& jeepItem)
	{
		return (JeepInfo&)jeepItem.Data;
	}

	void InitialiseJeep(short itemNumber)
	{
		auto& jeepItem = g_Level.Items[itemNumber];
		jeepItem.Data = JeepInfo();
		auto& jeep = GetJeepInfo(jeepItem);

		jeepItem.MeshBits.Set(JeepJoints);
		jeep.MomentumAngle = jeepItem.Pose.Orientation.y;
	}

	void JeepPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& jeepItem = g_Level.Items[itemNumber];
		auto& jeep = GetJeepInfo(jeepItem);
		auto& player = *GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0)
			return;

		if (player.Vehicle != NO_ITEM)
			return;


		auto mountType = GetVehicleMountType(jeepItem, *laraItem, *coll, JeepMountTypes, JEEP_MOUNT_DISTANCE);

		if (mountType == VehicleMountType::None)
		{
			ObjectCollision(itemNumber, laraItem, coll);
			return;
		}

		SetLaraVehicle(*laraItem, &jeepItem);
		DoJeepMount(jeepItem, *laraItem, mountType);
	}

	int JeepControl(ItemInfo& laraItem)
	{
		auto& player = *GetLaraInfo(&laraItem);
		auto& jeepItem = g_Level.Items[player.Vehicle];
		auto& jeep = GetJeepInfo(jeepItem);

		int drive = -1;
		bool isDead = false;

		auto impactDirection = JeepDynamics(jeepItem, laraItem);

		short roomNumber = jeepItem.RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z, &roomNumber);

		int floorHeight = GetFloorHeight(floor, jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z);
		int ceiling = GetCeiling(floor, jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z);

		auto pointFrontLeft = GetVehicleCollision(jeepItem, JEEP_FRONT, -JEEP_SIDE, true);
		auto pointFrontRight = GetVehicleCollision(jeepItem, JEEP_FRONT, JEEP_SIDE, true);
		auto pointBackCenter = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), 0, true);

		roomNumber = jeepItem.RoomNumber;
		floor = GetFloor(jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z, &roomNumber);
		floorHeight = GetFloorHeight(floor, jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z);

		TestTriggers(&jeepItem, true);
		TestTriggers(&jeepItem, false);

		if (laraItem.HitPoints <= 0)
		{
			isDead = true;
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		}

		int pitch = 0;
		if (jeep.Flags)
			impactDirection = VehicleImpactDirection::None;
		else if (laraItem.Animation.ActiveState == JEEP_STATE_MOUNT)
		{
			drive = -1;
			impactDirection = VehicleImpactDirection::None;
		}
		else
			drive = JeepUserControl(jeepItem, laraItem, floorHeight, &pitch);

		if (jeep.Velocity || jeep.Revs)
		{
			jeep.Pitch = pitch;
			if (pitch >= -32768)
			{
				if (pitch > 40960)
					jeep.Pitch = 40960;
			}
			else
				jeep.Pitch = -32768;

			SoundEffect(SFX_TR4_VEHICLE_JEEP_MOVING, &jeepItem.Pose, SoundEnvironment::Land, 0.5f + jeep.Pitch / 65535.0f);
		}
		else
		{
			if (drive != -1)
				SoundEffect(SFX_TR4_VEHICLE_JEEP_IDLE, &jeepItem.Pose);
			jeep.Pitch = 0;
		}

		jeepItem.Floor = floorHeight;
		short wheelRot = jeep.Velocity / 4;
		jeep.FrontRightWheelRotation -= wheelRot;
		jeep.FrontLeftWheelRotation -= wheelRot;
		jeep.BackRightWheelRotation -= wheelRot;
		jeep.BackLeftWheelRotation -= wheelRot;

		// TODO: Check.
		/*if ((jeepItem.Pose.Position.y + jeepItem.Animation.Velocity.y) > (floorHeight - 32))
		{
			if (jeepItem.Animation.Velocity.y > 150)
				laraItem.HitPoints += 150 - jeepItem.Animation.Velocity.y;
		}*/

		int prevYPos = jeepItem.Pose.Position.y;
		jeepItem.Animation.Velocity.y = DoVehicleDynamics(floorHeight, jeepItem.Animation.Velocity.y, JEEP_BOUNCE_MIN, JEEP_KICK_MAX, jeepItem.Pose.Position.y, 1.5f);
		jeep.Velocity = DoVehicleWaterMovement(jeepItem, laraItem, jeep.Velocity, JEEP_FRONT, jeep.TurnRate);

		floorHeight = (pointFrontLeft.Position.y + pointFrontRight.Position.y) / 2;
		
		short xRot;
		if (pointBackCenter.Position.y < pointBackCenter.FloorHeight)
		{
			if (floorHeight < (pointFrontLeft.FloorHeight + pointFrontRight.FloorHeight) / 2)
			{
				xRot = phd_atan(ANGLE(0.75f), prevYPos - jeepItem.Pose.Position.y);
				if (jeep.Velocity < 0)
					xRot = -xRot;
			}
			else
				xRot = phd_atan(JEEP_FRONT, jeepItem.Pose.Position.y - floorHeight);
		}
		else
		{
			if (floorHeight < (pointFrontLeft.FloorHeight + pointFrontRight.FloorHeight) / 2)
				xRot = phd_atan(JEEP_FRONT, pointBackCenter.FloorHeight - jeepItem.Pose.Position.y);
			else
				xRot = phd_atan(ANGLE(6.0f), pointBackCenter.FloorHeight - floorHeight);
		}

		short zRot = phd_atan(ANGLE(1.9f), floorHeight - pointFrontLeft.Position.y);
		jeepItem.Pose.Orientation.x += (xRot - jeepItem.Pose.Orientation.x) / 4;
		jeepItem.Pose.Orientation.z += (zRot - jeepItem.Pose.Orientation.z) / 4;

		if (!(jeep.Flags & JEEP_FLAG_DEAD))
		{
			if (roomNumber != jeepItem.RoomNumber)
			{
				ItemNewRoom(player.Vehicle, roomNumber);
				ItemNewRoom(player.ItemNumber, roomNumber);
			}

			laraItem.Pose = jeepItem.Pose;

			AnimateJeep(jeepItem, laraItem, impactDirection, isDead);
			AnimateItem(&laraItem);

			jeepItem.Animation.AnimNumber = Objects[ID_JEEP].animIndex + laraItem.Animation.AnimNumber - Objects[ID_JEEP_LARA_ANIMS].animIndex;
			jeepItem.Animation.FrameNumber = g_Level.Anims[jeepItem.Animation.AnimNumber].frameBase + (laraItem.Animation.FrameNumber - g_Level.Anims[laraItem.Animation.AnimNumber].frameBase);

			Camera.targetElevation = -ANGLE(30.0f);
			Camera.targetDistance = SECTOR(2);

			if (jeep.Gear == 1)
				jeep.CameraElevation += (ANGLE(179.0f) - jeep.CameraElevation) / 8;
			else
				jeep.CameraElevation -= jeep.CameraElevation / 8;

			Camera.targetAngle = jeep.CameraElevation;

			if (jeep.Flags & JEEP_FLAG_FALLING && jeepItem.Pose.Position.y == jeepItem.Floor)
			{
				laraItem.MeshBits = 0;
				ExplodeVehicle(laraItem, jeepItem);
				return 0;
			}
		}

		if (laraItem.Animation.ActiveState == JEEP_STATE_MOUNT ||
			laraItem.Animation.ActiveState == JEEP_STATE_DISMOUNT)
		{
			jeep.SmokeStart = 0;
		}
		else
		{
			short speed = 0;
			short angle = 0;

			auto pos = GetJointPosition(&jeepItem, 11, Vector3i(90, 0, -500));

			if (jeepItem.Animation.Velocity.z <= 32)
			{
				if (jeep.SmokeStart >= 16)
				{
					if (GetRandomControl() & 3)
						speed = 0;
					else
						speed = ((GetRandomControl() & 0xF) + GetRandomControl() & 0x10) * 64;
				}
				else
				{
					speed = ((GetRandomControl() & 7) + GetRandomControl() & 0x10 + 2 * jeep.SmokeStart) * 64;
					jeep.SmokeStart++;
				}

				TriggerJeepExhaustSmokeEffect(pos.x, pos.y, pos.z, jeepItem.Pose.Orientation.y + -ANGLE(180.0f), speed, 0);
			}
			else if (jeepItem.Animation.Velocity.z < 64)
				TriggerJeepExhaustSmokeEffect(pos.x, pos.y, pos.z, jeepItem.Pose.Orientation.y - ANGLE(180.0f), 64 - jeepItem.Animation.Velocity.z, 1);
		}

		return DoJeepDismount(laraItem);
	}

	int JeepUserControl(ItemInfo& jeepItem, ItemInfo& laraItem, int height, int* pitch)
	{
		auto& jeep = GetJeepInfo(jeepItem);

		if (laraItem.Animation.ActiveState == JEEP_STATE_DISMOUNT || laraItem.Animation.TargetState == JEEP_STATE_DISMOUNT)
			TrInput = 0;

		if (jeep.Revs <= 16)
			jeep.Revs = 0;
		else
		{
			jeep.Velocity += jeep.Revs / 16;
			jeep.Revs -= jeep.Revs / 8;
		}

		int maxTurnRate = 0;
		int turnRateAccel = 0;

		if (jeepItem.Pose.Position.y >= (height - CLICK(1)))
		{
			if (!jeep.Velocity)
			{
				if (TrInput & IN_LOOK)
					LookUpDown(&laraItem);
			}

			if (abs(jeep.Velocity) <= (JEEP_VELOCITY_MAX / 2))
			{
				maxTurnRate = ANGLE(5.0f) * abs(jeep.Velocity) / ANGLE(90.0f);
				turnRateAccel = ANGLE(0.33f) * abs(jeep.Velocity) / ANGLE(90.0f) + ANGLE(1.0f);
			}
			else
			{
				maxTurnRate = ANGLE(5.0f);
				turnRateAccel = ANGLE(1.33f);
			}

			if (jeep.Velocity < 0)
				ModulateVehicleTurnRateY(jeep.TurnRate, turnRateAccel, -maxTurnRate, maxTurnRate, true);
			else if (jeep.Velocity > 0)
				ModulateVehicleTurnRateY(jeep.TurnRate, turnRateAccel, -maxTurnRate, maxTurnRate);

			if (TrInput & VEHICLE_IN_BRAKE)
			{
				if (jeep.Velocity < 0)
				{
					jeep.Velocity += JEEP_VELOCITY_BRAKE_DECEL;
					if (jeep.Velocity > 0)
						jeep.Velocity = 0;
				}
				else if (jeep.Velocity > 0)
				{
					jeep.Velocity -= JEEP_VELOCITY_BRAKE_DECEL;
					if (jeep.Velocity < 0)
						jeep.Velocity = 0;
				}
			}
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (jeep.Gear)
				{
					if (jeep.Gear == 1 && jeep.Velocity > -JEEP_REVERSE_VELOCITY_MAX)
						jeep.Velocity -= (abs(-JEEP_REVERSE_VELOCITY_MAX - jeep.Velocity) / 8) - 2;
					else if (jeep.Gear == 1 && jeep.Velocity < -JEEP_REVERSE_VELOCITY_MAX)
						jeep.Velocity = -JEEP_REVERSE_VELOCITY_MAX;
				}
				else
				{
					if (jeep.Velocity < JEEP_VELOCITY_MAX)
					{
						if (jeep.Velocity < (64 * VEHICLE_VELOCITY_SCALE))
							jeep.Velocity += 8 + (((64 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - jeep.Velocity) / 8);
						else if (jeep.Velocity < (112 * VEHICLE_VELOCITY_SCALE))
							jeep.Velocity += 4 + (((112 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - jeep.Velocity) / 16);
						else if (jeep.Velocity < JEEP_VELOCITY_MAX)
							jeep.Velocity += 2 + ((JEEP_VELOCITY_MAX - jeep.Velocity) / 8);
					}
					else
						jeep.Velocity = JEEP_VELOCITY_MAX;
				}

				jeep.Velocity -= abs(jeepItem.Pose.Orientation.y - jeep.MomentumAngle) / 64;
			}
			else if (jeep.Velocity > (1 * VEHICLE_VELOCITY_SCALE))
				jeep.Velocity -= 1 * VEHICLE_VELOCITY_SCALE;
			else if (jeep.Velocity < -(1 * VEHICLE_VELOCITY_SCALE))
				jeep.Velocity += 1 * VEHICLE_VELOCITY_SCALE;
			else
				jeep.Velocity = 0;

			jeepItem.Animation.Velocity.z = jeep.Velocity / VEHICLE_VELOCITY_SCALE;
			if (jeep.EngineRevs > 0xC000)
				jeep.EngineRevs = (GetRandomControl() & 0x1FF) + 48896;

			int revs = jeep.Velocity;
			if (jeep.Velocity < 0)
				revs /= 2;

			jeep.EngineRevs += (abs(revs) - jeep.EngineRevs) / 8;
		}

		if (TrInput & VEHICLE_IN_BRAKE)
		{
			auto pos = GetJointPosition(&jeepItem, 11, Vector3i(0, -144, -1024));
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
			jeepItem.MeshBits.Set(17);
			jeepItem.MeshBits.Clear(JeepBrakeLightJoints);
		}
		else
		{
			jeepItem.MeshBits.Set(JeepBrakeLightJoints);
			jeepItem.MeshBits.Clear(17);
		}

		*pitch = jeep.EngineRevs;

		return 1;
	}

	void AnimateJeep(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleImpactDirection impactDirection, bool isDead)
	{
		auto& jeep = GetJeepInfo(jeepItem);

		bool dismount = false;
		if (!isDead &&
			jeepItem.Pose.Position.y != jeepItem.Floor &&
			laraItem.Animation.ActiveState != JEEP_STATE_LEAP &&
			laraItem.Animation.ActiveState != JEEP_STATE_LAND)
		{
			if (jeep.Gear == 1)
				laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_JUMP_START;
			else
				laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_JUMP_START;

			laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
			laraItem.Animation.ActiveState = JEEP_STATE_LEAP;
			laraItem.Animation.TargetState = JEEP_STATE_LEAP;
		}
		else if (impactDirection != VehicleImpactDirection::None && !jeep.Gear && !isDead &&
			laraItem.Animation.ActiveState != JEEP_STATE_IMPACT_FRONT &&
			laraItem.Animation.ActiveState != JEEP_STATE_IMPACT_BACK &&
			laraItem.Animation.ActiveState != JEEP_STATE_IMPACT_LEFT &&
			laraItem.Animation.ActiveState != JEEP_STATE_IMPACT_RIGHT &&
			laraItem.Animation.ActiveState != JEEP_STATE_LEAP &&
			jeep.Velocity > JEEP_IMPACT_VELOCITY_MIN)
		{
			DoJeepImpact(jeepItem, laraItem, impactDirection);
		}
		else
		{
			switch (laraItem.Animation.ActiveState)
			{
			case JEEP_STATE_IDLE:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_DEATH;
				else
				{
					if ((TrInput & VEHICLE_IN_BRAKE) && (TrInput & VEHICLE_IN_LEFT) &&
						!jeep.Velocity && !jeep.DisableDismount)
					{
						if (TestJeepDismount(jeepItem, laraItem))
							laraItem.Animation.TargetState = JEEP_STATE_DISMOUNT;

						break;
					}

					if (DbInput & JEEP_IN_TOGGLE_FORWARD)
					{
						if (jeep.Gear)
							jeep.Gear--;

						break;
					}
					else if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep.Gear < 1)
						{
							jeep.Gear++;
							if (jeep.Gear == 1)
								laraItem.Animation.TargetState = JEEP_STATE_DRIVE_BACK;

							break;
						}
					}
					else
					{
						if ((TrInput & VEHICLE_IN_ACCELERATE) && !(TrInput & VEHICLE_IN_BRAKE))
						{
							laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
							break;
						}
						else if (TrInput & VEHICLE_IN_LEFT)
							laraItem.Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
						else if (TrInput & VEHICLE_IN_RIGHT)
							laraItem.Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
					}

					/*if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
					{
						if (!(DbInput & JEEP_IN_TOGGLE_REVERSE))
						{
							if ((TrInput & VEHICLE_IN_ACCELERATE) && !(TrInput & VEHICLE_IN_BRAKE))
							{
								laraItem.TargetState = JEEP_STATE_DRIVE_FORWARD;
								break;
							}
							else if (TrInput & VEHICLE_IN_LEFT)
								laraItem.TargetState = JEEP_STATE_FORWARD_LEFT;
							else if (TrInput & VEHICLE_IN_RIGHT)
								laraItem.TargetState = JEEP_STATE_FORWARD_RIGHT;
						}
						else if (jeep.Gear < 1)
						{
							jeep.Gear++;
							if (jeep.Gear == 1)
								laraItem.TargetState = JEEP_STATE_DRIVE_BACK;

						}
					}
					else
					{
						if (jeep.Gear)
							jeep.Gear--;
					}*/
				}

				break;

			case JEEP_STATE_DRIVE_FORWARD:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				else
				{
					if (jeep.Velocity > (1 * VEHICLE_VELOCITY_SCALE) ||
						TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_BRAKE))
					{
						if (TrInput & VEHICLE_IN_BRAKE)
						{
							if (jeep.Velocity <= (int)round(85.33f * VEHICLE_VELOCITY_SCALE))
								laraItem.Animation.TargetState = JEEP_STATE_IDLE;
							else
								laraItem.Animation.TargetState = JEEP_STATE_BRAKE;
						}
						else
						{
							if (TrInput & VEHICLE_IN_LEFT)
								laraItem.Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
							else if (TrInput & VEHICLE_IN_RIGHT)
								laraItem.Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
						}
					}
					else
						laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				}

				break;

			case 2:
			case 3:
			case 4:
			case 5:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_BRAKE))
					laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;

				break;

			case JEEP_STATE_BRAKE:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				else if (jeep.Velocity > (1 * VEHICLE_VELOCITY_SCALE))
				{
					if (TrInput & VEHICLE_IN_LEFT)
						laraItem.Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem.Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
				}
				else
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;

				break;

			case JEEP_STATE_FORWARD_LEFT:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep.Gear < 1)
						{
							jeep.Gear++;
							if (jeep.Gear == 1)
							{
								laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_REVERSE_RIGHT;
								laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
								laraItem.Animation.ActiveState = JEEP_STATE_REVERSE_RIGHT;
								laraItem.Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
								break;
							}
						}
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem.Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
					else if (jeep.Velocity)
						laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else
						laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				}
				else
				{
					if (jeep.Gear)
						jeep.Gear--;
				}

				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_LEFT &&
					!jeep.Velocity)
				{
					laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_START;
					laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase + JEEP_ANIM_IDLE;
				}
				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_START)
				{
					if (jeep.Velocity)
					{
						laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_LEFT;
						laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_FORWARD_RIGHT:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;

				if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep.Gear < 1)
						{
							jeep.Gear++;
							if (jeep.Gear == 1)
							{
								laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_REVERSE_LEFT;
								laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
								laraItem.Animation.ActiveState = JEEP_STATE_REVERSE_LEFT;
								laraItem.Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
								break;
							}
						}
					}
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem.Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
					else if (jeep.Velocity)
						laraItem.Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else
						laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				}
				else
				{
					if (jeep.Gear)
						jeep.Gear--;
				}

				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_RIGHT && !jeep.Velocity)
				{
					laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_START;
					laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase + 14;//hmm
				}
				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_START)
				{
					if (jeep.Velocity)
					{
						laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_RIGHT;
						laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_LEAP:
				if (jeepItem.Pose.Position.y == jeepItem.Floor)
					laraItem.Animation.TargetState = JEEP_STATE_LAND;
				else if (jeepItem.Animation.Velocity.y > JEEP_DEATH_VERTICAL_VELOCITY_MIN)
					jeep.Flags |= JEEP_FLAG_FALLING;

				break;

			case JEEP_STATE_REVERSE:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				else if (abs(jeep.Velocity) > (1 * VEHICLE_VELOCITY_SCALE))
				{
					if (TrInput & VEHICLE_IN_LEFT)
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
				}
				else
					laraItem.Animation.TargetState = JEEP_STATE_DRIVE_BACK;

				break;

			case JEEP_STATE_REVERSE_LEFT:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep.Gear < 1)
							jeep.Gear++;
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
					else
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE;
				}
				else
				{
					if (jeep.Gear)
					{
						jeep.Gear--;
						if (!jeep.Gear)
						{
							laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_FORWARD_RIGHT;
							laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
							laraItem.Animation.ActiveState = JEEP_STATE_FORWARD_RIGHT;
							laraItem.Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
							break;
						}
					}
				}

				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_LEFT && !jeep.Velocity)
				{
					laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_BACK_START;
					laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase + 14;
				}
				if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_BACK_START)
				{
					if (jeep.Velocity)
					{
						laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_LEFT;
						laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_REVERSE_RIGHT:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep.Gear < 1)
							jeep.Gear++;
					}
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
					else
						laraItem.Animation.TargetState = JEEP_STATE_REVERSE;

					if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT && !jeep.Velocity)
					{
						laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START;
						laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase + 14;
					}

					if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START)
					{
						if (jeep.Velocity)
						{
							laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT;
							laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
						}
					}

					break;
				}
				else if (!jeep.Gear || (--jeep.Gear != 0))
				{
					if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT && !jeep.Velocity)
					{
						laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START;
						laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase + 14;
					}

					if (laraItem.Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START)
					{
						if (jeep.Velocity)
						{
							laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT;
							laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
						}
					}

					break;
				}

				laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_FORWARD_RIGHT;
				laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
				laraItem.Animation.ActiveState = JEEP_STATE_FORWARD_LEFT;
				laraItem.Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
				break;

			case JEEP_STATE_DRIVE_BACK:
				if (isDead)
					laraItem.Animation.TargetState = JEEP_STATE_IDLE;
				else // if (jeep.Velocity || jeep.DisableDismount)
				{
					if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
					{
						if (DbInput & JEEP_IN_TOGGLE_REVERSE)
						{
							if (jeep.Gear < 1)
								jeep.Gear++;
						}
						else if (!(TrInput & VEHICLE_IN_ACCELERATE) || TrInput & VEHICLE_IN_BRAKE)
						{
							if (TrInput & VEHICLE_IN_LEFT)
								laraItem.Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
							else if (TrInput & VEHICLE_IN_LEFT)
								laraItem.Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
						}
						else
							laraItem.Animation.TargetState = JEEP_STATE_REVERSE;
					}
					else
					{
						if (jeep.Gear)
						{
							jeep.Gear--;
							if (!jeep.Gear)
								laraItem.Animation.TargetState = JEEP_STATE_IDLE;
						}
					}
				}

				break;

			default:
				break;
			}
		}
	}

	void DoJeepMount(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleMountType mountType)
	{
		auto& jeep = GetJeepInfo(jeepItem);
		auto& player = *GetLaraInfo(&laraItem);

		// HACK: Hardcoded jeep keys check.
		/*if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
		{
			g_Gui.SetInventoryItemChosen(NO_ITEM);
			return true;
		}
		else
		{
			if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM1))
				g_Gui.SetEnterInventory(ID_PUZZLE_ITEM1);

			return false;
		}*/
		
		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE;
			laraItem.Animation.ActiveState = JEEP_STATE_IDLE;
			laraItem.Animation.TargetState = JEEP_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_MOUNT_LEFT;
			laraItem.Animation.ActiveState = JEEP_STATE_MOUNT;
			laraItem.Animation.TargetState = JEEP_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem.Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_GETIN_RIGHT;
			laraItem.Animation.ActiveState = JEEP_STATE_MOUNT;
			laraItem.Animation.TargetState = JEEP_STATE_MOUNT;
			break;
		}
		laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		ResetLaraFlex(&laraItem);
		laraItem.Pose.Position = jeepItem.Pose.Position;
		laraItem.Pose.Orientation.y = jeepItem.Pose.Orientation.y;
		player.Control.HandStatus = HandStatus::Busy;
		player.HitDirection = -1;
		jeepItem.HitPoints = 1;
		jeepItem.Flags |= TRIGGERED;
		jeep.Revs = 0;
		jeep.Gear = 0;

		AnimateItem(&laraItem);
	}

	bool TestJeepDismount(ItemInfo& jeepItem, ItemInfo& laraItem)
	{
		auto& player = *GetLaraInfo(&laraItem);

		short angle = jeepItem.Pose.Orientation.y + ANGLE(90.0f);

		int x = jeepItem.Pose.Position.x - JEEP_DISMOUNT_DISTANCE * phd_sin(angle);
		int y = jeepItem.Pose.Position.y;
		int z = jeepItem.Pose.Position.z - JEEP_DISMOUNT_DISTANCE * phd_cos(angle);

		auto probe = GetCollision(x, y, z, jeepItem.RoomNumber);

		if (probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT)
			return false;

		if (abs(probe.Position.Floor - jeepItem.Pose.Position.y) > WALL_SIZE / 2)
			return false;

		if ((probe.Position.Ceiling - jeepItem.Pose.Position.y) > -LARA_HEIGHT ||
			(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	int DoJeepDismount(ItemInfo& laraItem)
	{
		auto& player = *GetLaraInfo(&laraItem);

		if (laraItem.Animation.ActiveState == JEEP_STATE_DISMOUNT)
		{
			if (laraItem.Animation.FrameNumber == g_Level.Anims[laraItem.Animation.AnimNumber].frameEnd)
			{
				laraItem.Pose.Orientation.y += ANGLE(90.0f);
				TranslateItem(&laraItem, laraItem.Pose.Orientation.y, -JEEP_DISMOUNT_DISTANCE);
				SetAnimation(&laraItem, LA_STAND_SOLID);
				laraItem.Pose.Orientation.x = 0;
				laraItem.Pose.Orientation.z = 0;
				player.Vehicle = NO_ITEM;
				player.Control.HandStatus = HandStatus::Free;
				return false;
			}
		}

		return true;
	}

	void DoJeepImpact(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleImpactDirection impactDirection)
	{
		switch (impactDirection)
		{
		default:
		case VehicleImpactDirection::Front:
			laraItem.Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_FRONT;
			laraItem.Animation.ActiveState = JEEP_STATE_IMPACT_FRONT;
			laraItem.Animation.TargetState = JEEP_STATE_IMPACT_FRONT;
			break;

		case VehicleImpactDirection::Back:
			laraItem.Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_BACK;
			laraItem.Animation.ActiveState = JEEP_STATE_IMPACT_BACK;
			laraItem.Animation.TargetState = JEEP_STATE_IMPACT_BACK;
			break;

		case VehicleImpactDirection::Left:
			laraItem.Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_LEFT;
			laraItem.Animation.ActiveState = JEEP_STATE_IMPACT_LEFT;
			laraItem.Animation.TargetState = JEEP_STATE_IMPACT_LEFT;
			break;

		case VehicleImpactDirection::Right:
			laraItem.Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_RIGHT;
			laraItem.Animation.ActiveState = JEEP_STATE_IMPACT_RIGHT;
			laraItem.Animation.TargetState = JEEP_STATE_IMPACT_RIGHT;
			break;
		}
		laraItem.Animation.FrameNumber = g_Level.Anims[laraItem.Animation.AnimNumber].frameBase;
	}

	VehicleImpactDirection JeepDynamics(ItemInfo& jeepItem, ItemInfo& laraItem)
	{
		auto& jeep = GetJeepInfo(jeepItem);
		auto& player = *GetLaraInfo(&laraItem);

		// Get point/room collision at vehicle corners (+ back).
		auto prevPointFrontLeft  = GetVehicleCollision(jeepItem, JEEP_FRONT, -JEEP_SIDE, true);
		auto prevPointFrontRight  = GetVehicleCollision(jeepItem, JEEP_FRONT, JEEP_SIDE, true);
		auto prevPointBackLeft = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, true);
		auto prevPointBackRight = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, true);
		auto prevPointBack = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), 0, true);

		auto prevPos = jeepItem.Pose.Position;

		short rotation = 0;
		jeep.DisableDismount = false;

		if (prevPos.y <= (jeepItem.Floor - 8))
		{
			ResetVehicleTurnRateY(jeep.TurnRate, JEEP_TURN_RATE_DECEL);
			jeepItem.Pose.Orientation.y += jeep.TurnRate + jeep.ExtraRotation;

			jeep.MomentumAngle += (jeepItem.Pose.Orientation.y - jeep.MomentumAngle) / 32;
		}
		else
		{
			if (jeep.TurnRate < -ANGLE(1.0f))
				jeep.TurnRate += ANGLE(1.0f);
			else if (jeep.TurnRate > ANGLE(1.0f))
				jeep.TurnRate -= ANGLE(1.0f);
			else
				jeep.TurnRate = 0;

			jeepItem.Pose.Orientation.y += jeep.TurnRate + jeep.ExtraRotation;

			rotation = jeepItem.Pose.Orientation.y - jeep.MomentumAngle;
			short momentum = (short)(728 - ((3 * jeep.Velocity) / 2048));

			if (!(TrInput & IN_ACTION) && jeep.Velocity > 0)
				momentum -= momentum / 4;

			if (rotation < -ANGLE(1.5f))
			{
				if (rotation < -ANGLE(75.0f))
				{
					jeepItem.Pose.Position.y -= 41;
					jeepItem.Animation.Velocity.y = -6 - (GetRandomControl() & 3);
					jeep.TurnRate = 0;
					jeep.Velocity -= jeep.Velocity / 8;
				}

				if (rotation < -ANGLE(90.0f))
					jeep.MomentumAngle = jeepItem.Pose.Orientation.y + ANGLE(90.0f);
				else
					jeep.MomentumAngle -= momentum;
			}
			else if (rotation > ANGLE(1.5f))
			{
				if (rotation > ANGLE(75.0f))
				{
					jeepItem.Pose.Position.y -= 41;
					jeepItem.Animation.Velocity.y = -6 - (GetRandomControl() & 3);
					jeep.TurnRate = 0;
					jeep.Velocity -= (jeep.Velocity / 8);
				}

				if (rotation > ANGLE(90.0f))
					jeep.MomentumAngle = jeepItem.Pose.Orientation.y - ANGLE(90.0f);
				else
					jeep.MomentumAngle += momentum;
			}
			else
				jeep.MomentumAngle = jeepItem.Pose.Orientation.y;
		}
		
		short roomNumber = jeepItem.RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z, &roomNumber);
		int height = GetFloorHeight(floor, jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z);

		short speed;
		if (jeepItem.Pose.Position.y < height)
			speed = jeepItem.Animation.Velocity.z;
		else
			speed = jeepItem.Animation.Velocity.z * phd_cos(jeepItem.Pose.Orientation.x);

		TranslateItem(&jeepItem, jeep.MomentumAngle, speed);
	
		if (jeepItem.Pose.Position.y >= height)
		{
			short slip = JEEP_SLIP * phd_sin(jeepItem.Pose.Orientation.x);

			if (abs(slip) > 16)
			{
				jeep.DisableDismount = true;

				if (slip < 0)
					jeep.Velocity += SQUARE(slip + 16) / 2;
				else
					jeep.Velocity -= SQUARE(slip - 16) / 2;
			}

			slip = JEEP_SLIP_SIDE * phd_sin(jeepItem.Pose.Orientation.z);

			if (abs(slip) > (JEEP_SLIP_SIDE / 4))
			{
				jeep.DisableDismount = true;

				if (slip >= 0)
				{
					jeepItem.Pose.Position.x += (slip - 24) * phd_sin(jeepItem.Pose.Orientation.y + ANGLE(90.0f));
					jeepItem.Pose.Position.z += (slip - 24) * phd_cos(jeepItem.Pose.Orientation.y + ANGLE(90.0f));
				}
				else
				{
					jeepItem.Pose.Position.x += (slip - 24) * phd_sin(jeepItem.Pose.Orientation.y - ANGLE(90.0f));
					jeepItem.Pose.Position.z += (slip - 24) * phd_cos(jeepItem.Pose.Orientation.y - ANGLE(90.0f));
				}
			}
		}

		if (jeep.Velocity > JEEP_VELOCITY_MAX)
			jeep.Velocity = JEEP_VELOCITY_MAX;
		else if (jeep.Velocity < -JEEP_REVERSE_VELOCITY_MAX)
			jeep.Velocity = -JEEP_REVERSE_VELOCITY_MAX;

		// Store previous 2D position to determine movement delta later.
		auto moved = Vector3i(jeepItem.Pose.Position.x, 0, jeepItem.Pose.Position.z);

		// Process entity collision.
		if (!(jeepItem.Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(jeepItem, JEEP_FRONT);

		int rot1 = 0;
		int rot2 = 0;

		auto pointFrontLeft = GetVehicleCollision(jeepItem, JEEP_FRONT, -JEEP_SIDE, false);
		if (pointFrontLeft.FloorHeight < prevPointFrontLeft.Position.y - CLICK(1))
			rot1 = abs(4 * DoVehicleShift(jeepItem, pointFrontLeft.Position, prevPointFrontLeft.Position));

		auto pointBackLeft = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, false);
		if (pointBackLeft.FloorHeight < prevPointBackLeft.Position.y - CLICK(1))
		{
			if (rotation)
				rot1 += abs(4 * DoVehicleShift(jeepItem, pointBackLeft.Position, prevPointBackLeft.Position));
			else
				rot1 = -abs(4 * DoVehicleShift(jeepItem, pointBackLeft.Position, prevPointBackLeft.Position));
		}

		auto pointFrontRight = GetVehicleCollision(jeepItem, JEEP_FRONT, JEEP_SIDE, false);
		if (pointFrontRight.FloorHeight < pointFrontRight.Position.y - CLICK(1))
			rot2 = -abs(4 * DoVehicleShift(jeepItem, pointFrontRight.Position, prevPointFrontRight.Position));

		auto pointBack = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), 0, false);
		if (pointBack.FloorHeight < prevPointBack.Position.y - CLICK(1))
			DoVehicleShift(jeepItem, pointBack.Position, prevPointBack.Position);
	
		auto pointBackRight = GetVehicleCollision(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, false);
		if (pointBackRight.FloorHeight < prevPointBack.Position.y - CLICK(1))
		{
			if (rot2)
				rot2 -= abs(4 * DoVehicleShift(jeepItem, pointBackRight.Position, prevPointBack.Position));
			else
				rot2 = abs(4 * DoVehicleShift(jeepItem, pointBackRight.Position, prevPointBack.Position));
		}

		if (!rot1)
			rot1 = rot2;
	   
		roomNumber = jeepItem.RoomNumber;
		floor = GetFloor(jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z, &roomNumber);
		if (GetFloorHeight(floor, jeepItem.Pose.Position.x, jeepItem.Pose.Position.y, jeepItem.Pose.Position.z) < jeepItem.Pose.Position.y - STEP_SIZE)
			DoVehicleShift(jeepItem, jeepItem.Pose.Position, prevPos);

		if (!jeep.Velocity)
			rot1 = 0;

		jeep.ExtraRotationDrift = (jeep.ExtraRotationDrift + rot1) / 2;

		if (abs(jeep.ExtraRotationDrift) < 2)
			jeep.ExtraRotationDrift = 0;

		if (abs(jeep.ExtraRotationDrift - jeep.ExtraRotation) < 4)
			jeep.ExtraRotation = jeep.ExtraRotationDrift;
		else
			jeep.ExtraRotation += ((jeep.ExtraRotationDrift - jeep.ExtraRotation) / 4);

		auto impactDirection = GetVehicleImpactDirection(jeepItem, moved);
		if (impactDirection != VehicleImpactDirection::None)
		{
			int newspeed = (jeepItem.Pose.Position.z - prevPos.z) * phd_cos(jeep.MomentumAngle) + (jeepItem.Pose.Position.x - prevPos.x) * phd_sin(jeep.MomentumAngle);
			newspeed *= 256;

			if ((&g_Level.Items[player.Vehicle] == &jeepItem) && (jeep.Velocity == JEEP_VELOCITY_MAX) && (newspeed < (JEEP_VELOCITY_MAX - 10)))
				DoDamage(&laraItem, (JEEP_VELOCITY_MAX - newspeed) / 128);

			if (jeep.Velocity > 0 && newspeed < jeep.Velocity)
				jeep.Velocity = (newspeed < 0) ? 0 : newspeed;
			else if (jeep.Velocity < 0 && newspeed > jeep.Velocity)
				jeep.Velocity = (newspeed > 0) ? 0 : newspeed;

			if (jeep.Velocity < -JEEP_REVERSE_VELOCITY_MAX)
				jeep.Velocity = -JEEP_REVERSE_VELOCITY_MAX;
		}

		return impactDirection;
	}

	void TriggerJeepExhaustSmokeEffect(int x, int y, int z, short angle, short speed, int moving)
	{
		auto* spark = GetFreeParticle();

		spark->dR = 16;
		spark->dG = 16;
		spark->on = 1;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dB = 32;

		if (moving)
		{
			spark->dR = (spark->dR * speed) / 32;
			spark->dG = (spark->dG * speed) / 32;
			spark->dB = (spark->dB * speed) / 32;
		}

		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 4;
		spark->life = spark->sLife = (GetRandomControl() & 3) - (speed / 4096) + 20;;

		if (spark->life < 9)
		{
			spark->life = 9;
			spark->sLife = 9;
		}

		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->x = (GetRandomControl() & 0xF) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = (GetRandomControl() & 0xF) + z - 8;
		spark->xVel = speed * phd_sin(angle) / 4;
		spark->yVel = -8 - (GetRandomControl() & 7);
		spark->zVel = speed * phd_cos(angle) / 4;
		spark->friction = 4;

		if (GetRandomControl() & 1)
		{
			spark->flags = 538;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				spark->rotAdd = -24 - (GetRandomControl() & 7);
			else
				spark->rotAdd = (GetRandomControl() & 7) + 24;
		}
		else
			spark->flags = 522;

		spark->scalar = 1;
		spark->gravity = -4 - (GetRandomControl() & 3);
		spark->maxYvel = -8 - (GetRandomControl() & 7);
		spark->dSize = (GetRandomControl() & 7) + (speed / 128) + 32;
		spark->sSize = spark->dSize / 2;
		spark->size = spark->dSize / 2;
	}
}
