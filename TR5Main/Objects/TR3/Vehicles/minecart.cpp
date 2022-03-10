#include "framework.h"
#include "Objects/TR3/Vehicles/minecart.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/minecart_info.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

#define DISMOUNT_DISTANCE 330
#define CART_DEC -1536
#define CART_MIN_SPEED 2560
#define CART_MIN_VEL 32
#define TURN_DEATH_VEL 128
#define CART_FORWARD_GRADIENT -128
#define CART_BACK_GRADIENT 128
#define CART_JUMP_VELOCITY 64512
#define CART_GRAVITY (SECTOR(1) + 1)
#define CART_MAX_VERTICAL_VELOCITY 16128
#define TERMINAL_ANGLE SECTOR(4)
#define CART_RADIUS 100
#define CART_HEIGHT CLICK(3)
#define CART_NHITS 25
#define CART_ENTITY_RADIUS CLICK(1)

#define CART_IN_SWIPE		IN_ACTION
#define CART_IN_DUCK		IN_CROUCH
#define CART_IN_DISMOUNT	IN_ROLL
#define CART_IN_BRAKE		(IN_BACK | IN_JUMP)
#define CART_IN_LEFT		IN_LEFT
#define CART_IN_RIGHT		IN_RIGHT

enum MinecartState
{
	CART_STATE_MOUNT,
	CART_STATE_DISMOUNT,
	CART_STATE_DISMOUNT_LEFT,
	CART_STATE_DISMOUNT_RIGHT,
	CART_STATE_IDLE,
	CART_STATE_DUCK,
	CART_STATE_MOVE,
	CART_STATE_RIGHT,
	CART_STATE_HARD_LEFT,	// Unused.
	CART_STATE_LEFT,
	CART_STATE_HARD_RIGHT,	// Unused.
	CART_STATE_BRAKE,
	CART_STATE_FORWARD,
	CART_STATE_BACK,
	CART_TURN_DEATH,
	CART_FALL_DEATH,
	CART_WALL_DEATH,
	CART_STATE_HIT,
	CART_STATE_SWIPE,
	CART_STATE_BRAKING
};

enum MinecartAnim
{
	CART_ANIM_MOUNT_LEFT = 0,
	CART_ANIM_DISMOUNT_LEFT = 1,
	CART_ANIM_DUCK_START = 2,
	CART_ANIM_DUCK_CONTINUE = 3,
	CART_ANIM_DUCK_END = 4,
	CART_ANIM_PICK_UP_WRENCH = 5,
	CART_ANIM_SWIPE_WRENCH = 6,
	CART_ANIM_PUT_DOWN_WRENCH = 7,
	CART_ANIM_LEAN_LEFT_START = 8,
	CART_ANIM_LEAN_RIGHT_CONTINUE = 9,
	CART_ANIM_BRAKE_DISENGAGE = 10,	// Unused?
	CART_ANIM_BRAKE_ENGAGE = 11,	// Unused?
	CART_ANIM_LEAN_RIGHT_START = 12,
	CART_ANIM_LEAN_LEFT_CONTINUE = 13,
	CART_ANIM_LEAN_RIGHT_BRAKE_ENGAGE = 14,
	CART_ANIM_LEAN_LEFT_BRAKE_ENGAGE = 15,
	CART_ANIM_LEAN_RIGHT_HARD_CONTINUE = 16,	// Unused?
	CART_ANIM_LEAN_LEFT_HARD_CONTINUE = 17,		// Unused?
	CART_ANIM_LEAN_RIGHT_HARD_START = 18,		// Unused?
	CART_ANIM_LEAN_RIGHT_HARD_END = 19,			// Unused?
	CART_ANIM_LEAN_LEFT_HARD_START = 20,		// Unused?
	CART_ANIM_LEAN_LEFT_HARD_END = 21,			// Unused?
	CART_ANIM_RIDE_FORWARD = 22,
	CART_ANIM_WALL_DEATH = 23,
	CART_ANIM_LEAN_FORWARD_START = 24,
	CART_ANIM_LEAN_FORWARD_CONTINUE = 25,
	CART_ANIM_LEAN_FORWARD_END = 26,
	CART_ANIM_LEAN_BACK_START = 27,
	CART_ANIM_LEAN_BACK_CONTINUE = 28,
	CART_ANIM_LEAN_BACK_END = 29,
	CART_ANIM_FALL_DEATH = 30,	// Unused?
	CART_ANIM_TURN_DEATH = 31,	// Unused?
	CART_ANIM_FALL_OUT_START = 32,
	CART_ANIM_FALL_OUT_END = 33,
	CART_ANIM_BONK_HEAD = 34,
	CART_ANIM_LEAN_FORWARD_DUCK_START = 35,
	CART_ANIM_LEAN_FORWARD_DUCK_CONTINUE = 36,
	CART_ANIM_LEAN_FORWARD_DUCK_END = 37,
	CART_ANIM_LEAN_BACK_DUCK_START = 38,
	CART_ANIM_LEAN_BACK_DUCK_CONTINUE = 39,
	CART_ANIM_LEAN_BACK_DUCK_END = 40,
	CART_ANIM_LEAN_RIGHT_BRAKE_DISENGAGE = 41,	// Unused?
	CART_ANIM_LEAN_LEFT_BRAKE_DISENGAGE = 42,	// Unused?
	CART_ANIM_LEAN_RIGHT_END = 43,
	CART_ANIM_LEAN_LEFT_END = 44,
	CART_ANIM_IDLE = 45,
	CART_ANIM_MOUNT_RIGHT = 46,
	CART_ANIM_DISMOUNT_RIGHT = 47,
	CART_ANIM_BRAKE = 48,
};

