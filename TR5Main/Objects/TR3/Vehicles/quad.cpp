#include "framework.h"
#include "quad.h"
#include "lara.h"
#include "effects/effects.h"
#include "items.h"
#include "collide.h"
#include "camera.h"
#include "effects/tomb4fx.h"
#include "lara_flare.h"
#include "lara_one_gun.h"
#include "misc.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "animation.h"
#include "Sound/sound.h"
#include "Specific/prng.h"
#include "quad_info.h"
#include "item.h"
using std::vector;
using namespace TEN::Math::Random;
enum QUAD_EFFECTS_POSITIONS {
	EXHAUST_LEFT = 0,
	EXHAUST_RIGHT,
	BACKLEFT_TYRE,
	BACKRIGHT_TYRE,
	FRONTRIGHT_TYRE,
	FRONTLEFT_TYRE
};

enum QUAD_FLAGS {
	QUAD_FLAGS_DEAD = 0x80,
	QUAD_FLAGS_IS_FALLING = 0x40
};

enum QUAD_ANIM_STATES {
	QUAD_STATE_DRIVE = 1,
	QUAD_STATE_TURNL = 2,
	QUAD_STATE_SLOW = 5,
	QUAD_STATE_BRAKE = 6,
	QUAD_STATE_BIKEDEATH = 7,
	QUAD_STATE_FALL = 8,
	QUAD_STATE_GETONR = 9,
	QUAD_STATE_GETOFFR = 10,
	QUAD_STATE_HITBACK = 11,
	QUAD_STATE_HITFRONT = 12,
	QUAD_STATE_HITLEFT = 13,
	QUAD_STATE_HITRIGHT = 14,
	QUAD_STATE_STOP = 15,
	QUAD_STATE_LAND = 17,
	QUAD_STATE_STOPSLOWLY = 18,
	QUAD_STATE_FALLDEATH = 19,
	QUAD_STATE_FALLOFF = 20,
	QUAD_STATE_WHEELIE = 21,
	QUAD_STATE_TURNR = 22,
	QUAD_STATE_GETONL = 23,
	QUAD_STATE_GETOFFL = 24,
};

#define MAX_VELOCITY		0xA000
#define MIN_HANDBRAKE_SPEED	0x3000

#define BRAKE			0x0280

#define REVERSE_ACC		-0x0300
#define MAX_BACK		-0x3000

#define MAX_REVS		0xa000

#define TERMINAL_FALLSPEED 240
#define QUAD_SLIP 100
#define QUAD_SLIP_SIDE 50

#define QUAD_FRONT	550
#define QUAD_BACK  -550
#define QUAD_SIDE	260
#define QUAD_RADIUS	500
#define QUAD_HEIGHT	512

#define QUAD_HIT_LEFT  11
#define QUAD_HIT_RIGHT 12
#define QUAD_HIT_FRONT 13
#define QUAD_HIT_BACK  14

#define SMAN_SHOT_DAMAGE		10
#define SMAN_LARA_DAMAGE		50

#define DAMAGE_START  140
#define DAMAGE_LENGTH 14

#define GETOFF_DISTANCE 512	

#define QUAD_UNDO_TURN ANGLE(2)
#define QUAD_TURN (ANGLE(0.5) + QUAD_UNDO_TURN)
#define QUAD_MAX_TURN ANGLE(5)
#define QUAD_HTURN (ANGLE(0.75) + QUAD_UNDO_TURN)
#define QUAD_MAX_HTURN ANGLE(8)

#define MIN_MOMENTUM_TURN ANGLE(3)
#define MAX_MOMENTUM_TURN ANGLE(1.5)
#define QUAD_MAX_MOM_TURN ANGLE(150)

#define QUAD_FRONT 550
#define QUAD_SIDE 260
#define QUAD_RADIUS 500
#define QUAD_SNOW 500

#define QUAD_MAX_HEIGHT (STEP_SIZE)
#define QUAD_MIN_BOUNCE ((MAX_VELOCITY/2)/256)

#define QUADBIKE_TURNL_A		3
#define QUADBIKE_TURNL_F		GF2(ID_QUAD, QUADBIKE_TURNL_A, 0)
#define QUADBIKE_TURNR_A		20
#define QUADBIKE_TURNR_F		GF2(ID_QUAD, QUADBIKE_TURNR_A, 0)

#define QUADBIKE_FALLSTART_A	6
#define QUADBIKE_FALLSTART_F	GF2(ID_QUAD, QUADBIKE_FALLSTART_A, 0)
#define QUADBIKE_FALL_A		7
#define QUADBIKE_FALL_F		GF2(ID_QUAD, QUADBIKE_FALL_A, 0)
#define QUADBIKE_GETONR_A	9
#define QUADBIKE_GETONR_F	GF2(ID_QUAD, QUADBIKE_GETONR_A, 0)
#define Q_HITB_A			11
#define Q_HITF_A			12
#define Q_HITL_A			14
#define Q_HITR_A			13
#define QUADBIKE_GETONL_A	23
#define QUADBIKE_GETONL_F	GF2(ID_QUAD, QUADBIKE_GETONL_A, 0)
#define QUADBIKE_FALLSTART2_A	25

BITE_INFO quadEffectsPositions[6] = { 
	{ -56, -32, -380, 0	},
	{ 56, -32, -380, 0 },
	{ -8, 180, -48, 3 },
	{ 8, 180, -48, 4 },
	{ 90, 180, -32, 6 },
	{ -90, 180, -32, 7 }
};

