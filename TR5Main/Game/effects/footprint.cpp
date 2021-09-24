#include "framework.h"
#include "control/control.h"
#include "lara.h"
#include "animation.h"
#include "effects\groundfx.h"
#include "effects\footprint.h"
#include "level.h"
#include "items.h"

namespace TEN {
namespace Effects {
namespace Footprints {

	std::deque<FOOTPRINT_STRUCT> footprints = std::deque<FOOTPRINT_STRUCT>();

	bool CheckFootOnFloor(ITEM_INFO const & item, int mesh, PHD_3DPOS& outFootprintPosition) 
	{
		int x = item.pos.xPos;
		int y = item.pos.yPos;
		int z = item.pos.zPos;
		short roomNumber = item.roomNumber;

		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		if (!(floor->Material == GroundMaterial::Sand   ||
			  floor->Material == GroundMaterial::Snow   ||
			  floor->Material == GroundMaterial::Gravel ||
			  floor->Material == GroundMaterial::Mud)) 
		{
			return false;
		}

		int height = GetFloorHeight(floor, x, y, z);
		PHD_VECTOR pos;

		pos.x = pos.z = 0;
		pos.y = FOOT_HEIGHT_OFFSET;

		GetLaraJointPosition(&pos, mesh);

		outFootprintPosition.xPos = pos.x;
		outFootprintPosition.zPos = pos.z;
		outFootprintPosition.yPos = height - 1;
		outFootprintPosition.yRot = item.pos.yRot;

		return abs(pos.y - height) < 32;
	}

	void UpdateFootprints()
	{
		if (footprints.size() == 0)
			return;

		int numInvalidFootprints = 0;

		for (auto i = footprints.begin(); i != footprints.end(); i++) 
		{
			FOOTPRINT_STRUCT& footprint = *i;
			footprint.life--;

			if (footprint.life <= 0) 
			{
				numInvalidFootprints++;
				continue;
			}

			if (footprint.life > footprint.lifeStartFading) 
			{
				footprint.opacity = footprint.startOpacity;
			}
			else 
			{
				float opacity = lerp(0, footprint.startOpacity, fmax(0, fmin(1, footprint.life / (float)footprint.lifeStartFading)));
				footprint.opacity = opacity;
			}
		}

		for (int i = 0; i < numInvalidFootprints; i++) 
		{
			footprints.pop_back();
		}
	}
}
}
}