#pragma once
#include "Renderer11.h"
#include "../Specific/configuration.h"
#include <chrono>
#include "../Game/savegame.h"
#include "../Game/healt.h"
#include "../Game/camera.h"
#include "../Game/draw.h"
#include "../Game/inventory.h"
#include "../Game/lara.h"
#include "../Game/gameflow.h"
#include "../Game/rope.h"
#include "../Game/tomb4fx.h"
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_SFXVolumeBar;
extern RendererHUDBar* g_MusicVolumeBar;
int Renderer11::DrawPickup(short objectNum)
{
	drawObjectOn2DPosition(700 + PickupX, 450, objectNum, 0, m_pickupRotation, 0); // TODO: + PickupY
	m_pickupRotation += 45 * 360 / 30;
	return 0;
}

bool Renderer11::drawObjectOn2DPosition(short x, short y, short objectNum, short rotX, short rotY, short rotZ)
{
	Matrix translation;
	Matrix rotation;
	Matrix world;
	Matrix view;
	Matrix projection;
	Matrix scale;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	x *= (ScreenWidth / 800.0f);
	y *= (ScreenHeight / 600.0f);

	view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 2048.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
	projection = Matrix::CreateOrthographic(ScreenWidth, ScreenHeight, -1024.0f, 1024.0f);

	OBJECT_INFO * obj = &Objects[objectNum];
	RendererObject * moveableObj = m_moveableObjects[objectNum];

	if (obj->animIndex != -1)
	{
		updateAnimation(NULL, moveableObj, &Anims[obj->animIndex].framePtr, 0, 0, 0xFFFFFFFF);
	}

	Vector3 pos = m_viewportToolkit->Unproject(Vector3(x, y, 1), projection, view, Matrix::Identity);

	// Clear just the Z-buffer so we can start drawing on top of the scene
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set vertex buffer
	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsInventory, NULL, 0);
	m_context->PSSetShader(m_psInventory, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set matrices
	m_stCameraMatrices.View = view.Transpose();
	m_stCameraMatrices.Projection = projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	for (int n = 0; n < moveableObj->ObjectMeshes.size(); n++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[n];

		// Finish the world matrix
		translation = Matrix::CreateTranslation(pos.x, pos.y, pos.z + 1024.0f);
		rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(rotY), TR_ANGLE_TO_RAD(rotX), TR_ANGLE_TO_RAD(rotZ));
		scale = Matrix::CreateScale(0.5f);

		world = scale * rotation;
		world = world * translation;

		if (obj->animIndex != -1)
			m_stItem.World = (moveableObj->AnimationTransforms[n] * world).Transpose();
		else
			m_stItem.World = (moveableObj->BindPoseTransforms[n].Transpose() * world).Transpose();
		m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
		m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

		for (int m = 0; m < NUM_BUCKETS; m++)
		{
			RendererBucket* bucket = &mesh->Buckets[m];
			if (bucket->NumVertices == 0)
				continue;

			if (m < 2)
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
			else
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

			m_stMisc.AlphaTest = (m < 2);
			updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
			m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
		}
	}

	return true;
}

