#pragma once
#include <unordered_map>
#include <unordered_set>
#include "LuaHandler.h"
#include "Scripting/Objects/ScriptInterfaceObjectsHandler.h"
#include "Objects/Moveable/Moveable.h"
#include "Objects/Static/Static.h"
#include "Sink/Sink.h"
#include "Objects/AIObject/AIObject.h"
#include "Objects/SoundSource/SoundSource.h"
#include "Camera/Camera.h"

class ObjectsHandler : public ScriptInterfaceObjectsHandler, public LuaHandler
{

public:
	ObjectsHandler::ObjectsHandler(sol::state* lua, sol::table& parent);

	bool NotifyKilled(ITEM_INFO* key) override;
	bool NotifyHit(ITEM_INFO* key) override;
	bool AddMoveableToMap(ITEM_INFO* key, Moveable* mov);
	bool RemoveMoveableFromMap(ITEM_INFO* key, Moveable* mov);

private:
	// A map between moveables and the engine entities they represent. This is needed
	// so that something that is killed by the engine can notify all corresponding
	// Lua variables which can then become invalid.
	std::unordered_map<ITEM_INFO *, std::unordered_set<Moveable*>>		m_moveables{};
	std::unordered_map<std::string, VarMapVal>					m_nameMap{};
	std::unordered_map<std::string, short>	 					m_itemsMapName{};
	sol::table m_table_objects;

	void AssignLara() override;

	template <typename R, char const* S>
	std::unique_ptr<R> GetByName(std::string const& name)
	{
		ScriptAssertF(m_nameMap.find(name) != m_nameMap.end(), "{} name not found: {}", S, name);
		return std::make_unique<R>(std::get<R::IdentifierType>(m_nameMap.at(name)));
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
	

