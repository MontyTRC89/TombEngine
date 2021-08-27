#pragma once
#include "framework.h"

typedef DWORD D3DCOLOR;

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}

class GameScriptColor {
public:
	byte r{ 0 };
	byte g{ 0 };
	byte b{ 0 };
	byte a{ 255 };

	GameScriptColor(byte r, byte g, byte b);
	GameScriptColor(byte r, byte g, byte b, byte a);
	GameScriptColor(Vector3 const &);
	GameScriptColor(Vector4 const &);

	operator Vector3() const;
	operator Vector4() const;
	operator D3DCOLOR() const;

	byte								GetR() const;
	void								SetR(byte v);
	byte								GetG() const;
	void								SetG(byte v);
	byte								GetB() const;
	void								SetB(byte v);
	byte								GetA() const;
	void								SetA(byte v);

	std::string ToString() const;

	static void Register(sol::state* state);
};