enum MinecartFlags
{
	CART_FLAG_MESH = 1,
	CART_FLAG_TURNING_LEFT = 2,
	CART_FLAG_TURNING_RIGHT = 4,
	CART_FLAG_RDIR = 8,	// TODO
	CART_FLAG_CONTROL = 16,
	CART_FLAG_STOPPED = 32,
	CART_FLAG_NO_ANIM = 64,
	CART_FLAG_DEAD = 128
};

void InitialiseMineCart(short itemNumber)
{
	auto* minecartItem = &g_Level.Items[itemNumber];
	minecartItem->Data = MinecartInfo();
	auto* minecart = (MinecartInfo*)minecartItem->Data;

	minecart->Velocity = 0;
	minecart->VerticalVelocity = 0;
	minecart->Gradient = 0;
	minecart->Flags = NULL;
}

static int TestMinecartHeight(ITEM_INFO* minecartItem, int xOffset, int zOffset)
{
	float s = phd_sin(minecartItem->Position.yRot);
	float c = phd_cos(minecartItem->Position.yRot);

	PHD_VECTOR pos;
	pos.x = minecartItem->Position.xPos + zOffset * s + xOffset * c;
	pos.y = minecartItem->Position.yPos - zOffset * phd_sin(minecartItem->Position.xRot) + xOffset * phd_sin(minecartItem->Position.zRot);
	pos.z = minecartItem->Position.zPos + zOffset * c - xOffset * s;

	return GetCollisionResult(pos.x, pos.y, pos.z, minecartItem->RoomNumber).Position.Floor;
}

static short GetMinecartCollision(ITEM_INFO* minecartItem, short angle, int distance)
{
	auto probe = GetCollisionResult(minecartItem, angle, distance, -LARA_HEIGHT);

	if (probe.Position.Floor != NO_HEIGHT)
		probe.Position.Floor -= minecartItem->Position.yPos;

	return (short)probe.Position.Floor;
}

static bool GetInMineCart(ITEM_INFO* minecartItem, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(laraItem);

	if (!(TrInput & CART_IN_SWIPE) || lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Airborne)
	{
		return false;
	}

	if (!TestBoundsCollide(minecartItem, laraItem, coll->Setup.Radius))
		return false;

	if (!TestCollision(minecartItem, laraItem))
		return false;

	int x = laraItem->Position.xPos - minecartItem->Position.xPos;
	int z = laraItem->Position.zPos - minecartItem->Position.zPos;

	int distance = pow(x, 2) + pow(z, 2);
	if (distance > pow(CLICK(2), 2))
		return false;

	if (GetCollisionResult(minecartItem).Position.Floor < -32000)
		return false;

	return true;
}

