#include "framework.h"
#include "Objects/TR3/Vehicles/minecart.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/minecart_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"

using namespace TEN::Effects::Spark;
using namespace TEN::Input;
using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Vehicles
{
	const vector<VehicleMountType> MinecartMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	constexpr auto MINECART_RADIUS = 100;
	constexpr auto MINECART_ENTITY_RADIUS = CLICK(1);
	constexpr auto MINECART_HEIGHT = CLICK(2);
	constexpr auto MINECART_GRAVITY = SECTOR(1) + 1;
	constexpr auto MINECART_STEP_HEIGHT = CLICK(1);
	constexpr auto MINECART_MOUNT_DISTANCE = CLICK(2);
	constexpr auto MINECART_DISMOUNT_DISTANCE = 330;
	constexpr auto MINECART_NUM_HITS = 25;

	constexpr auto MINECART_VELOCITY_DECEL = 6 * VEHICLE_VELOCITY_SCALE;

	constexpr auto MINECART_VELOCITY_MIN = 10 * VEHICLE_VELOCITY_SCALE;
	constexpr auto MINECART_FRICTION_VELOCITY_MIN = 70 * VEHICLE_VELOCITY_SCALE;
	constexpr auto MINECART_STOP_VELOCITY_MIN = 1 * VEHICLE_VELOCITY_SCALE;
	constexpr auto MINECART_STOP_VELOCITY_MAX = 240 * VEHICLE_VELOCITY_SCALE;
	constexpr auto MINECART_VERTICAL_VELOCITY_MAX = 63 * VEHICLE_VELOCITY_SCALE;
	constexpr auto MINECART_JUMP_VERTICAL_VELOCITY = 252 * VEHICLE_VELOCITY_SCALE;

	constexpr auto MINECART_ANIM_VELOCITY_MIN = 32;
	constexpr auto MINECART_TURN_DEATH_ANIM_VELOCITY = 128;

	constexpr auto MINECART_FORWARD_GRADIENT = -CLICK(0.5f);
	constexpr auto MINECART_BACK_GRADIENT = CLICK(0.5f);

	constexpr auto MINECART_WRENCH_MESH_TOGGLE_FRAME = 20;

	#define MINECART_TERMINAL_ANGLE ANGLE(22.0f)

	#define MINECART_IN_DUCK	IN_CROUCH
	#define MINECART_IN_SWIPE	(IN_ACTION | IN_DRAW)

	int Wheels[4] = { 2, 3, 1, 4 };

	enum MinecartState
	{
		MINECART_STATE_MOUNT,
		MINECART_STATE_DISMOUNT,
		MINECART_STATE_DISMOUNT_LEFT,
		MINECART_STATE_DISMOUNT_RIGHT,
		MINECART_STATE_IDLE,
		MINECART_STATE_DUCK,
		MINECART_STATE_MOVE,
		MINECART_STATE_RIGHT,
		MINECART_STATE_HARD_LEFT,	// Unused.
		MINECART_STATE_LEFT,
		MINECART_STATE_HARD_RIGHT,	// Unused.
		MINECART_STATE_BRAKE,
		MINECART_STATE_FORWARD,
		MINECART_STATE_BACK,
		MINECART_STATE_TURN_DEATH,
		MINECART_STATE_FALL_DEATH,
		MINECART_STATE_WALL_DEATH,
		MINECART_STATE_HIT,
		MINECART_STATE_SWIPE,
		MINECART_STATE_BRAKING
	};

	enum MinecartAnim
	{
		MINECART_ANIM_MOUNT_LEFT = 0,
		MINECART_ANIM_DISMOUNT_LEFT = 1,
		MINECART_ANIM_DUCK_START = 2,
		MINECART_ANIM_DUCK_CONTINUE = 3,
		MINECART_ANIM_DUCK_END = 4,
		MINECART_ANIM_PICK_UP_WRENCH = 5,
		MINECART_ANIM_SWIPE_WRENCH = 6,
		MINECART_ANIM_PUT_DOWN_WRENCH = 7,
		MINECART_ANIM_LEAN_LEFT_START = 8,
		MINECART_ANIM_LEAN_RIGHT_CONTINUE = 9,
		MINECART_ANIM_BRAKE_DISENGAGE = 10,				// Unused?
		MINECART_ANIM_BRAKE_ENGAGE = 11,				// Unused?
		MINECART_ANIM_LEAN_RIGHT_START = 12,
		MINECART_ANIM_LEAN_LEFT_CONTINUE = 13,
		MINECART_ANIM_LEAN_RIGHT_BRAKE_ENGAGE = 14,
		MINECART_ANIM_LEAN_LEFT_BRAKE_ENGAGE = 15,
		MINECART_ANIM_LEAN_RIGHT_HARD_CONTINUE = 16,	// Unused?
		MINECART_ANIM_LEAN_LEFT_HARD_CONTINUE = 17,		// Unused?
		MINECART_ANIM_LEAN_RIGHT_HARD_START = 18,		// Unused?
		MINECART_ANIM_LEAN_RIGHT_HARD_END = 19,			// Unused?
		MINECART_ANIM_LEAN_LEFT_HARD_START = 20,		// Unused?
		MINECART_ANIM_LEAN_LEFT_HARD_END = 21,			// Unused?
		MINECART_ANIM_RIDE_FORWARD = 22,
		MINECART_ANIM_WALL_DEATH = 23,
		MINECART_ANIM_LEAN_FORWARD_START = 24,
		MINECART_ANIM_LEAN_FORWARD_CONTINUE = 25,
		MINECART_ANIM_LEAN_FORWARD_END = 26,
		MINECART_ANIM_LEAN_BACK_START = 27,
		MINECART_ANIM_LEAN_BACK_CONTINUE = 28,
		MINECART_ANIM_LEAN_BACK_END = 29,
		MINECART_ANIM_FALL_DEATH = 30,
		MINECART_ANIM_TURN_DEATH = 31,
		MINECART_ANIM_FALL_OUT_START = 32,
		MINECART_ANIM_FALL_OUT_END = 33,
		MINECART_ANIM_BONK_HEAD = 34,
		MINECART_ANIM_LEAN_FORWARD_DUCK_START = 35,
		MINECART_ANIM_LEAN_FORWARD_DUCK_CONTINUE = 36,
		MINECART_ANIM_LEAN_FORWARD_DUCK_END = 37,
		MINECART_ANIM_LEAN_BACK_DUCK_START = 38,
		MINECART_ANIM_LEAN_BACK_DUCK_CONTINUE = 39,
		MINECART_ANIM_LEAN_BACK_DUCK_END = 40,
		MINECART_ANIM_LEAN_RIGHT_BRAKE_DISENGAGE = 41,	// Unused?
		MINECART_ANIM_LEAN_LEFT_BRAKE_DISENGAGE = 42,	// Unused?
		MINECART_ANIM_LEAN_RIGHT_END = 43,
		MINECART_ANIM_LEAN_LEFT_END = 44,
		MINECART_ANIM_IDLE = 45,
		MINECART_ANIM_MOUNT_RIGHT = 46,
		MINECART_ANIM_DISMOUNT_RIGHT = 47,
		MINECART_ANIM_BRAKE = 48,
	};

	enum MinecartFlags
	{
		MINECART_FLAG_WRENCH_MESH = (1 << 0),
		MINECART_FLAG_TURNING_LEFT = (1 << 1),
		MINECART_FLAG_TURNING_RIGHT = (1 << 2),
		MINECART_FLAG_DISMOUNT_RIGHT = (1 << 3),
		MINECART_FLAG_CONTROL = (1 << 4),
		MINECART_FLAG_STOPPED = (1 << 5),
		MINECART_FLAG_NO_ANIM = (1 << 6),
		MINECART_FLAG_DEAD = (1 << 7)
	};

	MinecartInfo* GetMinecartInfo(ItemInfo* minecartItem)
	{
		return (MinecartInfo*)minecartItem->Data;
	}

	void InitialiseMinecart(short itemNumber)
	{
		auto* minecartItem = &g_Level.Items[itemNumber];
		minecartItem->Data = MinecartInfo();
		auto* minecart = GetMinecartInfo(minecartItem);
	}

	void MinecartPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* minecartItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		// Don't get into minecart if there are two stop blocks in front.
		// This allows creation of minecarts which will get to a point and stop forever.
		auto mountType = GetVehicleMountType(minecartItem, laraItem, coll, MinecartMountTypes, MINECART_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None ||
			GetCollision(minecartItem, minecartItem->Pose.Orientation.y, SECTOR(1)).BottomBlock->Flags.MinecartStop())
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			lara->Vehicle = itemNumber;
			DoMinecartMount(minecartItem, laraItem, mountType);
		}
	}

	void DoMinecartMount(ItemInfo* minecartItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* minecart = GetMinecartInfo(minecartItem);
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_IDLE;
			laraItem->Animation.ActiveState = MINECART_STATE_IDLE;
			laraItem->Animation.TargetState = MINECART_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = MINECART_STATE_MOUNT;
			laraItem->Animation.TargetState = MINECART_STATE_MOUNT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = MINECART_STATE_MOUNT;
			laraItem->Animation.TargetState = MINECART_STATE_MOUNT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		laraItem->Pose = minecartItem->Pose;
		lara->Control.HandStatus = HandStatus::Busy;
		minecart->Velocity = 0;
		minecart->VerticalVelocity = 0;
		minecart->Gradient = 0;
		minecart->Flags = 0;
	}

	static void TriggerWheelSparkles(ItemInfo* minecartItem, bool left)
	{
		for (int i = 0; i < 2; i++)
		{
			auto pos = Vector3Int();
			GetJointAbsPosition(minecartItem, &pos, Wheels[(left ? 0 : 2) + i]);
			TriggerFrictionSpark(&GameVector(pos.x, pos.y, pos.z, minecartItem->RoomNumber), minecartItem->Pose.Orientation, 512, 10);
			
			if (i)
			{
				float mult = GenerateFloat(0.7f, 1.0f);
				byte r = (byte)(mult * 190.0f);
				byte g = (byte)(mult * 100.0f);
				TriggerDynamicLight(pos.x, pos.y, pos.z, 2, r, g, 0);
			}
		}
	}

	static int GetMinecartCollision(ItemInfo* minecartItem, short angle, int distance)
	{
		auto probe = GetCollision(minecartItem, angle, distance, -LARA_HEIGHT);

		if (probe.Position.Floor != NO_HEIGHT)
			probe.Position.Floor -= minecartItem->Pose.Position.y;

		return probe.Position.Floor;
	}

	static bool TestMinecartDismount(ItemInfo* laraItem, int direction)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* minecartItem = &g_Level.Items[lara->Vehicle];

		short angle;
		if (direction < 0)
			angle = minecartItem->Pose.Orientation.y + ANGLE(90.0f);
		else
			angle = minecartItem->Pose.Orientation.y - ANGLE(90.0f);

		auto probe = GetCollision(minecartItem, angle, -MINECART_DISMOUNT_DISTANCE);

		if (probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT)
			return false;

		if (abs(probe.Position.Floor - minecartItem->Pose.Position.y) > CLICK(2))
			return false;

		if ((probe.Position.Ceiling - minecartItem->Pose.Position.y) > -LARA_HEIGHT ||
			(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	static void MinecartToEntityCollision(ItemInfo* minecartItem, ItemInfo* laraItem)
	{
		for (auto i : g_Level.Rooms[minecartItem->RoomNumber].neighbors)
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;

			while (itemNumber != NO_ITEM)
			{
				auto* item = &g_Level.Items[itemNumber];

				if (item->Collidable &&
					item->Status != ITEM_INVISIBLE &&
					item != laraItem && item != minecartItem)
				{
					auto* object = &Objects[item->ObjectNumber];
					if (object->collision &&
						(object->intelligent || item->ObjectNumber == ID_ROLLINGBALL || item->ObjectNumber == ID_ANIMATING2))
					{
						auto direction = minecartItem->Pose.Position - item->Pose.Position;
						if (direction.x > -SECTOR(2) && direction.x < SECTOR(2) &&
							direction.y > -SECTOR(2) && direction.y < SECTOR(2) &&
							direction.z > -SECTOR(2) && direction.z < SECTOR(2))
						{
							if (TestBoundsCollide(item, laraItem, MINECART_ENTITY_RADIUS))
							{
								if (item->ObjectNumber == ID_ANIMATING2)
								{
									if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
										(laraItem->Animation.ActiveState == MINECART_STATE_SWIPE &&
											laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_SWIPE_WRENCH))
									{
										int frame = laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
										if (frame >= 12 && frame <= 22)
										{
											SoundEffect(SFX_TR3_VEHICLE_MINECART_WRENCH, &item->Pose, SoundEnvironment::Always);
											TestTriggers(item, true);
											item->Animation.FrameNumber++;
										}
									}
								}
								else if (item->ObjectNumber == ID_ROLLINGBALL)
								{
									/*code, kill lara and stop both the boulder and the minecart*/
								}
								else
								{
									DoLotsOfBlood(
										item->Pose.Position.x,
										minecartItem->Pose.Position.y - CLICK(1),
										item->Pose.Position.z,
										GetRandomControl() & 3,
										minecartItem->Pose.Orientation.y,
										item->RoomNumber,
										3);

									DoDamage(item, INT_MAX);
								}
							}
						}
					}
				}

				itemNumber = item->NextItem;
			}
		}
	}

	static void MoveCart(ItemInfo* minecartItem, ItemInfo* laraItem)
	{
		auto* minecart = GetMinecartInfo(minecartItem);
		auto* lara = GetLaraInfo(laraItem);

		auto flags = GetCollision(minecartItem).BottomBlock->Flags;

		if (minecart->StopDelay)
			minecart->StopDelay--;

		if ((flags.MinecartStop() && !minecart->StopDelay) &&
			((minecartItem->Pose.Position.x & 0x380) == 512 || (minecartItem->Pose.Position.z & 0x380) == 512))
		{
			if (minecart->Velocity < MINECART_STOP_VELOCITY_MAX)
			{
				minecartItem->Animation.Velocity = 0;
				minecart->Velocity = 0;
				minecart->Flags |= MINECART_FLAG_STOPPED | MINECART_FLAG_CONTROL;
				return;
			}
			else
				minecart->StopDelay = 16;
		}

		if ((flags.MinecartLeft() || flags.MinecartRight()) &&
			!flags.MinecartStop() &&
			!minecart->StopDelay &&
			!(minecart->Flags & (MINECART_FLAG_TURNING_LEFT | MINECART_FLAG_TURNING_RIGHT)))
		{
			short angle;
			unsigned short rotation = (((unsigned short)minecartItem->Pose.Orientation.y) / ANGLE(90.0f)) | (flags.MinecartLeft() * 4);

			switch (rotation)
			{
			case 0:
				minecart->TurnX = (minecartItem->Pose.Position.x + 4096) & ~1023;
				minecart->TurnZ = minecartItem->Pose.Position.z & ~1023;
				break;

			case 1:
				minecart->TurnX = minecartItem->Pose.Position.x & ~1023;
				minecart->TurnZ = (minecartItem->Pose.Position.z - 4096) | 1023;
				break;

			case 2:
				minecart->TurnX = (minecartItem->Pose.Position.x - 4096) | 1023;
				minecart->TurnZ = minecartItem->Pose.Position.z | 1023;
				break;

			case 3:
				minecart->TurnX = minecartItem->Pose.Position.x | 1023;
				minecart->TurnZ = (minecartItem->Pose.Position.z + 4096) & ~1023;
				break;

			case 4:
				minecart->TurnX = (minecartItem->Pose.Position.x - 4096) | 1023;
				minecart->TurnZ = minecartItem->Pose.Position.z & ~1023;
				break;

			case 5:
				minecart->TurnX = minecartItem->Pose.Position.x & ~1023;
				minecart->TurnZ = (minecartItem->Pose.Position.z + 4096) & ~1023;
				break;

			case 6:
				minecart->TurnX = (minecartItem->Pose.Position.x + 4096) & ~1023;
				minecart->TurnZ = minecartItem->Pose.Position.z | 1023;
				break;

			case 7:
				minecart->TurnX = minecartItem->Pose.Position.x | 1023;
				minecart->TurnZ = (minecartItem->Pose.Position.z - 4096) | 1023;
				break;
			}

			angle = mGetAngle(minecartItem->Pose.Position.x, minecartItem->Pose.Position.z, minecart->TurnX, minecart->TurnZ) & 0x3fff;

			if (rotation < 4)
			{
				minecart->TurnRot = minecartItem->Pose.Orientation.y;
				minecart->TurnLen = angle;
			}
			else
			{
				minecart->TurnRot = minecartItem->Pose.Orientation.y;

				if (angle)
					angle = ANGLE(90.0f) - angle;

				minecart->TurnLen = angle;
			}

			minecart->Flags |= flags.MinecartLeft() ? MINECART_FLAG_TURNING_LEFT : MINECART_FLAG_TURNING_RIGHT;
		}

		if (minecart->Velocity < MINECART_VELOCITY_MIN)
			minecart->Velocity = MINECART_VELOCITY_MIN;

		minecart->Velocity -= minecart->Gradient * 4;
		minecartItem->Animation.Velocity = minecart->Velocity / VEHICLE_VELOCITY_SCALE;

		if (minecartItem->Animation.Velocity < MINECART_ANIM_VELOCITY_MIN)
		{
			minecartItem->Animation.Velocity = MINECART_ANIM_VELOCITY_MIN;
			StopSoundEffect(SFX_TR3_VEHICLE_MINECART_TRACK_LOOP);

			if (minecart->VerticalVelocity)
				StopSoundEffect(SFX_TR3_VEHICLE_MINECART_PULLY_LOOP);
			else
				SoundEffect(SFX_TR3_VEHICLE_MINECART_PULLY_LOOP, &minecartItem->Pose, SoundEnvironment::Always);
		}
		else
		{
			StopSoundEffect(SFX_TR3_VEHICLE_MINECART_PULLY_LOOP);

			if (minecart->VerticalVelocity)
				StopSoundEffect(SFX_TR3_VEHICLE_MINECART_TRACK_LOOP);
			else
				SoundEffect(SFX_TR3_VEHICLE_MINECART_TRACK_LOOP, &minecartItem->Pose, SoundEnvironment::Land, 1.0f + ((float)minecartItem->Animation.Velocity / SECTOR(8))); // TODO: check actual sound!
		}

		if (minecart->Flags & (MINECART_FLAG_TURNING_LEFT | MINECART_FLAG_TURNING_RIGHT))
		{
			minecart->TurnLen += minecartItem->Animation.Velocity * 3;
			if (minecart->TurnLen > ANGLE(90.0f))
			{
				if (minecart->Flags & MINECART_FLAG_TURNING_LEFT)
					minecartItem->Pose.Orientation.y = minecart->TurnRot - ANGLE(90.0f);
				else
					minecartItem->Pose.Orientation.y = minecart->TurnRot + ANGLE(90.0f);

				minecart->Flags &= ~(MINECART_FLAG_TURNING_LEFT | MINECART_FLAG_TURNING_RIGHT);
			}
			else
			{
				if (minecart->Flags & MINECART_FLAG_TURNING_LEFT)
					minecartItem->Pose.Orientation.y = minecart->TurnRot - minecart->TurnLen;
				else
					minecartItem->Pose.Orientation.y = minecart->TurnRot + minecart->TurnLen;
			}

			if (minecart->Flags & (MINECART_FLAG_TURNING_LEFT | MINECART_FLAG_TURNING_RIGHT))
			{
				unsigned short quadrant = ((unsigned short)minecartItem->Pose.Orientation.y) / ANGLE(90.0f); // TODO: Use GetQuadrant()?
				unsigned short degree = minecartItem->Pose.Orientation.y & 16383;

				float x, z;
				switch (quadrant)
				{
				case NORTH:
					x = -phd_cos(degree);
					z = phd_sin(degree);
					break;

				case EAST:
					x = phd_sin(degree);
					z = phd_cos(degree);
					break;

				case SOUTH:
					x = phd_cos(degree);
					z = -phd_sin(degree);
					break;

				case WEST:
				default:
					x = -phd_sin(degree);
					z = -phd_cos(degree);
					break;
				}

				if (minecart->Flags & MINECART_FLAG_TURNING_LEFT)
				{
					x = -x;
					z = -z;
				}

				minecartItem->Pose.Position.x = minecart->TurnX + x * 3584;
				minecartItem->Pose.Position.z = minecart->TurnZ + z * 3584;

				if (minecart->Velocity > MINECART_FRICTION_VELOCITY_MIN)
				{
					SoundEffect(SFX_TR3_VEHICLE_MINECART_BRAKE, &minecartItem->Pose, SoundEnvironment::Always);
					TriggerWheelSparkles(minecartItem, (minecart->Flags & MINECART_FLAG_TURNING_RIGHT) != 0);
				}
			}
		}
		else
			TranslateItem(minecartItem, minecartItem->Pose.Orientation.y, minecartItem->Animation.Velocity);

		minecart->FloorHeightMiddle = GetVehicleHeight(minecartItem, 0, 0, true, &Vector3Int());

		if (!minecart->VerticalVelocity)
		{
			minecartItem->Pose.Position.y = minecart->FloorHeightMiddle;
			minecart->FloorHeightFront = GetVehicleHeight(minecartItem, CLICK(1), 0, false, &Vector3Int());
			minecart->Gradient = minecart->FloorHeightMiddle - minecart->FloorHeightFront;
		}
		else
		{
			if (minecartItem->Pose.Position.y > minecart->FloorHeightMiddle)
			{
				if (minecart->VerticalVelocity > 0)
					SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_FRONT_IMPACT, &minecartItem->Pose, SoundEnvironment::Always);

				minecartItem->Pose.Position.y = minecart->FloorHeightMiddle;
				minecart->VerticalVelocity = 0;
			}
			else
			{
				minecart->VerticalVelocity += MINECART_GRAVITY;
				if (minecart->VerticalVelocity > MINECART_VERTICAL_VELOCITY_MAX)
					minecart->VerticalVelocity = MINECART_VERTICAL_VELOCITY_MAX;

				minecartItem->Pose.Position.y += minecart->VerticalVelocity / VEHICLE_VELOCITY_SCALE;
			}
		}

		minecartItem->Pose.Orientation.x = minecart->Gradient * 32;

		if (minecart->Flags & (MINECART_FLAG_TURNING_LEFT | MINECART_FLAG_TURNING_RIGHT))
		{
			short val = minecartItem->Pose.Orientation.y & 16383;
			if (minecart->Flags & MINECART_FLAG_TURNING_RIGHT)
				minecartItem->Pose.Orientation.z = -(val * minecartItem->Animation.Velocity) / 512;
			else
				minecartItem->Pose.Orientation.z = ((ANGLE(90.0f) - val) * minecartItem->Animation.Velocity) / 512;
		}
		else
			minecartItem->Pose.Orientation.z -= minecartItem->Pose.Orientation.z / 8;
	}

	static void DoUserInput(ItemInfo* minecartItem, ItemInfo* laraItem)
	{
		auto* minecart = GetMinecartInfo(minecartItem);
		auto* lara = GetLaraInfo(laraItem);

		int floorHeight;

		switch (laraItem->Animation.ActiveState)
		{
		case MINECART_STATE_MOVE:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & MINECART_IN_DUCK)
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;
			else if (minecart->Velocity <= MINECART_STOP_VELOCITY_MIN || minecart->Flags & MINECART_FLAG_STOPPED)
				laraItem->Animation.TargetState = MINECART_STATE_IDLE;
			else if (minecart->Gradient < MINECART_FORWARD_GRADIENT)
				laraItem->Animation.TargetState = MINECART_STATE_FORWARD;
			else if (minecart->Gradient > MINECART_BACK_GRADIENT)
				laraItem->Animation.TargetState = MINECART_STATE_BACK;
			else if (TrInput & VEHICLE_IN_LEFT)
				laraItem->Animation.TargetState = MINECART_STATE_LEFT;
			else if (TrInput & VEHICLE_IN_RIGHT)
				laraItem->Animation.TargetState = MINECART_STATE_RIGHT;

			break;

		case MINECART_STATE_FORWARD:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & MINECART_IN_DUCK)
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;
			else if (minecart->Gradient > MINECART_FORWARD_GRADIENT)
				laraItem->Animation.TargetState = MINECART_STATE_MOVE;

			break;

		case MINECART_STATE_BACK:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & MINECART_IN_DUCK)
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;
			else if (minecart->Gradient < MINECART_BACK_GRADIENT)
				laraItem->Animation.TargetState = MINECART_STATE_MOVE;

			break;

		case MINECART_STATE_LEFT:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & MINECART_IN_DUCK)
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;

			if (!(TrInput & VEHICLE_IN_LEFT))
				laraItem->Animation.TargetState = MINECART_STATE_MOVE;

			break;

		case MINECART_STATE_RIGHT:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & MINECART_IN_DUCK)
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;

			if (!(TrInput & VEHICLE_IN_RIGHT))
				laraItem->Animation.TargetState = MINECART_STATE_MOVE;

			break;

		case MINECART_STATE_IDLE:
			if (!(minecart->Flags & MINECART_FLAG_CONTROL))
			{
				SoundEffect(SFX_TR3_VEHICLE_MINECART_START, &minecartItem->Pose, SoundEnvironment::Always);
				minecart->Flags |= MINECART_FLAG_CONTROL;
				minecart->StopDelay = 64;
			}

			if (TrInput & VEHICLE_IN_DISMOUNT && minecart->Flags & MINECART_FLAG_STOPPED)
			{
				if (TrInput & VEHICLE_IN_LEFT && TestMinecartDismount(laraItem, -1))
				{
					laraItem->Animation.TargetState = MINECART_STATE_DISMOUNT;
					minecart->Flags &= ~MINECART_FLAG_DISMOUNT_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT && TestMinecartDismount(laraItem, 1))
				{
					laraItem->Animation.TargetState = MINECART_STATE_DISMOUNT;
					minecart->Flags |= MINECART_FLAG_DISMOUNT_RIGHT;
				}
			}

			if (minecart->Velocity >= MINECART_VELOCITY_MIN)
			{
				if (TrInput & MINECART_IN_DUCK)
					laraItem->Animation.TargetState = MINECART_STATE_DUCK;
				else
					laraItem->Animation.TargetState = MINECART_STATE_MOVE;
			}

			break;

		case MINECART_STATE_DUCK:
			if (TrInput & MINECART_IN_SWIPE)
				laraItem->Animation.TargetState = MINECART_STATE_SWIPE;
			else if (TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW))
				laraItem->Animation.TargetState = MINECART_STATE_BRAKE;
			else if (!(TrInput & MINECART_IN_DUCK))
				laraItem->Animation.TargetState = MINECART_STATE_IDLE;

			break;

		case MINECART_STATE_SWIPE:
			laraItem->Animation.TargetState = MINECART_STATE_MOVE;
			break;

		case MINECART_STATE_BRAKING:
			if (TrInput & MINECART_IN_DUCK)
			{
				laraItem->Animation.TargetState = MINECART_STATE_DUCK;
				StopSoundEffect(SFX_TR3_VEHICLE_MINECART_BRAKE);
			}
			else if (!(TrInput & (VEHICLE_IN_BRAKE | VEHICLE_IN_SLOW)) || minecart->Flags & MINECART_FLAG_STOPPED)
			{
				laraItem->Animation.TargetState = MINECART_STATE_MOVE;
				StopSoundEffect(SFX_TR3_VEHICLE_MINECART_BRAKE);
			}
			else
			{
				minecart->Velocity -= MINECART_VELOCITY_DECEL;
				SoundEffect(SFX_TR3_VEHICLE_MINECART_BRAKE, &laraItem->Pose, SoundEnvironment::Always);

				if (minecart->Velocity > MINECART_FRICTION_VELOCITY_MIN)
				{
					TriggerWheelSparkles(minecartItem, false);
					TriggerWheelSparkles(minecartItem, true);
				}
			}

			break;

		case MINECART_STATE_BRAKE:
			laraItem->Animation.TargetState = MINECART_STATE_BRAKING;
			break;

		case MINECART_STATE_DISMOUNT:
			if (laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_PUT_DOWN_WRENCH)
			{
				if (laraItem->Animation.FrameNumber == GetFrameNumber(minecartItem, MINECART_WRENCH_MESH_TOGGLE_FRAME) &&
					minecart->Flags & MINECART_FLAG_WRENCH_MESH)
				{
					lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
					minecart->Flags &= ~MINECART_FLAG_WRENCH_MESH;
				}

				if (minecart->Flags & MINECART_FLAG_DISMOUNT_RIGHT)
					laraItem->Animation.TargetState = MINECART_STATE_DISMOUNT_RIGHT;
				else
					laraItem->Animation.TargetState = MINECART_STATE_DISMOUNT_LEFT;
			}

			break;

		case MINECART_STATE_DISMOUNT_LEFT:
			if (laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_DISMOUNT_LEFT &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				auto pos = Vector3Int(0, 640, 0);
				GetLaraJointPosition(&pos, LM_HIPS);

				laraItem->Pose.Position = pos;
				laraItem->Pose.Orientation = Vector3Shrt(0, minecartItem->Pose.Orientation.y + ANGLE(90.0f), 0);

				SetAnimation(laraItem, LA_STAND_SOLID);
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
			}

			break;

		case MINECART_STATE_DISMOUNT_RIGHT:
			if (laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_DISMOUNT_RIGHT &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				auto pos = Vector3Int(0, 640, 0);
				GetLaraJointPosition(&pos, LM_HIPS);

				laraItem->Pose.Position = pos;
				laraItem->Pose.Orientation = Vector3Shrt(0, minecartItem->Pose.Orientation.y - ANGLE(90.0f), 0);

				SetAnimation(laraItem, LA_STAND_SOLID);
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
			}

			break;

		case MINECART_STATE_MOUNT:
			if (laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_PICK_UP_WRENCH &&
				!minecart->Flags & MINECART_FLAG_WRENCH_MESH)
			{
				if (!(minecart->Flags & MINECART_FLAG_WRENCH_MESH) &&
					laraItem->Animation.FrameNumber == GetFrameNumber(minecartItem, MINECART_WRENCH_MESH_TOGGLE_FRAME))
				{
					lara->MeshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;
					minecart->Flags |= MINECART_FLAG_WRENCH_MESH;
				}
			}

			break;

		case MINECART_STATE_WALL_DEATH:
			Camera.targetElevation = -ANGLE(25.0f);
			Camera.targetDistance = SECTOR(4);

			break;

		case MINECART_STATE_TURN_DEATH:
			Camera.targetElevation = -ANGLE(45.0f);
			Camera.targetDistance = SECTOR(2);

			floorHeight = GetMinecartCollision(minecartItem, minecartItem->Pose.Orientation.y, CLICK(2));
			if (abs(floorHeight) < MINECART_STEP_HEIGHT)
			{
				if ((Wibble & 7) == 0)
					SoundEffect(SFX_TR3_VEHICLE_QUADBIKE_FRONT_IMPACT, &minecartItem->Pose, SoundEnvironment::Always);

				TranslateItem(minecartItem, minecartItem->Pose.Orientation.y, MINECART_TURN_DEATH_ANIM_VELOCITY);
			}
			else
			{
				if (laraItem->Animation.AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_FALL_DEATH)
				{
					minecart->Flags |= MINECART_FLAG_NO_ANIM;
					laraItem->HitPoints = -1;
				}
			}

			break;

		case MINECART_STATE_HIT:
			if (laraItem->HitPoints <= 0 &&
				laraItem->Animation.FrameNumber == GetFrameNumber(minecartItem, 34) + 28)
			{
				laraItem->Animation.FrameNumber = GetFrameNumber(minecartItem, 34) + 28;
				minecartItem->Animation.Velocity = 0;
				minecart->Velocity = 0;
				minecart->Flags = (minecart->Flags & ~MINECART_FLAG_CONTROL) | MINECART_FLAG_NO_ANIM;
			}

			break;
		}

		if (lara->Vehicle != NO_ITEM &&
			!(minecart->Flags & MINECART_FLAG_NO_ANIM))
		{
			AnimateItem(laraItem);

			minecartItem->Animation.AnimNumber = Objects[ID_MINECART].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_MINECART_LARA_ANIMS].animIndex);
			minecartItem->Animation.FrameNumber = g_Level.Anims[minecartItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
		}
		if (laraItem->Animation.ActiveState != MINECART_STATE_TURN_DEATH &&
			laraItem->Animation.ActiveState != MINECART_STATE_WALL_DEATH &&
			laraItem->HitPoints > 0)
		{
			if (minecartItem->Pose.Orientation.z > MINECART_TERMINAL_ANGLE ||
				minecartItem->Pose.Orientation.z < -MINECART_TERMINAL_ANGLE)
			{
				laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_TURN_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = MINECART_STATE_TURN_DEATH;
				laraItem->Animation.TargetState = MINECART_STATE_TURN_DEATH;
				minecartItem->Animation.Velocity = 0;
				minecart->Velocity = 0;
				minecart->Flags = (minecart->Flags & ~MINECART_FLAG_CONTROL) | MINECART_FLAG_STOPPED | MINECART_FLAG_DEAD;
				return;
			}

			floorHeight = GetMinecartCollision(minecartItem, minecartItem->Pose.Orientation.y, CLICK(2));

			CollisionInfo coll = {};
			coll.Setup.Radius = MINECART_RADIUS;
			coll.Setup.Height = MINECART_HEIGHT;
			GetCollisionInfo(&coll, minecartItem);

			if (floorHeight < -CLICK(2) || coll.HitStatic)
			{
				laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_WALL_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = MINECART_STATE_WALL_DEATH;
				laraItem->Animation.TargetState = MINECART_STATE_WALL_DEATH;
				laraItem->HitPoints = -1;
				minecartItem->Animation.Velocity = 0;
				minecart->Velocity = 0;
				minecart->Flags = (minecart->Flags & ~MINECART_FLAG_CONTROL) | (MINECART_FLAG_STOPPED | MINECART_FLAG_DEAD);
				return;
			}

			if (laraItem->Animation.ActiveState != MINECART_STATE_DUCK &&
				laraItem->Animation.ActiveState != MINECART_STATE_HIT)
			{
				coll.Setup.Height = LARA_HEIGHT;
				GetCollisionInfo(&coll, laraItem);

				if (coll.HitStatic)
				{
					laraItem->Animation.AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + MINECART_ANIM_BONK_HEAD;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Animation.ActiveState = MINECART_STATE_HIT;
					laraItem->Animation.TargetState = MINECART_STATE_HIT;

					DoLotsOfBlood(
						laraItem->Pose.Position.x,
						laraItem->Pose.Position.y - CLICK(3),
						laraItem->Pose.Position.z,
						minecartItem->Animation.Velocity,
						minecartItem->Pose.Orientation.y,
						laraItem->RoomNumber,
						3);

					int hits = MINECART_NUM_HITS * short(minecart->Velocity / 2048);
					if (hits < 20)
						hits = 20;

					DoDamage(laraItem, hits);
					return;
				}
			}

			if (floorHeight > CLICK(2.25f) && !minecart->VerticalVelocity)
				minecart->VerticalVelocity = MINECART_JUMP_VERTICAL_VELOCITY;

			MinecartToEntityCollision(minecartItem, laraItem);
		}
	}

	bool MinecartControl(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* minecartItem = &g_Level.Items[lara->Vehicle];

		if (!minecartItem->Data)
		{
			TENLog("Minecart data is nullptr!", LogLevel::Error);
			return false;
		}
		auto* minecart = GetMinecartInfo(minecartItem);

		DoUserInput(minecartItem, laraItem);

		if (minecart->Flags & MINECART_FLAG_CONTROL)
			MoveCart(minecartItem, laraItem);

		if (lara->Vehicle != NO_ITEM)
			laraItem->Pose = minecartItem->Pose;

		short probedRoomNumber = GetCollision(minecartItem).RoomNumber;
		if (probedRoomNumber != minecartItem->RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probedRoomNumber);
			ItemNewRoom(lara->ItemNumber, probedRoomNumber);
		}

		TestTriggers(minecartItem, false);

		if (!(minecart->Flags & MINECART_FLAG_DEAD))
		{
			Camera.targetElevation = -ANGLE(45.0f);
			Camera.targetDistance = SECTOR(2);
		}

		return (lara->Vehicle == NO_ITEM) ? false : true;
	}
}
