#pragma once

#include "GameScriptNamedBase.h"
#include "room.h"

namespace sol {
	class state;
}

class GameScriptPosition;
class GameScriptColor;

class GameScriptMeshInfo : public GameScriptNamedBase<GameScriptMeshInfo, MESH_INFO &>
{
public:
	GameScriptMeshInfo& operator=(GameScriptMeshInfo const& other) = delete;
	GameScriptMeshInfo(GameScriptMeshInfo const& other) = delete;

	static void Register(sol::state *);

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
};
