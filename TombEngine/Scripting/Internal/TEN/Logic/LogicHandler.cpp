#include "framework.h"
#include "LogicHandler.h"

#include <filesystem>

#include "Game/control/volume.h"
#include "Game/effects/Electricity.h"
#include "Game/savegame.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Logic/LevelFunc.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"

using namespace TEN::Effects::Electricity;

/***
Saving data, triggering functions, and callbacks for level-specific scripts.
@tentable Logic 
@pragma nostrip
*/

enum class CallbackPoint
{
	PreStart,
	PostStart,
	PreLoad,
	PostLoad,
	PreControl,
	PostControl,
	PreSave,
	PostSave,
	PreEnd,
	PostEnd
};

static const std::unordered_map<std::string, CallbackPoint> CALLBACK_POINTS
{
	{ ScriptReserved_PreStart, CallbackPoint::PreStart },
	{ ScriptReserved_PostStart, CallbackPoint::PostStart },
	{ ScriptReserved_PreLoad, CallbackPoint::PreLoad },
	{ ScriptReserved_PostLoad, CallbackPoint::PostLoad },
	{ ScriptReserved_PreControlPhase, CallbackPoint::PreControl },
	{ ScriptReserved_PostControlPhase, CallbackPoint::PostControl },
	{ ScriptReserved_PostSave, CallbackPoint::PostSave },
	{ ScriptReserved_PreSave, CallbackPoint::PreSave },
	{ ScriptReserved_PreEnd, CallbackPoint::PreEnd },
	{ ScriptReserved_PostEnd, CallbackPoint::PostEnd }
};

static const std::unordered_map<std::string, VolumeEventType> EVENT_TYPES
{
	{ ScriptReserved_OnEnter, VolumeEventType::Enter },
	{ ScriptReserved_OnInside, VolumeEventType::Inside },
	{ ScriptReserved_OnLeave, VolumeEventType::Leave }
};

enum class LevelEndReason
{
	LevelComplete,
	LoadGame,
	ExitToTitle,
	Death,
	Other
};

static const std::unordered_map<std::string, LevelEndReason> LEVEL_END_REASONS
{
	{ ScriptReserved_EndReasonLevelComplete, LevelEndReason::LevelComplete },
	{ ScriptReserved_EndReasonLoadGame, LevelEndReason::LoadGame },
	{ ScriptReserved_EndReasonExitToTitle, LevelEndReason::ExitToTitle },
	{ ScriptReserved_EndReasonDeath, LevelEndReason::Death },
	{ ScriptReserved_EndReasonOther, LevelEndReason::Other }
};

static constexpr char const* strKey = "__internal_name";

void SetVariable(sol::table tab, sol::object key, sol::object value)
{
	auto PutVar = [](sol::table tab, sol::object key, sol::object value)
	{
		switch (key.get_type())
		{
		case sol::type::number:
		case sol::type::string:
			tab.raw_set(key, value);
			break;

		default:
			ScriptAssert(false, "Unsupported key type used for special table. Valid types are string and number.", ErrorMode::Terminate);
			break;
		}
	};

	auto UnsupportedValue = [](sol::table tab, sol::object key)
	{
		key.push();

		size_t stringLength = 0;
		auto string = std::string(luaL_tolstring(tab.lua_state(), -1, &stringLength));

		if (!string.empty())
		{
			ScriptAssert(false, "Variable " + string + " has an unsupported type.", ErrorMode::Terminate);
			lua_pop(tab.lua_state(), 1);
		}
		else
		{
			ScriptAssert(false, "Variable has an unsupported type.", ErrorMode::Terminate);
		}

		key.pop();
	};

	switch (value.get_type())
	{
	case sol::type::lua_nil:
	case sol::type::boolean:
	case sol::type::number:
	case sol::type::string:
	case sol::type::table:
		PutVar(tab, key, value);
		break;

	case sol::type::userdata:
	{
		if (value.is<Vec2>() ||
			value.is<Vec3>() ||
			value.is<Rotation>() ||
			value.is<ScriptColor>())
		{
			PutVar(tab, key, value);
		}
		else
		{
			UnsupportedValue(tab, key);
		}
	}
		break;

	default:
		UnsupportedValue(tab, key);
	}
}

sol::object GetVariable(sol::table tab, sol::object key)
{
	return tab.raw_get<sol::object>(key);
}

