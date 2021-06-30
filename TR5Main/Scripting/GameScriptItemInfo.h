#pragma once

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class GameScriptPosition;
class GameScriptRotation;

class GameScriptItemInfo
{
private:
	short m_num;
public:
	GameScriptItemInfo(short num);
	~GameScriptItemInfo();
	GameScriptItemInfo& operator=(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo const& other) = delete;
	static void Register(sol::state *);
	static std::unique_ptr<GameScriptItemInfo> CreateEmpty();
	static std::unique_ptr<GameScriptItemInfo> Create(
		GameScriptPosition pos,
		GameScriptRotation aRot,
		short aCurrentAnim,
		short aRequiredAnim,
		short aHp,
		short aOcb,
		sol::as_table_t<std::array<short, 8>> aFlags,
		byte aAiBits,
		short aStatus,
		bool aActive,
		bool aHitStatus
		);
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
};
