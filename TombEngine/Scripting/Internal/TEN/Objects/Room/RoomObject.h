#pragma once
#include "Game/room.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol
{
	class state;
}

class ScriptColor;
class Vec3; 
enum class ReverbType;

class Room : public NamedBase<Room, ROOM_INFO&>
{
public:
	using IdentifierType = std::reference_wrapper<ROOM_INFO>;
	Room(ROOM_INFO& volume);
	~Room() = default;

	Room& operator =(const Room& other) = delete;
	Room(const Room& other) = delete;

	static void Register(sol::table& parent);

	[[nodiscard]] bool GetActive() const;

	[[nodiscard]] std::string GetName() const;
	void SetName(const std::string& name);

	[[nodiscard]] bool GetFlag(RoomEnvFlags flag) const;
	void SetFlag(RoomEnvFlags flag, bool value);

	[[nodiscard]] ReverbType GetReverbType() const;
	void SetReverbType(ReverbType reverbType);

	[[nodiscard]] bool IsTagPresent(const std::string& tag) const;

private:
	ROOM_INFO& m_room;
};
