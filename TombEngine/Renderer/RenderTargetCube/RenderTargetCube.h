#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Renderer/TextureBase.h"

namespace TEN::Renderer 
{
	using Microsoft::WRL::ComPtr;
	using std::array;

	class RenderTargetCube : public TextureBase
	{
	public:
		static constexpr Vector3 forwardVectors[6] = 
		{
			//+X (right)
			Vector3(-1,0,0),
			//-X (left)
			Vector3(1,0,0),
			//-Y (up)
			Vector3(0,-1,0),
			//+Y (down)
			Vector3(0,1,0),
			//+Z (forward)
			Vector3(0,0,1),
			//-Z (backward)
			Vector3(0,0,-1),
		};

		static constexpr Vector3 upVectors[6] = 
		{
			Vector3(0,-1,0),
			Vector3(0,-1,0),
			Vector3(0,0,-1),
			Vector3(0,0,1),
			Vector3(0,-1,0),
			Vector3(0,-1,0),
		};

		array<ComPtr<ID3D11RenderTargetView>, 6> RenderTargetView;
		ComPtr<ID3D11Texture2D> Texture;
		array<ComPtr<ID3D11DepthStencilView>, 6> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		int resolution;
		D3D11_VIEWPORT viewport;

		RenderTargetCube() : resolution(0), viewport({}) {};
		RenderTargetCube(ID3D11Device* device, int resolution, DXGI_FORMAT format, DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);
	};
}