bool QuadHandbrakeStarting;
bool QuadCanHandbrakeStart;
int QuadSmokeStart;
bool QuadNoGetOff;

static void QuadbikeExplode(ITEM_INFO* item)
{
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(item, 1);
	}
	else
	{
		TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);
	}
	auto pos = PHD_3DPOS(item->pos.xPos, item->pos.yPos - 128, item->pos.zPos, 0, item->pos.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	item->status = ITEM_DEACTIVATED;

	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	Lara.Vehicle = NO_ITEM;
}

static int CanQuadbikeGetOff(int direction)
{
	short angle;

	auto item = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = item->pos.yRot - ANGLE(90);
	else
		angle = item->pos.yRot + ANGLE(90);

	int x = item->pos.xPos + 512 * phd_sin(angle);
	int y = item->pos.yPos;
	int z = item->pos.zPos + 512 * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (collResult.Position.Slope || collResult.Position.Floor == NO_HEIGHT)
		return false;

	if (abs(collResult.Position.Floor - item->pos.yPos) > 512)
		return false;

	if ((collResult.Position.Ceiling - item->pos.yPos > -LARA_HEIGHT) || (collResult.Position.Floor - collResult.Position.Ceiling < LARA_HEIGHT))
		return false;

	return true;
}

static int QuadCheckGetOff()
{
	if (Lara.Vehicle == NO_ITEM)
		return true;

	ITEM_INFO* item = &g_Level.Items[Lara.Vehicle];

	if (((LaraItem->currentAnimState == 10) || (LaraItem->currentAnimState == 24)) && (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd))
	{
		ITEM_INFO* item = &g_Level.Items[Lara.Vehicle];

		if (((LaraItem->currentAnimState == 10) || (LaraItem->currentAnimState == 24)) && (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd))
		{
			if (LaraItem->currentAnimState == QUAD_STATE_GETOFFL)
				LaraItem->pos.yRot += ANGLE(90);
			else
				LaraItem->pos.yRot -= ANGLE(90);

			LaraItem->animNumber = LA_STAND_SOLID;
			LaraItem->frameNumber = GF(LaraItem->animNumber, 0);
			LaraItem->currentAnimState = LaraItem->goalAnimState = LS_STOP;
			LaraItem->pos.xPos -= GETOFF_DISTANCE * phd_sin(LaraItem->pos.yRot);
			LaraItem->pos.zPos -= GETOFF_DISTANCE * phd_cos(LaraItem->pos.yRot);
			LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
		}
		else if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
		{
			QUAD_INFO* quad;

			quad = (QUAD_INFO *)item->data;
			if (LaraItem->currentAnimState == QUAD_STATE_FALLOFF)
			{
				PHD_VECTOR pos = { 0, 0, 0 };

				LaraItem->animNumber = LA_FREEFALL;
				LaraItem->frameNumber = GF(LA_FREEFALL, 0);
				LaraItem->currentAnimState = LS_FREEFALL;

				GetJointAbsPosition(LaraItem, &pos, LM_HIPS);

				LaraItem->pos.xPos = pos.x;
				LaraItem->pos.yPos = pos.y;
				LaraItem->pos.zPos = pos.z;
				LaraItem->fallspeed = item->fallspeed;
				LaraItem->gravityStatus = true;
				LaraItem->pos.xRot = 0;
				LaraItem->pos.zRot = 0;
				LaraItem->hitPoints = 0;
				Lara.gunStatus = LG_NO_ARMS;
				item->flags |= ONESHOT;

				return false;
			}
			else if (LaraItem->currentAnimState == QUAD_STATE_FALLDEATH)
			{
				LaraItem->goalAnimState = LS_DEATH;
				LaraItem->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
				LaraItem->speed = 0;
				quad->flags |= QUAD_FLAGS_DEAD;

				return false;
			}
		}

		return true;
	}
	else
	{
		return true;
	}
}

static int GetOnQuadBike(short itemNumber, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if ((!(TrInput & IN_ACTION)) || (item->flags & ONESHOT) || (Lara.gunStatus != LG_NO_ARMS) || (LaraItem->gravityStatus)
		|| ((abs(item->pos.yPos - LaraItem->pos.yPos)) > 256))
		return 0;

	int dx = LaraItem->pos.xPos - item->pos.xPos;
	int dz = LaraItem->pos.zPos - item->pos.zPos;

	int distance = dx * dx + dz * dz;

	if (distance > 170000)
		return false;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < -32000)
		return false;
	else
	{
		short angle = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
		angle -= item->pos.yRot;

		if ((angle > -ANGLE(45)) && (angle < ANGLE(135)))
		{
			int tempAngle = LaraItem->pos.yRot - item->pos.yRot;
			if (tempAngle > ANGLE(45) && tempAngle < ANGLE(135))
				return true;
			else
				return false;
		}
		else
		{
			int tempAngle = LaraItem->pos.yRot - item->pos.yRot;
			if (tempAngle > ANGLE(225) && tempAngle < ANGLE(315))
				return true;
			else
				return false;
		}
	}

	return true;
}

