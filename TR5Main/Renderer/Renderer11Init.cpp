#include "framework.h"
#include "Renderer11.h"
#include "configuration.h"
#include "winmain.h"
#include "GameFlowScript.h"
#include "Quad/RenderQuad.h"
using namespace T5M::Renderer;
using std::vector;
extern GameConfiguration g_Configuration;
extern GameFlow* g_GameFlow;

bool Renderer11::Initialise(int w, int h, int refreshRate, bool windowed, HWND handle)
{
	HRESULT res;

	//DB_Log(2, "Renderer::Initialise - DLL");
	printf("Initialising DX11\n");

	CoInitialize(NULL);

	ScreenWidth = w;
	ScreenHeight = h;
	Windowed = windowed;

	if (!initialiseScreen(w, h, refreshRate, windowed, handle, false))
		return false;

	// Initialise render states
	m_states = new CommonStates(m_device);

	// Load caustics
	const char* causticsNames[NUM_CAUSTICS_TEXTURES] = {
		"CausticsRender_001.bmp",
		"CausticsRender_002.bmp",
		"CausticsRender_003.bmp",
		"CausticsRender_004.bmp",
		"CausticsRender_005.bmp",
		"CausticsRender_006.bmp",
		"CausticsRender_007.bmp",
		"CausticsRender_008.bmp",
		"CausticsRender_009.bmp",
		"CausticsRender_010.bmp",
		"CausticsRender_011.bmp",
		"CausticsRender_012.bmp",
		"CausticsRender_013.bmp",
		"CausticsRender_014.bmp",
		"CausticsRender_015.bmp",
		"CausticsRender_016.bmp"
	};

	for (int i = 0; i < NUM_CAUSTICS_TEXTURES; i++)
	{
		wchar_t causticsFile[255];
		std::mbstowcs(causticsFile, causticsNames[i], 255);
		std::wstring causticsFilename = std::wstring(causticsFile);
		m_caustics[i] = Texture2D(m_device, causticsFilename);
	}
	m_HUDBarBorderTexture = Texture2D(m_device, L"bar_border.png");
	wchar_t titleScreenFile[255];
	std::wstring titleFile = std::wstring(titleScreenFile);
	std::mbstowcs(titleScreenFile, g_GameFlow->GetLevel(0)->Background.c_str(),255);

	m_titleScreen = Texture2D(m_device, titleScreenFile);

	m_binocularsTexture = Texture2D(m_device, L"Binoculars.png");
	m_whiteTexture = Texture2D(m_device, L"WhiteSprite.png");

	m_logo = Texture2D(m_device, L"Logo.png");

	// Load shaders
	ID3D10Blob * blob;

	m_vsRooms = compileVertexShader(L"Shaders\\DX11_Rooms.fx", "VS", "vs_4_0", &blob);
	if (m_vsRooms == NULL)
		return false;

	// Initialise input layout using the first vertex shader
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	m_inputLayout = NULL;
	res = m_device->CreateInputLayout(inputLayout, 7, blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout);
	if (FAILED(res))
		return false;

	m_psRooms = compilePixelShader(L"Shaders\\DX11_Rooms.fx", "PS", "ps_4_0", &blob);
	if (m_psRooms == NULL)
		return false;

	m_vsItems = compileVertexShader(L"Shaders\\DX11_Items.fx", "VS", "vs_4_0", &blob);
	if (m_vsItems == NULL)
		return false;

	m_psItems = compilePixelShader(L"Shaders\\DX11_Items.fx", "PS", "ps_4_0", &blob);
	if (m_psItems == NULL)
		return false;

	m_vsStatics = compileVertexShader(L"Shaders\\DX11_Statics.fx", "VS", "vs_4_0", &blob);
	if (m_vsStatics == NULL)
		return false;

	m_psStatics = compilePixelShader(L"Shaders\\DX11_Statics.fx", "PS", "ps_4_0", &blob);
	if (m_psStatics == NULL)
		return false;

	m_vsHairs = compileVertexShader(L"Shaders\\DX11_Hairs.fx", "VS", "vs_4_0", &blob);
	if (m_vsHairs == NULL)
		return false;

	m_psHairs = compilePixelShader(L"Shaders\\DX11_Hairs.fx", "PS", "ps_4_0", &blob);
	if (m_psHairs == NULL)
		return false;

	m_vsSky = compileVertexShader(L"Shaders\\DX11_Sky.fx", "VS", "vs_4_0", &blob);
	if (m_vsSky == NULL)
		return false;

	m_psSky = compilePixelShader(L"Shaders\\DX11_Sky.fx", "PS", "ps_4_0", &blob);
	if (m_psSky == NULL)
		return false;

	m_vsSprites = compileVertexShader(L"Shaders\\DX11_Sprites.fx", "VS", "vs_4_0", &blob);
	if (m_vsSprites == NULL)
		return false;

	m_psSprites = compilePixelShader(L"Shaders\\DX11_Sprites.fx", "PS", "ps_4_0", &blob);
	if (m_psSprites == NULL)
		return false;

	m_vsSolid = compileVertexShader(L"Shaders\\DX11_Solid.fx", "VS", "vs_4_0", &blob);
	if (m_vsSolid == NULL)
		return false;

	m_psSolid = compilePixelShader(L"Shaders\\DX11_Solid.fx", "PS", "ps_4_0", &blob);
	if (m_psSolid == NULL)
		return false;

	m_vsInventory = compileVertexShader(L"Shaders\\DX11_Inventory.fx", "VS", "vs_4_0", &blob);
	if (m_vsInventory == NULL)
		return false;

	m_psInventory = compilePixelShader(L"Shaders\\DX11_Inventory.fx", "PS", "ps_4_0", &blob);
	if (m_psInventory == NULL)
		return false;

	m_vsFullScreenQuad = compileVertexShader(L"Shaders\\DX11_FullScreenQuad.fx", "VS", "vs_4_0", &blob);
	if (m_vsFullScreenQuad == NULL)
		return false;

	m_psFullScreenQuad = compilePixelShader(L"Shaders\\DX11_FullScreenQuad.fx", "PS", "ps_4_0", &blob);
	if (m_psFullScreenQuad == NULL)
		return false;

	m_vsShadowMap = compileVertexShader(L"Shaders\\DX11_ShadowMap.fx", "VS", "vs_4_0", &blob);
	if (m_vsShadowMap == NULL)
		return false;

	m_psShadowMap = compilePixelShader(L"Shaders\\DX11_ShadowMap.fx", "PS", "ps_4_0", &blob);
	if (m_psShadowMap == NULL)
		return false;

	m_vsHUD = compileVertexShader(L"Shaders\\HUD\\DX11_VS_HUD.hlsl", "VS", "vs_4_0", &blob);
	if (m_vsHUD == NULL)
		return false;
	m_psHUDColor = compilePixelShader(L"Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSColored", "ps_4_0", &blob);
	if (m_psHUDColor == NULL)
		return false;
	m_psHUDTexture = compilePixelShader(L"Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSTextured", "ps_4_0", &blob);
	if (m_psHUDTexture == NULL)
		return false;
	m_psHUDBarColor = compilePixelShader(L"Shaders\\HUD\\DX11_PS_HUDBar.hlsl", "PSColored", "ps_4_0", &blob);
	if (m_psHUDBarColor == NULL)
		return false;

	// Initialise constant buffers
	m_cbCameraMatrices = createConstantBuffer(sizeof(CCameraMatrixBuffer));
	m_cbItem = createConstantBuffer(sizeof(CItemBuffer));
	m_cbStatic = createConstantBuffer(sizeof(CStaticBuffer));
	m_cbLights = createConstantBuffer(sizeof(CLightBuffer));
	m_cbMisc = createConstantBuffer(sizeof(CMiscBuffer));
	m_cbShadowMap = createConstantBuffer(sizeof(CShadowLightBuffer));
	m_cbRoom = createConstantBuffer(sizeof(CRoomBuffer));
	//Prepare HUD Constant buffer
	m_cbHUDBar = createConstantBuffer(sizeof(CHUDBarBuffer));
	m_cbHUD = createConstantBuffer(sizeof(CHUDBuffer));
	m_cbSprite = createConstantBuffer(sizeof(CSpriteBuffer));
	m_stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0));
	m_stHUD.Projection =Matrix::CreateOrthographicOffCenter(0, REFERENCE_RES_WIDTH, 0, REFERENCE_RES_HEIGHT, 0, 1.0f);
	updateConstantBuffer<CHUDBuffer>(m_cbHUD, m_stHUD);
	m_currentCausticsFrame = 0;
	m_firstWeather = true;

	// Preallocate lists
	m_roomsToDraw = vector<RendererRoom*>(NUM_ROOMS);
	m_itemsToDraw = vector<RendererItem*>(NUM_ITEMS);
	m_effectsToDraw = vector<RendererEffect*>(NUM_ITEMS);
	m_lightsToDraw = vector<RendererLight*>(MAX_LIGHTS_DRAW);
	m_dynamicLights = vector<RendererLight*>(MAX_DYNAMIC_LIGHTS);
	m_staticsToDraw = vector<RendererStatic*>(MAX_DRAW_STATICS);
	m_spritesToDraw = vector<RendererSpriteToDraw*>(MAX_SPRITES);
	m_lines3DToDraw = vector<RendererLine3D*>(MAX_LINES_3D);
	m_lines2DToDraw = vector<RendererLine2D*>(MAX_LINES_2D);
	m_tempItemLights = vector<RendererLight*>(MAX_LIGHTS);
	m_spritesBuffer = (RendererSpriteToDraw*)malloc(sizeof(RendererSpriteToDraw) * MAX_SPRITES);
	m_lines3DBuffer = (RendererLine3D*)malloc(sizeof(RendererLine3D) * MAX_LINES_3D);
	m_lines2DBuffer = (RendererLine2D*)malloc(sizeof(RendererLine2D) * MAX_LINES_2D);
	m_pickupRotation = 0;

	for (int i = 0; i < NUM_ITEMS; i++)
	{
		m_items[i].Lights = vector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
		m_effects[i].Lights = vector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
	}

	D3D11_BLEND_DESC blendStateDesc{};
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendStateDesc, &m_subtractiveBlendState);
	initialiseBars();
	initQuad(m_device);
	return true;
}

