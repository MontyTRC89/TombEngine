#include "framework.h"
#include "levelloader.h"
#include "ChunkId.h"
#include "ChunkReader.h"
#include "level.h"
#include <Specific\setup.h>

LevelLoader::LevelLoader(string filename)
{
	m_filename = filename;
}

LevelLoader::~LevelLoader()
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
	delete m_chunkFloorData;
}

bool LevelLoader::Load()
{
	m_stream = new FileStream((char*)m_filename.c_str(), true, false);
	m_reader = new ChunkReader(m_magicNumber, m_stream);

	// Read chunks
	m_reader->ReadChunks([this](ChunkId* id, long size, int arg) {
		if (id->EqualsTo(m_chunkTextureAtlas))
			return readTexture();
		else if (id->EqualsTo(m_chunkAnimatedTextureSequence))
			return readAnimatedTextureSequence();
		else if (id->EqualsTo(m_chunkRoom))
			return readRoom();
		else if (id->EqualsTo(m_chunkFloorData))
			return readFloorData();
		else if (id->EqualsTo(m_chunkMesh))
			return readMesh();
		else if (id->EqualsTo(m_chunkBone))
			return readBone();
		else if (id->EqualsTo(m_chunkStateChange))
			return readChange();
		else if (id->EqualsTo(m_chunkAnimDispatch))
			return readDispatch();
		else if (id->EqualsTo(m_chunkKeyFrame))
			return readKeyFrame();
		else if (id->EqualsTo(m_chunkAnimCommand))
			return readCommand();
		else if (id->EqualsTo(m_chunkAnimation))
			return readAnimation();
		else if (id->EqualsTo(m_chunkMoveable))
			return readMoveable();
		else if (id->EqualsTo(m_chunkStatic))
			return readStatic();
		else if (id->EqualsTo(m_chunkItem))
			return readItem();
		else if (id->EqualsTo(m_chunkAiItem))
			return readAiItem();
		else if (id->EqualsTo(m_chunkSpriteSequence))
			return readSpriteSequence();
		else if (id->EqualsTo(m_chunkCamera))
			return readCamera();
		else if (id->EqualsTo(m_chunkSink))
			return readSink();
		else if (id->EqualsTo(m_chunkFlybyCamera))
			return readFlybyCamera();
		else if (id->EqualsTo(m_chunkSoundSource))
			return readSoundSource();
		else if (id->EqualsTo(m_chunkSoundMap))
			return readSoundMap();
		else if (id->EqualsTo(m_chunkSoundDetail))
			return readSoundDetail();
		else if (id->EqualsTo(m_chunkSample))
			return readSample();
		else if (id->EqualsTo(m_chunkBox))
			return readBox();
		else if (id->EqualsTo(m_chunkOverlap))
			return readOverlap();
		else if (id->EqualsTo(m_chunkZone))
			return readZones();
		else
			return false;
		}, 0);

	// Close the stream
	m_stream->Close();
	//delete m_writer;
	//delete m_stream;
	g_Level = m_level;

	return true;
}

ChunkReader* m_reader;

bool LevelLoader::readTexture()
{
	TrTexturePage texture;

	m_stream->ReadInt32(&texture.width);
	m_stream->ReadInt32(&texture.height);
	m_stream->ReadInt32(&texture.format);
	m_stream->ReadInt32(&texture.flags);

	m_reader->ReadChunks([this, &texture](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkTextureColor))
		{
			int imageSize;
			m_stream->ReadInt32(&imageSize);
			texture.colorMap = (byte*)malloc(imageSize);
			m_stream->ReadBytes(texture.colorMap, imageSize);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);

	m_level.textures.push_back(texture);

	return true;
}

