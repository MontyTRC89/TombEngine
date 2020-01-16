#pragma once

#include "..\Global\global.h"

#define ExplodingDeath ((void (__cdecl*)(short, int, int)) 0x00484080)
#define InitialiseSmokeEmitter ((void (__cdecl*)(short)) 0x0043D9D0)
#define SmokeEmitterControl ((void (__cdecl*)(short)) 0x00431560)
#define DrawLensFlare ((void (__cdecl*)(ITEM_INFO*)) 0x00485290)

extern SPLASH_STRUCT Splashes[MAX_SPLASH];
extern RIPPLE_STRUCT Ripples[32];
extern int DeadlyBounds[6];
extern SPARKS Sparks[1024];
extern SP_DYNAMIC SparkDynamics[8];
extern SPLASH_SETUP SplashSetup;
extern int SmokeWeapon;
extern int SmokeCountL;
extern int SmokeCountR;
extern PHD_VECTOR NodeVectors[16];
extern NODEOFFSET_INFO NodeOffsets[16];

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
void SetupSplash(const SPLASH_SETUP* const setup);
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
void Inject_Effect2();