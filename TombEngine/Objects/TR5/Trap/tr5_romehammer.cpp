#include "framework.h"
#include "Objects/TR5/Trap/tr5_romehammer.h"

#include "Game/items.h"
#include "Specific/level.h"

// NOTES:
// item.ItemFlags[0] = Meshes bits that hurts Lara. (2 = 00000010 )
// item.ItemFlags[3] = Damage.
// item.ItemFlags[4] = if 0, it pushes Lara away when collides.

namespace TEN::Traps::TR5
{
	constexpr auto ROME_HAMMER_HARM_DAMAGE = 250;
	constexpr auto ROME_HAMMER_JOINT = MESH_BITS(1);

	const auto RomeHammerAttackJoints = std::vector<unsigned int>{ 2 };

	void InitializeRomeHammer(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[0] = ROME_HAMMER_JOINT;
		item.ItemFlags[3] = 0;
	}

	void ControlRomeHammer(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			item.ItemFlags[3] = ROME_HAMMER_HARM_DAMAGE;
			AnimateItem(&item);
		}
		else
		{
			item.ItemFlags[3] = 0;
		}
	}
}
