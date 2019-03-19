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
#define DX11_DELETE(x) if (x != NULL) { delete x; x = NULL; }

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

template <class T>
class PreallocatedVector 
{
private:
	T**			m_objects;
	__int32		m_maxItems;
	__int32		m_numItems;
	__int32		m_startSize;

public:
	PreallocatedVector()
	{
		m_objects = NULL;
		m_maxItems = 0;
		m_startSize = 0;
		m_numItems = 0;
	}

	~PreallocatedVector()
	{
		delete m_objects;
	}

	inline void Reserve(__int32 numItems)
	{
		m_objects = (T**)malloc(sizeof(T*) * numItems);
		ZeroMemory(m_objects, sizeof(T*) * m_maxItems);
		m_maxItems = numItems;
		m_numItems = 0;
		m_startSize = numItems;
	}

	inline void Clear()
	{
		m_numItems = 0;
		ZeroMemory(m_objects, sizeof(T*) * m_maxItems);
	}

	inline __int32 Size()
	{
		return m_numItems;
	}

	inline void Sort(__int32(*compareFunc)(T*, T*))
	{
		qsort(m_objects, m_numItems, sizeof(T), compareFunc);
	}

	inline T*& operator[] (__int32 x) {
		return m_objects[x];
	}

	inline void Add(T* value)
	{
		m_objects[m_numItems++] = value;
	}
};

struct RendererDisplayMode {
	__int32 Width;
	__int32 Height;
	__int32 RefreshRate;
};

struct RendererVideoAdapter {
	string Name;
	__int32 Index;
	vector<RendererDisplayMode> DisplayModes;
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
	ID3D11DepthStencilView*	DepthStencilView;
	ID3D11Texture2D* DepthStencilTexture;
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
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
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

		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
		depthTexDesc.Width = w;
		depthTexDesc.Height = h;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;

		rt->DepthStencilTexture = NULL;
		res = device->CreateTexture2D(&depthTexDesc, NULL, &rt->DepthStencilTexture);
		if (FAILED(res))
			return false;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = depthTexDesc.Format;
		dsvDesc.Flags = 0;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		rt->DepthStencilView = NULL;
		res = device->CreateDepthStencilView(rt->DepthStencilTexture, &dsvDesc, &rt->DepthStencilView);
		if (FAILED(res))
			return false;

