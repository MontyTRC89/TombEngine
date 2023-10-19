#include "framework.h"
#include "Texture2DUAV.h"
#include "Renderer/Utils.h"

namespace TEN::Renderer
{
	using Utils::throwIfFailed;

	Texture2DUAV::Texture2DUAV(ID3D11Device* device, int w, int h, DXGI_FORMAT format)
	{
		Width = w;
		Height = h;

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.Format = format;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = desc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;
		throwIfFailed(device->CreateShaderResourceView(Texture.Get(), &shaderDesc, ShaderResourceView.GetAddressOf()));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = w * h;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

		Utils::throwIfFailed(device->CreateUnorderedAccessView(Texture.Get(), &uavDesc, UnorderedAccessView.GetAddressOf()));
	}
}
