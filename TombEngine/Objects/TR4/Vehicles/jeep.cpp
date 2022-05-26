#include "framework.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/gui.h"
#include "Game/effects/effects.h"
#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/items.h"
#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara_flare.h"
#include "Specific/input.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Objects/TR4/Vehicles/jeep_info.h"

using std::vector;

enum JEEP_STATES 
{
	JS_STOP = 0,
	JS_DRIVE_FORWARD = 1,
	//2 3 4 5 are the collide with walls states
	JS_BRAKE = 6,//?
	JS_FWD_LEFT = 7,//FWD = forwards
	JS_FWD_RIGHT = 8,
	JS_GETIN = 9,
	JS_GETOFF = 10,
	JS_JUMP = 11,
	JS_LAND = 12,
	JS_BACK = 13,//when she's turning back, and when idle back.. hmm
	JS_BACK_LEFT = 14,
	JS_BACK_RIGHT = 15,
	JS_DEATH = 16,
	JS_DRIVE_BACK = 17
};

enum JEEP_ANIMS
{
	JA_DEATH = 0,
	JA_BRAKE = 1,
	JA_DRIVE_FORWARD = 2,
	JA_FWD_LEFT_START = 3,//FWD = forwards
	JA_FWD_LEFT = 4,
	JA_FWD_LEFT_END = 5,
	JA_FWD_JUMP_START = 6,
	JA_JUMP = 7,
	JA_FWD_JUMP_LAND = 8,
	JA_GETIN_RIGHT = 9,
	//10 11 12 13 are the collide with walls anims
	JA_IDLE = 14,
	JA_FWD_RIGHT_START = 15,
	JA_FWD_RIGHT = 16,
	JA_FWD_RIGHT_END = 17,
	JA_GETIN_LEFT = 18,
	JA_GETOFF = 19,
	JA_BACK_JUMP_START = 20,
	JA_BACK_JUMP = 21,
	JA_BACK_JUMP_LAND = 22,
	JA_REVERSE_START = 23,
	JA_REVERSE = 24,
	JA_REVERSE_END = 25, //aka turn head back forwards
	JA_BACK_RIGHT_START = 26,
	JA_BACK_RIGHT = 27,
	JA_BACK_RIGHT_END = 28,
	JA_BACK_LEFT_START = 29,
	JA_BACK_LEFT = 30,
	JA_BACK_LEFT_END = 31,
	JA_IDLE_RIGHT_START = 32,//turn steering whel right while idle
	JA_IDLE_LEFT_START = 33,// blah blah left while idle
	JA_IDLE_RIGHT_END = 34,//turn steering wheel straight from right while idle
	JA_IDLE_LEFT_END = 35,//blah blah straight from left while idle
	JA_IDLE_RIGHT_BACK_START = 36,//same as 32 but in reverse
	JA_IDLE_LEFT_BACK_START = 37,//same as 33 but in reverse
	JA_IDLE_BACK_RIGHT_END = 38,//same as 34 but in reverse
	JA_IDLE_BACK_LEFT_END = 39,//same as 35 but in reverse
	JA_IDLE_REVERSE_RIGHT = 40,//"change to reverse gear with the wheels still turned left"
	JA_IDLE_REVERSE_LEFT = 41,//"change to reverse gear with the wheels still turned right"
	JA_BACK_IDLE = 42,
	JA_IDLE_FWD_LEFT = 43,//aka the opposite of 41. "change to forward gear with the wheels still turned left"
	JA_IDLE_FWD_RIGHT = 44,//aka the opposite of 42. "change to forward gear with the wheels still turned right"
};

enum JEEP_FLAGS
{
	JF_FALLING = 0x40,
	JF_DEAD = 0x80
};

#define JEEP_GETOFF_DISTANCE		512
#define JEEP_UNDO_TURN				91
#define	JEEP_FRONT					550
#define JEEP_SIDE					256
#define JEEP_SLIP					100
#define JEEP_SLIP_SIDE				128
#define JEEP_MAX_SPEED				0x8000
#define JEEP_MAX_BACK				0x4000


#define JEEP_IN_ACCELERATE	(IN_ACTION)
#define JEEP_IN_BRAKE		(IN_JUMP)

//bool QuadHandbrakeStarting;
//bool QuadCanHandbrakeStart;
char JeepSmokeStart;
bool JeepNoGetOff;
short Unk_0080DE1A;
int Unk_0080DDE8;
short Unk_0080DE24;

static int TestJeepHeight(ItemInfo* item, int dz, int dx, Vector3Int* pos)
{
	pos->y = item->Pose.Position.y - dz * phd_sin(item->Pose.Orientation.x) + dx * phd_sin(item->Pose.Orientation.z);

	float c = phd_cos(item->Pose.Orientation.y);
	float s = phd_sin(item->Pose.Orientation.y);

	pos->z = item->Pose.Position.z + dz * c - dx * s;
	pos->x = item->Pose.Position.x + dz * s + dx * c;

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;

	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static int DoJeepShift(ItemInfo* jeep, Vector3Int* pos, Vector3Int* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);
	int  oldX = old->x / SECTOR(1);
	int oldZ = old->z / SECTOR(1);
	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			jeep->Pose.Position.z += (old->z - pos->z);
			jeep->Pose.Position.x += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			jeep->Pose.Position.z -= shiftZ + 1;
			return (pos->x - jeep->Pose.Position.x);
		}
		else
		{
			jeep->Pose.Position.z += WALL_SIZE - shiftZ;
			return (jeep->Pose.Position.x - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			jeep->Pose.Position.x -= shiftX + 1;
			return (jeep->Pose.Position.z - pos->z);
		}
		else
		{
			jeep->Pose.Position.x += WALL_SIZE - shiftX;
			return (pos->z - jeep->Pose.Position.z);
		}
	}
	else
	{
		x = 0;
		z = 0;

		short roomNumber = jeep->RoomNumber;
		FloorInfo* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = jeep->RoomNumber;
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
			jeep->Pose.Position.z += z;
			jeep->Pose.Position.x += x;
		}
		else if (z)
		{
			jeep->Pose.Position.z += z;
			if (z > 0)
				return (jeep->Pose.Position.x - pos->x);
			else
				return (pos->x - jeep->Pose.Position.x);
		}
		else if (x)
		{
			jeep->Pose.Position.x += x;
			if (x > 0)
				return (pos->z - jeep->Pose.Position.z);
			else
				return (jeep->Pose.Position.z - pos->z);
		}
		else
		{
			jeep->Pose.Position.z += (old->z - pos->z);
			jeep->Pose.Position.x += (old->x - pos->x);
		}
	}

	return 0;
}

