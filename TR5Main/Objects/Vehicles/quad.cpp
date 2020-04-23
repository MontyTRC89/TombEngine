#include "../newobjects.h"
#include "../../Game/lara.h"
#include "../../Game/effect2.h"
#include "../../Game/items.h"
#include "../../Game/sphere.h"
#include "../../Game/collide.h"
#include "../../Game/camera.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effects.h"
#include "../../Game/laraflar.h"
#include <vector>
#include "../../Game/lara1gun.h"
#include "../../Game/misc.h"
#include "../../Specific/setup.h"
#include "..\..\Specific\level.h"
#include "../../Specific/input.h"

using namespace std;

typedef enum QUAD_EFFECTS_POSITIONS {
	EXHAUST_LEFT = 0,
	EXHAUST_RIGHT,
	BACKLEFT_TYRE,
	BACKRIGHT_TYRE,
	FRONTRIGHT_TYRE,
	FRONTLEFT_TYRE
};

typedef enum QUAD_FLAGS {
	QUAD_FLAGS_DEAD = 0x80,
	QUAD_FLAGS_IS_FALLING = 0x40
};

typedef enum QUAD_ANIM_STATES {
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
#define QUAD_MIN_BOUNCE ((MAX_VELOCITY/2)>>8)

#define QUADBIKE_TURNL_A		3
#define QUADBIKE_TURNL_F		GF2(ID_QUADBIKE, QUADBIKE_TURNL_A, 0)
#define QUADBIKE_TURNR_A		20
#define QUADBIKE_TURNR_F		GF2(ID_QUADBIKE, QUADBIKE_TURNR_A, 0)

#define QUADBIKE_FALLSTART_A	6
#define QUADBIKE_FALLSTART_F	GF2(ID_QUADBIKE, QUADBIKE_FALLSTART_A, 0)
#define QUADBIKE_FALL_A		7
#define QUADBIKE_FALL_F		GF2(ID_QUADBIKE, QUADBIKE_FALL_A, 0)
#define QUADBIKE_GETONR_A	9
#define QUADBIKE_GETONR_F	GF2(ID_QUADBIKE, QUADBIKE_GETONR_A, 0)
#define Q_HITB_A			11
#define Q_HITF_A			12
#define Q_HITL_A			14
#define Q_HITR_A			13
#define QUADBIKE_GETONL_A	23
#define QUADBIKE_GETONL_F	GF2(ID_QUADBIKE, QUADBIKE_GETONL_A, 0)
#define QUADBIKE_FALLSTART2_A	25

BITE_INFO quadEffectsPositions[6] = { 
	{ -56, -32, -380, 0	},	// Left smoke
	{ 56, -32, -380, 0 },	// Right smoke
	{ -8, 180, -48, 3 },	// Left back
	{ 8, 180, -48, 4 },	// Right back
	{ 90, 180, -32, 6 },	// Right front
	{ -90, 180, -32, 7 }	// Left front
};

bool QuadHandbrakeStarting;
bool QuadCanHandbrakeStart;
int QuadSmokeStart;
bool QuadNoGetOff;

extern LaraExtraInfo g_LaraExtra;

void QuadbikeExplode(ITEM_INFO* item)
{
	if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(item);
	}
	else
	{
		TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);
	}

	ExplodingDeath(g_LaraExtra.Vehicle, 0xfffffffe, 1);
	KillItem(g_LaraExtra.Vehicle);
	item->status = ITEM_DEACTIVATED;

	SoundEffect(SFX_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_EXPLOSION2, NULL, 0);

	g_LaraExtra.Vehicle = NO_ITEM;
}

int CanQuadbikeGetOff(int direction)
{
	short angle;

	ITEM_INFO* item = &Items[g_LaraExtra.Vehicle];

	if (direction < 0)
		angle = item->pos.yRot - ANGLE(90);
	else
		angle = item->pos.yRot + ANGLE(90);

	int x = item->pos.xPos + (512 * SIN(angle) >> W2V_SHIFT);
	int y = item->pos.yPos;
	int z = item->pos.zPos + (512 * COS(angle) >> W2V_SHIFT);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);

	if ((HeightType == BIG_SLOPE) || (HeightType == DIAGONAL) || (height == NO_HEIGHT))
		return false;

	if (abs(height - item->pos.yPos) > 512)
		return false;

	int ceiling = GetCeiling(floor, x, y, z);

	if ((ceiling - item->pos.yPos > -LARA_HITE) || (height - ceiling < LARA_HITE))
		return false;

	return true;
}

