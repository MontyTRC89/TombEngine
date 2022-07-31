#include "framework.h"
#include "Objects/TR2/Trap/tr2_killerstatue.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseKillerStatue(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = 1;
}

void KillerStatueControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->Animation.ActiveState == 1)
		item->Animation.TargetState = 2;
	else
		item->Animation.TargetState = 1;

	if (item->TouchBits & 0x80 && item->Animation.ActiveState == 2)
	{
		DoDamage(LaraItem, 20);

		int x = LaraItem->Pose.Position.x + (GetRandomControl() - SECTOR(16)) / CLICK(1);
		int z = LaraItem->Pose.Position.z + (GetRandomControl() - SECTOR(16)) / CLICK(1);
		int y = LaraItem->Pose.Position.y - GetRandomControl() / 44;
		int d = (GetRandomControl() - SECTOR(16)) / 8 + LaraItem->Pose.Orientation.y;

		DoBloodSplat(x, y, z, LaraItem->Animation.Velocity, d, LaraItem->RoomNumber);
	}

	AnimateItem(item);
}
