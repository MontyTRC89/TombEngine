#include "framework.h"
#include "Objects/Generic/Traps/dart_emitter.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Sound/sound.h"

namespace TEN::Entities::Traps
{
	void DartControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->TouchBits)
		{
			LaraItem->HitPoints -= 25;
			LaraItem->HitStatus = true;
			Lara.Poisoned += 160;
			DoBloodSplat(item->Position.xPos, item->Position.yPos, item->Position.zPos, (GetRandomControl() & 3) + 4, LaraItem->Position.yRot, LaraItem->RoomNumber);
			KillItem(itemNumber);
		}
		else
		{
			int oldX = item->Position.xPos;
			int oldZ = item->Position.zPos-1000;

			int speed = item->Velocity * phd_cos(item->Position.xRot);

			item->Position.xPos += speed * phd_sin(item->Position.yRot);
			item->Position.yPos -= item->Velocity * phd_sin(item->Position.xRot);
			item->Position.zPos += speed * phd_cos(item->Position.yRot);

			short roomNumber = item->RoomNumber;
			FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

			if (item->RoomNumber != roomNumber)
				ItemNewRoom(itemNumber, roomNumber);

			int height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			item->Floor = height;

			if (item->Position.yPos >= height)
			{
				for (int i = 0; i < 4; i++)
				{
					TriggerDartSmoke(oldX, item->Position.yPos, oldZ, 0, 0, true);
				}

				KillItem(itemNumber);
			}
		}
	}

	void DartEmitterControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->Active)
		{
			if (item->Timer > 0)
			{
				item->Timer--;
				return;
			}
			else
			{
				item->Timer = 24;
			}
		}

		short dartItemNumber = CreateItem();

		if (dartItemNumber != NO_ITEM)
		{
			ITEM_INFO* dartItem = &g_Level.Items[dartItemNumber];

			dartItem->ObjectNumber = ID_DARTS;
			dartItem->RoomNumber = item->RoomNumber;

			int x = 0;
			int z = 0;

			switch (item->Position.yRot)
			{
			case 0:
				z = WALL_SIZE / 2;
				break;

			case 0x4000:
				x = WALL_SIZE / 2;
				break;

			case -0x8000:
				z = -WALL_SIZE / 2;
				break;

			case -0x4000:
				x = -WALL_SIZE / 2;
				break;
			}

			dartItem->Position.xPos = x + item->Position.xPos;
			dartItem->Position.yPos = item->Position.yPos - WALL_SIZE / 2;
			dartItem->Position.zPos = z + item->Position.zPos;

			InitialiseItem(dartItemNumber);

			dartItem->Position.xRot = 0;
			dartItem->Position.yRot = item->Position.yRot + -ANGLE(180);
			dartItem->Velocity = 256;

			int xf = 0;
			int zf = 0;

			if (x)
				xf = abs(2 * x) - 1;
			else
				zf = abs(2 * z) - 1;

			for (int i = 0; i < 5; i++)
			{
				int random = -GetRandomControl();

				int xv = 0;
				int zv = 0;

				if (z >= 0)
					zv = zf & random;
				else
					zv = -(zf & random);

				if (x >= 0)
					xv = xf & random;
				else
					xv = -(xf & random);

				TriggerDartSmoke(dartItem->Position.xPos, dartItem->Position.yPos, dartItem->Position.zPos, xv, zv, false);
			}

			AddActiveItem(dartItemNumber);
			dartItem->Status = ITEM_ACTIVE;

			SoundEffect(SFX_TR4_DART_SPITT, &dartItem->Position, 0);
		}
	}

	void TriggerDartSmoke(int x, int y, int z, int xv, int zv, bool hit)
	{
		int dx = LaraItem->Position.xPos - x;
		int dz = LaraItem->Position.zPos - z;

		if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
			return;

		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;
		
		spark->sR = 16;
		spark->sG = 8;
		spark->sB = 4;
		
		spark->dR = 64;
		spark->dG = 48;
		spark->dB = 32;

		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 4;

		spark->transType = TransTypeEnum::COLADD;

		spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	
		spark->x = x + ((GetRandomControl() & 31) - 16);
		spark->y = y + ((GetRandomControl() & 31) - 16);
		spark->z = z + ((GetRandomControl() & 31) - 16);
		
		if (hit)
		{
			spark->xVel = -xv + ((GetRandomControl() & 255) - 128);
			spark->yVel = -(GetRandomControl() & 3) - 4;
			spark->zVel = -zv + ((GetRandomControl() & 255) - 128);
			spark->friction = 3;
		}
		else
		{
			if (xv)
				spark->xVel = -xv;
			else
				spark->xVel = ((GetRandomControl() & 255) - 128);
			spark->yVel = -(GetRandomControl() & 3) - 4;
			if (zv)
				spark->zVel = -zv;
			else
				spark->zVel = ((GetRandomControl() & 255) - 128);
			spark->friction = 3;
		}

		spark->friction = 3;

		if (GetRandomControl() & 1)
		{
			spark->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
			spark->rotAng = GetRandomControl() & 0xFFF;
			if (GetRandomControl() & 1)
				spark->rotAdd = -16 - (GetRandomControl() & 0xF);
			else
				spark->rotAdd = (GetRandomControl() & 0xF) + 16;
		}
		else
		{
			spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
		}
	
		spark->scalar = 1;

		int size = (GetRandomControl() & 63) + 72;
		if (hit)
		{
			size >>= 1;
			spark->size = spark->sSize = size >> 2;
			spark->gravity = spark->maxYvel = 0;
		}
		else
		{
			spark->size = spark->sSize = size >> 4;
			spark->gravity = -(GetRandomControl() & 3) - 4;
			spark->maxYvel = -(GetRandomControl() & 3) - 4;
		}

		spark->dSize = size;
	}
}