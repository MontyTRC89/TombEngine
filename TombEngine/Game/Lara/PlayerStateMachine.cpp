#include "framework.h"
#include "Game/Lara/PlayerStateMachine.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara_basic.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_crawl.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_hang.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/lara_jump.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_objects.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_surface.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"

namespace TEN::Entities::Player
{
	using PlayerStateRoutine	 = std::function<void(ItemInfo* item, CollisionInfo* coll)>;
	using PlayerStateRoutinePair = std::pair<PlayerStateRoutine, PlayerStateRoutine>;
	using PlayerStateRoutineMap	 = std::unordered_map<int, PlayerStateRoutinePair>;

	static const auto PLAYER_STATE_ROUTINE_MAP = PlayerStateRoutineMap
	{
		{ LS_WALK_FORWARD, PlayerStateRoutinePair(lara_as_walk_forward, lara_col_walk_forward) },
		{ LS_RUN_FORWARD, PlayerStateRoutinePair(lara_as_run_forward, lara_col_run_forward) },
		{ LS_IDLE, PlayerStateRoutinePair(lara_as_idle, lara_col_idle) },
		{ LS_JUMP_FORWARD, PlayerStateRoutinePair(lara_as_jump_forward, lara_col_jump_forward) },
		{ LS_POSE, PlayerStateRoutinePair(lara_as_pose, lara_col_idle) },
		{ LS_RUN_BACK, PlayerStateRoutinePair(lara_as_run_back, lara_col_run_back) },
		{ LS_TURN_RIGHT_SLOW, PlayerStateRoutinePair(lara_as_turn_right_slow, lara_col_turn_right_slow) },
		{ LS_TURN_LEFT_SLOW, PlayerStateRoutinePair(lara_as_turn_left_slow, lara_col_turn_left_slow) },
		{ LS_DEATH, PlayerStateRoutinePair(lara_as_death, lara_col_death) },
		{ LS_FREEFALL, PlayerStateRoutinePair(lara_as_freefall, lara_col_freefall) },
		{ LS_HANG, PlayerStateRoutinePair(lara_as_hang, lara_col_hang) },
		{ LS_REACH, PlayerStateRoutinePair(lara_as_reach, lara_col_reach) },
		{ LS_SPLAT, PlayerStateRoutinePair(lara_as_splat, lara_col_splat) },
		{ LS_UNDERWATER_IDLE, PlayerStateRoutinePair(lara_as_underwater_idle, lara_col_underwater_idle) },
		{ LS_GRAB_TO_FALL, PlayerStateRoutinePair(lara_void_func, lara_col_land) },
		{ LS_JUMP_PREPARE, PlayerStateRoutinePair(lara_as_jump_prepare, lara_col_jump_prepare) },
		{ LS_WALK_BACK, PlayerStateRoutinePair(lara_as_walk_back, lara_col_walk_back) },
		{ LS_UNDERWATER_SWIM_FORWARD, PlayerStateRoutinePair(lara_as_underwater_swim_forward, lara_col_underwater_swim_forward) },
		{ LS_UNDERWATER_INERTIA, PlayerStateRoutinePair(lara_as_underwater_inertia, lara_col_underwater_inertia) },
		{ LS_GRABBING, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_default_col) },
		{ LS_TURN_RIGHT_FAST, PlayerStateRoutinePair(lara_as_turn_right_fast, lara_col_turn_right_fast) },
		{ LS_STEP_RIGHT, PlayerStateRoutinePair(lara_as_step_right, lara_col_step_right) },
		{ LS_STEP_LEFT, PlayerStateRoutinePair(lara_as_step_left, lara_col_step_left) },
		{ LS_ROLL_180_BACKWARD, PlayerStateRoutinePair(lara_as_roll_180_back, lara_col_roll_180_back) },
		{ LS_SLIDE_FORWARD, PlayerStateRoutinePair(lara_as_slide_forward, lara_col_slide_forward) },
		{ LS_JUMP_BACK, PlayerStateRoutinePair(lara_as_jump_back, lara_col_jump_back) },
		{ LS_JUMP_RIGHT, PlayerStateRoutinePair(lara_as_jump_right, lara_col_jump_right) },
		{ LS_JUMP_LEFT, PlayerStateRoutinePair(lara_as_jump_left, lara_col_jump_left) },
		{ LS_JUMP_UP, PlayerStateRoutinePair(lara_as_jump_up, lara_col_jump_up) },
		{ LS_FALL_BACK, PlayerStateRoutinePair(lara_as_fall_back, lara_col_fall_back) },
		{ LS_SHIMMY_LEFT, PlayerStateRoutinePair(lara_as_shimmy_left, lara_col_shimmy_left) },
		{ LS_SHIMMY_RIGHT, PlayerStateRoutinePair(lara_as_shimmy_right, lara_col_shimmy_right) },
		{ LS_SLIDE_BACK, PlayerStateRoutinePair(lara_as_slide_back, lara_col_slide_back) },
		{ LS_ONWATER_IDLE, PlayerStateRoutinePair(lara_as_surface_idle, lara_col_surface_idle) },
		{ LS_ONWATER_FORWARD, PlayerStateRoutinePair(lara_as_surface_swim_forward, lara_col_surface_swim_forward) },
		{ LS_ONWATER_DIVE, PlayerStateRoutinePair(lara_as_surface_dive, lara_col_surface_dive) },
		{ LS_PUSHABLE_PUSH, PlayerStateRoutinePair(lara_as_pushable_push, lara_default_col) },
		{ LS_PUSHABLE_PULL, PlayerStateRoutinePair(lara_as_pushable_pull, lara_default_col) },
		{ LS_PUSHABLE_GRAB, PlayerStateRoutinePair(lara_as_pushable_grab, lara_default_col) },
		{ LS_PICKUP, PlayerStateRoutinePair(lara_as_pickup, lara_default_col) },
		{ LS_SWITCH_DOWN, PlayerStateRoutinePair(lara_as_switch_on, lara_default_col) },
		{ LS_SWITCH_UP, PlayerStateRoutinePair(lara_as_switch_off, lara_default_col) },
		{ LS_INSERT_KEY, PlayerStateRoutinePair(lara_as_use_key, lara_default_col) },
		{ LS_INSERT_PUZZLE, PlayerStateRoutinePair(lara_as_use_puzzle, lara_default_col) },
		{ LS_WATER_DEATH, PlayerStateRoutinePair(lara_as_underwater_death, lara_col_underwater_death) },
		{ LS_ROLL_180_FORWARD, PlayerStateRoutinePair(lara_as_roll_180_forward, lara_col_roll_180_forward) },
		{ LS_BOULDER_DEATH, PlayerStateRoutinePair(lara_as_special, lara_void_func) },
		{ LS_ONWATER_BACK, PlayerStateRoutinePair(lara_as_surface_swim_back, lara_col_surface_swim_back) },
		{ LS_ONWATER_LEFT, PlayerStateRoutinePair(lara_as_surface_swim_left, lara_col_surface_swim_left) },
		{ LS_ONWATER_RIGHT, PlayerStateRoutinePair(lara_as_surface_swim_right, lara_col_surface_swim_right) },
		{ LS_USE_MIDAS, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_MIDAS_DEATH, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_SWAN_DIVE, PlayerStateRoutinePair(lara_as_swan_dive, lara_col_swan_dive) },
		{ LS_FREEFALL_DIVE, PlayerStateRoutinePair(lara_as_freefall_dive, lara_col_freefall_dive) },
		{ LS_HANDSTAND, PlayerStateRoutinePair(lara_as_handstand, lara_default_col) },
		{ LS_ONWATER_EXIT, PlayerStateRoutinePair(lara_as_surface_climb_out, lara_default_col) },
		{ LS_LADDER_IDLE, PlayerStateRoutinePair(lara_as_climb_idle, lara_col_climb_idle) },
		{ LS_LADDER_UP, PlayerStateRoutinePair(lara_as_climb_up, lara_col_climb_up) },
		{ LS_LADDER_LEFT, PlayerStateRoutinePair(lara_as_climb_left, lara_col_climb_left) },
		{ LS_LADDER_STOP, PlayerStateRoutinePair(lara_as_climb_end, lara_col_climb_end) },
		{ LS_LADDER_RIGHT, PlayerStateRoutinePair(lara_as_climb_right, lara_col_climb_right) },
		{ LS_LADDER_DOWN, PlayerStateRoutinePair(lara_as_climb_down, lara_col_climb_down) },
		{ LS_AUTO_JUMP, PlayerStateRoutinePair(lara_as_auto_jump, lara_col_jump_prepare) },
		{ LS_TEST_2, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_TEST_3, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_WADE_FORWARD, PlayerStateRoutinePair(lara_as_wade_forward, lara_col_wade_forward) },
		{ LS_UNDERWATER_ROLL, PlayerStateRoutinePair(lara_as_underwater_roll_180, lara_col_underwater_roll_180) },
		{ LS_PICKUP_FLARE, PlayerStateRoutinePair(lara_as_pickup_flare, lara_default_col) },
		{ LS_JUMP_ROLL_180, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_KICK, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_ZIP_LINE, PlayerStateRoutinePair(lara_as_zip_line, lara_void_func) },
		{ LS_CROUCH_IDLE, PlayerStateRoutinePair(lara_as_crouch_idle, lara_col_crouch_idle) },
		{ LS_CROUCH_ROLL, PlayerStateRoutinePair(lara_as_crouch_roll, lara_col_crouch_roll) },
		{ LS_SPRINT, PlayerStateRoutinePair(lara_as_sprint, lara_col_sprint) },
		{ LS_SPRINT_DIVE, PlayerStateRoutinePair(lara_as_sprint_dive, lara_col_sprint_dive) },
		{ LS_MONKEY_IDLE, PlayerStateRoutinePair(lara_as_monkey_idle, lara_col_monkey_idle) },
		{ LS_MONKEY_FORWARD, PlayerStateRoutinePair(lara_as_monkey_forward, lara_col_monkey_forward) },
		{ LS_MONKEY_SHIMMY_LEFT, PlayerStateRoutinePair(lara_as_monkey_shimmy_left, lara_col_monkey_shimmy_left) },
		{ LS_MONKEY_SHIMMY_RIGHT, PlayerStateRoutinePair(lara_as_monkey_shimmy_right, lara_col_monkey_shimmy_right) },
		{ LS_MONKEY_TURN_180, PlayerStateRoutinePair(lara_as_monkey_turn_180, lara_col_monkey_turn_180) },
		{ LS_CRAWL_IDLE, PlayerStateRoutinePair(lara_as_crawl_idle, lara_col_crawl_idle) },
		{ LS_CRAWL_FORWARD, PlayerStateRoutinePair(lara_as_crawl_forward, lara_col_crawl_forward) },
		{ LS_MONKEY_TURN_LEFT, PlayerStateRoutinePair(lara_as_monkey_turn_left, lara_col_monkey_turn_left) },
		{ LS_MONKEY_TURN_RIGHT, PlayerStateRoutinePair(lara_as_monkey_turn_right, lara_col_monkey_turn_right) },
		{ LS_CRAWL_TURN_LEFT, PlayerStateRoutinePair(lara_as_crawl_turn_left, lara_col_crawl_turn_left) },
		{ LS_CRAWL_TURN_RIGHT, PlayerStateRoutinePair(lara_as_crawl_turn_right, lara_col_crawl_turn_right) },
		{ LS_CRAWL_BACK, PlayerStateRoutinePair(lara_as_crawl_back, lara_col_crawl_back) },
		{ LS_HANG_TO_CRAWL, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_void_func) },
		{ LS_CRAWL_TO_HANG, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_col_crawl_to_hang) },
		{ LS_MISC_CONTROL, PlayerStateRoutinePair(lara_as_controlled, lara_default_col) },
		{ LS_ROPE_TURN_CLOCKWISE, PlayerStateRoutinePair(lara_as_rope_turn_clockwise, lara_void_func) },
		{ LS_ROPE_TURN_COUNTER_CLOCKWISE, PlayerStateRoutinePair(lara_as_rope_turn_counter_clockwise, lara_void_func) },
		{ LS_GIANT_BUTTON_PUSH, PlayerStateRoutinePair(lara_as_controlled, lara_default_col) },
		{ LS_TRAPDOOR_FLOOR_OPEN, PlayerStateRoutinePair(lara_as_controlled, lara_void_func) },
		{ LS_FREEFALL_BIS, PlayerStateRoutinePair(lara_as_controlled, lara_void_func) },
		{ LS_ROUND_HANDLE, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_col_turn_switch) },
		{ LS_COGWHEEL, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_void_func) },
		{ LS_LEVERSWITCH_PUSH, PlayerStateRoutinePair(lara_as_controlled, lara_void_func) },
		{ LS_HOLE, PlayerStateRoutinePair(lara_as_pickup, lara_default_col) },
		{ LS_POLE_IDLE, PlayerStateRoutinePair(lara_as_pole_idle, lara_col_pole_idle) },
		{ LS_POLE_UP, PlayerStateRoutinePair(lara_as_pole_up, lara_col_pole_up) },
		{ LS_POLE_DOWN, PlayerStateRoutinePair(lara_as_pole_down, lara_col_pole_down) },
		{ LS_POLE_TURN_CLOCKWISE, PlayerStateRoutinePair(lara_as_pole_turn_clockwise, lara_col_pole_turn_clockwise) },
		{ LS_POLE_TURN_COUNTER_CLOCKWISE, PlayerStateRoutinePair(lara_as_pole_turn_counter_clockwise, lara_col_pole_turn_counter_clockwise) },
		{ LS_PULLEY, PlayerStateRoutinePair(lara_as_pulley, lara_default_col) },
		{ LS_CROUCH_TURN_LEFT, PlayerStateRoutinePair(lara_as_crouch_turn_left, lara_col_crouch_turn_left) },
		{ LS_CROUCH_TURN_RIGHT, PlayerStateRoutinePair(lara_as_crouch_turn_right, lara_col_crouch_turn_right) },
		{ LS_SHIMMY_OUTER_LEFT, PlayerStateRoutinePair(lara_as_shimmy_corner, lara_as_null) },
		{ LS_SHIMMY_OUTER_RIGHT, PlayerStateRoutinePair(lara_as_shimmy_corner, lara_as_null) },
		{ LS_SHIMMY_INNER_LEFT, PlayerStateRoutinePair(lara_as_shimmy_corner, lara_as_null) },
		{ LS_SHIMMY_INNER_RIGHT, PlayerStateRoutinePair(lara_as_shimmy_corner, lara_as_null) },
		{ LS_ROPE_IDLE, PlayerStateRoutinePair(lara_as_rope_idle, lara_col_rope_idle) },
		{ LS_ROPE_UP, PlayerStateRoutinePair(lara_as_rope_up, lara_void_func) },
		{ LS_ROPE_DOWN, PlayerStateRoutinePair(lara_as_rope_down, lara_void_func) },
		{ LS_ROPE_SWING, PlayerStateRoutinePair(lara_as_rope_idle, lara_col_rope_swing) },
		{ LS_ROPE_UNKNOWN, PlayerStateRoutinePair(lara_as_rope_idle, lara_col_rope_swing) },
		{ LS_CORRECT_POSITION, PlayerStateRoutinePair(lara_void_func, lara_void_func) },
		{ LS_DOUBLEDOOR_PUSH, PlayerStateRoutinePair(lara_as_controlled, lara_void_func) },
		{ LS_DOZY, PlayerStateRoutinePair(lara_as_swimcheat, lara_col_underwater_swim_forward) },
		{ LS_TIGHTROPE_IDLE, PlayerStateRoutinePair(lara_as_tightrope_idle, lara_default_col) },
		{ LS_TIGHTROPE_TURN_180, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_default_col) },
		{ LS_TIGHTROPE_WALK, PlayerStateRoutinePair(lara_as_tightrope_walk, lara_default_col) },
		{ LS_TIGHTROPE_UNBALANCE_LEFT, PlayerStateRoutinePair(lara_as_tightrope_fall, lara_default_col) },
		{ LS_TIGHTROPE_UNBALANCE_RIGHT, PlayerStateRoutinePair(lara_as_tightrope_fall, lara_default_col) },
		{ LS_TIGHTROPE_ENTER, PlayerStateRoutinePair(lara_as_null, lara_default_col) },
		{ LS_TIGHTROPE_DISMOUNT, PlayerStateRoutinePair(lara_as_tightrope_dismount, lara_default_col) },
		{ LS_DOVE_SWITCH, PlayerStateRoutinePair(lara_as_switch_on, lara_default_col) },
		{ LS_TIGHTROPE_RECOVER_BALANCE, PlayerStateRoutinePair(lara_as_null, lara_default_col) },
		{ LS_HORIZONTAL_BAR_SWING, PlayerStateRoutinePair(lara_as_horizontal_bar_swing, lara_default_col) },
		{ LS_HORIZONTAL_BAR_LEAP, PlayerStateRoutinePair(lara_as_horizontal_bar_leap, lara_default_col) },
		{ LS_UNKNOWN_1, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_RADIO_LISTENING, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_void_func) },
		{ LS_RADIO_OFF, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_void_func) },
		{ LS_UNKNOWN_2, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_UNKNOWN_3, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_UNKNOWN_4, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_UNKNOWN_5, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_PICKUP_FROM_CHEST, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_LADDER_TO_CROUCH, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SHIMMY_45_OUTER_LEFT, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SHIMMY_45_OUTER_RIGHT, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SHIMMY_45_INNER_LEFT, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SHIMMY_45_INNER_RIGHT, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SLOPE_CLIMB_IDLE, PlayerStateRoutinePair(lara_as_slopeclimb, lara_col_slopeclimb) },
		{ LS_SLOPE_CLIMB_UP, PlayerStateRoutinePair(lara_as_slopeclimbup, lara_default_col) },
		{ LS_SLOPE_CLIMB_DOWN, PlayerStateRoutinePair(lara_as_slopeclimbdown, lara_default_col) },
		{ LS_COGWHEEL_UNGRAB, PlayerStateRoutinePair(lara_as_controlled_no_look, lara_void_func) },
		{ LS_STEP_UP, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_STEP_DOWN, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SLOPE_CLIMB_FALL, PlayerStateRoutinePair(lara_as_slopefall, lara_default_col) },
		{ LS_LADDER_DISMOUNT_LEFT, PlayerStateRoutinePair(lara_as_climb_stepoff_left, lara_default_col) },
		{ LS_LADDER_DISMOUNT_RIGHT, PlayerStateRoutinePair(lara_as_climb_stepoff_right, lara_default_col) },
		{ LS_TURN_LEFT_FAST, PlayerStateRoutinePair(lara_as_turn_left_fast, lara_col_turn_left_fast) },
		{ LS_CRAWL_EXIT_STEP_DOWN, PlayerStateRoutinePair(lara_as_controlled, lara_default_col) },
		{ LS_CRAWL_EXIT_JUMP, PlayerStateRoutinePair(lara_as_controlled, lara_default_col) },
		{ LS_CRAWL_EXIT_FLIP, PlayerStateRoutinePair(lara_as_controlled, lara_default_col) },
		{ LS_SLOPE_CLIMB_HANG, PlayerStateRoutinePair(lara_as_slopehang, lara_col_slopehang) },
		{ LS_SLOPE_CLIMB_SHIMMY, PlayerStateRoutinePair(lara_as_slopeshimmy, lara_col_slopeshimmy) },
		{ LS_SLOPE_CLIMB_START, PlayerStateRoutinePair(lara_as_sclimbstart, lara_default_col) },
		{ LS_SLOPE_CLIMB_STOP, PlayerStateRoutinePair(lara_as_sclimbstop, lara_default_col) },
		{ LS_SLOPE_CLIMB_END, PlayerStateRoutinePair(lara_as_sclimbend, lara_default_col) },
		{ LS_CRAWL_STEP_UP, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_CRAWL_STEP_DOWN, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_MONKEY_BACK, PlayerStateRoutinePair(lara_as_monkey_back, lara_col_monkey_back) },
		{ LS_VAULT, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_VAULT_2_STEPS, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_VAULT_3_STEPS, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_VAULT_1_STEP_CROUCH, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_VAULT_2_STEPS_CROUCH, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_VAULT_3_STEPS_CROUCH, PlayerStateRoutinePair(lara_as_vault, lara_void_func) },
		{ LS_SOFT_SPLAT, PlayerStateRoutinePair(lara_as_idle, lara_col_idle) },
		{ LS_CROUCH_TURN_180, PlayerStateRoutinePair(lara_as_crouch_turn_180, lara_col_crouch_turn_180) },
		{ LS_CRAWL_TURN_180, PlayerStateRoutinePair(lara_as_crawl_turn_180, lara_col_crawl_turn_180) },
		{ LS_TURN_180, PlayerStateRoutinePair(lara_as_turn_180, lara_col_turn_180) },

		// Reserved for ladder object.
		{ 174, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 175, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 176, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 177, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 178, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 179, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 180, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 181, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 182, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 183, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 184, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 185, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 186, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 187, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ 188, PlayerStateRoutinePair(lara_as_null, lara_void_func) },

		{ LS_REMOVE_PUZZLE, PlayerStateRoutinePair(lara_as_use_puzzle, lara_default_col) },
		{ LS_RESERVED_PUSHABLE_STATE, PlayerStateRoutinePair(lara_as_null, lara_void_func) },
		{ LS_SPRINT_SLIDE, PlayerStateRoutinePair(lara_as_sprint_slide, lara_col_sprint_slide) }
	};

	void HandlePlayerState(ItemInfo& item, CollisionInfo& coll, PlayerStateRoutineType routineType)
	{
		// Find state routine pair.
		auto it = PLAYER_STATE_ROUTINE_MAP.find(item.Animation.ActiveState);
		if (it == PLAYER_STATE_ROUTINE_MAP.end())
		{
			TENLog("Error handling unregistered player state " + std::to_string(item.Animation.ActiveState), LogLevel::Warning);
			return;
		}

		// Execute state routine.
		const auto& stateRoutinePair = it->second;
		switch (routineType)
		{
		default:
		case PlayerStateRoutineType::Control:
		{
			const auto& stateRoutine = stateRoutinePair.first;
			stateRoutine(&item, &coll);
		}
			break;

		case PlayerStateRoutineType::Collision:
		{
			const auto& stateRoutine = stateRoutinePair.second;
			stateRoutine(&item, &coll);
		}
			break;
		}
	}
}
