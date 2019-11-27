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

void __cdecl ClampRotation(PHD_3DPOS *pos, short angle, short rot);

// TR1 objects
void __cdecl InitialiseWolf(short itemNum);
void __cdecl WolfControl(short itemNum);
void __cdecl BearControl(short itemNum);
void __cdecl ApeControl(short itemNum);

// TR2 objects
void __cdecl BarracudaControl(short itemNum);
void __cdecl SharkControl(short itemNum);
void __cdecl InitialiseSpinningBlade(short itemNum);
void __cdecl SpinningBlade(short itemNum);
void __cdecl InitialiseKillerStatue(short itemNum);
void __cdecl KillerStatueControl(short itemNum);
void __cdecl SpringBoardControl(short itemNum);
void __cdecl RatControl(short itemNum);
void __cdecl SilencerControl(short itemNum);
void __cdecl InitialiseYeti(short itemNum);
void __cdecl YetiControl(short itemNum);
void __cdecl WorkerShotgunControl(short itemNum);
void __cdecl InitialiseWorkerShotgun(short itemNum);
void __cdecl WorkerMachineGunControl(short itemNum);
void __cdecl InitialiseWorkerMachineGun(short itemNum);
void __cdecl SmallSpiderControl(short itemNum);
void __cdecl BigSpiderControl(short itemNum);
void __cdecl WorkerDualGunControl(short itemNum);
void __cdecl BirdMonsterControl(short itemNum);
void __cdecl WorkerFlamethrower(short itemNum);
void __cdecl InitialiseWorkerFlamethrower(short itemNum);
void __cdecl KnifethrowerControl(short itemNum);
void __cdecl KnifeControl(short fxNum);
void __cdecl MercenaryUziControl(short itemNum);
void __cdecl MercenaryAutoPistolControl(short itemNum);
void __cdecl MonkControl(short itemNum);
void __cdecl DrawStatue(ITEM_INFO* item); // compatible with all statue :)
void __cdecl InitialiseSwordGuardian(short itemNum);
void __cdecl SwordGuardianFly(ITEM_INFO* item); // only effect to fly
void __cdecl SwordGuardianControl(short itemNum);
void __cdecl InitialiseSpearGuardian(short itemNum);
void __cdecl SpearGuardianControl(short itemNum);
void __cdecl DragonCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void __cdecl DragonControl(short backNum);
void __cdecl InitialiseBartoli(short itemNum);
void __cdecl BartoliControl(short itemNum);
void __cdecl InitialiseSkidman(short itemNum);
void __cdecl SkidManCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void __cdecl SkidManControl(short riderNum);

// TR3 objects
void __cdecl TigerControl(short itemNum);
void __cdecl InitialiseCobra(short itemNum);
void __cdecl CobraControl(short itemNum);
void __cdecl RaptorControl(short itemNum);
int __cdecl GetWaterSurface(int x, int y, int z, short roomNumber);
void __cdecl ShootHarpoon(ITEM_INFO* frogman, int x, int y, int z, short speed, short yRot, short roomNumber);
void __cdecl HarpoonControl(short itemNum);
void __cdecl ScubaControl(short itemNumber);
void __cdecl InitialiseEagle(short itemNum);
void __cdecl EagleControl(short itemNum);
void __cdecl TribemanAxeControl(short itemNum);
void __cdecl TribesmanShotDart(ITEM_INFO* item);
void __cdecl TribesmanDartsControl(short itemNum);
void __cdecl ControlSpikyWall(short itemNum);
void __cdecl LaraTyrannosaurDeath(ITEM_INFO* item);
void __cdecl TyrannosaurControl(short itemNum);
void __cdecl TriggerFlamethrowerFlame(int x, int y, int z, int xv, int yv, int zv, int fxnum);
void __cdecl TriggerPilotFlame(int itemnum);
short __cdecl TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, short speed);
void __cdecl FlameThrowerControl(short itemNumber);
void __cdecl ControlSpikyCeiling(short itemNumber);
void __cdecl InitialiseMonkey(short itemNumber);
void __cdecl MonkeyControl(short itemNumber);
void __cdecl MPGunControl(short itemNumber);
void InitialiseMPStick(short itemNumber);
void MPStickControl(short itemNumber);
void __cdecl SetupShoal(int shoalNumber);
void __cdecl SetupFish(int leader, ITEM_INFO* item);
void ControlFish(short itemNumber);
bool FishNearLara(PHD_3DPOS* pos, int distance, ITEM_INFO* item);
void __cdecl InitialiseTony(short itemNum);
void __cdecl TonyControl(short itemNum);
void __cdecl DrawTony(ITEM_INFO* item);
void __cdecl TonyFireBallControl(short fxNumber);
void __cdecl InitialiseShiva(short itemNum);
void __cdecl ShivaControl(short itemNum);