static int DoJeepDynamics(int height, int speed, int* y, int flags)
{
	int result = 0;

	if (height <= *y)//on the floor
	{
		if (flags)
			result = speed;
		else
		{
			int temp = height - *y;

			if (temp < -80)
				temp = -80;

			result = ((temp - speed) / 16) + speed;

			if (*y > height)
				*y = height;
		}
	}
	else//mid air
	{
		*y += speed;
		if (*y <= height - 32)
		{
			if (flags)
				result = flags + (flags / 2) + speed;
			else
				result = speed + GRAVITY;
		}
		else
		{
			*y = height;

			if (speed > 150)
				LaraItem->HitPoints += 150 - speed;

			result = 0;
		}
	}

	return result;
}

static int JeepCanGetOff()
{
	ItemInfo* item = &g_Level.Items[Lara.Vehicle];

	short angle = item->Pose.Orientation.y + 0x4000;

	int x = item->Pose.Position.x - JEEP_GETOFF_DISTANCE * phd_sin(angle);
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z - JEEP_GETOFF_DISTANCE * phd_cos(angle);

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);

	auto collResult = GetCollision(x, y, z, item->RoomNumber);

	if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT)
		return 0;

	if (abs(height - item->Pose.Position.y) > WALL_SIZE / 2)
		return 0;

	int ceiling = GetCeiling(floor, x, y, z);

	if ((ceiling - item->Pose.Position.y > -LARA_HEIGHT)
		|| (height - ceiling < LARA_HEIGHT))
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

	spark->transType = TransTypeEnum::COLADD;
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
	spark->dSize = (GetRandomControl() & 7) + (speed / 128) + 32;
	spark->sSize = spark->dSize / 2;
	spark->size = spark->dSize / 2;
}

void InitialiseJeep(short itemNum)
{
	ItemInfo* item = &g_Level.Items[itemNum];
	
	JeepInfo* jeep;
	item->Data = JeepInfo();
	jeep = item->Data;

	jeep->velocity = 0;
	jeep->revs = 0;
	jeep->jeepTurn = 0;
	jeep->fallSpeed = 0;
	jeep->extraRotation = 0;
	jeep->momentumAngle = item->Pose.Position.y;
	jeep->pitch = 0;
	jeep->flags = 0;
	jeep->unknown2 = 0;
	jeep->rot1 = 0;
	jeep->rot2 = 0;
	jeep->rot3 = 0;
	jeep->rot4 = 0;

	item->MeshBits = 114687;
}

static int JeepCheckGetOff()
{
	if (LaraItem->Animation.ActiveState == JS_GETOFF)
	{
		if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
		{
			LaraItem->Pose.Orientation.y += ANGLE(90);
			LaraItem->Animation.AnimNumber = LA_STAND_SOLID;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
			LaraItem->Animation.TargetState = LS_IDLE;
			LaraItem->Animation.ActiveState = LS_IDLE;
			LaraItem->Pose.Position.x -= JEEP_GETOFF_DISTANCE * phd_sin(LaraItem->Pose.Orientation.y);
			LaraItem->Pose.Position.z -= JEEP_GETOFF_DISTANCE * phd_cos(LaraItem->Pose.Orientation.y);
			LaraItem->Pose.Orientation.x = 0;
			LaraItem->Pose.Orientation.z = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.Control.HandStatus = HandStatus::Free;
			return false;
		}
	}

	return true;
}

static int GetOnJeep(int itemNumber)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

	if (!(TrInput & IN_ACTION) && g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM1)
		return 0;

	if (item->Flags & 0x100)
		return 0;

	if (Lara.Control.HandStatus != HandStatus::Free)
		return 0;

	if (LaraItem->Animation.ActiveState != LS_IDLE)
		return 0;

	if (LaraItem->Animation.AnimNumber != LA_STAND_IDLE)
		return 0;

	if (LaraItem->Animation.Airborne)
		return 0;

	if (abs(item->Pose.Position.y - LaraItem->Pose.Position.y) >= STEP_SIZE)
		return 0;

	if (!TestBoundsCollide(item, LaraItem, 100))
		return 0;

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) < -32000)
		return 0;

	short angle = phd_atan(item->Pose.Position.z - LaraItem->Pose.Position.z, item->Pose.Position.x - LaraItem->Pose.Position.x);
	angle -= item->Pose.Orientation.y;

	if ((angle > -ANGLE(45)) && (angle < ANGLE(135)))
	{
		int tempAngle = LaraItem->Pose.Orientation.y - item->Pose.Orientation.y;
		if (tempAngle > ANGLE(45) && tempAngle < ANGLE(135))
		{
			if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
			{
				g_Gui.SetInventoryItemChosen(NO_ITEM);
				return 1;
			}
			else
			{
				if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM1))
					g_Gui.SetEnterInventory(ID_PUZZLE_ITEM1);

				return 0;
			}
		}
		else
			return 0;
	}
	else
	{
		int tempAngle = LaraItem->Pose.Orientation.y - item->Pose.Orientation.y;
		if (tempAngle > ANGLE(225) && tempAngle < ANGLE(315))
		{
			if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
			{
				g_Gui.SetInventoryItemChosen(NO_ITEM);
				return 1;
			}
			else
			{
				if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM1))
					g_Gui.SetEnterInventory(ID_PUZZLE_ITEM1);

				return 0;
			}
		}
		else
			return 0;
	}
	return 0;
}

