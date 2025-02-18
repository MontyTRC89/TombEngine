#pragma once
#include "framework.h"
#include "Scripting/Internal/TEN/Effects/EffectsFunctions.h"


#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include <sol/sol.hpp>

#pragma once

namespace TEN::Scripting::Collision
{
    void RegisterPointCollision(sol::state& lua, sol::table& parent);
}