static void QuadBaddieCollision(ITEM_INFO* quad)
{
	vector<short> roomsList;

	roomsList.push_back(quad->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[quad->roomNumber];
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
			if (item->collidable && item->status != ITEM_INVISIBLE && item != LaraItem && item != quad) 
			{
				OBJECT_INFO* object = &Objects[item->objectNumber];
				if (object->collision && object->intelligent)
				{
					int x = quad->pos.xPos - item->pos.xPos;
					int y = quad->pos.yPos - item->pos.yPos;
					int z = quad->pos.zPos - item->pos.zPos;
					if (x > -4096 && x < 4096 && z > -4096 && z < 4096 && y > -4096 && y < 4096)
					{
						if (TestBoundsCollide(item, quad, QUAD_RADIUS))
						{
							DoLotsOfBlood(item->pos.xPos, quad->pos.yPos - STEP_SIZE, item->pos.zPos, quad->speed, quad->pos.yRot, item->roomNumber, 3);
							item->hitPoints = 0;
						}
					}
				}
			}
			itemNum = item->nextItem;
		}
	}
}

static int GetQuadCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p)
{
	p->x = item->pos.xPos - p->x;
	p->z = item->pos.zPos - p->z;

	if (p->x || p->z)
	{
		float c = phd_cos(item->pos.yRot);
		float s = phd_sin(item->pos.yRot);
		int front = p->z * c + p->x * s;
		int side = -p->z * s + p->x * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return QUAD_HIT_BACK;
			else
				return QUAD_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return QUAD_HIT_LEFT;
			else
				return QUAD_HIT_RIGHT;
		}
	}

	return 0;
}

