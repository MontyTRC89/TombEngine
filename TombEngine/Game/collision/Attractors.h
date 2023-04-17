#pragma once

class EulerAngles;
struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractors
{
	enum class AttractorType
	{
		Edge,
		/*VerticalPole,
		HorizontalPole,
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	class Attractor
	{
	private:
		// Members
		AttractorType		 Type		= AttractorType::Edge;
		std::vector<Vector3> Points		= {};
		int					 RoomNumber = 0;
		float				 Length		= 0.0f;

	public:
		// Constructors
		Attractor() {};
		Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber);

		// Getters
		AttractorType		 GetType() const;
		std::vector<Vector3> GetPoints() const;
		int					 GetRoomNumber() const;
		float				 GetLength() const;
		Vector3				 GetPointAtDistance(float dist) const;

		// Inquirers
		bool IsEdge() const;

		// Helpers
		void DrawDebug() const;

		Attractor& operator =(const Attractor& attrac);
	};

	struct AttractorCollisionData
	{
		const Attractor* AttractorPtr = nullptr;

		Vector3 TargetPoint = Vector3::Zero;
		float	Alpha		= 0.0f;

		float Distance			= 0.0f;
		float DistanceFromStart = 0.0f;
		short HeadingAngle		= 0;
		short SlopeAngle		= 0;

		bool IsIntersected = false;
		bool IsInFront	   = false;
	};

	struct AttractorTargetData
	{
		Vector3 TargetPoint	 = Vector3::Zero;
		float	Distance	 = 0.0f;
		int		SegmentIndex = 0;
	};

	// TODO: Probably overkill but keeping ready if needed.
	// Purpose of this struct would be to preserve probe setup data
	// without having a copy for every attractor collision data object.
	struct AttractorCollisions
	{
		const std::vector<AttractorCollisionData> Collisions = {};

		Vector3 RefPoint = Vector3::Zero;
		float	Range	 = 0.0f;
	};

	std::vector<const Attractor*>		GetNearbyAttractorPtrs(const Vector3& pos, int roomNumber, float range);
	std::vector<const Attractor*>		GetNearbyAttractorPtrs(const ItemInfo& item);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const std::vector<const Attractor*>& attracPtrs,
															   const Vector3& basePos, const EulerAngles& orient,
															   const Vector3& refPoint, float range);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const std::vector<const Attractor*>& attracPtrs,
															   const ItemInfo& item, const Vector3& refPoint, float range);

	Attractor				 GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop = true);
	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl);
}
