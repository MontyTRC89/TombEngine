#include "Renderer11.h"
#include "../Specific/configuration.h"

extern GameConfiguration g_Configuration;
extern GameFlow* g_GameFlow;

bool Renderer11::Initialise(int w, int h, int refreshRate, bool windowed, HWND handle)
{
	HRESULT res;

	DB_Log(2, "Renderer::Initialise - DLL");
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
		m_caustics[i] = Texture2D::LoadFromFile(m_device, causticsNames[i]);
		if (m_caustics[i] == NULL)
			return false;
	}
	m_HUDBarBorderTexture = Texture2D::LoadFromFile(m_device, "bar_border.png");
	if (!m_HUDBarBorderTexture)
		return false;
	m_titleScreen = Texture2D::LoadFromFile(m_device, (char*)g_GameFlow->GetLevel(0)->Background.c_str());
	if (m_titleScreen == NULL)
		return false;

	m_binocularsTexture = Texture2D::LoadFromFile(m_device, "Binoculars.png");
	if (m_binocularsTexture == NULL)
		return false;

	m_whiteTexture = Texture2D::LoadFromFile(m_device, "WhiteSprite.png");
	if (m_whiteTexture == NULL)
		return false;

	m_logo = Texture2D::LoadFromFile(m_device, "Logo.png");
	if (m_logo == NULL)
		return false;

	// Load shaders
	ID3D10Blob * blob;

	m_vsRooms = compileVertexShader("Shaders\\DX11_Rooms.fx", "VS", "vs_4_0", &blob);
	if (m_vsRooms == NULL)
		return false;

	// Initialise input layout using the first vertex shader
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_inputLayout = NULL;
	res = m_device->CreateInputLayout(inputLayout, 5, blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout);
	if (FAILED(res))
		return false;

	m_psRooms = compilePixelShader("Shaders\\DX11_Rooms.fx", "PS", "ps_4_0", &blob);
	if (m_psRooms == NULL)
		return false;

	m_vsItems = compileVertexShader("Shaders\\DX11_Items.fx", "VS", "vs_4_0", &blob);
	if (m_vsItems == NULL)
		return false;

	m_psItems = compilePixelShader("Shaders\\DX11_Items.fx", "PS", "ps_4_0", &blob);
	if (m_psItems == NULL)
		return false;

	m_vsStatics = compileVertexShader("Shaders\\DX11_Statics.fx", "VS", "vs_4_0", &blob);
	if (m_vsStatics == NULL)
		return false;

	m_psStatics = compilePixelShader("Shaders\\DX11_Statics.fx", "PS", "ps_4_0", &blob);
	if (m_psStatics == NULL)
		return false;

	m_vsHairs = compileVertexShader("Shaders\\DX11_Hairs.fx", "VS", "vs_4_0", &blob);
	if (m_vsHairs == NULL)
		return false;

	m_psHairs = compilePixelShader("Shaders\\DX11_Hairs.fx", "PS", "ps_4_0", &blob);
	if (m_psHairs == NULL)
		return false;

	m_vsSky = compileVertexShader("Shaders\\DX11_Sky.fx", "VS", "vs_4_0", &blob);
	if (m_vsSky == NULL)
		return false;

	m_psSky = compilePixelShader("Shaders\\DX11_Sky.fx", "PS", "ps_4_0", &blob);
	if (m_psSky == NULL)
		return false;

	m_vsSprites = compileVertexShader("Shaders\\DX11_Sprites.fx", "VS", "vs_4_0", &blob);
	if (m_vsSprites == NULL)
		return false;

	m_psSprites = compilePixelShader("Shaders\\DX11_Sprites.fx", "PS", "ps_4_0", &blob);
	if (m_psSprites == NULL)
		return false;

	m_vsSolid = compileVertexShader("Shaders\\DX11_Solid.fx", "VS", "vs_4_0", &blob);
	if (m_vsSolid == NULL)
		return false;

	m_psSolid = compilePixelShader("Shaders\\DX11_Solid.fx", "PS", "ps_4_0", &blob);
	if (m_psSolid == NULL)
		return false;

	m_vsInventory = compileVertexShader("Shaders\\DX11_Inventory.fx", "VS", "vs_4_0", &blob);
	if (m_vsInventory == NULL)
		return false;

	m_psInventory = compilePixelShader("Shaders\\DX11_Inventory.fx", "PS", "ps_4_0", &blob);
	if (m_psInventory == NULL)
		return false;

	m_vsFullScreenQuad = compileVertexShader("Shaders\\DX11_FullScreenQuad.fx", "VS", "vs_4_0", &blob);
	if (m_vsFullScreenQuad == NULL)
		return false;

	m_psFullScreenQuad = compilePixelShader("Shaders\\DX11_FullScreenQuad.fx", "PS", "ps_4_0", &blob);
	if (m_psFullScreenQuad == NULL)
		return false;

	m_vsShadowMap = compileVertexShader("Shaders\\DX11_ShadowMap.fx", "VS", "vs_4_0", &blob);
	if (m_vsShadowMap == NULL)
		return false;

	m_psShadowMap = compilePixelShader("Shaders\\DX11_ShadowMap.fx", "PS", "ps_4_0", &blob);
	if (m_psShadowMap == NULL)
		return false;

	m_vsHUD = compileVertexShader("Shaders\\HUD\\DX11_VS_HUD.hlsl", "VS", "vs_4_0", &blob);
	if (m_vsHUD == NULL)
		return false;
	m_psHUDColor = compilePixelShader("Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSColored", "ps_4_0", &blob);
	if (m_psHUDColor == NULL)
		return false;
	m_psHUDTexture = compilePixelShader("Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSTextured", "ps_4_0", &blob);
	if (m_psHUDTexture == NULL)
		return false;
	m_psHUDBarColor = compilePixelShader("Shaders\\HUD\\DX11_PS_HUDBar.hlsl", "PSColored", "ps_4_0", &blob);
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
	m_stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0)).Transpose();
	m_stHUD.Projection =Matrix::CreateOrthographicOffCenter(0, REFERENCE_RES_WIDTH, 0, REFERENCE_RES_HEIGHT, 0, 1.0f).Transpose();
	updateConstantBuffer(m_cbHUD, &m_stHUD, sizeof(CHUDBuffer));
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

	m_textureAtlas = NULL;
	m_skyTexture = NULL;
	D3D11_BLEND_DESC blendStateDesc{};
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&blendStateDesc, &m_subtractiveBlendState);
	initialiseBars();
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
	m_renderTarget = RenderTarget2D::Create(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D::Create(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_shadowMap = RenderTarget2D::Create(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT);

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

	// Setup legacy variables
	PhdWindowPosX = 0;
	PhdWindowPosY = 0;
	PhdWindowXmax = w - 1;
	PhdWindowYmax = h - 1;
	PhdWidth = w;
	PhdHeight = h;
	PhdCentreX = w / 2;
	PhdCentreY = h / 2;

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
	memset(m_normalLaraSkinJointRemap, -1, 15 * 32 * 2);
	memset(m_youngLaraSkinJointRemap, -1, 15 * 32 * 2);

	// Normal Lara
	m_normalLaraSkinJointRemap[1][0] = 0;
	m_normalLaraSkinJointRemap[1][1] = 0;
	m_normalLaraSkinJointRemap[1][2] = 0;
	m_normalLaraSkinJointRemap[1][3] = 0;
	m_normalLaraSkinJointRemap[1][4] = 0;
	m_normalLaraSkinJointRemap[1][5] = 0;

	m_normalLaraSkinJointRemap[2][0] = 1;
	m_normalLaraSkinJointRemap[2][1] = 1;
	m_normalLaraSkinJointRemap[2][2] = 1;
	m_normalLaraSkinJointRemap[2][3] = 1;
	m_normalLaraSkinJointRemap[2][4] = 1;

	m_normalLaraSkinJointRemap[3][4] = 2;
	m_normalLaraSkinJointRemap[3][5] = 2;
	m_normalLaraSkinJointRemap[3][6] = 2;
	m_normalLaraSkinJointRemap[3][7] = 2;

	m_normalLaraSkinJointRemap[4][0] = 0;
	m_normalLaraSkinJointRemap[4][1] = 0;
	m_normalLaraSkinJointRemap[4][2] = 0;
	m_normalLaraSkinJointRemap[4][3] = 0;
	m_normalLaraSkinJointRemap[4][4] = 0;
	m_normalLaraSkinJointRemap[4][5] = 0;

	m_normalLaraSkinJointRemap[5][0] = 4;
	m_normalLaraSkinJointRemap[5][1] = 4;
	m_normalLaraSkinJointRemap[5][2] = 4;
	m_normalLaraSkinJointRemap[5][3] = 4;
	m_normalLaraSkinJointRemap[5][4] = 4;

	m_normalLaraSkinJointRemap[6][4] = 5;
	m_normalLaraSkinJointRemap[6][5] = 5;
	m_normalLaraSkinJointRemap[6][6] = 5;
	m_normalLaraSkinJointRemap[6][7] = 5;

	m_normalLaraSkinJointRemap[7][0] = 0;
	m_normalLaraSkinJointRemap[7][1] = 0;
	m_normalLaraSkinJointRemap[7][2] = 0;
	m_normalLaraSkinJointRemap[7][3] = 0;
	m_normalLaraSkinJointRemap[7][4] = 0;
	m_normalLaraSkinJointRemap[7][5] = 0;

	m_normalLaraSkinJointRemap[8][6] = 7;
	m_normalLaraSkinJointRemap[8][7] = 7;
	m_normalLaraSkinJointRemap[8][8] = 7;
	m_normalLaraSkinJointRemap[8][9] = 7;
	m_normalLaraSkinJointRemap[8][10] = 7;
	m_normalLaraSkinJointRemap[8][11] = 7;

	m_normalLaraSkinJointRemap[9][5] = 8;
	m_normalLaraSkinJointRemap[9][6] = 8;
	m_normalLaraSkinJointRemap[9][7] = 8;
	m_normalLaraSkinJointRemap[9][8] = 8;
	m_normalLaraSkinJointRemap[9][9] = 8;

	m_normalLaraSkinJointRemap[10][0] = 9;
	m_normalLaraSkinJointRemap[10][1] = 9;
	m_normalLaraSkinJointRemap[10][2] = 9;
	m_normalLaraSkinJointRemap[10][3] = 9;
	m_normalLaraSkinJointRemap[10][4] = 9;

	m_normalLaraSkinJointRemap[11][6] = 7;
	m_normalLaraSkinJointRemap[11][7] = 7;
	m_normalLaraSkinJointRemap[11][8] = 7;
	m_normalLaraSkinJointRemap[11][9] = 7;
	m_normalLaraSkinJointRemap[11][10] = 7;
	m_normalLaraSkinJointRemap[11][11] = 7;

	m_normalLaraSkinJointRemap[12][5] = 11;
	m_normalLaraSkinJointRemap[12][6] = 11;
	m_normalLaraSkinJointRemap[12][7] = 11;
	m_normalLaraSkinJointRemap[12][8] = 11;
	m_normalLaraSkinJointRemap[12][9] = 11;

	m_normalLaraSkinJointRemap[13][0] = 12;
	m_normalLaraSkinJointRemap[13][1] = 12;
	m_normalLaraSkinJointRemap[13][2] = 12;
	m_normalLaraSkinJointRemap[13][3] = 12;
	m_normalLaraSkinJointRemap[13][4] = 12;

	m_normalLaraSkinJointRemap[14][6] = 7;
	m_normalLaraSkinJointRemap[14][7] = 7;
	m_normalLaraSkinJointRemap[14][8] = 7;
	m_normalLaraSkinJointRemap[14][9] = 7;
	m_normalLaraSkinJointRemap[14][10] = 7;
	m_normalLaraSkinJointRemap[14][11] = 7;

	// Young Lara
	m_youngLaraSkinJointRemap[1][0] = 0; // Left up leg
	m_youngLaraSkinJointRemap[1][1] = 0;
	m_youngLaraSkinJointRemap[1][2] = 0;
	m_youngLaraSkinJointRemap[1][3] = 0;
	m_youngLaraSkinJointRemap[1][4] = 0;
	m_youngLaraSkinJointRemap[1][5] = 0;

	m_youngLaraSkinJointRemap[2][0] = 1; // Bottom left leg
	m_youngLaraSkinJointRemap[2][1] = 1;
	m_youngLaraSkinJointRemap[2][2] = 1;
	m_youngLaraSkinJointRemap[2][3] = 1;
	m_youngLaraSkinJointRemap[2][4] = 1;

	m_youngLaraSkinJointRemap[3][0] = 2; // Left foot
	m_youngLaraSkinJointRemap[3][1] = 2;
	m_youngLaraSkinJointRemap[3][2] = 2;
	m_youngLaraSkinJointRemap[3][3] = 2;

	m_youngLaraSkinJointRemap[4][6] = 0; // Right upper leg
	m_youngLaraSkinJointRemap[4][7] = 0;
	m_youngLaraSkinJointRemap[4][8] = 0;
	m_youngLaraSkinJointRemap[4][9] = 0;
	m_youngLaraSkinJointRemap[4][10] = 0;
	m_youngLaraSkinJointRemap[4][11] = 0;

	m_youngLaraSkinJointRemap[5][0] = 4; // Right bottom leg
	m_youngLaraSkinJointRemap[5][1] = 4;
	m_youngLaraSkinJointRemap[5][2] = 4;
	m_youngLaraSkinJointRemap[5][3] = 4;
	m_youngLaraSkinJointRemap[5][4] = 4;

	m_youngLaraSkinJointRemap[6][0] = 5; // Right foot
	m_youngLaraSkinJointRemap[6][1] = 5;
	m_youngLaraSkinJointRemap[6][2] = 5;
	m_youngLaraSkinJointRemap[6][3] = 5;

	m_youngLaraSkinJointRemap[7][0] = 0; // Torso
	m_youngLaraSkinJointRemap[7][1] = 0;
	m_youngLaraSkinJointRemap[7][2] = 0;
	m_youngLaraSkinJointRemap[7][3] = 0;
	m_youngLaraSkinJointRemap[7][4] = 0;
	m_youngLaraSkinJointRemap[7][5] = 0;

	m_youngLaraSkinJointRemap[8][0] = 7; // Left upper arm
	m_youngLaraSkinJointRemap[8][1] = 7;
	m_youngLaraSkinJointRemap[8][2] = 7;
	m_youngLaraSkinJointRemap[8][3] = 7;
	m_youngLaraSkinJointRemap[8][4] = 7;
	m_youngLaraSkinJointRemap[8][5] = 7;

	m_youngLaraSkinJointRemap[9][5] = 8; // Left bottom arm
	m_youngLaraSkinJointRemap[9][6] = 8;
	m_youngLaraSkinJointRemap[9][7] = 8;
	m_youngLaraSkinJointRemap[9][8] = 8;
	m_youngLaraSkinJointRemap[9][9] = 8;

	m_youngLaraSkinJointRemap[10][0] = 9; // Left hand
	m_youngLaraSkinJointRemap[10][1] = 9;
	m_youngLaraSkinJointRemap[10][2] = 9;
	m_youngLaraSkinJointRemap[10][3] = 9;
	m_youngLaraSkinJointRemap[10][4] = 9;

	m_youngLaraSkinJointRemap[11][0] = 7; // Right upper arm
	m_youngLaraSkinJointRemap[11][1] = 7;
	m_youngLaraSkinJointRemap[11][2] = 7;
	m_youngLaraSkinJointRemap[11][3] = 7;
	m_youngLaraSkinJointRemap[11][4] = 7;
	m_youngLaraSkinJointRemap[11][5] = 7;

	m_youngLaraSkinJointRemap[12][5] = 11; // Right low arm
	m_youngLaraSkinJointRemap[12][6] = 11;
	m_youngLaraSkinJointRemap[12][7] = 11;
	m_youngLaraSkinJointRemap[12][8] = 11;
	m_youngLaraSkinJointRemap[12][9] = 11;

	m_youngLaraSkinJointRemap[13][0] = 12; // Right arm
	m_youngLaraSkinJointRemap[13][1] = 12;
	m_youngLaraSkinJointRemap[13][2] = 12;
	m_youngLaraSkinJointRemap[13][3] = 12;
	m_youngLaraSkinJointRemap[13][4] = 12;

	m_youngLaraSkinJointRemap[14][0] = 7; // Head
	m_youngLaraSkinJointRemap[14][1] = 7;
	m_youngLaraSkinJointRemap[14][2] = 7;
	m_youngLaraSkinJointRemap[14][3] = 7;
	m_youngLaraSkinJointRemap[14][4] = 7;
	m_youngLaraSkinJointRemap[14][5] = 7;
}
