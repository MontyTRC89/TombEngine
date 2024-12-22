#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace TEN::Renderer::Structures
{
	enum class ShaderType
	{
		Pixel,
		Vertex,
		PixelAndVertex,
		Compute
	};

	struct RendererPixelShaderAndBlob
	{
		ComPtr<ID3D11PixelShader> Shader = nullptr;
		ComPtr<ID3D10Blob> Blob;
	};

	struct RendererVertexShaderAndBlob
	{
		ComPtr<ID3D11VertexShader> Shader = nullptr;
		ComPtr<ID3D10Blob> Blob;
	};

	struct RendererComputeShaderAndBlob
	{
		ComPtr<ID3D11ComputeShader> Shader = nullptr;
		ComPtr<ID3D10Blob> Blob;
	};

	struct RendererShader
	{
		RendererPixelShaderAndBlob   Pixel;
		RendererVertexShaderAndBlob  Vertex;
		RendererComputeShaderAndBlob Compute;
	};
}