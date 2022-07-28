#include "framework.h"
#include "Texture2D.h"
#include "Utils.h"
#include <WICTextureLoader.h>

namespace TEN::Renderer
{
	using Utils::throwIfFailed;

	Texture2D::Texture2D(ID3D11Device* device, int w, int h, byte* data)
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;

		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = data;
		subresourceData.SysMemPitch = w * 4;
		subresourceData.SysMemSlicePitch = 0;
		throwIfFailed(device->CreateTexture2D(&desc, &subresourceData, &Texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = desc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;
		throwIfFailed(device->CreateShaderResourceView(Texture.Get(), &shaderDesc, ShaderResourceView.GetAddressOf()));
	}

	Texture2D::Texture2D(ID3D11Device* device, const std::wstring& fileName)
	{
		ComPtr<ID3D11Resource> resource;
		ID3D11DeviceContext* context = NULL;
		device->GetImmediateContext(&context);

		throwIfFailed(CreateWICTextureFromFile(device, context, fileName.c_str(), resource.GetAddressOf(), ShaderResourceView.GetAddressOf(), (size_t)0), L"Opening Texture file '" + fileName + L"': ");
		throwIfFailed(resource->QueryInterface(Texture.GetAddressOf()));
	}

	Texture2D::Texture2D(ID3D11Device* device, byte* data, int length)
	{
		ComPtr<ID3D11Resource> resource;
		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);
		throwIfFailed(CreateWICTextureFromMemoryEx(
			device,
			context,
			data,
			length, 
			(size_t)0,
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE,
			0,
			D3D11_RESOURCE_MISC_GENERATE_MIPS,
			WIC_LOADER_DEFAULT,
			resource.GetAddressOf(), 
			ShaderResourceView.GetAddressOf()));

		context->GenerateMips(ShaderResourceView.Get());

		throwIfFailed(resource->QueryInterface(Texture.GetAddressOf()));
	}
}
