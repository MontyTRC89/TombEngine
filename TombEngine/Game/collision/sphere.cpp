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
	std::vector<BoundingSphere> GetSpheres(const ItemInfo* itemPtr, int spaceFlags)
	{
		if (itemPtr == nullptr)
			return {};

		return g_Renderer.GetSpheres(itemPtr->Index, spaceFlags, Matrix::Identity);
	}

	int TestCollision(ItemInfo* creatureItemPtr, ItemInfo* playerItemPtr)
	{
		auto creatureSpheres = GetSpheres(creatureItemPtr, (int)SphereSpaceFlags::World);
		auto playerSpheres = GetSpheres(playerItemPtr, (int)SphereSpaceFlags::World);

		playerItemPtr->TouchBits.ClearAll();

		int flags = 0;
		if (creatureSpheres.empty())
		{
			creatureItemPtr->TouchBits.ClearAll();
			return flags;
		}

		// Run through creature spheres.
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

				// Calculate parameters.
				auto creatureSpherePos = creatureItemPtr->Pose.Position.ToVector3() + creatureSphere.Center;
				auto playerSpherePos = creatureItemPtr->Pose.Position.ToVector3() + playerSphere.Center;
				float distMax = SQUARE(creatureSphere.Radius + playerSphere.Radius);

				// Test distance.
				if (Vector3::DistanceSquared(creatureSpherePos, playerSpherePos) > distMax)
					continue;

				// Set touch bits.
				creatureItemPtr->TouchBits.Set(i);
				playerItemPtr->TouchBits.Set(j);
				flags |= 1 << i;
				break;
			}
		}

		return flags;
	}
}
