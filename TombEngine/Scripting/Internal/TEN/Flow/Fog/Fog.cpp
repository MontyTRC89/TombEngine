#include "framework.h"
#include "Fog.h"

/***
Fog

@tenclass Flow.Fog
@pragma nostrip
*/
 
void Fog::Register(sol::table & parent)
{
	using ctors = sol::constructors<Fog(ScriptColor const&, short, short)>;
	parent.new_usertype<Fog>("Fog",
		ctors(),
		sol::call_constructor, ctors(), 

		/// (@{Color}) RGB fog color
		//@mem color
		"color", sol::property(&Fog::GetColor, &Fog::SetColor),

		/*** (int) min distance.

		This is the distance at which the fog starts 

		@mem minDistance*/
		"minDistance", &Fog::MinDistance,

		/*** (int) max distance.

		This is the distance at which the fog reaches the maximum strength

		@mem maxDistance*/
		"maxDistance", &Fog::MaxDistance
		);
}

/***
@tparam Color color RGB color
@tparam int Min Distance fog starts (in Sectors)
@tparam int Max Distance fog ends (in Sectors)
@return A fog object.
@function Fog.new
*/
Fog::Fog(ScriptColor const& col, short minDistance, short maxDistance)
{
	SetColor(col);
	MinDistance = minDistance;
	MaxDistance = maxDistance;
	Enabled = true;
}

void Fog::SetColor(ScriptColor const& col)
{
	R = col.GetR();
	G = col.GetG();
	B = col.GetB();
}


ScriptColor Fog::GetColor() const
{
	return ScriptColor{ R, G, B };
}