// TR4 object
void __cdecl InitialiseWildBoar(short itemNum);
void __cdecl WildBoarControl(short itemNum);
void __cdecl InitialiseSmallScorpion(short itemNum);
void __cdecl SmallScorpionControl(short itemNum);
void __cdecl InitialiseBat(short itemNum);
void __cdecl BatControl(short itemNum);
void __cdecl InitialiseBaddy(short itemNum);
void __cdecl BaddyControl(short itemNum);
void __cdecl InitialiseSas(short itemNum);
void __cdecl SasControl(short itemNum);
void __cdecl InitialiseMummy(short itemNum);
void __cdecl MummyControl(short itemNum);
void __cdecl InitialiseSkeleton(short itemNum);
void __cdecl SkeletonControl(short itemNum);
void __cdecl WakeUpSkeleton(ITEM_INFO* item);
void __cdecl FourBladesControl(short itemNum);
void __cdecl BirdBladeControl(short itemNum);
void __cdecl CatwalkBlaldeControl(short itemNum);
void __cdecl PlinthBladeControl(short itemNum);
void __cdecl InitialiseSethBlade(short itemNum);
void __cdecl SethBladeControl(short itemNum);
void __cdecl ChainControl(short itemNum);
void __cdecl PloughControl(short itemNum);
void __cdecl CogControl(short itemNum);
void __cdecl SpikeballControl(short itemNum);
void __cdecl InitialiseKnightTemplar(short itemNum);
void __cdecl KnightTemplarControl(short itemNum);
void __cdecl StargateControl(short itemNum);
void __cdecl StargateCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSlicerDicer(short itemNum);
void __cdecl SlicerDicerControl(short itemNum);
void __cdecl BladeCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl SarcophagusCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseScorpion(short itemNum);
void __cdecl ScorpionControl(short itemNum);
void __cdecl InitialiseLaraDouble(short itemNum);
void __cdecl LaraDoubleControl(short itemNum);
void __cdecl InitialiseDemigod(short itemNum);
void __cdecl DemigodControl(short itemNum);
void __cdecl DemigodThrowEnergyAttack(PHD_3DPOS* pos, short roomNumber, int something);
void __cdecl DemigodEnergyAttack(short itemNum);
void __cdecl DemigodHammerAttack(int x, int y, int z, int something);
void __cdecl InitialiseMine(short itemNum);
void __cdecl MineControl(short itemNum);
void __cdecl MineCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSentryGun(short itemNum);
void __cdecl SentryGunControl(short itemNum);
void __cdecl SentryGunThrowFire(ITEM_INFO* item);
void __cdecl InitialiseJeanYves(short itemNum);
void __cdecl JeanYvesControl(short itemNum);
void __cdecl ScalesControl(short itemNum);
void __cdecl ScalesCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
int __cdecl RespawnAhmet(short itemNum);
void __cdecl InitialiseLittleBeetle(short itemNum);
void __cdecl LittleBeetleControl(short itemNum);
void __cdecl InitialiseTroops(short itemNum);
void __cdecl TroopsControl(short itemNum);
void __cdecl InitialiseHarpy(short itemNum);
void __cdecl HarpyControl(short itemNum);
void __cdecl HarpyBubbles(PHD_3DPOS* pos, short roomNumber, int count);
void __cdecl HarpyAttack(ITEM_INFO* item, short itemNum);
void __cdecl HarpySparks1(short itemNum, byte num, int size);
void __cdecl HarpySparks2(int x, int y, int z, int xv, int yv, int zv);
void __cdecl InitialiseGuide(short itemNum);
void __cdecl GuideControl(short itemNum);
void __cdecl InitialiseCrocodile(short itemNum);
void __cdecl CrocodileControl(short itemNum);
void __cdecl InitialiseSphinx(short itemNum);
void __cdecl SphinxControl(short itemNum);
void __cdecl InitialiseBurningFloor(short itemNum);
void __cdecl BurningFloorControl(short itemNum);
void __cdecl InitialiseHorse(short itemNum);
void __cdecl InitialiseHorseman(short itemNum);
void __cdecl HorsemanControl(short itemNum);
void __cdecl HorsemanSparks(PHD_3DPOS* pos, int param1, int num);
void __cdecl BubblesControl(short fxNum);
int __cdecl BubblesShatterFunction(FX_INFO* fx, int param1, int param2);
void __cdecl BubblesEffect1(short fxNum, short xVel, short yVel, short zVel);
void __cdecl BubblesEffect2(short fxNum, short xVel, short yVel, short zVel);
void __cdecl BubblesEffect3(short fxNum, short xVel, short yVel, short zVel);
void __cdecl BubblesEffect4(short fxNum, short xVel, short yVel, short zVel);