static int GetJeepCollisionAnim(ItemInfo* item, Vector3Int* p)
{
	JeepInfo* jeep = (JeepInfo*)item->Data;

	if (jeep->unknown2 != 0)
		return 0;

	p->x = item->Pose.Position.x - p->x;
	p->z = item->Pose.Position.z - p->z;

	if (p->x || p->z)
	{
		float c = phd_cos(item->Pose.Orientation.y);
		float s = phd_sin(item->Pose.Orientation.y);
		int front = p->z * c + p->x * s;
		int side = -p->z * s + p->x * c;

		if (abs(front) > abs(side))
			return (front > 0) + 13;
		else
			return (side <= 0) + 11;
	}

	return 0;
}

static void JeepBaddyCollision(ItemInfo* jeep)
{
	vector<short> roomsList;
	short* door, numDoors;

	roomsList.push_back(jeep->RoomNumber);

	ROOM_INFO* room = &g_Level.Rooms[jeep->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
	{
		roomsList.push_back(room->doors[i].room);
	}

	for (int i = 0; i < roomsList.size(); i++)
	{
		for (short itemNumber : g_Level.Rooms[roomsList[i]].Items)
		{
			ItemInfo* item = &g_Level.Items[itemNumber];
			if (item->Collidable && item->Status != ITEM_INVISIBLE && item != LaraItem && item != jeep)
			{
				if (item->ObjectNumber == ID_ENEMY_JEEP)
				{
					Unk_0080DE1A = 0;
					Unk_0080DDE8 = 400;
					Unk_0080DE24 = Unk_0080DE24 & 0xFFDF | 0x10;

					//ObjectCollision(item, jeep, )
				}
				else
				{
					ObjectInfo* object = &Objects[item->ObjectNumber];
					if (object->collision && object->intelligent ||
						item->ObjectNumber == ID_ROLLINGBALL)
					{
						int x = jeep->Pose.Position.x - item->Pose.Position.x;
						int y = jeep->Pose.Position.y - item->Pose.Position.y;
						int z = jeep->Pose.Position.z - item->Pose.Position.z;
						if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
						{
							if (item->ObjectNumber == ID_ROLLINGBALL)
							{
								if (TestBoundsCollide(item, LaraItem, 100))
								{
									if (LaraItem->HitPoints > 0)
									{
										DoLotsOfBlood(LaraItem->Pose.Position.x,
											LaraItem->Pose.Position.y - 512,
											LaraItem->Pose.Position.z,
											GetRandomControl() & 3,
											LaraItem->Pose.Orientation.y,
											LaraItem->RoomNumber,
											5);
										item->HitPoints -= 8;
									}
								}
							}
							else
							{
								if (TestBoundsCollide(item, jeep, JEEP_FRONT))
								{
									DoLotsOfBlood(item->Pose.Position.x,
										jeep->Pose.Position.y - STEP_SIZE,
										item->Pose.Position.z,
										GetRandomControl() & 3,
										jeep->Pose.Orientation.y,
										item->RoomNumber,
										3);
									item->HitPoints = 0;
								}
							}
						}
					}
				}
			}
		}
	}
}

static void JeepExplode(ItemInfo* item)
{
	if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(item, 1);
	}
	else
	{
		TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -2, 0, item->RoomNumber);
		for (int i = 0; i < 3; i++)
		{
			TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -1, 0, item->RoomNumber);
		}
	}

	ExplodingDeath(Lara.Vehicle, -1, 256);
	KillItem(Lara.Vehicle);
	item->Status = ITEM_DEACTIVATED;
	SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, 0, 0);
	Lara.Vehicle = NO_ITEM;
}

