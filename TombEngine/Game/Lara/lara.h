#pragma once
#include "Game/control/control.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/clock.h"

struct CollisionInfo;
struct ItemInfo;

const auto LARA_GRAB_THRESHOLD = ANGLE(40.0f);
const auto FRONT_ARC		   = ANGLE(90.0f); // TODO: Check use.

// Lean rates
const auto LARA_LEAN_RATE = ANGLE(1.5f);
const auto LARA_LEAN_MAX  = ANGLE(11.0f);

// Turn rate acceleration rates
const auto LARA_TURN_RATE_ACCEL			   = ANGLE(0.25f);
const auto LARA_CRAWL_MOVE_TURN_RATE_ACCEL = ANGLE(0.15f);
const auto LARA_POLE_TURN_RATE_ACCEL	   = ANGLE(0.25f);
const auto LARA_SUBSUIT_TURN_RATE_ACCEL	   = ANGLE(0.25f);

// Turn rate maxes
const auto LARA_SLOW_TURN_RATE_MAX		  = ANGLE(2.0f);
const auto LARA_SLOW_MED_TURN_RATE_MAX	  = ANGLE(3.0f);
const auto LARA_MED_TURN_RATE_MAX		  = ANGLE(4.0f);
const auto LARA_MED_FAST_TURN_RATE_MAX	  = ANGLE(5.0f);
const auto LARA_FAST_TURN_RATE_MAX		  = ANGLE(6.0f);
const auto LARA_WADE_TURN_RATE_MAX		  = ANGLE(3.5f);
const auto LARA_SWAMP_TURN_RATE_MAX		  = ANGLE(1.5f);
const auto LARA_JUMP_TURN_RATE_MAX		  = ANGLE(1.0f);
const auto LARA_REACH_TURN_RATE_MAX		  = ANGLE(0.8f);
const auto LARA_SLIDE_TURN_RATE_MAX		  = ANGLE(3.70f);
const auto LARA_CRAWL_TURN_RATE_MAX		  = ANGLE(1.5f);
const auto LARA_CRAWL_MOVE_TURN_RATE_MAX  = ANGLE(1.75f);
const auto LARA_CROUCH_ROLL_TURN_RATE_MAX = ANGLE(0.75f);
const auto LARA_POLE_TURN_RATE_MAX		  = ANGLE(2.5f);

// Flex rates
const auto LARA_CRAWL_FLEX_RATE = ANGLE(2.25f);
const auto LARA_CRAWL_FLEX_MAX	= ANGLE(50.0f) / 2; // 2 = hardcoded number of bones to flex (head and torso).

constexpr auto LARA_HEIGHT			  = CLICK(3) - 1; // Height in basic states.
constexpr auto LARA_HEIGHT_CRAWL	  = 350;		  // Height in crawl states.
constexpr auto LARA_HEIGHT_MONKEY	  = 850;		  // Height in monkey swing states.
constexpr auto LARA_HEIGHT_TREAD	  = 700;		  // Height in water tread states.
constexpr auto LARA_HEIGHT_STRETCH	  = 870;		  // Height in jump up and ledge hang states.
constexpr auto LARA_HEIGHT_REACH	  = 820;		  // Height in reach state.
constexpr auto LARA_HEIGHT_SURFACE	  = 800;		  // Height when resurfacing water.
constexpr auto LARA_HEADROOM		  = 160;		  // Reasonable space above head.
constexpr auto LARA_RADIUS			  = 100;
constexpr auto LARA_RADIUS_CRAWL	  = 200;
constexpr auto LARA_RADIUS_UNDERWATER = 300;
constexpr auto LARA_RADIUS_DEATH	  = 400;
constexpr auto LARA_ALIGN_VELOCITY	  = 12; // TODO: Float.

constexpr auto LARA_FREEFALL_VELOCITY   = 131.0f;
constexpr auto LARA_DAMAGE_VELOCITY		= 141.0f;
constexpr auto LARA_DEATH_VELOCITY		= 155.0f;
constexpr auto LARA_DIVE_DEATH_VELOCITY = 134.0f;
constexpr auto LARA_TERMINAL_VELOCITY	= CLICK(10);

constexpr auto LARA_SWIM_VELOCITY_ACCEL		   = 2.0f;
constexpr auto LARA_SWIM_VELOCITY_DECEL		   = 1.5f;
constexpr auto LARA_TREAD_VELOCITY_MAX		   = 17.5f;
constexpr auto LARA_SWIM_VELOCITY_MAX		   = 50.0f;
constexpr auto LARA_SWIM_INTERTIA_VELOCITY_MIN = 33.5f;

constexpr auto LARA_POSITION_ADJUST_MAX_TIME = 3 * FPS;	 // 3 seconds allowed for position adjustment.
constexpr auto LARA_POSE_TIME				 = 20 * FPS; // 20 seconds to AFK pose.
constexpr auto LARA_RUN_JUMP_TIME			 = 22;		 // Frames to count before a running jump is possible.
constexpr auto LARA_SPRINT_JUMP_TIME		 = 46;		 // Frames to count before a sprint jump is possible.

constexpr auto LARA_HEALTH_MAX		   = 1000.0f;
constexpr auto LARA_HEALTH_CRITICAL	   = LARA_HEALTH_MAX / 4;
constexpr auto LARA_AIR_MAX			   = 1800.0f;
constexpr auto LARA_AIR_CRITICAL	   = LARA_AIR_MAX / 4;
constexpr auto LARA_SPRINT_ENERGY_MAX  = 120.0f;
constexpr auto LARA_POISON_POTENCY_MAX = 64.0f;

constexpr auto STEPUP_HEIGHT	   = (int)CLICK(3.0f / 2);
constexpr auto BAD_JUMP_CEILING	   = (int)CLICK(6.0f / 8);
constexpr auto SHALLOW_WATER_DEPTH = (int)CLICK(1.0f / 2);
constexpr auto WADE_DEPTH		   = STEPUP_HEIGHT;
constexpr auto SWIM_DEPTH		   = CLICK(3) - 38;
constexpr auto SLOPE_DIFFERENCE	   = 60;

constexpr auto LARA_PUSHABLE_PUSH_BBOX_Z2 = 1108 - BLOCK(1);
constexpr auto LARA_PUSHABLE_PULL_BBOX_Z2 = BLOCK(1) - 944;

extern LaraInfo Lara;
extern ItemInfo* LaraItem;
extern CollisionInfo LaraCollision;
extern byte LaraNodeUnderwater[NUM_LARA_MESHES];

#define LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] = MESHES(slot, mesh)
#define CHECK_LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] == MESHES(slot, mesh)
#define INIT_LARA_MESHES(mesh, to, from) Lara.meshPtrs[mesh] = LARA_MESHES(to, mesh) = LARA_MESHES(from, mesh)

#define LaraRoutineFunction void(ItemInfo* item, CollisionInfo* coll)
extern std::function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1];
extern std::function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1];

void LaraControl(ItemInfo* item, CollisionInfo* coll);
void LaraAboveWater(ItemInfo* item, CollisionInfo* coll);
void LaraWaterSurface(ItemInfo* item, CollisionInfo* coll);
void LaraUnderwater(ItemInfo* item, CollisionInfo* coll);
void LaraCheat(ItemInfo* item, CollisionInfo* coll);
void AnimateLara(ItemInfo* item);
void UpdateLara(ItemInfo* item, bool isTitle);
bool UpdateLaraRoom(ItemInfo* item, int height, int xOffset = 0, int zOffset = 0);