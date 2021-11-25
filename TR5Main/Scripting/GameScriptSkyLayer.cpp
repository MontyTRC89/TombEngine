#include "framework.h"
#include <sol.hpp>
#include "GameScriptSkyLayer.h"

/*** Describes a layer of moving clouds.
As seen in TR4's City of the Dead.

@pregameclass SkyLayer
@pragma nostrip
*/

void GameScriptSkyLayer::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptSkyLayer>("SkyLayer",
		sol::constructors<GameScriptSkyLayer(GameScriptColor const &, short)>(),

/// (@{Color}) RGB sky color
//@mem color
		"color", sol::property(&GameScriptSkyLayer::SetColor),

/*** (int) cloud speed.

Values can be between [-32768, 32767], with positive numbers resulting in a sky that scrolls from
west to east, and negative numbers resulting in one that travels east to west.

Please note that speeds outside of the range of about [-1000, 1000] will cause the
sky to scroll so fast that it will no longer appear as a coherent stream of clouds.
Less is more. City of The Dead, for example, uses a speed value of 16.
 
@mem speed*/
		"speed", &GameScriptSkyLayer::CloudSpeed
		);
}

/*** 
@tparam Color color RGB color
@tparam int speed cloud speed
@return A SkyLayer object.
@function SkyLayer.new
*/
GameScriptSkyLayer::GameScriptSkyLayer(GameScriptColor const& col, short speed)
{
	SetColor(col);
	CloudSpeed = speed;
	Enabled = true;
}

void GameScriptSkyLayer::SetColor(GameScriptColor const & col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}


