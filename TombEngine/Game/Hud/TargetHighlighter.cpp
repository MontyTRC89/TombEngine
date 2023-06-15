#include "framework.h"
#include "Game/Hud/TargetHighlighter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/lara/lara_fire.h"
#include "Game/lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	// TODO: Not working?
	bool CrosshairData::IsOffscreen() const
	{
		float screenEdgeThreshold = GetRadius();

		return (Position2D.x <= -screenEdgeThreshold ||
				Position2D.y <= -screenEdgeThreshold ||
				Position2D.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	float CrosshairData::GetRadius() const
	{
		return ((Size / 2) * (RadiusScale * PulseScale));
	}

	Vector2 CrosshairData::Get2DPositionOffset(short orientOffset2D) const
	{
		constexpr auto ANGLE_OFFSET = ANGLE(-360.0f / (SEGMENT_COUNT * 2));

		float offsetDist = GetRadius();
		auto relPosOffset2D = Vector2(offsetDist, 0.0f);
		auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(Orientation2D + orientOffset2D + ANGLE_OFFSET));

		auto posOffset2D = Vector2::Transform(relPosOffset2D, rotMatrix);
		return GetAspectCorrect2DPosition(posOffset2D);
	}

	void CrosshairData::SetPrimary()
	{
		IsPrimary = true;
		ColorTarget = COLOR_GREEN;
	}

	void CrosshairData::SetPeripheral()
	{
		IsPrimary = false;
		ColorTarget = COLOR_GRAY;
	}

	static float GetCrosshairSize(float cameraDist)
	{
		constexpr auto RANGE			  = BLOCK(10);
		constexpr auto CROSSHAIR_SIZE_MAX = SCREEN_SPACE_RES.y * 0.15f;
		constexpr auto CROSSHAIR_SIZE_MIN = CROSSHAIR_SIZE_MAX / 4;

		auto alpha = cameraDist / RANGE;
		return Lerp(CROSSHAIR_SIZE_MAX, CROSSHAIR_SIZE_MIN, alpha);
	}

	void CrosshairData::Update(const Vector3& cameraPos, bool doPulse, bool isActive)
	{
		constexpr auto INVALID_2D_POS		   = Vector2(FLT_MAX);
		constexpr auto ROT					   = ANGLE(2.0f);
		constexpr auto ALIGN_ANGLE_STEP		   = ANGLE(360.0f / SEGMENT_COUNT);
		constexpr auto SIZE_SCALE_PERIPHERAL   = 0.5f;
		constexpr auto RADIUS_SCALE_PRIMARY	   = 0.5f * SQRT_2;
		constexpr auto RADIUS_SCALE_PERIPHERAL = 0.25f * SQRT_2;
		constexpr auto PULSE_SCALE_MAX		   = 1.3f;
		constexpr auto MORPH_LERP_ALPHA		   = 0.3f;
		constexpr auto ORIENT_LERP_ALPHA	   = 0.1f;
		constexpr auto RADIUS_LERP_ALPHA	   = 0.2f;

		// Update active status.
		IsActive = isActive;

		// Update 2D position.
		auto pos2D = g_Renderer.Get2DPosition(cameraPos);
		Position2D = pos2D.has_value() ? pos2D.value() : INVALID_2D_POS;

		// Update 2D orientation.
		if (IsPrimary)
		{
			Orientation2D += ROT;
		}
		else
		{
			short closestAlignAngle = (Orientation2D / ALIGN_ANGLE_STEP) * ALIGN_ANGLE_STEP;
			Orientation2D = (short)round(Lerp(Orientation2D, closestAlignAngle, ORIENT_LERP_ALPHA));
		}

		// Update color.
		if (!IsActive)
		{
			ColorTarget = CrosshairData::COLOR_GRAY;
			ColorTarget.w = 0.0f;
		}
		Color = Vector4::Lerp(Color, ColorTarget, MORPH_LERP_ALPHA);

		// Update size.
		if (IsActive)
		{
			float cameraDist = Vector3::Distance(Camera.pos.ToVector3(), cameraPos);

			float sizeTarget = GetCrosshairSize(cameraDist);
			if (!IsPrimary)
				sizeTarget *= SIZE_SCALE_PERIPHERAL;

			Size = Lerp(Size, sizeTarget, MORPH_LERP_ALPHA);
		}
		else
		{
			Size = Lerp(Size, 0.0f, MORPH_LERP_ALPHA);
		}

		// Update radius scale.
		float radiusScaleTarget = IsPrimary ? RADIUS_SCALE_PRIMARY : RADIUS_SCALE_PERIPHERAL;
		if (!IsActive)
			radiusScaleTarget = RADIUS_SCALE_PERIPHERAL;

		RadiusScale = Lerp(RadiusScale, radiusScaleTarget, RADIUS_LERP_ALPHA);

		// Update pulse scale.
		PulseScale = doPulse ? PULSE_SCALE_MAX : Lerp(PulseScale, 1.0f, MORPH_LERP_ALPHA);

		// Update segments.
		for (auto& segment : Segments)
			segment.PosOffset2D = Get2DPositionOffset(segment.OrientOffset2D);
	}

	void TargetHighlighterController::Update(const std::vector<int>& entityIds)
	{
		constexpr auto TARGET_BONE_ID = 0;

		// No crosshairs to update; return early.
		if (Crosshairs.empty() && entityIds.empty())
			return;

		// Update active crosshairs.
		for (int entityID : entityIds)
		{
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, TARGET_BONE_ID).ToVector3();

			// Update existing active crosshair.
			auto it = Crosshairs.find(entityID);
			if (it != Crosshairs.end())
			{
				auto& crosshair = it->second;
				if (crosshair.IsActive)
				{
					crosshair.Update(pos, item.HitStatus, true);
					continue;
				}
			}

			// Add new active crosshair.
			AddCrosshair(entityID, pos);
		}

		// Update inactive crosshairs.
		for (auto& [entityID, crosshair] : Crosshairs)
		{
			// Find crosshairs at absent entity ID keys.
			if (Contains(entityIds, entityID))
				continue;

			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Update inactive crosshair.
			crosshair.Update(pos, item.HitStatus, false);
		}

		ClearInactiveCrosshairs();
	}

	void TargetHighlighterController::Update(const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		// Loop over player targets.
		auto entityIds = std::vector<int>{};
		for (const auto* entityPtr : player.TargetList)
		{
			if (entityPtr == nullptr)
				continue;

			// Collect entity ID.
			entityIds.push_back(entityPtr->Index);

			// Find crosshair at entity ID key.
			auto it = Crosshairs.find(entityPtr->Index);
			if (it == Crosshairs.end())
				continue;

			// Set crosshair as primary or peripheral.
			auto& crosshair = it->second;
			if (player.TargetEntity != nullptr &&
				entityPtr->Index == player.TargetEntity->Index)
			{
				crosshair.SetPrimary();
			}
			else
			{
				crosshair.SetPeripheral();
			}
		}

		// Update crosshairs.
		Update(entityIds);
	}

	void TargetHighlighterController::Draw() const
	{
		constexpr auto SPRITE_STATIC_ELEMENT_INDEX	= 0;
		constexpr auto SPRITE_SEGMENT_ELEMENT_INDEX = 1;

		DrawDebug();

		if (Crosshairs.empty())
			return;

		for (const auto& [entityID, crosshair] : Crosshairs)
		{
			if (crosshair.IsOffscreen())
				continue;

			g_Renderer.DrawSpriteIn2DSpace(
				ID_CROSSHAIR, SPRITE_STATIC_ELEMENT_INDEX,
				crosshair.Position2D, crosshair.Orientation2D,
				crosshair.Color, Vector2(crosshair.Size));

			if (crosshair.RadiusScale > EPSILON)
			{
				for (const auto& segment : crosshair.Segments)
				{
					g_Renderer.DrawSpriteIn2DSpace(
						ID_CROSSHAIR, SPRITE_SEGMENT_ELEMENT_INDEX,
						crosshair.Position2D + segment.PosOffset2D, crosshair.Orientation2D + segment.OrientOffset2D,
						crosshair.Color, Vector2(crosshair.Size / 2));
				}
			}
		}
	}

	void TargetHighlighterController::Clear()
	{
		*this = {};
	}

	CrosshairData& TargetHighlighterController::GetNewCrosshair(int entityID)
	{
		constexpr auto COUNT_MAX = 16;

		// Map is full; clear smallest crosshair.
		if (Crosshairs.size() >= COUNT_MAX)
		{
			int key = 0;
			float smallestSize = INFINITY;
			
			for (auto& [entityID, crosshair] : Crosshairs)
			{
				if (crosshair.Size < smallestSize)
				{
					key = entityID;
					smallestSize = crosshair.Size;
				}
			}

			Crosshairs.erase(key);
		}

		// Return new crosshair.
		Crosshairs.insert({ entityID, {} });
		auto& crosshair = Crosshairs.at(entityID);
		crosshair = {};
		return crosshair;
	}

	// TODO: If crosshair happens to be in view upon spawn, first frame is garbage.
	void TargetHighlighterController::AddCrosshair(int entityID, const Vector3& pos)
	{
		constexpr auto SIZE_START		  = SCREEN_SPACE_RES.x / 2;
		constexpr auto RADIUS_SCALE_START = 1.5f * SQRT_2;
		constexpr auto ANGLE_STEP		  = ANGLE(360.0f / CrosshairData::SEGMENT_COUNT);

		auto pos2D = g_Renderer.Get2DPosition(pos);
		if (!pos2D.has_value())
			return;

		// Create new crosshair.
		auto& crosshair = GetNewCrosshair(entityID);

		crosshair.IsActive = true;
		crosshair.IsPrimary = false;
		crosshair.Position2D = pos2D.value();
		crosshair.Orientation2D = 0;
		crosshair.Color = CrosshairData::COLOR_GRAY;
		crosshair.Color.w = 0.0f;
		crosshair.ColorTarget = CrosshairData::COLOR_GRAY;
		crosshair.Size = SIZE_START;
		crosshair.RadiusScale = RADIUS_SCALE_START;
		crosshair.PulseScale = 1.0f;

		// Initialize segments.
		short angleOffset = 0;
		for (auto& segment : crosshair.Segments)
		{
			segment.PosOffset2D = crosshair.Get2DPositionOffset(segment.OrientOffset2D);
			segment.OrientOffset2D = angleOffset;

			angleOffset += ANGLE_STEP;
		}
	}

	void TargetHighlighterController::ClearInactiveCrosshairs()
	{
		for (auto it = Crosshairs.begin(); it != Crosshairs.end();)
		{
			const auto& crosshair = it->second;
			(!crosshair.IsActive && crosshair.Size <= EPSILON) ?
				(it = Crosshairs.erase(it)) : ++it;
		}
	}

	void TargetHighlighterController::DrawDebug() const
	{
		unsigned int primaryCount = 0;
		unsigned int peripheralCount = 0;
		unsigned int visibleCount = 0;

		for (const auto& [entityID, crosshair] : Crosshairs)
		{
			crosshair.IsPrimary ? primaryCount++ : peripheralCount++;
			
			if (!crosshair.IsOffscreen())
				visibleCount++;
		}

		g_Renderer.PrintDebugMessage("TARGET HIGHLIGHTER DEBUG");
		g_Renderer.PrintDebugMessage("Primary crosshairs: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral crosshairs: %d", peripheralCount);
		g_Renderer.PrintDebugMessage("Visible crosshairs: %d", peripheralCount);
	}
}
