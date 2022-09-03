#pragma once
#include <SimpleMath.h>

namespace TEN
{
	namespace Renderer
	{
		using DirectX::SimpleMath::Matrix;
		using DirectX::SimpleMath::Vector4;

		struct alignas(4) InstancedSprite
		{
			Matrix World;
			Vector4 Color;
			float IsBillboard;
			Vector3 Padding;
		};

		struct alignas(4) CInstancedSpriteBuffer
		{
			InstancedSprite Sprites[128];
		};
	}
}