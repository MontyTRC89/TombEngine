#pragma once

#define TEXTURE_PAGE	(256 * 256)

#define W2V_SHIFT 		14					// Shift scale of View.Frame to World.Frame
#define	W2V_SCALE 		(1 << W2V_SHIFT)	// Scale of View Frame to World Frame
#define WALL_SHIFT		10
#define STEP_SIZE		256
#define	WALL_SIZE		1024

#define GUARD			1
#define AMBUSH			2
#define PATROL1			4
#define MODIFY			8
#define FOLLOW			16

#define BOX_BLOCKED		(1 << 14) // unpassable for other enemies, always set for movable blocks & closed doors
#define BOX_LAST		(1 << 15) // unpassable by large enemies (T-Rex, Centaur, etc), always set behind doors

#define NUM_OBJECTS		488

#define UNIT_SHADOW		256
#define DEFAULT_RADIUS	10
#define ROT_X 4
#define ROT_Y 8
#define ROT_Z 16

#define FRONT_ARC		0x4000
#define NO_ITEM			-1

#define GAME_BUFFER_SIZE	128 * 1000000
#define NUM_SLOTS			32
#define NUM_ITEMS			1023
#define DRAW_DISTANCE		200 * 1024.0f
#define NUM_STATICS			1000

#define NO_ROOM				0xFF
#define NO_HEIGHT			(-0x7F00)

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