int JeepDynamics(ItemInfo* item)
{
	JeepInfo* jeep = (JeepInfo*)item->Data;

	Vector3Int f_old, b_old, mm_old, mt_old, mb_old;

	int hf_old = TestJeepHeight(item, JEEP_FRONT, -JEEP_SIDE, &f_old);
	int hb_old = TestJeepHeight(item, JEEP_FRONT, JEEP_SIDE, &b_old);
	int hmm_old = TestJeepHeight(item, -(JEEP_FRONT + 50), -JEEP_SIDE, &mm_old);
	int hmt_old = TestJeepHeight(item, -(JEEP_FRONT + 50), JEEP_SIDE, &mt_old);
	int hmb_old = TestJeepHeight(item, -(JEEP_FRONT + 50), 0, (Vector3Int*)&mb_old);

	Vector3Int oldPos;
	oldPos.x = item->Pose.Position.x;
	oldPos.y = item->Pose.Position.y;
	oldPos.z = item->Pose.Position.z;

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

	if (oldPos.y <= item->Floor - 8 )
	{
		if (jeep->jeepTurn < -JEEP_UNDO_TURN)
			jeep->jeepTurn += JEEP_UNDO_TURN;
		else if (jeep->jeepTurn > JEEP_UNDO_TURN)
			jeep->jeepTurn -= JEEP_UNDO_TURN;
		else
			jeep->jeepTurn = 0;

		item->Pose.Orientation.y += jeep->jeepTurn + jeep->extraRotation;
		jeep->momentumAngle += ((item->Pose.Orientation.y - jeep->momentumAngle) / 32);
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

		item->Pose.Orientation.y += jeep->jeepTurn + jeep->extraRotation;

		rot = item->Pose.Orientation.y - jeep->momentumAngle;
		momentum = 728 - ((3 * jeep->velocity) / 2048);

		if (!(TrInput & IN_ACTION) && jeep->velocity > 0)
			momentum -= (momentum / 4);

		if (rot >= -273)
		{
			if (rot <= 273)
				jeep->momentumAngle = item->Pose.Orientation.y;
			else
			{
				if (rot > 13650)
				{
					item->Pose.Position.y -= 41;
					item->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
					jeep->jeepTurn = 0;
					jeep->velocity -= (jeep->velocity / 8);
				}

				if (rot <= 16380)
					jeep->momentumAngle += momentum;
				else
					jeep->momentumAngle = item->Pose.Orientation.y - 16380;
			}
		}
		else
		{
			if (rot < -13650)
			{
				item->Pose.Position.y -= 41;
				item->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
				jeep->jeepTurn = 0;
				jeep->velocity -= (jeep->velocity / 8);
			}

			if (rot >= -16380)
				jeep->momentumAngle -= momentum;
			else
				jeep->momentumAngle = item->Pose.Orientation.y + 16380;
		}
	}

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	int height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

	short speed;
	if (item->Pose.Position.y < height)
		speed = item->Animation.Velocity;
	else
		speed = item->Animation.Velocity * phd_cos(item->Pose.Orientation.x);

	item->Pose.Position.x += speed * phd_sin(jeep->momentumAngle);
	item->Pose.Position.z += speed * phd_cos(jeep->momentumAngle);
	
	int slip = 0;
	if (item->Pose.Position.y >= height)
	{
		slip = JEEP_SLIP * phd_sin(item->Pose.Orientation.x);

		if (abs(slip) > 16)
		{
			JeepNoGetOff = 1;
			if (slip >= 0)
				slip = jeep->velocity - (SQUARE(slip - 16) / 2);
			else
				slip = (SQUARE(slip + 16) / 2) + jeep->velocity;
			jeep->velocity = slip;
		}
		else
			JeepNoGetOff = 0;

		slip = JEEP_SLIP_SIDE * phd_sin(item->Pose.Orientation.z);
		if (abs(slip) > JEEP_SLIP_SIDE / 4)
		{
			JeepNoGetOff = 1;

			if (slip >= 0)
			{
				item->Pose.Position.x += (slip - 24) * phd_sin(item->Pose.Orientation.y + ANGLE(90));
				item->Pose.Position.z += (slip - 24) * phd_cos(item->Pose.Orientation.y + ANGLE(90));
			}
			else
			{
				item->Pose.Position.x += (slip - 24) * phd_sin(item->Pose.Orientation.y - ANGLE(90));
				item->Pose.Position.z += (slip - 24) * phd_cos(item->Pose.Orientation.y - ANGLE(90));
			}
		}
		else
			JeepNoGetOff = 0;
	}

	if (jeep->velocity > JEEP_MAX_SPEED)
		jeep->velocity = JEEP_MAX_SPEED;
	else if (jeep->velocity < -JEEP_MAX_BACK)
		jeep->velocity = -JEEP_MAX_BACK;

	Vector3Int movedPos;
	movedPos.x = item->Pose.Position.x;
	movedPos.z = item->Pose.Position.z;

	if (!(item->Flags & ONESHOT))
	{
		JeepBaddyCollision(item);
		// v37 = sub_467850(item->pos.Position.x, item->pos.Position.y, item->pos.Position.z, item->roomNumber, 512);
	}

	Vector3Int f, b, mm, mt, mb;
	
	int rot1 = 0;
	int rot2 = 0;

	int hf = TestJeepHeight(item, JEEP_FRONT, -JEEP_SIDE, (Vector3Int*)&f);
	if (hf < f_old.y - STEP_SIZE)
		rot1 = abs(4 * DoJeepShift(item, &f, &f_old));

	int hmm = TestJeepHeight(item, -(JEEP_FRONT + 50), -JEEP_SIDE, (Vector3Int*)&mm);
	if (hmm < mm_old.y - STEP_SIZE)
	{
		if (rot)
			rot1 += abs(4 * DoJeepShift(item, &mm, &mm_old));
		else
			rot1 = -abs(4 * DoJeepShift(item, &mm, &mm_old));
	}

	int hb = TestJeepHeight(item, JEEP_FRONT, JEEP_SIDE, (Vector3Int*)&b);
	if (hb < b_old.y - STEP_SIZE)
		rot2 = -abs(4 * DoJeepShift(item, &b, &b_old));

	int hmb = TestJeepHeight(item, -(JEEP_FRONT + 50), 0, (Vector3Int*)&mb);
	if (hmb < mb_old.y - STEP_SIZE)
		DoJeepShift(item, &mb, &mb_old);
	
	int hmt = TestJeepHeight(item, -(JEEP_FRONT + 50), JEEP_SIDE, (Vector3Int*)&mt);
	if (hmt < mt_old.y - STEP_SIZE)
	{
		if (rot2)
			rot2 -= abs(4 * DoJeepShift(item, &mt, &mt_old));
		else
			rot2 = abs(4 * DoJeepShift(item, &mt, &mt_old));
	}

	if (!rot1)
		rot1 = rot2;
	   
	roomNumber = item->RoomNumber;
	floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) < item->Pose.Position.y - STEP_SIZE)
		DoJeepShift(item, (Vector3Int*)&item->Pose, &oldPos);

	if (!jeep->velocity)
		rot1 = 0;

	jeep->extraRotation = rot1;
	
	/*jeep->unknown0 += rot1 / 2;

	if (abs(jeep->unknown0) < 2)
		jeep->unknown0 = 0;

	if (abs(jeep->unknown0 - jeep->extraRotation) >= 4)
		jeep->extraRotation += ((jeep->unknown0 - jeep->extraRotation) / 4);
	else
		jeep->extraRotation = jeep->unknown0;
		*/ // just incase this code is ever uncommented.
	int newspeed = 0;
	int collide = GetJeepCollisionAnim(item, &movedPos);
	
	if (collide)
	{
		newspeed = (item->Pose.Position.z - oldPos.z) * phd_cos(jeep->momentumAngle) + (item->Pose.Position.x - oldPos.x) * phd_sin(jeep->momentumAngle);
		newspeed *= 256;

		if ((&g_Level.Items[Lara.Vehicle] == item) && (jeep->velocity == JEEP_MAX_SPEED) && (newspeed < (JEEP_MAX_SPEED - 10)))
		{
			LaraItem->HitPoints -= (JEEP_MAX_SPEED - newspeed) / 128;
			LaraItem->HitStatus = true;
		}

		if (jeep->velocity > 0 && newspeed < jeep->velocity)
			jeep->velocity = (newspeed > 0) ? 0 : newspeed;

		else if (jeep->velocity < 0 && newspeed > jeep->velocity)
			jeep->velocity = (newspeed < 0) ? 0 : newspeed;

		if (jeep->velocity < JEEP_MAX_BACK)
			jeep->velocity = JEEP_MAX_BACK;
	}

	return collide;
}

