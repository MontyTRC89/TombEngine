#include "framework.h"
#include "Game/Hud/TargetHighlighter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/lara/lara_fire.h"
#include "Game/lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	static float GetTargetHighlightSize(float dist)
	{
		constexpr auto RANGE			  = BLOCK(10);
		constexpr auto HIGHLIGHT_SIZE_MAX = SCREEN_SPACE_RES.y * 0.25f;
		constexpr auto HIGHLIGHT_SIZE_MIN = HIGHLIGHT_SIZE_MAX / 5;

		auto distAlpha = dist / RANGE;
		return Lerp(HIGHLIGHT_SIZE_MAX, HIGHLIGHT_SIZE_MIN, distAlpha);
	}

	bool TargetHighlightData::IsOffscreen() const
	{
		float screenEdgeThreshold = ((Size * 2) * (RadiusScalar + 1.0f)) * SQRT_2; // TODO: Check.

		return (Position2D.x <= -screenEdgeThreshold ||
				Position2D.y <= -screenEdgeThreshold ||
				Position2D.x >= (SCREEN_SPACE_RES.x + screenEdgeThreshold) ||
				Position2D.y >= (SCREEN_SPACE_RES.y + screenEdgeThreshold));
	}

	void TargetHighlightData::Update(const Vector3& cameraPos, bool isActive)
	{
		constexpr auto INVALID_2D_POS		= Vector2(FLT_MAX);
		constexpr auto ROT					= ANGLE(2.0f);
		constexpr auto RADIUS_ALPHA_PRIMARY = 0.5f;
		constexpr auto MORPH_LERP_ALPHA		= 0.3f;
		constexpr auto ORIENT_LERP_ALPHA	= 0.1f;
		constexpr auto RADIUS_LERP_ALPHA	= 0.2f;

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
			// TODO: Inaccurate.
			short closestCardinalAngle = (Orientation2D / ANGLE(90.0f)) * ANGLE(90.0f);
			Orientation2D = (short)round(Lerp(Orientation2D, closestCardinalAngle, ORIENT_LERP_ALPHA));
		}

		// Update color.
		ColorTarget.w = IsActive ? ColorTarget.w : 0.0f;
		Color = Vector4::Lerp(Color, ColorTarget, MORPH_LERP_ALPHA);

		// Update size.
		if (IsActive)
		{
			float dist = Vector3::Distance(Camera.pos.ToVector3(), cameraPos);
			float sizeTarget = GetTargetHighlightSize(dist);
			Size = Lerp(Size, sizeTarget, MORPH_LERP_ALPHA);
		}
		else
		{
			Size = Lerp(Size, 0.0f, MORPH_LERP_ALPHA);
		}

		// Update radius scalar.
		RadiusScalarTarget = IsActive ? (IsPrimary ? RADIUS_ALPHA_PRIMARY : 0.0f) : RADIUS_SCALAR_MAX;
		RadiusScalar = Lerp(RadiusScalar, RadiusScalarTarget, RADIUS_LERP_ALPHA);
	}

	void TargetHighlighterController::SetPrimary(std::vector<int> entityIds)
	{
		// No highlights to set; return early.
		if (TargetHighlights.empty() || entityIds.empty())
			return;

		// Set highlights as primary.
		for (int entityID : entityIds)
		{
			// Matching highlight; continue.
			if (!TargetHighlights.count(entityID))
				continue;

			auto& highlight = TargetHighlights.at(entityID);
			highlight.IsPrimary = true;
		}
	}

	void TargetHighlighterController::SetPrimary(int entityID)
	{
		SetPrimary(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::SetPeripheral(std::vector<int> entityIds)
	{
		// No highlights to set; return early.
		if (TargetHighlights.empty() || entityIds.empty())
			return;

		// Set highlights as peripheral.
		for (int entityID : entityIds)
		{
			// Matching highlight; continue.
			if (!TargetHighlights.count(entityID))
				continue;

			auto& highlight = TargetHighlights.at(entityID);
			highlight.IsPrimary = false;
			highlight.ColorTarget = TargetHighlightData::COLOR_GRAY;
		}
	}

	void TargetHighlighterController::SetPeripheral(int entityID)
	{
		SetPeripheral(std::vector<int>{ entityID });
	}

	void TargetHighlighterController::Update(std::vector<int> entityIds)
	{
		// No highlights to update; return early.
		if (TargetHighlights.empty() && entityIds.empty())
			return;

		// Update active highlights.
		for (int entityID : entityIds)
		{
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Find highlight.
			auto it = TargetHighlights.find(entityID);

			// Update existing active highlight.
			if (it != TargetHighlights.end() && it->second.IsActive)
			{
				auto& highlight = it->second;
				highlight.Update(pos, true);
			}
			// Add new active highlight.
			else
			{
				AddTargetHighlight(entityID, pos);
			}
		}

		// Update inactive highlights.
		for (auto& [entityID, highlight] : TargetHighlights)
		{
			// Find absent highlights.
			auto it = std::find(entityIds.begin(), entityIds.end(), entityID);
			if (it != entityIds.end())
				continue;

			// Get position.
			const auto& item = g_Level.Items[entityID];
			auto pos = GetJointPosition(item, 0).ToVector3();

			// Update inactive highlight.
			highlight.Update(pos, false);
		}

		ClearInactiveTargetHighlights();
	}

	void TargetHighlighterController::Update(const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		auto entityIds = GetTargetEntityIds(playerItem);

		// Determine peripheral highlight entity IDs.
		auto peripheralEntityIds = entityIds;
		if (player.TargetEntity != nullptr)
		{
			// Set primary highlight.
			int primaryEntityID = player.TargetEntity->Index;
			SetPrimary(primaryEntityID);

			// Clear primary target entity ID from peripheral entity IDs.
			auto it = std::find(peripheralEntityIds.begin(), peripheralEntityIds.end(), primaryEntityID);
			if (it != peripheralEntityIds.end())
				peripheralEntityIds.erase(it);
		}

		// Set peripheral highlights.
		SetPeripheral(peripheralEntityIds);

		// Update highlights.
		Update(entityIds);
	}

	void TargetHighlighterController::Draw() const
	{
		DrawDebug();

		if (TargetHighlights.empty())
			return;

		for (const auto& [entityID, highlight] : TargetHighlights)
		{
			if (highlight.IsOffscreen())
				continue;

			g_Renderer.DrawSpriteIn2DSpace(
				ID_DEFAULT_SPRITES, 18,
				highlight.Position2D, highlight.Orientation2D, highlight.Color, Vector2(highlight.Size));
		}
	}

	void TargetHighlighterController::Clear()
	{
		TargetHighlights.clear();
	}

	std::vector<int> TargetHighlighterController::GetTargetEntityIds(const ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
			return {};

		auto& player = GetLaraInfo(playerItem);

		auto entityIds = std::vector<int>{};
		for (const auto* targetPtr : player.TargetList)
		{
			if (targetPtr == nullptr)
				continue;

			entityIds.push_back((int)targetPtr->Index);
		}

		return entityIds;
	}

	TargetHighlightData& TargetHighlighterController::GetNewTargetHighlight(int entityID)
	{
		constexpr auto COUNT_MAX = 16;

		// Clear smallest highlight if map is full.
		if (TargetHighlights.size() >= COUNT_MAX)
		{
			int key = 0;
			float smallestSize = INFINITY;
			
			for (auto& [entityID, highlight] : TargetHighlights)
			{
				if (highlight.Size < smallestSize)
				{
					key = entityID;
					smallestSize = highlight.Size;
				}
			}

			TargetHighlights.erase(key);
		}

		// Return new target highlight.
		TargetHighlights.insert({ entityID, {} });
		auto& highlight = TargetHighlights.at(entityID);
		highlight = {};
		return highlight;
	}

	void TargetHighlighterController::AddTargetHighlight(int entityID, const Vector3& pos)
	{
		constexpr auto SIZE_DEFAULT = SCREEN_SPACE_RES.x / 2;

		auto pos2D = g_Renderer.Get2DPosition(pos);
		if (!pos2D.has_value())
			return;

		// Create new target highlight.
		auto& highlight = GetNewTargetHighlight(entityID);

		highlight.IsActive = true;
		highlight.IsPrimary = false;
		highlight.Position2D = pos2D.value();
		highlight.Orientation2D = 0;
		highlight.Color = TargetHighlightData::COLOR_GRAY;
		highlight.Color.w = 0.0f;
		highlight.ColorTarget = TargetHighlightData::COLOR_GRAY;
		highlight.Size = SIZE_DEFAULT;
		highlight.RadiusScalar = TargetHighlightData::RADIUS_SCALAR_MAX;
		highlight.RadiusScalarTarget = 0.0f;
	}

	void TargetHighlighterController::ClearInactiveTargetHighlights()
	{
		for (auto it = TargetHighlights.begin(); it != TargetHighlights.end();)
		{
			// Clear highlight if inactive and size is near 0.
			if (!it->second.IsActive && (it->second.Size <= EPSILON))
			{
				it = TargetHighlights.erase(it);
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

		for (const auto& [entityID, highlight] : TargetHighlights)
			highlight.IsPrimary ? primaryCount++ : peripheralCount++;

		g_Renderer.PrintDebugMessage("Highlights: %d", TargetHighlights.size());
		g_Renderer.PrintDebugMessage("Primary highlights: %d", primaryCount);
		g_Renderer.PrintDebugMessage("Peripheral highlights: %d", peripheralCount);
	}
}
