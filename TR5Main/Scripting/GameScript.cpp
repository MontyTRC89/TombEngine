#include "GameScript.h"

GameScript::GameScript()
{
	m_lua.open_libraries(sol::lib::base);

	m_lua.new_usertype<GameScriptSettings>("GameScriptSettings",
		"screenWidth", &GameScriptSettings::ScreenWidth,
		"screenHeight", &GameScriptSettings::ScreenHeight,
		"windowTitle", &GameScriptSettings::WindowTitle,
		"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows
		);
	
	m_lua["settings"] = &m_settings;
	
	m_strings.resize(10000);
	m_lua["strings"] = &m_strings;
}

GameScript::~GameScript()
{

}

string GameScript::loadScriptFromFile(char* luaFilename)
{
	ifstream ifs(luaFilename, ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);
	 
	vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return string(bytes.data(), fileSize);
}
 
bool GameScript::LoadGameStrings(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);

	return true;
}

bool GameScript::LoadGameSettings(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);

	return true;
}

bool GameScript::ExecuteScript(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);
	 
	return true;
}

char* GameScript::GetString(__int32 id)
{
	return ((char*)m_strings[id - 1].c_str());
}

GameScriptSettings* GameScript::GetSettings()
{
	return &m_settings;
}

GameScript* g_Script;