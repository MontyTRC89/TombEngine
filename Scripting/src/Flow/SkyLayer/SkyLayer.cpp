#include "frameworkandsol.h"
#include "SkyLayer.h"

/*** Describes a layer of moving clouds.
As seen in TR4's City of the Dead.

@tenclass Flow.SkyLayer
@pragma nostrip
*/

void SkyLayer::Register(sol::table & parent)
{
	parent.new_usertype<SkyLayer>("SkyLayer",
		sol::constructors<SkyLayer(GameScriptColor const &, short)>(),

/// (@{Color}) RGB sky color
//@mem color
		"color", sol::property(&SkyLayer::GetColor, &SkyLayer::SetColor),

/*** (int) cloud speed.

Values can be between [-32768, 32767], with positive numbers resulting in a sky that scrolls from
west to east, and negative numbers resulting in one that travels east to west.

Please note that speeds outside of the range of about [-1000, 1000] will cause the
sky to scroll so fast that it will no longer appear as a coherent stream of clouds.
Less is more. City of The Dead, for example, uses a speed value of 16.
 
@mem speed*/
		"speed", &SkyLayer::CloudSpeed
		);
}

/*** 
@tparam Color color RGB color
@tparam int speed cloud speed
@return A SkyLayer object.
@function SkyLayer.new
*/
SkyLayer::SkyLayer(GameScriptColor const& col, short speed)
{
	SetColor(col);
	CloudSpeed = speed;
	Enabled = true;
}

void SkyLayer::SetColor(GameScriptColor const & col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}

GameScriptColor SkyLayer::GetColor() const {
	return GameScriptColor{ R, G, B };
}
