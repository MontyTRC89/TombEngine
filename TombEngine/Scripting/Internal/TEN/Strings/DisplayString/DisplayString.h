#pragma once
#include <array>
#include <functional>

#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

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
	UserDisplayString(const std::string& key, const Vec2& pos, float scale, D3DCOLOR color, const FlagArray& flags, bool isTranslated);

private:
	UserDisplayString() = default;

	std::string m_key = {};
	Vec2 Position = Vec2(0, 0);
	D3DCOLOR m_color = 0xFFFFFFFF;
	FlagArray m_flags = {};
	float m_scale = 1.0f;
	bool m_deleteWhenZero = false;

	// Seconds
	float m_timeRemaining = 0.0f;
	bool m_isInfinite = false;
	bool m_isTranslated = false;

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
	DisplayStringIDType m_id = 0;

public:
	static void Register(sol::table& parent);

	DisplayString();
	~DisplayString();

	DisplayStringIDType GetID() const;
	std::string			GetKey() const;
	Vec2				GetPos() const;
	float				GetScale() const;
	ScriptColor			GetColor() const;

	void SetKey(const std::string&);
	void SetPosition(const Vec2& pos);
	void SetScale(float scale);
	void SetColor(const ScriptColor&);
	void SetTranslated(bool isTranslated);
	void SetFlags(const sol::table&);

	static SetItemCallback	  s_setItemCallback;
	static RemoveItemCallback s_removeItemCallback;
	static GetItemCallback	  s_getItemCallback;

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
