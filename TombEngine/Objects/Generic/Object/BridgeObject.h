#pragma once

class Vector3i;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	// TODO: Bridge properties.
	class BridgeObject
	{
	public:
		// Routines
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetFloorHeight	  = nullptr;
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetCeilingHeight = nullptr;
		std::function<int(const ItemInfo& item)> GetFloorBorder	  = nullptr;
		std::function<int(const ItemInfo& item)> GetCeilingBorder = nullptr;
	};

	const BridgeObject& GetBridgeObject(const ItemInfo& item);
	BridgeObject&		GetBridgeObject(ItemInfo& item);
}
