#pragma once
#include <d3d11.h>
#include <array>
#include <wrl/client.h>

namespace TEN::Renderer
{
	using Microsoft::WRL::ComPtr;
	using std::array;
	using std::vector;

	class RenderTargetCubeArray
	{
	private:
		static constexpr D3D11_VIEWPORT CreateViewport(size_t resolution)
		{
			return
			{
				0,
				0,
				static_cast<FLOAT>(resolution),
				static_cast<FLOAT>(resolution),
				0,
				1
			};
		}

	public:
		size_t numCubes;
		size_t resolution;
		D3D11_VIEWPORT viewport;
		vector<array<ComPtr<ID3D11RenderTargetView>, 6>> RenderTargetView;
		ComPtr<ID3D11ShaderResourceView> ShaderResourceView;
		ComPtr<ID3D11Texture2D> Texture;
		vector<array<ComPtr<ID3D11DepthStencilView>, 6>> DepthStencilView;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;

		RenderTargetCubeArray();
		RenderTargetCubeArray(ID3D11Device* device, size_t resolution,size_t arraySize, DXGI_FORMAT format,  DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT);

	};
}