bool Renderer11::initialiseScreen(int w, int h, int refreshRate, bool windowed, HWND handle, bool reset)
{
	HRESULT res;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = w;
	sd.BufferDesc.Height = h;
	sd.BufferDesc.RefreshRate.Numerator = refreshRate;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	sd.Windowed = windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = handle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	IDXGIDevice* dxgiDevice = NULL;
	res = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)& dxgiDevice);
	if (FAILED(res))
		return false;

	IDXGIAdapter* dxgiAdapter = NULL;
	res = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)& dxgiAdapter);
	if (FAILED(res))
		return false;

	IDXGIFactory* dxgiFactory = NULL;
	res = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)& dxgiFactory);
	if (FAILED(res))
		return false;

	if (reset)
	{
		// Always return to windowed mode otherwise crash will happen
		m_swapChain->SetFullscreenState(false, NULL);
		m_swapChain->Release();
	}

	m_swapChain = NULL;
	res = dxgiFactory->CreateSwapChain(m_device, &sd, &m_swapChain);
	if (FAILED(res))
		return false;

	dxgiFactory->MakeWindowAssociation(handle, 0);
	res = m_swapChain->SetFullscreenState(!windowed, NULL);

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Initialise the back buffer
	m_backBufferTexture = NULL;
	res = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast <void**>(&m_backBufferTexture));
	if (FAILED(res))
		return false;

	m_backBufferRTV = NULL;
	res = m_device->CreateRenderTargetView(m_backBufferTexture, NULL, &m_backBufferRTV);
	if (FAILED(res))
		return false;

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = w;
	depthStencilDesc.Height = h;
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

	// Bind the back buffer and the depth stencil
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);

	// Initialise sprites and font
	m_spriteBatch = new SpriteBatch(m_context);
	m_gameFont = new SpriteFont(m_device, L"Font.spritefont");
	m_primitiveBatch = new PrimitiveBatch<RendererVertex>(m_context);

	// Initialise buffers
	m_renderTarget = RenderTarget2D(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_shadowMap = RenderTarget2D(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT);
	m_reflectionCubemap = RenderTargetCube(m_device, 128, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	// Shadow map
	/*D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthTexDesc.Width = SHADOW_MAP_SIZE;
	depthTexDesc.Height = SHADOW_MAP_SIZE;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	res = m_device->CreateTexture2D(&depthTexDesc, NULL, &m_shadowMapTexture);
	if (FAILED(res))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_shadowMapDSV = NULL;
	res = m_device->CreateDepthStencilView(m_shadowMapTexture, &dsvDesc, &m_shadowMapDSV);
	if (FAILED(res))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	shaderDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	res = m_device->CreateShaderResourceView(m_shadowMapTexture, &shaderDesc, &m_shadowMapRV);
	if (FAILED(res))
		return false;*/

		// Initialise viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = w;
	m_viewport.Height = h;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_shadowMapViewport.TopLeftX = 0;
	m_shadowMapViewport.TopLeftY = 0;
	m_shadowMapViewport.Width = SHADOW_MAP_SIZE;
	m_shadowMapViewport.Height = SHADOW_MAP_SIZE;
	m_shadowMapViewport.MinDepth = 0.0f;
	m_shadowMapViewport.MaxDepth = 1.0f;

	m_viewportToolkit = new Viewport(m_viewport.TopLeftX, m_viewport.TopLeftY, m_viewport.Width, m_viewport.Height,
		m_viewport.MinDepth, m_viewport.MaxDepth);

	if (windowed)
	{
		SetWindowLong(WindowsHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, g_Configuration.Width, g_Configuration.Height,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowLong(WindowsHandle, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	UpdateWindow(handle);

	return true;
}

bool Renderer11::Create()
{
	D3D_FEATURE_LEVEL levels[1] = { D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT res;

#ifdef _RELEASE
	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context);
#else
	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context); // D3D11_CREATE_DEVICE_DEBUG
#endif

	if (FAILED(res))
		return false;

	return true;
}

void Renderer11::initialiseHairRemaps()
{
	
}