static int JeepUserControl(ItemInfo* item, int height, int* pitch)
{
	if (LaraItem->Animation.ActiveState == JS_GETOFF || LaraItem->Animation.TargetState == JS_GETOFF)
		TrInput = 0;
	
	JeepInfo* jeep = (JeepInfo*)item->Data;

	if (jeep->revs <= 16)
		jeep->revs = 0;
	else
	{
		jeep->velocity += (jeep->revs / 16);
		jeep->revs -= (jeep->revs / 8);
	}

	int rot1 = 0;
	int rot2 = 0;

	if (item->Pose.Position.y >= height - STEP_SIZE)
	{
		if (!jeep->velocity)
		{
			if (TrInput & IN_LOOK)
				LookUpDown(LaraItem);
		}

		if (abs(jeep->velocity) <= JEEP_MAX_SPEED / 2)
		{
			rot1 = ANGLE(5) * abs(jeep->velocity) / 16384;
			rot2 = 60 * abs(jeep->velocity) / 16384 + ANGLE(1);
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
					jeep->velocity -= (abs(-JEEP_MAX_BACK - jeep->velocity) / 8) - 2;
				else if (jeep->unknown2 == 1 && jeep->velocity < -JEEP_MAX_BACK)
					jeep->velocity = -JEEP_MAX_BACK;
			}
			else
			{
				if (jeep->velocity < JEEP_MAX_SPEED)
				{
					if (jeep->velocity < 0x4000)
						jeep->velocity += 8 + ((0x4000 + 0x800 - jeep->velocity) / 8);
					else if (jeep->velocity < 0x7000)
						jeep->velocity += 4 + ((0x7000 + 0x800 - jeep->velocity) / 16);
					else if (jeep->velocity < JEEP_MAX_SPEED)
						jeep->velocity += 2 + ((JEEP_MAX_SPEED - jeep->velocity) / 8);
				}
				else
					jeep->velocity = JEEP_MAX_SPEED;
			}

			jeep->velocity -= (abs(item->Pose.Orientation.y - jeep->momentumAngle) / 64);
		}
		else if (jeep->velocity > 256)
			jeep->velocity -= 256;
		else if (jeep->velocity < -256)
			jeep->velocity += 256;
		else
			jeep->velocity = 0;

		item->Animation.Velocity = jeep->velocity / 256;
		if (jeep->engineRevs > 0xC000)
			jeep->engineRevs = (GetRandomControl() & 0x1FF) + 48896;

		int revs = jeep->velocity;
		if (jeep->velocity < 0)
			revs /= 2;

		jeep->engineRevs += ((abs(revs) - jeep->engineRevs) / 8);
	}

	if (TrInput & JEEP_IN_BRAKE)
	{
		Vector3Int pos;
		pos.x = 0;
		pos.y = -144;
		pos.z = -1024;
		GetJointAbsPosition(item, &pos, 11);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
		item->MeshBits = 163839;
	}
	else
		item->MeshBits = 114687;
	
	*pitch = jeep->engineRevs;

	return 1;
}

