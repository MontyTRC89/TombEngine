#pragma once

class EulerAngles;
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
		AttractorCollisionData(Attractor& attrac, const Vector3& pos, short headingAngle, const Vector3& probePoint);

	private:
		// Helpers
		ProximityData GetProximity(const Vector3& probePoint) const;
	};

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, const Vector3& pos, short headingAngle, const Vector3& probePoint);
	
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   const Vector3& probePoint, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, const Vector3& dir,
															   float dist, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float detectRadius);

	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, float forward, float down, float right, float detectRadius);
}
