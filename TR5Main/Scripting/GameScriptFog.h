#pragma once
#include "framework.h"

typedef DWORD D3DCOLOR;

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}

class GameScriptFog {
public:
	byte r{ 0 };
	byte g{ 0 };
	byte b{ 0 };
	byte minDistance{ 0 };
	byte maxDistance{ 0 };

	GameScriptFog(byte r, byte g, byte b, int minDistance, int maxDistance);

	byte								GetR() const;
	void								SetR(byte v);
	byte								GetG() const;
	void								SetG(byte v);
	byte								GetB() const;
	void								SetB(byte v);
	int									GetMinDistance() const;
	void								SetMinDistance(int v);
	int									GetMaxDistance() const;
	void								SetMaxDistance(int v);

	std::string ToString() const;

	static void Register(sol::state* state);
};