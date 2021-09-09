#include "framework.h"
#include "tr4_cog.h"
#include "level.h"
#include "control.h"
#include "sphere.h"
#include "Sound\sound.h"

void CogControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TriggerActive(item))
	{
		item->status = ITEM_ACTIVE;
		AnimateItem(item);

		if (item->triggerFlags == 666)
		{
			PHD_VECTOR pos;
			GetJointAbsPosition(item, &pos, 0);
			SoundEffect(65, (PHD_3DPOS*)&pos, 0);

			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				item->flags &= 0xC1;
		}
	}
	else if (item->triggerFlags == 2)
	{
		item->status |= ITEM_INVISIBLE;
	}
}