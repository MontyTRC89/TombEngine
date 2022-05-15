#pragma once

class RGBAColor8Byte;

namespace sol {
	class state;
}

struct Fog
{
	bool Enabled{ false };
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short MinDistance{ 0 };
	short MaxDistance{ 0 };

	Fog() = default;
	Fog(RGBAColor8Byte const& col, short minDistance, short maxDistance);
	void SetColor(RGBAColor8Byte const& col);
	RGBAColor8Byte GetColor() const;

	static void Register(sol::table &);
};
