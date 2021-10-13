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
#include <Game/misc.h>

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

	std::vector<flatbuffers::Offset< Save::Item>> serializedItems{};

	// Savegame header
	auto levelNameOffset = fbb.CreateString(g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->NameStringKey.c_str()));

	Save::SaveGameHeaderBuilder sghb{ fbb };
	sghb.add_level_name(levelNameOffset);
	sghb.add_days((GameTimer / 30) / 8640);
	sghb.add_hours(((GameTimer / 30) % 86400) / 3600);
	sghb.add_minutes(((GameTimer / 30) / 60) % 6);
	sghb.add_seconds((GameTimer / 30) % 60);
	sghb.add_level(CurrentLevel);
	sghb.add_timer(GameTimer);
	sghb.add_count(++LastSaveGame);
	auto headerOffset = sghb.Finish();

	Save::SaveGameStatisticsBuilder sgLevelStatisticsBuilder{ fbb };
	sgLevelStatisticsBuilder.add_ammo_hits(Savegame.Level.AmmoHits);
	sgLevelStatisticsBuilder.add_ammo_used(Savegame.Level.AmmoUsed);
	sgLevelStatisticsBuilder.add_kills(Savegame.Level.Kills);
	sgLevelStatisticsBuilder.add_medipacks_used(Savegame.Level.HealthUsed);
	sgLevelStatisticsBuilder.add_distance(Savegame.Level.Distance);
	sgLevelStatisticsBuilder.add_secrets(Savegame.Level.Secrets);
	sgLevelStatisticsBuilder.add_timer(Savegame.Level.Timer);
	auto levelStatisticsOffset = sgLevelStatisticsBuilder.Finish();

	Save::SaveGameStatisticsBuilder sgGameStatisticsBuilder{ fbb };
	sgGameStatisticsBuilder.add_ammo_hits(Savegame.Game.AmmoHits);
	sgGameStatisticsBuilder.add_ammo_used(Savegame.Game.AmmoUsed);
	sgGameStatisticsBuilder.add_kills(Savegame.Game.Kills);
	sgGameStatisticsBuilder.add_medipacks_used(Savegame.Game.HealthUsed);
	sgGameStatisticsBuilder.add_distance(Savegame.Game.Distance);
	sgGameStatisticsBuilder.add_secrets(Savegame.Game.Secrets);
	sgGameStatisticsBuilder.add_timer(Savegame.Game.Timer);
	auto gameStatisticsOffset = sgGameStatisticsBuilder.Finish();

	// Lara
	std::vector<int> puzzles;
	for (int i = 0; i < NUM_PUZZLES; i++)
		puzzles.push_back(Lara.Puzzles[i]);
	auto puzzlesOffset = fbb.CreateVector(puzzles);

	std::vector<int> puzzlesCombo;
	for (int i = 0; i < NUM_PUZZLES * 2; i++)
		puzzlesCombo.push_back(Lara.PuzzlesCombo[i]);
	auto puzzlesComboOffset = fbb.CreateVector(puzzlesCombo);

	std::vector<int> keys;
	for (int i = 0; i < NUM_KEYS; i++)
		keys.push_back(Lara.Keys[i]);
	auto keysOffset = fbb.CreateVector(keys);

	std::vector<int> keysCombo;
	for (int i = 0; i < NUM_KEYS * 2; i++)
		keysCombo.push_back(Lara.KeysCombo[i]);
	auto keysComboOffset = fbb.CreateVector(keysCombo);

	std::vector<int> pickups;
	for (int i = 0; i < NUM_PICKUPS; i++)
		pickups.push_back(Lara.Pickups[i]);
	auto pickupsOffset = fbb.CreateVector(pickups);

	std::vector<int> pickupsCombo;
	for (int i = 0; i < NUM_PICKUPS * 2; i++)
		pickupsCombo.push_back(Lara.PickupsCombo[i]);
	auto pickupsComboOffset = fbb.CreateVector(pickupsCombo);

	std::vector<int> examines;
	for (int i = 0; i < NUM_EXAMINES; i++)
		examines.push_back(Lara.Examines[i]);
	auto examinesOffset = fbb.CreateVector(examines);

	std::vector<int> examinesCombo;
	for (int i = 0; i < NUM_EXAMINES * 2; i++)
		examinesCombo.push_back(Lara.ExaminesCombo[i]);
	auto examinesComboOffset = fbb.CreateVector(examinesCombo);

	std::vector<int> meshPtrs;
	for (int i = 0; i < 15; i++)
		meshPtrs.push_back(Lara.meshPtrs[i]);
	auto meshPtrsOffset = fbb.CreateVector(meshPtrs);

	std::vector<byte> wet;
	for (int i = 0; i < 15; i++)
		wet.push_back(Lara.wet[i] == 1);
	auto wetOffset = fbb.CreateVector(wet);

	Save::LaraArmInfoBuilder leftArm{ fbb };
	leftArm.add_anim_number(Lara.leftArm.animNumber);
	leftArm.add_flash_gun(Lara.leftArm.flash_gun);
	leftArm.add_frame_base(Lara.leftArm.frameBase);
	leftArm.add_frame_number(Lara.leftArm.frameNumber);
	leftArm.add_lock(Lara.leftArm.lock);
	leftArm.add_x_rot(Lara.leftArm.xRot);
	leftArm.add_y_rot(Lara.leftArm.yRot);
	leftArm.add_z_rot(Lara.leftArm.zRot);
	auto leftArmOffset = leftArm.Finish();

	Save::LaraArmInfoBuilder rightArm{ fbb };
	rightArm.add_anim_number(Lara.rightArm.animNumber);
	rightArm.add_flash_gun(Lara.rightArm.flash_gun);
	rightArm.add_frame_base(Lara.rightArm.frameBase);
	rightArm.add_frame_number(Lara.rightArm.frameNumber);
	rightArm.add_lock(Lara.rightArm.lock);
	rightArm.add_x_rot(Lara.rightArm.xRot);
	rightArm.add_y_rot(Lara.rightArm.yRot);
	rightArm.add_z_rot(Lara.rightArm.zRot);
	auto rightArmOffset = rightArm.Finish();

	Save::Vector3 lastPos = Save::Vector3(Lara.lastPos.x, Lara.lastPos.y, Lara.lastPos.z);

	std::vector<int> laraTargetAngles{};
	laraTargetAngles.push_back(Lara.targetAngles[0]);
	laraTargetAngles.push_back(Lara.targetAngles[1]);
	auto laraTargetAnglesOffset = fbb.CreateVector(laraTargetAngles);

	Save::LaraBuilder lara{ fbb };
	lara.add_air(Lara.air);
	lara.add_beetle_life(Lara.BeetleLife);
	lara.add_big_waterskin(Lara.big_waterskin);
	lara.add_binoculars(Lara.Binoculars);
	lara.add_burn(Lara.burn);
	lara.add_burn_blue(Lara.burnBlue);
	lara.add_burn_count(Lara.burnCount);
	lara.add_burn_smoke(Lara.burnSmoke);
	lara.add_busy(Lara.busy);
	lara.add_calc_fall_speed(Lara.calcFallSpeed);
	lara.add_can_monkey_swing(Lara.canMonkeySwing);
	lara.add_climb_status(Lara.climbStatus);
	lara.add_corner_x(Lara.cornerX);
	lara.add_corner_z(Lara.cornerZ);
	lara.add_crowbar(Lara.Crowbar);
	lara.add_current_active(Lara.currentActive);
	lara.add_current_x_vel(Lara.currentXvel);
	lara.add_current_y_vel(Lara.currentYvel);
	lara.add_current_z_vel(Lara.currentZvel);
	lara.add_death_count(Lara.deathCount);
	lara.add_dive_count(Lara.diveCount);
	lara.add_extra_anim(Lara.ExtraAnim);
	lara.add_examines(examinesOffset);
	lara.add_examines_combo(examinesComboOffset);
	lara.add_fired(Lara.fired);
	lara.add_flare_age(Lara.flareAge);
	lara.add_flare_control_left(Lara.flareControlLeft);
	lara.add_flare_frame(Lara.flareFrame);
	lara.add_gun_status(Lara.gunStatus);
	lara.add_gun_type(Lara.gunType);
	lara.add_has_beetle_things(Lara.hasBeetleThings);
	lara.add_has_fired(Lara.hasFired);
	lara.add_head_x_rot(Lara.headXrot);
	lara.add_head_y_rot(Lara.headYrot);
	lara.add_head_z_rot(Lara.headZrot);
	lara.add_highest_location(Lara.highestLocation);
	lara.add_hit_direction(Lara.hitDirection);
	lara.add_hit_frame(Lara.hitFrame);
	lara.add_interacted_item(Lara.interactedItem);
	lara.add_is_climbing(Lara.isClimbing);
	lara.add_is_ducked(Lara.isDucked);
	lara.add_is_moving(Lara.isMoving);
	lara.add_item_number(Lara.itemNumber);
	lara.add_keep_ducked(Lara.keepDucked);
	lara.add_keys(keysOffset);
	lara.add_keys_combo(keysComboOffset);
	lara.add_lasersight(Lara.Lasersight);
	lara.add_last_gun_type(Lara.lastGunType);
	lara.add_last_position(&lastPos);
	lara.add_left_arm(leftArmOffset);
	lara.add_lit_torch(Lara.litTorch);
	lara.add_location(Lara.location);
	lara.add_location_pad(Lara.locationPad);
	lara.add_look(Lara.look);
	lara.add_mesh_ptrs(meshPtrsOffset);
	lara.add_mine_l(Lara.mineL);
	lara.add_mine_r(Lara.mineR);
	lara.add_move_angle(Lara.moveAngle);
	lara.add_move_count(Lara.moveCount);
	lara.add_num_flares(Lara.NumFlares);
	lara.add_num_small_medipacks(Lara.NumSmallMedipacks);
	lara.add_num_large_medipacks(Lara.NumLargeMedipacks);
	lara.add_old_busy(Lara.oldBusy);
	lara.add_puzzles(puzzlesOffset);
	lara.add_puzzles_combo(puzzlesComboOffset);
	lara.add_poisoned(Lara.poisoned);
	lara.add_pose_count(Lara.poseCount);
	lara.add_pickups(pickupsOffset);
	lara.add_pickups_combo(pickupsComboOffset);
	lara.add_request_gun_type(Lara.requestGunType);
	lara.add_right_arm(rightArmOffset);
	lara.add_rope_arc_back(Lara.ropeArcBack);
	lara.add_rope_arc_front(Lara.ropeArcFront);
	lara.add_rope_dframe(Lara.ropeDFrame);
	lara.add_rope_direction(Lara.ropeDirection);
	lara.add_rope_down_vel(Lara.ropeDownVel);
	lara.add_rope_flag(Lara.ropeFlag);
	lara.add_rope_framerate(Lara.ropeFrameRate);
	lara.add_rope_frame(Lara.ropeFrame);
	lara.add_rope_last_x(Lara.ropeLastX);
	lara.add_rope_max_x_backward(Lara.ropeMaxXBackward);
	lara.add_rope_max_x_forward(Lara.ropeMaxXForward);
	lara.add_rope_offset(Lara.ropeOffset);
	lara.add_rope_ptr(Lara.ropePtr);
	lara.add_rope_segment(Lara.ropeSegment);
	lara.add_rope_y(Lara.ropeY);
	lara.add_secrets(Lara.Secrets);
	lara.add_silencer(Lara.Silencer);
	lara.add_small_waterskin(Lara.small_waterskin);
	lara.add_spaz_effect_count(Lara.spazEffectCount);
	lara.add_target_angles(laraTargetAnglesOffset);
	lara.add_target_item_number(Lara.target - g_Level.Items.data());
	lara.add_torch(Lara.Torch);
	lara.add_torso_x_rot(Lara.torsoXrot);
	lara.add_torso_y_rot(Lara.torsoYrot);
	lara.add_torso_z_rot(Lara.torsoZrot);
	lara.add_turn_rate(Lara.turnRate);
	lara.add_uncontrollable(Lara.uncontrollable);
	lara.add_vehicle(Lara.Vehicle);
	lara.add_water_status(Lara.waterStatus);
	lara.add_water_surface_dist(Lara.waterSurfaceDist);
	lara.add_weapon_item(Lara.weaponItem);
	lara.add_wet(wetOffset);
	auto laraOffset = lara.Finish();

	// Weapon info
	/*
		std::vector
	LARA_WEAPON_TYPE::NUM_WEAPONS
	Save::CarriedWeaponInfoBuilder carriedWeaponInfo{ fbb };
	carriedWeaponInfo.add_ammo(Lara.Weapons)
	*/

	for (auto& itemToSerialize : g_Level.Items) 
	{
		std::vector<int> itemFlags;
		for (int i = 0; i < 7; i++)
			itemFlags.push_back(itemToSerialize.itemFlags[i]);
		auto itemFlagsOffset = fbb.CreateVector(itemFlags);
				
		flatbuffers::Offset<Save::Creature> creatureOffset;

		if (Objects[itemToSerialize.objectNumber].intelligent)
		{
			auto creature = GetCreatureInfo(&itemToSerialize);

			std::vector<int> jointRotations;
			for (int i = 0; i < 4; i++)
				jointRotations.push_back(creature->jointRotation[i]);
			auto jointRotationsOffset = fbb.CreateVector(jointRotations);

			Save::CreatureBuilder creatureBuilder{ fbb };

			creatureBuilder.add_alerted(creature->alerted);
			creatureBuilder.add_can_jump(creature->LOT.canJump);
			creatureBuilder.add_can_monkey(creature->LOT.canMonkey);
			creatureBuilder.add_enemy(creature->enemy - g_Level.Items.data());
			creatureBuilder.add_flags(creature->flags);
			creatureBuilder.add_head_left(creature->headLeft);
			creatureBuilder.add_head_right(creature->headRight);
			creatureBuilder.add_hurt_by_lara(creature->hurtByLara);
			creatureBuilder.add_is_amphibious(creature->LOT.isAmphibious);
			creatureBuilder.add_is_jumping(creature->LOT.isJumping);
			creatureBuilder.add_is_monkeying(creature->LOT.isMonkeying);
			creatureBuilder.add_joint_rotation(jointRotationsOffset);
			creatureBuilder.add_jump_ahead(creature->jumpAhead);
			creatureBuilder.add_maximum_turn(creature->maximumTurn);
			creatureBuilder.add_monkey_ahead(creature->monkeyAhead);
			creatureBuilder.add_mood(creature->mood);
			creatureBuilder.add_patrol2(creature->patrol2);
			creatureBuilder.add_reached_goal(creature->reachedGoal);

			creatureOffset = creatureBuilder.Finish();
		}

		Save::ItemBuilder serializedItem{ fbb };

		serializedItem.add_anim_number(itemToSerialize.animNumber);
		serializedItem.add_after_death(itemToSerialize.afterDeath);
		serializedItem.add_box_number(itemToSerialize.boxNumber);
		serializedItem.add_carried_item(itemToSerialize.carriedItem);
		serializedItem.add_current_anim_state(itemToSerialize.currentAnimState);
		serializedItem.add_fall_speed(itemToSerialize.fallspeed);
		serializedItem.add_fired_weapon(itemToSerialize.firedWeapon);
		serializedItem.add_flags(itemToSerialize.flags);
		serializedItem.add_floor(itemToSerialize.floor);
		serializedItem.add_frame_number(itemToSerialize.frameNumber);
		serializedItem.add_goal_anim_state(itemToSerialize.goalAnimState);
		serializedItem.add_hit_points(itemToSerialize.hitPoints);
		serializedItem.add_item_flags(itemFlagsOffset);
		serializedItem.add_mesh_bits(itemToSerialize.meshBits);
		serializedItem.add_object_id(itemToSerialize.objectNumber);
		serializedItem.add_required_anim_state(itemToSerialize.requiredAnimState);
		serializedItem.add_room_number(itemToSerialize.roomNumber);
		serializedItem.add_speed(itemToSerialize.speed);
		serializedItem.add_timer(itemToSerialize.timer);
		serializedItem.add_touch_bits(itemToSerialize.touchBits);
		serializedItem.add_trigger_flags(itemToSerialize.triggerFlags);

		if (Objects[itemToSerialize.objectNumber].intelligent)
		{
			serializedItem.add_data(creatureOffset);
		}

		auto serializedItemOffset = serializedItem.Finish();
		serializedItems.push_back(serializedItemOffset);

		//serialize Items here
		//serializedItems.push_back(Save::CreateItem(fbb, &serializedItem));
	}

	Save::SaveGameBuilder sgb{ fbb };
	sgb.add_header(headerOffset);
	sgb.add_level(levelStatisticsOffset);
	sgb.add_game(gameStatisticsOffset);
	sgb.add_lara(laraOffset);
	auto sg = sgb.Finish();
	fbb.Finish(sg);

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

	int ammoUsed1 = s->game()->ammo_used();
	int ammoUsed2 = s->level()->ammo_used();

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