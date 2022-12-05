#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "RenderView/RenderView.h"

using namespace TEN::Math;

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using TEN::Memory::LinearArrayBuffer;
	using std::vector;

	void Renderer11::CollectRooms(RenderView &renderView, bool onlyRooms)
	{
		short baseRoomIndex = renderView.camera.RoomNumber;

		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{
			m_rooms[i].ItemsToDraw.clear();
			m_rooms[i].EffectsToDraw.clear();
			m_rooms[i].TransparentFacesToDraw.clear();
			m_rooms[i].StaticsToDraw.clear();
			m_rooms[i].LightsToDraw.clear();
			m_rooms[i].Visited = false;
			m_rooms[i].ViewPort = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
			m_rooms[i].BoundActive = 0;

			for (int j = 0; j < m_rooms[i].Doors.size(); j++)
			{
				m_rooms[i].Doors[j].Visited = false;
			}
		}

		GetVisibleRooms(NO_ROOM, Camera.pos.RoomNumber, Vector4(-1.0f, -1.0f, 1.0f, 1.0f), false, 0, onlyRooms, renderView);
	}

	bool Renderer11::CheckPortal(short parentRoomNumber, RendererDoor* door, Vector4 viewPort, Vector4* clipPort, RenderView& renderView)
	{
		m_numCheckPortalCalls++;

		RendererRoom* room = &m_rooms[parentRoomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[parentRoomNumber];

		if (!door->Visited)
		{
			door->CameraToDoor = Vector3(
				Camera.pos.x - (door->AbsoluteVertices[0].x),
				Camera.pos.y - (door->AbsoluteVertices[0].y),
				Camera.pos.z - (door->AbsoluteVertices[0].z));
		}

		if (door->Normal.x * door->CameraToDoor.x +
			door->Normal.y * door->CameraToDoor.y +
			door->Normal.z * door->CameraToDoor.z < 0)
		{
			return false;
		}

		int  zClip = 0;
		Vector4 p[4];

		*clipPort = Vector4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 4; i++)
		{
			if (!door->Visited)
			{
				p[i] = Vector4::Transform(door->AbsoluteVertices[i], renderView.camera.ViewProjection);
				if (p[i].w > 0.0f)
				{
					p[i].x *= (1.0f / p[i].w);
					p[i].y *= (1.0f / p[i].w);

				}
				door->TransformedVertices[i] = p[i];
			}
			else
			{
				p[i] = door->TransformedVertices[i];
			}

			if (p[i].w > 0.0f)
			{
				clipPort->x = std::min(clipPort->x, p[i].x);
				clipPort->y = std::min(clipPort->y, p[i].y);
				clipPort->z = std::max(clipPort->z, p[i].x);
				clipPort->w = std::max(clipPort->w, p[i].y);
			}
			else
			{
				zClip++;
			}
		}

		door->Visited = true;

		if (zClip == 4)
			return false;

		if (zClip > 0) {
			for (int i = 0; i < 4; i++) {
				Vector4 a = p[i];
				Vector4 b = p[(i + 1) % 4];

				if ((a.w > 0.0f) ^ (b.w > 0.0f)) {

					if (a.x < 0.0f && b.x < 0.0f)
						clipPort->x = -1.0f;
					else
						if (a.x > 0.0f && b.x > 0.0f)
							clipPort->z = 1.0f;
						else {
							clipPort->x = -1.0f;
							clipPort->z = 1.0f;
						}

					if (a.y < 0.0f && b.y < 0.0f)
						clipPort->y = -1.0f;
					else
						if (a.y > 0.0f && b.y > 0.0f)
							clipPort->w = 1.0f;
						else {
							clipPort->y = -1.0f;
							clipPort->w = 1.0f;
						}

				}
			}
		}

		if (clipPort->x > viewPort.z || clipPort->y > viewPort.w || clipPort->z < viewPort.x || clipPort->w < viewPort.y)
			return false;

		clipPort->x = std::max(clipPort->x, viewPort.x);
		clipPort->y = std::max(clipPort->y, viewPort.y);
		clipPort->z = std::min(clipPort->z, viewPort.z);
		clipPort->w = std::min(clipPort->w, viewPort.w);

		return true;
	}

	void Renderer11::GetVisibleRooms(short from, short to, Vector4 viewPort, bool water, int count, bool onlyRooms, RenderView& renderView)
	{
		m_numGetVisibleRoomsCalls++;

		if (count > 32)
		{
			//return;
		}

		RendererRoom* room = &m_rooms[to];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[to];

		auto cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		float xRad = nativeRoom->xSize * SECTOR(1) / 2.0f;
		float yRad = (nativeRoom->minfloor - nativeRoom->maxceiling) / 2.0f;
		float zRad = nativeRoom->zSize * SECTOR(1) / 2.0f;

		auto roomCentre = Vector3(nativeRoom->x + xRad, nativeRoom->minfloor - yRad, nativeRoom->z + zRad);

		float roomRad = std::max(std::max(xRad, yRad), zRad);
		float distance = std::max((roomCentre - cameraPosition).Length() - (roomRad * 1.5f), 0.0f);

		if (!m_rooms[to].Visited)
		{
			renderView.roomsToDraw.push_back(room);

			CollectLightsForRoom(to, renderView);

			if (!onlyRooms)
			{
				CollectItems(to, renderView);
				CollectStatics(to, renderView);
				CollectEffects(to);
			}
		}

		room->Distance = distance;
		room->Visited = true;
		room->ViewPort.x = std::min(room->ViewPort.x, viewPort.x);
		room->ViewPort.y = std::min(room->ViewPort.y, viewPort.y);
		room->ViewPort.z = std::max(room->ViewPort.z, viewPort.z);
		room->ViewPort.w = std::max(room->ViewPort.w, viewPort.w);

		Vector4 clipPort;
		for (int i = 0; i < room->Doors.size(); i++)
		{
			RendererDoor* door = &room->Doors[i];

			if (from != door->RoomNumber && CheckPortal(to, door, viewPort, &clipPort, renderView))
				GetVisibleRooms(to, door->RoomNumber, clipPort, water, count + 1, onlyRooms, renderView);
		}
	}

	void Renderer11::CollectItems(short roomNumber, RenderView& renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = g_Level.Items[itemNum].NextItem)
		{
			ItemInfo* item = &g_Level.Items[itemNum];

			if (item->ObjectNumber == ID_LARA && itemNum == g_Level.Items[itemNum].NextItem)
				break;

			if (item->Status == ITEM_INVISIBLE)
				continue;

			if (item->ObjectNumber == ID_LARA && (BinocularRange || SpotcamOverlay || SpotcamDontDrawLara || CurrentLevel == 0))
				continue;

			if (!m_moveableObjects[item->ObjectNumber].has_value())
				continue;

			auto& obj = m_moveableObjects[item->ObjectNumber].value();

			if (obj.DoNotDraw)
				continue;

			// Clip object by frustum only if it doesn't cast shadows. Otherwise we may see
			// disappearing shadows if object gets out of frustum.

			if (obj.ShadowType == ShadowMode::None)
			{
				// Get all spheres and check if frustum intersects any of them.
				static BoundingSphere spheres[MAX_BONES];
				int cnt = GetSpheres(itemNum, spheres, SPHERES_SPACE_WORLD, Matrix::Identity);

				bool inFrustum = false;
				for (int i = 0; !inFrustum, i < cnt; i++)
					// Blow up sphere radius by half for cases of too small calculated spheres.
					if (renderView.camera.Frustum.SphereInFrustum(spheres[i].Center, spheres[i].Radius * 1.5f))
						inFrustum = true;
				
				if (!inFrustum)
					continue;
			}

			auto newItem = &m_items[itemNum];

			newItem->ItemNumber = itemNum;
			newItem->ObjectNumber = item->ObjectNumber;
			newItem->Color = item->Color;
			newItem->Position = item->Pose.Position.ToVector3();
			newItem->Translation = Matrix::CreateTranslation(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
			newItem->Rotation = item->Pose.Orientation.ToRotationMatrix();
			newItem->Scale = Matrix::CreateScale(1.0f);
			newItem->World = newItem->Rotation * newItem->Translation;

			CalculateLightFades(newItem);
			CollectLightsForItem(newItem);

			room.ItemsToDraw.push_back(newItem);
		}
	}

	void Renderer11::CollectStatics(short roomNumber, RenderView& renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		if (r->mesh.size() == 0)
			return;

		int numStatics = r->mesh.size();
		for (int i = 0; i < numStatics; i++)
		{
			auto* mesh = &r->mesh[i];
			
			if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
				continue;

			if (!m_staticObjects[mesh->staticNumber].has_value())
				continue;
			
			auto& obj = *m_staticObjects[mesh->staticNumber];

			if (obj.ObjectMeshes.size() == 0)
				continue;

			const auto& bounds = GetBoundsAccurate(*mesh, true).ToBoundingOrientedBox(mesh->pos);
			auto length = Vector3(bounds.Extents).Length();
			if (!renderView.camera.Frustum.SphereInFrustum(bounds.Center, length))
				continue;

			std::vector<RendererLight*> lights;
			if (obj.ObjectMeshes.front()->LightMode != LIGHT_MODES::LIGHT_MODE_STATIC)
				CollectLights(mesh->pos.Position.ToVector3(), ITEM_LIGHT_COLLECTION_RADIUS, room.RoomNumber, NO_ROOM, false, lights);

			Matrix world = (mesh->pos.Orientation.ToRotationMatrix() *
							Matrix::CreateScale(mesh->scale) *
							Matrix::CreateTranslation(mesh->pos.Position.x, mesh->pos.Position.y, mesh->pos.Position.z));

			auto staticInfo = RendererStatic
			{
				mesh->staticNumber,
				room.RoomNumber,
				mesh->pos.Position.ToVector3(),
				world,
				mesh->color,
				room.AmbientLight,
				lights
			};

			room.StaticsToDraw.push_back(staticInfo);
		}
	}

	void Renderer11::CollectLights(Vector3 position, float radius, int roomNumber, int prevRoomNumber, bool prioritizeShadowLight, std::vector<RendererLight*>& lights)
	{
		if (m_rooms.size() < roomNumber)
			return;

		// Now collect lights from dynamic list and from rooms
		std::vector<RendererLight*> tempLights;
		tempLights.reserve(MAX_LIGHTS_DRAW);
		
		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[room.RoomNumber];

		RendererLight* brightestLight = nullptr;
		float brightest = 0.0f;

		// Dynamic lights have the priority
		for (auto& light : dynamicLights)
		{
			float distance = (position - light.Position).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= SECTOR(20))
				continue;

			// Check the out radius
			if (distance > light.Out + radius)
				continue;

			float attenuation = 1.0f - distance / light.Out;
			float intensity = std::max(0.0f, attenuation * light.Intensity * Luma(light.Color));

			light.LocalIntensity = intensity;
			light.Distance = distance;

			tempLights.push_back(&light);
		}

		// Check current room and also neighbour rooms
		for (int roomToCheck : room.Neighbors)
		{
			RendererRoom& currentRoom = m_rooms[roomToCheck];
			int numLights = currentRoom.Lights.size();

			for (int j = 0; j < numLights; j++)
			{
				RendererLight* light = &currentRoom.Lights[j];

				// Check only lights different from sun
				if (light->Type == LIGHT_TYPE_SUN)
				{
					// Suns from non-adjacent rooms are not added!
					if (roomToCheck != roomNumber && (prevRoomNumber != roomToCheck || prevRoomNumber == NO_ROOM))
						continue;

					// Sun is added without distance checks
					light->Distance = 0;
					light->LocalIntensity = 0;
				}
				else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
				{
					Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);
					float distance = (position - lightPosition).Length();

					// Collect only lights nearer than 20 sectors
					if (distance >= SECTOR(20))
						continue;

					// Check the out radius
					if (distance > light->Out + radius)
						continue;

					float attenuation = 1.0f - distance / light->Out;
					float intensity = std::max(0.0f, attenuation * light->Intensity * Luma(light->Color));

					light->LocalIntensity = intensity;
					light->Distance = distance;

					// If collecting shadows, try to collect shadow casting light
					if (light->CastShadows && prioritizeShadowLight && light->Type == LIGHT_TYPE_POINT)
					{
						if (intensity >= brightest)
						{
							brightest = intensity;
							brightestLight = light;
						}
					}
				}
				else if (light->Type == LIGHT_TYPE_SPOT)
				{
					Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);
					float distance = (position - lightPosition).Length();

					// Collect only lights nearer than 20 sectors
					if (distance >= SECTOR(20))
						continue;

					// Check the range
					if (distance > light->Out + radius)
						continue;

					float attenuation = 1.0f - distance / light->Out;
					float intensity = std::max(0.0f, attenuation * light->Intensity * Luma(light->Color));

					light->LocalIntensity = intensity;

					// If shadow pointer provided, try to collect shadow casting light
					if (light->CastShadows && prioritizeShadowLight)
					{
						if (intensity >= brightest)
						{
							brightest = intensity;
							brightestLight = light;
						}
					}

					light->Distance = distance;
				}
				else
				{
					// Invalid light type
					continue;
				}

				tempLights.push_back(light);
			}
		}

		// Sort lights by distance, if needed

		if (tempLights.size() > MAX_LIGHTS_PER_ITEM)
		{
			std::sort(
				tempLights.begin(),
				tempLights.end(),
				[](RendererLight* a, RendererLight* b)
				{
					return a->Distance < b->Distance;
				}
			);
		}

		// Now put actual lights to provided vector
		lights.clear();

		// Always add brightest light, if collecting shadow light is specified, even if it's far in range
		if (prioritizeShadowLight && brightestLight)
			lights.push_back(brightestLight);

		// Add max 8 lights per item, including the shadow light for Lara eventually
		for (auto l : tempLights)
		{
			if (prioritizeShadowLight && brightestLight == l)
				continue;

			lights.push_back(l);

			if (lights.size() == MAX_LIGHTS_PER_ITEM)
				break;
		}
	}

	void Renderer11::CollectLightsForCamera()
	{
		std::vector<RendererLight*> lightsToDraw;
		CollectLights(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), CAMERA_LIGHT_COLLECTION_RADIUS, Camera.pos.RoomNumber, NO_ROOM, true, lightsToDraw);

		if (lightsToDraw.size() > 0 && lightsToDraw.front()->CastShadows)
			shadowLight = lightsToDraw.front();
		else
			shadowLight = nullptr;
	}	
	
	void Renderer11::CollectLightsForEffect(short roomNumber, RendererEffect* effect)
	{
		CollectLights(effect->Position, ITEM_LIGHT_COLLECTION_RADIUS, roomNumber, NO_ROOM, false, effect->LightsToDraw);
	}

	void Renderer11::CollectLightsForItem(RendererItem* item)
	{
		CollectLights(item->Position, ITEM_LIGHT_COLLECTION_RADIUS, item->RoomNumber, item->PrevRoomNumber, false, item->LightsToDraw);
	}

	void Renderer11::CalculateLightFades(RendererItem *item)
	{
		ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];

		// Interpolate ambient light between rooms
		if (item->PrevRoomNumber == NO_ROOM)
		{
			item->PrevRoomNumber = nativeItem->RoomNumber;
			item->RoomNumber = nativeItem->RoomNumber;
			item->LightFade = 1.0f;
		}
		else if (nativeItem->RoomNumber != item->RoomNumber)
		{
			item->PrevRoomNumber = item->RoomNumber;
			item->RoomNumber = nativeItem->RoomNumber;
			item->LightFade = 0.0f;
		}
		else if (item->LightFade < 1.0f)
		{
			item->LightFade += AMBIENT_LIGHT_INTERPOLATION_STEP;
			item->LightFade = std::clamp(item->LightFade, 0.0f, 1.0f);
		}

		if (item->PrevRoomNumber == NO_ROOM || item->LightFade == 1.0f)
			item->AmbientLight = m_rooms[nativeItem->RoomNumber].AmbientLight;
		else
		{
			auto prev = m_rooms[item->PrevRoomNumber].AmbientLight;
			auto next = m_rooms[item->RoomNumber].AmbientLight;

			item->AmbientLight.x = Lerp(prev.x, next.x, item->LightFade);
			item->AmbientLight.y = Lerp(prev.y, next.y, item->LightFade);
			item->AmbientLight.z = Lerp(prev.z, next.z, item->LightFade);
		}

		// Multiply calculated ambient light by object tint
		item->AmbientLight *= nativeItem->Color;
	}

	void Renderer11::CollectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;
		
		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];
		
		Vector3 boxMin = Vector3(r->x + WALL_SIZE, r->maxceiling - STEP_SIZE, r->z + WALL_SIZE);
		Vector3 boxMax = Vector3(r->x + (r->xSize - 1) * WALL_SIZE, r->minfloor + STEP_SIZE, r->z + (r->zSize - 1) * WALL_SIZE);

		// Collect dynamic lights for rooms
		for (int i = 0; i < dynamicLights.size(); i++)
		{
			RendererLight* light = &dynamicLights[i];

			Vector3 center = Vector3(light->Position.x, light->Position.y, light->Position.z);

			// Light buffer is full
			if (renderView.lightsToDraw.size() >= NUM_LIGHTS_PER_BUFFER)
				continue;

			// Light already on a list
			if (std::find(renderView.lightsToDraw.begin(), renderView.lightsToDraw.end(), light) != renderView.lightsToDraw.end())
				continue;

			// Light is not within room bounds
			if (!SphereBoxIntersection(boxMin, boxMax, center, light->Out))
				continue;

			renderView.lightsToDraw.push_back(light);
			room.LightsToDraw.push_back(light);
		}
	}

	void Renderer11::CollectEffects(short roomNumber)
	{
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short fxNum = NO_ITEM;
		for (fxNum = r->fxNumber; fxNum != NO_ITEM; fxNum = EffectList[fxNum].nextFx)
		{
			FX_INFO *fx = &EffectList[fxNum];

			if (fx->objectNumber < 0)
				continue;

			ObjectInfo *obj = &Objects[fx->objectNumber];

			RendererEffect *newEffect = &m_effects[fxNum];

			Matrix translation = Matrix::CreateTranslation(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z);
			Matrix rotation = fx->pos.Orientation.ToRotationMatrix();

			newEffect->ObjectNumber = fx->objectNumber;
			newEffect->RoomNumber = fx->roomNumber;
			newEffect->Position = fx->pos.Position.ToVector3();
			newEffect->AmbientLight = room.AmbientLight;
			newEffect->Color = fx->color;
			newEffect->World = rotation * translation;
			newEffect->Mesh = GetMesh(obj->nmeshes ? obj->meshIndex : fx->frameNumber);

			CollectLightsForEffect(fx->roomNumber, newEffect);

			room.EffectsToDraw.push_back(newEffect);
		}
	}

	void Renderer11::ResetAnimations()
	{
		for (int i = 0; i < NUM_ITEMS; i++)
			m_items[i].DoneAnimations = false;
	}
} // namespace TEN::Renderer

