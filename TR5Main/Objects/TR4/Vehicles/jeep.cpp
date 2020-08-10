#include "framework.h"
#include "jeep.h"
#include "lara.h"
#include "inventory.h"
#include "effect2.h"
#include "collide.h"
#include "effect.h"
#include "lara_one_gun.h"
#include "items.h"
#include "camera.h"
#include "tomb4fx.h"
#include "sphere.h"
#include "lara_flare.h"
#include "input.h"
#include "sound.h"
#include "setup.h"
#include "level.h"
using std::vector;
typedef struct JEEP_INFO
{
	short rot1;
	short rot2;
	short rot3;
	short rot4;
	int velocity;
	int revs;
	short engineRevs;
	short trackMesh;
	int jeepTurn;
	int fallSpeed;
	short momentumAngle;
	short extraRotation;
	short unknown0;
	int pitch;
	short flags;
	short unknown1;
	short unknown2;
};

#define JF_FALLING					0x40
#define JF_DEAD						0x80

#define JEEP_GETOFF_DISTANCE		512
#define JEEP_UNDO_TURN				91
#define JEEP_SLIP					100
#define JEEP_SLIP_SIDE				128
#define JEEP_MAX_SPEED				0x8000
#define JEEP_MAX_BACK				0x4000

#define JEEP_LS_STOP		0

#define JEEP_LS_GETON		9
#define JEEP_LS_GETOFF		10
#define JEEP_LS_JEEPDEATH	16

#define JEEP_IN_ACCELERATE	(IN_ACTION)
#define JEEP_IN_BRAKE		(IN_JUMP)
#define JEEP_IN_DISMOUNT	(IN_JUMP|IN_LEFT)
#define JEEP_IN_HANDBRAKE	(IN_SPRINT|IN_DUCK)

//bool QuadHandbrakeStarting;
//bool QuadCanHandbrakeStart;
char JeepSmokeStart;
bool JeepNoGetOff;
short Unk_0080DE1A;
int Unk_0080DDE8;
short Unk_0080DE24;

extern Inventory g_Inventory;

