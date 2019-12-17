#pragma once

#define HAIR_SEGMENTS 7 // only classic lara (young lara have more segments !)
#define DOUBLE_HAIR_SEGMENTS 14

#define TEXTURE_PAGE	(256 * 256)

#define PREDICTIVE_SCALE_FACTOR 14
#define W2V_SHIFT 		14					// Shift scale of View.Frame to World.Frame
#define NODE_SHIFT      15
#define	W2V_SCALE 		(1 << W2V_SHIFT)	// Scale of View Frame to World Frame
#define WALL_SHIFT		10
#define STEP_SIZE		256
#define	WALL_SIZE		1024

/// obj->hitEFFect flag
#define HIT_NONE     0
#define HIT_BLOOD    1
#define HIT_SMOKE    2
#define HIT_FRAGMENT 3

#define GUARD			1
#define AMBUSH			2
#define PATROL1			4
#define MODIFY			8
#define FOLLOW			16

#define BOX_BLOCKED		(1 << 14) // unpassable for other enemies, always set for movable blocks & closed doors
#define BOX_LAST		(1 << 15) // unpassable by large enemies (T-Rex, Centaur, etc), always set behind doors
#define GRAVITY 6

#define NUM_SPRITES		256
#define UNIT_SHADOW		256
#define NO_SHADOW       0
#define DEFAULT_RADIUS	10
#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

// ExplodingDeath2() last argument flags
#define EXPLODE_HIT_EFFECT 258
#define EXPLODE_NORMAL 256

#define FRONT_ARC		ANGLE(90)
#define NO_ITEM			-1
#define ALL_MESHBITS    -1

#define GAME_BUFFER_SIZE	128 * 1000000
#define NUM_SLOTS			32
#define NUM_ITEMS			1023
#define NUM_ROOMS			1024
#define NUM_ANIMATED_SETS	1024
#define DRAW_DISTANCE		200 * 1024.0f
#define NUM_STATICS			1000
#define MAX_LIGHTS			100
#define MAX_LIGHTS_DRAW     16384
#define MAX_DYNAMIC_LIGHTS  16384
#define MAX_STATICS			1000
#define MAX_DRAW_STATICS    16384
#define MAX_BONES			32
#define MAX_SPRITES			16384
#define MAX_SPARKS          1024      // 128 normally
#define MAX_SPARKS_FIRE     20
#define MAX_FIRE_LIST       32        // FIRE_SPARKS list
#define MAX_SPARKS_SMOKE    64
#define MAX_SPARKS_BLOOD    64
#define MAX_GUNSHELL        24
#define MAX_GUNFLASH        6
#define MAX_DRIPS           32
#define MAX_BUBBLES         40
#define MAX_DYNAMICS        64
#define MAX_SPLASH          4
#define MAX_RIPPLES         32
#define MAX_CAMERA          18
#define MAX_SHOCKWAVE       16
#define MAX_EFFECTS         100
#define MAX_DEBRIS          256
#define NO_ROOM				0xFF
#define NO_HEIGHT			(-0x7F00)
#define NUM_PICKUPS			64

#define	TIMID				0
#define VIOLENT				1

#define LARA_HITE			762

// Room flags

#define ENV_FLAG_WATER				0x0001
#define ENV_FLAG_SFX_ALWAYS			0x0002
#define ENV_FLAG_PITCH_SHIFT		0x0004
#define ENV_FLAG_OUTSIDE			0x0008
#define ENV_FLAG_DYNAMIC_LIT		0x0010
#define ENV_FLAG_WIND				0x0020
#define ENV_FLAG_NOT_NEAR_OUTSIDE	0x0040
#define ENV_FLAG_NO_LENSFLARE		0x0080  // Was quicksand in TR3.
#define ENV_FLAG_MIST				0x0100
#define ENV_FLAG_CAUSTICS			0x0200
#define ENV_FLAG_UNKNOWN3			0x0400
#define ENV_FLAG_SWAMP              0x0800

// From TR3, we need to check
#define	SP_FLAT			1
#define	SP_SCALE		2
#define	SP_BLOOD		4
#define	SP_DEF			8
#define	SP_ROTATE		16
#define	SP_EXPLOSION	32
#define	SP_FX			64
#define	SP_ITEM			128
#define	SP_WIND			256
#define	SP_EXPDEF		512
#define	SP_USEFXOBJPOS	1024
#define	SP_UNDERWEXP	2048
#define	SP_NODEATTATCH	4096
#define	SP_PLASMAEXP	8192

#define ONESHOT 0x100

#define DATA_TYPE 0x1F
#define DATA_TILT 0xF    // tile type (FLOOR_TYPE enum)
#define DATA_STATIC 0xFF // probably add static collision
#define END_BIT 0x8000
#define VALUE_BITS 0x3FF

#define NUM_BATS	64
#define NUM_SPIDERS	64
#define NUM_RATS	128

#define NUM_DEBRIS 256

#define WEAPON_AMMO1 0
#define WEAPON_AMMO2 1
#define WEAPON_AMMO3 2
#define WEAPON_AMMO4 3

#define BLOCKABLE      0x8000
#define BLOCKED        0x4000
#define OVERLAP_INDEX  0x3FFF
#define SEARCH_NUMBER  0x7FFF
#define BLOCKED_SEARCH 0x8000

#define FOLLOW_CENTRE 	1
#define NO_CHUNKY     	2
#define CHASE_OBJECT  	3
#define NO_MINY		0xFFFFFF

#define NO_BOX  0x7FF
#define BOX_JUMP 0x800
#define BOX_MONKEY 0x2000

#define BOX_NUMBER  0x7FF
#define BOX_END_BIT	0x8000

#define EXPAND_LEFT   0x1
#define EXPAND_RIGHT  0x2
#define EXPAND_TOP    0x4
#define EXPAND_BOTTOM 0x8

#define NO_FLYING 0
#define FLY_ZONE 0x2000

#define CLIP_LEFT   1
#define CLIP_RIGHT  2
#define CLIP_TOP    4
#define CLIP_BOTTOM 8
#define ALL_CLIP (CLIP_LEFT|CLIP_RIGHT|CLIP_TOP|CLIP_BOTTOM)
#define SECONDARY_CLIP 16

#define SLOPE_DIF 60
#define LARA_FREEFALL_SPEED 131
#define LARA_LEAN_RATE ((ANGLE(1)/2) + ANGLE(1))
#define LARA_LEAN_MAX ANGLE(11)
#define LARA_TURN_RATE ((ANGLE(1)/4) + ANGLE(2))
#define LARA_JUMP_TURN ANGLE(3)
#define LARA_SLOW_TURN ANGLE(4)
#define LARA_MED_TURN ANGLE(6)
#define LARA_FAST_TURN ANGLE(8)
#define STEPUP_HEIGHT ((STEP_SIZE*3)/2)
#define BAD_JUMP_CEILING ((STEP_SIZE*3)/4)
#define LARA_RAD 100
#define LARA_VELOCITY 12