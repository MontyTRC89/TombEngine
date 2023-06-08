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
		const auto& anim = GetAnimData(item);

		long c;

		Vector3i pos;

		if (item->ItemFlags[0] > 80)
			item->HitPoints = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != MUTANT_CRAWLER_STATE_DEATH)
			{
				SetAnimation(item, MUTANT_CRAWLER_ANIM_DEATH);
				creature.Flags = 0;
			}

			else if (item->ItemFlags[0] > 80)
			{
				for (int i = 9; i < 17; i++)
				{
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;
					GetJointPosition(item, i, pos);
					//TriggerFireFlame(pos.x, pos.y, pos.z, -1, 255); not sure how to translate this
				}

				c = anim.frameEnd - GetAnimData(item).frameBase;

				if (c > 16)
				{
					c = anim.frameEnd - item->Animation.FrameNumber;

					if (c > 16)
						c = 16;
				}

				/* No idea how to translate this.
				b = GetRandomControl();
				r = (c * (255 - ((b >> 4) & 0x1F))) >> 4;
				g = (c * (192 - ((b >> 6) & 0x3F))) >> 4;
				b = (c * (b & 0x3F)) >> 4;
				TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 12, r, g, b);
				*/ 
			}

			else if (item->Animation.FrameNumber >= )
		}
	}
}