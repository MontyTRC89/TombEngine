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

constexpr auto LARA_HEIGHT = 762;		// The size of lara (from the floor to the top of the head)
constexpr auto LARA_HEIGHT_CRAWL = 400; // Size of Lara in crawl state
constexpr auto LARA_HEADROOM = 160;     // Amount of reasonable space above Lara's head
constexpr auto LARA_FREEFALL_SPEED = 131;
constexpr auto LARA_RAD = 100;
constexpr auto LARA_VELOCITY = 12;

extern LaraInfo Lara;
extern ITEM_INFO* LaraItem;
extern COLL_INFO lara_coll;
extern byte LaraNodeUnderwater[NUM_LARA_MESHES];

#define LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] = MESHES(slot, mesh)
#define CHECK_LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] == MESHES(slot, mesh)
#define INIT_LARA_MESHES(mesh, to, from) Lara.meshPtrs[mesh] = LARA_MESHES(to, mesh) = LARA_MESHES(from, mesh)

#define LaraRoutineFunction void(ITEM_INFO* item, COLL_INFO* coll)
extern std::function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1];
extern std::function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1];

void LaraControl(short itemNumber);
void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll);
void LaraSurface(ITEM_INFO* item, COLL_INFO* coll);
void LaraCheat(ITEM_INFO* item, COLL_INFO* coll);
void AnimateLara(ITEM_INFO* item);
