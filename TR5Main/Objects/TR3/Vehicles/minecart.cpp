#include "framework.h"
#include "minecart.h"
#include "lara.h"
#include "collide.h"
#include "effects/effects.h"
#include "lara_flare.h"
#include "items.h"
#include "sphere.h"
#include "animation.h"
#include "camera.h"
#include "level.h"
#include "setup.h"
#include "input.h"
#include "Sound/sound.h"
#include "minecart_info.h"

using std::vector;

enum MINECART_STATE {
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

	c = phd_cos(v->pos.yRot);
	s = phd_sin(v->pos.yRot);

	pos.x = v->pos.xPos + z * s + x * c;
	pos.y = v->pos.yPos - z * phd_sin(v->pos.xRot) + x * phd_sin(v->pos.zRot);
	pos.z = v->pos.zPos + z * c - x * s;

	roomNumber = v->roomNumber;
	floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
	return GetFloorHeight(floor, pos.x, pos.y, pos.z);
}

static short GetCollision(ITEM_INFO* v, short ang, int dist, short* ceiling)
{
	FLOOR_INFO* floor;
	int x, y, z, height, cheight;
	short roomNumber;

	x = v->pos.xPos + phd_sin(ang) * dist;
	y = v->pos.yPos - LARA_HEIGHT;
	z = v->pos.zPos + phd_cos(ang) * dist;

	roomNumber = v->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	height = GetFloorHeight(floor, x, y, z);
	cheight = GetCeiling(floor, x, y, z);

	*ceiling = ((short)cheight);

	if (height != NO_HEIGHT)
		height -= v->pos.yPos;

	return short(height);
}

static bool GetInMineCart(ITEM_INFO* v, ITEM_INFO* l, COLL_INFO* coll)
{
	int dist;
	int x, z;
	FLOOR_INFO* floor;
	short roomNumber;

	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS || l->gravityStatus)
		return 0;

	if (!TestBoundsCollide(v, l, coll->Setup.Radius))
		return false;

	if (!TestCollision(v, l))
		return false;

	x = l->pos.xPos - v->pos.xPos;
	z = l->pos.zPos - v->pos.zPos;

	dist = SQUARE(x) + SQUARE(z);

	if (dist > SQUARE(WALL_SIZE/2))
		return false;

	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos) < -32000)
		return false;

	return true;
}

static bool CanGetOut(int direction)
{
	auto v = &g_Level.Items[Lara.Vehicle];

	short angle;
	if (direction < 0)
		angle = v->pos.yRot + 0x4000;
	else
		angle = v->pos.yRot - 0x4000;

	int x = v->pos.xPos - GETOFF_DIST * phd_sin(angle);
	int y = v->pos.yPos;
	int z = v->pos.zPos - GETOFF_DIST * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, v->roomNumber);

	if (collResult.Position.Slope || collResult.Position.Floor == NO_HEIGHT)
		return false;

	if (abs(collResult.Position.Floor - v->pos.yPos) > WALL_SIZE / 2)
		return false;

	if ((collResult.Position.Ceiling - v->pos.yPos > -LARA_HEIGHT) || (collResult.Position.Floor - collResult.Position.Ceiling < LARA_HEIGHT))
		return false;

	return true;
}

static void CartToBaddieCollision(ITEM_INFO* v)
{
	vector<short> roomsList;
	short* door, numDoors;

	roomsList.push_back(v->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[v->roomNumber];
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
			if (item->collidable && item->status != ITEM_INVISIBLE && item != LaraItem && item != v)
			{
				OBJECT_INFO* object = &Objects[item->objectNumber];
				if (object->collision && (object->intelligent || item->objectNumber == ID_ROLLINGBALL || item->objectNumber == ID_ANIMATING2))
				{
					int x = v->pos.xPos - item->pos.xPos;
					int y = v->pos.yPos - item->pos.yPos;
					int z = v->pos.zPos - item->pos.zPos;
					if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
					{
						if (TestBoundsCollide(item, LaraItem, CART_BADDIE_RADIUS))
						{
							if (item->objectNumber == ID_ANIMATING2)
							{
								if ((item->frameNumber == g_Level.Anims[item->animNumber].frameBase) && (LaraItem->currentAnimState == CART_USE) && (LaraItem->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 6))
								{
									FLOOR_INFO* floor;
									short frame, roomNumber;

									frame = LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase;

									if ((frame >= 12) && (frame <= 22))
									{
										SoundEffect(220, &item->pos, 2);
										TestTriggers(item, true);
										item->frameNumber++;
									}
								}
							}
							else if (item->objectNumber == ID_ROLLINGBALL)
							{
								/*code, kill lara and stop both the boulder and the minecart*/
							}
							else
							{
								DoLotsOfBlood(item->pos.xPos, v->pos.yPos - STEP_SIZE, item->pos.zPos, GetRandomControl() & 3, v->pos.yRot, item->roomNumber, 3);
								item->hitPoints = 0;
							}
						}
					}
				}
			}
			itemNum = item->nextItem;
		}
	}
}

