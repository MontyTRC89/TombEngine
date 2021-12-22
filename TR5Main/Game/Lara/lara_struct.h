#pragma once
#include "objectslist.h"
#include "Specific\trmath.h"

#define NUM_PUZZLES	(ID_PUZZLE_ITEM16 - ID_PUZZLE_ITEM1 + 1)
#define NUM_PUZZLE_PIECES	(ID_PUZZLE_ITEM16_COMBO2 - ID_PUZZLE_ITEM1_COMBO1 + 1)
#define NUM_KEYS (ID_KEY_ITEM16 - ID_KEY_ITEM1 + 1)
#define NUM_KEY_PIECES	(ID_KEY_ITEM16_COMBO2 - ID_KEY_ITEM1_COMBO1 + 1)
#define NUM_PICKUPS (ID_PICKUP_ITEM16 - ID_PICKUP_ITEM1 + 1)
#define NUM_PICKUPS_PIECES	(ID_PICKUP_ITEM16_COMBO2 - ID_PICKUP_ITEM1_COMBO1 + 1)
#define NUM_EXAMINES (ID_EXAMINE8 - ID_EXAMINE1 + 1)
#define NUM_EXAMINES_PIECES	(ID_EXAMINE8_COMBO2 - ID_EXAMINE1_COMBO1 + 1)

struct CREATURE_INFO;
struct ITEM_INFO;
struct FX_INFO;

namespace TEN::Renderer {
	struct RendererMesh;
}


#pragma region state_and_animation
enum LARA_STATE
{
	// TR1
	LS_WALK_FORWARD = 0,
	LS_RUN_FORWARD = 1,
	LS_STOP = 2,
	LS_JUMP_FORWARD = 3,
	LS_POSE = 4,
	LS_HOP_BACK = 5,
	LS_TURN_RIGHT_SLOW = 6,
	LS_TURN_LEFT_SLOW = 7,
	LS_DEATH = 8,
	LS_FREEFALL = 9,
	LS_HANG = 10,
	LS_REACH = 11,
	LS_SPLAT = 12,
	LS_UNDERWATER_STOP = 13,
	LS_GRAB_TO_FALL = 14,
	LS_JUMP_PREPARE = 15,
	LS_WALK_BACK = 16,
	LS_UNDERWATER_FORWARD = 17,
	LS_UNDERWATER_INERTIA = 18,
	LS_GRABBING = 19,
	LS_TURN_FAST = 20,
	LS_STEP_RIGHT = 21,
	LS_STEP_LEFT = 22,
	LS_ROLL_BACK = 23,
	LS_SLIDE_FORWARD = 24,
	LS_JUMP_BACK = 25,
	LS_JUMP_RIGHT = 26,
	LS_JUMP_LEFT = 27,
	LS_JUMP_UP = 28,
	LS_FALL_BACK = 29,
	LS_SHIMMY_LEFT = 30,
	LS_SHIMMY_RIGHT = 31,
	LS_SLIDE_BACK = 32,
	LS_ONWATER_STOP = 33,
	LS_ONWATER_FORWARD = 34,
	LS_DIVE = 35,
	LS_PUSHABLE_PUSH = 36,
	LS_PUSHABLE_PULL = 37,
	LS_PUSHABLE_GRAB = 38,
	LS_PICKUP = 39,
	LS_SWITCH_DOWN = 40,
	LS_SWITCH_UP = 41,
	LS_INSERT_KEY = 42,
	LS_INSERT_PUZZLE = 43,
	LS_WATER_DEATH = 44,
	LS_ROLL_FORWARD = 45,
	LS_BOULDER_DEATH = 46,
	LS_ONWATER_BACK = 47,
	LS_ONWATER_LEFT = 48,
	LS_ONWATER_RIGHT = 49,
	LS_USE_MIDAS = 50,
	LS_MIDAS_DEATH = 51,
	LS_SWANDIVE_START = 52,
	LS_SWANDIVE_END = 53,
	LS_HANDSTAND = 54,
	LS_ONWATER_EXIT = 55,

	// TR2
	LS_LADDER_IDLE = 56,
	LS_LADDER_UP = 57,
	LS_LADDER_LEFT = 58,
	LS_LADDER_STOP = 59,
	LS_LADDER_RIGHT = 60,
	LS_LADDER_DOWN = 61,
	LS_MONKEY_VAULT = 62,	// Used for auto monkey up jump.
	LS_TEST_2 = 63,
	LS_TEST_3 = 64,
	LS_WADE_FORWARD = 65,
	LS_UNDERWATER_ROLL = 66,
	LS_PICKUP_FLARE = 67,
	LS_JUMP_ROLL_180 = 68,
	LS_KICK = 69,
	LS_ZIPLINE_RIDE = 70,

	// TR3
	LS_CROUCH_IDLE = 71,
	LS_CROUCH_ROLL = 72,
	LS_SPRINT = 73,
	LS_SPRINT_ROLL = 74,
	LS_MONKEYSWING_IDLE = 75,
	LS_MONKEYSWING_FORWARD = 76,
	LS_MONKEYSWING_LEFT = 77,
	LS_MONKEYSWING_RIGHT = 78,
	LS_MONKEYSWING_TURN_180 = 79,
	LS_CRAWL_IDLE = 80,
	LS_CRAWL_FORWARD = 81,
	LS_MONKEYSWING_TURN_LEFT = 82,
	LS_MONKEYSWING_TURN_RIGHT = 83,
	LS_CRAWL_TURN_LEFT = 84,
	LS_CRAWL_TURN_RIGHT = 85,
	LS_CRAWL_BACK = 86,
	LS_HANG_TO_CRAWL = 87,
	LS_CRAWL_TO_HANG = 88,
	LS_MISC_CONTROL = 89,

	// TR4
	LS_ROPE_TURN_CLOCKWISE = 90,
	LS_ROPE_TURN_COUNTER_CLOCKWISE = 91,
	LS_GIANT_BUTTON_PUSH = 92,
	LS_TRAPDOOR_FLOOR_OPEN = 93,
	LS_FREEFALL_BIS = 94,
	LS_ROUND_HANDLE = 95,
	LS_COGWHEEL = 96,
	LS_LEVERSWITCH_PUSH = 97,
	LS_HOLE = 98,
	LS_POLE_IDLE = 99,
	LS_POLE_UP = 100,
	LS_POLE_DOWN = 101,
	LS_POLE_TURN_CLOCKWISE = 102,
	LS_POLE_TURN_COUNTER_CLOCKWISE = 103,
	LS_PULLEY = 104,
	LS_CROUCH_TURN_LEFT = 105,
	LS_CROUCH_TURN_RIGHT = 106,
	LS_SHIMMY_OUTER_LEFT = 107,
	LS_SHIMMY_OUTER_RIGHT = 108,
	LS_SHIMMY_INNER_LEFT = 109,
	LS_SHIMMY_INNER_RIGHT = 110,
	LS_ROPE_IDLE = 111,
	LS_ROPE_UP = 112,
	LS_ROPE_DOWN = 113,
	LS_ROPE_SWING = 114,
	LS_LADDER_TO_SHIMMY = 115,
	LS_CORRECT_POSITION = 116,
	LS_DOUBLEDOOR_PUSH = 117,
	LS_DOZY = 118,

	// TR5
	LS_TIGHTROPE_IDLE = 119,
	LS_TIGHTROPE_TURN_180 = 120,
	LS_TIGHTROPE_FORWARD = 121,
	LS_TIGHTROPE_UNBALANCE_LEFT = 122,
	LS_TIGHTROPE_UNBALANCE_RIGHT = 123,
	LS_TIGHTROPE_ENTER = 124,
	LS_TIGHTROPE_EXIT = 125,
	LS_DOVESWITCH = 126,
	LS_TIGHTROPE_RECOVER_BALANCE = 127,
	LS_BARS_SWING = 128,
	LS_BARS_JUMP = 129,
	LS_UNKNOWN_1 = 130,
	LS_RADIO_LISTENING = 131,
	LS_RADIO_OFF = 132,
	LS_UNKNOWN_2 = 133,
	LS_UNKNOWN_3 = 134,
	LS_UNKNOWN_4 = 135,
	LS_UNKNOWN_5 = 136,
	LS_PICKUP_FROM_CHEST = 137,
	LS_LADDER_TO_CROUCH = 138,