/* Vehicles: */

// TODO: the boat is bugged, need to be fixed !
void __cdecl InitialiseBoat(short itemNum);
void __cdecl BoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
void __cdecl BoatControl(short itemNumber);

void __cdecl InitialiseSkidoo(short itemNum);
void __cdecl SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
int __cdecl SkidooControl();
void __cdecl DrawSkidoo(ITEM_INFO* item);

void __cdecl QuadbikeExplode(ITEM_INFO* item);
int __cdecl CanQuadbikeGetOff(int direction);
int __cdecl QuadCheckGetOff();
int GetOnQuadBike(short itemNumber, COLL_INFO* coll);
void __cdecl QuadBaddieCollision(ITEM_INFO* quad);
int __cdecl GetQuadCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
int __cdecl TestQuadHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos);
int __cdecl DoQuadShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old);
int __cdecl DoQuadDynamics(int height, int fallspeed, int* y);
int __cdecl QuadDynamics(ITEM_INFO* item);
void __cdecl AnimateQuadBike(ITEM_INFO* item, int collide, int dead);
int __cdecl QuadUserControl(ITEM_INFO* item, int height, int* pitch);
void __cdecl TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving);
void __cdecl InitialiseQuadBike(short itemNumber);
void __cdecl QuadBikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int __cdecl QuadBikeControl();

// TODO: the kayak is bugged, need to be fixed !
void __cdecl InitialiseKayak(short item_number);
void __cdecl DrawKayak(ITEM_INFO* kayak);
void __cdecl KayakCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll);
int __cdecl KayakControl();

/// Underwater Propeller
void __cdecl SubInitialise(short itemNum);
void __cdecl SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int __cdecl SubControl();
void __cdecl SubEffects(short item_number);

int __cdecl TestJeepHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos);
int __cdecl DoJeepShift(ITEM_INFO* jeep, PHD_VECTOR* pos, PHD_VECTOR* old);
int __cdecl DoJeepDynamics(int height, int speed, int* y, int flags);
int __cdecl JeepCanGetOff();
void __cdecl TriggerJeepExhaustSmoke(int x, int y, int z, short angle, short speed, int moving);
int __cdecl JeepCheckGetOff();
int __cdecl GetOnJeep(int itemNumber);
int __cdecl GetJeepCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void __cdecl JeepBaddieCollision(ITEM_INFO* jeep);
void __cdecl JeepExplode(ITEM_INFO* item);
int __cdecl JeepDynamics(ITEM_INFO* item);
int __cdecl JeepUserControl(ITEM_INFO* item, int height, int* pitch);
void __cdecl AnimateJeep(ITEM_INFO* item, int collide, int dead);
void __cdecl InitialiseJeep(short itemNum);
int __cdecl JeepControl();
void __cdecl JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);

void __cdecl InitialiseMotorbike(short itemNum);
int __cdecl MotorbikeControl();
int __cdecl MotorbikeCheckGetOff();
int __cdecl MotorbikeDynamics(ITEM_INFO* item);
int __cdecl MotorbikeUserControl(ITEM_INFO* item, int height, int* pitch);
void __cdecl AnimateMotorbike(ITEM_INFO* item, int collide, int dead);
void __cdecl MotorbikeExplode(ITEM_INFO* item);
int __cdecl MotorbikeCanGetOff();
int __cdecl DoMotorbikeDynamics(int height, int speed, int* y, int flags);
void __cdecl TriggerMotorbikeExhaustSmoke(int x, int y, int z, short angle, short speed, int moving);
int __cdecl GetOnMotorbike(int itemNumber);
void __cdecl MotorbikeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int __cdecl GetOnMotorbike(int itemNumber);
int __cdecl GetMotorbikeCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void __cdecl MotorbikeBaddieCollision(ITEM_INFO* jeep);