static int TestQuadHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - dz * phd_sin(item->pos.xRot) + dx * phd_sin(item->pos.zRot);

	float c = phd_cos(item->pos.yRot);
	float s = phd_sin(item->pos.yRot);

	pos->z = item->pos.zPos + dz * c - dx * s;
	pos->x = item->pos.xPos + dz * s + dx * c;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;
	
	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static int DoQuadShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int oldX= old->x / SECTOR(1);
	int oldZ = old->z / SECTOR(1);

	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			quad->pos.zPos += (old->z - pos->z);
			quad->pos.xPos += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			quad->pos.zPos -= shiftZ + 1;
			return (pos->x - quad->pos.xPos);
		}
		else
		{
			quad->pos.zPos += WALL_SIZE - shiftZ;
			return (quad->pos.xPos - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			quad->pos.xPos -= shiftX + 1;
			return (quad->pos.zPos - pos->z);
		}
		else
		{
			quad->pos.xPos += WALL_SIZE - shiftX;
			return (pos->z - quad->pos.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		short roomNumber = quad->roomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = quad->roomNumber;
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
			quad->pos.zPos += z;
			quad->pos.xPos += x;
		}
		else if (z)
		{
			quad->pos.zPos += z;
			if (z > 0)
				return (quad->pos.xPos - pos->x);
			else
				return (pos->x - quad->pos.xPos);
		}
		else if (x)
		{
			quad->pos.xPos += x;
			if (x > 0)
				return (pos->z - quad->pos.zPos);
			else
				return (quad->pos.zPos - pos->z);
		}
		else
		{
			quad->pos.zPos += (old->z - pos->z);
			quad->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

static int DoQuadDynamics(int height, int fallspeed, int *y)
{
	if (height > *y)
	{
		*y += fallspeed;
		if (*y > height - QUAD_MIN_BOUNCE)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += 6;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < -80)
			kick = -80;

		fallspeed += ((kick - fallspeed) / 8);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

static int QuadDynamics(ITEM_INFO* item)
{
	PHD_VECTOR moved, fl, fr, br, bl, mtl, mbl, mtr, mbr, mml, mmr;
	PHD_VECTOR old, oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight, mtl_old, moldBottomLeft, mtr_old, moldBottomRight, mml_old, mmr_old;
	int hfl, hfr, hbr, hbl, hmtl, hmbl, hmtr, hmbr, hmml, hmmr;
	int holdFrontRight, holdFrontLeft, holdBottomRight, holdBottomLeft, hmtl_old, hmoldBottomLeft, hmtr_old, hmoldBottomRight, hmml_old, hmmr_old;
	int slip, collide;
	short rot, rotadd;
	int newspeed;

	QuadNoGetOff = false;

	QUAD_INFO* quad = (QUAD_INFO*)item->data;

	holdFrontLeft = TestQuadHeight(item, QUAD_FRONT, -QUAD_SIDE, &oldFrontLeft);
	holdFrontRight = TestQuadHeight(item, QUAD_FRONT, QUAD_SIDE, &oldFrontRight);
	holdBottomLeft = TestQuadHeight(item, -QUAD_FRONT, -QUAD_SIDE, &oldBottomLeft);
	holdBottomRight = TestQuadHeight(item, -QUAD_FRONT, QUAD_SIDE, &oldBottomRight);
	hmml_old = TestQuadHeight(item, 0, -QUAD_SIDE, &mml_old);
	hmmr_old = TestQuadHeight(item, 0, QUAD_SIDE, &mmr_old);
	hmtl_old = TestQuadHeight(item, QUAD_FRONT / 2, -QUAD_SIDE, &mtl_old);
	hmtr_old = TestQuadHeight(item, QUAD_FRONT / 2, QUAD_SIDE, &mtr_old);
	hmoldBottomLeft = TestQuadHeight(item, -QUAD_FRONT / 2, -QUAD_SIDE, &moldBottomLeft);
	hmoldBottomRight = TestQuadHeight(item, -QUAD_FRONT / 2, QUAD_SIDE, &moldBottomRight);

	old.x = item->pos.xPos;
	old.y = item->pos.yPos;
	old.z = item->pos.zPos;

	if (oldBottomLeft.y > holdBottomLeft)
		oldBottomLeft.y = holdBottomLeft;
	if (oldBottomRight.y > holdBottomRight)
		oldBottomRight.y = holdBottomRight;
	if (oldFrontLeft.y > holdFrontLeft)
		oldFrontLeft.y = holdFrontLeft;
	if (oldFrontRight.y > holdFrontRight)
		oldFrontRight.y = holdFrontRight;
	if (moldBottomLeft.y > hmoldBottomLeft)
		moldBottomLeft.y = hmoldBottomLeft;
	if (moldBottomRight.y > hmoldBottomRight)
		moldBottomRight.y = hmoldBottomRight;
	if (mtl_old.y > hmtl_old)
		mtl_old.y = hmtl_old;
	if (mtr_old.y > hmtr_old)
		mtr_old.y = hmtr_old;
	if (mml_old.y > hmml_old)
		mml_old.y = hmml_old;
	if (mmr_old.y > hmmr_old)
		mmr_old.y = hmmr_old;


	if (item->pos.yPos > (item->floor - STEP_SIZE))
	{
		short momentum;

		if (quad->skidooTurn < -QUAD_UNDO_TURN)
			quad->skidooTurn += QUAD_UNDO_TURN;

		else if (quad->skidooTurn > QUAD_UNDO_TURN)
			quad->skidooTurn -= QUAD_UNDO_TURN;

		else
			quad->skidooTurn = 0;

		item->pos.yRot += quad->skidooTurn + quad->extraRotation;

		rot = item->pos.yRot - quad->momentumAngle;

		momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) * 256) / MAX_VELOCITY) * quad->velocity) / 256);
		if (!(TrInput & IN_ACTION) && quad->velocity > 0)
			momentum += (momentum / 4);

		if (rot < -MAX_MOMENTUM_TURN)
		{
			if (rot < -QUAD_MAX_MOM_TURN)
			{
				rot = -QUAD_MAX_MOM_TURN;
				quad->momentumAngle = item->pos.yRot - rot;
			}
			else
				quad->momentumAngle -= momentum;
		}
		else if (rot > MAX_MOMENTUM_TURN)
		{
			if (rot > QUAD_MAX_MOM_TURN)
			{
				rot = QUAD_MAX_MOM_TURN;
				quad->momentumAngle = item->pos.yRot - rot;
			}
			else
				quad->momentumAngle += momentum;
		}
		else
			quad->momentumAngle = item->pos.yRot;
	}

	else
		item->pos.yRot += quad->skidooTurn + quad->extraRotation;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int speed = 0;
	if (item->pos.yPos >= height)
		speed = item->speed * phd_cos(item->pos.xRot);
	else
		speed = item->speed;

	item->pos.zPos += speed * phd_cos(quad->momentumAngle);
	item->pos.xPos += speed * phd_sin(quad->momentumAngle);

	slip = QUAD_SLIP * phd_sin(item->pos.xRot);
	if (abs(slip) > QUAD_SLIP / 2)
	{
		if (slip > 0)
			slip -= 10;
		else
			slip += 10;
		item->pos.zPos -= slip * phd_cos(item->pos.yRot);
		item->pos.xPos -= slip * phd_sin(item->pos.yRot);
	}

	slip = QUAD_SLIP_SIDE * phd_sin(item->pos.zRot);
	if (abs(slip) > QUAD_SLIP_SIDE / 2)
	{
		item->pos.zPos -= slip * phd_sin(item->pos.yRot);
		item->pos.xPos += slip * phd_cos(item->pos.yRot);
	}

	moved.x = item->pos.xPos;
	moved.z = item->pos.zPos;

	if (!(item->flags & ONESHOT)) 
		QuadBaddieCollision(item);

	rot = 0;

	hfl = TestQuadHeight(item, QUAD_FRONT, -QUAD_SIDE, &fl);
	if (hfl < oldFrontLeft.y - STEP_SIZE)
		rot = DoQuadShift(item, &fl, &oldFrontLeft);

	hmtl = TestQuadHeight(item, QUAD_FRONT / 2, -QUAD_SIDE, &mtl);
	if (hmtl < mtl_old.y - STEP_SIZE)
		DoQuadShift(item, &mtl, &mtl_old);

	hmml = TestQuadHeight(item, 0, -QUAD_SIDE, &mml);
	if (hmml < mml_old.y - STEP_SIZE)
		DoQuadShift(item, &mml, &mml_old);

	hmbl = TestQuadHeight(item, -QUAD_FRONT / 2, -QUAD_SIDE, &mbl);
	if (hmbl < moldBottomLeft.y - STEP_SIZE)
		DoQuadShift(item, &mbl, &moldBottomLeft);

	hbl = TestQuadHeight(item, -QUAD_FRONT, -QUAD_SIDE, &bl);
	if (hbl < oldBottomLeft.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(item, &bl, &oldBottomLeft);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	hfr = TestQuadHeight(item, QUAD_FRONT, QUAD_SIDE, &fr);
	if (hfr < oldFrontRight.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(item, &fr, &oldFrontRight);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	hmtr = TestQuadHeight(item, QUAD_FRONT / 2, QUAD_SIDE, &mtr);
	if (hmtr < mtr_old.y - STEP_SIZE)
		DoQuadShift(item, &mtr, &mtr_old);

	hmmr = TestQuadHeight(item, 0, QUAD_SIDE, &mmr);
	if (hmmr < mmr_old.y - STEP_SIZE)
		DoQuadShift(item, &mmr, &mmr_old);

	hmbr = TestQuadHeight(item, -QUAD_FRONT / 2, QUAD_SIDE, &mbr);
	if (hmbr < moldBottomRight.y - STEP_SIZE)
		DoQuadShift(item, &mbr, &moldBottomRight);

	hbr = TestQuadHeight(item, -QUAD_FRONT, QUAD_SIDE, &br);
	if (hbr < oldBottomRight.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(item, &br, &oldBottomRight);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (height < item->pos.yPos - STEP_SIZE)
		DoQuadShift(item, (PHD_VECTOR *)&item->pos, &old);

	quad->extraRotation = rot;

	collide = GetQuadCollisionAnim(item, &moved);

	if (collide)
	{
		newspeed = (item->pos.zPos - old.z) * phd_cos(quad->momentumAngle) + (item->pos.xPos - old.x) * phd_sin(quad->momentumAngle);

		newspeed *= 256;

		if ((&g_Level.Items[Lara.Vehicle] == item)
			&& (quad->velocity == MAX_VELOCITY)
			&& (newspeed < (quad->velocity - 10)))
		{
			LaraItem->hitPoints -= ((quad->velocity - newspeed) / 128);
			LaraItem->hitStatus = 1;
		}

		if (quad->velocity > 0 && newspeed < quad->velocity)
			quad->velocity = (newspeed < 0) ? 0 : newspeed;

		else if (quad->velocity < 0 && newspeed > quad->velocity)
			quad->velocity = (newspeed > 0) ? 0 : newspeed;

		if (quad->velocity < MAX_BACK)
			quad->velocity = MAX_BACK;
	}

	return collide;
}

static void AnimateQuadBike(ITEM_INFO* item, int collide, int dead)
{
	QUAD_INFO* quad = (QUAD_INFO *)item->data;

	if ((item->pos.yPos != item->floor)
		&& (LaraItem->currentAnimState != QUAD_STATE_FALL)
		&& (LaraItem->currentAnimState != QUAD_STATE_LAND)
		&& (LaraItem->currentAnimState != QUAD_STATE_FALLOFF)
		&& (!dead))
	{
		if (quad->velocity < 0)
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_FALLSTART_A;
		else
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_FALLSTART2_A;

		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_FALL;
	}
	else if ((collide)
		&& (LaraItem->currentAnimState != QUAD_STATE_HITFRONT)
		&& (LaraItem->currentAnimState != QUAD_STATE_HITBACK)
		&& (LaraItem->currentAnimState != QUAD_STATE_HITLEFT)
		&& (LaraItem->currentAnimState != QUAD_STATE_HITRIGHT)
		&& (LaraItem->currentAnimState != QUAD_STATE_FALLOFF)
		&& (quad->velocity > (MAX_VELOCITY / 3))
		&& (!dead))
	{
		if (collide == QUAD_HIT_FRONT)
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + Q_HITF_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_HITFRONT;
		}
		else if (collide == QUAD_HIT_BACK)
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + Q_HITB_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_HITBACK;
		}
		else if (collide == QUAD_HIT_LEFT)
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + Q_HITL_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_HITLEFT;
		}
		else
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + Q_HITR_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_HITRIGHT;
		}

		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &item->pos, 0);
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case QUAD_STATE_STOP:
			if (dead)
				LaraItem->goalAnimState = QUAD_STATE_BIKEDEATH;

			else if (TrInput & IN_ROLL && quad->velocity == 0 && !QuadNoGetOff)
			{
				if ((TrInput & IN_RIGHT) && CanQuadbikeGetOff(1))
					LaraItem->goalAnimState = QUAD_STATE_GETOFFR;
				else if ((TrInput & IN_LEFT) && CanQuadbikeGetOff(-1))
					LaraItem->goalAnimState = QUAD_STATE_GETOFFL;
			}

			else if (TrInput & (IN_ACTION | IN_JUMP))
			{
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
			}

			break;

		case QUAD_STATE_DRIVE:
			if (dead)
			{
				if (quad->velocity > (MAX_VELOCITY / 2))
					LaraItem->goalAnimState = QUAD_STATE_FALLDEATH;
				else
					LaraItem->goalAnimState = QUAD_STATE_BIKEDEATH;
			}
			else if (((quad->velocity / 256) == 0)
				&& (!(TrInput & (IN_ACTION | IN_JUMP))))
				LaraItem->goalAnimState = QUAD_STATE_STOP;

			else if (TrInput & IN_LEFT && !QuadHandbrakeStarting)
				LaraItem->goalAnimState = QUAD_STATE_TURNL;

			else if (TrInput & IN_RIGHT && !QuadHandbrakeStarting)
				LaraItem->goalAnimState = QUAD_STATE_TURNR;

			else if (TrInput & IN_JUMP)
			{
				if (quad->velocity > ((MAX_VELOCITY / 3) * 2))
					LaraItem->goalAnimState = QUAD_STATE_BRAKE;
				else
					LaraItem->goalAnimState = QUAD_STATE_SLOW;
			}
			break;

		case QUAD_STATE_BRAKE:
		case QUAD_STATE_SLOW:
		case QUAD_STATE_STOPSLOWLY:
			if ((quad->velocity / 256) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;

			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = QUAD_STATE_TURNL;

			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = QUAD_STATE_TURNR;
			break;

		case QUAD_STATE_TURNL:
			if ((quad->velocity / 256) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;
			else if (TrInput & IN_RIGHT)
			{
				LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_TURNR_A;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_TURNR;
			}
			else if (!(TrInput & IN_LEFT))
			{
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
			}
			break;

		case QUAD_STATE_TURNR:
			if ((quad->velocity / 256) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;
			else if (TrInput & IN_LEFT)
			{
				LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_TURNL_A;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_TURNL;
			}
			else if (!(TrInput & IN_RIGHT))
			{
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
			}
			break;

		case QUAD_STATE_FALL:
			if (item->pos.yPos == item->floor)
				LaraItem->goalAnimState = QUAD_STATE_LAND;

			else if (item->fallspeed > TERMINAL_FALLSPEED)
				quad->flags |= QUAD_FLAGS_IS_FALLING;
			break;

		case QUAD_STATE_FALLOFF:
			break;

		case QUAD_STATE_HITFRONT:
		case QUAD_STATE_HITBACK:
		case QUAD_STATE_HITLEFT:
		case QUAD_STATE_HITRIGHT:
			if (TrInput & (IN_ACTION | IN_JUMP))
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
			break;
		}

		if (g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
		{
			LaraItem->goalAnimState = QUAD_STATE_FALLOFF;
			LaraItem->pos.yPos = item->pos.yPos + 700;
			LaraItem->roomNumber = item->roomNumber;
			LaraItem->hitPoints = 0;
			QuadbikeExplode(item);
		}
	}
}

static int QuadUserControl(ITEM_INFO* item, int height, int* pitch)
{
	bool drive = false;
	int revs = 0;

	QUAD_INFO* quad = (QUAD_INFO *)item->data;

	if (!quad->velocity && !(TrInput & (IN_DUCK | IN_SPRINT)) && !QuadCanHandbrakeStart)
		QuadCanHandbrakeStart = true;
	else if (quad->velocity)
		QuadCanHandbrakeStart = false;

	if (!(TrInput & (IN_DUCK | IN_SPRINT)))
		QuadHandbrakeStarting = false;

	if (!QuadHandbrakeStarting)
	{
		if (quad->revs > 0x10)
		{
			quad->velocity += (quad->revs / 16);
			quad->revs -= (quad->revs / 8);
		}
		else
			quad->revs = 0;
	}

	if (item->pos.yPos >= (height - STEP_SIZE))
	{
		if ((!quad->velocity) && (TrInput & IN_LOOK))
			LookUpDown();

		if (quad->velocity > 0)
		{
			if ((TrInput & (IN_DUCK | IN_SPRINT)) && !QuadHandbrakeStarting && quad->velocity > MIN_HANDBRAKE_SPEED)
			{
				if (TrInput & IN_LEFT)
				{
					quad->skidooTurn -= QUAD_HTURN;
					if (quad->skidooTurn < -QUAD_MAX_HTURN)
						quad->skidooTurn = -QUAD_MAX_HTURN;
				}
				else if (TrInput & IN_RIGHT)
				{
					quad->skidooTurn += QUAD_HTURN;
					if (quad->skidooTurn > QUAD_MAX_HTURN)
						quad->skidooTurn = QUAD_MAX_HTURN;
				}
			}
			else
			{
				if (TrInput & IN_LEFT)
				{
					quad->skidooTurn -= QUAD_TURN;
					if (quad->skidooTurn < -QUAD_MAX_TURN)
						quad->skidooTurn = -QUAD_MAX_TURN;
				}
				else if (TrInput & IN_RIGHT)
				{
					quad->skidooTurn += QUAD_TURN;
					if (quad->skidooTurn > QUAD_MAX_TURN)
						quad->skidooTurn = QUAD_MAX_TURN;
				}
			}
		}
		else if (quad->velocity < 0)
		{
			if ((TrInput & (IN_DUCK | IN_SPRINT)) && !QuadHandbrakeStarting && quad->velocity < -MIN_HANDBRAKE_SPEED + 0x800)
			{
				if (TrInput & IN_RIGHT)
				{
					quad->skidooTurn -= QUAD_HTURN;
					if (quad->skidooTurn < -QUAD_MAX_HTURN)
						quad->skidooTurn = -QUAD_MAX_HTURN;
				}
				else if (TrInput & IN_LEFT)
				{
					quad->skidooTurn += QUAD_HTURN;
					if (quad->skidooTurn > QUAD_MAX_HTURN)
						quad->skidooTurn = QUAD_MAX_HTURN;
				}
			}
			else
			{
				if (TrInput & IN_RIGHT)
				{
					quad->skidooTurn -= QUAD_TURN;
					if (quad->skidooTurn < -QUAD_MAX_TURN)
						quad->skidooTurn = -QUAD_MAX_TURN;
				}
				else if (TrInput & IN_LEFT)
				{
					quad->skidooTurn += QUAD_TURN;
					if (quad->skidooTurn > QUAD_MAX_TURN)
						quad->skidooTurn = QUAD_MAX_TURN;
				}
			}
		}
		
		if (TrInput & IN_JUMP)
		{
			if ((TrInput & (IN_DUCK|IN_SPRINT)) && (QuadCanHandbrakeStart || QuadHandbrakeStarting))
			{
				QuadHandbrakeStarting = true;
				quad->revs -= 0x200;
				if (quad->revs < MAX_BACK)
					quad->revs = MAX_BACK;
			}
			else if (quad->velocity > 0)
				quad->velocity -= BRAKE;
			else
			{
				if (quad->velocity > MAX_BACK)
					quad->velocity += REVERSE_ACC;
			}
		}
		else if (TrInput & IN_ACTION)
		{
			if ((TrInput & (IN_DUCK | IN_SPRINT)) && (QuadCanHandbrakeStart || QuadHandbrakeStarting))
			{
				QuadHandbrakeStarting = true;
				quad->revs += 0x200;
				if (quad->revs >= MAX_VELOCITY)
					quad->revs = MAX_VELOCITY;
			}
			else if (quad->velocity < MAX_VELOCITY)
			{
				if (quad->velocity < 0x4000)
					quad->velocity += (8 + ((0x4000 + 0x800 - quad->velocity)) / 8);
				else if (quad->velocity < 0x7000)
					quad->velocity += (4 + ((0x7000 + 0x800 - quad->velocity)) / 16);
				else if (quad->velocity < MAX_VELOCITY)
					quad->velocity += (2 + ((MAX_VELOCITY - quad->velocity)) / 8);
			}
			else
				quad->velocity = MAX_VELOCITY;

			quad->velocity -= ((abs(item->pos.yRot - quad->momentumAngle)) / 64);
		}

		else if (quad->velocity > 0x0100)
		{
			quad->velocity -= 0x0100;
		}
		else if (quad->velocity < -0x0100)
		{
			quad->velocity += 0x0100;
		}
		else
		{
			quad->velocity = 0;
		}

		if (QuadHandbrakeStarting && quad->revs && !(TrInput & (IN_ACTION | IN_JUMP)))
		{
			if (quad->revs > 0x8)
				quad->revs -= (quad->revs / 8);
			else
				quad->revs = 0;
		}

		item->speed = (quad->velocity / 256);

		if (quad->engineRevs > 0x7000)
			quad->engineRevs = -0x2000;

		if (quad->velocity < 0)
			revs = abs(quad->velocity / 2);
		else if (quad->velocity < 0x7000)
			revs = -0x2000 + ((quad->velocity * (0x6800 - -0x2000)) / 0x7000);
		else if (quad->velocity <= MAX_VELOCITY)
			revs = -0x2800 + (((quad->velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000));

		revs += abs(quad->revs);
		quad->engineRevs += ((revs - quad->engineRevs) / 8);
	}
	else
	{
		if (quad->engineRevs < 0xA000)
			quad->engineRevs += ((0xA000 - quad->engineRevs) / 8);
	}

	*pitch = quad->engineRevs;

	return drive;
}

void InitialiseQuadBike(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	item->data = QUAD_INFO();
	QUAD_INFO* quad = item->data;

	quad->velocity = 0;

	quad->skidooTurn = 0;
	quad->leftFallspeed = quad->rightFallspeed = 0;
	quad->momentumAngle = item->pos.yRot;
	quad->extraRotation = 0;
	quad->trackMesh = 0;
	quad->pitch = 0;
	quad->flags = 0;
}

void QuadBikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	int geton;

	if ((LaraItem->hitPoints < 0)
		|| (Lara.Vehicle != NO_ITEM))
		return;

	if ((geton = GetOnQuadBike(itemNumber, coll)))
	{
		short ang;

		Lara.Vehicle = itemNumber;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(ID_FLARE_ITEM, 0);
			undraw_flare_meshes();
			Lara.flareControlLeft = 0;
			Lara.requestGunType = Lara.gunType = WEAPON_NONE;
		}

		Lara.gunStatus = LG_HANDS_BUSY;

		ITEM_INFO* item = &g_Level.Items[itemNumber];

		ang = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
		ang -= item->pos.yRot;

		if ((ang > -(ONE_DEGREE * 45)) && (ang < (ONE_DEGREE * 135)))
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_GETONL_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_GETONL;
		}
		else
		{
			LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_GETONR_A;
			LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_GETONR;
		}

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

		QUAD_INFO* quad = (QUAD_INFO*)item->data;
		quad->revs = 0;
	}
	else
		ObjectCollision(itemNumber, l, coll);
}

