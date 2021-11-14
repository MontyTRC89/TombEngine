#pragma once

#define MAX_VELOCITY		0xA000
#define MIN_HANDBRAKE_SPEED	0x3000

#define BRAKE 0x0280

#define REVERSE_ACC -0x0300
#define MAX_BACK	-0x3000

#define MAX_REVS 0xa000

#define TERMINAL_FALLSPEED 240
#define QUAD_SLIP 100
#define QUAD_SLIP_SIDE 50

#define QUAD_FRONT	550
#define QUAD_BACK  -550
#define QUAD_SIDE	260
#define QUAD_RADIUS	500
#define QUAD_HEIGHT	512

#define QUAD_HIT_LEFT  11
#define QUAD_HIT_RIGHT 12
#define QUAD_HIT_FRONT 13
#define QUAD_HIT_BACK  14

#define SMAN_SHOT_DAMAGE 10
#define SMAN_LARA_DAMAGE 50

#define DAMAGE_START  140
#define DAMAGE_LENGTH 14

#define GETOFF_DISTANCE 512	

#define QUAD_UNDO_TURN ANGLE(2.0f)
#define QUAD_TURN (ANGLE(0.5f) + QUAD_UNDO_TURN)
#define QUAD_MAX_TURN ANGLE(5.0f)
#define QUAD_HTURN (ANGLE(0.75f) + QUAD_UNDO_TURN)
#define QUAD_MAX_HTURN ANGLE(8.0f)

#define MIN_MOMENTUM_TURN ANGLE(3.0f)
#define MAX_MOMENTUM_TURN ANGLE(1.5f)
#define QUAD_MAX_MOM_TURN ANGLE(150.0f)

#define QUAD_FRONT	550
#define QUAD_SIDE	260
#define QUAD_RADIUS 500
#define QUAD_SNOW	500

#define QUAD_MAX_HEIGHT (STEP_SIZE)
#define QUAD_MIN_BOUNCE ((MAX_VELOCITY/2)/256)

#define QUADBIKE_TURNL_A 3
#define QUADBIKE_TURNL_F GetFrameNumber(ID_QUAD, QUADBIKE_TURNL_A, 0)
#define QUADBIKE_TURNR_A 20
#define QUADBIKE_TURNR_F GetFrameNumber(ID_QUAD, QUADBIKE_TURNR_A, 0)

#define QUADBIKE_FALLSTART_A 6
#define QUADBIKE_FALLSTART_F GetFrameNumber(ID_QUAD, QUADBIKE_FALLSTART_A, 0)
#define QUADBIKE_FALL_A 7
#define QUADBIKE_FALL_F GetFrameNumber(ID_QUAD, QUADBIKE_FALL_A, 0)
#define QUADBIKE_GETONR_A 9
#define QUADBIKE_GETONR_F GetFrameNumber(ID_QUAD, QUADBIKE_GETONR_A, 0)
#define Q_HITB_A 11
#define Q_HITF_A 12
#define Q_HITL_A 14
#define Q_HITR_A 13
#define QUADBIKE_GETONL_A 23
#define QUADBIKE_GETONL_F GetFrameNumber(ID_QUAD, QUADBIKE_GETONL_A, 0)
#define QUADBIKE_FALLSTART2_A 25

// TODO: Common controls for all vehicles + unique settings page to set them. @Sezz 2021.11.14
#define QUAD_IN_ACCELERATE	IN_ACTION
#define QUAD_IN_BRAKE		IN_JUMP
#define QUAD_IN_DRIFT		IN_DUCK | IN_SPRINT
#define QUAD_IN_UNMOUNT		IN_ROLL
#define QUAD_IN_LEFT		IN_LEFT
#define QUAD_IN_RIGHT		IN_RIGHT

struct QUAD_INFO {
	int velocity;
	short frontRot;
	short rearRot;
	int revs;
	int engineRevs;
	short trackMesh;
	int turnRate;
	int leftFallspeed;
	int rightFallspeed;
	short momentumAngle;
	short extraRotation;
	int pitch;
	char flags;
};
