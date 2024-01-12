#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererLight.h"
#include "Game/room.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

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

		int RoomNumber = NO_ROOM;
		int PrevRoomNumber = NO_ROOM;
		Vector4 Color;
		Vector4 AmbientLight;
		std::vector<RendererLight*> LightsToDraw;
		float LightFade;

		std::vector<int> MeshIndex;

		bool DoneAnimations;
	};
}