bool Renderer11::drawShadowMap()
{
	m_shadowLight = NULL;
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;
	Vector3 itemPosition = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);

	for (int k = 0; k < m_roomsToDraw.size(); k++)
	{
		RendererRoom* room = m_roomsToDraw[k];
		int numLights = room->Lights.size();

		for (int j = 0; j < numLights; j++)
		{
			RendererLight* light = &room->Lights[j];

			// Check only lights different from sun
			if (light->Type == LIGHT_TYPE_SUN)
			{
				// Sun is added without checks
			}
			else if (light->Type == LIGHT_TYPE_POINT)
			{
				Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

				float distance = (itemPosition - lightPosition).Length();

				// Collect only lights nearer than 20 sectors
				if (distance >= 20 * WALL_SIZE)
					continue;

				// Check the out radius
				if (distance > light->Out)
					continue;

				float attenuation = 1.0f - distance / light->Out;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
			else if (light->Type == LIGHT_TYPE_SPOT)
			{
				Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

				float distance = (itemPosition - lightPosition).Length();

				// Collect only lights nearer than 20 sectors
				if (distance >= 20 * WALL_SIZE)
					continue;

				// Check the range
				if (distance > light->Range)
					continue;

				// If Lara, try to collect shadow casting light
				float attenuation = 1.0f - distance / light->Range;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
			else
			{
				// Invalid light type
				continue;
			}
		}
	}

	m_shadowLight = brightestLight;

	if (m_shadowLight == NULL)
		return true;

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Bind and clear render target
	m_context->ClearRenderTargetView(m_shadowMap->RenderTargetView, Colors::White);
	m_context->ClearDepthStencilView(m_shadowMap->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_shadowMap->RenderTargetView, m_shadowMap->DepthStencilView);

	m_context->RSSetViewports(1, &m_shadowMapViewport);

	//drawLara(false, true);

	Vector3 lightPos = Vector3(m_shadowLight->Position.x, m_shadowLight->Position.y, m_shadowLight->Position.z);
	Vector3 itemPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	if (lightPos == itemPos)
		return true;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Set shaders
	m_context->VSSetShader(m_vsShadowMap, NULL, 0);
	m_context->PSSetShader(m_psShadowMap, NULL, 0);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState * sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	Matrix view = Matrix::CreateLookAt(lightPos,
		itemPos,
		Vector3(0.0f, -1.0f, 0.0f));
	Matrix projection = Matrix::CreatePerspectiveFieldOfView(90.0f * RADIAN, 1.0f, 64.0f,
		(m_shadowLight->Type == LIGHT_TYPE_POINT ? m_shadowLight->Out : m_shadowLight->Range) * 1.2f);

	m_stCameraMatrices.View = view.Transpose();
	m_stCameraMatrices.Projection = projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stShadowMap.LightViewProjection = (view * projection).Transpose();

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	RendererObject * laraObj = m_moveableObjects[ID_LARA];
	RendererObject * laraSkin = m_moveableObjects[ID_LARA_SKIN];
	RendererRoom & const room = m_rooms[LaraItem->roomNumber];

	m_stItem.World = m_LaraWorldMatrix.Transpose();
	m_stItem.Position = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room.AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj->AnimationTransforms.data(), sizeof(Matrix) * 32);
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
	m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

	for (int k = 0; k < laraSkin->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(Lara.meshPtrs[k])];

		for (int j = 0; j < 2; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	if (m_moveableObjects[ID_LARA_SKIN_JOINTS] != NULL)
	{
		RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];

		for (int k = 0; k < laraSkinJoints->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = laraSkinJoints->ObjectMeshes[k];

			for (int j = 0; j < 2; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	for (int k = 0; k < laraSkin->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = laraSkin->ObjectMeshes[k];

		for (int j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	// Draw items


	// Hairs are pre-transformed
	Matrix matrices[8] = { Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity,
						   Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity };
	memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 8);
	m_stItem.World = Matrix::Identity;
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

	if (m_moveableObjects[ID_LARA_HAIR] != NULL)
	{
		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			(const unsigned short*)m_hairIndices.data(), m_numHairIndices,
			m_hairVertices.data(), m_numHairVertices);
		m_primitiveBatch->End();
	}

	return true;
}

bool Renderer11::DoTitleImage()
{
	Texture2D* texture = Texture2D::LoadFromFile(m_device, (char*)g_GameFlow->Intro);
	if (!texture)
		return false;

	float currentFade = 0;
	while (currentFade <= 1.0f)
	{
		drawFullScreenImage(texture->ShaderResourceView, currentFade);
		SyncRenderer();
		currentFade += FADE_FACTOR;
	}

	for (int i = 0; i < 30 * 1.5f; i++)
	{
		drawFullScreenImage(texture->ShaderResourceView, 1.0f);
		SyncRenderer();
	}

	currentFade = 1.0f;
	while (currentFade >= 0.0f)
	{
		drawFullScreenImage(texture->ShaderResourceView, currentFade);
		SyncRenderer();
		currentFade -= FADE_FACTOR;
	}

	delete texture;

	return true;
}

bool Renderer11::drawGunShells()
{
	RendererRoom& const room = m_rooms[LaraItem->roomNumber];
	RendererItem* item = &m_items[Lara.itemNumber];

	m_stItem.AmbientLight = room.AmbientLight;
	memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

	m_stLights.NumLights = item->Lights.size();
	for (int j = 0; j < item->Lights.size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (int i = 0; i < 24; i++)
	{
		GUNSHELL_STRUCT* gunshell = &Gunshells[i];

		if (gunshell->counter > 0)
		{
			OBJECT_INFO* obj = &Objects[gunshell->objectNumber];
			RendererObject* moveableObj = m_moveableObjects[gunshell->objectNumber];

			Matrix translation = Matrix::CreateTranslation(gunshell->pos.xPos, gunshell->pos.yPos, gunshell->pos.zPos);
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(gunshell->pos.yRot), TR_ANGLE_TO_RAD(gunshell->pos.xRot), TR_ANGLE_TO_RAD(gunshell->pos.zRot));
			Matrix world = rotation * translation;

			m_stItem.World = world.Transpose();
			updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
			m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

			RendererMesh* mesh = moveableObj->ObjectMeshes[0];

			for (int b = 0; b < NUM_BUCKETS; b++)
			{
				RendererBucket* bucket = &mesh->Buckets[b];

				if (bucket->NumVertices == 0)
					continue;

				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	return true;
}


int Renderer11::drawInventoryScene()
{
	char stringBuffer[255];

	bool drawLogo = true;

	RECT guiRect;
	Vector4 guiColor = Vector4(0.0f, 0.0f, 0.25f, 0.5f);
	bool drawGuiRect = false;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	m_lines2DToDraw.clear();
	m_strings.clear();

	m_nextLine2D = 0;

	// Set basic render states
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

	// Bind and clear render target
	m_context->ClearRenderTargetView(m_renderTarget->RenderTargetView, Colors::Black);
	m_context->ClearDepthStencilView(m_renderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_renderTarget->RenderTargetView, m_renderTarget->DepthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	// Clear the Z-Buffer after drawing the background	
	if (g_Inventory->GetType() == INV_TYPE_TITLE)
	{
		if (g_GameFlow->TitleType == TITLE_BACKGROUND)
			drawFullScreenQuad(m_titleScreen->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
		else
			drawFullScreenQuad(m_dumpScreenRenderTarget->ShaderResourceView, Vector3(1.0f, 1.0f, 1.0f), false);
	}
	else
	{
		drawFullScreenQuad(m_dumpScreenRenderTarget->ShaderResourceView, Vector3(0.2f, 0.2f, 0.2f), false);
	}

	m_context->ClearDepthStencilView(m_renderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Set vertex buffer
	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsInventory, NULL, 0);
	m_context->PSSetShader(m_psInventory, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	InventoryRing* activeRing = g_Inventory->GetRing(g_Inventory->GetActiveRing());
	int lastRing = 0;

	float cameraX = INV_CAMERA_DISTANCE * cos(g_Inventory->GetCameraTilt() * RADIAN);
	float cameraY = g_Inventory->GetCameraY() - INV_CAMERA_DISTANCE * sin(g_Inventory->GetCameraTilt() * RADIAN);
	float cameraZ = 0.0f;

	m_stCameraMatrices.View = Matrix::CreateLookAt(Vector3(cameraX, cameraY, cameraZ),
		Vector3(0.0f, g_Inventory->GetCameraY() - 512.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)).Transpose();
	m_stCameraMatrices.Projection = Matrix::CreatePerspectiveFieldOfView(80.0f * RADIAN,
		g_Renderer->ScreenWidth / (float)g_Renderer->ScreenHeight, 1.0f, 200000.0f).Transpose();

	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	for (int k = 0; k < NUM_INVENTORY_RINGS; k++)
	{
		InventoryRing* ring = g_Inventory->GetRing(k);
		if (ring->draw == false || ring->numObjects == 0)
			continue;

		short numObjects = ring->numObjects;
		float deltaAngle = 360.0f / numObjects;
		int objectIndex = 0;
		objectIndex = ring->currentObject;

		// Yellow title
		if (ring->focusState == INV_FOCUS_STATE_NONE && g_Inventory->GetType() != INV_TYPE_TITLE)
			PrintString(400, 20, g_GameFlow->GetString(activeRing->titleStringIndex), PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER);

		for (int i = 0; i < numObjects; i++)
		{
			short inventoryObject = ring->objects[objectIndex].inventoryObject;
			short objectNumber = g_Inventory->GetInventoryObject(ring->objects[objectIndex].inventoryObject)->objectNumber;

			//if (ring->focusState != INV_FOCUS_STATE_NONE && (k != g_Inventory->GetActiveRing() || inventoryObject != ring->objects[i].inventoryObject))
			//	continue;

			// Calculate the inventory object position and rotation
			float currentAngle = 0.0f;
			short steps = -objectIndex + ring->currentObject;
			if (steps < 0) steps += numObjects;
			currentAngle = steps * deltaAngle;
			currentAngle += ring->rotation;

			if (ring->focusState == INV_FOCUS_STATE_NONE && k == g_Inventory->GetActiveRing())
			{
				if (objectIndex == ring->currentObject)
					ring->objects[objectIndex].rotation += 45 * 360 / 30;
				else if (ring->objects[objectIndex].rotation != 0)
					ring->objects[objectIndex].rotation += 45 * 360 / 30;
			}
			else if (ring->focusState != INV_FOCUS_STATE_POPUP && ring->focusState != INV_FOCUS_STATE_POPOVER)
				g_Inventory->GetRing(k)->objects[objectIndex].rotation = 0;

			if (ring->objects[objectIndex].rotation > 65536.0f)
				ring->objects[objectIndex].rotation = 0;

			int x = ring->distance * cos(currentAngle * RADIAN);
			int y = g_Inventory->GetRing(k)->y;
			int z = ring->distance * sin(currentAngle * RADIAN);

			// Prepare the object transform
			Matrix scale = Matrix::CreateScale(ring->objects[objectIndex].scale, ring->objects[objectIndex].scale, ring->objects[objectIndex].scale);
			Matrix translation = Matrix::CreateTranslation(x, y, z);
			Matrix rotation = Matrix::CreateRotationY(TR_ANGLE_TO_RAD(ring->objects[objectIndex].rotation + 16384 + g_Inventory->GetInventoryObject(inventoryObject)->rotY));
			Matrix transform = (scale * rotation) * translation;

			OBJECT_INFO * obj = &Objects[objectNumber];
			RendererObject * moveableObj = m_moveableObjects[objectNumber];
			if (moveableObj == NULL)
				continue;

			// Build the object animation matrices
			if (ring->focusState == INV_FOCUS_STATE_FOCUSED && obj->animIndex != -1 &&
				objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				short* framePtr[2];
				int rate = 0;
				getFrame(obj->animIndex, ring->frameIndex, framePtr, &rate);
				updateAnimation(NULL, moveableObj, framePtr, 0, 1, 0xFFFFFFFF);
			}
			else
			{
				if (obj->animIndex != -1)
					updateAnimation(NULL, moveableObj, &Anims[obj->animIndex].framePtr, 0, 1, 0xFFFFFFFF);
			}

			for (int n = 0; n < moveableObj->ObjectMeshes.size(); n++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[n];

				// HACK: revolver and crossbow + lasersight
				if (moveableObj->Id == ID_REVOLVER_ITEM && !g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight && n > 0)
					break;

				if (moveableObj->Id == ID_CROSSBOW_ITEM && !g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight && n > 0)
					break;

				// Finish the world matrix
				if (obj->animIndex != -1)
					m_stItem.World = (moveableObj->AnimationTransforms[n] * transform).Transpose();
				else
					m_stItem.World = (moveableObj->BindPoseTransforms[n].Transpose() * transform).Transpose();
				m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
				updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
				m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
				m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

				for (int m = 0; m < NUM_BUCKETS; m++)
				{
					RendererBucket* bucket = &mesh->Buckets[m];
					if (bucket->NumVertices == 0)
						continue;

					if (m < 2)
						m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
					else
						m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

					m_stMisc.AlphaTest = (m < 2);
					updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
					m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				}
			}

			short inventoryItem = ring->objects[objectIndex].inventoryObject;

			// Draw special stuff if needed
			if (objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				if (g_Inventory->GetActiveRing() == INV_RING_OPTIONS)
				{
					/* **************** PASSAPORT ************* */
					if (inventoryItem == INV_OBJECT_PASSPORT && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						/* **************** LOAD AND SAVE MENU ************* */
						if (ring->passportAction == INV_WHAT_PASSPORT_LOAD_GAME || ring->passportAction == INV_WHAT_PASSPORT_SAVE_GAME)
						{
							y = 44;

							for (int n = 0; n < MAX_SAVEGAMES; n++)
							{
								if (!g_NewSavegameInfos[n].Present)
									PrintString(400, y, g_GameFlow->GetString(45), D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								else
								{
									sprintf(stringBuffer, "%05d", g_NewSavegameInfos[n].Count);
									PrintString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									PrintString(250, y, (char*)g_NewSavegameInfos[n].LevelName.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									sprintf(stringBuffer, g_GameFlow->GetString(44), g_NewSavegameInfos[n].Days, g_NewSavegameInfos[n].Hours, g_NewSavegameInfos[n].Minutes, g_NewSavegameInfos[n].Seconds);
									PrintString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								}

								y += 24;
							}

							drawLogo = false;

							drawGuiRect = true;
							guiRect.left = 180;
							guiRect.right = 440;
							guiRect.top = 24;
							guiRect.bottom = y + 20 - 24;

							//drawColoredQuad(180, 24, 440, y + 20 - 24, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
						}
						/* **************** SELECT LEVEL ************* */
						else if (ring->passportAction == INV_WHAT_PASSPORT_SELECT_LEVEL)
						{
							drawLogo = false;

							drawGuiRect = true;
							guiRect.left = 200;
							guiRect.right = 400;
							guiRect.top = 24;
							guiRect.bottom = 24 * (g_GameFlow->GetNumLevels() - 1) + 40;

							//drawColoredQuad(200, 24, 400, 24 * (g_GameFlow->GetNumLevels() - 1) + 40, Vector4(0.0f, 0.0f, 0.25f, 0.5f));

							short lastY = 50;

							for (int n = 1; n < g_GameFlow->GetNumLevels(); n++)
							{
								GameScriptLevel* levelScript = g_GameFlow->GetLevel(n);
								PrintString(400, lastY, g_GameFlow->GetString(levelScript->NameStringIndex), D3DCOLOR_ARGB(255, 255, 255, 255),
									PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n - 1 ? PRINTSTRING_BLINK : 0));

								lastY += 24;
							}
						}
						char* string = (char*)"";
						switch (ring->passportAction)
						{
						case INV_WHAT_PASSPORT_NEW_GAME:
							string = g_GameFlow->GetString(STRING_NEW_GAME);
							break;
						case INV_WHAT_PASSPORT_SELECT_LEVEL:
							string = g_GameFlow->GetString(STRING_SELECT_LEVEL);
							break;
						case INV_WHAT_PASSPORT_LOAD_GAME:
							string = g_GameFlow->GetString(STRING_LOAD_GAME);
							break;
						case INV_WHAT_PASSPORT_SAVE_GAME:
							string = g_GameFlow->GetString(STRING_SAVE_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_GAME:
							string = g_GameFlow->GetString(STRING_EXIT_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_TO_TITLE:
							string = g_GameFlow->GetString(STRING_EXIT_TO_TITLE);
							break;
						}

						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
					/* **************** GRAPHICS SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_SUNGLASSES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw settings menu
						RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];

						int y = 200;

						PrintString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

						y += 25;

						// Screen resolution
						PrintString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						RendererDisplayMode * mode = &adapter->DisplayModes[ring->SelectedVideoMode];
						char buffer[255];
						ZeroMemory(buffer, 255);
						sprintf(buffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

						PrintString(400, y, buffer, PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Windowed mode
						PrintString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.Windowed ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable dynamic shadows
						PrintString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableShadows ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable caustics
						PrintString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableCaustics ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable volumetric fog
						PrintString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableVolumetricFog ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));

						y += 25;

						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 6 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawLogo = false;

						drawGuiRect = true;
						guiRect.left = 180;
						guiRect.right = 440;
						guiRect.top = 180;
						guiRect.bottom = y + 20 - 180;

						//drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					/* **************** AUDIO SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_HEADPHONES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw sound menu

						y = 200;

						PrintString(400, y, g_GameFlow->GetString(STRING_SOUND),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

						y += 25;

						// Enable sound
						PrintString(200, y, g_GameFlow->GetString(STRING_ENABLE_SOUND),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableSound ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable sound special effects
						PrintString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableAudioSpecialEffects ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Music volume
						PrintString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
						//DrawBar(400, y + 4, 150, 18, ring->Configuration.MusicVolume, 0x0000FF, 0x0000FF);
						DrawBar(ring->Configuration.MusicVolume / 100.0f, g_MusicVolumeBar);

						y += 25;

						// Sound FX volume
						PrintString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
						//DrawBar(400, y + 4, 150, 18, ring->Configuration.SfxVolume, 0x0000FF, 0x0000FF);
						DrawBar(ring->Configuration.SfxVolume / 100.0f, g_SFXVolumeBar);
						y += 25;

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

						y += 25;

						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawLogo = false;

						drawGuiRect = true;
						guiRect.left = 180;
						guiRect.right = 440;
						guiRect.top = 180;
						guiRect.bottom = y + 20 - 180;

						//drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					/* **************** CONTROLS SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_KEYS && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw sound menu
						y = 40;

						PrintString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

						y += 25;

						for (int k = 0; k < 18; k++)
						{
							PrintString(200, y, g_GameFlow->GetString(STRING_CONTROLS_MOVE_FORWARD + k),
								PRINTSTRING_COLOR_WHITE,
								PRINTSTRING_OUTLINE | (ring->selectedIndex == k ? PRINTSTRING_BLINK : 0) |
								(ring->waitingForKey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

							if (ring->waitingForKey && k == ring->selectedIndex)
							{
								PrintString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
									PRINTSTRING_COLOR_YELLOW,
									PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
							}
							else
							{
								PrintString(400, y, (char*)g_KeyNames[KeyboardLayout1[k]],
									PRINTSTRING_COLOR_ORANGE,
									PRINTSTRING_OUTLINE);
							}

							y += 25;
						}

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 0 ? PRINTSTRING_BLINK : 0));

						y += 25;

						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawLogo = false;

						drawGuiRect = true;
						guiRect.left = 180;
						guiRect.right = 440;
						guiRect.top = 20;
						guiRect.bottom = y + 20 - 20;

						//drawColoredQuad(180, 20, 440, y + 20 - 20, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					else
					{
						// Draw the description below the object
						char* string = g_GameFlow->GetString(g_Inventory->GetInventoryObject(inventoryItem)->objectName); // (char*)g_NewStrings[g_Inventory->GetInventoryObject(inventoryItem)->objectName].c_str(); // &AllStrings[AllStringsOffsets[g_Inventory->GetInventoryObject(inventoryItem)->objectName]];
						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
				}
				else
				{
					short inventoryItem = g_Inventory->GetRing(k)->objects[objectIndex].inventoryObject;
					char* string = g_GameFlow->GetString(g_Inventory->GetInventoryObject(inventoryItem)->objectName); // &AllStrings[AllStringsOffsets[InventoryObjectsList[inventoryItem].objectName]];

					if (g_Inventory->IsCurrentObjectWeapon() && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						y = 100;

						for (int a = 0; a < ring->numActions; a++)
						{
							int stringIndex = 0;
							if (ring->actions[a] == INV_ACTION_USE) stringIndex = STRING_USE;
							if (ring->actions[a] == INV_ACTION_COMBINE) stringIndex = STRING_COMBINE;
							if (ring->actions[a] == INV_ACTION_SEPARE) stringIndex = STRING_SEPARE;
							if (ring->actions[a] == INV_ACTION_SELECT_AMMO) stringIndex = STRING_CHOOSE_AMMO;

							// Apply and cancel
							PrintString(400, y, g_GameFlow->GetString(stringIndex),
								PRINTSTRING_COLOR_WHITE,
								PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == a ? PRINTSTRING_BLINK : 0));

							y += 25;
						}

						drawLogo = false;

						drawGuiRect = true;
						guiRect.left = 300;
						guiRect.right = 200;
						guiRect.top = 80;
						guiRect.bottom = y + 20 - 80;

						//drawColoredQuad(300, 80, 200, y + 20 - 80, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}

					int quantity = -1;
					switch (objectNumber)
					{
					case ID_BIGMEDI_ITEM:
						quantity = g_LaraExtra.NumLargeMedipacks;
						break;
					case ID_SMALLMEDI_ITEM:
						quantity = g_LaraExtra.NumSmallMedipacks;
						break;
					case ID_FLARE_INV_ITEM:
						quantity = g_LaraExtra.NumFlares;
						break;
					case ID_SHOTGUN_AMMO1_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0];
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_SHOTGUN_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1];
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_HK_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_HK].Ammo[0];
						break;
					case ID_CROSSBOW_AMMO1_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0];
						break;
					case ID_CROSSBOW_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1];
						break;
					case ID_CROSSBOW_AMMO3_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2];
						break;
					case ID_REVOLVER_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0];
						break;
					case ID_UZI_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0];
						break;
					case ID_PISTOLS_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_PISTOLS].Ammo[0];
						break;
					case ID_GRENADE_AMMO1_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0];
						break;
					case ID_GRENADE_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1];
						break;
					case ID_GRENADE_AMMO3_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2];
						break;
					case ID_HARPOON_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0];
						break;
					case ID_ROCKET_LAUNCHER_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0];
						break;
					case ID_PICKUP_ITEM4:
						quantity = Savegame.Level.Secrets;
						break;
					default:
						if (objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8)
							quantity = g_LaraExtra.Puzzles[objectNumber - ID_PUZZLE_ITEM1];

						else if (objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2)
							quantity = g_LaraExtra.PuzzlesCombo[objectNumber - ID_PUZZLE_ITEM1_COMBO1];

						else if (objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8)
							quantity = g_LaraExtra.Keys[objectNumber - ID_KEY_ITEM1];

						else if (objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2)
							quantity = g_LaraExtra.KeysCombo[objectNumber - ID_KEY_ITEM1_COMBO1];

						else if (objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM3)
							quantity = g_LaraExtra.Pickups[objectNumber - ID_PICKUP_ITEM1];

						else if (objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM3_COMBO2)
							quantity = g_LaraExtra.PickupsCombo[objectNumber - ID_PICKUP_ITEM1_COMBO1];

						else if (objectNumber >= ID_EXAMINE1 && objectNumber <= ID_EXAMINE3)
							quantity = g_LaraExtra.Pickups[objectNumber - ID_EXAMINE1];

						else if (objectNumber >= ID_EXAMINE1_COMBO1 && objectNumber <= ID_EXAMINE3_COMBO2)
							quantity = g_LaraExtra.PickupsCombo[objectNumber - ID_EXAMINE1_COMBO1];

					}

					if (quantity < 1)
						PrintString(400, 550, string, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
					else
					{
						sprintf(stringBuffer, "%d x %s", quantity, string);
						PrintString(400, 550, stringBuffer, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
					}
				}
			}

			objectIndex++;
			if (objectIndex == numObjects) objectIndex = 0;
		}

		lastRing++;
	}

	if (drawGuiRect)
	{
		// Draw blu box
		drawColoredQuad(guiRect.left, guiRect.top, guiRect.right, guiRect.bottom, guiColor);
	}

	drawLines2D();
	drawAllStrings();

	if (g_Inventory->GetType() == INV_TYPE_TITLE && g_GameFlow->TitleType == TITLE_FLYBY && drawLogo)
	{
		// Draw main logo
		float factorX = (float)ScreenWidth / REFERENCE_RES_WIDTH;
		float factorY = (float)ScreenHeight / REFERENCE_RES_HEIGHT;

		RECT rect;
		rect.left = 250 * factorX;
		rect.right = 550 * factorX;
		rect.top = 50 * factorY;
		rect.bottom = 200 * factorY;

		m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->Additive());
		m_spriteBatch->Draw(m_logo->ShaderResourceView, rect, Vector4::One);
		m_spriteBatch->End();
	}

	return 0;
}

bool Renderer11::drawFullScreenQuad(ID3D11ShaderResourceView * texture, Vector3 color, bool cinematicBars)
{
	RendererVertex vertices[4];

	if (!cinematicBars)
	{
		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
	}
	else
	{
		float cinematicFactor = 0.12f;

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f - cinematicFactor * 2;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = cinematicFactor;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f - cinematicFactor * 2;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = cinematicFactor;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -(1.0f - cinematicFactor * 2);
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f - cinematicFactor;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -(1.0f - cinematicFactor * 2);
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f - cinematicFactor;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
	}

	m_context->VSSetShader(m_vsFullScreenQuad, NULL, 0);
	m_context->PSSetShader(m_psFullScreenQuad, NULL, 0);

	m_context->PSSetShaderResources(0, 1, &texture);
	ID3D11SamplerState * sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();
	m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
	m_primitiveBatch->End();

	return true;
}

bool Renderer11::drawRopes()
{
	for (int n = 0; n < NumRopes; n++)
	{
		ROPE_STRUCT* rope = &Ropes[n];

		if (rope->active)
		{
			// Original algorithm:
			// 1) Transform segment coordinates from 3D to 2D + depth
			// 2) Get dx, dy and the segment length
			// 3) Get sine and cosine from dx / length and dy / length
			// 4) Calculate a scale factor 
			// 5) Get the coordinates of the 4 corners of each sprite iteratively
			// 6) Last step only for us, unproject back to 3D coordinates

			// Tranform rope points
			Vector3 projected[24];
			Matrix world = Matrix::Identity;

			for (int i = 0; i < 24; i++)
			{
				Vector3 absolutePosition = Vector3(rope->position.x + rope->segment[i].x / 65536.0f,
					rope->position.y + rope->segment[i].y / 65536.0f,
					rope->position.z + rope->segment[i].z / 65536.0f);

				projected[i] = m_viewportToolkit->Project(absolutePosition, Projection, View, world);
			}

			// Now each rope point is transformed in screen X, Y and Z depth
			// Let's calculate dx, dy corrections and scaling
			float dx = projected[1].x - projected[0].x;
			float dy = projected[1].y - projected[0].y;
			float length = sqrt(dx * dx + dy * dy);
			float s = 0;
			float c = 0;

			if (length != 0)
			{
				s = -dy / length;
				c = dx / length;
			}

			float w = 6.0f;
			if (projected[0].z)
			{
				w = 6.0f * PhdPerspective / projected[0].z / 65536.0f;
				if (w < 3)
					w = 3;
			}

			float sdx = s * w;
			float sdy = c * w;

			float x1 = projected[0].x - sdx;
			float y1 = projected[0].y - sdy;

			float x2 = projected[0].x + sdx;
			float y2 = projected[0].y + sdy;

			float depth = projected[0].z;

			for (int j = 0; j < 24; j++)
			{
				Vector3 p1 = m_viewportToolkit->Unproject(Vector3(x1, y1, depth), Projection, View, world);
				Vector3 p2 = m_viewportToolkit->Unproject(Vector3(x2, y2, depth), Projection, View, world);

				dx = projected[j].x - projected[j - 1].x;
				dy = projected[j].y - projected[j - 1].y;
				length = sqrt(dx * dx + dy * dy);
				s = 0;
				c = 0;

				if (length != 0)
				{
					s = -dy / length;
					c = dx / length;
				}

				w = 6.0f;
				if (projected[j].z)
				{
					w = 6.0f * PhdPerspective / projected[j].z / 65536.0f;
					if (w < 3)
						w = 3;
				}

				float sdx = s * w;
				float sdy = c * w;

				float x3 = projected[j].x - sdx;
				float y3 = projected[j].y - sdy;

				float x4 = projected[j].x + sdx;
				float y4 = projected[j].y + sdy;

				depth = projected[j].z;

				Vector3 p3 = m_viewportToolkit->Unproject(Vector3(x3, y3, depth), Projection, View, world);
				Vector3 p4 = m_viewportToolkit->Unproject(Vector3(x4, y4, depth), Projection, View, world);

				AddSprite3D(m_sprites[20],
					Vector3(p1.x, p1.y, p1.z),
					Vector3(p2.x, p2.y, p2.z),
					Vector3(p3.x, p3.y, p3.z),
					Vector3(p4.x, p4.y, p4.z),
					Vector4(0.5f, 0.5f, 0.5f, 1.0f), 0, 1, 0, 0, BLENDMODE_OPAQUE);

				x1 = x4;
				y1 = y4;
				x2 = x3;
				y2 = y3;
			}

		}
	}

	return true;
}

bool Renderer11::drawLines2D()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	m_context->VSSetShader(m_vsSolid, NULL, 0);
	m_context->PSSetShader(m_psSolid, NULL, 0);
	Matrix world = Matrix::CreateOrthographicOffCenter(0, ScreenWidth, ScreenHeight, 0, m_viewport.MinDepth, m_viewport.MaxDepth);

	m_stCameraMatrices.View = Matrix::Identity;
	m_stCameraMatrices.Projection = Matrix::Identity;
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();

	for (int i = 0; i < m_lines2DToDraw.size(); i++)
	{
		RendererLine2D* line = m_lines2DToDraw[i];

		RendererVertex v1;
		v1.Position.x = line->Vertices[0].x;
		v1.Position.y = line->Vertices[0].y;
		v1.Position.z = 1.0f;
		v1.Color.x = line->Color.x / 255.0f;
		v1.Color.y = line->Color.y / 255.0f;
		v1.Color.z = line->Color.z / 255.0f;
		v1.Color.w = line->Color.w / 255.0f;

		RendererVertex v2;
		v2.Position.x = line->Vertices[1].x;
		v2.Position.y = line->Vertices[1].y;
		v2.Position.z = 1.0f;
		v2.Color.x = line->Color.x / 255.0f;
		v2.Color.y = line->Color.y / 255.0f;
		v2.Color.z = line->Color.z / 255.0f;
		v2.Color.w = line->Color.w / 255.0f;

		v1.Position = Vector3::Transform(v1.Position, world);
		v2.Position = Vector3::Transform(v2.Position, world);

		v1.Position.z = 0.5f;
		v2.Position.z = 0.5f;

		m_primitiveBatch->DrawLine(v1, v2);
	}

	m_primitiveBatch->End();

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::drawSpiders()
{
	/*XMMATRIX world;
	UINT cPasses = 1;

	if (Objects[ID_SPIDER].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_SPIDER];
		RendererObject* moveableObj = m_moveableObjects[ID_SPIDER].get();
		short* meshPtr = Meshes[Objects[ID_SPIDER].meshIndex + ((Wibble >> 2) & 2)];
		RendererMesh* mesh = m_meshPointersToMesh[meshPtr];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		if (bucket->NumVertices == 0)
			return true;

		setGpuStateForBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		LPD3DXEFFECT effect;
		if (pass == RENDERER_PASS_SHADOW_MAP)
			effect = m_shaderDepth->GetEffect();
		else if (pass == RENDERER_PASS_RECONSTRUCT_DEPTH)
			effect = m_shaderReconstructZBuffer->GetEffect();
		else if (pass == RENDERER_PASS_GBUFFER)
			effect = m_shaderFillGBuffer->GetEffect();
		else
			effect = m_shaderTransparent->GetEffect();

		effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
		effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPE_MOVEABLE);

		if (bucketIndex == RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKET_SOLID_DS)
			effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_OPAQUE);
		else
			effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_ALPHATEST);

		for (int i = 0; i < NUM_SPIDERS; i++)
		{
			SPIDER_STRUCT* spider = &Spiders[i];

			if (spider->on)
			{
				XMMATRIXTranslation(&m_tempTranslation, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);
				XMMATRIXRotationYawPitchRoll(&m_tempRotation, spider->pos.yRot, spider->pos.xRot, spider->pos.zRot);
				XMMATRIXMultiply(&m_tempWorld, &m_tempRotation, &m_tempTranslation);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_tempWorld);

				effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[spider->roomNumber]->AmbientLight);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					drawPrimitives(D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}
	}*/

	return true;
}

bool Renderer11::drawRats()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	if (Objects[ID_RATS].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_BATS];
		RendererObject* moveableObj = m_moveableObjects[ID_BATS];

		for (int m = 0; m < 32; m++)
			memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

		for (int i = 0; i < NUM_RATS; i += 4)
		{
			RAT_STRUCT* rat = &Rats[i];

			if (rat->on)
			{
				short* meshPtr = Meshes[Objects[ID_BATS].meshIndex + (((i + Wibble) >> 2) & 0xE)];
				RendererMesh * mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(meshPtr)];

				Matrix translation = Matrix::CreateTranslation(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
				Matrix rotation = Matrix::CreateFromYawPitchRoll(rat->pos.yRot, rat->pos.xRot, rat->pos.zRot);
				Matrix world = rotation * translation;

				m_stItem.World = world.Transpose();
				m_stItem.Position = Vector4(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, 1.0f);
				m_stItem.AmbientLight = m_rooms[rat->roomNumber].AmbientLight;
				updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

				for (int b = 0; b < 2; b++)
				{
					RendererBucket* bucket = &mesh->Buckets[b];

					if (bucket->NumVertices == 0)
						continue;

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	return true;
}

bool Renderer11::drawBats()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	if (Objects[ID_BATS].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_BATS];
		RendererObject* moveableObj = m_moveableObjects[ID_BATS];
		short* meshPtr = Meshes[Objects[ID_BATS].meshIndex + 2 * (-GlobalCounter & 3)];
		RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(meshPtr)];

		for (int m = 0; m < 32; m++)
			memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

		for (int b = 0; b < 2; b++)
		{
			RendererBucket* bucket = &mesh->Buckets[b];

			if (bucket->NumVertices == 0)
				continue;

			for (int i = 0; i < NUM_BATS; i++)
			{
				BAT_STRUCT* bat = &Bats[i];

				if (bat->on)
				{
					Matrix translation = Matrix::CreateTranslation(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(bat->pos.yRot, bat->pos.xRot, bat->pos.zRot);
					Matrix world = rotation * translation;

					m_stItem.World = world.Transpose();
					m_stItem.Position = Vector4(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[bat->roomNumber].AmbientLight;
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	return true;
}

bool Renderer11::doSnow()
{
	if (m_firstWeather)
	{
		for (int i = 0; i < NUM_SNOW_PARTICLES; i++)
			m_snow[i].Reset = true;
	}

	for (int i = 0; i < NUM_SNOW_PARTICLES; i++)
	{
		RendererWeatherParticle* snow = &m_snow[i];

		if (snow->Reset)
		{
			snow->X = LaraItem->pos.xPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			snow->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT) + (rand() % 512);
			snow->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;

			// Check if in inside room
			short roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO * floor = GetFloor(snow->X, snow->Y, snow->Z, &roomNumber);
			ROOM_INFO * room = &Rooms[roomNumber];
			if (!(room->flags & ENV_FLAG_OUTSIDE))
				continue;

			snow->Size = SNOW_DELTA_Y + (rand() % 64);
			snow->AngleH = (rand() % SNOW_MAX_ANGLE_H) * RADIAN;
			snow->AngleV = (rand() % SNOW_MAX_ANGLE_V) * RADIAN;
			snow->Reset = false;
		}

		float radius = snow->Size * sin(snow->AngleV);

		float dx = sin(snow->AngleH) * radius;
		float dz = cos(snow->AngleH) * radius;

		snow->X += dx;
		snow->Y += SNOW_DELTA_Y;
		snow->Z += dz;

		if (snow->X <= 0 || snow->Z <= 0 || snow->X >= 100 * 1024.0f || snow->Z >= 100 * 1024.0f)
		{
			snow->Reset = true;
			continue;
		}

		AddSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST], Vector3(snow->X, snow->Y, snow->Z), Vector4(1, 1, 1, 1),
			0.0f, 1.0f, SNOW_SIZE, SNOW_SIZE,
			BLENDMODE_ALPHABLEND);

		short roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO * floor = GetFloor(snow->X, snow->Y, snow->Z, &roomNumber);
		ROOM_INFO * room = &Rooms[roomNumber];
		if (snow->Y >= room->y + room->minfloor)
			snow->Reset = true;
	}

	m_firstWeather = false;

	return true;
}

bool Renderer11::doRain()
{
	if (m_firstWeather)
	{
		for (int i = 0; i < NUM_RAIN_DROPS; i++)
		{
			m_rain[i].Reset = true;
			m_rain[i].Draw = true;
		}
	}

	for (int i = 0; i < NUM_RAIN_DROPS; i++)
	{
		RendererWeatherParticle* drop = &m_rain[i];

		if (drop->Reset)
		{
			drop->Draw = true;

			drop->X = LaraItem->pos.xPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			drop->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT);
			drop->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;

			// Check if in inside room
			short roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO * floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
			ROOM_INFO * room = &Rooms[roomNumber];
			if (!(room->flags & ENV_FLAG_OUTSIDE))
			{
				drop->Reset = true;
				continue;
			}

			drop->Size = RAIN_SIZE + (rand() % 64);
			drop->AngleH = (rand() % RAIN_MAX_ANGLE_H) * RADIAN;
			drop->AngleV = (rand() % RAIN_MAX_ANGLE_V) * RADIAN;
			drop->Reset = false;
		}

		float x1 = drop->X;
		float y1 = drop->Y;
		float z1 = drop->Z;

		float radius = drop->Size * sin(drop->AngleV);

		float dx = sin(drop->AngleH) * radius;
		float dy = drop->Size * cos(drop->AngleV);
		float dz = cos(drop->AngleH) * radius;

		drop->X += dx;
		drop->Y += RAIN_DELTA_Y;
		drop->Z += dz;

		if (drop->Draw)
			AddLine3D(Vector3(x1, y1, z1), Vector3(drop->X, drop->Y, drop->Z), Vector4(RAIN_COLOR, RAIN_COLOR, RAIN_COLOR, 1.0f));

		// If rain drop has hit the ground, then reset it and add a little drip
		short roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO* floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
		ROOM_INFO* room = &Rooms[roomNumber];
		if (drop->Y >= room->y + room->minfloor)
		{
			drop->Reset = true;
			AddWaterSparks(drop->X, room->y + room->minfloor, drop->Z, 1);
		}
	}

	m_firstWeather = false;

	return true;
}

bool Renderer11::drawLines3D()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	m_context->VSSetShader(m_vsSolid, NULL, 0);
	m_context->PSSetShader(m_psSolid, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();

	for (int i = 0; i < m_lines3DToDraw.size(); i++)
	{
		RendererLine3D* line = m_lines3DToDraw[i];

		RendererVertex v1;
		v1.Position = line->start;
		v1.Color = line->color;

		RendererVertex v2;
		v2.Position = line->end;
		v2.Color = line->color;
		m_primitiveBatch->DrawLine(v1, v2);
	}

	m_primitiveBatch->End();

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}


void Renderer11::AddLine3D(Vector3 start, Vector3 end, Vector4 color)
{
	if (m_nextLine3D >= MAX_LINES_3D)
		return;

	RendererLine3D * line = &m_lines3DBuffer[m_nextLine3D++];

	line->start = start;
	line->end = end;
	line->color = color;

	m_lines3DToDraw.push_back(line);
}

void Renderer11::DrawLoadingScreen(char* fileName)
{
	return;

	Texture2D* texture = Texture2D::LoadFromFile(m_device, fileName);
	if (texture == NULL)
		return;

	m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
	m_fadeFactor = 0.0f;

	while (true)
	{
		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor < 1.0f)
			m_fadeFactor += FADE_FACTOR;

		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor > 0.0f)
			m_fadeFactor -= FADE_FACTOR;

		// Set basic render states
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

		// Clear screen
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Bind the back buffer
		m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
		m_context->RSSetViewports(1, &m_viewport);

		// Draw the full screen background
		drawFullScreenQuad(texture->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_swapChain->Present(0, 0);
		m_context->ClearState();
		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor >= 1.0f)
		{
			m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;
			m_fadeFactor = 1.0f;
		}

		if (m_fadeStatus == RENDERER_FADE_STATUS::NO_FADE && m_progress == 100)
		{
			m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
			m_fadeFactor = 1.0f;
		}

		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.0f)
		{
			break;
		}
	}

	delete texture;
}

void Renderer11::AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
{
	if (m_nextLight >= MAX_LIGHTS)
		return;

	RendererLight * dynamicLight = &m_lights[m_nextLight++];

	dynamicLight->Position = Vector3(float(x), float(y), float(z));
	dynamicLight->Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f);
	dynamicLight->Out = falloff * 256.0f;
	dynamicLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
	dynamicLight->Dynamic = 1;
	dynamicLight->Intensity = 2.0f;

	m_dynamicLights.push_back(dynamicLight);
	//NumDynamics++;
}

void Renderer11::ClearDynamicLights()
{
	m_dynamicLights.clear();
}

int Renderer11::drawFinalPass()
{
	// Update fade status
	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor > 0.99f)
		m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;

	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.01f)
		m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);

	drawFullScreenQuad(m_renderTarget->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), m_enableCinematicBars);

	m_swapChain->Present(0, 0);

	// Update fade status
	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN)
		m_fadeFactor += FADE_FACTOR;

	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT)
		m_fadeFactor -= FADE_FACTOR;

	return 0;
}

bool Renderer11::drawFullScreenImage(ID3D11ShaderResourceView* texture, float fade)
{
	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	m_context->ClearRenderTargetView(m_backBufferRTV, Colors::White);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	drawFullScreenQuad(texture, Vector3(fade, fade, fade), false);

	m_swapChain->Present(0, 0);

	return true;
}

int Renderer11::DrawInventory()
{
	if (CurrentLevel == 0 && g_GameFlow->TitleType == TITLE_FLYBY)
		drawScene(true);
	drawInventoryScene();
	drawFinalPass();

	return 0;
}

bool Renderer11::drawScene(bool dump)
{
	using ns = chrono::nanoseconds;
	using get_time = chrono::steady_clock;
	m_timeUpdate = 0;
	m_timeDraw = 0;
	m_timeFrame = 0;
	m_numDrawCalls = 0;
	m_nextLight = 0;
	m_nextSprite = 0;
	m_nextLine3D = 0;
	m_nextLine2D = 0;

	m_currentCausticsFrame++;
	m_currentCausticsFrame %= 32;

	m_strings.clear();

	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	ViewProjection = View * Projection;
	m_stLights.CameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

	// Prepare the scene to draw
	auto time1 = chrono::high_resolution_clock::now();

	clearSceneItems();
	collectRooms();
	updateLaraAnimations();
	updateItemsAnimations();
	updateEffects();

	m_items[Lara.itemNumber].Item = LaraItem;
	collectLightsForItem(LaraItem->roomNumber, &m_items[Lara.itemNumber]);

	// Update animated textures every 2 frames  
	if (GnFrameCounter % 2 == 0)
		updateAnimatedTextures();

	auto time2 = chrono::high_resolution_clock::now();
	m_timeUpdate = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;

	// Draw shadow map
	if (g_Configuration.EnableShadows)
		drawShadowMap();

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Bind and clear render target
	m_currentRenderTarget = (dump ? m_dumpScreenRenderTarget : m_renderTarget);

	m_context->ClearRenderTargetView(m_currentRenderTarget->RenderTargetView, Colors::Black);
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_currentRenderTarget->RenderTargetView, m_currentRenderTarget->DepthStencilView);

	m_context->RSSetViewports(1, &m_viewport);

	drawHorizonAndSky();

	// Opaque geometry
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

	drawRooms(false, false);
	drawRooms(false, true);
	drawStatics(false);
	drawLara(false, false);
	drawItems(false, false);
	drawItems(false, true);
	drawEffects(false);
	drawGunFlashes();
	drawGunShells();
	drawDebris(false);
	drawBats();
	drawRats();
	drawSpiders();

	// Transparent geometry
	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	drawRooms(true, false);
	drawRooms(true, true);
	drawStatics(true);
	drawLara(true, false);
	drawItems(true, false);
	drawItems(true, true);
	drawEffects(true);
	drawWaterfalls();
	drawDebris(true);

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Do special effects and weather
	drawFires();
	drawSmokes();
	drawFootprints();
	drawBlood();
	drawSparks();
	drawBubbles();
	drawDrips();
	drawRipples();
	drawUnderwaterDust();
	drawSplahes();
	drawShockwaves();



	switch (level->Weather)
	{
	case WEATHER_NORMAL:
		// no weather in normal
		break;
	case WEATHER_RAIN:
		doRain();
		break;
	case WEATHER_SNOW:
		doSnow();
		break;
	}

	drawRopes();
	drawSprites();
	drawLines3D();

	time2 = chrono::high_resolution_clock::now();
	m_timeFrame = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;

	// Bars
	int flash = FlashIt();
	if (DashTimer < 120)
		DrawBar(DashTimer / 120.0f, g_DashBar);
	UpdateHealtBar(flash);
	UpdateAirBar(flash);
	DrawAllPickups();

	drawLines2D();

	if (CurrentLevel != 0)
	{
		// Draw binoculars or lasersight
		drawOverlays();

		m_currentY = 60;
#ifdef _DEBUG
		ROOM_INFO* r = &Rooms[LaraItem->roomNumber];

		printDebugMessage("Update time: %d", m_timeUpdate);
		printDebugMessage("Frame time: %d", m_timeFrame);
		printDebugMessage("Draw calls: %d", m_numDrawCalls);
		printDebugMessage("Rooms: %d", m_roomsToDraw.size());
		printDebugMessage("Items: %d", m_itemsToDraw.size());
		printDebugMessage("Statics: %d", m_staticsToDraw.size());
		printDebugMessage("Lights: %d", m_lightsToDraw.size());
		printDebugMessage("Lara.roomNumber: %d", LaraItem->roomNumber);
		printDebugMessage("Lara.pos: %d %d %d", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		printDebugMessage("Lara.rot: %d %d %d", LaraItem->pos.xRot, LaraItem->pos.yRot, LaraItem->pos.zRot);
		printDebugMessage("Lara.animNumber: %d", LaraItem->animNumber);
		printDebugMessage("Lara.frameNumber: %d", LaraItem->frameNumber);
		printDebugMessage("Lara.currentAnimState: %d", LaraItem->currentAnimState);
		printDebugMessage("Lara.requiredAnimState: %d", LaraItem->requiredAnimState);
		printDebugMessage("Lara.goalAnimState: %d", LaraItem->goalAnimState);
		printDebugMessage("Lara.weaponItem: %d", Lara.weaponItem);
		printDebugMessage("Room: %d %d %d %d", r->x, r->z, r->x + r->xSize * WALL_SIZE, r->z + r->ySize * WALL_SIZE);
		printDebugMessage("Camera.pos: %d %d %d", Camera.pos.x, Camera.pos.y, Camera.pos.z);
		printDebugMessage("Camera.target: %d %d %d", Camera.target.x, Camera.target.y, Camera.target.z);
#endif
	}

	drawAllStrings();

	/*m_spriteBatch->Begin();
	RECT rect; rect.top = rect.left = 0; rect.right = rect.bottom = 128;
	m_spriteBatch->Draw(m_shadowMap->ShaderResourceView, rect, Colors::White);
	m_spriteBatch->End();*/

	if (!dump)
		m_swapChain->Present(0, 0);

	return true;
}

int Renderer11::DumpGameScene()
{
	drawScene(true);
	return 0;
}

bool Renderer11::drawItems(bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	RendererItem * item = &m_items[Lara.itemNumber];

	// Set shaders
	m_context->VSSetShader(m_vsItems, NULL, 0);
	m_context->PSSetShader(m_psItems, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState * sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (int i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererItem* item = m_itemsToDraw[i];
		RendererRoom& const room = m_rooms[item->Item->roomNumber];
		RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

		short objectNumber = item->Item->objectNumber;
		if (moveableObj->DoNotDraw)
		{
			continue;
		}
		else if (objectNumber == ID_TEETH_SPIKES || objectNumber == ID_RAISING_BLOCK1 || objectNumber == ID_RAISING_BLOCK2)
		{
			// Raising blocks and teeth spikes are normal animating objects but scaled on Y direction
			drawScaledSpikes(item, transparent, animated);
		}
		else if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
		{
			// We'll draw waterfalls later
			continue;
		}
		else
		{
			drawAnimatingItem(item, transparent, animated);
		}
	}

	return true;
}

bool Renderer11::drawAnimatingItem(RendererItem* item, bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);
	if (m_rooms.size() <= item->Item->roomNumber) {
		return true;
	}
	RendererRoom& const room = m_rooms[item->Item->roomNumber];
	RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

	m_stItem.World = item->World.Transpose();
	m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room.AmbientLight;
	memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

	m_stLights.NumLights = item->Lights.size();
	for (int j = 0; j < item->Lights.size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (int k = 0; k < moveableObj->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[k];

		for (int j = firstBucket; j < lastBucket; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	return true;
}

bool Renderer11::drawScaledSpikes(RendererItem* item, bool transparent, bool animated)
{
	short objectNumber = item->Item->objectNumber;
	if ((item->Item->objectNumber != ID_TEETH_SPIKES || item->Item->itemFlags[1])
		&& (item->Item->objectNumber != ID_RAISING_BLOCK1 || item->Item->triggerFlags > -1))
	{
		item->Scale = Matrix::CreateScale(1.0f, item->Item->itemFlags[1] / 4096.0f, 1.0f);
		item->World = item->Scale * item->Rotation * item->Translation;

		return drawAnimatingItem(item, transparent, animated);
	}
}

bool Renderer11::drawStatics(bool transparent)
{
	//return true;
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_staticsVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_staticsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsStatics, NULL, 0);
	m_context->PSSetShader(m_psStatics, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState * sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	for (int i = 0; i < m_staticsToDraw.size(); i++)
	{
		MESH_INFO* msh = m_staticsToDraw[i]->Mesh;

		if (!(msh->Flags & 1))
			continue;

		RendererRoom& const room = m_rooms[m_staticsToDraw[i]->RoomIndex];

		RendererObject* staticObj = m_staticObjects[msh->staticNumber];
		RendererMesh* mesh = staticObj->ObjectMeshes[0];

		m_stStatic.World = (Matrix::CreateRotationY(TR_ANGLE_TO_RAD(msh->yRot)) * Matrix::CreateTranslation(msh->x, msh->y, msh->z)).Transpose();
		m_stStatic.Color = Vector4(((msh->shade >> 10) & 0xFF) / 255.0f, ((msh->shade >> 5) & 0xFF) / 255.0f, ((msh->shade >> 0) & 0xFF) / 255.0f, 1.0f);
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);

		for (int j = firstBucket; j < lastBucket; j++)
		{
			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			RendererBucket * bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	return true;
}

bool Renderer11::drawRooms(bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);

	if (!animated)
	{
		// Set vertex buffer
		m_context->IASetVertexBuffers(0, 1, &m_roomsVertexBuffer->Buffer, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);
		m_context->IASetIndexBuffer(m_roomsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);
	}

	// Set shaders
	m_context->VSSetShader(m_vsRooms, NULL, 0);
	m_context->PSSetShader(m_psRooms, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicWrap();
	ID3D11SamplerState* shadowSampler = m_states->PointClamp();
	m_context->PSSetSamplers(0, 1, &sampler);
	m_context->PSSetShaderResources(1, 1, &m_caustics[m_currentCausticsFrame / 2]->ShaderResourceView);
	m_context->PSSetSamplers(1, 1, &shadowSampler);
	m_context->PSSetShaderResources(2, 1, &m_shadowMap->ShaderResourceView);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	// Set shadow map data
	if (m_shadowLight != NULL)
	{
		memcpy(&m_stShadowMap.Light, m_shadowLight, sizeof(ShaderLight));
		m_stShadowMap.CastShadows = true;
		//m_stShadowMap.ViewProjectionInverse = ViewProjection.Invert().Transpose();
	}
	else
	{
		m_stShadowMap.CastShadows = false;
	}

	updateConstantBuffer(m_cbShadowMap, &m_stShadowMap, sizeof(CShadowLightBuffer));
	m_context->VSSetConstantBuffers(4, 1, &m_cbShadowMap);
	m_context->PSSetConstantBuffers(4, 1, &m_cbShadowMap);

	if (animated)
		m_primitiveBatch->Begin();

	for (int i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_roomsToDraw[i];

		m_stLights.NumLights = room->LightsToDraw.size();
		for (int j = 0; j < room->LightsToDraw.size(); j++)
			memcpy(&m_stLights.Lights[j], room->LightsToDraw[j], sizeof(ShaderLight));
		updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
		m_context->PSSetConstantBuffers(1, 1, &m_cbLights);

		m_stMisc.Caustics = (room->Room->flags & ENV_FLAG_WATER);
		m_stMisc.AlphaTest = !transparent;
		updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
		m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);
		m_stRoom.AmbientColor = room->AmbientLight;
		updateConstantBuffer(m_cbRoom, &m_stRoom, sizeof(CRoomBuffer));
		m_context->PSSetConstantBuffers(5, 1, &m_cbRoom);
		for (int j = firstBucket; j < lastBucket; j++)
		{
			RendererBucket* bucket;
			if (!animated)
				bucket = &room->Buckets[j];
			else
				bucket = &room->AnimatedBuckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

				if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
					m_context->RSSetState(m_states->CullNone());
				else
					m_context->RSSetState(m_states->CullCounterClockwise());

			if (!animated)
			{
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
			else
			{
				for (int k = 0; k < bucket->Polygons.size(); k++)
				{
					RendererPolygon* poly = &bucket->Polygons[k];

					if (poly->Shape == SHAPE_RECTANGLE)
					{
						m_primitiveBatch->DrawQuad(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
							bucket->Vertices[poly->Indices[2]], bucket->Vertices[poly->Indices[3]]);
					}
					else
					{
						m_primitiveBatch->DrawTriangle(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
							bucket->Vertices[poly->Indices[2]]);
					}
				}
			}
		}
	}

	if (animated)
		m_primitiveBatch->End();

	return true;
}

bool Renderer11::drawHorizonAndSky()
{
	// Update the sky
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);
	Vector4 color = Vector4(SkyColor1.r / 255.0f, SkyColor1.g / 255.0f, SkyColor1.b / 255.0f, 1.0f);

	if (!level->Horizon)
		return true;

	if (BinocularRange)
		AlterFOV(14560 - BinocularRange);

	// Storm
	if (level->Storm)
	{
		if (Unk_00E6D74C || Unk_00E6D73C)
		{
			UpdateStorm();
			if (StormTimer > -1)
				StormTimer--;
			if (!StormTimer)
				SoundEffect(SFX_THUNDER_RUMBLE, NULL, 0);
		}
		else if (!(rand() & 0x7F))
		{
			Unk_00E6D74C = (rand() & 0x1F) + 16;
			Unk_00E6E4DC = rand() + 256;
			StormTimer = (rand() & 3) + 12;
		}

		color = Vector4((SkyStormColor[0]) / 255.0f, SkyStormColor[1] / 255.0f, SkyStormColor[2] / 255.0f, 1.0f);
	}

	ID3D11SamplerState* sampler;
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Draw the sky
	Matrix rotation = Matrix::CreateRotationX(PI);

	RendererVertex vertices[4];
	float size = 9728.0f;

	vertices[0].Position.x = -size / 2.0f;
	vertices[0].Position.y = 0.0f;
	vertices[0].Position.z = size / 2.0f;
	vertices[0].UV.x = 0.0f;
	vertices[0].UV.y = 0.0f;
	vertices[0].Color.x = 1.0f;
	vertices[0].Color.y = 1.0f;
	vertices[0].Color.z = 1.0f;
	vertices[0].Color.w = 1.0f;

	vertices[1].Position.x = size / 2.0f;
	vertices[1].Position.y = 0.0f;
	vertices[1].Position.z = size / 2.0f;
	vertices[1].UV.x = 1.0f;
	vertices[1].UV.y = 0.0f;
	vertices[1].Color.x = 1.0f;
	vertices[1].Color.y = 1.0f;
	vertices[1].Color.z = 1.0f;
	vertices[1].Color.w = 1.0f;

	vertices[2].Position.x = size / 2.0f;
	vertices[2].Position.y = 0.0f;
	vertices[2].Position.z = -size / 2.0f;
	vertices[2].UV.x = 1.0f;
	vertices[2].UV.y = 1.0f;
	vertices[2].Color.x = 1.0f;
	vertices[2].Color.y = 1.0f;
	vertices[2].Color.z = 1.0f;
	vertices[2].Color.w = 1.0f;

	vertices[3].Position.x = -size / 2.0f;
	vertices[3].Position.y = 0.0f;
	vertices[3].Position.z = -size / 2.0f;
	vertices[3].UV.x = 0.0f;
	vertices[3].UV.y = 1.0f;
	vertices[3].Color.x = 1.0f;
	vertices[3].Color.y = 1.0f;
	vertices[3].Color.z = 1.0f;
	vertices[3].Color.w = 1.0f;

	m_context->VSSetShader(m_vsSky, NULL, 0);
	m_context->PSSetShader(m_psSky, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_context->PSSetShaderResources(0, 1, &m_skyTexture->ShaderResourceView);
	sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);

	for (int i = 0; i < 2; i++)
	{
		Matrix translation = Matrix::CreateTranslation(Camera.pos.x + SkyPos1 - i * 9728.0f, Camera.pos.y - 1536.0f, Camera.pos.z);
		Matrix world = rotation * translation;

		m_stStatic.World = (rotation * translation).Transpose();
		m_stStatic.Color = color;
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);
		m_context->PSSetConstantBuffers(1, 1, &m_cbStatic);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	// Draw horizon
	if (m_moveableObjects[ID_HORIZON] != NULL)
	{
		m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

		m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
		sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		RendererObject* moveableObj = m_moveableObjects[ID_HORIZON];

		m_stStatic.World = Matrix::CreateTranslation(Camera.pos.x, Camera.pos.y, Camera.pos.z).Transpose();
		m_stStatic.Position = Vector4::Zero;
		m_stStatic.Color = Vector4::One;
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);
		m_context->PSSetConstantBuffers(1, 1, &m_cbStatic);

		m_stMisc.AlphaTest = true;
		updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
		m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

		for (int k = 0; k < moveableObj->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = moveableObj->ObjectMeshes[k];

			for (int j = 0; j < NUM_BUCKETS; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
					m_context->RSSetState(m_states->CullNone());
				else
					m_context->RSSetState(m_states->CullCounterClockwise());

				if (j == RENDERER_BUCKET_TRANSPARENT || j == RENDERER_BUCKET_TRANSPARENT_DS)
					m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
				else
					m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	// Clear just the Z-buffer so we can start drawing on top of the horizon
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return true;
}

bool Renderer11::drawAmbientCubeMap(short roomNumber)
{
	return true;
}

int	Renderer11::Draw()
{
	drawScene(false);
	drawFinalPass();

	return 0;
}