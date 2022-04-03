#include "framework.h"
#include "RenderTargetCubeArray.h"
#include "Utils.h"

namespace TEN::Renderer
{
	RenderTargetCubeArray::RenderTargetCubeArray(ID3D11Device* device, size_t resolution, size_t numCubes, DXGI_FORMAT colorFormat,DXGI_FORMAT depthFormat) : numCubes(numCubes), resolution(resolution), viewport(CreateViewport(resolution))
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.ArraySize = numCubes*6;
		desc.Height = resolution;
		desc.Width = resolution;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0x0;
		desc.SampleDesc.Count = 1;
		desc.MipLevels = 1;
		desc.Format = colorFormat;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.SampleDesc.Quality = 0;
		HRESULT res = device->CreateTexture2D(&desc, nullptr, Texture.GetAddressOf());
		Utils::throwIfFailed(res);

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = desc.Format;
		viewDesc.Texture2DArray.ArraySize = 1;
		viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		RenderTargetView.resize(numCubes);

		for (int i = 0; i < numCubes - 1; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				viewDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i * numCubes + j, 1);
				res = device->CreateRenderTargetView(Texture.Get(), &viewDesc, RenderTargetView[i][j].GetAddressOf());
				Utils::throwIfFailed(res);
			}
		}
		
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = colorFormat;
		srvDesc.TextureCubeArray.NumCubes = numCubes;
		srvDesc.TextureCubeArray.First2DArrayFace = 0;
		srvDesc.TextureCubeArray.MipLevels = 1;
		srvDesc.TextureCubeArray.MostDetailedMip = 0;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		res = device->CreateShaderResourceView(Texture.Get(), &srvDesc,ShaderResourceView.GetAddressOf());
		Utils::throwIfFailed(res);
		D3D11_TEXTURE2D_DESC depthTexDesc = {};
		depthTexDesc.Width = resolution;
		depthTexDesc.Height = resolution;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = numCubes*6;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Format = depthFormat;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		res = device->CreateTexture2D(&depthTexDesc, NULL, DepthStencilTexture.GetAddressOf());
		Utils::throwIfFailed(res);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = depthTexDesc.Format;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = 1;
		DepthStencilView.resize(numCubes);

		for (int i = 0; i < numCubes - 1; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				dsvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i * numCubes + j, 1);
				res = device->CreateDepthStencilView(DepthStencilTexture.Get(), &dsvDesc, DepthStencilView[i][j].GetAddressOf());
				Utils::throwIfFailed(res);
			}
		}
	}

	RenderTargetCubeArray::RenderTargetCubeArray() : resolution(0), viewport(CreateViewport(resolution)),numCubes(0)
	{

	}
}