static void MoveCart(ITEM_INFO* v, ITEM_INFO* l, CART_INFO* cart)
{
	short val;
	if (cart->StopDelay)
		cart->StopDelay--;

	if (((Lara.mineL) && (Lara.mineR) && (!cart->StopDelay)) && (((v->pos.xPos & 0x380) == 512) || ((v->pos.zRot & 0x380) == 512)))
	{
		if (cart->Speed < 0xf000)
		{
			cart->Flags |= CF_STOPPED | CF_CONTROL;
			cart->Speed = v->speed = 0;
			return;
		}
		else
			cart->StopDelay = 16;
	}

	if ((Lara.mineL || Lara.mineR) && (!(Lara.mineL && Lara.mineR)) && (!cart->StopDelay) && (!(cart->Flags & (CF_TURNINGL | CF_TURNINGR))))
	{
		short ang;
		unsigned short rot = (((unsigned short)v->pos.yRot) / 16384) | (Lara.mineL * 4);

		switch (rot)
		{
		case 0:
			cart->TurnX = (v->pos.xPos + 4096) & ~1023;
			cart->TurnZ = v->pos.zPos & ~1023;
			break;
		case 1:
			cart->TurnX = v->pos.xPos & ~1023;
			cart->TurnZ = (v->pos.zPos - 4096) | 1023;
			break;
		case 2:
			cart->TurnX = (v->pos.xPos - 4096) | 1023;
			cart->TurnZ = v->pos.zPos | 1023;
			break;
		case 3:
			cart->TurnX = v->pos.xPos | 1023;
			cart->TurnZ = (v->pos.zPos + 4096) & ~1023;
			break;

		case 4:
			cart->TurnX = (v->pos.xPos - 4096) | 1023;
			cart->TurnZ = v->pos.zPos & ~1023;
			break;
		case 5:
			cart->TurnX = v->pos.xPos & ~1023;
			cart->TurnZ = (v->pos.zPos + 4096) & ~1023;
			break;
		case 6:
			cart->TurnX = (v->pos.xPos + 4096) & ~1023;
			cart->TurnZ = v->pos.zPos | 1023;
			break;
		case 7:
			cart->TurnX = v->pos.xPos | 1023;
			cart->TurnZ = (v->pos.zPos - 4096) | 1023;
			break;
		}

		ang = mGetAngle(v->pos.xPos, v->pos.zPos, cart->TurnX, cart->TurnZ) & 0x3fff;

		if (rot < 4)
		{
			cart->TurnRot = v->pos.yRot;
			cart->TurnLen = ang;
		}
		else
		{
			cart->TurnRot = v->pos.yRot;

			if (ang)
				ang = 16384 - ang;

			cart->TurnLen = ang;
		}

		cart->Flags |= (Lara.mineL) ? CF_TURNINGL : CF_TURNINGR;
	}

	if (cart->Speed < CART_MIN_SPEED)
		cart->Speed = CART_MIN_SPEED;

	cart->Speed += (-cart->Gradient * 4);

	if ((v->speed = cart->Speed / 256) < CART_MIN_VEL)
	{
		v->speed = CART_MIN_VEL;

		StopSoundEffect(209);

		if (cart->YVel)
			StopSoundEffect(210);
		else
			SoundEffect(210, &v->pos, 2);
	}
	else
	{
		StopSoundEffect(210);

		if (cart->YVel)
			StopSoundEffect(209);
		else
			SoundEffect(209, &v->pos, (2 | 4) + 0x1000000 + (v->speed * 32768));
	}

	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
	{
		float x, z;
		unsigned short quad, deg;

		if ((cart->TurnLen += (v->speed * 3)) > ANGLE(90))
		{
			if (cart->Flags & CF_TURNINGL)
				v->pos.yRot = cart->TurnRot - 16384;
			else
				v->pos.yRot = cart->TurnRot + 16384;

			cart->Flags &= ~(CF_TURNINGL | CF_TURNINGR);
		}
		else
		{
			if (cart->Flags & CF_TURNINGL)
				v->pos.yRot = cart->TurnRot - cart->TurnLen;
			else
				v->pos.yRot = cart->TurnRot + cart->TurnLen;
		}

		if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
		{
			quad = ((unsigned short)v->pos.yRot) / 16384;
			deg = v->pos.yRot & 16383;

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

			v->pos.xPos = cart->TurnX + x * 3584;
			v->pos.zPos = cart->TurnZ + z * 3584;
		}
	}
	else
	{
		v->pos.xPos += v->speed * phd_sin(v->pos.yRot);
		v->pos.zPos += v->speed * phd_cos(v->pos.yRot);
	}

	cart->MidPos = TestMinecartHeight(v, 0, 0);

	if (!cart->YVel)
	{
		cart->FrontPos = TestMinecartHeight(v, 0, 256);
		cart->Gradient = cart->MidPos - cart->FrontPos;
		v->pos.yPos = cart->MidPos;
	}
	else
	{
		if (v->pos.yPos > cart->MidPos)
		{
			if (cart->YVel > 0)
				SoundEffect(202, &v->pos, 2);

			v->pos.yPos = cart->MidPos;
			cart->YVel = 0;
		}
		else
		{
			cart->YVel += CART_GRAVITY;

			if (cart->YVel > MAX_CART_YVEL)
				cart->YVel = MAX_CART_YVEL;

			v->pos.yPos += (cart->YVel / 256);
		}
	}

	v->pos.xRot = cart->Gradient * 32;

	val = v->pos.yRot & 16383;
	if (cart->Flags & (CF_TURNINGL | CF_TURNINGR))
	{
		if (cart->Flags & CF_TURNINGR)
			v->pos.zRot = -(val * v->speed) / 512;
		else
			v->pos.zRot = ((0x4000 - val) * v->speed) / 512;
	}
	else
	{
		v->pos.zRot -= (v->pos.zRot / 8);
	}
}

