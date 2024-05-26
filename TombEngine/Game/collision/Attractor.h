#pragma once
#include "Game/collision/AttractorDebug.h"
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractor
{
	enum class AttractorType
	{
		Edge,
		WallEdge,
		/*VerticalPole, // """""""""Polerope"""""""""
		HorizontalPole, // TODO: AOD pipe shimmy + pipe crawl.
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	class AttractorObject
	{
	private:
		// Members
		AttractorType		 _type			 = AttractorType::Edge;
		std::vector<Vector3> _points		 = {};
		int					 _roomNumber	 = 0;
		std::vector<float>	 _segmentLengths = {};
		float				 _length		 = 0.0f;
		BoundingBox			 _box			 = BoundingBox();

		std::set<int> _attachedPlayerItemNumbers = {};

	public:
		// Constructors
		AttractorObject();
		AttractorObject(AttractorType type, const std::vector<Vector3>& points, int roomNumber);

		// Destructors
		~AttractorObject();

		// Getters
		AttractorType				GetType() const;
		const std::vector<Vector3>& GetPoints() const;
		int							GetRoomNumber() const;
		const std::vector<float>&	GetSegmentLengths() const;
		float						GetLength() const;
		const BoundingBox&			GetBox() const;

		// Inquirers
		bool IsLooped() const;

		// Utilities
		unsigned int GetSegmentCount() const;
		unsigned int GetSegmentIDAtChainDistance(float chainDist) const;
		Vector3		 GetIntersectionAtChainDistance(float chainDist) const;

		void Update(const std::vector<Vector3>& points, int roomNumber);
		void AttachPlayer(ItemInfo& playerItem);
		void DetachPlayer(ItemInfo& playerItem);
		void DetachAllPlayers();

		void DrawDebug(unsigned int segmentID) const;
		void DrawDebug() const;

	private:
		// Helpers
		float NormalizeChainDistance(float chainDist) const;
		void  Cache();
	};

	class AttractorCollisionData
	{
	public:
		// Members
		AttractorObject* Attractor = nullptr;

		unsigned int SegmentID	   = 0;
		Vector3		 Intersection  = Vector3::Zero;
		float		 Distance2D	   = 0.0f;
		float		 Distance3D	   = 0.0f;
		float		 ChainDistance = 0.0f;

		short HeadingAngle = 0;
		short SlopeAngle   = 0;
		bool  IsInFront	   = false;

		// Constructors
		AttractorCollisionData() = default;
		AttractorCollisionData(AttractorObject& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle, const Vector3& axis);
	};

	// Getters
	AttractorCollisionData GetAttractorCollision(AttractorObject& attrac, unsigned int segmentID, const Vector3& pos, short headingAngle,
												 const Vector3& axis = Vector3::UnitY);
	AttractorCollisionData GetAttractorCollision(AttractorObject& attrac, float chainDist, short headingAngle,
												 const Vector3& axis = Vector3::UnitY);

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float radius,
															   const Vector3& axis = Vector3::UnitY);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float radius,
															   const Vector3& axis = Vector3::UnitY);

	// Debug
	void DrawNearbyAttractors(const Vector3& pos, int roomNumber, short headingAngle);
}