static bool TestMinecartDismount(ITEM_INFO* laraItem, int direction)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* minecartItem = &g_Level.Items[lara->Vehicle];
	
	short angle;
	if (direction < 0)
		angle = minecartItem->Position.yRot + ANGLE(90.0f);
	else
		angle = minecartItem->Position.yRot - ANGLE(90.0f);

	auto probe = GetCollisionResult(minecartItem, angle, -DISMOUNT_DISTANCE);

	if (probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT)
		return false;

	if (abs(probe.Position.Floor - minecartItem->Position.yPos) > CLICK(2))
		return false;

	if ((probe.Position.Ceiling - minecartItem->Position.yPos) > -LARA_HEIGHT ||
		(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

static void CartToEntityCollision(ITEM_INFO* laraItem, ITEM_INFO* minecartItem)
{
	vector<short> roomsList;
	roomsList.push_back(minecartItem->RoomNumber);

	auto* room = &g_Level.Rooms[minecartItem->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsList.push_back(room->doors[i].room);

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNumber = g_Level.Rooms[roomsList[i]].itemNumber;

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
					int x = minecartItem->Position.xPos - item->Position.xPos;
					int y = minecartItem->Position.yPos - item->Position.yPos;
					int z = minecartItem->Position.zPos - item->Position.zPos;
					if (x > -SECTOR(2) && x < SECTOR(2) &&
						y > -SECTOR(2) && y < SECTOR(2) &&
						z > -SECTOR(2) && z < SECTOR(2))
					{
						if (TestBoundsCollide(item, laraItem, CART_ENTITY_RADIUS))
						{
							if (item->ObjectNumber == ID_ANIMATING2)
							{
								if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase &&
									(laraItem->ActiveState == CART_STATE_SWIPE &&
										laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 6))
								{
									int frame = laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase;
									if (frame >= 12 && frame <= 22)
									{
										SoundEffect(220, &item->Position, 2);
										TestTriggers(item, true);
										item->FrameNumber++;
									}
								}
							}
							else if (item->ObjectNumber == ID_ROLLINGBALL)
							{
								/*code, kill lara and stop both the boulder and the minecart*/
							}
							else
							{
								DoLotsOfBlood(item->Position.xPos, minecartItem->Position.yPos - CLICK(1), item->Position.zPos, GetRandomControl() & 3, minecartItem->Position.yRot, item->RoomNumber, 3);
								item->HitPoints = 0;
							}
						}
					}
				}
			}

			itemNumber = item->NextItem;
		}
	}
}

