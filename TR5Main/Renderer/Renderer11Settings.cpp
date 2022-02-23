#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/winmain.h"

namespace TEN::Renderer 
{
	void Renderer11::ChangeScreenResolution(int width, int height, int frequency, bool windowed) 
	{
		HRESULT res;

		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_context->OMSetRenderTargets(0, nullViews, NULL);

		m_backBufferTexture->Release();
		m_backBufferRTV->Release();
		m_depthStencilView->Release();
		m_depthStencilTexture->Release();
		m_context->Flush();
		m_context->ClearState();

		IDXGIOutput* output;
		Utils::throwIfFailed(m_swapChain->GetContainingOutput(&output));

		DXGI_SWAP_CHAIN_DESC scd;
		Utils::throwIfFailed(m_swapChain->GetDesc(&scd));

		UINT numModes = 1024;
		DXGI_MODE_DESC modes[1024];
		Utils::throwIfFailed(output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes));

		DXGI_MODE_DESC* mode = &modes[0];
		for (int i = 0; i < numModes; i++) {
			mode = &modes[i];
			if (mode->Width == width && mode->Height == height)
				break;
		}

		Utils::throwIfFailed( m_swapChain->ResizeTarget(mode));

		InitialiseScreen(width, height, frequency, windowed, WindowsHandle, true);

		ScreenWidth = width;
		ScreenHeight = height;
		Windowed = windowed;
	}
}
