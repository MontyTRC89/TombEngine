#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Renderer/Graphics/TextureBase.h"
#include "Renderer/RendererUtils.h"

namespace TEN::Renderer::Graphics
{
	using namespace TEN::Renderer::Utils;

	using Microsoft::WRL::ComPtr;

	class UAVRenderTarget2D : public TextureBase
	{
	public:
		ComPtr<ID3D11Texture2D>				Texture;
		ComPtr<ID3D11UnorderedAccessView>	UnorderedAccessView;

		UAVRenderTarget2D() {};

		UAVRenderTarget2D(ID3D11Device* device, int width, int height, DXGI_FORMAT format)
		{
			auto uavDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

			auto desc = D3D11_TEXTURE2D_DESC{};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			auto res = device->CreateTexture2D(&desc, nullptr, &Texture);
			throwIfFailed(res);

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = format;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;

			res = device->CreateUnorderedAccessView(Texture.Get(), &uavDesc, UnorderedAccessView.GetAddressOf());
			throwIfFailed(res);

			// Set up description of shader resource view.
			auto shaderDesc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
			shaderDesc.Format = format;
			shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderDesc.Texture2D.MostDetailedMip = 0;
			shaderDesc.Texture2D.MipLevels = 1;

			res = device->CreateShaderResourceView(Texture.Get(), &shaderDesc, &ShaderResourceView);
			throwIfFailed(res);
		}
	};
}
