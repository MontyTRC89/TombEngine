#pragma once
#include "framework.h"
#include <variant>
#include <sol.hpp>
#include "GameScriptColor.h"

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
// This is separate from GameScriptDisplayString because the lifetimes
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
	friend class GameScript;
	friend class GameScriptDisplayString;
};

using DisplayStringIDType = uintptr_t;
using SetItemCallback = std::function<bool(DisplayStringIDType, UserDisplayString const&)>;
using RemoveItemCallback = std::function<bool(DisplayStringIDType)>;
using GetItemCallback = std::function<std::optional<std::reference_wrapper<UserDisplayString>>(DisplayStringIDType)>;

// Helper type to allow us to more easily specify "give a value of type X or just give nil" parameters.
// Sol doesn't (at the time of writing) have any mechanisms to do this kind of optional argument without
// drawbacks, or at least no mechanisms that I could find.
//
// sol::optional doesn't distinguish between nil values and values of the wrong type
// (so we can't provide the user with an error message to tell them they messed up).
//
// std::variant works better, providing an error if the user passes in an arg of the wrong type, but
// the error isn't too helpful and exposes a lot of C++ code which will not help them fix the error.
//
// sol::object lets us check that the user has given the right type, but takes valuable type information
// away from the function's C++ signature, giving us things like void func(sol::object, sol::object, sol::object),
// even if the function's actual expected parameter types are (for example) float, sol::table, SomeUserType.
// 
// This alias is an effort to avoid the above problems.
template <typename ... Ts> using TypeOrNil = std::variant<Ts..., sol::nil_t, sol::object>;

class GameScriptDisplayString
{
private:
	DisplayStringIDType m_id{ 0 };
public:
	GameScriptDisplayString();
	~GameScriptDisplayString();
	DisplayStringIDType GetID() const;
	static void Register(sol::state* state);

	void SetPos(int x, int y);
	std::tuple<int, int> GetPos() const;

	void SetCol(GameScriptColor const&);
	GameScriptColor GetCol();

	void SetKey(std::string const&);
	std::string GetKey() const;

	static SetItemCallback s_setItemCallback;
	static RemoveItemCallback s_removeItemCallback;
	static GetItemCallback s_getItemCallback;

	// Creating a GameScriptDisplayString requires us to add an identifier
	// to a data structure. We use callbacks so this class doesn't have
	// to know about said data structure.
	static void SetCallbacks(SetItemCallback cba, RemoveItemCallback cbr, GetItemCallback cbg)
	{
		s_setItemCallback = cba;
		s_removeItemCallback = cbr;
		s_getItemCallback = cbg;
	}

	static std::unique_ptr<GameScriptDisplayString> Create(std::string const& key, int x, int y, GameScriptColor col, TypeOrNil<sol::table>, TypeOrNil<bool>);
};