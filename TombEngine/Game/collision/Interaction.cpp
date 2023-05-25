#include "framework.h"
#include "Game/collision/Interaction.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"

using namespace TEN::Math;

//namespace TEN::Collision
//{
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset,
									   const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		PosOffset = posOffset;
		OrientOffset = orientOffset;
		Bounds = bounds;
		OrientConstraint = orientConstraint;
	}
	
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& bounds,
									   const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		PosOffset = posOffset;
		Bounds = bounds;
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& bounds,
									   const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		OrientOffset = orientOffset;
		Bounds = bounds;
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const GameBoundingBox& bounds, const std::pair<EulerAngles, EulerAngles>& orientConstraint)
	{
		Bounds = bounds;
		OrientConstraint = orientConstraint;
	};

	bool InteractionBasis::TestInteraction(const ItemInfo& entityFrom, const ItemInfo& entityTo, const GameBoundingBox& boundsExtension) const
	{
		// Avoid overriding active interactions. NOTE: For now, can only check offset blending status.
		if (entityFrom.OffsetBlend.IsActive)
			return false;

		auto orientConstraintAverage = (OrientConstraint.first + OrientConstraint.second) / 2;
		auto poseFrom = Pose(entityFrom.Pose.Position, entityFrom.Pose.Orientation - orientConstraintAverage); // TODO: Check sign.

		// TODO: May interfere with pickups?
		// Test if entityFrom is aligned toward entityTo.
		if (!Geometry::IsPointInFront(poseFrom, entityTo.Pose.Position.ToVector3()))
			return false;

		// Test if entityFrom's orientation is within interaction constraint.
		auto deltaOrient = entityFrom.Pose.Orientation - entityTo.Pose.Orientation;
		if (deltaOrient.x < OrientConstraint.first.x || deltaOrient.x > OrientConstraint.second.x ||
			deltaOrient.y < OrientConstraint.first.y || deltaOrient.y > OrientConstraint.second.y ||
			deltaOrient.z < OrientConstraint.first.z || deltaOrient.z > OrientConstraint.second.z)
		{
			return false;
		}

		auto direction = (entityFrom.Pose.Position - entityTo.Pose.Position).ToVector3();
		auto rotMatrix = entityTo.Pose.Orientation.ToRotationMatrix().Transpose(); // NOTE: Transpose() used as faster equivalent to Invert().
		auto relPos = Vector3::Transform(direction, rotMatrix);

		// Test if entityFrom is inside interaction bounds.
		auto bounds = Bounds + boundsExtension;
		if (relPos.x < bounds.X1 || relPos.x > bounds.X2 ||
			relPos.y < bounds.Y1 || relPos.y > bounds.Y2 ||
			relPos.z < bounds.Z1 || relPos.z > bounds.Z2)
		{
			return false;
		}

		return true;
	}

	void SetEntityInteraction(ItemInfo& entityFrom, const ItemInfo& entityTo, const InteractionBasis& basis,
							  const Vector3i& extraPosOffset, const EulerAngles& extraOrientOffset)
	{
		constexpr auto OFFSET_BLEND_ALPHA = 0.4f;

		auto relPosOffset = basis.PosOffset + extraPosOffset;
		auto relOrientOffset = basis.OrientOffset + extraOrientOffset;

		auto targetPos = Geometry::TranslatePoint(entityTo.Pose.Position, entityTo.Pose.Orientation, relPosOffset);
		auto targetOrient = entityTo.Pose.Orientation + relOrientOffset;

		auto absPosOffset = (targetPos - entityFrom.Pose.Position).ToVector3();
		auto absOrientOffset = targetOrient - entityFrom.Pose.Orientation;
		entityFrom.OffsetBlend.Set(absPosOffset, absOrientOffset, OFFSET_BLEND_ALPHA);
	}

	void SetPlayerAlignAnimation(ItemInfo& playerEntity, const ItemInfo& entity)
	{
		auto& player = GetLaraInfo(playerEntity);

		// Check if already aligning.
		if (player.Control.IsMoving)
			return;

		// Check water status.
		if (player.Control.WaterStatus == WaterStatus::Underwater ||
			player.Control.WaterStatus == WaterStatus::TreadWater)
		{
			return;
		}

		float dist = Vector3i::Distance(playerEntity.Pose.Position, entity.Pose.Position);
		bool doAlignAnim = ((dist - LARA_ALIGN_VELOCITY) > (LARA_ALIGN_VELOCITY * ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD));

		// Skip animating if very close to the object.
		if (!doAlignAnim)
			return;

		short headingAngle = Geometry::GetOrientToPoint(playerEntity.Pose.Position.ToVector3(), entity.Pose.Position.ToVector3()).y;
		int cardinalDir = GetQuadrant(headingAngle - playerEntity.Pose.Orientation.y);

		// Set appropriate animation.
		switch (cardinalDir)
		{
		default:
		case NORTH:
			SetAnimation(&playerEntity, LA_WALK);
			break;

		case SOUTH:
			SetAnimation(&playerEntity, LA_WALK_BACK);
			break;

		case EAST:
			SetAnimation(&playerEntity, LA_SIDESTEP_RIGHT);
			break;

		case WEST:
			SetAnimation(&playerEntity, LA_SIDESTEP_LEFT);
			break;
		}

		player.Control.HandStatus = HandStatus::Busy;
		player.Control.IsMoving = true;
		player.Control.Count.PositionAdjust = 0;
	}
//}
