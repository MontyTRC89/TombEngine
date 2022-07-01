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
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using std::vector;

namespace TEN::Entities::Vehicles
{
	const vector<int> JeepJoints = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16 };
	const vector<int> JeepBrakeLightJoints = { 15, 16 };
	const vector<VehicleMountType> JeepMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	constexpr auto JEEP_FRONT = 550;
	constexpr auto JEEP_SIDE = 280;
	constexpr auto JEEP_SLIP = 100;
	constexpr auto JEEP_SLIP_SIDE = 128;
	constexpr auto JEEP_MOUNT_DISTANCE = CLICK(2);
	constexpr auto JEEP_DISMOUNT_DISTANCE = 512;

	constexpr auto JEEP_IMPACT_VELOCITY_MIN = 42 * VEHICLE_VELOCITY_SCALE;
	constexpr auto JEEP_VELOCITY_MAX = 128 * VEHICLE_VELOCITY_SCALE;
	constexpr auto JEEP_REVERSE_VELOCITY_MAX = 64 * VEHICLE_VELOCITY_SCALE;

	constexpr auto JEEP_BOUNCE_MIN = 32;
	constexpr auto JEEP_KICK_MAX = -80;

	#define JEEP_TURN_RATE_DECEL ANGLE(0.5f)

	// TODO: Simpler toggle.
	#define JEEP_IN_TOGGLE_REVERSE	IN_SPRINT
	#define JEEP_IN_TOGGLE_FORWARD	IN_WALK

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
		JEEP_ANIM_IMPACT_FORWARD = 10,
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

	JeepInfo* GetJeepInfo(ItemInfo* jeepItem)
	{
		return (JeepInfo*)jeepItem->Data;
	}

	void InitialiseJeep(short itemNumber)
	{
		auto* jeepItem = &g_Level.Items[itemNumber];
		jeepItem->Data = JeepInfo();
		auto* jeep = GetJeepInfo(jeepItem);

		// TODO: Demagic.
		jeepItem->SetBits(JointBitType::Mesh, JeepJoints);
		jeepItem->ClearBits(JointBitType::Mesh, { 17 });
		jeep->MomentumAngle = jeepItem->Pose.Orientation.y;
	}

	int GetJeepCollisionAnim(ItemInfo* jeepItem, Vector3Int* pos)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		if (jeep->Gear != 0)
			return 0;

		pos->x = jeepItem->Pose.Position.x - pos->x;
		pos->z = jeepItem->Pose.Position.z - pos->z;

		if (pos->x || pos->z)
		{
			float sinY = phd_sin(jeepItem->Pose.Orientation.y);
			float cosY = phd_cos(jeepItem->Pose.Orientation.y);

			int front = (pos->z * cosY) + (pos->x * sinY);
			int side = (pos->z * -sinY) + (pos->x * cosY);

			if (abs(front) > abs(side))
				return ((front > 0) + 13);
			else
				return ((side <= 0) + 11);
		}

		return 0;
	}

	void JeepPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* jeepItem = &g_Level.Items[itemNumber];
		auto* jeep = GetJeepInfo(jeepItem);
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints <= 0 && lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(jeepItem, laraItem, coll, JeepMountTypes, JEEP_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
			ObjectCollision(itemNumber, laraItem, coll);
		else
		{
			lara->Vehicle = itemNumber;
			DoJeepMount(jeepItem, laraItem, mountType);
		}
	}

	int JeepControl(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* jeepItem = &g_Level.Items[lara->Vehicle];
		auto* jeep = GetJeepInfo(jeepItem);

		int drive = -1;
		bool dead = 0;

		int collide = JeepDynamics(jeepItem, laraItem);

		short roomNumber = jeepItem->RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);

		int floorHeight = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);
		int ceiling = GetCeiling(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		Vector3Int fl, fr, bc;
		int hfl = GetVehicleHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, true, &fl);
		int hfr = GetVehicleHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, true, &fr);
		int hbc = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), 0, true, &bc);

		roomNumber = jeepItem->RoomNumber;
		floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		floorHeight = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		TestTriggers(jeepItem, true);
		TestTriggers(jeepItem, false);

		if (laraItem->HitPoints <= 0)
		{
			dead = true;
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		}

		int pitch = 0;
		if (jeep->Flags)
			collide = 0;
		else if (laraItem->Animation.ActiveState == JEEP_STATE_MOUNT)
		{
			drive = -1;
			collide = 0;
		}
		else
			drive = JeepUserControl(jeepItem, laraItem, floorHeight, &pitch);

		if (jeep->Velocity || jeep->Revs)
		{
			jeep->Pitch = pitch;
			if (pitch >= -32768)
			{
				if (pitch > 40960)
					jeep->Pitch = 40960;
			}
			else
				jeep->Pitch = -32768;

			SoundEffect(SFX_TR4_VEHICLE_JEEP_MOVING, &jeepItem->Pose, SoundEnvironment::Land, 0.5f + jeep->Pitch / 65535.0f);
		}
		else
		{
			if (drive != -1)
				SoundEffect(SFX_TR4_VEHICLE_JEEP_IDLE, &jeepItem->Pose);
			jeep->Pitch = 0;
		}

		jeepItem->Floor = floorHeight;
		short rotAdd = jeep->Velocity / 4;
		jeep->FrontRightWheelRotation -= rotAdd;
		jeep->FrontLeftWheelRotation -= rotAdd;
		jeep->BackRightWheelRotation -= rotAdd;
		jeep->BackLeftWheelRotation -= rotAdd;

		int oldY = jeepItem->Pose.Position.y;
		jeepItem->Animation.VerticalVelocity = DoVehicleDynamics(floorHeight, jeepItem->Animation.VerticalVelocity, JEEP_BOUNCE_MIN, JEEP_KICK_MAX, &jeepItem->Pose.Position.y);
		jeep->Velocity = DoVehicleWaterMovement(jeepItem, laraItem, jeep->Velocity, JEEP_FRONT, &jeep->TurnRate);

		short xRot;
		floorHeight = (fl.y + fr.y) / 2;

		if (bc.y < hbc)
		{
			if (floorHeight < (hfl + hfr) / 2)
			{
				xRot = phd_atan(137, oldY - jeepItem->Pose.Position.y);
				if (jeep->Velocity < 0)
					xRot = -xRot;
			}
			else
				xRot = phd_atan(JEEP_FRONT, jeepItem->Pose.Position.y - floorHeight);
		}
		else
		{
			if (floorHeight < (hfl + hfr) / 2)
				xRot = phd_atan(JEEP_FRONT, hbc - jeepItem->Pose.Position.y);
			else
				xRot = phd_atan(1100, hbc - floorHeight);
		}

		short zRot = phd_atan(350, floorHeight - fl.y);
		jeepItem->Pose.Orientation.x += (xRot - jeepItem->Pose.Orientation.x) / 4;
		jeepItem->Pose.Orientation.z += (zRot - jeepItem->Pose.Orientation.z) / 4;

		if (!(jeep->Flags & JEEP_FLAG_DEAD))
		{
			if (roomNumber != jeepItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, roomNumber);
				ItemNewRoom(lara->ItemNumber, roomNumber);
			}

			laraItem->Pose = jeepItem->Pose;

			AnimateJeep(jeepItem, laraItem, collide, dead);
			AnimateItem(laraItem);

			jeepItem->Animation.AnimNumber = Objects[ID_JEEP].animIndex + laraItem->Animation.AnimNumber - Objects[ID_JEEP_LARA_ANIMS].animIndex;
			jeepItem->Animation.FrameNumber = g_Level.Anims[jeepItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			Camera.targetElevation = -ANGLE(30.0f);
			Camera.targetDistance = SECTOR(2);

			if (jeep->Gear == 1)
				jeep->CameraElevation += ((32578 - jeep->CameraElevation) / 8);
			else
				jeep->CameraElevation -= (jeep->CameraElevation / 8);

			Camera.targetAngle = jeep->CameraElevation;

			if (jeep->Flags & JEEP_FLAG_FALLING && jeepItem->Pose.Position.y == jeepItem->Floor)
			{
				laraItem->MeshBits = 0;
				ExplodeVehicle(laraItem, jeepItem);
				return 0;
			}
		}

		if (laraItem->Animation.ActiveState == JEEP_STATE_MOUNT ||
			laraItem->Animation.ActiveState == JEEP_STATE_DISMOUNT)
		{
			jeep->SmokeStart = 0;
		}
		else
		{
			short speed = 0;
			short angle = 0;

			auto pos = Vector3Int(90, 0, -500);
			GetJointAbsPosition(jeepItem, &pos, 11);

			if (jeepItem->Animation.Velocity <= 32)
			{
				if (jeep->SmokeStart >= 16)
				{
					if (GetRandomControl() & 3)
						speed = 0;
					else
						speed = ((GetRandomControl() & 0xF) + GetRandomControl() & 0x10) * 64;
				}
				else
				{
					speed = ((GetRandomControl() & 7) + GetRandomControl() & 0x10 + 2 * jeep->SmokeStart) * 64;
					jeep->SmokeStart++;
				}

				TriggerJeepExhaustSmokeEffect(pos.x, pos.y, pos.z, jeepItem->Pose.Orientation.y + -32768, speed, 0);
			}
			else if (jeepItem->Animation.Velocity < 64)
				TriggerJeepExhaustSmokeEffect(pos.x, pos.y, pos.z, jeepItem->Pose.Orientation.y - 32768, 64 - jeepItem->Animation.Velocity, 1);
		}

		return DoJeepDismount(laraItem);
	}

	int JeepUserControl(ItemInfo* jeepItem, ItemInfo* laraItem, int height, int* pitch)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		if (laraItem->Animation.ActiveState == JEEP_STATE_DISMOUNT || laraItem->Animation.TargetState == JEEP_STATE_DISMOUNT)
			TrInput = 0;

		if (jeep->Revs <= 16)
			jeep->Revs = 0;
		else
		{
			jeep->Velocity += (jeep->Revs / 16);
			jeep->Revs -= (jeep->Revs / 8);
		}

		int rot1 = 0;
		int rot2 = 0;

		if (jeepItem->Pose.Position.y >= height - STEP_SIZE)
		{
			if (!jeep->Velocity)
			{
				if (TrInput & IN_LOOK)
					LookUpDown(laraItem);
			}

			if (abs(jeep->Velocity) <= JEEP_VELOCITY_MAX / 2)
			{
				rot1 = ANGLE(5) * abs(jeep->Velocity) / 16384;
				rot2 = 60 * abs(jeep->Velocity) / 16384 + ANGLE(1);
			}
			else
			{
				rot1 = ANGLE(5);
				rot2 = 242;
			}

			if (jeep->Velocity < 0)
			{
				if (TrInput & VEHICLE_IN_RIGHT)
				{
					jeep->TurnRate -= rot2;
					if (jeep->TurnRate < -rot1)
						jeep->TurnRate = -rot1;
				}
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					jeep->TurnRate += rot2;
					if (jeep->TurnRate > rot1)
						jeep->TurnRate = rot1;
				}
			}
			else if (jeep->Velocity > 0)
			{
				if (TrInput & VEHICLE_IN_RIGHT)
				{
					jeep->TurnRate += rot2;
					if (jeep->TurnRate > rot1)
						jeep->TurnRate = rot1;
				}
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					jeep->TurnRate -= rot2;
					if (jeep->TurnRate < -rot1)
						jeep->TurnRate = -rot1;
				}
			}

			// Brake/reverse
			if (TrInput & VEHICLE_IN_BRAKE)
			{
				if (jeep->Velocity < 0)
				{
					jeep->Velocity += 3 * VEHICLE_VELOCITY_SCALE;
					if (jeep->Velocity > 0)
						jeep->Velocity = 0;
				}
				else if (jeep->Velocity > 0)
				{
					jeep->Velocity -= 3 * VEHICLE_VELOCITY_SCALE;
					if (jeep->Velocity < 0)
						jeep->Velocity = 0;
				}
			}
			// Accelerate
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				if (jeep->Gear)
				{
					if (jeep->Gear == 1 && jeep->Velocity > -JEEP_REVERSE_VELOCITY_MAX)
						jeep->Velocity -= (abs(-JEEP_REVERSE_VELOCITY_MAX - jeep->Velocity) / 8) - 2;
					else if (jeep->Gear == 1 && jeep->Velocity < -JEEP_REVERSE_VELOCITY_MAX)
						jeep->Velocity = -JEEP_REVERSE_VELOCITY_MAX;
				}
				else
				{
					if (jeep->Velocity < JEEP_VELOCITY_MAX)
					{
						if (jeep->Velocity < (64 * VEHICLE_VELOCITY_SCALE))
							jeep->Velocity += 8 + (((64 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - jeep->Velocity) / 8);
						else if (jeep->Velocity < (112 * VEHICLE_VELOCITY_SCALE))
							jeep->Velocity += 4 + (((112 * VEHICLE_VELOCITY_SCALE) + (8 * VEHICLE_VELOCITY_SCALE) - jeep->Velocity) / 16);
						else if (jeep->Velocity < JEEP_VELOCITY_MAX)
							jeep->Velocity += 2 + ((JEEP_VELOCITY_MAX - jeep->Velocity) / 8);
					}
					else
						jeep->Velocity = JEEP_VELOCITY_MAX;
				}

				jeep->Velocity -= (abs(jeepItem->Pose.Orientation.y - jeep->MomentumAngle) / 64);
			}
			else if (jeep->Velocity > (1 * VEHICLE_VELOCITY_SCALE))
				jeep->Velocity -= 1 * VEHICLE_VELOCITY_SCALE;
			else if (jeep->Velocity < -(1 * VEHICLE_VELOCITY_SCALE))
				jeep->Velocity += 1 * VEHICLE_VELOCITY_SCALE;
			else
				jeep->Velocity = 0;

			jeepItem->Animation.Velocity = jeep->Velocity / VEHICLE_VELOCITY_SCALE;
			if (jeep->EngineRevs > 0xC000)
				jeep->EngineRevs = (GetRandomControl() & 0x1FF) + 48896;

			int revs = jeep->Velocity;
			if (jeep->Velocity < 0)
				revs /= 2;

			jeep->EngineRevs += (abs(revs) - jeep->EngineRevs) / 8;
		}

		if (TrInput & VEHICLE_IN_BRAKE)
		{
			auto pos = Vector3Int(0, -144, -1024);
			GetJointAbsPosition(jeepItem, &pos, 11);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
			jeepItem->SetBits(JointBitType::Mesh, { 17 });
			jeepItem->ClearBits(JointBitType::Mesh, JeepBrakeLightJoints);
		}
		else
		{
			jeepItem->SetBits(JointBitType::Mesh, JeepBrakeLightJoints);
			jeepItem->ClearBits(JointBitType::Mesh, { 17 });
		}

		*pitch = jeep->EngineRevs;

		return 1;
	}

	void AnimateJeep(ItemInfo* jeepItem, ItemInfo* laraItem, int collide, int isDead)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		bool dismount;
		if (!isDead &&
			jeepItem->Pose.Position.y != jeepItem->Floor &&
			laraItem->Animation.ActiveState != JEEP_STATE_LEAP &&
			laraItem->Animation.ActiveState != JEEP_STATE_LAND)
		{
			if (jeep->Gear == 1)
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_JUMP_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_JUMP_START;

			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = JEEP_STATE_LEAP;
			laraItem->Animation.TargetState = JEEP_STATE_LEAP;
		}
		else if (collide && !isDead &&
			laraItem->Animation.ActiveState != 4 &&
			laraItem->Animation.ActiveState != 5 &&
			laraItem->Animation.ActiveState != 2 &&
			laraItem->Animation.ActiveState != 3 &&
			laraItem->Animation.ActiveState != JEEP_STATE_LEAP &&
			jeep->Velocity > JEEP_IMPACT_VELOCITY_MIN)
		{
			short state;
			switch (collide)
			{
			case 13:
				state = JEEP_STATE_IMPACT_LEFT;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_LEFT;
				break;

			case 14:
				state = JEEP_STATE_IMPACT_FRONT;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_FORWARD;
				break;

			case 11:
				state = JEEP_STATE_IMPACT_RIGHT;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_RIGHT;
				break;

			case 12:
				state = JEEP_STATE_IMPACT_BACK;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IMPACT_BACK;
				break;
			}

			laraItem->Animation.ActiveState = state;
			laraItem->Animation.TargetState = state;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case JEEP_STATE_IDLE:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_DEATH;
				else
				{
					dismount = ((TrInput & VEHICLE_IN_BRAKE) && (TrInput & VEHICLE_IN_LEFT)) ? true : false;
					if (dismount && !jeep->Velocity && !jeep->DisableDismount)
					{
						if (TestJeepDismount(jeepItem, laraItem))
							laraItem->Animation.TargetState = JEEP_STATE_DISMOUNT;

						break;
					}

					if (DbInput & JEEP_IN_TOGGLE_FORWARD)
					{
						if (jeep->Gear)
							jeep->Gear--;

						break;
					}
					else if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep->Gear < 1)
						{
							jeep->Gear++;
							if (jeep->Gear == 1)
								laraItem->Animation.TargetState = JEEP_STATE_DRIVE_BACK;

							break;
						}
					}
					else
					{
						if ((TrInput & VEHICLE_IN_ACCELERATE) && !(TrInput & VEHICLE_IN_BRAKE))
						{
							laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
							break;
						}
						else if (TrInput & VEHICLE_IN_LEFT)
							laraItem->Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
						else if (TrInput & VEHICLE_IN_RIGHT)
							laraItem->Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
					}

					/*				if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
									{
										if (!(DbInput & JEEP_IN_TOGGLE_REVERSE))
										{
											if ((TrInput & VEHICLE_IN_ACCELERATE) && !(TrInput & VEHICLE_IN_BRAKE))
											{
												laraItem->TargetState = JEEP_STATE_DRIVE_FORWARD;
												break;
											}
											else if (TrInput & VEHICLE_IN_LEFT)
												laraItem->TargetState = JEEP_STATE_FORWARD_LEFT;
											else if (TrInput & VEHICLE_IN_RIGHT)
												laraItem->TargetState = JEEP_STATE_FORWARD_RIGHT;
										}
										else if (jeep->Gear < 1)
										{
											jeep->Gear++;
											if (jeep->Gear == 1)
												laraItem->TargetState = JEEP_STATE_DRIVE_BACK;

										}
									}
									else
									{
										if (jeep->Gear)
											jeep->Gear--;
									}*/
				}

				break;

			case JEEP_STATE_DRIVE_FORWARD:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				else
				{
					if (jeep->Velocity & 0xFFFFFF00 ||
						TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_BRAKE))
					{
						if (TrInput & VEHICLE_IN_BRAKE)
						{
							if (jeep->Velocity <= 21844)
								laraItem->Animation.TargetState = JEEP_STATE_IDLE;
							else
								laraItem->Animation.TargetState = JEEP_STATE_BRAKE;
						}
						else
						{
							if (TrInput & VEHICLE_IN_LEFT)
								laraItem->Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
							else if (TrInput & VEHICLE_IN_RIGHT)
								laraItem->Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
						}
					}
					else
						laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				}

				break;

			case 2:
			case 3:
			case 4:
			case 5:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_BRAKE))
					laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;

				break;

			case JEEP_STATE_BRAKE:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				else if (jeep->Velocity & 0xFFFFFF00)
				{
					if (TrInput & VEHICLE_IN_LEFT)
						laraItem->Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
				}
				else
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;

				break;

			case JEEP_STATE_FORWARD_LEFT:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep->Gear < 1)
						{
							jeep->Gear++;
							if (jeep->Gear == 1)
							{
								laraItem->Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
								laraItem->Animation.ActiveState = JEEP_STATE_REVERSE_RIGHT;
								laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_REVERSE_RIGHT;
								laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
								break;
							}
						}
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem->Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
					else if (jeep->Velocity)
						laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else
						laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				}
				else
				{
					if (jeep->Gear)
						jeep->Gear--;
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_LEFT &&
					!jeep->Velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + JEEP_ANIM_IDLE;
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_START)
				{
					if (jeep->Velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_LEFT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_FORWARD_RIGHT:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;

				if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep->Gear < 1)
						{
							jeep->Gear++;
							if (jeep->Gear == 1)
							{
								laraItem->Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
								laraItem->Animation.ActiveState = JEEP_STATE_REVERSE_LEFT;
								laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_REVERSE_LEFT;
								laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
								break;
							}
						}
					}
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
					else if (jeep->Velocity)
						laraItem->Animation.TargetState = JEEP_STATE_DRIVE_FORWARD;
					else
						laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				}
				else
				{
					if (jeep->Gear)
						jeep->Gear--;
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_RIGHT && !jeep->Velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;//hmm
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_START)
				{
					if (jeep->Velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_FORWARD_RIGHT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_LEAP:
				if (jeepItem->Pose.Position.y == jeepItem->Floor)
					laraItem->Animation.TargetState = JEEP_STATE_LAND;
				else if (jeepItem->Animation.VerticalVelocity > 300)
					jeep->Flags |= JEEP_FLAG_FALLING;

				break;

			case JEEP_STATE_REVERSE:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				else if (abs(jeep->Velocity) & 0xFFFFFF00)
				{
					if (TrInput & VEHICLE_IN_LEFT)
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
				}
				else
					laraItem->Animation.TargetState = JEEP_STATE_DRIVE_BACK;

				break;

			case JEEP_STATE_REVERSE_LEFT:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep->Gear < 1)
							jeep->Gear++;
					}
					else if (TrInput & VEHICLE_IN_RIGHT)
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
					else
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE;
				}
				else
				{
					if (jeep->Gear)
					{
						jeep->Gear--;
						if (!jeep->Gear)
						{
							laraItem->Animation.TargetState = JEEP_STATE_FORWARD_RIGHT;
							laraItem->Animation.ActiveState = JEEP_STATE_FORWARD_RIGHT;
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_FORWARD_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
							break;
						}
					}
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_LEFT && !jeep->Velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_BACK_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_LEFT_BACK_START)
				{
					if (jeep->Velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_LEFT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JEEP_STATE_REVERSE_RIGHT:
				if (isDead)
				{
					laraItem->Animation.TargetState = JEEP_STATE_DRIVE_BACK;
				}
				else if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
				{
					if (DbInput & JEEP_IN_TOGGLE_REVERSE)
					{
						if (jeep->Gear < 1)
							jeep->Gear++;
					}
					else if (TrInput & VEHICLE_IN_LEFT)
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
					else
						laraItem->Animation.TargetState = JEEP_STATE_REVERSE;

					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT && !jeep->Velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
					}

					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START)
					{
						if (jeep->Velocity)
						{
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						}
					}
					break;
				}
				else if (!jeep->Gear || (--jeep->Gear != 0))
				{
					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT && !jeep->Velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
					}

					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_RIGHT_BACK_START)
					{
						if (jeep->Velocity)
						{
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_BACK_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						}
					}

					break;
				}

				laraItem->Animation.TargetState = JEEP_STATE_FORWARD_LEFT;
				laraItem->Animation.ActiveState = JEEP_STATE_FORWARD_LEFT;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE_FORWARD_RIGHT;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				break;

			case JEEP_STATE_DRIVE_BACK:
				if (isDead)
					laraItem->Animation.TargetState = JEEP_STATE_IDLE;
				else // if (jeep->Velocity || jeep->DisableDismount)
				{
					if (!(DbInput & JEEP_IN_TOGGLE_FORWARD))
					{
						if (DbInput & JEEP_IN_TOGGLE_REVERSE)
						{
							if (jeep->Gear < 1)
								jeep->Gear++;
						}
						else if (!(TrInput & VEHICLE_IN_ACCELERATE) || TrInput & VEHICLE_IN_BRAKE)
						{
							if (TrInput & VEHICLE_IN_LEFT)
								laraItem->Animation.TargetState = JEEP_STATE_REVERSE_RIGHT;
							else if (TrInput & VEHICLE_IN_LEFT)
								laraItem->Animation.TargetState = JEEP_STATE_REVERSE_LEFT;
						}
						else
							laraItem->Animation.TargetState = JEEP_STATE_REVERSE;
					}
					else
					{
						if (jeep->Gear)
						{
							jeep->Gear--;
							if (!jeep->Gear)
								laraItem->Animation.TargetState = JEEP_STATE_IDLE;
						}
					}
				}

				break;

			default:
				break;
			}
		}
	}

	void DoJeepMount(ItemInfo* jeepItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* jeep = GetJeepInfo(jeepItem);
		auto* lara = GetLaraInfo(laraItem);

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
			laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_IDLE;
			laraItem->Animation.ActiveState = JEEP_STATE_IDLE;
			laraItem->Animation.TargetState = JEEP_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = JEEP_STATE_MOUNT;
			laraItem->Animation.TargetState = JEEP_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JEEP_ANIM_GETIN_RIGHT;
			laraItem->Animation.ActiveState = JEEP_STATE_MOUNT;
			laraItem->Animation.TargetState = JEEP_STATE_MOUNT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		ResetLaraFlex(laraItem);
		laraItem->Pose.Position = jeepItem->Pose.Position;
		laraItem->Pose.Orientation.y = jeepItem->Pose.Orientation.y;
		lara->Control.HandStatus = HandStatus::Busy;
		lara->HitDirection = -1;
		jeepItem->HitPoints = 1;
		jeepItem->Flags |= TRIGGERED;
		jeep->Revs = 0;
		jeep->Gear = 0;

		AnimateItem(laraItem);
	}

	bool TestJeepDismount(ItemInfo* jeepItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		short angle = jeepItem->Pose.Orientation.y + ANGLE(90.0f);

		int x = jeepItem->Pose.Position.x - JEEP_DISMOUNT_DISTANCE * phd_sin(angle);
		int y = jeepItem->Pose.Position.y;
		int z = jeepItem->Pose.Position.z - JEEP_DISMOUNT_DISTANCE * phd_cos(angle);

		auto probe = GetCollision(x, y, z, jeepItem->RoomNumber);

		if (probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT)
			return false;

		if (abs(probe.Position.Floor - jeepItem->Pose.Position.y) > WALL_SIZE / 2)
			return false;

		if ((probe.Position.Ceiling - jeepItem->Pose.Position.y) > -LARA_HEIGHT ||
			(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	int DoJeepDismount(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->Animation.ActiveState == JEEP_STATE_DISMOUNT)
		{
			if (laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				laraItem->Pose.Orientation.y += ANGLE(90.0f);
				TranslateItem(laraItem, laraItem->Pose.Orientation.y, -JEEP_DISMOUNT_DISTANCE);
				SetAnimation(laraItem, LA_STAND_SOLID);
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				lara->Vehicle = NO_ITEM;
				lara->Control.HandStatus = HandStatus::Free;
				return false;
			}
		}

		return true;
	}

	int JeepDynamics(ItemInfo* jeepItem, ItemInfo* laraItem)
	{
		auto* jeep = GetJeepInfo(jeepItem);
		auto* lara = GetLaraInfo(laraItem);

		Vector3Int f_old, b_old, mm_old, mt_old, mb_old;

		int hf_old  = GetVehicleHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, true, &f_old);
		int hb_old  = GetVehicleHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, true, &b_old);
		int hmm_old = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, true, &mm_old);
		int hmt_old = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, true, &mt_old);
		int hmb_old = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), 0, true, &mb_old);

		auto oldPos = jeepItem->Pose.Position;

		short rot = 0;
		jeep->DisableDismount = false;

		if (oldPos.y <= jeepItem->Floor - 8)
		{
			if (jeep->TurnRate < -JEEP_TURN_RATE_DECEL)
				jeep->TurnRate += JEEP_TURN_RATE_DECEL;
			else if (jeep->TurnRate > JEEP_TURN_RATE_DECEL)
				jeep->TurnRate -= JEEP_TURN_RATE_DECEL;
			else
				jeep->TurnRate = 0;

			jeepItem->Pose.Orientation.y += (short)(jeep->TurnRate + jeep->ExtraRotation);
			jeep->MomentumAngle += ((short)(jeepItem->Pose.Orientation.y - jeep->MomentumAngle) / 32);
		}
		else
		{
			if (jeep->TurnRate < -ANGLE(1))
				jeep->TurnRate += ANGLE(1);
			else if (jeep->TurnRate > ANGLE(1))
				jeep->TurnRate -= ANGLE(1);
			else
				jeep->TurnRate = 0;

			jeepItem->Pose.Orientation.y += jeep->TurnRate + jeep->ExtraRotation;

			rot = jeepItem->Pose.Orientation.y - jeep->MomentumAngle;
			short momentum = (short)(728 - ((3 * jeep->Velocity) / 2048));

			if (!(TrInput & IN_ACTION) && jeep->Velocity > 0)
				momentum -= (momentum / 4);

			if (rot < -273)
			{
				if (rot < -ANGLE(75))
				{
					jeepItem->Pose.Position.y -= 41;
					jeepItem->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
					jeep->TurnRate = 0;
					jeep->Velocity -= (jeep->Velocity / 8);
				}

				if (rot < -ANGLE(90))
					jeep->MomentumAngle = jeepItem->Pose.Orientation.y + ANGLE(90);
				else
					jeep->MomentumAngle -= momentum;
			}
			else if (rot > 273)
			{
				if (rot > ANGLE(75))
				{
					jeepItem->Pose.Position.y -= 41;
					jeepItem->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
					jeep->TurnRate = 0;
					jeep->Velocity -= (jeep->Velocity / 8);
				}

				if (rot > ANGLE(90))
					jeep->MomentumAngle = jeepItem->Pose.Orientation.y - ANGLE(90);
				else
					jeep->MomentumAngle += momentum;
			}
			else
				jeep->MomentumAngle = jeepItem->Pose.Orientation.y;
		}
		
		short roomNumber = jeepItem->RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		int height = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		short speed;
		if (jeepItem->Pose.Position.y < height)
			speed = jeepItem->Animation.Velocity;
		else
			speed = jeepItem->Animation.Velocity * phd_cos(jeepItem->Pose.Orientation.x);

		jeepItem->Pose.Position.x += speed * phd_sin(jeep->MomentumAngle);
		jeepItem->Pose.Position.z += speed * phd_cos(jeep->MomentumAngle);
	
		if (jeepItem->Pose.Position.y >= height)
		{
			short slip = JEEP_SLIP * phd_sin(jeepItem->Pose.Orientation.x);

			if (abs(slip) > 16)
			{
				jeep->DisableDismount = true;

				if (slip < 0)
					jeep->Velocity += SQUARE(slip + 16) / 2;
				else
					jeep->Velocity -= SQUARE(slip - 16) / 2;
			}

			slip = JEEP_SLIP_SIDE * phd_sin(jeepItem->Pose.Orientation.z);

			if (abs(slip) > JEEP_SLIP_SIDE / 4)
			{
				jeep->DisableDismount = true;

				if (slip >= 0)
				{
					jeepItem->Pose.Position.x += (slip - 24) * phd_sin(jeepItem->Pose.Orientation.y + ANGLE(90));
					jeepItem->Pose.Position.z += (slip - 24) * phd_cos(jeepItem->Pose.Orientation.y + ANGLE(90));
				}
				else
				{
					jeepItem->Pose.Position.x += (slip - 24) * phd_sin(jeepItem->Pose.Orientation.y - ANGLE(90));
					jeepItem->Pose.Position.z += (slip - 24) * phd_cos(jeepItem->Pose.Orientation.y - ANGLE(90));
				}
			}
		}

		if (jeep->Velocity > JEEP_VELOCITY_MAX)
			jeep->Velocity = JEEP_VELOCITY_MAX;
		else if (jeep->Velocity < -JEEP_REVERSE_VELOCITY_MAX)
			jeep->Velocity = -JEEP_REVERSE_VELOCITY_MAX;

		Vector3Int movedPos;
		movedPos.x = jeepItem->Pose.Position.x;
		movedPos.z = jeepItem->Pose.Position.z;

		if (!(jeepItem->Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(jeepItem, JEEP_FRONT);

		Vector3Int f, b, mm, mt, mb;
	
		int rot1 = 0;
		int rot2 = 0;

		int hf = GetVehicleHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, false, &f);
		if (hf < f_old.y - STEP_SIZE)
			rot1 = abs(4 * DoVehicleShift(jeepItem, &f, &f_old));

		int hmm = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, false, &mm);
		if (hmm < mm_old.y - STEP_SIZE)
		{
			if (rot)
				rot1 += abs(4 * DoVehicleShift(jeepItem, &mm, &mm_old));
			else
				rot1 = -abs(4 * DoVehicleShift(jeepItem, &mm, &mm_old));
		}

		int hb = GetVehicleHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, false, &b);
		if (hb < b_old.y - STEP_SIZE)
			rot2 = -abs(4 * DoVehicleShift(jeepItem, &b, &b_old));

		int hmb = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), 0, false, &mb);
		if (hmb < mb_old.y - STEP_SIZE)
			DoVehicleShift(jeepItem, &mb, &mb_old);
	
		int hmt = GetVehicleHeight(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, false, &mt);
		if (hmt < mt_old.y - STEP_SIZE)
		{
			if (rot2)
				rot2 -= abs(4 * DoVehicleShift(jeepItem, &mt, &mt_old));
			else
				rot2 = abs(4 * DoVehicleShift(jeepItem, &mt, &mt_old));
		}

		if (!rot1)
			rot1 = rot2;
	   
		roomNumber = jeepItem->RoomNumber;
		floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		if (GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z) < jeepItem->Pose.Position.y - STEP_SIZE)
			DoVehicleShift(jeepItem, (Vector3Int*)&jeepItem->Pose, &oldPos);

		if (!jeep->Velocity)
			rot1 = 0;

		jeep->ExtraRotationDrift = (jeep->ExtraRotationDrift + rot1) / 2;

		if (abs(jeep->ExtraRotationDrift) < 2)
			jeep->ExtraRotationDrift = 0;

		if (abs(jeep->ExtraRotationDrift - jeep->ExtraRotation) < 4)
			jeep->ExtraRotation = jeep->ExtraRotationDrift;
		else
			jeep->ExtraRotation += ((jeep->ExtraRotationDrift - jeep->ExtraRotation) / 4);

		int anim = GetJeepCollisionAnim(jeepItem, &movedPos);
	
		if (anim)
		{
			int newspeed = (jeepItem->Pose.Position.z - oldPos.z) * phd_cos(jeep->MomentumAngle) + (jeepItem->Pose.Position.x - oldPos.x) * phd_sin(jeep->MomentumAngle);
			newspeed *= 256;

			if ((&g_Level.Items[lara->Vehicle] == jeepItem) && (jeep->Velocity == JEEP_VELOCITY_MAX) && (newspeed < (JEEP_VELOCITY_MAX - 10)))
			{
				DoDamage(laraItem, (JEEP_VELOCITY_MAX - newspeed) / 128);
			}

			if (jeep->Velocity > 0 && newspeed < jeep->Velocity)
				jeep->Velocity = (newspeed < 0) ? 0 : newspeed;
			else if (jeep->Velocity < 0 && newspeed > jeep->Velocity)
				jeep->Velocity = (newspeed > 0) ? 0 : newspeed;

			if (jeep->Velocity < -JEEP_REVERSE_VELOCITY_MAX)
				jeep->Velocity = -JEEP_REVERSE_VELOCITY_MAX;
		}

		return anim;
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
