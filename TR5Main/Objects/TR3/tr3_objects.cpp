#include "framework.h"
#include "Objects/TR3/tr3_objects.h"

/// entities
#include "Objects/TR3/Entity/tr3_civvy.h" // OK
#include "Objects/TR3/Entity/tr3_cobra.h" // OK
#include "Objects/TR3/Entity/tr3_fishemitter.h" // OK
#include "Objects/TR3/Entity/tr3_flamethrower.h" // OK
#include "Objects/TR3/Entity/tr3_monkey.h" // OK
#include "Objects/TR3/Entity/tr3_mpgun.h" // OK
#include "Objects/TR3/Entity/tr3_mpstick.h" // OK
#include "Objects/TR3/Entity/tr3_raptor.h" // OK
#include "Objects/TR3/Entity/tr3_scuba.h" // OK
#include "Objects/TR3/Entity/tr3_shiva.h" // OK
#include "Objects/TR3/Entity/tr3_sophia.h" // OK
#include "Objects/TR3/Entity/tr3_tiger.h" // OK
#include "Objects/TR3/Entity/tr3_tony.h" // OK
#include "Objects/TR3/Entity/tr3_trex.h" // OK
#include "Objects/TR3/Entity/tr3_tribesman.h" // OK
/// traps
#include "Objects/TR3/Trap/train.h"
/// vehicles
#include "Objects/TR3/Vehicles/biggun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR3/Vehicles/rubberboat.h"
/// necessary import
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/itemdata/creature_info.h"
#include "Specific/setup.h"
#include "Specific/level.h"