	// TombEngine
	LS_SHIMMY_45_OUTER_LEFT  = 139,
	LS_SHIMMY_45_OUTER_RIGHT = 140,
	LS_SHIMMY_45_INNER_LEFT  = 141,
	LS_SHIMMY_45_INNER_RIGHT = 142,
	LS_UNUSED1 = 143, // Foot hang leftovers - may be safely reused
	LS_UNUSED2 = 144, // Foot hang leftovers - may be safely reused
	LS_UNUSED3 = 145, // Foot hang leftovers - may be safely reused
	LS_COGWHEEL_UNGRAB = 146,
	LS_STEP_UP = 147,
	LS_STEP_DOWN = 148,
	LS_STEP_BACK_DOWN = 149,
	LS_LADDER_DISMOUNT_LEFT = 150,
	LS_LADDER_DISMOUNT_RIGHT = 151,
	
	NUM_LARA_STATES
};

enum LARA_ANIM
{
	// TR1
	LA_RUN = 0,												// Run forward (looped)
	LA_WALK = 1,											// Walk forward (looped)
	LA_WALK_TO_STAND_RIGHT = 2,								// Walk > stand, right foot first
	LA_WALK_TO_STAND_LEFT = 3,								// Walk > stand, left foot first
	LA_WALK_TO_RUN_RIGHT = 4,								// Walk > run, right foot first
	LA_WALK_TO_RUN_LEFT = 5,								// Walk > run, left foot first
	LA_STAND_TO_RUN = 6,									// Stand > run
	LA_RUN_TO_WALK_RIGHT = 7,								// Run > walk, right foot first
	LA_RUN_TO_STAND_LEFT = 8,								// Run > stand, left foot first
	LA_RUN_TO_WALK_LEFT = 9,								// Run > walk, right foot first
	LA_RUN_TO_STAND_RIGHT = 10,								// Run > stand, right foot first
	LA_STAND_SOLID = 11,									// Stand solid
																// TODO: gradually remove reliance on this anim for erroneous collisions.
	LA_TURN_RIGHT_SLOW = 12,								// Rotate right slowly
	LA_TURN_LEFT_SLOW = 13,									// Rotate left slowly
	LA_JUMP_FORWARD_LAND_START_UNUSED = 14,					// Forward jump > land (1/2)
	LA_JUMP_FORWARD_LAND_END_UNUSED = 15,					// Forward jump > land (2/2)
	LA_RUN_TO_JUMP_FORWARD_RIGHT_START = 16,				// Run > take off for forward jump, right foot first (1/2)
	LA_RUN_TO_JUMP_FORWARD_RIGHT_END = 17,					// Run > take off for forward jump, right foot first (2/2)
	LA_RUN_TO_JUMP_FORWARD_LEFT_START = 18,					// Run > take off for forward jump, left foot first (1/2)
	LA_RUN_TO_JUMP_FORWARD_LEFT_END = 19,					// Run > take off for forward jump, left foot first (2/2)
	LA_STAND_TO_WALK_START = 20,							// Stand > walk forward (1/2)
	LA_STAND_TO_WALK_END = 21,								// Stand > walk forward (1/2)
	LA_JUMP_FORWARD_TO_FREEFALL_UNUSED = 22,				// Jump > fall (possibly unused?)
																// TODO: confirm lack of dispatch for this anim.
	LA_FREEFALL = 23,										// Freefall, after falling more than 7 clicks (looped)
	LA_FREEFALL_LAND = 24,									// Freefall > hard landing
	LA_FREEFALL_DEATH = 25,									// Freefall death
	LA_STAND_TO_JUMP_UP_START = 26,							// Stand > jump up (1/2)
	LA_STAND_TO_JUMP_UP_END = 27,							// Stand > jump up (2/2)
	LA_JUMP_UP = 28,										// Jump up, ready to grab ledge (looped)
	LA_JUMP_UP_TO_HANG = 29,								// Grab ledge > hang
	LA_JUMP_UP_TO_FREEFALL = 30,							// Jump up > fall
	LA_JUMP_UP_LAND = 31,									// Jump up > land
	LA_JUMP_WALL_SMASH_START = 32,							// Directional jump smash > fall (1/2)
	LA_JUMP_WALL_SMASH_END = 33,							// Directional jump smash > fall (2/2)
																// TODO: create matching anims for all directional wall smashes.
	LA_FALL_START = 34,										// Start falling
	LA_FALL = 35,											// Light fall (looped)
	LA_FALL_TO_FREEFALL = 36,								// Light fall > freefall
	LA_HANG_TO_FREEFALL_UNUSED = 37,						// Hang > freefall
	LA_WALK_BACK_TO_STAND_RIGHT = 38,						// Walk backwards > stand, right foot first
	LA_WALK_BACK_TO_STAND_LEFT = 39,						// Walk back > stand, left foot first
	LA_WALK_BACK = 40,										// Walk back (looped)
	LA_STAND_TO_WALK_BACK = 41,								// Stand > walk back
	LA_VAULT_TO_STAND_3CLICK = 42,							// Standing 3-click vault > stand
	LA_VAULT_TO_STAND_3CLICK_TO_RUN = 43,					// Standing 3-click vault > run
																// TODO: implement properly.
	LA_TURN_RIGHT_FAST = 44,								// Rotate right quickly
	LA_JUMP_FORWARD_TO_FREEFALL_OG = 45,					// Jump forward > fall
																// TODO: this is the original, but incorrect, linking animation; in WAD should be 49 instead.
	LA_REACH_TO_FREEFALL_ALTERNATE_UNUSED = 46,				// Reach > freefall
	LA_ROLL_180_START_ALTERNATE_UNUSED = 47,				// Standing roll 180 (1/2)
	LA_ROLL_180_END_ALTERNATE_UNUSED = 48,					// Standing roll 180 (2/2)
	LA_JUMP_FORWARD_TO_FREEFALL = 49,						// Jump forward > freefall
	LA_VAULT_TO_STAND_2CLICK_START = 50,					// Standing 2-click vault > stand (1/2) 
	LA_VAULT_TO_STAND_2CLICK_END = 51,						// Standing 2-click vault > stand (2/2) 
	LA_VAULT_TO_STAND_2CLICK_END_TO_RUN = 52,				// Standing 2-click vault > stand (2/2) > run
	LA_RUN_WALL_SMASH_LEFT = 53,							// Running smash > stand, left foot first
	LA_RUN_WALL_SMASH_RIGHT = 54,							// Running smash > stand, right foot first
	LA_RUN_UP_STEP_LEFT = 55,								// Run up 1-click, left foot first
	LA_RUN_UP_STEP_RIGHT = 56,								// Run up 1-click, right foot first
	LA_WALK_UP_STEP_LEFT = 57,								// Walk up 1-click, left foot first
	LA_WALK_UP_STEP_RIGHT = 58,								// Walk up 1-click, right foot first
	LA_WALK_DOWN_STEP_RIGHT = 59,							// Walk down 1-click, right foot first
	LA_WALK_DOWN_STEP_LEFT = 60,							// Walk down 1-click, left foot first
	LA_WALK_BACK_DOWN_LEFT = 61,							// Walk back down 1-click, left foot first
	LA_WALK_BACK_DOWN_RIGHT = 62,							// Walk back down 1-click, right foot first
	LA_WALLSWITCH_DOWN = 63,								// Activate horizontal wall switch
	LA_WALLSWITCH_UP = 64,									// Deactivate horizontal wall switch
	LA_SIDESTEP_LEFT = 65,									// Sidestep left (looped)
	LA_SIDESTEP_LEFT_END = 66,								// Sidestep left > stand
	LA_SIDESTEP_RIGHT = 67,									// Sidestep right (looped)
	LA_SIDESTEP_RIGHT_END = 68,								// Sidestep right > stand
	LA_TURN_LEFT_FAST = 69,									// Rotate left quickly
	LA_SLIDE_FORWARD = 70,									// Slide forward (looped)
	LA_SLIDE_FORWARD_TO_STAND_START = 71,					// Slide forward > stand (1/2)
	LA_SLIDE_FORWARD_TO_STAND_END = 72,						// Slide forward > stand (2/2)
	LA_STAND_TO_JUMP_PREPARE = 73,							// Prepare standing jump
	LA_JUMP_BACK_START = 74,								// Prepare standing jump > jump back
	LA_JUMP_BACK = 75,										// Jump back (looped)
	LA_JUMP_FORWARD_START = 76,								// Prepare standing jump > jump forward
	LA_JUMP_FORWARD = 77,									// Jump forward (looped)
	LA_JUMP_LEFT_START = 78,								// Prepare standing jump > jump left
	LA_JUMP_LEFT = 79,										// Jump left (looped)
	LA_JUMP_RIGHT_START = 80,								// Prepare standing jump > jump right
	LA_JUMP_RIGHT = 81,										// Jump right (looped)
	LA_LAND = 82,											// Light fall > land
	LA_JUMP_BACK_TO_FREEFALL = 83,							// Jump back > freefall
	LA_JUMP_LEFT_TO_FREEFALL = 84,							// Jump left > freefall
	LA_JUMP_RIGHT_TO_FREEFALL = 85,							// Jump right > freefall
	LA_UNDERWATER_SWIM = 86,								// Swim forward (looped)
	LA_UNDERWATER_SWIM_DRIFT = 87,							// Swim forward drift
	LA_HOP_BACK_START = 88,									// Hop back (1/3)
	LA_HOP_BACK_CONTINUE = 89,								// Hop back (2/3)
	LA_HOP_BACK_END = 90,									// Hop back (3/3)
	LA_JUMP_UP_START = 91,									// Prepare standing jump > jump up
	LA_LAND_TO_RUN = 92,									// Jump forward > land running
	LA_FALL_BACK = 93,										// Light fall back
																// TODO: remove? This is a duplicate of 35, only without the RUN_FORWARD state change.
	LA_JUMP_FORWARD_TO_REACH_3 = 94,						// Jump forward > reach, 3rd opportunity
	LA_REACH = 95,											// Reach (looped)
	LA_REACH_TO_HANG = 96,									// Reach > hang
	LA_HANG_TO_STAND = 97,									// Pull up from hang > stand (1/2)
	LA_REACH_TO_FREEFALL = 98,								// Reach > freefall
	LA_LAND_TRANSITION_UNUSED = 99,							// Light fall > mid-land transition
																// TODO: remove. Possibly a leftover linking anim.
	LA_JUMP_FORWARD_TO_REACH_4 = 100,						// Jump forward > reach, 4th opportunity
	LA_JUMP_FORWARD_TO_REACH_ALTERNATE_UNUSED = 101,		// Jump forward (from jump forward anim) > reach
	LA_HANG_TO_STAND_END = 102,								// Pull up from hang > stand (2/2)
	LA_STAND_IDLE = 103,									// Stand idle (looped)
	LA_SLIDE_BACK_START = 104,								// Land on slope > slide back
	LA_SLIDE_BACK = 105,									// Slide back (looped) 
	LA_SLIDE_BACK_END = 106,								// Slide back > stand
	LA_UNDERWATER_SWIM_TO_IDLE = 107,						// Swim forward > underwater idle
	LA_UNDERWATER_IDLE = 108,								// Underwater idle (looped)
	LA_UNDERWARER_IDLE_TO_SWIM = 109,						// Underwater idle > swim forward underwater
	LA_ONWATER_IDLE = 110,									// Tread water idle (looped)
	LA_ONWATER_TO_STAND_1CLICK = 111,						// Pull up 1-click from tread > stand
	LA_FREEFALL_DIVE = 112,									// Freefall > underwater
	LA_ONWATER_DIVE_ALTERNATE_1_UNUSED = 113,				// Tread water > underwater
	LA_UNDERWATER_RESURFACE = 114,							// Underwater > tread water
	LA_ONWATER_DIVE_ALTERNATE_2_UNUSED = 115,				// Tread water > underwater
																// TODO: could reuse as link for an on-water "sprint".
	LA_ONWATER_SWIM = 116,									// Swim treading (looped)
	LA_ONWATER_SWIM_TO_IDLE = 117,							// Swim treading > tread water idle
	LA_ONWATER_IDLE_TO_SWIM = 118,							// Tread water > swim treading
	LA_ONWATER_DIVE = 119,									// Tread water > underwater
	LA_PUSHABLE_GRAB = 120,									// Grab pushable object (looped)
	LA_PUSHABLE_RELEASE = 121,								// Release pushable object
	LA_PUSHABLE_PULL = 122,									// Pull pushable object (looped)
	LA_PUSHABLE_PUSH = 123,									// Push pushable object (looped)
	LA_UNDERWATER_DEATH = 124,								// Drowning death
	LA_STAND_HIT_FRONT = 125,								// Jerk back standing from damage
	LA_STAND_HIT_BACK = 126,								// Jerk forward standing from damage
	LA_STAND_HIT_LEFT = 127,								// Jerk right standing from damage
	LA_STAND_HIT_RIGHT = 128,								// Jerk left standing from damage
	LA_WATERLEVER_PULL = 129,								// Pull underwater lever
	LA_UNDERWATER_PICKUP = 130,								// Pickup underwater
	LA_USE_KEY = 131,										// Use key
	LA_ONWATER_DEATH = 132,									// Treading death
	LA_RUN_DEATH = 133,										// Running death
	LA_USE_PUZZLE = 134,									// Insert receptacle item
	LA_PICKUP = 135,										// Standing pickup
	LA_SHIMMY_LEFT = 136,									// Shimmy left
	LA_SHIMMY_RIGHT = 137,									// Shimmy right
	LA_STAND_DEATH = 138,									// Standing death
	LA_BOULDER_DEATH = 139,									// Boulder death
	LA_ONWATER_IDLE_TO_SWIM_BACK = 140,						// Tread water > swim back treading
	LA_ONWATER_SWIM_BACK = 141,								// Swim back treading (looped)
	LA_ONWATER_SWIM_BACK_TO_IDLE = 142,						// Swim back treading > tread water idle
	LA_ONWATER_SWIM_LEFT = 143,								// Swim left treading (looped)
	LA_ONWATER_SWIM_RIGHT = 144,							// Swim right treading (looped)
	LA_LAND_DEATH = 145,									// Landing death
	LA_ROLL_180_START = 146,								// Standing roll 180 (1/3)
	LA_ROLL_180_CONTINUE = 147,								// Standing roll 180 (2/3)
	LA_ROLL_180_END = 148,									// Standing roll 180 (3/3)
	LA_SPIKE_DEATH = 149,									// Spike death
	LA_REACH_TO_MONKEYSWING = 150,							// Reach > monkey swing
	LA_SWANDIVE_ROLL = 151,									// Swan dive > roll landing
	LA_SWANDIVE_DIVE = 152,									// Swan dive > underwater
	LA_SWANDIVE_FREEFALL = 153,								// Swan dive freefall
	LA_SWANDIVE_FREEFALL_DIVE = 154,						// Swan dive freefall > underwater
	LA_SWANDIVE_FREEFALL_DEATH = 155,						// Swan dive freefall death
	LA_SWANDIVE_LEFT_START = 156,							// Run > swan dive, left foot first
	LA_SWANDIVE_RIGHT_START = 157,							// Run > swan dive, right foot first
	LA_SWANDIVE = 158,										// Swan dive
	LA_HANG_HANDSTAND = 159,								// Hang > stand via handstand

