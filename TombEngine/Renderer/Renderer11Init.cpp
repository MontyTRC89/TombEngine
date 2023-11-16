#include "framework.h"

#include <string>
#include <memory>
#include <filesystem>

#include "Renderer/Renderer11.h"
#include "Renderer/Quad/RenderQuad.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/memory/Vector.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

using namespace TEN::Renderer;
using std::vector;

extern GameConfiguration g_Configuration;

static std::wstring GetAssetPath(const wchar_t* fileName)
{
	return TEN::Utils::ToWString(g_GameFlow->GetGameDir()) + fileName;
}

void TEN::Renderer::Renderer11::Initialize(int w, int h, bool windowed, HWND handle)
{
	TENLog("Initializing DX11...", LogLevel::Info);

	m_screenWidth = w;
	m_screenHeight = h;
	m_windowed = windowed;
	InitializeScreen(w, h, handle, false);
	InitializeCommonTextures();

	// Initialize render states
	m_states = std::make_unique<CommonStates>(m_device.Get());

	// Load shaders
	ComPtr<ID3D10Blob> blob;
	const D3D_SHADER_MACRO roomDefinesAnimated[] = { "ANIMATED", "", nullptr, nullptr };
	const D3D_SHADER_MACRO roomDefinesShadowMap[] = { "SHADOW_MAP", "", nullptr, nullptr };

	m_vsRooms = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "VS", "vs_4_0", nullptr, blob);
  
	// Initialize input layout using first vertex shader.
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ANIMATIONFRAMEOFFSET", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EFFECTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POLYINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "DRAWINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "HASH", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	Utils::throwIfFailed(m_device->CreateInputLayout(inputLayout, 12, blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout));

	m_vsRooms_Anim = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "VS", "vs_4_0", &roomDefinesAnimated[0], blob);
	m_psRooms = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "PS", "ps_4_1", nullptr, blob);
	m_vsItems = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Items.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psItems = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Items.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsStatics = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Statics.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psStatics = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Statics.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsHairs = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Hairs.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psHairs = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Hairs.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsSky = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Sky.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psSky = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Sky.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsSprites = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Sprites.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psSprites = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Sprites.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsSolid = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Solid.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psSolid = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Solid.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsInventory = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Inventory.fx"), "VS", "vs_4_0",nullptr, blob);
	m_psInventory = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_Inventory.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsFullScreenQuad = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_FullScreenQuad.fx"), "VS", "vs_4_0",nullptr, blob);
	m_psFullScreenQuad = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_FullScreenQuad.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsShadowMap = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_ShadowMap.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psShadowMap = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_ShadowMap.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsHUD = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_VS_HUD.hlsl"), "VS", "vs_4_0", nullptr, blob);
	m_psHUDColor = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUD.hlsl"), "PSColored", "ps_4_0", nullptr, blob);
	m_psHUDTexture = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUD.hlsl"), "PSTextured", "ps_4_0", nullptr, blob);
	m_psHUDBarColor = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUDBar.hlsl"), "PSTextured", "ps_4_0", nullptr, blob);
	m_vsFinalPass = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_FinalPass.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psFinalPass = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_FinalPass.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsInstancedStaticMeshes = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedStatics.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psInstancedStaticMeshes = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedStatics.fx"), "PS", "ps_4_0", nullptr, blob);
	m_vsInstancedSprites = Utils::compileVertexShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedSprites.fx"), "VS", "vs_4_0", nullptr, blob);
	m_psInstancedSprites = Utils::compilePixelShader(m_device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedSprites.fx"), "PS", "ps_4_0", nullptr, blob);
 
	// Initialize constant buffers
	m_cbCameraMatrices = CreateConstantBuffer<CCameraMatrixBuffer>();
	m_cbItem = CreateConstantBuffer<CItemBuffer>();
	m_cbStatic = CreateConstantBuffer<CStaticBuffer>();
	m_cbLights = CreateConstantBuffer<CLightBuffer>();
	m_cbMisc = CreateConstantBuffer<CMiscBuffer>();
	m_cbShadowMap = CreateConstantBuffer<CShadowLightBuffer>();
	m_cbRoom = CreateConstantBuffer<CRoomBuffer>();
	m_cbAnimated = CreateConstantBuffer<CAnimatedBuffer>();
	m_cbPostProcessBuffer = CreateConstantBuffer<CPostProcessBuffer>();
	m_cbBlending = CreateConstantBuffer<CBlendingBuffer>();
	m_cbInstancedSpriteBuffer = CreateConstantBuffer<CInstancedSpriteBuffer>();
	m_cbInstancedStaticMeshBuffer = CreateConstantBuffer<CInstancedStaticMeshBuffer>();
	m_cbSky = CreateConstantBuffer<CSkyBuffer>();

	//Prepare HUD Constant buffer  
	m_cbHUDBar = CreateConstantBuffer<CHUDBarBuffer>();
	m_cbHUD = CreateConstantBuffer<CHUDBuffer>();
	m_cbSprite = CreateConstantBuffer<CSpriteBuffer>();
	m_stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0));
	m_stHUD.Projection = Matrix::CreateOrthographicOffCenter(0, DISPLAY_SPACE_RES.x, 0, DISPLAY_SPACE_RES.y, 0, 1.0f);
	m_cbHUD.updateData(m_stHUD, m_context.Get());
	m_currentCausticsFrame = 0;

	// Preallocate lists
	m_dynamicLights = createVector<RendererLight>(MAX_DYNAMIC_LIGHTS);
	m_lines2DToDraw = createVector<RendererLine2D>(MAX_LINES_2D);
	m_lines3DToDraw = createVector<RendererLine3D>(MAX_LINES_3D);
	_triangles3DToDraw = createVector<RendererTriangle3D>(TRIANGLE_3D_COUNT_MAX);
	m_transparentFaces = createVector<RendererTransparentFace>(MAX_TRANSPARENT_FACES);
	m_transparentFacesVertices = createVector<RendererVertex>(MAX_TRANSPARENT_VERTICES);
	m_transparentFacesIndices.reserve(MAX_TRANSPARENT_VERTICES); // = createVector<int>(MAX_TRANSPARENT_VERTICES);

	for (int i = 0; i < NUM_ITEMS; i++)
	{
		m_items[i].LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
		m_effects[i].LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
	}

	m_transparentFacesVertexBuffer = VertexBuffer(m_device.Get(), TRANSPARENT_BUCKET_SIZE);
	m_transparentFacesIndexBuffer = IndexBuffer(m_device.Get(), TRANSPARENT_BUCKET_SIZE);

	D3D11_BLEND_DESC blendStateDesc{};
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.IndependentBlendEnable = false;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
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
	
	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};

	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.DepthClipEnable = true;
	rasterizerStateDesc.MultisampleEnable = true;
	rasterizerStateDesc.AntialiasedLineEnable = true;
	rasterizerStateDesc.ScissorEnable = true;
	Utils::throwIfFailed(m_device->CreateRasterizerState(&rasterizerStateDesc, m_cullCounterClockwiseRasterizerState.GetAddressOf()));

	rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.DepthClipEnable = true;
	rasterizerStateDesc.MultisampleEnable = true;
	rasterizerStateDesc.AntialiasedLineEnable = true;
	rasterizerStateDesc.ScissorEnable = true;
	Utils::throwIfFailed(m_device->CreateRasterizerState(&rasterizerStateDesc, m_cullClockwiseRasterizerState.GetAddressOf()));

	rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.DepthClipEnable = true;
	rasterizerStateDesc.MultisampleEnable = true;
	rasterizerStateDesc.AntialiasedLineEnable = true;
	rasterizerStateDesc.ScissorEnable = true;
	Utils::throwIfFailed(m_device->CreateRasterizerState(&rasterizerStateDesc, m_cullNoneRasterizerState.GetAddressOf()));

	InitializeGameBars();
	initQuad(m_device.Get());
	InitializeSky();
}

