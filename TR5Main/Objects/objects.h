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

//void __cdecl InitialiseMummy(__int16 itemNum);
//void __cdecl MummyControl(__int16 itemNum);

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