	// TR2
	LA_STAND_TO_LADDER = 160,								// Stand > ladder idle
	LA_LADDER_UP = 161,										// Ascend ladder (looped)
	LA_LADDER_UP_LEFT_END = 162,							// Ascend ladder > ladder idle, left foot first
	LA_LADDER_UP_RIGHT_END = 163,							// Ascend ladder > ladder idle, right foot first
	LA_LADDER_IDLE = 164,									// Ladder idle (looped)
	LA_LADDER_UP_START = 165,								// Ladder idle > ascend ladder
	LA_LADDER_DOWN_LEFT_END = 166,							// Descend ladder > ladder idle, left foot first
	LA_LADDER_DOWN_RIGHT_END = 167,							// Descend ladder > ladder idle, right foot first
	LA_LADDER_DOWN = 168,									// Descend ladder (looped)
	LA_LADDER_DOWN_START = 169,								// Ladder idle > descend ladder
	LA_LADDER_RIGHT = 170,									// Climb ladder right (looped)
	LA_LADDER_LEFT = 171,									// Climb ladder left (looped)
	LA_LADDER_HANG = 172,									// Ladder hang (looped)
																// TODO: possible to make this a generic hang animation for ledges and ladders, thereby splitting 96?
	LA_LADDER_HANG_TO_IDLE = 173,							// Ladder hang > ladder idle
	LA_LADDER_TO_STAND = 174,								// Ladder idle > stand, pulling up
	LA_UNKNOWN = 175,										// Pushed back?
	LA_ONWATER_TO_WADE_0CLICK = 176,						// Tread water up 0-click > wade
																// TODO: implement this properly?
	LA_WADE = 177,											// Wade (looped)
	LA_RUN_TO_WADE_RIGHT = 178,								// Run > wade, right foot first
	LA_RUN_TO_WADE_LEFT = 179,								// Run > wade, left foot first
	LA_WADE_TO_RUN_RIGHT = 180,								// Wade > run, right foot first
	LA_WADE_TO_RUN_LEFT = 181,								// Wade > run, left foot first
	LA_LADDER_TO_JUMP_BACK_START = 182,						// Ladder idle > jump back (1/2)
	LA_LADDER_TO_JUMP_BACK_END = 183,						// Ladder idle > jump back (2/2)
																// TODO: combine 182 and 183?
	LA_WADE_TO_STAND_RIGHT = 184,							// Wade > stand, right foot first
	LA_WADE_TO_STAND_LEFT = 185,							// Wade > stand, left foot first
	LA_STAND_TO_WADE = 186,									// Stand > wade
	LA_LADDER_SHIMMY_UP = 187,								// Ascend ladder hanging
	LA_LADDER_SHIMMY_DOWN = 188,							// Descend ladder hanging
	LA_DISCARD_FLARE = 189,									// Throw flare standing
	LA_ONWATER_TO_WADE_1CLICK = 190,						// Tread water up 1-click > wade
	LA_ONWATER_TO_STAND_0CLICK = 191,						// Pull up 0-click from thread > stand
	LA_UNDERWATER_TO_STAND = 192,							// Underwater > stand
	LA_ONWATER_TO_STAND_M1CLICK = 193,						// Pull up -1-click from tread > stand
	LA_LADDER_TO_HANG_DOWN = 194,							// Descend ladder > hang
																// TODO: this links to regular hang at 96. Address this?
	LA_SWITCH_SMALL_DOWN = 195,								// Activate small switch
	LA_SWITCH_SMALL_UP = 196,								// Deactivate small switch
	LA_BUTTON_SMALL_PUSH = 197,								// Push small button
	LA_UNDERWATER_SWIM_TO_IDLE_2 = 198,						// Underwater swim > underwater idle, 2nd opportunity
	LA_UNDERWATER_SWIM_TO_IDLE_3 = 199,						// Underwater swim > underwater idle, 3rd opportunity
	LA_UNDERWATER_SWIM_TO_IDLE_1 = 200,						// Underwater swim > underwater idle, 1st opportunity
	LA_LADDER_TO_HANG_RIGHT = 201,							// Climb ladder right > hang
	LA_LADDER_TO_HANG_LEFT = 202,							// Climb ladder left > hang
	LA_UNDERWATER_ROLL_180_START = 203,						// Underwater roll 180 (1/2)
	LA_PICKUP_FLARE = 204,									// Pickup flare standing
	LA_UNDERWATER_ROLL_180_END = 205,						// Underwater roll 180 (2/2)
	LA_UNDERWATER_PICKUP_FLARE = 206,						// Pickup flare underwater
	LA_RUN_JUMP_RIGHT_ROLL_180_START = 207,					// Run jump, right foot first, roll 180 (1/2)
	LA_SWANDIVE_SOMERSAULT = 208,							// Swandive somersault
	LA_RUN_JUMP_RIGHT_ROLL_180_END = 209,					// Run jump, right foot first, roll 180 (2/2)
	LA_JUMP_FORWARD_ROLL_180_START = 210,					// Jump forward roll 180 (1/2)
	LA_JUMP_FORWARD_ROLL_180_END = 211,						// Jump forward roll 180 (2/2)
	LA_JUMP_BACK_ROLL_180_START = 212,						// Jump back roll 180 (1/2)
	LA_JUMP_BACK_ROLL_180_END = 213,						// Jump back roll 180 (2/2)
	LA_ZIPLINE_MOUNT = 214,									// Stand > ride sipline
	LA_ZIPLINE_RIDE = 215,									// Ride zipline (looped)
	LA_ZIPLINE_DISMOUNT = 216,								// Ride zipline > jump forward

