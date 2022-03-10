#pragma once

#include "Objects/NamedBase.h"
#include "Specific/level.h"

namespace sol {
	class state;
}
class Position;

class AIObject : public NamedBase<AIObject, AI_OBJECT&>
{
public:

	using IdentifierType = std::reference_wrapper<AI_OBJECT>;
	AIObject(AI_OBJECT& ref, bool temp);
	~AIObject();

	AIObject& operator=(AIObject const& other) = delete;
	AIObject(AIObject const& other) = delete;

	static void Register(sol::table & parent);

	Position GetPos() const;
	void SetPos(Position const& pos);

	short GetRoom() const;
	void SetRoom(short Room);

	std::string GetName() const;
	void SetName(std::string const &);

	GAME_OBJECT_ID GetObjectID() const;
	void SetObjectID(GAME_OBJECT_ID);

	short GetYRot() const;
	void SetYRot(short);

private:
	AI_OBJECT & m_aiObject;
	bool m_temporary;
};

