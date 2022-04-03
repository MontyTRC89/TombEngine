#include "framework.h"
#include "RenderPipelineState.h"

#include "Utils.h"

namespace TEN::Renderer
{
	using namespace Utils;

	RenderPipelineState::RenderPipelineState(ID3D11Device* device, const ShaderCompileOptions& vertexShader, const ShaderCompileOptions& pixelShader, const BlendStateOptions& blendingOptions)
	{
		ComPtr<ID3D10Blob> blob;
		this->vertexShader = compileVertexShader(device, vertexShader.fileName.c_str(), vertexShader.functionName.c_str(), vertexShader.profile.c_str(),nullptr, blob);
		this->pixelShader = compilePixelShader(device, pixelShader.fileName.c_str(), pixelShader.functionName.c_str(), pixelShader.profile.c_str(), nullptr, blob);
		D3D11_BLEND_DESC blndDesc = {};
		
		blndDesc.IndependentBlendEnable = blendingOptions.blendingEnabled;
		blndDesc.AlphaToCoverageEnable = false;
		blndDesc.RenderTarget[0].BlendEnable = blendingOptions.blendingEnabled;
		blndDesc.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)blendingOptions.blendFunction;
		blndDesc.RenderTarget[0].SrcBlend = (D3D11_BLEND)blendingOptions.sourceColorFactor;
		blndDesc.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)blendingOptions.sourceAlphaFactor;
		blndDesc.RenderTarget[0].DestBlend = (D3D11_BLEND)blendingOptions.destinationColorFactor;
		blndDesc.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)blendingOptions.destinationAlphaFactor;
		blndDesc.RenderTarget[0].RenderTargetWriteMask = 0xFF;
		device->CreateBlendState(&blndDesc, this->blendState.GetAddressOf());
	}
}
