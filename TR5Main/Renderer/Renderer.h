#pragma once

#include "..\Global\global.h"

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <vector>
#include <map>
#include <memory>

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
	vector<shared_ptr<RendererBone>> Children;
	shared_ptr<RendererBone> Parent;
	__int32 Index;
	D3DXVECTOR3 ExtraRotation;

	RendererBone(__int32 index)
	{
		Index = index;
		Translation = D3DXVECTOR3(0, 0, 0);
		ExtraRotation = D3DXVECTOR3(0, 0, 0);
	}
};

typedef struct RendererVertex {
	float x = 0;
	float y = 0;
	float z = 0;
	float nx = 0;
	float ny = 0;
	float nz = 0;
	float u = 0;
	float v = 0;
	float r = 0;
	float g = 0;
	float b = 0;
	float a = 0;
	float boneAndFlags = 0;
};

typedef struct RendererPolygon {
	byte Shape;
	__int32 AnimatedSet;
	__int32 TextureId;
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

typedef struct RendererDynamicLight {
	D3DXVECTOR4 Position;
	D3DXVECTOR4 Color;
	float Out;
};

typedef struct RendererAnimatedTexture {
	__int32 Id;
	D3DXVECTOR2 UV[4];
};

typedef struct RendererAnimatedTextureSet {
	vector<shared_ptr<RendererAnimatedTexture>> Textures;
};

typedef struct RendererRoom {
	ROOM_INFO* Room;
	shared_ptr<RendererObject> RoomObject;
	vector<shared_ptr<RendererLight>> Lights;
	D3DXVECTOR4 AmbientLight;
	bool Visited;

	RendererRoom()
	{
		Room = NULL;
		RoomObject = NULL;
	}

	~RendererRoom()
	{
		//Lights.clear();
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
	vector<D3DXMATRIX> AnimationTransforms;

	RendererItemToDraw(__int32 id, ITEM_INFO* item, __int32 numMeshes)
	{
		Id = id;
		Item = item;
		D3DXMatrixIdentity(&World);
		AnimationTransforms.reserve(numMeshes);
		D3DXMATRIX matrix;
		D3DXMatrixIdentity(&matrix);
		for (__int32 i = 0; i < numMeshes; i++)
			AnimationTransforms.push_back(matrix);
	}
};

typedef struct RendererEffectToDraw {
	__int32 Id;
	FX_INFO* Effect;
	D3DXMATRIX World;

	RendererEffectToDraw(__int32 id, FX_INFO* effect)
	{
		Id = id;
		Effect = effect;
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
	vector<shared_ptr<RendererSprite>> SpritesList;

	RendererSpriteSequence(__int32 id)
	{
		Id = id;
	}
};

typedef struct RendererSpriteToDraw {
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
};

typedef struct RendererTempVertex {
	D3DXVECTOR3 Position;
	vector<D3DXVECTOR3> Normals;
	D3DXVECTOR3 AveragedNormal;
};

typedef struct RendererWeatherParticle {
	float X, Y, Z;
	float AngleH;
	float AngleV;
	float Size;
	__int16 Room;
	bool Reset;
};

typedef struct RendererUnderwaterDustParticle {
	float X, Y, Z;
	__int16 Life;
	__int16 Room;
	bool Reset;
};

typedef struct RendererLine3DToDraw {
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

typedef struct RendererRoomNode {
	__int16 From;
	__int16 To;
	D3DXVECTOR4 ClipPort;
};

class Renderer
{
	LPDIRECT3D9						m_d3D = NULL;
	LPDIRECT3DDEVICE9				m_device = NULL;
	LPDIRECT3DTEXTURE9				m_textureAtlas;
	LPDIRECT3DTEXTURE9				m_fontAndMiscTexture;
	LPDIRECT3DTEXTURE9				m_titleScreen;
	LPDIRECT3DTEXTURE9				m_caustics[NUM_CAUSTICS_TEXTURES];
	LPD3DXFONT						m_debugFont;
	LPD3DXFONT						m_gameFont;
	LPD3DXSPRITE					m_dxSprite;
	char							m_message[255];
	map<__int32, shared_ptr<RendererRoom>>		m_rooms;
	map<__int32, shared_ptr<RendererObject>>	m_moveableObjects;
	map<__int32, shared_ptr<RendererObject>>	m_staticObjects;
	map<__int32, shared_ptr<RendererSprite>>	m_sprites;
	map<__int32, shared_ptr<RendererSpriteSequence>>	m_spriteSequences;
	vector<__int32>					m_roomsToDraw;
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDeclaration;
	__int32							m_numVertices;
	__int32							m_numTriangles;
	vector<RendererLine2D>			m_lines2D;
	LPD3DXLINE						m_dxLine;
	__int16							m_normalLaraSkinJointRemap[15][32];
	__int16							m_youngLaraSkinJointRemap[15][32];
	__int16							m_laraSkinJointRemap[15][32];
	__int32							m_fadeTimer;
	__int32							m_fadeMode;
	__int16							m_numHairVertices;
	__int16							m_numHairIndices;
	vector<RendererVertex>			m_hairVertices;
	vector<__int32>					m_hairIndices;
	shared_ptr<RenderTarget2D>		m_renderTarget;
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
	D3DXMATRIX						m_hairsMatrices[12];
	D3DXMATRIX						m_LaraWorldMatrix;
	shared_ptr<RenderTarget2D>		m_shadowMap;
	D3DXMATRIX						m_lightView;
	D3DXMATRIX						m_lightProjection;
	bool							m_enableShadows;
	RendererLight*					m_shadowLight;
	vector<__int32>					m_litItems;
	vector<shared_ptr<RendererLight>>			m_dynamicLights;
	vector<shared_ptr<RendererLight>>			m_lights;
	vector<shared_ptr<RendererItemToDraw>>		m_itemsToDraw;
	vector<shared_ptr<RendererEffectToDraw>>	m_effectsToDraw;
	vector<shared_ptr<RendererSpriteToDraw>>	m_spritesToDraw;
	vector<shared_ptr<RendererLine3DToDraw>>	m_lines3DToDraw;
	
	LPDIRECT3DSURFACE9				m_backBufferTarget;
	LPDIRECT3DSURFACE9				m_backBufferDepth;

	RENDERER_CULLMODE				m_cullMode;
	RENDERER_BLENDSTATE				m_blendState;
	RendererUnderwaterDustParticle  m_underwaterDustParticles[NUM_UNDERWATER_DUST_PARTICLES];
	bool							m_firstUnderwaterDustParticles = true;
	shared_ptr<RenderTarget2D>					m_depthBuffer;
	shared_ptr<RenderTarget2D>					m_normalBuffer;
	shared_ptr<RenderTarget2D>					m_colorBuffer;
	shared_ptr<RenderTarget2D>					m_outputBuffer;
	shared_ptr<RenderTarget2D>					m_lightBuffer; 
	shared_ptr<RenderTarget2D>					m_vertexLightBuffer;
	shared_ptr<RenderTarget2D>					m_postprocessBuffer;
	shared_ptr<Shader>							m_shaderClearGBuffer;
	shared_ptr<Shader>							m_shaderFillGBuffer;
	shared_ptr<Shader>							m_shaderLight;
	shared_ptr<Shader>							m_shaderCombine;
	shared_ptr<Shader>							m_shaderBasic;
	shared_ptr<Shader>							m_shaderDepth;
	shared_ptr<Shader>							m_shaderReconstructZBuffer;
	shared_ptr<Shader>							m_shaderSprites;
	shared_ptr<Shader>							m_shaderRain;
	shared_ptr<Shader>							m_shaderTransparent;
	shared_ptr<Shader>							m_shaderFinalPass;
	RendererVertex					m_fullscreenQuadVertices[4];
	__int32							m_fullscreenQuadIndices[6];
	shared_ptr<RendererSphere>					m_sphereMesh;
	shared_ptr<RendererQuad>					m_quad;
	shared_ptr<RendererQuad>					m_skyQuad;
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
	RendererWeatherParticle			m_rain[NUM_RAIN_DROPS];
	RendererWeatherParticle			m_snow[NUM_SNOW_PARTICLES];
	bool							m_firstWeather;
	PDIRECT3DVERTEXBUFFER9			m_spritesVertexBuffer;
	PDIRECT3DINDEXBUFFER9			m_spritesIndexBuffer;
	PDIRECT3DVERTEXBUFFER9			m_linesVertexBuffer;
	vector<RendererVertex>			m_lines3DVertices;
	bool							m_enableZwrite;
	bool							m_enableZtest;
	__int32							m_currentCausticsFrame = 0;
	vector<shared_ptr<RendererAnimatedTextureSet>> m_animatedTextureSets;
	map<__int16*, RendererMesh*>	m_meshPointersToMesh;
	RENDERER_FADE_STATUS			m_fadeStatus;
	float							m_fadeFactor;
	bool							m_cinematicBars;
	float							m_progress;

	__int32							getAnimatedTextureInfo(__int16 textureId);
	RendererMesh*					getRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs);
	void							fromTrAngle(D3DXMATRIX* matrix, __int16* frameptr, __int32 index);
	void							buildHierarchy(RendererObject* obj);
	void							buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
	void							updateAnimation(RendererItemToDraw* item, RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask);
	bool							printDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message);
	bool							checkPortal(__int16 roomIndex, __int16* portal, D3DXVECTOR4* viewPort, D3DXVECTOR4* clipPort);
	void							getVisibleRooms(int from, int to, D3DXVECTOR4* viewPort, bool water, int count);
	void							collectRooms();
	void							collectItems();
	void							collectEffects();
	void							collectSceneItems();
	void							prepareShadowMap();
	bool							drawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount);
	bool							drawGunFlashes();
	void							insertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b);
	void							drawAllLines2D();
	void							drawGameInfo();
	void							drawDebugInfo();
	void							drawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2);
	void							resetBlink();
	__int32							drawInventoryScene();
	__int32							drawObjectOn2DPosition(__int16 x, __int16 y, __int16 objectNum, __int16 rotX, __int16 rotY, __int16 rotZ);
	__int32							getFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate);
	void							updateLaraAnimations();
	void							updateItemsAnimations();
	void							updateEffects();
	void							addSpriteBillboard(RendererSprite* sprite, float x, float y, float z, byte r, byte g, byte b, float rotation, float scale, float width, float height);
	void							addSprite3D(RendererSprite* sprite, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, byte r, byte g, byte b, float rotation, float scale, float width, float height);
	void							addLine3D(__int32 x1, __int32 y1, __int32 z1, __int32 x2, __int32 y2, __int32 z2, byte r, byte g, byte b);
	void							drawFires();
	void							drawSparks();
	void							drawSmokes();
	void							drawBlood();
	void							drawDrips();
	void							drawBubbles();
	void							drawSplahes();
	void							drawRipples();
	void							drawUnderwaterDust();
	bool							drawGunshells(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	void							createBillboardMatrix(D3DXMATRIX* out, D3DXVECTOR3* particlePos, D3DXVECTOR3* cameraPos);
	bool							drawScene(bool dump);
	bool							bindRenderTargets(RenderTarget2D* rt1, RenderTarget2D* rt2, RenderTarget2D* rt3, RenderTarget2D* rt4);
	bool							restoreBackBuffer();
	bool							drawRoom(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass, bool animated);
	bool							drawStatic(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							drawLara(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							drawItem(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							drawEffect(RendererEffectToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
	bool							drawHorizonAndSky();
	void							collectLights();
	bool							doRain();
	bool							doSnow();
	bool							drawSprites();
	bool							drawLines3D();
	__int32							drawFinalPass();
	void							setCullMode(RENDERER_CULLMODE mode);
	void							setBlendState(RENDERER_BLENDSTATE state);
	void							setDepthWrite(bool value);
	void							setGpuStateForBucket(RENDERER_BUCKETS bucket);
	bool							isRoomUnderwater(__int16 roomNumber);
	bool							isInRoom(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
	void							updateAnimatedTextures();
	void							drawFullScreenBackground(LPDIRECT3DTEXTURE9 texture, float alpha);

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
	__int32							Draw();
	bool							PrepareDataForTheRenderer();
	__int32							DumpGameScene();
	__int32							DrawPauseMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawLoadGameMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawSaveGameMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawStatisticsMenu();
	__int32							DrawSettingsMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawInventory();
	__int32							DrawPickup(__int16 objectNum);
	__int32							SyncRenderer();
	void							PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags);
	void							DrawDashBar();
	void							DrawHealthBar(__int32 percentual);
	void							DrawAirBar(__int32 percentual);
	void							ClearDynamicLights();
	void							AddDynamicLight(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b);
	void							FreeRendererData();
	void							EnableCinematicBars(bool value);
	void							FadeIn();
	void							FadeOut();
	void							DrawLoadingScreen(char* fileName);
	void							UpdateProgress(float value);
	bool							IsFading();
	void							GetLaraBonePosition(D3DXVECTOR3* pos, __int32 bone);
};