LogicHandler::LogicHandler(sol::state* lua, sol::table & parent) : m_handler{ lua }
{
	m_handler.GetState()->set_function("print", &LogicHandler::LogPrint, this);

	sol::table tableLogic{ m_handler.GetState()->lua_state(), sol::create };

	parent.set(ScriptReserved_Logic, tableLogic);

	tableLogic.set_function(ScriptReserved_AddCallback, &LogicHandler::AddCallback, this);
	tableLogic.set_function(ScriptReserved_RemoveCallback, &LogicHandler::RemoveCallback, this);
	tableLogic.set_function(ScriptReserved_HandleEvent, &LogicHandler::HandleEvent, this);

	m_handler.MakeReadOnlyTable(tableLogic, ScriptReserved_EndReason, LEVEL_END_REASONS);
	m_handler.MakeReadOnlyTable(tableLogic, ScriptReserved_CallbackPoint, CALLBACK_POINTS);
	m_handler.MakeReadOnlyTable(tableLogic, ScriptReserved_EventType, EVENT_TYPES);

	m_callbacks.insert(std::make_pair(CallbackPoint::PreStart, &m_callbacksPreStart));
	m_callbacks.insert(std::make_pair(CallbackPoint::PostStart, &m_callbacksPostStart));
	m_callbacks.insert(std::make_pair(CallbackPoint::PreLoad, &m_callbacksPreLoad));
	m_callbacks.insert(std::make_pair(CallbackPoint::PostLoad, &m_callbacksPostLoad));
	m_callbacks.insert(std::make_pair(CallbackPoint::PreControl, &m_callbacksPreControl));
	m_callbacks.insert(std::make_pair(CallbackPoint::PostControl, &m_callbacksPostControl));
	m_callbacks.insert(std::make_pair(CallbackPoint::PreSave, &m_callbacksPreSave));
	m_callbacks.insert(std::make_pair(CallbackPoint::PostSave, &m_callbacksPostSave));
	m_callbacks.insert(std::make_pair(CallbackPoint::PreEnd, &m_callbacksPreEnd));
	m_callbacks.insert(std::make_pair(CallbackPoint::PostEnd, &m_callbacksPostEnd));

	LevelFunc::Register(tableLogic);

	ResetScripts(true);
}

void LogicHandler::ResetGameTables() 
{
	auto state = m_handler.GetState();
	MakeSpecialTable(state, ScriptReserved_GameVars, &GetVariable, &SetVariable);

	(*state)[ScriptReserved_GameVars][ScriptReserved_Engine] = sol::table{ *state, sol::create };
}

/*** Register a function as a callback.
@advancedDesc
This is intended for module/library developers who want their modules to do
stuff during level start/load/end/save/control phase, but don't want the level
designer to add calls to `OnStart`, `OnLoad`, etc. in their level script.

Possible values for CallbackPoint:
	-- These take functions which accept no arguments
	PRESTART -- will be called immediately before OnStart
	POSTSTART -- will be called immediately after OnStart

	PRESAVE -- will be called immediately before OnSave
	POSTSAVE -- will be called immediately after OnSave

	PRELOAD -- will be called immediately before OnLoad
	POSTLOAD -- will be called immediately after OnLoad

	-- These take a LevelEndReason arg, like OnEnd
	PREEND -- will be called immediately before OnEnd
	POSTEND -- will be called immediately after OnEnd

	-- These take functions which accepts a deltaTime argument
	PRECONTROLPHASE -- will be called immediately before OnControlPhase
	POSTCONTROLPHASE -- will be called immediately after OnControlPhase

The order in which two functions with the same CallbackPoint are called is undefined.
i.e. if you register `MyFunc` and `MyFunc2` with `PRECONTROLPHASE`, both will be called before `OnControlPhase`, but there is no guarantee that `MyFunc` will be called before `MyFunc2`, or vice-versa.

Any returned value will be discarded.

@function AddCallback
@tparam point CallbackPoint When should the callback be called?
@tparam function func The function to be called (must be in the `LevelFuncs` hierarchy). Will receive, as an argument, the time in seconds since the last frame.
@usage
	LevelFuncs.MyFunc = function(dt) print(dt) end
	TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRECONTROLPHASE, LevelFuncs.MyFunc)
*/
void LogicHandler::AddCallback(CallbackPoint point, const LevelFunc& levelFunc)
{
	auto it = m_callbacks.find(point);

	if (it == m_callbacks.end()) 
	{
		TENLog("Error: callback point not found. Attempted to access missing value.", LogLevel::Error, LogConfig::All, false);
		return;
	}
	
	if (it->second->find(levelFunc.m_funcName) != it->second->end())
	{
		TENLog("Warning: function " + levelFunc.m_funcName + " already registered in callbacks list.", LogLevel::Warning, LogConfig::All, true);
	}
	else
	{
		it->second->insert(levelFunc.m_funcName);
	}
}

