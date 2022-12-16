#pragma once

#include "Objects/NamedBase.h"
#include "Game/room.h"

namespace sol {
	class state;
}

class Vec3; 
class ScriptColor;
enum class ReverbType;

class Room : public NamedBase<Room, ROOM_INFO&>
{
public:
	using IdentifierType = std::reference_wrapper<ROOM_INFO>;
	Room(ROOM_INFO& volume);
	~Room() = default;

	Room& operator=(Room const& other) = delete;
	Room(Room const& other) = delete;

	static void Register(sol::table& parent);

	[[nodiscard]] bool GetActive() const;

	[[nodiscard]] std::string GetName() const;
	void SetName(std::string const& name);

	[[nodiscard]] bool GetFlag(RoomEnvFlags const& flag) const;
	void SetFlag(RoomEnvFlags const& flag, bool const& value);

	[[nodiscard]] ReverbType GetReverbType() const;
	void SetReverbType(ReverbType const& reverbType);

	[[nodiscard]] bool IsTagPresent(std::string const& tag) const;

private:
	ROOM_INFO& m_room;
};
