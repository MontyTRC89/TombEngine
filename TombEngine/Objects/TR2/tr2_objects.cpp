#include "framework.h"
#include "Objects/TR2/tr2_objects.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/level.h"
#include "Specific/setup.h"

// Creatures
#include "Objects/TR2/Entity/tr2_barracuda.h" // OK
#include "Objects/TR2/Entity/tr2_bird_monster.h" // OK
#include "Objects/TR2/Entity/tr2_dragon.h" // OK
#include "Objects/TR2/Entity/tr2_eagle_or_crow.h" // OK
#include "Objects/TR2/Entity/tr2_knife_thrower.h" // OK
#include "Objects/TR2/Entity/tr2_mercenary.h" // OK
#include "Objects/TR2/Entity/tr2_monk.h" // OK
#include "Objects/TR2/Entity/tr2_rat.h" // OK
#include "Objects/TR2/Entity/tr2_shark.h" // OK
#include "Objects/TR2/Entity/tr2_silencer.h" // OK
#include "Objects/TR2/Entity/tr2_skidman.h" // OK
#include "Objects/TR2/Entity/tr2_spear_guardian.h" // OK
#include "Objects/TR2/Entity/tr2_spider.h" // OK
#include "Objects/TR2/Entity/tr2_sword_guardian.h" // OK
#include "Objects/TR2/Entity/tr2_worker_dualrevolver.h" // OK
#include "Objects/TR2/Entity/tr2_worker_flamethrower.h" // OK
#include "Objects/TR2/Entity/tr2_worker_machinegun.h" // OK
#include "Objects/TR2/Entity/tr2_worker_shotgun.h" // OK
#include "Objects/TR2/Entity/tr2_yeti.h" // OK

// Traps
#include "Objects/TR2/Trap/tr2_spinningblade.h"
#include "Objects/TR2/Trap/tr2_springboard.h"
#include "Objects/TR2/Trap/tr2_killerstatue.h"

// Vehicles
#include "Objects/TR2/Vehicles/Speedboat.h"
#include "Objects/TR2/Vehicles/Skidoo.h"

using namespace TEN::Entities::Creatures::TR2;

