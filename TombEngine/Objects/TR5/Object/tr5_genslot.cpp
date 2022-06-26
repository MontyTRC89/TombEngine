#include "framework.h"
#include "tr5_genslot.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"

void GenSlot1Control(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && !item->TriggerFlags)
	{
		int df = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

		if (df == 10 || df == 11)
		{
			GetLaraDeadlyBounds();

			DeadlyBounds[0] -= 350;
			DeadlyBounds[1] += 350;
			DeadlyBounds[4] -= 350;
			DeadlyBounds[5] += 350;

			bool found = false;
			for (int i = 0; i < 6; i++)
			{
				Vector3Int pos = { 0, -350, 0 };
				GetJointAbsPosition(item, &pos, i + 1);

				if (pos.x > DeadlyBounds[0] &&
					pos.x < DeadlyBounds[1] &&
					pos.y > DeadlyBounds[2] &&
					pos.y < DeadlyBounds[3] &&
					pos.z > DeadlyBounds[4] &&
					pos.z < DeadlyBounds[5])
				{
					found = true;
				}
			}

			if (found)
			{
				for (int i = 0; i < 8; i++)
				{
					Vector3Int pos = { 0, 0, 0 };
					GetLaraJointPosition(&pos, i + 7);

					int x = pos.x + (GetRandomControl() & 0xFF) - 128;
					int y = pos.y + (GetRandomControl() & 0xFF) - 128;
					int z = pos.z + (GetRandomControl() & 0xFF) - 128;

					DoBloodSplat(x, y, z, 1, -1, LaraItem->RoomNumber);
				}

				DoDamage(LaraItem, INT_MAX);
			}
		}

		AnimateItem(item);
	}
}

void InitialiseGenSlot3(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (CurrentLevel != 7)
		item->MeshBits = item->TriggerFlags;
}

void InitialiseGenSlot4(short itemNumber)
{
	/*ItemInfo* item = &g_Level.Items[itemNumber];

	HIWORD(v1) = HIWORD(items);
	item = &g_Level.Items[itemNumber];
	LOWORD(v1) = item->pos.Orientation.y;
	v3 = item->pos.Position.x;
	v4 = 2 * ((v1 >> 3) & 0x1FFE);
	v5 = 5 * *(__int16*)((char*)rcossin_tbl + v4);
	v6 = item->pos.Position.z;
	v7 = v6 + (10240 * *(__int16*)((char*)& rcossin_tbl[1] + v4) >> 14);
	item->item_flags[2] = 1;
	BYTE1(v4) = v6 >> 9;
	LOBYTE(v4) = v3 >> 9;
	item->item_flags[0] = v4;
	LOBYTE(v6) = (item->pos.Position.x + (v5 << 11 >> 14)) >> 9;
	BYTE1(v6) = v7 >> 9;

	item->itemFlags[1] = item->pos.Position.x + 2560 * phd_sin(item->pos.Orientation.y) >> W2V_SHIFT;
	item->itemFlags[3] = 0;
	item->triggerFlags = 0;*/
}
