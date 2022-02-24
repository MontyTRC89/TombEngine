#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;

namespace TEN::Entities::Generic
{
	constexpr auto ROPE_SEGMENTS = 24;
	constexpr auto ROPE_WIDTH = 24;

	struct ROPE_STRUCT
	{
		PHD_VECTOR segment[ROPE_SEGMENTS];
		PHD_VECTOR velocity[ROPE_SEGMENTS];
		PHD_VECTOR normalisedSegment[ROPE_SEGMENTS];
		PHD_VECTOR meshSegment[ROPE_SEGMENTS];
		PHD_VECTOR position;
		PHD_VECTOR coords[ROPE_SEGMENTS];
		int segmentLength;
		short active;
		short coiled;
	};

	struct PENDULUM
	{
		PHD_VECTOR position;
		PHD_VECTOR velocity;
		int node;
		ROPE_STRUCT* rope;
	};

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
	int RopeNodeCollision(ROPE_STRUCT* rope, int x, int y, int z, int value);
	void ApplyVelocityToRope(int node, short angle, short n);
	void SetPendulumVelocity(int x, int y, int z);
	void SetPendulumPoint(ROPE_STRUCT* rope, int node);
	void ModelRigidRope(ROPE_STRUCT* rope, PENDULUM* pendulumPointer, PHD_VECTOR* ropeVelocity, PHD_VECTOR* pendulumVelocity, int value);
	void ModelRigid(PHD_VECTOR* segment, PHD_VECTOR* nextSegment, PHD_VECTOR* velocity, PHD_VECTOR* nextVelocity, int length);
	void DelAlignLaraToRope(ITEM_INFO* item);
	void UpdateRopeSwing(ITEM_INFO* item);
	void JumpOffRope(ITEM_INFO* item);
	void FallFromRope(ITEM_INFO* item);
	void LaraClimbRope(ITEM_INFO* item, COLL_INFO* coll);
}
