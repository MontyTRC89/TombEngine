#include "framework.h"

#include "ScriptAssert.h"
#include "Objects/Camera/CameraObject.h"
#include "Vec3/Vec3.h"
#include "ScriptUtil.h"
#include "ReservedScriptNames.h"
#include "Specific/level.h"

/***
Basic cameras that can point at Lara or at a CAMERA_TARGET.

@tenclass Objects.Camera
@pragma nostrip
*/

static auto index_error = index_error_maker(Camera, ScriptReserved_Camera);
static auto newindex_error = newindex_error_maker(Camera, ScriptReserved_Camera);

Camera::Camera(LEVEL_CAMERA_INFO & ref) : m_camera{ref}
{};

void Camera::Register(sol::table & parent)
{
	parent.new_usertype<Camera>(ScriptReserved_Camera,
		sol::no_constructor, // ability to spawn new ones could be added later
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Get the camera's position
		// @function Camera:GetPosition
		// @treturn Vec3 a copy of the camera's position
		ScriptReserved_GetPosition, &Camera::GetPos,

		/// Set the camera's position
		// @function Camera:SetPosition
		// @tparam Vec3 position the new position of the camera 
		ScriptReserved_SetPosition, &Camera::SetPos,

		/// Get the camera's unique string identifier
		// @function Camera:GetName
		// @treturn string the camera's name
		ScriptReserved_GetName, &Camera::GetName,

		/// Set the camera's name (its unique string identifier)
		// @function Camera:SetName
		// @tparam string name The camera's new name
		ScriptReserved_SetName, &Camera::SetName,

		/// Get the current room of the camera
		// @function Camera:GetRoom
		// @treturn int number representing the current room of the camera
		ScriptReserved_GetRoom, &Camera::GetRoom,

		/// Set room of camera 
		// This is used in conjunction with SetPosition to teleport the camera to a new room.
		// @function Camera:SetRoom
		// @tparam int ID the ID of the new room 
		ScriptReserved_SetRoom, &Camera::SetRoom
		);
}

Vec3 Camera::GetPos() const
{
	return Vec3{ m_camera.x, m_camera.y, m_camera.z };
}

void Camera::SetPos(Vec3 const& pos)
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
	if (!ScriptAssert(!id.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(id, m_camera))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_camera.luaName);
		m_camera.luaName = id;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does a camera with this name already exist?", id);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

short Camera::GetRoom() const
{
	return m_camera.roomNumber;
}

void Camera::SetRoom(short room)
{	
	const size_t nRooms = g_Level.Rooms.size();
	if (room < 0 || static_cast<size_t>(room) >= nRooms)
	{
		ScriptAssertF(false, "Invalid room number: {}. Value must be in range [0, {})", room, nRooms);
		TENLog("Room number will not be set", LogLevel::Warning, LogConfig::All);
		return;
	}

	m_camera.roomNumber = room;
}
