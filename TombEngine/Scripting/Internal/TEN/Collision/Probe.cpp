#include "framework.h"
#include "Scripting/Internal/TEN/Collision/Probe.h"

#include "Game/collision/Point.h"
#include "Game/room.h"
#include "Game/Lara/lara_climb.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Collision::Room;

namespace TEN::Scripting::Collision
{
	/// Represents a collision probe in the game world.
	// Provides collision information from a reference world position.
	//
	// @tenclass Collision.Probe
	// pragma nostrip

    void ScriptProbe::Register(sol::table& parent)
    {
		using ctors = sol::constructors<
			ScriptProbe(const Vec3&, int),
			ScriptProbe(const Vec3&, int, const Vec3&, float),
			ScriptProbe(const Vec3&, int, const Rotation&, float),
			ScriptProbe(const Vec3&, int, const Rotation&, const Vec3&)>;

		// Register type.
		parent.new_usertype<ScriptProbe>(
			"Probe",
			ctors(), sol::call_constructor, ctors(),

			// Getters
			"GetPosition", &ScriptProbe::GetPosition,
			"GetRoomNumber", &ScriptProbe::GetRoomNumber,
			"GetRoomName", &ScriptProbe::GetRoomName,
			"GetFloorHeight", &ScriptProbe::GetFloorHeight,
			"GetCeilingHeight", &ScriptProbe::GetCeilingHeight,
			"GetWaterSurfaceHeight", &ScriptProbe::GetWaterSurfaceHeight,
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

	/// Create a Probe at a specified world position in a room.
	// @function Probe
	// @tparam Vec3 pos World position.
	// @tparam int roomNumber Room number.
	// @treturn Probe a new Probe.
	ScriptProbe::ScriptProbe(const Vec3& pos, int roomNumber)
	{
		_pointCollision = GetPointCollision(pos.ToVector3i(), roomNumber);
	}

	/// Create a Probe that casts from an origin world position in a room in a given direction for a specified distance.
	// Required to correctly traverse between rooms.
	// @function Probe
	// @tparam Vec3 pos Origin world position to cast from.
	// @tparam int originRoomNumber Origin's room number.
	// @tparam Vec3 dir Direction in which to cast.
	// @tparam float dist Distance to cast.
	// @treturn Probe a new Probe.
	ScriptProbe::ScriptProbe(const Vec3& origin, int originRoomNumber, const Vec3& dir, float dist)
	{
		_pointCollision = GetPointCollision(origin.ToVector3i(), originRoomNumber, dir.ToVector3(), dist);
	}

	/// Create a Probe that casts from an origin world position in a room in the direction of a given Rotation for a specified distance.
	// Required to correctly traverse between rooms.
	// @function Probe
	// @tparam Vec3 Origin world position to cast from.
	// @tparam int originRoomNumber Origin's room number.
	// @tparam Rotation rot Rotation's direction in which to cast.
	// @tparam float dist Distance to cast.
	// @treturn Probe a new Probe.
	ScriptProbe::ScriptProbe(const Vec3& pos, int originRoomNumber, const Rotation& rot, float dist)
	{
		auto dir = rot.ToEulerAngles().ToDirection();
		_pointCollision = GetPointCollision(pos.ToVector3(), originRoomNumber, dir, dist);
	}

	/// Create a Probe that casts from an origin world position, where a given relative offset is rotated according to a given Rotation.
	// Required to correctly traverse between rooms.
	// @function Probe
	// @tparam Vec3 Origin world position to cast from.
	// @tparam int originRoomNumber Origin's room number.
	// @tparam Rotation rot Rotation's direction in which to cast.
	// @tparam Vec3 relOffset Relative offset to cast.
	// @treturn Probe a new Probe.
	ScriptProbe::ScriptProbe(const Vec3& pos, int originRoomNumber, const Rotation& rot, const Vec3& relOffset)
	{
		auto target = Geometry::TranslatePoint(pos.ToVector3(), rot.ToEulerAngles(), relOffset.ToVector3());
		float dist = Vector3::Distance(pos.ToVector3(), target);

		auto dir = target - pos.ToVector3();
		dir.Normalize();

		_pointCollision = GetPointCollision(pos.ToVector3(), originRoomNumber, dir, dist);
	}

	/// Get the world position of this Probe.
	// @function GetPosition
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

	// TODO: Return actual Room object? Not sure on Lua API conventions.
	/// Get the room name of this Probe.
	// @function GetRoomName
	// @treturn string Room name.
	std::string ScriptProbe::GetRoomName()
	{
		int roomNumber = _pointCollision.GetRoomNumber();
		const auto& room = g_Level.Rooms[roomNumber];

		return room.Name;
	}

	/// Get the floor height at this Probe.
	// @function GetFloorHeight
	// @treturn int[opt] Floor height. __nil: no floor exists.__
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
	// @function GetCeilingHeight
	// @treturn int[opt] Ceiling height. __nil: no ceiling exists.__
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
	// @function GetWaterSurfaceHeight
	// @treturn int[opt] Water surface height. __nil: no water surface exists.__
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
	// @function GetFloorNormal
	// @treturn Vec3[opt] Floor normal. __nil: no floor exists.__
	sol::optional<Vec3> ScriptProbe::GetFloorNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetFloorNormal());
	}