	// TR3
	LA_STAND_TO_CROUCH_START = 217,							// Stand > crouch (1/2)
	LA_CROUCH_ROLL_FORWARD_START_ALTERNATE = 218,			// Crouch roll forward (1/3)
																// TODO: this is the original, but incorrect, linking animation; in WAD it should be 47 instead.
	LA_CROUCH_ROLL_FORWARD_CONTINUE = 219,					// Crouch roll forward (2/3)
	LA_CROUCH_ROLL_FORWARD_END = 220,						// Crouch roll forward (3/3)
	LA_CROUCH_TO_STAND = 221,								// Crouch >  stand
	LA_CROUCH_IDLE = 222,									// Crouch (looped)
	LA_SPRINT = 223,										// Sprint (looped)
	LA_RUN_TO_SPRINT_LEFT = 224,							// Run > sprint, left foot first
	LA_RUN_TO_SPRINT_RIGHT = 225,							// Run > sprint, right foot first
	LA_SPRINT_TO_STAND_RIGHT = 226,							// Sprint > stand, right foot first
	LA_SPRINT_TO_STAND_RIGHT_END_ALTERNATE_UNUSED = 227,	// Sprint > stand, right foot first end
	LA_SPRINT_TO_STAND_LEFT = 228,							// Sprint > stand, left foot first
	LA_SPRINT_TO_STAND_LEFT_END_ALTERNATE_UNUSED = 229,		// Sprint > stand, left foot first end
	LA_SPRINT_ROLL_TO_RUN_LEFT_START = 230,					// Sprint roll, left foot first > run (1/2)
	LA_SPRINT_ROLL_TO_RUN_RIGHT_CONTINUE_ALTERNATE_UNUSED = 231,	// Sprint roll, left foot first > run (2/3)
	LA_SPRINT_ROLL_TO_RUN_LEFT_END = 232,					// Sprint roll, left foot first > run (2/2)
	LA_JUMP_UP_TO_MONKEYSWING = 233,						// Jump up > monkey swing
	LA_MONKEYSWING_IDLE = 234,								// Monkey swing idle (looped)
	LA_MONKEYSWING_TO_FREEFALL = 235,						// Monkey swing > freefall
	LA_MONKEYSWING_FORWARD = 236,							// Monkey swing forward (looped)
	LA_MONKEYSWING_FORWARD_TO_IDLE_RIGHT = 237,				// Monkey-swing forward > monkey swing idle, right hand first
	LA_MONKEYSWING_FORWARD_TO_IDLE_LEFT = 238,				// Monkey-swing forward > monkey swing idle, left hand first
	LA_MONKEYSWING_IDLE_TO_FORWARD_LEFT = 239,				// Monkey idle > monkey forward, left hand first
	LA_SPRINT_ROLL_TO_RUN_LEFT_START_ALTERNATE_UNUSED = 240,	// Sprint roll, left foot first > run (1/3)
	LA_SPRINT_ROLL_TO_RUN_LEFT_CONTINUE_ALTERNATE_UNUSED = 241,	// Sprint roll, left foot first > run (2/3)
	LA_SPRINT_ROLL_TO_RUN_LEFT_END_ALTERNATE_UNUSED = 242,		// Sprint roll, left foot first > run (3/3)
	LA_SPRINT_TO_RUN_LEFT = 243,							// Sprint > run, left foot first
	LA_SPRINT_TO_RUN_RIGHT = 244,							// Sprint > run, right foot first
	LA_STAND_TO_CROUCH_END = 245,							// Stand > crouch (2/2)
	LA_SLIDE_TO_RUN = 246,									// Slide forward > run
	LA_CROUCH_ROLL_FORWARD_START = 247,						// Crouch roll forward (1/3)
	LA_JUMP_FORWARD_TO_REACH_1 = 248,						// Jump forward > reach, 1st opportunity
	LA_JUMP_FORWARD_TO_REACH_2 = 249,						// Jump forward > reach, 2nd opportunity
	LA_RUN_JUMP_LEFT_TO_REACH = 251,						// Run jump, left foot first > reach
	LA_MONKEYSWING_IDLE_TO_FORWARD_RIGHT = 252,				// Monkey swing idle > monkey swing forward, right hand first
	LA_MONKEYSWING_SHIMMY_LEFT = 253,						// Monkey swing shimmy left (looped)
	LA_MONKEYSWING_SHIMMY_LEFT_END = 254,					// Monkey swing shimmy left > monkey swing idle
	LA_MONKEYSWING_SHIMMY_RIGHT = 255,						// Monkey swing shimmy right (looped)
	LA_MONKEYSWING_SHIMMY_RIGHT_END = 256,					// Monkey swing shimmy right > monkey swing idle
																// TODO: generic shimmy anims between ledges and ladders?
	LA_MONKEYSWING_TURN_180 = 257,							// Monkey swing turn 180
	LA_CROUCH_TO_CRAWL_START = 258,							// Crouch > crawl (1/3)
	LA_CRAWL_TO_CROUCH_START = 259,							// Crawl > crouch (1/3)
	LA_CRAWL = 260,											// Crawl forward (looped)
	LA_CRAWL_IDLE_TO_FORWARD = 261,							// Crawl idle > crawl forward
	LA_CRAWL_TO_IDLE_LEFT = 262,							// Crawl forward > crawl idle, left leg first
																// TODO: in WAD, link next to 263
	LA_CRAWL_IDLE = 263,									// Crwal idle
	LA_CROUCH_TO_CRAWL_END = 264,							// Crawl > crouch (2/2)
	LA_CRAWL_TO_CROUCH_END_UNUSED = 265,					// Crouch > crawl (3/3)
	LA_CRAWL_TO_IDLE_END_RIGHT_POINTLESS = 266,					// TODO: remove.//no dont remove thanks
	LA_CRAWL_TO_IDLE_RIGHT = 267,							// Crawl forward > crawl idle, right leg first
																// TODO: in WAD, link next to 263
	LA_CRAWL_TO_IDLE_END_LEFT_POINTLESS = 268,					// TODO: remove. //no dont remove thanks
	LA_CRAWL_TURN_LEFT = 269,								// Crawl rotate left (looped)
	LA_CRAWL_TURN_RIGHT = 270,								// Crawl rotate right (looped)
	LA_MONKEYSWING_TURN_LEFT = 271,							// Monkey swing rotate left
	LA_MONKEYSWING_TURN_RIGHT = 272,						// Monkey swing rotate right
	LA_CROUCH_TO_CRAWL_CONTINUE = 273,						// Crouch > crawl (2/3)
	LA_CRAWL_TO_CROUCH_CONTINUE = 274,						// Crouch > crawl (2/3)
																// TODO: properly link to 265 (adjustments to next anim necessary?)
	LA_CRAWL_IDLE_TO_CRAWL_BACK = 275,						// Crawl > crawl back
	LA_CRAWL_BACK = 276,									// Crawl back (looped)
	LA_CRAWL_BACK_TO_IDLE_RIGHT_START = 277,				// Crawl back > crawl idle, right foot first (1/2)
	LA_CRAWL_BACK_TO_IDLE_RIGHT_END = 278,					// Crawl back > crawl idle, right foot first (2/2)
	LA_CRAWL_BACK_TO_IDLE_LEFT_START = 279,					// Crawl back > crawl idle, left foot first (1/2)
	LA_CRAWL_BACK_TO_IDLE_LEFT_END = 280,					// Crawl back > crawl idle, left foot first (2/2)
	LA_CRAWL_TURN_LEFT_TO_IDLE_EARLY = 281,					// Crawl rotate left > crawl idle, early opportunity
	LA_CRAWL_TURN_RIGHT_TO_IDLE_EARLY = 282,				// Crawl rotate right > crawl idle, early opportunity
	LA_MONKEYSWING_TURN_LEFT_TO_IDLE_EARLY = 283,			// Turn left on monkey swing > monkey swing idle, 1st opportunity
	LA_MONKEYSWING_TURN_LEFT_TO_IDLE_LATE = 284,			// Turn left on monkey swing > monkey swing idle, 2nd opportunity
	LA_MONKEYSWING_TURN_RIGHT_TO_IDLE_EARLY = 285,			// Turn right on monkey swing > monkey swing idle, 1st opportunity
	LA_MONKEYSWING_TURN_RIGHT_TO_IDLE_LATE = 286,			// Turn right on monkey swing > monkey swing idle, 2nd opportunity
	LA_HANG_TO_CROUCH_START = 287,							// Pull up from hang > crouch (1/2)
	LA_HANG_TO_CROUCH_END = 288,							// Pull up from hang > crouch (2/2)
	LA_CRAWL_TO_HANG_START = 289,							// Crawl > hang (1/3)
	LA_CRAWL_TO_HANG_CONTINUE = 290,						// Crawl > hang (2/3)
																// TODO: position commands in 302 may be stacked in 290, so can remove 302?
	LA_CROUCH_PICKUP = 291,									// Crouching pickup
	LA_CRAWL_PICKUP = 292,									// Crawling pickup
	LA_CROUCH_HIT_BACK = 293,								// Jerk back crouching from damage
	LA_CROUCH_HIT_FRONT = 294,								// Jerk forward crouching from damage
	LA_CROUCH_HIT_LEFT = 295,								// Jerk right crouching from damage
	LA_CROUCH_HIT_RIGHT = 296,								// Jerk left crouching from damage
	LA_CRAWL_HIT_BACK = 297,								// Jerk forward crawling from damage
	LA_CRAWL_HIT_FRONT = 298,								// Jerk back crawling from damage
	LA_CRAWL_HIT_LEFT = 299,								// Jerk left crawling from damage
	LA_CRAWL_HIT_RIGHT = 300,								// Jerk right crawling from damage
	LA_CRAWL_DEATH = 301,									// Crawl death
	LA_CRAWL_TO_HANG_END = 302,								// Crawl > hang (3/3)
	LA_STAND_TO_CROUCH_ABORT = 303,							// Stand > crouch abort
																// TODO: implement this, OR the ability for any animation to be wound back like pistol aiming.
	LA_RUN_TO_CROUCH_LEFT_START = 304,						// Run > crouch, left foot first (1/2)
	LA_RUN_TO_CROUCH_RIGHT_START = 305,						// Run > crouch, right foot first (1/2)
	LA_RUN_TO_CROUCH_LEFT_END = 306,						// Run > crouch, left foot first (2/2)
	LA_RUN_TO_CROUCH_RIGHT_END = 307,						// Run > crouch, right foot first (2/2)
	LA_SPRINT_ROLL_TO_RUN_RIGHT_START = 308,				// Sprint roll, right foot first > run (2/2)
	LA_SPRINT_ROLL_TO_RUN_RIGHT_END = 309,					// Sprint roll, right foot first > run (2/2)
	LA_SPRINT_TO_CROUCH_LEFT = 310,							// Sprint roll, left foot first > crouch
	LA_SPRINT_TO_CROUCH_RIGHT = 311,						// Sprint roll, right foot first > crouch
	LA_CROUCH_PICKUP_FLARE = 312,							// Pickup flare crouching