static int TestJeepHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - (dz * phd_sin(item->pos.xRot) >> W2V_SHIFT) + (dx * phd_sin(item->pos.zRot) >> W2V_SHIFT);

	int c = phd_cos(item->pos.yRot);
	int s = phd_sin(item->pos.yRot);

	pos->z = item->pos.zPos + ((dz * c - dx * s) >> W2V_SHIFT);
	pos->x = item->pos.xPos + ((dz * s + dx * c) >> W2V_SHIFT);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;

	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static int DoJeepShift(ITEM_INFO* jeep, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x >> WALL_SHIFT;
	int z = pos->z >> WALL_SHIFT;
	int  oldX = old->x >> WALL_SHIFT;
	int oldZ = old->z >> WALL_SHIFT;
	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			jeep->pos.zPos += (old->z - pos->z);
			jeep->pos.xPos += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			jeep->pos.zPos -= shiftZ + 1;
			return (pos->x - jeep->pos.xPos);
		}
		else
		{
			jeep->pos.zPos += WALL_SIZE - shiftZ;
			return (jeep->pos.xPos - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			jeep->pos.xPos -= shiftX + 1;
			return (jeep->pos.zPos - pos->z);
		}
		else
		{
			jeep->pos.xPos += WALL_SIZE - shiftX;
			return (pos->z - jeep->pos.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		short roomNumber = jeep->roomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = jeep->roomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			jeep->pos.zPos += z;
			jeep->pos.xPos += x;
		}
		else if (z)
		{
			jeep->pos.zPos += z;
			if (z > 0)
				return (jeep->pos.xPos - pos->x);
			else
				return (pos->x - jeep->pos.xPos);
		}
		else if (x)
		{
			jeep->pos.xPos += x;
			if (x > 0)
				return (pos->z - jeep->pos.zPos);
			else
				return (jeep->pos.zPos - pos->z);
		}
		else
		{
			jeep->pos.zPos += (old->z - pos->z);
			jeep->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

static int DoJeepDynamics(int height, int speed, int* y, int flags)
{
	int result = 0;

	if (height <= *y)
	{
		if (flags)
		{
			result = speed;
		}
		else
		{
			int temp = height - *y;
			if (temp < -80)
			{
				temp = -80;
			}
			result = ((temp - speed) >> 4) + speed;
			if (*y > height)
			{
				*y = height;
			}
		}
	}
	else
	{
		*y += speed;
		if (*y <= height - 32)
		{
			if (flags)
			{
				result = flags + (flags >> 1) + speed;
			}
			else
			{
				result = speed + GRAVITY;
			}
		}
		else
		{
			*y = height;
			if (speed > 150)
			{
				LaraItem->hitPoints += 150 - speed;
			}
			result = 0;
		}
	}

	return result;
}

static int JeepCanGetOff()
{
	ITEM_INFO* item = &g_Level.Items[Lara.Vehicle];

	short angle = item->pos.yRot + 0x4000;

	int x = item->pos.xPos - (JEEP_GETOFF_DISTANCE * phd_sin(angle) >> W2V_SHIFT);
	int y = item->pos.yPos;
	int z = item->pos.zPos - (JEEP_GETOFF_DISTANCE * phd_cos(angle) >> W2V_SHIFT);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);

	if ((HeightType == BIG_SLOPE)
		|| (HeightType == DIAGONAL)
		|| (height == NO_HEIGHT))
		return 0;

	if (abs(height - item->pos.yPos) > WALL_SIZE / 2)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	if ((ceiling - item->pos.yPos > -LARA_HITE)
		|| (height - ceiling < LARA_HITE))
		return 0;

	return 1;
}

static void TriggerJeepExhaustSmoke(int x, int y, int z, short angle, short speed, int moving)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->dR = 16;
	spark->dG = 16;
	spark->on = 1;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;
	spark->dB = 32;

	if (moving)
	{
		spark->dR = (spark->dR * speed) >> 5;
		spark->dG = (spark->dG * speed) >> 5;
		spark->dB = (spark->dB * speed) >> 5;
	}

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->life = spark->sLife = (GetRandomControl() & 3) - (speed >> 12) + 20;;

	if (spark->life < 9)
	{
		spark->life = 9;
		spark->sLife = 9;
	}

	spark->transType = COLADD;
	spark->x = (GetRandomControl() & 0xF) + x - 8;
	spark->y = (GetRandomControl() & 0xF) + y - 8;
	spark->z = (GetRandomControl() & 0xF) + z - 8;
	spark->xVel = speed * phd_sin(angle) >> (W2V_SHIFT + 2);
	spark->yVel = -8 - (GetRandomControl() & 7);
	spark->zVel = speed * phd_cos(angle) >> (W2V_SHIFT + 2);
	spark->friction = 4;

	if (GetRandomControl() & 1)
	{
		spark->flags = 538;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
		{
			spark->rotAdd = -24 - (GetRandomControl() & 7);
		}
		else
		{
			spark->rotAdd = (GetRandomControl() & 7) + 24;
		}
	}
	else
	{
		spark->flags = 522;
	}

	spark->scalar = 1;
	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->maxYvel = -8 - (GetRandomControl() & 7);
	spark->dSize = (GetRandomControl() & 7) + (speed >> 7) + 32;
	spark->sSize = spark->dSize >> 1;
	spark->size = spark->dSize >> 1;
}

void InitialiseJeep(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	JEEP_INFO* jeep = game_malloc<JEEP_INFO>();
	item->data = jeep;

	jeep->velocity = 0;
	jeep->revs = 0;
	jeep->jeepTurn = 0;
	jeep->fallSpeed = 0;
	jeep->extraRotation = 0;
	jeep->momentumAngle = item->pos.yPos;
	jeep->pitch = 0;
	jeep->flags = 0;
	jeep->unknown2 = 0;
	jeep->rot1 = 0;
	jeep->rot2 = 0;
	jeep->rot3 = 0;
	jeep->rot4 = 0;

	item->meshBits = 114687;
}

static int JeepCheckGetOff()
{
	if (LaraItem->currentAnimState == 10)
	{
		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
		{
			LaraItem->pos.yRot += ANGLE(90);
			LaraItem->animNumber = LA_STAND_SOLID;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = LS_STOP;
			LaraItem->currentAnimState = LS_STOP;
			LaraItem->pos.xPos -= JEEP_GETOFF_DISTANCE * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT;
			LaraItem->pos.zPos -= JEEP_GETOFF_DISTANCE * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT;
			LaraItem->pos.xRot = 0;
			LaraItem->pos.zRot = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
			CurrentAtmosphere = 110;
			IsAtmospherePlaying = true;
			S_CDPlay(110, 1);

			return false;
		}
	}

	return true;
}

static int GetOnJeep(int itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!(TrInput & IN_ACTION) && g_Inventory.GetSelectedObject() != ID_PUZZLE_ITEM1)
	{
		return 0;
	}

	if (item->flags & 0x100)
	{
		return 0;
	}

	if (Lara.gunStatus)
	{
		return 0;
	}

	if (LaraItem->currentAnimState != LS_STOP)
	{
		return 0;
	}

	if (LaraItem->animNumber != LA_STAND_IDLE)
	{
		return 0;
	}

	if (LaraItem->gravityStatus)
	{
		return 0;
	}

	if (abs(item->pos.yPos - LaraItem->pos.yPos) >= 256)
	{
		return 0;
	}

	if (!TestBoundsCollide(item, LaraItem, 100))
	{
		return 0;
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < -32000)
		return 0;

	short angle = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
	angle -= item->pos.yRot;

	if ((angle > -ANGLE(45)) && (angle < ANGLE(135)))
	{
		int tempAngle = LaraItem->pos.yRot - item->pos.yRot;
		if (tempAngle > ANGLE(45) && tempAngle < ANGLE(135))
		{
			if (g_Inventory.GetSelectedObject() == ID_PUZZLE_ITEM1)
			{
				g_Inventory.SetSelectedObject(NO_ITEM);
				return 1;
			}
			else
			{
				if (g_Inventory.IsObjectPresentInInventory(ID_PUZZLE_ITEM1))
					g_Inventory.SetEnterObject(ID_PUZZLE_ITEM1);
				return 0;
			}
		}
		else
			return 0;
	}
	else
	{
		int tempAngle = LaraItem->pos.yRot - item->pos.yRot;
		if (tempAngle > ANGLE(225) && tempAngle < ANGLE(315))
		{
			if (g_Inventory.GetSelectedObject() == ID_PUZZLE_ITEM1)
			{
				g_Inventory.SetSelectedObject(NO_ITEM);
				return 1;
			}
			else
			{
				if (g_Inventory.IsObjectPresentInInventory(ID_PUZZLE_ITEM1))
					g_Inventory.SetEnterObject(ID_PUZZLE_ITEM1);
				return 0;
			}
		}
		else
			return 0;
	}

	return 0;
}

