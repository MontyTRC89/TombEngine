#include "framework.h"
#include "snowmobile.h"
#include "lara.h"
#include "items.h"
#include "collide.h"
#include "effect.h"
#include "lara_fire.h"
#include "lara_one_gun.h"
#include "effect2.h"
#include "lara_flare.h"
#include "lot.h"
#include "tomb4fx.h"
#include "sphere.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "sound.h"
#include <Game\particle\SimpleParticle.h>

using std::vector;

enum SKIDOO_STATE 
{ 
	STATE_SKIDOO_SIT,
	STATE_SKIDOO_GETON, 
	STATE_SKIDOO_LEFT, 
	STATE_SKIDOO_RIGHT, 
	STATE_SKIDOO_FALL, 
	STATE_SKIDOO_HIT, 
	STATE_SKIDOO_GETONL,
	STATE_SKIDOO_GETOFFL, 
	STATE_SKIDOO_STILL, 
	STATE_SKIDOO_GETOFF, 
	STATE_SKIDOO_LETGO,
	STATE_SKIDOO_DEATH,
	STATE_SKIDOO_FALLOFF
};

#define SWIM_DEPTH 730
#define WADE_DEPTH STEP_SIZE
#define DAMAGE_START 140
#define DAMAGE_LENGTH 14
#define SKIDOO_GETON_ANIM 1
#define SKIDOO_GETONL_ANIM 18
#define SKIDOO_FALL_ANIM 8
#define SKIDOO_DEAD_ANIM 15
#define SKIDOO_HIT_LEFT 11
#define SKIDOO_HIT_RIGHT 12
#define SKIDOO_HIT_FRONT 13
#define SKIDOO_HIT_BACK 14
#define SKIDOO_GETOFF_DIST 330
#define SKIDOO_UNDO_TURN ANGLE(2)
#define SKIDOO_TURN ((ANGLE(1)/2) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN ANGLE(6)
#define SKIDOO_MOMENTUM_TURN ANGLE(3)
#define SKIDOO_MAX_MOM_TURN ANGLE(150)
#define SKIDOO_FAST_SPEED 150
#define SKIDOO_MAX_SPEED 100
#define SKIDOO_SLOW_SPEED 50
#define SKIDOO_MIN_SPEED 15
#define SKIDOO_ACCELERATION 10
#define SKIDOO_BRAKE 5
#define SKIDOO_SLOWDOWN 2
#define SKIDOO_REVERSE -5
#define SKIDOO_MAX_BACK -30
#define SKIDOO_MAX_KICK -80
#define SKIDOO_SLIP 100
#define SKIDOO_SLIP_SIDE 50
#define SKIDOO_FRONT 550
#define SKIDOO_SIDE 260
#define SKIDOO_RADIUS 500
#define SKIDOO_SNOW 500
#define SKIDOO_MAX_HEIGHT STEP_SIZE
#define SKIDOO_MIN_BOUNCE ((SKIDOO_MAX_SPEED/2)>>8)

