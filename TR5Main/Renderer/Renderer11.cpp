#include "framework.h"
#include "Renderer11.h"
#include "input.h"
#include "winmain.h"
#include "level.h"
#include "configuration.h"
#include "draw.h"
#include "health.h"
#include "pickup.h"
#include "inventory.h"
#include "gameflow.h"
#include "Lara.h"
#include "effect2.h"
#include "rope.h"
#include "camera.h"
#include "tomb4fx.h"
#include "trmath.h"
#include "misc.h"
#include "footprint.h"
#include "setup.h"
#include "Utils.h"
#include "VertexBuffer/VertexBuffer.h"
namespace T5M::Renderer {
	using namespace T5M::Renderer::Utils;
	using std::array;
	Renderer11 g_Renderer;
	Renderer11::Renderer11() {
		initialiseHairRemaps();

		m_blinkColorDirection = 1;
	}

	Renderer11::~Renderer11() {
		FreeRendererData();

		DX11_RELEASE(m_backBufferRTV);
		DX11_RELEASE(m_backBufferTexture);
		DX11_RELEASE(m_depthStencilState);
		DX11_RELEASE(m_depthStencilTexture);
		DX11_RELEASE(m_depthStencilView);

		DX11_DELETE(m_primitiveBatch);
		DX11_DELETE(m_spriteBatch);
		DX11_DELETE(m_gameFont);
		DX11_DELETE(m_states);

		DX11_RELEASE(m_vsRooms);
		DX11_RELEASE(m_psRooms);
		DX11_RELEASE(m_vsItems);
		DX11_RELEASE(m_psItems);
		DX11_RELEASE(m_vsStatics);
		DX11_RELEASE(m_psStatics);
		DX11_RELEASE(m_vsHairs);
		DX11_RELEASE(m_psHairs);
		DX11_RELEASE(m_vsSky);
		DX11_RELEASE(m_psSky);
		DX11_RELEASE(m_vsSprites);
		DX11_RELEASE(m_psSprites);
		DX11_RELEASE(m_vsSolid);
		DX11_RELEASE(m_psSolid);
		DX11_RELEASE(m_vsInventory);
		DX11_RELEASE(m_psInventory);
		DX11_RELEASE(m_vsFullScreenQuad);
		DX11_RELEASE(m_psFullScreenQuad);
		DX11_RELEASE(m_cbCameraMatrices);
		DX11_RELEASE(m_cbItem);
		DX11_RELEASE(m_cbStatic);
		DX11_RELEASE(m_cbLights);
		DX11_RELEASE(m_cbMisc);

		DX11_RELEASE(m_swapChain);
		DX11_RELEASE(m_context);
		DX11_RELEASE(m_device);
	}

	void Renderer11::FreeRendererData() {
		m_meshPointersToMesh.clear();

		for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
			DX11_DELETE(m_moveableObjects[i]);
		free(m_moveableObjects);

		for (int i = 0; i < g_NumSprites; i++)
			DX11_DELETE(m_sprites[i]);
		free(m_sprites);

		for (int i = 0; i < MAX_STATICS; i++)
			DX11_DELETE(m_staticObjects[i]);
		free(m_staticObjects);

		m_rooms.clear();
		m_roomTextures.clear();
		m_moveablesTextures.clear();
		m_staticsTextures.clear();
		m_spritesTextures.clear();
	}

	void Renderer11::clearSceneItems() {
		m_roomsToDraw.clear();
		m_itemsToDraw.clear();
		m_effectsToDraw.clear();
		m_lightsToDraw.clear();
		m_staticsToDraw.clear();
		m_spritesToDraw.clear();
		m_lines3DToDraw.clear();
		m_lines2DToDraw.clear();
	}

	int Renderer11::SyncRenderer() {
		// Sync the renderer
		int nf = Sync();
		if (nf < 2) {
			int i = 2 - nf;
			nf = 2;
			do {
				while (!Sync());
				i--;
			} while (i);
		}

		GnFrameCounter++;
		return nf;
	}