static int GetJeepCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p)
{
	p->x = item->pos.xPos - p->x;
	p->z = item->pos.zPos - p->z;

	if (p->x || p->z)
	{
		int c = phd_cos(item->pos.yRot);
		int s = phd_sin(item->pos.yRot);
		int front = ((p->z * c) + (p->x * s)) >> W2V_SHIFT;
		int side = (-(p->z * s) + (p->x * c)) >> W2V_SHIFT;

		if (abs(front) > abs(side))
		{
			return (front > 0) + 13;
		}
		else
		{
			return (side <= 0) + 11;
		}
	}

	return 0;
}

static void JeepBaddieCollision(ITEM_INFO* jeep)
{
	vector<short> roomsList;
	short* door, numDoors;

	roomsList.push_back(jeep->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[jeep->roomNumber];
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
			if (item->collidable && item->status != ITEM_INVISIBLE && item != LaraItem && item != jeep)
			{
				if (item->objectNumber == ID_ENEMY_JEEP)
				{
					Unk_0080DE1A = 0;
					Unk_0080DDE8 = 400;
					Unk_0080DE24 = Unk_0080DE24 & 0xFFDF | 0x10;

					//ObjectCollision(item, jeep, )
				}
				else
				{
					OBJECT_INFO* object = &Objects[item->objectNumber];
					if (object->collision && object->intelligent ||
						item->objectNumber == ID_ROLLINGBALL)
					{
						int x = jeep->pos.xPos - item->pos.xPos;
						int y = jeep->pos.yPos - item->pos.yPos;
						int z = jeep->pos.zPos - item->pos.zPos;
						if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
						{
							if (item->objectNumber == ID_ROLLINGBALL)
							{
								if (TestBoundsCollide(item, LaraItem, 100))
								{
									if (LaraItem->hitPoints > 0)
									{
										DoLotsOfBlood(LaraItem->pos.xPos,
											LaraItem->pos.yPos - 512,
											LaraItem->pos.zPos,
											GetRandomControl() & 3,
											LaraItem->pos.yRot,
											LaraItem->roomNumber,
											5);
										item->hitPoints -= 8;
									}
								}
							}
							else
							{
								if (TestBoundsCollide(item, jeep, 550))
								{
									DoLotsOfBlood(item->pos.xPos,
										jeep->pos.yPos - STEP_SIZE,
										item->pos.zPos,
										GetRandomControl() & 3,
										jeep->pos.yRot,
										item->roomNumber,
										3);
									item->hitPoints = 0;
								}
							}
						}
					}
				}
			}
			itemNum = item->nextItem;
		}
	}
}

static void JeepExplode(ITEM_INFO* item)
{
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(item);
	}
	else
	{
		TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
		for (int i = 0; i < 3; i++)
		{
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);
		}
	}

	ExplodingDeath(Lara.Vehicle, -1, 256);
	KillItem(Lara.Vehicle);
	item->status = ITEM_DEACTIVATED;
	SoundEffect(SFX_EXPLOSION1, 0, 0);
	SoundEffect(SFX_EXPLOSION2, 0, 0);
	Lara.Vehicle = NO_ITEM;
}

