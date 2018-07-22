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

