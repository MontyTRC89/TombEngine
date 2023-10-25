#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Renderer/Graphics/TextureBase.h"
#include <vector>

namespace TEN::Renderer::Graphics
{
	using Microsoft::WRL::ComPtr;

	class Texture2DArray : public TextureBase
	{
	public:

		std::vector<ComPtr<ID3D11RenderTargetView>> RenderTargetView;
		ComPtr<ID3D11Texture2D> Texture;
		std::vector<ComPtr<ID3D11DepthStencilView>> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		int Resolution;
		D3D11_VIEWPORT Viewport;

		Texture2DArray() : Resolution(0), Viewport({}) {};
		Texture2DArray(ID3D11Device* device, int resolution,int count, DXGI_FORMAT format, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
	};
}
