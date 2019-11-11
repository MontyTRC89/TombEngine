#pragma once

#include "..\Global\global.h"

void __cdecl BaddyObjects(); //#define BaddyObjects ((void (__cdecl*)()) 0x004737C0)
void __cdecl ObjectObjects(); //#define ObjectObjects ((void (__cdecl*)()) 0x00476360)
void __cdecl TrapObjects(); //#define TrapObjects ((void (__cdecl*)()) 0x00475D40)
#define InitialiseHairs ((void (__cdecl*)()) 0x00438BE0)
#define InitialiseSpecialEffects ((void (__cdecl*)()) 0x0043D8B0)

void __cdecl CustomObjects();
void __cdecl InitialiseObjects();

void Inject_Setup();

/* BaddyObjects() */

#define DrawBaddieGunFlash ((void (__cdecl*)(ITEM_INFO*)) 0x00466880)
#define InitialiseLaraLoad ((void (__cdecl*)(__int16)) 0x004568C0)
#define InitialiseGuard ((void (__cdecl*)(__int16)) 0x0043F420)
#define ControlGuard ((void (__cdecl*)(__int16)) 0x0046F5E0)
#define InitialiseSubmarine ((void (__cdecl*)(__int16)) 0x0045D360)
#define ControlSubmarine ((void (__cdecl*)(__int16)) 0x0045D3F0)
#define InitialiseLion ((void (__cdecl*)(__int16)) 0x0045AC80)
#define LionControl ((void (__cdecl*)(__int16)) 0x0045AD00)
#define InitialiseDoberman ((void (__cdecl*)(__int16)) 0x00428940)
#define ControlDoberman ((void (__cdecl*)(__int16)) 0x00428A10)
#define InitialiseDog ((void (__cdecl*)(__int16)) 0x0043B670)
#define ControlDog ((void (__cdecl*)(__int16)) 0x0043B730)
#define InitialiseReaper ((void (__cdecl*)(__int16)) 0x0045DA70)
#define ControlReaper ((void (__cdecl*)(__int16)) 0x0045DAF0)
#define InitialiseArmedBaddy2 ((void (__cdecl*)(__int16)) 0x0045B7B0)
#define ControlArmedBaddy2 ((void (__cdecl*)(__int16)) 0x0045B840)
#define InitialiseLarson ((void (__cdecl*)(__int16)) 0x00469F70)
#define ControlLarson ((void (__cdecl*)(__int16)) 0x0046A080)
#define InitialiseCyborg ((void (__cdecl*)(__int16)) 0x0043A2C0)
#define ControlCyborg ((void (__cdecl*)(__int16)) 0x0043A340)
#define InitialiseGuardM16 ((void (__cdecl*)(__int16)) 0x00478180)
#define ControlGuardM16 ((void (__cdecl*)(__int16)) 0x00478250)
#define InitialiseChef ((void (__cdecl*)(__int16)) 0x00410990)
#define ControlChef ((void (__cdecl*)(__int16)) 0x00410A60)
#define InitialiseGuardLaser ((void (__cdecl*)(__int16)) 0x0048CD40)
#define ControlGuardLaser ((void (__cdecl*)(__int16)) 0x0048CDD0)
#define InitialiseSmallDragon ((void (__cdecl*)(__int16)) 0x0043BEA0)
#define ControlSmallDragon ((void (__cdecl*)(__int16)) 0x0043BF70)
#define InitialiseImp ((void (__cdecl*)(__int16)) 0x0043CCA0)
#define ControlImp ((void (__cdecl*)(__int16)) 0x0043BEA0)
#define InitialiseLightingGuide ((void (__cdecl*)(__int16)) 0x0048E500)
#define ControlLightingGuide ((void (__cdecl*)(__int16)) 0x0048E580)
#define InitialiseBrownBeast ((void (__cdecl*)(__int16)) 0x0048E8E0)
#define ControlBrowsBeast ((void (__cdecl*)(__int16)) 0x0048E960)
#define InitialiseLagoonWitch ((void (__cdecl*)(__int16)) 0x0047D2D0)
#define ControlLagoonWitch ((void (__cdecl*)(__int16)) 0x0047D360)
#define InitialiseInvisibleGhost ((void (__cdecl*)(__int16)) 0x00477A20)
#define ControlInvisibleGhost ((void (__cdecl*)(__int16)) 0x00477AB0)
#define InitialiseLittleRats ((void (__cdecl*)(__int16)) 0x0046B220)
#define ControlLittleRats ((void (__cdecl*)(__int16)) 0x0046AB30)
#define InitialiseLittleBats ((void (__cdecl*)(__int16)) 0x00407EC0)
#define ControlLittleBats ((void (__cdecl*)(__int16)) 0x00407F50)
#define InitialiseSpiders ((void (__cdecl*)(__int16)) 0x0043F2B0)
#define ControlSpiders ((void (__cdecl*)(__int16)) 0x0047A200)
#define InitialiseGladiator ((void (__cdecl*)(__int16)) 0x00436670)
#define ControlGladiator ((void (__cdecl*)(__int16)) 0x00436700)
#define InitialiseRomanStatue ((void (__cdecl*)(__int16)) 0x0046BB00)
#define ControlRomanStatue ((void (__cdecl*)(__int16)) 0x0046BC10)
#define InitialiseLaserHead ((void (__cdecl*)(__int16)) 0x00436FE0)
#define ControlLaserHead ((void (__cdecl*)(__int16)) 0x00437680)
#define InitialiseAutoGuns ((void (__cdecl*)(__int16)) 0x0043F8B0)
#define ControlAutoGuns ((void (__cdecl*)(__int16)) 0x004078A0)
#define ControlGunShip ((void (__cdecl*)(__int16)) 0x00487FF0)

