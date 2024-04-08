#include "framework.h"
#include "Game/collision/Sphere.h"

#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Collision::Sphere
{
	std::vector<BoundingSphere> GetSpheres(const ItemInfo& item, int flags)
	{
		return g_Renderer.GetSpheres(item.Index, flags, Matrix::Identity);
	}

	bool SetSphereTouchBits(ItemInfo& creatureItem, ItemInfo& playerItem)
	{
		auto creatureSpheres = GetSpheres(creatureItem, (int)SphereSpaceFlags::World | (int)SphereSpaceFlags::BoneOrigin);
		auto playerSpheres = GetSpheres(playerItem, (int)SphereSpaceFlags::World | (int)SphereSpaceFlags::BoneOrigin);

		playerItem.TouchBits.ClearAll();

		if (creatureSpheres.empty())
		{
			creatureItem.TouchBits.ClearAll();
			return false;
		}

		// Run through creature spheres.
		bool isCollided = false;
		for (int i = 0; i < creatureSpheres.size(); i++)
		{
			// Get creature sphere.
			const auto& creatureSphere = creatureSpheres[i];
			if (creatureSphere.Radius <= 0.0f)
				continue;

			// Run through player spheres.
			for (int j = 0; j < playerSpheres.size(); j++)
			{
				// Get player sphere.
				const auto& playerSphere = playerSpheres[j];
				if (playerSphere.Radius <= 0.0f)
					continue;

				// Test sphere collision.
				if (!creatureSphere.Intersects(playerSphere))
					continue;

				// Set touch bits.
				creatureItem.TouchBits.Set(i);
				playerItem.TouchBits.Set(j);

				isCollided = true;
				break;
			}
		}

		return isCollided;
	}
}
