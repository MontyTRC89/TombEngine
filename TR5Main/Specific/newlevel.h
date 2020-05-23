#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
#include <string>
#include "../Game/sound.h"
#include "../Specific/IO/Streams.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct TrTexturePage 
{
	int width;
	int height;
	int flags;
	int format;
	byte* colorMap;
	byte* normalMap;
};

struct TrPolygon
{
	vector<int> indices;
	int animatedSequence;
	int frame;
};

struct TrMaterial 
{
	int texture;
	byte blendMode;
	bool animated;
};

struct TrBucket 
{
	TrMaterial material;
	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector2> textureCoords;
	vector<Vector3> normals;
	vector<int> verticesEffects;
	vector<int> bones;
	vector<TrPolygon> polygons;
};

struct TrVolume 
{
	int type;
	Vector3 position;
	Quaternion rotation;
	BoundingBox box;
	BoundingSphere sphere;
	string script;
};

struct TrClimbVolume : TrVolume
{
	int climbType;
};

struct TrTriggerVolume : TrVolume
{
	int activators;
};

struct TrSector
{
	int boxIndex;
	int pathfindingFlags;
	int stepSound;
	int roomBelow;
	int roomAbove;
	int floor;
	int ceiling;
	vector<int> floorData;
};

struct TrLight 
{
	Vector3 position;
	Vector3 color;
	Vector3 direction;
	byte type;
	bool castShadows;
	float intensity;
	float in;
	float out;
	float len;
	float cutoff;
	int flags;
};

struct TrRoomStatic 
{
	string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	int staticNumber;
	Vector3 color;
	bool receiveShadows;
	bool castShadows;
	int flags;
	string script;
};

struct TrPortal
{
	int adjoiningRoom;
	Vector3 normal;
	vector<Vector3> vertices;
};

struct TrRoom
{
	int x;
	int z;
	int yBottom;
	int yTop;
	int numXsectors;
	int numZsectors;
	int roomType;
	int reverb;
	int effect;
	float effectStrength;
	int alternateRoom;
	int alternatGroup;
	int flags;
	Vector3 ambient;
	vector<TrBucket> buckets;
	vector<TrLight> lights;
	vector<TrRoomStatic> statics;
	vector<TrSector> sectors;
	vector<TrPortal> portals;
	vector<TrTriggerVolume> triggers;
	vector<TrClimbVolume> climbVolumes;
};

struct TrMesh 
{
	BoundingSphere sphere;
	vector<TrBucket> buckets;
};

struct TrBone
{
	int opcode;
	Vector3 offset;
};

struct TrKeyFrame 
{
	Vector3 origin;
	BoundingBox boundingBox;
	vector<Quaternion> angles;
};

struct TrAnimCommand
{
	int type;
	int frame;
	vector<int> params;
};

struct TrAnimDispatch 
{
	int inFrame;
	int outFrame;
	int nextAnimation;
	int nextFrame;
};

struct TrStateChange 
{
	int state;
	vector<TrAnimDispatch> dispatches;
};

struct TrAnimation
{
	int framerate;
	int state;
	int nextAnimation;
	int nextFrame;
	int frameStart;
	int frameEnd;
	float speed;
	float acceleration;
	float lateralSpeed;
	float lateralAcceleration;
	vector<TrKeyFrame> keyframes;
	vector<TrStateChange> changes;
	vector<TrAnimCommand> commands;
};

struct TrMoveable 
{
	int id;
	vector<TrMesh> meshes;
	vector<TrBone> bones;
	vector<TrAnimation> animations;
};

struct TrStatic 
{
	int id;
	BoundingBox visibilityBox;
	BoundingBox collisionBox;
	vector<TrMesh> meshes;
};

struct TrAnimatedTexturesFrame
{
	int texture;
	vector<Vector2> textureCoords;
};

struct TrAnimatedTexturesSequence 
{
	byte animationType;
	float fps;
	int uvRotate;
	vector<TrAnimatedTexturesFrame> frames;
};

struct TrItem
{
	string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Vector3 color;
	int roomNumber;
	int objectNumber;
	string script;
};

struct TrCamera 
{
	string name;
	Vector3 position;
	int roomNumber;
	int type;
	int flags;
	string script;
};

struct TrSoundSource 
{
	string name;
	Vector3 position;
	int roomNumber;
	float volume;
	int sound;
	int playMode;
};

struct TrSink 
{
	string name;
	Vector3 position;
	int roomNumber;
	float strength;
	int box;
};

struct TrOverlap 
{
	int flags;
	int box;
};

struct TrBox 
{
	Vector2 min;
	Vector2 max;
	int floor;
	vector<TrOverlap> overlaps;
};

struct TrSample 
{
	int uncompressedSize;
	int compressedSize;
	byte* data;
};

struct TrSoundDetails 
{
	float volume;
	float range;
	float chance;
	float pitch;
	bool randomizePitch;
	bool randomizeGain;
	bool noPanoramic;
	byte loop;
	vector<TrSample> samples;
};

struct TrFlybyCamera 
{
	string name;
	int sequence;
	int number;
	Vector3 position;
	Vector3 direction;
	float fov;
	float roll;
	float speed;
	int timer;
	int roomNumber;
	int flags;
	string script;
};

struct TrSprite 
{
	int texture;
	vector<Vector2> textureCoords;
};

struct TrSpriteSequence 
{
	int id;
	vector<TrSprite> sprites;
};