int QuadCheckGetOff()
{
	ITEM_INFO* item = &Items[g_LaraExtra.Vehicle];

	if (((LaraItem->currentAnimState == 10) || (LaraItem->currentAnimState == 24)) && (LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd))
	{
		if (LaraItem->currentAnimState == QUAD_STATE_GETOFFL)
			LaraItem->pos.yRot += ANGLE(90);
		else
			LaraItem->pos.yRot -= ANGLE(90);

		LaraItem->animNumber = ANIMATION_LARA_STAY_SOLID;
		LaraItem->frameNumber = GF(LaraItem->animNumber, 0);
		LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_LARA_STOP;
		LaraItem->pos.xPos -= GETOFF_DISTANCE * SIN(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.zPos -= GETOFF_DISTANCE * COS(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		g_LaraExtra.Vehicle = NO_ITEM;
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd)
	{
		QUAD_INFO* quad;

		quad = (QUAD_INFO *)item->data;
		if (LaraItem->currentAnimState == QUAD_STATE_FALLOFF)
		{
			PHD_VECTOR pos = { 0, 0, 0 };

			LaraItem->animNumber = ANIMATION_LARA_FREE_FALL_LONG;
			LaraItem->frameNumber = GF(ANIMATION_LARA_FREE_FALL_LONG, 0);
			LaraItem->currentAnimState = STATE_LARA_FREEFALL;

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
			LaraItem->goalAnimState = STATE_LARA_DEATH;
			LaraItem->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
			LaraItem->speed = 0;
			quad->flags |= QUAD_FLAGS_DEAD;

			return false;
		}
	}

	return true;
}

int GetOnQuadBike(short itemNumber, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNumber];

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
		short angle = ATAN(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
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

void QuadBaddieCollision(ITEM_INFO* quad)
{
	vector<short> roomsList;

	roomsList.push_back(quad->roomNumber);

	short* door = Rooms[quad->roomNumber].door;
	if (door)
	{
		short numDoors = *door;
		door++;
		for (int i = 0; i < numDoors; i++)
		{
			roomsList.push_back(*door);
			door += 16;
		}
	}

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNum = Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &Items[itemNum];
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

int GetQuadCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p)
{
	p->x = item->pos.xPos - p->x;
	p->z = item->pos.zPos - p->z;

	if (p->x || p->z)
	{
		int c = COS(item->pos.yRot);
		int s = SIN(item->pos.yRot);
		int front = ((p->z * c) + (p->x * s)) >> W2V_SHIFT;
		int side = ((-p->z * s) + (p->x * c)) >> W2V_SHIFT;

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

int TestQuadHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - (dz * SIN(item->pos.xRot) >> W2V_SHIFT) + (dx * SIN(item->pos.zRot) >> W2V_SHIFT);

	int c = COS(item->pos.yRot);
	int s = SIN(item->pos.yRot);

	pos->z = item->pos.zPos + ((dz * c - dx * s) >> W2V_SHIFT);
	pos->x = item->pos.xPos + ((dz * s + dx * c) >> W2V_SHIFT);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;
	
	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

int DoQuadShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x >> WALL_SHIFT;
	int z = pos->z >> WALL_SHIFT;

	int  oldX= old->x >> WALL_SHIFT;
	int oldZ = old->z >> WALL_SHIFT;

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

int DoQuadDynamics(int height, int fallspeed, int *y)
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
		int kick = (height - *y) << 2;

		if (kick < -80)
			kick = -80;

		fallspeed += ((kick - fallspeed) >> 3);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int QuadDynamics(ITEM_INFO* item)
{
	/* Does all skidoo movement and collision and returns if collide value */
	PHD_VECTOR moved, fl, fr, br, bl, mtl, mbl, mtr, mbr, mml, mmr;
	PHD_VECTOR old, oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight, mtl_old, moldBottomLeft, mtr_old, moldBottomRight, mml_old, mmr_old;
	int hfl, hfr, hbr, hbl, hmtl, hmbl, hmtr, hmbr, hmml, hmmr;
	int holdFrontRight, holdFrontLeft, holdBottomRight, holdBottomLeft, hmtl_old, hmoldBottomLeft, hmtr_old, hmoldBottomRight, hmml_old, hmmr_old;
	int slip, collide;
	short rot, rotadd;
	int newspeed;

	QuadNoGetOff = false;

	QUAD_INFO* quad = (QUAD_INFO*)item->data;

	/* First get positions and heights of skidoo's corners + centre */
	holdFrontLeft = TestQuadHeight(item, QUAD_FRONT, -QUAD_SIDE, &oldFrontLeft);
	holdFrontRight = TestQuadHeight(item, QUAD_FRONT, QUAD_SIDE, &oldFrontRight);
	holdBottomLeft = TestQuadHeight(item, -QUAD_FRONT, -QUAD_SIDE, &oldBottomLeft);
	holdBottomRight = TestQuadHeight(item, -QUAD_FRONT, QUAD_SIDE, &oldBottomRight);
	hmml_old = TestQuadHeight(item, 0, -QUAD_SIDE, &mml_old);
	hmmr_old = TestQuadHeight(item, 0, QUAD_SIDE, &mmr_old);
	hmtl_old = TestQuadHeight(item, QUAD_FRONT >> 1, -QUAD_SIDE, &mtl_old);
	hmtr_old = TestQuadHeight(item, QUAD_FRONT >> 1, QUAD_SIDE, &mtr_old);
	hmoldBottomLeft = TestQuadHeight(item, -QUAD_FRONT >> 1, -QUAD_SIDE, &moldBottomLeft);
	hmoldBottomRight = TestQuadHeight(item, -QUAD_FRONT >> 1, QUAD_SIDE, &moldBottomRight);

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

		momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) << 8) / MAX_VELOCITY) * quad->velocity) >> 8);
		if (!(TrInput & IN_ACTION) && quad->velocity > 0)
			momentum += momentum >> 2;

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
		speed = (item->speed * COS(item->pos.xRot)) >> W2V_SHIFT;
	else
		speed = item->speed;

	item->pos.zPos += (speed * COS(quad->momentumAngle)) >> W2V_SHIFT;
	item->pos.xPos += (speed * SIN(quad->momentumAngle)) >> W2V_SHIFT;

	slip = QUAD_SLIP * SIN(item->pos.xRot) >> W2V_SHIFT;
	if (abs(slip) > QUAD_SLIP / 2)
	{
		if (slip > 0)
			slip -= 10;
		else
			slip += 10;
		item->pos.zPos -= slip * COS(item->pos.yRot) >> W2V_SHIFT;
		item->pos.xPos -= slip * SIN(item->pos.yRot) >> W2V_SHIFT;
	}

	slip = QUAD_SLIP_SIDE * SIN(item->pos.zRot) >> W2V_SHIFT;
	if (abs(slip) > QUAD_SLIP_SIDE / 2)
	{
		item->pos.zPos -= slip * SIN(item->pos.yRot) >> W2V_SHIFT;
		item->pos.xPos += slip * COS(item->pos.yRot) >> W2V_SHIFT;
	}

	moved.x = item->pos.xPos;
	moved.z = item->pos.zPos;

	if (!(item->flags & ONESHOT)) 
		QuadBaddieCollision(item);

	rot = 0;

	hfl = TestQuadHeight(item, QUAD_FRONT, -QUAD_SIDE, &fl);
	if (hfl < oldFrontLeft.y - STEP_SIZE)
		rot = DoQuadShift(item, &fl, &oldFrontLeft);

	hmtl = TestQuadHeight(item, QUAD_FRONT >> 1, -QUAD_SIDE, &mtl);
	if (hmtl < mtl_old.y - STEP_SIZE)
		DoQuadShift(item, &mtl, &mtl_old);

	hmml = TestQuadHeight(item, 0, -QUAD_SIDE, &mml);
	if (hmml < mml_old.y - STEP_SIZE)
		DoQuadShift(item, &mml, &mml_old);

	hmbl = TestQuadHeight(item, -QUAD_FRONT >> 1, -QUAD_SIDE, &mbl);
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

	hmtr = TestQuadHeight(item, QUAD_FRONT >> 1, QUAD_SIDE, &mtr);
	if (hmtr < mtr_old.y - STEP_SIZE)
		DoQuadShift(item, &mtr, &mtr_old);

	hmmr = TestQuadHeight(item, 0, QUAD_SIDE, &mmr);
	if (hmmr < mmr_old.y - STEP_SIZE)
		DoQuadShift(item, &mmr, &mmr_old);

	hmbr = TestQuadHeight(item, -QUAD_FRONT >> 1, QUAD_SIDE, &mbr);
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
		newspeed = ((item->pos.zPos - old.z) * COS(quad->momentumAngle) + (item->pos.xPos - old.x) * SIN(quad->momentumAngle)) >> W2V_SHIFT;

		newspeed <<= 8;

		if ((&Items[g_LaraExtra.Vehicle] == item)
			&& (quad->velocity == MAX_VELOCITY)
			&& (newspeed < (quad->velocity - 10)))
		{
			LaraItem->hitPoints -= (quad->velocity - newspeed) >> 7;
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

void AnimateQuadBike(ITEM_INFO* item, int collide, int dead)
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

		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
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

		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
		SoundEffect(202, &item->pos, 0);
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
				//slowdamp = 0;
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
			else if (((quad->velocity >> 8) == 0)
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
			if ((quad->velocity >> 8) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;

			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = QUAD_STATE_TURNL;

			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = QUAD_STATE_TURNR;
			break;

		case QUAD_STATE_TURNL:
			if ((quad->velocity >> 8) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;
			else if (TrInput & IN_RIGHT)
			{
				LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_TURNR_A;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_TURNR;
			}
			else if (!(TrInput & IN_LEFT))
			{
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
				//slowdamp = 0;
			}
			break;

		case QUAD_STATE_TURNR:
			if ((quad->velocity >> 8) == 0)
				LaraItem->goalAnimState = QUAD_STATE_STOP;
			else if (TrInput & IN_LEFT)
			{
				LaraItem->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUADBIKE_TURNL_A;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LaraItem->goalAnimState = QUAD_STATE_TURNL;
			}
			else if (!(TrInput & IN_RIGHT))
			{
				LaraItem->goalAnimState = QUAD_STATE_DRIVE;
				//slowdamp = 0;
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

		if (Rooms[item->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
		{
			LaraItem->goalAnimState = QUAD_STATE_FALLOFF;
			LaraItem->hitPoints = 0;
			QuadbikeExplode(item);
		}
	}
}

int QuadUserControl(ITEM_INFO* item, int height, int* pitch)
{
	bool drive = false;
	int revs = 0;

	QUAD_INFO* quad = (QUAD_INFO *)item->data;

	if (!quad->velocity && !(TrInput & IN_DUCK) && !QuadCanHandbrakeStart)
		QuadCanHandbrakeStart = true;
	else if (quad->velocity)
		QuadCanHandbrakeStart = false;

	if (!(TrInput & IN_DUCK))
		QuadHandbrakeStarting = false;

	if (!QuadHandbrakeStarting)
	{
		if (quad->revs > 0x10)
		{
			quad->velocity += quad->revs >> 4;
			quad->revs -= quad->revs >> 3;
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
			if ((TrInput & IN_DUCK) && !QuadHandbrakeStarting && quad->velocity > MIN_HANDBRAKE_SPEED)
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
			if ((TrInput & IN_DUCK) && !QuadHandbrakeStarting && quad->velocity < -MIN_HANDBRAKE_SPEED + 0x800)
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
			if ((TrInput & IN_DUCK) && (QuadCanHandbrakeStart || QuadHandbrakeStarting))
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

		/* -------- accelerate */

		else if (TrInput & IN_ACTION)
		{
			if ((TrInput & IN_DUCK) && (QuadCanHandbrakeStart || QuadHandbrakeStarting))
			{
				QuadHandbrakeStarting = true;
				quad->revs += 0x200;
				if (quad->revs >= MAX_VELOCITY)
					quad->revs = MAX_VELOCITY;
			}
			else if (quad->velocity < MAX_VELOCITY)
			{
				if (quad->velocity < 0x4000)
					quad->velocity += 8 + ((0x4000 + 0x800 - quad->velocity) >> 3);
				else if (quad->velocity < 0x7000)
					quad->velocity += 4 + ((0x7000 + 0x800 - quad->velocity) >> 4);
				else if (quad->velocity < MAX_VELOCITY)
					quad->velocity += 2 + ((MAX_VELOCITY - quad->velocity) >> 3);
			}
			else
				quad->velocity = MAX_VELOCITY;

			quad->velocity -= (abs(item->pos.yRot - quad->momentumAngle)) >> 6;
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
				quad->revs -= quad->revs >> 3;
			else
				quad->revs = 0;
		}

		item->speed = quad->velocity >> 8;

		if (quad->engineRevs > 0x7000)
			quad->engineRevs = -0x2000;

		if (quad->velocity < 0)
			revs = abs(quad->velocity >> 1);
		else if (quad->velocity < 0x7000)
			revs = -0x2000 + ((quad->velocity * (0x6800 - -0x2000)) / 0x7000);
		else if (quad->velocity <= MAX_VELOCITY)
			revs = -0x2800 + (((quad->velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000));

		revs += abs(quad->revs);
		quad->engineRevs += (revs - quad->engineRevs) >> 3;
	}
	else
	{
		if (quad->engineRevs < 0xA000)
			quad->engineRevs += (0xA000 - quad->engineRevs) >> 3;
	}

	*pitch = quad->engineRevs;

	return drive;
}

void InitialiseQuadBike(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	item->data = (QUAD_INFO *)GameMalloc(sizeof(QUAD_INFO));
	QUAD_INFO* quad = (QUAD_INFO *)item->data;

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
		|| (g_LaraExtra.Vehicle != NO_ITEM))
		return;

	if ((geton = GetOnQuadBike(itemNumber, coll)))
	{
		short ang;

		g_LaraExtra.Vehicle = itemNumber;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(ID_FLARE_ITEM, 0);
			undraw_flare_meshes();
			Lara.flareControlLeft = 0;
			Lara.requestGunType = LG_NO_ARMS;
			Lara.gunType = LG_NO_ARMS;
		}

		Lara.gunStatus = LG_HANDS_BUSY;

		ITEM_INFO* item = &Items[itemNumber];

		ang = ATAN(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);
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

		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;

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

void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
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
		spark->dR = (spark->dR * speed) >> 5;
		spark->dG = (spark->dG * speed) >> 5;
		spark->dB = (spark->dB * speed) >> 5;
	}

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->sLife = spark->life = (GetRandomControl() & 3) + 20 - (speed >> 12);
	if (spark->sLife < 9)
		spark->sLife = spark->life = 9;

	spark->extras = 0;
	spark->dynamic = -1;

	spark->x = x + ((GetRandomControl() & 15) - 8);
	spark->y = y + ((GetRandomControl() & 15) - 8);
	spark->z = z + ((GetRandomControl() & 15) - 8);
	int zv = (speed * COS(angle)) >> (W2V_SHIFT + 2);
	int xv = (speed * SIN(angle)) >> (W2V_SHIFT + 2);
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
	int size = (GetRandomControl() & 7) + 16 + (speed >> 7);
	spark->dSize = size;
	spark->size = size >> 1;
}

int QuadBikeControl()
{
	short xRot, zRot, rotadd;
	int pitch, dead = 0;

	ITEM_INFO* item = &Items[g_LaraExtra.Vehicle];
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

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	TestTriggers(TriggerIndex, 0, 0);

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
		else if (quad->pitch > 0xa000)
			quad->pitch = 0xa000;
		SoundEffect(155, &item->pos, PITCH_SHIFT + ((0x10000 + quad->pitch) << 8));
	}
	else
	{
		if (drive != -1)
			SoundEffect(153, &item->pos, 0);
		quad->pitch = 0;
	}

	item->floor = height;

	rotadd = quad->velocity >> 2;
	quad->rearRot -= rotadd;
	quad->rearRot -= (quad->revs >> 3);
	quad->frontRot -= rotadd;

	quad->leftFallspeed = DoQuadDynamics(hfl, quad->leftFallspeed, (int *)&fl.y);
	quad->rightFallspeed = DoQuadDynamics(hfr, quad->rightFallspeed, (int *)&fr.y);
	item->fallspeed = DoQuadDynamics(height, item->fallspeed, (int *)&item->pos.yPos);

	height = (fl.y + fr.y) >> 1;
	xRot = ATAN(QUAD_FRONT, item->pos.yPos - height);
	zRot = ATAN(QUAD_SIDE, height - fl.y);

	item->pos.xRot += (xRot - item->pos.xRot) >> 1;
	item->pos.zRot += (zRot - item->pos.zRot) >> 1;

	if (!(quad->flags & QUAD_FLAGS_DEAD))
	{
		if (roomNumber != item->roomNumber)
		{
			ItemNewRoom(g_LaraExtra.Vehicle, roomNumber);
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
		item->frameNumber = Anims[item->animNumber].frameBase + (LaraItem->frameNumber - Anims[LaraItem->animNumber].frameBase);

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
		
		// Do smoke
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
					speed = ((QuadSmokeStart << 1) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) << 7;
					QuadSmokeStart++;
				}
				else if (QuadHandbrakeStarting)
					speed = (abs(quad->revs) >> 2) + ((GetRandomControl() & 7) << 7);
				else if ((GetRandomControl() & 3) == 0)
					speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) << 7;
				else
					speed = 0;
				TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
			}
		}
	}
	else
		QuadSmokeStart = 0;

	return QuadCheckGetOff();
}