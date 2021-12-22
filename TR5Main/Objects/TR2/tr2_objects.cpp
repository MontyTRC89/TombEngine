#include "framework.h"
#include "tr2_objects.h"
/// entities
#include "tr2_barracuda.h" // OK
#include "tr2_birdmonster.h" // OK
#include "tr2_dragon.h" // OK
#include "tr2_eagle_or_crow.h" // OK
#include "tr2_knifethrower.h" // OK
#include "tr2_mercenary.h" // OK
#include "tr2_monk.h" // OK
#include "tr2_rat.h" // OK
#include "tr2_shark.h" // OK
#include "tr2_silencer.h" // OK
#include "tr2_skidman.h" // OK
#include "tr2_spear_guardian.h" // OK
#include "tr2_spider.h" // OK
#include "tr2_sword_guardian.h" // OK
#include "tr2_worker_dualrevolver.h" // OK
#include "tr2_worker_flamethrower.h" // OK
#include "tr2_worker_machinegun.h" // OK
#include "tr2_worker_shotgun.h" // OK
#include "tr2_yeti.h" // OK
/// objects

/// trap
#include "tr2_spinningblade.h"
#include "tr2_springboard.h"
#include "tr2_killerstatue.h"
/// vehicles
#include "boat.h"
#include "snowmobile.h"
/// necessary import
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"