bool LevelLoader::readAnimatedTextureSequence()
{
	TrAnimatedTexturesSequence sequence;

	m_stream->ReadByte(&sequence.animationType);
	m_stream->ReadFloat(&sequence.fps);
	m_stream->ReadInt32(&sequence.uvRotate);

	m_reader->ReadChunks([this, &sequence](ChunkId * id, long size, int arg) {
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

	m_level.animatedTextures.push_back(sequence);

	return true;
}

bool LevelLoader::readRoom()
{
	TrRoom room;

	m_stream->ReadInt32(&room.x);
	m_stream->ReadInt32(&room.z);
	m_stream->ReadInt32(&room.yBottom);
	m_stream->ReadInt32(&room.yTop);
	m_stream->ReadInt32(&room.numXsectors);
	m_stream->ReadInt32(&room.numZsectors);
	m_stream->ReadInt32(&room.roomType);
	m_stream->ReadInt32(&room.reverb);
	m_stream->ReadInt32(&room.effect);
	m_stream->ReadFloat(&room.effectStrength);
	m_stream->ReadInt32(&room.alternateRoom);
	m_stream->ReadInt32(&room.alternatGroup);
	m_stream->ReadInt32(&room.flags);
	m_stream->ReadVector3(&room.ambient);

	m_reader->ReadChunks([this, &room](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkBucket))
		{
			TrBucket bucket;
			readBucket(&bucket);
			room.buckets.push_back(bucket);
			return true;
		}
		else if (id->EqualsTo(m_chunkRoomLight))
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
		else if (id->EqualsTo(m_chunkRoomStatic))
		{
			TrRoomStatic sm;
			m_stream->ReadString(&sm.name);
			m_stream->ReadVector3(&sm.position);
			m_stream->ReadQuaternion(&sm.rotation);
			m_stream->ReadVector3(&sm.scale);
			m_stream->ReadInt32(&sm.staticNumber);
			m_stream->ReadBool(&sm.receiveShadows);
			m_stream->ReadBool(&sm.castShadows);
			m_stream->ReadInt32(&sm.flags);
			m_stream->ReadString(&sm.script);
			m_reader->ReadChunks([this, room](ChunkId * id, long size, int arg) { return false; }, 0);
			room.statics.push_back(sm);
			return true;
		}
		else if (id->EqualsTo(m_chunkRoomPortal))
		{
			TrPortal portal;

			m_stream->ReadInt32(&portal.adjoiningRoom);
			m_stream->ReadVector3(&portal.normal);
			for (int i = 0; i < 4; i++)
			{
				Vector3 v;
				m_stream->ReadVector3(&v);
				portal.vertices.push_back(v);
			}				
			
			m_reader->ReadChunks([this, room](ChunkId * id, long size, int arg) { return false; }, 0);
			
			room.portals.push_back(portal);
			
			return true;
		}
		else if (id->EqualsTo(m_chunkRoomSector))
		{
			TrSector sector;

			m_stream->ReadInt32(&sector.floorDataIndex);
			m_stream->ReadInt32(&sector.floorDataCount);
			m_stream->ReadInt32(&sector.box);
			m_stream->ReadInt32(&sector.pathfindingFlags);
			m_stream->ReadInt32(&sector.stepSound);
			m_stream->ReadInt32(&sector.roomBelow);
			m_stream->ReadInt32(&sector.roomAbove);
			m_stream->ReadInt32(&sector.floor);
			m_stream->ReadInt32(&sector.ceiling);
			int count;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				int data;
				m_stream->ReadInt32(&data);
				sector.floorData.push_back(data);
			}
			m_reader->ReadChunks([this, room](ChunkId * id, long size, int arg) { return false; }, 0);
			room.sectors.push_back(sector);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);

	m_level.rooms.push_back(room);

	return true;
}

bool LevelLoader::readBucket(TrBucket* bucket)
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
			m_stream->ReadInt32(&polygon.animatedSequence);
			m_stream->ReadInt32(&polygon.frame);
			bucket->polygons.push_back(polygon);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);
	return true;
}

