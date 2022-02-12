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
#include "Objects/TR3/Vehicles/minecart_info.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

enum MinecartState
{
	CART_GETIN,
	CART_GETOUT,
	CART_GETOUTL,
	CART_GETOUTR,
	CART_STILL,
	CART_DUCK,
	CART_MOVE,
	CART_RIGHT,
	CART_HARDLEFT,
	CART_LEFT,
	CART_HARDRIGHT,
	CART_BRAKE,
	CART_FWD,
	CART_BACK,
	CART_TURNDEATH,
	CART_FALLDEATH,
	CART_WALLDEATH,
	CART_HIT,
	CART_USE,
	CART_BRAKING
};

enum MINECART_FLAGS
{
	CF_MESH = 1,
	CF_TURNINGL = 2,
	CF_TURNINGR = 4,
	CF_RDIR = 8,
	CF_CONTROL = 16,
	CF_STOPPED = 32,
	CF_NOANIM = 64,
	CF_DEAD = 128
};

#define GETOFF_DIST 330
#define CART_DEC -1536
#define CART_MIN_SPEED 2560
#define CART_MIN_VEL 32
#define TURN_DEATH_VEL 128
#define CART_FWD_GRAD -128
#define CART_BACK_GRAD 128
#define CART_JUMP_VEL 64512
#define CART_GRAVITY (WALL_SIZE + 1)
#define MAX_CART_YVEL 16128
#define TERMINAL_ANGLE (WALL_SIZE * 4)
#define CART_RADIUS 100
#define CART_HEIGHT (STEP_SIZE * 3)
#define CART_NHITS 25
#define CART_BADDIE_RADIUS STEP_SIZE

static int TestMinecartHeight(ITEM_INFO* v, int x, int z)
{
	PHD_VECTOR pos;
	FLOOR_INFO* floor;
	float s, c;
	short roomNumber;

	c = phd_cos(v->Position.yRot);
	s = phd_sin(v->Position.yRot);

	pos.x = v->Position.xPos + z * s + x * c;
	pos.y = v->Position.yPos - z * phd_sin(v->Position.xRot) + x * phd_sin(v->Position.zRot);
	pos.z = v->Position.zPos + z * c - x * s;

	roomNumber = v->RoomNumber;
	floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	return GetFloorHeight(floor, pos.x, pos.y, pos.z);
}

static short GetCollision(ITEM_INFO* v, short ang, int dist, short* ceiling)
{
	FLOOR_INFO* floor;
	int x, y, z, height, cheight;
	short roomNumber;

	x = v->Position.xPos + phd_sin(ang) * dist;
	y = v->Position.yPos - LARA_HEIGHT;
	z = v->Position.zPos + phd_cos(ang) * dist;

	roomNumber = v->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	height = GetFloorHeight(floor, x, y, z);
	cheight = GetCeiling(floor, x, y, z);

	*ceiling = ((short)cheight);

	if (height != NO_HEIGHT)
		height -= v->Position.yPos;

	return short(height);
}

static bool GetInMineCart(ITEM_INFO* v, ITEM_INFO* l, COLL_INFO* coll)
{
	int dist;
	int x, z;
	FLOOR_INFO* floor;
	short roomNumber;

	if (!(TrInput & IN_ACTION) || Lara.Control.HandStatus != HandStatus::Free || l->Airborne)
		return 0;

	if (!TestBoundsCollide(v, l, coll->Setup.Radius))
		return false;

	if (!TestCollision(v, l))
		return false;

	x = l->Position.xPos - v->Position.xPos;
	z = l->Position.zPos - v->Position.zPos;

	dist = SQUARE(x) + SQUARE(z);

	if (dist > SQUARE(WALL_SIZE/2))
		return false;

	roomNumber = v->RoomNumber;
	floor = GetFloor(v->Position.xPos, v->Position.yPos, v->Position.zPos, &roomNumber);
	if (GetFloorHeight(floor, v->Position.xPos, v->Position.yPos, v->Position.zPos) < -32000)
		return false;

	return true;
}

