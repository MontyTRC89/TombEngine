#pragma once
#include <wrl/client.h>
#include "Renderer/Graphics/TextureBase.h"

namespace TEN::Renderer::Graphics
{
	using Microsoft::WRL::ComPtr;

	class RenderTarget2D : public TextureBase
	{
	public:
		ComPtr<ID3D11RenderTargetView>	 RenderTargetView;
		ComPtr<ID3D11Texture2D>			 Texture;
		ComPtr<ID3D11DepthStencilView>	 DepthStencilView;
		ComPtr<ID3D11Texture2D>			 DepthStencilTexture;
		ComPtr<ID3D11ShaderResourceView> DepthShaderResourceView;

		RenderTarget2D() {};

		// Main constructor
		RenderTarget2D(ID3D11Device* device, int width, int height, DXGI_FORMAT colorFormat, bool isTypeless, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
		
		// Constructor for sharing the same texture of another render target, used for example by SMAA
		RenderTarget2D(ID3D11Device* device, RenderTarget2D* parent, DXGI_FORMAT colorFormat);

		// Constructor for creating the main back buffer
		RenderTarget2D(ID3D11Device* device, ID3D11Texture2D* texture, DXGI_FORMAT depthFormat);

	private:
		DXGI_FORMAT MakeTypeless(DXGI_FORMAT format);
	};
}
