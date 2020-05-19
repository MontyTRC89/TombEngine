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
	m_reader->ReadChunks([this](ChunkId* id, long size, int arg) {
		if (id->EqualsTo(m_chunkTextureAtlas))
			return readTexture();
		else if (id->EqualsTo(m_chunkAnimatedTextureSequence))
			return readAnimatedTextureSequence();
		else if (id->EqualsTo(m_chunkRoom))
			return readRoom();
		else
			return false;
		}, 0);

	// Close the stream
	m_stream->Close();
	//delete m_writer;
	//delete m_stream;
}

ChunkReader* m_reader;

bool TrLevel::readTexture()
{
	TrTexturePage texture;

	m_stream->ReadInt32(&texture.width);
	m_stream->ReadInt32(&texture.height);
	m_stream->ReadInt32(&texture.format);
	m_stream->ReadInt32(&texture.flags);

	m_reader->ReadChunks([this, texture](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkTextureColor))
		{
			int imageSize;
			m_stream->ReadInt32(&imageSize);
			m_stream->ReadBytes(texture.colorMap, imageSize);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);

	textures.push_back(texture);
}

bool TrLevel::readAnimatedTextureSequence()
{
	TrAnimatedTexturesSequence sequence;

	m_stream->ReadByte(&sequence.animationType);
	m_stream->ReadFloat(&sequence.fps);
	m_stream->ReadInt32(&sequence.uvRotate);

	m_reader->ReadChunks([this, sequence](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkAnimatedTextureFrame))
		{
			TrAnimatedTexturesFrame frame;
			
			m_stream->ReadInt32(&frame.texture);
			for (int i = 0; i < 4; i++)
			{
				Vector2 texCoords;
				m_stream->ReadVector2(&texCoords);
				frame.textureCoords.push_back(texCoords);
			}

			sequence.frames.push_back(frame);

			return true;
		}
		else
		{
			return false;
		}
		}, 0);

	animatedTextures.push_back(sequence);
}

bool TrLevel::readRoom()
{
	TrRoom room;

	m_stream->ReadInt32(&room.x);
	m_stream->ReadInt32(&room.z);
	m_stream->ReadInt32(&room.yBottom);
	m_stream->ReadInt32(&room.yTop);
	m_stream->ReadInt16(&room.numXsectors);
	m_stream->ReadInt16(&room.numZsectors);
	m_stream->ReadInt16(&room.roomType);
	m_stream->ReadInt16(&room.effect);
	m_stream->ReadFloat(&room.effectStrength);
	m_stream->ReadInt16(&room.alternateRoom);
	m_stream->ReadInt16(&room.alternatGroup);
	m_stream->ReadInt32(&room.flags);
	m_stream->ReadVector3(&room.ambient);

	m_reader->ReadChunks([this, room](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkBucket))
		{
			TrBucket bucket;
			readBucket(&bucket);
			room.buckets.push_back(bucket);
			return true;
		}
		if (id->EqualsTo(m_chunkRoomLight))
		{
			TrLight light;
			m_stream->ReadVector3(&light.position);
			m_stream->ReadVector3(&light.color);
			m_stream->ReadVector3(&light.direction);
			m_stream->ReadByte(&light.type);
			m_stream->ReadBool(&light.castShadows);
			m_stream->ReadFloat(&light.intensity);
			m_stream->ReadFloat(&light.in);
			m_stream->ReadFloat(&light.out);
			m_stream->ReadFloat(&light.len);
			m_stream->ReadFloat(&light.cutoff);
			m_stream->ReadInt32(&light.flags);
			m_reader->ReadChunks([this, room](ChunkId * id, long size, int arg) { return false; }, 0);
			room.lights.push_back(light);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);
}

bool TrLevel::readBucket(TrBucket* bucket)
{
	m_reader->ReadChunks([this, bucket](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkMaterial))
		{
			m_stream->ReadInt32(&bucket->material.texture);
			m_stream->ReadByte(&bucket->material.blendMode);
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesPositions))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				Vector3 vec;
				m_stream->ReadVector3(&vec);
				bucket->positions.push_back(vec);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesNormals))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				Vector3 vec;
				m_stream->ReadVector3(&vec);
				bucket->normals.push_back(vec);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesColors))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				Vector3 vec;
				m_stream->ReadVector3(&vec);
				bucket->colors.push_back(vec);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesTextureCoords))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				Vector2 vec;
				m_stream->ReadVector2(&vec);
				bucket->textureCoords.push_back(vec);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesEffects))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				int fx;
				m_stream->ReadInt32(&fx);
				bucket->verticesEffects.push_back(fx);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkVerticesBones))
		{
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				int bone;
				m_stream->ReadInt32(&bone);
				bucket->bones.push_back(bone);
			}
			return true;
		}
		else if (id->EqualsTo(m_chunkPolygon))
		{
			TrPolygon polygon;

			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				int index;
				m_stream->ReadInt32(&index);
				polygon.indices.push_back(index);
			}
			m_stream->ReadInt16(&polygon.animatedSequence);
			m_stream->ReadInt16(&polygon.frame);
			bucket->polygons.push_back(polygon);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);
}