#pragma once

#include "collide.h"

namespace TEN::Game::Rope
{
	struct ROPE_STRUCT
	{
		PHD_VECTOR segment[24]; // size=288, offset=0
		PHD_VECTOR velocity[24]; // size=288, offset=288
		PHD_VECTOR normalisedSegment[24]; // size=288, offset=576
		PHD_VECTOR meshSegment[24]; // size=288, offset=864
		PHD_VECTOR position; // size=12, offset=1152
		PHD_VECTOR coords[24];
		int segmentLength; // size=0, offset=1164
		short active; // size=0, offset=1168
		short coiled; // size=0, offset=1170
	};

	struct PENDULUM
	{
		PHD_VECTOR Position; // size=12, offset=0
		PHD_VECTOR Velocity; // size=12, offset=12
		int node; // size=0, offset=24
		ROPE_STRUCT* Rope; // size=1172, offset=28
	};

	constexpr auto ROPE_SEGMENTS = 24;
	constexpr auto ROPE_WIDTH = 24;

	extern PENDULUM CurrentPendulum;
	extern PENDULUM AlternatePendulum;
	extern std::vector<ROPE_STRUCT> Ropes;
	extern int RopeSwing;

	void InitialiseRope(short itemNumber);
	void PrepareRope(ROPE_STRUCT* rope, PHD_VECTOR* pos1, PHD_VECTOR* pos2, int length, ITEM_INFO* item);
	PHD_VECTOR* NormaliseRopeVector(PHD_VECTOR* vec);
	void GetRopePos(ROPE_STRUCT* rope, int segmentFrame, int* x, int* y, int* z);
	int DotProduct(PHD_VECTOR* u, PHD_VECTOR* v);
	void ScaleVector(PHD_VECTOR* src, int c, PHD_VECTOR* dest);
	void CrossProduct(PHD_VECTOR* u, PHD_VECTOR* v, PHD_VECTOR* dest);
	void phd_GetMatrixAngles(int* array, short* angle);
	void RopeControl(short itemNumber);
	void RopeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
	void RopeDynamics(ROPE_STRUCT* rope);
	int _0x0046D200(ROPE_STRUCT* rope, int x, int y, int z, int value);
	void ApplyVelocityToRope(int node, short angle, short n);
	void SetPendulumVelocity(int x, int y, int z);
	void _0x0046E1C0(ROPE_STRUCT* rope, int node);
	void _0x0046E080(ROPE_STRUCT* rope, PENDULUM* pendulumPointer, PHD_VECTOR* ropeVelocity, PHD_VECTOR* pendulumVelocity, int value);
	void _0x0046DF00(PHD_VECTOR* segment, PHD_VECTOR* nextSegment, PHD_VECTOR* velocity, PHD_VECTOR* nextVelocity, int length);
	void UpdateRopeSwing(ITEM_INFO* item);
	void JumpOffRope(ITEM_INFO* item);
	void FallFromRope(ITEM_INFO* item);
	void LaraClimbRope(ITEM_INFO* item, COLL_INFO* coll);
	void DelAlignLaraToRope(ITEM_INFO* item);
}