bool LevelLoader::readFloorData()
{
	int count;
	m_stream->ReadInt32(&count);
	for (int i = 0; i < count; i++)
	{
		int data;
		m_stream->ReadInt32(&data);
		m_level.floorData.push_back(data);
	}
	return true;
}

bool LevelLoader::readMesh()
{
	TrMesh mesh;

	m_stream->ReadBoundingSphere(&mesh.sphere);
	m_reader->ReadChunks([this, &mesh](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkBucket))
		{
			TrBucket bucket;
			readBucket(&bucket);
			mesh.buckets.push_back(bucket);
			return true;
		}
		else
		{
			return false;
		}
		}, 0);
	m_level.meshes.push_back(mesh);

	return true;
}

bool LevelLoader::readAnimation()
{
	TrAnimation animation;

	m_stream->ReadInt32(&animation.framerate);
	m_stream->ReadInt32(&animation.state);
	m_stream->ReadInt32(&animation.nextAnimation);
	m_stream->ReadInt32(&animation.nextFrame);
	m_stream->ReadInt32(&animation.frameBase);
	m_stream->ReadInt32(&animation.frameEnd);
	m_stream->ReadFloat(&animation.speed);
	m_stream->ReadFloat(&animation.acceleration);
	m_stream->ReadFloat(&animation.lateralSpeed);
	m_stream->ReadFloat(&animation.lateralAcceleration);
	m_stream->ReadInt32(&animation.framesIndex);
	m_stream->ReadInt32(&animation.framesCount);
	m_stream->ReadInt32(&animation.changesIndex);
	m_stream->ReadInt32(&animation.changesCount);
	m_stream->ReadInt32(&animation.commandsIndex);
	m_stream->ReadInt32(&animation.commandsCount);

	m_reader->ReadChunks([this, &animation](ChunkId * id, long size, int arg) {
		return false;

		/*
		if (id->EqualsTo(m_chunkKeyFrame))
		{
			TrKeyFrame kf;

			m_stream->ReadVector3(&kf.origin);
			m_stream->ReadBoundingBox(&kf.boundingBox);
			int count = 0;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				Quaternion q;
				m_stream->ReadQuaternion(&q);
				kf.angles.push_back(q);
			}

			animation->keyframes.push_back(kf);
			return true;
		} 
		else if (id->EqualsTo(m_chunkStateChange))
		{
			TrStateChange change;

			m_stream->ReadInt32(&change.state);
			m_reader->ReadChunks([this, &change](ChunkId * id, long size, int arg) {
				if (id->EqualsTo(m_chunkAnimDispatch))
				{
					TrAnimDispatch dispatch;

					m_stream->ReadInt32(&dispatch.inFrame);
					m_stream->ReadInt32(&dispatch.outFrame);
					m_stream->ReadInt32(&dispatch.nextAnimation);
					m_stream->ReadInt32(&dispatch.nextFrame);

					change.dispatches.push_back(dispatch);
					return true;
				}
				else
				{
					return false;
				}
				}, 0);

			animation->changes.push_back(change);
			return true;
		}
		else if (id->EqualsTo(m_chunkAnimCommand))
		{
			TrAnimCommand cmd;

			m_stream->ReadInt32(&cmd.type);
			m_stream->ReadInt32(&cmd.frame);
			int count = 0;
			m_stream->ReadInt32(&count);
			for (int i = 0; i < count; i++)
			{
				int p;
				m_stream->ReadInt32(&p);
				cmd.params.push_back(p);
			}

			animation->commands.push_back(cmd);
			return true;
		}
		else
		{
			return false;
		}*/
		}, 0);

	m_level.animations.push_back(animation);

	return true;
}

bool LevelLoader::readChange()
{
	TrStateChange change;

	m_stream->ReadInt32(&change.state);
	m_stream->ReadInt32(&change.dispatchIndex);
	m_stream->ReadInt32(&change.dispatchCount);

	m_reader->ReadChunks([this, &change](ChunkId * id, long size, int arg) {
		return false;
		}, 0);

	m_level.changes.push_back(change);

	return true;
}

