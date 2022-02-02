#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Specific/winmain.h"
#include "Scripting/GameFlowScript.h"
#include "Quad/RenderQuad.h"
#include "Specific/memory/Vector.h"
#include <string>
#include <memory>
#include <filesystem>

using namespace TEN::Renderer;
using std::vector;
extern GameConfiguration g_Configuration;

void TEN::Renderer::Renderer11::Initialise(int w, int h, int refreshRate, bool windowed, HWND handle)
{
	HRESULT res;

	TENLog("Initializing DX11...", LogLevel::Info);

	CoInitialize(NULL);

	ScreenWidth = w;
	ScreenHeight = h;
	Windowed = windowed;
	initialiseScreen(w, h, refreshRate, windowed, handle, false);

	// Initialise render states
	m_states = std::make_unique<CommonStates>(m_device.Get());

	wchar_t titleScreenFile[255];
	std::wstring titleFile = std::wstring(titleScreenFile);
	std::mbstowcs(titleScreenFile, g_GameFlow->TitleScreenImagePath.c_str(), 255);
	m_titleScreen = Texture2D(m_device.Get(), titleScreenFile);

	auto whiteSpriteName = L"Textures/WhiteSprite.png";
	m_whiteTexture = std::filesystem::exists(whiteSpriteName) ? Texture2D(m_device.Get(), L"Textures/WhiteSprite.png") : Texture2D();

	auto logoName = L"Textures/Logo.png";
	m_logo = std::filesystem::exists(logoName) ? Texture2D(m_device.Get(), logoName) : Texture2D();

	m_shadowMaps = RenderTargetCubeArray(m_device.Get(), g_Configuration.shadowMapSize, MAX_DYNAMIC_SHADOWS, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_D16_UNORM);
	// Load shaders
	ComPtr<ID3D10Blob> blob;
	//char shadowMapStringBuff[4];
	//_itoa(g_Configuration.shadowMapSize, shadowMapStringBuff,10);
	std::string shadowSizeString = std::to_string(g_Configuration.shadowMapSize);
	const D3D_SHADER_MACRO roomDefines[] = {"SHADOW_MAP_SIZE",shadowSizeString.c_str(),nullptr,nullptr};
	const D3D_SHADER_MACRO roomDefinesAnimated[] = { "SHADOW_MAP_SIZE",shadowSizeString.c_str(),"ANIMATED" ,"",nullptr,nullptr };
	   
	m_vsRooms = Utils::compileVertexShader(m_device.Get(),L"Shaders\\DX11_Rooms.fx", "VS", "vs_4_0", &roomDefines[0], blob);
	// Initialise input layout using the first vertex shader
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"EFFECTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"POLYINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"DRAWINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"HASH", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	Utils::throwIfFailed(m_device->CreateInputLayout(inputLayout, 11, blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout));

	m_vsRooms_Anim = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Rooms.fx", "VS", "vs_4_0", &roomDefinesAnimated[0], blob);
	m_psRooms = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Rooms.fx", "PS", "ps_4_0", &roomDefines[0], blob);
	m_vsItems = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Items.fx", "VS", "vs_4_0", nullptr, blob);
	m_psItems = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Items.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsStatics = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Statics.fx", "VS", "vs_4_0", nullptr, blob);
	m_psStatics = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Statics.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsHairs = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Hairs.fx", "VS", "vs_4_0", nullptr, blob);
	m_psHairs = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Hairs.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsSky = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Sky.fx", "VS", "vs_4_0", nullptr, blob);
	m_psSky = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Sky.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsSprites = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Sprites.fx", "VS", "vs_4_0", nullptr, blob);
	m_psSprites = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Sprites.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsSolid = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Solid.fx", "VS", "vs_4_0", nullptr, blob);
	m_psSolid = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Solid.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsInventory = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_Inventory.fx", "VS", "vs_4_0",nullptr, blob);
	m_psInventory = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_Inventory.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsFullScreenQuad = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_FullScreenQuad.fx", "VS", "vs_4_0",nullptr, blob);
	m_psFullScreenQuad = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_FullScreenQuad.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsShadowMap = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_ShadowMap.fx", "VS", "vs_4_0", nullptr, blob);
	m_psShadowMap = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_ShadowMap.fx", "PS", "ps_4_0", nullptr, blob);
	m_vsHUD = Utils::compileVertexShader(m_device.Get(), L"Shaders\\HUD\\DX11_VS_HUD.hlsl", "VS", "vs_4_0", nullptr, blob);
	m_psHUDColor = Utils::compilePixelShader(m_device.Get(), L"Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSColored", "ps_4_0", nullptr, blob);
	m_psHUDTexture = Utils::compilePixelShader(m_device.Get(), L"Shaders\\HUD\\DX11_PS_HUD.hlsl", "PSTextured", "ps_4_0", nullptr, blob);
	m_psHUDBarColor = Utils::compilePixelShader(m_device.Get(), L"Shaders\\HUD\\DX11_PS_HUDBar.hlsl", "PSTextured", "ps_4_0", nullptr, blob);
	m_vsFinalPass = Utils::compileVertexShader(m_device.Get(), L"Shaders\\DX11_FinalPass.fx", "VS", "vs_4_0", nullptr, blob);
	m_psFinalPass = Utils::compilePixelShader(m_device.Get(), L"Shaders\\DX11_FinalPass.fx", "PS", "ps_4_0", nullptr, blob);
	
	m_shadowMap = RenderTarget2D(m_device.Get(), g_Configuration.shadowMapSize, g_Configuration.shadowMapSize, DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_D16_UNORM);

	// Initialise constant buffers
	m_cbCameraMatrices = createConstantBuffer<CCameraMatrixBuffer>();
	m_cbItem = createConstantBuffer<CItemBuffer>();
	m_cbStatic = createConstantBuffer<CStaticBuffer>();
	m_cbLights = createConstantBuffer<CLightBuffer>();
	m_cbMisc = createConstantBuffer<CMiscBuffer>();
	m_cbShadowMap = createConstantBuffer<CShadowLightBuffer>();
	m_cbRoom = createConstantBuffer<CRoomBuffer>();
	m_cbAnimated = createConstantBuffer<CAnimatedBuffer>();
	m_cbPostProcessBuffer = createConstantBuffer<CPostProcessBuffer>();

	//Prepare HUD Constant buffer
	m_cbHUDBar = createConstantBuffer<CHUDBarBuffer>();
	m_cbHUD = createConstantBuffer<CHUDBuffer>();
	m_cbSprite = createConstantBuffer<CSpriteBuffer>();
	m_stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0));
	m_stHUD.Projection = Matrix::CreateOrthographicOffCenter(0, REFERENCE_RES_WIDTH, 0, REFERENCE_RES_HEIGHT, 0, 1.0f);
	m_cbHUD.updateData(m_stHUD, m_context.Get());
	m_currentCausticsFrame = 0;
	m_firstWeather = true;

	// Preallocate lists
	dynamicLights = createVector<RendererLight>(MAX_DYNAMIC_LIGHTS);
	m_lines3DToDraw = createVector<RendererLine3D>(MAX_LINES_3D);
	m_lines2DToDraw = createVector<RendererLine2D>(MAX_LINES_2D);
	m_transparentFaces = createVector<RendererTransparentFace>(MAX_TRANSPARENT_FACES);
	m_transparentFacesVertices = createVector<RendererVertex>(MAX_TRANSPARENT_VERTICES);
	m_transparentFacesIndices.reserve(MAX_TRANSPARENT_VERTICES); // = createVector<int>(MAX_TRANSPARENT_VERTICES);

	for (int i = 0; i < NUM_ITEMS; i++)
	{
		m_items[i].LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
		m_effects[i].Lights = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
	}

	m_transparentFacesVertexBuffer = VertexBuffer(m_device.Get(), TRANSPARENT_BUCKET_SIZE);
	m_transparentFacesIndexBuffer = IndexBuffer(m_device.Get(), TRANSPARENT_BUCKET_SIZE);

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
	Utils::throwIfFailed(m_device->CreateBlendState(&blendStateDesc, m_subtractiveBlendState.GetAddressOf()));
	
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Utils::throwIfFailed(m_device->CreateBlendState(&blendStateDesc, m_screenBlendState.GetAddressOf()));

	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Utils::throwIfFailed(m_device->CreateBlendState(&blendStateDesc, m_lightenBlendState.GetAddressOf()));

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
	Utils::throwIfFailed(m_device->CreateBlendState(&blendStateDesc, m_excludeBlendState.GetAddressOf()));

	D3D11_SAMPLER_DESC shadowSamplerDesc = {};
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	Utils::throwIfFailed(m_device->CreateSamplerState(&shadowSamplerDesc,m_shadowSampler.GetAddressOf()));
	m_shadowSampler->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("ShadowSampler") + 1, "ShadowSampler");
	initialiseBars();
	initQuad(m_device.Get());
}

