#include "framework.h"
#include "Objects/TR1/Trap/SwingingBlade.h"

#include "Game/Setup.h"

namespace TEN::Entities::Traps::TR1
{
	constexpr auto SWINGING_BLADE_HARM_DAMAGE = 100;

	const std::vector<unsigned int> SwingingBladeHarmJoints = { 1, 2 };

	enum SwingingBladeState
	{
		SWINGING_BLADE_STATE_DISABLED = 0,
		SWINGING_BLADE_STATE_ENABLED = 2
	};

	enum SwingingBladeAnim
	{
		SWINGING_BLADE_ANIM_DISABLED = 0,
		SWINGING_BLADE_ANIM_DEACTIVATING = 1,
		SWINGING_BLADE_ANIM_ENABLED = 2
	};

	void InitializeSwingingBlade(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		SetAnimation(&item, SWINGING_BLADE_ANIM_DISABLED);
		
		// Used for damage in GenericSphereBoxCollision().
		item.ItemFlags[3] = SWINGING_BLADE_HARM_DAMAGE;
	}

	void ControlSwingingBlade(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			if (item.Animation.TargetState != SWINGING_BLADE_STATE_ENABLED)
			{
				item.Animation.TargetState = SWINGING_BLADE_STATE_ENABLED;

				// Set harm joints.
				auto bitField = BitField::Default;
				bitField.Set(SwingingBladeHarmJoints);
				item.ItemFlags[0] = bitField.ToPackedBits();
			}
		}
		else
		{
			if (item.Animation.TargetState != SWINGING_BLADE_STATE_DISABLED)
			{
				item.Animation.TargetState = SWINGING_BLADE_STATE_DISABLED;

				// Unset harm joints.
				item.ItemFlags[0] = 0;
			}
		}

		AnimateItem(&item);
	}
}
