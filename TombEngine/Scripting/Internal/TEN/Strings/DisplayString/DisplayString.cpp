#include "framework.h"

#include "Scripting/Internal/TEN/Strings/DisplayString/DisplayString.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

/*** A string appearing on the screen.
Can be used for subtitles and "2001, somewhere in Egypt"-style messages.

Uses screen-space coordinates, with x values specifying the number of pixels from the left of the window,
and y values specifying the number of pixels from the top of the window.

Since different players will have different resolutions, you should work in terms of percentages where possible,
and use @{Util.ScreenToPercent|ScreenToPercent} and @{Util.PercentToScreen|PercentToScreen}
when you need to use screen-space coordinates.

@tenclass Strings.DisplayString
@pragma nostrip
*/

UserDisplayString::UserDisplayString(const std::string& key, const Vec2& pos, float scale, D3DCOLOR color, const FlagArray& flags, bool isTranslated) :
	_key(key),
	_position(pos),
	_scale(scale),
	_color(color),
	_flags(flags),
	_isTranslated(isTranslated)
{
}

DisplayString::DisplayString()
{
	// We don't ever dereference this pointer; it's just
	// a handy way to get a unique key for a hash map.

	_id = reinterpret_cast<DisplayStringID>(this);
}

/*** Create a DisplayString.
For use in @{Strings.ShowString|ShowString} and @{Strings.HideString|HideString}.
@function DisplayString
@tparam string string The string to display or key of the translated string.
@tparam Vec2 Position of the string in pixel coordinates.
@tparam[opt] float scale size of the string, relative to the default size. __Default: 1.0__
@tparam[opt] Color color the color of the text. __Default: white__
@tparam[opt] bool translated If false or omitted, the input string argument will be displayed.
If true, the string argument will be the key of a translated string specified in strings.lua. __Default: false__.
@tparam Strings.DisplayStringOption table
__Default: None.__ _Please note that Strings are automatically aligned to the LEFT_
@treturn DisplayString A new DisplayString object.
*/
static std::unique_ptr<DisplayString> CreateString(const std::string& key, const Vec2& pos, TypeOrNil<float> scale, TypeOrNil<ScriptColor> color,
												   TypeOrNil<bool> isTranslated, TypeOrNil<sol::table> flags, sol::this_state state)
{
	auto ptr = std::make_unique<DisplayString>();
	auto id = ptr->GetID();

	auto getCallStack = [state]
	{
		luaL_traceback(state, state, nullptr, 0);
		auto traceback = std::string(lua_tostring(state, -1));
		lua_pop(state, 1);
		return traceback;
	};

	auto flagArray = FlagArray{};
	if (std::holds_alternative<sol::table>(flags))
	{
		auto tab = std::get<sol::table>(flags);
		for (auto& e : tab)
		{
			auto i = e.second.as<size_t>();
			flagArray[i] = true;
		}
	}
	else if (!std::holds_alternative<sol::nil_t>(flags))
	{
		ScriptAssertF(false, "Wrong argument type for {}.new \"flags\" argument; must be a table or nil.\n{}", ScriptReserved_DisplayString, getCallStack());
	}

	if (!IsValidOptionalArg(isTranslated))	
		ScriptAssertF(false, "Wrong argument type for {}.new \"translated\" argument; must be a bool or nil.\n{}", ScriptReserved_DisplayString, getCallStack());

	if (!IsValidOptionalArg(color))	
		ScriptAssertF(false, "Wrong argument type for {}.new \"color\" argument; must be a {} or nil.\n{}", ScriptReserved_DisplayString, ScriptReserved_Color, getCallStack());

	if (!IsValidOptionalArg(scale))	
		ScriptAssertF(false, "Wrong argument type for {}.new \"scale\" argument; must be a float or nil.\n{}", ScriptReserved_DisplayString, getCallStack());

	auto string = UserDisplayString(key, pos, USE_IF_HAVE(float, scale, 1.0f), USE_IF_HAVE(ScriptColor, color, ScriptColor(255, 255, 255)), flagArray, USE_IF_HAVE(bool, isTranslated, false));
	DisplayString::SetItemCallbackRoutine(id, string);
	return ptr;
}

// HACK: Constructor wrapper for DisplayString smart pointer to maintain compatibility with deprecated version calls.
sol::object DisplayStringWrapper(const std::string& key, sol::object unkArg0, sol::object unkArg1, TypeOrNil<ScriptColor> color,
								 TypeOrNil<bool> isTranslated, TypeOrNil<sol::table> flags, sol::this_state state)
{
	// Regular constructor.
	if (unkArg0.is<Vec2>() && unkArg1.is<float>())
	{
		auto pos = (Vec2)unkArg0.as<Vec2>();
		float scale = unkArg1.as<float>();

		auto displayString = CreateString(key, pos, scale, color, isTranslated, flags, state);
		return sol::make_object(state, displayString.release());

	}
	// Deprecated constructor.
	else if (unkArg0.is<int>() && unkArg1.is<int>())
	{
		auto pos = Vec2((float)unkArg0.as<int>(), (float)unkArg1.as<int>());

		auto displayString = CreateString(key, pos, 1.0f, color, isTranslated, flags, state);
		return sol::make_object(state, displayString.release());
	}

	TENLog("Failed to create DisplayString. Unknown parameters.");
	return sol::object(state, sol::nil);
}

DisplayString::~DisplayString()
{
	RemoveItemCallbackRoutine(_id);
}

