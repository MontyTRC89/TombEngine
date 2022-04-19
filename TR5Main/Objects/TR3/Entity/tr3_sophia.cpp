#include "framework.h"
#include "Objects/TR3/Entity/tr3_sophia.h"

#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Objects/TR3/boss.h"
#include "Sound/sound.h"
#include "Specific/level.h"

static BOSS_STRUCT BossData;

#define MAX_TRIGGER_RANGE 0x4000
#define SMALL_FLASH 10
#define BIG_FLASH 16
#define BOLT_SPEED 384

enum { NOTHING, SUMMONING, KNOCKBACK };
enum { NORMAL_BOLT, LARGE_BOLT, SUMMON_BOLT };
enum { RIGHT_PRONG, ICONPOS, LEFT_PRONG };
enum { ATTACK_HEAD, ATTACK_HAND1, ATTACK_HAND2 };

enum londonboss_state
{
	LONDONBOSS_EMPTY,
	LONDONBOSS_STAND,
	LONDONBOSS_WALK,
	LONDONBOSS_RUN,
	LONDONBOSS_SUMMON,
	LONDONBOSS_BIGZAP,
	LONDONBOSS_DEATH,
	LONDONBOSS_LAUGH,
	LONDONBOSS_LILZAP,
	LONDONBOSS_VAULT2,
	LONDONBOSS_VAULT3,
	LONDONBOSS_VAULT4,
	LONDONBOSS_GODOWN
};

#define	LONDONBOSS_VAULT2_ANIM 9
#define	LONDONBOSS_VAULT3_ANIM 18
#define	LONDONBOSS_VAULT4_ANIM 15
#define	LONDONBOSS_GODOWN_ANIM 21
#define	LONDONBOSS_STND2SUM_ANIM 1
#define LONDONBOSS_SUMMON_ANIM 2
#define	LONDONBOSS_GODOWN_ANIM 21
#define LONDONBOSS_VAULT_SHIFT 96
#define LONDONBOSS_AWARE_DISTANCE SQUARE(WALL_SIZE)
#define LONDONBOSS_WALK_TURN ANGLE(4)
#define LONDONBOSS_RUN_TURN ANGLE(7)
#define LONDONBOSS_WALK_RANGE SQUARE(WALL_SIZE)
#define LONDONBOSS_WALK_CHANCE 0x100
#define LONDONBOSS_LAUGH_CHANCE 0x100
#define LONDONBOSS_TURN ANGLE(2.0f)
#define LONDONBOSS_DIE_ANIM 17
#define LONDONBOSS_FINAL_HEIGHT -11776
#define BIGZAP_TIMER 600

static void TriggerLaserBolt(PHD_VECTOR* pos, ITEM_INFO* item, long type, short yang)
{
	
}

static void TriggerPlasmaBallFlame(short fx_number, long type, long xv, long yv, long zv)
{
	
}

static void TriggerPlasmaBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, short roomNumber, short angle)
{
	
}

static int KnockBackCollision()
{
	return 0;
}

static void ExplodeLondonBoss(ITEM_INFO* item)
{
	
}

static void LondonBossDie(short item_number)
{
	ITEM_INFO* item;
	item = &g_Level.Items[item_number];
	item->collidable = false;
	item->hitPoints = -16384;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;
}

void ControlLaserBolts(short item_number)
{
	
}

void ControlLondBossPlasmaBall(short fx_number)
{
	
}

void InitialiseLondonBoss(short item_number)
{
	
}

void LondonBossControl(short item_number)
{
	
}

void S_DrawLondonBoss(ITEM_INFO* item)
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

	if (item->hitPoints <= 0 && BossData.ExplodeCount == 0)
	{
		/*code*/
	}
}