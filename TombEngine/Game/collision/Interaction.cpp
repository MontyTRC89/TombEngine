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

namespace TEN::Collision::Interaction
{
	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const std::optional<EulerAngles>& orientOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
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

	InteractionBasis::InteractionBasis(const std::optional<EulerAngles>& orientOffset, const BoundingOrientedBox& box, const OrientConstraintPair& orientConstraint)
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

	InteractionBasis::InteractionBasis(const Vector3i& posOffset, const EulerAngles& orientOffset, const GameBoundingBox& box, const OrientConstraintPair& orientConstraint)
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

	bool TestInteraction(const ItemInfo& interactor, const ItemInfo& interactable, const InteractionBasis& basis, std::optional<BoundingOrientedBox> expansionBox)
	{
		DrawDebug(interactable, basis);

		// 1) Avoid overriding active offset blend.
		if (interactor.OffsetBlend.IsActive)
			return false;

		// TODO: Currently unreliable because IteractedItemNumber is frequently not reset after completed interactions.
		// 2) Avoid overriding active player interactions.
		/*if (interactor.IsLara())
		{
			const auto& player = GetLaraInfo(interactor);
			if (player.Context.InteractedItem != NO_VALUE)
				return false;
		}*/

		// 3) Test if interactor's orientation is within interaction constraint.
		auto deltaOrient = interactor.Pose.Orientation - interactable.Pose.Orientation;
		if (deltaOrient.x < basis.OrientConstraint.first.x || deltaOrient.x > basis.OrientConstraint.second.x ||
			deltaOrient.y < basis.OrientConstraint.first.y || deltaOrient.y > basis.OrientConstraint.second.y ||
			deltaOrient.z < basis.OrientConstraint.first.z || deltaOrient.z > basis.OrientConstraint.second.z)
		{
			return false;
		}

		// Calculate box-relative position.
		auto deltaPos = (interactor.Pose.Position - interactable.Pose.Position).ToVector3();
		auto invRotMatrix = interactable.Pose.Orientation.ToRotationMatrix().Invert();
		auto relPos = Vector3::Transform(deltaPos, invRotMatrix);

		// Calculate box min and max.
		auto box = expansionBox.has_value() ? Geometry::GetExpandedBoundingOrientedBox(basis.Box, *expansionBox) : basis.Box;
		auto max = box.Center + box.Extents;
		auto min = box.Center - box.Extents;

		// 4) Test if interactor is inside interaction box.
		if (relPos.x < min.x || relPos.x > max.x ||
			relPos.y < min.y || relPos.y > max.y ||
			relPos.z < min.z || relPos.z > max.z)
		{
			return false;
		}

		// TODO: Not working.
		/*box = expansionBox.has_value() ? Geometry::GetExpandedBoundingOrientedBox(basis.Box, *expansionBox) : basis.Box;
		box.Center = Vector3::Transform(box.Center, box.Orientation) + interactible.Pose.Position.ToVector3();

		// 4) Test if interactor's position intersects interaction box.
		if (!Geometry::IsPointInBox(deltaPos, box))
			return false;*/

		return true;
	}

	static void SetLatchInteraction(ItemInfo& interactor, ItemInfo& interactable, const InteractionBasis& basis, const InteractionRoutine& routine)
	{
		constexpr auto OFFSET_BLEND_ALPHA = 0.2f;

		// Calculate targets.
		auto targetPos = Geometry::TranslatePoint(interactable.Pose.Position, interactable.Pose.Orientation, basis.PosOffset);
		auto targetOrient = basis.OrientOffset.has_value() ? (interactable.Pose.Orientation + *basis.OrientOffset) : interactor.Pose.Orientation;

		// Calculate absolute offsets.
		auto absPosOffset = (targetPos - interactor.Pose.Position).ToVector3();
		auto absOrientOffset = targetOrient - interactor.Pose.Orientation;

		// Set interactor parameters.
		interactor.Animation.Velocity = Vector3::Zero;
		interactor.OffsetBlend.SetLogarithmic(absPosOffset, absOrientOffset, OFFSET_BLEND_ALPHA);

		// Set player parameters.
		if (interactor.IsLara())
		{
			auto& player = GetLaraInfo(interactor);

			player.Control.TurnRate = 0;
			player.Control.HandStatus = HandStatus::Busy;
			player.Context.InteractedItem = interactable.Index;
		}

		// Call interaction routine.
		routine(interactor, interactable);
	}
	
	static void SetWalkInteraction(ItemInfo& interactor, ItemInfo& interactable, const InteractionBasis& basis, const InteractionRoutine& routine)
	{
		// Set player parameters.
		if (interactor.IsLara())
		{
			auto& player = GetLaraInfo(interactor);

			// FAILSAFE.
			if (player.Control.WaterStatus != WaterStatus::Dry &&
				player.Control.WaterStatus != WaterStatus::Wade)
			{
				SetLatchInteraction(interactor, interactable, basis, routine);
				TENLog("SetWalkInteraction(): player not grounded. Setting latch interaction instead.", LogLevel::Warning);
				return;
			}

			player.Control.TurnRate = 0;
			player.Control.HandStatus = HandStatus::Busy;
			player.Context.InteractedItem = interactable.Index;
			player.Context.WalkInteraction.Basis = basis;
			player.Context.WalkInteraction.Routine = routine;
			return;
		}

		// FAILSAFE.
		SetLatchInteraction(interactor, interactable, basis, routine);
		TENLog("SetWalkInteraction(): non-player passed as interactor. Setting latch interaction instead.", LogLevel::Warning);
	}

	void SetInteraction(ItemInfo& interactor, ItemInfo& interactable, const InteractionBasis& basis, const InteractionRoutine& routine, InteractionType type)
	{
		constexpr auto WALK_DIST_MIN = BLOCK(0.05f);

		switch (type)
		{
		default:
		case InteractionType::Latch:
			SetLatchInteraction(interactor, interactable, basis, routine);
			break;

		case InteractionType::Walk:
			float dist = Vector3i::Distance(interactor.Pose.Position, interactable.Pose.Position);
			if (dist > WALK_DIST_MIN)
			{
				SetWalkInteraction(interactor, interactable, basis, routine);
			}
			else
			{
				SetLatchInteraction(interactor, interactable, basis, routine);
			}

			break;
		}
	}

	void DrawDebug(const ItemInfo& interactable, const InteractionBasis& basis)
	{
		constexpr auto COLL_BOX_COLOR	  = Color(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr auto INTERACT_BOX_COLOR = Color(0.0f, 1.0f, 1.0f, 1.0f);

		// Draw collision box.
		auto collBox = GameBoundingBox(&interactable).ToBoundingOrientedBox(interactable.Pose);
		DrawDebugBox(collBox, COLL_BOX_COLOR);

		auto rotMatrix = interactable.Pose.Orientation.ToRotationMatrix();
		auto relCenter = Vector3::Transform(basis.Box.Center, rotMatrix);
		auto orient = interactable.Pose.Orientation.ToQuaternion();

		// Draw interaction box.
		auto interactBox = BoundingOrientedBox(interactable.Pose.Position.ToVector3() + relCenter, basis.Box.Extents, orient);
		DrawDebugBox(interactBox, INTERACT_BOX_COLOR);
	}
}
