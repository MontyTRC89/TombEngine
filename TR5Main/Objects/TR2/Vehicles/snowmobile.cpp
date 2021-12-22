#include "framework.h"
#include "snowmobile.h"
#include "lara.h"
#include "items.h"
#include "collide.h"
#include "lara_fire.h"
#include "lara_one_gun.h"
#include "effects/effects.h"
#include "lara_flare.h"
#include "effects/tomb4fx.h"
#include "sphere.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "animation.h"
#include "Sound/sound.h"
#include "particle/SimpleParticle.h"
#include "Specific/prng.h"
#include "camera.h"
#include "skidoo_info.h"

using std::vector;
using namespace TEN::Math::Random;

#define DAMAGE_START	140
#define DAMAGE_LENGTH	14

#define SKIDOO_DISMOUNT_DIST 295

#define SKIDOO_UNDO_TURN		ANGLE(2.0f)
#define SKIDOO_TURN				((ANGLE(1.0f) / 2) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN			ANGLE(6.0f)
#define SKIDOO_MOMENTUM_TURN	ANGLE(3.0f)
#define SKIDOO_MAX_MOM_TURN		ANGLE(150.0f)

#define SKIDOO_FAST_SPEED	150
#define SKIDOO_MAX_SPEED	100
#define SKIDOO_SLOW_SPEED	50
#define SKIDOO_MIN_SPEED	15

#define SKIDOO_ACCELERATION	10
#define SKIDOO_BRAKE		5
#define SKIDOO_SLOWDOWN		2
#define SKIDOO_REVERSE		-5
#define SKIDOO_MAX_BACK		-30
#define SKIDOO_MAX_KICK		-80

#define SKIDOO_SLIP			100
#define SKIDOO_SLIP_SIDE	50
#define SKIDOO_FRONT		550
#define SKIDOO_SIDE			260
#define SKIDOO_RADIUS		500
#define SKIDOO_SNOW			500

#define SKIDOO_MAX_HEIGHT STEP_SIZE
#define SKIDOO_MIN_BOUNCE ((SKIDOO_MAX_SPEED / 2) / 256)

#define SKIDOO_IN_FIRE			IN_ACTION
#define SKIDOO_IN_DISMOUNT		(IN_JUMP | IN_ROLL)
#define SKIDOO_IN_SLOW			IN_WALK
#define SKIDOO_IN_ACCELERATE	IN_FORWARD
#define SKIDOO_IN_BRAKE			IN_BACK
#define SKIDOO_IN_LEFT			IN_LEFT
#define SKIDOO_IN_RIGHT			IN_RIGHT

enum SkidooState 
{ 
	SKIDOO_STATE_SIT,
	SKIDOO_STATE_MOUNT, 
	SKIDOO_STATE_LEFT, 
	SKIDOO_STATE_RIGHT, 
	SKIDOO_STATE_FALL, 
	SKIDOO_STATE_HIT, 
	SKIDOO_STATE_MOUNT_LEFT,
	SKIDOO_STATE_DISMOUNT_RIGHT_LEFT, 
	SKIDOO_STATE_IDLE, 
	SKIDOO_STATE_DISMOUNT_RIGHT, 
	SKIDOO_STATE_JUMP_OFF,
	SKIDOO_STATE_DEATH,
	SKIDOO_STATE_FALLOFF
};

enum SkidooAnim
{
	SKIDOO_ANIM_DRIVE = 0,
	SKIDOO_ANIM_MOUNT_RIGHT = 1,
	SKIDOO_ANIM_TURN_LEFT_START = 2,
	SKIDOO_ANIM_TURN_LEFT_CONTINUE = 3,
	SKIDOO_ANIM_TURN_LEFT_END = 4,
	SKIDOO_ANIM_TURN_RIGHT_START = 5,
	SKIDOO_ANIM_TURN_RIGHT_CONTINUE = 6,
	SKIDOO_ANIM_TURN_RIGHT_END = 7,
	SKIDOO_ANIM_LEAP_START = 8,
	SKIDOO_ANIM_LEAP_END = 9,
	SKIDOO_ANIM_LEAP_CONTINUE = 10,
	SKIDOO_ANIM_HIT_LEFT = 11,
	SKIDOO_ANIM_HIT_RIGHT = 12,
	SKIDOO_ANIM_HIT_FRONT = 13,
	SKIDOO_ANIM_HIT_BACK = 14,
	SKIDOO_ANIM_IDLE = 15,
	SKIDOO_ANIM_DISMOUNT_RIGHT = 16,
	SKIDOO_ANIM_UNK = 17, // TODO
	SKIDOO_ANIM_MOUNT_LEFT = 18,
	SKIDOO_ANIM_DISMOUNT_LEFT = 19,
	SKIDOO_ANIM_FALL_OFF = 20,
	SKIDOO_ANIM_IDLE_DEATH = 21,
	SKIDOO_ANIM_FALL_DEATH = 22
};

