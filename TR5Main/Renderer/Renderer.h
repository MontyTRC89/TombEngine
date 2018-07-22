#pragma once

#include "..\Global\global.h"

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <vector>
#include <map>

#define TEXTURE_ATLAS_SIZE 4096
#define TEXTURE_PAGE_SIZE 262144
#define NUM_TEXTURE_PAGES_PER_ROW 16

#define GET_ATLAS_PAGE_X(p) (p % NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define GET_ATLAS_PAGE_Y(p) floor(p / NUM_TEXTURE_PAGES_PER_ROW) * 256.0f

#define SHAPE_RECTANGLE 0
#define SHAPE_TRIANGLE	1

#define MAX_VERTICES 200000
#define MAX_INDICES 400000
#define MAX_LINES_2D 128

#define NUM_BUCKETS 6

#define RENDERBUCKET_SOLID 0
#define RENDERBUCKET_SOLID_DS 1
#define RENDERBUCKET_ALPHA_TEST 2
#define RENDERBUCKET_ALPHA_TEST_DS 3
#define RENDERBUCKET_TRANSPARENT 4
#define RENDERBUCKET_TRANSPARENT_DS 5

#define	FADEMODE_NONE 0
#define FADEMODE_FADEIN 1
#define FADEMODE_FADEOUT 2

#define PRINTSTRING_CENTER	1
#define PRINTSTRING_BLINK	2

using namespace std;

typedef struct RendererLine2D {
	D3DXVECTOR2 Vertices[2];
	D3DCOLOR Color;
};

