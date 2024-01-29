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
		Attractor* AttractorPtr = nullptr;

		ProximityData Proximity	   = {};
		short		  HeadingAngle = 0;
		short		  SlopeAngle   = 0;
		bool		  IsInFront	   = false;

		// Constructors
		AttractorCollisionData() {};
		AttractorCollisionData(Attractor& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle, const Vector3& axis);

	private:
		// Helpers
		ProximityData GetProximity(const Vector3& pos, unsigned int segmentID, const Vector3& axis) const;
	};

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle, const Vector3& axis = Vector3::UnitY);
	AttractorCollisionData GetAttractorCollision(Attractor& attrac, float chainDist, short headingAngle, const Vector3& axis = Vector3::UnitY);

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float detectRadius, const Vector3& axis = Vector3::UnitY);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float detectRadius, const Vector3& axis = Vector3::UnitY);

	void DrawNearbyAttractors(const ItemInfo& item);
}
