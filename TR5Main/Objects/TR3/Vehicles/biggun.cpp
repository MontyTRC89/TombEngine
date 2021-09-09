#include "framework.h"
#include "biggun.h"
#include "items.h"
#include "level.h"
#include "collide.h"
#include "input.h"
#include "lara.h"
#include "lara_flare.h"
#include "Sound\sound.h"
#include "sphere.h"
#include "effects\effects.h"
#include "lara_struct.h"
#include "effects\tomb4fx.h"
#include "draw.h"

static long GunRotYAdd = 0;
bool barrelRotating;

#define RECOIL_TIME 26
#define RECOIL_Z	25

//flags
#define FLAG_UPDOWN	1
#define FLAG_AUTOROT	2
#define FLAG_GETOFF	4
#define FLAG_FIRE	8
//frames
#define BGUN_UPDOWN_FRAMES	59
#define GETOFF_FRAME 30//the frame where Lara isn't looking either up or down in her "main" animation, aka when she can get off

enum {//states
	BGUN_GETON,
	BGUN_GETOFF,
	BGUN_UPDOWN,
	BGUN_RECOIL
};
enum {//anims
	BGUN_GETON_A,
	BGUN_GETOFF_A,
	BGUN_UPDOWN_A,
	BGUN_RECOIL_A
};

void FireBigGun(ITEM_INFO *obj)
{
		short itemNumber = CreateItem();
		if (itemNumber != NO_ITEM)
		{

			BIGGUNINFO *gun = (BIGGUNINFO*)obj->data;

			ITEM_INFO* item = &g_Level.Items[itemNumber];
			item->objectNumber = ID_ROCKET;
			item->roomNumber = LaraItem->roomNumber;


			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 256;//520;

			GetJointAbsPosition(obj, &pos, 2);

			
			item->pos.xPos = pos.x;
			item->pos.yPos = pos.y;
			item->pos.zPos = pos.z;

			InitialiseItem(itemNumber);


			item->pos.xRot = -((gun->xRot - 32) * (ANGLE(1)));
			item->pos.yRot = obj->pos.yRot;
			item->pos.zRot = 0;
			item->speed = 16;
			item->itemFlags[0] = 1;

			AddActiveItem(itemNumber);

			SmokeCountL = 32;
			SmokeWeapon = WEAPON_ROCKET_LAUNCHER;

			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, WEAPON_ROCKET_LAUNCHER, 32);

			SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
		}

}

static int CanUseGun(ITEM_INFO *obj, ITEM_INFO *lara)
{
	int dist;
	long long x, z;

	if ((!(TrInput & IN_ACTION))
		|| (Lara.gunStatus != LG_NO_ARMS)
		|| (lara->gravityStatus))
		return 0;

	x = lara->pos.xPos - obj->pos.xPos;
	z = lara->pos.zPos - obj->pos.zPos;

	dist = SQUARE(x) + SQUARE(z);

	if (dist > 30000)
		return 0;

	short ang = abs(lara->pos.yRot - obj->pos.yRot);
	if (ang > ANGLE(35.0f) || ang < -ANGLE(35.0f))
		return 0;

		return 1;
}

void BigGunInitialise(short itemNum)
{
	ITEM_INFO *obj;
	BIGGUNINFO *gun;

	obj = &g_Level.Items[itemNum];

	gun = (BIGGUNINFO*)malloc(sizeof(BIGGUNINFO));
	obj->data = malloc(sizeof(BIGGUNINFO));

	gun->flags = 0;
	gun->fireCount = 0;
	gun->xRot = GETOFF_FRAME;
	gun->yRot = 0;
	gun->startYRot = obj->pos.yRot;
}

void BigGunCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll)
{
	ITEM_INFO* obj;

	if (lara->hitPoints <= 0 || Lara.Vehicle != NO_ITEM)
		return;

	obj = &g_Level.Items[itemNum];

	if (CanUseGun(obj, lara))
	{
		BIGGUNINFO *gun;
		Lara.Vehicle = itemNum;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(ID_FLARE_ITEM, 0);
			undraw_flare_meshes();
			Lara.flareControlLeft = false;
			Lara.requestGunType = WEAPON_NONE;
			Lara.gunType = WEAPON_NONE;
		}

		Lara.gunStatus = LG_HANDS_BUSY;

		obj->hitPoints = 1;
		lara->pos.xPos = obj->pos.xPos;
		lara->pos.xRot = obj->pos.xRot;
		lara->pos.yPos = obj->pos.yPos;
		lara->pos.yRot = obj->pos.yRot;
		lara->pos.zPos = obj->pos.zPos;
		lara->pos.zRot = obj->pos.zRot;

		lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_GETON_A;
		lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_GETON_A].frameBase;
		lara->currentAnimState = BGUN_GETON;
		lara->goalAnimState = BGUN_GETON;

		gun = (BIGGUNINFO*)obj->data;
		gun->flags = 0;
		gun->xRot = GETOFF_FRAME;

	}
	else
		ObjectCollision(itemNum, lara, coll);
}

