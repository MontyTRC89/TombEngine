#include "framework.h"
#include "Game/Setup.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/Hair.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/pickup/pickup.h"
#include "Game/room.h"
#include "Objects/Effects/effect_objects.h"
#include "Objects/Generic/generic_objects.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Objects/Generic/Switches/switch.h"
#include "Objects/Generic/Traps/falling_block.h"
#include "Objects/TR1/tr1_objects.h"
#include "Objects/TR2/tr2_objects.h"
#include "Objects/TR3/tr3_objects.h"
#include "Objects/TR4/tr4_objects.h"
#include "Objects/TR5/tr5_objects.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/Utils/object_helper.h"
#include "Specific/level.h"

using namespace TEN::Effects::Hair;
using namespace TEN::Entities;
using namespace TEN::Entities::Switches;

ObjectInfo Objects[ID_NUMBER_OBJECTS];
STATIC_INFO StaticObjects[MAX_STATICS];

void InitializeGameFlags()
{
	ZeroMemory(FlipMap, MAX_FLIPMAP * sizeof(int));
	ZeroMemory(FlipStats, MAX_FLIPMAP * sizeof(int));

	FlipEffect = -1;
	FlipStatus = 0;
	Camera.underwater = false;
}

void InitializeSpecialEffects()
{
	memset(&FireSparks, 0, MAX_SPARKS_FIRE * sizeof(FIRE_SPARKS));
	memset(&SmokeSparks, 0, MAX_SPARKS_SMOKE * sizeof(SMOKE_SPARKS));
	memset(&Gunshells, 0, MAX_GUNSHELL * sizeof(GUNSHELL_STRUCT));
	memset(&Gunflashes, 0, (MAX_GUNFLASH * sizeof(GUNFLASH_STRUCT)));
	memset(&Blood, 0, MAX_SPARKS_BLOOD * sizeof(BLOOD_STRUCT));
	memset(&Splashes, 0, MAX_SPLASHES * sizeof(SPLASH_STRUCT));
	memset(&ShockWaves, 0, MAX_SHOCKWAVE * sizeof(SHOCKWAVE_STRUCT));
	memset(&Particles, 0, MAX_PARTICLES * sizeof(Particle));

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		Particles[i].on = false;
		Particles[i].dynamic = -1;
	}

	NextFireSpark = 1;
	NextSmokeSpark = 0;
	NextGunShell = 0;
	NextBlood = 0;

	TEN::Entities::TR4::ClearBeetleSwarm();
}

void CustomObjects()
{
	
}

void InitializeObjects()
{
	AllocTR4Objects();
	AllocTR5Objects();

	ObjectInfo* obj;

	for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
	{
		obj = &Objects[i];
		obj->Initialize = nullptr;
		obj->collision = nullptr;
		obj->control = nullptr;
		obj->floor = nullptr;
		obj->ceiling = nullptr;
		obj->drawRoutine = DrawAnimatingItem;
		obj->HitRoutine = DefaultItemHit;
		obj->pivotLength = 0;
		obj->radius = DEFAULT_RADIUS;
		obj->shadowType = ShadowMode::None;
		obj->HitPoints = NOT_TARGETABLE;
		obj->hitEffect = HitEffect::None;
		obj->explodableMeshbits = 0;
		obj->intelligent = false;
		obj->waterCreature = false;
		obj->nonLot = false;
		obj->usingDrawAnimatingItem = true;
		obj->damageType = DamageMode::AnyWeapon;
		obj->LotType = LotType::Basic;
		obj->meshSwapSlot = NO_ITEM;
		obj->isPickup = false;
		obj->isPuzzleHole = false;
	}

	InitializeEffectsObjects();
	InitializeGenericObjects(); // Generic objects
	InitializeTR5Objects(); // Standard TR5 objects (NOTE: lara need to be loaded first, so entity like doppelganger can use this animIndex !)
	InitializeTR1Objects(); // Standard TR1 objects
	InitializeTR2Objects(); // Standard TR2 objects
	InitializeTR3Objects(); // Standard TR3 objects
	InitializeTR4Objects(); // Standard TR4 objects

	// User defined objects
	CustomObjects();

	HairEffect.Initialize();
	InitializeSpecialEffects();

	NumRPickups = 0;
	CurrentSequence = 0;
	SequenceResults[0][1][2] = 0;
	SequenceResults[0][2][1] = 1;
	SequenceResults[1][0][2] = 2;
	SequenceResults[1][2][0] = 3;
	SequenceResults[2][0][1] = 4;
	SequenceResults[2][1][0] = 5;
	SequenceUsed[0] = 0;
	SequenceUsed[1] = 0;
	SequenceUsed[2] = 0;
	SequenceUsed[3] = 0;
	SequenceUsed[4] = 0;
	SequenceUsed[5] = 0;
}
