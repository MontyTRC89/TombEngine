#include "framework.h"
#include "Objects/Camera/CameraObject.h"
#include "Game/camera.h"

#include "ReservedScriptNames.h"
#include "ScriptAssert.h"
#include "ScriptUtil.h"
#include "Specific/LevelCameraInfo.h"
#include "Specific/level.h"
#include "Vec3/Vec3.h"

/***
Basic cameras that can point at Lara or at a CAMERA_TARGET.

@tenclass Objects.Camera
@pragma nostrip
*/

static auto index_error = index_error_maker(CameraObject, ScriptReserved_Camera);
static auto newindex_error = newindex_error_maker(CameraObject, ScriptReserved_Camera);

CameraObject::CameraObject(LevelCameraInfo & ref) : m_camera{ref}
{};

void CameraObject::Register(sol::table & parent)
{
	parent.new_usertype<CameraObject>(ScriptReserved_Camera,
		sol::no_constructor, // ability to spawn new ones could be added later
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

		/// Get the camera's position
		// @function Camera:GetPosition
		// @treturn Vec3 a copy of the camera's position
		ScriptReserved_GetPosition, &CameraObject::GetPos,

		/// Set the camera's position
		// @function Camera:SetPosition
		// @tparam Vec3 position the new position of the camera 
		ScriptReserved_SetPosition, &CameraObject::SetPos,

		/// Get the camera's unique string identifier
		// @function Camera:GetName
		// @treturn string the camera's name
		ScriptReserved_GetName, &CameraObject::GetName,

		/// Set the camera's name (its unique string identifier)
		// @function Camera:SetName
		// @tparam string name The camera's new name
		ScriptReserved_SetName, &CameraObject::SetName,

		/// Get the current room of the camera
		// @function CameraObject:GetRoom
		// @treturn Room current room of the camera
		ScriptReserved_GetRoom, &CameraObject::GetRoom,

		/// Get the current room number of the camera
		// @function Camera:GetRoomNumber
		// @treturn int number representing the current room of the camera
		ScriptReserved_GetRoomNumber, &CameraObject::GetRoomNumber,

		/// Set room of camera 
		// This is used in conjunction with SetPosition to teleport the camera to a new room.
		// @function Camera:SetRoomNumber
		// @tparam int ID the ID of the new room 
		ScriptReserved_SetRoomNumber, &CameraObject::SetRoomNumber,

		/// Active the camera during that frame.
		// @function Camera:PlayCamera
		// @tparam[opt] Moveable target If you put a moveable, the camera will look at it. Otherwise, it will look at Lara.
		ScriptReserved_PlayCamera, &CameraObject::PlayCamera
		);
}

Vec3 CameraObject::GetPos() const
{
	return Vec3{ m_camera.Position };
}

void CameraObject::SetPos(Vec3 const& pos)
{
	m_camera.Position = Vector3i(pos.x, pos.y, pos.z);
	RefreshFixedCamera(m_camera.Index);
}

std::string CameraObject::GetName() const
{
	return m_camera.Name;
}

void CameraObject::SetName(std::string const & id) 
{
	if (!ScriptAssert(!id.empty(), "Name cannot be blank. Not setting name."))
	{
		return;
	}

	if (s_callbackSetName(id, m_camera))
	{
		// remove the old name if we have one
		s_callbackRemoveName(m_camera.Name);
		m_camera.Name = id;
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does a camera with this name already exist?", id);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);
	}
}

std::unique_ptr<Room> CameraObject::GetRoom() const
{
	return std::make_unique<Room>(g_Level.Rooms[m_camera.RoomNumber]);
}

int CameraObject::GetRoomNumber() const
{
	return m_camera.RoomNumber;
}

void CameraObject::SetRoomNumber(short room)
{	
	const size_t nRooms = g_Level.Rooms.size();
	if (room < 0 || static_cast<size_t>(room) >= nRooms)
	{
		ScriptAssertF(false, "Invalid room number: {}. Value must be in range [0, {})", room, nRooms);
		TENLog("Room number will not be set", LogLevel::Warning, LogConfig::All);
		return;
	}

	m_camera.RoomNumber = room;
}

void CameraObject::PlayCamera(sol::optional<Moveable&> TargetObj)
{
	Camera.number = m_camera.Index;
	Camera.type = CameraType::Fixed;

	if (TargetObj.has_value()) //Otherwise, it will point to Lara by default.
		Camera.item = &g_Level.Items[TargetObj.value().GetIndex()];
}