typedef struct RendererBone {
	D3DXVECTOR3 Translation;
	D3DXMATRIX GlobalTransform;
	D3DXMATRIX Transform;
	D3DXVECTOR3 GlobalTranslation;
	vector<RendererBone*> Children;
	RendererBone* Parent;
	__int32 Index;

	RendererBone(__int32 index)
	{
		Index = index;
		Translation = D3DXVECTOR3(0, 0, 0);
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

typedef struct RendererBucket {
	vector<RendererVertex> Vertices;
	vector<__int32> Indices;
	vector<RendererPolygon> Polygons;
	PDIRECT3DVERTEXBUFFER9 VertexBuffer;
	PDIRECT3DINDEXBUFFER9 IndexBuffer;
	__int32 StartVertex;
	__int32 NumTriangles;
	__int32 NumVertices;
	__int32 NumIndices;

	RendererBucket()
	{
		StartVertex = 0;
		NumTriangles = 0;
		NumVertices = 0;
		NumIndices = 0;
		VertexBuffer = NULL;
		IndexBuffer = NULL;
	}

	~RendererBucket()
	{
		if (VertexBuffer != NULL)
			VertexBuffer->Release();

		if (IndexBuffer != NULL)
			IndexBuffer->Release();

		VertexBuffer = NULL;
		IndexBuffer = NULL;
	}
};

typedef struct RendererMesh {
	RendererBucket** Buckets;
	vector<D3DXVECTOR3> Positions;

	RendererMesh()
	{
		Buckets = (RendererBucket**)malloc(6 * sizeof(RendererBucket*));
		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			Buckets[i] = new RendererBucket();
	}

	~RendererMesh()
	{
		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			delete Buckets[i];
		delete Buckets;
	}
};

typedef struct RendererObject {
	vector<RendererMesh*> ObjectMeshes;
	RendererBone* Skeleton;
	D3DXMATRIX* AnimationTransforms;
	D3DXMATRIX* BindPoseTransforms;
	vector<RendererBone*> LinearizedBones;
	__int32 Id;

	RendererObject(__int32 numMeshes, __int32 id)
	{
		Id = id;
		ObjectMeshes.reserve(numMeshes);
		AnimationTransforms = (D3DXMATRIX*)malloc(numMeshes * sizeof(D3DXMATRIX));
		BindPoseTransforms = (D3DXMATRIX*)malloc(numMeshes * sizeof(D3DXMATRIX));
	}

	~RendererObject()
	{
		delete AnimationTransforms;
		delete BindPoseTransforms;
		
		for (vector<RendererMesh*>::iterator it = ObjectMeshes.begin(); it != ObjectMeshes.end(); ++it)
			delete (*it);
		ObjectMeshes.clear();

		for (vector<RendererBone*>::iterator it = LinearizedBones.begin(); it != LinearizedBones.end(); ++it)
			delete (*it);
		LinearizedBones.clear();

		Skeleton = NULL;
	}
};

class Renderer
{
	LPDIRECT3D9						m_d3D = NULL;
	LPDIRECT3DDEVICE9				m_device = NULL;
	LPDIRECT3DTEXTURE9				m_textureAtlas;
	LPDIRECT3DTEXTURE9				m_fontAndMiscTexture;
	LPD3DXFONT						m_font;
	LPD3DXSPRITE					m_sprite;
	LPD3DXFONT						m_gameFont;
	LPD3DXEFFECT					m_effect;
	LPD3DXEFFECT					m_effectLines2D;
	LPD3DXEFFECT					m_menuInventoryBackground;
	char							m_message[255];
	map<__int32, RendererObject*>	m_roomObjects;
	map<__int32, RendererObject*>	m_moveableObjects;
	map<__int32, RendererObject*>	m_staticObjects;
	vector<__int32>					m_roomsToDraw;
	__int32*						m_meshTrees;
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDeclaration;
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDeclarationLines2D;
	RendererBucket					m_solidBucket;
	RendererBucket					m_solidDSBucket;
	RendererBucket					m_alphaTestBucket;
	RendererBucket					m_alphaTestDSBucket;
	RendererBucket					m_transparentBucket;
	RendererBucket					m_transparentDSBucket;
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
	LPDIRECT3DTEXTURE9				m_renderTarget;
	LPDIRECT3DSURFACE9				m_renderTargetDepth;
	__int32							m_blinkColorValue;
	__int32							m_blinkColorDirection;
	bool							m_needToDumpScene;
	RendererVertex					m_fullScreenQuadVertices[4];

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
	RendererMesh*					GetRendererMeshFromTrMesh(__int16* meshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs);
	void							FromTrAngle(D3DXMATRIX* matrix, __int16* frameptr, __int32 index);
	void							BuildHierarchy(RendererObject* obj);
	void							BuildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
	void							BuildAnimationPoseRecursive(RendererObject* obj, __int16** frmptr, D3DXMATRIX* parentTransform, __int16 frac, __int16 rate, RendererBone* bone);
	void							BuildAnimationPose(RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate);
	__int32							Draw();
	bool							DrawMessage(LPD3DXFONT font, unsigned int x, unsigned int y,
												int alpha, unsigned char r, unsigned char g, unsigned char b,
												LPCSTR Message);
	bool							PrepareDataForTheRenderer();
	void							CollectRoomsToDraw();
	void							InitialiseBucketBuffer(RendererBucket* bucket);
	void							DrawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount);
	void							DrawRoom(__int32 roomIndex, __int32 bucketIndex);
	void							DrawStatic(__int32 roomIndex, __int32 staticIndex, __int32 bucketIndex);
	void							DrawLara(__int32 bucketIndex);
	void							DrawItem(__int32 itemIndex, __int32 bucketIndex);
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
	__int32							DumpScreen();
	void							ResetBlink();
	__int32							DrawPauseMenu(__int32 selectedIndex, bool resetBlink);
	__int32							DrawStatisticsMenu();
	__int32							DrawSettingsMenu(__int32 selectedIndex, bool resetBlink);
	//void							FadeIn();
	//void							FadeOut();
	//void							DrawRoom(RendererObject* roomObj, ROOM_INFO* room, RendererBucket* bucket);
	//void							DrawStaticObject(RendererObject* staticObj, MESH_INFO* staticInfo, RendererBucket* bucket);
};