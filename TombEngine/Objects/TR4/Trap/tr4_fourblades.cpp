#include "framework.h"
#include "tr4_fourblades.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

// NOTES:
// item.ItemFlags[0] = Meshes that hurts Lara
// item.ItemFlags[3] = Damage.
// item.ItemFlags[4] = if 1, it won't push Lara away when collides.

namespace TEN::Entities::TR4
{
	constexpr auto FOUR_BLADES_HARM_DAMAGE_EMERGE = 20;
	constexpr auto FOUR_BLADES_HARM_DAMAGE_CUT = 200;
	constexpr auto FOUR_BLADES_JOINT = MESH_BITS(1) | MESH_BITS(2) | MESH_BITS(3) | MESH_BITS(4);

	void InitializeFourBlades(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[4] = 1;
	}

	void FourBladesControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
		{
			item.Animation.FrameNumber = GetAnimData(item).frameBase;
			item.ItemFlags[0] = 0;
		}
		else
		{
			int frameNumber = item.Animation.FrameNumber - GetAnimData(item).frameBase;
			if (frameNumber <= 5 ||
				frameNumber >= 58 ||
				frameNumber >= 8 && frameNumber <= 54)
			{
				item.ItemFlags[0] = 0;
				item.ItemFlags[3] = 0;
			}
			else
			{
				item.ItemFlags[0] = FOUR_BLADES_JOINT;

				if (frameNumber >= 6 && frameNumber <= 7)
				{
					item.ItemFlags[3] = FOUR_BLADES_HARM_DAMAGE_EMERGE;	
				}
				else if (frameNumber >= 55 && frameNumber <= 57)
				{
					item.ItemFlags[3] = FOUR_BLADES_HARM_DAMAGE_CUT;
				}
			}

			AnimateItem(&item);
		}
	}
}