/*** Deregister a function as a callback.
Will have no effect if the function was not registered as a callback

@function RemoveCallback
@tparam point CallbackPoint The callback point the function was registered with. See @{AddCallback}
@tparam func LevelFunc the function to remove; must be in the LevelFuncs hierarchy.
@usage
	TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PRECONTROLPHASE, LevelFuncs.MyFunc)
*/
void LogicHandler::RemoveCallback(CallbackPoint point, const LevelFunc& levelFunc)
{
	auto it = m_callbacks.find(point);
	if (it == m_callbacks.end())
	{
		TENLog("Error: callback point not found. Attempted to access missing value.", LogLevel::Error, LogConfig::All, false);
		return;
	}

	it->second->erase(levelFunc.m_funcName);
}

/*** Attempt to find an event set and exectute a particular event from it.

@function HandleEvent
@tparam name string Name of the event set to find.
@tparam type EventType Event to execute.
@tparam activator Moveable Optional activator. Default is the player object.
*/
void LogicHandler::HandleEvent(const std::string& name, VolumeEventType type, sol::optional<Moveable&> activator)
{
	bool success = TEN::Control::Volumes::HandleEvent(name, type, activator.has_value() ? 
					(VolumeActivator)activator.value().GetIndex() : nullptr);
	if (!success)
	{
		TENLog("Error: event " + name + " could not be executed. Check if event with such name exists in project.",
			   LogLevel::Error, LogConfig::All, false);
	}
}

void LogicHandler::ResetLevelTables()
{
	auto state = m_handler.GetState();
	MakeSpecialTable(state, ScriptReserved_LevelVars, &GetVariable, &SetVariable);

	(*state)[ScriptReserved_LevelVars][ScriptReserved_Engine] = sol::table{ *state, sol::create };
}

sol::object LogicHandler::GetLevelFuncsMember(sol::table tab, const std::string& name)
{
	std::string partName = tab.raw_get<std::string>(strKey);
	auto& map = m_levelFuncs_tablesOfNames[partName];

	auto fullNameIt = map.find(name);
	if (fullNameIt != std::cend(map))
	{
		std::string_view key = fullNameIt->second;
		if (m_levelFuncs_levelFuncObjects[key].valid())
			return m_levelFuncs_levelFuncObjects[key];
	}

	return sol::nil;
}

bool LogicHandler::SetLevelFuncsMember(sol::table tab, const std::string& name, sol::object value)
{
	if (sol::type::lua_nil == value.get_type())
	{
		std::string error{ "Tried to set " + std::string{ScriptReserved_LevelFuncs} + " member " };
		error += name + " to nil; this not permitted at this time.";
		return ScriptAssert(false, error);
	}
	else if (sol::type::function == value.get_type())
	{
		// Add name to table of names.
		auto partName = tab.raw_get<std::string>(strKey);
		auto fullName = partName + "." + name;
		auto& parentNameTab = m_levelFuncs_tablesOfNames[partName];
		parentNameTab.insert_or_assign(name, fullName);

		// Create LevelFunc userdata and add that too.
		LevelFunc levelFuncObject;
		levelFuncObject.m_funcName = fullName;
		levelFuncObject.m_handler = this;
		m_levelFuncs_levelFuncObjects[fullName] = levelFuncObject;

		// Add function itself.
		m_levelFuncs_luaFunctions[fullName] = value;
	}
	else if (sol::type::table == value.get_type())
	{
		// Create and add new name map.
		std::unordered_map<std::string, std::string> newNameMap;
		auto fullName = tab.raw_get<std::string>(strKey) + "." + name;
		m_levelFuncs_tablesOfNames.insert_or_assign(fullName, newNameMap);

		// Create new table to put in the LevelFuncs hierarchy.
		auto newLevelFuncsTab = MakeSpecialTable(m_handler.GetState(), name, &LogicHandler::GetLevelFuncsMember, &LogicHandler::SetLevelFuncsMember, this);
		newLevelFuncsTab.raw_set(strKey, fullName);
		tab.raw_set(name, newLevelFuncsTab);

		// "Populate" new table. This will trigger the __newindex metafunction and will
		// thus call this function recursively, handling all subtables and functions.
		for (auto& [key, val] : value.as<sol::table>())
			newLevelFuncsTab[key] = val;
	}
	else
	{
		std::string error{ "Failed to add " };
		error += name + " to " + ScriptReserved_LevelFuncs + " or one of its tables; it must be a function or a table of functions.";
		return ScriptAssert(false, error);
	}

	return true;
}

void LogicHandler::LogPrint(sol::variadic_args args)
{
	std::string str;
	for (const sol::object& o : args)
	{
		auto strPart = (*m_handler.GetState())["tostring"](o).get<std::string>();
		str += strPart;
		str += "\t";
	}

	TENLog(str, LogLevel::Info, LogConfig::All, true);
}

