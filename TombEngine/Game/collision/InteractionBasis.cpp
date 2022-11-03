#include "framework.h"
#include "Game/collision/InteractionBasis.h"

#include "Game/items.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include <Game/Lara/lara.h>

using namespace TEN::Math;

//namespace TEN::Collision
//{
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->PosOffset = posOffset;
		this->OrientOffset = orientOffset;
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	}
	
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->PosOffset = posOffset;
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	};

	bool InteractionBasis::TestInteraction(const ItemInfo& entity0, const ItemInfo& entity1, const GameBoundingBox& boundsExtension) const
	{
		/*if (entity1.IsLara())
		{
			const auto& player = GetLaraInfo(entity1);
			if (player.InteractedItem != NO_ITEM)
				return false;
		}*/

		return this->TestInteraction(entity0.Pose, entity1.Pose, boundsExtension);
	}

	bool InteractionBasis::TestInteraction(const Pose& pose0, const Pose& pose1, const GameBoundingBox& boundsExtension) const
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

		auto bounds = Bounds + boundsExtension;
		if (relativePos.x < bounds.X1 || relativePos.x > bounds.X2 ||
			relativePos.y < bounds.Y1 || relativePos.y > bounds.Y2 ||
			relativePos.z < bounds.Z1 || relativePos.z > bounds.Z2)
		{
			return false;
		}

		return true;
	}
//}