static void MoveCart(ITEM_INFO* laraItem, ITEM_INFO* minecartItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* minecart = (MinecartInfo*)minecartItem->Data;

	if (minecart->StopDelay)
		minecart->StopDelay--;

	if ((lara->Control.Minecart.Left && lara->Control.Minecart.Right && !minecart->StopDelay) &&
		(minecartItem->Position.xPos & 0x380 == 512 ||
			minecartItem->Position.zRot & 0x380 == 512))
	{
		if (minecart->Velocity < 0xf000)
		{
			minecartItem->Velocity = 0;
			minecart->Velocity = 0;
			minecart->Flags |= CART_FLAG_STOPPED | CART_FLAG_CONTROL;
			return;
		}
		else
			minecart->StopDelay = 16;
	}

	if ((lara->Control.Minecart.Left || lara->Control.Minecart.Right) &&
		!(lara->Control.Minecart.Left && lara->Control.Minecart.Right) &&
		!minecart->StopDelay &&
		!(minecart->Flags & (CART_FLAG_TURNING_LEFT | CART_FLAG_TURNING_RIGHT)))
	{
		short angle;
		unsigned short rotation = (((unsigned short)minecartItem->Position.yRot) / ANGLE(90.0f)) | (lara->Control.Minecart.Left * 4);

		switch (rotation)
		{
		case 0:
			minecart->TurnX = (minecartItem->Position.xPos + 4096) & ~1023;
			minecart->TurnZ = minecartItem->Position.zPos & ~1023;
			break;

		case 1:
			minecart->TurnX = minecartItem->Position.xPos & ~1023;
			minecart->TurnZ = (minecartItem->Position.zPos - 4096) | 1023;
			break;

		case 2:
			minecart->TurnX = (minecartItem->Position.xPos - 4096) | 1023;
			minecart->TurnZ = minecartItem->Position.zPos | 1023;
			break;

		case 3:
			minecart->TurnX = minecartItem->Position.xPos | 1023;
			minecart->TurnZ = (minecartItem->Position.zPos + 4096) & ~1023;
			break;

		case 4:
			minecart->TurnX = (minecartItem->Position.xPos - 4096) | 1023;
			minecart->TurnZ = minecartItem->Position.zPos & ~1023;
			break;

		case 5:
			minecart->TurnX = minecartItem->Position.xPos & ~1023;
			minecart->TurnZ = (minecartItem->Position.zPos + 4096) & ~1023;
			break;

		case 6:
			minecart->TurnX = (minecartItem->Position.xPos + 4096) & ~1023;
			minecart->TurnZ = minecartItem->Position.zPos | 1023;
			break;

		case 7:
			minecart->TurnX = minecartItem->Position.xPos | 1023;
			minecart->TurnZ = (minecartItem->Position.zPos - 4096) | 1023;
			break;
		}

		angle = mGetAngle(minecartItem->Position.xPos, minecartItem->Position.zPos, minecart->TurnX, minecart->TurnZ) & 0x3fff;

		if (rotation < 4)
		{
			minecart->TurnRot = minecartItem->Position.yRot;
			minecart->TurnLen = angle;
		}
		else
		{
			minecart->TurnRot = minecartItem->Position.yRot;

			if (angle)
				angle = ANGLE(90.0f) - angle;

			minecart->TurnLen = angle;
		}

		minecart->Flags |= (lara->Control.Minecart.Left) ? CART_FLAG_TURNING_LEFT : CART_FLAG_TURNING_RIGHT;
	}

	if (minecart->Velocity < CART_MIN_SPEED)
		minecart->Velocity = CART_MIN_SPEED;

	minecart->Velocity += -minecart->Gradient * 4;

	minecart->Velocity /= 256; // TODO: Then why use the huge values in the first place??
	if (minecartItem->Velocity < CART_MIN_VEL)
	{
		minecartItem->Velocity = CART_MIN_VEL;
		StopSoundEffect(209);

		if (minecart->VerticalVelocity)
			StopSoundEffect(210);
		else
			SoundEffect(210, &minecartItem->Position, 2);
	}
	else
	{
		StopSoundEffect(210);

		if (minecart->VerticalVelocity)
			StopSoundEffect(209);
		else
			SoundEffect(209, &minecartItem->Position, (2 | 4) + 0x1000000 + (minecartItem->Velocity * 32768));
	}

	if (minecart->Flags & (CART_FLAG_TURNING_LEFT | CART_FLAG_TURNING_RIGHT))
	{
		minecart->TurnLen += minecartItem->Velocity * 3;
		if (minecart->TurnLen > ANGLE(90.0f))
		{
			if (minecart->Flags & CART_FLAG_TURNING_LEFT)
				minecartItem->Position.yRot = minecart->TurnRot - ANGLE(90.0f);
			else
				minecartItem->Position.yRot = minecart->TurnRot + ANGLE(90.0f);

			minecart->Flags &= ~(CART_FLAG_TURNING_LEFT | CART_FLAG_TURNING_RIGHT);
		}
		else
		{
			if (minecart->Flags & CART_FLAG_TURNING_LEFT)
				minecartItem->Position.yRot = minecart->TurnRot - minecart->TurnLen;
			else
				minecartItem->Position.yRot = minecart->TurnRot + minecart->TurnLen;
		}

		if (minecart->Flags & (CART_FLAG_TURNING_LEFT | CART_FLAG_TURNING_RIGHT))
		{
			unsigned short quadrant = ((unsigned short)minecartItem->Position.yRot) / ANGLE(90.0f);
			unsigned short degree = minecartItem->Position.yRot & 16383;

			float x, z;
			switch (quadrant)
			{
			case 0:
				x = -phd_cos(degree);
				z = phd_sin(degree);
				break;

			case 1:
				x = phd_sin(degree);
				z = phd_cos(degree);
				break;

			case 2:
				x = phd_cos(degree);
				z = -phd_sin(degree);
				break;

			default:
				x = -phd_sin(degree);
				z = -phd_cos(degree);
				break;
			}

			if (minecart->Flags & CART_FLAG_TURNING_LEFT)
			{
				x = -x;
				z = -z;
			}

			minecartItem->Position.xPos = minecart->TurnX + x * 3584;
			minecartItem->Position.zPos = minecart->TurnZ + z * 3584;
		}
	}
	else
	{
		minecartItem->Position.xPos += minecartItem->Velocity * phd_sin(minecartItem->Position.yRot);
		minecartItem->Position.zPos += minecartItem->Velocity * phd_cos(minecartItem->Position.yRot);
	}

	minecart->FloorHeightMiddle = TestMinecartHeight(minecartItem, 0, 0);

	if (!minecart->VerticalVelocity)
	{
		minecart->FloorHeightFront = TestMinecartHeight(minecartItem, 0, CLICK(1));
		minecart->Gradient = minecart->FloorHeightMiddle - minecart->FloorHeightFront;
		minecartItem->Position.yPos = minecart->FloorHeightMiddle;
	}
	else
	{
		if (minecartItem->Position.yPos > minecart->FloorHeightMiddle)
		{
			if (minecart->VerticalVelocity > 0)
				SoundEffect(202, &minecartItem->Position, 2);

			minecartItem->Position.yPos = minecart->FloorHeightMiddle;
			minecart->VerticalVelocity = 0;
		}
		else
		{
			minecart->VerticalVelocity += CART_GRAVITY;
			if (minecart->VerticalVelocity > CART_MAX_VERTICAL_VELOCITY)
				minecart->VerticalVelocity = CART_MAX_VERTICAL_VELOCITY;

			minecartItem->Position.yPos += minecart->VerticalVelocity / 256;
		}
	}

	minecartItem->Position.xRot = minecart->Gradient * 32;

	if (minecart->Flags & (CART_FLAG_TURNING_LEFT | CART_FLAG_TURNING_RIGHT))
	{
		short val = minecartItem->Position.yRot & 16383;
		if (minecart->Flags & CART_FLAG_TURNING_RIGHT)
			minecartItem->Position.zRot = -(val * minecartItem->Velocity) / 512;
		else
			minecartItem->Position.zRot = ((ANGLE(90.0f) - val) * minecartItem->Velocity) / 512;
	}
	else
		minecartItem->Position.zRot -= minecartItem->Position.zRot / 8;
}

