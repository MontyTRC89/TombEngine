#include "frameworkandsol.h"
#include "GameScriptDisplayString.h"
#include "ScriptAssert.h"
#include "ReservedScriptNames.h"

/*** A string appearing on the screen.
Can be used for subtitles and "2001, somewhere in Egypt"-style messages.

Uses screen-space coordinates, with x values specifying the number of pixels from the left of the window,
and y values specifying the number of pixels from the top of the window.

Since different players will have different resolutions, you should work in terms of percentages where possible,
and use @{Level-specific.ScreenToPercent|ScreenToPercent} and @{Level-specific.PercentToScreen|PercentToScreen}
when you need to use screen-space coordinates.

@miscclass DisplayString
@pragma nostrip
*/

UserDisplayString::UserDisplayString(std::string const& key, int x, int y, D3DCOLOR col, FlagArray const & flags, bool translated) :
	m_key{ key },
	m_x{ x },
	m_y{ y },
	m_color{ col },
	m_flags{ flags },
	m_isTranslated{ translated }
{
}

GameScriptDisplayString::GameScriptDisplayString() {
	// We don't ever dereference this pointer; it's just
	// a handy way to get a unique key for a hash map.

	//TODO Make sure to reset this when loading a save,
	//TODO because this key will have a chance to no longer
	//TODO be unique. -- squidshire, 28/08/2021
	m_id = reinterpret_cast<DisplayStringIDType>(this);
}

/*** Create a DisplayString.
For use in @{Level-specific.ShowString|ShowString} and @{Level-specific.HideString|HideString}.
@function DisplayString.new
@tparam string str string to print or key of translated string
@tparam int x x-coordinate of top-left of string (or the center if DisplayStringOption.CENTER is given)
@tparam int y y-coordinate of top-left of string (or the center if DisplayStringOption.CENTER is given)
@tparam Color color the color of the text
@tparam table flags a table of display options. Can be empty or omitted. The possible values and their effects are...
	DisplayStringOption.CENTER -- see x and y parameters
	DisplayStringOption.SHADOW -- will give the text a small shadow
__Default: empty__
@tparam bool translated if false or omitted, the str argument will be printed.
If true, the str argument will be the key of a translated string specified in
strings.lua. __Default: false__.
@return A new DisplayString object.
*/
std::unique_ptr<GameScriptDisplayString> GameScriptDisplayString::Create(std::string const & key, int x, int y, GameScriptColor col, TypeOrNil<sol::table> flags, TypeOrNil<bool> maybeTranslated)
{
	auto ptr = std::make_unique<GameScriptDisplayString>();
	auto id = ptr->m_id;
	FlagArray f{};
	if (std::holds_alternative<sol::table>(flags))
	{
		auto tab = std::get<sol::table>(flags);
		for (auto& e : tab)
		{
			auto i = e.second.as<size_t>();
			f[i] = true;
		}
	}
	else if (!std::holds_alternative<sol::nil_t>(flags))
	{
		ScriptAssertF(false, "Wrong argument type for {}.new \"flags\" argument; must be a table or nil.", ScriptReserved_DisplayString);
	}

	bool translated = false;
	if (std::holds_alternative<bool>(maybeTranslated))	
		translated = std::get<bool>(maybeTranslated);
	else if (!std::holds_alternative<sol::nil_t>(maybeTranslated))
		ScriptAssertF(false, "Wrong argument type for {}.new \"translated\" argument; must be a bool or nil.", ScriptReserved_DisplayString);

	UserDisplayString ds{ key, x, y, col, f, translated};

	s_setItemCallback(id, ds);
	return ptr;
}

GameScriptDisplayString::~GameScriptDisplayString()
{
	s_removeItemCallback(m_id);
}

void GameScriptDisplayString::Register(sol::state* state)
{
	state->new_usertype<GameScriptDisplayString>(
		"DisplayString",
		"new", &GameScriptDisplayString::Create,

		/// (@{Color}) RBG color
		// @mem col
		"col", sol::property(&GameScriptDisplayString::GetCol, &GameScriptDisplayString::SetCol),

		/// (string) String key to use. If `translated` is true when @{DisplayString.new}
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @mem key
		"key", sol::property(&GameScriptDisplayString::SetKey, &GameScriptDisplayString::GetKey),

		/// Set the position of the string.
		// Screen-space coordinates are expected.
		// @function DisplayString:SetPos
		// @tparam int x x-coordinate of the string
		// @tparam int y y-coordinate of the string
		"SetPos", &GameScriptDisplayString::SetPos,

		/// Get the position of the string.
		// Screen-space coordinates are returned.
		// @function DisplayString:GetPos
		// @treturn int x x-coordinate of the string
		// @treturn int y y-coordinate of the string
		"GetPos", &GameScriptDisplayString::GetPos
	);
}

DisplayStringIDType GameScriptDisplayString::GetID() const
{
	return m_id;
}

void GameScriptDisplayString::SetPos(int x, int y)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_x = x;
	s.m_y = y;
}

std::tuple<int, int> GameScriptDisplayString::GetPos() const
{	
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return std::make_tuple(s.m_x, s.m_y);
}
	
void GameScriptDisplayString::SetCol(GameScriptColor const & col)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_color = col;
	//todo maybe change getItemCallback to return a ref instead? or move its
	//todo UserDisplayString object? and then move back?
	//s_addItemCallback(m_id, s);
}

GameScriptColor GameScriptDisplayString::GetCol() 
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return s.m_color;
}

void GameScriptDisplayString::SetKey(std::string const & key)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_key = key;
}

std::string GameScriptDisplayString::GetKey() const
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return s.m_key;
}

SetItemCallback GameScriptDisplayString::s_setItemCallback = [](DisplayStringIDType, UserDisplayString)
{
	std::string err = "\"Set string\" callback is not set.";
	throw TENScriptException(err);
	return false;
};

// This is called by a destructor (or will be if we forget to assign it during a refactor)
// and destructors "must never throw", so we terminate instead.
RemoveItemCallback GameScriptDisplayString::s_removeItemCallback = [](DisplayStringIDType)
{
	TENLog("\"Remove string\" callback is not set.", LogLevel::Error);
	std::terminate();
	return false;
};

GetItemCallback GameScriptDisplayString::s_getItemCallback = [](DisplayStringIDType)
{
	std::string err = "\"Get string\" callback is not set.";
	throw TENScriptException(err);
	return std::nullopt;
};