void TEN::Renderer::Renderer11::InitializeSky()
{
	RendererVertex vertices[SKY_VERTICES_COUNT];
	int indices[SKY_INDICES_COUNT];
	int size = SKY_SIZE;

	int lastVertex = 0;
	int lastIndex = 0;

	for (int x = 0; x < SKY_TILES_COUNT; x++)
	{
		for (int z = 0; z < SKY_TILES_COUNT; z++)
		{
			indices[lastIndex + 0] = lastVertex + 0;
			indices[lastIndex + 1] = lastVertex + 1;
			indices[lastIndex + 2] = lastVertex + 2;
			indices[lastIndex + 3] = lastVertex + 0;
			indices[lastIndex + 4] = lastVertex + 2;
			indices[lastIndex + 5] = lastVertex + 3;

			lastIndex += 6;

			vertices[lastVertex].Position.x = -size / 2.0f + x * 512.0f;
			vertices[lastVertex].Position.y = 0.0f;
			vertices[lastVertex].Position.z = -size / 2.0f + (z + 1) * 512.0f;
			vertices[lastVertex].UV.x = x / 20.0f;
			vertices[lastVertex].UV.y = (z + 1) / 20.0f;
			vertices[lastVertex].Color.x = 1.0f;
			vertices[lastVertex].Color.y = 1.0f;
			vertices[lastVertex].Color.z = 1.0f;
			vertices[lastVertex].Color.w = 1.0f;

			lastVertex++;

			vertices[lastVertex].Position.x = -size / 2.0f + (x + 1) * 512.0f;
			vertices[lastVertex].Position.y = 0.0f;
			vertices[lastVertex].Position.z = -size / 2.0f + (z + 1) * 512.0f;
			vertices[lastVertex].UV.x = (x + 1) / 20.0f;
			vertices[lastVertex].UV.y = (z + 1) / 20.0f;
			vertices[lastVertex].Color.x = 1.0f;
			vertices[lastVertex].Color.y = 1.0f;
			vertices[lastVertex].Color.z = 1.0f;
			vertices[lastVertex].Color.w = 1.0f;

			lastVertex++;

			vertices[lastVertex].Position.x = -size / 2.0f + (x + 1) * 512.0f;
			vertices[lastVertex].Position.y = 0.0f;
			vertices[lastVertex].Position.z = -size / 2.0f + z * 512.0f;
			vertices[lastVertex].UV.x = (x + 1) / 20.0f;
			vertices[lastVertex].UV.y = z / 20.0f;
			vertices[lastVertex].Color.x = 1.0f;
			vertices[lastVertex].Color.y = 1.0f;
			vertices[lastVertex].Color.z = 1.0f;
			vertices[lastVertex].Color.w = 1.0f;

			lastVertex++;

			vertices[lastVertex].Position.x = -size / 2.0f + x * 512.0f;
			vertices[lastVertex].Position.y = 0.0f;
			vertices[lastVertex].Position.z = -size / 2.0f + z * 512.0f;
			vertices[lastVertex].UV.x = x / 20.0f;
			vertices[lastVertex].UV.y = z / 20.0f;
			vertices[lastVertex].Color.x = 1.0f;
			vertices[lastVertex].Color.y = 1.0f;
			vertices[lastVertex].Color.z = 1.0f;
			vertices[lastVertex].Color.w = 1.0f;

			lastVertex++;
		}
	}

	m_skyVertexBuffer = VertexBuffer(m_device.Get(), SKY_VERTICES_COUNT, vertices);
	m_skyIndexBuffer = IndexBuffer(m_device.Get(), SKY_INDICES_COUNT, indices);
}

