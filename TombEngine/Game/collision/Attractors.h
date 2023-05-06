#pragma once

class EulerAngles;
struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractors
{
	class Attractor;

	enum class AttractorType
	{
		Edge,
		/*VerticalPole, // """Polerope""".
		HorizontalPole, // TODO: AOD pipe shimmy + pipe crawl.
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	struct AttractorProximityData
	{
		Vector3		 Point		  = Vector3::Zero;
		float		 Distance	  = 0.0f;
		float		 LineDistance = 0.0f;
		unsigned int SegmentIndex = 0;
	};

	struct AttractorCollisionData
	{
		const Attractor* AttracPtr = nullptr;

		AttractorProximityData Proximity	= {};
		short				   HeadingAngle	= 0;
		short				   SlopeAngle	= 0;

		bool IsIntersected = false;
		bool IsInFront	   = false;
	};

	class Attractor
	{
	private:
		// Members
		AttractorType		 Type		= AttractorType::Edge;
		std::vector<Vector3> Points		= {};
		int					 RoomNumber = 0;
		float				 Length		= 0.0f;

		// TODO: Crashes on init.
		//std::unordered_map<int, ItemInfo&> AttachedPlayers = {};

	public:
		// Constructors
		Attractor() {}; // TODO: Remove. This ctor is for debug only.
		Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber);

		// Destructors
		~Attractor();

		// Getters
		AttractorType				GetType() const;
		const std::vector<Vector3>& GetPoints() const;
		int							GetRoomNumber() const;
		float						GetLength() const;

		// Utilities
		AttractorCollisionData GetCollision(const Vector3& basePos, const EulerAngles& orient, const Vector3& refPoint, float range) const;
		AttractorProximityData GetProximityData(const Vector3& refPoint) const;
		Vector3				   GetPointAtLineDistance(float lineDist) const;
		unsigned int		   GetSegmentIndexAtDistance(float lineDist) const;

		// Inquirers
		bool IsEdge() const;
		bool IsLooped() const;

		// Helpers
		void AttachPlayer(ItemInfo& entity);
		void DetachPlayer(ItemInfo& entity);
		void Update(const std::vector<Vector3>& points, int roomNumber);
		void DrawDebug() const;

	private:
		float NormalizeLineDistance(float lineDist) const;
	};

	// Temp
	std::vector<const Attractor*> GetDebugAttractorPtrs(const ItemInfo& item);

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& refPoint, float range);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& refPoint, float range);

	Attractor				 GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop = true);
	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl);
}