static void DoUserInput(ITEM_INFO* v, ITEM_INFO* l, CART_INFO* cart)
{
	short fh, ch;

	switch (l->currentAnimState)
	{
	case CART_MOVE:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;
		else if ((cart->Speed == CART_MIN_VEL) || (cart->Flags & CF_STOPPED))
			l->goalAnimState = CART_STILL;
		else if (cart->Gradient < CART_FWD_GRAD)
			l->goalAnimState = CART_FWD;
		else if (cart->Gradient > CART_BACK_GRAD)
			l->goalAnimState = CART_BACK;
		else if (TrInput & IN_LEFT)
			l->goalAnimState = CART_LEFT;
		else if (TrInput & IN_RIGHT)
			l->goalAnimState = CART_RIGHT;
		break;

	case CART_FWD:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;
		else if (cart->Gradient > CART_FWD_GRAD)
			l->goalAnimState = CART_MOVE;
		break;

	case CART_BACK:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;
		else if (cart->Gradient < CART_BACK_GRAD)
			l->goalAnimState = CART_MOVE;
		break;

	case CART_LEFT:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;

		if (!(TrInput & IN_LEFT))
			l->goalAnimState = CART_MOVE;

		break;

	case CART_RIGHT:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;

		if (!(TrInput & IN_RIGHT))
			l->goalAnimState = CART_MOVE;
		break;

	case CART_STILL:
		if (!(cart->Flags & CF_CONTROL))
		{
			SoundEffect(211, &v->pos, 2);
			cart->Flags |= CF_CONTROL;
			cart->StopDelay = 64;
		}

		if (TrInput & IN_ROLL && (cart->Flags & CF_STOPPED))
		{
			if ((TrInput & IN_LEFT) && (CanGetOut(-1)))
			{
				l->goalAnimState = CART_GETOUT;
				cart->Flags &= ~CF_RDIR;
			}
			else if ((TrInput & IN_RIGHT) && (CanGetOut(1)))
			{
				l->goalAnimState = CART_GETOUT;
				cart->Flags |= CF_RDIR;
			}
		}

		if (TrInput & IN_DUCK)
			l->goalAnimState = CART_DUCK;
		else if (cart->Speed > CART_MIN_VEL)
			l->goalAnimState = CART_MOVE;
		break;

	case CART_DUCK:
		if (TrInput & IN_ACTION)
			l->goalAnimState = CART_USE;
		else if (TrInput & IN_JUMP)
			l->goalAnimState = CART_BRAKE;
		else if (!(TrInput & IN_DUCK))
			l->goalAnimState = CART_STILL;
		break;

	case CART_USE:
		l->goalAnimState = CART_MOVE;
		break;

	case CART_BRAKING:
		if (TrInput & IN_DUCK)
		{
			l->goalAnimState = CART_DUCK;
			StopSoundEffect(219);
		}
		else if ((!(TrInput & IN_JUMP)) || (cart->Flags & CF_STOPPED))
		{
			l->goalAnimState = CART_MOVE;
			StopSoundEffect(219);
		}
		else
		{
			cart->Speed += CART_DEC;
			SoundEffect(219, &l->pos, 2);
		}
		break;

	case CART_BRAKE:
		l->goalAnimState = CART_BRAKING;
		break;

	case CART_GETOUT:
		if (l->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 7)
		{
			if ((l->frameNumber == GetFrameNumber(v, 20)) && (cart->Flags & CF_MESH))
			{
				Lara.meshPtrs[LM_RHAND] = Objects[ID_MINECART_LARA_ANIMS].meshIndex + LM_RHAND;

				cart->Flags &= ~CF_MESH;
			}

			if (cart->Flags & CF_RDIR)
				l->goalAnimState = CART_GETOUTR;
			else
				l->goalAnimState = CART_GETOUTL;
		}
		break;

	case CART_GETOUTL:
		if ((l->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 1) && (l->frameNumber == g_Level.Anims[l->animNumber].frameEnd))
		{
			PHD_VECTOR vec = { 0, 640, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);
			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->pos.xRot = 0;
			l->pos.yRot = v->pos.yRot + 0x4000;
			l->pos.zRot = 0;

			SetAnimation(l, LA_STAND_SOLID);
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
		}
		break;

	case CART_GETOUTR:
		if ((l->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 47) && (l->frameNumber == g_Level.Anims[l->animNumber].frameEnd))
		{
			PHD_VECTOR vec = { 0, 640, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);
			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->pos.xRot = 0;
			l->pos.yRot = v->pos.yRot - 0x4000;
			l->pos.zRot = 0;

			SetAnimation(l, LA_STAND_SOLID);
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
		}
		break;

	case CART_GETIN:
		if ((l->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 5) && (l->frameNumber == GetFrameNumber(v, 20)) && (!cart->Flags & CF_MESH))
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

		fh = GetCollision(v, v->pos.yRot, 512, &ch);

		if ((fh > -STEP_SIZE) && (fh < STEP_SIZE))
		{
			if ((Wibble & 7) == 0)
				SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &v->pos, 2);

			v->pos.xPos += TURN_DEATH_VEL * phd_sin(v->pos.yRot);
			v->pos.zPos += TURN_DEATH_VEL * phd_cos(v->pos.yRot);
		}
		else
		{
			if (l->animNumber == Objects[ID_MINECART_LARA_ANIMS].animIndex + 30)
			{
				cart->Flags |= CF_NOANIM;
				l->hitPoints = -1;
			}
		}
		break;

	case CART_HIT:
		if ((l->hitPoints <= 0) && (l->frameNumber == GetFrameNumber(v, 34) + 28))
		{
			l->frameNumber = GetFrameNumber(v, 34) + 28;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_NOANIM);
			cart->Speed = v->speed = 0;
		}
		break;
	}

	if ((Lara.Vehicle != NO_ITEM) && (!(cart->Flags & CF_NOANIM)))
	{
		AnimateItem(l);

		v->animNumber = Objects[ID_MINECART].animIndex + (l->animNumber - Objects[ID_MINECART_LARA_ANIMS].animIndex);
		v->frameNumber = g_Level.Anims[v->animNumber].frameBase + (l->frameNumber - g_Level.Anims[l->animNumber].frameBase);
	}
	if ((l->currentAnimState != CART_TURNDEATH) && (l->currentAnimState != CART_WALLDEATH) && (l->hitPoints > 0))
	{
		if ((v->pos.zRot > TERMINAL_ANGLE) || (v->pos.zRot < -TERMINAL_ANGLE))
		{
			l->animNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 31;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = CART_TURNDEATH;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | CF_STOPPED | CF_DEAD;
			cart->Speed = v->speed = 0;
			return;
		}

		fh = GetCollision(v, v->pos.yRot, 512, &ch);
		if (fh < -(STEP_SIZE * 2))
		{
			l->animNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 23;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = CART_WALLDEATH;
			cart->Flags = (cart->Flags & ~CF_CONTROL) | (CF_STOPPED | CF_DEAD);
			cart->Speed = v->speed = 0;
			l->hitPoints = -1;
			return;
		}

		if ((l->currentAnimState != CART_DUCK) && (l->currentAnimState != CART_HIT))
		{
			COLL_INFO coll;
			coll.Setup.Radius = CART_RADIUS;

			DoObjectCollision(v, &coll);

			if (coll.HitStatic)
			{
				int hits;

				l->animNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 34;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->currentAnimState = l->goalAnimState = CART_HIT;
				DoLotsOfBlood(l->pos.xPos, l->pos.yPos - 768, l->pos.zPos, v->speed, v->pos.yRot, l->roomNumber, 3);

				hits = (CART_NHITS * short((cart->Speed) / 2048));
				if (hits < 20)
					hits = 20;

				l->hitPoints -= hits;
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
	v->data = CART_INFO();
	cart = v->data;
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

	if ((l->hitPoints < 0) || (Lara.Vehicle != NO_ITEM))
		return;

	v = &g_Level.Items[itemNum];

	if ((geton = GetInMineCart(v, l, coll)))
	{
		Lara.Vehicle = itemNum;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(LaraItem, ID_FLARE_ITEM, FALSE);
			UndrawFlareMeshes(l);
			Lara.flareControlLeft = false;
			Lara.requestGunType = Lara.gunType = WEAPON_NONE;
		}

		Lara.gunStatus = LG_HANDS_BUSY;

		ang = short(mGetAngle(v->pos.xPos, v->pos.zPos, l->pos.xPos, l->pos.zPos) - v->pos.yRot);

		if ((ang > -ANGLE(45)) && (ang < ANGLE(135)))
			l->animNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 46;
		else
			l->animNumber = Objects[ID_MINECART_LARA_ANIMS].animIndex + 0;

		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		l->currentAnimState = l->goalAnimState = CART_GETIN;

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot;
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

	if (!v->data) 
	{
		TENLog("Minecart data is nullptr!", LogLevel::Error);
		return 0; 
	}
	cart = v->data;

	DoUserInput(v, LaraItem, cart);

	if (cart->Flags & CF_CONTROL)
		MoveCart(v, LaraItem, cart);

	if (Lara.Vehicle != NO_ITEM)
	{
		LaraItem->pos.xPos = v->pos.xPos;
		LaraItem->pos.yPos = v->pos.yPos;
		LaraItem->pos.zPos = v->pos.zPos;
		LaraItem->pos.xRot = v->pos.xRot;
		LaraItem->pos.yRot = v->pos.yRot;
		LaraItem->pos.zRot = v->pos.zRot;
	}

	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);

	if (roomNumber != v->roomNumber)
	{
		ItemNewRoom(Lara.Vehicle, roomNumber);
		ItemNewRoom(Lara.itemNumber, roomNumber);
	}

	TestTriggers(v, false);

	if (!(cart->Flags & CF_DEAD))
	{
		Camera.targetElevation = -ANGLE(45);
		Camera.targetDistance = WALL_SIZE * 2;
	}

	return (Lara.Vehicle == NO_ITEM) ? 0 : 1;
}
