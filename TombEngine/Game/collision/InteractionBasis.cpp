#include "framework.h"
#include "Game/collision/InteractionBasis.h"

#include "Game/items.h"
#include "Math/Math.h"

using namespace TEN::Math;

//namespace TEN::Collision
//{
	InteractionBasis::InteractionBasis(const GameBoundingBox& bounds, const pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	};

	bool InteractionBasis::TestInteraction(const ItemInfo& entity0, const ItemInfo& entity1) const
	{
		return this->TestInteraction(entity0.Pose, entity1.Pose);
	}

	bool InteractionBasis::TestInteraction(const Pose& pose0, const Pose& pose1) const
	{
		auto deltaOrient = pose1.Orientation - pose0.Orientation;
		if (deltaOrient.x < OrientConstraint.first.x || deltaOrient.x > OrientConstraint.second.x ||
			deltaOrient.y < OrientConstraint.first.y || deltaOrient.y > OrientConstraint.second.y ||
			deltaOrient.z < OrientConstraint.first.z || deltaOrient.z > OrientConstraint.second.z)
		{
			return false;
		}

		auto direction = (pose1.Position - pose0.Position).ToVector3();
		auto rotMatrix = pose0.Orientation.ToRotationMatrix().Transpose(); // NOTE: Should be Invert(), but inverse/transpose of a rotation matrix are equal and transposing is faster.

		auto relativePos = Vector3::Transform(direction, rotMatrix);
		if (relativePos.x < Bounds.X1 || relativePos.x > Bounds.X2 ||
			relativePos.y < Bounds.Y1 || relativePos.y > Bounds.Y2 ||
			relativePos.z < Bounds.Z1 || relativePos.z > Bounds.Z2)
		{
			return false;
		}

		return true;
	}
//}
