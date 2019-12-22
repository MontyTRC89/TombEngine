#include "laraflar.h"

#include "..\Global\global.h"

#include "draw.h"
#include "items.h"
#include "sphere.h"
#include "larafire.h"
#include "Lara.h"
#include "collide.h"
#include "effect2.h"

extern LaraExtraInfo g_LaraExtra;

void FlareControl(short itemNumber)//4A418, 4A87C
{
	ITEM_INFO* item = &Items[itemNumber];
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

	int xv = 0;
	int zv = 0;

	item->pos.xPos += (xv = (item->speed * SIN(item->pos.yRot) >> W2V_SHIFT));
	item->pos.zPos += (zv = (item->speed * COS(item->pos.yRot) >> W2V_SHIFT));


	if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->fallspeed += (5 - item->fallspeed) / 2;
		item->speed += (5 - item->speed) / 2;
	}
	else
		item->fallspeed += 6;
	
	item->pos.yPos += item->fallspeed;

	DoProperDetection(itemNumber, oldX, oldY, oldZ, xv, zv);
	
	short age = (short)(item->data) & 0x7FFF;
	if (age >= 900)
	{
		if (!item->fallspeed && !item->speed)
			KillItem(itemNumber);
	}
	else
	{
		age++;
	}
	
	if (DoFlareLight((PHD_VECTOR*)&item->pos, age))
	{
		/*if (gfLevelFlags & 0x2000 && item->roomNumber == gfMirrorRoom)
		{
			item->pos.zPos = 2 * gfMirrorZPlane - item->pos.zPos;
			sub_401807(&item->pos, v9);
			v10 = 2 * gfMirrorZPlane - item->pos.zPos;
			item->pos.zPos = v10;
		}*/
		age |= 0x8000;
	}

	item->data = (void*)age;
}

void ready_flare()//4A3E4(<), 4A848(<) (F)
{
	Lara.gunStatus = LG_NO_ARMS;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.rightArm.lock = 0;
	Lara.leftArm.lock = 0;
	Lara.target = NULL;
}

void undraw_flare_meshes() 
{
	LARA_MESHES(ID_LARA, LM_LHAND);
}

void draw_flare_meshes()//4A394(<), 4A7F8(<) (F)
{
	LARA_MESHES(ID_FLARE_ANIM, LM_LHAND);
}

void undraw_flare()//4A108, 4A56C
{
	Lara.flareControlLeft = true;

	short frame1 = Lara.flareFrame;
	short frame2 = Lara.leftArm.frameNumber;
	
	if (LaraItem->goalAnimState == STATE_LARA_STOP &&
		g_LaraExtra.Vehicle == NO_ITEM)
	{
		if (LaraItem->animNumber == ANIMATION_LARA_STAY_IDLE)
		{
			LaraItem->animNumber = ANIMATION_LARA_FLARE_THROW;
			frame1 = frame2 + Anims[LaraItem->animNumber].frameBase;
			Lara.flareFrame = frame2 + Anims[LaraItem->animNumber].frameBase;
			LaraItem->frameNumber = Lara.flareFrame;
		}

		if (LaraItem->animNumber == ANIMATION_LARA_FLARE_THROW)
		{
			Lara.flareControlLeft = false;

			if (frame1 >= Anims[LaraItem->animNumber].frameBase + 31)
			{
				Lara.requestGunType = Lara.lastGunType;
				Lara.gunType = Lara.lastGunType;
				Lara.gunStatus = LG_NO_ARMS;

				InitialiseNewWeapon();

				Lara.target = NULL;
				Lara.rightArm.lock = 0;
				Lara.leftArm.lock = 0;
				LaraItem->animNumber = ANIMATION_LARA_STAY_SOLID;
				Lara.flareFrame = Anims[LaraItem->animNumber].frameBase;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = STATE_LARA_STOP;
				LaraItem->goalAnimState = STATE_LARA_STOP;
				return;
			}

			Lara.flareFrame++;
		}
	}
	else if (LaraItem->currentAnimState == STATE_LARA_STOP)
	{
		LaraItem->animNumber = ANIMATION_LARA_STAY_SOLID;
		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
	}

	if (!frame2)
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
		}
	}
	else if (frame2 >= 1 || frame2 < 33)
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
			Lara.rightArm.lock = 0;
			Lara.leftArm.lock = 0;
			Lara.flareFrame = 0;
		}
	}
	if (frame2 >= 95 && frame2 < 110)
	{
		frame2++;
		if (frame2 == 110)
			frame2 = 1;
	}

	if (frame2 >= 1 && frame2 < 21)
		DoFlareInHand(Lara.flareAge);

	Lara.leftArm.frameNumber = frame2;
	set_flare_arm(Lara.leftArm.frameNumber);
}

