#include "newobjects.h"
#include "../Game/Box.h"
#include "../Game/lara.h"
#include "../Game/collide.h"
#include "../Game/Items.h"
#include "../Game/effects.h"
#include "../Game/sphere.h"
#include "../Game/effect2.h"
#include "../Game/draw.h"

/*---------------------------------------------------------------------------
 *	Constants
\*--------------------------------------------------------------------------*/

extern LaraExtraInfo g_LaraExtra;

enum boat_Anims {
	BOAT_GETON,
	BOAT_STILL,
	BOAT_MOVING,
	BOAT_JUMPR,
	BOAT_JUMPL,
	BOAT_HIT,
	BOAT_FALL,
	BOAT_TURNR,
	BOAT_DEATH,
	BOAT_TURNL
};

#define BOAT_GETONLW_ANIM	0
#define BOAT_GETONRW_ANIM	8
#define BOAT_GETONJ_ANIM	6
#define BOAT_GETON_START	1
#define BOAT_FALL_ANIM		15
#define BOAT_DEATH_ANIM		18
#define BOAT_UNDO_TURN		(ANGLE(1)/4)
#define BOAT_TURN			(ANGLE(1)/8)
#define BOAT_MAX_TURN		ANGLE(4)
#define BOAT_MAX_SPEED		110
#define BOAT_SLOW_SPEED		(BOAT_MAX_SPEED/3)
#define BOAT_FAST_SPEED		(BOAT_MAX_SPEED+75)
#define BOAT_MIN_SPEED		20
#define BOAT_ACCELERATION	5
#define BOAT_BRAKE			5
#define BOAT_SLOWDOWN		1
#define BOAT_REVERSE		-2	// -5
#define BOAT_MAX_BACK		-20
#define BOAT_MAX_KICK		-80
#define BOAT_SLIP			10
#define BOAT_SIDE_SLIP		30
#define BOAT_FRONT			750
#define BOAT_SIDE			300
#define BOAT_RADIUS			500
#define BOAT_SNOW			500
#define BOAT_MAX_HEIGHT		STEP_SIZE
#define GETOFF_DIST			WALL_SIZE
#define BOAT_WAKE			700
#define BOAT_SOUND_CEILING	(WALL_SIZE*5)
#define BOAT_TIP			(BOAT_FRONT+250)
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_SPEED 			4
#define SKIDOO_HIT_LEFT		11
#define SKIDOO_HIT_RIGHT	12
#define SKIDOO_HIT_FRONT	13
#define SKIDOO_HIT_BACK		14

void __cdecl InitialiseBoat(__int16 itemNum)
{

}

void __cdecl BoatCollision(__int16 itemNum, ITEM_INFO* litem, COLL_INFO* coll)
{

}

void __cdecl BoatControl(__int16 itemNum)
{

}