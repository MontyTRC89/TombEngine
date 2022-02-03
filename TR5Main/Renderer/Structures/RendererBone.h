#pragma once
#include <vector>
#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererBone
	{
		Vector3 Translation;
		Matrix GlobalTransform;
		Matrix Transform;
		Vector3 GlobalTranslation;
		std::vector<RendererBone*> Children;
		RendererBone* Parent;
		int Index;
		Vector3 ExtraRotation;
		byte ExtraRotationFlags;

		RendererBone(int index)
		{
			Index = index;
			ExtraRotationFlags = 0;
			Translation = Vector3(0, 0, 0);
			ExtraRotation = Vector3(0, 0, 0);
		}
	};
}