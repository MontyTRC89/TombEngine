#pragma once

#include "..\Global\global.h"

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <vector>
#include <map>

#include "ShadowMapTarget.h"
#include "MainShader.h"
#include "DepthShader.h"
#include "RenderTarget2D.h"
#include "RenderTargetCube.h"
#include "Enums.h"
#include "Structs.h"
#include "RendererBucket.h"
#include "RendererMesh.h"
#include "RendererObject.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f

#define DX_RELEASE(x) if (x != NULL) x->Release()

using namespace std;

class RendererObject;

typedef struct RendererBone {
	D3DXVECTOR3 Translation;
	D3DXMATRIX GlobalTransform;
	D3DXMATRIX Transform;
	D3DXVECTOR3 GlobalTranslation;
	vector<RendererBone*> Children;
	RendererBone* Parent;
	__int32 Index;
	D3DXVECTOR3 ExtraRotation;

	RendererBone(__int32 index)
	{
		Index = index;
		Translation = D3DXVECTOR3(0, 0, 0);
		ExtraRotation = D3DXVECTOR3(0, 0, 0);
	}
};

typedef struct RendererJointLink {
	__int16 BoneIndex;
	__int16 ParentBoneIndex;
	__int16 ChildBoneIndex;
	__int16 ParentVertices[32];
	__int16 ChildVertices[32];
};

typedef struct RendererVertex {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float r, g, b, a;
	float bone;
};

typedef struct RendererPolygon {
	byte Shape;
	__int32 Distance;
	__int32 Indices[4];
};

typedef struct RendererLine2D {
	D3DXVECTOR2 Vertices[2];
	D3DCOLOR Color;
};

