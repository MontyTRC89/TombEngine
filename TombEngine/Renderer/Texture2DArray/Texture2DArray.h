#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Renderer/TextureBase.h"
#include <vector>

namespace TEN::Renderer 
{
	using Microsoft::WRL::ComPtr;

	class Texture2DArray : public TextureBase
	{
	public:

		std::vector<ComPtr<ID3D11RenderTargetView>> RenderTargetView;
		ComPtr<ID3D11Texture2D> Texture;
		std::vector<ComPtr<ID3D11DepthStencilView>> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		int resolution;
		D3D11_VIEWPORT viewport;

		Texture2DArray() : resolution(0), viewport({}) {};
		Texture2DArray(ID3D11Device* device, int resolution,int count, DXGI_FORMAT format, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
	};
}
