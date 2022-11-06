#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

//namespace TEN::Collision
//{
	struct InteractionBasis
	{
		Vector3i							PosOffset		 = Vector3i::Zero;
		EulerAngles							OrientOffset	 = EulerAngles::Zero;
		GameBoundingBox						Bounds			 = GameBoundingBox::Zero;
		std::pair<EulerAngles, EulerAngles> OrientConstraint = {};

		InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
		InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
		InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
		InteractionBasis(const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
	};

	bool TestEntityInteraction(const ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis, const GameBoundingBox& boundsExtension = GameBoundingBox::Zero);
	void SetEntityInteraction(ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis, const Vector3i& extraPosOffset = Vector3i::Zero, const EulerAngles& extraOrientOffset = EulerAngles::Zero);
//}