/* TrapObjects */

#define ElectricityWiresControl ((void (__cdecl*)(__int16)) 0x00442610)
#define InitialiseRomeHammer ((void (__cdecl*)(__int16)) 0x0043ECB0)
#define InitialiseDeathSlide ((void (__cdecl*)(__int16)) 0x0041CC70)
#define DeathSlideCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0041CCE0)
#define DeathSlideControl ((void (__cdecl*)(__int16)) 0x0041CE00)
#define RollingBallCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0048B6D0)
#define RollingBallControl ((void (__cdecl*)(__int16)) 0x0048AE60)
#define InitialiseTwoBlocksPlatform ((void (__cdecl*)(__int16)) 0x0043D5D0)
#define TwoBlocksPlatformControl ((void (__cdecl*)(__int16)) 0x0048BBB0)
#define TwoBlocksPlatformFloor ((void (__cdecl*)(ITEM_INFO*,__int32,__int32,__int32,int*)) 0x0048B9E0)
#define TwoBlocksPlatformCeiling ((void (__cdecl*)(ITEM_INFO*,__int32,__int32,__int32,int*)) 0x0048BA50)
#define KillAllTriggersControl ((void (__cdecl*)(__int16)) 0x00431030)
#define FallingCeilingCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x004127C0)
#define FallingCeilingControl ((void (__cdecl*)(__int16)) 0x004899D0)
#define InitialiseFallingBlock ((void (__cdecl*)(__int16)) 0x0043D330)
#define FallingBlockCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00489750)
#define FallingBlockControl ((void (__cdecl*)(__int16)) 0x00489820)
#define FallingBlockFloor ((void (__cdecl*)(ITEM_INFO*,__int32,__int32,__int32,int*)) 0x00489910)
#define FallingBlockCeiling ((void (__cdecl*)(ITEM_INFO*,__int32,__int32,__int32,int*)) 0x00489980)
#define InitialisePushableBlock ((void (__cdecl*)(__int16)) 0x0045E720)
#define PushableBlockControl ((void (__cdecl*)(__int16)) 0x0045EA30)
#define PushableBlockCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0045F570)
#define DartEmitterControl ((void (__cdecl*)(__int16)) 0x00489B30)
#define DrawDart ((void (__cdecl*)(ITEM_INFO*)) 0x004CBB10)
#define DartControl ((void (__cdecl*)(__int16)) 0x00489D60)
#define InitialiseFlameEmitter ((void (__cdecl*)(__int16)) 0x0043D370)
#define FlameEmitterCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00433B40)
#define FlameEmitterControl ((void (__cdecl*)(__int16)) 0x00489F70)
#define InitialiseFlameEmitter2 ((void (__cdecl*)(__int16)) 0x0043D4E0)
#define FlameEmitter2Control ((void (__cdecl*)(__int16)) 0x0048A3B0)
#define FlameEmitter3Control ((void (__cdecl*)(__int16)) 0x0048A570)
#define FlameControl ((void (__cdecl*)(__int16)) 0x0048AB80)
#define InitialiseRopeTrap ((void (__cdecl*)()) 0x0046EE40)
#define GenSlot1Control ((void (__cdecl*)(__int16)) 0x00406580)
#define InitialiseGenSlot2 ((void (__cdecl*)(__int16)) 0x0043FD70)
#define GenSlot2Control ((void (__cdecl*)(__int16)) 0x00488710)
#define DrawGenSlot2 ((void (__cdecl*)(ITEM_INFO*)) 0x004CFF80)
#define InitialiseGenSlot3 ((void (__cdecl*)(__int16)) 0x004402E0)
#define InitialiseGenSlot4 ((void (__cdecl*)(__int16)) 0x00440440)
#define GenSlot4Control ((void (__cdecl*)(__int16)) 0x00486450)
#define InitialiseHighObject1 ((void (__cdecl*)(__int16)) 0x0043FC30)
#define HighObject1Control ((void (__cdecl*)(__int16)) 0x004067E0)
#define InitialisePortal ((void (__cdecl*)(__int16)) 0x0043FAA0)
#define PortalControl ((void (__cdecl*)(__int16)) 0x00401AEB)
#define DrawPortal ((void (__cdecl*)(ITEM_INFO*)) 0x004CFF80)
#define InitialiseRope ((void (__cdecl*)(__int16)) 0x0046F060)
#define RopeControl ((void (__cdecl*)(__int16)) 0x0046DD40)
#define RopeCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0046DAE0)
#define InitialiseWreckingBall ((void (__cdecl*)(__int16)) 0x0043EF20)
#define WreckingBallCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00441D50)
#define WreckingBallControl ((void (__cdecl*)(__int16)) 0x00441410)
#define DrawWreckingBall ((void (__cdecl*)(ITEM_INFO*)) 0x00441F50)
#define InitialiseVentilator ((void (__cdecl*)(__int16)) 0x0043F3D0)
#define VentilatorControl ((void (__cdecl*)(__int16)) 0x00405610)
#define InitialiseRaisingCog ((void (__cdecl*)(__int16)) 0x00440320)
#define RaisingCogControl ((void (__cdecl*)(__int16)) 0x00406040)

