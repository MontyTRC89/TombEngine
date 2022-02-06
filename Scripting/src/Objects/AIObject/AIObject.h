#pragma once

#include "GameScriptNamedBase.h"
#include "Specific/level.h"

namespace sol {
	class state;
}
class GameScriptPosition;

class AIObject : public GameScriptNamedBase<AIObject, AI_OBJECT&>
{
public:
	AIObject(AI_OBJECT& ref, bool temp);
	~AIObject();

	AIObject& operator=(AIObject const& other) = delete;
	AIObject(AIObject const& other) = delete;

	static void Register(sol::table & parent);

	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	short GetRoom() const;
	void SetRoom(short Room);

	std::string GetName() const;
	void SetName(std::string const &);

	GAME_OBJECT_ID GetObjID() const;
	void SetObjID(GAME_OBJECT_ID);

	short GetTriggerFlags() const;
	void SetTriggerFlags(short);

	short GetFlags() const;
	void SetFlags(short);

	short GetYRot() const;
	void SetYRot(short);

	short GetBoxNumber() const;
	void SetBoxNumber(short);

private:
	AI_OBJECT & m_aiObject;
	bool m_temporary;
};

