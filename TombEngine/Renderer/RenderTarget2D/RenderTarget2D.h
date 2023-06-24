#pragma once
#include <wrl/client.h>
#include "Renderer/TextureBase.h"

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;

	class RenderTarget2D : public TextureBase
	{
	public:
		ComPtr<ID3D11RenderTargetView> RenderTargetView = nullptr;
		ComPtr<ID3D11Texture2D> Texture = nullptr;
		ComPtr<ID3D11DepthStencilView> DepthStencilView = nullptr;
		ComPtr<ID3D11Texture2D> DepthStencilTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> DepthShaderResourceView = nullptr;

		RenderTarget2D() {};
		RenderTarget2D(ID3D11Device* device, int w, int h, bool allowMultisampled, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
	};

}
