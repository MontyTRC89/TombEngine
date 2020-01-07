#pragma once
#include "../Global/global.h"

//#define ControlGuard ((void (__cdecl*)(short)) 0x0046F5E0)
#define ControlSubmarine ((void (__cdecl*)(short)) 0x0045D3F0)
//#define ControlDoberman ((void (__cdecl*)(short)) 0x00428A10)
///#define ControlDog ((void (__cdecl*)(short)) 0x0043B730)
//#define ControlReaper ((void (__cdecl*)(short)) 0x0045DAF0)
//#define ControlLarson ((void (__cdecl*)(short)) 0x0046A080)
#define ControlCyborg ((void (__cdecl*)(short)) 0x0043A340)
//#define ControlGuardM16 ((void (__cdecl*)(short)) 0x00478250)
#define InitialiseChef ((void (__cdecl*)(short)) 0x00410990) // not used anymore !
#define ControlChef ((void (__cdecl*)(short)) 0x00410A60)    // not used anymore !
#define ControlGuardLaser ((void (__cdecl*)(short)) 0x0048CDD0)
#define ControlHydra ((void (__cdecl*)(short)) 0x0043BF70)
//#define ControlImp ((void (__cdecl*)(short)) 0x0043BEA0)
#define ControlLightingGuide ((void (__cdecl*)(short)) 0x0048E580)
//#define ControlBrowsBeast ((void (__cdecl*)(short)) 0x0048E960)
#define InitialiseLagoonWitch ((void (__cdecl*)(short)) 0x0047D2D0) // not used anymore !
#define ControlLagoonWitch ((void (__cdecl*)(short)) 0x0047D360)    // not used anymore !
//#define ControlInvisibleGhost ((void (__cdecl*)(short)) 0x00477AB0)
#define InitialiseLittleBats ((void (__cdecl*)(short)) 0x00407EC0)
#define ControlLittleBats ((void (__cdecl*)(short)) 0x00407F50)
#define InitialiseSpiders ((void (__cdecl*)(short)) 0x0043F2B0)
#define ControlSpiders ((void (__cdecl*)(short)) 0x0047A200)
//#define ControlGladiator ((void (__cdecl*)(short)) 0x00436700)
#define ControlRomanStatue ((void (__cdecl*)(short)) 0x0046BC10)
#define ControlAutoGuns ((void (__cdecl*)(short)) 0x004078A0)
#define ControlGunShip ((void (__cdecl*)(short)) 0x00487FF0)

#define InitialiseRomanStatue ((void (__cdecl*)(short)) 0x0046BB00) // need to check a dword_ variable before decompiling

void InitialiseGuard(short itemNum);
void InitialiseGuardM16(short itemNum);
void InitialiseGuardLaser(short itemNum);
void InitialiseSubmarine(short itemNum);
void InitialiseDoberman(short itemNum);
void InitialiseDog(short itemNum);
void InitialiseReaper(short itemNum);
void InitialiseLarson(short itemNum);
void InitialiseCyborg(short itemNum);
void InitialiseHydra(short itemNum);
void InitialiseImp(short itemNum);
void InitialiseLightingGuide(short itemNum);
void InitialiseBrownBeast(short itemNum);
void InitialiseInvisibleGhost(short itemNum);
void InitialiseGladiator(short itemNum);
/// void InitialiseRomanStatue(short itemNum)
void InitialiseAutoGuns(short itemNum);
void InitialisePushableBlock(short itemNum);
void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void PushableBlockControl(short itemNumber);
void PushableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
int TestBlockMovable(ITEM_INFO* item, int blokhite);
int TestBlockPush(ITEM_INFO* item, int blokhite, unsigned short quadrant);
int TestBlockPull(ITEM_INFO* item, int blokhite, short quadrant);
void ControlGuard(short itemNum);
void ControlDoberman(short itemNumber);
void ControlReaper(short itemNumber);
void ControlGuardM16(short itemNumber);
void ImpThrowStones(ITEM_INFO* item);
void ControlImp(short itemNumber);
void ControlGladiator(short itemNumber);
void ControlBrowsBeast(short itemNumber);
void ControlInvisibleGhost(short itemNumber);
short GetNextRat();
void ControlLittleRats(short itemNumber);
void InitialiseArmedBaddy2(short itemNum);
void ArmedBaddy2Control(short itemNum);
void ControlDog(short itemNumber);
void ControlLarson(short itemNumber);
void InitialiseRaisingBlock(short itemNumber);
void ControlRaisingBlock(short itemNumber);
void InitialiseTeethSpikes(short itemNumber);
int CollidedWithTeethSpikes(ITEM_INFO* item);
void ControlTeethSpikes(short itemNumber);
void PulseLightControl(short itemNumber);
void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int rot, __int16 roomNumber, __int16 falloff);
void StrobeLightControl(short itemNumber);
void ColorLightControl(short itemNumber);
void ElectricalLightControl(short itemNumber);
void BlinkingLightControl(short itemNumber);
void InitialiseTwoBlocksPlatform(short itemNumber);
void TwoBlocksPlatformControl(short itemNumber);
void TwoBlocksPlatformFloor(ITEM_INFO* item, int x, int y, int z, int* height);
void TwoBlocksPlatformCeiling(ITEM_INFO* item, int x, int y, int z, int* height);
int IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z);
void InitialiseRaisingCog(short itemNumber);
void RaisingCogControl(short itemNumber);