class TrLevel : IHasChunkReader
{
private:
	ChunkId* m_chunkTextureAtlas = ChunkId::FromString("T5MTex");
	ChunkId* m_chunkTextureColor = ChunkId::FromString("T5MTexColMap");
	ChunkId* m_chunkTextureNormalMap = ChunkId::FromString("T5MTexNrmMap");
	ChunkId* m_chunkRoom = ChunkId::FromString("T5MRoom");
	ChunkId* m_chunkRoomInfo = ChunkId::FromString("T5MRoomInfo");
	ChunkId* m_chunkBucket = ChunkId::FromString("T5MBckt");
	ChunkId* m_chunkRoomLight = ChunkId::FromString("T5MLight");
	ChunkId* m_chunkRoomStatic = ChunkId::FromString("T5MRoomSt");
	ChunkId* m_chunkRoomPortal = ChunkId::FromString("T5MPortal");
	ChunkId* m_chunkRoomSector = ChunkId::FromString("T5MSector");
	ChunkId* m_chunkRoomTriggerVolume = ChunkId::FromString("T5MTrigVol");
	ChunkId* m_chunkRoomClimbVolume = ChunkId::FromString("T5MClimbVol");
	ChunkId* m_chunkMaterial = ChunkId::FromString("T5MMat");
	ChunkId* m_chunkVerticesPositions = ChunkId::FromString("T5MVrtPos");
	ChunkId* m_chunkVerticesNormals = ChunkId::FromString("T5MVrtN");
	ChunkId* m_chunkVerticesTextureCoords = ChunkId::FromString("T5MVrtUV");
	ChunkId* m_chunkVerticesColors = ChunkId::FromString("T5MVrtCol");
	ChunkId* m_chunkVerticesEffects = ChunkId::FromString("T5MVrtFX");
	ChunkId* m_chunkVerticesBones = ChunkId::FromString("T5MVrtB");
	ChunkId* m_chunkPolygon = ChunkId::FromString("T5MPoly");
	ChunkId* m_chunkMesh = ChunkId::FromString("T5MMesh");
	ChunkId* m_chunkBone = ChunkId::FromString("T5MBone");
	ChunkId* m_chunkKeyFrame = ChunkId::FromString("T5MKf");
	ChunkId* m_chunkAnimCommand = ChunkId::FromString("T5MAnCmd");
	ChunkId* m_chunkStateChange = ChunkId::FromString("T5MStCh");
	ChunkId* m_chunkAnimDispatch = ChunkId::FromString("T5MAnDisp");
	ChunkId* m_chunkAnimation = ChunkId::FromString("T5MAnim");
	ChunkId* m_chunkMoveable = ChunkId::FromString("T5MMoveable");
	ChunkId* m_chunkStatic = ChunkId::FromString("T5MStatic");
	ChunkId* m_chunkItem = ChunkId::FromString("T5MItem");
	ChunkId* m_chunkAiItem = ChunkId::FromString("T5MAiItem");
	ChunkId* m_chunkCamera = ChunkId::FromString("T5MCamera");
	ChunkId* m_chunkSink = ChunkId::FromString("T5MSink");
	ChunkId* m_chunkFlybyCamera = ChunkId::FromString("T5MFlyBy");
	ChunkId* m_chunkSoundSource = ChunkId::FromString("T5MSndSrc");
	ChunkId* m_chunkBox = ChunkId::FromString("T5MBox");
	ChunkId* m_chunkOverlap = ChunkId::FromString("T5MOv");
	ChunkId* m_chunkZone = ChunkId::FromString("T5MZone");
	ChunkId* m_chunkSoundMap = ChunkId::FromString("T5MSoundMap");
	ChunkId* m_chunkSoundDetail = ChunkId::FromString("T5MSndDet");
	ChunkId* m_chunkSample = ChunkId::FromString("T5MSam");
	ChunkId* m_chunkLeelScript = ChunkId::FromString("T5MScript");
	ChunkId* m_chunkSprite = ChunkId::FromString("T5MSpr");
	ChunkId* m_chunkSpriteSequence = ChunkId::FromString("T5MSprSeq");
	ChunkId* m_chunkDummy = ChunkId::FromString("T5MDummy");
	ChunkId* m_chunkAnimatedTextureSequence = ChunkId::FromString("T5MAnTxSeq");
	ChunkId* m_chunkAnimatedTextureFrame = ChunkId::FromString("T5MAnTxFr");
	
	int m_magicNumber = 0x4D355254;
	string m_filename;
	ChunkReader* m_reader;
	FileStream* m_stream;

	bool readTexture();
	bool readAnimatedTextureSequence();
	bool readRoom();
	bool readBucket(TrBucket* bucket);
	bool readMesh(TrMesh* mesh);
	bool readAnimation(TrAnimation* animation);
	bool readMoveable();
	bool readStatic();
	bool readSpriteSequence();
	bool readItem();
	bool readAiItem();
	bool readSink();
	bool readCamera();
	bool readFlybyCamera();
	bool readSoundSource();
	bool readBox();
	bool readZones();
	bool readSoundMap();
	bool readSoundDetail();

public:
	vector<TrTexturePage> textures;
	vector<TrRoom> rooms;
	vector<TrMoveable> moveables;
	vector<TrStatic> statics;
	vector<TrItem> items;
	vector<TrItem> aiItems;
	vector<TrCamera> cameras;
	vector<TrSoundSource> soundSources;
	vector<TrSink> sinks;
	vector<TrFlybyCamera> flybyCameras;
	vector<TrSpriteSequence> spriteSequences;
	vector<TrAnimatedTexturesSequence> animatedTextures;
	vector<TrBox> boxes;
	vector<int> zones;
	vector<int> soundMap;
	vector<TrSoundDetails> soundDetails;
	string script;

	TrLevel(string filename);
	~TrLevel();
	bool Load();
};