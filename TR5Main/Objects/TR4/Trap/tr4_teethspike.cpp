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
		int angle;

		auto* item = &g_Level.Items[itemNumber];
		item->Status = ITEM_INVISIBLE;

		// Set mutators to 0 by default
		for (int i = 0; i < item->Mutator.size(); i++)
			item->Mutator[i].Scale.y = 0.0f;

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

		if (item->TriggerFlags & 8)
		{
			angle = item->TriggerFlags & 7;
			item->Position.xRot = rotations[angle];
			item->Position.yRot = ANGLE(90.0f);
			item->Position.zPos -= SPxzoffs[angle];
		}
		else
		{
			angle = item->TriggerFlags & 7;
			item->Position.xPos += SPxzoffs[angle];
			item->Position.zRot = rotations[angle];
		}

		item->ItemFlags[0] = 1024;
		item->ItemFlags[2] = 0;
		item->Position.yPos += SPyoffs[angle];
	}

	bool TestBoundsCollideTeethSpikes(ITEM_INFO* item)
	{
		int x;
		int z;

		short angle = item->TriggerFlags & 7;

		if (item->TriggerFlags & 8)
		{
			x = (item->Position.xPos & (~1023)) | 512;
			z = ((item->Position.zPos + SPxzoffs[angle]) & (~1023)) | 512;
		}
		else
		{
			x = ((item->Position.xPos - SPxzoffs[angle]) & (~1023)) | 512;
			z = (item->Position.zPos & (~1023)) | 512;
		}

		int size = (item->TriggerFlags & 1 ? 300 : 480);
		int y = item->Position.yPos + SPDETyoffs[angle];

		ANIM_FRAME* frames = GetBestFrame(LaraItem);

		if (LaraItem->Position.yPos + frames->boundingBox.Y1 <= y
			&& LaraItem->Position.yPos + frames->boundingBox.Y2 >= y - 900)
		{
			if (LaraItem->Position.xPos + frames->boundingBox.X1 <= (x + size)
				&& LaraItem->Position.xPos + frames->boundingBox.X2 >= (x - size))
			{
				if (LaraItem->Position.zPos + frames->boundingBox.Z1 <= (z + size)
					&& LaraItem->Position.zPos + frames->boundingBox.Z2 >= (z - size))
					return true;
			}
		}

		return false;
	}

	void ControlTeethSpikes(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item) && item->ItemFlags[2] == 0)
		{
			bool hit = false;

			if (item->ItemFlags[0] == 1024)	// Just started.
				SoundEffect(SFX_TR4_TEETH_SPIKES, (PHD_3DPOS*)&item->Position, 0);

			item->Status = ITEM_ACTIVE;

			hit = TestBoundsCollideTeethSpikes(item);

			if (LaraItem->HitPoints > 0 && hit)
			{
				BOUNDING_BOX* bounds = (BOUNDING_BOX*)GetBestFrame(item);
				BOUNDING_BOX* laraBounds = (BOUNDING_BOX*)GetBestFrame(LaraItem);

				int bloodCount = 0;

				if ((item->ItemFlags[0] > 1024 || LaraItem->Airborne) 
					&& (item->TriggerFlags & 7) > 2 
					&& (item->TriggerFlags & 7) < 6)
				{
					if (LaraItem->VerticalVelocity > 6 || item->ItemFlags[0] > 1024)
					{
						LaraItem->HitPoints = -1;
						bloodCount = 20;
					}
				}
				else if (LaraItem->Velocity >= 30)
				{
					LaraItem->HitPoints -= 8;
					bloodCount = (GetRandomControl() & 3) + 2;
				}
				else
					bloodCount = 0;

				int yTop = laraBounds->Y1 + LaraItem->Position.yPos;
				int yBottom = laraBounds->Y2 + LaraItem->Position.yPos;
				int y1, y2;

				if ((item->TriggerFlags & 15) == 8 ||
					(item->TriggerFlags & 15) == 0)
				{
					y1 = -bounds->Y2;
					y2 = -bounds->Y1;
				}
				else
				{
					y1 = bounds->Y1;
					y2 = bounds->Y2;
				}

				if (yTop < y1 + item->Position.yPos)
					yTop = y1 + item->Position.yPos;
				if (yBottom > y2 + item->Position.yPos)
					yBottom = y2 + item->Position.yPos;	
				int dy = (abs(yTop - yBottom)) + 1;

				if ((item->TriggerFlags & 7) == 2 
					|| (item->TriggerFlags & 7) == 6)
					bloodCount >>= 1;

				for (int i = 0; i < bloodCount; i++)
				{
					int dx = LaraItem->Position.xPos + (GetRandomControl() & 127) - 64;
					int dz = LaraItem->Position.zPos + (GetRandomControl() & 127) - 64;
					TriggerBlood(dx, yBottom - (GetRandomControl() % dy), dz, GetRandomControl() << 1, 1);
				}

				if (LaraItem->HitPoints <= 0)
				{
					short roomNumber = LaraItem->RoomNumber;
					FLOOR_INFO* floor = GetFloor(LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos, &roomNumber);
					int height = GetFloorHeight(floor, LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos);
					height -= LaraItem->Position.yPos;
					if (item->Position.yPos >= LaraItem->Position.yPos && height < 50)
					{
						LaraItem->AnimNumber = LA_SPIKE_DEATH;
						LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
						LaraItem->ActiveState = LS_DEATH;
						LaraItem->TargetState = LS_DEATH;
						LaraItem->Airborne = false;
					}
				}
			}

			item->ItemFlags[0] += 128;
			item->ItemFlags[1] += item->ItemFlags[0];
			if (item->ItemFlags[1] >= 5120)
			{
				item->ItemFlags[1] = 5120;
				if (item->ItemFlags[0] <= 1024)
				{
					item->ItemFlags[0] = 0;
					if ((item->TriggerFlags & 16) == 0 && LaraItem->HitPoints > 0)
						item->ItemFlags[2] = 64;
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] >> 1;
				}
			}

		}
		else if (TriggerActive(item))
		{
			item->ItemFlags[0] += (item->ItemFlags[0] >> 3) + 32;
			item->ItemFlags[1] -= item->ItemFlags[0];
			if (item->ItemFlags[1] < 0)
			{
				item->ItemFlags[0] = 1024;
				item->ItemFlags[1] = 0;
				item->Status = ITEM_INVISIBLE;
			}

			if ((item->TriggerFlags & 32) == 0)
			{
				if (item->ItemFlags[2])
					item->ItemFlags[2]--;
			}
			else
				item->ItemFlags[2] = 1;
		}
		else if (!item->Timer)
		{
			item->ItemFlags[0] += (item->ItemFlags[0] >> 3) + 32;
			if (item->ItemFlags[1] > 0)
			{
				item->ItemFlags[1] -= item->ItemFlags[0];
				if (item->ItemFlags[1] < 0)
					item->ItemFlags[1] = 0;
			}
		}

		// Update bone mutators
		if (item->ItemFlags[1])
		{
			for (int i = 0; i < item->Mutator.size(); i++)
				item->Mutator[i].Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
		}
	}
}
