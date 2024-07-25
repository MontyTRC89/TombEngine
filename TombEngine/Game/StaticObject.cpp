#include "framework.h"

#include "Game/Setup.h"

const StaticAsset& StaticObject::GetAsset() const
{
	return GetStaticAsset(AssetID);
}
