#pragma once

#include "Game/collision/Point.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Objects/Room/RoomObject.h"

namespace sol { class state; };
namespace TEN::Scripting { class Rotation; }
class Moveable;
class Vec3;

namespace TEN::Scripting::Creature
{

    class LuaCreatureInfo
    {
    public:
        static void Register(sol::table& parent);

    private:
        // Fields

		CreatureInfo* m_Creature;

    public:
        // Constructors
		LuaCreatureInfo(const Moveable& mov);
				
        // Getters
		MoodType					GetMood();
		std::optional<Moveable>		GetTarget();
		Vec3						GetTargetPosition();
		
		// Setters
		void SetTarget(Moveable& moveable);
		void SetTargetPosition(Vec3& position);
		void ClearTarget();

		// Inquirers
		bool				IsAlerted();
		bool				IsFriendly();
		bool				IsHurtByLara();
		bool				IsPoisoned();
		bool				IsAtGoal();

    };

	void Register(sol::state* lua, sol::table& parent);
}
