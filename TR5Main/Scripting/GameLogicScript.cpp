#include "framework.h"
#include "GameLogicScript.h"
#include "GameScript.h"
#include "GameFlowScript.h"
#include "items.h"
#include "box.h"
#include "lara.h"
#include "savegame.h"
#include "lot.h"
#include "sound.h"
#include "setup.h"
#include "level.h"

namespace T5M::Script
{
	GameScript* g_GameScript;
	bool WarningsAsErrors = false;

	GameScript::GameScript()
	{
		m_lua.open_libraries(sol::lib::base);
		m_lua.set_exception_handler(lua_exception_handler);

		// Add constants
		//ExecuteScript("Scripts\\Constants.lua");

		m_lua.new_enum<GAME_OBJECT_ID>("Object", {
			{"LARA", ID_LARA}
			});

		// Add the item type
		m_lua.new_usertype<GameScriptPosition>("Position",
			"PosX", sol::property(&GameScriptPosition::GetXPos, &GameScriptPosition::SetXPos),
			"PosY", sol::property(&GameScriptPosition::GetYPos, &GameScriptPosition::SetYPos),
			"PosZ", sol::property(&GameScriptPosition::GetZPos, &GameScriptPosition::SetZPos),
			"RotX", sol::property(&GameScriptPosition::GetXRot, &GameScriptPosition::SetXRot),
			"RotY", sol::property(&GameScriptPosition::GetYRot, &GameScriptPosition::SetYRot),
			"RotZ", sol::property(&GameScriptPosition::GetZRot, &GameScriptPosition::SetZRot),
			"new", sol::no_constructor
			);

	
		m_lua.new_usertype<GameScriptItem>("Item",
			"Position", sol::property(&GameScriptItem::GetPosition),
			"HP", sol::property(&GameScriptItem::GetHP, &GameScriptItem::SetHP),
			"Room", sol::property(&GameScriptItem::GetRoom, &GameScriptItem::SetRoom),
			"Animation", sol::property(&GameScriptItem::GetAnimation),
			"CurrentState", sol::property(&GameScriptItem::GetCurrentState, &GameScriptItem::SetCurrentState),
			"GoalState", sol::property(&GameScriptItem::GetGoalState, &GameScriptItem::SetGoalState),
			"RequiredState", sol::property(&GameScriptItem::GetRequiredState, &GameScriptItem::SetRequiredState),
			"new", sol::no_constructor
			);

		m_lua.set_function("EnableItem", &GameScriptItem::EnableItem);
		m_lua.set_function("DisableItem", &GameScriptItem::DisableItem);

		m_lua.new_usertype<LuaVariables>("Variable",
			sol::meta_function::index, &LuaVariables::GetVariable,
			sol::meta_function::new_index, &LuaVariables::SetVariable,
			"new", sol::no_constructor
			);

		// GameScript type
		/*m_lua.new_usertype<GameScript>("GameScript",
			"PlayAudioTrack", &GameScript::PlayAudioTrack,
			"ChangeAmbientSoundTrack", &GameScript::ChangeAmbientSoundTrack,
			"MakeItemInvisible", &GameScript::MakeItemInvisible,
			"GetSecretsCount", &GameScript::GetSecretsCount,
			"SetSecretsCount", &GameScript::SetSecretsCount,
			"AddOneSecret", &GameScript::AddOneSecret,
			"JumpToLevel", &GameScript::JumpToLevel,
			"PlaySoundEffect", &GameScript::PlaySoundEffect,
			"PlaySoundEffectAtPosition", &GameScript::PlaySoundEffectAtPosition
			);*/

		m_lua.set_function("GetItemByID", &GameScript::GetItemById);
		m_lua.set_function("GetItemByName", &GameScript::GetItemByName);
		m_lua.set_function("NewPosition", &GameScript::NewPosition);
		m_lua.set_function("NewSectorPosition", &GameScript::NewSectorPosition);
		m_lua.set_function("NewRotation", &GameScript::NewRotation);
		m_lua.set_function("NewPosRot", &GameScript::NewPosRot);
		m_lua.set_function("CalculateDistance", &GameScript::CalculateDistance);
		m_lua.set_function("CalculateHorizontalDistance", &GameScript::CalculateHorizontalDistance);

		// Add global variables and namespaces
		//m_lua["TR"] = this;
	}

