#include "framework.h"
#include "GameScriptFog.h"

/***
An RGBA or RGB color.
Components are specified in bytes; all values are clamped to [0, 255].

@miscclass Color
@pragma nostrip
*/

void GameScriptFog::Register(sol::state* state)
{
	state->new_usertype<GameScriptFog>("Fog",
		sol::constructors<GameScriptFog(byte, byte, byte, int, int)>(),
		sol::meta_function::to_string, &GameScriptFog::ToString,

		/// (int) red component
		//@mem r
		"r", sol::property(&GameScriptFog::GetR, &GameScriptFog::SetR),

		/// (int) green component
		//@mem g
		"g", sol::property(&GameScriptFog::GetG, &GameScriptFog::SetG),

		/// (int) blue component
		//@mem b
		"b", sol::property(&GameScriptFog::GetB, &GameScriptFog::SetB),

		/// (int) the distance at fog starts (in sectors)
		//@mem minDistance
		"minDistance", sol::property(&GameScriptFog::GetMinDistance, &GameScriptFog::SetMinDistance),

		/// (int) the distance at fog reaches the max strength (in sectors)
		//@mem maxDistance
		"maxDistance", sol::property(&GameScriptFog::GetMaxDistance, &GameScriptFog::SetMaxDistance)
		);
}

/***
@int R red component
@int G green component
@int B blue component
@int minDistance min distance of the fog
@int maxDistance max distance of the fog
@return A Fog object.
@function Fog.new
*/
GameScriptFog::GameScriptFog(byte r, byte g, byte b, int minDistance, int maxDistance)
{
	SetR(r);
	SetG(g);
	SetB(b);
	SetMinDistance(minDistance);
	SetMaxDistance(maxDistance);
}

byte GameScriptFog::GetR() const
{
	return r;
}

void GameScriptFog::SetR(byte v)
{
	r = std::clamp<byte>(v, 0, 255);
}

byte GameScriptFog::GetG() const
{
	return g;
}

void GameScriptFog::SetG(byte v)
{
	g = std::clamp<byte>(v, 0, 255);
}

byte GameScriptFog::GetB() const
{
	return b;
}

void GameScriptFog::SetB(byte v)
{
	b = std::clamp<byte>(v, 0, 255);
}

int GameScriptFog::GetMinDistance() const
{
	return minDistance;
}

void GameScriptFog::SetMinDistance(int v)
{
	minDistance = std::clamp<byte>(v, 0, 200);
}

int GameScriptFog::GetMaxDistance() const
{
	return maxDistance;
}

void GameScriptFog::SetMaxDistance(int v)
{
	maxDistance = std::clamp<byte>(v, 0, 200);
}

/***
@tparam Color color this color
@treturn string A string showing the r, g, b, and a values of the color
@function __tostring
*/
std::string GameScriptFog::ToString() const
{
	return "{" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(minDistance) + ", " + std::to_string(maxDistance) + "}";
}