int BigGunControl(COLL_INFO *coll)
{
	BIGGUNINFO *gun;
	ITEM_INFO *obj;
	ITEM_INFO *lara;

	lara = LaraItem;
	obj = &g_Level.Items[Lara.Vehicle];
	gun = (BIGGUNINFO *)obj->data;

	if (gun->flags & FLAG_UPDOWN)
	{
		if (barrelRotating)
			gun->barrelZ--;

		if (!gun->barrelZ)
			barrelRotating = false;

		if ((lara->hitPoints <= 0) || (TrInput & IN_ROLL))
		{
			gun->flags = FLAG_AUTOROT;
		}
		else
		{
			if ((TrInput & IN_ACTION) && gun->fireCount == 0)
			{
				FireBigGun(obj);
				gun->fireCount = RECOIL_TIME;
				gun->barrelZ = RECOIL_Z;
				barrelRotating = true;
			}

			if (TrInput & IN_LEFT)
			{
				if (GunRotYAdd > 0)
					GunRotYAdd /= 2;

				GunRotYAdd -= 8;

				if (GunRotYAdd < -64)
					GunRotYAdd = -64;
			}
			else
			if (TrInput & IN_RIGHT)
			{		
				if (GunRotYAdd < 0)
					GunRotYAdd /= 2;

				GunRotYAdd += 8;

				if (GunRotYAdd > 64)
					GunRotYAdd = 64;
			}
			else
			{
				GunRotYAdd -= GunRotYAdd / 4;
				if (abs(GunRotYAdd) < 8)			
					GunRotYAdd = 0;
			}

			gun->yRot += GunRotYAdd / 4;

			if ((TrInput & IN_FORWARD) && (gun->xRot < BGUN_UPDOWN_FRAMES))
				gun->xRot++;			
			else 
			if ((TrInput & IN_BACK) && (gun->xRot))
				gun->xRot--;
		}
	}

	if (gun->flags & FLAG_AUTOROT)
	{
		if (gun->xRot == GETOFF_FRAME)
		{
			lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_GETOFF_A;
			lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_GETOFF_A].frameBase;
			lara->currentAnimState = BGUN_GETOFF;
			lara->goalAnimState = BGUN_GETOFF;
			gun->flags = FLAG_GETOFF;
		}
		else if (gun->xRot > GETOFF_FRAME)
			gun->xRot--;
		else if (gun->xRot < GETOFF_FRAME)
			gun->xRot++;
	}

	switch (lara->currentAnimState)
	{
	case BGUN_GETON:
	case BGUN_GETOFF:
		AnimateItem(lara);
		obj->animNumber = Objects[ID_BIGGUN].animIndex + (lara->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		obj->frameNumber = g_Level.Anims[obj->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);

		if ((gun->flags & FLAG_GETOFF) && lara->frameNumber == g_Level.Anims[lara->animNumber].frameEnd)
		{
			lara->animNumber = LA_STAND_SOLID;
			lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
			lara->currentAnimState = LS_STOP;
			lara->goalAnimState = LS_STOP;
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
			obj->hitPoints = 0;
		}
		break;
	case BGUN_UPDOWN:
		lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_UPDOWN_A;
		lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_UPDOWN_A].frameBase + gun->xRot;

		obj->animNumber = Objects[ID_BIGGUN].animIndex + (lara->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		obj->frameNumber = g_Level.Anims[obj->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);

		if (gun->fireCount > 0)
			gun->fireCount--;
		else if (gun->fireCount <= 0)
			gun->fireCount = 0;

		gun->flags = FLAG_UPDOWN;
		break;
	}
	
	lara->pos.yRot = obj->pos.yRot = gun->startYRot + (gun->yRot * (ANGLE(1) / 4));

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;
	DoObjectCollision(lara, coll);
	
	Camera.targetElevation = -ANGLE(15);
	return 1;
}
