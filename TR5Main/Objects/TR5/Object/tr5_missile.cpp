#include "framework.h"
#include "tr5_missile.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"
#include "Game/effects/debris.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "tr5_roman_statue.h"
#include "tr5_hydra.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/lara_fx.h"

using namespace TEN::Effects::Lara;

int DebrisFlags;

void MissileControl(short itemNumber)
{
	auto* fx = &EffectList[itemNumber];
	if (fx->flag1 == 2)
	{
		fx->pos.zRot += 16 * fx->speed;

		if (fx->speed > 64)
			fx->speed -= 4;

		if (fx->pos.xRot > -12288)
		{
			if (fx->fallspeed < 512)
				fx->fallspeed += 36;
			fx->pos.xRot -= fx->fallspeed;
		}
	}
	else
	{
		short angles[2];
		phd_GetVectorAngles(
			LaraItem->Position.xPos - fx->pos.xPos,
			LaraItem->Position.yPos - fx->pos.yPos - 256,
			LaraItem->Position.zPos - fx->pos.zPos,
			angles);

		int dh;
		if (fx->flag1)
			dh = fx->flag1 != 1 ? 768 : 384;
		else
		{
			if (fx->counter)
				fx->counter--;
			dh = 256;
		}

		if (fx->speed < 192)
		{
			if (fx->flag1 == 0 || fx->flag1 == 1)
				fx->speed++;

			int dy = angles[0] - fx->pos.yRot;
			if (abs(dy) > 0x8000)
				dy = -dy;
			dy /= 8;

			int dx = angles[1] - fx->pos.xRot;
			if (abs(dx) > 0x8000)
				dx = -dx;
			dx /= 8;

			if (dy <= dh)
			{
				if (dy < -dh)
					dy = -dh;
			}
			else
				dy = dh;
			
			if (dx <= dh)
			{
				if (dx < -dh)
					dx = -dh;
			}
			else
				dx = dh;

			fx->pos.yRot += dy;
			fx->pos.xRot += dx;
		}
		
		fx->pos.zRot += 16 * fx->speed;

		if (!fx->flag1)
			fx->pos.zRot += 16 * fx->speed;
	}

	int x = fx->pos.xPos;
	int y = fx->pos.yPos;
	int z = fx->pos.zPos;

	int c = fx->speed * phd_cos(fx->pos.xRot);

	fx->pos.xPos += c * phd_sin(fx->pos.yRot);
	fx->pos.yPos += fx->speed * phd_sin(-fx->pos.xRot);
	fx->pos.zPos += c * phd_cos(fx->pos.yRot);

	auto probe = GetCollisionResult(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
	
	if (fx->pos.yPos >= probe.Position.Floor || fx->pos.yPos <= probe.Position.Ceiling)
	{
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;

		if (fx->flag1)
		{
			if (fx->flag1 == 1)
			{
				TriggerExplosionSparks(x, y, z, 3, -2, 2, fx->roomNumber);
				fx->pos.yPos -= 64;
				TriggerShockwave((PHD_3DPOS*)fx, 48, 256, 64, 64, 128, 0, 24, 0, 1);
				fx->pos.yPos -= 128;
				TriggerShockwave((PHD_3DPOS*)fx, 48, 256, 48, 64, 128, 0, 24, 0, 1);
			}
			else if (fx->flag1 == 2)
			{
				ExplodeFX(fx, 0, 32);
				SoundEffect(251, &fx->pos, 0);
			}
		}
		else
		{
			TriggerExplosionSparks(x, y, z, 3, -2, 0, fx->roomNumber);
			TriggerShockwave((PHD_3DPOS*)fx, 48, 240, 48, 0, 96, 128, 24, 0, 2);
		}
		
		KillEffect(itemNumber);
	}
	else if (ItemNearLara((PHD_3DPOS*)fx, 200))
	{
		LaraItem->HitStatus = true;
		
		if (fx->flag1)
		{
			if (fx->flag1 == 1)
			{
				// ROMAN_GOD hit effect
				TriggerExplosionSparks(x, y, z, 3, -2, 2, fx->roomNumber);
				fx->pos.yPos -= 64;
				TriggerShockwave((PHD_3DPOS*)fx, 48, 256, 64, 0, 128, 64, 24, 0, 1);
				fx->pos.yPos -= 128;
				TriggerShockwave((PHD_3DPOS*)fx, 48, 256, 48, 0, 128, 64, 24, 0, 1);
				LaraItem->HitPoints -= 200;			
				KillEffect(itemNumber);
			}
			else
			{
				if (fx->flag1 == 2)
				{
					// IMP hit effect
					ExplodeFX(fx, 0, 32);
					LaraItem->HitPoints -= 50;
					DoBloodSplat(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, (GetRandomControl() & 3) + 2, LaraItem->Position.yRot, LaraItem->RoomNumber);
					SoundEffect(SFX_TR5_IMP_STONE_HIT, &fx->pos, 0);
					SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->Position, 0);
				}
				
				KillEffect(itemNumber);
			}
		}
		else
		{
			// HYDRA hit effect
			TriggerExplosionSparks(x, y, z, 3, -2, 0, fx->roomNumber);
			TriggerShockwave((PHD_3DPOS*)fx, 48, 240, 48, 0, 96, 128, 24, 0, 0);
			if (LaraItem->HitPoints >= 500)
				LaraItem->HitPoints -= 300;
			else
				LaraBurn(LaraItem);
			KillEffect(itemNumber);
		}
	}
	else
	{
		if (probe.RoomNumber != fx->roomNumber)
			EffectNewRoom(itemNumber, probe.RoomNumber);

		if (GlobalCounter & 1)
		{
			PHD_VECTOR pos = { x, y, z };

			int xv = x - fx->pos.xPos;
			int yv = y - fx->pos.yPos;
			int zv = z - fx->pos.zPos;

			if (fx->flag1 == 1)
				TriggerRomanStatueMissileSparks(&pos, itemNumber);
			else
			{
				TriggerHydraMissileSparks(&pos, 4 * xv, 4 * yv, 4 * zv);
				TriggerHydraMissileSparks((PHD_VECTOR*)&fx, 4 * xv, 4 * yv, 4 * zv);
			}
		}
	}
}

void ExplodeFX(FX_INFO* fx, int noXZVel, int bits)
{
	ShatterItem.yRot = fx->pos.yRot;
	ShatterItem.meshIndex = fx->frameNumber;
	ShatterItem.sphere.x = fx->pos.xPos;
	ShatterItem.sphere.y = fx->pos.yPos;
	ShatterItem.sphere.z = fx->pos.zPos;
	ShatterItem.bit = 0;
	ShatterItem.flags = fx->flag2 & 0x1400;

	if (fx->flag2 & 0x2000)
		DebrisFlags = 1;

	ShatterObject(&ShatterItem, 0, bits, fx->roomNumber, noXZVel);

	DebrisFlags = 0;
}
