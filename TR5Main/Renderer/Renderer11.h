#pragma once

#include <vector>
#include <string>
#include <array>
#include "Enums.h"

#include <D3D11.h>

#include <SimpleMath.h>
#include <CommonStates.h>
#include <DirectXHelpers.h>
#include <Effects.h>
#include <GeometricPrimitive.h>
#include <PostProcess.h>
#include <PrimitiveBatch.h>
#include <ScreenGrab.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>

#include "..\Global\global.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f

#define DX11_RELEASE(x) if (x != NULL) x->Release()
#define DX11_DELETE(x) if (x != NULL) delete x

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

template <class T>
class PreallocatedVector 
{
private:
	T*			m_objects;
	__int32		m_maxItems;
	__int32		m_numItems;
	__int32		m_startSize;

public:
	PreallocatedVector()
	{
		m_objects = NULL;
		m_size = 0;
		m_startSize = 0;
		m_numItems = 0;
	}

	~PreallocatedVector()
	{
		delete m_objects;
	}

	void Reserve(__int32 numItems)
	{
		m_objects = (T*)malloc(sizeof(T) * numItems);
		ZeroMemory(m_objects, sizeof(T) * m_maxItems);
		m_maxItems = numItems;
		m_numItems = 0;
		m_startSize = numItems;
	}

	void Clear()
	{
		m_numItems = 0;
		ZeroMemory(m_objects, sizeof(T) * m_maxItems);
	}

	T* Allocate()
	{
		return Allocate(NULL);
	}

	T* Allocate(T* from)
	{
		if (m_numItems >= m_maxItems)
		{
			// Try to reallocate
			__int32 newSize = m_maxItems + m_startSize;
			T* temp = (T*)malloc(sizeof(T) * newSize);
			ZeroMemory(temp, sizeof(T) * newSize);
			memcpy(temp, m_objects, sizeof(T) * m_maxItems);
			delete m_objects;
			m_objects = temp;
			m_maxItems = newSize;
		}

		T* obj = Get(m_numItems);
		if (obj == NULL)
			return NULL;

		m_numItems++;

		if (from != NULL)
			memcpy(obj, from, sizeof(T));

		return obj;
	}

	T* Get(__int32 index)
	{
		if (index >= m_numItems)
			return NULL;

		return &m_objects[index];
	}

	__int32 Size()
	{
		return m_numItems;
	}

	void Sort(__int32(*compareFunc)(const void*, const void*))
	{
		qsort(m_objects, m_numItems, sizeof(T), compareFunc);
	}
};

struct RendererVertex {
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;
	Vector4 Color;
	float Bone;
};

class RenderTarget2D
{
public:
	ID3D11RenderTargetView*	RenderTargetView;
	ID3D11ShaderResourceView* ShaderResourceView;
	ID3D11Texture2D* Texture;
	bool IsValid = false;

	RenderTarget2D()
	{

	}

	static RenderTarget2D* Create(ID3D11Device* device, __int32 w, __int32 h, DXGI_FORMAT format)
	{
		RenderTarget2D* rt = new RenderTarget2D();

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 4;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		rt->Texture = NULL;
		HRESULT res = device->CreateTexture2D(&desc, NULL, &rt->Texture);
		if (FAILED(res))
			return NULL;

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = desc.Format;
		viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;

		res = device->CreateRenderTargetView(rt->Texture, &viewDesc, &rt->RenderTargetView);
		if (FAILED(res))
			return NULL;

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = desc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		res = device->CreateShaderResourceView(rt->Texture, &shaderDesc, &rt->ShaderResourceView);
		if (FAILED(res))
			return NULL;

		return rt;
	}

	~RenderTarget2D()
	{
		DX11_RELEASE(RenderTargetView);
		DX11_RELEASE(ShaderResourceView);
		DX11_RELEASE(Texture);
	}
};

class Texture2D
{
public:
	ID3D11ShaderResourceView* ShaderResourceView;
	ID3D11Texture2D* Texture;

	Texture2D()
	{		
		
	}

	~Texture2D()
	{
		DX11_RELEASE(ShaderResourceView);
		DX11_RELEASE(Texture);
	}

