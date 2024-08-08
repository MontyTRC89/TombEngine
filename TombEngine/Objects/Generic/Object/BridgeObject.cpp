#include "framework.h"
#include "Objects/Generic/Object/BridgeObject.h"

#include "Game/items.h"
#include "Objects/Generic/Object/Pushable/PushableInfo.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"

namespace TEN::Entities::Generic
{
	const BridgeObject& GetBridgeObject(const ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			const auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}

	BridgeObject& GetBridgeObject(ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}
}
