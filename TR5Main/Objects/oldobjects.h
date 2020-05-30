#pragma once
#include "collide.h"
#include "effect.h"

void InitialiseRaisingBlock(short itemNumber);
void ControlRaisingBlock(short itemNumber);
void InitialiseTeethSpikes(short itemNumber);
int CollidedWithTeethSpikes(ITEM_INFO* item);
void ControlTeethSpikes(short itemNumber);
void PulseLightControl(short itemNumber);
void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int rot, short roomNumber, short falloff);
void StrobeLightControl(short itemNumber);
void ColorLightControl(short itemNumber);
void ElectricalLightControl(short itemNumber);
void BlinkingLightControl(short itemNumber);
void TriggerElectricityWiresSparks(int x, int z, char objNum, char node, int flags);
void TriggerLaraElectricitySparks(int flame);
int ElectricityWireCheckDeadlyBounds(PHD_VECTOR* pos, short delta);
void ElectricityWiresControl(short itemNumber);
void InitialiseSmokeEmitter(short itemNumber);
void SmokeEmitterControl(short itemNumber);
void InitialiseTeleporter(short itemNumber);
void ControlTeleporter(short itemNumber);
void InitialiseHighObject1(short itemNumber);
void ControlHighObject1(short itemNumber);
void VentilatorEffect(short* bounds, int intensity, short rot, int speed);
void InitialiseVentilator(short itemNumber);
void VentilatorControl(short itemNumber);
void GenSlot1Control(short itemNumber);
void InitialiseGenSlot3(short itemNumber);
void DartControl(short itemNumber);
void DartEmitterControl(short itemNumber);
void FallingCeilingControl(short itemNumber);
void RollingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void RollingBallControl(short itemNumber);
void DeathSlideCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void ControlDeathSlide(short itemNumber);
void InitialiseDeathSlide(short itemNumber);
void ChaffFlareControl(short itemNumber);
void MissileControl(short itemNumber);
void ControlBodyPart(short fxNumber);
void ExplodeFX(FX_INFO* fx, int noXZVel, int bits);
void InitialiseRomeHammer(short itemNumber);