static void StartEntity(ObjectInfo* obj)
{
	obj = &Objects[ID_SHARK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = SharkControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 30;
		obj->pivotLength = 200;
		obj->radius = 340;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->LotType = LotType::Water;
		obj->SetBoneRotationFlags(9, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BARRACUDA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = BarracudaControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 12;
		obj->pivotLength = 200;
		obj->radius = 204;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->LotType = LotType::Water;
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_EAGLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->radius = 204;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->LotType = LotType::Flyer;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_CROW];
	if (obj->loaded)
	{
		obj->initialise = InitialiseEagle;
		obj->control = EagleControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 15;
		obj->radius = 204;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->LotType = LotType::Flyer;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_RAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = RatControl;
		obj->collision = CreatureCollision;
		obj->HitPoints = 5;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 50;
		obj->radius = 204;
		obj->intelligent = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_YETI];
	if (obj->loaded)
	{
		obj->initialise = InitialiseYeti;
		obj->collision = CreatureCollision;
		obj->control = YetiControl;
		obj->HitPoints = 30;
		obj->shadowType = ShadowMode::All;
		obj->radius = 128;
		obj->pivotLength = 100;
		obj->intelligent = true;
		obj->LotType = LotType::Human;
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetBoneRotationFlags(14, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GOON_SILENCER1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 25;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GOON_SILENCER2];
	if (obj->loaded)
	{
		AssignObjectAnimations(*obj, ID_GOON_SILENCER1, "ID_GOON_SILENCER2", "ID_GOON_SILENCER1");
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 25;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_GOON_SILENCER3];
	if (obj->loaded)
	{
		AssignObjectAnimations(*obj, ID_GOON_SILENCER1, "ID_GOON_SILENCER3", "ID_GOON_SILENCER1");
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SilencerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 25;
		obj->biteOffset = 0;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(1, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WORKER_SHOTGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWorkerShotgun;
		obj->collision = CreatureCollision;
		obj->control = WorkerShotgunControl;
		obj->shadowType = ShadowMode::All;
		obj->biteOffset = 0;
		obj->HitPoints = 25;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WORKER_MACHINEGUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWorkerMachineGun;
		obj->collision = CreatureCollision;
		obj->control = WorkerMachineGunControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SMALL_SPIDER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = SmallSpiderControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 5;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->LotType = LotType::Spider;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BIG_SPIDER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = BigSpiderControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 40;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WORKER_DUAL_REVOLVER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = WorkerDualGunControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 150;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(11, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_BIRDMONSTER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = BirdMonsterControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 200;
		obj->pivotLength = 0;
		obj->radius = 341;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(14, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_WORKER_FLAMETHROWER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseWorkerFlamethrower;
		obj->collision = CreatureCollision;
		obj->control = WorkerFlamethrower;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(4, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(14, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_KNIFETHROWER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = KnifeThrowerControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 60;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
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
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 45;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MERCENARY_AUTOPISTOLS1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MercenaryAutoPistolControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 50;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MERCENARY_AUTOPISTOLS2];
	if (obj->loaded)
	{
		AssignObjectAnimations(*obj, ID_MERCENARY_AUTOPISTOLS1, "ID_MERCENARY_AUTOPISTOLS2", "ID_MERCENARY_AUTOPISTOLS1");
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MercenaryAutoPistolControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 50;
		obj->pivotLength = 0;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(8, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MONK1];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MonkControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 50;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MONK2];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->collision = CreatureCollision;
		obj->control = MonkControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 50;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SWORD_GUARDIAN];
	if (obj->loaded)
	{
		CheckIfSlotExists(ID_SWORD_GUARDIAN_STATUE, "ID_SWORD_GUARDIAN", "ID_SWORD_GUARDIAN_STATUE");
		obj->initialise = InitialiseSwordGuardian;
		obj->collision = CreatureCollision;
		obj->control = SwordGuardianControl;
		obj->HitRoutine = SwordGuardianHit;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 80;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(17, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SPEAR_GUARDIAN];
	if (obj->loaded)
	{
		CheckIfSlotExists(ID_SPEAR_GUARDIAN_STATUE, "ID_SPEAR_GUARDIAN", "ID_SPEAR_GUARDIAN_STATUE");
		obj->initialise = InitialiseSpearGuardian;
		obj->collision = CreatureCollision;
		obj->control = SpearGuardianControl;
		obj->HitRoutine = SpearGuardianHit;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(12, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_DRAGON_FRONT];
	if (obj->loaded)
	{
		CheckIfSlotExists(ID_DRAGON_BACK, "ID_DRAGON_FRONT", "ID_DRAGON_BACK");
		obj->initialise = InitialiseCreature;
		obj->collision = DragonCollision;
		obj->control = DragonControl;
		obj->HitPoints = 300;
		obj->pivotLength = 300;
		obj->radius = 256;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(10, ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_DRAGON_BACK];
	if (obj->loaded)
	{
		CheckIfSlotExists(ID_MARCO_BARTOLI, "ID_DRAGON_BACK", "ID_MARCO_BARTOLI");
		obj->initialise = InitialiseCreature;
		obj->collision = DragonCollision;
		obj->control = DragonControl;
		obj->radius = 256;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MARCO_BARTOLI];
	if (obj->loaded)
	{
		CheckIfSlotExists(ID_DRAGON_BACK, "ID_MARCO_BARTOLI", "ID_DRAGON_BACK");
		obj->initialise = InitialiseBartoli;
		obj->control = BartoliControl;
	}

	// TODO: Recreate renderer for skidoo.
	obj = &Objects[ID_SNOWMOBILE_GUN];
	if (obj->loaded)
	{
		obj->collision = SkidooManCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->pivotLength = 0;
		obj->radius = 256;
		obj->intelligent = true;
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SNOWMOBILE_DRIVER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkidooMan;
		obj->control = SkidooManControl;
		obj->HitPoints = 1;
		obj->SetupHitEffect(true);
	}
}

static void StartObject(ObjectInfo* obj)
{
	
}

static void StartTrap(ObjectInfo* obj)
{
	obj = &Objects[ID_ROLLING_SPINDLE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpinningBlade;
		obj->control = SpinningBladeControl;
		obj->collision = ObjectCollision;
	}

	obj = &Objects[ID_SPRINGBOARD];
	if (obj->loaded)
	{
		obj->control = SpringBoardControl;
	}

	obj = &Objects[ID_STATUE_WITH_BLADE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseKillerStatue;
		obj->control = KillerStatueControl;
		obj->collision = ObjectCollision;
		obj->SetupHitEffect(true);
	}
}

// boat, snowmobile, snowmobile gun
static void StartVehicles(ObjectInfo* obj)
{
	// TODO: Fix BoatControl() not using BoatControl().
	obj = &Objects[ID_SPEEDBOAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSpeedboat;
		obj->collision = SpeedboatPlayerCollision;
		obj->control = SpeedboatControl;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);
	}

	// TODO: Create a new renderer for the skidoo with animated track.
	obj = &Objects[ID_SNOWMOBILE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseSkidoo;
		obj->collision = SkidooPlayerCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);
	}
}

void InitialiseTR2Objects()
{
	ObjectInfo* objectPtr = nullptr;
	StartEntity(objectPtr);
	StartObject(objectPtr);
	StartTrap(objectPtr);
	StartVehicles(objectPtr);
}
