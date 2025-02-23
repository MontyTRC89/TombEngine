#pragma once

#include "Game/collision/Point.h"

using namespace TEN::Collision::Point;

class Moveable;
class Vec3;

namespace TEN::Scripting::Collision
{
	// TODO:
	// Integrate line of sight queries (LOS).
	// Integrate moveable and static queries.

    class ScriptProbe
    {
    public:
        static void Register(sol::table& parent);

    private:
        // Fields

		PointCollisionData _pointCollision = PointCollisionData();

    public:
        // Constructors
		// TODO: Harmonise with Vec3 Translate() methods for consistency.

		ScriptProbe() = default;
		ScriptProbe(const Vec3& pos, int roomNumber);
		ScriptProbe(const Vec3& pos, int roomNumber, const Vec3& dir, float dist);
		ScriptProbe(const Vec3& pos, int roomNumber, float headingAngle, float forward, float down, float right, sol::optional<Vec3>& axis);
		ScriptProbe(const Moveable& mov);
		
        // Getters

		Vec3 GetPosition();
		int  GetRoomNumber();
		sol::optional<std::string>  GetRoomName();
		sol::optional<int>			GetFloorHeight();
		sol::optional<int>			GetCeilingHeight();
		sol::optional<int>			GetWaterSurfaceHeight();
		sol::optional<Vec3>			GetFloorNormal();
		sol::optional<Vec3>			GetCeilingNormal();
		sol::optional<MaterialType> GetFloorMaterialType();
		sol::optional<MaterialType> GetCeilingMaterialType();

		// Inquirers

		sol::optional<bool> IsSteepFloor();
		sol::optional<bool> IsSteepCeiling();
		bool				IsWall();
		bool				IsInsideSolidGeometry();
		bool				IsClimbableWall(float headingAngle);
		bool				IsMonkeySwing();
		bool				IsDeath();
    };
}