bool LevelLoader::readDispatch()
{
	TrAnimDispatch dispatch;

	m_stream->ReadInt32(&dispatch.inFrame);
	m_stream->ReadInt32(&dispatch.outFrame);
	m_stream->ReadInt32(&dispatch.nextAnimation);
	m_stream->ReadInt32(&dispatch.nextFrame);

	m_level.dispatches.push_back(dispatch);

	return true;
}

bool LevelLoader::readBone()
{
	TrBone bone;

	m_stream->ReadInt32(&bone.opcode);
	m_stream->ReadVector3(&bone.offset);

	m_level.bones.push_back(bone);

	return true;
}

bool LevelLoader::readCommand()
{
	TrAnimCommand cmd;

	m_stream->ReadInt32(&cmd.type);
	m_stream->ReadInt32(&cmd.frame);
	int count = 0;
	m_stream->ReadInt32(&count);
	for (int i = 0; i < count; i++)
	{
		int p;
		m_stream->ReadInt32(&p);
		cmd.params.push_back(p);
	}

	m_level.commands.push_back(cmd);

	return true;
}

bool LevelLoader::readKeyFrame()
{
	TrKeyFrame kf;

	m_stream->ReadVector3(&kf.origin);
	m_stream->ReadBoundingBox(&kf.boundingBox);
	int count = 0;
	m_stream->ReadInt32(&count);
	for (int i = 0; i < count; i++)
	{
		Quaternion q;
		m_stream->ReadQuaternion(&q);
		kf.angles.push_back(q);
	}

	m_level.frames.push_back(kf);
	
	return true;
}

bool LevelLoader::readMoveable()
{
	TrMoveable moveable;

	m_stream->ReadInt32(&moveable.id);
	m_stream->ReadInt32(&moveable.meshIndex);
	m_stream->ReadInt32(&moveable.meshCount);
	m_stream->ReadInt32(&moveable.bonesIndex);
	m_stream->ReadInt32(&moveable.bonesCount);
	m_stream->ReadInt32(&moveable.animationIndex);
	m_stream->ReadInt32(&moveable.animationCount);

	m_reader->ReadChunks([this, &moveable](ChunkId * id, long size, int arg) {
		return false;
		/*if (id->EqualsTo(m_chunkMesh))
		{
			TrMesh mesh;
			readMesh(&mesh);
			moveable.meshes.push_back(mesh);
			return true;
		}
		else if (id->EqualsTo(m_chunkBone))
		{
			TrBone bone;
			m_stream->ReadInt32(&bone.opcode);
			m_stream->ReadVector3(&bone.offset);
			moveable.bones.push_back(bone);
			return true;
		}
		else if (id->EqualsTo(m_chunkAnimation))
		{
			TrAnimation animation;
			readAnimation(&animation);
			moveable.animations.push_back(animation);
			return true;
		}
		else
		{
			return false;
		}*/
		}, 0);

	m_level.moveables.push_back(moveable);

	return true;
}

bool LevelLoader::readStatic()
{
	TrStatic st;

	m_stream->ReadInt32(&st.id);
	m_stream->ReadBoundingBox(&st.visibilityBox);
	m_stream->ReadBoundingBox(&st.collisionBox);
	m_stream->ReadInt32(&st.meshNumber);
	m_stream->ReadInt32(&st.meshCount);

	m_reader->ReadChunks([this, &st](ChunkId * id, long size, int arg) {
		/*if (id->EqualsTo(m_chunkMesh))
		{
			TrMesh mesh;
			readMesh(&mesh);
			st.meshes.push_back(mesh);
			return true;
		}
		else
		{
			return false;
		}*/
		return false;
		}, 0);
	
	m_level.statics.push_back(st);

	return true;
}


