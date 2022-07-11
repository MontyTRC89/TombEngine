#include "framework.h"
#include "EffectsFunctions.h"

#include "ReservedScriptNames.h"

/***
Functions to generate effects.
@tentable Effects 
@pragma nostrip
*/

namespace Effects
{
	void Register(sol::state* state, sol::table& parent) {
		sol::table table_effects{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Effects, table_effects);
	}
}


