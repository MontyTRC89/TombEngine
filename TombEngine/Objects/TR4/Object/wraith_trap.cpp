#include "framework.h"
#include "wraith_trap.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Specific/Input/Input.h"
#include "Game/animation.h"
#include "Game/effects/Electricity.h"
#include "Game/effects/debris.h"

using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Spark;
using namespace TEN::Input;

void InitialiseWraithTrap(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

	//AddActiveItem(itemNumber);
	//item->Status = ITEM_ACTIVE;

	if (item->TriggerFlags == 2)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			auto* currentItem = &g_Level.Items[i];

			if (currentItem->ObjectNumber == ID_OBELISK)
				item->ItemFlags[0]++;

			if (currentItem->ObjectNumber == ID_ANIMATING3)
				item->ItemFlags[2] = i;
		}
	}

}

void WraithTrapControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short someNumber = 0;
	auto pos = Vector3::Zero;
	auto pos2 = Vector3::Zero;

	if (TriggerActive(item))
	{

		if (item->ItemFlags[6] = 1)
		{

			GameVector pos1 = GetJointPosition(item, 0, Vector3i::Zero);
			auto sparkColor = Vector3(255, 255, 255);
			TriggerAttackSpark(pos1.ToVector3(), sparkColor);


		}

		AnimateItem(item);
	}
}
