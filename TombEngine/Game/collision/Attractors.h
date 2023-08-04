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
		/*VerticalPole, // """""""""Polerope"""""""""
		HorizontalPole, // TODO: AOD pipe shimmy + pipe crawl.
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	struct AttractorProximityData
	{
		Vector3		 IntersectPoint = Vector3::Zero;
		float		 Distance		= 0.0f;
		float		 ChainDistance	= 0.0f;
		unsigned int SegmentID		= 0;
	};

	struct AttractorCollisionData
	{
		const Attractor& Attrac;

		AttractorProximityData Proximity = {};
		short HeadingAngle	  = 0;
		short SlopeAngle	  = 0;
		bool  IsIntersected	  = false;
		bool  IsFacingForward = false;
		bool  IsInFront		  = false;

		AttractorCollisionData(const Attractor& attrac) : Attrac(attrac) {};
	};

	class Attractor
	{
	private:
		// Members
		AttractorType		 Type		= AttractorType::Edge;
		std::vector<Vector3> Points		= {};
		int					 RoomNumber = 0;
		float				 Length		= 0.0f;
		BoundingBox			 Box		= BoundingBox(); 

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
		const BoundingBox&			GetBoundingBox() const;

		// Utilities
		AttractorCollisionData GetCollision(const Vector3& basePos, const EulerAngles& orient, const Vector3& refPoint, float range) const;
		AttractorProximityData GetProximity(const Vector3& refPoint) const;
		Vector3				   GetPointAtChainDistance(float chainDist) const;
		unsigned int		   GetSegmentIDAtChainDistance(float chainDist) const;

		// Inquirers
		bool IsEdge() const;
		bool IsLooped() const;

		// Helpers
		void AttachPlayer(ItemInfo& entity);
		void DetachPlayer(ItemInfo& entity);
		void Update(const std::vector<Vector3>& points, int roomNumber);
		void DrawDebug() const;

	private:
		// Helpers
		float NormalizeChainDistance(float chainDist) const;
	};

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& refPoint, float range);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& refPoint, float range);

	Attractor				 GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop = true);
	
	// TEMP
	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl);
}