static void DoUserInput(ITEM_INFO* minecartItem, ITEM_INFO* laraItem, MinecartInfo* minecart)
{
	auto* lara = GetLaraInfo(laraItem);

	short floorHeight;

	switch (laraItem->ActiveState)
	{
	case CART_STATE_MOVE:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;
		else if (minecart->Velocity == CART_MIN_VEL || minecart->Flags & CART_FLAG_STOPPED)
			laraItem->TargetState = CART_STATE_IDLE;
		else if (minecart->Gradient < CART_FORWARD_GRADIENT)
			laraItem->TargetState = CART_STATE_FORWARD;
		else if (minecart->Gradient > CART_BACK_GRADIENT)
			laraItem->TargetState = CART_STATE_BACK;
		else if (TrInput & CART_IN_LEFT)
			laraItem->TargetState = CART_STATE_LEFT;
		else if (TrInput & CART_IN_RIGHT)
			laraItem->TargetState = CART_STATE_RIGHT;

		break;

	case CART_STATE_FORWARD:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;
		else if (minecart->Gradient > CART_FORWARD_GRADIENT)
			laraItem->TargetState = CART_STATE_MOVE;

		break;

	case CART_STATE_BACK:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;
		else if (minecart->Gradient < CART_BACK_GRADIENT)
			laraItem->TargetState = CART_STATE_MOVE;

		break;

	case CART_STATE_LEFT:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;

		if (!(TrInput & CART_IN_LEFT))
			laraItem->TargetState = CART_STATE_MOVE;

		break;

	case CART_STATE_RIGHT:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;

		if (!(TrInput & CART_IN_RIGHT))
			laraItem->TargetState = CART_STATE_MOVE;

		break;

	case CART_STATE_IDLE:
		if (!(minecart->Flags & CART_FLAG_CONTROL))
		{
			SoundEffect(211, &minecartItem->Position, 2);
			minecart->Flags |= CART_FLAG_CONTROL;
			minecart->StopDelay = 64;
		}

		if (TrInput & CART_IN_DISMOUNT && minecart->Flags & CART_FLAG_STOPPED)
		{
			if (TrInput & CART_IN_LEFT && TestMinecartDismount(laraItem, -1))
			{
				laraItem->TargetState = CART_STATE_DISMOUNT;
				minecart->Flags &= ~CART_FLAG_RDIR;
			}
			else if (TrInput & CART_IN_RIGHT && TestMinecartDismount(laraItem, 1))
			{
				laraItem->TargetState = CART_STATE_DISMOUNT;
				minecart->Flags |= CART_FLAG_RDIR;
			}
		}

		if (TrInput & CART_IN_DUCK)
			laraItem->TargetState = CART_STATE_DUCK;
		else if (minecart->Velocity > CART_MIN_VEL)
			laraItem->TargetState = CART_STATE_MOVE;

		break;

	case CART_STATE_DUCK:
		if (TrInput & CART_IN_SWIPE)
			laraItem->TargetState = CART_STATE_SWIPE;
		else if (TrInput & CART_IN_BRAKE)
			laraItem->TargetState = CART_STATE_BRAKE;
		else if (!(TrInput & CART_IN_DUCK))
			laraItem->TargetState = CART_STATE_IDLE;

		break;

	case CART_STATE_SWIPE:
		laraItem->TargetState = CART_STATE_MOVE;
		break;

	case CART_STATE_BRAKING:
		if (TrInput & CART_IN_DUCK)
		{
			laraItem->TargetState = CART_STATE_DUCK;
			StopSoundEffect(219);
		}
		else if (!(TrInput & CART_IN_BRAKE) || minecart->Flags & CART_FLAG_STOPPED)
		{
			laraItem->TargetState = CART_STATE_MOVE;
			StopSoundEffect(219);
		}
		else
		{
			minecart->Velocity += CART_DEC;
			SoundEffect(219, &laraItem->Position, 2);
		}

		break;

	case CART_STATE_BRAKE:
		laraItem->TargetState = CART_STATE_BRAKING;
		break;

	case CART_STATE_DISMOUNT:
		if (laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 7)
		{
			if (laraItem->FrameNumber == GetFrameNumber(minecartItem, 20) &&
				minecart->Flags & CART_FLAG_MESH)
			{
				lara->MeshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;
				minecart->Flags &= ~CART_FLAG_MESH;
			}

			if (minecart->Flags & CART_FLAG_RDIR)
				laraItem->TargetState = CART_STATE_DISMOUNT_RIGHT;
			else
				laraItem->TargetState = CART_STATE_DISMOUNT_LEFT;
		}

		break;

	case CART_STATE_DISMOUNT_LEFT:
		if (laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 1 &&
			laraItem->FrameNumber == g_Level.Anims[laraItem->AnimNumber].frameEnd)
		{
			PHD_VECTOR pos = { 0, 640, 0 };
			GetLaraJointPosition(&pos, LM_HIPS);

			laraItem->Position.xPos = pos.x;
			laraItem->Position.yPos = pos.y;
			laraItem->Position.zPos = pos.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			minecartItem->Position.yRot + ANGLE(90.0f);

			SetAnimation(laraItem, LA_STAND_SOLID);
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
		}

		break;

	case CART_STATE_DISMOUNT_RIGHT:
		if (laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 47 &&
			laraItem->FrameNumber == g_Level.Anims[laraItem->AnimNumber].frameEnd)
		{
			PHD_VECTOR pos = { 0, 640, 0 };
			GetLaraJointPosition(&pos, LM_HIPS);

			laraItem->Position.xPos = pos.x;
			laraItem->Position.yPos = pos.y;
			laraItem->Position.zPos = pos.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			minecartItem->Position.yRot + ANGLE(90.0f);

			SetAnimation(laraItem, LA_STAND_SOLID);
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
		}

		break;

	case CART_STATE_MOUNT:
		if (laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 5 &&
			laraItem->FrameNumber == GetFrameNumber(minecartItem, 20) &&
			!minecart->Flags & CART_FLAG_MESH)
		{
			auto temp = g_Level.Meshes[lara->MeshPtrs[LM_RHAND]];

			lara->MeshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;
			g_Level.Meshes[Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND] = temp;

			minecart->Flags |= CART_FLAG_MESH;
		}

		break;

	case CART_WALL_DEATH:
		Camera.targetElevation = -ANGLE(25.0f);
		Camera.targetDistance = SECTOR(4);

		break;

	case CART_TURN_DEATH:
		Camera.targetElevation = -ANGLE(45.0f);
		Camera.targetDistance = SECTOR(2);

		floorHeight = GetMinecartCollision(minecartItem, minecartItem->Position.yRot, CLICK(2));
		if (floorHeight > -CLICK(1) &&
			floorHeight < CLICK(1))
		{
			if (Wibble & 7 == 0)
				SoundEffect(SFX_TR3_QUADBIKE_FRONT_IMPACT, &minecartItem->Position, 2);

			minecartItem->Position.xPos += TURN_DEATH_VEL * phd_sin(minecartItem->Position.yRot);
			minecartItem->Position.zPos += TURN_DEATH_VEL * phd_cos(minecartItem->Position.yRot);
		}
		else
		{
			if (laraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 30)
			{
				minecart->Flags |= CART_FLAG_NO_ANIM;
				laraItem->HitPoints = -1;
			}
		}

		break;

	case CART_STATE_HIT:
		if (laraItem->HitPoints <= 0 &&
			laraItem->FrameNumber == GetFrameNumber(minecartItem, 34) + 28)
		{
			laraItem->FrameNumber = GetFrameNumber(minecartItem, 34) + 28;
			minecartItem->Velocity = 0;
			minecart->Flags = (minecart->Flags & ~CART_FLAG_CONTROL) | CART_FLAG_NO_ANIM;
			minecart->Velocity = 0;
		}

		break;
	}

	if (lara->Vehicle != NO_ITEM &&
		!(minecart->Flags & CART_FLAG_NO_ANIM))
	{
		AnimateItem(laraItem);

		minecartItem->AnimNumber = Objects[ID_MINECART].animIndex + (laraItem->AnimNumber - Objects[ID_MINECART_LARA_ANIMS].animIndex);
		minecartItem->FrameNumber = g_Level.Anims[minecartItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);
	}
	if (laraItem->ActiveState != CART_TURN_DEATH &&
		laraItem->ActiveState != CART_WALL_DEATH &&
		laraItem->HitPoints > 0)
	{
		if (minecartItem->Position.zRot > TERMINAL_ANGLE ||
			minecartItem->Position.zRot < -TERMINAL_ANGLE)
		{
			laraItem->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 31;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraItem->ActiveState = laraItem->TargetState = CART_TURN_DEATH;
			minecartItem->Velocity = 0;
			minecart->Flags = (minecart->Flags & ~CART_FLAG_CONTROL) | CART_FLAG_STOPPED | CART_FLAG_DEAD;
			minecart->Velocity = 0;
			return;
		}

		floorHeight = GetMinecartCollision(minecartItem, minecartItem->Position.yRot, CLICK(2));
		if (floorHeight < -CLICK(2))
		{
			laraItem->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 23;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraItem->ActiveState = laraItem->TargetState = CART_WALL_DEATH;
			laraItem->HitPoints = -1;
			minecartItem->Velocity = 0;
			minecart->Flags = (minecart->Flags & ~CART_FLAG_CONTROL) | (CART_FLAG_STOPPED | CART_FLAG_DEAD);
			minecart->Velocity = 0;
			return;
		}

		if (laraItem->ActiveState != CART_STATE_DUCK &&
			laraItem->ActiveState != CART_STATE_HIT)
		{
			COLL_INFO coll;
			coll.Setup.Radius = CART_RADIUS;
			DoObjectCollision(minecartItem, &coll);

			if (coll.HitStatic)
			{
				laraItem->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 34;
				laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
				laraItem->ActiveState = laraItem->TargetState = CART_STATE_HIT;
				DoLotsOfBlood(laraItem->Position.xPos, laraItem->Position.yPos - CLICK(3), laraItem->Position.zPos, minecartItem->Velocity, minecartItem->Position.yRot, laraItem->RoomNumber, 3);

				int hits = CART_NHITS * short(minecart->Velocity / 2048);
				if (hits < 20)
					hits = 20;

				laraItem->HitPoints -= hits;
				return;
			}
		}

		if (floorHeight > CLICK(2.25f) && !minecart->VerticalVelocity)
			minecart->VerticalVelocity = CART_JUMP_VELOCITY;

		CartToEntityCollision(laraItem, minecartItem);
	}
}

void MineCartCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(laraItem);

	if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
		return;

	auto* minecartItem = &g_Level.Items[itemNumber];

	int mount = GetInMineCart(minecartItem, laraItem, coll);
	if (mount)
	{
		lara->Vehicle = itemNumber;

		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, FALSE);
			UndrawFlareMeshes(laraItem);
			lara->Flare.ControlLeft = false;
			lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
			lara->Control.Weapon.GunType = LaraWeaponType::None;
		}

		lara->Control.HandStatus = HandStatus::Busy;

		short angle = short(mGetAngle(minecartItem->Position.xPos, minecartItem->Position.zPos, laraItem->Position.xPos, laraItem->Position.zPos) - minecartItem->Position.yRot);
		if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
			laraItem->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + CART_ANIM_MOUNT_RIGHT;
		else
			laraItem->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + CART_ANIM_MOUNT_LEFT;

		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->TargetState = CART_STATE_MOUNT;
		laraItem->ActiveState = CART_STATE_MOUNT;

		laraItem->Position.xPos = minecartItem->Position.xPos;
		laraItem->Position.yPos = minecartItem->Position.yPos;
		laraItem->Position.zPos = minecartItem->Position.zPos;
		laraItem->Position.xRot = minecartItem->Position.xRot;
		laraItem->Position.yRot = minecartItem->Position.yRot;
		laraItem->Position.zRot = minecartItem->Position.zRot;
	}
	else
		ObjectCollision(itemNumber, laraItem, coll);
}