	// TR4
	LA_DOOR_OPEN_PUSH = 313,								// Push door open using doorknob
	LA_DOOR_OPEN_PULL = 314,								// Pull door open using doorknob
	LA_DOOR_OPEN_KICK = 315,								// Kick door open
	LA_BUTTON_GIANT_PUSH = 316,								// Push giant button 
	LA_TRAPDOOR_FLOOR_OPEN = 317,							// Open trapdoor below
	LA_TRAPDOOR_CEILING_OPEN = 318,							// Jump up to open trapdoor above
	LA_TURNSWITCH_GRAB_CLOCKWISE = 319,						// Grab turnswitch to push clockwise	
	LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE = 320,				// Grab turnswitch to push counter-clockwise
	LA_COGWHEEL_PULL = 321,									// Pull cog wheel
	LA_COGWHEEL_GRAB = 322,									// Stand > grab cog wheel
	LA_COGWHEEL_RELEASE = 323,								// Release cogwheel > stand
	LA_LEVER_PUSH = 324,									// Push floor lever
	LA_HOLESWITCH_ACTIVATE = 325,							// Reach inside hole to activate/pickup
	LA_STAND_TO_POLE = 326,									// Stand > pole idle
	LA_POLE_JUMP_BACK = 327,								// Pole idle > jump back
	LA_POLE_IDLE = 328,										// Pole idle (looped)
	LA_POLE_UP_START = 329,									// Ascend pole (1/2)
	LA_POLE_TO_FREEFALL = 330,								// Pole idle > freefall
	LA_REACH_TO_POLE = 331,									// Reach > pole idle
	LA_POLE_TURN_CLOCKWISE_START = 332,						// Rotate clockwise on pole (1/2)
	LA_POLE_TURN_COUNTER_CLOCKWISE_START = 333,				// Rotate counter-clockwise on pole (1/2)
	LA_POLE_DOWN_START = 334,								// Pole idle > descend pole
	LA_POLE_DOWN = 335,										// Descend pole (looped)
	LA_POLE_DOWN_END = 336,									// Descend pole > pole idle
	LA_JUMP_UP_TO_POLE = 337,								// Jump up > pole
	LA_POLE_UP_END = 338,									// Ascend pole (2/2)
	LA_PULLEY_GRAB = 339,									// Stand > pull pulley
	LA_PULLEY_PULL = 340,									// Pull pulley
	LA_PULLEY_RELEASE = 341,								// Pull pulley > stand
	LA_POLE_TO_STAND = 342,									// Pole > stand
	LA_POLE_TURN_CLOCKWISE_CONTINUE_UNUSED = 343,				// TODO: remove.
	LA_POLE_TURN_CLOCKWISE_END = 344,						// Rotate clickwise on pole (2/2)
	LA_POLE_TURN_COUNTER_CLOCKWISE_CONTINUE_UNUSED = 345,		// TODO: remove.
	LA_POLE_TURN_COUNTER_CLOCKWISE_END = 346,				// Rotate counter-clockwise on pole (2/2)
	LA_TURNSWITCH_PUSH_CLOCKWISE_START = 347,				// Push turnswitch clockwise (1/3)
	LA_TURNSWITCH_PUSH_CLOCKWISE_CONTINUE = 348,			// Push turnswitch clockwise (2/3) 
	LA_TURNSWITCH_PUSH_CLOCKWISE_END = 349,					// Push turnswitch clockwise (3/3)
	LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START = 350,		// Push turnswitch counter-clockwise (1/3)
	LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_CONTINUE = 351,	// Push turnswitch counter-clockwise (2/3)
	LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END = 352,			// Push turnswitch counter-clockwise (3/3)
	LA_CROUCH_TURN_LEFT = 353,								// Rotate left crouching (looped)
	LA_CROUCH_TURN_RIGHT = 354,								// Rotate right crouching (looped)
	LA_SHIMMY_LEFT_CORNER_OUTER_90 = 355,				    // Shimmy around outer left corner (90)
	LA_SHIMMY_LEFT_CORNER_OUTER_45 = 356,					// Shimmy around outer left corner (45)
	LA_SHIMMY_RIGHT_CORNER_OUTER_90 = 357,				    // Shimmy around outer right corner (90)
	LA_SHIMMY_RIGHT_CORNER_OUTER_45 = 358,					// Shimmy around outer right corner (45)
	LA_SHIMMY_LEFT_CORNER_INNER_90 = 359,				    // Shimmy around inner left corner (90)
	LA_SHIMMY_LEFT_CORNER_INNER_45 = 360,					// Shimmy around inner left corner (45)
	LA_SHIMMY_RIGHT_CORNER_INNER_90 = 361,				    // Shimmy around inner right corner (90)
	LA_SHIMMY_RIGHT_CORNER_INNER_45 = 362,					// Shimmy around inner right corner (45)
	LA_LADDER_LEFT_CORNER_OUTER_START = 363,				// Ladder around outer left corner (1/2)
	LA_LADDER_LEFT_CORNER_OUTER_END = 364,					// Ladder around outer left corner (2/2)
	LA_LADDER_RIGHT_CORNER_OUTER_START = 365,				// Ladder around outer right corner (1/2)
	LA_LADDER_RIGHT_CORNER_OUTER_END = 366,					// Ladder around outer right corner (2/2)
	LA_LADDER_LEFT_CORNER_INNER_START = 367,				// Ladder around inner left corner (1/2)
	LA_LADDER_LEFT_CORNER_INNER_END = 368,					// Ladder around inner left corner (2/2)
	LA_LADDER_RIGHT_CORNER_INNER_START = 369,				// Ladder around inner right corner (1/2)
	LA_LADDER_RIGHT_CORNER_INNER_END = 370,					// Ladder around inner right corner (2/2)
	LA_JUMP_UP_TO_ROPE_START = 371,							// Jump up > rope idle (1/2)
	LA_TRAIN_OVERBOARD_DEATH = 372,							// Train overboard death
	LA_JUMP_UP_TO_ROPE_END = 373,							// Jump up > rope idle (2/2)
	LA_ROPE_IDLE = 374,										// Rope idle (looped)
	LA_ROPE_DOWN_START = 375,								// Rope idle > descend rope
	LA_ROPE_UP = 376,										// Ascend rope (looped)
	LA_ROPE_UP_TO_HANG_IDLE_UNUSED = 377,					// Ascend rope > rope hang
	LA_ROPE_HANG_TO_FREEFALL_UNUSED = 378,					// Rope hang > freefall
																// NOTE: 391 is a duplicate with a different state.
	LA_REACH_TO_ROPE_SWING = 379,							// Reach > rope swing
	LA_ROPE_SWING_JUMP_FLIP_TO_REACH_UNUSED = 380,			// Rope swing backflip > reach
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_1 = 381,				// Rope swing > freefall, 1st opportunity
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_3 = 382,				// Rope swing > freefall, 3rd opportunity
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_6 = 383,				// Rope swing > freefall, 6th opportunity
	LA_ROPE_DOWN = 384,										// Descend rope (looped)
	LA_ROPE_DOWN_END = 385,									// Descend rope > rope idle
	LA_ROPE_SWING_TO_REACH_3 = 386,							// Rope swing > reach, 3rd opportunity
	LA_ROPE_IDLE_TO_SWING = 387,							// Rope idle > swing
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_5 = 388,				// Rope swing > freefall, 5th opportunity
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_4 = 389,				// Rope swing > freefall, 4th opportunity
	LA_ROPE_SWING_TO_FREEFALL_UNUSED_2 = 390,				// Rope swing > freefall, 2nd opportunity
	LA_ROPE_HANG_TO_FREEFALL_DUPLICATE_UNUSED = 391,		// Rope hang > freefall
	LA_ROPE_TURN_CLOCKWISE = 392,							// Turn clockwise on rope
	LA_ROPE_TURN_COUNTER_CLOCKWISE = 393,					// Turn counter-clockwise on rope
	LA_ROPE_SWING = 394,									// Swing on rope
	LA_LADDER_TO_HANG_UNUSED = 395,							// Ladder idle > hang
	LA_ROPE_HANG_TO_SWING_BACK_CONTINUE_UNUSED = 396,		// Rope hang > swing back (2/3)
	LA_ROPE_HANG_TO_SWING_BACK_END_UNUSED = 397,			// Rope hang > swing back (3/3)
	LA_ROPE_HANG_TO_SWING_BACK_START_UNUSED = 398,			// Rope hang > swing back (1/3)
	LA_ROPE_HANG_TO_SWING_FORWARD_SOFT_UNUSED = 399,		// Rope hang > swing forward, soft
																// NOTE: same as first 16 frames of 408.
	LA_WATERSKIN_POUR_LOW = 400,							// Pour waterskin low
	LA_WATERSKIN_FILL = 401,								// Fill waterskin
	LA_WATERSKIN_POUR_HIGH = 402,							// Pour waterskin on scale
	LA_DOOR_OPEN_CROWBAR = 403,								// Open door with crowbar
	LA_ROPE_HANG_TO_SWING_FORWARD_HARD_UNUSED = 404,		// Rope hang > swing forward, hard
	LA_ROPE_SWING_HANDOVER_UNUSED = 405,					// Rope swing > hand over to next rope
	LA_ROPE_SWING_TO_REACH_1 = 406,							// Rope swing > reach, 1st opportunity
	LA_ROPE_SWING_TO_REACH_2 = 407,							// Rope swing > reach, 2nd opportunity
	LA_ROPE_HANG_IDLE_UNUSED = 408,							// Rope hang (looped)
	LA_ROPE_SWING_TO_REACH_UNUSED_3 = 409,					// Rope swing > reach, 3rd unused opportunity
	LA_ROPE_SWING_TO_REACH_UNUSED_2 = 410,					// Rope swing > reach, 2nd unused opportunity
	LA_ROPE_SWING_TO_REACH_UNUSED_1 = 411,					// Rope swing > reach, 1st unused opportunity
	LA_DOUBLEDOOR_OPEN_PUSH = 412,							// Push double doors 
	LA_BUTTON_LARGE_PUSH = 413,								// Push big button 
	LA_JUMPSWITCH_PULL = 414,								// Pull jumpswitch
	LA_UNDERWATER_CEILING_SWITCH_PULL = 415,				// Pull underwater ceiling switch
	LA_UNDERWATER_DOOR_OPEN = 416,							// Open underwater_door
	LA_PUSHABLE_PUSH_TO_STAND = 417,						// Push pushable object (not looped) > stand
	LA_PUSHABLE_PULL_TO_STAND = 418,						// Pull pushable object (not looped) > stand
																// TODO: add TR1-3 push/pull anims.
	LA_CROWBAR_PRY_WALL_FAST = 419,							// Pry item off wall quickly
	LA_CROWBAR_USE_ON_FLOOR = 420,							// Use crowbar to activate floor lever
	LA_CRAWL_JUMP_FLIP_DOWN = 421,							// Roll jump down from crawl
	LA_HARP_PLAY = 422,										// Play harp
	LA_TRIDENT_SET = 423,									// Place trident on Poseidon statue
	LA_PICKUP_PEDESTAL_HIGH = 424,							// Standing pickup from high pedestal
	LA_PICKUP_PEDESTAL_LOW = 425,							// Standing pickup from low pedestal
	LA_SENET_ROLL = 426,									// Roll Senet sticks
	LA_TORCH_LIGHT_1 = 427,									// Light torch with flame 0-1 clicks high
	LA_TORCH_LIGHT_2 = 428,									// Light torch with flame 2-3 clicks high
	LA_TORCH_LIGHT_3 = 429,									// Light torch with flame 4-5 clicks high
	LA_TORCH_LIGHT_4 = 430,									// Light torch with flame 6-7 clicks high
	LA_TORCH_LIGHT_5 = 431,									// Light torch with flame higher than 7 clicks
	LA_DETONATOR_USE = 432,									// Use mine detonator
	LA_CORRECT_POSITION_FRONT = 433,						// Adjust position forward
	LA_CORRECT_POSITION_LEFT = 434,							// Adjust position left
	LA_CORRECT_POSITION_RIGHT = 435,						// Adjust position right
	LA_CROWBAR_USE_ON_FLOOR_FAIL = 436,						// Use crowbar on floor fail
	LA_KEYCARD_USE = 437,									// Use swipe card
																// TODO: 437 is also taken by MAGIC_DEATH, currently absent from default WAD.
	LA_MINE_DEATH = 438,									// Mine explosion death
	LA_PICKUP_SARCOPHAGUS = 439,							// Pickup from sarcophagus
	LA_DRAG_BODY = 440,										// Drag dead body
	LA_BINOCULARS_IDLE = 441,								// Stand, looking through binoculars
	LA_BIG_SCORPION_DEATH = 442,							// Big scorpion death
	LA_ELEVATOR_RECOVER = 443,								// Recover from elevator crash
																// TODO: 443 is also taken by SETH_DEATH, currently absent from default WAD.
	LA_MECHANICAL_BEETLE_USE = 444,							// Wind mechanical beetle, place on floor
	LA_DOZY = 445,											// DOZY fly cheat

