#pragma once
#include <vector>
#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererBone
	{
		int Index;
		Vector3 GlobalTranslation;
		Vector3 Translation;
		Matrix GlobalTransform;
		Matrix Transform;
		Quaternion ExtraRotation = Quaternion::Identity;
		RendererBone* Parent;
		std::vector<RendererBone*> Children;
		byte ExtraRotationFlags;

		RendererBone(int index)
		{
			Index = index;
			Translation = Vector3::Zero;
			ExtraRotation = Quaternion::Identity;
			ExtraRotationFlags = 0;
		}
	};
}