static int JeepDynamics(ITEM_INFO* item)
{
	JEEP_INFO* jeep = (JEEP_INFO*)item->data;

	PHD_VECTOR f_old, b_old, mm_old, mt_old, mb_old;

	int hf_old = TestJeepHeight(item, 550, -256, &f_old);
	int hb_old = TestJeepHeight(item, 550, 256,&b_old);
	int hmm_old = TestJeepHeight(item, -600, -256, &mm_old);
	int hmt_old = TestJeepHeight(item, -600, 256, &mt_old);
	int hmb_old = TestJeepHeight(item, -600, 0, (PHD_VECTOR*)&mb_old);

	PHD_VECTOR oldPos;
	oldPos.x = item->pos.xPos;
	oldPos.y = item->pos.yPos;
	oldPos.z = item->pos.zPos;

	if (mm_old.y > hmm_old)
		mm_old.y = hmm_old;
	if (mt_old.y > hmt_old)
		mt_old.y = hmt_old;
	if (f_old.y > hf_old)
		f_old.y = hf_old;
	if (b_old.y > hb_old)
		b_old.y = hb_old;
	if (mb_old.y > hmb_old)
		mb_old.y = hmb_old;

	short rot = 0;

	if (oldPos.y <= item->floor - 8 )
	{
		if (jeep->jeepTurn < -91)
			jeep->jeepTurn += 91;
		else if (jeep->jeepTurn > 91)
			jeep->jeepTurn -= 91;
		else
			jeep->jeepTurn = 0;

		item->pos.yRot += jeep->jeepTurn + jeep->extraRotation;
		jeep->momentumAngle += (item->pos.yRot - jeep->momentumAngle) >> 5;
	}
	else
	{
		short rot2 = 0;
		short momentum = 0;

		if (jeep->jeepTurn < -ANGLE(1))
			jeep->jeepTurn += ANGLE(1);
		else if (jeep->jeepTurn > ANGLE(1))
			jeep->jeepTurn -= ANGLE(1);
		else
			jeep->jeepTurn = 0;

		item->pos.yRot += jeep->jeepTurn + jeep->extraRotation;

		rot = item->pos.yRot - jeep->momentumAngle;
		momentum = 728 - (3 * jeep->velocity >> 11);

		if (!(TrInput & IN_ACTION) && jeep->velocity > 0)
			momentum -= momentum >> 2;

		if (rot >= -273)
		{
			if (rot <= 273)
				jeep->momentumAngle = item->pos.yRot;
			else
			{
				if (rot > 13650)
				{
					item->pos.yPos -= 41;
					item->fallspeed = -6 - (GetRandomControl() & 3);
					jeep->jeepTurn = 0;
					jeep->velocity -= jeep->velocity >> 3;
				}

				if (rot <= 16380)
					jeep->momentumAngle += momentum;
				else
					jeep->momentumAngle = item->pos.yRot - 16380;
			}
		}
		else
		{
			if (rot < -13650)
			{
				item->pos.yPos -= 41;
				item->fallspeed = -6 - (GetRandomControl() & 3);
				jeep->jeepTurn = 0;
				jeep->velocity -= jeep->velocity >> 3;
			}

			if (rot >= -16380)
				jeep->momentumAngle -= momentum;
			else
				jeep->momentumAngle = item->pos.yRot + 16380;
		}
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	short speed;
	if (item->pos.yPos < height)
		speed = item->speed;
	else
		speed = item->speed * phd_cos(item->pos.xRot) >> W2V_SHIFT;

	item->pos.xPos += (speed * phd_sin(jeep->momentumAngle)) >> W2V_SHIFT;
	item->pos.zPos += (speed * phd_cos(jeep->momentumAngle)) >> W2V_SHIFT;
	
	int slip = 0;
	if (item->pos.yPos >= height)
	{
		slip = JEEP_SLIP * phd_sin(item->pos.xRot) >> W2V_SHIFT;

		if (abs(slip) > 16)
		{
			JeepNoGetOff = 1;
			if (slip >= 0)
				slip = jeep->velocity - (SQUARE(slip - 16) >> 1);
			else
				slip = (SQUARE(slip + 16) >> 1) + jeep->velocity;
			jeep->velocity = slip;
		}

		slip = JEEP_SLIP_SIDE * phd_sin(item->pos.zRot) >> W2V_SHIFT;
		if (abs(slip) > JEEP_SLIP_SIDE / 4)
		{
			JeepNoGetOff = 1;

			if (slip >= 0)
			{
				item->pos.xPos += (slip - 24) * phd_sin(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
				item->pos.zPos += (slip - 24) * phd_cos(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
			}
			else
			{
				item->pos.xPos += (slip - 24) * phd_sin(item->pos.yRot - ANGLE(90)) >> W2V_SHIFT;
				item->pos.zPos += (slip - 24) * phd_cos(item->pos.yRot - ANGLE(90)) >> W2V_SHIFT;
			}
		}
	}

	if (jeep->velocity > JEEP_MAX_SPEED)
		jeep->velocity = JEEP_MAX_SPEED;
	else if (jeep->velocity < -JEEP_MAX_BACK)
		jeep->velocity = -JEEP_MAX_BACK;

	PHD_VECTOR movedPos;
	movedPos.x = item->pos.xPos;
	movedPos.z = item->pos.zPos;

	if (!(item->flags & ONESHOT))
	{
		JeepBaddieCollision(item);
		// v37 = sub_467850(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 512);
	}

	PHD_VECTOR f, b, mm, mt, mb;
	
	int rot1 = 0;
	int rot2 = 0;

	int hf = TestJeepHeight(item, 550, -256, (PHD_VECTOR*)&f);
	if (hf < f_old.y - STEP_SIZE)
		rot1 = abs(4 * DoJeepShift(item, &f, &f_old));

	int hmm = TestJeepHeight(item, -600, -256, (PHD_VECTOR*)&mm);
	if (hmm < mm_old.y - STEP_SIZE)
	{
		if (rot)
			rot1 += abs(4 * DoJeepShift(item, &mm, &mm_old));
		else
			rot1 = -abs(4 * DoJeepShift(item, &mm, &mm_old));
	}

	int hb = TestJeepHeight(item, 550, 256, (PHD_VECTOR*)&b);
	if (hb < b_old.y - STEP_SIZE)
		rot2 = -abs(4 * DoJeepShift(item, &b, &b_old));

	int hmb = TestJeepHeight(item, -600, 0, (PHD_VECTOR*)&mb);
	if (hmb < mb_old.y - STEP_SIZE)
		DoJeepShift(item, &mb, &mb_old);
	
	int hmt = TestJeepHeight(item, -600, 256, (PHD_VECTOR*)&mt);
	if (hmt < mt_old.y - STEP_SIZE)
	{
		if (rot2)
			rot2 -= abs(4 * DoJeepShift(item, &mt, &mt_old));
		else
			rot2 = abs(4 * DoJeepShift(item, &mt, &mt_old));
	}

	if (!rot1)
		rot1 = rot2;
	   
	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos - STEP_SIZE)
		DoJeepShift(item, (PHD_VECTOR*)&item->pos, &oldPos);

	if (!jeep->velocity)
		rot1 = 0;

	jeep->extraRotation = rot1;
	
	/*jeep->unknown0 += rot1 >> 1;

	if (abs(jeep->unknown0) < 2)
		jeep->unknown0 = 0;

	if (abs(jeep->unknown0 - jeep->extraRotation) >= 4)
		jeep->extraRotation += ((jeep->unknown0 - jeep->extraRotation) >> 2);
	else
		jeep->extraRotation = jeep->unknown0;
		*/
	int newspeed = 0;
	int collide = GetJeepCollisionAnim(item, &movedPos);
	
	if (collide)
	{
		newspeed = ((item->pos.zPos - oldPos.z) * phd_cos(jeep->momentumAngle) + (item->pos.xPos - oldPos.x) * phd_sin(jeep->momentumAngle)) >> W2V_SHIFT;
		newspeed <<= 8;

		if ((&g_Level.Items[Lara.Vehicle] == item) && (jeep->velocity == JEEP_MAX_SPEED) && (newspeed < (JEEP_MAX_SPEED - 10)))
		{
			LaraItem->hitPoints -= (JEEP_MAX_SPEED - newspeed) >> 7;
			LaraItem->hitStatus = true;
		}

		if (jeep->velocity > 0 && newspeed < jeep->velocity)
			jeep->velocity = (newspeed < 0) ? 0 : newspeed;

		else if (jeep->velocity < 0 && newspeed > jeep->velocity)
			jeep->velocity = (newspeed > 0) ? 0 : newspeed;

		if (jeep->velocity < JEEP_MAX_BACK)
			jeep->velocity = JEEP_MAX_BACK;
	}

	return collide;
}

static int JeepUserControl(ITEM_INFO* item, int height, int* pitch)
{
	if (LaraItem->currentAnimState == 10 || LaraItem->goalAnimState == 10)
		TrInput = 0;
	
	JEEP_INFO* jeep = (JEEP_INFO*)item->data;

	if (jeep->revs <= 16)
		jeep->revs = 0;
	else
	{
		jeep->velocity += jeep->revs >> 4;
		jeep->revs -= (jeep->revs >> 3);
	}

	int rot1 = 0;
	int rot2 = 0;

	if (item->pos.yPos >= height - STEP_SIZE)
	{
		if (!jeep->velocity)
		{
			if (TrInput & IN_LOOK)
				LookUpDown();
		}

		if (abs(jeep->velocity) <= JEEP_MAX_SPEED / 2)
		{
			rot1 = (int)ANGLE(5) * abs(jeep->velocity) >> W2V_SHIFT;
			rot2 = (60 * abs(jeep->velocity) >> W2V_SHIFT) + ANGLE(1);
		}
		else
		{
			rot1 = ANGLE(5);
			rot2 = 242;
		}

		if (jeep->velocity < 0)
		{
			if (TrInput & (IN_RIGHT | IN_RSTEP))
			{
				jeep->jeepTurn -= rot2;
				if (jeep->jeepTurn < -rot1)
					jeep->jeepTurn = -rot1;
			}
			else if (TrInput & (IN_LEFT | IN_LSTEP))
			{
				jeep->jeepTurn += rot2;
				if (jeep->jeepTurn > rot1)
					jeep->jeepTurn = rot1;
			}
		}
		else if (jeep->velocity > 0)
		{
			if (TrInput & (IN_RIGHT | IN_RSTEP))
			{
				jeep->jeepTurn += rot2;
				if (jeep->jeepTurn > rot1)
					jeep->jeepTurn = rot1;
			}
			else if (TrInput & (IN_LEFT | IN_LSTEP))
			{
				jeep->jeepTurn -= rot2;
				if (jeep->jeepTurn < -rot1)
					jeep->jeepTurn = -rot1;
			}
		}

		// Break/reverse
		if (TrInput & JEEP_IN_BRAKE)
		{
			if (jeep->velocity < 0)
			{
				jeep->velocity += 768;
				if (jeep->velocity > 0)
					jeep->velocity = 0;
			}
			else if (jeep->velocity > 0)
			{
				jeep->velocity -= 768;
				if (jeep->velocity < 0)
					jeep->velocity = 0;
			}
		}
		// Accelerate
		else if (TrInput & JEEP_IN_ACCELERATE)
		{
			if (jeep->unknown2)
			{
				if (jeep->unknown2 == 1 && jeep->velocity > -JEEP_MAX_BACK)
				{
					jeep->velocity -= (abs(-JEEP_MAX_BACK - jeep->velocity) >> 3) - 2;
				}
			}
			else
			{
				if (jeep->velocity < JEEP_MAX_SPEED)
				{
					if (jeep->velocity < 0x4000)
						jeep->velocity += 8 + ((0x4000 + 0x800 - jeep->velocity) >> 3);
					else if (jeep->velocity < 0x7000)
						jeep->velocity += 4 + ((0x7000 + 0x800 - jeep->velocity) >> 4);
					else if (jeep->velocity < JEEP_MAX_SPEED)
						jeep->velocity += 2 + ((JEEP_MAX_SPEED - jeep->velocity) >> 3);
				}
				else
					jeep->velocity = JEEP_MAX_SPEED;
			}

			jeep->velocity -= abs(item->pos.yRot - jeep->momentumAngle) >> 6;
		}
		else if (jeep->velocity > 256)
		{
			jeep->velocity -= 256;
		}
		else if (jeep->velocity < -256)
		{
			jeep->velocity += 256;
		}
		else
		{
			jeep->velocity = 0;
		}

		item->speed = jeep->velocity >> 8;
		if (jeep->engineRevs > 0xC000)
		{
			jeep->engineRevs = (GetRandomControl() & 0x1FF) + 48896;
		}

		int revs = jeep->velocity;
		if (jeep->velocity < 0)
		{
			revs >>= 1;
		}

		jeep->engineRevs += (abs(revs) - jeep->engineRevs) >> 3;
	}

	if (TrInput & JEEP_IN_BRAKE)
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = -144;
		pos.z = -1024;
		GetJointAbsPosition(item, &pos, 11);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
		item->meshBits = 163839;
	}
	else
	{
		item->meshBits = 114687;
	}
	
	*pitch = jeep->engineRevs;

	return 1;
}

static void AnimateJeep(ITEM_INFO* item, int collide, int dead)
{
	JEEP_INFO* jeep = (JEEP_INFO*)item->data;

	if (item->pos.yPos != item->floor && 
		LaraItem->currentAnimState != 11 && 
		LaraItem->currentAnimState != 12 && 
		!dead)
	{
		if (jeep->unknown2 == 1)
			LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 20;
		else
			LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 6;

		LaraItem->currentAnimState = 11;
		LaraItem->goalAnimState = 11;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
	}
	else if (collide)
	{
		if (LaraItem->currentAnimState != 4 &&
			LaraItem->currentAnimState != 5 &&
			LaraItem->currentAnimState != 2 &&
			LaraItem->currentAnimState != 3 &&
			LaraItem->currentAnimState != 11 &&
			jeep->velocity > JEEP_MAX_SPEED / 3 &&
			!dead)
		{
			switch (collide)
			{
			case 13:
				LaraItem->goalAnimState = LaraItem->currentAnimState = 4;
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 11;
				break;

			case 14:
				LaraItem->goalAnimState = LaraItem->currentAnimState = 5;
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 10;
				break;

			case 11:
				LaraItem->goalAnimState = LaraItem->currentAnimState = 2;
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 12;
				break;

			default:
				LaraItem->goalAnimState = LaraItem->currentAnimState = 3;
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 13;
				break;

			}

			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case 0:
			if (dead)
				LaraItem->goalAnimState = 16;
			else
			{
				if (((TrInput & JEEP_IN_DISMOUNT) == JEEP_IN_DISMOUNT) &&
					!jeep->velocity &&
					!JeepNoGetOff)
				{
					if (JeepCanGetOff())
						LaraItem->goalAnimState = 10;
					break;
				}

				if (!(DbInput & 0x800))
				{
					if (!(DbInput & 0x40000000))
					{
						if ((TrInput & JEEP_IN_ACCELERATE) && !(TrInput & JEEP_IN_BRAKE))
						{
							LaraItem->goalAnimState = 1;
							break;
						}
						else if (TrInput & (IN_LEFT | IN_LSTEP))
						{
							LaraItem->goalAnimState = 7;
						}
						else if (TrInput & (IN_RIGHT | IN_RSTEP))
						{
							LaraItem->goalAnimState = 8;
						}
					}
					else if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
							LaraItem->goalAnimState = 17;

					}
				}
				else
				{
					if (jeep->unknown2)
						jeep->unknown2--;
				}
			}

			break;

		case 1:
			if (dead)
				LaraItem->goalAnimState = 0;
			else
			{
				if (jeep->velocity & 0xFFFFFF00 ||
					TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
				{
					if (TrInput & JEEP_IN_BRAKE)
					{
						if (jeep->velocity <= 21844)
							LaraItem->goalAnimState = 0;
						else
							LaraItem->goalAnimState = 6;
					}
					else
					{
						if (TrInput & (IN_LEFT | IN_LSTEP))
						{
							LaraItem->goalAnimState = 7;
						}
						else if (TrInput & (IN_RIGHT | IN_RSTEP))
						{
							LaraItem->goalAnimState = 8;
						}
					}
				}
				else
				{
					LaraItem->goalAnimState = 0;
				}
			}

			break;

		case 2:
		case 3:
		case 4:
		case 5:
			if (dead)
			{
				LaraItem->goalAnimState = 0;
			}
			else if (TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
			{
				LaraItem->goalAnimState = 1;
			}

			break;

		case 6:
			if (dead)
			{
				LaraItem->goalAnimState = 0;
			}
			else if (jeep->velocity & 0xFFFFFF00)
			{
				if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					LaraItem->goalAnimState = 7;
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					LaraItem->goalAnimState = 8;
				}
			}
			else
			{
				LaraItem->goalAnimState = 0;
			}

			break;

		case 7:
			if (dead)
			{
				LaraItem->goalAnimState = 0;
			}
			else if ((DbInput & 0x80u) == 0)
			{
				if (DbInput & 0x40000000)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
						{
							LaraItem->goalAnimState = 15;
							LaraItem->currentAnimState = 15;
							LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 40;
							LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
							break;
						}
					}
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					LaraItem->goalAnimState = 1;
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					LaraItem->goalAnimState = 7;
				}
				else if (jeep->velocity)
				{
					LaraItem->goalAnimState = 1;
				}
				else
				{
					LaraItem->goalAnimState = 0;
				}
			}
			else
			{
				if (jeep->unknown2)
				{
					jeep->unknown2--;
				}
			}

			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 4 &&
				!jeep->velocity)
			{
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 32;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase + 14;
			}
			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 32)
			{
				if (jeep->velocity)
				{
					LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 4;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				}
			}

			break;

		case 8:
			if (dead)
			{
				LaraItem->goalAnimState = 0;
			}
			if ((DbInput & 0x80u) == 0)
			{
				if (DbInput & 0x40000000)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
						{
							LaraItem->goalAnimState = 14;
							LaraItem->currentAnimState = 14;
							LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 41;
							LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
							break;
						}
					}
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					LaraItem->goalAnimState = 1;
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					LaraItem->goalAnimState = 8;
				}
				else if (jeep->velocity)
				{
					LaraItem->goalAnimState = 1;
				}
				else
				{
					LaraItem->goalAnimState = 0;
				}
			}
			else
			{
				if (jeep->unknown2)
				{
					jeep->unknown2--;
				}
			}

			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 16 && !jeep->velocity)
			{
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 33;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase + 14;
			}
			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 33)
			{
				if (jeep->velocity)
				{
					LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 16;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				}
			}

			break;

		case 11:
			if (item->pos.yPos == item->floor)
			{
				LaraItem->goalAnimState = 12;
			}
			else if (item->fallspeed > 300)
			{
				jeep->flags |= 0x40u;
			}

			break;

		case 13:
			if (dead)
			{
				LaraItem->goalAnimState = 17;
			}
			else if (abs(jeep->velocity) & 0xFFFFFF00)
			{
				if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					LaraItem->goalAnimState = 15;
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					LaraItem->goalAnimState = 14;
				}
			}
			else
			{
				LaraItem->goalAnimState = 17;
			}

			break;

		case 14:
			if (dead)
			{
				LaraItem->goalAnimState = 17;
			}
			else if ((DbInput & 0x80u) == 0)
			{
				if (DbInput & 0x40000000)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
					}
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					LaraItem->goalAnimState = 14;
				}
				else
				{
					LaraItem->goalAnimState = 13;
				}
			}
			else
			{
				if (jeep->unknown2)
				{
					jeep->unknown2--;
					if (!jeep->unknown2)
					{
						LaraItem->goalAnimState = 8;
						LaraItem->currentAnimState = 8;
						LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 44;
						LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
						break;
					}
				}
			}

			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 30 && !jeep->velocity)
			{
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 37;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase + 14;
			}
			if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 37)
			{
				if (jeep->velocity)
				{
					LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 30;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				}
			}

			break;

		case 15:
			if (dead)
			{
				LaraItem->goalAnimState = 17;
			}
			else if ((DbInput & 0x80u) == 0)
			{
				if (DbInput & 0x40000000)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
					}
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					LaraItem->goalAnimState = 15;
				}
				else
				{
					LaraItem->goalAnimState = 13;
				}

				if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 27 && !jeep->velocity)
				{
					LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 36;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase + 14;
				}
				if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 36)
				{
					if (jeep->velocity)
					{
						LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 27;
						LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
					}
				}
				break;
			}
			else if (!jeep->unknown2 || (--jeep->unknown2 != 0))
			{
				if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 27 && !jeep->velocity)
				{
					LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 36;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase + 14;
				}
				if (LaraItem->animNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + 36)
				{
					if (jeep->velocity)
					{
						LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 27;
						LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
					}
				}
				break;
			}

			LaraItem->goalAnimState = 7;
			LaraItem->currentAnimState = 7;
			LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 44;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;

			break;

		case 17:
			if (dead)
			{
				LaraItem->goalAnimState = 0;
			}
			if ((TrInput & JEEP_IN_DISMOUNT) != JEEP_IN_DISMOUNT || jeep->velocity || JeepNoGetOff)
			{
				if ((DbInput & 0x80u) == 0)
				{
					if (DbInput & 0x40000000)
					{
						if (jeep->unknown2 < 1)
						{
							jeep->unknown2++;
						}
					}
					else if (!(TrInput & JEEP_IN_ACCELERATE) || TrInput & JEEP_IN_BRAKE)
					{
						if (TrInput & (IN_LEFT | IN_LSTEP))
						{
							LaraItem->goalAnimState = 15;
						}
						else if (TrInput & (IN_LEFT | IN_LSTEP))
						{
							LaraItem->goalAnimState = 14;
						}
					}
					else
					{
						LaraItem->goalAnimState = 13;
					}
				}
				else
				{
					if (jeep->unknown2)
					{
						jeep->unknown2--;
						if (!jeep->unknown2)
						{
							LaraItem->goalAnimState = 0;
						}
					}
				}
			}
			else
			{
				if (JeepCanGetOff())
				{
					LaraItem->goalAnimState = 10;
				}
			}
			break;

		default:
			break;
		}
	}

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		LaraItem->goalAnimState = 11;
		LaraItem->hitPoints = 0;
		JeepExplode(item);
	}
}

void JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	if (l->hitPoints > 0 && Lara.Vehicle == NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (GetOnJeep(itemNumber))
		{
			Lara.Vehicle = itemNumber;

			if (Lara.gunType == WEAPON_FLARE)
			{
				CreateFlare(ID_FLARE_ITEM, 0);
				undraw_flare_meshes();
				Lara.flareControlLeft = 0;
				Lara.requestGunType = WEAPON_NONE;
				Lara.gunType = WEAPON_NONE;
			}

			Lara.gunStatus = LG_HANDS_BUSY;

			/*v4 = *(_WORD*)(Rooms + 148 * (signed short)v3->roomNumber + 72);
			// Enable ENEMY_JEEP
			if (v4 != -1)
			{
				while (1)
				{
					v5 = Items + 5622 * v4;
					if (*(_WORD*)(Items + 5622 * v4 + 12) == 34)
					{
						break;
					}
					v4 = *(_WORD*)(v5 + 26);
					if (v4 == -1)
					{
						goto LABEL_11;
					}
				}
				EnableBaddieAI(v4, 1);
				*(_DWORD*)(v5 + 5610) = *(_DWORD*)(v5 + 5610) & 0xFFFFFFFB | 2;
				AddActiveItem(v4);
			}*/

			short ang = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
			ang -= item->pos.yRot;

			if ((ang > -(ONE_DEGREE * 45)) && (ang < (ONE_DEGREE * 135)))
			{
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 18;
			}
			else
			{
				LaraItem->animNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 9;
			}

			LaraItem->goalAnimState = 9;
			LaraItem->currentAnimState = 9;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;

			item->hitPoints = 1;
			LaraItem->pos.xPos = item->pos.xPos;
			LaraItem->pos.yPos = item->pos.yPos;
			LaraItem->pos.zPos = item->pos.zPos;
			LaraItem->pos.yRot = item->pos.yRot;

			Lara.headXrot = Lara.headYrot = 0;
			Lara.torsoXrot = Lara.torsoYrot = 0;
			Lara.hitDirection = -1;

			AnimateItem(l);

			int anim = LaraItem->animNumber;

			JEEP_INFO* jeep = (JEEP_INFO*)item->data;
			jeep->revs = 0;
			jeep->unknown2 = 0;

			item->flags |= 0x20;

			CurrentAtmosphere = 98;
			IsAtmospherePlaying = 1;
			S_CDPlay(98, 1);
		}
		else
		{
			ObjectCollision(itemNumber, l, coll);
		}
	}
}