void LogicHandler::ResetScripts(bool clearGameVars)
{
	FreeLevelScripts();

	for (auto& [first, second] : m_callbacks)
		second->clear();

	auto currentPackage = m_handler.GetState()->get<sol::table>("package");
	auto currentLoaded = currentPackage.get<sol::table>("loaded");

	for (auto& [first, second] : currentLoaded)
		currentLoaded[first] = sol::nil;

	if (clearGameVars)
		ResetGameTables();

	m_handler.ResetGlobals();

	m_shortenedCalls = false;

	m_handler.GetState()->collect_garbage();
}

void LogicHandler::FreeLevelScripts()
{
	m_levelFuncs = MakeSpecialTable(m_handler.GetState(), ScriptReserved_LevelFuncs, &LogicHandler::GetLevelFuncsMember, &LogicHandler::SetLevelFuncsMember, this);
	m_levelFuncs.raw_set(strKey, ScriptReserved_LevelFuncs);

	m_levelFuncs[ScriptReserved_Engine] = sol::table{ *m_handler.GetState(), sol::create };

	m_levelFuncs_tablesOfNames.clear();
	m_levelFuncs_luaFunctions.clear();
	m_levelFuncs_levelFuncObjects = sol::table{ *m_handler.GetState(), sol::create };

	m_levelFuncs_tablesOfNames.emplace(std::make_pair(ScriptReserved_LevelFuncs, std::unordered_map<std::string, std::string>{}));

	ResetLevelTables();
	m_onStart = sol::nil;
	m_onLoad = sol::nil;
	m_onControlPhase = sol::nil;
	m_onSave = sol::nil;
	m_onEnd = sol::nil;
	m_handler.GetState()->collect_garbage();
}

// Used when loading.
void LogicHandler::SetVariables(const std::vector<SavedVar>& vars)
{
	ResetGameTables();
	ResetLevelTables();

	std::unordered_map<unsigned int, sol::table> solTables;

	for(int i = 0; i < vars.size(); ++i)
	{
		if (std::holds_alternative<IndexTable>(vars[i]))
		{
			solTables.try_emplace(i, *m_handler.GetState(), sol::create);
			auto indexTab = std::get<IndexTable>(vars[i]);
			for (auto& [first, second] : indexTab)
			{
				// if we're wanting to reference a table, make sure that table exists
				// create it if need be
				if (std::holds_alternative<IndexTable>(vars[second]))
				{
					solTables.try_emplace(second, *m_handler.GetState(), sol::create);
					solTables[i][vars[first]] = solTables[second];
				}
				else if (std::holds_alternative<double>(vars[second]))
				{
					double theNum = std::get<double>(vars[second]);
					// If this is representable as an integer use an integer.
					// This is to ensure something saved as 1 is not loaded as 1.0
					// which would be confusing for the user.
					// todo: should we throw a warning if the user tries to save or load a value
					// outside of these bounds? - squidshire 30/04/2022
					if (std::trunc(theNum) == theNum && theNum <= INT64_MAX && theNum >= INT64_MIN)
					{
						solTables[i][vars[first]] = (int64_t)theNum;
					}
					else
					{
						solTables[i][vars[first]] = vars[second];
					}
				}
				else if (vars[second].index() == (int)SavedVarType::Vec2)
				{
					auto vec2 = Vec2(std::get<(int)SavedVarType::Vec2>(vars[second]));
					solTables[i][vars[first]] = vec2;
				}
				else if (vars[second].index() == int(SavedVarType::Vec3))
				{
					auto vec3 = Vec3(std::get<int(SavedVarType::Vec3)>(vars[second]));
					solTables[i][vars[first]] = vec3;
				}
				else if (vars[second].index() == int(SavedVarType::Rotation))
				{
					auto vec3 = Rotation(std::get<int(SavedVarType::Rotation)>(vars[second]));
					solTables[i][vars[first]] = vec3;
				}
				else if (vars[second].index() == int(SavedVarType::Color))
				{
					auto color = D3DCOLOR(std::get<int(SavedVarType::Color)>(vars[second]));
					solTables[i][vars[first]] = ScriptColor{color};
				}
				else if (std::holds_alternative<FuncName>(vars[second]))
				{
					LevelFunc levelFunc;
					levelFunc.m_funcName = std::get<FuncName>(vars[second]).name;
					levelFunc.m_handler = this;
					solTables[i][vars[first]] = levelFunc;
				}
				else
				{
					solTables[i][vars[first]] = vars[second];
				}
			}
		}
	}
	
	auto rootTable = solTables[0];

	sol::table levelVars = rootTable[ScriptReserved_LevelVars];
	for (auto& [first, second] : levelVars)
		(*m_handler.GetState())[ScriptReserved_LevelVars][first] = second;

	sol::table gameVars = rootTable[ScriptReserved_GameVars];
	for (auto& [first, second] : gameVars)
		(*m_handler.GetState())[ScriptReserved_GameVars][first] = second;
}