void draw_flare()//49F74, 4A3D8 (F)
{
	short frame;

	if (LaraItem->currentAnimState == STATE_LARA_FLARE_PICKUP ||
		LaraItem->currentAnimState == STATE_LARA_PICKUP)
	{
		DoFlareInHand(Lara.flareAge);
		Lara.flareControlLeft = false;
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
				SoundEffect(SFX_RAVESTICK, &LaraItem->pos, Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER);
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

void set_flare_arm(int frame)//49ED4, 4A338 (F)
{
	short anim = Objects[ID_FLARE_ANIM].animIndex;

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
	Lara.leftArm.frameBase = Anims[anim].framePtr;
}

void CreateFlare(short objectNum, int thrown)//49BBC, 4A020
{
	short itemNum = CreateItem();
	if (itemNum != NO_ITEM)
	{
		bool flag = false;
		ITEM_INFO* item = &Items[itemNum];
		item->objectNumber = objectNum;
		item->roomNumber = LaraItem->roomNumber;

		PHD_VECTOR pos;
		pos.x = -16;
		pos.y = 32;
		pos.z = 42;

		GetLaraJointPosition(&pos, LJ_LHAND);

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;

		ITEM_INFO* tmp1[128];
		MESH_INFO* tmp2[128];

		short roomNumber = LaraItem->roomNumber;
		FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		int collided = GetCollidedObjects(item, 0, 1, &tmp1[0], &tmp2[0], 0);
		if (collided || GetFloorHeight(floor, pos.x, pos.y, pos.z) < pos.y)
		{
			flag = true;
			item->pos.yRot = LaraItem->pos.yRot - ANGLE(180);
			item->pos.xPos = LaraItem->pos.xPos + 80 * SIN(item->pos.yRot) >> W2V_SHIFT;
			item->pos.zPos = LaraItem->pos.zPos + 80 * COS(item->pos.yRot) >> W2V_SHIFT;
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
			if (DoFlareLight((PHD_VECTOR*)&item->pos, Lara.flareAge))
				item->data = (void*)(Lara.flareAge | 0x8000);
			else
				item->data = (void*)(Lara.flareAge & 0x7FFF);
		}
		else
		{
			item->itemFlags[3] = (Lara.currentZvel >> W2V_SHIFT) & 1;
		}

		AddActiveItem(itemNum);
		item->status = ITEM_ACTIVE;
	}
}

void DoFlareInHand(int flare_age)//49984, 49DE8
{
	PHD_VECTOR pos;

	pos.x = 11;
	pos.y = 32;
	pos.z = 41;

	GetLaraJointPosition(&pos, LJ_LHAND);
	DoFlareLight(&pos, flare_age);

	/*if (gfLevelFlags & GF_LVOP_MIRROR_USED && LaraItem->room_number == gfMirrorRoom)
	{
		pos.z = 2 * gfMirrorZPlane - pos.z;

		DoFlareLight(&pos, flare_age);
	}*/

	if (Lara.flareAge >= 900)
	{
		if (Lara.gunStatus == LG_NO_ARMS)
			Lara.gunStatus = LG_UNDRAW_GUNS;
	}
	else if (Lara.flareAge != 0)
	{
		Lara.flareAge++;
	}
}

int DoFlareLight(PHD_VECTOR* pos, int age)//49708, 49B6C (F)
{
	int x, y, z;
	int r, g, b;
	int random;
	int falloff;

	if (age >= 900 || age == 0)
		return 0;

	random = GetRandomControl();

	x = pos->x + (random << 3 & 120);
	y = pos->y + (random >> 1 & 120) - 256;
	z = pos->z + (random >> 5 & 120);

	if (age < 4)
	{
		falloff = (random & 3) + 4 * (age + 1);
		if (falloff > 16)
			falloff -= (random >> 12) & 3;

		r = (random >> 4 & 0x1F) + 8 * (age + 4);
		g = (random & 0x1F) + 16 * (age + 10);
		b = (random >> 8 & 0x1F) + 16 * age;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (age < 16)
	{
		falloff = (random & 1) + age + 2;

		r = (random >> 4 & 0x1F) + 4 * age + 64;
		g = (random & 0x3F) + 4 * age + 128;
		b = (random >> 8 & 0x1F) + 4 * age + 16;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (age < 810 || random & 0x2000)
	{
		falloff = 16;

		r = (random >> 4 & 0x1F) + 128;
		g = (random & 0x3F) + 192;
		b = (random >> 8 & 0x20) + (random >> 8 & 0x3F);

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (age >= 876)
	{
		falloff = 16 - ((age - 876) >> 1);

		r = (GetRandomControl() & 0x3F) + 64;
		g = (GetRandomControl() & 0x3F) + 192;
		b = GetRandomControl() & 0x1F;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return random & 1;
	}
	else
	{
		falloff = (GetRandomControl() & 6) + 8;

		r = (GetRandomControl() & 0x3F) + 64;
		g = (GetRandomControl() & 0x3F) + 192;
		b = GetRandomControl() & 0x7F;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);
		return 0;
	}
}

void Inject_LaraFlar()
{

}
