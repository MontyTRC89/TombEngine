#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Collision
{
	using PlayerInteractRoutine = std::function<void(ItemInfo& playerEntity, ItemInfo& interactedEntity)>;
	using OrientConstraintPair	= std::pair<EulerAngles, EulerAngles>;

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
	void SetPlayerAlignAnim(ItemInfo& playerEntity, const ItemInfo& interactedEntity);
	
	void HandlePlayerInteraction(ItemInfo& playerEntity, ItemInfo& interactedEntity, const InteractionBasis& basis,
								 const PlayerInteractRoutine& interactRoutine);
}
