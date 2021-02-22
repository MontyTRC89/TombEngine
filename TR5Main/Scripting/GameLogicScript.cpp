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

		// Settings type
		m_lua.new_usertype<GameScriptSettings>("GameScriptSettings",
			"screenWidth", &GameScriptSettings::ScreenWidth,
			"screenHeight", &GameScriptSettings::ScreenHeight,
			"windowTitle", &GameScriptSettings::WindowTitle,
			"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows,
			"windowed", &GameScriptSettings::Windowed,
			"enableWaterCaustics", &GameScriptSettings::EnableWaterCaustics,
			"drawingDistance", &GameScriptSettings::DrawingDistance,
			"showRendererSteps", &GameScriptSettings::ShowRendererSteps,
			"showDebugInfo", &GameScriptSettings::ShowDebugInfo
			);

		// Layer type
		m_lua.new_usertype<GameScriptSkyLayer>("SkyLayer",
			sol::constructors<GameScriptSkyLayer(byte, byte, byte, short)>(),
			"r", &GameScriptSkyLayer::R,
			"g", &GameScriptSkyLayer::G,
			"b", &GameScriptSkyLayer::B,
			"speed", &GameScriptSkyLayer::CloudSpeed
			);

		// Mirror type
		m_lua.new_usertype<GameScriptMirror>("Mirror",
			sol::constructors<GameScriptMirror(short, int, int, int, int)>(),
			"room", &GameScriptMirror::Room,
			"startX", &GameScriptMirror::StartX,
			"endX", &GameScriptMirror::EndX,
			"startZ", &GameScriptMirror::StartZ,
			"endZ", &GameScriptMirror::EndZ
			);

		// Fog type
		m_lua.new_usertype<GameScriptFog>("Fog",
			sol::constructors<GameScriptFog(byte, byte, byte)>(),
			"r", &GameScriptFog::R,
			"g", &GameScriptFog::G,
			"b", &GameScriptFog::B
			);

		// Level type
		/*m_lua.new_usertype<GameScriptLevel>("Level",
			sol::constructors<GameScriptLevel()>(),
			"name", &GameScriptLevel::Name,
			"script", &GameScriptLevel::ScriptFileName,
			"fileName", &GameScriptLevel::FileName,
			"loadScreen", &GameScriptLevel::LoadScreenFileName,
			"soundTrack", &GameScriptLevel::Soundtrack,
			"layer1", &GameScriptLevel::Layer1,
			"layer2", &GameScriptLevel::Layer2,
			"fog", &GameScriptLevel::Fog,
			"horizon", &GameScriptLevel::Horizon,
			"colAddHorizon", &GameScriptLevel::ColAddHorizon,
			"storm", &GameScriptLevel::Storm,
			"background", &GameScriptLevel::Background,
			"rain", &GameScriptLevel::Rain,
			"snow", &GameScriptLevel::Snow,
			"laraType", &GameScriptLevel::LaraType,
			"rumble", &GameScriptLevel::Rumble,
			"resetHub", &GameScriptLevel::ResetHub,
			"mirror", &GameScriptLevel::Mirror
			);*/

		//m_lua["Gameflow"] = this;

		// Add constants
		//ExecuteScript("Scripts\\Constants.lua");

		m_lua.new_enum<GAME_OBJECT_ID>("Object", {
			{"LARA", ID_LARA}
			});

		// Add the item type
		m_lua.new_usertype<GameScriptPosition>("Position",
			"X", sol::property(&GameScriptPosition::GetXPos, &GameScriptPosition::SetXPos),
			"Y", sol::property(&GameScriptPosition::GetYPos, &GameScriptPosition::SetYPos),
			"Z", sol::property(&GameScriptPosition::GetZPos, &GameScriptPosition::SetZPos),
			"new", sol::no_constructor
			);
		
		m_lua.new_usertype<GameScriptPosition>("Rotation",
			"X", sol::property(&GameScriptRotation::GetXRot, &GameScriptRotation::SetXRot),
			"Y", sol::property(&GameScriptRotation::GetYRot, &GameScriptRotation::SetYRot),
			"Z", sol::property(&GameScriptRotation::GetZRot, &GameScriptRotation::SetZRot),
			"new", sol::no_constructor
			);
	
		m_lua.new_usertype<GameScriptItem>("Item",
			"Pos", sol::property(&GameScriptItem::GetPosition),
			"Rot", sol::property(&GameScriptItem::GetRotation),
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
		m_lua.set_function("NewPosition", &NewPosition);
		m_lua.set_function("NewSectorPosition", &NewSectorPosition);
		m_lua.set_function("NewRotation", &NewRotation);
		m_lua.set_function("CalculateDistance", &CalculateDistance);
		m_lua.set_function("CalculateHorizontalDistance", &CalculateHorizontalDistance);

		// Add global variables and namespaces
		//m_lua["TR"] = this;
	}

	bool GameScript::LoadGameStrings(char* luaFilename)
	{
		std::string script = g_GameFlow->loadScriptFromFile(luaFilename);
		m_lua.script(script);

		return true;
	}

	bool GameScript::LoadGameSettings(char* luaFilename)
	{
		std::string script = g_GameFlow->loadScriptFromFile(luaFilename);
		m_lua.script(script);

		return true;
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

	float GameScriptRotation::GetXRot()
	{
		return TO_DEGREES(ref.xRot);
	}

	void GameScriptRotation::SetXRot(float x)
	{
		x = remainder(x, 360);
		if (x < 0)
			x += 360;
		
		ref.xRot = ANGLE(x);
	}

	float GameScriptRotation::GetYRot()
	{
		return TO_DEGREES(ref.yRot);
	}

	void GameScriptRotation::SetYRot(float y)
	{
		y = remainder(y, 360);
		if (y < 0)
			y += 360;

		ref.xRot = ANGLE(y);
	}

	float GameScriptRotation::GetZRot()
	{
		return TO_DEGREES(ref.zRot);
	}

	void GameScriptRotation::SetZRot(float z)
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

	GameScriptRotation GameScriptItem::GetRotation()
	{
		return GameScriptRotation(g_Level.Items[NativeItemNumber].pos);
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

	GameScriptPosition NewPosition(int x, int y, int z)
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

	GameScriptPosition NewSectorPosition(int x, int y, int z)
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

	GameScriptRotation NewRotation(float x, float y, float z)
	{
		GameScriptRotation rot;

		rot.SetXRot(ANGLE(x));
		rot.SetYRot(ANGLE(y));
		rot.SetZRot(ANGLE(z));

		return rot;
	}

	float CalculateDistance(GameScriptPosition& pos1, GameScriptPosition& pos2)
	{
		return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetYPos() - pos2.GetYPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
	}

	float CalculateHorizontalDistance(GameScriptPosition& pos1, GameScriptPosition& pos2)
	{
		return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
	}
}