	// TR5
	LA_TIGHTROPE_WALK = 446,								// Tightrope walk (looped)
	LA_TIGHTROPE_WALK_TO_STAND_LEFT = 447,					// Tightrope walk > tightrope stand, left foot first
	LA_TIGHTROPE_IDLE = 448,								// Stand idle on tightrope (looped)
	LA_TIGHTROPE_WALK_TO_STAND_RIGHT = 449,					// Tightrope walk > tightrope stand, right foot first
	LA_TIGHTROPE_STAND_TO_WALK = 450,						// Tightrope stand > tightrope walk 
	LA_TIGHTROPE_TURN_180 = 451,							// Turn 180 on rightrope
	LA_TIGHTROPE_UNBALANCE_LEFT = 452,						// Lean left on tightrope
	LA_TIGHTROPE_RECOVER_LEFT = 453,						// Lean left on tightrope > tightrope stand
	LA_TIGHTROPE_FALL_LEFT = 454,							// Fall left from tightrope > freefall
																// TODO: investigate why this doesn't link directly to freefall as other anims do.
	LA_TIGHTROPE_UNBALANCE_RIGHT = 455,						// Lean right on tightrope
	LA_TIGHTROPE_RECOVER_RIGHT = 456,						// Lean right on tightrope > tightrope stand
	LA_TIGHTROPE_FALL_RIGHT = 457,							// Fall right from tightrope > freefall
	LA_TIGHTROPE_START = 458,								// Stand > tightrope walk
	LA_TIGHTROPE_END = 459,									// Tightrope walk > stand
	LA_DOVESWITCH_TURN = 460,								// Examine and turn doveswitch
	LA_SWINGBAR_GRAB = 461,									// Reach > swingbar swing
	LA_SWINGBAR_SWING = 462,								// Swing around swingbar (looped)
	LA_SWINGBAR_JUMP = 463,									// Jump off swingbar > reach
	LA_LOOT_CABINET = 464,									// Search cabinet
	LA_LOOT_DRAWER = 465,									// Search drawer
	LA_LOOT_SHELF = 466,									// Search shelves
	LA_RADIO_START = 467,									// Stand > listen to headgear
	LA_RADIO_IDLE = 468,									// Listen to headgear (looped)
	LA_RADIO_END = 469,										// Listen to headgear > stand
	LA_VALVE_TURN = 470,									// Turn valve wheel
	LA_CROWBAR_PRY_WALL_SLOW = 471,							// Pry item off wall slowly
	LA_LOOT_CHEST = 472,									// Search chest on ground
	LA_LADDER_TO_CROUCH = 473,								// Pull up from ladder > crouch

