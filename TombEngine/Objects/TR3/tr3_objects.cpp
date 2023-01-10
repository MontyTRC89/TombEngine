#include "framework.h"
#include "Objects/TR3/tr3_objects.h"

#include "Objects/TR5/Object/tr5_missile.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Specific/level.h"
#include "Specific/setup.h"

// Creatures
#include "Objects/TR3/Entity/tr3_civvy.h" // OK
#include "Objects/TR3/Entity/tr3_cobra.h" // OK
#include "Objects/TR3/Entity/tr3_fish_emitter.h" // OK
#include "Objects/TR3/Entity/tr3_flamethrower.h" // OK
#include "Objects/TR3/Entity/tr3_monkey.h" // OK
#include "Objects/TR3/Entity/tr3_mp_gun.h" // OK
#include "Objects/TR3/Entity/tr3_mp_stick.h" // OK
#include "Objects/TR3/Entity/tr3_raptor.h" // OK
#include "Objects/TR3/Entity/tr3_scuba_diver.h" // OK
#include "Objects/TR3/Entity/tr3_shiva.h" // OK
#include "Objects/TR3/Entity/tr3_sophia.h" // OK
#include "Objects/TR3/Entity/tr3_tiger.h" // OK
#include "Objects/TR3/Entity/tr3_tony.h" // OK
#include "Objects/TR3/Entity/tr3_trex.h" // OK
#include "Objects/TR3/Entity/tr3_tribesman.h" // OK
#include "Objects/TR3/Entity/Lizard.h" // OK
#include "Objects/TR3/Entity/PunaBoss.h" // OK

// Object
#include "Objects/TR3/Object/tr3_boss_object.h"

// Traps
#include "Objects/TR3/Trap/train.h"

// Vehicles
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR3/Vehicles/rubber_boat.h"
#include "Objects/Utils/object_helper.h"

using namespace TEN::Entities::Creatures::TR3;
using namespace TEN::Effects::Boss;

