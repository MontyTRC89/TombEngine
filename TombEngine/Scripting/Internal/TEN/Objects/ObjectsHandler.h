#pragma once
#include <unordered_map>
#include <unordered_set>

#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Objects/Static/StaticObject.h"
#include "Scripting/Internal/TEN/Objects/AIObject/AIObject.h"

class ObjectsHandler : public ScriptInterfaceObjectsHandler
{
public:
	ObjectsHandler::ObjectsHandler(sol::state* lua, sol::table& parent);

	bool NotifyKilled(ItemInfo* key) override;
	bool AddMoveableToMap(ItemInfo* key, Moveable* mov);
	bool RemoveMoveableFromMap(ItemInfo* key, Moveable* mov);

	bool TryAddColliding(short id) override
	{
		ItemInfo* item = &g_Level.Items[id];
		bool hasName = !(item->Callbacks.OnObjectCollided.empty() && item->Callbacks.OnRoomCollided.empty());
		if (hasName && item->Collidable && (item->Status != ITEM_INVISIBLE))
			return m_collidingItems.insert(id).second;

		return false;
	}

	bool TryRemoveColliding(short id, bool force = false) override
	{
		ItemInfo* item = &g_Level.Items[id];
		bool hasName = !(item->Callbacks.OnObjectCollided.empty() && item->Callbacks.OnRoomCollided.empty());
		if(!force && hasName && item->Collidable && (item->Status != ITEM_INVISIBLE))
			return false;

		return m_collidingItemsToRemove.insert(id).second;
	}

	void TestCollidingObjects() override;

private:
	LuaHandler m_handler;
	// A map between moveables and the engine entities they represent. This is needed
	// so that something that is killed by the engine can notify all corresponding
	// Lua variables which can then become invalid.
	std::unordered_map<ItemInfo *, std::unordered_set<Moveable*>>	moveables{};
	std::unordered_map<std::string, VarMapVal>						m_nameMap{};
	std::unordered_map<std::string, short>	 						m_itemsMapName{};
	// A set of items that are visible, collidable, and have Lua OnCollide callbacks.
	std::unordered_set<short>		 								m_collidingItems{};
	std::unordered_set<short>		 								m_collidingItemsToRemove{};
	sol::table m_table_objects;


	void AssignLara() override;

	template <typename R, char const* S>
	std::unique_ptr<R> GetByName(const std::string& name)
	{
		if (!ScriptAssertF(m_nameMap.find(name) != m_nameMap.end(), "{} name not found: {}", S, name))
			return nullptr;
		else
			return std::make_unique<R>(std::get<R::IdentifierType>(m_nameMap.at(name)));
	}

	template <typename R>
	std::vector <std::unique_ptr<R>> GetMoveablesBySlot(GAME_OBJECT_ID objID)
	{
		std::vector<std::unique_ptr<R>> items = {};
		for (auto& [key, val] : m_nameMap)
		{
			if (!std::holds_alternative<short>(val))
				continue;

			auto& item = g_Level.Items[GetIndexByName(key)];

			if (objID == item.ObjectNumber)
				items.push_back(GetByName<Moveable, ScriptReserved_Moveable>(key));
		}

		return items;
	}

	template <typename R>
	std::vector <std::unique_ptr<R>> GetStaticsBySlot(int slot)
	{
		std::vector<std::unique_ptr<R>> items = {};
		for (auto& [key, val] : m_nameMap)
		{
			if (!std::holds_alternative<std::reference_wrapper<MESH_INFO>>(val))
				continue;
			
			auto meshInfo = std::get<std::reference_wrapper<MESH_INFO>>(val).get();

			if (meshInfo.staticNumber == slot)
				items.push_back(GetByName<Static, ScriptReserved_Static>(key));
		}

		return items;
	}

	template <typename R>
	std::vector <std::unique_ptr<R>> GetRoomsByTag(std::string tag)
	{
		std::vector<std::unique_ptr<R>> rooms = {};
		for (auto& [key, val] : m_nameMap)
		{
			if (!std::holds_alternative<std::reference_wrapper<ROOM_INFO>>(val))
				continue;

			auto room = std::get<std::reference_wrapper<ROOM_INFO>>(val).get();
			
			if (std::any_of(room.tags.begin(), room.tags.end(),
				[&tag](const std::string& value) { return value == tag; }))
			{
				rooms.push_back(GetByName<Room, ScriptReserved_Room>(key));
			}
		}

		return rooms;
	}

	[[nodiscard]] short GetIndexByName(std::string const& name) const override
	{
		return std::get<short>(m_nameMap.at(name));
	}

	bool AddName(const std::string& key, VarMapVal val) override
	{
		if (key.empty())
			return false;

		auto p = std::pair< const std::string&, VarMapVal>{ key, val };
		return m_nameMap.insert(p).second;
	}

	bool RemoveName(const std::string& key)
	{
		return m_nameMap.erase(key);
	}

	void FreeEntities() override
	{
		m_nameMap.clear();
	}
};
