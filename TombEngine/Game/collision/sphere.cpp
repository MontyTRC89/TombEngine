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

	bool HandleItemSphereCollision(ItemInfo& item0, ItemInfo& item1)
	{
		constexpr auto FLAGS = (int)SphereSpaceFlags::World | (int)SphereSpaceFlags::BoneOrigin;

		auto spheres0 = GetSpheres(item0, FLAGS);
		auto spheres1 = GetSpheres(item1, FLAGS);

		item1.TouchBits.ClearAll();

		if (spheres0.empty())
		{
			item0.TouchBits.ClearAll();
			return false;
		}

		// Run through spheres of item 0.
		bool isCollided = false;
		for (int i = 0; i < spheres0.size(); i++)
		{
			// Get sphere 0.
			const auto& sphere0 = spheres0[i];
			if (sphere0.Radius <= 0.0f)
				continue;

			// Run through spheres of item 1.
			for (int j = 0; j < spheres1.size(); j++)
			{
				// Get sphere 1.
				const auto& sphere1 = spheres1[j];
				if (sphere1.Radius <= 0.0f)
					continue;

				// Test sphere collision.
				if (!sphere0.Intersects(sphere1))
					continue;

				// Set touch bits.
				item0.TouchBits.Set(i);
				item1.TouchBits.Set(j);

				isCollided = true;
				break;
			}
		}

		return isCollided;
	}
}
