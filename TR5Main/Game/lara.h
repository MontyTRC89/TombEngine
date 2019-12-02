#pragma once

#include "..\Global\global.h"
#include "..\Renderer\Renderer11.h"

typedef struct CarriedWeaponInfo {
	bool Present;
	__int16 Ammo[3];
	byte SelectedAmmo;
	bool HasLasersight;
	bool HasSilencer;
};

typedef struct DiaryInfo {
	bool Present;
};

typedef struct WaterskinInfo {
	bool Present;
	__int32 Quantity;
};

typedef struct LaraExtraInfo {
	__int16 Vehicle;
	__int16 ExtraAnim;
	CarriedWeaponInfo Weapons[NUM_WEAPONS];
	DiaryInfo Diary;
	WaterskinInfo Waterskin1;
	WaterskinInfo Waterskin2;
	RendererMesh* MeshesPointers[15];
};

extern LaraExtraInfo g_LaraExtra;

//#define LookUpDown ((void (__cdecl*)()) 0x0044D310)
//#define LookLeftRight ((void (__cdecl*)()) 0x0044D440)
//#define ResetLook ((void (__cdecl*)()) 0x0044D220)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)
//#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
#define GetLaraJointPosition ((void (__cdecl*)(PHD_VECTOR*, __int32)) 0x0041E2A0)
#define CheckForHoldingState ((__int32 (__cdecl*)(__int16)) 0x00452AF0)
#define AnimateLara ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004563F0)

//#define LaraFloorFront ((__int32 (__cdecl*)(ITEM_INFO*, __int16, __int32)) 0x004438F0)
//#define LaraCeilingFront ((__int32 (__cdecl*)(ITEM_INFO*, __int16, __int32, __int32)) 0x00442DB0)
//#define GetLaraCollisionInfo ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00444F80)
//#define TestLaraVault ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00445100)
//#define ShiftItem ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x004120A0)
//#define lara_as_wade ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x0044B770)
//#define lara_as_back ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x0044AE20)
//#define lara_as_run ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00449330)
//#define lara_as_walk ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00449260)

#define LaraHangTest ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x004460F0)
#define TestLaraSlide ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x004431F0)

extern void(*lara_control_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);
extern void(*lara_collision_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);

void __cdecl lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl GetTighRopeFallOff(__int32 Regularity);
void __cdecl LookLeftRight();
void __cdecl LookUpDown();
void __cdecl ResetLook();
void __cdecl lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_default_col(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_wade(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_roll(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_slide(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_back(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_compress(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_land(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_splat(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_death(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_pose(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_run(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_walk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_wade(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_special(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_slide(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_fastturn(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_null(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_back(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_compress(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_splat(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, __int16 rot, __int16 flip);
void __cdecl lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_hang(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hang(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl CanLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, __int16 angle);
void __cdecl lara_void_func(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_death(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_run(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_walk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_reach(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_reach(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_stop(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_stop(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbroped(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_climbrope(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_ropefwd(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_roper(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_ropel(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_rope(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_rope(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl ApplyVelocityToRope(int node, unsigned __int16 angle, unsigned __int16 n);
void __cdecl UpdateRopeSwing(ITEM_INFO* item);
void __cdecl JumpOffRope(ITEM_INFO* item);
void __cdecl FallFromRope(ITEM_INFO* item);
void __cdecl lara_col_poledown(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_poleup(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_poleright(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_poleleft(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_polestat(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_monkey180(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_monkey180(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hangturnr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_hangturnlr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hangturnl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_monkeyr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_monkeyr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_monkeyl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_monkeyl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_monkeyswing(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_monkeyswing(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_hang2(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_hang2(ITEM_INFO* item, COLL_INFO* coll);
__int16 __cdecl TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll);
__int16 __cdecl TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll);
__int16 __cdecl GetDirOctant(__int32 rot);
void __cdecl MonkeySwingSnap(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl MonkeySwingFall(ITEM_INFO* item);
void __cdecl lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_dash(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_dash(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_crawlb(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_crawlb(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_all4turnr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_all4turnlr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_all4turnl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_crawl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_crawl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_all4s(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_all4s(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_duck(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_duck(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_ducklr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_duckr(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_duckl(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl TestHangSwingIn(ITEM_INFO* item, __int16 angle);
__int32 __cdecl LaraHangLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraHangRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl IsValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
//__int32 __cdecl LaraHangTest(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, __int16 angle);
__int32 __cdecl LaraTestHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl TestWall(ITEM_INFO* item, __int32 front, __int32 right, __int32 down);
//__int32 __cdecl TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraTestClimbStance(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, __int32* edge);
__int32 __cdecl LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl LaraLandedBad(ITEM_INFO* l, COLL_INFO* coll);
__int32 __cdecl LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
//__int32 __cdecl TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
__int16 LaraCeilingFront(ITEM_INFO* item, __int16 ang, __int32 dist, __int32 h);
__int16 LaraFloorFront(ITEM_INFO* item, __int16 ang, __int32 dist);
void __cdecl GetLaraCollisionInfo(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
//__int32 __cdecl GetLaraJointPos(PHD_VECTOR* arg1, __int32 arg2);
//void __cdecl AnimateLara(ITEM_INFO* item);
/*void __cdecl SetLaraUnderwaterNodes();
void __cdecl SetPendulumVelocity(int x, int y, int z);
void __cdecl LaraClimbRope(ITEM_INFO* item, COLL_INFO* coll); // 0xC (RelocPtr + 0x28)
void __cdecl FireChaff();
void __cdecl GetLaraJointPosRot(PHD_VECTOR* a1, int a2, int a3, SVECTOR* a4);
void __cdecl DoSubsuitStuff();*/

void Inject_Lara();