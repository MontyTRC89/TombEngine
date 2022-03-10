#pragma once
#include "Color/Color.h"
#include <functional>
#include <array>

enum class DisplayStringOptions : size_t
{
	CENTER,
	OUTLINE,
	NUM_OPTIONS
};

static const std::unordered_map<std::string, DisplayStringOptions> kDisplayStringOptionNames
{
	{"CENTER", DisplayStringOptions::CENTER},
	{"SHADOW", DisplayStringOptions::OUTLINE}
};

using FlagArray = std::array<bool, static_cast<size_t>(DisplayStringOptions::NUM_OPTIONS)>;
// Used to store data used to render the string.
// This is separate from DisplayString because the lifetimes
// of the classes differ slightly.
class UserDisplayString
{
public:
	UserDisplayString(std::string const& key, int x, int y, D3DCOLOR col, FlagArray const & flags, bool translated);

private:
	UserDisplayString() = default;
	std::string m_key{};
	D3DCOLOR m_color{ 0xFFFFFFFF };
	FlagArray m_flags{};
	int m_x{ 0 };
	int m_y{ 0 };
	bool m_deleteWhenZero{ false };

	//seconds
	float m_timeRemaining{ 0.0f };
	bool m_isInfinite{ false };
	bool m_isTranslated{ false };
	friend class StringsHandler;
	friend class DisplayString;
};

using DisplayStringIDType = uintptr_t;
using SetItemCallback = std::function<bool(DisplayStringIDType, UserDisplayString const&)>;
using RemoveItemCallback = std::function<bool(DisplayStringIDType)>;
using GetItemCallback = std::function<std::optional<std::reference_wrapper<UserDisplayString>>(DisplayStringIDType)>;


class DisplayString
{
private:
	DisplayStringIDType m_id{ 0 };
public:
	DisplayString();
	~DisplayString();
	DisplayStringIDType GetID() const;
	static void Register(sol::table & parent);

	void SetPos(int x, int y);
	std::tuple<int, int> GetPos() const;

	void SetCol(ScriptColor const&);
	ScriptColor GetCol();

	void SetKey(std::string const&);
	std::string GetKey() const;

	static SetItemCallback s_setItemCallback;
	static RemoveItemCallback s_removeItemCallback;
	static GetItemCallback s_getItemCallback;

	// Creating a DisplayString requires us to add an identifier
	// to a data structure. We use callbacks so this class doesn't have
	// to know about said data structure.
	static void SetCallbacks(SetItemCallback cba, RemoveItemCallback cbr, GetItemCallback cbg)
	{
		s_setItemCallback = cba;
		s_removeItemCallback = cbr;
		s_getItemCallback = cbg;
	}

};