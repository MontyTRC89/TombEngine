#include "framework.h"
#include "Objects/TR4/Vehicles/motorbike.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/gui.h"
#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_flare.h"
#include "Specific/setup.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Game/health.h"
#include "Game/camera.h"
#include "Game/animation.h"
#include "Specific/prng.h"
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Game/items.h"

using namespace TEN::Math::Random;

/*collision stuff*/
#define BIKE_FRONT 500
#define BIKE_SIDE 350
#define BIKE_RADIUS 500
#define MOTORBIKE_SLIP 100
#define MOTORBIKE_FRICTION 0x180
/*movement stuff*/
#define MIN_MOMENTUM_TURN ANGLE(4.0f)
#define MAX_MOMENTUM_TURN ANGLE(1.5f)
#define MOTORBIKE_MAX_MOM_TURN ANGLE(150.0f)
#define MOTORBIKE_DEFAULT_HTURN ANGLE(1.5f)
#define MOTORBIKE_ACCEL_1 0x4000
#define MOTORBIKE_ACCEL_2 0x7000
#define MOTORBIKE_ACCEL_MAX 0xC000 //with the boost
#define MOTORBIKE_ACCEL 0x8000  //without the boost
#define MOTORBIKE_BIG_SLOWDOWN 0x3000
#define MOTORBIKE_SLOWDOWN1 0x440
#define MOTORBIKE_SLOWDOWN2 0x600
#define MOTORBIKE_HTURN ANGLE(0.5f)
#define MOTORBIKE_MAX_HTURN ANGLE(5.0f)
#define MOTORBIKE_PITCH_SLOWDOWN 0x8000
#define MOTORBIKE_PITCH_MAX 0xA000
#define MOTORBIKE_BACKING_VEL 0x800
/*controls*/
#define IN_ACCELERATE IN_ACTION
#define IN_REVERSE IN_BACK
#define IN_BRAKE IN_JUMP
#define IN_TURBO IN_SPRINT
#define	IN_TURNR (IN_RIGHT|IN_RSTEP)
#define	IN_TURNL (IN_LEFT|IN_LSTEP)

enum MOTORBIKE_STATE
{
    BIKE_EMPTY,
    BIKE_MOVING_FRONT,
    BIKE_MOVING_LEFT,
    BIKE_MOVING_BACK,
    BIKE_MOVING_BACK_LOOP,
    BIKE_EMPTY3,
    BIKE_STOP,
    BIKE_DEATH,
    BIKE_FALLING,
    BIKE_ENTER, // include unlocking state
    BIKE_EXIT,
    BIKE_HITFRONT,
    BIKE_HITBACK,
    BIKE_HITRIGHT,
    BIKE_HITLEFT,
    BIKE_IDLE,
    BIKE_LOADING_BOOST, // not used !
    BIKE_LANDING,
    BIKE_ACCELERATE,
    BIKE_EMPTY5,
    BIKE_EMPTY6,
    BIKE_NOTUSED,
    BIKE_MOVING_RIGHT
};

enum MOTORBIKE_ANIMS
{
    BA_DIE = 0,
    BA_BRAKE = 1,
    BA_MOVE_FORWARD = 2,
    BA_START_LEFT = 3,
    BA_LEFT = 4,
    BA_END_LEFT = 5,
    BA_START_FALL = 6,
    BA_FALLING = 7,
    BA_FALL_LAND = 8,
    BA_ENTER = 9,
    BA_EXIT = 10,
    BA_FRONT_HIT = 11,
    BA_BACK_HIT = 12,
    BA_LEFT_HIT = 13,
    BA_RIGHT_HIT = 14,
    BA_REV = 15, //unused? it looks like she's revving the engine but I've never seen it before
    BA_SLOWDOWN = 16,
    BA_UNUSED = 17,
    BA_IDLE = 18,
    BA_START_RIGHT = 19,
    BA_RIGHT = 20,
    BA_END_RIGHT = 21,
    BA_START_JUMP = 22,
    BA_JUMPING = 23,
    BA_JUMP_LAND = 24,
    BA_KICKSTART = 25,
    BA_BACK_START = 26,
    BA_BACK_LOOP = 27,
    BA_UNLOCK = 28
};

enum MOTORBIKE_FLAGS
{
    FL_BOOST = 1,
    FL_FALLING = 64,
    FL_DEATH = 128
};

static char ExhaustStart = 0;
static bool NoGetOff = false;

static MotorbikeInfo* GetMotorbikeInfo(ItemInfo* item)
{
    return (MotorbikeInfo*)item->Data;
}

void InitialiseMotorbike(short itemNumber)
{
    ItemInfo* item;
    MotorbikeInfo* motorbike;

    item = &g_Level.Items[itemNumber];
    item->Data = ITEM_DATA(MotorbikeInfo());
    motorbike = item->Data;
    motorbike->velocity = 0;
    motorbike->bikeTurn = 0;
    motorbike->pitch = 0;
    motorbike->momentumAngle = item->Pose.Orientation.y;
    motorbike->wallShiftRotation = 0;
    motorbike->extraRotation = 0;
    motorbike->flags = NULL;
    motorbike->lightPower = 0;
    motorbike->wheelLeft = 0; // left wheel
    motorbike->wheelRight = 0; // two wheel in the principal body
    item->MeshBits = 0x3F7;
}

