#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

//namespace TEN::Collision
//{
	class InteractionBasis
	{
	public://private:
		// Components
		Vector3i							PosOffset		 = Vector3i::Zero;
		EulerAngles							OrientOffset	 = EulerAngles::Zero;
		GameBoundingBox						Bounds			 = GameBoundingBox::Zero;
		std::pair<EulerAngles, EulerAngles> OrientConstraint = {};

	public:
		// Constructors
		InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
		InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);
		InteractionBasis(const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint);

		// Inquirers
		bool TestInteraction(const ItemInfo& entity0, const ItemInfo& entity1, const GameBoundingBox& boundsExtension = GameBoundingBox::Zero) const;
		bool TestInteraction(const Pose& pose0, const Pose& pose1, const GameBoundingBox& boundsExtension = GameBoundingBox::Zero) const;

		// Utilities
		void SetEntityOffsetBlend(ItemInfo& entity0, ItemInfo& entity1, const Vector3i extraPosOffset = Vector3i::Zero, const EulerAngles& extraOrientOffset = EulerAngles::Zero) const;
	};
//}
