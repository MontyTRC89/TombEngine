#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/winmain.h"

namespace TEN::Renderer
{
	void Renderer11::toggleFullScreen()
	{
		
	}

	void Renderer11::changeScreenResolution(int width, int height, int frequency, bool windowed)
	{
		HRESULT res;

		/*if (windowed && !Windowed)
		{
			res = m_swapChain->SetFullscreenState(false, NULL);
			if (FAILED(res))
				return false;
		}
		else if (!windowed && Windowed)
		{
			res = m_swapChain->SetFullscreenState(true, NULL);
			if(FAILED(res))
				return false;
		}

		IDXGIOutput* output;
		res = m_swapChain->GetContainingOutput(&output);
		if(FAILED(res))
			return false;

		DXGI_SWAP_CHAIN_DESC scd;
		res = m_swapChain->GetDesc(&scd);
		if (FAILED(res))
			return false;

		UINT numModes = 1024;
		DXGI_MODE_DESC modes[1024];
		res = output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes);
		if (FAILED(res))
			return false;

		DXGI_MODE_DESC* mode = &modes[0];
		for (int i = 0; i < numModes; i++)
		{
			mode = &modes[i];
			if (mode->Width == width && mode->Height == height)
				break;
		}


		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		m_backBufferRTV->Release(); // Microsoft::WRL::ComPtr here does a Release();
		m_depthStencilView->Release();
		m_context->Flush();

		res = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);




		res = m_swapChain->ResizeTarget(mode);
		if (FAILED(res))
			return false;

		RECT rect;
		GetClientRect(WindowsHandle, &rect);
		UINT w = static_cast<UINT>(rect.right);
		UINT h = static_cast<UINT>(rect.bottom);

		m_context->ClearState();
		m_backBufferRTV->Release();
		m_depthStencilView->Release();
		m_depthStencilTexture->Release();

		res = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(res))
			return false;

		// Recreate render target
		res = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backBufferTexture);
		if (FAILED(res))
			return false;

		m_device->CreateRenderTargetView(m_backBufferTexture, NULL, &m_backBufferRTV);

		D3D11_TEXTURE2D_DESC backBufferDesc;
		m_backBufferTexture->GetDesc(&backBufferDesc);
		m_backBufferTexture->Release();

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		m_depthStencilTexture = NULL;
		res = m_device->CreateTexture2D(&depthStencilDesc, NULL, &m_depthStencilTexture);
		if (FAILED(res))
			return false;

		m_depthStencilView = NULL;
		res = m_device->CreateDepthStencilView(m_depthStencilTexture, NULL, &m_depthStencilView);
		if (FAILED(res))
			return false;

		DX11_DELETE(m_renderTarget);
		DX11_DELETE(m_dumpScreenRenderTarget);
		DX11_DELETE(m_shadowMap);

		m_renderTarget = RenderTarget2D::Create(m_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_dumpScreenRenderTarget = RenderTarget2D::Create(m_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_shadowMap = RenderTarget2D::Create(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT);

		DX11_DELETE(m_gameFont);
		DX11_DELETE(m_spriteBatch);
		DX11_DELETE(m_primitiveBatch);

		m_spriteBatch = new SpriteBatch(m_context);
		m_gameFont = new SpriteFont(m_device, L"Textures/Font.spritefont");
		m_primitiveBatch = new PrimitiveBatch<RendererVertex>(m_context);

		ScreenWidth = width;
		ScreenHeight = height;
		Windowed = windowed;*/

		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_context->OMSetRenderTargets(0, nullViews, NULL);

		m_backBufferTexture->Release();
		m_backBufferRTV->Release();
		m_depthStencilView->Release();
		m_depthStencilTexture->Release();
		m_context->Flush();
		m_context->ClearState();

		//res = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		/*res = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(res))
			return false;*/

		IDXGIOutput* output;
		Utils::throwIfFailed(m_swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::throwIfFailed(m_swapChain->GetDesc(&scd));

		UINT numModes = 1024;
		DXGI_MODE_DESC modes[1024];
		Utils::throwIfFailed(output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes));

		DXGI_MODE_DESC* mode = &modes[0];
		for (int i = 0; i < numModes; i++)
		{
			mode = &modes[i];
			if (mode->Width == width && mode->Height == height)
				break;
		}

		Utils::throwIfFailed( m_swapChain->ResizeTarget(mode));

		initialiseScreen(width, height, frequency, windowed, WindowsHandle, true);

		ScreenWidth = width;
		ScreenHeight = height;
		Windowed = windowed;
	}
}
