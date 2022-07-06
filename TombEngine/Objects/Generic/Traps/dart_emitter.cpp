#include "framework.h"
#include "Objects/Generic/Traps/dart_emitter.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Entities::Traps
{
	void DartControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TouchBits)
		{
			DoDamage(LaraItem, 25);
			DoBloodSplat(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, (GetRandomControl() & 3) + 4, LaraItem->Pose.Orientation.y, LaraItem->RoomNumber);
			Lara.PoisonPotency += 1; // Was 160 with the total poison potency later shifted right by 8 when applied to Lara's health. The effect was that each dart contributed a mere fraction to the potency. @Sezz 2022.03.09
			KillItem(itemNumber);
		}
		else
		{
			int oldX = item->Pose.Position.x;
			int oldZ = item->Pose.Position.z - 1000;

			int velocity = item->Animation.Velocity * phd_cos(item->Pose.Orientation.x);

			item->Pose.Position.x += velocity * phd_sin(item->Pose.Orientation.y);
			item->Pose.Position.y -= item->Animation.Velocity * phd_sin(item->Pose.Orientation.x);
			item->Pose.Position.z += velocity * phd_cos(item->Pose.Orientation.y);

			short roomNumber = item->RoomNumber;
			FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

			if (item->RoomNumber != roomNumber)
				ItemNewRoom(itemNumber, roomNumber);

			int height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
			item->Floor = height;

			if (item->Pose.Position.y >= height)
			{
				for (int i = 0; i < 4; i++)
					TriggerDartSmoke(oldX, item->Pose.Position.y, oldZ, 0, 0, true);

				KillItem(itemNumber);
			}
		}
	}

	void DartEmitterControl(short itemNumber)
	{
		ItemInfo* item = &g_Level.Items[itemNumber];

		if (item->Active)
		{
			if (item->Timer > 0)
			{
				item->Timer--;
				return;
			}
			else
				item->Timer = 24;
		}

		short dartItemNumber = CreateItem();

		if (dartItemNumber != NO_ITEM)
		{
			ItemInfo* dartItem = &g_Level.Items[dartItemNumber];

			dartItem->ObjectNumber = ID_DARTS;
			dartItem->RoomNumber = item->RoomNumber;

			int x = 0;
			int z = 0;

			switch (item->Pose.Orientation.y)
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

			dartItem->Pose.Position.x = x + item->Pose.Position.x;
			dartItem->Pose.Position.y = item->Pose.Position.y - WALL_SIZE / 2;
			dartItem->Pose.Position.z = z + item->Pose.Position.z;

			InitialiseItem(dartItemNumber);

			dartItem->Pose.Orientation.x = 0;
			dartItem->Pose.Orientation.y = item->Pose.Orientation.y + -ANGLE(180);
			dartItem->Animation.Velocity = 256;

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

				TriggerDartSmoke(dartItem->Pose.Position.x, dartItem->Pose.Position.y, dartItem->Pose.Position.z, xv, zv, false);
			}

			AddActiveItem(dartItemNumber);
			dartItem->Status = ITEM_ACTIVE;

			SoundEffect(SFX_TR4_DART_SPIT, &dartItem->Pose);
		}
	}

	void TriggerDartSmoke(int x, int y, int z, int xv, int zv, bool hit)
	{
		int dx = LaraItem->Pose.Position.x - x;
		int dz = LaraItem->Pose.Position.z - z;

		if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
			return;

		auto* spark = GetFreeParticle();

		spark->on = true;
		
		spark->sR = 16;
		spark->sG = 8;
		spark->sB = 4;
		
		spark->dR = 64;
		spark->dG = 48;
		spark->dB = 32;

		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 4;

		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

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