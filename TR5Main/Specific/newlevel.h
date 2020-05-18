#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
#include <string>
#include "../Game/sound.h"

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
	short animatedSequence;
	short frame;
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
	short type;
	Vector3 position;
	Quaternion rotation;
	BoundingBox box;
	BoundingSphere sphere;
	string script;
};

struct TrClimbVolume : TrVolume
{
	short climbType;
};

struct TrTriggerVolume : TrVolume
{
	short activators;
};

struct TrSector
{
	int boxIndex;
	short pathfindingFlags;
	short stepSound;
	short roomBelow;
	short roomAbove;
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
	Quaternion rotiation;
	Vector3 scale;
	short staticNumber;
	Vector3 color;
	bool receiveShadows;
	bool castShadows;
	int flags;
	string script;
};

struct TrPortal
{
	short AdjoiningRoom;
	Vector3 normal;
	vector<Vector3> vertices;
};

struct TrRoom
{
	int x;
	int z;
	int yBottom;
	int yTop;
	short numXsectors;
	short numZsectors;
	short roomType;
	short reverb;
	short effect;
	float effectStrength;
	short alternateRoom;
	short alternatGroup;
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
	short type;
	vector<short> params;
};

struct TrAnimDispatch 
{
	short inFrame;
	short outFrame;
	short nextAnimation;
	short nextFrame;
};

struct TrStateChange 
{
	short state;
	vector<TrAnimDispatch> dispatches;
};

struct TrAnimation
{
	short framerate;
	short state;
	short nextAnimation;
	short nextFrame;
	short frameStart;
	short frameEnd;
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
	short id;
	vector<TrMesh> meshes;
	vector<TrBone> bones;
	vector<TrAnimation> animations;
};

struct TrStatic 
{
	short id;
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
	short roomNumber;
	short objectNumber;
	string script;
};

struct TrCamera 
{
	string name;
	Vector3 position;
	short roomNumber;
	short type;
	short flags;
	string script;
};

struct TrSoundSource 
{
	string name;
	Vector3 position;
	short roomNumber;
	float volume;
	short sound;
	short playMode;
};

struct TrSink 
{
	string name;
	Vector3 position;
	short roomNumber;
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
	short sequence;
	short number;
	Vector3 position;
	Vector3 direction;
	float fov;
	float roll;
	float speed;
	short timer;
	short roomNumber;
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

struct TrLevel {
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
	vector<short> soundMap;
	vector<TrSoundDetails> soundDetails;
	string script;
};