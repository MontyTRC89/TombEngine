#pragma once

#include "GameScriptNamedBase.h"

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class GameScriptPosition;
class GameScriptRotation;
struct ITEM_INFO;
enum GAME_OBJECT_ID : short;


class Moveable : public GameScriptNamedBase<Moveable, short>
{
public:
	using IdentifierType = short;
	Moveable(short num, bool temporary);
	~Moveable();
	Moveable& operator=(Moveable const& other) = delete;
	Moveable(Moveable const& other) = delete;
	Moveable(Moveable && other) noexcept;

	static void Register(sol::table & parent);

	GAME_OBJECT_ID GetObjectID() const;
	void SetObjectID(GAME_OBJECT_ID id);

	std::string GetName() const;
	void SetName(std::string const &);

	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	GameScriptRotation GetRot() const;
	void SetRot(GameScriptRotation const& rot);

	int GetCurrentAnimState() const;
	void SetCurrentAnimState(int animState);

	int GetAnimNumber() const;
	void SetAnimNumber(int animNumber);

	int GetFrameNumber() const;
	void SetFrameNumber(int frameNumber);

	int GetRequiredAnimState() const;
	void SetRequiredAnimState(short animState);

	int GetGoalAnimState() const;
	void SetGoalAnimState(int animState);

	short GetHP() const;
	void SetHP(short hp);

	short GetOCB() const;
	void SetOCB(short ocb);

	sol::as_table_t<std::array<short, 8>>  GetItemFlags() const;
	void SetItemFlags(sol::as_table_t<std::array<short, 8>>  const & arr);

	byte GetAIBits() const;
	void SetAIBits(byte bits);

	short GetStatus() const;
	void SetStatus(short status);

	bool GetHitStatus() const;
	void SetHitStatus(bool status);

	bool GetActive() const;
	void SetActive(bool active);

	short GetRoom() const;
	void SetRoom(short Room);

	void EnableItem();
	void DisableItem();

	void Init();

private:
	ITEM_INFO* m_item;
	short m_num;
	bool m_initialised;
	bool m_temporary;

};
