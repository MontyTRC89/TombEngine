#include "framework.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Effects/enemy_missile.h"

namespace TEN::Entities::Creatures::TR3
{

	const auto MutantCrawlerGasBite = CreatureBiteInfo(Vector3i(0, 40, 140), 10);

	enum MutantCrawlerState
	{
		MUTANT_CRAWLER_STATE_IDLE = 0,
		MUTANT_CRAWLER_STATE_WALK = 1,
		MUTANT_CRAWLER_STATE_ATTACK = 2,
		MUTANT_CRAWLER_STATE_DEATH = 3
	};

	enum MutantCrawlerAnim
	{
		MUTANT_CRAWLER_ANIM_IDLE = 0,
		MUTANT_CRAWLER_ANIM_IDLE_TO_WALK = 1,
		MUTANT_CRAWLER_ANIM_WALKING = 2,
		MUTANT_CRAWLER_ANIM_WALK_TO_IDLE = 3,
		MUTANT_CRAWLER_ANIM_ATTACKING = 4,
		MUTANT_CRAWLER_ANIM_DEATH = 5,
		MUTANT_CRAWLER_ANIM_TRAP = 6 // This animation is from TR3 Antarctica mutant crawler variation where it wakes up, spits poison, dies. needs to be inserted to Mutant_crawler Kubsy 08/06/2022
	};

	void MutantCrawlerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(item);

		if (item->ItemFlags[0] > 80)
			item->HitPoints = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 3)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = 3;
				creature.Flags = 0;
			}

			else if (item->ItemFlags[0] > 80)
			{
				for (int i = 9; i < 17; i++)
				{
					getJoi
				}
			}

		}

	}

}