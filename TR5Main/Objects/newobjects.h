#pragma once
#include "global.h"

typedef struct QUAD_INFO {
	int velocity;
	short frontRot;
	short rearRot;
	int revs;
	int engineRevs;
	short trackMesh;
	int skidooTurn;
	int leftFallspeed;
	int rightFallspeed;
	short momentumAngle;
	short extraRotation;
	int pitch;
	char flags;
};



struct MOTORBIKE_INFO
{
	int wheelRight;  // (two wheel: front and back)
	int wheelLeft;   // (one wheel: left)
	int velocity;
	int revs;
	int engineRevs;
	short momentumAngle;
	short extraRotation;
	short wallShiftRotation;
	int bikeTurn;
	int pitch;
	short flags;
	short lightPower;
};

typedef struct SUB_INFO {
	int Vel;
	int Rot;
	int RotX;
	short FanRot;
	char Flags;
	char WeaponTimer;
};

typedef struct CART_INFO {
	int Speed;
	int MidPos;
	int FrontPos;
	int TurnX;
	int TurnZ;
	short TurnLen;
	short TurnRot;
	short YVel;
	short Gradient;
	char Flags;
	char StopDelay;
};

typedef struct FISH_INFO
{
	short x;
	short y;
	short z;
	int	angle;
	short destY;
	short angAdd;
	byte speed;
	byte acc;
	byte swim;
};

typedef struct FISH_LEADER_INFO
{
	short	angle;
	byte speed;
	byte on;
	short	angleTime;
	short	speedTime;
	short	xRange, yRange, zRange;
};



typedef struct SKIDOO_INFO
{
	short track_mesh;
	int skidoo_turn;
	int left_fallspeed, right_fallspeed;
	short momentum_angle, extra_rotation;
	int pitch;
	bool already_cd_played;
	bool armed;
	int flash_timer;
};

typedef struct KAYAK_INFO {
	int Vel;
	int Rot;
	int FallSpeedF;
	int FallSpeedL;
	int FallSpeedR;
	int Water;
	PHD_3DPOS OldPos;
	char Turn;
	char Forward;
	char TrueWater;
	char Flags;
};

typedef struct BOSS_STRUCT
{
	PHD_VECTOR BeamTarget;
	bool DroppedIcon;
	bool IsInvincible;
	bool DrawExplode; // allow explosion geometry
	bool Charged;
	bool Dead;
	short AttackCount;
	short DeathCount;
	short AttackFlag;
	short AttackType;
	short AttackHeadCount;
	short RingCount;
	short ExplodeCount;
	short LizmanItem, LizmanRoom;
	short HpCounter;
};

typedef struct SHIELD_POINTS
{
	short x;
	short y;
	short z;
	byte rsub;
	byte gsub;
	byte bsub;
	byte pad[3];
	long rgb;
};

typedef struct EXPLOSION_VERTS
{
	short x;
	short z;
	long rgb;
};

typedef struct EXPLOSION_RING
{
	short on;
	short life;   // 0 - 32.
	short speed;
	short radius; // Width is 1/4 of radius.
	short xrot;
	short zrot;
	int x;
	int y;
	int z;
	EXPLOSION_VERTS	verts[16];
};

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

// TR2 objects
void BarracudaControl(short itemNum);
void SharkControl(short itemNum);
void InitialiseSpinningBlade(short itemNum);
void SpinningBlade(short itemNum);
void InitialiseKillerStatue(short itemNum);
void KillerStatueControl(short itemNum);
void SpringBoardControl(short itemNum);
void RatControl(short itemNum);
void SilencerControl(short itemNum);
void InitialiseYeti(short itemNum);
void YetiControl(short itemNum);
void WorkerShotgunControl(short itemNum);
void InitialiseWorkerShotgun(short itemNum);
void WorkerMachineGunControl(short itemNum);
void InitialiseWorkerMachineGun(short itemNum);
void SmallSpiderControl(short itemNum);
void BigSpiderControl(short itemNum);
void WorkerDualGunControl(short itemNum);
void BirdMonsterControl(short itemNum);
void WorkerFlamethrower(short itemNum);
void InitialiseWorkerFlamethrower(short itemNum);
void KnifethrowerControl(short itemNum);
void KnifeControl(short fxNum);
void MercenaryUziControl(short itemNum);
void MercenaryAutoPistolControl(short itemNum);
void MonkControl(short itemNum);
void DrawStatue(ITEM_INFO* item); // compatible with all statue :)
void InitialiseSwordGuardian(short itemNum);
void SwordGuardianFly(ITEM_INFO* item); // only effect to fly
void SwordGuardianControl(short itemNum);
void InitialiseSpearGuardian(short itemNum);
void SpearGuardianControl(short itemNum);
void DragonCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void DragonControl(short backNum);
void InitialiseBartoli(short itemNum);
void BartoliControl(short itemNum);
void InitialiseSkidman(short itemNum);
void SkidManCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void SkidManControl(short riderNum);

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
void InitialiseTony(short itemNum);
void TonyControl(short itemNum);
void S_DrawTonyBoss(ITEM_INFO* item);
void ControlTonyFireBall(short fxNumber);
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
void FourBladesControl(short itemNum);
void BirdBladeControl(short itemNum);
void CatwalkBlaldeControl(short itemNum);
void PlinthBladeControl(short itemNum);
void InitialiseSethBlade(short itemNum);
void SethBladeControl(short itemNum);
void ChainControl(short itemNum);
void PloughControl(short itemNum);
void CogControl(short itemNum);
void SpikeballControl(short itemNum);
void StargateControl(short itemNum);
void StargateCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseSlicerDicer(short itemNum);
void SlicerDicerControl(short itemNum);
void BladeCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void SarcophagusCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseLaraDouble(short itemNum);
void LaraDoubleControl(short itemNum);
void InitialiseMine(short itemNum);
void MineControl(short itemNum);
void MineCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void ScalesControl(short itemNum);
void ScalesCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
int RespawnAhmet(short itemNum);
void InitialiseBurningFloor(short itemNum);
void BurningFloorControl(short itemNum);
void BubblesControl(short fxNum);
int BubblesShatterFunction(FX_INFO* fx, int param1, int param2);
void BubblesEffect1(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect2(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect3(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect4(short fxNum, short xVel, short yVel, short zVel);