bool MineCartControl(ITEM_INFO* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* minecartItem = &g_Level.Items[lara->Vehicle];

	if (!minecartItem->Data) 
	{
		TENLog("Minecart data is nullptr!", LogLevel::Error);
		return false; 
	}
	auto* minecart = (MinecartInfo*)minecartItem->Data;

	DoUserInput(minecartItem, laraItem, minecart);

	if (minecart->Flags & CART_FLAG_CONTROL)
		MoveCart(laraItem, minecartItem);

	if (lara->Vehicle != NO_ITEM)
	{
		laraItem->Position.xPos = minecartItem->Position.xPos;
		laraItem->Position.yPos = minecartItem->Position.yPos;
		laraItem->Position.zPos = minecartItem->Position.zPos;
		laraItem->Position.xRot = minecartItem->Position.xRot;
		laraItem->Position.yRot = minecartItem->Position.yRot;
		laraItem->Position.zRot = minecartItem->Position.zRot;
	}

	short probedRoomNumber = GetCollisionResult(minecartItem).RoomNumber;
	if (probedRoomNumber != minecartItem->RoomNumber)
	{
		ItemNewRoom(lara->Vehicle, probedRoomNumber);
		ItemNewRoom(lara->ItemNumber, probedRoomNumber);
	}

	TestTriggers(minecartItem, false);

	if (!(minecart->Flags & CART_FLAG_DEAD))
	{
		Camera.targetElevation = -ANGLE(45.0f);
		Camera.targetDistance = SECTOR(2);
	}

	return (lara->Vehicle == NO_ITEM) ? false : true;
}