static bool CanGetOut(int direction)
{
	auto v = &g_Level.Items[Lara.Vehicle];

	short angle;
	if (direction < 0)
		angle = v->Position.yRot + 0x4000;
	else
		angle = v->Position.yRot - 0x4000;

	int x = v->Position.xPos - GETOFF_DIST * phd_sin(angle);
	int y = v->Position.yPos;
	int z = v->Position.zPos - GETOFF_DIST * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, v->RoomNumber);

	if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT)
		return false;

	if (abs(collResult.Position.Floor - v->Position.yPos) > WALL_SIZE / 2)
		return false;

	if ((collResult.Position.Ceiling - v->Position.yPos > -LARA_HEIGHT) || (collResult.Position.Floor - collResult.Position.Ceiling < LARA_HEIGHT))
		return false;

	return true;
}

static void CartToBaddieCollision(ITEM_INFO* v)
{
	vector<short> roomsList;
	short* door, numDoors;

	roomsList.push_back(v->RoomNumber);

	ROOM_INFO* room = &g_Level.Rooms[v->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
	{
		roomsList.push_back(room->doors[i].room);
	}

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];
			if (item->Collidable && item->Status != ITEM_INVISIBLE && item != LaraItem && item != v)
			{
				OBJECT_INFO* object = &Objects[item->ObjectNumber];
				if (object->collision && (object->intelligent || item->ObjectNumber == ID_ROLLINGBALL || item->ObjectNumber == ID_ANIMATING2))
				{
					int x = v->Position.xPos - item->Position.xPos;
					int y = v->Position.yPos - item->Position.yPos;
					int z = v->Position.zPos - item->Position.zPos;
					if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
					{
						if (TestBoundsCollide(item, LaraItem, CART_BADDIE_RADIUS))
						{
							if (item->ObjectNumber == ID_ANIMATING2)
							{
								if ((item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase) && (LaraItem->ActiveState == CART_USE) && (LaraItem->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 6))
								{
									FLOOR_INFO* floor;
									short frame, roomNumber;

									frame = LaraItem->FrameNumber - g_Level.Anims[LaraItem->AnimNumber].frameBase;

									if ((frame >= 12) && (frame <= 22))
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
								DoLotsOfBlood(item->Position.xPos, v->Position.yPos - STEP_SIZE, item->Position.zPos, GetRandomControl() & 3, v->Position.yRot, item->RoomNumber, 3);
								item->HitPoints = 0;
							}
						}
					}
				}
			}
			itemNum = item->NextItem;
		}
	}
}