template<SavedVarType TypeEnum, typename TypeTo, typename TypeFrom, typename MapType>
int Handle(TypeFrom& var, MapType& varsMap, size_t& numVars, std::vector<SavedVar>& vars)
{
	auto [first, second] = varsMap.insert(std::make_pair(&var, (int)numVars));

	if (second)
	{
		SavedVar savedVar;
		TypeTo varTo = (TypeTo)var;
		savedVar.emplace<(int)TypeEnum>(varTo);
		vars.push_back(savedVar);
		++numVars;
	}

	return first->second;
}

std::string LogicHandler::GetRequestedPath() const
{
	std::string path;
	for (unsigned int i = 0; i < m_savedVarPath.size(); ++i)
	{
		auto key = m_savedVarPath[i];
		if (std::holds_alternative<unsigned int>(key))
		{
			path += "[" + std::to_string(std::get<unsigned int>(key)) + "]";
		}
		else if (std::holds_alternative<std::string>(key))
		{
			auto part = std::get<std::string>(key);
			if (i > 0)
				path += "." + part;
			else
				path += part;
		}
	}

	return path;
}

// Used when saving.
void LogicHandler::GetVariables(std::vector<SavedVar>& vars)
{
	sol::table tab{ *m_handler.GetState(), sol::create };
	tab[ScriptReserved_LevelVars] = (*m_handler.GetState())[ScriptReserved_LevelVars];
	tab[ScriptReserved_GameVars] = (*m_handler.GetState())[ScriptReserved_GameVars];

	std::unordered_map<void const*, unsigned int> varsMap;
	std::unordered_map<double, unsigned int> numMap;
	std::unordered_map<bool, unsigned int> boolMap;

	size_t numVars = 0;

	// The following functions will all try to put their values in a map. If it succeeds
	// then the value was not already in the map, so we can put it into the var vector.
	// If it fails, the value is in the map, and thus will also be in the var vector.
	// We then return the value's position in the var vector.

	// The purpose of this is to only store each value once, and to fill our tables with
	// indices to the values rather than copies of the values.

	auto handleNum = [&](auto num, auto map)
	{
		auto [first, second] = map.insert(std::make_pair(num, (int)numVars));

		if (second)
		{
			vars.push_back(num);
			++numVars;
		}

		return first->second;
	};

	auto handleStr = [&](const sol::object& obj)
	{
		auto str = obj.as<sol::string_view>();
		auto [first, second] = varsMap.insert(std::make_pair(str.data(), (int)numVars));

		if (second)
		{
			vars.push_back(std::string{ str.data() });
			++numVars;
		}

		return first->second;
	};

	auto handleFuncName = [&](const LevelFunc& fnh)
	{
		auto [first, second] = varsMap.insert(std::make_pair(&fnh, (int)numVars));

		if (second)
		{
			vars.push_back(FuncName{ std::string{ fnh.m_funcName } });
			++numVars;
		}

		return first->second;
	};

	std::function<unsigned int(const sol::table&)> populate = [&](const sol::table& obj)
	{
		auto [first, second] = varsMap.insert(std::make_pair(obj.pointer(), (int)numVars));

		if(second)
		{
			++numVars;
			auto id = first->second;

			vars.push_back(IndexTable{});

			for (auto& [first, second] : obj)
			{
				bool validKey = true;
				unsigned int keyIndex = 0;
				std::variant<std::string, unsigned int> key{unsigned int(0)};

				// Strings and numbers can be keys AND values.
				switch (first.get_type())
				{
				case sol::type::string:
				{
					keyIndex = handleStr(first);
					key = std::string{ first.as<sol::string_view>().data() };
					m_savedVarPath.push_back(key);
				}
					break;

				case sol::type::number:
				{
					if (double data = first.as<double>(); std::floor(data) != data)
					{
						ScriptAssert(false, "Tried using a non-integer number " + std::to_string(data) + " as a key in table " + GetRequestedPath());
						validKey = false;
					}
					else
					{
						keyIndex = handleNum(data, numMap);
						key = static_cast<unsigned int>(data);
						m_savedVarPath.push_back(key);
					}
				}
					break;

				default:
					validKey = false;
					ScriptAssert(false, "Tried using an unsupported type as a key in table " + GetRequestedPath());
				}

				if (!validKey)
					continue;

				auto putInVars = [&vars, id, keyIndex](unsigned int valIndex)
				{
					std::get<IndexTable>(vars[id]).push_back(std::make_pair(keyIndex, valIndex));
				};

				switch (second.get_type())
				{
				case sol::type::table:
					putInVars(populate(second.as<sol::table>()));
					break;

				case sol::type::string:
					putInVars(handleStr(second));
					break;

				case sol::type::number:
					putInVars(handleNum(second.as<double>(), numMap));
					break;

				case sol::type::boolean:
					putInVars(handleNum(second.as<bool>(), boolMap));
					break;

				case sol::type::userdata:
				{
					if (second.is<Vec2>())
					{
						putInVars(Handle<SavedVarType::Vec2, Vector2>(second.as<Vec2>(), varsMap, numVars, vars));
					}
					else if (second.is<Vec3>())
					{
						putInVars(Handle<SavedVarType::Vec3, Vector3>(second.as<Vec3>(), varsMap, numVars, vars));
					}
					else if (second.is<Rotation>())
					{
						putInVars(Handle<SavedVarType::Rotation, Vector3>(second.as<Rotation>(), varsMap, numVars, vars));
					}
					else if (second.is<ScriptColor>())
					{
						putInVars(Handle<SavedVarType::Color, D3DCOLOR>(second.as<ScriptColor>(), varsMap, numVars, vars));
					}
					else if (second.is<LevelFunc>())
					{
						putInVars(handleFuncName(second.as<LevelFunc>()));
					}
					else
					{
						ScriptAssert(false, "Tried saving an unsupported userdata as a value; variable is " + GetRequestedPath());
					}
				}
					break;

				default:
					ScriptAssert(false, "Tried saving an unsupported type as a value; variable is " + GetRequestedPath());
				}

				m_savedVarPath.pop_back();
			}
		}

		return first->second;
	};

	populate(tab);
}

