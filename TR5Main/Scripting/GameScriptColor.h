#pragma once

#include "framework.h"

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}

class GameScriptColor {
public:
	byte r;
	byte g;
	byte b;
	byte a;

	GameScriptColor(byte r, byte g, byte b);
	GameScriptColor(byte r, byte g, byte b, byte a);
	GameScriptColor(Vector3 const &);
	GameScriptColor(Vector4 const &);

	operator Vector3() const;
	operator Vector4() const;

	byte								GetR();
	void								SetR(byte v);
	byte								GetG();
	void								SetG(byte v);
	byte								GetB();
	void								SetB(byte v);
	byte								GetA();
	void								SetA(byte v);

	static void Register(sol::state* state);
};