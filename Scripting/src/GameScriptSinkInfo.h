#pragma once

#include "GameScriptNamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class GameScriptPosition;

class GameScriptSinkInfo : public GameScriptNamedBase<GameScriptSinkInfo, SINK_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<SINK_INFO>;
	GameScriptSinkInfo(SINK_INFO& ref, bool temp);
	~GameScriptSinkInfo();
	GameScriptSinkInfo& operator=(GameScriptSinkInfo const& other) = delete;
	GameScriptSinkInfo(GameScriptSinkInfo const& other) = delete;

	static void Register(sol::state *);
	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	int GetStrength() const;
	void SetStrength(int strength);

	int GetBoxIndex() const;
	void SetBoxIndex(int Room);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SINK_INFO & m_sink;
	bool m_temporary;
};