static void StartBaddy(OBJECT_INFO* obj)
{
	obj = &Objects[ID_TONY_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseTony;
		obj->collision = CreatureCollision;
		obj->control = TonyControl;
		obj->drawRoutine = S_DrawTonyBoss;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 50;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TIGER];
	if (obj->loaded)
	{
		obj->control = TigerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->HitPoints = 24;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 200;
		obj->radius = 340;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		g_Level.Bones[obj->boneIndex + 21 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_COBRA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCobra;
		obj->control = CobraControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->HitPoints = 8;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->nonLot = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		g_Level.Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_RAPTOR];
	if (obj->loaded)
	{
		obj->control = RaptorControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 341;
		obj->pivotLength = 600;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		g_Level.Bones[obj->boneIndex + 20 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 21 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 23 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 25 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TRIBESMAN_WITH_AX];
	if (obj->loaded)
	{
		obj->control = TribemanAxeControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->HitPoints = 28;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TRIBESMAN_WITH_DARTS];
	if (obj->loaded)
	{
		obj->control = TribemanDartsControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = 128;
		obj->HitPoints = 28;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_TYRANNOSAUR];
	if (obj->loaded)
	{
		obj->control = TyrannosaurControl;
		obj->collision = CreatureCollision;
		obj->HitPoints = 800;
		obj->hitEffect = HIT_BLOOD;
		obj->shadowSize = 64;
		obj->pivotLength = 1800;
		obj->radius = 512;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;

		g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 11 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SCUBA_DIVER];
	if (obj->loaded)
	{
		obj->control = ScubaControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 20;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 340;
		obj->intelligent = true;
		obj->waterCreature = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 50;
		obj->zoneType = ZONE_WATER;

		g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 14 * 4] |= ROT_Z;
	}

	obj = &Objects[ID_SCUBA_HARPOON];
	if (obj->loaded)
	{
		obj->control = ScubaHarpoonControl;
		obj->collision = ObjectCollision;
		obj->savePosition = true;
	}

	obj = &Objects[ID_FLAMETHROWER_BADDY];
	if (obj->loaded)
	{
		obj->control = FlameThrowerControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 36;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		g_Level.Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 0 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_MONKEY];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMonkey;
		obj->control = MonkeyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 8;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;

		g_Level.Bones[obj->boneIndex + 0 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 0 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_MP_WITH_GUN];
	if (obj->loaded)
	{
		obj->control = MPGunControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 28;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		obj->biteOffset = 0;

		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_MP_WITH_STICK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMPStick;
		obj->control = MPStickControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 28;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		obj->zoneType = ZONE_HUMAN_CLASSIC;

		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_SHIVA];
	if (obj->loaded)
	{
		obj->initialise = InitialiseShiva;
		obj->collision = CreatureCollision;
		obj->control = ShivaControl;
		//obj->drawRoutine = DrawStatue;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 100;
		obj->hitEffect = HIT_SMOKE;
		obj->pivotLength = 0;
		obj->radius = 256;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->saveHitpoints = true;
		obj->savePosition = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= (ROT_X | ROT_Y);
		g_Level.Bones[obj->boneIndex + 25 * 4] |= (ROT_X | ROT_Y);
	}

	obj = &Objects[ID_SOPHIA_LEE_BOSS];
	if (obj->loaded)
	{
		obj->initialise = InitialiseLondonBoss;
		obj->collision = CreatureCollision;
		obj->control = LondonBossControl;
		obj->drawRoutine = S_DrawLondonBoss;
		obj->shadowSize = 0;
		obj->pivotLength = 50;
		obj->HitPoints = 300;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}

	obj = &Objects[ID_CIVVIE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseCivvy;
		obj->control = CivvyControl;
		obj->collision = CreatureCollision;
		obj->shadowSize = UNIT_SHADOW / 2;
		obj->HitPoints = 15;
		obj->hitEffect = HIT_BLOOD;
		obj->radius = 102;
		obj->intelligent = true;
		obj->savePosition = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->pivotLength = 0;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y;
		g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X;
		g_Level.Bones[obj->boneIndex + 13 * 4] |= ROT_Y;
	}
}

static void StartObject(OBJECT_INFO* obj)
{

}

static void StartTrap(OBJECT_INFO* obj)
{
	obj = &Objects[ID_TRAIN];
	if (obj->loaded)
	{
		obj->control = TrainControl;
		obj->collision = TrainCollision;
		obj->hitEffect = HIT_RICOCHET;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}
}

static void StartVehicles(OBJECT_INFO* obj)
{
	obj = &Objects[ID_QUAD];
	if (obj->loaded)
	{
		obj->initialise = InitialiseQuadBike;
		obj->collision = QuadBikeCollision;
		obj->hitEffect = HIT_RICOCHET;
		obj->savePosition = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
	}

	obj = &Objects[ID_RUBBER_BOAT];
	if (obj->loaded)
	{
		obj->initialise = InitialiseRubberBoat;
		obj->control = RubberBoatControl;
		obj->collision = RubberBoatCollision;
		obj->drawRoutine = DrawRubberBoat;
		obj->hitEffect = HIT_RICOCHET;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_KAYAK];
	if (obj->loaded)
	{
		obj->initialise = InitialiseKayak;
		obj->collision = KayakCollision;
		//obj->drawRoutine = KayakDraw;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_MINECART];
	if (obj->loaded)
	{
		obj->initialise = InitialiseMineCart;
		obj->collision = MineCartCollision;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}

	obj = &Objects[ID_BIGGUN];
	if (obj->loaded)
	{
		obj->initialise = BigGunInitialise;
		obj->collision = BigGunCollision;
//		obj->draw_routine = BigGunDraw;
		obj->hitEffect = HIT_RICOCHET;
		obj->savePosition = true;
		obj->saveFlags = true;
		obj->saveAnim = true;
	}

	obj = &Objects[ID_UPV];
	if (obj->loaded)
	{
		obj->initialise = SubInitialise;
		obj->control = SubEffects;
		obj->collision = SubCollision;
	//	obj->drawRoutine = SubDraw;
		obj->hitEffect = HIT_RICOCHET;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->savePosition = true;
	}
}

static void StartProjectiles(OBJECT_INFO* obj)
{
	obj = &Objects[ID_TONY_BOSS_FLAME];
	obj->control = ControlTonyFireBall;
	obj->drawRoutine = NULL;
}

static OBJECT_INFO* objToInit;
void InitialiseTR3Objects()
{
	StartBaddy(objToInit);
	StartObject(objToInit);
	StartTrap(objToInit);
	StartVehicles(objToInit);
	StartProjectiles(objToInit);
}
