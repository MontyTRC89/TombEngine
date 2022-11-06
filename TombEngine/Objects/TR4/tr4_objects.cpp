#include "framework.h"
#include "Objects/TR4/tr4_objects.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/pickup/pickup.h"
#include "Objects/Generic/Object/objects.h"
#include "Specific/setup.h"
#include "Specific/level.h"

// Creatures
#include "tr4_enemy_jeep.h"
#include "tr4_ahmet.h" // OK
#include "tr4_baddy.h" // OK
#include "tr4_bat.h" // OK
#include "tr4_big_scorpion.h" // OK
#include "tr4_crocodile.h" // OK
#include "tr4_demigod.h" // OK
#include "tr4_guide.h" // OK
#include "tr4_harpy.h" // OK
#include "tr4_horseman.h" // OFF
#include "tr4_jean_yves.h" // OK
#include "tr4_knight_templar.h" // OK
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "tr4_mummy.h" // OK
#include "tr4_sas.h" // OK
#include "tr4_sentry_gun.h" // OK
#include "tr4_skeleton.h" // OK
#include "tr4_small_scorpion.h" // OK
#include "tr4_sphinx.h" // OK
#include "tr4_troops.h" // OK
#include "tr4_wild_boar.h" // OK
#include "tr4_wraith.h" // OFF
#include "tr4_baboon.h" // OK
#include "tr4_mutant.h" // OK
#include "tr4_locusts.h" // OK
#include "tr4_big_beetle.h" // OFF
#include "tr4_joby_spikes.h"
#include "tr4_mapper.h"
#include "tr4_moving_blade.h"
#include "tr4_element_puzzle.h"
#include "tr4_von_croy.h"
#include "tr4_hammerhead.h"
#include "tr4_dog.h"
#include "tr4_hammer.h"

// Objects
#include "tr4_sas_drag_bloke.h"
#include "tr4_sarcophagus.h"
#include "tr4_senet.h"
#include "Objects/TR4/Object/tr4_clockwork_beetle.h"
#include "tr4_obelisk.h"

// Puzzles
#include "tr4_scales.h"

// Switches

// Traps
#include "tr4_birdblade.h"
#include "tr4_blade.h"
#include "tr4_catwalkblade.h"
#include "tr4_chain.h"
#include "tr4_fourblades.h"
#include "tr4_mine.h"
#include "tr4_plinthblade.h"
#include "tr4_plough.h"
#include "tr4_sethblade.h"
#include "tr4_slicerdicer.h"
#include "tr4_spikeball.h"
#include "tr4_spikywall.h"
#include "tr4_spikyceiling.h"
#include "tr4_stargate.h"
#include "tr4_cog.h"
#include "tr4_lara_double.h"
#include "tr4_setha.h"
#include "tr4_teethspike.h"

// Vehicles
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

using namespace TEN::Entities::Traps;

