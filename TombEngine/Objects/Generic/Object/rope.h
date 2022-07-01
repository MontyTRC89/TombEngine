#pragma once

struct ItemInfo;
struct CollisionInfo;
struct Vector3Int;

namespace TEN::Entities::Generic
{
	constexpr auto ROPE_SEGMENTS = 24;
	constexpr auto ROPE_WIDTH = 24;

	struct ROPE_STRUCT
	{
		int room;
		Vector3Int position;

		Vector3Int segment[ROPE_SEGMENTS];
		Vector3Int velocity[ROPE_SEGMENTS];
		Vector3Int normalisedSegment[ROPE_SEGMENTS];
		Vector3Int meshSegment[ROPE_SEGMENTS];
		Vector3Int coords[ROPE_SEGMENTS];

		int segmentLength;
		short active;
		short coiled;
	};

	struct PENDULUM
	{
		Vector3Int position;
		Vector3Int velocity;
		int node;
		ROPE_STRUCT* rope;
	};

	extern PENDULUM CurrentPendulum;
	extern PENDULUM AlternatePendulum;
	extern std::vector<ROPE_STRUCT> Ropes;
	extern int RopeSwing;

	void InitialiseRope(short itemNumber);
	void PrepareRope(ROPE_STRUCT* rope, Vector3Int* pos1, Vector3Int* pos2, int length, ItemInfo* item);
	Vector3Int* NormaliseRopeVector(Vector3Int* vec);
	void GetRopePos(ROPE_STRUCT* rope, int segmentFrame, int* x, int* y, int* z);
	int DotProduct(Vector3Int* u, Vector3Int* v);
	void ScaleVector(Vector3Int* src, int c, Vector3Int* dest);
	void CrossProduct(Vector3Int* u, Vector3Int* v, Vector3Int* dest);
	void phd_GetMatrixAngles(int* array, short* angle);
	void RopeControl(short itemNumber);
	void RopeCollision(short itemNumber, ItemInfo* l, CollisionInfo* coll);
	void RopeDynamics(ROPE_STRUCT* rope);
	int RopeNodeCollision(ROPE_STRUCT* rope, int x, int y, int z, int value);
	void ApplyVelocityToRope(int node, short angle, short n);
	void SetPendulumVelocity(int x, int y, int z);
	void SetPendulumPoint(ROPE_STRUCT* rope, int node);
	void ModelRigidRope(ROPE_STRUCT* rope, PENDULUM* pendulumPointer, Vector3Int* ropeVelocity, Vector3Int* pendulumVelocity, int value);
	void ModelRigid(Vector3Int* segment, Vector3Int* nextSegment, Vector3Int* velocity, Vector3Int* nextVelocity, int length);
	void DelAlignLaraToRope(ItemInfo* item);
	void UpdateRopeSwing(ItemInfo* item);
	void JumpOffRope(ItemInfo* item);
	void FallFromRope(ItemInfo* item, bool stumble = false);
	void LaraClimbRope(ItemInfo* item, CollisionInfo* coll);
	bool RopeSwingCollision(ItemInfo* item, CollisionInfo* coll);
}
