#pragma once
#include <array>
#include <functional>

#include "Scripting/Internal/TEN/Color/Color.h"

enum class DisplayStringOptions
{
	Center,
	Outline,
	Right,
	Blink,

	Count
};

static const std::unordered_map<std::string, DisplayStringOptions> DISPLAY_STRING_OPTION_NAMES
{
	{ "CENTER", DisplayStringOptions::Center },
	{ "SHADOW", DisplayStringOptions::Outline },
	{ "RIGHT", DisplayStringOptions::Right },
	{ "BLINK", DisplayStringOptions::Blink }
};

using FlagArray = std::array<bool, (int)DisplayStringOptions::Count>;
// Used to store data used to render the string.
// This is separate from DisplayString because the lifetimes of the classes differ slightly.

class UserDisplayString
{
public:
	UserDisplayString(const std::string& key, int x, int y, D3DCOLOR color, const FlagArray& flags, bool translated);

private:
	UserDisplayString() = default;
	std::string m_key{};
	D3DCOLOR m_color{ 0xFFFFFFFF };
	FlagArray m_flags{};
	int m_x{ 0 };
	int m_y{ 0 };
	bool m_deleteWhenZero{ false };

	// Seconds
	float m_timeRemaining{ 0.0f };
	bool m_isInfinite{ false };
	bool m_isTranslated{ false };
	friend class StringsHandler;
	friend class DisplayString;
};

using DisplayStringIDType = uintptr_t;
using SetItemCallback = std::function<bool(DisplayStringIDType, const UserDisplayString&)>;
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
	static void Register(sol::table& parent);

	void SetPos(int x, int y);
	std::tuple<int, int> GetPos() const;

	void SetColor(const ScriptColor&);
	ScriptColor GetColor();

	void SetKey(const std::string&);
	std::string GetKey() const;

	void SetFlags(const sol::table&);

	void SetTranslated(bool isTranslated);

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