	// TombEngine
	LA_VAULT_TO_CROUCH_1CLICK = 474, 						// Vault standing up 1-click > crouch
	LA_VAULT_TO_CROUCH_2CLICK = 475,						// Vault standing up 2-click > crouch
	LA_VAULT_TO_CROUCH_3CLICK = 476,						// Vault standing up 2-click > crouch
	LA_CRAWL_JUMP_DOWN_1CLICK = 477,						// Jump down 1-click from crawl > stand
	LA_CRAWL_JUMP_DOWN_23CLICK = 478,						// Jump down 2-3-click from crawl > stand
	LA_CRAWL_UP_STEP = 479,									// Crawl up step
	LA_CRAWL_DOWN_STEP = 480,								// Crawl down step
	LA_ONWATER_TO_CROUCH_1CLICK = 481,						// Pull up 1-click from tread > stand
	LA_ONWATER_TO_CROUCH_0CLICK = 482,						// Pull up 0-click from tread > stand
	LA_ONWATER_TO_CROUCH_M1CLICK = 483,						// Pull up -1-click from tread > stand
	LA_UNUSED1  = 484,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED2  = 485,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED3  = 486,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED4  = 487,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED5  = 488,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED6  = 489,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED7  = 490,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED8  = 491,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED9  = 492,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED10 = 493,                                      // Foot hang leftovers - may be safely reused
	LA_UNUSED11 = 494,                                      // Foot hang leftovers - may be safely reused
	LA_REACH_TO_HANG_OSCILLATE = 495,						// Reach > hang, thin ledge
	LA_SWANDIVE_ROLL_TO_RUN = 496,							// Swandive roll > run
	LA_LADDER_DISMOUNT_LEFT_START = 497,					// Ladder dismount left (1/2)
	LA_LADDER_DISMOUNT_LEFT_END = 498,						// Ladder dismount left (2/2)
	LA_LADDER_DISMOUNT_RIGHT_START = 499,					// Ladder dismount right (1/2)
	LA_LADDER_DISMOUNT_RIGHT_END = 500,						// Ladder dismount right (2/2)
	LA_ONWATER_TO_LADDER = 501,								// Tread water > climb to ladder idle

	NUM_LARA_ANIMS
};
#pragma endregion

enum LARA_WATER_STATUS
{
	LW_ABOVE_WATER,
	LW_UNDERWATER,
	LW_SURFACE,
	LW_FLYCHEAT,
	LW_WADE
};

enum LARA_GUN_STATUS
{
	LG_NO_ARMS,
	LG_HANDS_BUSY,
	LG_DRAW_GUNS,
	LG_UNDRAW_GUNS,
	LG_READY,
	LG_SPECIAL
};

enum WeaponAmmoType
{
	WEAPON_AMMO1,
	WEAPON_AMMO2,
	WEAPON_AMMO3,
	MAX_AMMOTYPE
};

enum LARA_MESHES
{
	LM_HIPS,
	LM_LTHIGH,
	LM_LSHIN,
	LM_LFOOT,
	LM_RTHIGH,
	LM_RSHIN,
	LM_RFOOT,
	LM_TORSO,
	LM_RINARM,
	LM_ROUTARM,
	LM_RHAND,
	LM_LINARM,
	LM_LOUTARM,
	LM_LHAND,
	LM_HEAD,
	NUM_LARA_MESHES
};

enum LARA_WEAPON_TYPE
{
	WEAPON_NONE,
	WEAPON_PISTOLS,
	WEAPON_REVOLVER,
	WEAPON_UZI,
	WEAPON_SHOTGUN,
	WEAPON_HK = 5,
	WEAPON_CROSSBOW,
	WEAPON_FLARE,
	WEAPON_TORCH,
	WEAPON_GRENADE_LAUNCHER,
	WEAPON_HARPOON_GUN,
	WEAPON_ROCKET_LAUNCHER,
	WEAPON_SNOWMOBILE,
	NUM_WEAPONS
};

enum LARA_WEAPON_TYPE_CARRIED
{
	WTYPE_MISSING = 0x0,
	WTYPE_PRESENT = 0x1,
	WTYPE_SILENCER = 0x2,
	WTYPE_LASERSIGHT = 0x4,
	WTYPE_AMMO_1 = 0x8,
	WTYPE_AMMO_2 = 0x10,
	WTYPE_AMMO_3 = 0x20,
	WTYPE_MASK_AMMO = WTYPE_AMMO_1 | WTYPE_AMMO_2 | WTYPE_AMMO_3,
};


