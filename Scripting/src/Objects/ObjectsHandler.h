#pragma once
#include <unordered_map>
#include "LuaHandler.h"
#include "Scripting/Objects/ScriptInterfaceObjectsHandler.h"
#include "Objects/Moveable/Moveable.h"
#include "Objects/Static/Static.h"
#include "GameScriptSinkInfo.h"
#include "Objects/AIObject/AIObject.h"
#include "GameScriptSoundSourceInfo.h"
#include "GameScriptCameraInfo.h"

class ObjectsHandler : public ScriptInterfaceObjectsHandler, public LuaHandler
{

public:
	ObjectsHandler::ObjectsHandler(sol::state* lua, sol::table & parent);

private:
	std::unordered_map<std::string, VarMapVal>					m_nameMap{};
	std::unordered_map<std::string, short>	 					m_itemsMapName{};


	void AssignLara() override;

	template <typename R, char const* S>
	std::unique_ptr<R> GetByName(std::string const& name)
	{
		ScriptAssertF(m_nameMap.find(name) != m_nameMap.end(), "{} name not found: {}", S, name);
		return std::make_unique<R>(std::get<R::IdentifierType>(m_nameMap.at(name)), false);
	}

	bool AddName(std::string const& key, VarMapVal val) override
	{
		auto p = std::pair<std::string const&, VarMapVal>{ key, val };
		return m_nameMap.insert(p).second;
	}

	bool RemoveName(std::string const& key)
	{
		return m_nameMap.erase(key);
	}

	void FreeEntities() override
	{
		m_nameMap.clear();
	}
};
	