int JeepControl(void)
{
	ITEM_INFO* item = &g_Level.Items[Lara.Vehicle];
	JEEP_INFO* jeep = (JEEP_INFO*)item->data;

	int drive = -1;
	bool dead = 0;

	int collide = JeepDynamics(item);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	GAME_VECTOR oldPos;
	oldPos.x = item->pos.xPos;
	oldPos.y = item->pos.yPos;
	oldPos.z = item->pos.zPos;

	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	PHD_VECTOR fl, fr, bc;
	int hfl = TestJeepHeight(item, 550, -256, &fl);
	int hfr = TestJeepHeight(item, 550, 256, &fr);
	int hbc = TestJeepHeight(item, -600, 0, &bc);

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	TestTriggers(TriggerIndex, 1, 0);
	TestTriggers(TriggerIndex, 0, 0);

	if (LaraItem->hitPoints <= 0)
	{
		dead = true;
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
	}

	int pitch = 0;
	if (jeep->flags)
	{
		collide = 0;
	}
	else if (LaraItem->currentAnimState == 9)
	{
		drive = -1;
		collide = 0;
	}
	else
	{
		drive = JeepUserControl(item, height, &pitch);
	}

	if (jeep->velocity || jeep->revs)
	{
		jeep->pitch = pitch;
		if (pitch >= -32768)
		{
			if (pitch > 40960)
			{
				jeep->pitch = 40960;
			}
		}
		else
		{
			jeep->pitch = -32768;
		}

		SoundEffect(155, &item->pos, (jeep->pitch << 8) + 16777220);
	}
	else
	{
		if (drive != -1)
		{
			SoundEffect(153, &item->pos, 0);
		}
		jeep->pitch = 0;
	}

	item->floor = height;
	short rotAdd = jeep->velocity >> 2;
	jeep->rot1 -= rotAdd;
	jeep->rot2 -= rotAdd;
	jeep->rot3 -= rotAdd;
	jeep->rot4 -= rotAdd;

	int oldY = item->pos.yPos;
	item->fallspeed = DoJeepDynamics(height, item->fallspeed, &item->pos.yPos, 0);

	height = (fl.y + fr.y) >> 1;
	short xRot;
	short zRot;
	if (bc.y >= hbc)
	{
		if (height >= (hfl + hfr) >> 1)
			xRot = phd_atan(1100, hbc - height);
		else
			xRot = phd_atan(550, hbc - item->pos.yPos);
	}
	else
	{
		if (height >= (hfl + hfr) >> 1)
		{
			xRot = phd_atan(550, item->pos.yPos - height);
		}
		else
		{
			xRot = -phd_atan(137, oldY - item->pos.yPos);
			if (jeep->velocity < 0)
				xRot = -xRot;
		}
	}

	item->pos.xRot += (xRot - item->pos.xRot) >> 2;
	item->pos.zRot += (phd_atan(256, height - fl.y) - item->pos.zRot) >> 2;

	if (!(jeep->flags & JF_DEAD))
	{
		if (roomNumber != item->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		LaraItem->pos.xPos = item->pos.xPos;
		LaraItem->pos.yPos = item->pos.yPos;
		LaraItem->pos.zPos = item->pos.zPos;
		LaraItem->pos.xRot = item->pos.xRot;
		LaraItem->pos.yRot = item->pos.yRot;
		LaraItem->pos.zRot = item->pos.zRot;

		int jeepAnim = Objects[ID_JEEP].animIndex;
		int laraAnim = LaraItem->animNumber;
		int extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;

		AnimateJeep(item, collide, dead);
		AnimateItem(LaraItem);

		item->animNumber = Objects[ID_JEEP].animIndex + LaraItem->animNumber - Objects[ID_JEEP_LARA_ANIMS].animIndex;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);

		jeepAnim = Objects[ID_JEEP].animIndex;
		laraAnim = LaraItem->animNumber;
		extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;


		Camera.targetElevation = -ANGLE(30);
		Camera.targetDistance = 2 * WALL_SIZE;

		if (jeep->unknown2)
		{
			if (jeep->unknown2 == 1)
				jeep->fallSpeed += (32578 - jeep->fallSpeed) >> 3;
		}
		else
		{
			jeep->fallSpeed -= jeep->fallSpeed >> 3;
		}

		Camera.targetAngle = jeep->fallSpeed;

		if (jeep->flags & JF_FALLING && item->pos.yPos == item->floor)
		{
			LaraItem->meshBits = 0;
			LaraItem->hitPoints = 0;
			LaraItem->flags |= ONESHOT;
			JeepExplode(item);
			return 0;
		}
	}

	if (LaraItem->currentAnimState == 9 ||
		LaraItem->currentAnimState == 10)
	{
		JeepSmokeStart = 0;
	}
	else
	{
		short speed = 0;
		short angle = 0;

		PHD_VECTOR pos;
		pos.x = 80;
		pos.y = 0;
		pos.z = -500;

		GetJointAbsPosition(item, &pos, 11);

		if (item->speed <= 32)
		{
			if (JeepSmokeStart >= 16)
			{
				if (GetRandomControl() & 3)
					speed = 0;
				else
					speed = ((GetRandomControl() & 0xF) + GetRandomControl() & 0x10) << 6;
			}
			else
			{
				speed = ((GetRandomControl() & 7) + GetRandomControl() & 0x10 + 2 * JeepSmokeStart) << 6;
				JeepSmokeStart++;
			}
			TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, item->pos.yRot + -32768, speed, 0);
		}
		else if (item->speed < 64)
		{
			TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, item->pos.yRot - 32768, 64 - item->speed, 1);
		}
	}

	return JeepCheckGetOff();
}