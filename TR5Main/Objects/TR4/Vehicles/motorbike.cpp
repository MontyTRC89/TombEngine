#include "framework.h"
#include "motorbike.h"
#include "level.h"
#include "control/control.h"
#include "effects/effects.h"
#include "lara.h"
#include "gui.h"
#include "collide.h"
#include "lara_flare.h"
#include "setup.h"
#include "lara_one_gun.h"
#include "effects/tomb4fx.h"
#include "items.h"
#include "Sound/sound.h"
#include "health.h"
#include "camera.h"
#include "animation.h"
#include "Specific/prng.h"
#include "motorbike_info.h"
#include "items.h"

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

static MOTORBIKE_INFO* GetMotorbikeInfo(ITEM_INFO* item)
{
    return (MOTORBIKE_INFO*)item->data;
}

void InitialiseMotorbike(short itemNumber)
{
    ITEM_INFO* item;
    MOTORBIKE_INFO* motorbike;

    item = &g_Level.Items[itemNumber];
    item->data = ITEM_DATA(MOTORBIKE_INFO());
    motorbike = item->data;
    motorbike->velocity = 0;
    motorbike->bikeTurn = 0;
    motorbike->pitch = 0;
    motorbike->momentumAngle = item->pos.yRot;
    motorbike->wallShiftRotation = 0;
    motorbike->extraRotation = 0;
    motorbike->flags = NULL;
    motorbike->lightPower = 0;
    motorbike->wheelLeft = 0; // left wheel
    motorbike->wheelRight = 0; // two wheel in the principal body
    item->meshBits = 0x3F7;
}

static int TestMotorbikeHeight(ITEM_INFO* item, int dz, int dx, PHD_VECTOR* pos)
{
    pos->y = item->pos.yPos - dz * phd_sin(item->pos.xRot) + dx * phd_sin(item->pos.zRot);

    float c = phd_cos(item->pos.yRot);
    float s = phd_sin(item->pos.yRot);

    pos->z = item->pos.zPos + dz * c - dx * s;
    pos->x = item->pos.xPos + dz * s + dx * c;

    short roomNumber = item->roomNumber;
    FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
    int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
    if (pos->y < ceiling || ceiling == NO_HEIGHT)
        return NO_HEIGHT;

    return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static int DoMotorbikeShift(ITEM_INFO* motorbike, PHD_VECTOR* pos, PHD_VECTOR* old)
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
            motorbike->pos.zPos += (old->z - pos->z);
            motorbike->pos.xPos += (old->x - pos->x);
        }
        else if (z > oldZ)
        {
            motorbike->pos.zPos -= shiftZ + 1;
            return (pos->x - motorbike->pos.xPos);
        }
        else
        {
            motorbike->pos.zPos += WALL_SIZE - shiftZ;
            return (motorbike->pos.xPos - pos->x);
        }
    }
    else if (z == oldZ)
    {
        if (x > oldX)
        {
            motorbike->pos.xPos -= shiftX + 1;
            return (motorbike->pos.zPos - pos->z);
        }
        else
        {
            motorbike->pos.xPos += WALL_SIZE - shiftX;
            return (pos->z - motorbike->pos.zPos);
        }
    }
    else
    {
        x = 0;
        z = 0;

        short roomNumber = motorbike->roomNumber;
        FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
        int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
        if (height < old->y - STEP_SIZE)
        {
            if (pos->z > old->z)
                z = -shiftZ - 1;
            else
                z = WALL_SIZE - shiftZ;
        }

        roomNumber = motorbike->roomNumber;
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
            motorbike->pos.zPos += z;
            motorbike->pos.xPos += x;
        }
        else if (z)
        {
            motorbike->pos.zPos += z;
            if (z > 0)
                return (motorbike->pos.xPos - pos->x);
            else
                return (pos->x - motorbike->pos.xPos);
        }
        else if (x)
        {
            motorbike->pos.xPos += x;
            if (x > 0)
                return (pos->z - motorbike->pos.zPos);
            else
                return (motorbike->pos.zPos - pos->z);
        }
        else
        {
            motorbike->pos.zPos += (old->z - pos->z);
            motorbike->pos.xPos += (old->x - pos->x);
        }
    }

    return 0;
}

static void DrawMotorbikeLight(ITEM_INFO* item)
{
    MOTORBIKE_INFO* motorbike;
    PHD_VECTOR start, target;
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
        CreateSpotLight(&start, &target, item->pos.yRot, rnd);*/
}

