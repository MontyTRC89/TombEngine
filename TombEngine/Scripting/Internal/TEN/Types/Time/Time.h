#pragma once

#include <string>
#include <sol/sol.hpp>

class Time
{
public:
	static void Register(sol::table& parent);

	// Members
	int frameCount = 0;

	// Constructors
	Time() = default;
	Time(int frames);
	Time(const std::string& formattedTime);
	Time(const sol::table& hmsTable);

	// Getters
	int GetFrameCount() const;
	sol::table GetTimeUnits(sol::state_view lua) const;

	// Utilities
	std::string ToString() const;

	// Fields
	void Time::SetHours(int value);
	void Time::SetMinutes(int value);
	void Time::SetSeconds(int value);
	void Time::SetCents(int value);

	int Time::GetHours() const;
	int Time::GetMinutes() const;
	int Time::GetSeconds() const;
	int Time::GetCents() const;

	// Operators
	Time  operator+(const Time& other) const;
	Time  operator-(const Time& other) const;
	Time  operator<(const Time& other) const;
	Time  operator<=(const Time& other) const;
	bool  operator==(const Time& other) const;
	Time& operator+=(const Time& other);
	Time& operator-=(const Time& other);
	Time& Time::operator++();
	Time& Time::operator++(int);
	operator int() const { return (int)frameCount; }

private:
	struct HMSC
	{
		int Hours   = 0;
		int Minutes = 0;
		int Seconds = 0;
		int Cents   = 0;
	};

	HMSC GetHMSC() const;
	static HMSC ParseFormattedString(const std::string& formattedTime);

	// Setters
	void SetFromHMSC(int hours, int minutes = 0, int seconds = 0, int cents = 0);
	void SetFromFormattedString(const std::string& formattedTime);
	void SetFromTable(const sol::table& hmscTable);
};