static void AnimateJeep(ItemInfo* item, int collide, int dead)
{
	JeepInfo* jeep = (JeepInfo*)item->Data;
	bool dismount;
	if (item->Pose.Position.y != item->Floor && 
		LaraItem->Animation.ActiveState != JS_JUMP && 
		LaraItem->Animation.ActiveState != JS_LAND && 
		!dead)
	{
		if (jeep->unknown2 == 1)
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_JUMP_START;
		else
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_JUMP_START;

		LaraItem->Animation.ActiveState = JS_JUMP;
		LaraItem->Animation.TargetState = JS_JUMP;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
	}
	else if  ( (collide) && 
		(LaraItem->Animation.ActiveState != 4) && 
		(LaraItem->Animation.ActiveState != 5) && 
		(LaraItem->Animation.ActiveState != 2) &&
		(LaraItem->Animation.ActiveState != 3) &&
		(LaraItem->Animation.ActiveState != JS_JUMP) &&
		(0x2AAA < (short)jeep->velocity) &&
		(!dead) )
	{
		short state;
		switch (collide)
		{
		case 13:
			state = 4;
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 11;
			break;

		case 14:
			state = 5;
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 10;
			break;

		case 11:
			state = 2;
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 12;
			break;

		case 12:
			state = 3;
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 13;
			break;
		}

		LaraItem->Animation.ActiveState = state;
		LaraItem->Animation.TargetState = state;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
	}
	else
	{
		switch (LaraItem->Animation.ActiveState)
		{
		case JS_STOP:
			if (dead)
				LaraItem->Animation.TargetState = JS_DEATH;
			else
			{
				dismount = ((TrInput & JEEP_IN_BRAKE) && (TrInput & IN_LEFT)) ? true : false;
				if (dismount &&
					!jeep->velocity &&
					!JeepNoGetOff)
				{
					if (JeepCanGetOff())
						LaraItem->Animation.TargetState = JS_GETOFF;
					break;
				}

				if (DbInput & IN_WALK)
				{
					if (jeep->unknown2)
						jeep->unknown2--;
					break;
				}
				else if (DbInput & IN_SPRINT)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
							LaraItem->Animation.TargetState = JS_DRIVE_BACK;
						break;
					}
				}
				else
				{
					if ((TrInput & JEEP_IN_ACCELERATE) && !(TrInput & JEEP_IN_BRAKE))
					{
						LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
						break;
					}
					else if (TrInput & (IN_LEFT | IN_LSTEP))
						LaraItem->Animation.TargetState = JS_FWD_LEFT;
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						LaraItem->Animation.TargetState = JS_FWD_RIGHT;
				}

/*				if (!(DbInput & IN_WALK))
				{
					if (!(DbInput & IN_SPRINT))
					{
						if ((TrInput & JEEP_IN_ACCELERATE) && !(TrInput & JEEP_IN_BRAKE))
						{
							LaraItem->TargetState = JS_DRIVE_FORWARD;
							break;
						}
						else if (TrInput & (IN_LEFT | IN_LSTEP))
							LaraItem->TargetState = JS_FWD_LEFT;
						else if (TrInput & (IN_RIGHT | IN_RSTEP))
							LaraItem->TargetState = JS_FWD_RIGHT;
					}
					else if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
							LaraItem->TargetState = JS_DRIVE_BACK;

					}
				}
				else
				{
					if (jeep->unknown2)
						jeep->unknown2--;
				}*/
			}

			break;

		case JS_DRIVE_FORWARD:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			else
			{
				if (jeep->velocity & 0xFFFFFF00 ||
					TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
				{
					if (TrInput & JEEP_IN_BRAKE)
					{
						if (jeep->velocity <= 21844)
							LaraItem->Animation.TargetState = JS_STOP;
						else
							LaraItem->Animation.TargetState = JS_BRAKE;
					}
					else
					{
						if (TrInput & (IN_LEFT | IN_LSTEP))
							LaraItem->Animation.TargetState = JS_FWD_LEFT;
						else if (TrInput & (IN_RIGHT | IN_RSTEP))
							LaraItem->Animation.TargetState = JS_FWD_RIGHT;
					}
				}
				else
					LaraItem->Animation.TargetState = JS_STOP;
			}

			break;

		case 2:
		case 3:
		case 4:
		case 5:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			else if (TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
				LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
			break;

		case JS_BRAKE:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			else if (jeep->velocity & 0xFFFFFF00)
			{
				if (TrInput & (IN_LEFT | IN_LSTEP))
					LaraItem->Animation.TargetState = JS_FWD_LEFT;
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
					LaraItem->Animation.TargetState = JS_FWD_RIGHT;
			}
			else
				LaraItem->Animation.TargetState = JS_STOP;
			break;

		case JS_FWD_LEFT:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			else if (!(DbInput & IN_WALK))
			{
				if (DbInput & IN_SPRINT)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
						{
							LaraItem->Animation.TargetState = JS_BACK_RIGHT;
							LaraItem->Animation.ActiveState = JS_BACK_RIGHT;
							LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_REVERSE_RIGHT;
							LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
							break;
						}
					}
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
					LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
				else if (TrInput & (IN_LEFT | IN_LSTEP))
					LaraItem->Animation.TargetState = JS_FWD_LEFT;
				else if (jeep->velocity)
					LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
				else
					LaraItem->Animation.TargetState = JS_STOP;
			}
			else
			{
				if (jeep->unknown2)
					jeep->unknown2--;
			}

			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_LEFT &&
				!jeep->velocity)
			{
				LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_START;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + JA_IDLE;
			}
			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_START)
			{
				if (jeep->velocity)
				{
					LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_LEFT;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				}
			}

			break;

		case JS_FWD_RIGHT:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			if (!(DbInput & IN_WALK))
			{
				if (DbInput & IN_SPRINT)
				{
					if (jeep->unknown2 < 1)
					{
						jeep->unknown2++;
						if (jeep->unknown2 == 1)
						{
							LaraItem->Animation.TargetState = JS_BACK_LEFT;
							LaraItem->Animation.ActiveState = JS_BACK_LEFT;
							LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_REVERSE_LEFT;
							LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
							break;
						}
					}
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
					LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
					LaraItem->Animation.TargetState = JS_FWD_RIGHT;
				else if (jeep->velocity)
					LaraItem->Animation.TargetState = JS_DRIVE_FORWARD;
				else
					LaraItem->Animation.TargetState = JS_STOP;
			}
			else
			{
				if (jeep->unknown2)
					jeep->unknown2--;
			}

			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_RIGHT && !jeep->velocity)
			{
				LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_START;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + 14;//hmm
			}
			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_START)
			{
				if (jeep->velocity)
				{
					LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_RIGHT;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				}
			}

			break;

		case JS_JUMP:
			if (item->Pose.Position.y == item->Floor)
				LaraItem->Animation.TargetState = JS_LAND;
			else if (item->Animation.VerticalVelocity > 300)
				jeep->flags |= JF_FALLING;
			break;

		case JS_BACK:
			if (dead)
				LaraItem->Animation.TargetState = JS_DRIVE_BACK;
			else if (abs(jeep->velocity) & 0xFFFFFF00)
			{
				if (TrInput & (IN_LEFT | IN_LSTEP))
					LaraItem->Animation.TargetState = JS_BACK_RIGHT;
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
					LaraItem->Animation.TargetState = JS_BACK_LEFT;
			}
			else
				LaraItem->Animation.TargetState = JS_DRIVE_BACK;

			break;

		case JS_BACK_LEFT:
			if (dead)
				LaraItem->Animation.TargetState = JS_DRIVE_BACK;
			else if (!(DbInput & IN_WALK))
			{
				if (DbInput & IN_SPRINT)
				{
					if (jeep->unknown2 < 1)
						jeep->unknown2++;
				}
				else if (TrInput & (IN_RIGHT | IN_RSTEP))
					LaraItem->Animation.TargetState = JS_BACK_LEFT;
				else
					LaraItem->Animation.TargetState = JS_BACK;
			}
			else
			{
				if (jeep->unknown2)
				{
					jeep->unknown2--;
					if (!jeep->unknown2)
					{
						LaraItem->Animation.TargetState = JS_FWD_RIGHT;
						LaraItem->Animation.ActiveState = JS_FWD_RIGHT;
						LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_FWD_RIGHT;
						LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
						break;
					}
				}
			}

			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_LEFT && !jeep->velocity)
			{
				LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_BACK_START;
				LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + 14;
			}
			if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_BACK_START)
			{
				if (jeep->velocity)
				{
					LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_LEFT;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
				}
			}

			break;

		case JS_BACK_RIGHT:
			if (dead)
			{
				LaraItem->Animation.TargetState = JS_DRIVE_BACK;
			}
			else if (!(DbInput & IN_WALK))
			{
				if (DbInput & IN_SPRINT)
				{
					if (jeep->unknown2 < 1)
						jeep->unknown2++;
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
					LaraItem->Animation.TargetState = JS_BACK_RIGHT;
				else
					LaraItem->Animation.TargetState = JS_BACK;

				if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT && !jeep->velocity)
				{
					LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + 14;
				}
				if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START)
				{
					if (jeep->velocity)
					{
						LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT;
						LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
					}
				}
				break;
			}
			else if (!jeep->unknown2 || (--jeep->unknown2 != 0))
			{
				if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT && !jeep->velocity)
				{
					LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + 14;
				}
				if (LaraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START)
				{
					if (jeep->velocity)
					{
						LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT;
						LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
					}
				}
				break;
			}

			LaraItem->Animation.TargetState = JS_FWD_LEFT;
			LaraItem->Animation.ActiveState = JS_FWD_LEFT;
			LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_FWD_RIGHT;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;

			break;

		case JS_DRIVE_BACK:
			if (dead)
				LaraItem->Animation.TargetState = JS_STOP;
			else
