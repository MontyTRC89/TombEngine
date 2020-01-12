#include "Renderer11.h"

#include "..\Specific\input.h"
#include "..\Specific\winmain.h"
#include "..\Specific\roomload.h"
#include "..\Specific\game.h"
#include "..\Specific\configuration.h"

#include "..\Game\draw.h"
#include "..\Game\healt.h"
#include "..\Game\pickup.h"
#include "..\Game\inventory.h"
#include "..\Game\gameflow.h"
#include "..\Game\Lara.h"
#include "..\Game\effect2.h"
#include "..\Game\rope.h"
#include "..\Game\items.h"
#include "..\Game\Camera.h"
#include "..\Game\healt.h"
#include "../Game/tomb4fx.h"
#include "math.h"
#include <D3Dcompiler.h>
#include <chrono> 
#include <stack>
#include "../Game/misc.h"
#include "../Game/footprint.h"

extern std::deque<FOOTPRINT_STRUCT> footprints;

Renderer11::Renderer11()
{
	initialiseHairRemaps();

	m_blinkColorDirection = 1;
}

Renderer11::~Renderer11()
{
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

	for (int i = 0; i < NUM_CAUSTICS_TEXTURES; i++)
		DX11_DELETE(m_caustics[i]);

	DX11_DELETE(m_titleScreen);
	DX11_DELETE(m_binocularsTexture);
	DX11_DELETE(m_whiteTexture);
	DX11_DELETE(m_logo);

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

	DX11_DELETE(m_renderTarget);
	DX11_DELETE(m_dumpScreenRenderTarget);
	DX11_DELETE(m_shadowMap);

	DX11_RELEASE(m_swapChain);
	DX11_RELEASE(m_context);
	DX11_RELEASE(m_device);
}

void Renderer11::FreeRendererData()
{
	m_meshPointersToMesh.clear();

	for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
		DX11_DELETE(m_moveableObjects[i]);
	free(m_moveableObjects);

	for (int i = 0; i < g_NumSprites; i++)
		DX11_DELETE(m_sprites[i]);
	free(m_sprites);

	for (int i = 0; i < NUM_STATICS; i++)
		DX11_DELETE(m_staticObjects[i]);
	free(m_staticObjects);

	m_rooms.clear();

	DX11_DELETE(m_textureAtlas);
	DX11_DELETE(m_skyTexture);
	DX11_DELETE(m_roomsVertexBuffer);
	DX11_DELETE(m_roomsIndexBuffer);
	DX11_DELETE(m_moveablesVertexBuffer);
	DX11_DELETE(m_moveablesIndexBuffer);
	DX11_DELETE(m_staticsVertexBuffer);
	DX11_DELETE(m_staticsIndexBuffer);
}

void Renderer11::clearSceneItems()
{
	m_roomsToDraw.clear();
	m_itemsToDraw.clear();
	m_effectsToDraw.clear();
	m_lightsToDraw.clear();
	m_staticsToDraw.clear();
	m_spritesToDraw.clear();
	m_lines3DToDraw.clear();
	m_lines2DToDraw.clear();
}

int Renderer11::SyncRenderer()
{
	// Sync the renderer
	int nf = Sync();
	if (nf < 2)
	{
		int i = 2 - nf;
		nf = 2;
		do
		{
			while (!Sync());
			i--;
		} while (i);
	}

	GnFrameCounter++;
	return nf;
}

