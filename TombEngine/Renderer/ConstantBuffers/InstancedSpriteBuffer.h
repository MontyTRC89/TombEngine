#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector4;

	struct alignas(16) InstancedSprite
	{
		Matrix World;
		Vector4 Color;
		float IsBillboard;
		float IsSoftParticle;
		Vector2 SpritesPadding;
	};

	struct alignas(16) CInstancedSpriteBuffer
	{
		InstancedSprite Sprites[INSTANCED_SPRITES_BUCKET_SIZE];
	};
}