/* ObjectObjects */

#define InitialiseSmashObject ((void (__cdecl*)(__int16)) 0x0043D7F0)
#define InitialiseSwitch ((void (__cdecl*)(__int16)) 0x00440070)
#define SequenceControl ((void (__cdecl*)(__int16)) 0x0047F520)
#define SequenceCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0047F610)
#define LeverSwitchCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x0047EE00)
#define InitialisePulleySwitch ((void (__cdecl*)(__int16)) 0x0043E1F0)
#define InitialiseCrowDoveSwitch ((void (__cdecl*)(__int16)) 0x0043ECF0)
#define InitialiseDoor ((void (__cdecl*)(__int16)) 0x0043DB60)
#define DrawLiftDoor ((void (__cdecl*)(ITEM_INFO*)) 0x0045AAF0)
#define DoubleDoorControl ((void (__cdecl*)(__int16)) 0x00429840)
#define InitialiseTrapDoor ((void (__cdecl*)(__int16)) 0x0043D2F0)
#define TrapDoorFloorCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x004891F0)
#define TrapDoorCeilingCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00489450)
#define TrapDoorNormalCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x004896D0)
#define TrapDoorControl ((void (__cdecl*)(__int16)) 0x00488FA0)
#define InitialiseCupboard ((void (__cdecl*)(__int16)) 0x0043EDB0)
#define CupboardCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x004699A0)
#define CupboardControl ((void (__cdecl*)(__int16)) 0x00469660)
#define FlareControl ((void (__cdecl*)(__int16)) 0x00455460)
#define DrawFlare ((void (__cdecl*)(ITEM_INFO*)) 0x00454A90)
#define TorchControl ((void (__cdecl*)(__int16)) 0x00434390)
#define ChaffControl ((void (__cdecl*)(__int16)) 0x0045CFB0)
#define TorpedoControl ((void (__cdecl*)(__int16)) 0x0045C9F0)
#define ControlCrossbowOrGrapplingBolt ((void (__cdecl*)(__int16)) 0x0044E8B0)
#define DrawCrossbowOrGrapplingBolt ((void (__cdecl*)(ITEM_INFO*)) 0x004852E0)
#define KeyHoleCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00468930)
#define PuzzleDoneCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00468C00)
#define InitialiseAnimating ((void (__cdecl*)(__int16)) 0x00440100)
#define InitialiseTightRope ((void (__cdecl*)(__int16)) 0x0043ED30)
#define InitialiseSteelDoor ((void (__cdecl*)(__int16)) 0x0043F180)
#define SteelDoorControl ((void (__cdecl*)(__int16)) 0x00486BE0)
#define SteelDoorCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00487AD0)
#define HeadSetTalkingPointControl ((void (__cdecl*)(__int16)) 0x0048E3C0)
#define InitialiseXRayMachine ((void (__cdecl*)(__int16)) 0x0043FA20)
#define InitialiseGrapplingGunTarget ((void (__cdecl*)(__int16)) 0x0043F270)
#define InitialiseGrapplingGunSwitch ((void (__cdecl*)(__int16)) 0x0043FBC0)
#define GrapplingGunTargetCollision ((void (__cdecl*)(__int16,ITEM_INFO*,COLL_INFO*)) 0x00412770)
#define InitialiseTeethSpike ((void (__cdecl*)(__int16)) 0x0043FBC0)
#define TeethSpikeControl ((void (__cdecl*)(__int16)) 0x0043FBC0)
#define DrawScaledSpike ((void (__cdecl*)(ITEM_INFO*)) 0x0043FBC0)
#define InitialiseRaisingBlock  ((void (__cdecl*)(__int16)) 0x0043D730)
#define RaisingBlockControl ((void (__cdecl*)(__int16)) 0x0048C3D0)
#define InitialiseSmokeEmitter ((void (__cdecl*)(__int16)) 0x0043D9D0)
#define SmokeEmitterControl ((void (__cdecl*)(__int16)) 0x00431560)
#define DrawLensFlare ((void (__cdecl*)(ITEM_INFO*)) 0x00485290)
#define ControlWaterfallMist ((void (__cdecl*)(__int16)) 0x00432CA0)
#define ControlEnemyMissile ((void (__cdecl*)(__int16)) 0x00431E70)
#define HighObject2Control ((void (__cdecl*)(__int16)) 0x004070D0)

#define INIT_PICKUP(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialisePickup; \
	obj->collision = PickupCollision; \
	obj->control = PickupControl; \
}

#define INIT_KEYHOLE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = KeyHoleCollision; \
}

#define INIT_PUZZLEHOLE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleHoleCollision; \
	obj->control = AnimatingControl; \
}

#define INIT_PUZZLEDONE(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->collision = PuzzleDoneCollision; \
	obj->control = AnimatingControl; \
}

// normally INIT_ANIMATING have:
//Bones[obj->boneIndex] |= ROT_Y;
//Bones[obj->boneIndex + 4] |= ROT_X;

#define INIT_ANIMATING(obj_id) \
obj = &Objects[obj_id]; \
if (obj->loaded) \
{ \
	obj->initialise = InitialiseAnimating; \
	obj->control = AnimatingControl; \
	obj->collision = ObjectCollision; \
}