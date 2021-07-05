#pragma once

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class GameScriptPosition;
class GameScriptRotation;
struct ITEM_INFO;

class GameScriptItemInfo
{
private:
	ITEM_INFO* m_item;
	short m_num;
public:
	GameScriptItemInfo(short num);
	~GameScriptItemInfo();
	GameScriptItemInfo& operator=(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo && other) noexcept;

	static void Register(sol::state *);
	static std::unique_ptr<GameScriptItemInfo> CreateEmpty();
	static std::unique_ptr<GameScriptItemInfo> Create(
		std::string Name,
		GameScriptPosition pos,
		GameScriptRotation aRot,
		short room,
		short aCurrentAnim,
		short aRequiredAnim,
		short goalAnim,
		short aHp,
		short aOcb,
		sol::as_table_t<std::array<short, 8>> aFlags,
		byte aAiBits,
		short aStatus,
		bool aActive,
		bool aHitStatus
		);
	std::string GetName() const;
	void SetName(std::string const &);
	GameScriptPosition GetPos() const;
	GameScriptRotation GetRot() const;
	short GetCurrentAnim() const;
	short GetRequiredAnim() const;
	short GetHP() const;
	short GetOCB() const;
	sol::as_table_t<std::array<short, 8>>  GetItemFlags() const;
	byte GetAIBits() const;
	short GetStatus() const;
	bool GetHitStatus() const;
	bool GetActive() const;
	short GetRoom() const;
	short GetGoalAnim() const;
	void EnableItem();
	void DisableItem();

	void Init();

	void SetPos(GameScriptPosition const& pos);
	void SetRot(GameScriptRotation const& rot);
	void SetCurrentAnim(short anim);
	void SetRequiredAnim(short anim);
	void SetHP(short hp);
	void SetOCB(short ocb);
	void SetItemFlags(sol::as_table_t<std::array<short, 8>>  const & arr);
	void SetAIBits(byte bits);
	void SetStatus(short status);
	void SetHitStatus(bool hitStatus);
	void SetActive(bool active);
	void SetRoom(short Room);
	void SetGoalAnim(short state);
};
