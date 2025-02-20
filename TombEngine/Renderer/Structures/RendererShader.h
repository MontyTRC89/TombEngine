#pragma once
#include <wrl/client.h>
#include <utility>

using Microsoft::WRL::ComPtr;

namespace TEN::Renderer::Structures
{
	enum class ShaderType
	{
		Pixel,
		Vertex,
		PixelAndVertex,
		Compute,
		Hull,
		Domain
	};

	struct RendererPixelShaderAndBlob
	{
		ComPtr<ID3D11PixelShader> Shader = nullptr;
		ComPtr<ID3D10Blob>		  Blob	 = nullptr;
	};

	struct RendererVertexShaderAndBlob
	{
		ComPtr<ID3D11VertexShader> Shader = nullptr;
		ComPtr<ID3D10Blob>		   Blob	  = nullptr;
	};

	struct RendererComputeShaderAndBlob
	{
		ComPtr<ID3D11ComputeShader> Shader = nullptr;
		ComPtr<ID3D10Blob>			Blob   = nullptr;
	};

	struct RendererHullShaderAndBlob
	{
		ComPtr<ID3D11HullShader>  Shader = nullptr;
		ComPtr<ID3D10Blob>		  Blob = nullptr;
	};

	struct RendererDomainShaderAndBlob
	{
		ComPtr<ID3D11DomainShader> Shader = nullptr;
		ComPtr<ID3D10Blob>		   Blob = nullptr;
	};

	struct RendererShader
	{
		RendererPixelShaderAndBlob	 Pixel	 = {};
		RendererVertexShaderAndBlob	 Vertex	 = {};
		RendererComputeShaderAndBlob Compute = {};
		RendererHullShaderAndBlob	 Hull = {};
		RendererDomainShaderAndBlob  Domain = {};
	};
}
