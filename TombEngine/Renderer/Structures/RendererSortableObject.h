#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererBucket.h"
#include "Renderer/Structures/RendererMesh.h"
#include "Renderer/Structures/RendererItem.h"
#include "Renderer/Structures/RendererStatic.h"
#include "Renderer/Structures/RendererRoom.h"
#include "Renderer/Structures/RendererEffect.h"
#include "Renderer/Structures/RendererSpriteToDraw.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererSortableObject
	{
		RendererObjectType ObjectType;
		int Distance;
		Vector3 Centre;
		RendererMesh* Mesh;
		RendererBucket* Bucket;
		RendererRoom* Room;
		RendererStatic* Static;
		RendererItem* Item;
		RendererEffect* Effect;
		RendererSpriteToDraw* Sprite;
	};
}