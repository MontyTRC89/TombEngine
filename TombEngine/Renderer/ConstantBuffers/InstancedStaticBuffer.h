#pragma once
#include <SimpleMath.h>
#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/Renderer11.h"
#include "Renderer/ConstantBuffers/ShaderLight.h"

namespace TEN::Renderer
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector4;

	constexpr int INSTANCED_STATIC_MESH_BUCKET_SIZE = 32;
	
	struct RendererStatic;
	struct RendererMesh;

	struct RendererInstancedStaticMeshData
	{
		RendererStatic* Static;
		RendererMesh* Mesh;
		Matrix World;
		Vector4 Color;
		Vector4 Ambient;
		int LightMode;
		int NumLights;
		ShaderLight Lights[8];
	};

	struct alignas(16) InstancedStaticMesh
	{
		Matrix World;
		Vector4 Color;
		Vector4 Ambient;
		ShaderLight Lights[8];
		int NumLights;
		int LightMode;
		int Padding1;
		int Padding2;
	};

	struct alignas(16) CInstancedStaticMeshBuffer
	{
		InstancedStaticMesh StaticMeshes[INSTANCED_STATIC_MESH_BUCKET_SIZE];
	};
}