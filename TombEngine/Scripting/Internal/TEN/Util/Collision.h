#pragma once

#include "Game/collision/Point.h"

using namespace TEN::Collision::Point;

class Moveable;
class Vec3;

namespace TEN::Scripting::Util
{
	// TODO:
	// Integrate line of sight queries (LOS).
	// Integrate moveable and static queries.

    class ScriptCollision
    {
    public:
        static void Register(sol::table& parent);

    private:
        // Fields

		PointCollisionData _pointCollision = PointCollisionData();

    public:
        // Constructors

		ScriptCollision() = default;
		ScriptCollision(const Vec3& pos, int roomNumber);
		ScriptCollision(const Vec3& pos, int roomNumber, const Vec3& dir, float dist);
		ScriptCollision(const Vec3& pos, int roomNumber, float headingAngle, float forward, float down, float right, sol::optional<Vec3>& axis);
		ScriptCollision(const Moveable& mov);
		
        // Getters

		Vec3 GetPosition();
		int  GetRoomNumber();

		sol::optional<int>	GetFloorHeight();
		sol::optional<int>	GetCeilingHeight();
		sol::optional<Vec3> GetFloorNormal();
		sol::optional<Vec3> GetCeilingNormal();
		sol::optional<MaterialType> GetSurfaceMaterial();

		sol::optional<int> GetWaterSurfaceHeight();

		// Inquirers

		bool IsSteepFloor();
		bool IsSteepCeiling();
		bool IsWall();
    };

}
