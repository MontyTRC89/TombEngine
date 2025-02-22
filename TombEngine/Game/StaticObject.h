#pragma once

#include "Math/Math.h"
#include "Objects/objectslist.h"

using namespace TEN::Math;

enum GAME_OBJECT_ID : short;

// TODO: Come up with a namespace.
//namespace TEN
//{
	enum StaticMeshFlags
	{
		SM_VISIBLE = 1 << 0,
		SM_SOLID   = 1 << 1
	};

	// class StaticObject
	struct MESH_INFO
	{
	public:
		// Fields

		std::string	   Name		= {};
		//int			   Id		= 0; // TODO: Store static ID.
		GAME_OBJECT_ID ObjectId = GAME_OBJECT_ID::ID_NO_OBJECT;

		Pose  Transform  = Pose::Zero;
		int	  RoomNumber = 0;
		Color Color		 = SimpleMath::Color();

		int	 HitPoints = 0;
		int	 Flags	   = 0;
		bool Dirty	   = false;

		// Getters

		GameBoundingBox GetVisibilityAabb() const; // TODO: Use DX BoundingBox natively.
		GameBoundingBox GetCollisionAabb() const; // TODO: Use DX BoundingBox natively.

	private:
		// Helpers

		void ScaleAabb(GameBoundingBox& aabb) const;
	};

	// TODO: Deprecate.
	GameBoundingBox& GetBoundsAccurate(const MESH_INFO& staticObj, bool getVisibilityAabb);
//}
