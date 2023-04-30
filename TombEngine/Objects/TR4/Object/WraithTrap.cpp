#include "framework.h"
#include "WraithTrap.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/Electricity.h"
#include "Game/effects/spark.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Spark;
using namespace TEN::Input;

namespace TEN::Entities::TR4
{
	void InitializeWraithTrap(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex;
		item.Animation.FrameNumber = g_Level.Anims[item.Animation.AnimNumber].frameBase;
		item.ItemFlags[6] = 0;
	}

	void WraithTrapControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			if (item.ItemFlags[6] && item.TriggerFlags > 0)
			{
				auto pos = GetJointPosition(&item, 0, Vector3i::Zero);

				SoundEffect(SFX_TR4_LIGHT_BEAM_LOOP, &Pose(pos));

				auto color = Vector3(255.0f);
				TriggerAttackSpark(pos.ToVector3(), color);
			}
		}
	}
}
