#include "framework.h"
#include "Objects/TR3/Entity/tr3_sophia.h"

#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Objects/TR3/boss.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR3
{
	static BOSS_STRUCT BossData;

	#define MAX_TRIGGER_RANGE 0x4000
	#define SMALL_FLASH 10
	#define BIG_FLASH 16
	#define BOLT_SPEED 384

	enum { NOTHING, SUMMONING, KNOCKBACK };
	enum { NORMAL_BOLT, LARGE_BOLT, SUMMON_BOLT };
	enum { RIGHT_PRONG, ICONPOS, LEFT_PRONG };
	enum { ATTACK_HEAD, ATTACK_HAND1, ATTACK_HAND2 };

	enum SophiaState
	{
		LONDONBOSS_EMPTY = 0,
		LONDONBOSS_STAND = 1,
		LONDONBOSS_WALK = 2,
		LONDONBOSS_RUN = 3,
		LONDONBOSS_SUMMON = 4,
		LONDONBOSS_BIGZAP = 5,
		LONDONBOSS_DEATH = 6,
		LONDONBOSS_LAUGH = 7,
		LONDONBOSS_LILZAP = 8,
		LONDONBOSS_VAULT2 = 9,
		LONDONBOSS_VAULT3 = 10,
		LONDONBOSS_VAULT4 = 11,
		LONDONBOSS_GODOWN = 12,
	};

	// TODO
	enum SophiaAnim
	{

	};

#define	LONDONBOSS_VAULT2_ANIM 9
#define	LONDONBOSS_VAULT3_ANIM 18
#define	LONDONBOSS_VAULT4_ANIM 15
#define	LONDONBOSS_GODOWN_ANIM 21
#define	LONDONBOSS_STND2SUM_ANIM 1
#define LONDONBOSS_SUMMON_ANIM 2
#define	LONDONBOSS_GODOWN_ANIM 21

#define LONDONBOSS_VAULT_SHIFT 96
#define LONDONBOSS_AWARE_DISTANCE pow(SECTOR(1), 2)
#define LONDONBOSS_WALK_TURN ANGLE(4)
#define LONDONBOSS_RUN_TURN ANGLE(7)
#define LONDONBOSS_WALK_RANGE pow(SECTOR(1), 2)
#define LONDONBOSS_WALK_CHANCE 0x100
#define LONDONBOSS_LAUGH_CHANCE 0x100
#define LONDONBOSS_TURN ANGLE(2.0f)
#define LONDONBOSS_DIE_ANIM 17
#define LONDONBOSS_FINAL_HEIGHT -11776
#define BIGZAP_TIMER 600

	static void TriggerLaserBolt(Vector3Int* pos, ItemInfo* item, long type, short yAngle)
	{

	}

	static void TriggerPlasmaBallFlame(short fxNumber, long type, long xv, long yv, long zv)
	{

	}

	static void TriggerPlasmaBall(ItemInfo* item, long type, Vector3Int* pos1, short roomNumber, short angle)
	{

	}

	static int KnockBackCollision()
	{
		return 0;
	}

	static void ExplodeLondonBoss(ItemInfo* item)
	{

	}

	static void LondonBossDie(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Collidable = false;
		item->HitPoints = NOT_TARGETABLE;

		KillItem(itemNumber);
		DisableEntityAI(itemNumber);

		item->Flags |= IFLAG_INVISIBLE;
	}

	void ControlLaserBolts(short itemNumber)
	{

	}

	void ControlLondBossPlasmaBall(short fxNumber)
	{

	}

	void InitialiseLondonBoss(short itemNumber)
	{

	}

	void LondonBossControl(short itemNumber)
	{

	}

	void S_DrawLondonBoss(ItemInfo* item)
	{
		DrawAnimatingItem(item);

		if (BossData.ExplodeCount)
		{
			/*code*/
		}
		else
		{
			/*code*/
		}

		if (item->HitPoints <= 0 && BossData.ExplodeCount == 0)
		{
			/*code*/
		}
	}
}