namespace TEN::Entities
{
	static void StartEntity(ObjectInfo* obj)
	{
		obj = &Objects[ID_SMALL_SCORPION];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSmallScorpion;
			obj->control = SmallScorpionControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 8;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 20;
			obj->radius = 128;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_BIG_SCORPION];
		if (obj->loaded)
		{
			obj->initialise = InitialiseScorpion;
			obj->control = ScorpionControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 80;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 512;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_HAMMERHEAD];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHammerhead;
			obj->control = HammerheadControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 8;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 20;
			obj->radius = 128;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->waterCreature = true;
			obj->ZoneType = ZoneType::Water;
		}

		obj = &Objects[ID_WILD_BOAR];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWildBoar;
			obj->control = WildBoarControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 40;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 48 * 4] |= ROT_Z;
			g_Level.Bones[obj->boneIndex + 48 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 52 * 4] |= ROT_Z;
			g_Level.Bones[obj->boneIndex + 52 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_DOG];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTr4Dog;
			obj->collision = CreatureCollision;
			obj->control = Tr4DogControl;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 18;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 256;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->saveHitpoints = true;
			obj->ZoneType = ZoneType::Basic;
			g_Level.Bones[obj->boneIndex + 19 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_BAT];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBat;
			obj->control = BatControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 5;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 10;
			obj->radius = 102;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Flyer;
		}

		obj = &Objects[ID_AHMET];
		if (obj->loaded)
		{
			obj->initialise = InitialiseAhmet;
			obj->control = AhmetControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 80;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 300;
			obj->radius = 341;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveHitpoints = true;
			obj->savePosition = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_BADDY1];
		if (obj->loaded)
		{
			obj->biteOffset = 9;
			obj->initialise = InitialiseBaddy;
			obj->control = BaddyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->meshSwapSlot = ID_MESHSWAP_BADDY1;
			obj->ZoneType = ZoneType::HumanJumpAndMonkey;

			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_X;
			//g_Level.Bones[obj->boneIndex + 88 * 4] |= ROT_Y;
			//g_Level.Bones[obj->boneIndex + 88 * 4] |= ROT_X;
		}

		obj = &Objects[ID_BADDY2];
		if (obj->loaded)
		{
			obj->biteOffset = 9;
			obj->initialise = InitialiseBaddy;
			obj->control = BaddyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->meshSwapSlot = ID_MESHSWAP_BADDY2;
			obj->ZoneType = ZoneType::HumanJumpAndMonkey;

			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_X;
			//g_Level.Bones[obj->boneIndex + 88 * 4] |= ROT_Y;
			//g_Level.Bones[obj->boneIndex + 88 * 4] |= ROT_X;
		}

		obj = &Objects[ID_SAS_CAIRO];
		if (obj->loaded)
		{
			obj->biteOffset = 10;
			obj->initialise = InitialiseSas;
			obj->control = SasControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 40;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::HumanClassic;

			g_Level.Bones[obj->boneIndex] |= ROT_Y;
			g_Level.Bones[obj->boneIndex] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 28 * 4] |= ROT_X;
		}

		obj = &Objects[ID_MUMMY];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMummy;
			obj->control = MummyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 15;
			obj->hitEffect = HIT_SMOKE;
			obj->radius = 170;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->undead = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_X;

		}

		obj = &Objects[ID_SKELETON];
		if (obj->loaded)
		{
			obj->initialise = TEN::Entities::TR4::InitialiseSkeleton;
			obj->control = TEN::Entities::TR4::SkeletonControl;
			obj->collision = CreatureCollision;
			obj->HitPoints = 15;
			obj->hitEffect = HIT_SMOKE;
			obj->shadowType = ShadowMode::All;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->explodableMeshbits = 0xA00;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->undead = true;
			obj->ZoneType = ZoneType::Skeleton;
		}

		obj = &Objects[ID_KNIGHT_TEMPLAR];
		if (obj->loaded)
		{
			obj->initialise = InitialiseKnightTemplar;
			obj->control = KnightTemplarControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 15;
			obj->hitEffect = HIT_SMOKE;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X | ROT_Y;
			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_BIG_BEETLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBigBeetle;
			obj->control = BigBeetleControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 204;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->savePosition = true;
			obj->undead = false;
			obj->ZoneType = ZoneType::Flyer;
		}

		obj = &Objects[ID_SETHA];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSetha;
			obj->control = SethaControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 500;
			obj->hitEffect = HIT_NONE;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->undead = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_DEMIGOD1];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->undead = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
			g_Level.Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_DEMIGOD2];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;
			g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
			g_Level.Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_DEMIGOD3];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;
			g_Level.Bones[obj->boneIndex + 4 * 4] |= ROT_X | ROT_Y | ROT_Z;
			g_Level.Bones[obj->boneIndex + 5 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_JEAN_YVES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJeanYves;
			obj->control = JeanYvesControl;
			obj->collision = ObjectCollision;
			obj->hitEffect = HIT_BLOOD;
			obj->nonLot = true;
			obj->savePosition = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_TROOPS];
		if (obj->loaded)
		{
			obj->biteOffset = 11;
			obj->initialise = InitialiseTroops;
			obj->control = TroopsControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 40;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex] |= ROT_X | ROT_Y;
			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_X | ROT_Y;
		}

		obj = &Objects[ID_SENTRY_GUN];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSentryGun;
			obj->control = SentryGunControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->undead = true;
			obj->HitPoints = 30;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 50;
			obj->radius = 204;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->explodableMeshbits = 64;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 0] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 1 * 4] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 2 * 4] |= ROT_Z;
			g_Level.Bones[obj->boneIndex + 3 * 4] |= ROT_Z;
		}

		obj = &Objects[ID_HARPY];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHarpy;
			obj->control = HarpyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 60;
			obj->hitEffect = HIT_SMOKE;
			obj->pivotLength = 50;
			obj->radius = 409;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Flyer;
		}

	obj = &Objects[ID_GUIDE];
	if (obj->loaded)
	{
		obj->initialise = InitialiseGuide;
		obj->control = GuideControl;
		obj->collision = CreatureCollision;
		obj->shadowType = ShadowMode::All;
		obj->HitPoints = NOT_TARGETABLE;
		obj->hitEffect = HIT_BLOOD;
		obj->pivotLength = 0;
		obj->radius = 128;
		obj->intelligent = true;
		obj->saveHitpoints = true;
		obj->saveAnim = true;
		obj->saveFlags = true;
		obj->meshSwapSlot = ID_MESHSWAP2;
		obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_X | ROT_Y;
			g_Level.Bones[obj->boneIndex + 20 * 4] |= ROT_X | ROT_Y;
		}

		obj = &Objects[ID_CROCODILE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseCrocodile;
			obj->control = CrocodileControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 36;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 300;
			obj->radius = 409;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->waterCreature = true;
			obj->ZoneType = ZoneType::Water;

			g_Level.Bones[obj->boneIndex] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 9 * 4] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 10 * 4] |= ROT_Y;
		}

		obj = &Objects[ID_SPHINX];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSphinx;
			obj->control = SphinxControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 1000;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 500;
			obj->radius = 512;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_HORSE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHorse;
			obj->control = nullptr;
			obj->collision = ObjectCollision;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_HORSEMAN];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHorseman;
			obj->control = HorsemanControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->hitEffect = HIT_RICOCHET;
			obj->pivotLength = 500;
			obj->radius = 409;
			obj->intelligent = true;
			obj->saveHitpoints = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->savePosition = true;
			obj->saveMesh = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_BABOON_NORMAL];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveHitpoints = true;
			obj->savePosition = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_BABOON_INV];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveHitpoints = true;
			obj->savePosition = true;
			obj->ZoneType = ZoneType::Basic;

			if (Objects[ID_BABOON_NORMAL].loaded)
				Objects[ID_BABOON_INV].animIndex = Objects[ID_BABOON_NORMAL].animIndex;
		}

		obj = &Objects[ID_BABOON_SILENT];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->hitEffect = HIT_BLOOD;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveHitpoints = true;
			obj->savePosition = true;
			obj->ZoneType = ZoneType::Basic;

			if (Objects[ID_BABOON_NORMAL].loaded)
				Objects[ID_BABOON_SILENT].animIndex = Objects[ID_BABOON_NORMAL].animIndex;
		}

		obj = &Objects[ID_CROCODILE_GOD];
		if (obj->loaded)
		{
			obj->initialise = TEN::Entities::TR4::InitialiseCrocgod;
			obj->control = TEN::Entities::TR4::CrocgodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = NOT_TARGETABLE;
			obj->hitEffect = HIT_SMOKE;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->saveMesh = true;
			obj->savePosition = true;
			obj->undead = true;
			obj->ZoneType = ZoneType::Water;
			g_Level.Bones[obj->boneIndex + 6 * 4] |= ROT_Y | ROT_X;
			g_Level.Bones[obj->boneIndex + 7 * 4] |= ROT_Y | ROT_X;
		}

		obj = &Objects[ID_LOCUSTS_EMITTER];
		if (obj->loaded)
		{
			obj->initialise = TEN::Entities::TR4::InitialiseLocust;
			obj->control = TEN::Entities::TR4::LocustControl;
			obj->drawRoutine = NULL;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_WRAITH1];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}

		obj = &Objects[ID_WRAITH2];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}

		obj = &Objects[ID_WRAITH3];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}	

		obj = &Objects[ID_LITTLE_BEETLE];
		if (obj->loaded)
		{
			obj->initialise = TEN::Entities::TR4::InitialiseBeetleSwarm;
			obj->control = TEN::Entities::TR4::BeetleSwarmControl;
			obj->drawRoutine = NULL;
			obj->saveFlags = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_SAS_DYING];
		if (obj->loaded)
		{
			obj->initialise = InitialiseInjuredSas;
			obj->control = InjuredSasControl;
			obj->collision = ObjectCollision;
			obj->hitEffect = HIT_BLOOD;
			obj->saveFlags = true;
			obj->savePosition = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_ENEMY_JEEP];
		if (obj->loaded)
		{
			obj->initialise = InitialiseEnemyJeep;
			obj->control = EnemyJeepControl;
			obj->collision = CreatureCollision;
			obj->saveFlags = true;
			obj->savePosition = true;
			obj->saveAnim = true;
			obj->intelligent = true;
			obj->saveHitpoints = true; 
			obj->pivotLength = 500;
			obj->shadowType = ShadowMode::All;
			obj->radius = 512;
			obj->HitPoints = 40;
			obj->ZoneType = ZoneType::Basic;

			g_Level.Bones[obj->boneIndex + 4 * 8] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 4 * 9] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 4 * 11] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 4 * 12] |= ROT_X;
		}

		obj = &Objects[ID_VON_CROY];
		if (obj->loaded)
		{
			obj->initialise = InitialiseVonCroy;
			obj->control = VonCroyControl;
			obj->collision = CreatureCollision;
			obj->pivotLength = 0;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 15;
			obj->explodableMeshbits = 0x200000;
			obj->intelligent = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveMesh = true;
			obj->ZoneType = ZoneType::HumanLongJumpAndMonkey;

			g_Level.Bones[obj->boneIndex + 4 * 6] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 4 * 6] |= ROT_Y;
			g_Level.Bones[obj->boneIndex + 4 * 20] |= ROT_X;
			g_Level.Bones[obj->boneIndex + 4 * 20] |= ROT_Y;
		}
	}

	static void StartObject(ObjectInfo* obj)
	{
		obj = &Objects[ID_SAS_DRAG_BLOKE];
		if (obj->loaded)
		{
			obj->control = AnimatingControl;
			obj->collision = DragSASCollision;
			obj->savePosition = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}

		obj = &Objects[ID_SARCOPHAGUS];
		if (obj->loaded)
		{
			obj->control = AnimatingControl;
			obj->collision = SarcophagusCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}

		obj = &Objects[ID_MAPPER];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMapper;
			obj->control = MapperControl;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->drawRoutine = nullptr;
		}

		obj = &Objects[ID_ELEMENT_PUZZLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseElementPuzzle;
			obj->control = ElementPuzzleControl;
			obj->collision = ElementPuzzleCollision;
			obj->saveFlags = true;
			obj->saveMesh = true;
		}

		obj = &Objects[ID_WHEEL_OF_FORTUNE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseGameSticks;
			obj->control = GameSticksControl;
			obj->collision = GameSticksCollision;
			g_Level.Bones[obj->boneIndex] |= 0x10;
			g_Level.Bones[obj->boneIndex + 4] |= 0x10;
			g_Level.Bones[obj->boneIndex + 8] |= 0x10;
			g_Level.Bones[obj->boneIndex + 12] |= 0x10;
			obj->saveAnim = 1;
			obj->saveFlags = 1;
			obj->HitPoints = 1;
		}

		obj = &Objects[ID_ENEMY_PIECE];
		if (obj->loaded)
		{
			obj->collision = ObjectCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = 1;
		}

		obj = &Objects[ID_GOD_HEAD];
		if (obj->loaded)
		{
			obj->control = ControlGodHead;
		//	obj->drawRoutine = DrawGodHead;
		//	obj->usingDrawAnimatingItem = 0;
			obj->saveFlags = 1;
		}

		for (int i = 0; i < 3; i++)
		{
			obj = &Objects[ID_GAME_PIECE1 + i];

			if (obj->loaded)
			{
				obj->initialise = InitialiseGamePiece;
				obj->control = SenetControl;
				obj->collision = ObjectCollision;
				obj->hitEffect = HIT_RICOCHET;
				obj->savePosition = 1;
				obj->saveHitpoints = 1;
				obj->saveFlags = 1;
			}
		}

		obj = &Objects[ID_CLOCKWORK_BEETLE];
		if (obj->loaded)
		{
			obj->initialise = 0;
			obj->control = ClockworkBeetleControl;
			obj->collision = PickupCollision;
		}

		obj = &Objects[ID_CLOCKWORK_BEETLE_COMBO1];
		if (obj->loaded)
		{
			obj->collision = PickupCollision;
		}

		obj = &Objects[ID_CLOCKWORK_BEETLE_COMBO2];
		if (obj->loaded)
		{
			obj->collision = PickupCollision;
		}

		obj = &Objects[ID_OBELISK];
		if (obj->loaded)
		{
			obj->initialise = InitialiseObelisk;
			obj->control = ObeliskControl;
			obj->collision = ObjectCollision;
			obj->savePosition = true;
			obj->saveFlags = true;
		}
	}

	static void StartTrap(ObjectInfo* obj)
	{
		obj = &Objects[ID_CHAIN];
		if (obj->loaded)
		{
			obj->control = ChainControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_PLOUGH];
		if (obj->loaded)
		{
			obj->control = PloughControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_CATWALK_BLADE];
		if (obj->loaded)
		{
			obj->control = CatwalkBladeControl;
			obj->collision = BladeCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_SETH_BLADE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSethBlade;
			obj->control = SethBladeControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_PLINTH_BLADE];
		if (obj->loaded)
		{
			obj->control = PlinthBladeControl;
			obj->collision = BladeCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_BIRD_BLADE];
		if (obj->loaded)
		{
			obj->control = BirdBladeControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_JOBY_SPIKES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJobySpikes;
			obj->control = JobySpikesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_MOVING_BLADE];
		if (obj->loaded)
		{
			obj->control = MovingBladeControl;
			obj->collision = BladeCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveFlags = true;
			obj->saveAnim = true;
		}

		obj = &Objects[ID_SPIKEBALL];
		if (obj->loaded)
		{
			obj->control = SpikeballControl;
			obj->collision = GenericSphereBoxCollision;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_CHAIN];
		if (obj->loaded)
		{
			obj->control = ChainControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_PLOUGH];
		if (obj->loaded)
		{
			obj->control = PloughControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_FLOOR_4BLADES];
		if (obj->loaded)
		{
			obj->control = FourBladesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_CEILING_4BLADES];
		if (obj->loaded)
		{
			obj->control = FourBladesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_STARGATE];
		if (obj->loaded)
		{
			obj->control = StargateControl;
			obj->collision = StargateCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_SLICER_DICER];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSlicerDicer;
			obj->control = SlicerDicerControl;
			obj->collision = BladeCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_MINE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMine;
			obj->control = MineControl;
			obj->collision = MineCollision;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->savePosition = true;
		}

		obj = &Objects[ID_SPIKY_WALL];
		if (obj->loaded)
		{
			obj->control = ControlSpikyWall;
			obj->collision = ObjectCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->savePosition = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_SPIKY_CEILING];
		if (obj->loaded)
		{
			obj->control = ControlSpikyCeiling;
			obj->collision = TrapCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->savePosition = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_COG];
		if (obj->loaded)
		{
			obj->control = CogControl;
			obj->collision = CogCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->saveAnim = true;
			obj->saveFlags = true;
		}

		obj = &Objects[ID_LARA_DOUBLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseLaraDouble;
			obj->control = LaraDoubleControl;
			obj->collision = CreatureCollision;
			obj->hitEffect = HIT_SMOKE;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 1000;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->savePosition = true;
			obj->saveHitpoints = true;
			obj->saveFlags = true;
			obj->saveAnim = true;
			obj->ZoneType = ZoneType::Basic;
		}

		obj = &Objects[ID_TEETH_SPIKES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTeethSpikes;
			obj->control = ControlTeethSpikes;
			obj->saveFlags = 1;
		}

		obj = &Objects[ID_HAMMER];
		if (obj->loaded)
		{
			obj->control = HammerControl;
			obj->collision = GenericSphereBoxCollision;
			obj->saveFlags = 1;
			obj->saveAnim = 1;
		}
	}

	static void StartVehicles(ObjectInfo* obj)
	{
		obj = &Objects[ID_JEEP];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJeep;
			obj->collision = JeepPlayerCollision;
			obj->hitEffect = HIT_RICOCHET;
			obj->savePosition = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->shadowType = ShadowMode::Lara;

		}

		obj = &Objects[ID_MOTORBIKE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMotorbike;
			obj->collision = MotorbikePlayerCollision;
			//obj->drawRoutine = DrawMotorbike; // for wheel rotation
			obj->hitEffect = HIT_RICOCHET;
			obj->savePosition = true;
			obj->saveAnim = true;
			obj->saveFlags = true;
			obj->shadowType = ShadowMode::Lara;

		}
	}

	static void StartSwitch(ObjectInfo* obj)
	{
	
	}

	static ObjectInfo* objToInit;
	void InitialiseTR4Objects()
	{
		StartEntity(objToInit);
		StartObject(objToInit);
		StartSwitch(objToInit);
		StartTrap(objToInit);
		StartVehicles(objToInit);
	}

	void AllocTR4Objects()
	{
		ZeroMemory(TEN::Entities::TR4::BeetleSwarm, TEN::Entities::TR4::NUM_BEETLES * sizeof(BeetleData));
	}
}
