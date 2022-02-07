#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "Objects/Camera/Camera.h"
#include "Position/Position.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"

/***
Camera info

@tenclass Objects.Camera
@pragma nostrip
*/

static auto index_error = index_error_maker(Camera, ScriptReserved_Camera);
static auto newindex_error = newindex_error_maker(Camera, ScriptReserved_Camera);

Camera::Camera(LEVEL_CAMERA_INFO & ref, bool temp) : m_camera{ref}, m_temporary{ temp }
{};

Camera::~Camera() {
	if (m_temporary)
	{
		s_callbackRemoveName(m_camera.luaName);
	}
}

void Camera::Register(sol::table & parent)
{
	parent.new_usertype<Camera>(ScriptReserved_Camera,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// (@{Position}) position in level
		// @mem pos
		"pos", sol::property(&Camera::GetPos, &Camera::SetPos),

		/// (string) unique string identifier.
		// e.g. "flyby\_start" or "big\_door\_hint"
		// @mem name
		"name", sol::property(&Camera::GetName, &Camera::SetName),

		/// (string) room number
		// @mem room
		"room", sol::property(&Camera::GetRoom, &Camera::SetRoom)
		);
}

Position Camera::GetPos() const
{
	return Position{ m_camera.x, m_camera.y, m_camera.z };
}

void Camera::SetPos(Position const& pos)
{
	m_camera.x = pos.x;
	m_camera.y = pos.y;
	m_camera.z = pos.z;
}

std::string Camera::GetName() const
{
	return m_camera.luaName;
}

void Camera::SetName(std::string const & id) 
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

short Camera::GetRoom() const
{
	return m_camera.roomNumber;
}

void Camera::SetRoom(short room)
{	
	m_camera.roomNumber = room;
}

