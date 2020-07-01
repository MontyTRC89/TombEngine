#pragma once
#include <wrl/client.h>
#include <d3d11.h>
namespace T5M::Renderer {
	using Microsoft::WRL::ComPtr;
	using std::array;
	class RenderTargetCube {
	public:
		static constexpr Vector3 forwardVectors[6] = {
			Vector3(-1,0,0),
			Vector3(1,0,0),
			Vector3(0,-1,0),
			Vector3(0,1,0),
			Vector3(0,0,-1),
			Vector3(0,0,1),
		};
		static constexpr Vector3 upVectors[6] = {
			Vector3(0,1,0),
			Vector3(0,1,0),
			Vector3(0,0,-1),
			Vector3(0,0,1),
			Vector3(0,1,0),
			Vector3(0,1,0),
		};
		array<ComPtr<ID3D11RenderTargetView>,6> RenderTargetView;
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
		ComPtr<ID3D11Texture2D> Texture;
		array<ComPtr<ID3D11DepthStencilView>,6> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		int resolution;
		RenderTargetCube():resolution(0) {};
		RenderTargetCube(ID3D11Device* device, int resolution, DXGI_FORMAT format);
	};
}