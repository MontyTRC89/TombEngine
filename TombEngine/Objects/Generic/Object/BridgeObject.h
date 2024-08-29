#pragma once
#include "Game/collision/Attractor.h"

enum GAME_OBJECT_ID : short;
class Vector3i;
struct ItemInfo;

using namespace TEN::Collision::Attractor;

namespace TEN::Entities::Generic
{
	class BridgeObject
	{
	private:
		// Members

		AttractorObject _attractor = AttractorObject();

	public:
		// Routines

		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetFloorHeight	  = nullptr;
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetCeilingHeight = nullptr;
		std::function<int(const ItemInfo& item)> GetFloorBorder	  = nullptr;
		std::function<int(const ItemInfo& item)> GetCeilingBorder = nullptr;

		// Getters

		AttractorObject& GetAttractor();

		// Utilities

		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);

	private:
		std::vector<Vector3> GetAttractorPoints(const ItemInfo& item) const;
	};

	const BridgeObject& GetBridgeObject(const ItemInfo& item);
	BridgeObject&		GetBridgeObject(ItemInfo& item);
}