// TODO
enum SkidooMountType
{
	SKIDOO_MOUNT_NONE = 0,
	SKIDOO_MOUNT_RIGHT = 1,
	SKIDOO_MOUNT_LEFT = 2
};

void InitialiseSkidoo(short itemNum)
{
	ITEM_INFO* skidoo = &g_Level.Items[itemNum];
	skidoo->data = SKIDOO_INFO();
	auto skidooInfo = (SKIDOO_INFO*)skidoo->data;
	
	skidooInfo->alreadyCdPlayed = false;

	if (skidoo->objectNumber == ID_SNOWMOBILE_GUN)
		skidooInfo->armed = true;
	else
		skidooInfo->armed = false;

	skidooInfo->extraRotation = 0;
	skidooInfo->flashTimer = 0;
	skidooInfo->leftFallspeed = 0;
	skidooInfo->momentumAngle = skidoo->pos.yRot;
	skidooInfo->pitch = 0;
	skidooInfo->rightFallspeed = 0;
	skidooInfo->skidooTurn = 0;

	if (skidoo->status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		skidoo->status = ITEM_ACTIVE;
	}
}

void SkidooBaddieCollision(ITEM_INFO* skidoo)
{
	int x, y, z, i;

	vector<short> roomsList;
	roomsList.push_back(skidoo->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[skidoo->roomNumber];
	for (i = 0; i < room->doors.size(); i++)
	{
		roomsList.push_back(room->doors[i].room);
	}

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];

			if (item->collidable && item->status != IFLAG_INVISIBLE && item != LaraItem && item != skidoo)
			{
				OBJECT_INFO* object = &Objects[item->objectNumber];

				if (object->collision && (object->intelligent))
				{
					x = skidoo->pos.xPos - item->pos.xPos;
					y = skidoo->pos.yPos - item->pos.yPos;
					z = skidoo->pos.zPos - item->pos.zPos;

					if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
					{
						if (item->objectNumber == ID_ROLLINGBALL)
						{
							if (TestBoundsCollide(item, LaraItem, 100))
							{
								if (LaraItem->hitPoints > 0)
								{
									DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - (STEP_SIZE * 2), LaraItem->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 5);
									item->hitPoints -= 8;
								}
							}
						}
						else
						{
							if (TestBoundsCollide(item, skidoo, SKIDOO_FRONT))
							{
								DoLotsOfBlood(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 3);
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

void SkidooGuns(ITEM_INFO* lara, ITEM_INFO* skidoo)
{
	LaraInfo*& laraInfo = lara->data;
	SKIDOO_INFO* skidooInfo = (SKIDOO_INFO*)skidoo->data;
	WEAPON_INFO* wepInfo = &Weapons[WEAPON_SNOWMOBILE];

	LaraGetNewTarget(lara, wepInfo);
	AimWeapon(lara, wepInfo, &laraInfo->rightArm);

	if (TrInput & SKIDOO_IN_FIRE && !skidoo->itemFlags[0])
	{
		short angles[] = {
			laraInfo->rightArm.yRot + lara->pos.yRot,
			laraInfo->rightArm.xRot
		};
		
		if (FireWeapon(WEAPON_PISTOLS, laraInfo->target, lara, angles) +
			FireWeapon(WEAPON_PISTOLS, laraInfo->target, lara, angles))
		{
			skidooInfo->flashTimer = 2;
			SoundEffect(wepInfo->sampleNum, &lara->pos, 0);
			skidoo->itemFlags[0] = 4;
		}
	}

	if (skidoo->itemFlags[0])
		skidoo->itemFlags[0]--;
}

void SkidooExplode(ITEM_INFO* lara, ITEM_INFO* skidoo)
{
	LaraInfo*& laraInfo = lara->data;

	if (g_Level.Rooms[skidoo->roomNumber].flags & ENV_FLAG_WATER)
		TriggerUnderwaterExplosion(skidoo, 1);
	else
	{
		TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -2, 0, skidoo->roomNumber);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -1, 0, skidoo->roomNumber);
	}

	auto pos = PHD_3DPOS(skidoo->pos.xPos, skidoo->pos.yPos - 128, skidoo->pos.zPos, 0, skidoo->pos.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	//ExplodingDeath(laraInfo->Vehicle, -1, 256);
	KillItem(laraInfo->Vehicle);
	skidoo->status = ITEM_DEACTIVATED;

	SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, 0, 0);

	laraInfo->Vehicle = NO_ITEM;
}

bool SkidooCheckGetOffOK(ITEM_INFO* skidoo, int dir)
{
	short angle;
	if (dir == SKIDOO_STATE_DISMOUNT_RIGHT_LEFT)
		angle = skidoo->pos.yRot + ANGLE(90.0f);
	else
		angle = skidoo->pos.yRot - ANGLE(90.0f);

	int x = skidoo->pos.xPos - SKIDOO_DISMOUNT_DIST * phd_sin(angle);
	int y = skidoo->pos.yPos;
	int z = skidoo->pos.zPos - SKIDOO_DISMOUNT_DIST * phd_cos(angle);
	auto probe = GetCollisionResult(x, y, z, skidoo->roomNumber);

	if ((probe.Position.Slope || probe.Position.Floor == NO_HEIGHT) ||
		abs(probe.Position.Floor - skidoo->pos.yPos) > (WALL_SIZE / 2) ||
		((probe.Position.Ceiling - skidoo->pos.yPos) > -LARA_HEIGHT || (probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
	{
		return false;
	}

	return true;
}

bool SkidooCheckGetOff(ITEM_INFO* lara, ITEM_INFO* skidoo)
{
	LaraInfo*& laraInfo = lara->data;

	if (laraInfo->Vehicle != NO_ITEM)
	{
		if ((lara->currentAnimState == SKIDOO_STATE_DISMOUNT_RIGHT || lara->currentAnimState == SKIDOO_STATE_DISMOUNT_RIGHT_LEFT) &&
			lara->frameNumber == g_Level.Anims[lara->animNumber].frameEnd)
		{
			if (lara->currentAnimState == SKIDOO_STATE_DISMOUNT_RIGHT_LEFT)
				lara->pos.yRot += ANGLE(90.0f);
			else
				lara->pos.yRot -= ANGLE(90.0f);

			SetAnimation(lara, LA_STAND_IDLE);
			lara->pos.xPos -= SKIDOO_DISMOUNT_DIST * phd_sin(lara->pos.yRot);
			lara->pos.zPos -= SKIDOO_DISMOUNT_DIST * phd_cos(lara->pos.yRot);
			lara->pos.xRot = 0;
			lara->pos.zRot = 0;
			laraInfo->Vehicle = NO_ITEM;
			laraInfo->gunStatus = LG_NO_ARMS;
		}
		else if (lara->currentAnimState == SKIDOO_STATE_JUMP_OFF &&
			(skidoo->pos.yPos == skidoo->floor || TestLastFrame(lara)))
		{
			SetAnimation(lara, LA_FREEFALL);

			if (skidoo->pos.yPos == skidoo->floor)
			{
				lara->goalAnimState = LS_DEATH;
				lara->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
				lara->speed = 0;
				SkidooExplode(lara, skidoo);
			}
			else
			{
				lara->goalAnimState = LS_FREEFALL;
				lara->pos.yPos -= 200;
				lara->fallspeed = skidoo->fallspeed;
				lara->speed = skidoo->speed;
				SoundEffect(SFX_TR4_LARA_FALL, &lara->pos, 0);
			}

			lara->pos.xRot = 0;
			lara->pos.zRot = 0;
			lara->gravityStatus = true;
			laraInfo->gunStatus = LG_NO_ARMS;
			laraInfo->moveAngle = skidoo->pos.yRot;
			skidoo->flags |= ONESHOT;
			skidoo->collidable = false;

			return false;
		}

		return true;
	}
	else
		return true;
}

void DoSnowEffect(ITEM_INFO* skidoo)
{
	auto material = GetCollisionResult(skidoo).BottomBlock->Material;
	if (material != FLOOR_MATERIAL::Ice && material != FLOOR_MATERIAL::Snow)
		return;

	TEN::Effects::TriggerSnowmobileSnow(skidoo);
}

void SkidooAnimation(ITEM_INFO* lara, ITEM_INFO* skidoo, int collide, bool dead)
{
	SKIDOO_INFO* skidooInfo = (SKIDOO_INFO*)skidoo->data;

	if (skidoo->pos.yPos != skidoo->floor &&
		skidoo->fallspeed > 0 &&
		lara->currentAnimState != SKIDOO_STATE_FALL &&
		!dead)
	{
		lara->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_LEAP_START;
		lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
		lara->currentAnimState = SKIDOO_STATE_FALL;
		lara->goalAnimState = SKIDOO_STATE_FALL;
	}
	else if (collide &&
		!dead &&
		lara->currentAnimState != SKIDOO_STATE_FALL)
	{
		if (lara->currentAnimState != SKIDOO_STATE_HIT)
		{
			if (collide == SKIDOO_ANIM_HIT_FRONT)
				SoundEffect(SFX_TR2_CLATTER_1, &skidoo->pos, 0);
			else
				SoundEffect(SFX_TR2_CLATTER_2, &skidoo->pos, 0);

			lara->animNumber = (short)(Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + collide);
			lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
			lara->currentAnimState = lara->goalAnimState = SKIDOO_STATE_HIT;
		}
	}
	else
	{
		switch (lara->currentAnimState)
		{
		case SKIDOO_STATE_IDLE:

			if (dead)
			{
				lara->goalAnimState = SKIDOO_STATE_DEATH;

				break;
			}

			lara->goalAnimState = SKIDOO_STATE_IDLE; 

			if (TrInput & SKIDOO_IN_DISMOUNT)
			{
				if (TrInput & SKIDOO_IN_RIGHT &&
					SkidooCheckGetOffOK(skidoo, SKIDOO_STATE_DISMOUNT_RIGHT))
				{
					lara->goalAnimState = SKIDOO_STATE_DISMOUNT_RIGHT;
					skidoo->speed = 0;
				}
				else if (TrInput & SKIDOO_IN_LEFT &&
					SkidooCheckGetOffOK(skidoo, SKIDOO_STATE_DISMOUNT_RIGHT_LEFT))
				{
					lara->goalAnimState = SKIDOO_STATE_DISMOUNT_RIGHT_LEFT;
					skidoo->speed = 0;
				}
			}
			else if (TrInput & SKIDOO_IN_LEFT)
				lara->goalAnimState = SKIDOO_STATE_LEFT;
			else if (TrInput & SKIDOO_IN_RIGHT)
				lara->goalAnimState = SKIDOO_STATE_RIGHT;
			else if (TrInput & (SKIDOO_IN_ACCELERATE | SKIDOO_IN_BRAKE))
				lara->goalAnimState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_SIT:
			if (skidoo->speed == 0)
				lara->goalAnimState = SKIDOO_STATE_IDLE;

			if (dead)
				lara->goalAnimState = SKIDOO_STATE_FALLOFF;
			else if (TrInput & SKIDOO_IN_LEFT)
				lara->goalAnimState = SKIDOO_STATE_LEFT;
			else if (TrInput & SKIDOO_IN_RIGHT)
				lara->goalAnimState = SKIDOO_STATE_RIGHT;

			break;

		case SKIDOO_STATE_LEFT:
			if (!(TrInput & SKIDOO_IN_LEFT))
				lara->goalAnimState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_RIGHT:
			if (!(TrInput & SKIDOO_IN_RIGHT))
				lara->goalAnimState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_FALL:
			if (skidoo->fallspeed <= 0 ||
				skidooInfo->leftFallspeed <= 0 ||
				skidooInfo->rightFallspeed <= 0)
			{
				lara->goalAnimState = SKIDOO_STATE_SIT;
				SoundEffect(SFX_TR2_CLATTER_3, &skidoo->pos, 0);
			}
			else if (skidoo->fallspeed > (DAMAGE_START + DAMAGE_LENGTH))
				lara->goalAnimState = SKIDOO_STATE_JUMP_OFF;

			break;
		}
	}

	if (g_Level.Rooms[skidoo->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		lara->goalAnimState = SKIDOO_STATE_JUMP_OFF;
		lara->hitPoints = 0;
		lara->roomNumber = skidoo->roomNumber;
		SkidooExplode(lara, skidoo);
	}
}

int GetSkidooCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	moved->x = skidoo->pos.xPos - moved->x;
	moved->z = skidoo->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		float s = phd_sin(skidoo->pos.yRot);
		float c = phd_cos(skidoo->pos.yRot);
		
		int side = -moved->z * s + moved->x * c;
		int front = moved->z * c + moved->x * s;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return SKIDOO_ANIM_HIT_BACK;
			else
				return SKIDOO_ANIM_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return SKIDOO_ANIM_HIT_LEFT;
			else
				return SKIDOO_ANIM_HIT_RIGHT;
		}
	}

	return 0;
}

bool SkidooUserControl(ITEM_INFO* skidoo, int height, int* pitch)
{
	auto skidooInfo = (SKIDOO_INFO*)skidoo->data;
	bool drive = false;
	int maxSpeed = 0;

	if (skidoo->pos.yPos >= height - STEP_SIZE)
	{
		*pitch = skidoo->speed + (height - skidoo->pos.yPos);

		if (TrInput & IN_LOOK && skidoo->speed == 0)
			LookUpDown();

		if ((TrInput & SKIDOO_IN_LEFT && !(TrInput & SKIDOO_IN_BRAKE)) ||
			(TrInput & SKIDOO_IN_RIGHT && TrInput & SKIDOO_IN_BRAKE))
		{
			skidooInfo->skidooTurn -= SKIDOO_TURN;
			if (skidooInfo->skidooTurn < -SKIDOO_MAX_TURN)
				skidooInfo->skidooTurn = -SKIDOO_MAX_TURN;
		}

		if ((TrInput & SKIDOO_IN_RIGHT && !(TrInput & SKIDOO_IN_BRAKE)) ||
			(TrInput & SKIDOO_IN_LEFT && TrInput & SKIDOO_IN_BRAKE))
		{
			skidooInfo->skidooTurn += SKIDOO_TURN;
			if (skidooInfo->skidooTurn > SKIDOO_MAX_TURN)
				skidooInfo->skidooTurn = SKIDOO_MAX_TURN;
		}

		if (TrInput & SKIDOO_IN_BRAKE)
		{
			if (skidoo->speed > 0)
				skidoo->speed -= SKIDOO_BRAKE;
			else
			{
				if (skidoo->speed > SKIDOO_MAX_BACK)
					skidoo->speed += SKIDOO_REVERSE;

				drive = true;
			}
		}
		else if (TrInput & SKIDOO_IN_ACCELERATE)
		{
			if (TrInput & SKIDOO_IN_FIRE && !skidooInfo->armed)
				maxSpeed = SKIDOO_FAST_SPEED;
			else if (TrInput & SKIDOO_IN_SLOW)
				maxSpeed = SKIDOO_SLOW_SPEED;
			else
				maxSpeed = SKIDOO_MAX_SPEED;

			if (skidoo->speed < maxSpeed)
				skidoo->speed += SKIDOO_ACCELERATION / 2 + SKIDOO_ACCELERATION * skidoo->speed / (2 * maxSpeed);
			else if (skidoo->speed > maxSpeed + SKIDOO_SLOWDOWN)
				skidoo->speed -= SKIDOO_SLOWDOWN;
			drive = true;
		}
		else if (skidoo->speed >= 0 &&
			skidoo->speed < SKIDOO_MIN_SPEED &&
			TrInput & (SKIDOO_IN_LEFT | SKIDOO_IN_RIGHT))
		{
			skidoo->speed = SKIDOO_MIN_SPEED;
			drive = true;
		}
		else if (skidoo->speed > SKIDOO_SLOWDOWN)
		{
			skidoo->speed -= SKIDOO_SLOWDOWN;
			if ((GetRandomControl() & 0x7f) < skidoo->speed)
				drive = true;
		}
		else
			skidoo->speed = 0;
	}
	else if (TrInput & (SKIDOO_IN_ACCELERATE | SKIDOO_IN_BRAKE))
	{
		drive = true;
		*pitch = skidooInfo->pitch + 50;
	}

	return drive;
}

int DoSkidooDynamics(int height, int fallspeed, int* y)
{
	int kick;

	if (height > * y)
	{
		*y += fallspeed;
		if (*y > height - SKIDOO_MIN_BOUNCE)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += GRAVITY;
	}
	else
	{
		kick = (height - *y) * 4;
		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;

		fallspeed += ((kick - fallspeed) / 8);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int SkidooCheckGetOn(ITEM_INFO* lara, ITEM_INFO* skidoo, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	int mountType = 0;

	if (!(TrInput & IN_ACTION) ||
		laraInfo->gunStatus != LG_NO_ARMS ||
		lara->gravityStatus)
	{
		return mountType = 0;
	}

	auto probe = GetCollisionResult(skidoo);
	short rot = skidoo->pos.yRot - lara->pos.yRot;

	if (rot > ANGLE(45.0f) && rot < ANGLE(135.0f))
		mountType = 1;
	else if (rot > -ANGLE(135.0f) && rot < -ANGLE(45.0f))
		mountType = 2;
	else
		mountType = 0;

	if (probe.Position.Floor < -32000 ||
		!TestBoundsCollide(skidoo, lara, coll->Setup.Radius) ||
		!TestCollision(skidoo, lara))
	{
		mountType = 0;
	}

	return mountType;
}

void SkidooCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* skidoo = &g_Level.Items[itemNum];

	if (lara->hitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	int mountType = SkidooCheckGetOn(lara, skidoo, coll);
	if (mountType == 0)
	{
		ObjectCollision(itemNum, lara, coll);

		return;
	}

	laraInfo->Vehicle = itemNum;

	if (laraInfo->gunType == WEAPON_FLARE)
	{
		CreateFlare(lara, ID_FLARE_ITEM, false);
		UndrawFlareMeshes(lara);
		laraInfo->flareControlLeft = 0;
		laraInfo->requestGunType = WEAPON_NONE;
		laraInfo->gunStatus = LG_NO_ARMS;
	}

	if (mountType == 1)
		lara->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_RIGHT;
	else if (mountType == 2)
		lara->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_LEFT;

	lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
	lara->currentAnimState = SKIDOO_STATE_MOUNT;
	lara->pos.yRot = skidoo->pos.yRot;
	lara->pos.xPos = skidoo->pos.xPos;
	lara->pos.yPos = skidoo->pos.yPos;
	lara->pos.zPos = skidoo->pos.zPos;
	laraInfo->gunStatus = LG_HANDS_BUSY;
	skidoo->collidable = true;
}

int TestSkidooHeight(ITEM_INFO* skidoo, int zOff, int xOff, PHD_VECTOR* pos)
{
	pos->y = skidoo->pos.yPos - zOff * phd_sin(skidoo->pos.xRot) + xOff * phd_sin(skidoo->pos.zRot);

	float s = phd_sin(skidoo->pos.yRot);
	float c = phd_cos(skidoo->pos.yRot);

	pos->x = skidoo->pos.xPos + zOff * s + xOff * c;
	pos->z = skidoo->pos.zPos + zOff * c - xOff * s;
	
	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, skidoo->roomNumber);
	if (probe.Position.Ceiling > pos->y ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return NO_HEIGHT;
	}

	return probe.Position.Floor;
}

short DoSkidooShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	COLL_RESULT probe;
	int	x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);
	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);
	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			skidoo->pos.zPos -= shiftZ + 1;

			return (pos->x - skidoo->pos.xPos);
		}
		else
		{
			skidoo->pos.zPos += WALL_SIZE - shiftZ;

			return (skidoo->pos.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			skidoo->pos.xPos -= shiftX + 1;

			return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			skidoo->pos.xPos += WALL_SIZE - shiftX;

			return (pos->z - skidoo->pos.zPos);
		}
	}
	else
	{
		x = z = 0;

		probe = GetCollisionResult(old->x, pos->y, pos->z, skidoo->roomNumber);
		if (probe.Position.Floor < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, skidoo->roomNumber);
		if (probe.Position.Floor < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			skidoo->pos.zPos += z;
			skidoo->pos.xPos += x;
			skidoo->speed -= 50;
		}
		else if (z)
		{
			skidoo->pos.zPos += z;
			skidoo->speed -= 50;

			if (z > 0)
				return (skidoo->pos.xPos - pos->x);
			else
				return (pos->x - skidoo->pos.xPos);
		}
		else if (x)
		{
			skidoo->pos.xPos += x;
			skidoo->speed -= 50;

			if (x > 0)
				return (pos->z - skidoo->pos.zPos);
			else
				return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
			skidoo->speed -= 50;
		}
	}

	return 0;
}

int SkidooDynamics(ITEM_INFO* lara, ITEM_INFO* skidoo)
{
	auto skidooInfo = (SKIDOO_INFO*)skidoo->data;
	PHD_VECTOR moved, frontLeft, frontRight, backRight, backLeft;
	PHD_VECTOR old, frontLeftOld, frontRightOld, backLeftOld, backRightOld;
	
	auto heightFrontLeftOld = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeftOld);
	auto heightFrontRightOld = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &frontRightOld);
	auto heightBackLeftOld = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &backLeftOld);
	auto heightBackRightOld = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &backRightOld);
	old.x = skidoo->pos.xPos;
	old.y = skidoo->pos.yPos;
	old.z = skidoo->pos.zPos;

	if (backLeftOld.y > heightBackLeftOld)
		backLeftOld.y = heightBackLeftOld;
	if (backRightOld.y > heightBackRightOld)
		backRightOld.y = heightBackRightOld;
	if (frontLeftOld.y > heightFrontLeftOld)
		frontLeftOld.y = heightFrontLeftOld;
	if (frontRightOld.y > heightFrontRightOld)
		frontRightOld.y = heightFrontRightOld;

	short rot;

	if (skidoo->pos.yPos > skidoo->floor - STEP_SIZE)
	{
		if (skidooInfo->skidooTurn < -SKIDOO_UNDO_TURN)
			skidooInfo->skidooTurn += SKIDOO_UNDO_TURN;
		else if (skidooInfo->skidooTurn > SKIDOO_UNDO_TURN)
			skidooInfo->skidooTurn -= SKIDOO_UNDO_TURN;
		else
			skidooInfo->skidooTurn = 0;
		skidoo->pos.yRot += skidooInfo->skidooTurn + skidooInfo->extraRotation;

		rot = skidoo->pos.yRot - skidooInfo->momentumAngle;
		if (rot < -SKIDOO_MOMENTUM_TURN)
		{
			if (rot < -SKIDOO_MAX_MOM_TURN)
			{
				rot = -SKIDOO_MAX_MOM_TURN;
				skidooInfo->momentumAngle = skidoo->pos.yRot - rot;
			}
			else
				skidooInfo->momentumAngle -= SKIDOO_MOMENTUM_TURN;
		}
		else if (rot > SKIDOO_MOMENTUM_TURN)
		{
			if (rot > SKIDOO_MAX_MOM_TURN)
			{
				rot = SKIDOO_MAX_MOM_TURN;
				skidooInfo->momentumAngle = skidoo->pos.yRot - rot;
			}
			else
				skidooInfo->momentumAngle += SKIDOO_MOMENTUM_TURN;
		}
		else
			skidooInfo->momentumAngle = skidoo->pos.yRot;
	}
	else
		skidoo->pos.yRot += skidooInfo->skidooTurn + skidooInfo->extraRotation;

	skidoo->pos.zPos += skidoo->speed * phd_cos(skidooInfo->momentumAngle);
	skidoo->pos.xPos += skidoo->speed * phd_sin(skidooInfo->momentumAngle);

	int slip = SKIDOO_SLIP * phd_sin(skidoo->pos.xRot);
	if (abs(slip) > SKIDOO_SLIP / 2)
	{
		skidoo->pos.zPos -= slip * phd_cos(skidoo->pos.yRot);
		skidoo->pos.xPos -= slip * phd_sin(skidoo->pos.yRot);
	}

	slip = SKIDOO_SLIP_SIDE * phd_sin(skidoo->pos.zRot);
	if (abs(slip) > SKIDOO_SLIP_SIDE / 2)
	{
		skidoo->pos.zPos -= slip * phd_sin(skidoo->pos.yRot);
		skidoo->pos.xPos += slip * phd_cos(skidoo->pos.yRot);
	}

	moved.x = skidoo->pos.xPos;
	moved.z = skidoo->pos.zPos;

	if (!(skidoo->flags & ONESHOT))
		SkidooBaddieCollision(skidoo);

	rot = 0;
	auto heightBackLeft = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &backLeft);
	if (heightBackLeft < (backLeftOld.y - STEP_SIZE))
		rot = DoSkidooShift(skidoo, &backLeft, &backLeftOld);

	auto heightBackRight = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &backRight);
	if (heightBackRight < (backRightOld.y - STEP_SIZE))
		rot += DoSkidooShift(skidoo, &backRight, &backRightOld);

	auto heightFrontLeft = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeft);
	if (heightFrontLeft < (frontLeftOld.y - STEP_SIZE))
		rot += DoSkidooShift(skidoo, &frontLeft, &frontLeftOld);

	auto heightFrontRight = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &frontRight);
	if (heightFrontRight < (frontRightOld.y - STEP_SIZE))
		rot += DoSkidooShift(skidoo, &frontRight, &frontRightOld);

	auto probe = GetCollisionResult(skidoo);
	if (probe.Position.Floor < (skidoo->pos.yPos - STEP_SIZE))
		DoSkidooShift(skidoo, (PHD_VECTOR*)&skidoo->pos, &old);

	skidooInfo->extraRotation = rot;

	auto collide = GetSkidooCollisionAnim(skidoo, &moved);
	if (collide)
	{
		int newspeed = (skidoo->pos.zPos - old.z) * phd_cos(skidooInfo->momentumAngle) + (skidoo->pos.xPos - old.x) * phd_sin(skidooInfo->momentumAngle);
		if (skidoo->speed > SKIDOO_MAX_SPEED + SKIDOO_ACCELERATION && newspeed < skidoo->speed - 10)
		{
			lara->hitPoints -= (skidoo->speed - newspeed) / 2;
			lara->hitStatus = true;
		}

		if (skidoo->speed > 0 && newspeed < skidoo->speed)
			skidoo->speed = (newspeed < 0) ? 0 : newspeed;
		else if (skidoo->speed < 0 && newspeed > skidoo->speed)
			skidoo->speed = (newspeed > 0) ? 0 : newspeed;

		if (skidoo->speed < SKIDOO_MAX_BACK)
			skidoo->speed = SKIDOO_MAX_BACK;
	}

	return collide;
}

