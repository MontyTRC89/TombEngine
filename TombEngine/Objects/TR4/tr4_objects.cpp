#include "framework.h"
#include "Objects/TR4/tr4_objects.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/pickup/pickup.h"
#include "Objects/Utils/object_helper.h"
#include "Objects/Generic/Object/objects.h"
#include "Specific/setup.h"
#include "Specific/level.h"

// Creatures
#include "Objects/TR4/Entity/tr4_enemy_jeep.h"
#include "Objects/TR4/Entity/tr4_ahmet.h" // OK
#include "Objects/TR4/Entity/tr4_baddy.h" // OK
#include "Objects/TR4/Entity/tr4_bat.h" // OK
#include "Objects/TR4/Entity/tr4_big_scorpion.h" // OK
#include "Objects/TR4/Entity/tr4_crocodile.h" // OK
#include "Objects/TR4/Entity/tr4_demigod.h" // OK
#include "Objects/TR4/Entity/tr4_guide.h" // OK
#include "Objects/TR4/Entity/tr4_harpy.h" // OK
#include "Objects/TR4/Entity/tr4_horseman.h" // OFF
#include "Objects/TR4/Entity/tr4_jean_yves.h" // OK
#include "Objects/TR4/Entity/tr4_knight_templar.h" // OK
#include "Objects/TR4/Entity/tr4_lara_double.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR4/Entity/tr4_mummy.h" // OK
#include "Objects/TR4/Entity/tr4_sas.h" // OK
#include "Objects/TR4/Entity/tr4_sentry_gun.h" // OK
#include "Objects/TR4/Entity/tr4_skeleton.h" // OK
#include "Objects/TR4/Entity/tr4_small_scorpion.h" // OK
#include "Objects/TR4/Entity/tr4_sphinx.h" // OK
#include "Objects/TR4/Entity/tr4_troops.h" // OK
#include "Objects/TR4/Entity/tr4_wild_boar.h" // OK
#include "Objects/TR4/Entity/tr4_wraith.h" // OFF
#include "Objects/TR4/Entity/tr4_baboon.h" // OK
#include "Objects/TR4/Entity/tr4_mutant.h" // OK
#include "Objects/TR4/Entity/tr4_big_beetle.h" // OFF
#include "Objects/TR4/Entity/tr4_von_croy.h"
#include "Objects/TR4/Entity/tr4_hammerhead.h"
#include "Objects/TR4/Entity/tr4_dog.h"
#include "Objects/TR4/Entity/tr4_setha.h"

// Objects
#include "Objects/TR4/Object/tr4_element_puzzle.h"
#include "Objects/TR4/Object/tr4_mapper.h"
#include "Objects/TR4/Object/tr4_sarcophagus.h"
#include "Objects/TR4/Object/tr4_senet.h"
#include "Objects/TR4/Object/tr4_clockwork_beetle.h"
#include "Objects/TR4/Object/tr4_obelisk.h"
#include "Objects/TR4/Object/tr4_scales.h"

// Switches

// Traps
#include "Objects/TR4/Trap/tr4_birdblade.h"
#include "Objects/TR4/Trap/tr4_blade.h"
#include "Objects/TR4/Trap/tr4_catwalkblade.h"
#include "Objects/TR4/Trap/tr4_chain.h"
#include "Objects/TR4/Trap/tr4_fourblades.h"
#include "Objects/TR4/Trap/tr4_hammer.h"
#include "Objects/TR4/Trap/tr4_joby_spikes.h"
#include "Objects/TR4/Trap/tr4_mine.h"
#include "Objects/TR4/Trap/tr4_moving_blade.h"
#include "Objects/TR4/Trap/tr4_plinthblade.h"
#include "Objects/TR4/Trap/tr4_plough.h"
#include "Objects/TR4/Trap/tr4_sethblade.h"
#include "Objects/TR4/Trap/tr4_slicerdicer.h"
#include "Objects/TR4/Trap/tr4_spikeball.h"
#include "Objects/TR4/Trap/tr4_spikywall.h"
#include "Objects/TR4/Trap/tr4_spikyceiling.h"
#include "Objects/TR4/Trap/tr4_stargate.h"
#include "Objects/TR4/Trap/tr4_cog.h"
#include "Objects/TR4/Trap/tr4_teethspike.h"