	/// Get the normal of the ceiling at this Probe.
	// @function GetCeilingNormal
	// @treturn Vec3[opt] Ceiling normal. __nil: no ceiling exists.__
	sol::optional<Vec3> ScriptProbe::GetCeilingNormal()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;

		return Vec3(_pointCollision.GetCeilingNormal());
	}

	/// Get the material type of the floor at this Probe.
	// @function GetFloorMaterialType
	// @treturn Collision.MaterialType[opt] Floor material type. __nil: no floor exists.__
	sol::optional<MaterialType> ScriptProbe::GetFloorMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetBottomSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, true);
		return material;
	}

	/// Get the material type of the ceiling at this Probe.
	// @function GetCeilingMaterialType
	// @treturn Collision.MaterialType[opt] Ceiling material type. __nil: no ceiling exists.__
	sol::optional<MaterialType> ScriptProbe::GetCeilingMaterialType()
	{
		if (_pointCollision.IsWall())
			return sol::nullopt;
		
		const auto& sector = _pointCollision.GetTopSector();
		auto material = sector.GetSurfaceMaterial(_pointCollision.GetPosition().x, _pointCollision.GetPosition().z, false);
		return material;
	}

	/// Check if the floor at this Probe is steep.
	// @function IsSteepFloor
	// @treturn bool[opt] Steep floor status. __true: is steep floor, false: isn't steep floor, nil: no floor exists.__
	sol::optional<bool> ScriptProbe::IsSteepFloor()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepFloor();
	}

	/// Check if the ceiling at this Probe is steep.
	// @function IsSteepCeiling
	// @treturn bool[opt] Steep ceiling status. __true: is steep ceiling, false: isn't steep ceiling, nil: no ceiling exists.__
	sol::optional<bool> ScriptProbe::IsSteepCeiling()
	{
		if (_pointCollision.IsWall())
			return false;

		return _pointCollision.IsSteepCeiling();
	}

	/// Check if there is a wall at this Probe. Can be used to determine if a wall and ceiling exist.
	// @function IsWall
	// @treturn bool Wall status. __true: is a wall, false: isn't a wall__
	bool ScriptProbe::IsWall()
	{
		return _pointCollision.IsWall();
	}

	/// Check if this Probe is inside solid geometry, i.e. below a floor, above a ceiling, or inside a wall.
	// @function IsInsideSolidGeometry
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
	// @function IsClimbableWall
	// @tparam float headingAngle Heading angle at which to check for a climbable wall.
	// @treturn bool Climbable wall status. __true: is climbable, false: isn't climbable__
	bool ScriptProbe::IsClimbableWall(float headingAngle)
	{
		const auto& sector = _pointCollision.GetBottomSector();
		auto dirFlag = GetClimbDirectionFlags(ANGLE(headingAngle));
		return sector.Flags.IsWallClimbable(dirFlag);
	}

	/// Check if there is a monkey swing at this Probe.
	// @function IsMonkeySwing
	// @treturn bool Monkey swing status. __true: is a monkey swing, false: isn't a monkey swing__
	bool ScriptProbe::IsMonkeySwing()
	{
		const auto& sector = _pointCollision.GetTopSector();
		return sector.Flags.Monkeyswing;
	}

	/// Check if there is a death tile at this Probe.
	// @function IsDeath
	// @treturn bool Death tile status. __true: is a death tile, false: isn't a death tile__
	bool ScriptProbe::IsDeath()
	{
		const auto& sector = _pointCollision.GetBottomSector();
		return sector.Flags.Death;
	}
}
