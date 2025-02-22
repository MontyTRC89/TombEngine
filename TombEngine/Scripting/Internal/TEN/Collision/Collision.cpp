#include "framework.h"
#include "Scripting/Internal/TEN/Collision/Collision.h"

#include "Game/collision/Point.h"
#include "Game/Lara/lara_climb.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"

using namespace TEN::Collision::Point;

namespace TEN::Scripting::Collision
{
	/// Represents a collision object in the game world.
	// Provides collision information at a given world position.
	//
	// @tenclass Collision.Collision
	// pragma nostrip

    void ScriptCollision::Register(sol::table& parent)
    {
		using ctors = sol::constructors<
			ScriptCollision(const Vec3&, int),
			ScriptCollision(const Vec3&, int, const Vec3&, float),
			ScriptCollision(const Vec3&, int, float, float, float, float, sol::optional<Vec3>&),
			ScriptCollision(const Moveable& mov)>;

		// Register type.
		parent.new_usertype<ScriptCollision>(
			"Collision",
			ctors(), sol::call_constructor, ctors(),

			// Getters
			"GetPosition", &ScriptCollision::GetPosition,
			"GetRoomNumber", &ScriptCollision::GetRoomNumber,
			"GetFloorHeight", &ScriptCollision::GetFloorHeight,
			"GetWaterSurfaceHeight", &ScriptCollision::GetWaterSurfaceHeight,
			"GetCeilingHeight", &ScriptCollision::GetCeilingHeight,
			"GetFloorNormal", &ScriptCollision::GetFloorNormal,
			"GetCeilingNormal", &ScriptCollision::GetCeilingNormal,
			"GetFloorMaterialType", &ScriptCollision::GetFloorMaterialType,
			"GetCeilingMaterialType", &ScriptCollision::GetCeilingMaterialType,

			// Inquirers
			"IsSteepFloor", &ScriptCollision::IsSteepFloor,
			"IsSteepCeiling", &ScriptCollision::IsSteepCeiling,
			"IsInsideSolidGeometry", &ScriptCollision::IsInsideSolidGeometry,
			"IsWall", &ScriptCollision::IsWall,
			"IsClimbableWall", &ScriptCollision::IsClimbableWall,
			"IsMonkeySwing", &ScriptCollision::IsMonkeySwing,
			"IsDeathTile", &ScriptCollision::IsDeath);

    }

	ScriptCollision::ScriptCollision(const Vec3& pos, int roomNumber)
	{
		_pointCollision = GetPointCollision(pos.ToVector3i(), roomNumber);
	}

	ScriptCollision::ScriptCollision(const Moveable& mov)
	{
		// TODO: *MUST* pass native ItemInfo moveable to allow PointCollisionData to handle quirks associated with the way moveables update their room numebrs.
		// GetPointCollision(mov.GetNativeMoveable());

		_pointCollision = GetPointCollision(mov.GetPosition().ToVector3i(), mov.GetRoomNumber());
	}

	ScriptCollision::ScriptCollision(const Vec3& pos, int roomNumber, const Vec3& dir, float dist)
	{
		_pointCollision = GetPointCollision(pos.ToVector3(), roomNumber, dir.ToVector3(), dist);
	}

	ScriptCollision::ScriptCollision(const Vec3& pos, int roomNumber, float headingAngle, float forward, float down, float right, sol::optional<Vec3>& axis)
	{
		static const auto DEFAULT_AXIS = Vec3(0.0f, 1.0f, 0.0f);

		short convertedAngle = ANGLE(headingAngle);
		auto convertedAxis = axis.value_or(DEFAULT_AXIS).ToVector3();
		_pointCollision = GetPointCollision(pos.ToVector3i(), roomNumber, convertedAngle, forward, down, right, convertedAxis);
	}

	Vec3 ScriptCollision::GetPosition()
	{
		return Vec3(_pointCollision.GetPosition());
	}

	int ScriptCollision::GetRoomNumber()
	{
		return _pointCollision.GetRoomNumber();
	}

	sol::optional<int> ScriptCollision::GetFloorHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetFloorHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	sol::optional<int> ScriptCollision::GetCeilingHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetCeilingHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	sol::optional<int> ScriptCollision::GetWaterSurfaceHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetWaterSurfaceHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	sol::optional<Vec3> ScriptCollision::GetFloorNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetFloorNormal());
	}

	sol::optional<Vec3> ScriptCollision::GetCeilingNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetCeilingNormal());
	}

	sol::optional<MaterialType> ScriptCollision::GetFloorMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetBottomSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, true);
		return material;
	}
	
	sol::optional<MaterialType> ScriptCollision::GetCeilingMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetTopSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, false);
		return material;
	}

	bool ScriptCollision::IsSteepFloor()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepFloor();
	}

	bool ScriptCollision::IsSteepCeiling()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepCeiling();
	}

	bool ScriptCollision::IsWall()
	{
		return _pointCollision.IsWall();
	}

	bool ScriptCollision::IsInsideSolidGeometry()
	{
		if (_pointCollision.IsWall() ||
			_pointCollision.GetPosition().y > _pointCollision.GetFloorHeight() ||
			_pointCollision.GetPosition().y < _pointCollision.GetCeilingHeight())
		{
			return true;
		}

		return false;
	}

	bool ScriptCollision::IsClimbableWall(float headingAngle)
	{
		const auto& sector = _pointCollision.GetBottomSector();
		auto dirFlag = GetClimbDirectionFlags(ANGLE(headingAngle));
		return sector.Flags.IsWallClimbable(dirFlag);
	}

	bool ScriptCollision::IsMonkeySwing()
	{
		const auto& sector = _pointCollision.GetTopSector();
		return sector.Flags.Monkeyswing;
	}

	bool ScriptCollision::IsDeath()
	{
		const auto& sector = _pointCollision.GetBottomSector();
		return sector.Flags.Death;
	}
}