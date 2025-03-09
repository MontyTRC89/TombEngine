#include "framework.h"
#include "Scripting/Internal/TEN/Objects/Creature/Creature.h"

#include "Game/collision/Point.h"
#include "Game/Lara/lara_climb.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Objects/Room/RoomObject.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"
#include "Specific/level.h"
#include <Game/misc.h>

using namespace TEN::Collision::Point;

namespace TEN::Scripting::Creature
{
	/// Represents a collision probe in the game world.
	// Provides collision information from a reference world position.
	//
	// @tenclass Collision.Probe
	// @pragma nostrip

	void LuaCreatureInfo::Register(sol::table& parent)
	{
		using ctors = sol::constructors<
			LuaCreatureInfo(const Moveable& mov)>;

		// Register type.
		parent.new_usertype<LuaCreatureInfo>("CreatureInfo",
			ctors(), sol::call_constructor, ctors(),

			// Getters
			"GetMood", & LuaCreatureInfo::GetMood);
			
	}


	LuaCreatureInfo::LuaCreatureInfo(const Moveable& mov)
	{
			auto* item = &g_Level.Items[mov.GetIndex()];
			m_Creature = GetCreatureInfo(item);
	}

	/// Get the world position of this Probe.
	// @function GetPosition
	// @treturn Vec3 World position.
	MoodType LuaCreatureInfo::GetMood()
	{
		return m_Creature->Mood;
	}

	/*			"GetTarget", & LuaCreatureInfo::GetTarget,

			// Setters
			"SetTarget", & LuaCreatureInfo::SetTarget,
			"SetTargetPosition", & LuaCreatureInfo::SetTargetPosition,

			// Inquirers
			"IsAlerted", & LuaCreatureInfo::IsAlerted,
			"IsFriendly", & LuaCreatureInfo::IsFriendly,
			"IsHurtByLara", &LuaCreatureInfo::IsHurtByLara,
			"IsPoisoned", & LuaCreatureInfo::IsPoisoned,
			"IsAtGoal", & LuaCreatureInfo::IsAtGoal
			
			/// Get the Room object of this Probe.
	// @function GetRoom
	// @treturn Room Room object.
	std::unique_ptr<Moveable&> LuaCreatureInfo::GetTarget()
	{
	
	}

	*/
	

	void Register(sol::state* state, sol::table& parent)
	{
		auto collTable2 = sol::table(state->lua_state(), sol::create);
		parent.set(ScriptReserved_Collision, collTable2);

		LuaCreatureInfo::Register(collTable2);

		//auto handler = LuaHandler(state);
		//handler.MakeReadOnlyTable(collTable, ScriptReserved_MaterialType, MATERIAL_TYPES);
	}
}