	ID3D11VertexShader* Renderer11::compileVertexShader(const wchar_t* fileName, const char* function, const char* model, ID3D10Blob** bytecode) {
		HRESULT res;

		*bytecode = NULL;
		ID3DBlob* errors = NULL;

		printf("Compiling vertex shader: %s\n", fileName);
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
		res = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, function, model, flags, 0, bytecode, &errors);
		throwIfFailed(res);


		ID3D11VertexShader* shader = NULL;
		res = m_device->CreateVertexShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader);
		throwIfFailed(res);

		return shader;
	}

	ID3D11PixelShader* Renderer11::compilePixelShader(const wchar_t* fileName, const char* function, const char* model, ID3D10Blob** bytecode) {
		HRESULT res;

		*bytecode = NULL;
		ID3DBlob* errors = NULL;

		printf("Compiling pixel shader: %s\n", fileName);
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
		throwIfFailed(D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, function, model, flags, 0, bytecode, &errors));

		ID3D11PixelShader* shader = NULL;
		throwIfFailed(m_device->CreatePixelShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader));
		return shader;
	}

	ID3D11GeometryShader* Renderer11::compileGeometryShader(const wchar_t* fileName) {
		HRESULT res;

		ID3DBlob* bytecode = NULL;
		ID3DBlob* errors = NULL;

		res = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, &bytecode, &errors);
		if (FAILED(res))
			return NULL;

		ID3D11GeometryShader* shader = NULL;
		res = m_device->CreateGeometryShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
		if (FAILED(res))
			return NULL;

		return shader;
	}

	ID3D11ComputeShader* Renderer11::compileComputeShader(const wchar_t* fileName) {
		HRESULT res;

		ID3DBlob* bytecode = NULL;
		ID3DBlob* errors = NULL;

		res = D3DCompileFromFile(fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, &bytecode, &errors);
		if (FAILED(res))
			return NULL;

		ID3D11ComputeShader* shader = NULL;
		res = m_device->CreateComputeShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
		if (FAILED(res))
			return NULL;

		return shader;
	}

	void Renderer11::UpdateProgress(float value) {
		m_progress = value;
	}


	ID3D11Buffer* Renderer11::createConstantBuffer(size_t size) {
		ID3D11Buffer* buffer;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

		desc.ByteWidth = size; // Constant buffer must have a size multiple of 16 bytes
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT res = m_device->CreateBuffer(&desc, NULL, &buffer);
		if (FAILED(res))
			return NULL;

		return buffer;
	}

	RendererHUDBar::RendererHUDBar(ID3D11Device* m_device, int x, int y, int w, int h, int borderSize, array<Vector4, 9> colors) {
		array<Vector3, 9> barVertices = {
			Vector3(x, HUD_ZERO_Y + y, 0.5),
			Vector3(x + (w / 2.0f), HUD_ZERO_Y + y, 0.5),
			Vector3(x + w, HUD_ZERO_Y + y, 0.5),
			Vector3(x, HUD_ZERO_Y + (y + h / 2.0f), 0.5),
			Vector3(x + (w / 2.0f), HUD_ZERO_Y + (y + h / 2.0f), 0.5),
			Vector3(x + w, HUD_ZERO_Y + (y + h / 2.0f), 0.5),
			Vector3(x, HUD_ZERO_Y + y + h, 0.5),
			Vector3(x + (w / 2.0f), HUD_ZERO_Y + y + h, 0.5),
			Vector3(x + w, HUD_ZERO_Y + y + h, 0.5),

		};
		const float HUD_BORDER_WIDTH = borderSize * (REFERENCE_RES_WIDTH / REFERENCE_RES_HEIGHT);
		const float HUD_BORDER_HEIGT = borderSize;
		array<Vector3, 16> barBorderVertices = {
			//top left
			Vector3(x - HUD_BORDER_WIDTH	,HUD_ZERO_Y + y - HUD_BORDER_HEIGT,0),
			Vector3(x						,HUD_ZERO_Y + y - HUD_BORDER_HEIGT,0),
			Vector3(x						,HUD_ZERO_Y + y,0),
			Vector3(x - HUD_BORDER_WIDTH	,HUD_ZERO_Y + y,0),
			//top right
			Vector3(x + w					,HUD_ZERO_Y + y - HUD_BORDER_HEIGT,0),
			Vector3(x + w + HUD_BORDER_WIDTH,HUD_ZERO_Y + y - HUD_BORDER_HEIGT,0),
			Vector3(x + w + HUD_BORDER_WIDTH,HUD_ZERO_Y + y,0),
			Vector3(x + w					,HUD_ZERO_Y + y,0),
			//bottom right
			Vector3(x + w					,HUD_ZERO_Y + y + h,0),
			Vector3(x + w + HUD_BORDER_WIDTH,HUD_ZERO_Y + y + h,0),
			Vector3(x + w + HUD_BORDER_WIDTH,HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT,0),
			Vector3(x + w					,HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT,0),
			//bottom left
			Vector3(x - HUD_BORDER_WIDTH	,HUD_ZERO_Y + y + h,0),
			Vector3(x						,HUD_ZERO_Y + y + h,0),
			Vector3(x						,HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT,0),
			Vector3(x - HUD_BORDER_WIDTH	,HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT,0)
		};

		array<Vector2, 9> barUVs = {
			Vector2(0,0),
			Vector2(0.5,0),
			Vector2(1,0),
			Vector2(0,0.5),
			Vector2(0.5,0.5),
			Vector2(1,0.5),
			Vector2(0,1),
			Vector2(0.5,1),
			Vector2(1,1),
		};
		array<Vector2, 16> barBorderUVs = {
			//top left
			Vector2(0,0),
			Vector2(0.25,0),
			Vector2(0.25,0.25),
			Vector2(0,0.25),
			//top right
			Vector2(0.75,0),
			Vector2(1,0),
			Vector2(1,0.25),
			Vector2(0.75,0.25),
			//bottom right
			Vector2(0.75,0.75),
			Vector2(1,0.75),
			Vector2(1,1),
			Vector2(0.75,1),
			//bottom left
			Vector2(0,0.75),
			Vector2(0.25,0.75),
			Vector2(0.25,1),
			Vector2(0,1),
		};

		array<int, 24> barIndices = {
			0,1,3,1,4,3,
			//
			1,2,4,2,5,4,
			//
			3,4,6,4,7,6,
			//
			4,5,7,5,8,7
		};
		array<int, 56> barBorderIndices = {
			//top left
			0,1,3,1,2,3,
			//top center
			1,4,2,4,7,2,
			//top right
			4,5,7,5,6,7,
			//right
			7,6,8,6,9,8,
			//bottom right
			8,9,11,9,10,11,
			//bottom
			13,8,14,8,11,14,
			//bottom left
			12,13,15,13,14,15,
			//left
			3,2,12,2,13,12,
			//center
			2,7,13,7,8,13
		};
		array<RendererVertex, 9> vertices;
		for (int i = 0; i < 9; i++) {

			vertices[i].Position = barVertices[i];
			vertices[i].Color = colors[i];
			vertices[i].UV = barUVs[i];
			vertices[i].Normal = Vector3(0, 0, 0);
			vertices[i].Bone = 0.0f;
		}
		vertexBuffer = VertexBuffer(m_device, vertices.size(), vertices.data());
		indexBuffer = IndexBuffer(m_device, barIndices.size(), barIndices.data());

		array<RendererVertex, barBorderVertices.size()> verticesBorder;
		for (int i = 0; i < barBorderVertices.size(); i++) {
			verticesBorder[i].Position = barBorderVertices[i];
			verticesBorder[i].Color = Vector4(1, 1, 1, 1);
			verticesBorder[i].UV = barBorderUVs[i];
			verticesBorder[i].Normal = Vector3(0, 0, 0);
			verticesBorder[i].Bone = 0.0f;
		}
		vertexBufferBorder = VertexBuffer(m_device, verticesBorder.size(), verticesBorder.data());
		indexBufferBorder = IndexBuffer(m_device, barBorderIndices.size(), barBorderIndices.data());

	}

}
