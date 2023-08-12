#include "framework.h"
#include "Objects/TR1/Trap/SwingingTrap.h"

#include "Game/Setup.h"

namespace TEN::Entities::Traps::TR1
{
	constexpr auto SWINGING_BLADE_DAMAGE = 100;

	enum SlammingDoorsState
	{
		SWINGING_BLADE_DISABLED = 0,
		SWINGING_BLADE_ENABLED = 2
	};

	enum SlammingDoorsAnim
	{
		SWINGING_BLADE_ANIM_DISABLED = 0,
		SWINGING_BLADE_ANIM_DEACTIVATION = 1,
		SWINGING_BLADE_ANIM_ENABLED = 2
	};

	void InitializeSwingingBlade(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		SetAnimation(&item, SWINGING_BLADE_ANIM_DISABLED);

		// Used by GenericSphereBoxCollision. Bits correspond to joint damage index.
		// 6 = 000000000 000000110, so damage joints are bit 1 (pole) and bit 2 (blade)
		item.ItemFlags[0] = 0;

		// Used by GenericSphereBoxCollision for trap damage value.
		item.ItemFlags[3] = SWINGING_BLADE_DAMAGE;
	}

	void ControlSwingingBlade(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			if (item.Animation.TargetState != SWINGING_BLADE_ANIM_ENABLED)
			{
				item.Animation.TargetState = SWINGING_BLADE_ANIM_ENABLED;
				item.ItemFlags[0] = 6;
			}
		}
		else
		{
			if (item.Animation.TargetState != SWINGING_BLADE_ANIM_DISABLED)
			{
				item.Animation.TargetState = SWINGING_BLADE_ANIM_DISABLED;
				item.ItemFlags[0] = 0;
			}
		}

		AnimateItem(&item);
	}
}