void InitialiseSkidoo(short itemNum)
{
	ITEM_INFO* skidoo = &g_Level.Items[itemNum];

	SKIDOO_INFO* skinfo = game_malloc<SKIDOO_INFO>();
	skidoo->data = (void*)skinfo;
	
	skinfo->alreadyCdPlayed = false;

	if (skidoo->objectNumber == ID_SNOWMOBILE_GUN)
		skinfo->armed = true;
	else
		skinfo->armed = false;

	skinfo->extraRotation = 0;
	skinfo->flashTimer = 0;
	skinfo->leftFallspeed = 0;
	skinfo->momentumAngle = skidoo->pos.yRot;
	skinfo->pitch = 0;
	skinfo->rightFallspeed = 0;
	skinfo->skidooTurn = 0;

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

void SkidooGuns()
{
	WEAPON_INFO* winfo = &Weapons[WEAPON_SNOWMOBILE];
	ITEM_INFO* skidoo = &g_Level.Items[Lara.Vehicle];
	SKIDOO_INFO* skinfo = (SKIDOO_INFO*)skidoo->data;

	// Get the new target 
	LaraGetNewTarget(winfo);
	AimWeapon(winfo, &Lara.rightArm);

	if (!skidoo->itemFlags[0] && (TrInput & IN_ACTION))
	{
		short angles[] = {
			Lara.rightArm.yRot + LaraItem->pos.yRot,
			Lara.rightArm.xRot
		};
		
		if (FireWeapon(WEAPON_PISTOLS, Lara.target, LaraItem, angles) +
			FireWeapon(WEAPON_PISTOLS, Lara.target, LaraItem, angles))
		{
			skinfo->flashTimer = 2; // for custom render
			SoundEffect(winfo->sampleNum, &LaraItem->pos, 0);
			skidoo->itemFlags[0] = 4;
		}
	}

	if (skidoo->itemFlags[0])
		skidoo->itemFlags[0]--;
}

void SkidooExplode(ITEM_INFO* skidoo)
{
	if (g_Level.Rooms[skidoo->roomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(skidoo, 1);
	}
	else
	{
		TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -2, 0, skidoo->roomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -1, 0, skidoo->roomNumber);
	}

	ExplodingDeath(Lara.Vehicle, -1, 256);
	KillItem(Lara.Vehicle);
	skidoo->status = ITEM_DEACTIVATED;

	SoundEffect(SFX_EXPLOSION1, 0, 0);
	SoundEffect(SFX_EXPLOSION2, 0, 0);

	Lara.Vehicle = NO_ITEM;
}

bool SkidooCheckGetOffOK(int direction)
{
	ITEM_INFO* skidoo = &g_Level.Items[Lara.Vehicle];

	short angle;
	if (direction == STATE_SKIDOO_GETOFFL)
		angle = skidoo->pos.yRot + 0x4000;
	else
		angle = skidoo->pos.yRot - 0x4000;

	int x = skidoo->pos.xPos - (SKIDOO_GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT);
	int y = skidoo->pos.yPos;
	int z = skidoo->pos.zPos - (SKIDOO_GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

	short roomNumber = skidoo->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);

	if (HeightType == BIG_SLOPE || height == NO_HEIGHT || HeightType == DIAGONAL)
		return false;

	if (abs(height - skidoo->pos.yPos) > WALL_SIZE / 2)
		return false;

	int ceiling = GetCeiling(floor, x, y, z);
	if (ceiling - skidoo->pos.yPos > -LARA_HITE || height - ceiling < LARA_HITE)
		return false;

	return true;
}

// Check if Lara is still under skidoo control. Return 0 if she is in that limbo state of the skidoo still needing
// control (it is falling) and her needing normal control (so is she)
bool SkidooCheckGetOff()
{
	ITEM_INFO* skidoo = &g_Level.Items[Lara.Vehicle];

	if ((LaraItem->currentAnimState == STATE_SKIDOO_GETOFF 
		|| LaraItem->currentAnimState == STATE_SKIDOO_GETOFFL) 
		&& LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
	{
		// Wait for last frame of GETOFF anim before returning to normal Lara control
		if (LaraItem->currentAnimState == STATE_SKIDOO_GETOFFL)
			LaraItem->pos.yRot += ANGLE(90);
		else
			LaraItem->pos.yRot -= ANGLE(90);

		LaraItem->animNumber = LA_STAND_SOLID;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = LS_STOP;
		LaraItem->pos.xPos -= SKIDOO_GETOFF_DIST * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.zPos -= SKIDOO_GETOFF_DIST * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		Lara.Vehicle = NO_ITEM;
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (LaraItem->currentAnimState == STATE_SKIDOO_LETGO 
		&& (skidoo->pos.yPos == skidoo->floor 
			|| LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd))
	{
		// Lara is falling
		LaraItem->animNumber = LA_FREEFALL;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LS_FREEFALL;

		if (skidoo->pos.yPos == skidoo->floor)
		{
			// Skidoo has hit the floor, then explode
			LaraItem->goalAnimState = LS_DEATH;
			LaraItem->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
			LaraItem->speed = 0;
			SkidooExplode(skidoo);
		}
		else
		{
			// Continue the freefall
			LaraItem->goalAnimState = LS_FREEFALL;
			LaraItem->pos.yPos -= 200;
			LaraItem->fallspeed = skidoo->fallspeed;
			LaraItem->speed = skidoo->speed;
			SoundEffect(SFX_LARA_FALL, &LaraItem->pos, 0);
		}

		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		LaraItem->gravityStatus = true;
		Lara.gunStatus = LG_NO_ARMS;
		Lara.moveAngle = skidoo->pos.yRot - LaraItem->pos.yRot;
		skidoo->flags |= ONESHOT; // skidoo is dead
		skidoo->collidable = false;

		return false;
	}

	return true;
}

void DoSnowEffect(ITEM_INFO* skidoo)
{
	T5M::Effects::TriggerSnowmobileSnow(skidoo);
	/*SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = 64;
	spark->sG = 64;
	spark->sB = 64;
	spark->dR = 64;
	spark->dG = 64;
	spark->dB = 64;
	spark->colFadeSpeed = 1;
	spark->transType = COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
	spark->fadeToBlack = spark->life - 4;
	spark->x = (GetRandomControl() & 255) + skidoo->pos.xPos - 8;
	spark->y = (GetRandomControl() & 15) + skidoo->pos.yPos - 8;
	spark->z = (GetRandomControl() & 255) + skidoo->pos.zPos - 8;
	spark->xVel = 0;
	spark->zVel = 0;
	spark->friction = 0;
	spark->flags = 538;
	spark->yVel = (GetRandomControl() & 0x7F) - 256;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->scalar = 3;
	spark->maxYvel = 0;
	spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
	spark->gravity = -spark->yVel >> 2;
	spark->sSize = spark->size = ((GetRandomControl() & 3) + 16) * 32;
	spark->dSize = 2 * spark->size;*/
}

void SkidooAnimation(ITEM_INFO* skidoo, int collide, bool dead)
{
	SKIDOO_INFO* skinfo = (SKIDOO_INFO*)skidoo->data;

	if (skidoo->pos.yPos != skidoo->floor 
		&& skidoo->fallspeed > 0 
		&& LaraItem->currentAnimState != STATE_SKIDOO_FALL 
		&& !dead)
	{
		LaraItem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_FALL_ANIM;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_SKIDOO_FALL;
	}
	else if (collide 
		&& !dead 
		&& LaraItem->currentAnimState != STATE_SKIDOO_FALL)
	{
		if (LaraItem->currentAnimState != STATE_SKIDOO_HIT)
		{
			if (collide == SKIDOO_HIT_FRONT)
				SoundEffect(SFX_TR2_CLATTER_1, &skidoo->pos, 0);
			else
				SoundEffect(SFX_TR2_CLATTER_2, &skidoo->pos, 0);
			LaraItem->animNumber = (short)(Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_SKIDOO_HIT;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case STATE_SKIDOO_STILL:
			// TODO: script this
			/*cd = (skinfo->alreadyCdPlayed == false) ? 53 : 52;
			if (!skinfo->alreadyCdPlayed)
			{
				S_CDPlay(cd, FALSE);
				skinfo->alreadyCdPlayed = true;
			}*/

			if (dead)
			{
				LaraItem->goalAnimState = STATE_SKIDOO_DEATH;
				break;
			}

			LaraItem->goalAnimState = STATE_SKIDOO_STILL; 

			if (TrInput & IN_JUMP)
			{
				if ((TrInput & IN_RIGHT) && SkidooCheckGetOffOK(STATE_SKIDOO_GETOFF))
				{
					LaraItem->goalAnimState = STATE_SKIDOO_GETOFF;
					skidoo->speed = 0;
				}
				else if ((TrInput & IN_LEFT) && SkidooCheckGetOffOK(STATE_SKIDOO_GETOFFL))
				{
					LaraItem->goalAnimState = STATE_SKIDOO_GETOFFL;
					skidoo->speed = 0;
				}
			}
			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = STATE_SKIDOO_LEFT;
			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = STATE_SKIDOO_RIGHT;
			else if (TrInput & (IN_FORWARD | IN_BACK))
				LaraItem->goalAnimState = STATE_SKIDOO_SIT;
			break;

		case STATE_SKIDOO_SIT:
			if (skidoo->speed == 0)
				LaraItem->goalAnimState = STATE_SKIDOO_STILL;

			if (dead)
				LaraItem->goalAnimState = STATE_SKIDOO_FALLOFF;
			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = STATE_SKIDOO_LEFT;
			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = STATE_SKIDOO_RIGHT;
			break;

		case STATE_SKIDOO_LEFT:
			if (!(TrInput & IN_LEFT))
				LaraItem->goalAnimState = STATE_SKIDOO_SIT;
			break;

		case STATE_SKIDOO_RIGHT:
			if (!(TrInput & IN_RIGHT))
				LaraItem->goalAnimState = STATE_SKIDOO_SIT;
			break;

		case STATE_SKIDOO_FALL:
			if (skidoo->fallspeed <= 0 || skinfo->leftFallspeed <= 0 || skinfo->rightFallspeed <= 0)
			{
				SoundEffect(SFX_TR2_CLATTER_3, &skidoo->pos, 0);
				LaraItem->goalAnimState = STATE_SKIDOO_SIT;
			}
			else if (skidoo->fallspeed > DAMAGE_START + DAMAGE_LENGTH)
				LaraItem->goalAnimState = STATE_SKIDOO_LETGO;
			break;
		}
	}
}

int GetSkidooCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	moved->x = skidoo->pos.xPos - moved->x;
	moved->z = skidoo->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		// Get direction of movement relative to facing 
		int s = phd_sin(skidoo->pos.yRot);
		int c = phd_cos(skidoo->pos.yRot);
		
		int side = (-moved->z * s + moved->x * c) >> W2V_SHIFT;
		int front = (moved->z * c + moved->x * s) >> W2V_SHIFT;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return SKIDOO_HIT_BACK;
			else
				return SKIDOO_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return SKIDOO_HIT_LEFT;
			else
				return SKIDOO_HIT_RIGHT;
		}
	}

	return 0;
}

bool SkidooUserControl(ITEM_INFO* skidoo, int height, int* pitch)
{
	SKIDOO_INFO* skinfo = (SKIDOO_INFO*)skidoo->data;

	bool drive = false;
	int maxSpeed = 0;

	// Handle user input. It will affect the next frame.
	if (skidoo->pos.yPos >= height - STEP_SIZE)
	{
		// Engine pitch depends on speed + height off ground
		*pitch = skidoo->speed + (height - skidoo->pos.yPos);

		if (skidoo->speed == 0 && (TrInput & IN_LOOK))
			LookUpDown();

		// If tracks on the ground, user has control; allow for reversing! 
		if (((TrInput & IN_LEFT) 
			&& !(TrInput & IN_BACK)) 
			|| ((TrInput & IN_RIGHT) 
				&& (TrInput & IN_BACK)))
		{
			skinfo->skidooTurn -= SKIDOO_TURN;
			if (skinfo->skidooTurn < -SKIDOO_MAX_TURN)
				skinfo->skidooTurn = -SKIDOO_MAX_TURN;
		}

		if (((TrInput & IN_RIGHT) 
			&& !(TrInput & IN_BACK)) 
			|| ((TrInput & IN_LEFT) 
				&& (TrInput & IN_BACK)))
		{
			skinfo->skidooTurn += SKIDOO_TURN;
			if (skinfo->skidooTurn > SKIDOO_MAX_TURN)
				skinfo->skidooTurn = SKIDOO_MAX_TURN;
		}

		if (TrInput & IN_BACK)
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
		else if (TrInput & IN_FORWARD)
		{
			if ((TrInput & IN_ACTION) && !skinfo->armed) // Red skidoo can go faster than bandit ones
				maxSpeed = SKIDOO_FAST_SPEED;
			else if (TrInput & IN_STEPSHIFT)
				maxSpeed = SKIDOO_SLOW_SPEED;
			else
				maxSpeed = SKIDOO_MAX_SPEED;

			if (skidoo->speed < maxSpeed)
				skidoo->speed += SKIDOO_ACCELERATION / 2 + SKIDOO_ACCELERATION * skidoo->speed / (2 * maxSpeed);
			else if (skidoo->speed > maxSpeed + SKIDOO_SLOWDOWN)
				skidoo->speed -= SKIDOO_SLOWDOWN;
			drive = true;
		}
		else if (skidoo->speed >= 0 
			&& skidoo->speed < SKIDOO_MIN_SPEED 
			&& (TrInput & (IN_LEFT | IN_RIGHT)))
		{
			skidoo->speed = SKIDOO_MIN_SPEED; // If user wants to turn, skidoo will move forward
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
	else if (TrInput & (IN_FORWARD | IN_BACK))
	{
		drive = true;
		*pitch = skinfo->pitch + 50;
	}

	return drive;
}

int DoSkidooDynamics(int height, int fallspeed, int* y)
{
	int kick;

	if (height > * y)
	{
		// In air
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
		// On ground: get up push from height change
		kick = height - *y * 4;
		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;
		fallspeed += (kick - fallspeed / 8);
		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int SkidooCheckGetOn(short itemNum, COLL_INFO* coll)
{
	// Check if Lara is close enough and in right position to get onto skidoo 
	if (!(TrInput & IN_ACTION) 
		|| Lara.gunStatus != LG_NO_ARMS 
		|| LaraItem->gravityStatus)
		return 0;

	ITEM_INFO* skidoo = &g_Level.Items[itemNum];

	short rot = (skidoo->pos.yRot - LaraItem->pos.yRot);
	int geton = 0;

	if (rot > ANGLE(45) && rot < ANGLE(135))
		geton = 1; // Right
	else if (rot > -ANGLE(135) && rot < -ANGLE(45))
		geton = 2; // Left
	else
		return 0;

	if (!TestBoundsCollide(skidoo, LaraItem, coll->radius))
		return 0;

	if (!TestCollision(skidoo, LaraItem))
		return 0;

	short roomNumber = skidoo->roomNumber;
	FLOOR_INFO* floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos) < -32000)
		return 0;

	return geton;
}

void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll)
{
	// Don't do collision if Lara is dead or if it's already on the skidoo
	if (litem->hitPoints < 0 || Lara.Vehicle != NO_ITEM)
		return;

	// If Lara can't get on, the do normal object collision  
	int geton = SkidooCheckGetOn(itemNum, coll);
	if (!geton)
	{
		ObjectCollision(itemNum, litem, coll);
		return;
	}

	// If here, it means that Lara can get on
	Lara.Vehicle = itemNum;

	// Flares are not allowed on the skidoo
	if (Lara.gunType == WEAPON_FLARE)
	{
		CreateFlare(ID_FLARE_ITEM, FALSE);
		undraw_flare_meshes();
		Lara.flareControlLeft = 0;
		Lara.requestGunType = WEAPON_NONE;
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (geton == 1)
		litem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_GETON_ANIM;
	else
		litem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_GETONL_ANIM;

	litem->frameNumber = g_Level.Anims[litem->animNumber].frameBase;
	litem->currentAnimState = STATE_SKIDOO_GETON;
	Lara.gunStatus = LG_HANDS_BUSY;

	ITEM_INFO* skidoo = &g_Level.Items[itemNum];
	litem->pos.yRot = skidoo->pos.yRot;
	litem->pos.xPos = skidoo->pos.xPos;
	litem->pos.yPos = skidoo->pos.yPos;
	litem->pos.zPos = skidoo->pos.zPos;

	skidoo->collidable = true;
}

int TestSkidooHeight(ITEM_INFO* item, int zOff, int xOff, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - (zOff * phd_sin(item->pos.xRot) >> W2V_SHIFT) +
		                      (xOff * phd_sin(item->pos.zRot) >> W2V_SHIFT);

	int s = phd_sin(item->pos.yRot);
	int c = phd_cos(item->pos.yRot);

	pos->x = item->pos.xPos + ((zOff * s + xOff * c) >> W2V_SHIFT);
	pos->z = item->pos.zPos + ((zOff * c - xOff * s) >> W2V_SHIFT);
	
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;

	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

short DoSkidooShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int	x = pos->x >> WALL_SHIFT;
	int z = pos->z >> WALL_SHIFT;
	int xOld = old->x >> WALL_SHIFT;
	int zOld = old->z >> WALL_SHIFT;
	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			// Neither shift; may have hit a very steep slope, so need to push back to old position  
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			// Z shift left 
			skidoo->pos.zPos -= shiftZ + 1;
			return (pos->x - skidoo->pos.xPos);
		}
		else
		{
			// Z shift right 
			skidoo->pos.zPos += WALL_SIZE - shiftZ;
			return (skidoo->pos.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			// X shift up 
			skidoo->pos.xPos -= shiftX + 1;
			return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			// X shift down 
			skidoo->pos.xPos += WALL_SIZE - shiftX;
			return (pos->z - skidoo->pos.zPos);
		}
	}
	else
	{
		// A diagonal hit; means a barrage of tests needed to determine best shift 
		x = z = 0;

		short roomNumber = skidoo->roomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = skidoo->roomNumber;
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
			// Corner or side collision 
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
			// Pure diagonal collision 
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
			skidoo->speed -= 50;
		}
	}

	return 0;
}

int SkidooDynamics(ITEM_INFO* skidoo)
{
	// Does all skidoo movement and collision and returns if collide value 
	PHD_VECTOR moved, fl, fr, br, bl;
	PHD_VECTOR old, flOld, frOld, blOld, brOld;
	
	SKIDOO_INFO* skinfo = (SKIDOO_INFO*)skidoo->data;

	// First get positions and heights of skidoo's corners + centre 
	int hflOld = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &flOld);
	int hfrOld = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &frOld);
	int hblOld = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &blOld);
	int hbrOld = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &brOld);
	old.x = skidoo->pos.xPos;
	old.y = skidoo->pos.yPos;
	old.z = skidoo->pos.zPos;

	// Back left/right may be slightly below ground, so correct for this 
	if (blOld.y > hblOld)
		blOld.y = hblOld;
	if (brOld.y > hbrOld)
		brOld.y = hbrOld;
	if (flOld.y > hflOld)
		flOld.y = hflOld;
	if (frOld.y > hfrOld)
		frOld.y = hfrOld;

	short rot;

	// First undo any turn the skidoo may have applied 
	if (skidoo->pos.yPos > skidoo->floor - STEP_SIZE)
	{
		if (skinfo->skidooTurn < -SKIDOO_UNDO_TURN)
			skinfo->skidooTurn += SKIDOO_UNDO_TURN;
		else if (skinfo->skidooTurn > SKIDOO_UNDO_TURN)
			skinfo->skidooTurn -= SKIDOO_UNDO_TURN;
		else
			skinfo->skidooTurn = 0;
		skidoo->pos.yRot += skinfo->skidooTurn + skinfo->extraRotation;

		// Deal with momentum; do it with an angle that tracks direction of travel, but slower than turn 
		rot = skidoo->pos.yRot - skinfo->momentumAngle;
		if (rot < -SKIDOO_MOMENTUM_TURN)
		{
			if (rot < -SKIDOO_MAX_MOM_TURN)
			{
				rot = -SKIDOO_MAX_MOM_TURN;
				skinfo->momentumAngle = skidoo->pos.yRot - rot;
			}
			else
				skinfo->momentumAngle -= SKIDOO_MOMENTUM_TURN;
		}
		else if (rot > SKIDOO_MOMENTUM_TURN)
		{
			if (rot > SKIDOO_MAX_MOM_TURN)
			{
				rot = SKIDOO_MAX_MOM_TURN;
				skinfo->momentumAngle = skidoo->pos.yRot - rot;
			}
			else
				skinfo->momentumAngle += SKIDOO_MOMENTUM_TURN;
		}
		else
			skinfo->momentumAngle = skidoo->pos.yRot;
	}
	else
		skidoo->pos.yRot += skinfo->skidooTurn + skinfo->extraRotation;

	// Move skidoo according to speed 
	skidoo->pos.zPos += skidoo->speed * phd_cos(skinfo->momentumAngle) >> W2V_SHIFT;
	skidoo->pos.xPos += skidoo->speed * phd_sin(skinfo->momentumAngle) >> W2V_SHIFT;

	// Slide skidoo according to tilts (to avoid getting stuck on slopes) 
	int slip = SKIDOO_SLIP * phd_sin(skidoo->pos.xRot) >> W2V_SHIFT;
	if (abs(slip) > SKIDOO_SLIP / 2)
	{
		skidoo->pos.zPos -= slip * phd_cos(skidoo->pos.yRot) >> W2V_SHIFT;
		skidoo->pos.xPos -= slip * phd_sin(skidoo->pos.yRot) >> W2V_SHIFT;
	}

	slip = SKIDOO_SLIP_SIDE * phd_sin(skidoo->pos.zRot) >> W2V_SHIFT;
	if (abs(slip) > SKIDOO_SLIP_SIDE / 2)
	{
		skidoo->pos.zPos -= slip * phd_sin(skidoo->pos.yRot) >> W2V_SHIFT;
		skidoo->pos.xPos += slip * phd_cos(skidoo->pos.yRot) >> W2V_SHIFT;
	}

	// Remember desired position in case of collisions moving us about 
	moved.x = skidoo->pos.xPos;
	moved.z = skidoo->pos.zPos;

	// Test against bad guys too 
	if (!(skidoo->flags & ONESHOT)) // ONESHOT flag set if skidoo no longer travelling with Lara
		SkidooBaddieCollision(skidoo);

	// Test new positions of points (one at a time) and shift skidoo accordingly 
	rot = 0;
	int hbl = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl);
	if (hbl < blOld.y - STEP_SIZE)
		rot = DoSkidooShift(skidoo, &bl, &blOld);

	int hbr = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &br);
	if (hbr < brOld.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &br, &brOld);

	int hfl = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
	if (hfl < flOld.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &fl, &flOld);

	int hfr = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);
	if (hfr < frOld.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &fr, &frOld);

	short roomNumber = skidoo->roomNumber;
	FLOOR_INFO* floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos);
	if (height < skidoo->pos.yPos - STEP_SIZE)
		DoSkidooShift(skidoo, (PHD_VECTOR*)&skidoo->pos, &old);

	skinfo->extraRotation = rot;

	// Get collision anim if skidoo has been moved from desired position by collisions 
	int collide = GetSkidooCollisionAnim(skidoo, &moved);

	// Check final actual movement; if speed is more than halved then reduce to zero 
	if (collide)
	{
		int newspeed = ((skidoo->pos.zPos - old.z) * phd_cos(skinfo->momentumAngle) + (skidoo->pos.xPos - old.x) * phd_sin(skinfo->momentumAngle)) >> W2V_SHIFT;
		if (skidoo->speed > SKIDOO_MAX_SPEED + SKIDOO_ACCELERATION && newspeed < skidoo->speed - 10)
		{
			LaraItem->hitPoints -= (skidoo->speed - newspeed) / 2;
			LaraItem->hitStatus = true;
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

bool SkidooControl()
{
	PHD_VECTOR fl, fr;
	
	ITEM_INFO* skidoo = &g_Level.Items[Lara.Vehicle];
	SKIDOO_INFO* skinfo = (SKIDOO_INFO*)skidoo->data;
	int collide = SkidooDynamics(skidoo);

	// Now got final position, so get heights under middle and corners (will only have changed
	// from above if collision occurred, but recalc anyway as hardly big maths) 
	int hfl = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
	int hfr = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);

	short roomNumber = skidoo->roomNumber;
	FLOOR_INFO* floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos);

	TestTriggers(TriggerIndex, 0, 0);
	TestTriggers(TriggerIndex, 1, 0);

	bool dead = false;
	int drive = 0;

	// Need to know what status Lara has w.r.t. the skidoo; has she died or fallen off? 
	if (LaraItem->hitPoints <= 0)
	{
		// Disable user input if Lara is dead 
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = true;
	}
	else if (LaraItem->currentAnimState == STATE_SKIDOO_LETGO)
	{
		dead = true;
		collide = 0;
	}

	int pitch = 0;

	// Deal with user input (if allowed) 
	if (skidoo->flags & ONESHOT)
	{
		drive = false;
		collide = 0;
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case STATE_SKIDOO_GETON:
		case STATE_SKIDOO_GETOFF:
		case STATE_SKIDOO_GETOFFL:
		case STATE_SKIDOO_LETGO:
			// No control 
			drive = -1;
			collide = 0;
			break;

		default:
			// Reduce user input if Lara is dead 
			drive = SkidooUserControl(skidoo, height, &pitch);
			break;
		}
	}

	// Do track meshes 
	bool banditSkidoo = skinfo->armed;
	if (drive > 0)
	{
		skinfo->trackMesh = ((skinfo->trackMesh & 3) == 1) ? 2 : 1;

		// Do engine noise 
		skinfo->pitch += (pitch - skinfo->pitch) / 4;
		SoundEffect(SFX_TR2_SNOWMOBILE_HIGH_ENGINE_RPM, &skidoo->pos, 4 + ((0x10000 - (SKIDOO_MAX_SPEED - skinfo->pitch) * 100) * 256));
	}
	else
	{
		skinfo->trackMesh = 0;
		if (!drive)
			SoundEffect(SFX_TR2_SNOWMOBILE_IDLE, &skidoo->pos, 0);
		skinfo->pitch = 0;
	}
	skidoo->floor = height;

	// Do fallspeed effects on skidoo 
	skinfo->leftFallspeed = DoSkidooDynamics(hfl, skinfo->leftFallspeed, (int*)&fl.y);
	skinfo->rightFallspeed = DoSkidooDynamics(hfr, skinfo->rightFallspeed, (int*)&fr.y);
	skidoo->fallspeed = DoSkidooDynamics(height, skidoo->fallspeed, (int*)&skidoo->pos.yPos);

	// Rotate skidoo to match these heights 
	height = (fl.y + fr.y) / 2;
	short xRot = phd_atan(SKIDOO_FRONT, skidoo->pos.yPos - height);
	short zRot = phd_atan(SKIDOO_SIDE, height - fl.y);

	skidoo->pos.xRot += (xRot - skidoo->pos.xRot) >> 1;
	skidoo->pos.zRot += (zRot - skidoo->pos.zRot) >> 1;

	if (skidoo->flags & ONESHOT)
	{
		// This is a falling skidoo - Lara is elsewhere 
		if (roomNumber != skidoo->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		AnimateItem(LaraItem);

		// Has it hit the ground? If so, explode 
		if (skidoo->pos.yPos == skidoo->floor)
			SkidooExplode(skidoo);
		return 0;
	}

	SkidooAnimation(skidoo, collide, dead);

	if (roomNumber != skidoo->roomNumber)
	{
		ItemNewRoom(Lara.Vehicle, roomNumber);
		ItemNewRoom(Lara.itemNumber, roomNumber);
	}

	// Move Lara to the skidoo position 
	if (LaraItem->currentAnimState != STATE_SKIDOO_FALLOFF)
	{
		LaraItem->pos.xPos = skidoo->pos.xPos;
		LaraItem->pos.yPos = skidoo->pos.yPos;
		LaraItem->pos.zPos = skidoo->pos.zPos;
		LaraItem->pos.yRot = skidoo->pos.yRot;
		if (drive >= 0)
		{
			LaraItem->pos.xRot = skidoo->pos.xRot;
			LaraItem->pos.zRot = skidoo->pos.zRot;
		}
		else
			// Don't tilt Lara during geton/off 
			LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
	}
	else
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;

	AnimateItem(LaraItem);

	if (!dead && drive >= 0 && banditSkidoo)
		SkidooGuns();

	// Set skidoo on the exact same anim frame 
	if (!dead)
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + (LaraItem->animNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);
	}
	else
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_DEAD_ANIM;
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase;
	}

	// If skidoo is moving, then set off a snow spray sprite 
	if (skidoo->speed && skidoo->floor == skidoo->pos.yPos /*&& Utils.getFloorSound(skidoo, FS_SNOW)*/)
	{
		DoSnowEffect(skidoo);
		if (skidoo->speed < 50)
			DoSnowEffect(skidoo);
	}

	return SkidooCheckGetOff();
}

void DrawSkidoo(ITEM_INFO* item)
{

}