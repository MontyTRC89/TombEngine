#include "framework.h"
#include "lara_flare.h"
#include "lara_tests.h"
#include "level.h"
#include "setup.h"
#include "Sound/sound.h"
#include "animation.h"
#include "items.h"
#include "lara_fire.h"
#include "Lara.h"
#include "collide.h"
#include "effects/effects.h"
#include "effects/chaffFX.h"
#include "Specific/prng.h"

constexpr std::array<float, 28> FlareFlickerTable = { 0.7590,0.9880,0.8790,0.920,0.8020,0.7610,0.97878,0.8978,0.9983,0.934763,0.8485,0.762573,0.84642,0.7896,0.817634,0.923424,0.7589,0.81399,0.92834,0.9978,0.7610,0.97878,0.8978,0.9983,0.934763,0.8485,0.762573,0.74642 };
constexpr DirectX::SimpleMath::Vector3 FlareMainColor = Vector3(1,0.52947, 0.3921);
constexpr std::array<float, 28> FlareFlickerTableLow = { 0.7590,0.1880,0.0790,0.920,0.8020,0.07610,0.197878,0.38978,0.09983,0.00934763,0.8485,0.0762573,0.84642,0.7896,0.517634,0.0923424,0.7589,0.081399,0.92834,0.01978,0.17610,0.497878,0.8978,0.69983,0.934763,0.28485,0.1762573,0.374642 };

using namespace TEN::Math::Random;

void FlareControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestLaraSwamp(item))
	{
		KillItem(itemNumber);
		return;
	}

	if (item->fallspeed)
	{
		item->pos.xRot += ANGLE(3);
		item->pos.zRot += ANGLE(5);
	}
	else
	{
		item->pos.xRot = 0;
		item->pos.zRot = 0;
	}

	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	int xv = item->speed * phd_sin(item->pos.yRot);
	int zv = item->speed * phd_cos(item->pos.yRot);

	item->pos.xPos += xv;
	item->pos.zPos += zv;


	if (g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		item->fallspeed += (5 - item->fallspeed) / 2;
		item->speed += (5 - item->speed) / 2;
	}
	else
		item->fallspeed += 6;

	item->pos.yPos += item->fallspeed;

	DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, item->fallspeed, zv);

	short& age = item->data;
	age &= 0x7FFF;
	if (age >= FLARE_AGE)
	{
		if (!item->fallspeed && !item->speed)
		{
			KillItem(itemNumber);
			return;
		}
	}
	else
	{
		age++;
	}

	if (DoFlareLight((PHD_VECTOR*)&item->pos, age))
	{
		TriggerChaffEffects(item,age);
		/* Hardcoded code */

		age |= 0x8000;
	}
}

void ready_flare()
{
	Lara.gunStatus = LG_NO_ARMS;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
	Lara.target = NULL;
}

