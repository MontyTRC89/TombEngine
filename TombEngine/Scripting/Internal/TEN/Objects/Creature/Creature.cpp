#include "framework.h"
#include "Scripting/Internal/TEN/Objects/Creature/Creature.h"
#include "Scripting/Internal/TEN/Objects/Creature/CreatureStates.h"

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
			"GetMood", &LuaCreatureInfo::GetMood,
			"GetTarget", &LuaCreatureInfo::GetTarget,
			"GetTargetPosition", &LuaCreatureInfo::GetTargetPosition,
			"SetTarget", & LuaCreatureInfo::SetTarget,
			"SetTargetPosition", & LuaCreatureInfo::SetTargetPosition,
			"ClearTarget", & LuaCreatureInfo::ClearTarget,
			"IsAlerted", & LuaCreatureInfo::IsAlerted,
			"IsFriendly", & LuaCreatureInfo::IsFriendly,
			"IsHurtByLara", & LuaCreatureInfo::IsHurtByLara,
			"IsPoisoned", & LuaCreatureInfo::IsPoisoned,
			"IsAtGoa", & LuaCreatureInfo::IsAtGoal);
			
	}

	LuaCreatureInfo::LuaCreatureInfo(const Moveable& mov)
	{
			auto* item = &g_Level.Items[mov.GetIndex()];
			
			if (!CreatureActive(mov.GetIndex()))
				return;

			if (item->IsCreature())

			m_Creature = GetCreatureInfo(item);
	}

	MoodType LuaCreatureInfo::GetMood()
	{
		return m_Creature->Mood;
	}

	std::optional<Moveable> LuaCreatureInfo::GetTarget()
	{
		auto enemy = m_Creature->Enemy;
		return Moveable(enemy->Index);
	}

	Vec3 LuaCreatureInfo::GetTargetPosition()
	{
		return m_Creature->Target;
	}

	void LuaCreatureInfo::SetTarget(Moveable& moveable)
	{
		auto* item = &g_Level.Items[moveable.GetIndex()];
		m_Creature->Enemy = item;
	}

	void LuaCreatureInfo::SetTargetPosition(Vec3& position)
	{
		m_Creature->Target = position.ToVector3i();
	}

	void LuaCreatureInfo::ClearTarget()
	{
		m_Creature->Enemy = nullptr;
	}

	bool LuaCreatureInfo::IsAlerted()
	{
		return m_Creature->Alerted;
	}

	bool LuaCreatureInfo::IsFriendly()
	{
		return m_Creature->Friendly;
	}

	bool LuaCreatureInfo::IsHurtByLara()
	{
		return m_Creature->HurtByLara;
	}

	bool LuaCreatureInfo::IsPoisoned()
	{
		return m_Creature->Poisoned;
	}

	bool LuaCreatureInfo::IsAtGoal()
	{
		return m_Creature->ReachedGoal;
	}

	void Register(sol::state* state, sol::table& parent)
	{
		auto collTable2 = sol::table(state->lua_state(), sol::create);
		parent.set(ScriptReserved_Collision, collTable2);

		LuaCreatureInfo::Register(collTable2);

		auto handler = LuaHandler(state);
		handler.MakeReadOnlyTable(collTable2, ScriptReserved_MaterialType, CREATURE_MOOD);
	}
}
