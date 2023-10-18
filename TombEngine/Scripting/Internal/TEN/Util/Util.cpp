#include "framework.h"
#include "Scripting/Internal/TEN/Util/Util.h"

#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Util/LevelLog.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Specific/configuration.h"
#include "Specific/level.h"

/// Utility functions for various calculations.
// @tentable Utils 
// @pragma nostrip

namespace Util
{
	/// Determine if there is a clear line of sight between two positions.
	// NOTE: Limited to room geometry. Objects are ignored.
	// @function HasLineOfSight()
	// @tparam float roomID Room ID of the first position's room.
	// @tparam Vec3 posA First position.
	// @tparam Vec3 posB Second position.
	// @treturn bool __true__ if there is a line of sight, __false__ if not.
	// @usage
	// local flamePlinthPos = flamePlinth:GetPosition() + Vec3(0, flamePlinthHeight, 0);
	// print(Misc.HasLineOfSight(enemyHead:GetRoomNumber(), enemyHead:GetPosition(), flamePlinthPos))
	[[nodiscard]] static bool HasLineOfSight(int roomID, const Vec3& posA, const Vec3& posB)
	{
		auto vector0 = posA.ToGameVector();
		auto vector1 = posB.ToGameVector();

		MESH_INFO* meshPtr = nullptr;
		auto vector = Vector3i::Zero;
		return (LOS(&vector0, &vector1) &&
			ObjectOnLOS2(&vector0, &vector1, &vector, &meshPtr) == NO_LOS_ITEM);
	}

	///Calculate the distance between two positions.
	//@function CalculateDistance
	//@tparam Vec3 posA First position.
	//@tparam Vec3 posB Second position.
	//@treturn float Distance between two positions.
	static float CalculateDistance(const Vec3& posA, const Vec3& posB)
	{
		return posA.Distance(posB);
	}

	// TODO: Deprecated. Also should not use int!
	///Calculate the horizontal distance between two positions.
	//@function CalculateHorizontalDistance
	//@tparam Vec3 posA first position
	//@tparam Vec3 posB second position
	//@treturn float the direct distance on the XZ plane from one position to the other
	static int CalculateHorizontalDistance(const Vec3& pos1, const Vec3& pos2)
	{
		auto p1 = Vector2(pos1.x, pos1.z);
		auto p2 = Vector2(pos2.x, pos2.z);
		return (int)round(Vector2::Distance(p1, p2));
	}

	/// Translate a pair display position coordinates to pixel coordinates.
	//To be used with @{ Strings.DisplayString:SetPosition } and @{ Strings.DisplayString }.
	//@function PercentToScreen
	//@tparam float x X component of the display position.
	//@tparam float y Y component of the display position.
	//@treturn int x X coordinate in pixels.
	//@treturn int y Y coordinate in pixels.
	//@usage	
	//local halfwayX, halfwayY = PercentToScreen(50, 50)
	//local baddy
	//local spawnLocationNullmesh = GetMoveableByName("position_behind_left_pillar")
	//local str1 = DisplayString("You spawned an enemy!", halfwayX, halfwayY, Color(255, 100, 100), false, { DisplayStringOption.SHADOW, DisplayStringOption.CENTER })
	//
	//LevelFuncs.triggerOne = function(obj) 
	//	ShowString(str1, 4)
	//end
	static std::tuple<int, int> PercentToScreen(float x, float y)
	{
		float fWidth = g_Configuration.ScreenWidth;
		float fHeight = g_Configuration.ScreenHeight;
		int resX = (int)std::round(fWidth / 100.0f * x);
		int resY = (int)std::round(fHeight / 100.0f * y);

		return std::make_tuple(resX, resY);
	}

	/// Translate a pair of pixel coordinates to display position coordinates.
	//To be used with @{ Strings.DisplayString:GetPosition }.
	//@function ScreenToPercent
	//@tparam int x X pixel coordinate to translate to display position.
	//@tparam int y Y pixel coordinate to translate to display position.
	//@treturn float x X component of display position.
	//@treturn float y Y component of display position.
	static std::tuple<float, float> ScreenToPercent(int x, int y)
	{
		float fWidth = g_Configuration.ScreenWidth;
		float fHeight = g_Configuration.ScreenHeight;
		float resX = x / fWidth * 100.0f;
		float resY = y / fHeight * 100.0f;
		return std::make_tuple(resX, resY);
	}

	/// Write messages within the Log file
	//@advancedDesc
	//For native Lua handling of errors, see the official Lua website:
	//
	//<a href="https://www.lua.org/pil/8.3.html">Error management</a>
	//
	//<a href="https://www.lua.org/manual/5.4/manual.html#pdf-debug.traceback">debug.traceback</a>
	//@function PrintLog
	//@tparam string message to be displayed within the Log
	//@tparam Misc.LogLevel logLevel log level to be displayed
	//@tparam[opt] bool allowSpam true allows spamming of the message
	// 
	//@usage
	//PrintLog('test info log', LogLevel.INFO)
	//PrintLog('test warning log', LogLevel.WARNING)
	//PrintLog('test error log', LogLevel.ERROR)
	//-- spammed message
	//PrintLog('test spam log', LogLevel.INFO, true)
	// 
	static void PrintLog(const std::string& message, const LogLevel& level, TypeOrNil<bool> allowSpam)
	{
		TENLog(message, level, LogConfig::All, USE_IF_HAVE(bool, allowSpam, false));
	}

	void Register(sol::state* state, sol::table& parent)
	{
		auto tableUtil = sol::table(state->lua_state(), sol::create);
		parent.set(ScriptReserved_Util, tableUtil);

		tableUtil.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);
		tableUtil.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);
		tableUtil.set_function(ScriptReserved_HasLineOfSight, &HasLineOfSight);
		tableUtil.set_function(ScriptReserved_PercentToScreen, &PercentToScreen);
		tableUtil.set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);
		tableUtil.set_function(ScriptReserved_PrintLog, &PrintLog);

		auto handler = LuaHandler(state);
		handler.MakeReadOnlyTable(tableUtil, ScriptReserved_LogLevel, LOG_LEVEL);
	}
}
