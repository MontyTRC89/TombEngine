#pragma once
#include "Specific/RGBAColor8Byte.h"

typedef DWORD D3DCOLOR;

namespace sol
{
	class state;
	template <typename T> struct as_table_t;
}

class ScriptColor
{
public:
	ScriptColor(byte r, byte g, byte b);
	ScriptColor(byte r, byte g, byte b, byte a);
	ScriptColor(const Vector3& color);
	ScriptColor(const Vector4& color);
	ScriptColor(D3DCOLOR);

	byte GetR() const;
	byte GetG() const;
	byte GetB() const;
	byte GetA() const;

	void SetR(byte value);
	void SetG(byte value);
	void SetB(byte value);
	void SetA(byte value);

	std::string ToString() const;

	operator Vector3() const;
	operator Vector4() const;
	operator D3DCOLOR() const;
	operator RGBAColor8Byte() const;

	static void Register(sol::table& parent);

private:
	RGBAColor8Byte m_color;
};
