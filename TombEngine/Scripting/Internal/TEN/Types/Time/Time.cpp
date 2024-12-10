#include "framework.h"
#include "Time.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <regex>

#include "Math/Math.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Specific/clock.h"

constexpr int TIME_UNIT = 60;
constexpr int CENTISECOND = 100;

/// Represents a time value with support for formatting to hours, minutes, seconds, and centiseconds (1/100th of a second).
// @tenprimitive Time
// @pragma nostrip

void Time::Register(sol::table& parent)
{
	using ctors = sol::constructors<
		Time(), Time(int),
		Time(const std::string&), Time(const sol::table&)>;

	parent.new_usertype<Time>(
		"Time", ctors(),
		sol::call_constructor, ctors(),

		// Meta functions
		sol::meta_function::to_string, &Time::ToString,
		sol::meta_function::addition, &Time::operator+,
		sol::meta_function::subtraction, &Time::operator-,
		sol::meta_function::equal_to, &Time::operator==,
		sol::meta_function::less_than, &Time::operator<,
		sol::meta_function::less_than_or_equal_to, &Time::operator<=,


		// Methods
		"GetTimeUnits", &Time::GetTimeUnits,
		"GetFrameCount", &Time::GetFrameCount,

		// Readable and writable fields
		"h", sol::property(&Time::GetHours,   &Time::SetHours),
		"m", sol::property(&Time::GetMinutes, &Time::SetMinutes),
		"s", sol::property(&Time::GetSeconds, &Time::SetSeconds),
		"c", sol::property(&Time::GetCents,   &Time::SetCents)
	);
}

/// Create a Time object.
// @function Time
// @treturn Time A new Time object initialized to zero time.

/// Create a Time object from a total game frame count (1 second = 30 frames).
// @function Time
// @tparam int frames Total game frame count.
// @treturn Time A new Time object initialized with the given frame count.
Time::Time(int frames) : frameCount(frames) {}

/// Create a Time object from a formatted string.
// @function Time
// @tparam string formattedTime Time in the format "HH:MM:SS[.CC]", where [.CC] is centiseconds, and is optional.
// @treturn Time A new Time object parsed from the given string.
Time::Time(const std::string& formattedTime)
{
	SetFromFormattedString(formattedTime);
}

/// Create a Time object from a time unit table (hours, minutes, seconds, centiseconds).
// @function Time
// @tparam table timeUnits A time unit table in the format {HH, MM, SS, [CC]}, where [CC] is optional.
// @treturn Time A new Time object initialized with the given values.
Time::Time(const sol::table& hmsTable)
{
	SetFromTable(hmsTable);
}

/// Get the total game frame count.
// @function Time:GetFrameCount
// @treturn int Total number of game frames.
int Time::GetFrameCount() const
{
	return frameCount;
}

/// Get the time in hours, minutes, seconds, and centiseconds as a table.
// @function Time:GetTimeUnits
// @treturn table A table in the format {HH, MM, SS, CC}.
sol::table Time::GetTimeUnits(sol::state_view lua) const
{
	HMSC hmsc = GetHMSC();
	sol::table tbl = lua.create_table();
	tbl.add(hmsc.Hours);
	tbl.add(hmsc.Minutes);
	tbl.add(hmsc.Seconds);
	tbl.add(hmsc.Cents);
	return tbl;
}

///  Convert this Time object to a formatted string.
// @function tostring
// @tparam Time this Time.
// @treturn string A string showing time in "HH:MM:SS.CC" format.
std::string Time::ToString() const
{
	HMSC hmsf = GetHMSC();
	std::ostringstream str;
	str << std::setw(2) << std::setfill('0') << hmsf.Hours   << ":"
		<< std::setw(2) << std::setfill('0') << hmsf.Minutes << ":"
		<< std::setw(2) << std::setfill('0') << hmsf.Seconds << "."
		<< std::setw(2) << std::setfill('0') << hmsf.Cents;
	return str.str();
}

/// (int) Hours component.
// @mem h
int Time::GetHours() const
{
	auto [h, m, s, f] = GetHMSC();
	return h;
}

