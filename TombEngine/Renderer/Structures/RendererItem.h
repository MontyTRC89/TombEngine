#pragma once

#include "Game/room.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
	struct RendererItem
	{
		int ItemNumber;
		int ObjectNumber;

		Vector3 Position;
		Matrix World;
		Matrix Translation;
		Matrix Rotation;
		Matrix Scale;
		Matrix AnimationTransforms[MAX_BONES];

		Quaternion BoneOrientations[MAX_BONES];

		int RoomNumber = NO_VALUE;
		int PrevRoomNumber = NO_VALUE;
		Vector4 Color;
		Vector4 AmbientLight;
		std::vector<RendererLight*> LightsToDraw;
		float LightFade;

		std::vector<int> MeshIndex;

		bool DoneAnimations;
	};
}
