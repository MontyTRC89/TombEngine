#include "framework.h"
#include "GameScriptFog.h"
/*** Describes a layer of moving clouds.
As seen in TR4's City of the Dead.

@pregameclass SkyLayer
@pragma nostrip
*/
 
void GameScriptFog::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptFog>("Fog",
		sol::constructors<GameScriptFog(GameScriptColor const&, short, short)>(),

		/// (@{Color}) RGB sky color
		//@mem color
		"color", sol::property(&GameScriptFog::SetColor),

		/*** (int) min distance.

		This is the distance at which the fog starts 

		@mem minDistance*/
		"minDistance", &GameScriptFog::MinDistance,

		/*** (int) max distance.

		This is the distance at which the fog reaches the maximum strength

		@mem maxDistance*/
		"maxDistance", & GameScriptFog::MaxDistance
		);
}

/***
@tparam Color color RGB color
@tparam int speed cloud speed
@return A SkyLayer object.
@function SkyLayer.new
*/
GameScriptFog::GameScriptFog(GameScriptColor const& col, short minDistance, short maxDistance)
{
	SetColor(col);
	MinDistance = minDistance;
	MaxDistance = maxDistance;
	Enabled = true;
}

void GameScriptFog::SetColor(GameScriptColor const& col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}


