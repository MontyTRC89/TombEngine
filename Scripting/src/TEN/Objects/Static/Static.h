#pragma once

#include "Objects/NamedBase.h"
#include "Game/room.h"

namespace sol {
	class state;
}

class Position;
class Rotation;
class ScriptColor;

class Static : public NamedBase<Static, MESH_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<MESH_INFO>;
	Static(MESH_INFO & id, bool temporary);
	~Static();
	Static& operator=(Static const& other) = delete;
	Static(Static const& other) = delete;

	static void Register(sol::table & parent);
	Rotation GetRot() const;
	void SetRot(Rotation const& rot);

	Position GetPos() const;
	void SetPos(Position const & pos);
	std::string GetName() const;
	void SetName(std::string const & name);
	int GetSlot() const;
	void SetSlot(int slot);
	ScriptColor GetColor() const;
	void SetColor(ScriptColor const & col);
	int GetHP() const;
	void SetHP(int hp);
private:
	MESH_INFO & m_mesh;
	bool m_temporary;
};
