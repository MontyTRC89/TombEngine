#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
#include <string>
#include "../Game/sound.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct TrTexturePage {
	short width;
	short height;
	short flags;
	short format;
	byte* colorMap;
	byte* normalMap;
};

struct TrPolygon {
	int indices[3];
	byte animatedSequence;
	byte frame;
};

struct TrBucket {
	TrTexturePage* texture;
	byte blendMode;
	byte doubleSided;
	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector2> textureCoords;
	vector<Vector3> normals;
	vector<int> verticesEffects;
	vector<TrPolygon> polygons;
};

struct TrVolume {
	Vector3 position;
	Quaternion rotation;
	BoundingBox box;
	BoundingSphere sphere;
	byte type;
};

struct TrClimbVolume : TrVolume {
	byte climbType;	
};

struct TrTriggerVolume : TrVolume {
	string script;
};

struct TrSector {
	int boxIndex;
	short pathfindingFlags;
	short stepSound;
	short roomBelow;
	short roomAbove;
	int floor;
	int ceiling;
	vector<short> floorData;
};

struct TrLight {
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
};

struct TrRoomStatic {
	Vector3 position;
	Quaternion rotiation;
	Vector3 scale;
	short staticNumber;
	Vector3 color;
	bool receiveShadows;
	bool castShadows;
	short flags;
	string script;
};

struct TrRoom {
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
	vector<TrBucket> buckets;
	vector<TrLight> lights;
	vector<TrRoomStatic> statics;
	vector<TrSector> sectors;
	vector<TrTriggerVolume> triggers;
	vector<TrClimbVolume> climbVolumes;
};

struct TrMesh {
	BoundingSphere sphere;
	vector<TrBucket> buckets;
};

struct TrBone {
	int index;
	TrBone* parent;
	vector<TrBone*> children;
	Vector3 translation;
	Matrix bindPoseTransform;
	Matrix animationTransform;
};

struct TrKeyFrame {
	Vector3 origin;
	BoundingBox boundingBox;
	vector<Quaternion> angles;
};

struct TrAnimCommand {
	short type;
	vector<short> params;
};

struct TrAnimDispatch {
	short inFrame;
	short outFrame;
	short nextAnimation;
	short nextFrame;
};

struct TrStateChange {
	short state;
	vector<TrAnimDispatch> dispatches;
};

struct TrAnimation {
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

struct TrMoveable {
	short id;
	vector<TrMesh> meshes;
	vector<TrBone> bones;
	TrBone* skeleton;
	vector<TrAnimation> animations;
};

struct TrStatic {
	short id;
	BoundingBox visibilityBox;
	BoundingBox collisionBox;
	vector<TrMesh> meshes;
};

struct TrAnimatedTexturesFrame {
	TrTexturePage* texture;
	Vector2 textureCoords[3];
};

struct TrAnimatedTexturesSequence {
	byte frameRate;
	byte effect;
	short params;
	vector<TrAnimatedTexturesFrame> frames;
};

struct TrItem {
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	short roomNumber;
	short objectNumber;
	string script;
};

struct TrCamera {
	Vector3 position;
	short roomNumber;
	short flags;
	string script;
};

struct TrSoundSource {
	Vector3 position;
	short roomNumber;
	float volume;
	short sound;
	short flags;
};

struct TrSink {
	Vector3 position;
	short roomNumber;
	float strength;
};

struct TrOverlap {
	short flags;
	vector<TrBox*> boxes;
};

struct TrBox {
	Vector2 min;
	Vector2 max;
	int floor;
	vector<TrOverlap> overlaps;
};

struct TrSample {
	byte* data;
};

struct TrSoundDetails {
	float volume;
	float range;
	float chance;
	float pitch;
	bool randomizePitch;
	bool randomizeGain;
	bool noPanoramic;
	byte loop;
	vector<TrSample*> samples;
};

struct TrFlybyCamera {
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

struct TrFlybySequences {
	vector<TrFlybyCamera> cameras;
	int flags;
	string script;
};

struct TrSprite {
	TrTexturePage* texture;
	Vector2 textureCoords[4];
};

struct TrSpriteSequence {
	vector<TrSprite> sprites;
};

struct TrLevel {
	vector<TrTexturePage> textures;
	vector<TrRoom> rooms;
	vector<TrMoveable> moveables;
	vector<TrStatic> statics;
	vector<TrAnimatedTexturesSequence> animatedTextures;
	vector<TrItem> items;
	vector<TrItem> nullmeshItems;
	vector<TrCamera> cameras;
	vector<TrSoundSource> soundSources;
	vector<TrSink> sinks;
	vector<TrFlybySequences> flybySequences;
	vector<TrBox> boxes;
	vector<int> zones;
	short soundMap[SOUND_NEW_SOUNDMAP_MAX_SIZE];
	vector<TrSample> samples;
	vector<TrSoundDetails> soundDetails;
	string script;
};