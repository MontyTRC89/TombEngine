#include "framework.h"
#include "GameScriptDisplayString.h"

UserDisplayString::UserDisplayString(std::string const& key, int x, int y, D3DCOLOR col, int flags) :
	m_key{ key },
	m_x{ x },
	m_y{ y },
	m_color{ col },
	m_flags{ flags }
{
}

GameScriptDisplayString::GameScriptDisplayString() {
	// We don't ever dereference this pointer; it's just
	// a handy way to get a most-likely-unique key
	// for a hash map.
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

	s_addItemCallback(id, ds);
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
		"new", sol::factories(&GameScriptDisplayString::Create)
		);
}

DisplayStringIDType GameScriptDisplayString::GetID() const
{
	return m_id;
}

AddItemCallback GameScriptDisplayString::s_addItemCallback = [](DisplayStringIDType, UserDisplayString)
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