bool LevelLoader::readSpriteSequence()
{
	TrSpriteSequence sequence;

	m_stream->ReadInt32(&sequence.id);
	m_stream->ReadInt32(&sequence.spritesIndex);
	m_stream->ReadInt32(&sequence.spritesCount);

	m_reader->ReadChunks([this, &sequence](ChunkId * id, long size, int arg) {
		/*if (id->EqualsTo(m_chunkSprite))
		{
			TrSprite sprite;

			m_stream->ReadInt32(&sprite.texture);
			for (int i = 0; i < 4; i++)
			{
				Vector2 coords;
				m_stream->ReadVector2(&coords);
				sprite.textureCoords.push_back(coords);
			}

			sequence.sprites.push_back(sprite);

			return true;
		}
		else
		{
			return false;
		}*/

		return false;
		}, 0);

	m_level.spriteSequences.push_back(sequence);

	return true;
}

bool  LevelLoader::readItem()
{
	TrItem item;

	m_stream->ReadString(&item.name);
	m_stream->ReadVector3(&item.position);
	m_stream->ReadQuaternion(&item.rotation);
	m_stream->ReadFloat(&item.angle);
	m_stream->ReadVector3(&item.scale);
	m_stream->ReadVector3(&item.color);
	m_stream->ReadInt32(&item.roomNumber);
	m_stream->ReadInt32(&item.objectNumber);
	m_stream->ReadString(&item.script);

	m_level.items.push_back(item);

	return true;
}

bool LevelLoader::readAiItem()
{
	TrItem item;

	m_stream->ReadString(&item.name);
	m_stream->ReadVector3(&item.position);
	m_stream->ReadQuaternion(&item.rotation);
	m_stream->ReadFloat(&item.angle);
	m_stream->ReadVector3(&item.scale);
	m_stream->ReadVector3(&item.color);
	m_stream->ReadInt32(&item.roomNumber);
	m_stream->ReadInt32(&item.objectNumber);
	m_stream->ReadString(&item.script);

	m_level.aiItems.push_back(item);

	return true;
}

bool  LevelLoader::readSink()
{
	TrSink sink;

	m_stream->ReadString(&sink.name);
	m_stream->ReadVector3(&sink.position);
	m_stream->ReadInt32(&sink.roomNumber);
	m_stream->ReadFloat(&sink.strength);
	m_stream->ReadInt32(&sink.box);

	m_reader->ReadChunks([this, sink](ChunkId * id, long size, int arg) { return false; }, 0);

	m_level.sinks.push_back(sink);

	return true;
}

bool  LevelLoader::readCamera()
{
	TrCamera camera;

	m_stream->ReadString(&camera.name);
	m_stream->ReadVector3(&camera.position);
	m_stream->ReadInt32(&camera.roomNumber);
	m_stream->ReadInt32(&camera.type);
	m_stream->ReadInt32(&camera.flags);
	m_stream->ReadString(&camera.script);

	m_reader->ReadChunks([this, camera](ChunkId * id, long size, int arg) { return false; }, 0);

	m_level.cameras.push_back(camera);

	return true;
}

bool  LevelLoader::readFlybyCamera()
{
	TrFlybyCamera camera;

	m_stream->ReadString(&camera.name);
	m_stream->ReadInt32(&camera.sequence);
	m_stream->ReadInt32(&camera.number);
	m_stream->ReadVector3(&camera.position);
	m_stream->ReadVector3(&camera.direction);
	m_stream->ReadFloat(&camera.fov);
	m_stream->ReadFloat(&camera.roll);
	m_stream->ReadFloat(&camera.speed);
	m_stream->ReadInt32(&camera.timer);
	m_stream->ReadInt32(&camera.roomNumber);
	m_stream->ReadInt32(&camera.flags);
	m_stream->ReadString(&camera.script);

	m_reader->ReadChunks([this, camera](ChunkId * id, long size, int arg) { return false; }, 0);

	m_level.flybyCameras.push_back(camera);

	return true;
}

