#include "framework.h"
#include "RenderTarget2D.h"
#include "Utils.h"

namespace TEN::Renderer
{
	using TEN::Renderer::Utils::throwIfFailed;

	RenderTarget2D::RenderTarget2D(ID3D11Device* device, int w, int h, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = colorFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT res = device->CreateTexture2D(&desc, NULL, &Texture);
		throwIfFailed(res);

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = desc.Format;
		viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;

		res = device->CreateRenderTargetView(Texture.Get(), &viewDesc, &RenderTargetView);
		throwIfFailed(res);

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = desc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		res = device->CreateShaderResourceView(Texture.Get(), &shaderDesc, &ShaderResourceView);
		throwIfFailed(res);

		D3D11_TEXTURE2D_DESC depthTexDesc = {};
		depthTexDesc.Width = w;
		depthTexDesc.Height = h;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Format = depthFormat;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;

		res = device->CreateTexture2D(&depthTexDesc, NULL, &DepthStencilTexture);
		throwIfFailed(res);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = depthTexDesc.Format;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		res = device->CreateDepthStencilView(DepthStencilTexture.Get(), &dsvDesc, &DepthStencilView);
		throwIfFailed(res);
	}
}