void LogicHandler::GetCallbackStrings(	
	std::vector<std::string>& preStart,
	std::vector<std::string>& postStart,
	std::vector<std::string>& preEnd,
	std::vector<std::string>& postEnd,
	std::vector<std::string>& preSave,
	std::vector<std::string>& postSave,
	std::vector<std::string>& preLoad,
	std::vector<std::string>& postLoad,
	std::vector<std::string>& preControl,
	std::vector<std::string>& postControl) const
{
	auto populateWith = [](std::vector<std::string>& dest, const std::unordered_set<std::string>& src)
	{
		for (const auto& s : src)
			dest.push_back(s);
	};

	populateWith(preStart, m_callbacksPreStart);
	populateWith(postStart, m_callbacksPostStart);

	populateWith(preEnd, m_callbacksPreEnd);
	populateWith(postEnd, m_callbacksPostEnd);

	populateWith(preSave, m_callbacksPreSave);
	populateWith(postSave, m_callbacksPostSave);

	populateWith(preLoad, m_callbacksPreLoad);
	populateWith(postLoad, m_callbacksPostLoad);

	populateWith(preControl, m_callbacksPreControl);
	populateWith(postControl, m_callbacksPostControl);
}

void LogicHandler::SetCallbackStrings(	
	const std::vector<std::string>& preStart,
	const std::vector<std::string>& postStart,
	const std::vector<std::string>& preEnd,
	const std::vector<std::string>& postEnd,
	const std::vector<std::string>& preSave,
	const std::vector<std::string>& postSave,
	const std::vector<std::string>& preLoad,
	const std::vector<std::string>& postLoad,
	const std::vector<std::string>& preControl,
	const std::vector<std::string>& postControl)
{
	auto populateWith = [](std::unordered_set<std::string>& dest, const std::vector<std::string>& src)
	{
		for (const auto& string : src)
			dest.insert(string);
	};

	populateWith(m_callbacksPreStart, preStart);
	populateWith(m_callbacksPostStart, postStart);

	populateWith(m_callbacksPreEnd, preEnd);
	populateWith(m_callbacksPostEnd, postEnd);

	populateWith(m_callbacksPreSave, preSave);
	populateWith(m_callbacksPostSave, postSave);

	populateWith(m_callbacksPreLoad, preLoad);
	populateWith(m_callbacksPostLoad, postLoad);

	populateWith(m_callbacksPreControl, preControl);
	populateWith(m_callbacksPostControl, postControl);
}

template <typename R, char const * S, typename mapType>
std::unique_ptr<R> GetByName(const std::string& type, const std::string& name, const mapType& map)
{
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ErrorMode::Terminate);
	return std::make_unique<R>(map.at(name), false);
}

/*** Special objects
@section specialobjects
*/

/*** A @{Objects.Moveable} representing Lara herself.
@table Lara
*/
void LogicHandler::ResetVariables()
{
	(*m_handler.GetState())["Lara"] = nullptr;
}