	static Texture2D* LoadFromByteArray(ID3D11Device* device, __int32 w, __int32 h, byte* data)
	{
		Texture2D* texture = new Texture2D();

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = w;
		desc.Height = h;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		
		D3D11_SUBRESOURCE_DATA subresourceData; 
		subresourceData.pSysMem = data; 
		subresourceData.SysMemPitch = w * 4; 
		subresourceData.SysMemSlicePitch = 0;
		
		HRESULT res = device->CreateTexture2D(&desc, &subresourceData, &texture->Texture);
		if (FAILED(res))
			return NULL;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
		shaderDesc.Format = desc.Format;
		shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderDesc.Texture2D.MostDetailedMip = 0;
		shaderDesc.Texture2D.MipLevels = 1;

		res = device->CreateShaderResourceView(texture->Texture, &shaderDesc, &texture->ShaderResourceView);
		if (FAILED(res))
			return NULL;
		
		return texture;
	}

	static Texture2D* LoadFromFile(ID3D11Device* device, char* fileName)
	{
		Texture2D* texture = new Texture2D();

		wchar_t buffer[255];
		size_t converted = 0;
		mbstowcs_s(&converted, buffer, fileName, strlen(fileName));

		ID3D11Resource* resource = NULL;
		ID3D11DeviceContext* context = NULL;
		device->GetImmediateContext(&context);

		HRESULT res = CreateWICTextureFromFile(device, context, &buffer[0], &resource, &texture->ShaderResourceView, (size_t)0);
		if (FAILED(res))
			return NULL;

		resource->QueryInterface(IID_ID3D11Texture2D, (void **)&texture->Texture);
		
		return texture;
	}
};

class VertexBuffer
{
public:
	ID3D11Buffer* Buffer;

	VertexBuffer()
	{

	}

	~VertexBuffer()
	{
		DX11_RELEASE(Buffer);
	}

	static VertexBuffer* Create(ID3D11Device* device, __int32 numVertices, RendererVertex* vertices)
	{
		HRESULT res;

		VertexBuffer* vb = new VertexBuffer();

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Usage = D3D11_USAGE_DYNAMIC;   
		desc.ByteWidth = sizeof(RendererVertex) * numVertices; 
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 

		res = device->CreateBuffer(&desc, NULL, &vb->Buffer);
		if (FAILED(res))
			return NULL;

		if (vertices != NULL)
		{
			ID3D11DeviceContext* context = NULL;
			device->GetImmediateContext(&context);

			D3D11_MAPPED_SUBRESOURCE ms;

			res = context->Map(vb->Buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			if (FAILED(res))
				return NULL;

			memcpy(ms.pData, vertices, sizeof(RendererVertex) * numVertices);

			context->Unmap(vb->Buffer, NULL);
		}

		return vb;
	}
};

class IndexBuffer
{
public:
	ID3D11Buffer* Buffer;

	IndexBuffer()
	{

	}

	~IndexBuffer()
	{
		DX11_RELEASE(Buffer);
	}

	static IndexBuffer* Create(ID3D11Device* device, __int32 numIndices, __int32* indices)
	{
		HRESULT res;

		IndexBuffer* ib = new IndexBuffer();

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(__int32) * numIndices;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
		res = device->CreateBuffer(&desc, NULL, &ib->Buffer);
		if (FAILED(res))
			return NULL;

		if (indices != NULL)
		{
			ID3D11DeviceContext* context = NULL;
			device->GetImmediateContext(&context);

			D3D11_MAPPED_SUBRESOURCE ms;

			res = context->Map(ib->Buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			if (FAILED(res))
				return NULL;

			memcpy(ms.pData, indices, sizeof(__int32) * numIndices);

			context->Unmap(ib->Buffer, NULL);
		}

		return ib;
	}
};

struct RendererStringToDraw
{
	float X;
	float Y;
	__int32 Flags;
	wstring String;
	Vector3 Color;
};

struct RendererPolygon 
{
	byte Shape;
	__int32 AnimatedSet;
	__int32 TextureId;
	__int32 Distance;
	__int32 Indices[4];
};

struct RendererBone 
{
	Vector3 Translation;
	Matrix GlobalTransform;
	Matrix Transform;
	Vector3 GlobalTranslation;
	vector<RendererBone*> Children;
	RendererBone* Parent;
	__int32 Index;
	Vector3 ExtraRotation;
	byte ExtraRotationFlags;

