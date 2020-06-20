#pragma once
#include "framework.h"

struct TEXTURE
{
	int width;
	int height;
	int size;
	vector<byte> data;
};

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
	int floorDataIndex;
	int floorDataCount;
	int box;
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
	int itemNumber;
	int fxNumber;
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
	int dispatchIndex;
	int dispatchCount;
	vector<TrAnimDispatch> dispatches;
};

struct TrAnimation
{
	int framerate;
	int state;
	int nextAnimation;
	int nextFrame;
	int frameBase;
	int frameEnd;
	float speed;
	float acceleration;
	float lateralSpeed;
	float lateralAcceleration;
	int framesIndex;
	int framesCount;
	int changesIndex;
	int changesCount;
	int commandsIndex;
	int commandsCount;
	vector<TrKeyFrame> keyframes;
	vector<TrStateChange> changes;
	vector<TrAnimCommand> commands;
};

struct TrMoveable
{
	int id;
	int animationIndex;
	int animationCount;
	int meshIndex;
	int meshCount;
	int bonesIndex;
	int bonesCount;
	vector<TrMesh> meshes;
	vector<TrBone> bones;
	vector<TrAnimation> animations;
};

struct TrStatic
{
	int id;
	BoundingBox visibilityBox;
	BoundingBox collisionBox;
	int meshNumber;
	int meshCount;
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
	float angle;
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
	int flags;
	int overlapsIndex;
	int overlapsCount;
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
	int spritesIndex;
	int spritesCount;
	vector<TrSprite> sprites;
};

struct TrLevel
{
	vector<TrTexturePage> textures;
	vector<TrRoom> rooms;
	vector<int> floorData;
	vector<TrMesh> meshes;
	vector<TrAnimation> animations;
	vector<TrBone> bones;
	vector<TrStateChange> changes;
	vector<TrAnimDispatch> dispatches;
	vector<TrKeyFrame> frames;
	vector<TrAnimCommand> commands;
	vector<TrAnimatedTexturesSequence> animatedTextures;
	vector<TrSprite> sprites;
	vector<TrSpriteSequence> spriteSequences;
	vector<int> soundMap;
	vector<TrSoundDetails> soundDetails;
	vector<TrSample> samples;
	vector<TrItem> items;
	vector<TrItem> aiItems;
	vector<TrMoveable> moveables;
	vector<TrStatic> statics;
	vector<TrBox> boxes;
	vector<TrOverlap> overlaps;
	vector<int> zones[5][2];
	vector<TrSink> sinks;
	vector<TrCamera> cameras;
	vector<TrSoundSource> soundSources;
	vector<TrFlybyCamera> flybyCameras;
};