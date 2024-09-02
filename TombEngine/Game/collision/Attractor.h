#pragma once

#include "Game/collision/AttractorDebug.h"
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractor
{
	class AttractorObject;

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

	struct AttractorCollisionData
	{
		AttractorObject* Attractor = nullptr;

		Vector3		 Intersection = Vector3::Zero;
		float		 Distance2D	  = 0.0f;
		float		 Distance3D	  = 0.0f;
		float		 PathDistance = 0.0f;
		unsigned int SegmentID	  = 0;

		short HeadingAngle = 0;
		short SlopeAngle   = 0;
		bool  IsInFront	   = false;
	};

	class AttractorObject
	{
	private:
		// Members

		AttractorType _type		  = AttractorType::Edge;
		int			  _roomNumber = 0;
		float		  _length	  = 0.0f;
		BoundingBox	  _aabb		  = BoundingBox();

		Vector3				 _position		 = Vector3::Zero;
		Quaternion			 _orientation	 = Quaternion::Identity;
		std::vector<Vector3> _points		 = {};
		std::vector<float>	 _segmentLengths = {};

		std::set<int> _playerItemNumbers = {};

	public:
		// Constructors

		AttractorObject() { _points.push_back(Vector3::Zero); }; // TEMP
		AttractorObject(AttractorType type, const Vector3& pos, const Quaternion& orient, int roomNumber, const std::vector<Vector3>& points);

		// Destructors

		~AttractorObject();

		// Getters

		AttractorType	   GetType() const;
		int				   GetRoomNumber() const;
		float			   GetLength() const;
		const BoundingBox& GetLocalAabb() const;
		BoundingOrientedBox GetWorldObb() const; // TEMP

		unsigned int GetSegmentCount() const;
		unsigned int GetSegmentIDAtPathDistance(float pathDist) const;
		Vector3		 GetIntersectionAtPathDistance(float pathDist) const;

		AttractorCollisionData				  GetCollision(float pathDist, short headingAngle, const Vector3& axis = Vector3::UnitY);
		std::optional<AttractorCollisionData> GetCollision(const BoundingSphere& sphere, short headingAngle, unsigned int segmentID, const Vector3& axis = Vector3::UnitY);

		// Setters
		
		void SetPosition(const Vector3& pos);
		void SetOrientation(const Quaternion& orient);

		// Inquirers

		bool IsLoop() const;

		// Utilities

		void AttachPlayer(ItemInfo& playerItem);
		void DetachPlayer(ItemInfo& playerItem);
		void DetachAllPlayers();

		void DrawDebug(unsigned int segmentID) const;
		void DrawDebug() const;

	private:
		// Helpers

		float  NormalizePathDistance(float pathDist) const;
		Matrix GetTransformMatrix() const;
		Matrix GetRotationMatrix() const;
	};

	// Getters

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float radius,
															   const Vector3& axis = Vector3::UnitY);
	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float radius,
															   const Vector3& axis = Vector3::UnitY);

	// Debug

	void DrawNearbyAttractors(const Vector3& pos, int roomNumber, short headingAngle);
}