ID3D11VertexShader* Renderer11::compileVertexShader(const char* fileName, const char* function, const char* model, ID3D10Blob** bytecode)
{
	HRESULT res;

	*bytecode = NULL;
	ID3DBlob* errors = NULL;

	printf("Compiling vertex shader: %s\n", fileName);

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, function, model, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, bytecode, &errors, NULL);
	if (FAILED(res))
	{
		printf("Compilation failed: %s\n", errors->GetBufferPointer());
		return NULL;
	}

	ID3D11VertexShader* shader = NULL;
	res = m_device->CreateVertexShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11PixelShader* Renderer11::compilePixelShader(const char* fileName, const char* function, const char* model, ID3D10Blob** bytecode)
{
	HRESULT res;

	*bytecode = NULL;
	ID3DBlob* errors = NULL;

	printf("Compiling pixel shader: %s\n", fileName);

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, function, model, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, bytecode, &errors, NULL);
	if (FAILED(res))
	{
		printf("Compilation failed: %s\n", errors->GetBufferPointer());
		return NULL;
	}

	ID3D11PixelShader* shader = NULL;
	res = m_device->CreatePixelShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11GeometryShader* Renderer11::compileGeometryShader(const char* fileName)
{
	HRESULT res;

	ID3DBlob* bytecode = NULL;
	ID3DBlob* errors = NULL;

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, &bytecode, &errors, NULL);
	if (FAILED(res))
		return NULL;

	ID3D11GeometryShader* shader = NULL;
	res = m_device->CreateGeometryShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11ComputeShader* Renderer11::compileComputeShader(const char* fileName)
{
	HRESULT res;

	ID3DBlob* bytecode = NULL;
	ID3DBlob* errors = NULL;

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, &bytecode, &errors, NULL);
	if (FAILED(res))
		return NULL;

	ID3D11ComputeShader* shader = NULL;
	res = m_device->CreateComputeShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

void Renderer11::UpdateProgress(float value)
{
	m_progress = value;
}


ID3D11Buffer* Renderer11::createConstantBuffer(int size)
{
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC desc;	
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		
	desc.ByteWidth = ceil(size / 16) * 16; // Constant buffer must have a size multiple of 16 bytes
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT res = m_device->CreateBuffer(&desc, NULL, &buffer);
	if (FAILED(res))
		return NULL;

	return buffer;
}




RendererHUDBar::RendererHUDBar(ID3D11Device* m_device,int x, int y, int w, int h, int borderSize, array<Vector4,9> colors)
{

	array<Vector3, 9> barVertices = {
		Vector3(x, y, 0),
		Vector3(x + (w / 2.0f), y, 0),
		Vector3(x + w, y, 0),
		Vector3(x, (y + h / 2.0f), 0),
		Vector3(x + (w / 2.0f), (y + h / 2.0f), 0),
		Vector3(x + w, (y + h / 2.0f), 0),
		Vector3(x, y + h, 0),
		Vector3(x + (w / 2.0f), y + h, 0),
		Vector3(x + w, y + h, 0),

	};
	array<Vector3, 16> barBorderVertices = {
		//top left
		Vector3(x - HUD_UNIT_X,y - HUD_UNIT_Y,0),
		Vector3(x,y - HUD_UNIT_Y,0),
		Vector3(x,y,0),
		Vector3(x - HUD_UNIT_X,y,0),
		//top right
		Vector3(x + w,y - HUD_UNIT_Y,0),
		Vector3(x + w + HUD_UNIT_X,y - HUD_UNIT_Y,0),
		Vector3(x + w + HUD_UNIT_X,y,0),
		Vector3(x + w,y,0),
		//bottom right
		Vector3(x + w,1,0),
		Vector3(x + w + HUD_UNIT_X,y + h,0),
		Vector3(x + w + HUD_UNIT_X,y + h + HUD_UNIT_Y,0),
		Vector3(x + w,y + h + HUD_UNIT_Y,0),
		//bottom left
		Vector3(x - HUD_UNIT_X,y + h,0),
		Vector3(x,y + h,0),
		Vector3(x,y + h + HUD_UNIT_Y,0),
		Vector3(x - HUD_UNIT_X,y + h + HUD_UNIT_Y,0)
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
	Matrix hudMatrix = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0))* Matrix::CreateOrthographic(REFERENCE_RES_WIDTH, REFERENCE_RES_HEIGHT, -10, 10);
	Vector4 vec = XMVector4Transform(Vector4(x + (w / 2.0f), y, 0, 1), hudMatrix);
	array<RendererVertex, 9> vertices;
	for (int i = 0; i < 9; i++) {

		vertices[i].Position = barVertices[i];
		vertices[i].Color = colors[i];
		vertices[i].UV = barUVs[i];
	}
	vertexBuffer = VertexBuffer::Create(m_device, vertices.size(), vertices._Elems);
	indexBuffer = IndexBuffer::Create(m_device, barIndices.size(), barIndices._Elems);
}
