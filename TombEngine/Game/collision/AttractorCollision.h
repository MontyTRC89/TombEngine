#pragma once

class EulerAngles;
struct ItemInfo;

namespace TEN::Collision::Attractor
{
	class Attractor;

	struct AttractorProximityData
	{
		Vector3		 Intersection  = Vector3::Zero;
		float		 Distance	   = 0.0f;
		float		 ChainDistance = 0.0f;
		unsigned int SegmentID	   = 0;
	};

	struct AttractorCollisionData
	{
		Attractor& Attrac;

		AttractorProximityData Proximity = {};
		short HeadingAngle	  = 0;
		short SlopeAngle	  = 0;
		bool  IsFacingForward = false;
		bool  IsInFront		  = false;

		AttractorCollisionData(Attractor& attrac) : Attrac(attrac) {};
	};

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, const Vector3& basePos, const EulerAngles& orient, const Vector3& probePoint);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& probePoint, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& probePoint, float detectRadius);
}
