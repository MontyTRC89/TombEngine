#include "framework.h"
#include "savegame.h"
#include "Lara.h"
#include "items.h"
#include "control/box.h"
#include "control/lot.h"
#include "spotcam.h"
#include "traps.h"
#include "floordata.h"
#include "room.h"
#include "sound/sound.h"
#include "level.h"
#include "setup.h"
#include "sound.h"
#include "flipeffect.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "fullblock_switch.h"
#include "itemdata/creature_info.h"
#include "Game/effects/lara_fx.h"
#include "Specific/savegame/flatbuffers/ten_savegame_generated.h"
#include "Game/misc.h"
#include "Game/puzzles_keys.h"
#include "Objects/TR4/Entity/tr4_littlebeetle.h"
#include "Objects/Generic/Object/rope.h"

#include <filesystem>

using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;
using namespace TEN::Entities::Generic;
using namespace TEN::Floordata;
using namespace flatbuffers;

namespace Save = TEN::Save;

const std::string SAVEGAME_PATH = "Save//";

GameStats Statistics;
SaveGameHeader SavegameInfos[SAVEGAME_MAX];

FileStream* SaveGame::m_stream;
std::vector<LuaVariable> SaveGame::m_luaVariables;
int SaveGame::LastSaveGame;

void LoadSavegameInfos()
{
	for (int i = 0; i < SAVEGAME_MAX; i++)
		SavegameInfos[i].Present = false;

	if (!std::filesystem::exists(SAVEGAME_PATH))
		return;

	// try to load the savegame
	for (int i = 0; i < SAVEGAME_MAX; i++)
	{
		auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(i);
		auto savegamePtr = fopen(fileName.c_str(), "rb");

		if (savegamePtr == NULL)
			continue;

		fclose(savegamePtr);

		SavegameInfos[i].Present = true;
		SaveGame::LoadHeader(i, &SavegameInfos[i]);

		fclose(savegamePtr);
	}

	return;
}

