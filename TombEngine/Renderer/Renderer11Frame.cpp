#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/spotcam.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "RenderView/RenderView.h"

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
			m_rooms[i].Clip = RendererRectangle(m_screenWidth, m_screenHeight, 0, 0);
			m_rooms[i].ClipTest = RendererRectangle(m_screenWidth, m_screenHeight, 0, 0);
			m_rooms[i].BoundActive = 0;
		}

		GetVisibleObjects(renderView, onlyRooms);
	}

	void Renderer11::SetRoomBounds(ROOM_DOOR* door, short parentRoomNumber, RenderView& renderView)
	{
		RendererRoom* room = &m_rooms[door->room];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[door->room];

		RendererRoom* parentRoom = &m_rooms[parentRoomNumber];
		ROOM_INFO* parentNativeRoom = &g_Level.Rooms[parentRoomNumber];

		// If parent's bounds are bigger than current bounds test, then don't do anything else
		if (room->Clip.left <= parentRoom->ClipTest.left
			&& room->Clip.right >= parentRoom->ClipTest.right
			&& room->Clip.top <= parentRoom->ClipTest.top
			&& room->Clip.bottom >= parentRoom->ClipTest.bottom)
		{
			return;
		}

		int left = parentRoom->ClipTest.right;
		int right = parentRoom->ClipTest.left;
		int top = parentRoom->ClipTest.bottom;
		int bottom = parentRoom->ClipTest.top;

		int zBehind = 0;
		int zTooFar = 0;

		Vector4 p[4];

		int xs = 0;
		int ys = 0;

		for (int i = 0; i < 4; i++)
		{
			// Project vertices of the door in clip space
			Vector4 tmp = Vector4(
				door->vertices[i].x + parentNativeRoom->x,
				door->vertices[i].y,
				door->vertices[i].z + parentNativeRoom->z,
				1.0f);

			Vector4::Transform(tmp, renderView.camera.ViewProjection, p[i]);

			// Convert coordinates to screen space
			if (p[i].w > 0.0f)
			{
				p[i].x *= (1.0f / p[i].w);
				p[i].y *= (1.0f / p[i].w);
				p[i].z *= (1.0f / p[i].w);

				if (p[i].w > 0)
				{
					xs = 0.5f * (p[i].x + 1.0f) * m_screenWidth;
					ys = 0.5f * (p[i].y + 1.0f) * m_screenHeight;
				}
				else
				{
					xs = (p[i].x >= 0) ? 0 : m_screenWidth;
					ys = (p[i].y >= 0) ? m_screenHeight : 0;
				}

				// Has bound changed?
				if (xs - 1 < left)
					left = xs - 1;
				if (xs + 1 > right)
					right = xs + 1;

				if (ys - 1 < top)
					top = ys - 1;
				if (ys + 1 > bottom)
					bottom = ys + 1;
			}
			else
			{
				zBehind++;
			}
		}

		// If all vertices of the door ar behind the camera, exit now
		if (zBehind == 4)
			return;

		// If some vertices are behind the camera, we need to properly clip
		if (zBehind > 0)
		{
			for (int i = 0; i < 4; i++)
			{
				Vector4 a = p[i];
				Vector4 b = p[(i + 1) % 4];

				if ((a.z <= 0) ^ (b.z <= 0))
				{
					// X clip
					if (a.x < 0 && b.x < 0)
						left = 0;
					else if (a.x > 0 && b.x > 0)
						right = m_screenWidth;
					else
					{
						left = 0;
						right = m_screenWidth;
					}

					// Y clip
					if (a.y < 0 && b.y < 0)
						top = 0;
					else if (a.y > 0 && b.y > 0)
						bottom = m_screenHeight;
					else
					{
						top = 0;
						bottom = m_screenHeight;
					}
				}
			}
		}

		// Clip bounds to parent bounds
		if (left < parentRoom->ClipTest.left)
			left = parentRoom->ClipTest.left;
		if (right > parentRoom->ClipTest.right)
			right = parentRoom->ClipTest.right;
		if (top < parentRoom->ClipTest.top)
			top = parentRoom->ClipTest.top;
		if (bottom > parentRoom->ClipTest.bottom)
			bottom = parentRoom->ClipTest.bottom;

		if (left >= right || top >= bottom)
			return;

		// Store the new calculated bounds
		if (room->BoundActive & 2)
		{
			// The room is already in the bounds list

			if (left < room->ClipTest.left)
			{
				room->Clip.left = left;
				room->ClipTest.left = left;
			}

			if (top < room->ClipTest.top)
			{
				room->Clip.top = top;
				room->ClipTest.top = top;
			}

			if (right > room->ClipTest.right)
			{
				room->Clip.right = right;
				room->ClipTest.right = right;
			}

			if (bottom > room->ClipTest.bottom)
			{
				room->Clip.bottom = bottom;
				room->ClipTest.bottom = bottom;
			}
		}
		else if (!m_rooms[door->room].Visited)
		{
			// The room must be added to the bounds list

			m_boundList[(m_boundEnd++) % MAX_ROOM_BOUNDS] = door->room;
			room->BoundActive |= 2;

			room->ClipTest.left = left;
			room->ClipTest.right = right;
			room->ClipTest.top = top;
			room->ClipTest.bottom = bottom;

			room->Clip.left = left;
			room->Clip.right = right;
			room->Clip.top = top;
			room->Clip.bottom = bottom;
		}
	}

	void Renderer11::GetRoomBounds(RenderView& renderView, bool onlyRooms)
	{
		auto cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		while (m_boundStart != m_boundEnd)
		{
			short roomNumber = m_boundList[(m_boundStart++) % MAX_ROOM_BOUNDS];
			RendererRoom* room = &m_rooms[roomNumber];
			ROOM_INFO* nativeRoom = &g_Level.Rooms[roomNumber];

			float xRad = nativeRoom->xSize * SECTOR(1) / 2.0f;
			float yRad = (nativeRoom->minfloor - nativeRoom->maxceiling) / 2.0f;
			float zRad = nativeRoom->zSize * SECTOR(1) / 2.0f;

			auto roomCentre = Vector3(nativeRoom->x + xRad, nativeRoom->minfloor - yRad, nativeRoom->z + zRad);

			float roomRad = std::max(std::max(xRad, yRad), zRad);
			float distance = std::max((roomCentre - cameraPosition).Length() - (roomRad * 1.5f), 0.0f);

			room->BoundActive -= 2;

			if (room->ClipTest.left < room->Clip.left)
				room->Clip.left = room->ClipTest.left;
			if (room->ClipTest.top < room->Clip.top)
				room->Clip.top = room->ClipTest.top;
			if (room->ClipTest.right > room->Clip.right)
				room->Clip.right = room->ClipTest.right;
			if (room->ClipTest.bottom > room->Clip.bottom)
				room->Clip.bottom = room->ClipTest.bottom;

			// Add room to the list of rooms to draw
			if (!((room->BoundActive & 1) || distance > m_farView))
			{
				renderView.roomsToDraw.push_back(room);

				room->BoundActive |= 1;

				if (nativeRoom->flags & ENV_FLAG_OUTSIDE)
					m_outside = true;

				if (!m_rooms[roomNumber].Visited)
				{
					CollectLightsForRoom(roomNumber, renderView);

					if (!onlyRooms)
					{
						CollectItems(roomNumber, renderView);
						CollectStatics(roomNumber, renderView);
						CollectEffects(roomNumber);
					}
				}

				m_rooms[roomNumber].Distance = distance;
				m_rooms[roomNumber].Visited = true;
			}

			if (nativeRoom->flags & ENV_FLAG_OUTSIDE)
			{
				if (room->Clip.left < m_outsideClip.left)
					m_outsideClip.left = room->Clip.left;
				if (room->Clip.right > m_outsideClip.right)
					m_outsideClip.right = room->Clip.right;
				if (room->Clip.top < m_outsideClip.top)
					m_outsideClip.top = room->Clip.top;
				if (room->Clip.bottom > m_outsideClip.bottom)
					m_outsideClip.bottom = room->Clip.bottom;
			}

			for (int i = 0; i < nativeRoom->doors.size(); i++)
			{
				ROOM_DOOR* portal = &nativeRoom->doors[i];

				Vector3Int n = Vector3Int(portal->normal.x, portal->normal.y, portal->normal.z);
				Vector3Int v = Vector3Int(
					Camera.pos.x - (nativeRoom->x + portal->vertices[0].x),
					Camera.pos.y - (nativeRoom->y + portal->vertices[0].y),
					Camera.pos.z - (nativeRoom->z + portal->vertices[0].z));

				if (n.x * v.x + n.y * v.y + n.z * v.z < 0)
					continue;

				SetRoomBounds(portal, roomNumber, renderView);
			}
		}
	}

	void Renderer11::GetVisibleObjects(RenderView& renderView, bool onlyRooms)
	{
		RendererRoom* room = &m_rooms[Camera.pos.roomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[Camera.pos.roomNumber];

		room->ClipTest = RendererRectangle(0, 0, m_screenWidth, m_screenHeight);
		m_outside = nativeRoom->flags & ENV_FLAG_OUTSIDE;
		m_cameraUnderwater = (nativeRoom->flags & ENV_FLAG_WATER);

		room->BoundActive = 2;

		// Initialise bounds list
		m_boundList[0] = Camera.pos.roomNumber;
		m_boundStart = 0;
		m_boundEnd = 1;

		// Horizon clipping
		if (m_outside)
		{
			m_outsideClip = RendererRectangle(0, 0, m_screenWidth, m_screenHeight);
		}
		else
		{
			m_outsideClip = RendererRectangle(m_screenWidth, m_screenHeight, 0, 0);
		}

		// Get all rooms and objects to draw
		GetRoomBounds(renderView, onlyRooms);
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
					if (renderView.camera.frustum.SphereInFrustum(spheres[i].Center, spheres[i].Radius * 1.5f))
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
			newItem->Rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Pose.Orientation.y),
															   TO_RAD(item->Pose.Orientation.x),
															   TO_RAD(item->Pose.Orientation.z));
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
			auto mesh = &r->mesh[i];
			
			if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
				continue;

			if (!m_staticObjects[mesh->staticNumber].has_value())
				continue;
			
			auto& obj = *m_staticObjects[mesh->staticNumber];

			if (obj.ObjectMeshes.size() == 0)
				continue;

			auto bounds = TO_DX_BBOX(mesh->pos, GetBoundsAccurate(mesh, true));
			auto length = Vector3(bounds.Extents).Length();
			if (!renderView.camera.frustum.SphereInFrustum(bounds.Center, length))
				continue;

			std::vector<RendererLight*> lights;
			if (obj.ObjectMeshes.front()->LightMode != LIGHT_MODES::LIGHT_MODE_STATIC)
				CollectLights(mesh->pos.Position.ToVector3(), ITEM_LIGHT_COLLECTION_RADIUS, room.RoomNumber, NO_ROOM, false, lights);

			Matrix world = (Matrix::CreateFromYawPitchRoll(TO_RAD(mesh->pos.Orientation.y), TO_RAD(mesh->pos.Orientation.x), TO_RAD(mesh->pos.Orientation.z)) *
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
		CollectLights(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), CAMERA_LIGHT_COLLECTION_RADIUS, Camera.pos.roomNumber, NO_ROOM, true, lightsToDraw);

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
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(fx->pos.Orientation.y), TO_RAD(fx->pos.Orientation.x), TO_RAD(fx->pos.Orientation.z));

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

