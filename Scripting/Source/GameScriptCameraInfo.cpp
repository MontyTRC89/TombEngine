#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "GameScriptCameraInfo.h"
#include "GameScriptPosition.h"
#include "ScriptUtil.h"
/***
Camera info

@entityclass CameraInfo
@pragma nostrip
*/

static constexpr auto LUA_CLASS_NAME{ "CameraInfo" };

static auto index_error = index_error_maker(GameScriptCameraInfo, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptCameraInfo, LUA_CLASS_NAME);

GameScriptCameraInfo::GameScriptCameraInfo(LEVEL_CAMERA_INFO & ref, bool temp) : m_camera{ref}, m_temporary{ temp }
{};

GameScriptCameraInfo::~GameScriptCameraInfo() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_camera.luaName);
	}
}

void GameScriptCameraInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptCameraInfo>(LUA_CLASS_NAME,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&GameScriptCameraInfo::GetPos, &GameScriptCameraInfo::SetPos),

		/// (string) unique string identifier.
		// e.g. "flyby\_start" or "big\_door\_hint"
		// @mem name
		"name", sol::property(&GameScriptCameraInfo::GetName, &GameScriptCameraInfo::SetName),

		/// (string) room number
		// @mem room
		"room", sol::property(&GameScriptCameraInfo::GetRoom, &GameScriptCameraInfo::SetRoom)
		);
}

GameScriptPosition GameScriptCameraInfo::GetPos() const
{
	return GameScriptPosition{ m_camera.x, m_camera.y, m_camera.z };
}

void GameScriptCameraInfo::SetPos(GameScriptPosition const& pos)
{
	m_camera.x = pos.x;
	m_camera.y = pos.y;
	m_camera.z = pos.z;
}

std::string GameScriptCameraInfo::GetName() const
{
	return m_camera.luaName;
}

void GameScriptCameraInfo::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_camera.luaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_camera.luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_camera);
}

short GameScriptCameraInfo::GetRoom() const
{
	return m_camera.roomNumber;
}

void GameScriptCameraInfo::SetRoom(short room)
{	
	m_camera.roomNumber = room;
}

