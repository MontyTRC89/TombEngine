#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Renderer/Graphics/TextureBase.h"
#include "Renderer/RendererUtils.h"

namespace TEN::Renderer::Graphics
{
	using namespace TEN::Renderer::Utils;

	using Microsoft::WRL::ComPtr;

	template <typename T>
	class UAVBuffer : public TextureBase
	{
	public:
		ComPtr<ID3D11Buffer>				Buffer;
		ComPtr<ID3D11UnorderedAccessView>	UnorderedAccessView;

		UAVBuffer() {};

		UAVBuffer(ID3D11Device* device, int elementsCount)
		{
			HRESULT res;

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(T) * elementsCount;
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			bufferDesc.StructureByteStride = sizeof(T);
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			res = device->CreateBuffer(&bufferDesc, nullptr, Buffer.GetAddressOf());
			throwIfFailed(res);

			// Creazione SRV (Shader Resource View) per leggere le particelle in Pixel Shader
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = elementsCount;
			res = device->CreateShaderResourceView(Buffer.Get(), &srvDesc, ShaderResourceView.GetAddressOf());
			throwIfFailed(res);

			// Creazione UAV (Unordered Access View) per aggiornare le particelle nel Compute Shader
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = elementsCount;
			res = device->CreateUnorderedAccessView(Buffer.Get(), &uavDesc, UnorderedAccessView.GetAddressOf());
			throwIfFailed(res);
		}
	};
}