bool SaveGame::Save(int slot)
{
	auto fileName = std::string(SAVEGAME_PATH) + "savegame." + std::to_string(slot);

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
	sgLevelStatisticsBuilder.add_ammo_hits(Statistics.Level.AmmoHits);
	sgLevelStatisticsBuilder.add_ammo_used(Statistics.Level.AmmoUsed);
	sgLevelStatisticsBuilder.add_kills(Statistics.Level.Kills);
	sgLevelStatisticsBuilder.add_medipacks_used(Statistics.Level.HealthUsed);
	sgLevelStatisticsBuilder.add_distance(Statistics.Level.Distance);
	sgLevelStatisticsBuilder.add_secrets(Statistics.Level.Secrets);
	sgLevelStatisticsBuilder.add_timer(Statistics.Level.Timer);
	auto levelStatisticsOffset = sgLevelStatisticsBuilder.Finish();

	Save::SaveGameStatisticsBuilder sgGameStatisticsBuilder{ fbb };
	sgGameStatisticsBuilder.add_ammo_hits(Statistics.Game.AmmoHits);
	sgGameStatisticsBuilder.add_ammo_used(Statistics.Game.AmmoUsed);
	sgGameStatisticsBuilder.add_kills(Statistics.Game.Kills);
	sgGameStatisticsBuilder.add_medipacks_used(Statistics.Game.HealthUsed);
	sgGameStatisticsBuilder.add_distance(Statistics.Game.Distance);
	sgGameStatisticsBuilder.add_secrets(Statistics.Game.Secrets);
	sgGameStatisticsBuilder.add_timer(Statistics.Game.Timer);
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
	Save::Vector3 nextCornerPos = Save::Vector3(Lara.nextCornerPos.xPos, Lara.nextCornerPos.yPos, Lara.nextCornerPos.zPos);
	Save::Vector3 nextCornerRot = Save::Vector3(Lara.nextCornerPos.xRot, Lara.nextCornerPos.yRot, Lara.nextCornerPos.zRot);

	std::vector<int> laraTargetAngles{};
	laraTargetAngles.push_back(Lara.targetAngles[0]);
	laraTargetAngles.push_back(Lara.targetAngles[1]);
	auto laraTargetAnglesOffset = fbb.CreateVector(laraTargetAngles);

	Save::LaraTightropeInfoBuilder tightRope{ fbb };
	tightRope.add_balance(Lara.tightrope.balance);
	tightRope.add_can_go_off(Lara.tightrope.canGoOff);
	tightRope.add_tightrope_item(Lara.tightrope.tightropeItem);
	tightRope.add_time_on_tightrope(Lara.tightrope.timeOnTightrope);
	auto tightRopeOffset = tightRope.Finish();

	Save::HolsterInfoBuilder holsterInfo{ fbb };
	holsterInfo.add_back_holster((int)Lara.holsterInfo.backHolster);
	holsterInfo.add_left_holster((int)Lara.holsterInfo.leftHolster);
	holsterInfo.add_right_holster((int)Lara.holsterInfo.rightHolster);
	auto holsterInfoOffset = holsterInfo.Finish();

	std::vector<flatbuffers::Offset<Save::CarriedWeaponInfo>> carriedWeapons;
	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		CarriedWeaponInfo* info = &Lara.Weapons[i];
		
		std::vector<flatbuffers::Offset<Save::AmmoInfo>> ammos;
		for (int j = 0; j < MAX_AMMOTYPE; j++)
		{
			Save::AmmoInfoBuilder ammo{ fbb };
			ammo.add_count(info->Ammo[j].getCount());
			ammo.add_is_infinite(info->Ammo[j].hasInfinite());
			auto ammoOffset = ammo.Finish();
			ammos.push_back(ammoOffset);
		}
		auto ammosOffset = fbb.CreateVector(ammos);
		
		Save::CarriedWeaponInfoBuilder serializedInfo{ fbb };
		serializedInfo.add_ammo(ammosOffset);
		serializedInfo.add_has_lasersight(info->HasLasersight);
		serializedInfo.add_has_silencer(info->HasSilencer);
		serializedInfo.add_present(info->Present);
		serializedInfo.add_selected_ammo(info->SelectedAmmo);
		auto serializedInfoOffset = serializedInfo.Finish();

		carriedWeapons.push_back(serializedInfoOffset);
	}
	auto carriedWeaponsOffset = fbb.CreateVector(carriedWeapons);

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
	lara.add_next_corner_position(&nextCornerPos);
	lara.add_next_corner_rotation(&nextCornerRot);
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
	lara.add_rope_count(Lara.ropeCount);
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
	lara.add_tightrope(tightRopeOffset);
	lara.add_holster_info(holsterInfoOffset);
	lara.add_weapons(carriedWeaponsOffset);

	auto laraOffset = lara.Finish();

	for (auto& itemToSerialize : g_Level.Items) 
	{
		OBJECT_INFO* obj = &Objects[itemToSerialize.objectNumber];

		std::vector<int> itemFlags;
		for (int i = 0; i < 7; i++)
			itemFlags.push_back(itemToSerialize.itemFlags[i]);
		auto itemFlagsOffset = fbb.CreateVector(itemFlags);
				
		flatbuffers::Offset<Save::Creature> creatureOffset;

		if (Objects[itemToSerialize.objectNumber].intelligent 
			&& itemToSerialize.data.is<CREATURE_INFO>())
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

		Save::Position position = Save::Position(
			(int32_t)itemToSerialize.pos.xPos,
			(int32_t)itemToSerialize.pos.yPos,
			(int32_t)itemToSerialize.pos.zPos,
			(int32_t)itemToSerialize.pos.xRot,
			(int32_t)itemToSerialize.pos.yRot,
			(int32_t)itemToSerialize.pos.zRot);

		Save::ItemBuilder serializedItem{ fbb };

		serializedItem.add_anim_number(itemToSerialize.animNumber - obj->animIndex);
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
		serializedItem.add_position(&position);
		serializedItem.add_required_anim_state(itemToSerialize.requiredAnimState);
		serializedItem.add_room_number(itemToSerialize.roomNumber);
		serializedItem.add_speed(itemToSerialize.speed);
		serializedItem.add_timer(itemToSerialize.timer);
		serializedItem.add_touch_bits(itemToSerialize.touchBits);
		serializedItem.add_trigger_flags(itemToSerialize.triggerFlags);
		serializedItem.add_triggered((itemToSerialize.flags & (TRIGGERED | CODE_BITS | ONESHOT)) != 0);
		serializedItem.add_active(itemToSerialize.active);
		serializedItem.add_status(itemToSerialize.status);
		serializedItem.add_gravity_status(itemToSerialize.gravityStatus);
		serializedItem.add_hit_stauts(itemToSerialize.hitStatus);
		serializedItem.add_poisoned(itemToSerialize.poisoned);
		serializedItem.add_ai_bits(itemToSerialize.aiBits);
		serializedItem.add_collidable(itemToSerialize.collidable);
		serializedItem.add_looked_at(itemToSerialize.lookedAt);
		serializedItem.add_swap_mesh_flags(itemToSerialize.swapMeshFlags);

		if (Objects[itemToSerialize.objectNumber].intelligent 
			&& itemToSerialize.data.is<CREATURE_INFO>())
		{
			serializedItem.add_data_type(Save::ItemData::Creature);
			serializedItem.add_data(creatureOffset.Union());
		}
		else if (itemToSerialize.data.is<short>())
		{
			short& data = itemToSerialize.data;
			serializedItem.add_data_type(Save::ItemData::Short);
			serializedItem.add_data(data);
		}
		else if (itemToSerialize.data.is<int>())
		{
			int& data = itemToSerialize.data;
			serializedItem.add_data_type(Save::ItemData::Int);
			serializedItem.add_data(data);
		}

		auto serializedItemOffset = serializedItem.Finish();
		serializedItems.push_back(serializedItemOffset);
	}

	auto serializedItemsOffset = fbb.CreateVector(serializedItems);

	// Soundtrack playheads
	auto bgmTrackData = GetSoundTrackNameAndPosition(SOUNDTRACK_PLAYTYPE::BGM);
	auto oneshotTrackData = GetSoundTrackNameAndPosition(SOUNDTRACK_PLAYTYPE::OneShot);
	auto bgmTrackOffset = fbb.CreateString(bgmTrackData.first);
	auto oneshotTrackOffset = fbb.CreateString(oneshotTrackData.first);

	// Legacy soundtrack map
	std::vector<int> soundTrackMap;
	for (auto& track : SoundTracks) { soundTrackMap.push_back(track.Mask); }
	auto soundtrackMapOffset = fbb.CreateVector(soundTrackMap);

	// Flipmaps
	std::vector<int> flipMaps;
	for (int i = 0; i < MAX_FLIPMAP; i++)
		flipMaps.push_back(FlipMap[i] >> 8);
	auto flipMapsOffset = fbb.CreateVector(flipMaps);

	std::vector<int> flipStats;
	for (int i = 0; i < MAX_FLIPMAP; i++)
		flipStats.push_back(FlipStats[i]);
	auto flipStatsOffset = fbb.CreateVector(flipStats);

	// Cameras
	std::vector<flatbuffers::Offset<Save::FixedCamera>> cameras;
	for (int i = 0; i < g_Level.Cameras.size(); i++)
	{
		Save::FixedCameraBuilder camera{ fbb };
		camera.add_flags(g_Level.Cameras[i].flags);
		cameras.push_back(camera.Finish());
	}
	auto camerasOffset = fbb.CreateVector(cameras);

	// Sinks
	std::vector<flatbuffers::Offset<Save::Sink>> sinks;
	for (int i = 0; i < g_Level.Sinks.size(); i++)
	{
		Save::SinkBuilder sink{ fbb };
		sink.add_flags(g_Level.Sinks[i].strength);
		sinks.push_back(sink.Finish());
	}
	auto sinksOffset = fbb.CreateVector(sinks);

	// Flyby cameras
	std::vector<flatbuffers::Offset<Save::FlyByCamera>> flybyCameras;
	for (int i = 0; i < NumberSpotcams; i++)
	{
		Save::FlyByCameraBuilder flyby{ fbb };
		flyby.add_flags(SpotCam[i].flags);
		flybyCameras.push_back(flyby.Finish());
	}
	auto flybyCamerasOffset = fbb.CreateVector(flybyCameras);

	// Static meshes
	std::vector<flatbuffers::Offset<Save::StaticMeshInfo>> staticMeshes;
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		ROOM_INFO* room = &g_Level.Rooms[i];
		for (int j = 0; j < room->mesh.size(); j++)
		{
			Save::StaticMeshInfoBuilder staticMesh{ fbb };
			staticMesh.add_flags(room->mesh[j].flags);
			staticMesh.add_room_number(i);
			staticMeshes.push_back(staticMesh.Finish());
		}
	}
	auto staticMeshesOffset = fbb.CreateVector(staticMeshes);

	// Particle enemies
	std::vector<flatbuffers::Offset<Save::BatInfo>> bats;
	for (int i = 0; i < NUM_BATS; i++)
	{
		BAT_STRUCT* bat = &Bats[i];

		Save::BatInfoBuilder batInfo{ fbb };

		batInfo.add_counter(bat->counter);
		batInfo.add_on(bat->on);
		batInfo.add_room_number(bat->roomNumber);
		batInfo.add_x(bat->pos.xPos);
		batInfo.add_y(bat->pos.yPos);
		batInfo.add_z(bat->pos.zPos);
		batInfo.add_x_rot(bat->pos.xRot);
		batInfo.add_y_rot(bat->pos.yRot);
		batInfo.add_z_rot(bat->pos.zRot);

		bats.push_back(batInfo.Finish());
	}
	auto batsOffset = fbb.CreateVector(bats);

	std::vector<flatbuffers::Offset<Save::SpiderInfo>> spiders;
	for (int i = 0; i < NUM_SPIDERS; i++)
	{
		SPIDER_STRUCT* spider = &Spiders[i];

		Save::SpiderInfoBuilder spiderInfo{ fbb };

		spiderInfo.add_flags(spider->flags);
		spiderInfo.add_on(spider->on);
		spiderInfo.add_room_number(spider->roomNumber);
		spiderInfo.add_x(spider->pos.xPos);
		spiderInfo.add_y(spider->pos.yPos);
		spiderInfo.add_z(spider->pos.zPos);
		spiderInfo.add_x_rot(spider->pos.xRot);
		spiderInfo.add_y_rot(spider->pos.yRot);
		spiderInfo.add_z_rot(spider->pos.zRot);

		spiders.push_back(spiderInfo.Finish());
	}
	auto spidersOffset = fbb.CreateVector(spiders);

	std::vector<flatbuffers::Offset<Save::RatInfo>> rats;
	for (int i = 0; i < NUM_RATS; i++)
	{
		RAT_STRUCT* rat = &Rats[i];

		Save::RatInfoBuilder ratInfo{ fbb };

		ratInfo.add_flags(rat->flags);
		ratInfo.add_on(rat->on);
		ratInfo.add_room_number(rat->roomNumber);
		ratInfo.add_x(rat->pos.xPos);
		ratInfo.add_y(rat->pos.yPos);
		ratInfo.add_z(rat->pos.zPos);
		ratInfo.add_x_rot(rat->pos.xRot);
		ratInfo.add_y_rot(rat->pos.yRot);
		ratInfo.add_z_rot(rat->pos.zRot);

		rats.push_back(ratInfo.Finish());
	}
	auto ratsOffset = fbb.CreateVector(rats);

	std::vector<flatbuffers::Offset<Save::ScarabInfo>> scarabs;
	for (int i = 0; i < NUM_BATS; i++)
	{
		SCARAB_STRUCT* scarab = &Scarabs[i];

		Save::ScarabInfoBuilder scarabInfo{ fbb };

		scarabInfo.add_flags(scarab->flags);
		scarabInfo.add_on(scarab->on);
		scarabInfo.add_room_number(scarab->roomNumber);
		scarabInfo.add_x(scarab->pos.xPos);
		scarabInfo.add_y(scarab->pos.yPos);
		scarabInfo.add_z(scarab->pos.zPos);
		scarabInfo.add_x_rot(scarab->pos.xRot);
		scarabInfo.add_y_rot(scarab->pos.yRot);
		scarabInfo.add_z_rot(scarab->pos.zRot);

		scarabs.push_back(scarabInfo.Finish());
	}
	auto scarabsOffset = fbb.CreateVector(scarabs);

	// Rope
	flatbuffers::Offset<Save::Rope> ropeOffset;
	flatbuffers::Offset<Save::Pendulum> pendulumOffset;
	flatbuffers::Offset<Save::Pendulum> alternatePendulumOffset;

	if (Lara.ropePtr != -1)
	{
		ROPE_STRUCT* rope = &Ropes[Lara.ropePtr];

		std::vector<const Save::Vector3*> segments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			segments.push_back(&Save::Vector3(
				rope->segment[i].x, 
				rope->segment[i].y, 
				rope->segment[i].z));
		auto segmentsOffset = fbb.CreateVector(segments);

		std::vector<const Save::Vector3*> velocities;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			velocities.push_back(&Save::Vector3(
				rope->velocity[i].x,
				rope->velocity[i].y,
				rope->velocity[i].z));
		auto velocitiesOffset = fbb.CreateVector(velocities);

		std::vector<const Save::Vector3*> normalisedSegments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			normalisedSegments.push_back(&Save::Vector3(
				rope->normalisedSegment[i].x,
				rope->normalisedSegment[i].y,
				rope->normalisedSegment[i].z));
		auto normalisedSegmentsOffset = fbb.CreateVector(normalisedSegments);

		std::vector<const Save::Vector3*> meshSegments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			meshSegments.push_back(&Save::Vector3(
				rope->meshSegment[i].x,
				rope->meshSegment[i].y,
				rope->meshSegment[i].z));
		auto meshSegmentsOffset = fbb.CreateVector(meshSegments);

		std::vector<const Save::Vector3*> coords;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			coords.push_back(&Save::Vector3(
				rope->coords[i].x,
				rope->coords[i].y,
				rope->coords[i].z));
		auto coordsOffset = fbb.CreateVector(coords);

		Save::RopeBuilder ropeInfo{ fbb };

		ropeInfo.add_segments(segmentsOffset);
		ropeInfo.add_velocities(velocitiesOffset);
		ropeInfo.add_mesh_segments(meshSegmentsOffset);
		ropeInfo.add_normalised_segments(normalisedSegmentsOffset);
		ropeInfo.add_coords(coordsOffset);
		ropeInfo.add_coiled(rope->coiled);
		ropeInfo.add_position(&Save::Vector3(
			rope->position.x,
			rope->position.y,
			rope->position.z));
		ropeInfo.add_segment_length(rope->segmentLength);

		ropeOffset = ropeInfo.Finish();

		Save::PendulumBuilder pendulumInfo{ fbb };
		pendulumInfo.add_node(CurrentPendulum.node);
		pendulumInfo.add_position(&Save::Vector3(
			CurrentPendulum.position.x,
			CurrentPendulum.position.y,
			CurrentPendulum.position.z));
		pendulumInfo.add_velocity(&Save::Vector3(
			CurrentPendulum.velocity.x,
			CurrentPendulum.velocity.y,
			CurrentPendulum.velocity.z));
		pendulumOffset = pendulumInfo.Finish();

		Save::PendulumBuilder alternatePendulumInfo{ fbb };
		alternatePendulumInfo.add_node(AlternatePendulum.node);
		alternatePendulumInfo.add_position(&Save::Vector3(
			AlternatePendulum.position.x,
			AlternatePendulum.position.y,
			AlternatePendulum.position.z));
		alternatePendulumInfo.add_velocity(&Save::Vector3(
			AlternatePendulum.velocity.x,
			AlternatePendulum.velocity.y,
			AlternatePendulum.velocity.z));
		alternatePendulumOffset = alternatePendulumInfo.Finish();
	}

	Save::SaveGameBuilder sgb{ fbb };

	sgb.add_header(headerOffset);
	sgb.add_level(levelStatisticsOffset);
	sgb.add_game(gameStatisticsOffset);
	sgb.add_lara(laraOffset);
	sgb.add_items(serializedItemsOffset);
	sgb.add_ambient_track(bgmTrackOffset);
	sgb.add_ambient_position(bgmTrackData.second);
	sgb.add_oneshot_track(oneshotTrackOffset);
	sgb.add_oneshot_position(oneshotTrackData.second);
	sgb.add_cd_flags(soundtrackMapOffset);
	sgb.add_flip_maps(flipMapsOffset);
	sgb.add_flip_stats(flipStatsOffset);
	sgb.add_flip_effect(FlipEffect);
	sgb.add_flip_status(FlipStatus);
	sgb.add_flip_timer(0);
	sgb.add_static_meshes(staticMeshesOffset);
	sgb.add_fixed_cameras(camerasOffset);
	sgb.add_bats(batsOffset);
	sgb.add_rats(ratsOffset);
	sgb.add_spiders(spidersOffset);
	sgb.add_scarabs(scarabsOffset);
	sgb.add_sinks(sinksOffset);
	sgb.add_flyby_cameras(flybyCamerasOffset);

	if (Lara.ropePtr != -1)
	{
		sgb.add_rope(ropeOffset);
		sgb.add_pendulum(pendulumOffset);
		sgb.add_alternate_pendulum(alternatePendulumOffset);
	}

	auto sg = sgb.Finish();
	fbb.Finish(sg);

	auto bufferToSerialize = fbb.GetBufferPointer();
	auto bufferSize = fbb.GetSize();

	if (!std::filesystem::exists(SAVEGAME_PATH))
		std::filesystem::create_directory(SAVEGAME_PATH);

	std::ofstream fileOut{};
	fileOut.open(fileName, std::ios_base::binary | std::ios_base::out);
	fileOut.write((char*)bufferToSerialize, bufferSize);
	fileOut.close();

	return true;
}

