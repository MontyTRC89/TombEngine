#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Collision::Attractor
{
	class Attractor;

	class AttractorCollisionData
	{
	private:
		struct ProximityData
		{
			Vector3		 Intersection  = Vector3::Zero;
			float		 Distance2D	   = 0.0f;
			float		 Distance3D	   = 0.0f;
			float		 ChainDistance = 0.0f;
			unsigned int SegmentID	   = 0;
		};

	public:
		// Members
		Attractor* AttracPtr = nullptr;

		ProximityData Proximity		  = {};
		short		  HeadingAngle	  = 0;
		short		  SlopeAngle	  = 0;
		bool		  IsFacingForward = false;
		bool		  IsInFront		  = false;

		// Constructors
		AttractorCollisionData() {};
		AttractorCollisionData(Attractor& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle);

	private:
		// Helpers
		ProximityData GetProximity(const Vector3& pos, unsigned int segmentID) const;
	};

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle);
	AttractorCollisionData GetAttractorCollision(Attractor& attrac, float chainDist, short headingAngle);

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, float forward, float down, float right, float detectRadius);

	void DrawNearbyAttractors(const ItemInfo& item);
}