void undraw_flare_meshes()
{
	Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void draw_flare_meshes()
{
	Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND;
}

void undraw_flare()
{
	Lara.flareControlLeft = true;

	short frame1 = Lara.flareFrame;
	short frame2 = Lara.leftArm.frameNumber;

	if (LaraItem->goalAnimState == LS_STOP 
		&& Lara.Vehicle == NO_ITEM)
	{
		if (LaraItem->animNumber == LA_STAND_IDLE)
		{
			LaraItem->animNumber = LA_DISCARD_FLARE;
			frame1 = frame2 + g_Level.Anims[LaraItem->animNumber].frameBase;
			Lara.flareFrame = frame1;
			LaraItem->frameNumber = frame1;
		}

		if (LaraItem->animNumber == LA_DISCARD_FLARE)
		{
			Lara.flareControlLeft = false;

			if (frame1 >= g_Level.Anims[LaraItem->animNumber].frameBase + 31)
			{
				Lara.requestGunType = Lara.lastGunType;
				Lara.gunType = Lara.lastGunType;
				Lara.gunStatus = LG_NO_ARMS;

				InitialiseNewWeapon();

				Lara.target = NULL;
				Lara.rightArm.lock = false;
				Lara.leftArm.lock = false;
				LaraItem->animNumber = LA_STAND_SOLID;
				Lara.flareFrame = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LS_STOP;
				LaraItem->goalAnimState = LS_STOP;
				return;
			}

			Lara.flareFrame++;
		}
	}
	else if (LaraItem->animNumber == LA_DISCARD_FLARE)
	{
		LaraItem->animNumber = LA_STAND_SOLID;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
	}

	if (frame2 >= 33 && frame2 < 72)
	{
		frame2 = 2;
		DoFlareInHand(Lara.flareAge);
	}
	else if (!frame2)
	{
		frame2 = 1;
		DoFlareInHand(Lara.flareAge);
	}
	else if (frame2 >= 72 && frame2 < 95)
	{
		frame2++;
		if (frame2 == 94)
		{
			frame2 = 1;
			DoFlareInHand(Lara.flareAge);
		}
	}
	else if (frame2 >= 1 && frame2 < 33)
	{
		frame2++;
		if (frame2 == 21)
		{
			CreateFlare(ID_FLARE_ITEM, 1);
			undraw_flare_meshes();
			Lara.flareAge = 0;
		}
		else if (frame2 == 33)
		{
			frame2 = 0;
			Lara.requestGunType = Lara.lastGunType;
			Lara.gunType = Lara.lastGunType;
			Lara.gunStatus = LG_NO_ARMS;

			InitialiseNewWeapon();

			Lara.flareControlLeft = false;
			Lara.target = NULL;
			Lara.rightArm.lock = false;
			Lara.leftArm.lock = false;
			Lara.flareFrame = 0;
		}
		else if (frame2 < 21)
		{
			DoFlareInHand(Lara.flareAge);
		}
	}
	else if (frame2 >= 95 && frame2 < 110)
	{
		frame2++;
		if (frame2 == 110)
		{
			frame2 = 1;
			DoFlareInHand(Lara.flareAge);
		}
	}

	Lara.leftArm.frameNumber = frame2;
	set_flare_arm(Lara.leftArm.frameNumber);
}

void draw_flare()
{
	short frame;

	if (LaraItem->currentAnimState == LS_PICKUP_FLARE ||
		LaraItem->currentAnimState == LS_PICKUP)
	{
		DoFlareInHand(Lara.flareAge);
		Lara.flareControlLeft = false;
		Lara.leftArm.frameNumber = 93;
		set_flare_arm(93);
	}
	else
	{
		frame = Lara.leftArm.frameNumber + 1;
		Lara.flareControlLeft = true;

		if (frame < 33 || frame > 94)
		{
			frame = 33;
		}
		else if (frame == 46)
		{
			draw_flare_meshes();
		}
		else if (frame >= 72 && frame <= 93)
		{
			if (frame == 72)
			{
				SoundEffect(SFX_TR4_OBJ_GEM_SMASH, &LaraItem->pos, g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER);
				Lara.flareAge = 1;
			}

			DoFlareInHand(Lara.flareAge);
		}
		else
		{
			if (frame == 94)
			{
				ready_flare();
				frame = 0;
				DoFlareInHand(Lara.flareAge);
			}
		}

		Lara.leftArm.frameNumber = frame;
		set_flare_arm(frame);
	}
}

void set_flare_arm(int frame)
{
	short anim = Objects[ID_LARA_FLARE_ANIM].animIndex;

	if (frame >= 95)
	{
		anim += 4;
	}
	else if (frame >= 72)
	{
		anim += 3;
	}
	else if (frame >= 33)
	{
		anim += 2;
	}
	else if (frame >= 1)
	{
		anim += 1;
	}

	Lara.leftArm.animNumber = anim;
	Lara.leftArm.frameBase = g_Level.Anims[anim].framePtr;
}

void CreateFlare(GAME_OBJECT_ID objectNum, int thrown)
{
	short itemNum = CreateItem();
	if (itemNum != NO_ITEM)
	{
		bool flag = false;
		ITEM_INFO* item = &g_Level.Items[itemNum];
		item->objectNumber = objectNum;
		item->roomNumber = LaraItem->roomNumber;

		PHD_VECTOR pos;
		pos.x = -16;
		pos.y = 32;
		pos.z = 42;

		GetLaraJointPosition(&pos, LM_LHAND);

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;

		short roomNumber = LaraItem->roomNumber;
		FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		int collided = GetCollidedObjects(item, 0, 1, CollidedItems, CollidedMeshes, true);
		if (collided || GetFloorHeight(floor, pos.x, pos.y, pos.z) < pos.y)
		{
			flag = true;
			item->pos.yRot = LaraItem->pos.yRot + ANGLE(180);
			item->pos.xPos = LaraItem->pos.xPos + 320 * phd_sin(item->pos.yRot);
			item->pos.zPos = LaraItem->pos.zPos + 320 * phd_cos(item->pos.yRot);
			item->roomNumber = LaraItem->roomNumber;
		}
		else
		{
			if (thrown)
				item->pos.yRot = LaraItem->pos.yRot;
			else
				item->pos.yRot = LaraItem->pos.yRot - ANGLE(45);
			item->roomNumber = roomNumber;
		}

		InitialiseItem(itemNum);

		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->shade = -1;

		if (thrown)
		{
			item->speed = LaraItem->speed + 50;
			item->fallspeed = LaraItem->fallspeed - 50;
		}
		else
		{
			item->speed = LaraItem->speed + 10;
			item->fallspeed = LaraItem->fallspeed + 50;
		}

		if (flag)
			item->speed >>= 1;

		if (objectNum == ID_FLARE_ITEM)
		{
			item->data = (short)0;
			short& age = item->data;
			if (DoFlareLight((PHD_VECTOR*)&item->pos, Lara.flareAge))
				age = (Lara.flareAge | 0x8000);
			else
				age = (Lara.flareAge & 0x7FFF);
		}
		else
		{
			item->itemFlags[3] = Lara.litTorch;
		}

		AddActiveItem(itemNum);
		item->status = ITEM_ACTIVE;
	}
}

void DrawFlareInAir(ITEM_INFO* item)
{
	TENLog("DrawFlareInAir() not implemented!", LogLevel::Warning);
}

void DoFlareInHand(int flare_age)
{
	PHD_VECTOR pos;

	pos.x = 11;
	pos.y = 32;
	pos.z = 41;

	GetLaraJointPosition(&pos, LM_LHAND);
	if (DoFlareLight(&pos, flare_age))
		TriggerChaffEffects(flare_age);

	/* Hardcoded code */

	if (Lara.flareAge >= FLARE_AGE)
	{
		if (Lara.gunStatus == LG_NO_ARMS)
			Lara.gunStatus = LG_UNDRAW_GUNS;
	}
	else if (Lara.flareAge != 0)
	{
		Lara.flareAge++;
	}
}

int DoFlareLight(PHD_VECTOR* pos, int age)
{
	int x, y, z;
	int r, g, b;
	float random;
	int falloff;
	if (age >= FLARE_AGE || age == 0)
		return 0;
	random = GenerateFloat();

	x = pos->x + (random* 120);
	y = pos->y + (random * 120) - 256;
	z = pos->z + (random * 120);

	if (age < 4)
	{
		falloff = 12 + ((1-(age / 4.0f))*16);
		
		r = FlareMainColor.x*255;
		g = FlareMainColor.y * 255;
		b = FlareMainColor.z * 255;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return (random < 0.9f);
	}
	else if (age < (FLARE_AGE - 90))
	{
		float multiplier = FlareFlickerTable[age % FlareFlickerTable.size()];
		falloff = 12*multiplier;

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return (random < 0.4f);
	}
	else 
	{
		float multiplier = FlareFlickerTableLow[age % FlareFlickerTableLow.size()];
		
		falloff = 12 * (1.0f - ((age - (FLARE_AGE - 90)) / (FLARE_AGE - (FLARE_AGE - 90))));
		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
			TriggerDynamicLight(x, y, z, falloff, r, g, b);
			return (random < .3f);
		
	}

}