typedef struct RendererLight {
	D3DXVECTOR4 Position;
	D3DXVECTOR4 Color;
	D3DXVECTOR4 Direction;
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

typedef struct RendererLightInfo {
	bool		Active;
	D3DXVECTOR4 AmbientLight;
	RendererLight* Light;
};

typedef struct RendererDynamicLight {
	D3DXVECTOR4 Position;
	D3DXVECTOR4 Color;
	float Out;
};

typedef struct RendererRoom {
	ROOM_INFO* Room;
	RendererObject* RoomObject;
	vector<RendererLight*> Lights;
	D3DXVECTOR4 AmbientLight;
	bool Visited;

	RendererRoom()
	{
		Room = NULL;
		RoomObject = NULL;
	}

	~RendererRoom()
	{
		for (vector<RendererLight*>::iterator it = Lights.begin(); it != Lights.end(); ++it)
			delete (*it);
		Lights.clear();
		delete RoomObject;
	}
};

typedef struct RendererSphere {
	PDIRECT3DVERTEXBUFFER9 VertexBuffer;
	PDIRECT3DINDEXBUFFER9 IndexBuffer;
	__int32 NumVertices;
	__int32 NumIndices;

	RendererSphere(LPDIRECT3DDEVICE9 device, float radius,__int32 tessellation)
	{
		__int32 verticalSegments = tessellation;
		__int32 horizontalSegments = tessellation * 2;

		RendererVertex* vertices = (RendererVertex*)malloc((verticalSegments + 1) * (horizontalSegments + 1) * sizeof(RendererVertex));
		__int32* indices = (__int32*)malloc(verticalSegments * (horizontalSegments + 1) * 6 * sizeof(__int32));

		int vertexCount = 0;

		// Create rings of vertices at progressively higher latitudes.
		for (int i = 0; i <= verticalSegments; i++)
		{
			float v = 1.0f - (float)i / verticalSegments;

			float latitude = (float)(i * PI / verticalSegments - PI / 2.0);
			float dy = (float)sin(latitude);
			float dxz = (float)cos(latitude);

			// Create a single ring of vertices at this latitude.
			for (int j = 0; j <= horizontalSegments; j++)
			{
				float u = (float)j / horizontalSegments;

				float longitude = (float)(j * 2.0 * PI / horizontalSegments);
				float dx = (float)sin(longitude);
				float dz = (float)cos(longitude);

				dx *= dxz;
				dz *= dxz;

				D3DXVECTOR3 normal = D3DXVECTOR3(dx, dy, dz);

				vertices[vertexCount].x = normal.x * radius;
				vertices[vertexCount].y = normal.y * radius;
				vertices[vertexCount].z = normal.z * radius;

				vertexCount++;
			}
		}

		// Fill the index buffer with triangles joining each pair of latitude rings.
		__int32 stride = horizontalSegments + 1;

		__int32 indexCount = 0;
		for (__int32 i = 0; i < verticalSegments; i++)
		{
			for (__int32 j = 0; j <= horizontalSegments; j++)
			{
				__int32 nextI = i + 1;
				__int32 nextJ = (j + 1) % stride;

				indices[indexCount++] = i * stride + nextJ;
				indices[indexCount++] = nextI * stride + j;
				indices[indexCount++] = i * stride + j;

				indices[indexCount++] = nextI * stride + nextJ;
				indices[indexCount++] = nextI * stride + j;
				indices[indexCount++] = i * stride + nextJ;
			}
		}

		NumVertices = vertexCount;
		NumIndices = indexCount;

		HRESULT res;

		res = device->CreateVertexBuffer(vertexCount * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
			0, D3DPOOL_MANAGED, &VertexBuffer, NULL);
		if (res != S_OK)
			return;

		void* pVertices;

		res = VertexBuffer->Lock(0, 0, &pVertices, 0);
		if (res != S_OK)
			return;

		memcpy(pVertices, vertices, vertexCount * sizeof(RendererVertex));

		res = VertexBuffer->Unlock();
		if (res != S_OK)
			return;

		res = device->CreateIndexBuffer(indexCount * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED,
			&IndexBuffer, NULL);
		if (res != S_OK)
			return;

		void* pIndices;

		res = IndexBuffer->Lock(0, 0, &pIndices, 0);
		if (res != S_OK)
			return;

		memcpy(pIndices, indices, indexCount * 4);

		IndexBuffer->Unlock();
		if (res != S_OK)
			return;
	}

	~RendererSphere()
	{
		if (VertexBuffer != NULL)
			VertexBuffer->Release();
		if (IndexBuffer != NULL)
			IndexBuffer->Release();
	}
};

typedef struct RendererItemToDraw {
	__int32 Id;
	ITEM_INFO* Item;
	D3DXMATRIX World;

	RendererItemToDraw(__int32 id, ITEM_INFO* item)
	{
		Id = id;
		Item = item;
		D3DXMatrixIdentity(&World);
	}
};

typedef struct RendererQuad {
	PDIRECT3DVERTEXBUFFER9 VertexBuffer;
	PDIRECT3DINDEXBUFFER9 IndexBuffer;
	__int32 NumVertices;
	__int32 NumIndices;

	RendererQuad(LPDIRECT3DDEVICE9 device, float size)
	{
		RendererVertex* vertices = (RendererVertex*)malloc(4 * sizeof(RendererVertex));
		__int32* indices = (__int32*)malloc(6 * sizeof(__int32));

		NumVertices = 4;
		NumIndices = 6;

		vertices[0].x = -size / 2.0f;
		vertices[0].y = 0.0f;
		vertices[0].z = size / 2.0f;
		vertices[0].u = 0.0f;
		vertices[0].v = 0.0f;

		vertices[1].x = size / 2.0f;
		vertices[1].y = 0.0f;
		vertices[1].z = size / 2.0f;
		vertices[1].u = 1.0f;
		vertices[1].v = 0.0f;

		vertices[2].x = size / 2.0f;
		vertices[2].y = 0.0f;
		vertices[2].z = -size / 2.0f;
		vertices[2].u = 1.0f;
		vertices[2].v = 1.0f;

		vertices[3].x = -size / 2.0f;
		vertices[3].y = 0.0f;
		vertices[3].z = -size / 2.0f;
		vertices[3].u = 0.0f;
		vertices[3].v = 1.0f;

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;
		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 1;

		HRESULT res;

		res = device->CreateVertexBuffer(NumVertices * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
			0, D3DPOOL_MANAGED, &VertexBuffer, NULL);
		if (res != S_OK)
			return;

		void* pVertices;

		res = VertexBuffer->Lock(0, 0, &pVertices, 0);
		if (res != S_OK)
			return;

		memcpy(pVertices, vertices, NumVertices * sizeof(RendererVertex));

		res = VertexBuffer->Unlock();
		if (res != S_OK)
			return;

		res = device->CreateIndexBuffer(NumIndices * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED,
			&IndexBuffer, NULL);
		if (res != S_OK)
			return;

		void* pIndices;

		res = IndexBuffer->Lock(0, 0, &pIndices, 0);
		if (res != S_OK)
			return;

		memcpy(pIndices, indices, NumIndices * 4);

		IndexBuffer->Unlock();
		if (res != S_OK)
			return;
	}

	~RendererQuad()
	{
		if (VertexBuffer != NULL)
			VertexBuffer->Release();
		if (IndexBuffer != NULL)
			IndexBuffer->Release();
	}
};

typedef struct RendererSprite {
	__int32 Width;
	__int32 Height;
	D3DXVECTOR2 UV[4];
};

typedef struct RendererSpriteSequence {
	__int32 Id;
	vector<RendererSprite*> SpritesList;

	RendererSpriteSequence(__int32 id)
	{
		Id = id;
	}
};

typedef struct RendererSpriteToDraw {
	RendererSprite* Sprite;
	float Distance;
	float Scale;
	float X;
	float Y;
	float Z;
	byte R;
	byte G;
	byte B;
	float Rotation;
	RendererVertex Vertices[4];
	__int32	Indices[6];
};

class Renderer
{
	LPDIRECT3D9						m_d3D = NULL;
	LPDIRECT3DDEVICE9				m_device = NULL;
	LPDIRECT3DTEXTURE9				m_textureAtlas;
	LPDIRECT3DTEXTURE9				m_fontAndMiscTexture;
	LPDIRECT3DTEXTURE9				m_titleScreen;
	LPD3DXFONT						m_debugFont;
	LPD3DXFONT						m_gameFont;
	LPD3DXSPRITE					m_sprite;
	MainShader*						m_mainShader;
	DepthShader*					m_depthShader;
	char							m_message[255];
	map<__int32, RendererRoom*>		m_rooms;
	map<__int32, RendererObject*>	m_moveableObjects;
	map<__int32, RendererObject*>	m_staticObjects;
	map<__int32, RendererSprite*>	m_sprites;
	map<__int32, RendererSpriteSequence*>	m_spriteSequences;
	vector<__int32>					m_roomsToDraw;
	__int32*						m_meshTrees;
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDeclaration;
	LPDIRECT3DVERTEXDECLARATION9	m_spriteVertexDeclaration;
	__int32							m_numVertices;
	__int32							m_numTriangles;
	RendererLine2D*					m_lines;
	__int32							m_numLines;
	LPD3DXLINE						m_line;
	__int16							m_skinJointRemap[15][128];
	__int16							m_hairsRemap[8][32];
	__int32							m_fadeTimer;
	__int32							m_fadeMode;
	__int16							m_numHairVertices;
	__int16							m_numHairIndices;
	RendererVertex*					m_hairVertices;
	__int32*						m_hairIndices;
	RenderTarget2D*					m_renderTarget;
	__int32							m_blinkColorValue;
	__int32							m_blinkColorDirection;
	bool							m_needToDumpScene;
	__int16							m_pickupRotation;
	D3DXMATRIX						m_tempTranslation;
	D3DXMATRIX						m_tempScale;
	D3DXMATRIX						m_tempTransform;
	D3DXMATRIX						m_tempRotation;
	D3DXMATRIX						m_tempWorld;
	D3DXMATRIX						m_tempView;
	D3DXMATRIX						m_tempProjection;
	D3DXMATRIX						m_hairsMatrices[6];
	D3DXMATRIX						m_LaraWorldMatrix;
	RendererLightInfo*				m_itemsLightInfo;
	RenderTarget2D*					m_shadowMap;
	RenderTargetCube*				m_shadowMapCube;
	D3DXMATRIX						m_lightView;
	D3DXMATRIX						m_lightProjection;
	bool							m_enableShadows;
	RendererLight*					m_shadowLight;
	vector<__int32>					m_litItems;
	vector<RendererLight*>			m_dynamicLights;
	vector<RendererLight*>			m_lights;
	vector<RendererLight*>			m_testLights;
	vector<RendererItemToDraw*>		m_itemsToDraw;
	vector<RendererSpriteToDraw*>	m_spritesToDraw;
	
	LPDIRECT3DSURFACE9				m_backBufferTarget;
	LPDIRECT3DSURFACE9				m_backBufferDepth;

	// New light pre-pass renderer
	RenderTarget2D*					m_depthBuffer;
	RenderTarget2D*					m_normalBuffer;
	RenderTarget2D*					m_colorBuffer;
	RenderTarget2D*					m_outputBuffer;
	RenderTarget2D*					m_lightBuffer;
	RenderTarget2D*					m_shadowBuffer;
	RenderTarget2D*					m_vertexLightBuffer;
	RenderTarget2D*					m_postprocessBuffer;
	Shader*							m_shaderClearGBuffer;
	Shader*							m_shaderFillGBuffer;
	Shader*							m_shaderLight;
	Shader*							m_shaderCombine;
	Shader*							m_shaderBasic;
	Shader*							m_shaderFXAA;
	Shader*							m_shaderReconstructZBuffer;
	Shader*							m_shaderSprites;
	RendererVertex					m_quadVertices[4];
	__int32							m_quadIndices[6];
	RendererSphere*					m_sphereMesh;
	RendererQuad*					m_quad;
	RendererQuad*					m_skyQuad;
	float							m_halfPixelX;
	float							m_halfPixelY;
	D3DXMATRIX						m_viewProjection;
	D3DXMATRIX						m_inverseViewProjection;
	__int32							m_timeClearGBuffer;
	__int32							m_timePrepareShadowMap;
	__int32							m_timeFillGBuffer;
	__int32							m_timeLight;
	__int32							m_timeCombine;
	__int32							m_timeUpdate = 0;
	__int32							m_timeDrawScene;
	__int32							m_timeReconstructZBuffer;
	__int32							m_numDrawCalls;
	LPDIRECT3DTEXTURE9				m_randomTexture;
	LPDIRECT3DTEXTURE9				m_skyTexture;
	__int32							m_skyTimer;
	vector<RendererVertex>			m_spritesVertices;
	vector<__int32>					m_spritesIndices;

public:
	D3DXMATRIX						ViewMatrix;
	D3DXMATRIX						ProjectionMatrix;
	D3DXMATRIX						ViewProjectionMatrix;
	__int32							ScreenWidth;
	__int32							ScreenHeight;
	bool							Windowed;
	__int32							NumTexturePages;
	float							FOV;
	
	Renderer();
	~Renderer();

	bool							Initialise(__int32 w, __int32 h, bool windowed, HWND handle);
	RendererMesh*					GetRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs);
	void							FromTrAngle(D3DXMATRIX* matrix, __int16* frameptr, __int32 index);
	void							BuildHierarchy(RendererObject* obj);
	void							BuildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
	void							BuildAnimationPoseRecursive(RendererObject* obj, __int16** frmptr, D3DXMATRIX* parentTransform, __int16 frac, __int16 rate, RendererBone* bone, __int32 mask);
	void							BuildAnimationPose(RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask);
	__int32							Draw();
	bool							PrintDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message);
	bool							CheckPortal(__int16 roomIndex, __int16* portal, D3DXVECTOR4* viewPort, D3DXVECTOR4* clipPort);
	void							GetVisibleRooms(int from, int to, D3DXVECTOR4* viewPort, bool water, int count);
	bool							PrepareDataForTheRenderer();
	void							CollectRooms();
	void							CollectItems();
	void							CollectLights();
	void							PrepareShadowMaps();
	bool							DrawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount);
	bool							DrawRoom(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawStatic(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawLara(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawGunFlashes(RENDERER_PASSES pass);
	void							DrawNormalLara(__int32 bucketIndex);
	void							DrawDivesuitLara(__int32 bucketIndex);
	void							DrawCatsuitLara(__int32 bucketIndex);
	void							DrawYoungLara(__int32 bucketIndex);
	bool							DrawItem(__int32 itemIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	void							DrawSky();
	void							InsertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b);
	void							DrawAllLines2D();
	void							DrawGameInfo();
	void							DrawDashBar();
	void							DrawHealthBar(__int32 percentual);
	void							DrawAirBar(__int32 percentual);
	void							DrawDebugInfo();
	void							DrawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2);
	void							PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags);
	__int32							DrawScene();
	__int32							DumpGameScene();
	void							ResetBlink();
	__int32							BeginRenderToTexture();
	__int32							EndRenderToTexture();
	__int32							DrawPauseMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawLoadGameMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawSaveGameMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawStatisticsMenu();
	__int32							DrawSettingsMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawInventory();
	__int32							DrawInventoryScene();
	__int32							DrawObjectOn2DPosition(__int16 x, __int16 y, __int16 objectNum, __int16 rotX, __int16 rotY, __int16 rotZ);
	__int32							DrawPickup(__int16 objectNum);
	__int32							SyncRenderer();
	__int32							GetFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate);
	void							UpdateLaraAnimations();
	void							UpdateItemsAnimations();
	void							UpdateGunFlashes();
	void							UpdateFlares();
	void							UpdateFires();
	void							AddDynamicLight(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);
	void							ClearDynamicLights();
	void							AddSprite(RendererSprite* sprite, __int32 x, __int32 y, __int32 z, byte r, byte g, byte b, float rotation, float scale, float width, float height);
	void							DrawFires();
	void							DrawSmokes();
	void							DrawBlood();
	bool							DrawGunshells(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	void							CreateBillboardMatrix(D3DXMATRIX* out, D3DXVECTOR3* particlePos, D3DXVECTOR3* cameraPos);

	// New light pre-pass renderer
	bool							DrawSceneLightPrePass(bool dump);
	bool							BindRenderTargets(RenderTarget2D* rt1, RenderTarget2D* rt2, RenderTarget2D* rt3, RenderTarget2D* rt4);
	bool							RestoreBackBuffer();
	bool							DrawRoomLPP(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawStaticLPP(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawLaraLPP(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawItemLPP(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							DrawSkyLPP();
	void							CollectLightsLPP();
};