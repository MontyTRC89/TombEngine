#pragma once
#include "framework.h"

struct TEXTURE
{
	int width;
	int height;
	int size;
	std::vector<byte> data;
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
	std::vector<int> indices;
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
	std::vector<Vector3> positions;
	std::vector<Vector3> colors;
	std::vector<Vector2> textureCoords;
	std::vector<Vector3> normals;
	std::vector<int> verticesEffects;
	std::vector<int> bones;
	std::vector<TrPolygon> polygons;
};

struct TrVolume
{
	int type;
	Vector3 position;
	Quaternion rotation;
	BoundingBox box;
	BoundingSphere sphere;
	std::string script;
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
	std::vector<int> floorData;
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
	std::string name;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	int staticNumber;
	Vector3 color;
	bool receiveShadows;
	bool castShadows;
	int flags;
	std::string script;
};

struct TrPortal
{
	int adjoiningRoom;
	Vector3 normal;
	std::vector<Vector3> vertices;
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
	std::vector<TrBucket> buckets;
	std::vector<TrLight> lights;
	std::vector<TrRoomStatic> statics;
	std::vector<TrSector> sectors;
	std::vector<TrPortal> portals;
	std::vector<TrTriggerVolume> triggers;
	std::vector<TrClimbVolume> climbVolumes;
	int itemNumber;
	int fxNumber;
};

struct TrMesh
{
	BoundingSphere sphere;
	std::vector<TrBucket> buckets;
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
	std::vector<Quaternion> angles;
};

struct TrAnimCommand
{
	int type;
	int frame;
	std::vector<int> params;
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
	std::vector<TrAnimDispatch> dispatches;
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
	std::vector<TrKeyFrame> keyframes;
	std::vector<TrStateChange> changes;
	std::vector<TrAnimCommand> commands;
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
	std::vector<TrMesh> meshes;
	std::vector<TrBone> bones;
	std::vector<TrAnimation> animations;
};

struct TrStatic
{
	int id;
	BoundingBox visibilityBox;
	BoundingBox collisionBox;
	int meshNumber;
	int meshCount;
	std::vector<TrMesh> meshes;
};

struct TrAnimatedTexturesFrame
{
	int texture;
	std::vector<Vector2> textureCoords;
};

struct TrAnimatedTexturesSequence
{
	byte animationType;
	float fps;
	int uvRotate;
	std::vector<TrAnimatedTexturesFrame> frames;
};

struct TrItem
{
	std::string name;
	Vector3 position;
	Quaternion rotation;
	float angle;
	Vector3 scale;
	Vector3 color;
	int roomNumber;
	int objectNumber;
	std::string script;
};

struct TrCamera
{
	std::string name;
	Vector3 position;
	int roomNumber;
	int type;
	int flags;
	std::string script;
};

struct TrSoundSource
{
	std::string name;
	Vector3 position;
	int roomNumber;
	float volume;
	int sound;
	int playMode;
};

struct TrSink
{
	std::string name;
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
	std::vector<TrOverlap> overlaps;
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
	std::vector<TrSample> samples;
};

struct TrFlybyCamera
{
	std::string name;
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
	std::string script;
};

struct TrSprite
{
	int texture;
	std::vector<Vector2> textureCoords;
};

struct TrSpriteSequence
{
	int id;
	int spritesIndex;
	int spritesCount;
	std::vector<TrSprite> sprites;
};

struct TrLevel
{
	std::vector<TrTexturePage> textures;
	std::vector<TrRoom> rooms;
	std::vector<int> floorData;
	std::vector<TrMesh> meshes;
	std::vector<TrAnimation> animations;
	std::vector<TrBone> bones;
	std::vector<TrStateChange> changes;
	std::vector<TrAnimDispatch> dispatches;
	std::vector<TrKeyFrame> frames;
	std::vector<TrAnimCommand> commands;
	std::vector<TrAnimatedTexturesSequence> animatedTextures;
	std::vector<TrSprite> sprites;
	std::vector<TrSpriteSequence> spriteSequences;
	std::vector<int> soundMap;
	std::vector<TrSoundDetails> soundDetails;
	std::vector<TrSample> samples;
	std::vector<TrItem> items;
	std::vector<TrItem> aiItems;
	std::vector<TrMoveable> moveables;
	std::vector<TrStatic> statics;
	std::vector<TrBox> boxes;
	std::vector<TrOverlap> overlaps;
	std::vector<int> zones[5][2];
	std::vector<TrSink> sinks;
	std::vector<TrCamera> cameras;
	std::vector<TrSoundSource> soundSources;
	std::vector<TrFlybyCamera> flybyCameras;
};