static int TestMotorbikeHeight(ItemInfo* item, int dz, int dx, Vector3Int* pos)
{
    pos->y = item->Pose.Position.y - dz * phd_sin(item->Pose.Orientation.x) + dx * phd_sin(item->Pose.Orientation.z);

    float c = phd_cos(item->Pose.Orientation.y);
    float s = phd_sin(item->Pose.Orientation.y);

    pos->z = item->Pose.Position.z + dz * c - dx * s;
    pos->x = item->Pose.Position.x + dz * s + dx * c;

    short roomNumber = item->RoomNumber;
    FloorInfo* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
    int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
    if (pos->y < ceiling || ceiling == NO_HEIGHT)
        return NO_HEIGHT;

    return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static int DoMotorbikeShift(ItemInfo* motorbike, Vector3Int* pos, Vector3Int* old)
{
    int x = pos->x / SECTOR(1);
    int z = pos->z / SECTOR(1);
    int  oldX = old->x / SECTOR(1);
    int oldZ = old->z / SECTOR(1);
    int shiftX = pos->x & (WALL_SIZE - 1);
    int shiftZ = pos->z & (WALL_SIZE - 1);

    if (x == oldX)
    {
        if (z == oldZ)
        {
            motorbike->Pose.Position.z += (old->z - pos->z);
            motorbike->Pose.Position.x += (old->x - pos->x);
        }
        else if (z > oldZ)
        {
            motorbike->Pose.Position.z -= shiftZ + 1;
            return (pos->x - motorbike->Pose.Position.x);
        }
        else
        {
            motorbike->Pose.Position.z += WALL_SIZE - shiftZ;
            return (motorbike->Pose.Position.x - pos->x);
        }
    }
    else if (z == oldZ)
    {
        if (x > oldX)
        {
            motorbike->Pose.Position.x -= shiftX + 1;
            return (motorbike->Pose.Position.z - pos->z);
        }
        else
        {
            motorbike->Pose.Position.x += WALL_SIZE - shiftX;
            return (pos->z - motorbike->Pose.Position.z);
        }
    }
    else
    {
        x = 0;
        z = 0;

        short roomNumber = motorbike->RoomNumber;
        FloorInfo* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
        int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
        if (height < old->y - STEP_SIZE)
        {
            if (pos->z > old->z)
                z = -shiftZ - 1;
            else
                z = WALL_SIZE - shiftZ;
        }

        roomNumber = motorbike->RoomNumber;
        floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
        height = GetFloorHeight(floor, pos->x, pos->y, old->z);
        if (height < old->y - STEP_SIZE)
        {
            if (pos->x > old->x)
                x = -shiftX - 1;
            else
                x = WALL_SIZE - shiftX;
        }

        if (x && z)
        {
            motorbike->Pose.Position.z += z;
            motorbike->Pose.Position.x += x;
        }
        else if (z)
        {
            motorbike->Pose.Position.z += z;
            if (z > 0)
                return (motorbike->Pose.Position.x - pos->x);
            else
                return (pos->x - motorbike->Pose.Position.x);
        }
        else if (x)
        {
            motorbike->Pose.Position.x += x;
            if (x > 0)
                return (pos->z - motorbike->Pose.Position.z);
            else
                return (motorbike->Pose.Position.z - pos->z);
        }
        else
        {
            motorbike->Pose.Position.z += (old->z - pos->z);
            motorbike->Pose.Position.x += (old->x - pos->x);
        }
    }

    return 0;
}

static void DrawMotorbikeLight(ItemInfo* item)
{
    MotorbikeInfo* motorbike;
    Vector3Int start, target;
    int rnd;

    motorbike = GetMotorbikeInfo(item);
    start.x = 0;
    start.y = -470;
    start.z = 1836;
    GetJointAbsPosition(item, &start, 0);
    target.x = 0;
    target.y = -470;
    target.z = 20780;
    GetJointAbsPosition(item, &target, 0);
    rnd = (2 * motorbike->lightPower) - (GetRandomControl() & 0xF);
    // TODO: Spot Light
    /*if (rnd <= 0)
        SpotLightEnabled = false;
    else
        CreateSpotLight(&start, &target, item->pos.Orientation.y, rnd);*/
}

static BOOL GetOnMotorBike(short itemNumber)
{
    ItemInfo* item;
    FloorInfo* floor;
    int dx, dz, distance, height;
    unsigned short tempangle;
    short angle;
    short room_number;

    item = &g_Level.Items[itemNumber];
    if (item->Flags & ONESHOT || Lara.Control.HandStatus != HandStatus::Free || LaraItem->Animation.Airborne)
        return false;

    if ((abs(item->Pose.Position.y - LaraItem->Pose.Position.y) >= STEP_SIZE || !(TrInput & IN_ACTION)) && g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM1)
        return false;

    dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
    dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
    distance = SQUARE(dx) + SQUARE(dz);
    if (distance > 170000)
        return false;

    room_number = item->RoomNumber;
    floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room_number);
    height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
    if (height < -32000)
        return false;

    angle = phd_atan(item->Pose.Position.z - LaraItem->Pose.Position.z, item->Pose.Position.x - LaraItem->Pose.Position.x) - item->Pose.Orientation.y;
    tempangle = angle - item->Pose.Orientation.y;
    if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
    {
        // left
        if (tempangle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
            return false;
    }
    else
    {
        // right
        if (tempangle > ANGLE(225.0f) && tempangle < ANGLE(315.0f))
            return false;
    }
    return true;
}

void MotorbikeCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll)
{
    ItemInfo* item;
    MotorbikeInfo* motorbike;

    if (laraitem->HitPoints >= 0 && Lara.Vehicle == NO_ITEM)
    {
        item = &g_Level.Items[itemNumber];
        motorbike = GetMotorbikeInfo(item);

        // update motorbike light
        //if (motorbike->bikeTurn)
      //  {
//          motorbike->bikeTurn -= (motorbike->bikeTurn / 8) - 1;
            DrawMotorbikeLight(item);
  //      }

        if (GetOnMotorBike(itemNumber))
        {
            Lara.Vehicle = itemNumber;

            if (Lara.Control.Weapon.GunType == LaraWeaponType::Flare)
            {
                CreateFlare(LaraItem, ID_FLARE_ITEM, FALSE);
                UndrawFlareMeshes(laraitem);
                Lara.Flare.ControlLeft = false;
                Lara.Control.Weapon.GunType = LaraWeaponType::None;
                Lara.Control.Weapon.RequestGunType = LaraWeaponType::None;
                Lara.Flare.Life = 0;
            }

            Lara.Control.HandStatus = HandStatus::Free;

            short angle = phd_atan(item->Pose.Position.z - laraitem->Pose.Position.z, item->Pose.Position.x - laraitem->Pose.Position.x) - item->Pose.Orientation.y;
            if (angle <= -ANGLE(45.0f) || angle >= ANGLE(135.0f))
            {
                if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
                {
                    laraitem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_UNLOCK;
                    g_Gui.SetInventoryItemChosen(NO_ITEM);
                }
                else
                {
                    laraitem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_ENTER;
                }
                laraitem->Animation.TargetState = BIKE_ENTER;
                laraitem->Animation.ActiveState = BIKE_ENTER;
            }
            laraitem->Animation.FrameNumber = g_Level.Anims[laraitem->Animation.AnimNumber].frameBase;

            item->HitPoints = 1;
            laraitem->Pose.Position.x = item->Pose.Position.x;
            laraitem->Pose.Position.y = item->Pose.Position.y;
            laraitem->Pose.Position.z = item->Pose.Position.z;
            laraitem->Pose.Orientation.y = item->Pose.Orientation.y;
            ResetLaraFlex(laraitem);
            Lara.HitDirection = -1;
            AnimateItem(laraitem);
            motorbike->revs = 0;
            item->Collidable = true;
        }
        else
        {
            ObjectCollision(itemNumber, laraitem, coll);
        }
    }
}

