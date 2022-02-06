#pragma once

#include "GameScriptNamedBase.h"
#include "Game/room.h"

namespace sol {
	class state;
}

class GameScriptPosition;
class GameScriptColor;

class Static : public GameScriptNamedBase<Static, MESH_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<MESH_INFO>;
	Static(MESH_INFO & id, bool temporary);
	~Static();
	Static& operator=(Static const& other) = delete;
	Static(Static const& other) = delete;

	static void Register(sol::state *, sol::table & parent);

	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const & pos);
	int GetRot() const;
	void SetRot(int yRot);
	std::string GetName() const;
	void SetName(std::string const & name);
	int GetStaticNumber() const;
	void SetStaticNumber(int staticNumber);
	GameScriptColor GetColor() const;
	void SetColor(GameScriptColor const & col);
	int GetHP() const;
	void SetHP(int hp);
private:
	MESH_INFO & m_mesh;
	bool m_temporary;
};