static void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = true;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;

	spark->dR = 96;
	spark->dG = 96;
	spark->dB = 128;

	if (moving)
	{
		spark->dR = (spark->dR * speed) / 32;
		spark->dG = (spark->dG * speed) / 32;
		spark->dB = (spark->dB * speed) / 32;
	}

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->sLife = spark->life = (GetRandomControl() & 3) + 20 - (speed / 4096);
	if (spark->sLife < 9)
		spark->sLife = spark->life = 9;

	spark->extras = 0;
	spark->dynamic = -1;

	spark->x = x + ((GetRandomControl() & 15) - 8);
	spark->y = y + ((GetRandomControl() & 15) - 8);
	spark->z = z + ((GetRandomControl() & 15) - 8);
	int zv = speed * phd_cos(angle) / 4;
	int xv = speed * phd_sin(angle) / 4;
	spark->xVel = xv + ((GetRandomControl() & 255) - 128);
	spark->yVel = -(GetRandomControl() & 7) - 8;
	spark->zVel = zv + ((GetRandomControl() & 255) - 128);
	spark->friction = 4;

	if (GetRandomControl() & 1)
	{
		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			spark->rotAdd = -(GetRandomControl() & 7) - 24;
		else
			spark->rotAdd = (GetRandomControl() & 7) + 24;
	}
	else
		spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	spark->scalar = 2;
	spark->gravity = -(GetRandomControl() & 3) - 4;
	spark->maxYvel = -(GetRandomControl() & 7) - 8;
	int size = (GetRandomControl() & 7) + 64 + (speed / 128);
	spark->dSize = size;
	spark->size = spark->sSize =  size / 2;
}