	RendererBone(__int32 index)
	{
		Index = index;
		ExtraRotationFlags = 0;
		Translation = Vector3(0, 0, 0);
		ExtraRotation = Vector3(0, 0, 0);
	}
};

struct CMatrixBuffer
{
	Matrix World;
	Matrix View;
	Matrix Projection;
};

struct RendererAnimatedTexture 
{
	__int32 Id;
	Vector2 UV[4];
};

struct RendererAnimatedTextureSet 
{
	vector<RendererAnimatedTexture*> Textures;
};

struct RendererBucket
{
	vector<RendererVertex>		Vertices;
	vector<__int32>				Indices;
	vector<RendererPolygon>		Polygons;
	vector<RendererPolygon>		AnimatedPolygons;
	__int32						StartVertex;
	__int32						StartIndex;
	__int32						NumTriangles;
	__int32						NumVertices;
	__int32						NumIndices;
};

struct RendererLight {
	Vector4 Position;
	Vector4 Color;
	Vector4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
	LIGHT_TYPES Type;
	bool Dynamic;

	RendererLight()
	{
		Dynamic = false;
	}
};

struct RendererStatic {
	__int32 Id;
	__int16 RoomIndex;
	MESH_INFO* Mesh;
	Matrix World;
};

struct RendererRoom 
{
	ROOM_INFO* Room;
	VertexBuffer* VertexBuffer;
	IndexBuffer* IndexBuffer;
	Vector4 AmbientLight;
	RendererBucket Buckets[NUM_BUCKETS];
	RendererBucket AnimatedBuckets[NUM_BUCKETS];
	vector<RendererLight> Lights;
	vector<RendererStatic> Statics;
	bool Visited;
	float Distance;
	__int32 RoomNumber;
};

struct RendererRoomNode {
	__int16 From;
	__int16 To;
	Vector4 ClipPort;
};

struct RendererItem {
	__int32 Id;
	ITEM_INFO* Item;
	Matrix World;
	vector<Matrix> AnimationTransforms;
};

struct RendererEffect {
	__int32 Id;
	FX_INFO* Effect;
	Matrix World;
};

struct RendererMesh
{
	RendererBucket				Buckets[NUM_BUCKETS];
	RendererBucket				AnimatedBuckets[NUM_BUCKETS];
	vector<Vector3>				Positions;
};

struct RendererObject
{
	__int32 Id;
	vector<RendererMesh*>		ObjectMeshes;
	RendererBone*				Skeleton;
	vector<Matrix>				AnimationTransforms;
	vector<Matrix>				BindPoseTransforms;
	vector<RendererBone*>		LinearizedBones;
	bool						DoNotDraw;
	bool						HasDataInBucket[NUM_BUCKETS];
	bool						HasDataInAnimatedBucket[NUM_BUCKETS];
};

class Renderer11
{
private:
	// Core DX11 objects
	ID3D11Device*							m_device = NULL;
	ID3D11DeviceContext*					m_context = NULL;
	IDXGISwapChain*							m_swapChain = NULL;
	IDXGIDevice*							m_dxgiDevice = NULL;
	CommonStates*							m_states = NULL;
	ID3D11InputLayout*						m_inputLayout = NULL;
	D3D11_VIEWPORT							m_viewport;

	// Main back buffer
	ID3D11RenderTargetView*					m_backBufferRTV;
	ID3D11Texture2D*						m_backBufferTexture;

	ID3D11DepthStencilState*				m_depthStencilState;
	ID3D11DepthStencilView*					m_depthStencilView;
	ID3D11Texture2D*						m_depthStencilTexture;

	// Shaders
	ID3D11VertexShader*						m_vs;
	ID3D11PixelShader*						m_ps;

	// Constant buffers
	CMatrixBuffer							m_stMatrices;
	ID3D11Buffer*							m_cbMatrices;

	// Text and sprites
	SpriteFont*								m_gameFont;
	SpriteBatch*							m_spriteBatch;
	vector<RendererStringToDraw>			m_strings;
	__int32									m_blinkColorValue;
	__int32									m_blinkColorDirection;

	// System resources
	Texture2D*								m_caustics[NUM_CAUSTICS_TEXTURES];
	Texture2D*								m_binocularsTexture;

