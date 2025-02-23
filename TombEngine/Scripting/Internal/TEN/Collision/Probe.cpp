#include "framework.h"
#include "Scripting/Internal/TEN/Collision/Probe.h"

#include "Game/collision/Point.h"
#include "Game/Lara/lara_climb.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"

using namespace TEN::Collision::Point;

namespace TEN::Scripting::Collision
{
	/// Represents a collision probe in the game world.
	// Provides collision information from a reference world position.
	//
	// @tenclass Collision.Collision
	// pragma nostrip

    void ScriptProbe::Register(sol::table& parent)
    {
		using ctors = sol::constructors<
			ScriptProbe(const Vec3&, int),
			ScriptProbe(const Vec3&, int, const Vec3&, float),
			ScriptProbe(const Vec3&, int, float, float, float, float, sol::optional<Vec3>&),
			ScriptProbe(const Moveable& mov)>;

		// Register type.
		parent.new_usertype<ScriptProbe>(
			"Probe",
			ctors(), sol::call_constructor, ctors(),

			// Getters
			"GetPosition", &ScriptProbe::GetPosition,
			"GetRoomNumber", &ScriptProbe::GetRoomNumber,
			"GetFloorHeight", &ScriptProbe::GetFloorHeight,
			"GetWaterSurfaceHeight", &ScriptProbe::GetWaterSurfaceHeight,
			"GetCeilingHeight", &ScriptProbe::GetCeilingHeight,
			"GetFloorNormal", &ScriptProbe::GetFloorNormal,
			"GetCeilingNormal", &ScriptProbe::GetCeilingNormal,
			"GetFloorMaterialType", &ScriptProbe::GetFloorMaterialType,
			"GetCeilingMaterialType", &ScriptProbe::GetCeilingMaterialType,

			// Inquirers
			"IsSteepFloor", &ScriptProbe::IsSteepFloor,
			"IsSteepCeiling", &ScriptProbe::IsSteepCeiling,
			"IsWall", &ScriptProbe::IsWall,
			"IsInsideSolidGeometry", &ScriptProbe::IsInsideSolidGeometry,
			"IsClimbableWall", &ScriptProbe::IsClimbableWall,
			"IsMonkeySwing", &ScriptProbe::IsMonkeySwing,
			"IsDeathTile", &ScriptProbe::IsDeath);
    }

	ScriptProbe::ScriptProbe(const Vec3& pos, int roomNumber)
	{
		_pointCollision = GetPointCollision(pos.ToVector3i(), roomNumber);
	}

	ScriptProbe::ScriptProbe(const Moveable& mov)
	{
		// TODO: *MUST* pass native ItemInfo moveable to allow PointCollisionData to handle quirks associated with the way moveables update their room numebrs.
		// GetPointCollision(mov.GetNativeMoveable());

		_pointCollision = GetPointCollision(mov.GetPosition().ToVector3i(), mov.GetRoomNumber());
	}

	ScriptProbe::ScriptProbe(const Vec3& pos, int roomNumber, const Vec3& dir, float dist)
	{
		_pointCollision = GetPointCollision(pos.ToVector3(), roomNumber, dir.ToVector3(), dist);
	}

	ScriptProbe::ScriptProbe(const Vec3& pos, int roomNumber, float headingAngle, float forward, float down, float right, sol::optional<Vec3>& axis)
	{
		static const auto DEFAULT_AXIS = Vec3(0.0f, 1.0f, 0.0f);

		short convertedAngle = ANGLE(headingAngle);
		auto convertedAxis = axis.value_or(DEFAULT_AXIS).ToVector3();
		_pointCollision = GetPointCollision(pos.ToVector3i(), roomNumber, convertedAngle, forward, down, right, convertedAxis);
	}

	/// Get the world position of this Probe.
	// @treturn Vec3 World position.
	Vec3 ScriptProbe::GetPosition()
	{
		return Vec3(_pointCollision.GetPosition());
	}

	// TODO: Return actualy room object? Not sure on Lua API conventions.
	/// Get the room number of this Probe.
	// @treturn int Room number.
	int ScriptProbe::GetRoomNumber()
	{
		return _pointCollision.GetRoomNumber();
	}

