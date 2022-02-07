#pragma once
#include "Specific/RGBAColor8Byte.h"

typedef DWORD D3DCOLOR;

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}

class ScriptColor {
public:
	ScriptColor(byte r, byte g, byte b);
	ScriptColor(byte r, byte g, byte b, byte a);
	ScriptColor(Vector3 const &);
	ScriptColor(Vector4 const &);
	ScriptColor(D3DCOLOR);

	operator Vector3() const;
	operator Vector4() const;
	operator D3DCOLOR() const;
	operator RGBAColor8Byte() const;

	byte GetR() const;
	void SetR(byte v);
	byte GetG() const;
	void SetG(byte v);
	byte GetB() const;
	void SetB(byte v);
	byte GetA() const;
	void SetA(byte v);

	std::string ToString() const;

	static void Register(sol::table & parent);
private:
	RGBAColor8Byte m_color;
};