	// Level data
	Texture2D*										m_textureAtlas;
	vector<RendererAnimatedTextureSet*>				m_animatedTextureSets;
	VertexBuffer*									m_roomsVertexBuffer;
	IndexBuffer*									m_roomsIndexBuffer;
	VertexBuffer*									m_moveablesVertexBuffer;
	IndexBuffer*									m_moveablesIndexBuffer;
	VertexBuffer*									m_staticsVertexBuffer;
	IndexBuffer*									m_staticsIndexBuffer;
	map<__int32, RendererRoom*>						m_rooms;
	Matrix											m_hairsMatrices[12];
	__int16											m_normalLaraSkinJointRemap[15][32];
	__int16											m_youngLaraSkinJointRemap[15][32];
	__int16											m_laraSkinJointRemap[15][32];
	__int16											m_numHairVertices;
	__int16											m_numHairIndices;
	vector<RendererVertex>							m_hairVertices;
	vector<__int32>									m_hairIndices;
	vector<RendererRoom*>							m_roomsToDraw;
	vector<RendererItem*>							m_itemsToDraw;
	vector<RendererEffect*>							m_effectsToDraw;
	vector<RendererStatic*>							m_staticsToDraw;
	vector<RendererLight*>							m_lightsToDraw;
	vector<RendererLight*>							m_dynamicLights;
	RendererLight*									m_shadowLight;
	map<__int32, RendererObject*>					m_moveableObjects;
	map<__int32, RendererObject*>					m_staticObjects;
	map<__int16*, RendererMesh*>					m_meshPointersToMesh;

	// Preallocated pools of objects for avoiding new/delete
	// Items and effects are safe (can't be more than 1024 items in TR), 
	// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
	RendererItem									m_items[NUM_ITEMS];
	RendererEffect									m_effects[NUM_ITEMS];
	RendererLight									m_lights[MAX_LIGHTS];
	__int32											m_nextLight;

	// Private functions
	bool									drawScene(bool dump);
	bool									drawAllStrings();
	ID3D11VertexShader*						compileVertexShader(char* fileName);
	ID3D11GeometryShader*					compileGeometryShader(char* fileName);
	ID3D11PixelShader*						compilePixelShader(char* fileName);
	ID3D11ComputeShader*					compileComputeShader(char* fileName);
	ID3D11Buffer*							createConstantBuffer(__int32 size);
	__int32									getAnimatedTextureInfo(__int16 textureId);
	void									initialiseHairRemaps();
	RendererMesh*							getRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs);
	void									fromTrAngle(Matrix* matrix, __int16* frameptr, __int32 index);
	void									buildHierarchy(RendererObject* obj);
	void									buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
	void									updateAnimation(RendererItem* item, RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask);
	bool									printDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message);
	bool									checkPortal(__int16 roomIndex, __int16* portal, Vector4* viewPort, Vector4* clipPort);
	void									getVisibleRooms(int from, int to, Vector4* viewPort, bool water, int count);
	void									collectRooms();
	void									collectItems(__int16 roomNumber);
	void									collectStatics(__int16 roomNumber);
	void									collectLights(__int16 roomNumber);
	void									collectEffects(__int16 roomNumber);
	void									prepareLights();
	void									collectSceneItems();
	void									clearSceneItems();
	bool									updateConstantBuffer(ID3D11Buffer* buffer, void* data, __int32 size);

public:
	Matrix									View;
	Matrix									Projection;
	Matrix									ViewProjection;
	float									FieldOfView;
	__int32									ScreenWidth;
	__int32									ScreenHeight;
	bool									Windowed;
	__int32									NumTexturePages;

	Renderer11();
	~Renderer11();

	void Test();
	bool									Create();
	bool									EnumerateVideoModes();
	bool									Initialise(__int32 w, __int32 h, __int32 refreshRate, bool windowed, HWND handle);
	__int32									Draw();
	bool									PrepareDataForTheRenderer();
	void									UpdateCameraMatrices(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float roll, float fov);
	__int32									DumpGameScene();
	__int32									DrawInventory();
	__int32									DrawPickup(__int16 objectNum);
	__int32									SyncRenderer();
	bool									PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags);
	void									DrawDashBar();
	void									DrawHealthBar(__int32 percentual);
	void									DrawAirBar(__int32 percentual);
	void									ClearDynamicLights();
	void									AddDynamicLight(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);
	void									FreeRendererData();
	void									EnableCinematicBars(bool value);
	void									FadeIn();
	void									FadeOut();
	void									DrawLoadingScreen(char* fileName);
	void									UpdateProgress(float value);
	bool									IsFading();
	void									GetLaraBonePosition(Vector3* pos, __int32 bone);
	bool									ToggleFullScreen();
	bool									IsFullsScreen();
	bool									ChangeScreenResolution(__int32 width, __int32 height, __int32 frequency, bool windowed);
};