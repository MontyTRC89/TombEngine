#include "framework.h"
#include "savegame.h"
#include "Lara.h"
#include "items.h"
#include "control/box.h"
#include "control/lot.h"
#include "spotcam.h"
#include "traps.h"
#include "room.h"
#include "sound/sound.h"
#include "level.h"
#include "setup.h"
#include "flipeffect.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "fullblock_switch.h"
#include "itemdata/creature_info.h"
#include "Game/effects/lara_burn.h"
#include "Specific/savegame/flatbuffers/ten_savegame_generated.h"

using namespace TEN::Effects::Fire;
using namespace TEN::Entities::Switches;
using namespace flatbuffers;

namespace Save = TEN::Save;
using std::string;
using std::vector;

FileStream* SaveGame::m_stream;
vector<LuaVariable> SaveGame::m_luaVariables;
int SaveGame::LastSaveGame;

SAVEGAME_INFO Savegame;

bool SaveGame::Save(char* fileName)
{
	ITEM_INFO itemToSerialize{};
	FlatBufferBuilder fbb{};
	Save::SaveGameBuilder sgb = Save::SaveGameBuilder(fbb);
	std::vector<flatbuffers::Offset< Save::Item>> serializedItems{};

	// Serialize Lara
	/*Save::LaraT serializedLara;

	for (int i = 0; i < NUM_PUZZLES; i++)
		serializedLara.puzzles.push_back(Lara.Puzzles[i]);

	for (int i = 0; i < NUM_PUZZLES * 2; i++)
		serializedLara.puzzles_combo.push_back(Lara.PuzzlesCombo[i]);

	for (int i = 0; i < NUM_KEYS; i++)
		serializedLara.keys.push_back(Lara.Keys[i]);

	for (int i = 0; i < NUM_KEYS * 2; i++)
		serializedLara.keys_combo.push_back(Lara.KeysCombo[i]);

	for (int i = 0; i < NUM_PICKUPS; i++)
		serializedLara.pickups.push_back(Lara.Pickups[i]);

	for (int i = 0; i < NUM_PICKUPS * 2; i++)
		serializedLara.pickups_combo.push_back(Lara.PickupsCombo[i]);

	for (int i = 0; i < 15; i++)
		serializedLara.mesh_ptrs.push_back(Lara.meshPtrs[i]);

	for (int i = 0; i < 15; i++)
		serializedLara.wet.push_back(Lara.wet[i] == 1);

	auto arm = Save::CreateLaraArmInfo(fbb, 0, 0, 0, 0, 0, 0, 0, 0);

	serializedLara.left_arm = std::make_unique<Save::LaraArmInfoT>(new Save::LaraArmInfoT());
	Save::LaraArmInfoT* leftArm = serializedLara.left_arm.get();
	leftArm->anim_number = Lara.leftArm.animNumber;
	leftArm->flash_gun = Lara.leftArm.flash_gun;
	leftArm->frame_base = Lara.leftArm.frameBase;
	leftArm->frame_number = Lara.leftArm.frameNumber;
	leftArm->lock = Lara.leftArm.lock;
	leftArm->x_rot = Lara.leftArm.xRot;
	leftArm->y_rot = Lara.leftArm.yRot;
	leftArm->z_rot = Lara.leftArm.zRot;

	serializedLara.right_arm = std::make_unique<Save::LaraArmInfoT>(new Save::LaraArmInfoT());
	Save::LaraArmInfoT* rightArm = serializedLara.right_arm.get();
	rightArm->anim_number = Lara.rightArm.animNumber;
	rightArm->flash_gun = Lara.rightArm.flash_gun;
	rightArm->frame_base = Lara.rightArm.frameBase;
	rightArm->frame_number = Lara.rightArm.frameNumber;
	rightArm->lock = Lara.rightArm.lock;
	rightArm->x_rot = Lara.rightArm.xRot;
	rightArm->y_rot = Lara.rightArm.yRot;
	rightArm->z_rot = Lara.rightArm.zRot;
	
	serializedLara.air = Lara.air;
	serializedLara.beetle_life = Lara.BeetleLife;
	serializedLara.big_waterskin = Lara.big_waterskin;
	serializedLara.binoculars = Lara.Binoculars;
	serializedLara.burn = Lara.burn;
	serializedLara.burn_blue = Lara.burnBlue;
	serializedLara.burn_count = Lara.burnCount;
	serializedLara.burn_smoke = Lara.burnSmoke;
	serializedLara.busy = Lara.busy;
	serializedLara.calc_fall_speed = Lara.calcFallSpeed;
	serializedLara.can_monkey_swing = Lara.canMonkeySwing;
	serializedLara.climb_status = Lara.climbStatus;
	serializedLara.corner_x = Lara.cornerX;
	serializedLara.corner_z = Lara.cornerZ;
	serializedLara.crowbar = Lara.Crowbar;
	serializedLara.current_active = Lara.currentActive;
	serializedLara.current_x_vel = Lara.currentXvel;
	serializedLara.current_y_vel = Lara.currentYvel;
	serializedLara.current_z_vel = Lara.currentZvel;
	serializedLara.death_count = Lara.deathCount;
	serializedLara.dive_count = Lara.diveCount;
	serializedLara.extra_anim = Lara.ExtraAnim;
	//serializedLara.fall_speed = Lara.fallSpeed;
	serializedLara.fired = Lara.fired;
	serializedLara.flare_age = Lara.flareAge;
	serializedLara.flare_control_left = Lara.flareControlLeft;
	serializedLara.flare_frame = Lara.flareFrame;
	serializedLara.gun_status = Lara.gunStatus;
	serializedLara.gun_type = Lara.gunType;
	serializedLara.has_beetle_things = Lara.hasBeetleThings;
	serializedLara.has_fired = Lara.hasFired;
	serializedLara.head_x_rot = Lara.headXrot;
	serializedLara.head_y_rot = Lara.headYrot;
	serializedLara.head_z_rot = Lara.headZrot;
	serializedLara.highest_location = Lara.highestLocation;
	serializedLara.hit_direction = Lara.hitDirection;
	serializedLara.hit_frame = Lara.hitFrame;
	serializedLara.interacted_item = Lara.interactedItem;
	serializedLara.is_climbing = Lara.isClimbing;
	serializedLara.is_ducked = Lara.isDucked;
	serializedLara.is_moving = Lara.isMoving;
	serializedLara.item_number = Lara.itemNumber;
	serializedLara.keep_ducked = Lara.keepDucked;
	serializedLara.lasersight = Lara.Lasersight;
	serializedLara.last_gun_type = Lara.lastGunType;
	Save::Vector3 lastPos = Save::Vector3(Lara.lastPos.x, Lara.lastPos.y, Lara.lastPos.z);
	serializedLara.last_position = std::make_unique<Save::Vector3>(&lastPos);
	serializedLara.lit_torch = Lara.litTorch;
	serializedLara.location = Lara.location;
	serializedLara.location_pad  = Lara.locationPad;
	serializedLara.look = Lara.look;
	serializedLara.mine_l = Lara.mineL;
	serializedLara.mine_r = Lara.mineR;
	serializedLara.move_angle = Lara.moveAngle;
	serializedLara.move_count = Lara.moveCount;
	serializedLara.num_flares = Lara.NumFlares;
	serializedLara.num_small_medipacks = Lara.NumSmallMedipacks;
	serializedLara.num_large_medipacks = Lara.NumLargeMedipacks;
	serializedLara.old_busy = Lara.oldBusy;
	serializedLara.poisoned = Lara.poisoned;
	serializedLara.pose_count = Lara.poseCount;
	serializedLara.request_gun_type = Lara.requestGunType;
	serializedLara.rope_arc_back = Lara.ropeArcBack;
	serializedLara.rope_arc_front = Lara.ropeArcFront;
	serializedLara.rope_dframe = Lara.ropeDFrame;
	serializedLara.rope_direction = Lara.ropeDirection;
	serializedLara.rope_down_vel = Lara.ropeDownVel;
	serializedLara.rope_flag = Lara.ropeFlag;
	serializedLara.rope_framerate = Lara.ropeFrameRate;
	serializedLara.rope_frame = Lara.ropeFrame;
	serializedLara.rope_last_x = Lara.ropeLastX;
	serializedLara.rope_max_x_backward = Lara.ropeMaxXBackward;
	serializedLara.rope_max_x_forward = Lara.ropeMaxXForward;
	serializedLara.rope_offset = Lara.ropeOffset;
	serializedLara.rope_ptr = Lara.ropePtr;
	serializedLara.rope_segment = Lara.ropeSegment;
	serializedLara.rope_y = Lara.ropeY;
	serializedLara.secrets = Lara.Secrets;
	serializedLara.silencer = Lara.Silencer;
	serializedLara.small_waterskin = Lara.small_waterskin;
	serializedLara.spaz_effect_count = Lara.spazEffectCount;
	serializedLara.target_angles.push_back(Lara.targetAngles[0]);
	serializedLara.target_angles.push_back(Lara.targetAngles[1]);
	serializedLara.target_item_number = Lara.target - g_Level.Items.data();
	//serializedLara.tightrope = Lara.air;
	serializedLara.torch = Lara.Torch;
	serializedLara.torso_x_rot = Lara.torsoXrot;
	serializedLara.torso_y_rot = Lara.torsoYrot;
	serializedLara.torso_z_rot = Lara.torsoZrot;
	serializedLara.turn_rate = Lara.turnRate;
	serializedLara.uncontrollable = Lara.uncontrollable;
	serializedLara.vehicle = Lara.Vehicle;
	serializedLara.water_status = Lara.waterStatus;
	serializedLara.water_surface_dist = Lara.waterSurfaceDist;
	serializedLara.weapon_item = Lara.weaponItem;
	*/

	Save::SaveGameT sg;

	// Create the savegame header
	Save::SaveGameHeaderT* header = sg.header.get();
	header->level_name = g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->NameStringKey.c_str());
	header->days = (GameTimer / 30) / 86400;
	header->hours = ((GameTimer / 30) % 86400) / 3600;
	header->minutes = ((GameTimer / 30) / 60) % 6;
	header->seconds = (GameTimer / 30) % 60;
	header->level = CurrentLevel;
	header->level = GameTimer;
	header->level = ++LastSaveGame;

	Save::SaveGameStatisticsT* levelStatistics = sg.level.get();
	levelStatistics->ammo_hits = Savegame.Level.AmmoHits;
	levelStatistics->ammo_used = Savegame.Level.AmmoUsed;
	levelStatistics->kills = Savegame.Level.Kills;
	levelStatistics->medipacks_used = Savegame.Level.HealthUsed;
	levelStatistics->distance = Savegame.Level.Distance;
	levelStatistics->secrets = Savegame.Level.Secrets;
	levelStatistics->timer = Savegame.Level.Timer;

	Save::SaveGameStatisticsT* gameStatistics = sg.game.get();
	gameStatistics->ammo_hits = Savegame.Game.AmmoHits;
	gameStatistics->ammo_used = Savegame.Game.AmmoUsed;
	gameStatistics->kills = Savegame.Game.Kills;
	gameStatistics->medipacks_used = Savegame.Game.HealthUsed;
	gameStatistics->distance = Savegame.Game.Distance;
	gameStatistics->secrets = Savegame.Game.Secrets;
	gameStatistics->timer = Savegame.Game.Timer;

	for (const auto& itemToSerialize : g_Level.Items) {
		Save::ItemT serializedItem{};

		serializedItem.anim_number = itemToSerialize.animNumber;
		serializedItem.after_death = itemToSerialize.afterDeath;
		serializedItem.box_number = itemToSerialize.boxNumber;
		serializedItem.carried_item = itemToSerialize.carriedItem;
		serializedItem.current_anim_state = itemToSerialize.currentAnimState;
		serializedItem.fall_speed = itemToSerialize.fallspeed;
		serializedItem.fired_weapon = itemToSerialize.firedWeapon;
		serializedItem.flags = itemToSerialize.flags;
		serializedItem.floor = itemToSerialize.floor;
		serializedItem.frame_number = itemToSerialize.frameNumber;
		serializedItem.goal_anim_state = itemToSerialize.goalAnimState;
		serializedItem.hit_points = itemToSerialize.hitPoints;
		for (int i = 0; i < 7; i++)
			serializedItem.item_flags.push_back(itemToSerialize.itemFlags[i]);
		serializedItem.mesh_bits = itemToSerialize.meshBits;
		serializedItem.object_id = itemToSerialize.objectNumber;
		serializedItem.required_anim_state = itemToSerialize.requiredAnimState;
		serializedItem.room_number = itemToSerialize.roomNumber;
		serializedItem.speed = itemToSerialize.speed;
		serializedItem.timer = itemToSerialize.timer;
		serializedItem.touch_bits = itemToSerialize.touchBits;
		serializedItem.trigger_flags = itemToSerialize.triggerFlags;

		//serialize Items here
		serializedItems.push_back(Save::CreateItem(fbb, &serializedItem));
	}

	//sgb.add_ambient_track(4);
	//sgb.add_items(fbb.CreateVector(serializedItems));

	//auto serializedSave = sgb.Finish();
	fbb.Finish(Save::SaveGame::Pack(fbb, &sg));
	auto bufferToSerialize = fbb.GetBufferPointer();
	auto bufferSize = fbb.GetSize();

	std::ofstream fileOut{};
	fileOut.open(fileName, std::ios_base::binary | std::ios_base::out);
	fileOut.write((char*)bufferToSerialize, bufferSize);
	fileOut.close();

	return true;
}

bool SaveGame::Load(char* fileName)
{
	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());


	//read stuff here like this:

	//g_Level.Items.begin() -> replaces Items in g_Level with new ones until SaveGame Vector reaches and (possible out of bounds since g_Level Items has size of 1024, but logically impossible)
	// std::back_inserter(g_Level.Items) would append!
	/*std::transform(s->items()->begin(), s->items()->end(), g_Level.Items.begin(), [](const Save::Item& savedItem) {

		//transform savegame item into game item
		ITEM_INFO i{};
		i.hitPoints = savedItem.hit_points();
		i.animNumber = savedItem.anim_number();
		// and so on;
		return i;
		});*/

	return true;
}

bool SaveGame::LoadHeader(char* fileName, SaveGameHeader* header)
{
	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());

	header->levelName = s->header()->level_name()->str();
	header->days = s->header()->days();
	header->hours = s->header()->hours();
	header->minutes = s->header()->minutes();
	header->seconds = s->header()->seconds();
	header->level = s->header()->level();
	header->timer = s->header()->timer();
	header->count = s->header()->count();

	return true;
}
