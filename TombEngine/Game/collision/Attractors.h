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

	class Attractor
	{
	private:
		// Members
		AttractorType		 _type		 = AttractorType::Edge;
		std::vector<Vector3> _points	 = {};
		int					 _roomNumber = 0;
		float				 _length	 = 0.0f;
		BoundingBox			 _box		 = BoundingBox(); 

		// TODO
		//std::unordered_map<int, ItemInfo*> _attachedPlayers = {}; // Key = item number

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
		const BoundingBox&			GetBox() const;

		// Inquirers
		bool IsEdge() const;
		bool IsLooped() const;

		// Utilities
		AttractorCollisionData GetCollision(const Vector3& basePos, const EulerAngles& orient, const Vector3& probePoint);
		Vector3				   GetPointAtChainDistance(float chainDist) const;
		unsigned int		   GetSegmentIDAtChainDistance(float chainDist) const;

		void Update(const std::vector<Vector3>& points, int roomNumber);
		void AttachPlayer(ItemInfo& itemNumber);
		void DetachPlayer(ItemInfo& itemNumber);

		void DrawDebug() const;

	private:
		// Helpers
		AttractorProximityData GetProximity(const Vector3& probePoint) const;
		float NormalizeChainDistance(float chainDist) const;
		void  CacheLength();
		void  CacheBox();
	};

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& probePoint, float detectRadius);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& probePoint, float detectRadius);

	Attractor GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop = true);
	
	// TEMP
	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl);
}
