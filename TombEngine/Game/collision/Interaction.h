#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Collision
{
	using PlayerInteractRoutine = std::function<void(ItemInfo& playerItem, ItemInfo& interactedItem)>;
	using OrientConstraintPair	= std::pair<EulerAngles, EulerAngles>;

	class InteractionBasis
	{
	public:
		// Members
		Vector3i			 PosOffset		  = Vector3i::Zero;
		EulerAngles			 OrientOffset	  = EulerAngles::Identity;
		BoundingOrientedBox	 Box			  = BoundingOrientedBox();
		OrientConstraintPair OrientConstraint = OrientConstraintPair(EulerAngles::Identity, EulerAngles::Identity);

		// Constructors
		InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const BoundingOrientedBox& box,
						 const OrientConstraintPair& orientConstraint);
		InteractionBasis(const Vector3i& posOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const EulerAngles& orientOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint);

		// TODO: Deprecated constructors.
		InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& box,
						 const OrientConstraintPair& orientConstraint);
		InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& box, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& box, const OrientConstraintPair& orientConstraint);
		InteractionBasis(const GameBoundingBox& box, const OrientConstraintPair& orientConstraint);

		// Inquirers
		bool TestInteraction(const ItemInfo& itemFrom, const ItemInfo& itemTo, std::optional<BoundingOrientedBox> expansionBox = std::nullopt) const;
	
		// Utilities
		void DrawDebug(const ItemInfo& item) const;
	};

	void SetItemInteraction(ItemInfo& itemFrom, const ItemInfo& itemTo, const InteractionBasis& basis,
							const Vector3i& extraPosOffset = Vector3i::Zero, const EulerAngles& extraOrientOffset = EulerAngles::Identity);
	void SetPlayerAlignAnim(ItemInfo& playerItem, const ItemInfo& interactedItem);
	
	bool HandlePlayerInteraction(ItemInfo& playerItem, ItemInfo& interactedItem, const InteractionBasis& basis, const PlayerInteractRoutine& interactRoutine);
}