//			if (jeep->velocity || JeepNoGetOff)
			{
				if (!(DbInput & IN_WALK))
				{
					if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
							jeep->unknown2++;
					}
					else if (!(TrInput & JEEP_IN_ACCELERATE) || TrInput & JEEP_IN_BRAKE)
					{
						if (TrInput & (IN_LEFT | IN_LSTEP))
							LaraItem->Animation.TargetState = JS_BACK_RIGHT;
						else if (TrInput & (IN_LEFT | IN_LSTEP))
							LaraItem->Animation.TargetState = JS_BACK_LEFT;
					}
					else
						LaraItem->Animation.TargetState = JS_BACK;
				}
				else
				{
					if (jeep->unknown2)
					{
						jeep->unknown2--;
						if (!jeep->unknown2)
							LaraItem->Animation.TargetState = JS_STOP;
					}
				}
			}
			break;

		default:
			break;
		}
	}
	if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
	{
		LaraItem->Animation.TargetState = JS_JUMP;
		LaraItem->HitPoints = 0;
		JeepExplode(item);
	}
}

void JeepCollision(short itemNumber, ItemInfo* l, CollisionInfo* coll)
{
	if (l->HitPoints > 0 && Lara.Vehicle == NO_ITEM)
	{
		ItemInfo* item = &g_Level.Items[itemNumber];

		if (GetOnJeep(itemNumber))
		{
			Lara.Vehicle = itemNumber;

			if (Lara.Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				CreateFlare(LaraItem, ID_FLARE_ITEM, 0);
				UndrawFlareMeshes(l);
				Lara.Flare.ControlLeft = 0;
				Lara.Control.Weapon.RequestGunType = LaraWeaponType::None;
				Lara.Control.Weapon.GunType = LaraWeaponType::None;
			}

			Lara.Control.HandStatus = HandStatus::Busy;

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
				EnableBaddyAI(v4, 1);
				*(_DWORD*)(v5 + 5610) = *(_DWORD*)(v5 + 5610) & 0xFFFFFFFB | 2;
				AddActiveItem(v4);
			}*/

			short ang = phd_atan(item->Pose.Position.z - LaraItem->Pose.Position.z, item->Pose.Position.x - LaraItem->Pose.Position.x);
			ang -= item->Pose.Orientation.y;

			if ((ang > -(ANGLE(45))) && (ang < (ANGLE(135))))
				LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_GETIN_LEFT;
			else
				LaraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_GETIN_RIGHT;

			LaraItem->Animation.TargetState = JS_GETIN;
			LaraItem->Animation.ActiveState = JS_GETIN;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;

			item->HitPoints = 1;
			LaraItem->Pose.Position.x = item->Pose.Position.x;
			LaraItem->Pose.Position.y = item->Pose.Position.y;
			LaraItem->Pose.Position.z = item->Pose.Position.z;
			LaraItem->Pose.Orientation.y = item->Pose.Orientation.y;

			ResetLaraFlex(LaraItem);
			Lara.HitDirection = -1;

			AnimateItem(l);

			int anim = LaraItem->Animation.AnimNumber;

			JeepInfo* jeep = (JeepInfo*)item->Data;
			jeep->revs = 0;
			jeep->unknown2 = 0;

			item->Flags |= TRIGGERED;
		}
		else
			ObjectCollision(itemNumber, l, coll);
	}
}

