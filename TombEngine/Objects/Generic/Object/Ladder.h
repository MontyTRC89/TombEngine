#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	class LadderObject
	{
	private:
		// Members

		bool _isDoubleSided = false;

	public:
		std::vector<ItemInfo*> _attachedItems = {};
		Pose PrevPose = Pose::Zero;

		// Constructors
		LadderObject(bool isDoubleSided);

		// Inquirers
		bool IsDoubleSided() const;

		// Utilities
		void AttachItem(ItemInfo& item);
		void DetachItem(ItemInfo& item);
	};

	void InitializeLadder(short itemNumber);
	void ControlLadder(short itemNumber);
	void CollideLadder(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
