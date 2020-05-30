#pragma once
#include "framework.h"
#include "sound.h"
#include "Streams.h"
#include "newtypes.h"
#include "ChunkId.h"
#include "ChunkReader.h"

class LevelLoader 
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
	ChunkId* m_chunkFloorData = ChunkId::FromString("T5MFloorData");
	
	int m_magicNumber = 0x4D355254;
	string m_filename;
	ChunkReader* m_reader;
	FileStream* m_stream;
	int m_numSamples = 0;
	TrLevel m_level;

	bool readTexture();
	bool readAnimatedTextureSequence();
	bool readRoom();
	bool readBucket(TrBucket* bucket);
	bool readMesh();
	bool readAnimation();
	bool readChange();
	bool readDispatch();
	bool readBone();
	bool readKeyFrame();
	bool readCommand();
	bool readOverlap();
	bool readFloorData();
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
	bool readSample();

public:
	LevelLoader(string filename);
	~LevelLoader();
	bool Load();
};