#pragma once

#include "Game/room.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol { class state; }
enum class ReverbType;
class ScriptColor;
class Vec3; 

//namespace TEN::Scripting
//{
	class Room : public NamedBase<Room, ROOM_INFO&>
	{
	private:
		// Members

		ROOM_INFO& _room;

	public:
		using IdentifierType = std::reference_wrapper<ROOM_INFO>;

		static void Register(sol::table& parent);

		// Constructors

		Room(ROOM_INFO& room);
		Room(const Room& room) = delete;

		// Destructors

		~Room() = default;

		// Getters

		int			GetRoomNumber() const;
		std::string GetName() const;
		ScriptColor GetColor() const;
		ReverbType	GetReverbType() const;

		// Setters

		void SetName(const std::string& name);
		void SetReverbType(ReverbType reverbType);
		void SetFlag(RoomEnvFlags flag, bool value);

		// Inquirers

		bool IsTagPresent(const std::string& tag) const;
		bool GetActive() const; // TODO: Rename to IsActive().
		bool GetFlag(RoomEnvFlags flag) const; // TODO: Rename to HasFlag().

		// Operators

		Room& operator =(const Room& room) = delete;
	};
//}
