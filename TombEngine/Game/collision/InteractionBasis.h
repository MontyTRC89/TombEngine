#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

//namespace TEN::Collision
//{
	class InteractionBasis
	{
	public:
		// Components
		GameBoundingBox				   Bounds			= GameBoundingBox::Zero;
		pair<EulerAngles, EulerAngles> OrientConstraint = {};

		// Constructors
		InteractionBasis(const GameBoundingBox& bounds, const pair<EulerAngles, EulerAngles>& orientConstraint);

		// Utilities
		bool TestInteraction(const ItemInfo& entity0, const ItemInfo& entity1) const;
		bool TestInteraction(const Pose& pose0, const Pose& pose1) const;
	};
//}
