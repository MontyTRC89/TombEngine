#include "framework.h"
#include "Objects/Effects/lens_flare.h"
#include "Specific/level.h"

namespace TEN::Entities::Effects
{
	std::vector<LensFlare> LensFlares;

	void LensFlareControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			LensFlare lensFlare;
			
			lensFlare.Position = item->Pose.Position.ToVector3();
			lensFlare.RoomNumber = item->RoomNumber;

			LensFlares.push_back(lensFlare);
		}
	}

	void ClearLensFlares()
	{
		LensFlares.clear();
	}
}