int JeepControl(void)
{
	ItemInfo* item = &g_Level.Items[Lara.Vehicle];
	JeepInfo* jeep = (JeepInfo*)item->Data;

	int drive = -1;
	bool dead = 0;

	int collide = JeepDynamics(item);

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	GameVector oldPos;
	oldPos.x = item->Pose.Position.x;
	oldPos.y = item->Pose.Position.y;
	oldPos.z = item->Pose.Position.z;

	int height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
	int ceiling = GetCeiling(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

	Vector3Int fl, fr, bc;
	int hfl = TestJeepHeight(item, JEEP_FRONT, -JEEP_SIDE, &fl);
	int hfr = TestJeepHeight(item, JEEP_FRONT, JEEP_SIDE, &fr);
	int hbc = TestJeepHeight(item, -(JEEP_FRONT + 50), 0, &bc);

	roomNumber = item->RoomNumber;
	floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

	TestTriggers(item, true);
	TestTriggers(item, false);

	if (LaraItem->HitPoints <= 0)
	{
		dead = true;
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
	}

	int pitch = 0;
	if (jeep->flags)
		collide = 0;
	else if (LaraItem->Animation.ActiveState == JS_GETIN)
	{
		drive = -1;
		collide = 0;
	}
	else
		drive = JeepUserControl(item, height, &pitch);

	if (jeep->velocity || jeep->revs)
	{
		jeep->pitch = pitch;
		if (pitch >= -32768)
		{
			if (pitch > 40960)
				jeep->pitch = 40960;
		}
		else
			jeep->pitch = -32768;

		SoundEffect(SFX_TR4_JEEP_MOVE, &item->Pose, (jeep->pitch * 256) + 16777220);
	}
	else
	{
		if (drive != -1)
			SoundEffect(SFX_TR4_JEEP_IDLE, &item->Pose, 0);
		jeep->pitch = 0;
	}

	item->Floor = height;
	short rotAdd = jeep->velocity / 4;
	jeep->rot1 -= rotAdd;
	jeep->rot2 -= rotAdd;
	jeep->rot3 -= rotAdd;
	jeep->rot4 -= rotAdd;

	int oldY = item->Pose.Position.y;
	item->Animation.VerticalVelocity = DoJeepDynamics(height, item->Animation.VerticalVelocity, &item->Pose.Position.y, 0);

	height = (fl.y + fr.y) / 2;
	short xRot;
	short zRot;
	if (bc.y >= hbc)
	{
		if (height >= (hfl + hfr) / 2)
			xRot = phd_atan(1100, hbc - height);
		else
			xRot = phd_atan(JEEP_FRONT, hbc - item->Pose.Position.y);
	}
	else
	{
		if (height >= (hfl + hfr) / 2)
			xRot = phd_atan(JEEP_FRONT, item->Pose.Position.y - height);
		else
		{
			xRot = -phd_atan(137, oldY - item->Pose.Position.y);
			if (jeep->velocity < 0)
				xRot = -xRot;
		}
	}

	item->Pose.Orientation.x += (xRot - item->Pose.Orientation.x) / 4;
	item->Pose.Orientation.z += (phd_atan(256, height - fl.y) - item->Pose.Orientation.z) / 4;
	if (jeep->velocity == 0)
	{
		item->Pose.Orientation.x = 0;
		item->Pose.Orientation.z = 0;
	}
	if (!(jeep->flags & JF_DEAD))
	{
		if (roomNumber != item->RoomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.ItemNumber, roomNumber);
		}

		LaraItem->Pose.Position.x = item->Pose.Position.x;
		LaraItem->Pose.Position.y = item->Pose.Position.y;
		LaraItem->Pose.Position.z = item->Pose.Position.z;
		LaraItem->Pose.Orientation.x = item->Pose.Orientation.x;
		LaraItem->Pose.Orientation.y = item->Pose.Orientation.y;
		LaraItem->Pose.Orientation.z = item->Pose.Orientation.z;

		int jeepAnim = Objects[ID_JEEP].animIndex;
		int laraAnim = LaraItem->Animation.AnimNumber;
		int extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;

		AnimateJeep(item, collide, dead);
		AnimateItem(LaraItem);

		item->Animation.AnimNumber = Objects[ID_JEEP].animIndex + LaraItem->Animation.AnimNumber - Objects[ID_JEEP_LARA_ANIMS].animIndex;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + (LaraItem->Animation.FrameNumber - g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase);

		jeepAnim = Objects[ID_JEEP].animIndex;
		laraAnim = LaraItem->Animation.AnimNumber;
		extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;


		Camera.targetElevation = -ANGLE(30);
		Camera.targetDistance = 2 * WALL_SIZE;

		if (jeep->unknown2)
		{
			if (jeep->unknown2 == 1)
				jeep->fallSpeed += ((32578 - jeep->fallSpeed) / 8);
		}
		else
			jeep->fallSpeed -= (jeep->fallSpeed / 8);

		Camera.targetAngle = jeep->fallSpeed;

		if (jeep->flags & JF_FALLING && item->Pose.Position.y == item->Floor)
		{
			LaraItem->MeshBits = 0;
			LaraItem->HitPoints = 0;
			LaraItem->Flags |= ONESHOT;
			JeepExplode(item);
			return 0;
		}
	}

	if (LaraItem->Animation.ActiveState == JS_GETIN ||
		LaraItem->Animation.ActiveState == JS_GETOFF)
		JeepSmokeStart = 0;
	else
	{
		short speed = 0;
		short angle = 0;

		Vector3Int pos;
		pos.x = 80;
		pos.y = 0;
		pos.z = -500;

		GetJointAbsPosition(item, &pos, 11);

		if (item->Animation.Velocity <= 32)
		{
			if (JeepSmokeStart >= 16)
			{
				if (GetRandomControl() & 3)
					speed = 0;
				else
					speed = ((GetRandomControl() & 0xF) + GetRandomControl() & 0x10) * 64;
			}
			else
			{
				speed = ((GetRandomControl() & 7) + GetRandomControl() & 0x10 + 2 * JeepSmokeStart) * 64;
				JeepSmokeStart++;
			}
			TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, item->Pose.Orientation.y + -32768, speed, 0);
		}
		else if (item->Animation.Velocity < 64)
			TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, item->Pose.Orientation.y - 32768, 64 - item->Animation.Velocity, 1);
	}

	return JeepCheckGetOff();
}