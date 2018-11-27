#pragma once

#include "..\Global\global.h"

typedef struct QUAD_INFO {
	__int32 velocity;
	__int16 frontRot;
	__int16 rearRot;
	__int32 revs;
	__int32 engineRevs;
	__int16 trackMesh;
	__int32 skidooTurn;
	__int32 leftFallspeed;
	__int32 rightFallspeed;
	__int16 momentumAngle;
	__int16 extraRotation;
	__int32 pitch;
	char flags;
};

void __cdecl ClampRotation(PHD_3DPOS *pos, __int16 angle, __int16 rot);

// TR1 objects
void __cdecl InitialiseWolf(__int16 itemNum);
void __cdecl WolfControl(__int16 itemNum);
void __cdecl BearControl(__int16 itemNum);
void __cdecl ApeControl(__int16 itemNum);

// TR2 objects
void __cdecl BarracudaControl(__int16 itemNum);
void __cdecl SharkControl(__int16 itemNum);
void __cdecl InitialiseSpinningBlade(__int16 itemNum);
void __cdecl SpinningBlade(__int16 itemNum);
void __cdecl InitialiseKillerStatue(__int16 itemNum);
void __cdecl KillerStatueControl(__int16 itemNum);
void __cdecl SpringBoardControl(__int16 itemNum);
void __cdecl RatControl(__int16 itemNum);

// TR3 objects
void __cdecl TigerControl(__int16 itemNum);
void __cdecl InitialiseCobra(__int16 itemNum);
void __cdecl CobraControl(__int16 itemNum);
void __cdecl RaptorControl(__int16 itemNum);
__int32 __cdecl GetWaterSurface(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
void __cdecl ShootHarpoon(ITEM_INFO* frogman, __int32 x, __int32 y, __int32 z, __int16 speed, __int16 yRot, __int16 roomNumber);
void __cdecl HarpoonControl(__int16 itemNum);
void __cdecl ScubaControl(__int16 itemNumber);
void InitialiseEagle(__int16 itemNum);
void EagleControl(__int16 itemNum);
void __cdecl TribemanAxeControl(__int16 itemNum);
void __cdecl TribesmanShotDart(ITEM_INFO* item);
void __cdecl TribesmanDartsControl(__int16 itemNum);
void __cdecl ControlSpikeWall(__int16 itemNum);
void LaraTyrannosaurDeath(ITEM_INFO* item);
void TyrannosaurControl(__int16 itemNum);

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

// TR4 object
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
void __cdecl InitialiseKnightTemplar(__int16 itemNum);
void __cdecl KnightTemplarControl(__int16 itemNum);
void __cdecl StargateControl(__int16 itemNum);
void __cdecl StargateCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSlicerDicer(__int16 itemNum);
void __cdecl SlicerDicerControl(__int16 itemNum);
void __cdecl BladeCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl SarcophagusCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseScorpion(__int16 itemNum);
void __cdecl ScorpionControl(__int16 itemNum);
void __cdecl InitialiseLaraDouble(__int16 itemNum);
void __cdecl LaraDoubleControl(__int16 itemNum);
void __cdecl InitialiseDemigod(__int16 itemNum);
void __cdecl DemigodControl(__int16 itemNum);
void __cdecl DemigodThrowEnergyAttack(PHD_3DPOS* pos, __int16 roomNumber, __int32 something);
void __cdecl DemigodEnergyAttack(__int16 itemNum);
void __cdecl DemigodHammerAttack(__int32 x, __int32 y, __int32 z, __int32 something);
void __cdecl InitialiseMine(__int16 itemNum);
void __cdecl MineControl(__int16 itemNum);
void __cdecl MineCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSentryGun(__int16 itemNum);
void __cdecl SentryGunControl(__int16 itemNum);
void __cdecl SentryGunEffect(ITEM_INFO* item);
void __cdecl InitialiseJeanYves(__int16 itemNum);
void __cdecl JeanYvesControl(__int16 itemNum);

//extern __int16 LaraMotorbike;
//void __cdecl InitialiseMotorbike(__int16 itemNum);