void DisplayString::Register(sol::table& parent)
{
	parent.new_usertype<DisplayString>(
		ScriptReserved_DisplayString,
		sol::call_constructor, &DisplayStringWrapper,

		/// Get the display string's color
		// @function DisplayString:GetColor
		// @treturn Color a copy of the display string's color
		ScriptReserved_GetColor, &DisplayString::GetColor,

		/// Set the display string's color 
		// @function DisplayString:SetColor
		// @tparam Color color the new color of the display string 
		ScriptReserved_SetColor, &DisplayString::SetColor,

		/// Get the string key to use. If `isTranslated` is true when @{DisplayString}
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @function DisplayString:GetKey()
		// @treturn string the string to use
		ScriptReserved_GetKey, &DisplayString::GetKey, 

		/// Set the string key to use. If `isTranslated` is true when @{DisplayString}
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @function DisplayString:SetKey()
		// @tparam string string the new key for the display string 
		ScriptReserved_SetKey, &DisplayString::SetKey, 

		/// Set the scale of the string.
		// @function DisplayString:SetScale()
		// @tparam float scale New scale of the string relative to the default size.
		ScriptReserved_SetScale, &DisplayString::SetScale,

		/// Get the scale of the string.
		// @function DisplayString:GetScale()
		// @treturn float Scale.
		ScriptReserved_GetScale, &DisplayString::GetScale,

		/// Set the position of the string.
		// Screen-space coordinates are expected.
		// @function DisplayString:SetPosition()
		// @tparam Vec2 pos New position in pixel coordinates.
		ScriptReserved_SetPosition, &DisplayString::SetPosition,

		/// Get the position of the string.
		// Screen-space coordinates are returned.
		// @function DisplayString:GetPosition()
		// @treturn Vec2 pos Position in pixel coordinates.
		ScriptReserved_GetPosition, &DisplayString::GetPosition,

		/// Set the display string's flags 
		// @function DisplayString:SetFlags()
		// @tparam table table the new table with display flags options
		// @usage
		// local varDisplayString = DisplayString('example string', 0, 0, Color(255, 255, 255), false)
		// possible values:
		// varDisplayString:SetFlags({})
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.SHADOW })
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.CENTER })
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.SHADOW, TEN.Strings.DisplayStringOption.CENTER })
		// -- When passing a table to a function, you can omit the parentheses
		// varDisplayString:SetFlags{ TEN.Strings.DisplayStringOption.CENTER }
		ScriptReserved_SetFlags, &DisplayString::SetFlags,

		/// Set translated parameter of the string
		// @function DisplayString:SetTranslated
		// @tparam bool shouldTranslate if true, the string's key will be used as the key for the translation that will be displayed.
		// If false, the key itself will be displayed
		ScriptReserved_SetTranslated, &DisplayString::SetTranslated);
}

DisplayStringID DisplayString::GetID() const
{
	return _id;
}

void DisplayString::SetScale(float scale)
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	displayString._scale = scale;
}

float DisplayString::GetScale() const
{
	const UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	return displayString._scale;
}

void DisplayString::SetPosition(const sol::variadic_args& args)
{
	auto& displayString = GetItemCallbackRoutine(_id).value();

	if (args.size() == 1)
	{
		// Handle case when single argument provided.
		if (args[0].is<Vec2>()) 
			displayString.get()._position = args[0].as<Vec2>();
	}
	else if (args.size() == 2)
	{
		// Handle case when two arguments provided, assuming they are integers.
		if (args[0].is<int>() && args[1].is<int>())
		{
			int x = args[0].as<int>();
			int y = args[1].as<int>();
			displayString.get()._position = Vec2((int)x, (int)y);
		}
	}
	else
	{
		TENLog("Invalid arguments in SetPosition() call.");
	}
}

Vec2 DisplayString::GetPosition() const
{	
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	return displayString._position;
}
	
void DisplayString::SetColor(const ScriptColor& color)
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	displayString._color = color;

	//todo maybe change getItemCallback to return a ref instead? or move its
	//todo UserDisplayString object? and then move back?
	//s_addItemCallback(m_id, s);
}

ScriptColor DisplayString::GetColor() const
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	return displayString._color;
}

void DisplayString::SetKey(const std::string& key)
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	displayString._key = key;
}

std::string DisplayString::GetKey() const
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	return displayString._key;
}

void DisplayString::SetFlags(const sol::table& flags) 
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();

	auto flagArray = FlagArray {};
	for (const auto& val : flags)
	{
		auto i = val.second.as<size_t>();
		flagArray[i] = true;
	}

	displayString._flags = flagArray;
}

void DisplayString::SetTranslated(bool isTranslated)
{
	UserDisplayString& displayString = GetItemCallbackRoutine(_id).value();
	TENLog(isTranslated ? "Translated string " : "Untranslated string " + std::to_string(isTranslated), LogLevel::Info);
	displayString._isTranslated = isTranslated;
}

SetItemCallback DisplayString::SetItemCallbackRoutine = [](DisplayStringID, UserDisplayString)
{
	std::string err = "\"Set string\" callback is not set.";
	throw TENScriptException(err);
	return false;
};

// This is called by a destructor (or will be if we forget to assign it during a refactor)
// and destructors "must never throw", so we terminate instead.
RemoveItemCallback DisplayString::RemoveItemCallbackRoutine = [](DisplayStringID)
{
	TENLog("\"Remove string\" callback is not set.", LogLevel::Error);
	std::terminate();
	return false;
};

GetItemCallback DisplayString::GetItemCallbackRoutine = [](DisplayStringID)
{
	std::string err = "\"Get string\" callback is not set.";
	throw TENScriptException(err);
	return std::nullopt;
};
