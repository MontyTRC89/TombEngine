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
		SOPHIALEIGH_EMPTY = 0,
		SOPHIALEIGH_STAND = 1,
		SOPHIALEIGH_WALK = 2,
		SOPHIALEIGH_RUN = 3,
		SOPHIALEIGH_SUMMON = 4,
		SOPHIALEIGH_BIGZAP = 5,
		SOPHIALEIGH_DEATH = 6,
		SOPHIALEIGH_LAUGH = 7,
		SOPHIALEIGH_LILZAP = 8,
		SOPHIALEIGH_VAULT2 = 9,
		SOPHIALEIGH_VAULT3 = 10,
		SOPHIALEIGH_VAULT4 = 11,
		SOPHIALEIGH_GODOWN = 12,
	};

	// TODO
	enum SophiaAnim
	{

	};

#define	SOPHIALEIGH_VAULT2_ANIM 9
#define	SOPHIALEIGH_VAULT3_ANIM 18
#define	SOPHIALEIGH_VAULT4_ANIM 15
#define	SOPHIALEIGH_GODOWN_ANIM 21
#define	SOPHIALEIGH_STND2SUM_ANIM 1
#define SOPHIALEIGH_SUMMON_ANIM 2
#define	SOPHIALEIGH_GODOWN_ANIM 21

#define SOPHIALEIGH_VAULT_SHIFT 96
#define SOPHIALEIGH_AWARE_DISTANCE pow(SECTOR(1), 2)
#define SOPHIALEIGH_WALK_TURN ANGLE(4)
#define SOPHIALEIGH_RUN_TURN ANGLE(7)
#define SOPHIALEIGH_WALK_RANGE pow(SECTOR(1), 2)
#define SOPHIALEIGH_WALK_CHANCE 0x100
#define SOPHIALEIGH_LAUGH_CHANCE 0x100
#define SOPHIALEIGH_TURN ANGLE(2.0f)
#define SOPHIALEIGH_DIE_ANIM 17
#define SOPHIALEIGH_FINAL_HEIGHT -11776
#define BIGZAP_TIMER 600

	static void TriggerLaserBolt(Vector3i* pos, ItemInfo* item, long type, short yAngle)
	{

	}

	static void TriggerPlasmaBallFlame(short fxNumber, long type, long xv, long yv, long zv)
	{

	}

	static void TriggerPlasmaBall(ItemInfo* item, long type, Vector3i* pos1, short roomNumber, short angle)
	{

	}

	static int KnockBackCollision()
	{
		return 0;
	}

	static void ExplodeSophiaLeigh(ItemInfo* item)
	{

	}

	static void SophiaLeighDie(short itemNumber)
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

	void InitialiseSophiaLeigh(short itemNumber)
	{

	}

	void SophiaLeighControl(short itemNumber)
	{

	}

	void S_DrawSophiaLeigh(ItemInfo* item)
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