static void TriggerMotorbikeExhaustSmoke(int x, int y, int z, short angle, short speed, BOOL moving)
{
    SPARKS* sptr;
    int rnd = 0;
    BYTE trans, size;

    sptr = &Sparks[GetFreeSpark()];
    sptr->dR = 96;
    sptr->dG = 96;
    sptr->on = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dB = 128;
    if (moving)
    {
        trans = speed * 4;
        sptr->dR = trans;
        sptr->dG = trans;
        sptr->dB = trans;
    }
    sptr->colFadeSpeed = 4;
    sptr->fadeToBlack = 4;
    rnd = (GetRandomControl() & 3) - (speed / 4096) + 20;
    if (rnd < 9)
    {
        sptr->life = 9;
        sptr->sLife = 9;
    }
    else
    {
        sptr->life = rnd;
        sptr->sLife = rnd;
    }

    sptr->transType = TransTypeEnum::COLADD;
    sptr->x = x + (GetRandomControl() & 0xF) - 8;
    sptr->y = y + (GetRandomControl() & 0xF) - 8;
    sptr->z = z + (GetRandomControl() & 0xF) - 8;
    sptr->xVel = speed * phd_sin(angle) / 4;
    sptr->yVel = (GetRandomControl() & 7) - 8;
    sptr->zVel = speed * phd_cos(angle) / 4;
    sptr->friction = 4;

    if (GetRandomControl() & 1)
    {
        sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
        sptr->rotAng = (GetRandomControl() & 0xFFF);
        if (GetRandomControl() & 1)
            sptr->rotAdd = (GetRandomControl() & 7) - 24;
        else
            sptr->rotAdd = (GetRandomControl() & 7) + 24;
    }
    else
    {
        sptr->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
    }

    sptr->scalar = 1;
    sptr->def = (unsigned char)Objects[ID_DEFAULT_SPRITES].meshIndex;
    sptr->gravity = (GetRandomControl() & 3) - 4;
    sptr->maxYvel = (GetRandomControl() & 7) - 8;
    size = (GetRandomControl() & 7) + (speed / 128) + 32;
    sptr->dSize = size;
    sptr->sSize = size / 2;
    sptr->size = size / 2;
}

static void DrawMotorBikeSmoke(ItemInfo* item)
{
    if (Lara.Vehicle == NO_ITEM)
        return;

    if (LaraItem->Animation.ActiveState != BIKE_ENTER && LaraItem->Animation.ActiveState != BIKE_EXIT)
    {
        Vector3Int pos;
        int speed;

        pos.x = 56;
        pos.y = -144;
        pos.z = -500;
        GetJointAbsPosition(item, &pos, 0);

        speed = item->Animation.Velocity;
        if (speed > 32 && speed < 64)
        {
            TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, item->Pose.Orientation.y - ANGLE(180), 64 - speed, TRUE);
            return;
        }

        if (ExhaustStart >= 16)
        {
            if (GetRandomControl() & 3)
                speed = 0;
            else
                speed = ((GetRandomControl() & 0xF) + (GetRandomControl() & 0x10)) * 64;
        }
        else
        {
            speed = ((GetRandomControl() & 0xF) + (GetRandomControl() & 0x10) + 2 * ExhaustStart) * 64;
        }

        TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, item->Pose.Orientation.y - ANGLE(180), speed, FALSE);
    }
}