static void MoveCart(ITEM_INFO* v, ITEM_INFO* l, CART_INFO* cart)
{
	short val;
	if (cart->StopDelay)
		cart->StopDelay--;

	if (((Lara.mineL) && (Lara.mineR) && (!cart->StopDelay)) && (((v->Position.xPos & 0x380) == 512) || ((v->Position.zRot & 0x380) == 512)))
	{
		if (cart->Speed < 0xf000)
		{
			cart->Flags |= CF_STOPPED | CF_CONTROL;
			cart->Speed = v->Velocity = 0;
			return;
		}
		else
			cart->StopDelay = 16;
	}

	if ((Lara.mineL || Lara.mineR) && (!(Lara.mineL && Lara.mineR)) && (!cart->StopDelay) && (!(cart->Flags & (CF_TURNINGL | CF_TURNINGR))))
	{
		short ang;
		unsigned short rot = (((unsigned short)v->Position.yRot) / 16384) | (Lara.mineL * 4);

		switch (rot)
		{
		case 0:
			cart->TurnX = (v->Position.xPos + 4096) & ~1023;
			cart->TurnZ = v->Position.zPos & ~1023;
			break;
		case 1:
			cart->TurnX = v->Position.xPos & ~1023;
			cart->TurnZ = (v->Position.zPos - 4096) | 1023;
			break;
		case 2:
			cart->TurnX = (v->Position.xPos - 4096) | 1023;
			cart->TurnZ = v->Position.zPos | 1023;
			break;
		case 3:
			cart->TurnX = v->Position.xPos | 1023;
			cart->TurnZ = (v->Position.zPos + 4096) & ~1023;
			break;

		case 4:
			cart->TurnX = (v->Position.xPos - 4096) | 1023;
			cart->TurnZ = v->Position.zPos & ~1023;
			break;
		case 5:
			cart->TurnX = v->Position.xPos & ~1023;
			cart->TurnZ = (v->Position.zPos + 4096) & ~1023;
			break;
		case 6:
			cart->TurnX = (v->Position.xPos + 4096) & ~1023;
			cart->TurnZ = v->Position.zPos | 1023;
			break;
		case 7:
			cart->TurnX = v->Position.xPos | 1023;
			cart->TurnZ = (v->Position.zPos - 4096) | 1023;
			break;
		}

		ang = mGetAngle(v->Position.xPos, v->Position.zPos, cart->TurnX, cart->TurnZ) & 0x3fff;

		if (rot < 4)
		{
			cart->TurnRot = v->Position.yRot;
			cart->TurnLen = ang;
		}
		else
		{
			cart->TurnRot = v->Position.yRot;

			if (ang)
				ang = 16384 - ang;

			cart->TurnLen = ang;
		}

		cart->Flags |= (Lara.mineL) ? CF_TURNINGL : CF_TURNINGR;
	}

	if (cart->Speed < CART_MIN_SPEED)
		cart->Speed = CART_MIN_SPEED;

	cart->Speed += (-cart->Gradient * 4);

	if ((v->Velocity = cart->Speed / 256) < CART_MIN_VEL)
	{
		v->Velocity = CART_MIN_VEL;

		StopSoundEffect(209);

		if (cart->YVel)
			StopSoundEffect(210);
		else
			SoundEffect(210, &v->Position, 2);
	}
	else
	{
		StopSoundEffect(210);

		if (cart->YVel)
			StopSoundEffect(209);
		else
			SoundEffect(209, &v->Position, (2 | 4) + 0x1000000 + (v->Velocity * 32768));
	}

	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
	{
		float x, z;
		unsigned short quad, deg;

		if ((cart->TurnLen += (v->Velocity * 3)) > ANGLE(90))
		{
			if (cart->Flags & CF_TURNINGL)
				v->Position.yRot = cart->TurnRot - 16384;
			else
				v->Position.yRot = cart->TurnRot + 16384;

			cart->Flags &= ~(CF_TURNINGL | CF_TURNINGR);
		}
		else
		{
			if (cart->Flags & CF_TURNINGL)
				v->Position.yRot = cart->TurnRot - cart->TurnLen;
			else
				v->Position.yRot = cart->TurnRot + cart->TurnLen;
		}

		if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
		{
			quad = ((unsigned short)v->Position.yRot) / 16384;
			deg = v->Position.yRot & 16383;

			switch (quad)
			{
			case 0:
				x = -phd_cos(deg);
				z = phd_sin(deg);
				break;
			case 1:
				x = phd_sin(deg);
				z = phd_cos(deg);
				break;
			case 2:
				x = phd_cos(deg);
				z = -phd_sin(deg);
				break;
			default:
				x = -phd_sin(deg);
				z = -phd_cos(deg);
				break;
			}

			if (cart->Flags & CF_TURNINGL)
			{
				x = -x;
				z = -z;
			}

			v->Position.xPos = cart->TurnX + x * 3584;
			v->Position.zPos = cart->TurnZ + z * 3584;
		}
	}
	else
	{
		v->Position.xPos += v->Velocity * phd_sin(v->Position.yRot);
		v->Position.zPos += v->Velocity * phd_cos(v->Position.yRot);
	}

	cart->MidPos = TestMinecartHeight(v, 0, 0);

	if (!cart->YVel)
	{
		cart->FrontPos = TestMinecartHeight(v, 0, 256);
		cart->Gradient = cart->MidPos - cart->FrontPos;
		v->Position.yPos = cart->MidPos;
	}
	else
	{
		if (v->Position.yPos > cart->MidPos)
		{
			if (cart->YVel > 0)
				SoundEffect(202, &v->Position, 2);

			v->Position.yPos = cart->MidPos;
			cart->YVel = 0;
		}
		else
		{
			cart->YVel += CART_GRAVITY;

			if (cart->YVel > MAX_CART_YVEL)
				cart->YVel = MAX_CART_YVEL;

			v->Position.yPos += (cart->YVel / 256);
		}
	}

	v->Position.xRot = cart->Gradient * 32;

	val = v->Position.yRot & 16383;
	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
	{
		if (cart->Flags & CF_TURNINGR)
			v->Position.zRot = -(val * v->Velocity) / 512;
		else
			v->Position.zRot = ((0x4000 - val) * v->Velocity) / 512;
	}
	else
	{
		v->Position.zRot -= (v->Position.zRot / 8);
	}
}

