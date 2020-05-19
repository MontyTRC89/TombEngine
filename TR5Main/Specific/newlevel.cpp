#include "newlevel.h"
#include "../Specific/IO/ChunkId.h"
#include "../Specific/IO/ChunkReader.h"

TrLevel::TrLevel(string filename)
{
	m_filename = filename;
}

TrLevel::~TrLevel()
{
	delete m_chunkTextureAtlas;
	delete m_chunkTextureColor;
	delete m_chunkTextureNormalMap;
	delete m_chunkRoom;
	delete m_chunkRoomInfo;
	delete m_chunkBucket;
	delete m_chunkRoomLight;
	delete m_chunkRoomStatic;
	delete m_chunkRoomPortal;
	delete m_chunkRoomSector;
	delete m_chunkRoomTriggerVolume;
	delete m_chunkRoomClimbVolume;
	delete m_chunkMaterial;
	delete m_chunkVerticesPositions;
	delete m_chunkVerticesNormals;
	delete m_chunkVerticesTextureCoords;
	delete m_chunkVerticesColors;
	delete m_chunkVerticesEffects;
	delete m_chunkVerticesBones;
	delete m_chunkPolygon;
	delete m_chunkMesh;
	delete m_chunkBone;
	delete m_chunkKeyFrame;
	delete m_chunkAnimCommand;
	delete m_chunkStateChange;
	delete m_chunkAnimDispatch;
	delete m_chunkAnimation;
	delete m_chunkMoveable;
	delete m_chunkStatic;
	delete m_chunkItem;
	delete m_chunkAiItem;
	delete m_chunkCamera;
	delete m_chunkSink;
	delete m_chunkFlybyCamera;
	delete m_chunkSoundSource;
	delete m_chunkBox;
	delete m_chunkOverlap;
	delete m_chunkZone;
	delete m_chunkSoundMap;
	delete m_chunkSoundDetail;
	delete m_chunkSample;
	delete m_chunkLeelScript;
	delete m_chunkSprite;
	delete m_chunkSpriteSequence;
	delete m_chunkDummy;
	delete m_chunkAnimatedTextureSequence;
	delete m_chunkAnimatedTextureFrame;
}

bool TrLevel::Load()
{
	m_stream = new FileStream(m_filename.c_str, true, false);
	m_reader = new ChunkReader(m_magicNumber, m_stream);

	// Read chunks
	m_reader->ReadChunks(&readLevelChunks, 0);

	// Close the stream
	m_stream->Close();
	//delete m_writer;
	//delete m_stream;
}

ChunkReader* m_reader;

bool readTextures()
{

}

bool readLevelChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(m_chunk))
		return readTextures();
}