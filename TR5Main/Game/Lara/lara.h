#pragma once
#include "lara_struct.h"

struct ITEM_INFO;
struct COLL_INFO;

#define FRONT_ARC ANGLE(90.0f)
#define LARA_LEAN_RATE ANGLE(1.5f)
#define LARA_LEAN_MAX ANGLE(11.0f)
#define LARA_LEAN_DASH_MAX ANGLE(16.0f)
#define LARA_TURN_RATE ANGLE(2.25f)
#define SUB_SUIT_TURN_RATE ANGLE(0.75f)
#define LARA_JUMP_TURN ANGLE(3.0f)
#define LARA_SLOW_TURN ANGLE(4.0f)
#define LARA_MED_TURN  ANGLE(6.0f)
#define LARA_FAST_TURN ANGLE(8.0f)
#define LARA_GRAB_THRESHOLD ANGLE(30.0f)

constexpr auto LARA_HEIGHT = CLICK(3) - 1; // The size of Lara (from the floor to the top of the head)
constexpr auto LARA_HEIGHT_CRAWL = 400;    // Size of Lara in crawl state
constexpr auto LARA_HEIGHT_MONKEY = 600;   // Size of Lara in monkey state
constexpr auto LARA_HEIGHT_SURFSWIM = 700; // Size of Lara in surface swim state
constexpr auto LARA_HEIGHT_STRETCH = 870;  // Size of Lara in jump-up or ledge hanging state
constexpr auto LARA_HEIGHT_SURFACE = 800;  // Size of Lara when surfacing water
constexpr auto LARA_HEADROOM = 160;        // Amount of reasonable space above Lara's head
constexpr auto LARA_RAD = 100;
constexpr auto LARA_RAD_CRAWL = 200;
constexpr auto LARA_RAD_UNDERWATER = 300;
constexpr auto LARA_RAD_DEATH = 400;
constexpr auto LARA_FREEFALL_SPEED = 131;
constexpr auto LARA_VELOCITY = 12;

constexpr auto LARA_HEALTH_MAX = 1000.0f;
constexpr auto LARA_AIR_MAX = 1800.0f;
constexpr auto LARA_SPRINT_MAX = 120.0f;

extern LaraInfo Lara;
extern ITEM_INFO* LaraItem;
extern COLL_INFO LaraCollision;
extern byte LaraNodeUnderwater[NUM_LARA_MESHES];

#define LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] = MESHES(slot, mesh)
#define CHECK_LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] == MESHES(slot, mesh)
#define INIT_LARA_MESHES(mesh, to, from) Lara.meshPtrs[mesh] = LARA_MESHES(to, mesh) = LARA_MESHES(from, mesh)

#define LaraRoutineFunction void(ITEM_INFO* item, COLL_INFO* coll)
extern std::function<LaraRoutineFunction> lara_control_routines[NUM_LARA_STATES + 1];
extern std::function<LaraRoutineFunction> lara_collision_routines[NUM_LARA_STATES + 1];

void LaraControl(ITEM_INFO* item, COLL_INFO* coll);
void LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll);
void LaraSurface(ITEM_INFO* item, COLL_INFO* coll);
void LaraCheat(ITEM_INFO* item, COLL_INFO* coll);
void AnimateLara(ITEM_INFO* item);