bool SaveGame::Load(int slot)
{
	auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(slot);

	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());

	// Flipmaps
	for (int i = 0; i < s->flip_stats()->size(); i++)
	{
		if (s->flip_stats()->Get(i) != 0)
			DoFlipMap(i);

		FlipMap[i] = s->flip_maps()->Get(i) << 8;
	}

	// Effects
	FlipEffect = s->flip_effect();
	FlipStatus = s->flip_status();
	//FlipTimer = s->flip_timer();

	// Restore soundtracks
	PlaySoundTrack(s->ambient_track()->str(), SOUNDTRACK_PLAYTYPE::BGM, s->ambient_position());
	PlaySoundTrack(s->oneshot_track()->str(), SOUNDTRACK_PLAYTYPE::OneShot, s->oneshot_position());

	// Legacy soundtrack map
	for (int i = 0; i < s->cd_flags()->size(); i++)
	{
		// Safety check for cases when soundtrack map was externally modified and became smaller
		if (i >= SoundTracks.size())
			break;

		SoundTracks[i].Mask = s->cd_flags()->Get(i);
	}

	// Static objects
	for (int i = 0; i < s->static_meshes()->size(); i++)
	{
		auto staticMesh = s->static_meshes()->Get(i);
		auto room = &g_Level.Rooms[staticMesh->room_number()];
		if (i >= room->mesh.size())
			break;
		room->mesh[i].flags = staticMesh->flags();
		if (!room->mesh[i].flags)
		{
			short roomNumber = staticMesh->room_number();
			FLOOR_INFO* floor = GetFloor(room->mesh[i].pos.xPos, room->mesh[i].pos.yPos, room->mesh[i].pos.zPos, &roomNumber);
			TestTriggers(room->mesh[i].pos.xPos, room->mesh[i].pos.yPos, room->mesh[i].pos.zPos, staticMesh->room_number(), true, 0);
			floor->Stopper = false;
		}
	}

	// Cameras 
	for (int i = 0; i < s->fixed_cameras()->size(); i++)
	{
		if (i < g_Level.Cameras.size())
			g_Level.Cameras[i].flags = s->fixed_cameras()->Get(i)->flags();
	}

	// Sinks 
	for (int i = 0; i < s->sinks()->size(); i++)
	{
		if (i < g_Level.Sinks.size())
			g_Level.Sinks[i].strength = s->sinks()->Get(i)->flags();
	}

	// Flyby cameras 
	for (int i = 0; i < s->flyby_cameras()->size(); i++)
	{
		if (i < NumberSpotcams)
			SpotCam[i].flags = s->flyby_cameras()->Get(i)->flags();
	}

	ZeroMemory(&Lara, sizeof(LaraInfo));

	// Items
	for (int i = 0; i < s->items()->size(); i++)
	{
		const Save::Item* savedItem = s->items()->Get(i);

		short itemNumber = i;
		bool dynamicItem = false;

		if (i >= g_Level.NumItems)
		{
			// Items beyond items level space must be active
			if (!savedItem->active())
				continue;

			// Items beyond items level space must be initialised differently
			itemNumber = CreateItem();
			if (itemNumber == NO_ITEM)
				continue;
			dynamicItem = true;
		}

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[item->objectNumber];

		if (!dynamicItem)
		{
			// Kill immediately item if already killed and continue
			if (savedItem->flags() & IFLAG_KILLED)
			{
				if (obj->floor != nullptr)
					UpdateBridgeItem(itemNumber, true);

				KillItem(i);
				item->status = ITEM_DEACTIVATED;
				item->flags |= ONESHOT;
				continue;
			}

			// If not triggered, don't load remaining data
			if (item->objectNumber != ID_LARA && !(savedItem->flags() & (TRIGGERED | CODE_BITS | ONESHOT)))
				continue;
		}

		item->pos.xPos = savedItem->position()->x_pos();
		item->pos.yPos = savedItem->position()->y_pos();
		item->pos.zPos = savedItem->position()->z_pos();
		item->pos.xRot = savedItem->position()->x_rot();
		item->pos.yRot = savedItem->position()->y_rot();
		item->pos.zRot = savedItem->position()->z_rot();

		short roomNumber = savedItem->room_number();

		if (dynamicItem)
		{
			item->roomNumber = roomNumber;

			InitialiseItem(itemNumber);
			
			// InitialiseItem could overwrite position so restore it
			item->pos.xPos = savedItem->position()->x_pos();
			item->pos.yPos = savedItem->position()->y_pos();
			item->pos.zPos = savedItem->position()->z_pos();
			item->pos.xRot = savedItem->position()->x_rot();
			item->pos.yRot = savedItem->position()->y_rot();
			item->pos.zRot = savedItem->position()->z_rot();
		}

		item->speed = savedItem->speed();
		item->fallspeed = savedItem->fall_speed();

		// Do the correct way for assigning new room number
		if (item->objectNumber == ID_LARA)
		{
			LaraItem->location.roomNumber = roomNumber;
			LaraItem->location.yNumber = item->pos.yPos;
			item->roomNumber = roomNumber;
			Lara.itemNumber = i;
			LaraItem = item;
			UpdateItemRoom(item, -LARA_HEIGHT / 2);
		}
		else
		{
			if (item->roomNumber != roomNumber)
				ItemNewRoom(i, roomNumber);

			if (obj->shadowSize)
			{
				FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			}
		}

		// Animations
		item->currentAnimState = savedItem->current_anim_state();
		item->requiredAnimState = savedItem->required_anim_state();
		item->goalAnimState = savedItem->goal_anim_state();
		item->animNumber = obj->animIndex + savedItem->anim_number();
		item->frameNumber = savedItem->frame_number();

		// Hit points
		item->hitPoints = savedItem->hit_points();

		// Flags and timers
		for (int j = 0; j < 7; j++)
			item->itemFlags[j] = savedItem->item_flags()->Get(j);
		item->timer = savedItem->timer();
		item->triggerFlags = savedItem->trigger_flags();
		item->flags = savedItem->flags();

		// Carried item
		item->carriedItem = savedItem->carried_item();

		// Activate item if needed
		if (savedItem->active() && !item->active)
			AddActiveItem(i);

		item->active = savedItem->active();
		item->hitStatus = savedItem->hit_stauts();
		item->status = savedItem->status();
		item->aiBits = savedItem->ai_bits();
		item->gravityStatus = savedItem->gravity_status();
		item->collidable = savedItem->collidable();
		item->lookedAt = savedItem->looked_at();
		item->poisoned = savedItem->poisoned();

		// Creature data for intelligent items
		if (item->objectNumber != ID_LARA && obj->intelligent)
		{
			EnableBaddieAI(i, true);

			auto creature = GetCreatureInfo(item);
			auto data = savedItem->data();
			auto savedCreature = (Save::Creature*)data;

			if (savedCreature == nullptr)
				continue;

			creature->alerted = savedCreature->alerted();
			creature->LOT.canJump = savedCreature->can_jump();
			creature->LOT.canMonkey = savedCreature->can_monkey();
			if (savedCreature->enemy() >= 0)
				creature->enemy = &g_Level.Items[savedCreature->enemy()];
			creature->flags = savedCreature->flags();
			creature->headLeft = savedCreature->head_left();
			creature->headRight = savedCreature->head_right();
			creature->hurtByLara = savedCreature->hurt_by_lara();
			creature->LOT.isAmphibious = savedCreature->is_amphibious();
			creature->LOT.isJumping = savedCreature->is_jumping();
			creature->LOT.isMonkeying = savedCreature->is_monkeying();
			for (int j = 0; j < 4; j++)
				creature->jointRotation[j] = savedCreature->joint_rotation()->Get(j);
			creature->jumpAhead = savedCreature->jump_ahead();
			creature->maximumTurn = savedCreature->maximum_turn();
			creature->monkeyAhead = savedCreature->monkey_ahead();
			creature->mood = (MOOD_TYPE)savedCreature->mood();
			creature->patrol2 = savedCreature->patrol2();
			creature->reachedGoal = savedCreature->reached_goal();
		}
		else if (savedItem->data_type() == Save::ItemData::Short)
		{
			auto data = savedItem->data();
			auto savedData = (Save::Short*)data;
			item->data = savedData->scalar();
		}

		// Mesh stuff
		item->meshBits = savedItem->mesh_bits();
		item->swapMeshFlags = savedItem->swap_mesh_flags();

		// Now some post-load specific hacks for objects
		if (item->objectNumber >= ID_PUZZLE_HOLE1 
			&& item->objectNumber <= ID_PUZZLE_HOLE16 
			&& (item->status == ITEM_ACTIVE
				|| item->status == ITEM_DEACTIVATED))
		{
			item->objectNumber = (GAME_OBJECT_ID)((int)item->objectNumber + ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1);
			item->animNumber = Objects[item->objectNumber].animIndex + savedItem->anim_number();
		}

		if ((item->objectNumber >= ID_SMASH_OBJECT1)
			&& (item->objectNumber <= ID_SMASH_OBJECT8)
			&& (item->flags & ONESHOT))
			item->meshBits = 0x00100;

		if (obj->floor != nullptr)
			UpdateBridgeItem(itemNumber);
	}

	for (int i = 0; i < s->bats()->size(); i++)
	{
		auto batInfo = s->bats()->Get(i);
		BAT_STRUCT* bat = &Bats[i];

		bat->on = batInfo->on();
		bat->counter = batInfo->counter();
		bat->roomNumber = batInfo->room_number();
		bat->pos.xPos = batInfo->x();
		bat->pos.yPos = batInfo->y();
		bat->pos.zPos = batInfo->z();
		bat->pos.xRot = batInfo->x_rot();
		bat->pos.yRot = batInfo->y_rot();
		bat->pos.zRot = batInfo->z_rot();
	}

	for (int i = 0; i < s->rats()->size(); i++)
	{
		auto ratInfo = s->rats()->Get(i);
		RAT_STRUCT* rat = &Rats[i];

		rat->on = ratInfo->on();
		rat->flags = ratInfo->flags();
		rat->roomNumber = ratInfo->room_number();
		rat->pos.xPos = ratInfo->x();
		rat->pos.yPos = ratInfo->y();
		rat->pos.zPos = ratInfo->z();
		rat->pos.xRot = ratInfo->x_rot();
		rat->pos.yRot = ratInfo->y_rot();
		rat->pos.zRot = ratInfo->z_rot();
	}

	for (int i = 0; i < s->spiders()->size(); i++)
	{
		auto spiderInfo = s->spiders()->Get(i);
		SPIDER_STRUCT* spider = &Spiders[i];

		spider->on = spiderInfo->on();
		spider->flags = spiderInfo->flags();
		spider->roomNumber = spiderInfo->room_number();
		spider->pos.xPos = spiderInfo->x();
		spider->pos.yPos = spiderInfo->y();
		spider->pos.zPos = spiderInfo->z();
		spider->pos.xRot = spiderInfo->x_rot();
		spider->pos.yRot = spiderInfo->y_rot();
		spider->pos.zRot = spiderInfo->z_rot();
	}

	for (int i = 0; i < s->scarabs()->size(); i++)
	{
		auto scarabInfo = s->scarabs()->Get(i);
		SCARAB_STRUCT* scarab = &Scarabs[i];

		scarab->on = scarabInfo->on();
		scarab->flags = scarabInfo->flags();
		scarab->roomNumber = scarabInfo->room_number();
		scarab->pos.xPos = scarabInfo->x();
		scarab->pos.yPos = scarabInfo->y();
		scarab->pos.zPos = scarabInfo->z();
		scarab->pos.xRot = scarabInfo->x_rot();
		scarab->pos.yRot = scarabInfo->y_rot();
		scarab->pos.zRot = scarabInfo->z_rot();
	}

	JustLoaded = 1;	

	// Lara
	ZeroMemory(Lara.Puzzles, NUM_PUZZLES * sizeof(int));
	for (int i = 0; i < s->lara()->puzzles()->size(); i++)
	{
		Lara.Puzzles[i] = s->lara()->puzzles()->Get(i);
	}

	ZeroMemory(Lara.PuzzlesCombo, NUM_PUZZLES * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->puzzles_combo()->size(); i++)
	{
		Lara.PuzzlesCombo[i] = s->lara()->puzzles_combo()->Get(i);
	}

	ZeroMemory(Lara.Keys, NUM_KEYS * sizeof(int));
	for (int i = 0; i < s->lara()->keys()->size(); i++)
	{
		Lara.Keys[i] = s->lara()->keys()->Get(i);
	}

	ZeroMemory(Lara.KeysCombo, NUM_KEYS * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->keys_combo()->size(); i++)
	{
		Lara.KeysCombo[i] = s->lara()->keys_combo()->Get(i);
	}

	ZeroMemory(Lara.Pickups, NUM_PICKUPS * sizeof(int));
	for (int i = 0; i < s->lara()->pickups()->size(); i++)
	{
		Lara.Pickups[i] = s->lara()->pickups()->Get(i);
	}

	ZeroMemory(Lara.PickupsCombo, NUM_PICKUPS * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->pickups_combo()->size(); i++)
	{
		Lara.Pickups[i] = s->lara()->pickups_combo()->Get(i);
	}

	ZeroMemory(Lara.Examines, NUM_EXAMINES * sizeof(int));
	for (int i = 0; i < s->lara()->examines()->size(); i++)
	{
		Lara.Examines[i] = s->lara()->examines()->Get(i);
	}

	ZeroMemory(Lara.ExaminesCombo, NUM_EXAMINES * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->examines_combo()->size(); i++)
	{
		Lara.ExaminesCombo[i] = s->lara()->examines_combo()->Get(i);
	}

	for (int i = 0; i < s->lara()->mesh_ptrs()->size(); i++)
	{
		Lara.meshPtrs[i] = s->lara()->mesh_ptrs()->Get(i);
	}

	for (int i = 0; i < 15; i++)
	{
		Lara.wet[i] = s->lara()->wet()->Get(i);
	}

	Lara.air = s->lara()->air();
	Lara.BeetleLife = s->lara()->beetle_life();
	Lara.big_waterskin = s->lara()->big_waterskin();
	Lara.Binoculars = s->lara()->binoculars();
	Lara.burn = s->lara()->burn();
	Lara.burnBlue = s->lara()->burn_blue();
	Lara.burnCount = s->lara()->burn_count();
	Lara.burnSmoke = s->lara()->burn_smoke();
	Lara.busy = s->lara()->busy();
	Lara.calcFallSpeed = s->lara()->calc_fall_speed();
	Lara.canMonkeySwing = s->lara()->can_monkey_swing();
	Lara.climbStatus = s->lara()->climb_status();
	Lara.nextCornerPos = PHD_3DPOS(
		s->lara()->next_corner_position()->x(),
		s->lara()->next_corner_position()->y(),
		s->lara()->next_corner_position()->z(),
		s->lara()->next_corner_rotation()->x(),
		s->lara()->next_corner_rotation()->y(),
		s->lara()->next_corner_rotation()->z());
	Lara.Crowbar = s->lara()->crowbar();
	Lara.currentActive = s->lara()->current_active();
	Lara.currentXvel = s->lara()->current_x_vel();
	Lara.currentYvel = s->lara()->current_y_vel();
	Lara.currentZvel = s->lara()->current_z_vel();
	Lara.deathCount = s->lara()->death_count();
	Lara.diveCount = s->lara()->dive_count();
	Lara.ExtraAnim = s->lara()->extra_anim();
	Lara.fired = s->lara()->fired();
	Lara.flareAge = s->lara()->flare_age();
	Lara.flareControlLeft = s->lara()->flare_control_left();
	Lara.flareFrame = s->lara()->flare_frame();
	Lara.gunStatus = (LARA_GUN_STATUS)s->lara()->gun_status();
	Lara.gunType = (LARA_WEAPON_TYPE)s->lara()->gun_type();
	Lara.hasBeetleThings = s->lara()->has_beetle_things();
	Lara.hasFired = s->lara()->has_fired();
	Lara.headXrot = s->lara()->head_x_rot();
	Lara.headYrot = s->lara()->head_y_rot();
	Lara.headZrot = s->lara()->head_z_rot();
	Lara.highestLocation = s->lara()->highest_location();
	Lara.hitDirection = s->lara()->hit_direction();
	Lara.hitFrame = s->lara()->hit_frame();
	Lara.interactedItem = s->lara()->interacted_item();
	Lara.isClimbing = s->lara()->is_climbing();
	Lara.isDucked = s->lara()->is_ducked();
	Lara.isMoving = s->lara()->is_moving();
	Lara.itemNumber = s->lara()->item_number();
	Lara.keepDucked = s->lara()->keep_ducked();
	Lara.Lasersight = s->lara()->lasersight();
	Lara.lastGunType = (LARA_WEAPON_TYPE)s->lara()->last_gun_type();
	Lara.lastPos = PHD_VECTOR(
		s->lara()->last_position()->x(), 
		s->lara()->last_position()->y(),
		s->lara()->last_position()->z());
	Lara.leftArm.animNumber = s->lara()->left_arm()->anim_number();
	Lara.leftArm.flash_gun = s->lara()->left_arm()->flash_gun();
	Lara.leftArm.frameBase = s->lara()->left_arm()->frame_base();
	Lara.leftArm.frameNumber = s->lara()->left_arm()->frame_number();
	Lara.leftArm.lock = s->lara()->left_arm()->lock();
	Lara.leftArm.xRot = s->lara()->left_arm()->x_rot();
	Lara.leftArm.yRot = s->lara()->left_arm()->y_rot();
	Lara.leftArm.zRot = s->lara()->left_arm()->z_rot();
	Lara.litTorch = s->lara()->lit_torch();
	Lara.location = s->lara()->location();
	Lara.locationPad = s->lara()->location_pad();
	Lara.look = s->lara()->look();
	Lara.mineL = s->lara()->mine_l();
	Lara.mineR = s->lara()->mine_r();
	Lara.moveAngle = s->lara()->move_angle();
	Lara.moveCount = s->lara()->move_count();
	Lara.NumFlares = s->lara()->num_flares();
	Lara.NumLargeMedipacks = s->lara()->num_large_medipacks();
	Lara.NumSmallMedipacks = s->lara()->num_small_medipacks();
	Lara.oldBusy = s->lara()->old_busy();
	Lara.poisoned = s->lara()->poisoned();
	Lara.poseCount = s->lara()->pose_count();
	Lara.requestGunType = (LARA_WEAPON_TYPE)s->lara()->request_gun_type();
	Lara.rightArm.animNumber = s->lara()->right_arm()->anim_number();
	Lara.rightArm.flash_gun = s->lara()->right_arm()->flash_gun();
	Lara.rightArm.frameBase = s->lara()->right_arm()->frame_base();
	Lara.rightArm.frameNumber = s->lara()->right_arm()->frame_number();
	Lara.rightArm.lock = s->lara()->right_arm()->lock();
	Lara.rightArm.xRot = s->lara()->right_arm()->x_rot();
	Lara.rightArm.yRot = s->lara()->right_arm()->y_rot();
	Lara.rightArm.zRot = s->lara()->right_arm()->z_rot();
	Lara.ropeArcBack = s->lara()->rope_arc_back();
	Lara.ropeArcFront = s->lara()->rope_arc_front();
	Lara.ropeCount = s->lara()->rope_count();
	Lara.ropeDFrame = s->lara()->rope_dframe();
	Lara.ropeDirection = s->lara()->rope_direction();
	Lara.ropeDownVel = s->lara()->rope_down_vel();
	Lara.ropeFlag = s->lara()->rope_flag();
	Lara.ropeFrame = s->lara()->rope_frame();
	Lara.ropeFrameRate = s->lara()->rope_framerate();
	Lara.ropeLastX = s->lara()->rope_last_x();
	Lara.ropeMaxXBackward = s->lara()->rope_max_x_backward();
	Lara.ropeMaxXForward = s->lara()->rope_max_x_forward();
	Lara.ropeOffset = s->lara()->rope_offset();
	Lara.ropePtr = s->lara()->rope_ptr();
	Lara.ropeSegment = s->lara()->rope_segment();
	Lara.ropeY = s->lara()->rope_y();
	Lara.Secrets = s->lara()->secrets();
	Lara.Silencer = s->lara()->silencer();
	Lara.small_waterskin = s->lara()->small_waterskin();
	Lara.spazEffectCount = s->lara()->spaz_effect_count();
	Lara.target = (s->lara()->target_item_number() >= 0 ? &g_Level.Items[s->lara()->target_item_number()] : nullptr);
	Lara.targetAngles[0] = s->lara()->target_angles()->Get(0);
	Lara.targetAngles[1] = s->lara()->target_angles()->Get(1);
	Lara.Torch = s->lara()->torch();
	Lara.torsoXrot = s->lara()->torso_x_rot();
	Lara.torsoYrot = s->lara()->torso_y_rot();
	Lara.torsoZrot = s->lara()->torso_z_rot();
	Lara.turnRate = s->lara()->turn_rate();
	Lara.uncontrollable = s->lara()->uncontrollable();
	Lara.Vehicle = s->lara()->vehicle();
	Lara.waterStatus = (LARA_WATER_STATUS)s->lara()->water_status();
	Lara.waterSurfaceDist = s->lara()->water_surface_dist();
	Lara.weaponItem = s->lara()->weapon_item();
	Lara.holsterInfo.backHolster = (HOLSTER_SLOT)s->lara()->holster_info()->back_holster();
	Lara.holsterInfo.leftHolster = (HOLSTER_SLOT)s->lara()->holster_info()->left_holster();
	Lara.holsterInfo.rightHolster = (HOLSTER_SLOT)s->lara()->holster_info()->right_holster();
	Lara.tightrope.balance = s->lara()->tightrope()->balance();
	Lara.tightrope.canGoOff = s->lara()->tightrope()->can_go_off();
	Lara.tightrope.tightropeItem = s->lara()->tightrope()->tightrope_item();
	Lara.tightrope.timeOnTightrope = s->lara()->tightrope()->time_on_tightrope();

	for (int i = 0; i < s->lara()->weapons()->size(); i++)
	{
		auto info = s->lara()->weapons()->Get(i);

		for (int j = 0; j < info->ammo()->size(); j++)
		{
			Lara.Weapons[i].Ammo[j].setInfinite(info->ammo()->Get(j)->is_infinite());
			Lara.Weapons[i].Ammo[j] = info->ammo()->Get(j)->count();
		}
		Lara.Weapons[i].HasLasersight = info->has_lasersight();
		Lara.Weapons[i].HasSilencer = info->has_silencer();
		Lara.Weapons[i].Present = info->present();
		Lara.Weapons[i].SelectedAmmo = info->selected_ammo();
	}

	if (Lara.burn)
	{
		char flag = 0;
		Lara.burn = 0;
		if (Lara.burnSmoke)
		{
			flag = 1;
			Lara.burnSmoke = 0;
		}
		LaraBurn(LaraItem);
		if (flag)
			Lara.burnSmoke = 1;
	}

	// Rope
	if (Lara.ropePtr >= 0)
	{
		ROPE_STRUCT* rope = &Ropes[Lara.ropePtr];
		
		for (int i = 0; i < ROPE_SEGMENTS; i++)
		{
			rope->segment[i] = PHD_VECTOR(
				s->rope()->segments()->Get(i)->x(),
				s->rope()->segments()->Get(i)->y(),
				s->rope()->segments()->Get(i)->z());

			rope->normalisedSegment[i] = PHD_VECTOR(
				s->rope()->normalised_segments()->Get(i)->x(),
				s->rope()->normalised_segments()->Get(i)->y(),
				s->rope()->normalised_segments()->Get(i)->z());

			rope->meshSegment[i] = PHD_VECTOR(
				s->rope()->mesh_segments()->Get(i)->x(),
				s->rope()->mesh_segments()->Get(i)->y(),
				s->rope()->mesh_segments()->Get(i)->z());

			rope->coords[i] = PHD_VECTOR(
				s->rope()->coords()->Get(i)->x(),
				s->rope()->coords()->Get(i)->y(),
				s->rope()->coords()->Get(i)->z());

			rope->velocity[i] = PHD_VECTOR(
				s->rope()->velocities()->Get(i)->x(),
				s->rope()->velocities()->Get(i)->y(),
				s->rope()->velocities()->Get(i)->z());
		}

		rope->coiled = s->rope()->coiled();
		rope->active = s->rope()->active();
		rope->position = PHD_VECTOR(
			s->rope()->position()->x(),
			s->rope()->position()->y(),
			s->rope()->position()->z());

		CurrentPendulum.position = PHD_VECTOR(
			s->pendulum()->position()->x(),
			s->pendulum()->position()->y(),
			s->pendulum()->position()->z());

		CurrentPendulum.velocity = PHD_VECTOR(
			s->pendulum()->velocity()->x(),
			s->pendulum()->velocity()->y(),
			s->pendulum()->velocity()->z());

		CurrentPendulum.node = s->pendulum()->node();
		CurrentPendulum.rope = rope;

		AlternatePendulum.position = PHD_VECTOR(
			s->alternate_pendulum()->position()->x(),
			s->alternate_pendulum()->position()->y(),
			s->alternate_pendulum()->position()->z());

		AlternatePendulum.velocity = PHD_VECTOR(
			s->alternate_pendulum()->velocity()->x(),
			s->alternate_pendulum()->velocity()->y(),
			s->alternate_pendulum()->velocity()->z());

		AlternatePendulum.node = s->alternate_pendulum()->node();
		AlternatePendulum.rope = rope;
	}

	return true;
}

bool SaveGame::LoadHeader(int slot, SaveGameHeader* header)
{
	auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(slot);

	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());

	header->Level = s->header()->level();
	header->LevelName = s->header()->level_name()->str();
	header->Days = s->header()->days();
	header->Hours = s->header()->hours();
	header->Minutes = s->header()->minutes();
	header->Seconds = s->header()->seconds();
	header->Level = s->header()->level();
	header->Timer = s->header()->timer();
	header->Count = s->header()->count();

	return true;
}