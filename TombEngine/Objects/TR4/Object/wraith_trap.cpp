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

	item->ItemFlags[6] = 0;
}

void WraithTrapControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->ItemFlags[6] && item->TriggerFlags > 0)
		{
			GameVector pos1 = GetJointPosition(item, 0, Vector3i::Zero);

			SoundEffect(SFX_TR4_LIGHT_BEAM_LOOP, &Pose(pos1.ToVector3i()));

			auto sparkColor = Vector3(255, 255, 255);
			TriggerAttackSpark(pos1.ToVector3(), sparkColor);
		}

		AnimateItem(item);
	}
}