	/// Get the floor height at this Probe.
	// @treturn int[opt] Floor height. __nil if no floor exists.__
	sol::optional<int> ScriptProbe::GetFloorHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetFloorHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	/// Get the ceiling height at this Probe.
	// @treturn int[opt] Ceiling height. __nil if no ceiling exists.__
	sol::optional<int> ScriptProbe::GetCeilingHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetCeilingHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	/// Get the water surface height at this Probe.
	// @treturn int[opt] Water surface height. __nil if no water surface exists.__
	sol::optional<int> ScriptProbe::GetWaterSurfaceHeight()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		int height = _pointCollision.GetWaterSurfaceHeight();
		if (height != NO_HEIGHT)
			return height;

		return sol::nullopt;
	}

	/// Get the normal of the floor at this Probe.
	// @treturn Vec3[opt] Floor normal. __nil if no floor exists.__
	sol::optional<Vec3> ScriptProbe::GetFloorNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetFloorNormal());
	}

	/// Get the normal of the ceiling at this Probe.
	// @treturn Vec3[opt] Ceiling normal. __nil if no ceiling exists.__
	sol::optional<Vec3> ScriptProbe::GetCeilingNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetCeilingNormal());
	}

	/// Get the material type of the floor at this Probe.
	// @treturn Collision.MaterialType[opt] Floor material type. __nil if no floor exists.__
	sol::optional<MaterialType> ScriptProbe::GetFloorMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetBottomSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, true);
		return material;
	}

	/// Get the material type of the ceiling at this Probe.
	// @treturn Collision.MaterialType[opt] Ceiling material type. __nil if no ceiling exists.__
	sol::optional<MaterialType> ScriptProbe::GetCeilingMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetTopSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, false);
		return material;
	}

	/// Check if the floor at this Probe is steep.
	// @treturn bool[opt] Steep floor status. __nil if no floor exists.__
	sol::optional<bool> ScriptProbe::IsSteepFloor()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepFloor();
	}

	/// Check if the ceiling at this Probe is steep.
	// @treturn bool[opt] Steep ceiling status. __nil if no ceiling exists.__
	sol::optional<bool> ScriptProbe::IsSteepCeiling()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepCeiling();
	}

	/// Check if there is a wall at this Probe. Can be used to determine if a wall and ceiling exists.
	// @treturn bool Wall status. __true: is a wall, false: isn't a wall__
	bool ScriptProbe::IsWall()
	{
		return _pointCollision.IsWall();
	}

	/// Check if this Probe is inside solid geometry, i.e. below a floor, above a ceiling, or inside a wall.
	// @treturn bool Inside geometry status. __true: is inside, false: is outside__
	bool ScriptProbe::IsInsideSolidGeometry()
	{
		if (_pointCollision.IsWall() ||
			_pointCollision.GetPosition().y > _pointCollision.GetFloorHeight() ||
			_pointCollision.GetPosition().y < _pointCollision.GetCeilingHeight())
		{
			return true;
		}

		return false;
	}

	/// Check if there is a climbable wall in the given heading angle at this Probe.
	// @tparam float headingAngle Heading angle at which to check for a climbable wall.
	// @treturn bool Climbable wall status. __true: is climbable, false: isn't climbable__
	bool ScriptProbe::IsClimbableWall(float headingAngle)
	{
		const auto& sector = _pointCollision.GetBottomSector();
		auto dirFlag = GetClimbDirectionFlags(ANGLE(headingAngle));
		return sector.Flags.IsWallClimbable(dirFlag);
	}

	/// Check if there is a monkey swing at this Probe.
	// @treturn bool Monkey swing status. __true: is a monkey swing, false: isn't a monkey swing__
	bool ScriptProbe::IsMonkeySwing()
	{
		const auto& sector = _pointCollision.GetTopSector();
		return sector.Flags.Monkeyswing;
	}

	/// Check if there is a death tile at this Probe.
	// @treturn bool Death tile status. __true: is a death tile, false: isn't a death tile__
	bool ScriptProbe::IsDeath()
	{
		const auto& sector = _pointCollision.GetBottomSector();
		return sector.Flags.Death;
	}
}