static void DoUserInput(ITEM_INFO* v, ITEM_INFO* l, CART_INFO* cart)
{
	short fh, ch;

	switch (l->ActiveState)
	{
	case CART_MOVE:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;
		else if ((cart->Speed == CART_MIN_VEL) || (cart->Flags & CF_STOPPED))
			l->TargetState = CART_STILL;
		else if (cart->Gradient < CART_FWD_GRAD)
			l->TargetState = CART_FWD;
		else if (cart->Gradient > CART_BACK_GRAD)
			l->TargetState = CART_BACK;
		else if (TrInput & IN_LEFT)
			l->TargetState = CART_LEFT;
		else if (TrInput & IN_RIGHT)
			l->TargetState = CART_RIGHT;
		break;

	case CART_FWD:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;
		else if (cart->Gradient > CART_FWD_GRAD)
			l->TargetState = CART_MOVE;
		break;

	case CART_BACK:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;
		else if (cart->Gradient < CART_BACK_GRAD)
			l->TargetState = CART_MOVE;
		break;

	case CART_LEFT:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;

		if (!(TrInput & IN_LEFT))
			l->TargetState = CART_MOVE;

		break;

	case CART_RIGHT:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;

		if (!(TrInput & IN_RIGHT))
			l->TargetState = CART_MOVE;
		break;

	case CART_STILL:
		if (!(cart->Flags & CF_CONTROL))
		{
			SoundEffect(211, &v->Position, 2);
			cart->Flags |= CF_CONTROL;
			cart->StopDelay = 64;
		}

		if (TrInput & IN_ROLL && (cart->Flags & CF_STOPPED))
		{
			if ((TrInput & IN_LEFT) && (CanGetOut(-1)))
			{
				l->TargetState = CART_GETOUT;
				cart->Flags &= ~CF_RDIR;
			}
			else if ((TrInput & IN_RIGHT) && (CanGetOut(1)))
			{
				l->TargetState = CART_GETOUT;
				cart->Flags |= CF_RDIR;
			}
		}

		if (TrInput & IN_CROUCH)
			l->TargetState = CART_DUCK;
		else if (cart->Speed > CART_MIN_VEL)
			l->TargetState = CART_MOVE;
		break;

	case CART_DUCK:
		if (TrInput & IN_ACTION)
			l->TargetState = CART_USE;
		else if (TrInput & IN_JUMP)
			l->TargetState = CART_BRAKE;
		else if (!(TrInput & IN_CROUCH))
			l->TargetState = CART_STILL;
		break;

	case CART_USE:
		l->TargetState = CART_MOVE;
		break;

	case CART_BRAKING:
		if (TrInput & IN_CROUCH)
		{
			l->TargetState = CART_DUCK;
			StopSoundEffect(219);
		}
		else if ((!(TrInput & IN_JUMP)) || (cart->Flags & CF_STOPPED))
		{
			l->TargetState = CART_MOVE;
			StopSoundEffect(219);
		}
		else
		{
			cart->Speed += CART_DEC;
			SoundEffect(219, &l->Position, 2);
		}
		break;

	case CART_BRAKE:
		l->TargetState = CART_BRAKING;
		break;

	case CART_GETOUT:
		if (l->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 7)
		{
			if ((l->FrameNumber == GetFrameNumber(v, 20)) && (cart->Flags & CF_MESH))
			{
				Lara.meshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;

				cart->Flags &= ~CF_MESH;
			}

			if (cart->Flags & CF_RDIR)
				l->TargetState = CART_GETOUTR;
			else
				l->TargetState = CART_GETOUTL;
		}
		break;

	case CART_GETOUTL:
		if ((l->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 1) && (l->FrameNumber == g_Level.Anims[l->AnimNumber].frameEnd))
		{
			PHD_VECTOR vec = { 0, 640, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);
			l->Position.xPos = vec.x;
			l->Position.yPos = vec.y;
			l->Position.zPos = vec.z;
			l->Position.xRot = 0;
			l->Position.yRot = v->Position.yRot + 0x4000;
			l->Position.zRot = 0;

			SetAnimation(l, LA_STAND_SOLID);
			Lara.Vehicle = NO_ITEM;
			Lara.Control.HandStatus = HandStatus::Free;
		}
		break;

	case CART_GETOUTR:
		if ((l->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 47) && (l->FrameNumber == g_Level.Anims[l->AnimNumber].frameEnd))
		{
			PHD_VECTOR vec = { 0, 640, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);
			l->Position.xPos = vec.x;
			l->Position.yPos = vec.y;
			l->Position.zPos = vec.z;
			l->Position.xRot = 0;
			l->Position.yRot = v->Position.yRot - 0x4000;
			l->Position.zRot = 0;

			SetAnimation(l, LA_STAND_SOLID);
			Lara.Vehicle = NO_ITEM;
			Lara.Control.HandStatus = HandStatus::Free;
		}
		break;

	case CART_GETIN:
		if ((l->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 5) && (l->FrameNumber == GetFrameNumber(v, 20)) && (!cart->Flags & CF_MESH))
		{
			MESH tmp = g_Level.Meshes[Lara.meshPtrs[LM_RHAND]];

			Lara.meshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;
			g_Level.Meshes[Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND] = tmp;

			cart->Flags |= CF_MESH;
		}
		break;

	case CART_WALLDEATH:
		Camera.targetElevation = -ANGLE(25);
		Camera.targetDistance = WALL_SIZE * 4;
		break;

	case CART_TURNDEATH:
		Camera.targetElevation = -ANGLE(45);
		Camera.targetDistance = WALL_SIZE * 2;

		fh = GetCollision(v, v->Position.yRot, 512, &ch);

		if ((fh > -STEP_SIZE) && (fh < STEP_SIZE))
		{
			if ((Wibble & 7) == 0)
				SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &v->Position, 2);

			v->Position.xPos += TURN_DEATH_VEL * phd_sin(v->Position.yRot);
			v->Position.zPos += TURN_DEATH_VEL * phd_cos(v->Position.yRot);
		}
		else
		{
			if (l->AnimNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 30)
			{
				cart->Flags |= CF_NOANIM;
				l->HitPoints = -1;
			}
		}
		break;

	case CART_HIT:
		if ((l->HitPoints <= 0) && (l->FrameNumber == GetFrameNumber(v, 34) + 28))
		{
			l->FrameNumber = GetFrameNumber(v, 34) + 28;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_NOANIM);
			cart->Speed = v->Velocity = 0;
		}
		break;
	}

	if ((Lara.Vehicle != NO_ITEM) && (!(cart->Flags & CF_NOANIM)))
	{
		AnimateItem(l);

		v->AnimNumber = Objects[ID_MINECART].animIndex + (l->AnimNumber - Objects[ID_MINECART_LARA_ANIMS].animIndex);
		v->FrameNumber = g_Level.Anims[v->AnimNumber].frameBase + (l->FrameNumber - g_Level.Anims[l->AnimNumber].frameBase);
	}
	if ((l->ActiveState != CART_TURNDEATH) && (l->ActiveState != CART_WALLDEATH) && (l->HitPoints > 0))
	{
		if ((v->Position.zRot > TERMINAL_ANGLE) || (v->Position.zRot < -TERMINAL_ANGLE))
		{
			l->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 31;
			l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
			l->ActiveState = l->TargetState = CART_TURNDEATH;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | CF_STOPPED | CF_DEAD;
			cart->Speed = v->Velocity = 0;
			return;
		}

		fh = GetCollision(v, v->Position.yRot, 512, &ch);
		if (fh < -(STEP_SIZE * 2))
		{
			l->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 23;
			l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
			l->ActiveState = l->TargetState = CART_WALLDEATH;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_STOPPED | CF_DEAD);
			cart->Speed = v->Velocity = 0;
			l->HitPoints = -1;
			return;
		}

		if ((l->ActiveState != CART_DUCK) && (l->ActiveState != CART_HIT))
		{
			COLL_INFO coll;
			coll.Setup.Radius = CART_RADIUS;

			DoObjectCollision(v, &coll);

			if (coll.HitStatic)
			{
				int hits;

				l->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 34;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				l->ActiveState = l->TargetState = CART_HIT;
				DoLotsOfBlood(l->Position.xPos, l->Position.yPos - 768, l->Position.zPos, v->Velocity, v->Position.yRot, l->RoomNumber, 3);

				hits = (CART_NHITS * short((cart->Speed) / 2048));
				if (hits < 20)
					hits = 20;

				l->HitPoints -= hits;
				return;
			}
		}

		if ((fh > 512 + 64) && (!cart->YVel))
			cart->YVel = CART_JUMP_VEL;

		CartToBaddieCollision(v);
	}
}

