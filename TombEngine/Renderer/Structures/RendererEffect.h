#pragma once
#include <SimpleMath.h>
#include "Renderer/Structures/RendererMesh.h"
#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererEffect
	{
		int ObjectNumber;
		int RoomNumber;
		Vector3 Position;
		Matrix World;
		Matrix Translation;
		Matrix Rotation;
		Matrix Scale;
		Vector4 Color;
		Vector4 AmbientLight;
		RendererMesh* Mesh;
		std::vector<RendererLight*> LightsToDraw;

		Vector3 OldPosition;
		Matrix OldWorld;
		Matrix OldTranslation;
		Matrix OldRotation;
		Matrix OldScale;

		Vector3 InterpolatedPosition;
		Matrix InterpolatedWorld;
		Matrix InterpolatedTranslation;
		Matrix InterpolatedRotation;
		Matrix InterpolatedScale;
	};
}