void TEN::Renderer::Renderer11::initialiseScreen(int w, int h, int refreshRate, bool windowed, HWND handle, bool reset)
{
	HRESULT res;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = w;
	sd.BufferDesc.Height = h;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.Windowed = windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = handle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	ComPtr<IDXGIDevice> dxgiDevice;
	Utils::throwIfFailed(m_device.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	Utils::throwIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter));

	ComPtr<IDXGIFactory> dxgiFactory;
	Utils::throwIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

	if (reset)
	{
		// Always return to windowed mode otherwise crash will happen
		m_swapChain->SetFullscreenState(false, NULL);
	}

	Utils::throwIfFailed(dxgiFactory->CreateSwapChain(m_device.Get(), &sd, &m_swapChain));


	dxgiFactory->MakeWindowAssociation(handle, 0);
	res = m_swapChain->SetFullscreenState(!windowed, NULL);

	// Initialise the back buffer
	m_backBufferTexture = NULL;
	Utils::throwIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast <void**>(&m_backBufferTexture)));


	m_backBufferRTV = NULL;
	Utils::throwIfFailed(m_device->CreateRenderTargetView(m_backBufferTexture, NULL, &m_backBufferRTV));


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
	Utils::throwIfFailed(m_device->CreateTexture2D(&depthStencilDesc, NULL, &m_depthStencilTexture));


	m_depthStencilView = NULL;
	Utils::throwIfFailed(m_device->CreateDepthStencilView(m_depthStencilTexture, NULL, &m_depthStencilView));


	// Bind the back buffer and the depth stencil
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);

	// Initialise sprites and font
	m_spriteBatch = std::make_unique<SpriteBatch>(m_context.Get());
	m_gameFont = std::make_unique<SpriteFont>(m_device.Get(), L"Textures/Font.spritefont");
	m_primitiveBatch = std::make_unique<PrimitiveBatch<RendererVertex>>(m_context.Get());

	// Initialise buffers
	m_renderTarget = RenderTarget2D(m_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D(m_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_reflectionCubemap = RenderTargetCube(m_device.Get(), 128, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
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
	m_shadowMapViewport.Width = g_Configuration.shadowMapSize;
	m_shadowMapViewport.Height = g_Configuration.shadowMapSize;
	m_shadowMapViewport.MinDepth = 0.0f;
	m_shadowMapViewport.MaxDepth = 1.0f;

	m_viewportToolkit = Viewport(m_viewport.TopLeftX, m_viewport.TopLeftY, m_viewport.Width, m_viewport.Height,
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

}

void TEN::Renderer::Renderer11::Create()
{

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_10_1 }; // {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1};
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT res;

#ifdef _RELEASE
	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context);
#else
	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, /*D3D11_CREATE_DEVICE_DEBUG*/ NULL, levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context); // D3D11_CREATE_DEVICE_DEBUG
#endif
	Utils::throwIfFailed(res);

}