int QuadBikeControl(void)
{
	short xRot, zRot, rotadd;
	int pitch, dead = 0;

	ITEM_INFO* item = &g_Level.Items[Lara.Vehicle];
	QUAD_INFO* quad = (QUAD_INFO *)item->data;

	GAME_VECTOR	oldpos;
	oldpos.x = item->pos.xPos;
	oldpos.y = item->pos.yPos;
	oldpos.z = item->pos.zPos;
	oldpos.roomNumber = item->roomNumber;

	bool collide = QuadDynamics(item);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	PHD_VECTOR fl;
	PHD_VECTOR fr;
	int hfl = TestQuadHeight(item, QUAD_FRONT, -QUAD_SIDE, &fl);
	int hfr = TestQuadHeight(item, QUAD_FRONT, QUAD_SIDE, &fr);

	TestTriggers(item, false);

	if (LaraItem->hitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = 1;
	}

	int drive = -1;

	if (quad->flags)
		collide = false;
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case QUAD_STATE_GETONL:
		case QUAD_STATE_GETONR:
		case QUAD_STATE_GETOFFL:
		case QUAD_STATE_GETOFFR:
			drive = -1;
			collide = false;
			break;

		default:
			drive = QuadUserControl(item, height, &pitch);
			break;
		}
	}

	if (quad->velocity || quad->revs)
	{
		int absvel = abs(quad->velocity) + 1;
		quad->pitch = pitch;
		if (quad->pitch < -0x8000)
			quad->pitch = -0x8000;
		else if (quad->pitch > 0xA000)
			quad->pitch = 0xA000;
		SoundEffect(SFX_TR3_QUAD_MOVE, &item->pos, 0, 0.5f + (float)abs(quad->pitch) / (float)MAX_VELOCITY);
	}
	else
	{
		if (drive != -1)
			SoundEffect(SFX_TR3_QUAD_IDLE, &item->pos, 0);
		quad->pitch = 0;
	}

	item->floor = height;

	rotadd = quad->velocity / 4;
	quad->rearRot -= rotadd;
	quad->rearRot -= (quad->revs / 8);
	quad->frontRot -= rotadd;

	quad->leftFallspeed = DoQuadDynamics(hfl, quad->leftFallspeed, (int *)&fl.y);
	quad->rightFallspeed = DoQuadDynamics(hfr, quad->rightFallspeed, (int *)&fr.y);
	item->fallspeed = DoQuadDynamics(height, item->fallspeed, (int *)&item->pos.yPos);

	height = (fl.y + fr.y) / 2;
	xRot = phd_atan(QUAD_FRONT, item->pos.yPos - height);
	zRot = phd_atan(QUAD_SIDE, height - fl.y);

	item->pos.xRot += ((xRot - item->pos.xRot) / 2);
	item->pos.zRot += ((zRot - item->pos.zRot) / 2);

	if (!(quad->flags & QUAD_FLAGS_DEAD))
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

		AnimateQuadBike(item, collide, dead);
		AnimateItem(LaraItem);

		item->animNumber = Objects[ID_QUAD].animIndex + (LaraItem->animNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);

		Camera.targetElevation = -30 * ONE_DEGREE;

		if (quad->flags & QUAD_FLAGS_IS_FALLING)
		{
			if (item->pos.yPos == item->floor)
			{
				ExplodingDeath(Lara.itemNumber, 0xffffffff, 1);
				LaraItem->hitPoints = 0;
				LaraItem->flags |= ONESHOT;
				QuadbikeExplode(item);
				return 0;
			}
		}
	}

	if (LaraItem->currentAnimState != QUAD_STATE_GETONR && LaraItem->currentAnimState != QUAD_STATE_GETONL &&
		LaraItem->currentAnimState != QUAD_STATE_GETOFFR && LaraItem->currentAnimState != QUAD_STATE_GETOFFL)
	{
		PHD_VECTOR pos;
		int speed = 0;
		short angle = 0;
		
		for (int i = 0; i < 2; i++)
		{
			pos.x = quadEffectsPositions[i].x;
			pos.y = quadEffectsPositions[i].y;
			pos.z = quadEffectsPositions[i].z;
			GetJointAbsPosition(item, &pos, quadEffectsPositions[i].meshNum);
			angle = item->pos.yRot + ((i == 0) ? 0x9000 : 0x7000);
			if (item->speed > 32)
			{
				if (item->speed < 64)
				{
					speed = 64 - item->speed;
					TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
				}
			}
			else
			{
				if (QuadSmokeStart < 16)
				{
					speed = ((QuadSmokeStart * 2) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) * 128;
					QuadSmokeStart++;
				}
				else if (QuadHandbrakeStarting)
					speed = (abs(quad->revs) * 2) + ((GetRandomControl() & 7) * 128);
				else if ((GetRandomControl() & 3) == 0)
					speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) * 128;
				else
					speed = 0;
				TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
			}
		}
	}
	else
	{
		QuadSmokeStart = 0;
	}

	return QuadCheckGetOff();
}
