#pragma once
#include <wrl/client.h>
#include <d3d11.h>
namespace T5M::Renderer {
	using Microsoft::WRL::ComPtr;
	using std::array;
	class RenderTargetCube {
	public:
		array<ComPtr<ID3D11RenderTargetView>,6> RenderTargetView;
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
		ComPtr<ID3D11Texture2D> Texture;
		ComPtr<ID3D11DepthStencilView> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		RenderTargetCube() {};
		RenderTargetCube(ID3D11Device* device, int resolution, DXGI_FORMAT format);
	};
}