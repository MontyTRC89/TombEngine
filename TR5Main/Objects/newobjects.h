#pragma once
#include "global.h"

void ClampRotation(PHD_3DPOS *pos, short angle, short rot);

// TR1 objects
void InitialiseWolf(short itemNum);
void WolfControl(short itemNum);
void BearControl(short itemNum);
void Tr1RaptorControl(short itemNum);
void Tr1LarsonControl(short itemNum);
void Tr1PierreControl(short itemNum);
void ApeControl(short itemNum);
void InitialiseEvilLara(short itemNum);
void LaraEvilControl(short itemNum);
void DrawEvilLara(ITEM_INFO* item);
void NatlaControl(short itemNum);
void NatlaEvilControl(short itemNum);

// TR3 objects
void TigerControl(short itemNum);
void InitialiseCobra(short itemNum);
void CobraControl(short itemNum);
void Tr3RaptorControl(short itemNum);
void ShootHarpoon(ITEM_INFO* frogman, int x, int y, int z, short speed, short yRot, short roomNumber);
void HarpoonControl(short itemNum);
void ScubaControl(short itemNumber);
void InitialiseEagle(short itemNum);
void EagleControl(short itemNum);
void TribemanAxeControl(short itemNum);
void TribesmanShotDart(ITEM_INFO* item);
void TribesmanDartsControl(short itemNum);
void ControlSpikyWall(short itemNum);
void LaraTyrannosaurDeath(ITEM_INFO* item);
void TyrannosaurControl(short itemNum);
void TriggerFlamethrowerFlame(int x, int y, int z, int xv, int yv, int zv, int fxnum);
void TriggerPilotFlame(int itemnum);
short TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, short speed);
void FlameThrowerControl(short itemNumber);
void ControlSpikyCeiling(short itemNumber);
void InitialiseMonkey(short itemNumber);
void MonkeyControl(short itemNumber);
void MPGunControl(short itemNumber);
void InitialiseMPStick(short itemNumber);
void MPStickControl(short itemNumber);
void SetupShoal(int shoalNumber);
void SetupFish(int leader, ITEM_INFO* item);
void ControlFish(short itemNumber);
bool FishNearLara(PHD_3DPOS* pos, int distance, ITEM_INFO* item);
void InitialiseShiva(short itemNum);
void ShivaControl(short itemNum);
void ControlLaserBolts(short item_number);
void ControlLondBossPlasmaBall(short fx_number);
void InitialiseLondonBoss(short item_number);
void LondonBossControl(short item_number);
void S_DrawLondonBoss(ITEM_INFO* item);
void InitialiseCivvy(short item_number);
void CivvyControl(short item_number);

// TR4 object
void SarcophagusCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseLaraDouble(short itemNum);
void LaraDoubleControl(short itemNum);
void ScalesControl(short itemNum);
void ScalesCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
int RespawnAhmet(short itemNum);
void BubblesControl(short fxNum);
int BubblesShatterFunction(FX_INFO* fx, int param1, int param2);
void BubblesEffect1(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect2(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect3(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect4(short fxNum, short xVel, short yVel, short zVel);
