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

/***
Utility functions, mainly mathematical, for in-game calculations.
@tentable Utils 
@pragma nostrip
*/

namespace Util
{
	[[nodiscard]] static bool HasLineOfSight(short roomNumber1, Vec3 pos1, Vec3 pos2)
	{
		GameVector vec1, vec2;
		pos1.StoreInGameVector(vec1);
		vec1.RoomNumber = roomNumber1;
		pos2.StoreInGameVector(vec2);

		MESH_INFO* mesh;
		Vector3i vector;
		return LOS(&vec1, &vec2) && (ObjectOnLOS2(&vec1, &vec2, &vector, &mesh) == NO_LOS_ITEM);
	}

	static int CalculateDistance(const Vec3& pos1, const Vec3& pos2)
	{
		auto p1 = Vector3(pos1.x, pos1.y, pos1.z);
		auto p2 = Vector3(pos2.x, pos2.y, pos2.z);
		return (int)round(Vector3::Distance(p1, p2));
	}

	static int CalculateHorizontalDistance(const Vec3& pos1, const Vec3& pos2)
	{
		auto p1 = Vector2(pos1.x, pos1.z);
		auto p2 = Vector2(pos2.x, pos2.z);
		return (int)round(Vector2::Distance(p1, p2));
	}

	static std::tuple<int, int> PercentToScreen(double x, double y)
	{
		auto fWidth = static_cast<double>(g_Configuration.ScreenWidth);
		auto fHeight = static_cast<double>(g_Configuration.ScreenHeight);
		int resX = static_cast<int>(std::round(fWidth / 100.0 * x));
		int resY = static_cast<int>(std::round(fHeight / 100.0 * y));
		//todo this still assumes a resolution of 800/600. account for this somehow
		return std::make_tuple(resX, resY);
	}

	static std::tuple<double, double> ScreenToPercent(int x, int y)
	{
		auto fWidth = static_cast<double>(g_Configuration.ScreenWidth);
		auto fHeight = static_cast<double>(g_Configuration.ScreenHeight);
		double resX = x / fWidth * 100.0;
		double resY = y / fHeight * 100.0;
		return std::make_tuple(resX, resY);
	}

	static void PrintLog(const std::string& message, const LogLevel& level, TypeOrNil<bool> allowSpam)
	{
		TENLog(message, level, LogConfig::All, USE_IF_HAVE(bool, allowSpam, false));
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table tableUtil{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Util, tableUtil);

		///Calculate the distance between two positions.
		//@function CalculateDistance
		//@tparam Vec3 posA first position
		//@tparam Vec3 posB second position
		//@treturn int the direct distance from one position to the other
		tableUtil.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

		///Calculate the horizontal distance between two positions.
		//@function CalculateHorizontalDistance
		//@tparam Vec3 posA first position
		//@tparam Vec3 posB second position
		//@treturn int the direct distance on the XZ plane from one position to the other
		tableUtil.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);

		///Determine if there's a line of sight between two points.
		//
		//i.e. if we run a direct line from one position to another
		//will any geometry get in the way?
		//
		//Note: if you use this with Moveable:GetPosition to test if (for example)
		//two creatures can see one another, you might have to do some extra adjustments.
		//
		//This is because the "position" for most objects refers to its base, i.e., the floor.
		//As a solution, you can increase the y-coordinate of this position to correspond to roughly where the
		//eyes of the creatures would be.
		//@function HasLineOfSight
		//@tparam float room1 ID of the room where the first position is
		//@tparam Vec3 pos1 first position
		//@tparam Vec3 pos2 second position
		//@treturn bool is there a direct line of sight between the two positions?
		//@usage
		//local flamePlinthPos = flamePlinth:GetPosition() + Vec3(0, flamePlinthHeight, 0);
		//print(Util.HasLineOfSight(enemyHead:GetRoomNumber(), enemyHead:GetPosition(), flamePlinthPos))
		tableUtil.set_function(ScriptReserved_HasLineOfSight, &HasLineOfSight);

		///Translate a pair of percentages to screen-space pixel coordinates.
		//To be used with @{Strings.DisplayString:SetPosition} and @{Strings.DisplayString}.
		//@function PercentToScreen
		//@tparam float x percent value to translate to x-coordinate
		//@tparam float y percent value to translate to y-coordinate
		//@treturn int x coordinate in pixels
		//@treturn int y coordinate in pixels
		//@usage	
		//local halfwayX, halfwayY = PercentToScreen(50, 50)
		//local baddy
		//local spawnLocationNullmesh = GetMoveableByName("position_behind_left_pillar")
		//local str1 = DisplayString("You spawned a baddy!", halfwayX, halfwayY, Color(255, 100, 100), false, {DisplayStringOption.SHADOW, DisplayStringOption.CENTER})
		//
		//LevelFuncs.triggerOne = function(obj) 
		//	ShowString(str1, 4)
		//end
		tableUtil.set_function(ScriptReserved_PercentToScreen, &PercentToScreen);

		///Translate a pair of coordinates to percentages of window dimensions.
		//To be used with @{Strings.DisplayString:GetPosition}.
		//@function ScreenToPercent
		//@tparam int x pixel value to translate to a percentage of the window width
		//@tparam int y pixel value to translate to a percentage of the window height
		//@treturn float x coordinate as percentage
		//@treturn float y coordinate as percentage
		tableUtil.set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);

		/// Write messages within the Log file
		//@advancedDesc
		//For native Lua handling of errors, see the official Lua website:
		//
		//<a href="https://www.lua.org/pil/8.3.html">Error management</a>
		//
		//<a href="https://www.lua.org/manual/5.4/manual.html#pdf-debug.traceback">debug.traceback</a>
		//@function PrintLog
		//@tparam string message to be displayed within the Log
		//@tparam Util.LogLevel logLevel log level to be displayed
		//@tparam[opt] bool allowSpam true allows spamming of the message
		// 
		//@usage
		//PrintLog('test info log', LogLevel.INFO)
		//PrintLog('test warning log', LogLevel.WARNING)
		//PrintLog('test error log', LogLevel.ERROR)
		//-- spammed message
		//PrintLog('test spam log', LogLevel.INFO, true)
		// 
		tableUtil.set_function(ScriptReserved_PrintLog, &PrintLog);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(tableUtil, ScriptReserved_LogLevel, LOG_LEVEL);
	}
}
