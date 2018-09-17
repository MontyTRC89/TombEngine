#pragma once

#include "..\Global\global.h"

void __cdecl ClampRotation(PHD_3DPOS *pos, __int16 angle, __int16 rot);

void __cdecl InitialiseWildBoar(__int16 itemNum);
void __cdecl WildBoarControl(__int16 itemNum);

void __cdecl InitialiseSmallScorpion(__int16 itemNum);
void __cdecl SmallScorpionControl(__int16 itemNum);

void __cdecl InitialiseBat(__int16 itemNum);
void __cdecl BatControl(__int16 itemNum);

void __cdecl InitialiseBaddy(__int16 itemNum);
void __cdecl BaddyControl(__int16 itemNum);

void __cdecl InitialiseSas(__int16 itemNum);
void __cdecl SasControl(__int16 itemNum);

void __cdecl InitialiseMummy(__int16 itemNum);
void __cdecl MummyControl(__int16 itemNum);

void __cdecl InitialiseSkeleton(__int16 itemNum);
void __cdecl SkeletonControl(__int16 itemNum);

//extern __int16 LaraMotorbike;
//void __cdecl InitialiseMotorbike(__int16 itemNum);
__int32 __cdecl GetCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
__int32 __cdecl TestHeight(ITEM_INFO* item, __int32 dz, __int32 dx, PHD_VECTOR* pos);
__int32 __cdecl DoShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old);
__int32 __cdecl DoDynamics(__int32 height, __int32 fallspeed, __int32 *y);
__int32 __cdecl QuadDynamics(ITEM_INFO* item);
void __cdecl AnimateQuadBike(ITEM_INFO* item, __int32 collide, __int32 dead);
__int32 __cdecl QuadUserControl(ITEM_INFO* item, __int32 height, int* pitch);
void __cdecl InitialiseQuadBike(__int16 itemNumber);
void __cdecl QuadBikeCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll);
__int32 __cdecl QuadBikeControl();
void __cdecl TriggerExhaustSmoke(__int32 x, __int32 y, __int32 z, __int16 angle, __int32 speed, __int32 moving);

//void __cdecl InitialiseSkeleton(__int16 itemNum);
//void __cdecl SkeletonControl(__int16 itemNum);

void __cdecl FourBladesControl(__int16 itemNum);

void __cdecl BirdBladeControl(__int16 itemNum);

void __cdecl CatwalkBlaldeControl(__int16 itemNum);

void __cdecl PlinthBladeControl(__int16 itemNum);

void __cdecl InitialiseSethBlade(__int16 itemNum);
void __cdecl SethBladeControl(__int16 itemNum);

void __cdecl ChainControl(__int16 itemNum);

void __cdecl PloughControl(__int16 itemNum);

void __cdecl CogControl(__int16 itemNum);

void __cdecl SpikeballControl(__int16 itemNum);

void __cdecl BarracudaControl(__int16 itemNum);

void __cdecl SharkControl(__int16 itemNum);

void __cdecl ControlSpikeWall(__int16 itemNum);

void __cdecl InitialiseSpinningBlade(__int16 item_number);
void __cdecl SpinningBlade(__int16 item_number);

void __cdecl InitialiseKillerStatue(__int16 item_number);
void __cdecl KillerStatueControl(__int16 item_number);

void __cdecl SpringBoardControl(__int16 item_number);

void __cdecl TigerControl(__int16 itemNum);

void __cdecl InitialiseCobra(__int16 itemNum);
void __cdecl CobraControl(__int16 itemNum);


