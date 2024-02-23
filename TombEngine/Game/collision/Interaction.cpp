#include "framework.h"
#include "Game/collision/Interaction.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision
{
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset,
									   const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
	{
		PosOffset = posOffset;
		OrientOffset = orientOffset;
		Box = box;
		OrientConstraint = orientConstraint;
	}
	
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
	{
		PosOffset = posOffset;
		Box = box;
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const EulerAngles& orientOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
	{
		OrientOffset = orientOffset;
		Box = box;
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
	{
		Box = box;
		OrientConstraint = orientConstraint;
	};

	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset,
									   const GameBoundingBox& box, const OrientConstraintPair& orientConstraint)
	{
		PosOffset = posOffset;
		OrientOffset = orientOffset;
		Box = box.ToBoundingOrientedBox(Pose::Zero);
		OrientConstraint = orientConstraint;
	}
	
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const GameBoundingBox& box, const OrientConstraintPair& orientConstraint)
	{
		PosOffset = posOffset;
		Box = box.ToBoundingOrientedBox(Pose::Zero);
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const EulerAngles& orientOffset, const GameBoundingBox& box, const OrientConstraintPair& orientConstraint)
	{
		OrientOffset = orientOffset;
		Box = box.ToBoundingOrientedBox(Pose::Zero);
		OrientConstraint = orientConstraint;
	}

	InteractionBasis::InteractionBasis(const GameBoundingBox& box, const OrientConstraintPair& orientConstraint)
	{
		Box = box.ToBoundingOrientedBox(Pose::Zero);
		OrientConstraint = orientConstraint;
	};

	bool InteractionBasis::TestInteraction(const ItemInfo& itemFrom, const ItemInfo& itemTo, std::optional<BoundingOrientedBox> expansionBox) const
	{
		DrawDebug(itemTo);

		// 1) Avoid overriding active offset blend.
		if (itemFrom.OffsetBlend.IsActive)
			return false;

		// TODO: Currently unreliable because IteractedItem is frequently not reset after completed interactions.
		// 2) Avoid overriding active player interactions.
		if (itemFrom.IsLara())
		{
			const auto& player = GetLaraInfo(itemFrom);
			if (player.Context.InteractedItem != NO_ITEM)
				return false;
		}

		// 3) Test if itemFrom's orientation is within interaction constraint.
		auto deltaOrient = itemFrom.Pose.Orientation - itemTo.Pose.Orientation;
		if (deltaOrient.x < OrientConstraint.first.x || deltaOrient.x > OrientConstraint.second.x ||
			deltaOrient.y < OrientConstraint.first.y || deltaOrient.y > OrientConstraint.second.y ||
			deltaOrient.z < OrientConstraint.first.z || deltaOrient.z > OrientConstraint.second.z)
		{
			return false;
		}

		// Calculate box-relative position.
		auto deltaPos = (itemFrom.Pose.Position - itemTo.Pose.Position).ToVector3();
		auto invRotMatrix = itemTo.Pose.Orientation.ToRotationMatrix().Invert();
		auto relPos = Vector3::Transform(deltaPos, invRotMatrix);

		// Calculate box min and max.
		auto box = expansionBox.has_value() ? Geometry::GetExpandedBoundingOrientedBox(Box, *expansionBox) : Box;
		auto max = box.Center + box.Extents;
		auto min = box.Center - box.Extents;

		// 4) Test if itemFrom is inside interaction box.
		if (relPos.x < min.x || relPos.x > max.x ||
			relPos.y < min.y || relPos.y > max.y ||
			relPos.z < min.z || relPos.z > max.z)
		{
			return false;
		}

		// TODO: Not working.
		/*box = expansionBox.has_value() ? Geometry::GetExpandedBoundingOrientedBox(Box, *expansionBox) : Box;
		box.Center = Vector3::Transform(box.Center, box.Orientation) + itemTo.Pose.Position.ToVector3();

		// 4) Test if itemFrom's position intersects interaction box.
		if (!Geometry::IsPointInBox(deltaPos, box))
			return false;*/

		return true;
	}

	void InteractionBasis::DrawDebug(const ItemInfo& item) const
	{
		constexpr auto COLL_BOX_COLOR	  = Color(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr auto INTERACT_BOX_COLOR = Color(0.0f, 1.0f, 1.0f, 1.0f);

		// Draw collision box.
		auto collBox = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		g_Renderer.AddDebugBox(collBox, COLL_BOX_COLOR);

		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		auto relCenter = Vector3::Transform(Box.Center, rotMatrix);
		auto orient = item.Pose.Orientation.ToQuaternion();

		// Draw interaction box.
		auto interactBox = BoundingOrientedBox(item.Pose.Position.ToVector3() + relCenter, Box.Extents, orient);
		g_Renderer.AddDebugBox(interactBox, INTERACT_BOX_COLOR);
	}

	void SetItemInteraction(ItemInfo& itemFrom, const ItemInfo& itemTo, const InteractionBasis& basis,
							const Vector3i& extraPosOffset, const EulerAngles& extraOrientOffset)
	{
		constexpr auto OFFSET_BLEND_LOG_ALPHA = 0.25f;

		// Calculate relative offsets.
		auto relPosOffset = basis.PosOffset + extraPosOffset;
		auto relOrientOffset = basis.OrientOffset + extraOrientOffset;

		// Calculate targets.
		auto targetPos = Geometry::TranslatePoint(itemTo.Pose.Position, itemTo.Pose.Orientation, relPosOffset);
		auto targetOrient = itemTo.Pose.Orientation + relOrientOffset;

		// Calculate absolute offsets.
		auto absPosOffset = (targetPos - itemFrom.Pose.Position).ToVector3();
		auto absOrientOffset = targetOrient - itemFrom.Pose.Orientation;

		// Set item parameters.
		itemFrom.Animation.Velocity = Vector3::Zero;
		itemFrom.OffsetBlend.SetLogarithmic(absPosOffset, absOrientOffset, OFFSET_BLEND_LOG_ALPHA);

		// Set player parameters.
		if (itemFrom.IsLara())
		{
			auto& player = GetLaraInfo(itemFrom);

			player.Control.TurnRate = 0;
			player.Control.HandStatus = HandStatus::Busy;
			player.Context.InteractedItem = itemTo.Index;
		}
	}

	static int GetPlayerAlignAnim(const Pose& poseFrom, const Pose& poseTo)
	{
		short headingAngle = Geometry::GetOrientToPoint(poseFrom.Position.ToVector3(), poseTo.Position.ToVector3()).y;
		int cardinalDir = GetQuadrant(headingAngle - poseFrom.Orientation.y);

		// Return alignment anim number.
		switch (cardinalDir)
		{
		default:
		case NORTH:
			return LA_WALK;

		case SOUTH:
			return LA_WALK_BACK;

		case EAST:
			return LA_SIDESTEP_RIGHT;

		case WEST:
			return LA_SIDESTEP_LEFT;
		}
	}

	void SetPlayerAlignAnim(ItemInfo& playerItem, const ItemInfo& interactedItem)
	{
		constexpr auto ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD = 6;

		auto& player = GetLaraInfo(playerItem);

		// Check if already aligning.
		if (player.Control.IsMoving)
			return;

		// Check water status.
		if (player.Control.WaterStatus == WaterStatus::Underwater ||
			player.Control.WaterStatus == WaterStatus::TreadWater)
		{
			return;
		}

		float dist = Vector3i::Distance(playerItem.Pose.Position, interactedItem.Pose.Position);
		bool doAlignAnim = ((dist - LARA_ALIGN_VELOCITY) > (LARA_ALIGN_VELOCITY * ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD));

		// Skip animating if very close to object.
		if (!doAlignAnim)
			return;

		SetAnimation(&playerItem, GetPlayerAlignAnim(playerItem.Pose, interactedItem.Pose));
		player.Control.HandStatus = HandStatus::Busy;
		player.Control.IsMoving = true;
		player.Control.Count.PositionAdjust = 0;
	}

	// TODO: Don't return bool.
	bool HandlePlayerInteraction(ItemInfo& playerItem, ItemInfo& interactedItem, const InteractionBasis& basis,
								 const PlayerInteractRoutine& interactRoutine)
	{
		auto& player = GetLaraInfo(playerItem);
		
		// Shift.
		if (true)
		{
			if (basis.TestInteraction(playerItem, interactedItem))
			{
				SetItemInteraction(playerItem, interactedItem, basis);
				interactRoutine(playerItem, interactedItem);

				return true;
			}
		}
		// TODO
		// Walk over.
		else
		{
			if (basis.TestInteraction(playerItem, interactedItem))
			{
				if (MoveLaraPosition(basis.PosOffset, &interactedItem, &playerItem))
				{
					interactRoutine(playerItem, interactedItem);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
				{
					player.Context.InteractedItem = interactedItem.Index;
				}
			}
			else if (player.Control.IsMoving && player.Context.InteractedItem == interactedItem.Index)
			{
				player.Control.IsMoving = false;
				player.Control.HandStatus = HandStatus::Free;
			}

			return true;
		}

		return false;
	}
}
