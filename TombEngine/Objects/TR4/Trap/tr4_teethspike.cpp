#include "framework.h"
#include "tr4_teethspike.h"
#include "Game/collision/collide_room.h"
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
		for (int i = 0; i < item->Animation.Mutator.size(); i++)
			item->Animation.Mutator[i].Scale.y = 0.0f;

		short rotations[8] =
		{
			ANGLE(180.0f),
			ANGLE(225.0f),
			ANGLE(270.0f),
			ANGLE(315.0f),
			ANGLE(0.0f),
			ANGLE(45.0f),
			ANGLE(90.0f),
			ANGLE(135.0f)
		};

		if (item->TriggerFlags & 8)
		{
			angle = item->TriggerFlags & 7;
			item->Pose.Orientation.x = rotations[angle];
			item->Pose.Orientation.y = ANGLE(90.0f);
			item->Pose.Position.z -= SPxzoffs[angle];
		}
		else
		{
			angle = item->TriggerFlags & 7;
			item->Pose.Position.x += SPxzoffs[angle];
			item->Pose.Orientation.z = rotations[angle];
		}

		item->ItemFlags[0] = 1024;
		item->ItemFlags[2] = 0;
		item->Pose.Position.y += SPyoffs[angle];
	}

	bool TestBoundsCollideTeethSpikes(ItemInfo* item)
	{
		int x;
		int z;

		short angle = item->TriggerFlags & 7;

		if (item->TriggerFlags & 8)
		{
			x = (item->Pose.Position.x & (~1023)) | 512;
			z = ((item->Pose.Position.z + SPxzoffs[angle]) & (~1023)) | 512;
		}
		else
		{
			x = ((item->Pose.Position.x - SPxzoffs[angle]) & (~1023)) | 512;
			z = (item->Pose.Position.z & (~1023)) | 512;
		}

		int size = (item->TriggerFlags & 1 ? 300 : 480);
		int y = item->Pose.Position.y + SPDETyoffs[angle];

		auto* frames = GetBestFrame(LaraItem);

		if (LaraItem->Pose.Position.y + frames->boundingBox.Y1 <= y &&
			LaraItem->Pose.Position.y + frames->boundingBox.Y2 >= y - 900)
		{
			if (LaraItem->Pose.Position.x + frames->boundingBox.X1 <= (x + size) &&
				LaraItem->Pose.Position.x + frames->boundingBox.X2 >= (x - size))
			{
				if (LaraItem->Pose.Position.z + frames->boundingBox.Z1 <= (z + size) &&
					LaraItem->Pose.Position.z + frames->boundingBox.Z2 >= (z - size))
				{
					return true;
				}
			}
		}

		return false;
	}

	void ControlTeethSpikes(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item) && item->ItemFlags[2] == 0)
		{
			// Just emerging.
			if (item->ItemFlags[0] == 1024)
				SoundEffect(SFX_TR4_TEETH_SPIKES, &item->Pose);

			item->Status = ITEM_ACTIVE;

			if (LaraItem->Animation.ActiveState != LS_DEATH &&
				TestBoundsCollideTeethSpikes(item))
			{
				auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);
				auto* laraBounds = (BOUNDING_BOX*)GetBestFrame(LaraItem);

				int bloodCount = 0;

				if ((item->ItemFlags[0] > 1024 ||
					LaraItem->Animation.IsAirborne) &&
					(item->TriggerFlags & 7) > 2 &&
					(item->TriggerFlags & 7) < 6)
				{
					if (LaraItem->Animation.VerticalVelocity > 6 ||
						item->ItemFlags[0] > 1024)
					{
						LaraItem->HitPoints = -1;
						bloodCount = 20;
					}
				}
				else if (LaraItem->Animation.Velocity >= 30)
				{
					DoDamage(LaraItem, 8);
					bloodCount = (GetRandomControl() & 3) + 2;
				}
				else
					bloodCount = 0;

				int yTop = laraBounds->Y1 + LaraItem->Pose.Position.y;
				int yBottom = laraBounds->Y2 + LaraItem->Pose.Position.y;
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

				if (yTop < y1 + item->Pose.Position.y)
					yTop = y1 + item->Pose.Position.y;
				if (yBottom > y2 + item->Pose.Position.y)
					yBottom = y2 + item->Pose.Position.y;	
				int dy = (abs(yTop - yBottom)) + 1;

				if ((item->TriggerFlags & 7) == 2 ||
					(item->TriggerFlags & 7) == 6)
				{
					bloodCount >>= 1;
				}

				for (int i = 0; i < bloodCount; i++)
				{
					int dx = LaraItem->Pose.Position.x + (GetRandomControl() & 127) - 64;
					int dz = LaraItem->Pose.Position.z + (GetRandomControl() & 127) - 64;
					TriggerBlood(dx, yBottom - (GetRandomControl() % dy), dz, GetRandomControl() << 1, 1);
				}

				if (LaraItem->HitPoints <= 0 && Lara.Vehicle == NO_ITEM)
				{
					int heightFromFloor = GetCollision(LaraItem).Position.Floor - LaraItem->Pose.Position.y;
					if (item->Pose.Position.y >= LaraItem->Pose.Position.y &&
						heightFromFloor < CLICK(1))
					{
						SetAnimation(LaraItem, LA_SPIKE_DEATH);
						LaraItem->Animation.IsAirborne = false;
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
					item->ItemFlags[0] = -item->ItemFlags[0] >> 1;
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

		// Update bone mutators.
		if (item->ItemFlags[1])
		{
			for (int i = 0; i < item->Animation.Mutator.size(); i++)
				item->Animation.Mutator[i].Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
		}
	}
}
