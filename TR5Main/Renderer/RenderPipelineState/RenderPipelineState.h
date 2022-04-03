#pragma once
#include <wrl/client.h>
#include <d3d11.h>

namespace TEN::Renderer
{
	class RenderPipelineState;

	struct ShaderCompileOptions
	{
		std::wstring fileName;
		std::string functionName;
		std::string profile;
		std::string source;
	};

	struct BlendStateOptions
	{
		enum BlendFunction
		{
			SRC_ADD_DST,
			SRC_SUBTRACT_DST,
			DST_SUBTRACT_SRC,
			MIN,
			MAX
		};

		enum BlendFactor
		{
			ZERO,
			ONE,
			SRC_COLOR,
			INV_SRC_COLOR,
			SRC_ALPHA,
			INV_SRC_ALPHA,
			DST_ALPHA,
			INV_DST_ALPHA,
			DST_COLOR,
			INV_DST_COLOR,
			ALPHA_SAT,
			BLEND_FACTOR,
			INV_BLEND_FACTOR,
		};

		BlendFunction blendFunction;
		BlendFactor sourceColorFactor;
		BlendFactor sourceAlphaFactor;
		BlendFactor destinationColorFactor;
		BlendFactor destinationAlphaFactor;
		bool blendingEnabled;
	};

	using Microsoft::WRL::ComPtr;

	class RenderPipelineState
	{
	private:
		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11VertexShader> vertexShader;
		ComPtr<ID3D11PixelShader> pixelShader;
		ComPtr<ID3D11BlendState> blendState;
		ComPtr<ID3D11DepthStencilState> depthState;

	public:
		RenderPipelineState(ID3D11Device* device, const ShaderCompileOptions& vertexShader, const ShaderCompileOptions& pixelShader, const BlendStateOptions& blendingOptions);
	};
}
