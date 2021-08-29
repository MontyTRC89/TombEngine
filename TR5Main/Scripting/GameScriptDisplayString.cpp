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

std::unique_ptr<GameScriptDisplayString> GameScriptDisplayString::Create(std::string const & key, int x, int y, GameScriptColor col, int flags)
{
	auto ptr = std::make_unique<GameScriptDisplayString>();
	auto id = ptr->m_id;
	UserDisplayString ds{ key, x, y, col, flags };

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

