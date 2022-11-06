#include "framework.h"
#include "Game/collision/InteractionBasis.h"

#include "Game/items.h"
#include "Math/Math.h"

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

	InteractionBasis::InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->OrientOffset = orientOffset;
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		this->Bounds = bounds;
		this->OrientConstraint = orientConstraint;
	};

	bool TestEntityInteraction(const ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis, const GameBoundingBox& boundsExtension)
	{
		// TODO: If player is already performing an interaction, don't interfere by allowing a new one.
		/*if (entity1.IsLara())
		{
			const auto& player = GetLaraInfo(entity1);
			if (player.InteractedItem != NO_ITEM)
				return false;
		}*/

		// Check whether interacting entity's orientation is within interaction constraint.
		auto deltaOrient = entityFrom.Pose.Orientation - entityTo.Pose.Orientation;
		if (deltaOrient.x < basis.OrientConstraint.first.x || deltaOrient.x > basis.OrientConstraint.second.x ||
			deltaOrient.y < basis.OrientConstraint.first.y || deltaOrient.y > basis.OrientConstraint.second.y ||
			deltaOrient.z < basis.OrientConstraint.first.z || deltaOrient.z > basis.OrientConstraint.second.z)
		{
			return false;
		}

		auto direction = (entityFrom.Pose.Position - entityTo.Pose.Position).ToVector3();
		auto rotMatrix = entityTo.Pose.Orientation.ToRotationMatrix().Transpose(); // NOTE: Should be Invert(), but inverse/transpose of a rotation matrix are equal and transposing is faster.
		auto relPos = Vector3::Transform(direction, rotMatrix);

		// Check whether interacting entity is inside interaction bounds.
		auto bounds = basis.Bounds + boundsExtension;
		if (relPos.x < bounds.X1 || relPos.x > bounds.X2 ||
			relPos.y < bounds.Y1 || relPos.y > bounds.Y2 ||
			relPos.z < bounds.Z1 || relPos.z > bounds.Z2)
		{
			return false;
		}

		return true;
	}

	void SetEntityInteraction(ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis, const Vector3i& extraPosOffset, const EulerAngles& extraOrientOffset)
	{
		auto relPosOffset = basis.PosOffset + extraPosOffset;
		auto relOrientOffset = basis.OrientOffset + extraOrientOffset;

		auto targetPos = Geometry::TranslatePoint(entityTo.Pose.Position, entityTo.Pose.Orientation, relPosOffset);
		auto targetOrient = entityTo.Pose.Orientation + relOrientOffset;

		auto absPosOffset = (targetPos - entityFrom.Pose.Position).ToVector3();
		auto absOrientOffset = targetOrient - entityFrom.Pose.Orientation;
		entityFrom.SetOffsetBlend(absPosOffset, absOrientOffset);
	}
//}