	void GameScript::AddTrigger(LuaFunction* function)
	{
		m_triggers.push_back(function);
		m_lua.script(function->Code);
	}


	void GameScript::AddLuaId(int luaId, short itemNumber)
	{
		m_itemsMapId.insert(std::pair<int, short>(luaId, itemNumber));
	}

	void GameScript::AddLuaName(std::string luaName, short itemNumber)
	{
		m_itemsMapName.insert(std::pair<std::string, short>(luaName, itemNumber));
	}

	void GameScript::FreeLevelScripts()
	{
		/*
		// Delete all triggers
		for (int i = 0; i < m_triggers.size(); i++)
		{
			LuaFunction* trigger = m_triggers[i];
			char* name = (char*)trigger->Name.c_str();
			m_lua[name] = NULL;
			delete m_triggers[i];
		}
		m_triggers.clear();

		// Clear the items mapping
		m_itemsMap.clear();

		m_lua["Lara"] = NULL;
		//delete m_Lara;
		*/
	}

	bool GameScript::ExecuteScript(const std::string& luaFilename, std::string& message)
	{ 
		auto result = m_lua.safe_script_file(luaFilename, sol::environment(m_lua.lua_state(), sol::create, m_lua.globals()), sol::script_pass_on_error);
		if (!result.valid())
		{
			sol::error error = result;
			message = error.what();
			return false;
		}
		return true;
	}

	bool GameScript::ExecuteString(const std::string& command, std::string& message)
	{
		auto result = m_lua.safe_script(command, sol::environment(m_lua.lua_state(), sol::create, m_lua.globals()), sol::script_pass_on_error);
		if (!result.valid())
		{
			sol::error error = result;
			message = error.what();
			return false;
		}
		return true;
	}

	bool GameScript::ExecuteTrigger(short index)
	{
		return true;
		/*
		// Is this a valid trigger?
		if (index >= m_triggers.size())
			return true;

		LuaFunction* trigger = m_triggers[index];

		// We want to execute a trigger just one time 
		// TODO: implement in the future continoous trigger?
		if (trigger->Executed)
			return true;

		// Get the trigger function name
		char* name = (char*)trigger->Name.c_str();

		// Execute trigger
		bool result = m_lua[name]();

		// Trigger was executed, don't execute it anymore
		trigger->Executed = result;

		m_locals.for_each([&](sol::object const& key, sol::object const& value) {
			if (value.is<bool>())
				std::cout << key.as<std::string>() << " " << value.as<bool>() << std::endl;
			else if (value.is<std::string>())
				std::cout << key.as<std::string>() << " " << value.as<std::string>() << std::endl;
			else
				std::cout << key.as<std::string>() << " " << value.as<int>() << std::endl;
		});

		return result;
		*/
	}

	void GameScript::PlayAudioTrack(short track)
	{
		S_CDPlay(track, SOUND_TRACK_ONESHOT);
	}

	void GameScript::ChangeAmbientSoundTrack(short track)
	{
		CurrentAtmosphere = track;
		S_CDStop();
		S_CDPlay(track, SOUND_TRACK_BGM);
	}

	void GameScript::JumpToLevel(int levelNum)
	{
		if (levelNum >= g_GameFlow->GetNumLevels())
			return;
		LevelComplete = levelNum;
	}

	int GameScript::GetSecretsCount()
	{
		return Savegame.Level.Secrets;
	}

	void GameScript::SetSecretsCount(int secretsNum)
	{
		if (secretsNum > 255)
			return;
		Savegame.Level.Secrets = secretsNum;
	}

