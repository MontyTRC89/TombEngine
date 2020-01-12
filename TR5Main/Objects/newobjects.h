#pragma once
#include "../Global/global.h"

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

typedef struct JEEP_INFO {
	short rot1;
	short rot2;
	short rot3;
	short rot4;
	int velocity;
	int revs;
	short engineRevs;
	short trackMesh;
	int jeepTurn;
	int fallSpeed;
	short momentumAngle;
	short extraRotation;
	short unknown0;
	int pitch;
	short flags;
	short unknown1;
	short unknown2;
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

typedef struct BOAT_INFO
{
	int boat_turn;
	int left_fallspeed;
	int right_fallspeed;
	int water;
	int pitch;
	short tilt_angle;
	short extra_rotation;
	short prop_rot;
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
	short attack_count;
	short death_count;
	byte attack_flag;
	byte attack_type;
	byte attack_head_count;
	byte ring_count;
	short explode_count;
	short lizman_item, lizman_room;
	short hp_counter;
	short dropped_icon;
	byte charged;
	byte dead;
	PHD_VECTOR	BeamTarget;
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
void DrawTony(ITEM_INFO* item);
void TonyFireBallControl(short fxNumber);
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
void InitialiseWildBoar(short itemNum);
void WildBoarControl(short itemNum);
void InitialiseSmallScorpion(short itemNum);
void SmallScorpionControl(short itemNum);
void InitialiseBat(short itemNum);
void BatControl(short itemNum);
void InitialiseBaddy(short itemNum);
void BaddyControl(short itemNum);
void InitialiseSas(short itemNum);
void SasControl(short itemNum);
void InitialiseMummy(short itemNum);
void MummyControl(short itemNum);
void InitialiseSkeleton(short itemNum);
void SkeletonControl(short itemNum);
void WakeUpSkeleton(ITEM_INFO* item);
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
void InitialiseKnightTemplar(short itemNum);
void KnightTemplarControl(short itemNum);
void StargateControl(short itemNum);
void StargateCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseSlicerDicer(short itemNum);
void SlicerDicerControl(short itemNum);
void BladeCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void SarcophagusCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseScorpion(short itemNum);
void ScorpionControl(short itemNum);
void InitialiseLaraDouble(short itemNum);
void LaraDoubleControl(short itemNum);
void InitialiseDemigod(short itemNum);
void DemigodControl(short itemNum);
void DemigodThrowEnergyAttack(PHD_3DPOS* pos, short roomNumber, int something);
void DemigodEnergyAttack(short itemNum);
void DemigodHammerAttack(int x, int y, int z, int something);
void InitialiseMine(short itemNum);
void MineControl(short itemNum);
void MineCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseSentryGun(short itemNum);
void SentryGunControl(short itemNum);
void SentryGunThrowFire(ITEM_INFO* item);
void InitialiseJeanYves(short itemNum);
void JeanYvesControl(short itemNum);
void ScalesControl(short itemNum);
void ScalesCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
int RespawnAhmet(short itemNum);
void InitialiseLittleBeetle(short itemNum);
void LittleBeetleControl(short itemNum);
void InitialiseTroops(short itemNum);
void TroopsControl(short itemNum);
void InitialiseHarpy(short itemNum);
void HarpyControl(short itemNum);
void HarpyBubbles(PHD_3DPOS* pos, short roomNumber, int count);
void HarpyAttack(ITEM_INFO* item, short itemNum);
void HarpySparks1(short itemNum, byte num, int size);
void HarpySparks2(int x, int y, int z, int xv, int yv, int zv);
void InitialiseGuide(short itemNum);
void GuideControl(short itemNum);
void InitialiseCrocodile(short itemNum);
void CrocodileControl(short itemNum);
void InitialiseSphinx(short itemNum);
void SphinxControl(short itemNum);
void InitialiseBurningFloor(short itemNum);
void BurningFloorControl(short itemNum);
void InitialiseHorse(short itemNum);
void InitialiseHorseman(short itemNum);
void HorsemanControl(short itemNum);
void HorsemanSparks(PHD_3DPOS* pos, int param1, int num);
void BubblesControl(short fxNum);
int BubblesShatterFunction(FX_INFO* fx, int param1, int param2);
void BubblesEffect1(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect2(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect3(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect4(short fxNum, short xVel, short yVel, short zVel);

/* Vehicles: */

// TODO: the boat is bugged, need to be fixed !
void InitialiseBoat(short itemNum);
void BoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
void BoatControl(short itemNumber);

void InitialiseSkidoo(short itemNum);
void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
int SkidooControl();
void DrawSkidoo(ITEM_INFO* item);
void DoSnowEffect(ITEM_INFO* skidoo);

void QuadbikeExplode(ITEM_INFO* item);
int CanQuadbikeGetOff(int direction);
int QuadCheckGetOff();
int GetOnQuadBike(short itemNumber, COLL_INFO* coll);
void QuadBaddieCollision(ITEM_INFO* quad);
int GetQuadCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
int TestQuadHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos);
int DoQuadShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old);
int DoQuadDynamics(int height, int fallspeed, int* y);
int QuadDynamics(ITEM_INFO* item);
void AnimateQuadBike(ITEM_INFO* item, int collide, int dead);
int QuadUserControl(ITEM_INFO* item, int height, int* pitch);
void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving);
void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int QuadBikeControl();

// TODO: the kayak is bugged, need to be fixed !
void InitialiseKayak(short item_number);
void DrawKayak(ITEM_INFO* kayak);
void KayakCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll);
int KayakControl();

/// Underwater Propeller
void SubInitialise(short itemNum);
void SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int SubControl();

void InitialiseMineCart(short itemNum);
void MineCartCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int MineCartControl();

int TestJeepHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos);
int DoJeepShift(ITEM_INFO* jeep, PHD_VECTOR* pos, PHD_VECTOR* old);
int DoJeepDynamics(int height, int speed, int* y, int flags);
int JeepCanGetOff();
void TriggerJeepExhaustSmoke(int x, int y, int z, short angle, short speed, int moving);
int JeepCheckGetOff();
int GetOnJeep(int itemNumber);
int GetJeepCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void JeepBaddieCollision(ITEM_INFO* jeep);
void JeepExplode(ITEM_INFO* item);
int JeepDynamics(ITEM_INFO* item);
int JeepUserControl(ITEM_INFO* item, int height, int* pitch);
void AnimateJeep(ITEM_INFO* item, int collide, int dead);
void InitialiseJeep(short itemNum);
int JeepControl();
void JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);

void InitialiseMotorbike(short itemNum);
int MotorbikeControl();
int MotorbikeCheckGetOff();
int MotorbikeDynamics(ITEM_INFO* item);
int MotorbikeUserControl(ITEM_INFO* item, int height, int* pitch);
void AnimateMotorbike(ITEM_INFO* item, int collide, int dead);
void MotorbikeExplode(ITEM_INFO* item);
int MotorbikeCanGetOff();
int DoMotorbikeDynamics(int height, int speed, int* y, int flags);
void TriggerMotorbikeExhaustSmoke(int x, int y, int z, short angle, short speed, int moving);
int GetOnMotorbike(int itemNumber);
void MotorbikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int GetOnMotorbike(int itemNumber);
int GetMotorbikeCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void MotorbikeBaddieCollision(ITEM_INFO* bike);