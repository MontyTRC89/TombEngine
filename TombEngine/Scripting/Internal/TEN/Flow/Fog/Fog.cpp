#include "framework.h"
#include "Fog.h"

#include "Scripting/Internal/TEN/Types/Color/Color.h"

using namespace TEN::Scripting::Types;

/***
Represesnts distance fog. To be used with @{Flow.Level.fog} property.

@tenprimitive Flow.Fog
@pragma nostrip
*/
 
void Fog::Register(sol::table& parent)
{
	using ctors = sol::constructors<Fog(ScriptColor const&, short, short)>;
	parent.new_usertype<Fog>("Fog",
		ctors(),
		sol::call_constructor, ctors(), 

		/// (@{Color}) RGB fog color.
		// @mem color
		"color", sol::property(&Fog::GetColor, &Fog::SetColor),

		/// (int) Minimum distance.
		// This is the distance at which the fog starts.
		// @mem minDistance
		"minDistance", &Fog::MinDistance,

		/// (int) Maximum distance.
		// This is the distance at which the fog reaches the maximum strength.
		// @mem maxDistance
		"maxDistance", &Fog::MaxDistance
		);
}

/***
@tparam Color color RGB color
@tparam int min Distance at which fog starts (in sectors)
@tparam int max Distance at which fog reaches the maximum strength (in sectors)
@treturn Fog A fog object.
@function Fog
*/
Fog::Fog(ScriptColor const& col, short minDistance, short maxDistance)
{
	SetColor(col);
	MinDistance = minDistance;
	MaxDistance = maxDistance;
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
