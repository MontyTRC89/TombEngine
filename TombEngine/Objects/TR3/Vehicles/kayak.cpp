#include "framework.h"
#include "Objects/TR3/Vehicles/kayak.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/kayak_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;
using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	struct WAKE_PTS
	{
		int x[2];
		int y;
		int z[2];
		short xvel[2];
		short zvel[2];
		byte life;
		byte pad[3];
	};
	static vector<int> KayakLaraLegJoints = { LM_HIPS, LM_LTHIGH, LM_LSHIN, LM_LFOOT, LM_RTHIGH, LM_RSHIN, LM_RFOOT };
	static vector<VehicleMountType> KayakMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	constexpr auto KAYAK_TO_ENTITY_RADIUS = CLICK(1);
	constexpr auto KAYAK_COLLIDE = CLICK(0.25f);
	constexpr auto KAYAK_MOUNT_DISTANCE = CLICK(1.5f);
	constexpr auto KAYAK_DISMOUNT_DISTANCE = CLICK(3); // TODO: Find accurate distance.

	constexpr int KAYAK_VELOCITY_FORWARD_ACCEL = 24 * VEHICLE_VELOCITY_SCALE;
	constexpr int KAYAK_VELOCITY_LR_ACCEL = 16 * VEHICLE_VELOCITY_SCALE;
	constexpr int KAYAK_VELOCITY_HOLD_TURN_DECEL = 0.5f * VEHICLE_VELOCITY_SCALE;
	constexpr int KAYAK_VELOCITY_FRICTION_DECEL = 0.5f * VEHICLE_VELOCITY_SCALE;

	constexpr int KAYAK_VELOCITY_MAX = 56 * VEHICLE_VELOCITY_SCALE;

	// TODO: Very confusing.
	#define KAYAK_TURN_RATE_FRICTION_DECEL ANGLE(0.03f)
	#define KAYAK_TURN_RATE_DEFLECT ANGLE(0.05f)
	#define KAYAK_TURN_RATE_FORWARD_ACCEL ANGLE(0.7f)
	#define KAYAK_TURN_RATE_LR_ACCEL ANGLE(1.0f)
	#define KAYAK_TURN_RATE_LR_MAX ANGLE(1.0f)
	#define KAYAK_TURN_ROTATION ANGLE(0.18f)
	#define KAYAK_TURN_RATE_MAX ANGLE(1.4f)
	#define KAYAK_TURN_RATE_HOLD_ACCEL ANGLE(1.4f)
	#define KAYAK_TURN_RATE_HOLD_MAX ANGLE(1.4f)

	constexpr auto HIT_BACK = 1;
	constexpr auto HIT_FRONT = 2;
	constexpr auto HIT_LEFT = 3;
	constexpr auto HIT_RIGHT = 4;

	#define KAYAK_MOUNT_LEFT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_RIGHT, 0)
	#define KAYAK_IDLE_FRAME		GetFrameNumber(KAYAK_ANIM_IDLE, 0)
	#define KAYAK_MOUNT_RIGHT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_LEFT, 0)

	constexpr auto KAYAK_DRAW_SHIFT = 32;
	constexpr auto NUM_WAKE_SPRITES = 32;
	constexpr auto WAKE_SIZE = 32;
	constexpr auto WAKE_VELOCITY = 4;
	constexpr auto KAYAK_X = 128;
	constexpr auto KAYAK_Z = 128;
	constexpr auto KAYAK_MAX_KICK = -80;
	constexpr auto KAYAK_MIN_BOUNCE = (KAYAK_VELOCITY_MAX / 2) / VEHICLE_VELOCITY_SCALE;

	// TODO: Kayak control is fairly unique. Keep this? @Sezz 2022.06.25
	constexpr auto KAYAK_IN_FORWARD	   = IN_FORWARD;
	constexpr auto KAYAK_IN_BACK	   = IN_BACK;
	constexpr auto KAYAK_IN_LEFT	   = IN_LEFT;
	constexpr auto KAYAK_IN_RIGHT	   = IN_RIGHT;
	constexpr auto KAYAK_IN_HOLD	   = IN_WALK;
	constexpr auto KAYAK_IN_HOLD_LEFT  = IN_LSTEP;
	constexpr auto KAYAK_IN_HOLD_RIGHT = IN_RSTEP;

	WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];

	enum KayakState
	{
		KAYAK_STATE_BACK = 0,
		KAYAK_STATE_IDLE = 1,
		KAYAK_STATE_TURN_LEFT = 2,
		KAYAK_STATE_TURN_RIGHT = 3,
		KAYAK_STATE_MOUNT_LEFT = 4,
		KAYAK_STATE_IDLE_DEATH = 5,
		KAYAK_STATE_FORWARD = 6,
		KAYAK_STATE_CAPSIZE_RECOVER = 7, // Unused.
		KAYAK_STATE_CAPSIZE_DEATH = 8,	 // Unused.
		KAYAK_STATE_DISMOUNT = 9,
		KAYAK_STATE_HOLD_LEFT = 10,
		KAYAK_STATE_HOLD_RIGHT = 11,
		KAYAK_STATE_MOUNT_RIGHT = 12,
		KAYAK_STATE_DISMOUNT_LEFT = 13,
		KAYAK_STATE_DISMOUNT_RIGHT = 14,
	};

	enum KayakAnim
	{
		KAYAK_ANIM_PADDLE_BACK_END = 0,
		KAYAK_ANIM_PADDLE_BACK_START = 1,
		KAYAK_ANIM_PADDLE_BACK = 2,
		KAYAK_ANIM_MOUNT_RIGHT = 3,
		KAYAK_ANIM_GET_PADDLE = 4,
		KAYAK_ANIM_IDLE_DEATH = 5,
		KAYAK_ANIM_CAPSIZE_DEATH = 6,			// Unused.
		KAYAK_ANIM_PADDLE_FORWARD_END = 7,		// Unused.
		KAYAK_ANIM_PADDLE_FORWARD = 8,			// Unused.
		KAYAK_ANIM_PADDLE_FORWARD_START = 9,	// Unused.
		KAYAK_ANIM_HIT_BACK = 10,
		KAYAK_ANIM_HIT_FRONT = 11,
		KAYAK_ANIM_HIT_RIGHT = 12,
		KAYAK_ANIM_CAPSIZE_LEFT = 13,			// Unused.
		KAYAK_ANIM_DISMOUNT_START = 14,
		KAYAK_ANIM_PADDLE_LEFT = 15,
		KAYAK_ANIM_IDLE = 16,
		KAYAK_ANIM_PADDLE_RIGHT = 17,
		KAYAK_ANIM_CAPSIZE_STRUGGLE = 18,		// Unused.
		KAYAK_ANIM_CAPSIZE_RECOVER_LEFT = 19,	// Unused.
		KAYAK_ANIM_HOLD_PADDLE_LEFT_START = 20,
		KAYAK_ANIM_HOLD_PADDLE_LEFT_END = 21,
		KAYAK_ANIM_HOLD_PADDLE_RIGHT_START = 22,
		KAYAK_ANIM_HOLD_PADDLE_RIGHT_END = 23,
		KAYAK_ANIM_DISMOUNT_LEFT = 24,
		KAYAK_ANIM_OVERBOARD_DEATH = 25,
		KAYAK_ANIM_HOLD_PADDLE_LEFT = 26,
		KAYAK_ANIM_HOLD_PADDLE_RIGHT = 27,
		KAYAK_ANIM_MOUNT_LEFT = 28,
		KAYAK_ANIM_HIT_LEFT = 29,
		KAYAK_ANIM_CAPSIZE_RIGHT = 30,			// Unused.
		KAYAK_ANIM_CAPSIZE_RECOVER_RIGHT = 31,	// Unused.
		KAYAK_ANIM_DISMOUNT_RIGHT = 32
	};

	KayakInfo* GetKayakInfo(ItemInfo* kayakItem)
	{
		return (KayakInfo*)kayakItem->Data;
	}

	void InitialiseKayak(short itemNumber)
	{
		auto* kayakItem = &g_Level.Items[itemNumber];
		kayakItem->Data = KayakInfo();
		auto* kayak = GetKayakInfo(kayakItem);

		kayak->OldPose = kayakItem->Pose;

		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			WakePts[i][0].life = 0;
			WakePts[i][1].life = 0;
		}
	}

	void KayakPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* kayakItem = &g_Level.Items[itemNumber];
		auto* kayak = GetKayakInfo(kayakItem);
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(kayakItem, laraItem, coll, KayakMountTypes, KAYAK_MOUNT_DISTANCE, LARA_HEIGHT);
		if (mountType == VehicleMountType::None)
		{
			coll->Setup.EnableObjectPush = true;
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			lara->Vehicle = itemNumber;
			DoKayakMount(kayakItem, laraItem, mountType);
		}
	}

	void DoKayakMount(ItemInfo* kayakItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* kayak = GetKayakInfo(kayakItem);
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_IDLE;
			laraItem->Animation.ActiveState = KAYAK_STATE_IDLE;
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = KAYAK_STATE_MOUNT_LEFT;
			laraItem->Animation.TargetState = KAYAK_STATE_MOUNT_LEFT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = KAYAK_STATE_MOUNT_RIGHT;
			laraItem->Animation.TargetState = KAYAK_STATE_MOUNT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		if (laraItem->RoomNumber != kayakItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, kayakItem->RoomNumber);

		DoVehicleFlareDiscard(laraItem);
		laraItem->Pose.Position = kayakItem->Pose.Position;
		laraItem->Pose.Orientation = Vector3Shrt(0, kayakItem->Pose.Orientation.y, 0);
		laraItem->Animation.IsAirborne = false;
		laraItem->Animation.Velocity = 0;
		laraItem->Animation.VerticalVelocity = 0;
		lara->Control.WaterStatus = WaterStatus::Dry;
		kayak->WaterHeight = kayakItem->Pose.Position.y;
		kayak->Flags = 0;

		AnimateItem(laraItem);
	}

	void KayakDraw(ItemInfo* kayakItem)
	{
		DrawAnimatingItem(kayakItem);
	}

	void KayakDoWake(ItemInfo* kayakItem, int xOffset, int zOffset, short rotate)
	{
		auto* kayak = GetKayakInfo(kayakItem);

		if (WakePts[kayak->CurrentStartWake][rotate].life)
			return;

		float sinY = phd_sin(kayakItem->Pose.Orientation.y);
		float cosY = phd_cos(kayakItem->Pose.Orientation.y);

		int x = kayakItem->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = kayakItem->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, kayakItem->Pose.Position.y, z, kayakItem->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, kayakItem->Pose.Position.y, z, probedRoomNumber);

		if (waterHeight != NO_HEIGHT)
		{
			short angle1, angle2;
			if (kayakItem->Animation.Velocity < 0)
			{
				if (!rotate)
				{
					angle1 = kayakItem->Pose.Orientation.y - ANGLE(10.0f);
					angle2 = kayakItem->Pose.Orientation.y - ANGLE(30.0f);
				}
				else
				{
					angle1 = kayakItem->Pose.Orientation.y + ANGLE(10.0f);
					angle2 = kayakItem->Pose.Orientation.y + ANGLE(30.0f);
				}
			}
			else
			{
				if (!rotate)
				{
					angle1 = kayakItem->Pose.Orientation.y - ANGLE(170.0f);
					angle2 = kayakItem->Pose.Orientation.y - ANGLE(150.0f);
				}
				else
				{
					angle1 = kayakItem->Pose.Orientation.y + ANGLE(170.0f);
					angle2 = kayakItem->Pose.Orientation.y + ANGLE(150.0f);
				}
			}

			int xv[2], zv[2];
			xv[0] = WAKE_VELOCITY * phd_sin(angle1);
			zv[0] = WAKE_VELOCITY * phd_cos(angle1);
			xv[1] = (WAKE_VELOCITY + 2) * phd_sin(angle2);
			zv[1] = (WAKE_VELOCITY + 2) * phd_cos(angle2);

			WakePts[kayak->CurrentStartWake][rotate].y = kayakItem->Pose.Position.y + KAYAK_DRAW_SHIFT;
			WakePts[kayak->CurrentStartWake][rotate].life = 0x40;

			for (int i = 0; i < 2; i++)
			{
				WakePts[kayak->CurrentStartWake][rotate].x[i] = x;
				WakePts[kayak->CurrentStartWake][rotate].z[i] = z;
				WakePts[kayak->CurrentStartWake][rotate].xvel[i] = xv[i];
				WakePts[kayak->CurrentStartWake][rotate].zvel[i] = zv[i];
			}

			if (rotate == 1)
			{
				kayak->CurrentStartWake++;
				kayak->CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
			}
		}
	}

	void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset)
	{
		float sinY = phd_sin(kayakItem->Pose.Orientation.y);
		float cosY = phd_cos(kayakItem->Pose.Orientation.y);

		int x = kayakItem->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = kayakItem->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, kayakItem->Pose.Position.y, z, kayakItem->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, kayakItem->Pose.Position.y, z, probedRoomNumber);

		//if (waterHeight != NO_HEIGHT)
		//	SetupRipple(x, kayakItem->Pose.Position.y, z, -2 - (GetRandomControl() & 1), 0, Objects[ID_KAYAK_PADDLE_TRAIL_SPRITE].meshIndex,TO_RAD(kayakItem->Pose.Orientation.y));
	}

	void KayakUpdateWakeFX()
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < NUM_WAKE_SPRITES; j++)
			{
				if (WakePts[j][i].life)
				{
					WakePts[j][i].life--;
					WakePts[j][i].x[0] += WakePts[j][i].xvel[0];
					WakePts[j][i].z[0] += WakePts[j][i].zvel[0];
					WakePts[j][i].x[1] += WakePts[j][i].xvel[1];
					WakePts[j][i].z[1] += WakePts[j][i].zvel[1];
				}
			}
		}
	}

	int KayakGetCollisionAnim(ItemInfo* kayakItem, int xDiff, int zDiff)
	{
		xDiff = kayakItem->Pose.Position.x - xDiff;
		zDiff = kayakItem->Pose.Position.z - zDiff;

		if ((xDiff) || (zDiff))
		{
			float s = phd_sin(kayakItem->Pose.Orientation.y);
			float c = phd_cos(kayakItem->Pose.Orientation.y);

			int front = zDiff * c + xDiff * s;
			int side = -zDiff * s + xDiff * c;

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return HIT_BACK;
				else
					return HIT_FRONT;
			}
			else
			{
				if (side > 0)
					return HIT_LEFT;
				else
					return HIT_RIGHT;
			}
		}

		return 0;
	}

	int KayakDoDynamics(int height, int verticalVelocity, int* y)
	{
		if (height > * y)
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
			int kick = (height - *y) * 4;

			if (kick < KAYAK_MAX_KICK)
				kick = KAYAK_MAX_KICK;

			verticalVelocity += (kick - verticalVelocity) / 8;

			if (*y > height)
				*y = height;
		}

		return verticalVelocity;
	}

	void KayakDoCurrent(ItemInfo* kayakItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* room = &g_Level.Rooms[kayakItem->RoomNumber]; // Unused.

		if (!lara->WaterCurrentActive)
		{
			int absVelocity = abs(lara->WaterCurrentPull.x);
			int shift;

			if (absVelocity > 16)
				shift = 4;
			else if (absVelocity > 8)
				shift = 3;
			else
				shift = 2;

			lara->WaterCurrentPull.x -= lara->WaterCurrentPull.x >> shift;

			if (abs(lara->WaterCurrentPull.x) < 4)
				lara->WaterCurrentPull.x = 0;

			absVelocity = abs(lara->WaterCurrentPull.z);
			if (absVelocity > 16)
				shift = 4;
			else if (absVelocity > 8)
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
			int sinkval = lara->WaterCurrentActive - 1;
		
			auto target = Vector3Int(g_Level.Sinks[sinkval].x, g_Level.Sinks[sinkval].y, g_Level.Sinks[sinkval].z);
			int angle = (((mGetAngle(target.x, target.z, laraItem->Pose.Position.x, laraItem->Pose.Position.z) - ANGLE(90.0f))) / 16) & 4095;

			int dx = target.x - laraItem->Pose.Position.x;
			int dz = target.z - laraItem->Pose.Position.z;

			int velocity = g_Level.Sinks[sinkval].strength;
			dx = phd_sin(angle * 16) * velocity * 1024;
			dz = phd_cos(angle * 16) * velocity * 1024;

			lara->WaterCurrentPull.x += (dx - lara->WaterCurrentPull.x) / 16;
			lara->WaterCurrentPull.z += (dz - lara->WaterCurrentPull.z) / 16;
		}

		kayakItem->Pose.Position.x += lara->WaterCurrentPull.x / 256;
		kayakItem->Pose.Position.z += lara->WaterCurrentPull.z / 256;

		lara->WaterCurrentActive = 0;
	}

	bool KayakCanGetOut(ItemInfo* kayakItem, int direction)
	{
		Vector3Int pos;
		int height = GetVehicleWaterHeight(kayakItem, 0, (direction < 0) ? -KAYAK_DISMOUNT_DISTANCE : KAYAK_DISMOUNT_DISTANCE, false, &pos);

		if ((kayakItem->Pose.Position.y - height) > 0)
			return false;

		return true;
	}

	int KayakDoShift(ItemInfo* kayakItem, Vector3Int* pos, Vector3Int* old)
	{
		int x = pos->x / SECTOR(1);
		int z = pos->z / SECTOR(1);

		int xOld = old->x / SECTOR(1);
		int zOld = old->z / SECTOR(1);

		int xShift = pos->x & (SECTOR(1) - 1);
		int zShift = pos->z & (SECTOR(1) - 1);

		if (x == xOld)
		{
			old->x = 0;

			if (z == zOld)
			{
				kayakItem->Pose.Position.z += old->z - pos->z;
				kayakItem->Pose.Position.x += old->x - pos->x;
			}
			else if (z > zOld)
			{
				kayakItem->Pose.Position.z -= zShift + 1;
				return (pos->x - kayakItem->Pose.Position.x);
			}
			else
			{
				kayakItem->Pose.Position.z += SECTOR(1) - zShift;
				return (kayakItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == zOld)
		{
			old->z = 0;

			if (x > xOld)
			{
				kayakItem->Pose.Position.x -= xShift + 1;
				return (kayakItem->Pose.Position.z - pos->z);
			}

			else
			{
				kayakItem->Pose.Position.x += SECTOR(1) - xShift;
				return (pos->z - kayakItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			auto probe = GetCollision(old->x, pos->y, pos->z, kayakItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -zShift - 1;
				else
					z = SECTOR(1) - zShift;
			}

			probe = GetCollision(pos->x, pos->y, old->z, kayakItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -xShift - 1;
				else
					x = SECTOR(1) - xShift;
			}

			if (x && z)
			{
				kayakItem->Pose.Position.x += x;
				kayakItem->Pose.Position.z += z;
			}
			else if (x)
			{
				kayakItem->Pose.Position.x += x;

				if (x > 0)
					return (pos->z - kayakItem->Pose.Position.z);
				else
					return (kayakItem->Pose.Position.z - pos->z);
			}
			else if (z)
			{
				kayakItem->Pose.Position.z += z;

				if (z > 0)
					return (kayakItem->Pose.Position.x - pos->x);
				else
					return (pos->x - kayakItem->Pose.Position.x);
			}
			else
			{
				kayakItem->Pose.Position.x += (old->x - pos->x);
				kayakItem->Pose.Position.z += (old->z - pos->z);
			}
		}

		return 0;
	}

	void KayakToBackground(ItemInfo* kayakItem, ItemInfo* laraItem)
	{
		auto* kayak = GetKayakInfo(kayakItem);

		kayak->OldPose = kayakItem->Pose;

		Vector3Int oldPos[9];
		int height[8];
		height[0] = GetVehicleWaterHeight(kayakItem, 1024, 0, true, &oldPos[0]);
		height[1] = GetVehicleWaterHeight(kayakItem, 512, -96, true, &oldPos[1]);
		height[2] = GetVehicleWaterHeight(kayakItem, 512, 96, true, &oldPos[2]);
		height[3] = GetVehicleWaterHeight(kayakItem, 128, -128, true, &oldPos[3]);
		height[4] = GetVehicleWaterHeight(kayakItem, 128, 128, true, &oldPos[4]);
		height[5] = GetVehicleWaterHeight(kayakItem, -320, -128, true, &oldPos[5]);
		height[6] = GetVehicleWaterHeight(kayakItem, -320, 128, true, &oldPos[6]);
		height[7] = GetVehicleWaterHeight(kayakItem, -640, 0, true, &oldPos[7]);

		oldPos[8].x = kayakItem->Pose.Position.x;
		oldPos[8].y = kayakItem->Pose.Position.y;
		oldPos[8].z = kayakItem->Pose.Position.z;
 
		Vector3Int frontPos, leftPos, rightPos;
		int frontHeight = GetVehicleWaterHeight(kayakItem, 1024, 0, false, &frontPos);
		int leftHeight  = GetVehicleWaterHeight(kayakItem, KAYAK_Z, -KAYAK_X,  false, &leftPos);
		int rightHeight = GetVehicleWaterHeight(kayakItem, KAYAK_Z, KAYAK_X, false, &rightPos);

		kayakItem->Pose.Position.x += kayakItem->Animation.Velocity * phd_sin(kayakItem->Pose.Orientation.y);
		kayakItem->Pose.Position.z += kayakItem->Animation.Velocity * phd_cos(kayakItem->Pose.Orientation.y);
		kayakItem->Pose.Orientation.y += kayak->TurnRate;

		KayakDoCurrent(kayakItem, laraItem);

		kayak->LeftVerticalVelocity = KayakDoDynamics(leftHeight, kayak->LeftVerticalVelocity, &leftPos.y);
		kayak->RightVerticalVelocity = KayakDoDynamics(rightHeight, kayak->RightVerticalVelocity, &rightPos.y);
		kayak->FrontVerticalVelocity = KayakDoDynamics(frontHeight, kayak->FrontVerticalVelocity, &frontPos.y);

		kayakItem->Animation.VerticalVelocity = KayakDoDynamics(kayak->WaterHeight, kayakItem->Animation.VerticalVelocity, &kayakItem->Pose.Position.y);

		int height2 = (leftPos.y + rightPos.y) / 2;
		int x = phd_atan(1024, kayakItem->Pose.Position.y - frontPos.y);
		int z = phd_atan(KAYAK_X, height2 - leftPos.y);

		kayakItem->Pose.Orientation.x = x;
		kayakItem->Pose.Orientation.z = z;

		int xOld = kayakItem->Pose.Position.x;
		int zOld = kayakItem->Pose.Position.z;

		int rot = 0;
		Vector3Int pos;

		if ((height2 = GetVehicleWaterHeight(kayakItem, -CLICK(2.5f), 0, false, &pos)) < (oldPos[7].y - KAYAK_COLLIDE))
			rot = KayakDoShift(kayakItem, &pos, &oldPos[7]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, -CLICK(1.25f), CLICK(0.5f), false, &pos)) < (oldPos[6].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[6]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, -CLICK(1.25f), -CLICK(0.5f), false, &pos)) < (oldPos[5].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[5]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, CLICK(0.5f), CLICK(0.5f), false, &pos)) < (oldPos[4].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[4]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, CLICK(0.5f), -CLICK(0.5f), false, &pos)) < (oldPos[3].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[3]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, CLICK(2), 96, false, &pos)) < (oldPos[2].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[2]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, CLICK(2), -96, false, &pos)) < (oldPos[1].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[1]);

		if ((height2 = GetVehicleWaterHeight(kayakItem, CLICK(4), 0, false, &pos)) < (oldPos[0].y - KAYAK_COLLIDE))
			rot += KayakDoShift(kayakItem, &pos, &oldPos[0]);

		kayakItem->Pose.Orientation.y += rot;

		auto probe = GetCollision(kayakItem);
		int probedRoomNum = probe.RoomNumber;

		height2 = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probedRoomNum);
		if (height2 == NO_HEIGHT)
			height2 = probe.Position.Floor;

		if (height2 < (kayakItem->Pose.Position.y - KAYAK_COLLIDE))
			KayakDoShift(kayakItem, (Vector3Int*)&kayakItem->Pose, &oldPos[8]);

		probe = GetCollision(kayakItem);
		probedRoomNum = probe.RoomNumber;

		height2 = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probedRoomNum);
		if (height2 == NO_HEIGHT)
			height2 = probe.Position.Floor;

		if (height2 == NO_HEIGHT)
		{
			GameVector kayakPos;
			kayakPos.x = kayak->OldPose.Position.x;
			kayakPos.y = kayak->OldPose.Position.y;
			kayakPos.z = kayak->OldPose.Position.z;
			kayakPos.roomNumber = kayakItem->RoomNumber;

			CameraCollisionBounds(&kayakPos, 256, 0);
			{
				kayakItem->Pose.Position.x = kayakPos.x;
				kayakItem->Pose.Position.y = kayakPos.y;
				kayakItem->Pose.Position.z = kayakPos.z;
				kayakItem->RoomNumber = kayakPos.roomNumber;
			}
		}

		DoVehicleCollision(kayakItem, KAYAK_Z); // FIXME: kayak is thin, what should we do about it?

		int collide = KayakGetCollisionAnim(kayakItem, xOld, zOld);

		int slip = 0; // Remnant?
		if (slip || collide)
		{
			int newVelocity;

			newVelocity = (kayakItem->Pose.Position.z - oldPos[8].z) * phd_cos(kayakItem->Pose.Orientation.y) + (kayakItem->Pose.Position.x - oldPos[8].x) * phd_sin(kayakItem->Pose.Orientation.y);
			newVelocity *= VEHICLE_VELOCITY_SCALE;

			if (slip)
			{
				if (kayak->Velocity <= KAYAK_VELOCITY_MAX)
					kayak->Velocity = newVelocity;
			}
			else
			{
				if (kayak->Velocity > 0 && newVelocity < kayak->Velocity)
					kayak->Velocity = newVelocity;

				else if (kayak->Velocity < 0 && newVelocity > kayak->Velocity)
					kayak->Velocity = newVelocity;
			}

			if (kayak->Velocity < -KAYAK_VELOCITY_MAX)
				kayak->Velocity = -KAYAK_VELOCITY_MAX;
		}
	}

	void KayakUserInput(ItemInfo* kayakItem, ItemInfo* laraItem)
	{
		auto* kayak = GetKayakInfo(kayakItem);
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints <= 0 &&
			laraItem->Animation.ActiveState != KAYAK_STATE_IDLE_DEATH)
		{
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_IDLE_DEATH;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = laraItem->Animation.TargetState = KAYAK_STATE_IDLE_DEATH;
		}

		int frame = laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		switch (laraItem->Animation.ActiveState)
		{
		case KAYAK_STATE_IDLE:
			if (TrInput & VEHICLE_IN_DISMOUNT &&
				!lara->WaterCurrentActive &&
				!lara->WaterCurrentPull.x && !lara->WaterCurrentPull.z)
			{
				if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD) && KayakCanGetOut(kayakItem, -1))
				{
					laraItem->Animation.TargetState = KAYAK_STATE_DISMOUNT;
					laraItem->Animation.RequiredState = KAYAK_STATE_DISMOUNT_LEFT;
				}
				else if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD) && KayakCanGetOut(kayakItem, 1))
				{
					laraItem->Animation.TargetState = KAYAK_STATE_DISMOUNT;
					laraItem->Animation.RequiredState = KAYAK_STATE_DISMOUNT_RIGHT;
				}
			}
			else if (TrInput & KAYAK_IN_FORWARD)
			{
				laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
				kayak->Turn = false;
				kayak->Forward = true;
			}
			else if (TrInput & KAYAK_IN_BACK)
				laraItem->Animation.TargetState = KAYAK_STATE_BACK;
			else if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;

				if (kayak->Velocity)
					kayak->Turn = false;
				else
					kayak->Turn = true;

				kayak->Forward = false;
			}

			else if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;

				if (kayak->Velocity)
					kayak->Turn = false;
				else
					kayak->Turn = true;

				kayak->Forward = false;
			}
			else if ((TrInput & KAYAK_IN_HOLD_LEFT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_LEFT)) &&
				(kayak->Velocity ||
					lara->WaterCurrentPull.x || lara->WaterCurrentPull.z))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_HOLD_LEFT;
			}
			else if ((TrInput & KAYAK_IN_HOLD_RIGHT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_RIGHT)) &&
				(kayak->Velocity ||
					lara->WaterCurrentPull.x || lara->WaterCurrentPull.z))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_HOLD_RIGHT;
			}

			break;
		
		case KAYAK_STATE_TURN_LEFT:
			if (kayak->Forward)
			{
				if (!frame)
					kayak->LeftRightPaddleCount = 0;

				// TODO: Sort out the bitwise operations.
				if (frame == 2 && !(kayak->LeftRightPaddleCount & 0x80))
					kayak->LeftRightPaddleCount++;

				else if (frame > 2)
					kayak->LeftRightPaddleCount &= ~0x80;

				if (TrInput & KAYAK_IN_FORWARD)
				{
					if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD))
					{
						if ((kayak->LeftRightPaddleCount & ~0x80) >= 2)
							laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
					}
					else
						laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
				}
				else
					laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
			}
			else if (!(TrInput & KAYAK_IN_LEFT))
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

			if (frame == 7)
			{
				if (kayak->Forward)
				{
					kayak->TurnRate -= KAYAK_TURN_RATE_FORWARD_ACCEL;
					if (kayak->TurnRate < -KAYAK_TURN_RATE_MAX)
						kayak->TurnRate = -KAYAK_TURN_RATE_MAX;

					kayak->Velocity += KAYAK_VELOCITY_FORWARD_ACCEL;
				}
				else if (kayak->Turn)
				{
					kayak->TurnRate -= KAYAK_TURN_RATE_HOLD_ACCEL;
					if (kayak->TurnRate < -KAYAK_TURN_RATE_HOLD_MAX)
						kayak->TurnRate = -KAYAK_TURN_RATE_HOLD_MAX;
				}
				else
				{
					kayak->TurnRate -= KAYAK_TURN_RATE_LR_ACCEL;
					if (kayak->TurnRate < -KAYAK_TURN_RATE_LR_MAX)
						kayak->TurnRate = -KAYAK_TURN_RATE_LR_MAX;

					kayak->Velocity += KAYAK_VELOCITY_LR_ACCEL;
				}
			}

			if (frame > 6 && frame < 24 && frame & 1)
				KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.25f));

			break;
		
		case KAYAK_STATE_TURN_RIGHT:	
			if (kayak->Forward)
			{
				if (!frame)
					kayak->LeftRightPaddleCount = 0;

				if (frame == 2 && !(kayak->LeftRightPaddleCount & 0x80))
					kayak->LeftRightPaddleCount++;

				else if (frame > 2)
					kayak->LeftRightPaddleCount &= ~0x80;

				if (TrInput & KAYAK_IN_FORWARD)
				{
					if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD))
					{
						if ((kayak->LeftRightPaddleCount & ~0x80) >= 2)
							laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;
					}
					else
						laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;
				}
				else
					laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
			}

			else if (!(TrInput & KAYAK_IN_RIGHT))
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

			if (frame == 7)
			{
				if (kayak->Forward)
				{
					kayak->TurnRate += KAYAK_TURN_RATE_FORWARD_ACCEL;
					if (kayak->TurnRate > KAYAK_TURN_RATE_MAX)
						kayak->TurnRate = KAYAK_TURN_RATE_MAX;

					kayak->Velocity += KAYAK_VELOCITY_FORWARD_ACCEL;
				}
				else if (kayak->Turn)
				{
					kayak->TurnRate += KAYAK_TURN_RATE_HOLD_ACCEL;
					if (kayak->TurnRate > KAYAK_TURN_RATE_HOLD_MAX)
						kayak->TurnRate = KAYAK_TURN_RATE_HOLD_MAX;
				}
				else
				{
					kayak->TurnRate += KAYAK_TURN_RATE_LR_ACCEL;
					if (kayak->TurnRate > KAYAK_TURN_RATE_LR_MAX)
						kayak->TurnRate = KAYAK_TURN_RATE_LR_MAX;

					kayak->Velocity += KAYAK_VELOCITY_LR_ACCEL;
				}
			}

			if (frame > 6 && frame < 24 && frame & 1)
				KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.25f));

			break;
		
		case KAYAK_STATE_BACK:
			if (!(TrInput & KAYAK_IN_BACK))
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

			if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_PADDLE_BACK)
			{
				if (frame == 8)
				{
					kayak->TurnRate += KAYAK_TURN_RATE_FORWARD_ACCEL;
					kayak->Velocity -= KAYAK_VELOCITY_FORWARD_ACCEL;
				}

				if (frame == 31)
				{
					kayak->TurnRate -= KAYAK_TURN_RATE_FORWARD_ACCEL;
					kayak->Velocity -= KAYAK_VELOCITY_FORWARD_ACCEL;
				}

				if (frame < 15 && frame & 1)
					KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.5f));

				else if (frame >= 20 && frame <= 34 && frame & 1)
					KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.5f));
			}

			break;
		
		case KAYAK_STATE_HOLD_LEFT:
			if (!(TrInput & KAYAK_IN_HOLD_LEFT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_LEFT)) ||
				(!kayak->Velocity &&
					!lara->WaterCurrentPull.x &&
					!lara->WaterCurrentPull.z))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
			}
			else if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_LEFT)
			{
				if (kayak->Velocity >= 0)
				{
					kayak->TurnRate -= KAYAK_TURN_ROTATION;
					if (kayak->TurnRate < -KAYAK_TURN_RATE_MAX)
						kayak->TurnRate = -KAYAK_TURN_RATE_MAX;

					kayak->Velocity += -KAYAK_VELOCITY_HOLD_TURN_DECEL;
					if (kayak->Velocity < 0)
						kayak->Velocity = 0;
				}

				if (kayak->Velocity < 0)
				{
					kayak->TurnRate += KAYAK_TURN_ROTATION;

					kayak->Velocity += KAYAK_VELOCITY_HOLD_TURN_DECEL;
					if (kayak->Velocity > 0)
						kayak->Velocity = 0;
				}

				if (!(Wibble & 3))
					KayakDoRipple(kayakItem, -CLICK(1), -CLICK(1));
			}

			break;
		
		case KAYAK_STATE_HOLD_RIGHT:
			if (!(TrInput & KAYAK_IN_HOLD_RIGHT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_RIGHT)) ||
				(!kayak->Velocity &&
					!lara->WaterCurrentPull.x &&
					!lara->WaterCurrentPull.z))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
			}
			else if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_RIGHT)
			{
				if (kayak->Velocity >= 0)
				{
					kayak->TurnRate += KAYAK_TURN_ROTATION;
					if (kayak->TurnRate > KAYAK_TURN_RATE_MAX)
						kayak->TurnRate = KAYAK_TURN_RATE_MAX;

					kayak->Velocity += -KAYAK_VELOCITY_HOLD_TURN_DECEL;
					if (kayak->Velocity < 0)
						kayak->Velocity = 0;
				}

				if (kayak->Velocity < 0)
				{
					kayak->TurnRate -= KAYAK_TURN_ROTATION;

					kayak->Velocity += KAYAK_VELOCITY_HOLD_TURN_DECEL;
					if (kayak->Velocity > 0)
						kayak->Velocity = 0;
				}

				if (!(Wibble & 3))
					KayakDoRipple(kayakItem, CLICK(1), -CLICK(1));
			}

			break;
		
		case KAYAK_STATE_MOUNT_LEFT:
			if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_GET_PADDLE &&
				frame == 24 &&
				!(kayak->Flags & 0x80))
			{
				kayak->Flags |= 0x80;
				lara->MeshPtrs[LM_RHAND] = Objects[ID_KAYAK_LARA_ANIMS].meshIndex + LM_RHAND;
				laraItem->ClearBits(JointBitType::Mesh, KayakLaraLegJoints);
			}

			break;
		
		case KAYAK_STATE_DISMOUNT:
			if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_START &&
				frame == 27 &&
				kayak->Flags & 0x80)
			{
				kayak->Flags &= ~0x80;
				lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
				laraItem->SetBits(JointBitType::Mesh, KayakLaraLegJoints);
			}

			laraItem->Animation.TargetState = laraItem->Animation.RequiredState;
			break;
		
		case KAYAK_STATE_DISMOUNT_LEFT:
			if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_LEFT &&
				frame == 83)
			{
				Vector3Int vec = { 0, 350, 500 };
				GetLaraJointPosition(&vec, LM_HIPS);

				SetAnimation(laraItem, LA_JUMP_FORWARD);
				laraItem->Pose.Position = vec;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y - ANGLE(90.0f);
				laraItem->Pose.Orientation.z = 0;
				laraItem->Animation.Velocity = 40;
				laraItem->Animation.VerticalVelocity = -50;
				laraItem->Animation.IsAirborne = true;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
				kayak->LeftRightPaddleCount = 0;
			}

			break;
		
		case KAYAK_STATE_DISMOUNT_RIGHT:
			if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_RIGHT &&
				frame == 83)
			{
				Vector3Int vec = { 0, 350, 500 };
				GetLaraJointPosition(&vec, LM_HIPS);

				SetAnimation(laraItem, LA_JUMP_FORWARD);
				laraItem->Pose.Position = vec;
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y + ANGLE(90.0f);
				laraItem->Pose.Orientation.z = 0;
				laraItem->Animation.IsAirborne = true;
				laraItem->Animation.Velocity = 40;
				laraItem->Animation.VerticalVelocity = -50;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
				kayak->LeftRightPaddleCount = 0;
			}
		}

		if (kayak->Velocity > 0)
		{
			kayak->Velocity -= KAYAK_VELOCITY_FRICTION_DECEL;
			if (kayak->Velocity < 0)
				kayak->Velocity = 0;
		}
		else if (kayak->Velocity < 0)
		{
			kayak->Velocity += KAYAK_VELOCITY_FRICTION_DECEL;
			if (kayak->Velocity > 0)
				kayak->Velocity = 0;
		}

		if (kayak->Velocity > KAYAK_VELOCITY_MAX)
			kayak->Velocity = KAYAK_VELOCITY_MAX;
		else if (kayak->Velocity < -KAYAK_VELOCITY_MAX)
			kayak->Velocity = -KAYAK_VELOCITY_MAX;

		kayakItem->Animation.Velocity = kayak->Velocity / VEHICLE_VELOCITY_SCALE;
		
		if (kayak->TurnRate >= 0)
		{
			kayak->TurnRate -= KAYAK_TURN_RATE_FRICTION_DECEL;
			if (kayak->TurnRate < 0)
				kayak->TurnRate = 0;
		}
		else if (kayak->TurnRate < 0)
		{
			kayak->TurnRate += KAYAK_TURN_RATE_FRICTION_DECEL;
			if (kayak->TurnRate > 0)
				kayak->TurnRate = 0;
		}
	}

	void KayakToItemCollision(ItemInfo* kayakItem, ItemInfo* laraItem)
	{
		for (auto i : g_Level.Rooms[kayakItem->RoomNumber].neighbors)
		{
			short itemNum = g_Level.Rooms[i].itemNumber;

			while (itemNum != NO_ITEM)
			{
				auto* item = &g_Level.Items[itemNum];
				short nextItem = item->NextItem;

				if (item->Collidable && item->Status != ITEM_INVISIBLE)
				{
					auto* object = &Objects[item->ObjectNumber];

					if (object->collision &&
						(item->ObjectNumber == ID_TEETH_SPIKES ||
							item->ObjectNumber == ID_DARTS &&
							item->Animation.ActiveState != 1))
					{
						int x = kayakItem->Pose.Position.x - item->Pose.Position.x;
						int y = kayakItem->Pose.Position.y - item->Pose.Position.y;
						int z = kayakItem->Pose.Position.z - item->Pose.Position.z;

						if (x > -2048 && x < 2048 &&
							y > -2048 && y < 2048 &&
							z > -2048 && z < 2048)
						{
							if (TestBoundsCollide(item, kayakItem, KAYAK_TO_ENTITY_RADIUS))
							{
								DoLotsOfBlood(laraItem->Pose.Position.x, laraItem->Pose.Position.y - STEP_SIZE, laraItem->Pose.Position.z, kayakItem->Animation.Velocity, kayakItem->Pose.Orientation.y, laraItem->RoomNumber, 3);
								DoDamage(laraItem, 5);
							}
						}
					}
				}

				itemNum = nextItem;
			}
		}
	}

	void KayakLaraRapidsDrown(ItemInfo* laraItem)
	{
		// Already drowning...
		if (laraItem->HitPoints <= 0)
			return;

		auto* lara = GetLaraInfo(laraItem);

		// Prevent engaging in flycheat mode
		if (lara->Control.WaterStatus == WaterStatus::FlyCheat)
			return;

		laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_OVERBOARD_DEATH;
		laraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = 12; // TODO
		laraItem->Animation.TargetState = 12;
		laraItem->Animation.IsAirborne = false;
		laraItem->Animation.Velocity = 0;
		laraItem->Animation.VerticalVelocity = 0;
		laraItem->HitPoints = -1;

		AnimateItem(laraItem);

		lara->Control.HandStatus = HandStatus::Busy;
		lara->Control.Weapon.GunType = LaraWeaponType::None;
		lara->ExtraAnim = 1;
		lara->HitDirection = -1;
	}

	bool KayakControl(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* kayakItem = &g_Level.Items[lara->Vehicle];
		auto* kayak = GetKayakInfo(kayakItem);

		if (TrInput & IN_LOOK)
			LookUpDown(laraItem);

		int ofs = kayakItem->Animation.VerticalVelocity;

		KayakUserInput(kayakItem, laraItem);
		KayakToBackground(kayakItem, laraItem);
		TestTriggers(kayakItem, false);

		auto probe = GetCollision(kayakItem);
		int water = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probe.RoomNumber);
		kayak->WaterHeight = water;

		if (kayak->WaterHeight == NO_HEIGHT)
		{
			water = probe.Position.Floor;
			kayak->WaterHeight = water;
			kayak->TrueWater = false;
		}
		else
		{
			kayak->WaterHeight -= 5;
			kayak->TrueWater = true;
		}

		if ((ofs - kayakItem->Animation.VerticalVelocity) > 128 &&
			kayakItem->Animation.VerticalVelocity == 0 &&
			water != NO_HEIGHT)
		{
			int damage = ofs - kayakItem->Animation.VerticalVelocity;
			if (damage > 160)
				DoDamage(laraItem, (damage - 160) * 8);
		}

		if (lara->Vehicle != NO_ITEM)
		{
			if (kayakItem->RoomNumber != probe.RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose.Position = kayakItem->Pose.Position;
			laraItem->Pose.Orientation.x = kayakItem->Pose.Orientation.x;
			laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y;
			laraItem->Pose.Orientation.z = kayakItem->Pose.Orientation.z / 2;

			AnimateItem(laraItem);

			kayakItem->Animation.AnimNumber = Objects[ID_KAYAK].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
			kayakItem->Animation.FrameNumber = g_Level.Anims[kayakItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			Camera.targetElevation = -ANGLE(30.0f);
			Camera.targetDistance = CLICK(8);
		}

		if (!(Wibble & 15) && kayak->TrueWater)
		{
			KayakDoWake(kayakItem, -CLICK(0.5f), 0, 0);
			KayakDoWake(kayakItem, CLICK(0.5f), 0, 1);
		}

		if (Wibble & 7)
		{
			if (!kayak->TrueWater && kayakItem->Animation.VerticalVelocity < 20)
			{
				Vector3Int dest;
				char cnt = 0;
				short MistZPos[10] = { 900, 750, 600, 450, 300, 150, 0,  -150, -300, -450 };
				short MistXPos[10] = { 32,  96,  170, 220, 300, 400, 400, 300,  200,  64 };

				cnt ^= 1;

				for (int i = cnt; i < 10; i += 2)
				{
					if (GetRandomControl() & 1)
						dest.x = (MistXPos[i] / 2);
					else
						dest.x = -(MistXPos[i] / 2);
					dest.y = 50;
					dest.z = MistZPos[i];
				}
			}
		}

		if (!kayakItem->Animation.Velocity &&
			!lara->WaterCurrentPull.x &&
			!lara->WaterCurrentPull.z)
		{
			if (kayak->WakeShade)
				kayak->WakeShade--;
		}
		else
		{
			if (kayak->WakeShade < 16)
				kayak->WakeShade++;
		}

		KayakUpdateWakeFX();
		KayakToItemCollision(kayakItem, laraItem);

		return (lara->Vehicle != NO_ITEM) ? true : false;
	}
}
