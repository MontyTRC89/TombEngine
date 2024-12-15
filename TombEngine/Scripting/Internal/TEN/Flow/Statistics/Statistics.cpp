#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Statistics/Statistics.h"

#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"

/***
A set of gameplay statistics. Can be accessed using @{Flow.GetStatistics} and @{Flow.SetStatistics} functions.

@tenprimitive Flow.Statistics
@pragma nostrip
*/

namespace TEN::Scripting
{
	void Statistics::Register(sol::table& parent)
	{
		parent.new_usertype<Statistics>(
			ScriptReserved_Statistics, sol::constructors<Statistics()>(),
			sol::call_constructor, sol::constructors<Statistics()>(),
			sol::meta_function::new_index, newindex_error_maker(Statistics, ScriptReserved_Statistics),

			/*** Ammo hits.
			@tfield int ammoHits amount of successful enemy hits.
			*/
			"ammoHits", &Statistics::AmmoHits,

			/*** Ammo used.
			@tfield int ammoUsed amount of used ammo.
			*/
			"ammoUsed", &Statistics::AmmoUsed,

			/*** Distance traveled.
			@tfield int traveledDistance amount of traveled distance in world units. One meter is 420 world units.
			*/
			"traveledDistance", &Statistics::Distance,

			/*** Health packs used.
			@tfield int healthPacksUsed amount of used medipacks.
			*/
			"healthPacksUsed", &Statistics::HealthUsed,

			/*** Damage taken.
			@tfield int damageTaken overall amount of taken damage.
			*/
			"damageTaken", &Statistics::DamageTaken,

			/*** Kills.
			@tfield int kills amount of killed enemies.
			*/
			"kills", &Statistics::Kills,

			/*** Secrets.
			@tfield int secrets amount of found secrets.
			*/
			"secrets", &Statistics::Secrets,

			/*** Time taken.
			@tfield Time timeTaken amount of time passed.
			*/
			"timeTaken", &Statistics::TimeTaken);
	}
}
