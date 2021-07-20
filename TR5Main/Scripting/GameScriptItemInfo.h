#pragma once
#include <functional>
namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class GameScriptPosition;
class GameScriptRotation;
struct ITEM_INFO;
enum GAME_OBJECT_ID : short;

using callbackSetName = std::function<bool(std::string const&, short itemID)>;
using callbackRemoveName = std::function<bool(std::string const&)>;

class GameScriptItemInfo
{
public:
	GameScriptItemInfo(short num, bool temporary);
	~GameScriptItemInfo();
	GameScriptItemInfo& operator=(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo && other) noexcept;

	static void Register(sol::state *);

	static void SetNameCallbacks(callbackSetName, callbackRemoveName);

	GAME_OBJECT_ID GetObjectID() const;
	void SetObjectID(GAME_OBJECT_ID id);

	std::string GetName() const;
	void SetName(std::string const &);

	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	GameScriptRotation GetRot() const;
	void SetRot(GameScriptRotation const& rot);

	short GetCurrentAnimState() const;
	void SetCurrentAnimState(short animState);

	short GetAnimNumber() const;
	void SetAnimNumber(short animNumber);

	short GetFrameNumber() const;
	void SetFrameNumber(short frameNumber);

	short GetRequiredAnimState() const;
	void SetRequiredAnimState(short animState);

	short GetGoalAnimState() const;
	void SetGoalAnimState(short animState);

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
	static callbackSetName s_callbackSetName;
	static callbackRemoveName s_callbackRemoveName;

};