void TEN::Renderer::Renderer11::InitializeScreen(int w, int h, HWND handle, bool reset)
{
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = w;
	sd.BufferDesc.Height = h;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;
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

	Utils::throwIfFailed(dxgiFactory->CreateSwapChain(m_device.Get(), &sd, &m_swapChain));

	dxgiFactory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER);

	// Initialize the back buffer
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

	// Initialize sprite and primitive batches
	m_spriteBatch = std::make_unique<SpriteBatch>(m_context.Get());
	m_primitiveBatch = std::make_unique<PrimitiveBatch<RendererVertex>>(m_context.Get());

	// Initialize buffers
	m_renderTarget = RenderTarget2D(m_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D(m_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_depthMap = RenderTarget2D(m_device.Get(), w, h, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM);
	m_reflectionCubemap = RenderTargetCube(m_device.Get(), 128, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	m_shadowMap = Texture2DArray(m_device.Get(), g_Configuration.ShadowMapSize, 6, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM);

	// Initialize viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = w;
	m_viewport.Height = h;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_shadowMapViewport.TopLeftX = 0;
	m_shadowMapViewport.TopLeftY = 0;
	m_shadowMapViewport.Width = g_Configuration.ShadowMapSize;
	m_shadowMapViewport.Height = g_Configuration.ShadowMapSize;
	m_shadowMapViewport.MinDepth = 0.0f;
	m_shadowMapViewport.MaxDepth = 1.0f;

	m_viewportToolkit = Viewport(m_viewport.TopLeftX, m_viewport.TopLeftY, m_viewport.Width, m_viewport.Height,
		m_viewport.MinDepth, m_viewport.MaxDepth);

	SetFullScreen();
}

void Renderer11::InitializeCommonTextures()
{
	// Initialize font.
	auto fontPath = GetAssetPath(L"Textures/Font.spritefont");

	if (!std::filesystem::is_regular_file(fontPath))
		throw std::runtime_error("Font not found; path " + TEN::Utils::ToString(fontPath) + " is missing.");

	m_gameFont = std::make_unique<SpriteFont>(m_device.Get(), fontPath.c_str());

	// Initialize common textures.
	SetTextureOrDefault(m_logo, GetAssetPath(L"Textures/Logo.png"));
	SetTextureOrDefault(m_loadingBarBorder, GetAssetPath(L"Textures/LoadingBarBorder.png"));
	SetTextureOrDefault(m_loadingBarInner, GetAssetPath(L"Textures/LoadingBarInner.png"));
	SetTextureOrDefault(m_whiteTexture, GetAssetPath(L"Textures/WhiteSprite.png"));

	m_whiteSprite.Height = m_whiteTexture.Height;
	m_whiteSprite.Width = m_whiteTexture.Width;
	m_whiteSprite.UV[0] = Vector2(0.0f, 0.0f);
	m_whiteSprite.UV[1] = Vector2(1.0f, 0.0f);
	m_whiteSprite.UV[2] = Vector2(1.0f, 1.0f);
	m_whiteSprite.UV[3] = Vector2(0.0f, 1.0f);
	m_whiteSprite.Texture = &m_whiteTexture;
}

void TEN::Renderer::Renderer11::Create()
{
	TENLog("Creating DX11 renderer device...", LogLevel::Info);

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_10_1 }; 
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT res;

	if constexpr (DebugBuild)
	{
		res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, 
			levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context);
	}	
	else
	{
		res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, 
			levels, 1, D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context);
	}

	Utils::throwIfFailed(res);
}

void Renderer11::ToggleFullScreen(bool force)
{
	m_windowed = force ? false : !m_windowed;
	SetFullScreen();
}

void Renderer11::SetFullScreen()
{
	if (!m_windowed)
	{
		SetWindowLongPtr(WindowsHandle, GWL_STYLE, 0);
		SetWindowLongPtr(WindowsHandle, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowWindow(WindowsHandle, SW_SHOWMAXIMIZED);
	}
	else
	{
		int frameW = GetSystemMetrics(SM_CXPADDEDBORDER);
		int frameX = GetSystemMetrics(SM_CXSIZEFRAME);
		int frameY = GetSystemMetrics(SM_CYSIZEFRAME);
		int frameC = GetSystemMetrics(SM_CYCAPTION);

		int borderWidth  = (frameX + frameW) * 2;
		int borderHeight = (frameY + frameW) * 2 + frameC;

		SetWindowLongPtr(WindowsHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowLongPtr(WindowsHandle, GWL_EXSTYLE, 0);
		ShowWindow(WindowsHandle, SW_SHOWNORMAL);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, m_screenWidth + borderWidth, m_screenHeight + borderHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	UpdateWindow(WindowsHandle);
}