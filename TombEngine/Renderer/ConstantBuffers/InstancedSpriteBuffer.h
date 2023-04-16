#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector4;

	struct alignas(16) InstancedSprite
	{
		Matrix World;
		Vector4 UV[2];
		Vector4 Color;
		float IsBillboard;
		float IsSoftParticle;
	};

	struct alignas(16) CInstancedSpriteBuffer
	{
		InstancedSprite Sprites[INSTANCED_SPRITES_BUCKET_SIZE];
	};
}