static void MotorBikeExplode(ItemInfo* item)
{
	if (g_Level.Rooms[item->RoomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
	{
		TriggerUnderwaterExplosion(item, 1);
	}
	else
	{
		TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -2, 0, item->RoomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -1, 0, item->RoomNumber);
	}
    auto pos = PHD_3DPOS(item->Pose.Position.x, item->Pose.Position.y - 128, item->Pose.Position.z, 0, item->Pose.Orientation.y, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	ExplodingDeath(Lara.Vehicle, -2, 256);
	ExplodingDeath(Lara.ItemNumber, -2, 258); // enable blood
	LaraItem->HitPoints = 0;
	item->Status = ITEM_DEACTIVATED;

	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	Lara.Vehicle = NO_ITEM;
}

static int MotorBikeCheckGetOff(void)
{
    ItemInfo* item;

	if (Lara.Vehicle != NO_ITEM)
	{
		item = &g_Level.Items[Lara.Vehicle];
		if (LaraItem->Animation.ActiveState == BIKE_EXIT && LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
		{
			LaraItem->Pose.Orientation.y -= 0x4000;
			LaraItem->Animation.AnimNumber = LA_STAND_SOLID;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
			LaraItem->Animation.TargetState = LS_IDLE;
			LaraItem->Animation.ActiveState = LS_IDLE;
			LaraItem->Pose.Position.x -= 2 * phd_sin(item->Pose.Orientation.y);
			LaraItem->Pose.Position.z -= 2 * phd_cos(item->Pose.Orientation.y);
			LaraItem->Pose.Orientation.x = 0;
			LaraItem->Pose.Orientation.z = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.Control.HandStatus = HandStatus::Free;
			Lara.SprintEnergy = 120;
			return true;
		}

		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd)
			return true;

		// exit when falling
		// if (LaraItem->state_current == BIKE_EMPTY6) {

		// }
		// else if (LaraItem->state_current == BIKE_EMPTY5) {
		// lara death when falling too much
		// }

		return false;
	}
	else
		return false;
}

static int DoMotorBikeDynamics(int height, int fallspeed, int* y, int flags)
{
    int kick;

    if (height <= *y)
    {
        if (flags)
            return fallspeed;
        else
        {
            // On ground
            kick = (height - *y);

            if (kick < -80)
                kick = -80;

            fallspeed += ((kick - fallspeed) / 16);

            if (*y > height)
                *y = height;
        }
    }
    else
    {
        // In air
        *y += fallspeed;
        if (*y > height - 32)
        {
            *y = height;
            fallspeed = 0;
        }
        else
        {
            if (flags)
                fallspeed += flags;
            else
                fallspeed += GRAVITY;
        }
    }

    return fallspeed;
}

static int GetMotorbikeCollisionAnim(ItemInfo* item, Vector3Int* pos)
{
    pos->x = item->Pose.Position.x - pos->x;
    pos->z = item->Pose.Position.z - pos->z;

    if (pos->x || pos->z)
    {
        float c = phd_cos(item->Pose.Orientation.y);
        float s = phd_sin(item->Pose.Orientation.y);
        int front = pos->z * c + pos->x * s;
        int side = -pos->z * s + pos->x * c;

        if (abs(front) > abs(side))
        {
            return (front > 0) + 13;
        }
        else
        {
            return (side <= 0) + 11;
        }
    }

    return 0;
}

void MotorbikeBaddyCollision(ItemInfo* bike)
{
    int x, y, z, i;

    std::vector<short> roomsList;
    roomsList.push_back(bike->RoomNumber);

    ROOM_INFO* room = &g_Level.Rooms[bike->RoomNumber];
    for (i = 0; i < room->doors.size(); i++)
    {
        roomsList.push_back(room->doors[i].room);
    }

    for (int i = 0; i < roomsList.size(); i++)
    {
        for (short itemNumber : g_Level.Rooms[roomsList[i]].Items)
        {
            ItemInfo* item = &g_Level.Items[itemNumber];

            if (item->Collidable && item->Status != IFLAG_INVISIBLE && item != LaraItem && item != bike)
            {
                ObjectInfo* object = &Objects[item->ObjectNumber];

                if (object->collision && (object->intelligent))
                {
                    x = bike->Pose.Position.x - item->Pose.Position.x;
                    y = bike->Pose.Position.y - item->Pose.Position.y;
                    z = bike->Pose.Position.z - item->Pose.Position.z;

                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
                    {
                        if (item->ObjectNumber == ID_ROLLINGBALL)
                        {
                            if (TestBoundsCollide(item, LaraItem, 100))
                            {
                                if (LaraItem->HitPoints > 0)
                                {
                                    DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y - (STEP_SIZE * 2), LaraItem->Pose.Position.z, GetRandomControl() & 3, LaraItem->Pose.Orientation.y, LaraItem->RoomNumber, 5);
                                    LaraItem->HitPoints -= 8;
                                }
                            }
                        }
                        else
                        {
                            if (TestBoundsCollide(item, bike, BIKE_FRONT))
                            {
                                DoLotsOfBlood(bike->Pose.Position.x, bike->Pose.Position.y, bike->Pose.Position.z, GetRandomControl() & 3, LaraItem->Pose.Orientation.y, LaraItem->RoomNumber, 3);
                                item->HitPoints = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

static int MotorBikeDynamics(ItemInfo* item)
{
    MotorbikeInfo* motorbike;
    Vector3Int bl_old, mtb_old, br_old, mtf_old, fl_old;
    Vector3Int fl, bl, mtf, mtb, br;
    Vector3Int oldpos, moved;
    FloorInfo* floor;
    int hmf_old, hbl_old, hbr_old, hmtb_old, hfl_old;
    int height, collide, speed, newspeed;
    short momentum = 0, rot, room_number;

    NoGetOff = false;
    motorbike = GetMotorbikeInfo(item);

    hfl_old = TestMotorbikeHeight(item, BIKE_FRONT, -BIKE_SIDE, &fl_old);
    hmf_old = TestMotorbikeHeight(item, BIKE_FRONT, STEP_SIZE / 2, &mtf_old);
    hbl_old = TestMotorbikeHeight(item, -BIKE_FRONT, -BIKE_SIDE, &bl_old);
    hbr_old = TestMotorbikeHeight(item, -BIKE_FRONT, STEP_SIZE / 2, &br_old);
    hmtb_old = TestMotorbikeHeight(item, -BIKE_FRONT, 0, &mtb_old);

    oldpos.x = item->Pose.Position.x;
    oldpos.y = item->Pose.Position.y;
    oldpos.z = item->Pose.Position.z;

    if (bl_old.y > hbl_old)
        bl_old.y = hbl_old;
    if (br_old.y > hbr_old)
        br_old.y = hbr_old;
    if (fl_old.y > hfl_old)
        fl_old.y = hfl_old;
    if (mtf_old.y > hmf_old)
        mtf_old.y = hmf_old;
    if (mtb_old.y > hmtb_old)
        mtb_old.y = hmtb_old;

    if (item->Pose.Position.y <= (item->Floor - 8))
    {
        if (motorbike->bikeTurn < -91)
            motorbike->bikeTurn += 91;
        else if (motorbike->bikeTurn > 91)
            motorbike->bikeTurn -= 91;
        else
            motorbike->bikeTurn = 0;

        item->Pose.Orientation.y += motorbike->bikeTurn + motorbike->extraRotation;
        motorbike->momentumAngle += ((item->Pose.Orientation.y - motorbike->momentumAngle) / 32);
    }
    else
    {
        if (motorbike->bikeTurn >= -182)
        {
            if (motorbike->bikeTurn <= 182)
                motorbike->bikeTurn = 0;
            else
                motorbike->bikeTurn -= 182;
        }
        else
        {
            motorbike->bikeTurn += 182;
        }

        item->Pose.Orientation.y += motorbike->bikeTurn + motorbike->extraRotation;
        rot = item->Pose.Orientation.y - motorbike->momentumAngle;
        momentum = MIN_MOMENTUM_TURN - ((2 * motorbike->velocity) / SECTOR(1));

        if (!(TrInput & IN_ACCELERATE) && motorbike->velocity > 0)
            momentum += (momentum / 2);

        if (rot < -MAX_MOMENTUM_TURN)
        {
            if (rot < -MOTORBIKE_MAX_MOM_TURN)
            {
                rot = -MOTORBIKE_MAX_MOM_TURN;
                motorbike->momentumAngle = item->Pose.Orientation.y - rot;
            }
            else
            {
                motorbike->momentumAngle -= momentum;
            }
        }
        else if (rot > MAX_MOMENTUM_TURN)
        {
            if (rot > MOTORBIKE_MAX_MOM_TURN)
            {
                rot = MOTORBIKE_MAX_MOM_TURN;
                motorbike->momentumAngle = item->Pose.Orientation.y - rot;
            }
            else
            {
                motorbike->momentumAngle += momentum;
            }
        }
        else
        {
            motorbike->momentumAngle = item->Pose.Orientation.y;
        }
    }

    room_number = item->RoomNumber;
    floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room_number);
    height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
    if (item->Pose.Position.y >= height)
        speed = item->Animation.Velocity * phd_cos(item->Pose.Orientation.x);
    else
        speed = item->Animation.Velocity;

    item->Pose.Position.z += speed * phd_cos(motorbike->momentumAngle);
    item->Pose.Position.x += speed * phd_sin(motorbike->momentumAngle);

    if (item->Pose.Position.y >= height)
    {
        short anglex = MOTORBIKE_SLIP * phd_sin(item->Pose.Orientation.x);
        if (abs(anglex) > 16)
        {
            short anglex2 = MOTORBIKE_SLIP * phd_sin(item->Pose.Orientation.x);
            if (anglex < 0)
                anglex2 = -anglex;
            if (anglex2 > 24)
                NoGetOff = true;
            anglex *= 16;
            motorbike->velocity -= anglex;
        }

        short anglez = MOTORBIKE_SLIP * phd_sin(item->Pose.Orientation.z);
        if (abs(anglez) > 32)
        {
            short ang, angabs;
            NoGetOff = true;
            if (anglez >= 0)
                ang = item->Pose.Orientation.y + 0x4000;
            else
                ang = item->Pose.Orientation.y - 0x4000;
            angabs = abs(anglez) - 24;
            item->Pose.Position.x += angabs * phd_sin(ang);
            item->Pose.Position.z += angabs * phd_cos(ang);
        }
    }

    if (motorbike->velocity <= MOTORBIKE_ACCEL || motorbike->flags & FL_BOOST)
    {
        if (motorbike->velocity <= MOTORBIKE_ACCEL_MAX)
        {
            if (motorbike->velocity < -MOTORBIKE_BIG_SLOWDOWN)
                motorbike->velocity = -MOTORBIKE_BIG_SLOWDOWN;
        }
        else
            motorbike->velocity = MOTORBIKE_ACCEL_MAX;
    }
    else
        motorbike->velocity -= MOTORBIKE_SLOWDOWN1;

    moved.x = item->Pose.Position.x;
    moved.z = item->Pose.Position.z;

    if (!(item->Flags & ONESHOT))
    {
        MotorbikeBaddyCollision(item);
        //MotorBikeStaticCollision(item->pos.x, item->pos.y, item->pos.z, item->room_number, (WALL_L / 2));
    }

    int rot1 = 0;
    int rot2 = 0;

    int hfl = TestMotorbikeHeight(item, BIKE_FRONT, -BIKE_SIDE, &fl);
    if (hfl < fl_old.y - STEP_SIZE)
    {
        rot1 = abs(4 * DoMotorbikeShift(item, &fl, &fl_old));
    }

    int hbl = TestMotorbikeHeight(item, -BIKE_FRONT, -BIKE_SIDE, &bl);
    if (hbl < bl_old.y - STEP_SIZE)
    {
        if (rot1)
            rot1 += abs(4 * DoMotorbikeShift(item, &bl, &bl_old));
        else
            rot1 -= abs(4 * DoMotorbikeShift(item, &bl, &bl_old));
    }

    int hmtf = TestMotorbikeHeight(item, BIKE_FRONT, STEP_SIZE / 2, &mtf);
    if (hmtf < mtf_old.y - STEP_SIZE)
        rot2 -= abs(4 * DoMotorbikeShift(item, &bl, &bl_old));

    int hmtb = TestMotorbikeHeight(item, -BIKE_FRONT, 0, &mtb);
    if (hmtb < mtb_old.y - STEP_SIZE)
        DoMotorbikeShift(item, &mtb, &mtb_old);

    int hbr = TestMotorbikeHeight(item, -BIKE_FRONT, STEP_SIZE / 2, &br);
    if (hbr < br_old.y - STEP_SIZE)
    {
        if (rot2)
            rot2 -= abs(4 * DoMotorbikeShift(item, &bl, &bl_old));
        else
            rot2 += abs(4 * DoMotorbikeShift(item, &bl, &bl_old));
    }

    if (rot1)
        rot2 = rot1;

    room_number = item->RoomNumber;
    floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room_number);
    height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
    if (height < (item->Pose.Position.y - STEP_SIZE))
        DoMotorbikeShift(item, (Vector3Int*)&item->Pose, &oldpos);

    if (!motorbike->velocity)
        rot2 = 0;

    motorbike->wallShiftRotation = (motorbike->wallShiftRotation + rot2) / 2;
    if (abs(motorbike->wallShiftRotation) < 2)
        motorbike->wallShiftRotation = 0;

    if (abs(motorbike->wallShiftRotation - motorbike->extraRotation) >= 4)
        motorbike->extraRotation += ((motorbike->wallShiftRotation - motorbike->extraRotation) / 4);
    else
        motorbike->extraRotation = motorbike->wallShiftRotation;

    collide = GetMotorbikeCollisionAnim(item, &moved);
    if (collide)
    {
        newspeed = ((item->Pose.Position.z - oldpos.z) * phd_cos(motorbike->momentumAngle) + (item->Pose.Position.x - oldpos.x) * phd_sin(motorbike->momentumAngle)) * 256;
        if (&g_Level.Items[Lara.Vehicle] == item && motorbike->velocity >= MOTORBIKE_ACCEL && newspeed < (motorbike->velocity - 10))
        {
            LaraItem->HitPoints -= ((motorbike->velocity - newspeed) / 128);
            LaraItem->HitStatus = true;
        }

        if (motorbike->velocity > 0 && newspeed < motorbike->velocity)
            motorbike->velocity = (newspeed < 0) ? 0 : newspeed;
        else if (motorbike->velocity < 0 && newspeed > motorbike->velocity)
            motorbike->velocity = (newspeed > 0) ? 0 : newspeed;

        if (motorbike->velocity < -MOTORBIKE_BIG_SLOWDOWN)
            motorbike->velocity = -MOTORBIKE_BIG_SLOWDOWN;
    }

    return collide;
}

static BOOL MotorbikeCanGetOff(void)
{
    auto item = &g_Level.Items[Lara.Vehicle];
    auto angle = item->Pose.Orientation.y + 0x4000;
    auto x = item->Pose.Position.x + BIKE_RADIUS * phd_sin(angle);
    auto y = item->Pose.Position.y;
    auto z = item->Pose.Position.z + BIKE_RADIUS * phd_cos(angle);

	auto collResult = GetCollision(x, y, z, item->RoomNumber);

    if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT) // Was previously set to -NO_HEIGHT by TokyoSU -- Lwmte 23.08.21
        return false;
    if (abs(collResult.Position.Floor - item->Pose.Position.y) > STEP_SIZE)
        return false;
    if ((collResult.Position.Ceiling - item->Pose.Position.y) > -LARA_HEIGHT)
        return false;
    if ((collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
        return false;

    return true;
}

static void AnimateMotorbike(ItemInfo* item, int collide, BOOL dead)
{
    MotorbikeInfo* motorbike;
    motorbike = GetMotorbikeInfo(item);

    if (item->Pose.Position.y == item->Floor
        || LaraItem->Animation.ActiveState == BIKE_FALLING
        || LaraItem->Animation.ActiveState == BIKE_LANDING
        || LaraItem->Animation.ActiveState == BIKE_EMPTY6
        || dead)
    {
        if (!collide
            || LaraItem->Animation.ActiveState == BIKE_HITBACK
            || LaraItem->Animation.ActiveState == BIKE_HITFRONT
            || LaraItem->Animation.ActiveState == BIKE_HITLEFT
            || LaraItem->Animation.ActiveState == BIKE_EMPTY6
            || motorbike->velocity <= 10922
            || dead)
        {
            switch (LaraItem->Animation.ActiveState)
            {
            case BIKE_IDLE:
                if (dead)
                    LaraItem->Animation.TargetState = BIKE_DEATH;
                else 
                {

                    bool dismount;
                    if ((TrInput & IN_TURNR) && (TrInput & IN_BRAKE))
                        dismount = true;
                    else if (!((TrInput & IN_TURNR) && (TrInput & IN_BRAKE)))
                        dismount = false;

                    if (!dismount || motorbike->velocity || NoGetOff)
                    {
                        if (TrInput & IN_ACCELERATE && !(TrInput & IN_BRAKE))
                            LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
                        else if (TrInput & IN_REVERSE)
                            LaraItem->Animation.TargetState = BIKE_MOVING_BACK;
                    }
                    else if (dismount && MotorbikeCanGetOff())
                    {
                        LaraItem->Animation.TargetState = BIKE_EXIT;
                    }
                    else
                    {
                        LaraItem->Animation.TargetState = BIKE_IDLE;
                    }
                }
                break;

            case BIKE_MOVING_FRONT:
                if (dead)
                {
                    if (motorbike->velocity <= MOTORBIKE_ACCEL_1)
                        LaraItem->Animation.TargetState = BIKE_DEATH;
                    else
                        LaraItem->Animation.TargetState = BIKE_EMPTY5;
                }
                else if (motorbike->velocity & -256 || TrInput & (IN_ACCELERATE | IN_BRAKE))
                {
                    if (TrInput & IN_TURNL)
                        LaraItem->Animation.TargetState = BIKE_MOVING_LEFT;
                    else if (TrInput & IN_TURNR)
                        LaraItem->Animation.TargetState = BIKE_MOVING_RIGHT;
                    else if (TrInput & IN_BRAKE)
                    {
                        if (motorbike->velocity <= 0x5554)
                            LaraItem->Animation.TargetState = BIKE_EMPTY3;
                        else
                            LaraItem->Animation.TargetState = BIKE_STOP;
                    }
                    else if (TrInput & IN_REVERSE && motorbike->velocity <= MOTORBIKE_BACKING_VEL)
                        LaraItem->Animation.TargetState = BIKE_MOVING_BACK;
                    else if (motorbike->velocity == 0)
                        LaraItem->Animation.TargetState = BIKE_IDLE;
                }
                else
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                break;

            case BIKE_MOVING_LEFT:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNR || !(TrInput & IN_TURNL))
                        LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
                }
                else
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                if (motorbike->velocity == 0)
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                break;

            case BIKE_MOVING_BACK:
                if (TrInput & IN_REVERSE)
                    LaraItem->Animation.TargetState = BIKE_MOVING_BACK_LOOP;
                else
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                break;

            case BIKE_MOVING_RIGHT:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNL || !(TrInput & IN_TURNR))
                        LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
                }
                else
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                if (motorbike->velocity == 0)
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                break;

            case BIKE_EMPTY3:
            case BIKE_STOP:
            case BIKE_ACCELERATE:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNL)
                        LaraItem->Animation.TargetState = BIKE_MOVING_LEFT;
                    if (TrInput & IN_TURNR)
                        LaraItem->Animation.TargetState = BIKE_MOVING_RIGHT;
                }
                else
                {
                    LaraItem->Animation.TargetState = BIKE_IDLE;
                }
                break;

            case BIKE_FALLING:
                if (item->Pose.Position.y == item->Floor)
                {
                    LaraItem->Animation.TargetState = BIKE_LANDING;

                    int fallspeed_damage = item->Animation.VerticalVelocity - 140;
                    if (fallspeed_damage > 0)
                    {
                        if (fallspeed_damage <= 100)
                            LaraItem->HitPoints -= (-1000 * fallspeed_damage * fallspeed_damage) / 10000;
                        else
                            LaraItem->HitPoints = 0;
                    }
                }
                else if (item->Animation.VerticalVelocity > 220)
                    motorbike->flags |= FL_FALLING;
                break;

            case BIKE_HITFRONT:
            case BIKE_HITBACK:
            case BIKE_HITRIGHT:
            case BIKE_HITLEFT:
                if (TrInput & (IN_ACCELERATE | IN_BRAKE))
                    LaraItem->Animation.TargetState = BIKE_MOVING_FRONT;
                break;

            }
        }
        else
        {
            switch (collide)
            {

            case 13:
                LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_BACK_HIT;
                LaraItem->Animation.ActiveState = BIKE_HITBACK;
                LaraItem->Animation.TargetState = BIKE_HITBACK;
                LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
                break;

            case 14:
                LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_FRONT_HIT;
                LaraItem->Animation.ActiveState = BIKE_HITFRONT;
                LaraItem->Animation.TargetState = BIKE_HITFRONT;
                LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
                break;

            case 11:
                LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_RIGHT_HIT;
                LaraItem->Animation.ActiveState = BIKE_HITRIGHT;
                LaraItem->Animation.TargetState = BIKE_HITRIGHT;
                LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
                break;

            default:
            case 12:
                LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_LEFT_HIT;
                LaraItem->Animation.ActiveState = BIKE_HITLEFT;
                LaraItem->Animation.TargetState = BIKE_HITLEFT;
                LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
                break;
            }
        }
    }
    else
    {
        if (motorbike->velocity >= 0)
            LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_START_JUMP;
        else
            LaraItem->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_START_FALL;
        LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
        LaraItem->Animation.ActiveState = BIKE_FALLING;
        LaraItem->Animation.TargetState = BIKE_FALLING;
    }

    if (g_Level.Rooms[item->RoomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
    {
        LaraItem->Animation.TargetState = BIKE_EMPTY6;
        MotorBikeExplode(item);
    }
}

static int MotorbikeUserControl(ItemInfo* item, int height, int* pitch)
{
    MotorbikeInfo* motorbike;
    Vector3Int pos;
    int drive = 0;
    int rotation_speed, vel, newvel;

    motorbike = GetMotorbikeInfo(item);
    if (motorbike->lightPower < 127)
    {
        motorbike->lightPower += (GetRandomControl() & 7) + 3;
        if (motorbike->lightPower > 127)
            motorbike->lightPower = 127;
    }

    if (motorbike->revs > 0x10)
    {
        motorbike->velocity += (motorbike->revs / 16);
        motorbike->revs -= (motorbike->revs / 80);
    }
    else
    {
        motorbike->revs = 0;
    }

    if ((TrInput & IN_TURBO) && (TrInput & IN_ACCELERATE) && Lara.SprintEnergy)
    {
        motorbike->flags |= FL_BOOST;
        Lara.SprintEnergy -= 2;
        if (Lara.SprintEnergy > MOTORBIKE_ACCEL)//hmm
        {
            motorbike->flags &= ~FL_BOOST;
            Lara.SprintEnergy = 0;
        }
    }
    else
        motorbike->flags &= ~FL_BOOST;

    if (item->Pose.Position.y >= (height - STEP_SIZE))
    {
        if (!motorbike->velocity && (TrInput & IN_LOOK))
            LookUpDown(LaraItem);

        if (motorbike->velocity > 0)
        {
            if (TrInput & IN_TURNL)
            {
                if (motorbike->velocity > MOTORBIKE_ACCEL_1)
                    motorbike->bikeTurn -= MOTORBIKE_DEFAULT_HTURN;
                else
                    motorbike->bikeTurn -= (ANGLE(1) - MOTORBIKE_HTURN * motorbike->velocity) / 16384;

                if (motorbike->bikeTurn < -MOTORBIKE_MAX_HTURN)
                    motorbike->bikeTurn = -MOTORBIKE_MAX_HTURN;
            }
            else if (TrInput & IN_TURNR)
            {
                if (motorbike->velocity > MOTORBIKE_ACCEL_1)
                    motorbike->bikeTurn += MOTORBIKE_DEFAULT_HTURN;
                else
                    motorbike->bikeTurn += (ANGLE(1) + MOTORBIKE_HTURN * motorbike->velocity) / 16384;

                if (motorbike->bikeTurn > MOTORBIKE_MAX_HTURN)
                    motorbike->bikeTurn = MOTORBIKE_MAX_HTURN;
            }
        }
        else if (motorbike->velocity < 0)//moving backwards so the turning is inverted
        {
            if (TrInput & IN_TURNL)
            {
                motorbike->bikeTurn += MOTORBIKE_HTURN;
                if (motorbike->bikeTurn > MOTORBIKE_MAX_HTURN)
                    motorbike->bikeTurn = MOTORBIKE_MAX_HTURN;
            }
            else if (TrInput & IN_TURNR)
            {
                motorbike->bikeTurn -= MOTORBIKE_HTURN;
                if (motorbike->bikeTurn < -MOTORBIKE_MAX_HTURN)
                    motorbike->bikeTurn = -MOTORBIKE_MAX_HTURN;
            }
        }

        if (TrInput & IN_BRAKE)
        {
            pos.x = 0;
            pos.y = -144;
            pos.z = -1024;
            GetJointAbsPosition(item, &pos, NULL);
            TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 0x40, 0, 0);
            item->MeshBits = 0x5F7;
        }
        else
        {
            item->MeshBits = 0x3F7;
        }

        if (TrInput & IN_BRAKE)
        {
            if (motorbike->velocity < 0)
            {
                motorbike->velocity += 768;
                if (motorbike->velocity > 0)
                    motorbike->velocity = 0;
            }
            else
            {
                motorbike->velocity -= 768;
                if (motorbike->velocity < 0)
                    motorbike->velocity = 0;
            }
        }
        else if (TrInput & IN_ACCELERATE)
        {
            if (motorbike->velocity < MOTORBIKE_ACCEL_MAX)
            {
                if (motorbike->velocity < MOTORBIKE_ACCEL_1)
                    motorbike->velocity += 8 + ((MOTORBIKE_ACCEL_1 + MOTORBIKE_BACKING_VEL - motorbike->velocity) / 8);
                else if (motorbike->velocity < MOTORBIKE_ACCEL_2)
                    motorbike->velocity += 4 + ((MOTORBIKE_ACCEL_2 + MOTORBIKE_BACKING_VEL - motorbike->velocity) / 16);
                else if (motorbike->velocity < MOTORBIKE_ACCEL_MAX)
                    motorbike->velocity += 2 + ((MOTORBIKE_ACCEL_MAX - motorbike->velocity) / 16);

                if (motorbike->flags & FL_BOOST)
                    motorbike->velocity += 256;
            }
            else
            {
                motorbike->velocity = MOTORBIKE_ACCEL_MAX;
            }

            // apply friction according to turn
            motorbike->velocity -= (abs(item->Pose.Orientation.y - motorbike->momentumAngle) / 64);
        }
        else if (motorbike->velocity > MOTORBIKE_FRICTION)
        {
            motorbike->velocity -= MOTORBIKE_FRICTION;
            if (motorbike->velocity < 0)
                motorbike->velocity = 0;
        }
        else if (motorbike->velocity < MOTORBIKE_FRICTION)
        {
            motorbike->velocity += MOTORBIKE_FRICTION;
            if (motorbike->velocity > 0)
                motorbike->velocity = 0;
        }
        else
            motorbike->velocity = 0;

        if (LaraItem->Animation.ActiveState == BIKE_MOVING_BACK)
        {
			int framenow = LaraItem->Animation.FrameNumber;
			int framebase = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;

            if (framenow >= framebase + 24 && framenow <= framebase + 29)
            {
                if (motorbike->velocity > -MOTORBIKE_BIG_SLOWDOWN)
                    motorbike->velocity -= MOTORBIKE_SLOWDOWN2;
            }
        }

        item->Animation.Velocity = motorbike->velocity / 256;

        if (motorbike->engineRevs > MOTORBIKE_ACCEL_MAX)
            motorbike->engineRevs = (GetRandomControl() & 0x1FF) + 0xBF00;
        int newpitch = motorbike->velocity;
        if (motorbike->velocity < 0)
            newpitch /= 2;
        motorbike->engineRevs += ((abs(newpitch) - 0x2000 - motorbike->engineRevs) / 8);
        *pitch = motorbike->engineRevs;

    }
    else
    {
        if (motorbike->engineRevs < 0xFFFF)
            motorbike->engineRevs += ((motorbike->engineRevs - 0xFFFF) / 8);
        *pitch = motorbike->engineRevs;
    }

    return drive;
}

void SetLaraOnMotorBike(ItemInfo* item, ItemInfo* lara)//is this function even used
{
    MotorbikeInfo* motorbike;
    motorbike = GetMotorbikeInfo(item);

    Lara.Control.HandStatus = HandStatus::Busy;
    Lara.HitDirection = -1;
    lara->Animation.ActiveState = BIKE_IDLE;
    lara->Animation.TargetState = BIKE_IDLE;
    lara->Animation.AnimNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_IDLE;
    lara->Animation.FrameNumber = g_Level.Anims[lara->Animation.AnimNumber].frameBase;
    lara->Animation.Airborne = false;
    item->Animation.AnimNumber = lara->Animation.AnimNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
    item->Animation.FrameNumber = lara->Animation.FrameNumber + (g_Level.Anims[ID_MOTORBIKE].frameBase - g_Level.Anims[ID_MOTORBIKE_LARA_ANIMS].frameBase);
    item->HitPoints = 1;
    item->Flags = short(IFLAG_KILLED); // hmm... maybe wrong name (it can be IFLAG_CODEBITS) ?
    motorbike->revs = 0;
}

int MotorbikeControl(void)
{
    ItemInfo* item;
    MotorbikeInfo* motorbike;
    FloorInfo* floor;
    Vector3Int oldpos, fl, fr, fm;
    int drive, collide, pitch = 0, dead, ceiling;

    item = &g_Level.Items[Lara.Vehicle];
    motorbike = GetMotorbikeInfo(item);
    collide = MotorBikeDynamics(item);
    drive = -1;

    oldpos.x = item->Pose.Position.x;
    oldpos.y = item->Pose.Position.y;
    oldpos.z = item->Pose.Position.z;

    int hfl = TestMotorbikeHeight(item, BIKE_FRONT, -BIKE_SIDE, &fl);
    int hfr = TestMotorbikeHeight(item, BIKE_FRONT, STEP_SIZE / 2, &fr);
    int hfm = TestMotorbikeHeight(item, -BIKE_FRONT, 0, &fm);

	auto room_number = item->RoomNumber;
    floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room_number);
    int height = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

	TestTriggers(item, true);
	TestTriggers(item, false);

    if (LaraItem->HitPoints <= 0)
    {
        TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
        dead = true;
    }
    else
        dead = false;

    if (motorbike->flags)
        collide = 0;
    else
    {
        DrawMotorbikeLight(item);
        if (LaraItem->Animation.ActiveState < BIKE_ENTER || LaraItem->Animation.ActiveState > BIKE_EXIT)
            drive = MotorbikeUserControl(item, height, &pitch);
        else
        {
            drive = -1;
            collide = 0;
        }
    }

    if (motorbike->velocity > 0 || motorbike->revs)
    {
        motorbike->pitch = pitch;

    if (motorbike->pitch < -MOTORBIKE_PITCH_SLOWDOWN) 
        motorbike->pitch = -MOTORBIKE_PITCH_SLOWDOWN; 
    else 
    if (motorbike->pitch > MOTORBIKE_PITCH_MAX)
        motorbike->pitch = MOTORBIKE_PITCH_MAX;

        SoundEffect(SFX_TR4_BIKE_MOVING, &item->Pose, (motorbike->pitch * 256) + 0x1000004);
    }
    else
    {
        if (drive != -1)
        {
            SoundEffect(SFX_TR4_BIKE_IDLE, &item->Pose, 0);
            SoundEffect(SFX_TR4_BIKE_MOVING, &item->Pose, (motorbike->pitch * 256) + 0x1000004);
        }
        motorbike->pitch = 0;
    }

    item->Floor = height;
    int rotation = motorbike->velocity / 4;
    motorbike->wheelLeft = rotation;
    motorbike->wheelRight = rotation;
    int newy = item->Pose.Position.y;
    item->Animation.VerticalVelocity = DoMotorBikeDynamics(height, item->Animation.VerticalVelocity, &item->Pose.Position.y, 0);

    short xrot = 0, zrot = 0;
    int r1, r2;
    r1 = (fr.y + fl.y) / 2;
    r2 = (fr.y + fl.y) / 2;

    if (fm.y >= hfm)
    {
        if (r1 >= ((hfl + hfr) / 2))
        {
            xrot = phd_atan(1000, hfm - r1);
            zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
        }
        else
        {
            xrot = phd_atan(BIKE_FRONT, hfm - item->Pose.Position.y);
            zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
        }
    }
    else if (r1 >= ((hfl + hfr) / 2))
    {
        xrot = phd_atan(BIKE_FRONT, item->Pose.Position.y - r1);
        zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
    }
    else
    {
        xrot = phd_atan(125, newy - item->Pose.Position.y);
        zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
    }

    item->Pose.Orientation.x += ((xrot - item->Pose.Orientation.x) / 4);
    item->Pose.Orientation.z += ((zrot - item->Pose.Orientation.z) / 4);

    if (motorbike->flags >= 0)
    {
        if (room_number != item->RoomNumber)
        {
            ItemNewRoom(Lara.Vehicle, room_number);
            ItemNewRoom(Lara.ItemNumber, room_number);
        }

        LaraItem->Pose.Position.x = item->Pose.Position.x;
        LaraItem->Pose.Position.y = item->Pose.Position.y;
        LaraItem->Pose.Position.z = item->Pose.Position.z;
        LaraItem->Pose.Orientation.y = item->Pose.Orientation.y;
        LaraItem->Pose.Orientation.x = item->Pose.Orientation.x;
        LaraItem->Pose.Orientation.z = item->Pose.Orientation.z;

        AnimateMotorbike(item, collide, dead);
        AnimateItem(LaraItem);

        item->Animation.AnimNumber = LaraItem->Animation.AnimNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
        item->Animation.FrameNumber = LaraItem->Animation.FrameNumber + (g_Level.Anims[item->Animation.AnimNumber].frameBase - g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase);

        Camera.targetElevation = -5460;

        if (motorbike->flags & FL_FALLING)
        {
            if (item->Pose.Position.y == item->Floor)
            {
                ExplodingDeath(Lara.ItemNumber, -1, 256);
                LaraItem->Flags = ONESHOT;
                MotorBikeExplode(item);
                return 0;
            }
        }
    }

    if (LaraItem->Animation.ActiveState == BIKE_EXIT)
    {
        ExhaustStart = false;
        MotorBikeCheckGetOff();
        return 1;
    }

    MotorBikeCheckGetOff();
    return 1;
}

void DrawMotorbike(ItemInfo* item)
{
    // TODO: recreate the motorbike render here, then include the rotation for the wheel:
    //MOTORBIKE_INFO* motorbike = GetMotorbikeInfo(item);
}

void DrawMotorbikeEffect(ItemInfo* item)
{
    // TODO: speedometer
    //MOTORBIKE_INFO* motorbike = GetMotorbikeInfo(item);
    //if (Lara.Vehicle != NO_ITEM)
        //DrawMotorBikeSpeedoMeter(phd_winwidth - 64, phd_winheight - 16, motorbike->velocity, ANGLE(180), ANGLE(270), 32); // angle are 2D angle...
    DrawMotorBikeSmoke(item);
}
