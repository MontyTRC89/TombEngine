#pragma once
#include "lara_struct.h"

#define FRONT_ARC ANGLE(90.0f)
#define LARA_LEAN_RATE ANGLE(1.5f)
#define LARA_LEAN_MAX ANGLE(11.0f)
#define LARA_TURN_RATE ANGLE(2.25f)
#define LARA_JUMP_TURN ANGLE(3.0f)
#define LARA_SLOW_TURN ANGLE(4.0f)
#define LARA_MED_TURN ANGLE(6.0f)
#define LARA_FAST_TURN ANGLE(8.0f)

constexpr auto LARA_HITE = 762; // the size of lara (from the floor to the top of the head)
constexpr auto LARA_FREEFALL_SPEED = 131;
constexpr auto LARA_RAD = 100;
constexpr auto LARA_VELOCITY = 12;

extern LaraInfo Lara;
extern ITEM_INFO* LaraItem;
extern byte LaraNodeUnderwater[NUM_LARA_MESHES];

#define LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] = MESHES(slot, mesh)
#define CHECK_LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] == MESHES(slot, mesh)
#define INIT_LARA_MESHES(mesh, to, from) Lara.meshPtrs[mesh] = LARA_MESHES(to, mesh) = LARA_MESHES(from, mesh)

#define LaraRoutineFunction void(ITEM_INFO* item, COLL_INFO* coll)
extern std::function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1];
extern std::function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1];


void lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll);
void lara_default_col(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_land(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_run(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll);

void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_special(ITEM_INFO* item, COLL_INFO* coll);

void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastturn(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_null(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll);
void lara_void_func(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_run(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll);

void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll);