	void GameScript::AddOneSecret()
	{
		if (Savegame.Level.Secrets >= 255)
			return;
		Savegame.Level.Secrets++;
		S_CDPlay(6, 0);
	}
	/*
	void GameScript::MakeItemInvisible(short id)
	{
		if (m_itemsMap.find(id) == m_itemsMap.end())
			return;

		short itemNum = m_itemsMap[id];

		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->active)
		{
			if (Objects[item->objectNumber].intelligent)
			{
				if (item->status == ITEM_ACTIVE)
				{
					item->touchBits = 0;
					item->status = ITEM_INVISIBLE;
					DisableBaddieAI(itemNum);
				}
			}
			else
			{
				item->touchBits = 0;
				item->status = ITEM_INVISIBLE;
			}
		}
	}
	*/
	template <typename T>
	void GameScript::GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
	{
		for (const auto& it : m_locals.variables)
		{
			if (it.second.is<T>())
				locals.insert(std::pair<std::string, T>(it.first, it.second.as<T>()));
		}
		for (const auto& it : m_globals.variables)
		{
			if (it.second.is<T>())
				globals.insert(std::pair<std::string, T>(it.first, it.second.as<T>()));
		}
	}

	template void GameScript::GetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
	template void GameScript::GetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
	template void GameScript::GetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