enum LARA_CLOTH_TYPES
{
	CLOTH_MISSING,
	CLOTH_DRY,
	CLOTH_WET
};

enum class HOLSTER_SLOT : int {
	Empty = ID_LARA_HOLSTERS,
	Pistols = ID_LARA_HOLSTERS_PISTOLS,
	Uzis = ID_LARA_HOLSTERS_UZIS,
	Revolver = ID_LARA_HOLSTERS_REVOLVER,
	Shotgun = ID_SHOTGUN_ANIM,
	HK = ID_HK_ANIM,
	Harpoon = ID_HARPOON_ANIM,
	Crowssbow = ID_CROSSBOW_ANIM,
	GrenadeLauncher = ID_GRENADE_ANIM,
	RocketLauncher = ID_ROCKET_ANIM,

};
struct HolsterInfo {
	HOLSTER_SLOT leftHolster;
	HOLSTER_SLOT rightHolster;
	HOLSTER_SLOT backHolster;
};
struct Ammo {
	using CountType = uint16_t;
private:
	CountType count;
	bool isInfinite;
public:

	Ammo& operator --() {
		--count;
		return *this;
	}

	Ammo operator --(int) {
		Ammo tmp = *this;
		--*this;
		return tmp;
	}

	Ammo& operator ++() {
		++count;
		return *this;
	}

	Ammo operator ++(int) {
		Ammo tmp = *this;
		++*this;
		return tmp;
	}

	Ammo& operator =(size_t val) {
		count = clamp(val);
		return *this;
	}

	bool operator ==(size_t val) {
		return count == clamp(val);
	}

	Ammo& operator =(Ammo& rhs) {
		count = rhs.count;
		isInfinite = rhs.count;
		return *this;
	}

	Ammo operator +(size_t val) {
		Ammo tmp = *this;
		tmp += val;
		return tmp;
	}

	Ammo operator -(size_t val) {
		Ammo tmp = *this;
		tmp -= val;
		return tmp;
	}

	Ammo& operator +=(size_t val) {
		int tmp = this->count + val;
		this->count = clamp(tmp);
		return *this;
	}

	Ammo& operator -=(size_t val) {
		int tmp = this->count - val;
		this->count = clamp(tmp);
		return *this;
	}

	operator bool() {
		return isInfinite || (count > 0);
	}
	static CountType clamp(int val) {
		return std::clamp(val, 0, static_cast<int>(std::numeric_limits<CountType>::max()));
	}

	bool hasInfinite() const {
		return isInfinite;
	}

	CountType getCount() const {
		return count;
	}

	void setInfinite(bool infinite) {
		isInfinite = infinite;
	}
};
struct CarriedWeaponInfo
{
	bool Present;
	Ammo Ammo[MAX_AMMOTYPE];
	int SelectedAmmo; // WeaponAmmoType_enum
	bool HasLasersight;
	bool HasSilencer;
};

#define MaxDiaryPages	64
#define MaxStringsPerPage	8

struct DiaryString
{
	int x, y;
	short stringID;
};

struct DiaryPage
{
	DiaryString	Strings[MaxStringsPerPage];
};

struct DiaryInfo
{
	bool Present;
	short numPages;
	short currentPage;
	DiaryPage	Pages[MaxDiaryPages];
};

struct LARA_ARM
{
	int frameBase;
	short frameNumber;
	short animNumber;
	byte lock;
	short yRot;
	short xRot;
	short zRot;
	short flash_gun;
};

struct AnimsNew
{
	bool CrouchRoll;				// crouch roll
	bool Monkey180Roll;				// the 180 degrees roll on monkeybars
	bool Crawl1clickup;				// going 1 click up in crawlspaces
	bool Crawl1clickdown;			// going 1 click down in crawlspaces
	bool CrawlExit1click;			// crawlspace exit at 1 click
	bool CrawlExit2click;			// crawlspace exit at 2 clicks
	bool CrawlExit3click;			// crawlspace exit at 3 clicks
	bool CrawlVault1click;			// vault into crawlspace at 1 click
	bool CrawlVault2click;			// vault into crawlspace at 2 clicks
	bool CrawlVault3click;			// vault into crawlspace at 3 clicks
	bool MonkeyVault;				// vault up to monkeybars when pressing up + action underneath them. super annoying :)
	bool CrawlExitJump;				// TR5 crawlspace exit with jump!
	bool SwandiveRollRun;			// the transition from swandive roll to run
	bool OscillateHanging;			// the thin ledge grab animation from TR1 and 2
	bool CrawlFlexWaterPullUp;
	bool CrawlFlexSubmerged;
};

#ifdef NEW_TIGHTROPE
struct LaraTightrope
{
	float balance;
	unsigned short timeOnTightrope;
	bool canGoOff;
	short tightropeItem; // maybe give Tightrope Item a property for difficulty?
};
#endif

struct LaraInfo
{
	short itemNumber;
	LARA_GUN_STATUS gunStatus; // LG_enum
	LARA_WEAPON_TYPE gunType; // WEAPON_enum
	LARA_WEAPON_TYPE requestGunType; // WEAPON_enum
	LARA_WEAPON_TYPE lastGunType; // WEAPON_enum
	short calcFallSpeed;
	LARA_WATER_STATUS waterStatus; // LW_enum
	short climbStatus;
	short poseCount;
	short hitFrame;
	short hitDirection;
	int sprintTimer;
	short air;
	short diveCount;
	short deathCount;
	short currentActive;
	short currentXvel;
	short currentYvel;
	short currentZvel;
	short spazEffectCount;
	short flareAge;
	short burnCount;
	short weaponItem;
	HolsterInfo holsterInfo;
	short flareFrame;
	short poisoned;
	byte wet[NUM_LARA_MESHES];
	bool flareControlLeft;
	bool look;
	bool burn;
	bool keepDucked;
	bool isMoving;
	bool canMonkeySwing;
	byte burnBlue;
	bool burnSmoke;
	bool isDucked;
	bool hasFired;
	bool busy;
	bool oldBusy;
	bool uncontrollable;
	bool litTorch;
	bool isClimbing;
	bool fired;
	int waterSurfaceDist;
	PHD_VECTOR lastPos;
	PHD_3DPOS nextCornerPos;
	FX_INFO* spazEffect;
	int meshPtrs[NUM_LARA_MESHES];
	ITEM_INFO* target;
	short targetAngles[2];
	short turnRate;
	short moveAngle;
	short headYrot;
	short headXrot;
	short headZrot;
	short torsoYrot;
	short torsoXrot;
	short torsoZrot;
	LARA_ARM leftArm;
	LARA_ARM rightArm;
	CREATURE_INFO* creature;
	byte ropeSegment;
	byte ropeDirection;
	short ropeArcFront;
	short ropeArcBack;
	short ropeLastX;
	short ropeMaxXForward;
	short ropeMaxXBackward;
	int ropeDFrame;
	int ropeFrame;
	unsigned short ropeFrameRate;
	unsigned short ropeY;
	int ropePtr;
	short interactedItem;
	int ropeOffset;
	int ropeDownVel;
	byte ropeFlag;
	byte moveCount;
	int ropeCount;
	signed char location;
	signed char highestLocation;
	signed char locationPad;
#if NEW_TIGHTROPE
	LaraTightrope tightrope;
#else
	byte tightRopeOnCount;
	byte tightRopeOff;
	byte tightRopeFall;
#endif
	/// =================================== NEW:
	byte BeetleLife;
	short hasBeetleThings;// & 1 -> beetle. & 2 -> combo1. & 4 ->combo2
	byte small_waterskin;// 1 = has the waterskin. 2 = has the waterskin and it has 1 liter. etc. max value is 4: has skin + 3 = 4
	byte big_waterskin;// 1 = has the waterskin. 2 = has the waterskin and it has 1 liter. etc. max value is 6: has skin + 5 liters = 6
	short Vehicle;
	short ExtraAnim;
	bool mineL;
	bool mineR;
	CarriedWeaponInfo Weapons[static_cast<int>(LARA_WEAPON_TYPE::NUM_WEAPONS)];
	DiaryInfo Diary;
	int Puzzles[NUM_PUZZLES];
	int Keys[NUM_KEYS];
	int Pickups[NUM_PICKUPS];
	int Examines[NUM_EXAMINES];
	int PuzzlesCombo[NUM_PUZZLES * 2];
	int KeysCombo[NUM_KEYS * 2];
	int PickupsCombo[NUM_PICKUPS * 2];
	int ExaminesCombo[NUM_EXAMINES * 2];
	int Secrets;
	bool Lasersight;
	bool Crowbar;
	bool Torch;
	bool Silencer;
	bool Binoculars;
	int NumSmallMedipacks;
	int NumLargeMedipacks;
	int NumFlares;
};