static void StartBaddy(OBJECT_INFO* obj)
{
	obj = &Objects[ID_SHARK];
	if (obj->loaded)
	{
		obj->control = SharkControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 30;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 200;
		obj->radius = 340;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->zoneType = ZONE_WATER;

		g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_BARRACUDA];
	if (obj->loaded)
	{
		obj->control = BarracudaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 12;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->zoneType = ZONE_WATER;

		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_EAGLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		obj->zoneType = ZONE_FLYER;
	}

	obj = &Objects[ID_CROW];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->hitPoints = 15;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		obj->zoneType = ZONE_FLYER;
	}

	obj = &Objects[ID_RAT];
	if (obj->loaded)
	{
		obj->control = RatControl;
		obj->collision = CreatureCollision;
		obj->hitPoints = 5;
		obj->hitEffect = HIT_BLOOD;
		obj->shadowSize = 128;
		obj->pivotLength = 50;
		obj->radius = 204;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_YETI];
	if (obj->loaded)
	{
		obj->initialise = InitialiseYeti;
		obj->collision = CreatureCollision;
		obj->control = YetiControl;
		obj->hitPoints = 30;
		obj->hitEffect = HIT_BLOOD;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->radius = 128;
		obj->pivotLength = 100;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		obj->zoneType = ZONE_HUMAN_CLASSIC;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_Y);
		g_Level.Bones[obj->boneIndex + 14 * 4] |= (ROT_Y);
	}

	obj = &Objects[ID_GOON_SILENCER1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 25;
		obj->hitEffect = HIT_BLOOD;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 0] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 1 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_GOON_SILENCER2];
	if (obj->loaded)
	{
		if (Objects[ID_GOON_SILENCER1].loaded)
		{
			obj->animIndex = Objects[ID_GOON_SILENCER1].animIndex;
			obj->frameBase = Objects[ID_GOON_SILENCER1].frameBase;
		}
		else
		{
			TENLog("ID_GOON_SILENCER1 not found!", LogLevel::Error);
		}

		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 25;
		obj->hitEffect = HIT_BLOOD;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 0] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 1 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_GOON_SILENCER3];
	if (obj->loaded)
	{
		if (Objects[ID_GOON_SILENCER1].loaded)
		{
			obj->animIndex = Objects[ID_GOON_SILENCER1].animIndex;
			obj->frameBase = Objects[ID_GOON_SILENCER1].frameBase;
		}
		else
		{
			TENLog("ID_GOON_SILENCER1 not found!", LogLevel::Error);
		}

		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 25;
		obj->hitEffect = HIT_BLOOD;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 0] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 1 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_WORKER_SHOTGUN];
	if (obj->loaded)
	{
		obj->biteOffset = 0;
		obj->initialise = InitialiseWorkerShotgun;
		obj->collision = CreatureCollision;
		obj->control = WorkerShotgunControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 25;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		//g_Level.Bones[obj->boneIndex + 5*4] |= (ROT_X | ROT_Y);
		//g_Level.Bones[obj->boneIndex + 14*4] |= (ROT_X | ROT_Y);
		// TODO: get the correct torso and head Bones value and assign ROT_X and ROT_Y to it !
	}

	obj = &Objects[ID_WORKER_MACHINEGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWorkerMachineGun;
		obj->collision = CreatureCollision;
		obj->control = WorkerMachineGunControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		//g_Level.Bones[obj->boneIndex + 5*4] |= (ROT_X | ROT_Y);
		//g_Level.Bones[obj->boneIndex + 14*4] |= (ROT_X | ROT_Y);
		// TODO: get the correct torso and head Bones value and assign ROT_X and ROT_Y to it !
	}

	obj = &Objects[ID_SMALL_SPIDER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SmallSpiderControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 5;
		obj->hitEffect = HIT_SMOKE;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_BIG_SPIDER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = BigSpiderControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 40;
		obj->hitEffect = HIT_SMOKE;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_WORKER_DUAL_REVOLVER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = WorkerDualGunControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 150;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 11 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 0 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_BIRDMONSTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = BirdMonsterControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 200;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 341;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 14 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_WORKER_FLAMETHROWER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWorkerFlamethrower;
		obj->collision = CreatureCollision;
		obj->control = WorkerFlamethrower;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 4 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 14 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_KNIFETHROWER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = KnifethrowerControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 60;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		//g_Level.Bones[obj->boneIndex + 8 * 4] |= (ROT_X | ROT_Y);
		//g_Level.Bones[obj->boneIndex + 0 * 4] |= (ROT_X | ROT_Y);
		// TODO: find the correct for Bones (knifethrower).
	}

	obj = &Objects[ID_KNIFETHROWER_KNIFE];
	if (obj->loaded)
		obj->control = KnifeControl;

	obj = &Objects[ID_MERCENARY_UZI];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MercenaryUziControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 45;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 8 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_MERCENARY_AUTOPISTOLS1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MercenaryAutoPistolControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 8 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_MERCENARY_AUTOPISTOLS2];
	if (obj->loaded)
	{
		if (Objects[ID_MERCENARY_AUTOPISTOLS1].loaded)
		{
			obj->animIndex = Objects[ID_MERCENARY_AUTOPISTOLS1].animIndex;
			obj->frameBase = Objects[ID_MERCENARY_AUTOPISTOLS1].frameBase;
		}
		else
		{
			TENLog("ID_MERCENARY_AUTOPISTOLS1 not found!", LogLevel::Error);
		}

		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MercenaryAutoPistolControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 8 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_MONK1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MonkControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_MONK2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MonkControl;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 50;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_SWORD_GUARDIAN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSwordGuardian;
		obj->collision = CreatureCollision;
		obj->control = SwordGuardianControl;
		//obj->drawRoutine = DrawStatue;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 80;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 16 * 4] |= (ROT_X | ROT_Y);
		// TODO: Bones value is not correct (shiva) !
		// need the correct one.
	}

	obj = &Objects[ID_SPEAR_GUARDIAN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpearGuardian;
		obj->collision = CreatureCollision;
		obj->control = SpearGuardianControl;
		//obj->drawRoutine = DrawStatue;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		//g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		//g_Level.Bones[obj->boneIndex + 12 * 4] |= (ROT_X | ROT_Y);
		// TODO: get the correct id for Bones ! (spear)
	}

	obj = &Objects[ID_DRAGON_FRONT];
	if (obj->loaded)
	{
		if (!Objects[ID_DRAGON_BACK].loaded)
			TENLog("ID_DRAGON_FRONT needs ID_DRAGON_BACK!", LogLevel::Error);

		obj->collision = DragonCollision;
		obj->control = DragonControl;
		obj->hitPoints = 300;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 300;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Z;
	}

	obj = &Objects[ID_DRAGON_BACK];
	if (obj->loaded)
	{
		if (!Objects[ID_MARCO_BARTOLI].loaded)
			TENLog("ID_DRAGON_BACK needs ID_MARCO_BARTOLI!", LogLevel::Error);

		obj->collision = DragonCollision;
		obj->control = DragonControl;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 256;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_MARCO_BARTOLI];
	if (obj->loaded)
	{
		obj->initialise = InitialiseBartoli;
		obj->control = BartoliControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SNOWMOBILE_GUN];
	if (obj->loaded)
	{
		obj->collision = SkidManCollision;
		//obj->drawRoutine = DrawSkidoo; // TODO: recreate renderer for skidoo
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->hitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_SNOWMOBILE_DRIVER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkidman;
		obj->control = SkidManControl;
		obj->hitPoints = 1;
		obj->hitEffect = HIT_BLOOD;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}
}

static void StartObject(OBJECT_INFO* obj)
{
	
}

static void StartTrap(OBJECT_INFO* obj)
{
	obj = &Objects[ID_ROLLING_SPINDLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpinningBlade;
		obj->control = SpinningBladeControl;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_SPRINGBOARD];
	if (obj->loaded)
	{
		obj->control = SpringBoardControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}
}

// boat, snowmobile, snowmobile gun
static void StartVehicles(OBJECT_INFO* obj)
{
	// TODO: fix BoatControl() not using int BoatControl(void)
	obj = &Objects[ID_SPEEDBOAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpeedBoat;
		obj->collision = SpeedBoatCollision;
		obj->control = SpeedBoatControl;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_RICOCHET;
	}

	obj = &Objects[ID_SNOWMOBILE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkidoo;
		obj->collision = SkidooCollision;
		//obj->drawRoutine = DrawSkidoo; // TODO: create a new render for the skidoo. (with track animated)
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->hitEffect = HIT_RICOCHET;
	}
}

static OBJECT_INFO* objToInit;
void InitialiseTR2Objects()
{
	StartBaddy(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartVehicles(objToInit);
}
