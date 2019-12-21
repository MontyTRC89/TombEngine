#pragma once

#include "..\Global\global.h"

//#define DetatchSpark ((SPARKS* (__cdecl*)(int, int)) 0x0042E6A0)
//#define AddWaterSparks ((void (__cdecl*)(int, int, int, int)) 0x00483180)
//#define TriggerUnderwaterExplosion ((void (__cdecl*)(ITEM_INFO*)) 0x0044F500)
//#define TriggerExplosionSparks ((void (__cdecl*)(int, int, int, int, int, int, short)) 0x0042F610)
#define ExplodingDeath ((void (__cdecl*)(short, int, int)) 0x00484080)
//#define GetFreeSpark ((short (__cdecl*)()) 0x0042E790)
//#define GetFreeDrip ((short (__cdecl*)()) 0x00483D00)
//#define GetFreeSmokeSpark ((short (__cdecl*)()) 0x00481D40)
//#define TriggerDartSmoke ((void (__cdecl*)(int, int, int, int, int, int)) 0x00430D90)
//#define TriggerGunShell ((void (__cdecl*)(short, int, int)) 0x00482A60)
//#define TriggerLaraDrips ((void (__cdecl*)()) 0x00483F00)
//#define SetupRipple ((void (__cdecl*)(int, int, int, int, int)) 0x00430910)
//#define TriggerShockwave ((void (__cdecl*)(PHD_3DPOS*, int, int, int, int, int)) 0x00484670)
//#define TriggerExplosionBubbles ((void (__cdecl*)(int, int, int, short)) 0x00431070)
//#define AddFire ((void (__cdecl*)(int, int, int, byte, short, int)) 0x00481B40)
#define InitialiseSmokeEmitter ((void (__cdecl*)(short)) 0x0043D9D0)
#define SmokeEmitterControl ((void (__cdecl*)(short)) 0x00431560)
#define DrawLensFlare ((void (__cdecl*)(ITEM_INFO*)) 0x00485290)
#define ControlWaterfallMist ((void (__cdecl*)(short)) 0x00432CA0)
#define TriggerRicochetSparks ((void (__cdecl*)(GAME_VECTOR*, short, int, int)) 0x0042F060)

extern SPLASH_STRUCT Splashes[4];
extern RIPPLE_STRUCT Ripples[32];
extern int DeadlyBounds[6];
extern SPARKS Sparks[1024];
extern SP_DYNAMIC SparkDynamics[8];
extern SPLASH_SETUP SplashSetup;
extern int SmokeWeapon;
extern int SmokeCountL;
extern int SmokeCountR;

void DetatchSpark(int num, int type);
int GetFreeSpark();
void UpdateSparks();
void TriggerRicochetSpark(GAME_VECTOR* pos, short angle, int num, int unk);
void TriggerCyborgSpark(int x, int y, int z, short xv, short yv, short zv);
void TriggerExplosionSparks(int x, int y, int z, int extraTrig, int dynamic, int uw, int roomNumber);
void TriggerExplosionSmokeEnd(int x, int y, int z, int uw);
void TriggerExplosionSmoke(int x, int y, int z, int uw);
void TriggerFireFlame(int x, int y, int z, int fxObj, int type);
void TriggerSuperJetFlame(ITEM_INFO* item, int yvel, int deadly);
void SetupSplash(SPLASH_SETUP* setup);
void UpdateSplashes();
void SetupRipple(int x, int y, int z, char size, char flags);
void TriggerUnderwaterBlood(int x, int y, int z, int sizeme);
void TriggerWaterfallMist(int x, int y, int z, int angle);
void TriggerDartSmoke(int x, int y, int z, int xv, int zv, int hit);
void KillAllCurrentItems(short itemNumber);
void TriggerDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
void ClearDynamicLights();
void TriggerRocketFlame(int x, int y, int z, int xv, int yv, int zv, int itemNumber);
void TriggerRocketSmoke(int x, int y, int z, int bodyPart);
void GrenadeLauncherSpecialEffect1(int x, int y, int z, int flag1, int flag2);
void GrenadeExplosionEffects(int x, int y, int z, short roomNumber);
void TriggerMetalSparks(int x, int y, int z, int xv, int yv, int zv, int additional);
void WadeSplash(ITEM_INFO* item, int wh, int wd);
void Splash(ITEM_INFO* item);
void SetupSplash(SPLASH_SETUP* setup);

void Inject_Effect2();