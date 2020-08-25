#pragma once
#include <wrl/client.h>
namespace T5M::Renderer {
	using Microsoft::WRL::ComPtr;
	class RenderTarget2D {
	public:
		ComPtr<ID3D11RenderTargetView> RenderTargetView;
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
		ComPtr<ID3D11Texture2D> Texture;
		ComPtr<ID3D11DepthStencilView> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		ComPtr<ID3D11ShaderResourceView> DepthShaderResourceView;
		RenderTarget2D() {};
		RenderTarget2D(ID3D11Device* device, int w, int h, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
	};

}