// Vehicles
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

// Effects
#include "Objects/Effects/tr4_locusts.h" // OK

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
			obj->pivotLength = 20;
			obj->radius = 128;
			obj->intelligent = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BIG_SCORPION];
		if (obj->loaded)
		{
			obj->initialise = InitialiseScorpion;
			obj->control = ScorpionControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 80;
			obj->pivotLength = 50;
			obj->radius = 512;
			obj->intelligent = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_HAMMERHEAD];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHammerhead;
			obj->control = HammerheadControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 8;
			obj->pivotLength = 20;
			obj->radius = 128;
			obj->intelligent = true;
			obj->waterCreature = true;
			obj->LotType = LotType::Water;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_WILD_BOAR];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWildBoar;
			obj->control = WildBoarControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 40;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(12, ROT_Y | ROT_Z);
			obj->SetBoneRotationFlags(13, ROT_Y | ROT_Z);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_DOG];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTr4Dog;
			obj->collision = CreatureCollision;
			obj->control = Tr4DogControl;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 18;
			obj->pivotLength = 50;
			obj->radius = 256;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(0, ROT_Y);
			obj->SetBoneRotationFlags(2, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BAT];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBat;
			obj->control = BatControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 5;
			obj->pivotLength = 10;
			obj->radius = 102;
			obj->intelligent = true;
			obj->LotType = LotType::Flyer;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_AHMET];
		if (obj->loaded)
		{
			obj->initialise = InitialiseAhmet;
			obj->control = AhmetControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 80;
			obj->pivotLength = 300;
			obj->radius = 341;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(9, ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BADDY1];
		if (obj->loaded)
		{
			AssignObjectMeshSwap(*obj, ID_MESHSWAP_BADDY1, "ID_BADDY1", "ID_MESHSWAP_BADDY1");
			obj->initialise = InitialiseBaddy;
			obj->control = BaddyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->biteOffset = 9;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->meshSwapSlot = ID_MESHSWAP_BADDY1;
			obj->LotType = LotType::HumanPlusJumpAndMonkey;
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(22, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BADDY2];
		if (obj->loaded)
		{
			AssignObjectMeshSwap(*obj, ID_MESHSWAP_BADDY2, "ID_BADDY2", "ID_MESHSWAP_BADDY2");
			obj->initialise = InitialiseBaddy;
			obj->control = BaddyControl;
			obj->collision = CreatureCollision;
			obj->HitRoutine = Baddy2Hit;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->pivotLength = 50;
			obj->biteOffset = 9;
			obj->radius = 102;
			obj->intelligent = true;
			obj->meshSwapSlot = ID_MESHSWAP_BADDY2;
			obj->LotType = LotType::HumanPlusJumpAndMonkey;
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(22, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_SAS_CAIRO];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSas;
			obj->control = SasControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->biteOffset = 10;
			obj->HitPoints = 40;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_MUMMY];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMummy;
			obj->control = MummyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 15;
			obj->radius = 170;
			obj->intelligent = true;
			obj->undead = true;
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_SKELETON];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSkeleton;
			obj->control = SkeletonControl;
			obj->collision = CreatureCollision;
			obj->HitPoints = 15;
			obj->shadowType = ShadowMode::All;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->explodableMeshbits = 0xA00;
			obj->intelligent = true;
			obj->undead = true;
			obj->LotType = LotType::Skeleton;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_KNIGHT_TEMPLAR];
		if (obj->loaded)
		{
			obj->initialise = InitialiseKnightTemplar;
			obj->control = KnightTemplarControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 15;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->undead = true;
			obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(7, ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BIG_BEETLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBigBeetle;
			obj->control = BigBeetleControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->pivotLength = 50;
			obj->radius = 204;
			obj->intelligent = true;
			obj->undead = false;
			obj->LotType = LotType::Flyer;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_SETHA];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSeth;
			obj->control = SethControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 500;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->undead = true;
			obj->LotType = LotType::Skeleton;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_DEMIGOD1];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->undead = true;
			obj->SetBoneRotationFlags(8, ROT_X | ROT_Y | ROT_Z);
			obj->SetBoneRotationFlags(20, ROT_Y);
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_DEMIGOD2];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(8, ROT_X | ROT_Y | ROT_Z);
			obj->SetBoneRotationFlags(20, ROT_Y);
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_DEMIGOD3];
		if (obj->loaded)
		{
			obj->initialise = InitialiseDemigod;
			obj->control = DemigodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 200;
			obj->pivotLength = 50;
			obj->radius = 341;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(8, ROT_X | ROT_Y | ROT_Z);
			obj->SetBoneRotationFlags(20, ROT_Y);
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_JEAN_YVES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJeanYves;
			obj->control = JeanYvesControl;
			obj->collision = ObjectCollision;
			obj->nonLot = true;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_TROOPS];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTroops;
			obj->control = TroopsControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->biteOffset = 11;
			obj->HitPoints = 40;
			obj->pivotLength = 50;
			obj->radius = 102;
			obj->intelligent = true;
			obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetupHitEffect();
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
			obj->pivotLength = 50;
			obj->radius = 204;
			obj->intelligent = true;
			obj->explodableMeshbits = 0x40;
			obj->SetBoneRotationFlags(0, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(1, ROT_X | ROT_X);
			obj->SetBoneRotationFlags(2, ROT_X | ROT_Z);
			obj->SetBoneRotationFlags(3, ROT_X | ROT_Z);
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_HARPY];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHarpy;
			obj->control = HarpyControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 60;
			obj->pivotLength = 50;
			obj->radius = 409;
			obj->intelligent = true;
			obj->LotType = LotType::Flyer;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_GUIDE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseGuide;
			obj->control = GuideControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = NOT_TARGETABLE;
			obj->pivotLength = 0;
			obj->radius = 128;
			obj->intelligent = true;
			obj->meshSwapSlot = ID_MESHSWAP2;
			obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(20, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_CROCODILE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseCrocodile;
			obj->control = CrocodileControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 36;
			obj->pivotLength = 300;
			obj->radius = 409;
			obj->intelligent = true;
			obj->waterCreature = true;
			obj->LotType = LotType::Water; // TODO: later, change it to WaterAndLand.
			obj->SetBoneRotationFlags(0, ROT_Y);
			obj->SetBoneRotationFlags(7, ROT_Y);
			obj->SetBoneRotationFlags(9, ROT_Y);
			obj->SetBoneRotationFlags(10, ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_SPHINX];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSphinx;
			obj->control = SphinxControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 1000;
			obj->pivotLength = 500;
			obj->radius = 512;
			obj->intelligent = true;
			obj->undead = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_HORSE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHorse;
			obj->collision = ObjectCollision;
			obj->control = nullptr;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_HORSEMAN];
		if (obj->loaded)
		{
			obj->initialise = InitialiseHorseman;
			obj->control = HorsemanControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 25;
			obj->pivotLength = 500;
			obj->radius = 409;
			obj->intelligent = true;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_BABOON_NORMAL];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BABOON_INV];
		if (obj->loaded)
		{
			AssignObjectAnimations(*obj, ID_BABOON_NORMAL, "ID_BABOON_INV", "ID_BABOON_NORMAL");
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_BABOON_SILENT];
		if (obj->loaded)
		{
			AssignObjectAnimations(*obj, ID_BABOON_NORMAL, "ID_BABOON_SILENT", "ID_BABOON_NORMAL");
			obj->initialise = InitialiseBaboon;
			obj->control = BaboonControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 30;
			obj->pivotLength = 200;
			obj->radius = 256;
			obj->intelligent = true;
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_CROCODILE_GOD];
		if (obj->loaded)
		{
			obj->initialise = InitialiseCrocgod;
			obj->control = CrocgodControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = NOT_TARGETABLE;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->undead = true;
			obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(7, ROT_X | ROT_Y);
			obj->SetupHitEffect();
		}

		obj = &Objects[ID_LOCUSTS_EMITTER];
		if (obj->loaded)
		{
			obj->initialise = InitialiseLocust;
			obj->control = LocustControl;
			obj->drawRoutine = NULL;
		}

		obj = &Objects[ID_WRAITH1];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
		}

		obj = &Objects[ID_WRAITH2];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
		}

		obj = &Objects[ID_WRAITH3];
		if (obj->loaded)
		{
			obj->initialise = InitialiseWraith;
			obj->control = WraithControl;
		}	

		obj = &Objects[ID_LITTLE_BEETLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseBeetleSwarm;
			obj->control = BeetleSwarmControl;
			obj->drawRoutine = NULL;
		}

		obj = &Objects[ID_SAS_DYING];
		if (obj->loaded)
		{
			obj->initialise = InitialiseInjuredSas;
			obj->control = InjuredSasControl;
			obj->collision = ObjectCollision;
			obj->SetupHitEffect(false, true);
		}

		obj = &Objects[ID_ENEMY_JEEP];
		if (obj->loaded)
		{
			obj->initialise = InitialiseEnemyJeep;
			obj->control = EnemyJeepControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 40;
			obj->pivotLength = 500;
			obj->radius = 512;
			obj->intelligent = true;
			obj->undead = true; // NOTE: Prevents enemy jeep from being killed with skidoo gun or something like that.
			obj->LotType = LotType::HumanPlusJumpAndMonkey;
			obj->SetBoneRotationFlags(8, ROT_X);
			obj->SetBoneRotationFlags(9, ROT_X);
			obj->SetBoneRotationFlags(11, ROT_X);
			obj->SetBoneRotationFlags(12, ROT_X);
			obj->SetupHitEffect(true);
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
			obj->LotType = LotType::HumanPlusJumpAndMonkey;
			obj->SetBoneRotationFlags(6, ROT_X | ROT_Y);
			obj->SetBoneRotationFlags(20, ROT_X | ROT_Y);
		}
	}

	static void StartObject(ObjectInfo* obj)
	{
		obj = &Objects[ID_SAS_DRAG_BLOKE];
		if (obj->loaded)
		{
			obj->control = AnimatingControl;
			obj->collision = SasDragBlokeCollision;
			obj->SetupHitEffect(false, true);
		}

		obj = &Objects[ID_SARCOPHAGUS];
		if (obj->loaded)
		{
			obj->control = AnimatingControl;
			obj->collision = SarcophagusCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_MAPPER];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMapper;
			obj->control = MapperControl;
			obj->drawRoutine = nullptr;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_ELEMENT_PUZZLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseElementPuzzle;
			obj->control = ElementPuzzleControl;
			obj->collision = ElementPuzzleCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_WHEEL_OF_FORTUNE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseGameSticks;
			obj->control = GameSticksControl;
			obj->collision = GameSticksCollision;
			obj->HitPoints = 1;
			obj->SetBoneRotationFlags(0, ROT_Z);
			obj->SetBoneRotationFlags(1, ROT_Z);
			obj->SetBoneRotationFlags(2, ROT_Z);
			obj->SetBoneRotationFlags(3, ROT_Z);
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_ENEMY_PIECE];
		if (obj->loaded)
		{
			obj->collision = ObjectCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_GOD_HEAD];
		if (obj->loaded)
			obj->control = ControlGodHead;

		for (int i = 0; i < 3; i++)
		{
			obj = &Objects[ID_GAME_PIECE1 + i];
			if (obj->loaded)
			{
				obj->initialise = InitialiseGamePiece;
				obj->control = SenetControl;
				obj->collision = ObjectCollision;
				obj->SetupHitEffect(true);
			}
		}

		InitPickup(obj, ID_CLOCKWORK_BEETLE, ClockworkBeetleControl);
		InitPickup(obj, ID_CLOCKWORK_BEETLE_COMBO1);
		InitPickup(obj, ID_CLOCKWORK_BEETLE_COMBO2);

		obj = &Objects[ID_OBELISK];
		if (obj->loaded)
		{
			obj->initialise = InitialiseObelisk;
			obj->control = ObeliskControl;
			obj->collision = ObjectCollision;
			obj->SetupHitEffect(true);
		}
	}

	static void StartTrap(ObjectInfo* obj)
	{
		obj = &Objects[ID_CHAIN];
		if (obj->loaded)
		{
			obj->control = ChainControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_PLOUGH];
		if (obj->loaded)
		{
			obj->control = PloughControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_CATWALK_BLADE];
		if (obj->loaded)
		{
			obj->control = CatwalkBladeControl;
			obj->collision = BladeCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_SETH_BLADE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSethBlade;
			obj->control = SethBladeControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_PLINTH_BLADE];
		if (obj->loaded)
		{
			obj->control = PlinthBladeControl;
			obj->collision = BladeCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_BIRD_BLADE];
		if (obj->loaded)
		{
			obj->control = BirdBladeControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_JOBY_SPIKES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJobySpikes;
			obj->control = JobySpikesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_MOVING_BLADE];
		if (obj->loaded)
		{
			obj->control = MovingBladeControl;
			obj->collision = BladeCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_SPIKEBALL];
		if (obj->loaded)
		{
			obj->control = SpikeballControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_CHAIN];
		if (obj->loaded)
		{
			obj->control = ChainControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_PLOUGH];
		if (obj->loaded)
		{
			obj->control = PloughControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_FLOOR_4BLADES];
		if (obj->loaded)
		{
			obj->control = FourBladesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_CEILING_4BLADES];
		if (obj->loaded)
		{
			obj->control = FourBladesControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_STARGATE];
		if (obj->loaded)
		{
			obj->control = StargateControl;
			obj->collision = StargateCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_SLICER_DICER];
		if (obj->loaded)
		{
			obj->initialise = InitialiseSlicerDicer;
			obj->control = SlicerDicerControl;
			obj->collision = BladeCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_MINE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMine;
			obj->control = MineControl;
			obj->collision = MineCollision;
		}

		obj = &Objects[ID_SPIKY_WALL];
		if (obj->loaded)
		{
			obj->control = ControlSpikyWall;
			obj->collision = ObjectCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_SPIKY_CEILING];
		if (obj->loaded)
		{
			obj->control = ControlSpikyCeiling;
			obj->collision = TrapCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_COG];
		if (obj->loaded)
		{
			obj->control = CogControl;
			obj->collision = CogCollision;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_LARA_DOUBLE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseCreature;
			obj->control = LaraDoubleControl;
			obj->collision = CreatureCollision;
			obj->shadowType = ShadowMode::All;
			obj->HitPoints = 1000;
			obj->pivotLength = 50;
			obj->radius = 128;
			obj->intelligent = true;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_TEETH_SPIKES];
		if (obj->loaded)
		{
			obj->initialise = InitialiseTeethSpikes;
			obj->control = ControlTeethSpikes;
		}

		obj = &Objects[ID_HAMMER];
		if (obj->loaded)
		{
			obj->control = HammerControl;
			obj->collision = GenericSphereBoxCollision;
			obj->SetupHitEffect(true);
		}
	}

	static void StartVehicles(ObjectInfo* obj)
	{
		obj = &Objects[ID_JEEP];
		if (obj->loaded)
		{
			obj->initialise = InitialiseJeep;
			obj->collision = JeepPlayerCollision;
			obj->shadowType = ShadowMode::Lara;
			obj->SetupHitEffect(true);
		}

		obj = &Objects[ID_MOTORBIKE];
		if (obj->loaded)
		{
			obj->initialise = InitialiseMotorbike;
			obj->collision = MotorbikePlayerCollision;
			obj->shadowType = ShadowMode::Lara;
			obj->SetupHitEffect(true);
		}
	}

	static void StartSwitch(ObjectInfo* obj)
	{
	
	}

	void InitialiseTR4Objects()
	{
		ObjectInfo* objectPtr = nullptr;
		StartEntity(objectPtr);
		StartObject(objectPtr);
		StartSwitch(objectPtr);
		StartTrap(objectPtr);
		StartVehicles(objectPtr);
	}

	void AllocTR4Objects()
	{
		ZeroMemory(BeetleSwarm, NUM_BEETLES * sizeof(BeetleData));
	}
}
