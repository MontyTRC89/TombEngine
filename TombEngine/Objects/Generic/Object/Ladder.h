#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	// TODO
	class LadderObject
	{
	private:
		// Members
		std::vector<ItemInfo*> _attachedEntities = {};

		bool _isDoubleSided = false;

	public:
		// Constructors
		LadderObject() {};

		// Utilities
		void AttachEntity(ItemInfo& entity);
		void DetachEntity(ItemInfo& entity);
	};

	void CollideLadder(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
