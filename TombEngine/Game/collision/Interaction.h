#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

//namespace TEN::Collision
//{
	using OrientConstraintPair = std::pair<EulerAngles, EulerAngles>;

	constexpr auto ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD = 6;

	class InteractionBasis
	{
	public:
		// Members
		Vector3i			 PosOffset		  = Vector3i::Zero;
		EulerAngles			 OrientOffset	  = EulerAngles::Zero;
		GameBoundingBox		 Bounds			  = GameBoundingBox::Zero;
		OrientConstraintPair OrientConstraint = OrientConstraintPair(EulerAngles::Zero, EulerAngles::Zero);

		// Constructors
		InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& bounds,
						 const OrientConstraintPair& orientConstraint);
		InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& bounds, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& bounds, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const GameBoundingBox& bounds, const OrientConstraintPair& orientConstraint);

		// Inquirers
		bool TestInteraction(const ItemInfo& entityFrom, const ItemInfo& entityTo, const GameBoundingBox& boundsExtension = GameBoundingBox::Zero) const;
	};

	void SetEntityInteraction(ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis,
							  const Vector3i& extraPosOffset = Vector3i::Zero, const EulerAngles& extraOrientOffset = EulerAngles::Zero);
	void SetPlayerAlignAnimation(ItemInfo& playerEntity, const ItemInfo& interactedEntity);

	//bool AlignPlayerToEntity(ItemInfo& playerEntity, const ItemInfo& entity, const Vector3i& posOffset = Vector3i::Zero, const EulerAngles& orientOffset = EulerAngles::Zero, bool doSnapAlign = false);
	//bool AlignPlayerToPose(ItemInfo* item, const Pose& toPose, float velocity, short turnRate);
//}