static BOOL GetOnMotorBike(short itemNumber)
{
    ITEM_INFO* item;
    FLOOR_INFO* floor;
    int dx, dz, distance, height;
    unsigned short tempangle;
    short angle;
    short room_number;

    item = &g_Level.Items[itemNumber];
    if (item->flags & ONESHOT || Lara.gunStatus != LG_NO_ARMS || LaraItem->gravityStatus)
        return false;

    if ((abs(item->pos.yPos - LaraItem->pos.yPos) >= STEP_SIZE || !(TrInput & IN_ACTION)) && g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM1)
        return false;

    dx = LaraItem->pos.xPos - item->pos.xPos;
    dz = LaraItem->pos.zPos - item->pos.zPos;
    distance = SQUARE(dx) + SQUARE(dz);
    if (distance > 170000)
        return false;

    room_number = item->roomNumber;
    floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
    height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
    if (height < -32000)
        return false;

    angle = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos) - item->pos.yRot;
    tempangle = angle - item->pos.yRot;
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

void MotorbikeCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* coll)
{
    ITEM_INFO* item;
    MOTORBIKE_INFO* motorbike;

    if (laraitem->hitPoints >= 0 && Lara.Vehicle == NO_ITEM)
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

            if (Lara.gunType == WEAPON_FLARE)
            {
                CreateFlare(LaraItem, ID_FLARE_ITEM, FALSE);
                UndrawFlareMeshes(laraitem);
                Lara.flareControlLeft = false;
                Lara.gunType = WEAPON_NONE;
                Lara.requestGunType = WEAPON_NONE;
                Lara.flareAge = 0;
            }

            Lara.gunStatus = LG_NO_ARMS;

            short angle = phd_atan(item->pos.zPos - laraitem->pos.zPos, item->pos.xPos - laraitem->pos.xPos) - item->pos.yRot;
            if (angle <= -ANGLE(45.0f) || angle >= ANGLE(135.0f))
            {
                if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
                {
                    laraitem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_UNLOCK;
                    g_Gui.SetInventoryItemChosen(NO_ITEM);
                }
                else
                {
                    laraitem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_ENTER;
                }
                laraitem->goalAnimState = BIKE_ENTER;
                laraitem->currentAnimState = BIKE_ENTER;
            }
            laraitem->frameNumber = g_Level.Anims[laraitem->animNumber].frameBase;

            item->hitPoints = 1;
            laraitem->pos.xPos = item->pos.xPos;
            laraitem->pos.yPos = item->pos.yPos;
            laraitem->pos.zPos = item->pos.zPos;
            laraitem->pos.yRot = item->pos.yRot;
            Lara.headYrot = 0;
            Lara.headXrot = 0;
            Lara.torsoYrot = 0;
            Lara.torsoXrot = 0;
            Lara.hitDirection = -1;
            AnimateItem(laraitem);
            motorbike->revs = 0;
            item->collidable = true;
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

static void DrawMotorBikeSmoke(ITEM_INFO* item)
{
    if (Lara.Vehicle == NO_ITEM)
        return;

    if (LaraItem->currentAnimState != BIKE_ENTER && LaraItem->currentAnimState != BIKE_EXIT)
    {
        PHD_VECTOR pos;
        int speed;

        pos.x = 56;
        pos.y = -144;
        pos.z = -500;
        GetJointAbsPosition(item, &pos, 0);

        speed = item->speed;
        if (speed > 32 && speed < 64)
        {
            TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, item->pos.yRot - ANGLE(180), 64 - speed, TRUE);
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

        TriggerMotorbikeExhaustSmoke(pos.x, pos.y, pos.z, item->pos.yRot - ANGLE(180), speed, FALSE);
    }
}

static void MotorBikeExplode(ITEM_INFO* item)
{
	if (g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
	{
		TriggerUnderwaterExplosion(item, 1);
	}
	else
	{
		TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);
	}
    auto pos = PHD_3DPOS(item->pos.xPos, item->pos.yPos - 128, item->pos.zPos, 0, item->pos.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	ExplodingDeath(Lara.Vehicle, -2, 256);
	ExplodingDeath(Lara.itemNumber, -2, 258); // enable blood
	LaraItem->hitPoints = 0;
	item->status = ITEM_DEACTIVATED;

	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	Lara.Vehicle = NO_ITEM;
}

static int MotorBikeCheckGetOff(void)
{
    ITEM_INFO* item;

	if (Lara.Vehicle != NO_ITEM)
	{
		item = &g_Level.Items[Lara.Vehicle];
		if (LaraItem->currentAnimState == BIKE_EXIT && LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
		{
			LaraItem->pos.yRot -= 0x4000;
			LaraItem->animNumber = LA_STAND_SOLID;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = LS_STOP;
			LaraItem->currentAnimState = LS_STOP;
			LaraItem->pos.xPos -= 2 * phd_sin(item->pos.yRot);
			LaraItem->pos.zPos -= 2 * phd_cos(item->pos.yRot);
			LaraItem->pos.xRot = 0;
			LaraItem->pos.zRot = 0;
			Lara.Vehicle = NO_ITEM;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.sprintTimer = 120;
			return true;
		}

		if (LaraItem->frameNumber != g_Level.Anims[LaraItem->animNumber].frameEnd)
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

static int GetMotorbikeCollisionAnim(ITEM_INFO* item, PHD_VECTOR* pos)
{
    pos->x = item->pos.xPos - pos->x;
    pos->z = item->pos.zPos - pos->z;

    if (pos->x || pos->z)
    {
        float c = phd_cos(item->pos.yRot);
        float s = phd_sin(item->pos.yRot);
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

void MotorbikeBaddieCollision(ITEM_INFO* bike)
{
    int x, y, z, i;

    std::vector<short> roomsList;
    roomsList.push_back(bike->roomNumber);

    ROOM_INFO* room = &g_Level.Rooms[bike->roomNumber];
    for (i = 0; i < room->doors.size(); i++)
    {
        roomsList.push_back(room->doors[i].room);
    }

    for (int i = 0; i < roomsList.size(); i++)
    {
        short itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

        while (itemNum != NO_ITEM)
        {
            ITEM_INFO* item = &g_Level.Items[itemNum];

            if (item->collidable && item->status != IFLAG_INVISIBLE && item != LaraItem && item != bike)
            {
                OBJECT_INFO* object = &Objects[item->objectNumber];

                if (object->collision && (object->intelligent))
                {
                    x = bike->pos.xPos - item->pos.xPos;
                    y = bike->pos.yPos - item->pos.yPos;
                    z = bike->pos.zPos - item->pos.zPos;

                    if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
                    {
                        if (item->objectNumber == ID_ROLLINGBALL)
                        {
                            if (TestBoundsCollide(item, LaraItem, 100))
                            {
                                if (LaraItem->hitPoints > 0)
                                {
                                    DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - (STEP_SIZE * 2), LaraItem->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 5);
                                    LaraItem->hitPoints -= 8;
                                }
                            }
                        }
                        else
                        {
                            if (TestBoundsCollide(item, bike, BIKE_FRONT))
                            {
                                DoLotsOfBlood(bike->pos.xPos, bike->pos.yPos, bike->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 3);
                                item->hitPoints = 0;
                            }
                        }
                    }
                }
            }

            itemNum = item->nextItem;
        }
    }
}

static int MotorBikeDynamics(ITEM_INFO* item)
{
    MOTORBIKE_INFO* motorbike;
    PHD_VECTOR bl_old, mtb_old, br_old, mtf_old, fl_old;
    PHD_VECTOR fl, bl, mtf, mtb, br;
    PHD_VECTOR oldpos, moved;
    FLOOR_INFO* floor;
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

    oldpos.x = item->pos.xPos;
    oldpos.y = item->pos.yPos;
    oldpos.z = item->pos.zPos;

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

    if (item->pos.yPos <= (item->floor - 8))
    {
        if (motorbike->bikeTurn < -91)
            motorbike->bikeTurn += 91;
        else if (motorbike->bikeTurn > 91)
            motorbike->bikeTurn -= 91;
        else
            motorbike->bikeTurn = 0;

        item->pos.yRot += motorbike->bikeTurn + motorbike->extraRotation;
        motorbike->momentumAngle += ((item->pos.yRot - motorbike->momentumAngle) / 32);
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

        item->pos.yRot += motorbike->bikeTurn + motorbike->extraRotation;
        rot = item->pos.yRot - motorbike->momentumAngle;
        momentum = MIN_MOMENTUM_TURN - ((2 * motorbike->velocity) / SECTOR(1));

        if (!(TrInput & IN_ACCELERATE) && motorbike->velocity > 0)
            momentum += (momentum / 2);

        if (rot < -MAX_MOMENTUM_TURN)
        {
            if (rot < -MOTORBIKE_MAX_MOM_TURN)
            {
                rot = -MOTORBIKE_MAX_MOM_TURN;
                motorbike->momentumAngle = item->pos.yRot - rot;
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
                motorbike->momentumAngle = item->pos.yRot - rot;
            }
            else
            {
                motorbike->momentumAngle += momentum;
            }
        }
        else
        {
            motorbike->momentumAngle = item->pos.yRot;
        }
    }

    room_number = item->roomNumber;
    floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
    height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
    if (item->pos.yPos >= height)
        speed = item->speed * phd_cos(item->pos.xRot);
    else
        speed = item->speed;

    item->pos.zPos += speed * phd_cos(motorbike->momentumAngle);
    item->pos.xPos += speed * phd_sin(motorbike->momentumAngle);

    if (item->pos.yPos >= height)
    {
        short anglex = MOTORBIKE_SLIP * phd_sin(item->pos.xRot);
        if (abs(anglex) > 16)
        {
            short anglex2 = MOTORBIKE_SLIP * phd_sin(item->pos.xRot);
            if (anglex < 0)
                anglex2 = -anglex;
            if (anglex2 > 24)
                NoGetOff = true;
            anglex *= 16;
            motorbike->velocity -= anglex;
        }

        short anglez = MOTORBIKE_SLIP * phd_sin(item->pos.zRot);
        if (abs(anglez) > 32)
        {
            short ang, angabs;
            NoGetOff = true;
            if (anglez >= 0)
                ang = item->pos.yRot + 0x4000;
            else
                ang = item->pos.yRot - 0x4000;
            angabs = abs(anglez) - 24;
            item->pos.xPos += angabs * phd_sin(ang);
            item->pos.zPos += angabs * phd_cos(ang);
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

    moved.x = item->pos.xPos;
    moved.z = item->pos.zPos;

    if (!(item->flags & ONESHOT))
    {
        MotorbikeBaddieCollision(item);
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

    room_number = item->roomNumber;
    floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
    height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
    if (height < (item->pos.yPos - STEP_SIZE))
        DoMotorbikeShift(item, (PHD_VECTOR*)&item->pos, &oldpos);

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
        newspeed = ((item->pos.zPos - oldpos.z) * phd_cos(motorbike->momentumAngle) + (item->pos.xPos - oldpos.x) * phd_sin(motorbike->momentumAngle)) * 256;
        if (&g_Level.Items[Lara.Vehicle] == item && motorbike->velocity >= MOTORBIKE_ACCEL && newspeed < (motorbike->velocity - 10))
        {
            LaraItem->hitPoints -= ((motorbike->velocity - newspeed) / 128);
            LaraItem->hitStatus = true;
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
    auto angle = item->pos.yRot + 0x4000;
    auto x = item->pos.xPos + BIKE_RADIUS * phd_sin(angle);
    auto y = item->pos.yPos;
    auto z = item->pos.zPos + BIKE_RADIUS * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, item->roomNumber);

    if (collResult.Position.Slope || collResult.Position.Floor == NO_HEIGHT) // Was previously set to -NO_HEIGHT by TokyoSU -- Lwmte 23.08.21
        return false;
    if (abs(collResult.Position.Floor - item->pos.yPos) > STEP_SIZE)
        return false;
    if ((collResult.Position.Ceiling - item->pos.yPos) > -LARA_HEIGHT)
        return false;
    if ((collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
        return false;

    return true;
}

static void AnimateMotorbike(ITEM_INFO* item, int collide, BOOL dead)
{
    MOTORBIKE_INFO* motorbike;
    motorbike = GetMotorbikeInfo(item);

    if (item->pos.yPos == item->floor
        || LaraItem->currentAnimState == BIKE_FALLING
        || LaraItem->currentAnimState == BIKE_LANDING
        || LaraItem->currentAnimState == BIKE_EMPTY6
        || dead)
    {
        if (!collide
            || LaraItem->currentAnimState == BIKE_HITBACK
            || LaraItem->currentAnimState == BIKE_HITFRONT
            || LaraItem->currentAnimState == BIKE_HITLEFT
            || LaraItem->currentAnimState == BIKE_EMPTY6
            || motorbike->velocity <= 10922
            || dead)
        {
            switch (LaraItem->currentAnimState)
            {
            case BIKE_IDLE:
                if (dead)
                    LaraItem->goalAnimState = BIKE_DEATH;
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
                            LaraItem->goalAnimState = BIKE_MOVING_FRONT;
                        else if (TrInput & IN_REVERSE)
                            LaraItem->goalAnimState = BIKE_MOVING_BACK;
                    }
                    else if (dismount && MotorbikeCanGetOff())
                    {
                        LaraItem->goalAnimState = BIKE_EXIT;
                    }
                    else
                    {
                        LaraItem->goalAnimState = BIKE_IDLE;
                    }
                }
                break;

            case BIKE_MOVING_FRONT:
                if (dead)
                {
                    if (motorbike->velocity <= MOTORBIKE_ACCEL_1)
                        LaraItem->goalAnimState = BIKE_DEATH;
                    else
                        LaraItem->goalAnimState = BIKE_EMPTY5;
                }
                else if (motorbike->velocity & -256 || TrInput & (IN_ACCELERATE | IN_BRAKE))
                {
                    if (TrInput & IN_TURNL)
                        LaraItem->goalAnimState = BIKE_MOVING_LEFT;
                    else if (TrInput & IN_TURNR)
                        LaraItem->goalAnimState = BIKE_MOVING_RIGHT;
                    else if (TrInput & IN_BRAKE)
                    {
                        if (motorbike->velocity <= 0x5554)
                            LaraItem->goalAnimState = BIKE_EMPTY3;
                        else
                            LaraItem->goalAnimState = BIKE_STOP;
                    }
                    else if (TrInput & IN_REVERSE && motorbike->velocity <= MOTORBIKE_BACKING_VEL)
                        LaraItem->goalAnimState = BIKE_MOVING_BACK;
                    else if (motorbike->velocity == 0)
                        LaraItem->goalAnimState = BIKE_IDLE;
                }
                else
                    LaraItem->goalAnimState = BIKE_IDLE;
                break;

            case BIKE_MOVING_LEFT:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNR || !(TrInput & IN_TURNL))
                        LaraItem->goalAnimState = BIKE_MOVING_FRONT;
                }
                else
                    LaraItem->goalAnimState = BIKE_IDLE;
                if (motorbike->velocity == 0)
                    LaraItem->goalAnimState = BIKE_IDLE;
                break;

            case BIKE_MOVING_BACK:
                if (TrInput & IN_REVERSE)
                    LaraItem->goalAnimState = BIKE_MOVING_BACK_LOOP;
                else
                    LaraItem->goalAnimState = BIKE_IDLE;
                break;

            case BIKE_MOVING_RIGHT:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNL || !(TrInput & IN_TURNR))
                        LaraItem->goalAnimState = BIKE_MOVING_FRONT;
                }
                else
                    LaraItem->goalAnimState = BIKE_IDLE;
                if (motorbike->velocity == 0)
                    LaraItem->goalAnimState = BIKE_IDLE;
                break;

            case BIKE_EMPTY3:
            case BIKE_STOP:
            case BIKE_ACCELERATE:
                if (motorbike->velocity & -256)
                {
                    if (TrInput & IN_TURNL)
                        LaraItem->goalAnimState = BIKE_MOVING_LEFT;
                    if (TrInput & IN_TURNR)
                        LaraItem->goalAnimState = BIKE_MOVING_RIGHT;
                }
                else
                {
                    LaraItem->goalAnimState = BIKE_IDLE;
                }
                break;

            case BIKE_FALLING:
                if (item->pos.yPos == item->floor)
                {
                    LaraItem->goalAnimState = BIKE_LANDING;

                    int fallspeed_damage = item->fallspeed - 140;
                    if (fallspeed_damage > 0)
                    {
                        if (fallspeed_damage <= 100)
                            LaraItem->hitPoints -= (-1000 * fallspeed_damage * fallspeed_damage) / 10000;
                        else
                            LaraItem->hitPoints = 0;
                    }
                }
                else if (item->fallspeed > 220)
                    motorbike->flags |= FL_FALLING;
                break;

            case BIKE_HITFRONT:
            case BIKE_HITBACK:
            case BIKE_HITRIGHT:
            case BIKE_HITLEFT:
                if (TrInput & (IN_ACCELERATE | IN_BRAKE))
                    LaraItem->goalAnimState = BIKE_MOVING_FRONT;
                break;

            }
        }
        else
        {
            switch (collide)
            {

            case 13:
                LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_BACK_HIT;
                LaraItem->currentAnimState = BIKE_HITBACK;
                LaraItem->goalAnimState = BIKE_HITBACK;
                LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
                break;

            case 14:
                LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_FRONT_HIT;
                LaraItem->currentAnimState = BIKE_HITFRONT;
                LaraItem->goalAnimState = BIKE_HITFRONT;
                LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
                break;

            case 11:
                LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_RIGHT_HIT;
                LaraItem->currentAnimState = BIKE_HITRIGHT;
                LaraItem->goalAnimState = BIKE_HITRIGHT;
                LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
                break;

            default:
            case 12:
                LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_LEFT_HIT;
                LaraItem->currentAnimState = BIKE_HITLEFT;
                LaraItem->goalAnimState = BIKE_HITLEFT;
                LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
                break;
            }
        }
    }
    else
    {
        if (motorbike->velocity >= 0)
            LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_START_JUMP;
        else
            LaraItem->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_START_FALL;
        LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
        LaraItem->currentAnimState = BIKE_FALLING;
        LaraItem->goalAnimState = BIKE_FALLING;
    }

    if (g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
    {
        LaraItem->goalAnimState = BIKE_EMPTY6;
        MotorBikeExplode(item);
    }
}

static int MotorbikeUserControl(ITEM_INFO* item, int height, int* pitch)
{
    MOTORBIKE_INFO* motorbike;
    PHD_VECTOR pos;
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

    if ((TrInput & IN_TURBO) && (TrInput & IN_ACCELERATE) && Lara.sprintTimer)
    {
        motorbike->flags |= FL_BOOST;
        Lara.sprintTimer -= 2;
        if (Lara.sprintTimer > MOTORBIKE_ACCEL)//hmm
        {
            motorbike->flags &= ~FL_BOOST;
            Lara.sprintTimer = 0;
        }
    }
    else
        motorbike->flags &= ~FL_BOOST;

    if (item->pos.yPos >= (height - STEP_SIZE))
    {
        if (!motorbike->velocity && (TrInput & IN_LOOK))
            LookUpDown();

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
            item->meshBits = 0x5F7;
        }
        else
        {
            item->meshBits = 0x3F7;
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
            motorbike->velocity -= (abs(item->pos.yRot - motorbike->momentumAngle) / 64);
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

        if (LaraItem->currentAnimState == BIKE_MOVING_BACK)
        {
            short framenow = LaraItem->frameNumber;
            short framebase = g_Level.Anims[LaraItem->animNumber].frameBase;

            if (framenow >= framebase + 24 && framenow <= framebase + 29)
            {
                if (motorbike->velocity > -MOTORBIKE_BIG_SLOWDOWN)
                    motorbike->velocity -= MOTORBIKE_SLOWDOWN2;
            }
        }

        item->speed = motorbike->velocity / 256;

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

void SetLaraOnMotorBike(ITEM_INFO* item, ITEM_INFO* lara)//is this function even used
{
    MOTORBIKE_INFO* motorbike;
    motorbike = GetMotorbikeInfo(item);

    Lara.gunStatus = LG_HANDS_BUSY;
    Lara.hitDirection = -1;
    lara->currentAnimState = BIKE_IDLE;
    lara->goalAnimState = BIKE_IDLE;
    lara->animNumber = Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex + BA_IDLE;
    lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
    lara->gravityStatus = false;
    item->animNumber = lara->animNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
    item->frameNumber = lara->frameNumber + (g_Level.Anims[ID_MOTORBIKE].frameBase - g_Level.Anims[ID_MOTORBIKE_LARA_ANIMS].frameBase);
    item->hitPoints = 1;
    item->flags = short(IFLAG_KILLED); // hmm... maybe wrong name (it can be IFLAG_CODEBITS) ?
    motorbike->revs = 0;
}

int MotorbikeControl(void)
{
    ITEM_INFO* item;
    MOTORBIKE_INFO* motorbike;
    FLOOR_INFO* floor;
    PHD_VECTOR oldpos, fl, fr, fm;
    int drive, collide, pitch = 0, dead, ceiling;

    item = &g_Level.Items[Lara.Vehicle];
    motorbike = GetMotorbikeInfo(item);
    collide = MotorBikeDynamics(item);
    drive = -1;

    oldpos.x = item->pos.xPos;
    oldpos.y = item->pos.yPos;
    oldpos.z = item->pos.zPos;

    int hfl = TestMotorbikeHeight(item, BIKE_FRONT, -BIKE_SIDE, &fl);
    int hfr = TestMotorbikeHeight(item, BIKE_FRONT, STEP_SIZE / 2, &fr);
    int hfm = TestMotorbikeHeight(item, -BIKE_FRONT, 0, &fm);

	auto room_number = item->roomNumber;
    floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
    int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	TestTriggers(item, true);
	TestTriggers(item, false);

    if (LaraItem->hitPoints <= 0)
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
        if (LaraItem->currentAnimState < BIKE_ENTER || LaraItem->currentAnimState > BIKE_EXIT)
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

        SoundEffect(SFX_TR4_BIKE_MOVING, &item->pos, (motorbike->pitch * 256) + 0x1000004);
    }
    else
    {
        if (drive != -1)
        {
            SoundEffect(SFX_TR4_BIKE_IDLE, &item->pos, 0);
            SoundEffect(SFX_TR4_BIKE_MOVING, &item->pos, (motorbike->pitch * 256) + 0x1000004);
        }
        motorbike->pitch = 0;
    }

    item->floor = height;
    int rotation = motorbike->velocity / 4;
    motorbike->wheelLeft = rotation;
    motorbike->wheelRight = rotation;
    int newy = item->pos.yPos;
    item->fallspeed = DoMotorBikeDynamics(height, item->fallspeed, &item->pos.yPos, 0);

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
            xrot = phd_atan(BIKE_FRONT, hfm - item->pos.yPos);
            zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
        }
    }
    else if (r1 >= ((hfl + hfr) / 2))
    {
        xrot = phd_atan(BIKE_FRONT, item->pos.yPos - r1);
        zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
    }
    else
    {
        xrot = phd_atan(125, newy - item->pos.yPos);
        zrot = phd_atan(BIKE_SIDE, r2 - fl.y);
    }

    item->pos.xRot += ((xrot - item->pos.xRot) / 4);
    item->pos.zRot += ((zrot - item->pos.zRot) / 4);

    if (motorbike->flags >= 0)
    {
        if (room_number != item->roomNumber)
        {
            ItemNewRoom(Lara.Vehicle, room_number);
            ItemNewRoom(Lara.itemNumber, room_number);
        }

        LaraItem->pos.xPos = item->pos.xPos;
        LaraItem->pos.yPos = item->pos.yPos;
        LaraItem->pos.zPos = item->pos.zPos;
        LaraItem->pos.yRot = item->pos.yRot;
        LaraItem->pos.xRot = item->pos.xRot;
        LaraItem->pos.zRot = item->pos.zRot;

        AnimateMotorbike(item, collide, dead);
        AnimateItem(LaraItem);

        item->animNumber = LaraItem->animNumber + (Objects[ID_MOTORBIKE].animIndex - Objects[ID_MOTORBIKE_LARA_ANIMS].animIndex);
        item->frameNumber = LaraItem->frameNumber + (g_Level.Anims[item->animNumber].frameBase - g_Level.Anims[LaraItem->animNumber].frameBase);

        Camera.targetElevation = -5460;

        if (motorbike->flags & FL_FALLING)
        {
            if (item->pos.yPos == item->floor)
            {
                ExplodingDeath(Lara.itemNumber, -1, 256);
                LaraItem->flags = ONESHOT;
                MotorBikeExplode(item);
                return 0;
            }
        }
    }

    if (LaraItem->currentAnimState == BIKE_EXIT)
    {
        ExhaustStart = false;
        MotorBikeCheckGetOff();
        return 1;
    }

    MotorBikeCheckGetOff();
    return 1;
}

void DrawMotorbike(ITEM_INFO* item)
{
    // TODO: recreate the motorbike render here, then include the rotation for the wheel:
    //MOTORBIKE_INFO* motorbike = GetMotorbikeInfo(item);
}

void DrawMotorbikeEffect(ITEM_INFO* item)
{
    // TODO: speedometer
    //MOTORBIKE_INFO* motorbike = GetMotorbikeInfo(item);
    //if (Lara.Vehicle != NO_ITEM)
        //DrawMotorBikeSpeedoMeter(phd_winwidth - 64, phd_winheight - 16, motorbike->velocity, ANGLE(180), ANGLE(270), 32); // angle are 2D angle...
    DrawMotorBikeSmoke(item);
}
