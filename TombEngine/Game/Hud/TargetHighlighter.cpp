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
	Vector2 CrosshairData::Get2DPositionOffset(short segmentOrient2D) const
	{
		constexpr auto ANGLE_OFFSET = ANGLE(-45.0f);

		float offsetDist = (Size / 2) * RadiusScalar;
		auto posOffset2D = Vector2(offsetDist, 0.0f);
		auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(Orientation2D + segmentOrient2D + ANGLE_OFFSET));

		return GetAspectCorrect2DPosition(Vector2::Transform(posOffset2D, rotMatrix));
	}

	bool CrosshairData::IsOffscreen() const
	{
		float screenEdgeThreshold = ((Size / 2) * SQRT_2) * RadiusScalar;

		return (Position2D.x <= -screenEdgeThreshold ||
				Position2D.y <= -screenEdgeThreshold ||
				Position2D.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	static float GetCrosshairSize(float cameraDist)
	{
		constexpr auto RANGE			  = BLOCK(10);
		constexpr auto CROSSHAIR_SIZE_MAX = SCREEN_SPACE_RES.y * 0.15f;
		constexpr auto CROSSHAIR_SIZE_MIN = CROSSHAIR_SIZE_MAX / 5;

		auto alpha = cameraDist / RANGE;
		return Lerp(CROSSHAIR_SIZE_MAX, CROSSHAIR_SIZE_MIN, alpha);
	}

	void CrosshairData::Update(const Vector3& cameraPos, bool doPulse, bool isActive)
	{
		constexpr auto INVALID_2D_POS			= Vector2(FLT_MAX);
		constexpr auto ROT						= ANGLE(2.0f);
		constexpr auto CARDINAL_ANGLE_STEP		= ANGLE(90.0f);
		constexpr auto SCALE_PERIPHERAL			= 0.7f;
		constexpr auto SCALE_PULSE				= 1.1f;
		constexpr auto RADIUS_SCALAR_PRIMARY	= 1.0f * SQRT_2;
		constexpr auto RADIUS_SCALAR_PERIPHERAL = 0.5f * SQRT_2;
		constexpr auto MORPH_LERP_ALPHA			= 0.3f;
		constexpr auto ORIENT_LERP_ALPHA		= 0.1f;
		constexpr auto RADIUS_LERP_ALPHA		= 0.2f;

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
			short closestCardinalAngle = (Orientation2D / CARDINAL_ANGLE_STEP) * CARDINAL_ANGLE_STEP;
			Orientation2D = (short)round(Lerp(Orientation2D, closestCardinalAngle, ORIENT_LERP_ALPHA));
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
				sizeTarget *= SCALE_PERIPHERAL;

			Size = Lerp(Size, sizeTarget, MORPH_LERP_ALPHA);
			if (doPulse)
				Size *= SCALE_PULSE;
		}
		else
		{
			Size = Lerp(Size, 0.0f, MORPH_LERP_ALPHA);
		}

		// Update radius scalar.
		float radiusScalarTarget = IsPrimary ? RADIUS_SCALAR_PRIMARY : RADIUS_SCALAR_PERIPHERAL;
		if (!IsActive)
			radiusScalarTarget = RADIUS_SCALAR_PERIPHERAL;

		RadiusScalar = Lerp(RadiusScalar, radiusScalarTarget, RADIUS_LERP_ALPHA);

		// Update segments.
		for (auto& segment : Segments)
			segment.PosOffset2D = Get2DPositionOffset(segment.OrientOffset2D);
	}

	void TargetHighlighterController::SetPrimary(int entityID)
	{
		auto it = Crosshairs.find(entityID);
		if (it == Crosshairs.end())
			return;

		auto& crosshair = it->second;
		crosshair.IsPrimary = true;
		crosshair.ColorTarget = CrosshairData::COLOR_GREEN;
	}

	void TargetHighlighterController::SetPeripheral(int entityID)
	{
		auto it = Crosshairs.find(entityID);
		if (it == Crosshairs.end())
			return;

		auto& crosshair = it->second;
		crosshair.IsPrimary = false;
		crosshair.ColorTarget = CrosshairData::COLOR_GRAY;
	}

	void TargetHighlighterController::Update(std::vector<int> entityIds)
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
			auto it = std::find(entityIds.begin(), entityIds.end(), entityID);
			if (it != entityIds.end())
				continue;

			// Get position.
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

		// Loop through player targets.
		auto entityIds = std::vector<int>{};
		for (const auto* entityPtr : player.TargetList)
		{
			if (entityPtr == nullptr)
				continue;

			int entityID = entityPtr->Index;

			// Set crosshair as primary or peripheral.
			if (player.TargetEntity != nullptr && entityID == player.TargetEntity->Index)
			{
				SetPrimary(entityID);
			}
			else
			{
				SetPeripheral(entityID);
			}

			// Collect entity ID.
			entityIds.push_back(entityPtr->Index);
		}

		// Update crosshairs.
		Update(entityIds);
	}

	void TargetHighlighterController::Draw() const
	{
		DrawDebug();

		if (Crosshairs.empty())
			return;

		for (const auto& [entityID, crosshair] : Crosshairs)
		{
			if (crosshair.IsOffscreen())
				continue;

			for (const auto& segment : crosshair.Segments)
			{
				g_Renderer.DrawSpriteIn2DSpace(
					ID_CROSSHAIR_SEGMENT, 0,
					crosshair.Position2D + segment.PosOffset2D, crosshair.Orientation2D + segment.OrientOffset2D,
					crosshair.Color, Vector2(crosshair.Size / 2));
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

		// Clear smallest crosshair if map is full.
		if (Crosshairs.size() >= COUNT_MAX)
		{
			int entityIDKey = 0;
			float smallestSize = INFINITY;
			
			for (auto& [entityID, crosshair] : Crosshairs)
			{
				if (crosshair.Size < smallestSize)
				{
					entityIDKey = entityID;
					smallestSize = crosshair.Size;
				}
			}

			Crosshairs.erase(entityIDKey);
		}

		// Return new crosshair.
		Crosshairs.insert({ entityID, {} });
		auto& crosshair = Crosshairs.at(entityID);
		crosshair = {};
		return crosshair;
	}

	void TargetHighlighterController::AddCrosshair(int entityID, const Vector3& pos)
	{
		constexpr auto SIZE_START		   = SCREEN_SPACE_RES.x / 2;
		constexpr auto RADIUS_SCALAR_START = 1.5f * SQRT_2;
		constexpr auto ANGLE_STEP		   = ANGLE(360.0f / CrosshairData::SEGMENT_COUNT);

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
		crosshair.RadiusScalar = RADIUS_SCALAR_START;

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
			// Clear crosshair if inactive and size is near 0.
			if (!it->second.IsActive && it->second.Size <= EPSILON)
			{
				it = Crosshairs.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void TargetHighlighterController::DrawDebug() const
	{
		unsigned int primaryCount = 0;
		unsigned int peripheralCount = 0;

		for (const auto& [entityID, crosshair] : Crosshairs)
			crosshair.IsPrimary ? primaryCount++ : peripheralCount++;

		g_Renderer.PrintDebugMessage("Crosshairs: %d", Crosshairs.size());
		g_Renderer.PrintDebugMessage("Primary crosshairs: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral crosshairs: %d", peripheralCount);
	}
}