		return rt;
	}

	~RenderTarget2D()
	{
		DX11_RELEASE(RenderTargetView);
		DX11_RELEASE(ShaderResourceView);
		DX11_RELEASE(Texture);
		DX11_RELEASE(DepthStencilView);
		DX11_RELEASE(DepthStencilTexture);
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

struct RendererLight {
	Vector3 Position;
	float Type;
	Vector3 Color;
	float Dynamic;
	Vector4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;

	RendererLight()
	{
		Dynamic = false;
	}
};

struct ShaderLight {
	Vector4 Position;
	Vector4 Color;
	Vector4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};

struct CCameraMatrixBuffer
{
	Matrix View;
	Matrix Projection;
};

struct CItemBuffer
{
	Matrix World;
	Matrix BonesMatrices[32];
	Vector4 Position;
	Vector4 AmbientLight;
};

struct CStaticBuffer
{
	Matrix World;
	Vector4 Position;
	Vector4 Color;
};

struct CLightBuffer {
	ShaderLight Lights[NUM_LIGHTS_PER_BUFFER];
	__int32 NumLights;
	Vector3 CameraPosition;
};

struct CMiscBuffer {
	__int32 AlphaTest;
	__int32 Caustics;
	float Padding[14];
};

struct RendererAnimatedTexture 
{
	__int32 Id;
	Vector2 UV[4];
};

struct RendererAnimatedTextureSet 
{
	__int32 NumTextures;
	RendererAnimatedTexture** Textures;

	~RendererAnimatedTextureSet()
	{
		/*for (__int32 i = 0; i < NumTextures; i++)
			delete Textures[i];
		free(Textures);*/
	}
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

struct RendererStatic {
	__int32 Id;
	__int16 RoomIndex;
	MESH_INFO* Mesh;
	Matrix World;
};

struct RendererRoom 
{
	ROOM_INFO* Room;
	Vector4 AmbientLight;
	RendererBucket Buckets[NUM_BUCKETS];
	RendererBucket AnimatedBuckets[NUM_BUCKETS];
	vector<RendererLight> Lights;
	vector<RendererStatic> Statics;
	bool Visited;
	float Distance;
	__int32 RoomNumber;
	PreallocatedVector<RendererLight> LightsToDraw;
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
	Matrix AnimationTransforms[32];
	__int32 NumMeshes;
	PreallocatedVector<RendererLight> Lights;
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

	~RendererObject()
	{
		/*for (__int32 i = 0; i < ObjectMeshes.size(); i++)
			delete ObjectMeshes[i];
		for (__int32 i = 0; i < LinearizedBones.size(); i++)
			delete LinearizedBones[i];*/
	}
};

struct RendererSprite {
	__int32 Width;
	__int32 Height;
	Vector2 UV[4];
};

struct RendererSpriteSequence {
	__int32 Id;
	RendererSprite** SpritesList;
	__int32 NumSprites;

	RendererSpriteSequence(__int32 id, __int32 num)
	{
		Id = id;
		NumSprites = num;
		SpritesList = (RendererSprite**)malloc(sizeof(RendererSprite*) * num);
	}

	~RendererSpriteSequence()
	{
		/*for (__int32 i = 0; i < NumSprites; i++)
			delete SpritesList[i];
		free(SpritesList);*/
	}
};

struct RendererSpriteToDraw {
	RENDERER_SPRITE_TYPE Type;
	RendererSprite* Sprite;
	float Distance;
	float Scale;
	float X, Y, Z;
	float X1, Y1, Z1;
	float X2, Y2, Z2;
	float X3, Y3, Z3;
	float X4, Y4, Z4;
	byte R;
	byte G;
	byte B;
	float Rotation;
	float Width;
	float Height;
	BLEND_MODES BlendMode;
};

struct RendererLine3D {
	float X1;
	float Y1;
	float Z1;
	float X2;
	float Y2;
	float Z2;
	byte R;
	byte G;
	byte B;
};

struct RendererWeatherParticle {
	float X, Y, Z;
	float AngleH;
	float AngleV;
	float Size;
	__int16 Room;
	bool Reset;
};

struct RendererUnderwaterDustParticle {
	float X, Y, Z;
	__int16 Life;
	__int16 Room;
	bool Reset;
};

struct RendererLine2D {
	Vector2 Vertices[2];
	Vector4 Color;
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
	vector<RendererVideoAdapter>			m_adapters;

	// Main back buffer
	ID3D11RenderTargetView*					m_backBufferRTV;
	ID3D11Texture2D*						m_backBufferTexture;

	ID3D11DepthStencilState*				m_depthStencilState;
	ID3D11DepthStencilView*					m_depthStencilView;
	ID3D11Texture2D*						m_depthStencilTexture;

	RenderTarget2D*							m_dumpScreenRenderTarget;
	RenderTarget2D*							m_renderTarget;

	// Shaders
	ID3D11VertexShader*						m_vsRooms;
	ID3D11PixelShader*						m_psRooms;
	ID3D11VertexShader*						m_vsItems;
	ID3D11PixelShader*						m_psItems;
	ID3D11VertexShader*						m_vsHairs;
	ID3D11PixelShader*						m_psHairs;
	ID3D11VertexShader*						m_vsStatics;
	ID3D11PixelShader*						m_psStatics;
	ID3D11VertexShader*						m_vsSky;
	ID3D11PixelShader*						m_psSky;
	ID3D11VertexShader*						m_vsSprites;
	ID3D11PixelShader*						m_psSprites;
	ID3D11VertexShader*						m_vsSolid;
	ID3D11PixelShader*						m_psSolid;
	ID3D11VertexShader*						m_vsInventory;
	ID3D11PixelShader*						m_psInventory;
	ID3D11VertexShader*						m_vsFullScreenQuad;
	ID3D11PixelShader*						m_psFullScreenQuad;

	// Constant buffers
	CCameraMatrixBuffer						m_stCameraMatrices;
	ID3D11Buffer*							m_cbCameraMatrices;
	CItemBuffer								m_stItem;
	ID3D11Buffer*							m_cbItem;
	CStaticBuffer							m_stStatic;
	ID3D11Buffer*							m_cbStatic;
	CLightBuffer							m_stLights;
	ID3D11Buffer*							m_cbLights;
	CMiscBuffer								m_stMisc;
	ID3D11Buffer*							m_cbMisc;

	// Text and sprites
	SpriteFont*								m_gameFont;
	SpriteBatch*							m_spriteBatch;
	vector<RendererStringToDraw>			m_strings;
	__int32									m_blinkColorValue;
	__int32									m_blinkColorDirection;
	PrimitiveBatch<RendererVertex>*			m_primitiveBatch;

	// System resources
	Texture2D*								m_caustics[NUM_CAUSTICS_TEXTURES];
	Texture2D*								m_binocularsTexture;

	// Level data
	Texture2D*										m_titleScreen;
	Texture2D*										m_loadScreen;
	Texture2D*										m_textureAtlas;
	Texture2D*										m_skyTexture;
	VertexBuffer*									m_roomsVertexBuffer;
	IndexBuffer*									m_roomsIndexBuffer;
	VertexBuffer*									m_moveablesVertexBuffer;
	IndexBuffer*									m_moveablesIndexBuffer;
	VertexBuffer*									m_staticsVertexBuffer;
	IndexBuffer*									m_staticsIndexBuffer;
	RendererRoom**									m_rooms;
	Matrix											m_hairsMatrices[12];
	__int16											m_normalLaraSkinJointRemap[15][32];
	__int16											m_youngLaraSkinJointRemap[15][32];
	__int16											m_laraSkinJointRemap[15][32];
	__int16											m_numHairVertices;
	__int16											m_numHairIndices;
	vector<RendererVertex>							m_hairVertices;
	vector<__int16>									m_hairIndices;
	PreallocatedVector<RendererRoom>				m_roomsToDraw;
	PreallocatedVector<RendererItem>				m_itemsToDraw;
	PreallocatedVector<RendererEffect>			    m_effectsToDraw;
	PreallocatedVector<RendererStatic>				m_staticsToDraw;
	PreallocatedVector<RendererLight>				m_lightsToDraw;
	PreallocatedVector<RendererLight>				m_dynamicLights;
	PreallocatedVector<RendererSpriteToDraw>		m_spritesToDraw;
	PreallocatedVector<RendererLine3D>				m_lines3DToDraw;
	PreallocatedVector<RendererLine2D>				m_lines2DToDraw;
	PreallocatedVector<RendererLight>				m_tempItemLights;
	RendererSpriteToDraw*							m_spritesBuffer;
	__int32											m_nextSprite;
	RendererLine3D*									m_lines3DBuffer;
	__int32											m_nextLine3D;
	RendererLine2D*									m_lines2DBuffer;
	__int32											m_nextLine2D;
	RendererLight*									m_shadowLight;
	RendererObject**								m_moveableObjects;
	RendererObject**								m_staticObjects;
	RendererSprite**								m_sprites;
	__int32											m_numMoveables;
	__int32											m_numStatics;
	__int32											m_numSprites;
	__int32											m_numSpritesSequences;
	RendererSpriteSequence**						m_spriteSequences;
	unordered_map<__int16*, RendererMesh*>			m_meshPointersToMesh;
	Matrix											m_LaraWorldMatrix;
	RendererAnimatedTextureSet**					m_animatedTextureSets;
	__int32											m_numAnimatedTextureSets;
	__int32											m_currentCausticsFrame;
	RendererUnderwaterDustParticle					m_underwaterDustParticles[NUM_UNDERWATER_DUST_PARTICLES];
	bool											m_firstUnderwaterDustParticles = true;

	// Debug variables
	__int32											m_numDrawCalls = 0;

	// Preallocated pools of objects for avoiding new/delete
	// Items and effects are safe (can't be more than 1024 items in TR), 
	// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
	RendererItem									m_items[NUM_ITEMS];
	RendererEffect									m_effects[NUM_ITEMS];
	RendererLight									m_lights[MAX_LIGHTS];
	__int32											m_nextLight;
	__int32											m_currentY;

	// Times for debug
	__int32											m_timeUpdate;
	__int32											m_timeDraw;
	__int32											m_timeFrame;

	// Others
	bool											m_firstWeather;
	RendererWeatherParticle							m_rain[NUM_RAIN_DROPS];
	RendererWeatherParticle							m_snow[NUM_SNOW_PARTICLES];
	RENDERER_FADE_STATUS							m_fadeStatus;
	float											m_fadeFactor;
	__int32											m_progress;
	bool											m_enableCinematicBars = false;

	// Private functions
	bool									drawScene(bool dump);
	bool									drawAllStrings();
	ID3D11VertexShader*						compileVertexShader(char* fileName, char* function, char* model, ID3D10Blob** bytecode);
	ID3D11GeometryShader*					compileGeometryShader(char* fileName);
	ID3D11PixelShader*						compilePixelShader(char* fileName, char* function, char* model, ID3D10Blob** bytecode);
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
	inline void								collectItems(__int16 roomNumber);
	inline void								collectStatics(__int16 roomNumber);
	inline void								collectLightsForRoom(__int16 roomNumber);
	inline void								collectLightsForItem(__int16 roomNumber, RendererItem* item);
	inline void								collectEffects(__int16 roomNumber);
	void									prepareLights();
	void									clearSceneItems();
	bool									updateConstantBuffer(ID3D11Buffer* buffer, void* data, __int32 size);
	void									updateLaraAnimations();
	void									updateItemsAnimations();
	void									updateEffects();
	__int32									getFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate);
	bool									drawAmbientCubeMap(__int16 roomNumber);
	bool									sphereBoxIntersection(Vector3 boxMin, Vector3 boxMax, Vector3 sphereCentre, float sphereRadius);
	bool									drawHorizonAndSky();
	bool									drawRooms(bool transparent, bool animated);
	bool									drawStatics(bool transparent);
	bool									drawItems(bool transparent, bool animated);
	bool									drawAnimatingItem(RendererItem* item, bool transparent, bool animated);
	bool									drawWaterfalls();
	bool									drawLara(bool transparent);
	void									printDebugMessage(char* message, ...);
	void									drawFires();
	void									drawSparks();
	void									drawSmokes();
	void									drawBlood();
	void									drawDrips();
	void									drawBubbles();
	void									drawSplahes();
	bool									drawSprites();
	bool									drawLines3D();
	bool									drawLines2D();
	bool									drawOverlays();
	bool									drawRopes();
	bool									drawBats();
	bool									drawRats();
	bool									drawSpiders();
	bool									drawGunFlashes();
	bool									drawGunShells();
	bool									drawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2);
	void									insertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b);
	bool									drawDebris(bool transparent);
	__int32									drawInventoryScene();
	__int32									drawFinalPass();
	void									updateAnimatedTextures();
	void									createBillboardMatrix(Matrix* out, Vector3* particlePos, Vector3* cameraPos, float rotation);
	void									drawShockwaves();
	void									drawRipples();
	void									drawUnderwaterDust();	
	bool									doRain();
	bool									doSnow();
	bool									drawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color, bool cinematicBars);
	bool									isRoomUnderwater(__int16 roomNumber);
	bool									isInRoom(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
	void									addSpriteBillboard(RendererSprite* sprite, float x, float y, float z, byte r, byte g, byte b, float rotation, float scale, float width, float height, BLEND_MODES blendMode);
	void									addSprite3D(RendererSprite* sprite, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, byte r, byte g, byte b, float rotation, float scale, float width, float height, BLEND_MODES blendMode);
	void									addLine3D(__int32 x1, __int32 y1, __int32 z1, __int32 x2, __int32 y2, __int32 z2, byte r, byte g, byte b);

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