void LogicHandler::ShortenTENCalls()
{
	auto str = R"(local ShortenInner 

	ShortenInner = function(tab)
		for k, v in pairs(tab) do
			if _G[k] then
				print("WARNING! Key " .. k .. " already exists in global environment!")
			else
				_G[k] = v
				if "table" == type(v) then
					if nil == v.__type then
						ShortenInner(v)
					end
				end
			end
		end
	end
	ShortenInner(TEN))";

	ExecuteString(str);

	m_shortenedCalls = true;
}

void LogicHandler::ExecuteScriptFile(const std::string& luaFilename)
{
	if (!m_shortenedCalls)
		ShortenTENCalls();

	m_handler.ExecuteScript(luaFilename);
}

void LogicHandler::ExecuteString(const std::string& command)
{
	m_handler.ExecuteString(command);
}

// These wind up calling CallLevelFunc, which is where all error checking is.
void LogicHandler::ExecuteFunction(const std::string& name, short idOne, short idTwo) 
{
	sol::protected_function func = m_levelFuncs_luaFunctions[name];

	func(std::make_unique<Moveable>(idOne), std::make_unique<Moveable>(idTwo));
}

void LogicHandler::ExecuteFunction(const std::string& name, TEN::Control::Volumes::VolumeActivator activator, const std::string& arguments)
{
	sol::protected_function func = (*m_handler.GetState())[ScriptReserved_LevelFuncs][name.c_str()];
	if (std::holds_alternative<short>(activator))
	{
		func(std::make_unique<Moveable>(std::get<short>(activator), true), arguments);
	}
	else
	{
		func(nullptr, arguments);
	}
}


void LogicHandler::OnStart()
{
	for (auto& name : m_callbacksPreStart)
		CallLevelFuncByName(name);

	if (m_onStart.valid())
		CallLevelFunc(m_onStart);

	for (auto& name : m_callbacksPostStart)
		CallLevelFuncByName(name);
}

void LogicHandler::OnLoad()
{
	for (auto& name : m_callbacksPreLoad)
		CallLevelFuncByName(name);

	if (m_onLoad.valid())
		CallLevelFunc(m_onLoad);

	for (auto& name : m_callbacksPostLoad)
		CallLevelFuncByName(name);
}

void LogicHandler::OnControlPhase(float deltaTime)
{
	for (auto& name : m_callbacksPreControl)
		CallLevelFuncByName(name, deltaTime);

	lua_gc(m_handler.GetState()->lua_state(), LUA_GCCOLLECT, 0);
	if (m_onControlPhase.valid())
		CallLevelFunc(m_onControlPhase, deltaTime);

	for (auto& name : m_callbacksPostControl)
		CallLevelFuncByName(name, deltaTime);
}

void LogicHandler::OnSave()
{
	for (auto& name : m_callbacksPreSave)
		CallLevelFuncByName(name);

	if (m_onSave.valid())
		CallLevelFunc(m_onSave);

	for (auto& name : m_callbacksPostSave)
		CallLevelFuncByName(name);
}

void LogicHandler::OnEnd(GameStatus reason)
{
	auto endReason{LevelEndReason::Other};

	switch (reason)
	{
	case GameStatus::LaraDead:
		endReason = LevelEndReason::Death;
		break;

	case GameStatus::LevelComplete:
		endReason = LevelEndReason::LevelComplete;
		break;

	case GameStatus::ExitToTitle:
		endReason = LevelEndReason::ExitToTitle;
		break;

	case GameStatus::LoadGame:
		endReason = LevelEndReason::LoadGame;
		break;
	}

	for (auto& name : m_callbacksPreEnd)
		CallLevelFuncByName(name, endReason);

	if(m_onEnd.valid())
		CallLevelFunc(m_onEnd, endReason);

	for (auto& name : m_callbacksPostEnd)
		CallLevelFuncByName(name, endReason);
}

/*** Special tables

TombEngine uses the following tables for specific things.

@section levelandgametables
*/

/*** A table with level-specific data which will be saved and loaded.
This is for level-specific information that you want to store in saved games.

@advancedDesc
For example, you may have a level with a custom puzzle where Lara has
to kill exactly seven enemies to open a door to a secret. You could use
the following line each time an enemy is killed:

	LevelVars.enemiesKilled = LevelVars.enemiesKilled + 1

If the player saves the level after killing three, saves, and then reloads the save
some time later, the values `3` will be put back into `LevelVars.enemiesKilled.`

__This table is emptied when a level is finished.__ If the player needs to be able
to return to the level (like in the Karnak and Alexandria levels in *The Last Revelation*),
you will need to use the @{GameVars} table, below.

__LevelVars.Engine is a reserved table used internally by TombEngine's libs. Do not modify, overwrite, or add to it.__

@table LevelVars
*/