	template <typename T>
	void GameScript::SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
	{
		m_locals.variables.clear();
		for (const auto& it : locals)
		{
			m_locals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua.lua_state(), sol::in_place, it.second)));
		}
		for (const auto& it : globals)
		{
			m_globals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua.lua_state(), sol::in_place, it.second)));
		}
	}

	template void GameScript::SetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
	template void GameScript::SetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
	template void GameScript::SetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

	std::unique_ptr<GameScriptItem> GameScript::GetItemById(int id)
	{
		if (m_itemsMapId.find(id) == m_itemsMapId.end())
		{
			if (WarningsAsErrors)
				throw "item id not found";
			return std::unique_ptr<GameScriptItem>(nullptr);
		}

		return std::unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapId[id]));
	}

	std::unique_ptr<GameScriptItem> GameScript::GetItemByName(std::string name)
	{
		if (m_itemsMapName.find(name) == m_itemsMapName.end())
		{
			if (WarningsAsErrors)
				throw "item name not found";
			return std::unique_ptr<GameScriptItem>(nullptr);
		}

		return std::unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapName[name]));
	}

	void GameScript::PlaySoundEffectAtPosition(short id, int x, int y, int z, int flags)
	{
		PHD_3DPOS pos;

		pos.xPos = x;
		pos.yPos = y;
		pos.zPos = z;
		pos.xRot = 0;
		pos.yRot = 0;
		pos.zRot = 0;

		SoundEffect(id, &pos, flags);
	}

	void GameScript::PlaySoundEffect(short id, int flags)
	{
		SoundEffect(id, NULL, flags);
	}

	void GameScript::AssignItemsAndLara()
	{
		m_lua.set("Level", m_locals);
		m_lua.set("Game", m_globals);
		m_lua.set("Lara", GameScriptItem(Lara.itemNumber));
	}

	void GameScript::ResetVariables()
	{
		m_lua["Lara"] = NULL;
	}

	GameScriptPosition GameScript::NewPosition(int x, int y, int z)
	{
		GameScriptPosition pos;

		if (x < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative X coordinate");
			x = 0;
		}
		if (z < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative Z coordinate");
			z = 0;
		}

		pos.SetXPos(x);
		pos.SetYPos(y);
		pos.SetZPos(z);

		return pos;
	}

	GameScriptPosition GameScript::NewSectorPosition(int x, int y, int z)
	{
		GameScriptPosition pos;

		if (x < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative X coordinate");
			x = 0;
		}
		if (z < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative Z coordinate");
			z = 0;
		}

		pos.SetXPos(SECTOR(x) + CLICK(2));
		pos.SetYPos(SECTOR(y) + CLICK(2));
		pos.SetZPos(SECTOR(y) + CLICK(2));

		return pos;
	}

	GameScriptPosition GameScript::NewRotation(float x, float y, float z)
	{
		GameScriptPosition rot;

		x = remainder(x, 360);
		if (x < 0)
			x += 360;

		y = remainder(y, 360);
		if (y < 0)
			y += 360;

		z = remainder(z, 360);
		if (z < 0)
			z += 360;

		rot.SetXRot(ANGLE(x));
		rot.SetYRot(ANGLE(y));
		rot.SetZRot(ANGLE(z));

		return rot;
	}

	GameScriptPosition GameScript::NewPosRot(int xPos, int yPos, int zPos, float xRot, float yRot, float zRot)
	{
		GameScriptPosition posrot;

		if (xPos < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative X coordinate");
			xPos = 0;
		}
		if (zPos < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative Z coordinate");
			zPos = 0;
		}

		posrot.SetXPos(xPos);
		posrot.SetYPos(yPos);
		posrot.SetZPos(zPos);

		xRot = remainder(xRot, 360);
		if (xRot < 0)
			xRot += 360;

		yRot = remainder(yRot, 360);
		if (yRot < 0)
			yRot += 360;

		zRot = remainder(zRot, 360);
		if (zRot < 0)
			zRot += 360;

		posrot.SetXRot(ANGLE(xRot));
		posrot.SetYRot(ANGLE(yRot));
		posrot.SetZRot(ANGLE(zRot));

		return posrot;
	}

	float GameScript::CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2)
	{
		return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetYPos() - pos2.GetYPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
	}

	float GameScript::CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2)
	{
		return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
	}

	int GameScriptPosition::GetXPos()
	{
		return ref.xPos;
	}

	void GameScriptPosition::SetXPos(int x)
	{
		if (x < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative X coordinate");
			return;
		}
		ref.xPos = x;
	}

	int GameScriptPosition::GetYPos()
	{
		return ref.yPos;
	}

	void GameScriptPosition::SetYPos(int y)
	{
		ref.yPos = y;
	}

	int GameScriptPosition::GetZPos()
	{
		return ref.zPos;
	}

	void GameScriptPosition::SetZPos(int z)
	{
		if (z < 0)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Attempt to set negative Z coordinate");
			return;
		}
		ref.xPos = z;
	}

	float GameScriptPosition::GetXRot()
	{
		return TO_DEGREES(ref.xRot);
	}

	void GameScriptPosition::SetXRot(float x)
	{
		x = remainder(x, 360);
		if (x < 0)
			x += 360;
		
		ref.xRot = ANGLE(x);
	}

	float GameScriptPosition::GetYRot()
	{
		return TO_DEGREES(ref.yRot);
	}

	void GameScriptPosition::SetYRot(float y)
	{
		y = remainder(y, 360);
		if (y < 0)
			y += 360;

		ref.xRot = ANGLE(y);
	}

	float GameScriptPosition::GetZRot()
	{
		return TO_DEGREES(ref.zRot);
	}

	void GameScriptPosition::SetZRot(float z)
	{
		z = remainder(z, 360);
		if (z < 0)
			z += 360;
		
		ref.zRot = ANGLE(z);
	}

	GameScriptItem::GameScriptItem(short itemNumber) : NativeItemNumber(itemNumber) {}

	GameScriptPosition GameScriptItem::GetPosition()
	{
		return GameScriptPosition(g_Level.Items[NativeItemNumber].pos);
	}

	short GameScriptItem::GetHP()
	{
		return g_Level.Items[NativeItemNumber].hitPoints;
	}

	void GameScriptItem::SetHP(short hp)
	{
		int maxHitPoints = Objects[g_Level.Items[NativeItemNumber].objectNumber].hitPoints;

		if (hp < 0 || hp > maxHitPoints)
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Invalid HP");
			if (hp < 0)
			{
				hp = 0;
			}
			else if (hp > maxHitPoints)
			{
				hp = maxHitPoints;
			}
		}
		g_Level.Items[NativeItemNumber].hitPoints = hp;
	}

	short GameScriptItem::GetRoom()
	{
		return g_Level.Items[NativeItemNumber].roomNumber;
	}

	void GameScriptItem::SetRoom(short room)
	{
		if (room < 0 || room >= g_Level.Rooms.size())
		{
			if (WarningsAsErrors)
				throw std::runtime_error("Invalid room number");
			return;
		}
		g_Level.Items[NativeItemNumber].roomNumber = room;
	}

	void GameScriptItem::EnableItem()
	{
		auto NativeItem = &g_Level.Items[NativeItemNumber];

		if (!NativeItem->active)
		{
			if (Objects[NativeItem->objectNumber].intelligent)
			{
				if (NativeItem->status == ITEM_DEACTIVATED)
				{
					NativeItem->touchBits = 0;
					NativeItem->status = ITEM_ACTIVE;
					AddActiveItem(NativeItemNumber);
					EnableBaddieAI(NativeItemNumber, 1);
				}
				else if (NativeItem->status == ITEM_INVISIBLE)
				{
					NativeItem->touchBits = 0;
					if (EnableBaddieAI(NativeItemNumber, 0))
						NativeItem->status = ITEM_ACTIVE;
					else
						NativeItem->status = ITEM_INVISIBLE;
					AddActiveItem(NativeItemNumber);
				}
			}
			else
			{
				NativeItem->touchBits = 0;
				AddActiveItem(NativeItemNumber);
				NativeItem->status = ITEM_ACTIVE;
			}
		}
	}

	void GameScriptItem::DisableItem()
	{
		auto NativeItem = &g_Level.Items[NativeItemNumber];

		if (NativeItem->active)
		{
			if (Objects[NativeItem->objectNumber].intelligent)
			{
				if (NativeItem->status == ITEM_ACTIVE)
				{
					NativeItem->touchBits = 0;
					NativeItem->status = ITEM_DEACTIVATED;
					RemoveActiveItem(NativeItemNumber);
					DisableBaddieAI(NativeItemNumber);
				}
			}
			else
			{
				NativeItem->touchBits = 0;
				RemoveActiveItem(NativeItemNumber);
				NativeItem->status = ITEM_DEACTIVATED;
			}
		}
	}

	short GameScriptItem::GetAnimation()
	{
		short animIndex = Objects[g_Level.Items[NativeItemNumber].objectNumber].animIndex;
		return (g_Level.Items[NativeItemNumber].animNumber - animIndex);
	}

	short GameScriptItem::GetCurrentState()
	{
		return g_Level.Items[NativeItemNumber].currentAnimState;
	}

	void GameScriptItem::SetCurrentState(short state)
	{
		g_Level.Items[NativeItemNumber].currentAnimState = state;
	}

	short GameScriptItem::GetGoalState()
	{
		return g_Level.Items[NativeItemNumber].goalAnimState;
	}

	void GameScriptItem::SetGoalState(short state)
	{
		g_Level.Items[NativeItemNumber].goalAnimState = state;
	}

	short GameScriptItem::GetRequiredState()
	{
		return g_Level.Items[NativeItemNumber].requiredAnimState;
	}

	void GameScriptItem::SetRequiredState(short state)
	{
		g_Level.Items[NativeItemNumber].goalAnimState = state;
	}

	sol::object LuaVariables::GetVariable(std::string key)
	{
		if (variables.find(key) == variables.end())
			return sol::lua_nil;
		return variables[key];
	}

	void LuaVariables::SetVariable(std::string key, sol::object value)
	{
		switch (value.get_type())
		{
		case sol::type::lua_nil:
			variables.erase(key);
			break;
		case sol::type::boolean:
		case sol::type::number:
		case sol::type::string:
			variables[key] = value;
			break;
		default:
			if (WarningsAsErrors)
				throw std::runtime_error("unsupported variable type");
			break;
		}
	}
}
