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
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Vehicles
{
	#define SPEEDBOAT_UNDO_TURN		ANGLE(0.25f)
	#define SPEEDBOAT_TURN			(ANGLE(0.25f) / 2)
	#define SPEEDBOAT_MAX_TURN		ANGLE(4.0f)
	#define SPEEDBOAT_MAX_VELOCITY	110
	#define SPEEDBOAT_SLOW_SPEED	(SPEEDBOAT_MAX_VELOCITY / 3)
	#define SPEEDBOAT_FAST_SPEED	(SPEEDBOAT_MAX_VELOCITY + 75)
	#define SPEEDBOAT_MIN_SPEED		20
	#define SPEEDBOAT_ACCELERATION	5
	#define SPEEDBOAT_BRAKE			5
	#define SPEEDBOAT_SLOWDOWN		1
	#define SPEEDBOAT_REVERSE		-2	// -5
	#define SPEEDBOAT_MAX_BACK		-20
	#define SPEEDBOAT_MAX_KICK		-80
	#define SPEEDBOAT_SLIP			10
	#define SPEEDBOAT_SIDE_SLIP		30
	#define SPEEDBOAT_FRONT			750
	#define SPEEDBOAT_BACK			-700
	#define SPEEDBOAT_SIDE			300
	#define SPEEDBOAT_RADIUS			500
	#define SPEEDBOAT_SNOW			500
	#define SPEEDBOAT_MAX_HEIGHT		CLICK(1)
	#define SPEEDBOAT_DISMOUNT_DISTANCE	SECTOR(1)
	#define SPEEDBOAT_SOUND_CEILING	SECTOR(5)
	#define SPEEDBOAT_TIP			(SPEEDBOAT_FRONT + 250)

	#define	SPEEDBOAT_IN_ACCELERATE	IN_FORWARD
	#define	SPEEDBOAT_IN_REVERSE	IN_BACK
	#define	SPEEDBOAT_IN_SPEED		(IN_ACTION | IN_SPRINT)
	#define SPEEDBOAT_IN_SLOW		IN_WALK
	#define	SPEEDBOAT_IN_DISMOUNT	(IN_JUMP | IN_ROLL)
	#define	SPEEDBOAT_IN_LEFT		(IN_LEFT | IN_LSTEP)
	#define	SPEEDBOAT_IN_RIGHT		(IN_RIGHT | IN_RSTEP)

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

	SpeedboatMountType GetSpeedboatMountType(ItemInfo* laraItem, ItemInfo* speedboatItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);

		auto mountType = SpeedboatMountType::None;

		if (lara->Control.HandStatus != HandStatus::Free)
			return mountType;

		if (!TestBoundsCollide(speedboatItem, laraItem, coll->Setup.Radius))
			return mountType;

		if (!TestCollision(speedboatItem, laraItem))
			return mountType;

		int distance = (laraItem->Pose.Position.z - speedboatItem->Pose.Position.z) * phd_cos(-speedboatItem->Pose.Orientation.y) - (laraItem->Pose.Position.x - speedboatItem->Pose.Position.x) * phd_sin(-speedboatItem->Pose.Orientation.y);
		if (distance > 200)
			return mountType;

		short deltaAngle = speedboatItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		if (lara->Control.WaterStatus == WaterStatus::TreadWater || lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (!(TrInput & IN_ACTION) || laraItem->Animation.Airborne || speedboatItem->Animation.Velocity)
				return mountType;

			if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
				mountType = SpeedboatMountType::WaterRight;
			else if (deltaAngle > -ANGLE(135.0f) && deltaAngle < -ANGLE(45.0f))
				mountType = SpeedboatMountType::WaterLeft;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Dry)
		{
			if (laraItem->Animation.VerticalVelocity > 0)
			{
				if (deltaAngle > -ANGLE(135.0f) && deltaAngle < ANGLE(135.0f) &&
					laraItem->Pose.Position.y > speedboatItem->Pose.Position.y)
				{
					mountType = SpeedboatMountType::Jump;
				}
			}
			else if (laraItem->Animation.VerticalVelocity == 0)
			{
				if (deltaAngle > -ANGLE(135.0f) && deltaAngle < ANGLE(135.0f))
				{
					if (laraItem->Pose.Position.x == speedboatItem->Pose.Position.x &&
						laraItem->Pose.Position.y == speedboatItem->Pose.Position.y &&
						laraItem->Pose.Position.z == speedboatItem->Pose.Position.z)
					{
						mountType = SpeedboatMountType::StartPosition;
					}
					else
						mountType = SpeedboatMountType::Jump;
				}
			}
		}

		return mountType;
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

	void DoSpeedboatDismount(ItemInfo* laraItem, ItemInfo* speedboatItem)
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
			laraItem->Animation.Velocity = 40;
			laraItem->Animation.VerticalVelocity = -50;
			laraItem->Animation.Airborne = true;
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

	int SpeedboatTestWaterHeight(ItemInfo* speedboatItem, int zOffset, int xOffset, Vector3Int* pos)
	{
		float sinX = phd_sin(speedboatItem->Pose.Orientation.x);
		float sinY = phd_sin(speedboatItem->Pose.Orientation.y);
		float cosY = phd_cos(speedboatItem->Pose.Orientation.y);
		float sinZ = phd_sin(speedboatItem->Pose.Orientation.z);

		pos->x = speedboatItem->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		pos->y = speedboatItem->Pose.Position.y - (zOffset * sinX) + (xOffset * sinZ);
		pos->z = speedboatItem->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		auto probe = GetCollision(pos->x, pos->y, pos->z, speedboatItem->RoomNumber);
		auto height = GetWaterHeight(pos->x, pos->y, pos->z, probe.RoomNumber);

		if (height == NO_HEIGHT)
		{
			height = probe.Position.Floor;
			if (height == NO_HEIGHT)
				return height;
		}

		return (height - 5);
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
			float s = phd_sin(speedboatItem->Pose.Orientation.y);
			float c = phd_cos(speedboatItem->Pose.Orientation.y);

			int front = moved->z * c + moved->x * s;
			int side = -moved->z * s + moved->x * c;

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
			if (verticalVelocity < SPEEDBOAT_MAX_BACK)
				verticalVelocity = SPEEDBOAT_MAX_BACK;

			if (*y > height)
				*y = height;
		}

		return verticalVelocity;
	}

	int SpeedboatDynamics(ItemInfo* laraItem, short itemNumber)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* speedboatItem = &g_Level.Items[itemNumber];
		auto* speedboat = GetSpeedboatInfo(speedboatItem);

		speedboatItem->Pose.Orientation.z -= speedboat->LeanAngle;

		Vector3Int old, frontLeftOld, frontRightOld, backLeftOld, backRightOld, frontOld;
		int heightFrontLeftOld = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, &frontLeftOld);
		int heightFrontRightOld = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, &frontRightOld);
		int heightBackLeftOld = SpeedboatTestWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, &backLeftOld);
		int heightBackRightOld = SpeedboatTestWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, &backRightOld);
		int heightFrontOld = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_TIP, 0, &frontOld);

		old.x = speedboatItem->Pose.Position.x;
		old.y = speedboatItem->Pose.Position.y;
		old.z = speedboatItem->Pose.Position.z;

		if (backLeftOld.y > heightBackLeftOld)
			backLeftOld.y = heightBackLeftOld;
		if (backRightOld.y > heightBackRightOld)
			backRightOld.y = heightBackRightOld;
		if (frontLeftOld.y > heightFrontLeftOld)
			frontLeftOld.y = heightFrontLeftOld;
		if (frontRightOld.y > heightFrontRightOld)
			frontRightOld.y = heightFrontRightOld;
		if (frontOld.y > heightFrontOld)
			frontOld.y = heightFrontOld;

		speedboatItem->Pose.Orientation.y += speedboat->TurnRate + speedboat->ExtraRotation;
		speedboat->LeanAngle = speedboat->TurnRate * 6;

		speedboatItem->Pose.Position.x += speedboatItem->Animation.Velocity * phd_sin(speedboatItem->Pose.Orientation.y);
		speedboatItem->Pose.Position.z += speedboatItem->Animation.Velocity * phd_cos(speedboatItem->Pose.Orientation.y);

		int slip = SPEEDBOAT_SIDE_SLIP * phd_sin(speedboatItem->Pose.Orientation.z);
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
		auto heightBackLeft = SpeedboatTestWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, &bl);
		if (heightBackLeft < (backLeftOld.y - CLICK(0.5f)))
			rotation = SpeedboatDoShift(speedboatItem, &bl, &backLeftOld);

		auto heightBackRight = SpeedboatTestWaterHeight(speedboatItem, -SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, &br);
		if (heightBackRight < (backRightOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &br, &backRightOld);

		auto heightFrontLeft = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, &fl);
		if (heightFrontLeft < (frontLeftOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &fl, &frontLeftOld);

		auto heightFrontRight = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, &fr);
		if (heightFrontRight < (frontRightOld.y - CLICK(0.5f)))
			rotation += SpeedboatDoShift(speedboatItem, &fr, &frontRightOld);

		int heightFront = 0;
		if (!slip)
		{
			heightFront = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_TIP, 0, &f);
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

			if (lara->Vehicle == itemNumber && speedboatItem->Animation.Velocity > SPEEDBOAT_MAX_VELOCITY + SPEEDBOAT_ACCELERATION && newVelocity < speedboatItem->Animation.Velocity - 10)
			{
				laraItem->HitPoints -= speedboatItem->Animation.Velocity;
				laraItem->HitStatus = true;
				SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Pose);
				newVelocity /= 2;
				speedboatItem->Animation.Velocity /= 2;
			}

			if (slip)
			{
				if (speedboatItem->Animation.Velocity <= SPEEDBOAT_MAX_VELOCITY + 10)
					speedboatItem->Animation.Velocity = newVelocity;
			}
			else
			{
				if (speedboatItem->Animation.Velocity > 0 && newVelocity < speedboatItem->Animation.Velocity)
					speedboatItem->Animation.Velocity = newVelocity;
				else if (speedboatItem->Animation.Velocity < 0 && newVelocity > speedboatItem->Animation.Velocity)
					speedboatItem->Animation.Velocity = newVelocity;
			}

			if (speedboatItem->Animation.Velocity < SPEEDBOAT_MAX_BACK)
				speedboatItem->Animation.Velocity = SPEEDBOAT_MAX_BACK;
		}

		return collide;
	}

	bool SpeedboatUserControl(ItemInfo* laraItem, ItemInfo* speedboatItem)
	{
		auto* speedboat = GetSpeedboatInfo(speedboatItem);

		bool noTurn = true;
		int maxVelocity;

		if (speedboatItem->Pose.Position.y >= speedboat->Water - CLICK(0.5f) && speedboat->Water != NO_HEIGHT)
		{
			if (!(TrInput & SPEEDBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) ||
				speedboatItem->Animation.Velocity)
			{
				if (TrInput & SPEEDBOAT_IN_LEFT && !(TrInput & SPEEDBOAT_IN_REVERSE) ||
					TrInput & SPEEDBOAT_IN_RIGHT && TrInput & SPEEDBOAT_IN_REVERSE)
				{
					if (speedboat->TurnRate > 0)
						speedboat->TurnRate -= SPEEDBOAT_UNDO_TURN;
					else
					{
						speedboat->TurnRate -= SPEEDBOAT_TURN;
						if (speedboat->TurnRate < -SPEEDBOAT_MAX_TURN)
							speedboat->TurnRate = -SPEEDBOAT_MAX_TURN;
					}

					noTurn = false;
				}
				else if (TrInput & SPEEDBOAT_IN_RIGHT && !(TrInput & SPEEDBOAT_IN_REVERSE) ||
					TrInput & SPEEDBOAT_IN_LEFT && TrInput & SPEEDBOAT_IN_REVERSE)
				{
					if (speedboat->TurnRate < 0)
						speedboat->TurnRate += SPEEDBOAT_UNDO_TURN;
					else
					{
						speedboat->TurnRate += SPEEDBOAT_TURN;
						if (speedboat->TurnRate > SPEEDBOAT_MAX_TURN)
							speedboat->TurnRate = SPEEDBOAT_MAX_TURN;
					}

					noTurn = false;
				}

				if (TrInput & SPEEDBOAT_IN_REVERSE)
				{
					if (speedboatItem->Animation.Velocity > 0)
						speedboatItem->Animation.Velocity -= SPEEDBOAT_BRAKE;
					else if (speedboatItem->Animation.Velocity > SPEEDBOAT_MAX_BACK)
						speedboatItem->Animation.Velocity += SPEEDBOAT_REVERSE;
				}
				else if (TrInput & SPEEDBOAT_IN_ACCELERATE)
				{
					if (TrInput & SPEEDBOAT_IN_SPEED)
						maxVelocity = SPEEDBOAT_FAST_SPEED;
					else
						maxVelocity = (TrInput & SPEEDBOAT_IN_SLOW) ? SPEEDBOAT_SLOW_SPEED : SPEEDBOAT_MAX_VELOCITY;

					if (speedboatItem->Animation.Velocity < maxVelocity)
						speedboatItem->Animation.Velocity += (SPEEDBOAT_ACCELERATION / 2) + (SPEEDBOAT_ACCELERATION * (speedboatItem->Animation.Velocity / (maxVelocity * 2)));
					else if (speedboatItem->Animation.Velocity > maxVelocity + SPEEDBOAT_SLOWDOWN)
						speedboatItem->Animation.Velocity -= SPEEDBOAT_SLOWDOWN;
				}
				else if (TrInput & (SPEEDBOAT_IN_LEFT | SPEEDBOAT_IN_RIGHT) &&
					speedboatItem->Animation.Velocity >= 0 &&
					speedboatItem->Animation.Velocity < SPEEDBOAT_MIN_SPEED)
				{
					if (!(TrInput & SPEEDBOAT_IN_DISMOUNT) &&
						speedboatItem->Animation.Velocity == 0)
						speedboatItem->Animation.Velocity = SPEEDBOAT_MIN_SPEED;
				}
				else if (speedboatItem->Animation.Velocity > SPEEDBOAT_SLOWDOWN)
					speedboatItem->Animation.Velocity -= SPEEDBOAT_SLOWDOWN;
				else
					speedboatItem->Animation.Velocity = 0;
			}
			else
			{
				if (TrInput & (SPEEDBOAT_IN_LEFT | SPEEDBOAT_IN_RIGHT) &&
					speedboatItem->Animation.Velocity >= 0 &&
					speedboatItem->Animation.Velocity < SPEEDBOAT_MIN_SPEED)
				{
					if (speedboatItem->Animation.Velocity == 0 && !(TrInput & SPEEDBOAT_IN_DISMOUNT))
						speedboatItem->Animation.Velocity = SPEEDBOAT_MIN_SPEED;
				}
				else if (speedboatItem->Animation.Velocity > SPEEDBOAT_SLOWDOWN)
					speedboatItem->Animation.Velocity -= SPEEDBOAT_SLOWDOWN;
				else
					speedboatItem->Animation.Velocity = 0;

				if (TrInput & IN_LOOK && speedboatItem->Animation.Velocity == 0)
					LookUpDown(laraItem);
			}
		}

		return noTurn;
	}

	void SpeedboatAnimation(ItemInfo* laraItem, ItemInfo* speedboatItem, int collide)
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
				if (TrInput & SPEEDBOAT_IN_DISMOUNT)
				{
					if (speedboatItem->Animation.Velocity == 0)
					{
						if (TrInput & SPEEDBOAT_IN_RIGHT && TestSpeedboatDismount(speedboatItem, speedboatItem->Pose.Orientation.y + ANGLE(90.0f)))
							laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_RIGHT;
						else if (TrInput & SPEEDBOAT_IN_LEFT && TestSpeedboatDismount(speedboatItem, speedboatItem->Pose.Orientation.y - ANGLE(90.0f)))
							laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_LEFT;
					}
				}

				if (speedboatItem->Animation.Velocity > 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;

			case SPEEDBOAT_STATE_MOVING:
				if (TrInput & SPEEDBOAT_IN_DISMOUNT)
				{
					if (TrInput & SPEEDBOAT_IN_RIGHT)
						laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_RIGHT;
					else if (TrInput & SPEEDBOAT_IN_RIGHT)
						laraItem->Animation.TargetState = SPEEDBOAT_STATE_DISMOUNT_LEFT;
				}
				else if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;

				break;

			case SPEEDBOAT_STATE_FALL:
				laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;
				break;

				//case SPEEDBOAT_TURNR:
				if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;
				else if (!(TrInput & SPEEDBOAT_IN_RIGHT))
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;

			case SPEEDBOAT_STATE_TURN_LEFT:
				if (speedboatItem->Animation.Velocity <= 0)
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_IDLE;
				else if (!(TrInput & SPEEDBOAT_IN_LEFT))
					laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOVING;

				break;
			}
		}
	}

	void SpeedboatSplash(ItemInfo* item, long verticalVelocity, long water)
	{
		//OLD SPLASH
		/*
		splash_setup.x = item->pos.x_pos;
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

	void SpeedboatCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto* speedboatItem = &g_Level.Items[itemNumber];

		switch (GetSpeedboatMountType(laraItem, speedboatItem, coll))
		{
		case SpeedboatMountType::None:
			coll->Setup.EnableObjectPush = true;
			ObjectCollision(itemNumber, laraItem, coll);
			return;

		case SpeedboatMountType::WaterLeft:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_LEFT;
			break;

		case SpeedboatMountType::WaterRight:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_RIGHT;
			break;

		case SpeedboatMountType::Jump:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_MOUNT_JUMP;
			break;

		case SpeedboatMountType::StartPosition:
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SPEEDBOAT_ANIM_IDLE;
			break;
		}

		laraItem->Pose.Position.x = speedboatItem->Pose.Position.x;
		laraItem->Pose.Position.y = speedboatItem->Pose.Position.y - 5;
		laraItem->Pose.Position.z = speedboatItem->Pose.Position.z;
		laraItem->Pose.Orientation.x = 0;
		laraItem->Pose.Orientation.y = speedboatItem->Pose.Orientation.y;
		laraItem->Pose.Orientation.z = 0;
		laraItem->Animation.Velocity = 0;
		laraItem->Animation.VerticalVelocity = 0;
		laraItem->Animation.Airborne = false;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = SPEEDBOAT_STATE_MOUNT;
		laraItem->Animation.TargetState = SPEEDBOAT_STATE_MOUNT;
		lara->Control.WaterStatus = WaterStatus::Dry;

		if (laraItem->RoomNumber != speedboatItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, speedboatItem->RoomNumber);

		AnimateItem(laraItem);

		if (g_Level.Items[itemNumber].Status != ITEM_ACTIVE)
		{
			AddActiveItem(itemNumber);
			g_Level.Items[itemNumber].Status = ITEM_ACTIVE;
		}

		lara->Vehicle = itemNumber;
	}

	void SpeedboatControl(short itemNumber)
	{
		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);
		auto* speedboatItem = &g_Level.Items[itemNumber];
		auto* speedboat = GetSpeedboatInfo(speedboatItem);

		int collide = SpeedboatDynamics(laraItem, itemNumber);

		Vector3Int frontLeft, frontRight;
		int heightFrontLeft = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, -SPEEDBOAT_SIDE, &frontLeft);
		int heightFrontRight = SpeedboatTestWaterHeight(speedboatItem, SPEEDBOAT_FRONT, SPEEDBOAT_SIDE, &frontRight);

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
				noTurn = SpeedboatUserControl(laraItem, speedboatItem);
				break;
			}
		}
		else
		{
			if (speedboatItem->Animation.Velocity > SPEEDBOAT_SLOWDOWN)
				speedboatItem->Animation.Velocity -= SPEEDBOAT_SLOWDOWN;
			else
				speedboatItem->Animation.Velocity = 0;
		}

		if (noTurn)
		{
			if (speedboat->TurnRate < -SPEEDBOAT_UNDO_TURN)
				speedboat->TurnRate += SPEEDBOAT_UNDO_TURN;
			else if (speedboat->TurnRate > SPEEDBOAT_UNDO_TURN)
				speedboat->TurnRate -= SPEEDBOAT_UNDO_TURN;
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
			SpeedboatAnimation(laraItem, speedboatItem, collide);

			if (probe.RoomNumber != speedboatItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			laraItem->Pose.Position.x = speedboatItem->Pose.Position.x;
			laraItem->Pose.Position.y = speedboatItem->Pose.Position.y;
			laraItem->Pose.Position.z = speedboatItem->Pose.Position.z;
			laraItem->Pose.Orientation.x = speedboatItem->Pose.Orientation.x;
			laraItem->Pose.Orientation.y = speedboatItem->Pose.Orientation.y;
			laraItem->Pose.Orientation.z = speedboatItem->Pose.Orientation.z;
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
				float pitch  = idle ? 1.0f : 1.0f + speedboat->Pitch / (float)SPEEDBOAT_MAX_VELOCITY / 4.0f;
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

		DoSpeedboatDismount(laraItem, speedboatItem);
	}
}