bool SkidooControl(ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* skidoo = &g_Level.Items[laraInfo->Vehicle];
	SKIDOO_INFO* skidooInfo = (SKIDOO_INFO*)skidoo->data;

	PHD_VECTOR fl, fr;
	auto collide = SkidooDynamics(lara, skidoo);
	auto heightFrontLeft = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
	auto heightFrontRight = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);

	short roomNumber = skidoo->roomNumber;
	FLOOR_INFO* floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos);

	TestTriggers(skidoo, true);
	TestTriggers(skidoo, false);

	bool dead = false;
	int drive = 0;

	if (lara->hitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = true;
	}
	else if (lara->currentAnimState == SKIDOO_STATE_JUMP_OFF)
	{
		dead = true;
		collide = 0;
	}

	int pitch = 0;

	if (skidoo->flags & ONESHOT)
	{
		drive = false;
		collide = 0;
	}
	else
	{
		switch (lara->currentAnimState)
		{
		case SKIDOO_STATE_MOUNT:
		case SKIDOO_STATE_DISMOUNT_RIGHT:
		case SKIDOO_STATE_DISMOUNT_RIGHT_LEFT:
		case SKIDOO_STATE_JUMP_OFF:
			drive = -1;
			collide = 0;

			break;

		default:
			drive = SkidooUserControl(skidoo, height, &pitch);

			break;
		}
	}

	bool banditSkidoo = skidooInfo->armed;
	if (drive > 0)
	{
		skidooInfo->trackMesh = ((skidooInfo->trackMesh & 3) == 1) ? 2 : 1;

		skidooInfo->pitch += (pitch - skidooInfo->pitch) / 4;
		SoundEffect(skidooInfo->pitch ? SFX_TR2_SKIDOO_MOVING : SFX_TR2_SKIDOO_ACCELERATE, &skidoo->pos, 0, 0.5f + skidooInfo->pitch / (float)SKIDOO_MAX_SPEED);
	}
	else
	{
		skidooInfo->trackMesh = 0;
		if (!drive)
			SoundEffect(SFX_TR2_SKIDOO_IDLE, &skidoo->pos, 0);
		skidooInfo->pitch = 0;
	}
	skidoo->floor = height;

	skidooInfo->leftFallspeed = DoSkidooDynamics(heightFrontLeft, skidooInfo->leftFallspeed, (int*)&fl.y);
	skidooInfo->rightFallspeed = DoSkidooDynamics(heightFrontRight, skidooInfo->rightFallspeed, (int*)&fr.y);
	skidoo->fallspeed = DoSkidooDynamics(height, skidoo->fallspeed, (int*)&skidoo->pos.yPos);

	height = (fl.y + fr.y) / 2;
	short xRot = phd_atan(SKIDOO_FRONT, skidoo->pos.yPos - height);
	short zRot = phd_atan(SKIDOO_SIDE, height - fl.y);

	skidoo->pos.xRot += ((xRot - skidoo->pos.xRot) / 2);
	skidoo->pos.zRot += ((zRot - skidoo->pos.zRot) / 2);

	if (skidoo->flags & ONESHOT)
	{
		if (roomNumber != skidoo->roomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, roomNumber);
			ItemNewRoom(laraInfo->itemNumber, roomNumber);
		}

		AnimateItem(lara);

		if (skidoo->pos.yPos == skidoo->floor)
			SkidooExplode(lara, skidoo);

		return 0;
	}

	SkidooAnimation(lara, skidoo, collide, dead);

	if (roomNumber != skidoo->roomNumber)
	{
		ItemNewRoom(laraInfo->Vehicle, roomNumber);
		ItemNewRoom(laraInfo->itemNumber, roomNumber);
	}

	if (lara->currentAnimState != SKIDOO_STATE_FALLOFF)
	{
		lara->pos.xPos = skidoo->pos.xPos;
		lara->pos.yPos = skidoo->pos.yPos;
		lara->pos.zPos = skidoo->pos.zPos;
		lara->pos.yRot = skidoo->pos.yRot;
		if (drive >= 0)
		{
			lara->pos.xRot = skidoo->pos.xRot;
			lara->pos.zRot = skidoo->pos.zRot;
		}
		else
			lara->pos.xRot = lara->pos.zRot = 0;
	}
	else
		lara->pos.xRot = lara->pos.zRot = 0;

	AnimateItem(lara);

	if (!dead && drive >= 0 && banditSkidoo)
		SkidooGuns(lara, skidoo);

	if (!dead)
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + (lara->animNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);
	}
	else
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_ANIM_IDLE;
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase;
	}

	if (skidoo->speed && skidoo->floor == skidoo->pos.yPos)
	{
		DoSnowEffect(skidoo);
		if (skidoo->speed < 50)
			DoSnowEffect(skidoo);
	}

	return SkidooCheckGetOff(lara, skidoo);
}

void DrawSkidoo(ITEM_INFO* item)
{

}
