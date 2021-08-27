#pragma once
#include "framework.h"
#include "GameScriptColor.h"

// Used to store data used to render the string.
// This is separate from GameScriptDisplayString because the lifetimes
// of the classes differ slightly.
class UserDisplayString
{
public:
	UserDisplayString(std::string const& key, int x, int y, D3DCOLOR col, int flags);
private:
	std::string m_key{};
	D3DCOLOR m_color{ 0xFFFFFFFF };
	int m_flags{ 0 };
	int m_x{ 0 };
	int m_y{ 0 };
	bool m_deleteWhenZero{ false };

	//seconds
	float m_timeRemaining{ 0.0f };
	bool m_isInfinite{ false };
	friend class GameScript;
};

using DisplayStringIDType = uintptr_t;
using AddItemCallback = std::function<void(DisplayStringIDType, UserDisplayString)>;
using RemoveItemCallback = std::function<void(DisplayStringIDType)>;

class GameScriptDisplayString
{
private:
	DisplayStringIDType m_id{ 0 };
public:
	GameScriptDisplayString();
	~GameScriptDisplayString();
	DisplayStringIDType GetID() const;
	static void Register(sol::state* state);

	static AddItemCallback s_addItemCallback;
	static RemoveItemCallback s_removeItemCallback;

	// Creating a GameScriptDisplayString requires us to add an identifier
	// to a data structure. We use callbacks so this class doesn't have
	// to know about said data structure.
	static void SetCallbacks(AddItemCallback cba, RemoveItemCallback cbr)
	{
		s_addItemCallback = cba;
		s_removeItemCallback = cbr;
	}

	static std::unique_ptr<GameScriptDisplayString> Create(std::string const& key, int x, int y, GameScriptColor col, int flags);
};