void Time::SetHours(int value)
{
	auto [h, m, s, f] = GetHMSC();
	SetFromHMSC(value, m, s, f);
}

/// (int) Minutes component.
// @mem m
int Time::GetMinutes() const
{
	auto [h, m, s, f] = GetHMSC();
	return m;
}

void Time::SetMinutes(int value)
{
	auto [h, m, s, f] = GetHMSC();
	SetFromHMSC(h, value, s, f);
}

/// (int) Seconds component.
// @mem s
int Time::GetSeconds() const
{
	auto [h, m, s, c] = GetHMSC();
	return s;
}

void Time::SetSeconds(int value)
{
	auto [h, m, s, c] = GetHMSC();
	SetFromHMSC(h, m, value, c);
}

/// (int) Centiseconds component.
// @mem c
int Time::GetCents() const
{
	auto [h, m, s, c] = GetHMSC();
	return c;
}

void Time::SetCents(int value)
{
	auto [h, m, s, c] = GetHMSC();
	SetFromHMSC(h, m, s, value);
}

Time::HMSC Time::GetHMSC() const
{
	int totalSeconds = frameCount / FPS;

	return
	{
		totalSeconds / SQUARE(TIME_UNIT),
		(totalSeconds % SQUARE(TIME_UNIT)) / TIME_UNIT,
		totalSeconds % TIME_UNIT,
		(frameCount * 100 / FPS) % CENTISECOND
	};
}

Time::HMSC Time::ParseFormattedString(const std::string& formattedTime)
{
	std::regex timeFormat(R"((\d{2}):(\d{2}):(\d{2})\.(\d{2}))");
	std::smatch match;

	if (!std::regex_match(formattedTime, match, timeFormat))
	{
		TENLog("Invalid time format. Expected HH:MM:SS or HH:MM:SS.CC", LogLevel::Warning);
		return Time::HMSC();
	}

	return
	{
		std::stoi(match[1].str()),
		std::stoi(match[2].str()),
		std::stoi(match[3].str()),
		match[4].matched ? std::stoi(match[4].str()) : 0
	};
}

void Time::SetFromHMSC(int hours, int minutes, int seconds, int cents)
{
	frameCount = (hours * SQUARE(TIME_UNIT) + minutes * TIME_UNIT + seconds) * FPS +
				  round((float)cents / ((float)CENTISECOND / (float)FPS));
}

void Time::SetFromFormattedString(const std::string& formattedTime)
{
	HMSC hmsf = ParseFormattedString(formattedTime);
	SetFromHMSC(hmsf.Hours, hmsf.Minutes, hmsf.Seconds, hmsf.Cents);
}

void Time::SetFromTable(const sol::table& hmsTable)
{
	if (!hmsTable.valid() || hmsTable.size() < 1 || hmsTable.size() > 4)
	{
		throw std::invalid_argument("Invalid time unit table. Expected {HH, MM, SS, [CC]}");
	}

	int hours   = hmsTable.get_or(1, 0);
	int minutes = hmsTable.get_or(2, 0);
	int seconds = hmsTable.get_or(3, 0);
	int cents   = hmsTable.get_or(4, 0);

	SetFromHMSC(hours, minutes, seconds, cents);
}


Time& Time::operator++()
{
	++frameCount;
	return *this;
}

Time& Time::operator++(int)
{
	frameCount++;
	return *this;
}

Time& Time::operator+=(const Time& other)
{
	frameCount += other.frameCount;
	return *this;
}

Time& Time::operator-=(const Time& other)
{
	frameCount -= other.frameCount;
	return *this;
}

Time Time::operator+(const Time& other)  const { return Time(frameCount +  other.frameCount); }
Time Time::operator-(const Time& other)  const { return Time(frameCount -  other.frameCount); }
Time Time::operator<(const Time& other)  const { return Time(frameCount <  other.frameCount); }
Time Time::operator<=(const Time& other) const { return Time(frameCount <= other.frameCount); }
bool Time::operator==(const Time& other) const { return frameCount == other.frameCount; }