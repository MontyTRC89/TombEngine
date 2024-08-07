#pragma once
#include "Game/room.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol { class state; }
enum class ReverbType;
class ScriptColor;
class Vec3; 

class Room : public NamedBase<Room, RoomData&>
{
public:
	using IdentifierType = std::reference_wrapper<RoomData>;
	Room(RoomData& room);
	~Room() = default;

	Room& operator =(const Room& other) = delete;
	Room(const Room& other) = delete;

	static void Register(sol::table& parent);

	[[nodiscard]] bool GetActive() const;
	[[nodiscard]] ScriptColor GetColor() const;

	[[nodiscard]] std::string GetName() const;
	void SetName(const std::string& name);

	[[nodiscard]] bool GetFlag(RoomEnvFlags flag) const;
	void SetFlag(RoomEnvFlags flag, bool value);

	[[nodiscard]] ReverbType GetReverbType() const;
	void SetReverbType(ReverbType reverbType);

	[[nodiscard]] bool IsTagPresent(const std::string& tag) const;

private:
	RoomData& m_room;
};
