#include "framework.h"
#include "tr5_teethspike.h"
#include "control/control.h"
#include "animation.h"
#include "lara.h"
#include "level.h"
#include "Sound/sound.h"
#include "effects/tomb4fx.h"
#include "Specific/trmath.h"
#include "setup.h"

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

	short rotations[8] =
	{
		-ANGLE(180.0f),
		-ANGLE(135.0f),
		-ANGLE(90.0f),
		-ANGLE(45.0f),
		ANGLE(0.0f),
		ANGLE(45.0f), 
		ANGLE(90.0f),
		ANGLE(135.0f)
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
		x = item->pos.xPos & (~1023) | 512;
		z = (item->pos.zPos + SPxzoffs[angle]) & (~1023) | 512;
	}
	else
	{
		x = (item->pos.xPos - SPxzoffs[angle]) & (~1023) | 512;
		z = item->pos.zPos & (~1023) | 512;
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

	if (!TriggerActive(item) || item->itemFlags[2])
	{
		if (TriggerActive(item))
		{
			item->itemFlags[1] -= item->itemFlags[0];
			item->itemFlags[0] += (item->itemFlags[0] / 8) + 32;

			if (item->itemFlags[1] < 0)
			{
				item->itemFlags[0] = 1024;
				item->itemFlags[1] = 0;
				item->status = ITEM_INVISIBLE;
			}

			if (item->triggerFlags & 32)
			{
				item->itemFlags[2] = 1;
			}
			else
			{
				if (item->itemFlags[2])
				{
					item->itemFlags[2]--;
				}
			}
		}
		else if (!item->timer)
		{
			item->itemFlags[0] += (item->itemFlags[0] / 8) + 32;

			if (item->itemFlags[1] > 0)
			{
				item->itemFlags[1] -= item->itemFlags[0];
				if (item->itemFlags[1] < 0)
					item->itemFlags[1] = 0;
			}
		}
	}
	else
	{
		if (item->itemFlags[0] == 1024)
			SoundEffect(SFX_TR4_TEETH_SPIKES, &item->pos, 0);

		item->status = ITEM_ACTIVE;

		if (LaraItem->hitPoints > 0 && TestBoundsCollideTeethSpikes(item))
		{
			ANIM_FRAME* itemFrames = GetBestFrame(item);
			ANIM_FRAME* laraFrames = GetBestFrame(LaraItem);

			short angle = item->triggerFlags & 7;
			int numBloods = 0;

			if ((item->itemFlags[0] > 1024
				|| LaraItem->gravityStatus)
				&& angle > 2 
				&& angle < 6)
			{
				if (LaraItem->fallspeed > GRAVITY
					|| item->itemFlags[0] > 1024)
				{
					LaraItem->hitPoints = -1;
					numBloods = 20;
				}
			}
			else if (LaraItem->speed < 30)
			{
				numBloods = 0;
			}
			else
			{
				LaraItem->hitPoints -= 8;
				numBloods = (GetRandomControl() & 3) + 2;
			}

			int laraY1 = LaraItem->pos.yPos + laraFrames->boundingBox.Y1;
			int laraY2 = LaraItem->pos.yPos + laraFrames->boundingBox.Y2;

			short triggerFlags = item->triggerFlags & 15;

			int itemY1;
			int itemY2;

			if (triggerFlags != 8 && triggerFlags)
			{
				itemY1 = itemFrames->boundingBox.Y1;
				itemY2 = itemFrames->boundingBox.Y2;
			}
			else
			{
				itemY1 = -itemFrames->boundingBox.Y2;
				itemY2 = -itemFrames->boundingBox.Y1;
			}
			if (laraY1 < item->pos.yPos + itemY1)
				laraY1 = itemY1 + item->pos.yPos;
			if (laraY2 > item->pos.yPos + itemY2)
				laraY2 = itemY2 + item->pos.yPos;

			int dy = abs(laraY1 - laraY2) + 1;
			
			if (angle == 2 || angle == 6)
				numBloods /= 2;

			for (int i = 0; i < numBloods; i++)
			{
				TriggerBlood(
					(GetRandomControl() & 127) + LaraItem->pos.xPos - 64,
					laraY2 - GetRandomControl() % dy,
					(GetRandomControl() & 127) + LaraItem->pos.zPos - 64,
					2 * GetRandomControl(),
					1);
			}

			if (LaraItem->hitPoints <= 0)
			{
				short roomNumber = LaraItem->roomNumber;
				FLOOR_INFO* floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNumber);
				int height = GetFloorHeight(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);

				if (item->pos.yPos >= LaraItem->pos.yPos && height - LaraItem->pos.yPos < 50)
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
				if (!(item->triggerFlags & 16))
				{
					if (LaraItem->hitPoints > 0)
						item->itemFlags[2] = 64;
				}
			}
			else
			{
				item->itemFlags[0] = -item->itemFlags[0] / 2;
			}
		}
	}
}