#include "framework.h"
#include "tr4_teethspike.h"
#include "control.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/trmath.h"
#include "Specific/setup.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	short SPyoffs[8] =
	{
		-1024, 0, -512, 0, 0, 0, -512, 0
	};

	short SPxzoffs[8] =
	{
		0, 0, 512, 0, 0, 0, -512, 0
	};

	short SPDETyoffs[8] =
	{
		1024, 512, 512, 512, 0, 512, 512, 512
	};

	void InitialiseTeethSpikes(short itemNumber)
	{
		ITEM_INFO* item;
		int angle;

		item = &g_Level.Items[itemNumber];
		item->status = ITEM_INVISIBLE;

		// Set mutators to 0 by default
		for (int i = 0; i < item->mutator.size(); i++)
			item->mutator[i].Scale.y = 0.0f;

		short rotations[8] =
		{
			ANGLE(180),
			ANGLE(225),
			ANGLE(270),
			ANGLE(315),
			ANGLE(0),
			ANGLE(45),
			ANGLE(90),
			ANGLE(135)
		};

		if (item->triggerFlags & 8)
		{
			angle = item->triggerFlags & 7;
			item->pos.xRot = rotations[angle];
			item->pos.yRot = ANGLE(90.0f);
			item->pos.zPos -= SPxzoffs[angle];
		}
		else
		{
			angle = item->triggerFlags & 7;
			item->pos.xPos += SPxzoffs[angle];
			item->pos.zRot = rotations[angle];
		}

		item->itemFlags[0] = 1024;
		item->itemFlags[2] = 0;
		item->pos.yPos += SPyoffs[angle];
	}

	bool TestBoundsCollideTeethSpikes(ITEM_INFO* item)
	{
		int x;
		int z;

		short angle = item->triggerFlags & 7;

		if (item->triggerFlags & 8)
		{
			x = (item->pos.xPos & (~1023)) | 512;
			z = ((item->pos.zPos + SPxzoffs[angle]) & (~1023)) | 512;
		}
		else
		{
			x = ((item->pos.xPos - SPxzoffs[angle]) & (~1023)) | 512;
			z = (item->pos.zPos & (~1023)) | 512;
		}

		int size = (item->triggerFlags & 1 ? 300 : 480);
		int y = item->pos.yPos + SPDETyoffs[angle];

		ANIM_FRAME* frames = GetBestFrame(LaraItem);

		if (LaraItem->pos.yPos + frames->boundingBox.Y1 <= y
			&& LaraItem->pos.yPos + frames->boundingBox.Y2 >= y - 900)
		{
			if (LaraItem->pos.xPos + frames->boundingBox.X1 <= (x + size)
				&& LaraItem->pos.xPos + frames->boundingBox.X2 >= (x - size))
			{
				if (LaraItem->pos.zPos + frames->boundingBox.Z1 <= (z + size)
					&& LaraItem->pos.zPos + frames->boundingBox.Z2 >= (z - size))
					return true;
			}
		}

		return false;
	}

	void ControlTeethSpikes(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item) && item->itemFlags[2] == 0)
		{
			bool hit = false;

			if (item->itemFlags[0] == 1024)	// Just started.
				SoundEffect(SFX_TR4_TEETH_SPIKES, (PHD_3DPOS*)&item->pos, 0);

			item->status = ITEM_ACTIVE;

			hit = TestBoundsCollideTeethSpikes(item);

			if (LaraItem->hitPoints > 0 && hit)
			{
				BOUNDING_BOX* bounds = (BOUNDING_BOX*)GetBestFrame(item);
				BOUNDING_BOX* laraBounds = (BOUNDING_BOX*)GetBestFrame(LaraItem);

				int bloodCount = 0;

				if ((item->itemFlags[0] > 1024 || LaraItem->gravityStatus) 
					&& (item->triggerFlags & 7) > 2 
					&& (item->triggerFlags & 7) < 6)
				{
					if (LaraItem->fallspeed > 6 || item->itemFlags[0] > 1024)
					{
						LaraItem->hitPoints = -1;
						bloodCount = 20;
					}
				}
				else if (LaraItem->speed >= 30)
				{
					LaraItem->hitPoints -= 8;
					bloodCount = (GetRandomControl() & 3) + 2;
				}
				else
					bloodCount = 0;

				int yTop = laraBounds->Y1 + LaraItem->pos.yPos;
				int yBottom = laraBounds->Y2 + LaraItem->pos.yPos;
				int y1, y2;

				if ((item->triggerFlags & 15) == 8 ||
					(item->triggerFlags & 15) == 0)
				{
					y1 = -bounds->Y2;
					y2 = -bounds->Y1;
				}
				else
				{
					y1 = bounds->Y1;
					y2 = bounds->Y2;
				}

				if (yTop < y1 + item->pos.yPos)
					yTop = y1 + item->pos.yPos;
				if (yBottom > y2 + item->pos.yPos)
					yBottom = y2 + item->pos.yPos;	
				int dy = (abs(yTop - yBottom)) + 1;

				if ((item->triggerFlags & 7) == 2 
					|| (item->triggerFlags & 7) == 6)
					bloodCount >>= 1;

				for (int i = 0; i < bloodCount; i++)
				{
					int dx = LaraItem->pos.xPos + (GetRandomControl() & 127) - 64;
					int dz = LaraItem->pos.zPos + (GetRandomControl() & 127) - 64;
					TriggerBlood(dx, yBottom - (GetRandomControl() % dy), dz, GetRandomControl() << 1, 1);
				}

				if (LaraItem->hitPoints <= 0)
				{
					short roomNumber = LaraItem->roomNumber;
					FLOOR_INFO* floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNumber);
					int height = GetFloorHeight(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
					height -= LaraItem->pos.yPos;
					if (item->pos.yPos >= LaraItem->pos.yPos && height < 50)
					{
						LaraItem->animNumber = LA_SPIKE_DEATH;
						LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
						LaraItem->currentAnimState = LS_DEATH;
						LaraItem->goalAnimState = LS_DEATH;
						LaraItem->gravityStatus = false;
					}
				}
			}

			item->itemFlags[0] += 128;
			item->itemFlags[1] += item->itemFlags[0];
			if (item->itemFlags[1] >= 5120)
			{
				item->itemFlags[1] = 5120;
				if (item->itemFlags[0] <= 1024)
				{
					item->itemFlags[0] = 0;
					if ((item->triggerFlags & 16) == 0 && LaraItem->hitPoints > 0)
						item->itemFlags[2] = 64;
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] >> 1;
				}
			}

		}
		else if (TriggerActive(item))
		{
			item->itemFlags[0] += (item->itemFlags[0] >> 3) + 32;
			item->itemFlags[1] -= item->itemFlags[0];
			if (item->itemFlags[1] < 0)
			{
				item->itemFlags[0] = 1024;
				item->itemFlags[1] = 0;
				item->status = ITEM_INVISIBLE;
			}

			if ((item->triggerFlags & 32) == 0)
			{
				if (item->itemFlags[2])
					item->itemFlags[2]--;
			}
			else
				item->itemFlags[2] = 1;
		}
		else if (!item->timer)
		{
			item->itemFlags[0] += (item->itemFlags[0] >> 3) + 32;
			if (item->itemFlags[1] > 0)
			{
				item->itemFlags[1] -= item->itemFlags[0];
				if (item->itemFlags[1] < 0)
					item->itemFlags[1] = 0;
			}
		}

		// Update bone mutators
		if (item->itemFlags[1])
		{
			for (int i = 0; i < item->mutator.size(); i++)
				item->mutator[i].Scale = Vector3(1.0f, item->itemFlags[1] / 4096.0f, 1.0f);
		}
	}
}