#include "framework.h"
#include "RenderTarget2D.h"
#include "Renderer/Utils.h"
#include "Specific/configuration.h"

namespace TEN::Renderer
{
	using TEN::Renderer::Utils::throwIfFailed;

	RenderTarget2D::RenderTarget2D(ID3D11Device* device, int w, int h, DXGI_FORMAT colorFormat, bool typeless, DXGI_FORMAT depthFormat)
	{
		 // Check if antialiasing quality is available, and set it if it is.
		
		D3D11_SRV_DIMENSION srvDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		D3D11_DSV_DIMENSION dsvDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		D3D11_RTV_DIMENSION rtvDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		// Now set up render target.

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = typeless ? MakeTypeless(colorFormat) : colorFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT res = device->CreateTexture2D(&desc, NULL, &Texture);
		throwIfFailed(res);

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = colorFormat;
		viewDesc.ViewDimension = rtvDimension;
		viewDesc.Texture2D.MipSlice = 0;

		res = device->CreateRenderTargetView(Texture.Get(), &viewDesc, &RenderTargetView);
		throwIfFailed(res);

		if (typeless && false)
		{
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = rtvDimension;
			viewDesc.Texture2D.MipSlice = 0;

			res = device->CreateRenderTargetView(Texture.Get(), &viewDesc, &SRGBRenderTargetView);
			throwIfFailed(res);
		}

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = colorFormat;
		shaderDesc.ViewDimension = srvDimension;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		res = device->CreateShaderResourceView(Texture.Get(), &shaderDesc, &ShaderResourceView);
		throwIfFailed(res);

		// Setup the description of the shader resource view.
		if (typeless && false)
		{
			shaderDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			shaderDesc.ViewDimension = srvDimension;
			shaderDesc.Texture2D.MostDetailedMip = 0;
			shaderDesc.Texture2D.MipLevels = 1;

			res = device->CreateShaderResourceView(Texture.Get(), &shaderDesc, &SRGBShaderResourceView);
			throwIfFailed(res);
		}

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
		dsvDesc.ViewDimension = dsvDimension;
		dsvDesc.Texture2D.MipSlice = 0;

		res = device->CreateDepthStencilView(DepthStencilTexture.Get(), &dsvDesc, &DepthStencilView);
		throwIfFailed(res);
	}

	RenderTarget2D::RenderTarget2D(ID3D11Device* device, RenderTarget2D* parent, DXGI_FORMAT colorFormat)
	{
		D3D11_TEXTURE2D_DESC desc;
		parent->Texture.Get()->GetDesc(&desc);
		int width = desc.Width;
		int height = desc.Height;
		parent->Texture.CopyTo(Texture.GetAddressOf());

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = colorFormat;
		viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;

		HRESULT res;
		res = device->CreateRenderTargetView(Texture.Get(), &viewDesc, &RenderTargetView);
		throwIfFailed(res);

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = colorFormat;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		res = device->CreateShaderResourceView(Texture.Get(), &shaderDesc, &ShaderResourceView);
		throwIfFailed(res);
	}

	DXGI_FORMAT RenderTarget2D::MakeTypeless(DXGI_FORMAT format) 
	{
		switch (format) {
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_TYPELESS;
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_TYPELESS;
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_TYPELESS;
		};

		return format;
	}
}