bool  LevelLoader::readSoundSource()
{
	TrSoundSource soundSource;

	m_stream->ReadString(&soundSource.name);
	m_stream->ReadVector3(&soundSource.position);
	m_stream->ReadInt32(&soundSource.roomNumber);
	m_stream->ReadFloat(&soundSource.volume);
	m_stream->ReadInt32(&soundSource.sound);

	m_reader->ReadChunks([this, soundSource](ChunkId * id, long size, int arg) { return false; }, 0);

	m_level.soundSources.push_back(soundSource);

	return true;
}

bool  LevelLoader::readBox()
{
	TrBox box;

	m_stream->ReadVector2(&box.min);
	m_stream->ReadVector2(&box.max);
	m_stream->ReadInt32(&box.floor);
	m_stream->ReadInt32(&box.overlapsIndex);
	m_stream->ReadInt32(&box.overlapsCount);

	m_reader->ReadChunks([this, &box](ChunkId * id, long size, int arg) {
		if (id->EqualsTo(m_chunkOverlap))
		{
			TrOverlap overlap;

			m_stream->ReadInt32(&overlap.box);
			m_stream->ReadInt32(&overlap.flags);

			box.overlaps.push_back(overlap);

			return true;
		}
		else
		{
			return false;
		}
		}, 0);

	m_level.boxes.push_back(box);

	return true;
}

bool  LevelLoader::readZones()
{
	for (int a = 0; a < 2; a++)
		for (int z = 0; z < 5; z++)
			for (int b = 0; b < m_level.boxes.size(); b++)
			{
				int zone;
				m_stream->ReadInt32(&zone);
				m_level.zones[z][a].push_back(zone);
			}

	return true;
}

bool  LevelLoader::readSoundMap()
{
	int count;
	m_stream->ReadInt32(&count);
	for (int i = 0; i < count; i++)
	{
		int sound;
		m_stream->ReadInt32(&sound);
		m_level.soundMap.push_back(sound);
	}

	return true;
}

bool  LevelLoader::readSoundDetail()
{
	TrSoundDetails detail;

	m_stream->ReadFloat(&detail.volume);
	m_stream->ReadFloat(&detail.range);
	m_stream->ReadFloat(&detail.chance);
	m_stream->ReadFloat(&detail.pitch);
	m_stream->ReadBool(&detail.randomizeGain);
	m_stream->ReadBool(&detail.randomizePitch);
	m_stream->ReadBool(&detail.noPanoramic);
	m_stream->ReadByte(&detail.loop);

	m_reader->ReadChunks([this, &detail](ChunkId * id, long size, int arg) {
		/*if (id->EqualsTo(m_chunkSample))
		{
			TrSample sample;

			m_stream->ReadInt32(&sample.uncompressedSize);
			m_stream->ReadInt32(&sample.compressedSize);
			sample.data = (byte*)malloc(sample.compressedSize);
			m_stream->ReadBytes(sample.data, sample.compressedSize);

			detail.samples.push_back(sample);

			return true;
		}
		else
		{
			return false;
		}*/
		return false;
		}, 0); 

	m_level.soundDetails.push_back(detail);

	return true;
}

bool LevelLoader::readSample()
{
	TrSample sample;

	m_stream->ReadInt32(&sample.uncompressedSize);
	m_stream->ReadInt32(&sample.compressedSize);
	sample.data = (byte*)malloc(sample.compressedSize);
	m_stream->ReadBytes(sample.data, sample.compressedSize);

	Sound_LoadSample((char*)sample.data, sample.compressedSize, sample.uncompressedSize, m_numSamples);
	m_numSamples++;

	return true;
}

bool LevelLoader::readOverlap()
{
	TrOverlap overlap;

	m_stream->ReadInt32(&overlap.box);
	m_stream->ReadInt32(&overlap.flags);

	m_reader->ReadChunks([this, &overlap](ChunkId * id, long size, int arg) { return false; }, 0);

	m_level.overlaps.push_back(overlap);

	return true;
}
