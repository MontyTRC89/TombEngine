#include "frameworkandsol.h"
#include "Fog.h"
#include "Specific/RGBAColor8Byte.h"
/*** Describes a layer of moving clouds.
As seen in TR4's City of the Dead.

@pregameclass SkyLayer
@pragma nostrip
*/
 
void GameScriptFog::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptFog>("Fog",
		sol::constructors<GameScriptFog(RGBAColor8Byte const&, short, short)>(),

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
GameScriptFog::GameScriptFog(RGBAColor8Byte const& col, short minDistance, short maxDistance)
{
	SetColor(col);
	MinDistance = minDistance;
	MaxDistance = maxDistance;
	Enabled = true;
}

void GameScriptFog::SetColor(RGBAColor8Byte const& col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}


RGBAColor8Byte GameScriptFog::GetColor() const
{
	return RGBAColor8Byte{ R, G, B };
}