static void StartEntity(ObjectInfo* obj)
{
	obj = &Objects[ID_TONY_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTony;
		obj->collision = CreatureCollision;
		obj->control = TonyControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_TIGER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 24;
		obj->pivotLength = 200;
		obj->radius = 340;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(21, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_COBRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCobra;
		obj->control = CobraControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 8;
		obj->radius = 102;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->SetBoneRotationFlags(0, ROT_Y);
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_RAPTOR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = RaptorControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->radius = 341;
		obj->pivotLength = 600;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(20, ROT_Y);
		obj->SetBoneRotationFlags(21, ROT_Y);
		obj->SetBoneRotationFlags(23, ROT_Y);
		obj->SetBoneRotationFlags(25, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_TRIBESMAN_WITH_AX];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = TribemanAxeControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_TRIBESMAN_WITH_DARTS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = TribemanDartsControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->SetBoneRotationFlags(6, ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_TYRANNOSAUR];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = TRexControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 800;
		obj->pivotLength = 1800;
		obj->radius = 512;
		obj->intelligent = true;
		obj->ZoneType = ZoneType::Blockable;
		obj->SetBoneRotationFlags(10, ROT_Y);
		obj->SetBoneRotationFlags(11, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SCUBA_DIVER];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = ScubaControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 20;
		obj->radius = 340;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->pivotLength = 50;
		obj->ZoneType = ZoneType::Water;
		obj->SetBoneRotationFlags(10, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(14, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SCUBA_HARPOON];
	if (obj->loaded)
	{
		obj->control = ScubaHarpoonControl;
		obj->collision = ObjectCollision;
	}

	obj = &Objects[ID_FLAMETHROWER_BADDY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = FlameThrowerControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 36;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(7, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MONKEY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMonkey;
		obj->control = MonkeyControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 8;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(7, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MP_WITH_GUN];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = MPGunControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->biteOffset = 0;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_MP_WITH_STICK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMPStick;
		obj->control = MPStickControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 28;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SHIVA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseShiva;
		obj->collision = CreatureCollision;
		obj->control = ShivaControl;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 100;
		obj->pivotLength = 0;
		obj->radius = 256;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(25, ROT_X | ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_SOPHIA_LEE_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLondonBoss;
		obj->collision = CreatureCollision;
		obj->control = LondonBossControl;
		obj->drawRoutine = S_DrawLondonBoss;
		obj->shadowType = ShadowMode::All;
		obj->pivotLength = 50;
		obj->HitPoints = 300;
		obj->radius = 102;
		obj->intelligent = true;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_CIVVIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCivvy;
		obj->control = CivvyControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 15;
		obj->radius = 102;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
		obj->SetBoneRotationFlags(13, ROT_Y);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_LIZARD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCreature;
		obj->control = LizardControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 36;
		obj->intelligent = true;
		obj->pivotLength = 0;
		obj->radius = 204;
		obj->ZoneType = ZoneType::Human;
		obj->SetBoneRotationFlags(9, ROT_X | ROT_Z);
		obj->SetupHitEffect();
	}

	obj = &Objects[ID_PUNA_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialisePuna;
		obj->control = PunaControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = 200;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->radius = 102;
		obj->pivotLength = 50;
		obj->SetBoneRotationFlags(4, ROT_Y);		 // Puna quest object.
		obj->SetBoneRotationFlags(7, ROT_X | ROT_Y); // Head.
		obj->SetupHitEffect();
	}
}

static void StartObject(ObjectInfo* obj)
{
	obj = &Objects[ID_BOSS_SHIELD];
	if (obj->loaded)
	{
		obj->initialise = nullptr;
		obj->collision = ObjectCollision;
		obj->control = ShieldControl;
		obj->shadowType = ShadowMode::None;
	}

	obj = &Objects[ID_BOSS_EXPLOSION_RING];
	if (obj->loaded)
	{
		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = ShockwaveRingControl;
		obj->shadowType = ShadowMode::None;
	}

	obj = &Objects[ID_BOSS_EXPLOSION_SHOCKWAVE];
	if (obj->loaded)
	{
		obj->initialise = nullptr;
		obj->collision = nullptr;
		obj->control = ShockwaveExplosionControl;
		obj->shadowType = ShadowMode::None;
	}
}

static void StartTrap(ObjectInfo* obj)
{
	obj = &Objects[ID_TRAIN];
	if (obj->loaded)
	{
		obj->control = TrainControl;
		obj->collision = TrainCollision;
		obj->SetupHitEffect(true);
	}
}

static void StartVehicles(ObjectInfo* obj)
{
	obj = &Objects[ID_QUAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseQuadBike;
		obj->collision = QuadBikePlayerCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_RUBBER_BOAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRubberBoat;
		obj->control = RubberBoatControl;
		obj->collision = RubberBoatPlayerCollision;
		obj->drawRoutine = DrawRubberBoat;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);

	}

	obj = &Objects[ID_KAYAK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseKayak;
		obj->collision = KayakPlayerCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);

	}

	obj = &Objects[ID_MINECART];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMinecart;
		obj->collision = MinecartPlayerCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);

	}

	obj = &Objects[ID_BIGGUN];
	if (obj->loaded)
	{
		obj->initialise = BigGunInitialise;
		obj->collision = BigGunCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);
	}

	obj = &Objects[ID_UPV];
	if (obj->loaded)
	{
		obj->initialise = UPVInitialise;
		obj->control = UPVEffects;
		obj->collision = UPVPlayerCollision;
		obj->shadowType = ShadowMode::Lara;
		obj->SetupHitEffect(true);
	}
}

static void StartProjectiles(ObjectInfo* obj)
{
	obj = &Objects[ID_TONY_BOSS_FLAME];
	obj->control = ControlTonyFireBall;
	obj->drawRoutine = nullptr;
}

void InitialiseTR3Objects()
{
	ObjectInfo* objectPtr = nullptr;
	StartEntity(objectPtr);
	StartObject(objectPtr);
	StartTrap(objectPtr);
	StartVehicles(objectPtr);
	StartProjectiles(objectPtr);
}
