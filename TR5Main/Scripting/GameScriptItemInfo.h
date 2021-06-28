#pragma once

namespace sol {
	class state;
	template <typename T> class as_table_t;
}


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
	static std::unique_ptr<GameScriptItemInfo> Create(
		short hp,
		short currentAnim,
		short requiredAnimState,
		sol::as_table_t<std::array<int, 3>> pos,
		sol::as_table_t<std::array<short, 3>> rot,
		sol::as_table_t<std::array<short, 8>> itemFlags,
		short ocb,
		byte aiBits,
		short status,
		bool active,
		bool hitStatus);
};