/*** A table with game data which will be saved and loaded.
This is for information not specific to any level, but which concerns your whole
levelset or game, that you want to store in saved games.

@advancedDesc
For example, you may wish to have a final boss say a specific voice line based on
a choice the player made in a previous level. In the level with the choice, you could
write:

	GameVars.playerSnoopedInDrawers = true

And in the script file for the level with the boss, you could write:

	if GameVars.playerSnoopedInDrawers then
		PlayAudioTrack("how_dare_you.wav")
	end

Unlike @{LevelVars}, this table will remain intact for the entirety of the game.

__GameVars.Engine is a reserved table used internally by TombEngine's libs. Do not modify, overwrite, or add to it.__

@table GameVars
*/

/*** A table nested table system for level-specific functions.

@advancedDesc
This serves a few purposes: it holds the level callbacks (listed below) as well as
any trigger functions you might have specified. For example, if you give a trigger
a Lua name of "my_trigger" in Tomb Editor, you will have to implement it as a member
of this table:

	LevelFuncs.my_trigger = function() 
		-- implementation goes here
	end

You can organise functions into tables within the hierarchy:

	LevelFuncs.enemyFuncs = {}

	LevelFuncs.enemyFuncs.makeBaddyRunAway = function() 
		-- implementation goes here
	end

	LevelFuncs.enemyFuncs.makeBaddyUseMedkit = function() 
		-- implementation goes here
	end

There are two special subtables which you should __not__ overwrite:

	LevelFuncs.Engine -- this is for 'first-party' functions, i.e. ones that come with TombEngine.
	LevelFuncs.External -- this is for 'third-party' functions. If you write a library providing LevelFuncs functions for other builders to use in their levels, put those functions in LevelFuncs.External.YourLibraryNameHere

The following are the level callbacks. They are optional; if your level has no special
behaviour for a particular scenario, you do not need to implement the function. For
example, if your level does not need any special initialisation when it is loaded,
you can just leave out `LevelFuncs.OnStart`.

__The order of loading is as follows:__

1. The level data itself is loaded.
2. The level script itself is run (i.e. any code you put outside the `LevelFuncs` callbacks is executed).
3. Save data is loaded, if saving from a saved game (will empty `LevelVars` and `GameVars` and repopulate them with what they contained when the game was saved).
4. If loading from a save, `OnLoaded` will be called. Otherwise, `OnStart` will be called.
5. The control loop, in which `OnControlPhase` will be called once per frame, begins.

@tfield function OnStart Will be called when a level is entered by completing a previous level or by selecting it in the menu. Will not be called when loaded from a saved game.
@tfield function OnLoad Will be called when a saved game is loaded, just *after* data is loaded
@tfield function(float) OnControlPhase Will be called during the game's update loop,
and provides the delta time (a float representing game time since last call) via its argument.
@tfield function OnSave Will be called when the player saves the game, just *before* data is saved
@tfield function OnEnd(EndReason) Will be called when leaving a level. This includes finishing it, exiting to the menu, or loading a save in a different level. It can take an `EndReason` arg:

	EXITTOTITLE
	LEVELCOMPLETE
	LOADGAME
	DEATH
	OTHER

For example:
	LevelFuncs.OnEnd = function(reason)
		if(reason == TEN.Logic.EndReason.DEATH) then
			print("death")
		end
	end
@table LevelFuncs
*/

void LogicHandler::InitCallbacks()
{
	auto assignCB = [this](sol::protected_function& func, const std::string& luaFunc)
	{
		auto state = m_handler.GetState();
		std::string fullName = std::string{ ScriptReserved_LevelFuncs } + "." + luaFunc;

		sol::object theData = (*state)[ScriptReserved_LevelFuncs][luaFunc];

		std::string msg{ "Level's script does not define callback " + fullName + ". Defaulting to no " + fullName + " behaviour." };

		if (!theData.valid())
		{
			TENLog(msg);
			return;
		}

		LevelFunc fnh = (*state)[ScriptReserved_LevelFuncs][luaFunc];

		func = m_levelFuncs_luaFunctions[fnh.m_funcName];

		if (!func.valid())
			TENLog(msg);
	};

	assignCB(m_onStart, ScriptReserved_OnStart);
	assignCB(m_onLoad, ScriptReserved_OnLoad);
	assignCB(m_onControlPhase, ScriptReserved_OnControlPhase);
	assignCB(m_onSave, ScriptReserved_OnSave);
	assignCB(m_onEnd, ScriptReserved_OnEnd);
}