void InitialiseMineCart(short itemNum)
{
	ITEM_INFO* v;
	CART_INFO* cart;

	v = &g_Level.Items[itemNum];
	v->Data = CART_INFO();
	cart = v->Data;
	cart->Flags = NULL;
	cart->Speed = 0;
	cart->YVel = 0;
	cart->Gradient = 0;
}

void MineCartCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* v;
	CART_INFO* cart;
	int geton;
	short ang;

	if ((l->HitPoints < 0) || (Lara.Vehicle != NO_ITEM))
		return;

	v = &g_Level.Items[itemNum];

	if ((geton = GetInMineCart(v, l, coll)))
	{
		Lara.Vehicle = itemNum;

		if (Lara.Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			CreateFlare(LaraItem, ID_FLARE_ITEM, FALSE);
			UndrawFlareMeshes(l);
			Lara.Flare.ControlLeft = false;
			Lara.Control.WeaponControl.RequestGunType = Lara.Control.WeaponControl.GunType = WEAPON_NONE;
		}

		Lara.Control.HandStatus = HandStatus::Busy;

		ang = short(mGetAngle(v->Position.xPos, v->Position.zPos, l->Position.xPos, l->Position.zPos) - v->Position.yRot);

		if ((ang > -ANGLE(45)) && (ang < ANGLE(135)))
			l->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 46;
		else
			l->AnimNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 0;

		l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
		l->ActiveState = l->TargetState = CART_GETIN;

		l->Position.xPos = v->Position.xPos;
		l->Position.yPos = v->Position.yPos;
		l->Position.zPos = v->Position.zPos;
		l->Position.xRot = v->Position.xRot;
		l->Position.yRot = v->Position.yRot;
		l->Position.zRot = v->Position.zRot;
	}
	else
	{
		ObjectCollision(itemNum, l, coll);
	}
}

int MineCartControl(void)
{
	CART_INFO* cart;
	ITEM_INFO* v;
	FLOOR_INFO* floor;
	short roomNumber;

	v = &g_Level.Items[Lara.Vehicle];

	if (!v->Data) 
	{
		TENLog("Minecart data is nullptr!", LogLevel::Error);
		return 0; 
	}
	cart = v->Data;

	DoUserInput(v, LaraItem, cart);

	if (cart->Flags & CF_CONTROL)
		MoveCart(v, LaraItem, cart);

	if (Lara.Vehicle != NO_ITEM)
	{
		LaraItem->Position.xPos = v->Position.xPos;
		LaraItem->Position.yPos = v->Position.yPos;
		LaraItem->Position.zPos = v->Position.zPos;
		LaraItem->Position.xRot = v->Position.xRot;
		LaraItem->Position.yRot = v->Position.yRot;
		LaraItem->Position.zRot = v->Position.zRot;
	}

	roomNumber = v->RoomNumber;
	floor = GetFloor(v->Position.xPos, v->Position.yPos, v->Position.zPos, &roomNumber);

	if (roomNumber != v->RoomNumber)
	{
		ItemNewRoom(Lara.Vehicle, roomNumber);
		ItemNewRoom(Lara.ItemNumber, roomNumber);
	}

	TestTriggers(v, false);

	if (!(cart->Flags & CF_DEAD))
	{
		Camera.targetElevation = -ANGLE(45);
		Camera.targetDistance = WALL_SIZE * 2;
	}

	return (Lara.Vehicle == NO_ITEM) ? 0 : 1;
}
