#include "framework.h"
#include "Renderer11.h"
#include "animation.h"
#include "effects\hair.h"
#include "lara.h"
#include "control/control.h"
#include "spotcam.h"
#include "camera.h"
#include "sphere.h"
#include "level.h"
#include "GameFlowScript.h"
#include <Specific\setup.h>
#include "lara_fire.h"
#include "items.h"

using namespace TEN::Renderer;

extern GameFlow *g_GameFlow;

bool shouldAnimateUpperBody(const LARA_WEAPON_TYPE& weapon) {
	ITEM_INFO& laraItem = *LaraItem;
	LaraInfo& laraInfo = Lara;
	switch(weapon){
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_CROSSBOW:
		case WEAPON_SHOTGUN:
			return (LaraItem->currentAnimState == LS_STOP || LaraItem->currentAnimState == LS_TURN_FAST || LaraItem->currentAnimState == LS_TURN_LEFT_SLOW || LaraItem->currentAnimState == LS_TURN_RIGHT_SLOW);
			break;
		case WEAPON_HK:
		{
			//Animate upper body if Lara is shooting from shoulder OR if Lara is standing still/turning
			int baseAnim = Objects[WeaponObject(weapon)].animIndex;
			if(laraInfo.rightArm.animNumber - baseAnim == 0 || laraInfo.rightArm.animNumber - baseAnim == 2 || laraInfo.rightArm.animNumber - baseAnim == 4){
				return true;
			} else
				return (LaraItem->currentAnimState == LS_STOP || LaraItem->currentAnimState == LS_TURN_FAST || LaraItem->currentAnimState == LS_TURN_LEFT_SLOW || LaraItem->currentAnimState == LS_TURN_RIGHT_SLOW);
		}
			break;
		default:
			return false;
			break;
		
	}
}
void Renderer11::updateLaraAnimations(bool force)
{
	Matrix translation;
	Matrix rotation;
	Matrix lastMatrix;
	Matrix hairMatrix;
	Matrix identity;
	Matrix world;

	RendererItem *item = &m_items[Lara.itemNumber];
	item->Id = Lara.itemNumber;
	item->Item = LaraItem;

	if (!force && item->DoneAnimations)
		return;

	RendererObject &laraObj = *m_moveableObjects[ID_LARA];

	// Clear extra rotations
	for (int i = 0; i < laraObj.LinearizedBones.size(); i++)
		laraObj.LinearizedBones[i]->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);

	// Lara world matrix
	translation = Matrix::CreateTranslation(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(LaraItem->pos.yRot), TO_RAD(LaraItem->pos.xRot), TO_RAD(LaraItem->pos.zRot));

	m_LaraWorldMatrix = rotation * translation;
	item->World = m_LaraWorldMatrix;

	// Update first Lara's animations
	laraObj.LinearizedBones[LM_TORSO]->ExtraRotation = Vector3(TO_RAD(Lara.torsoXrot), TO_RAD(Lara.torsoYrot), TO_RAD(Lara.torsoZrot));
	laraObj.LinearizedBones[LM_HEAD]->ExtraRotation = Vector3(TO_RAD(Lara.headXrot), TO_RAD(Lara.headYrot), TO_RAD(Lara.headZrot));

	// First calculate matrices for legs, hips, head and torso
	int mask = MESH_BITS(LM_HIPS) | MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT) | MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
	ANIM_FRAME* framePtr[2];
	int rate, frac;

	frac = GetFrame(LaraItem, framePtr, &rate);
	updateAnimation(item, laraObj, framePtr, frac, rate, mask);

	// Then the arms, based on current weapon status
	if (Lara.gunType != WEAPON_FLARE && (Lara.gunStatus == LG_NO_ARMS || Lara.gunStatus == LG_HANDS_BUSY) || Lara.gunType == WEAPON_FLARE && !Lara.flareControlLeft)
	{
		// Both arms
		mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND) | MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
		frac = GetFrame(LaraItem, framePtr, &rate);
		updateAnimation(item, laraObj, framePtr, frac, rate, mask);
	}
	else
	{
		// While handling weapon some extra rotation could be applied to arms

		if (Lara.gunType == WEAPON_REVOLVER) // im so sorry but it's either this or crazy arms with the revolver
		{
			laraObj.LinearizedBones[LM_RINARM]->ExtraRotation += Vector3(TO_RAD(Lara.rightArm.xRot), TO_RAD(Lara.rightArm.yRot), TO_RAD(Lara.rightArm.zRot));
			laraObj.LinearizedBones[LM_LINARM]->ExtraRotation = laraObj.LinearizedBones[LM_RINARM]->ExtraRotation;
		}
		else
		{
			laraObj.LinearizedBones[LM_LINARM]->ExtraRotation += Vector3(TO_RAD(Lara.leftArm.xRot), TO_RAD(Lara.leftArm.yRot), TO_RAD(Lara.leftArm.zRot));
			laraObj.LinearizedBones[LM_RINARM]->ExtraRotation += Vector3(TO_RAD(Lara.rightArm.xRot), TO_RAD(Lara.rightArm.yRot), TO_RAD(Lara.rightArm.zRot));
		}
		// yRot still messed up in some situations but it's definitely better now!!

		LARA_ARM *leftArm = &Lara.leftArm;
		LARA_ARM *rightArm = &Lara.rightArm;

		// HACK: backguns handles differently // TokyoSU: not really a hack since it's the original way to do that.
		switch (Lara.gunType)
		{
		case WEAPON_SHOTGUN:
		case WEAPON_HK:
		case WEAPON_CROSSBOW:
		case WEAPON_GRENADE_LAUNCHER:
		case WEAPON_ROCKET_LAUNCHER:
		case WEAPON_HARPOON_GUN:
		{
			ANIM_FRAME* shotgunFramePtr;

			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);

			if(shouldAnimateUpperBody(Lara.gunType)){
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
			}
			shotgunFramePtr = &g_Level.Frames[Lara.leftArm.frameBase + Lara.leftArm.frameNumber];
			updateAnimation(item, laraObj, &shotgunFramePtr, 0, 1, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			if(shouldAnimateUpperBody(Lara.gunType)){
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
			}
			shotgunFramePtr = &g_Level.Frames[Lara.rightArm.frameBase + Lara.rightArm.frameNumber];
			updateAnimation(item, laraObj, &shotgunFramePtr, 0, 1, mask);
		}
			break;
		case WEAPON_REVOLVER:
		{
			ANIM_FRAME* revolverFramePtr;

			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			revolverFramePtr = &g_Level.Frames[Lara.leftArm.frameBase + Lara.leftArm.frameNumber - g_Level.Anims[Lara.leftArm.animNumber].frameBase];
			updateAnimation(item, laraObj, &revolverFramePtr, 0, 1, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			revolverFramePtr = &g_Level.Frames[Lara.rightArm.frameBase + Lara.rightArm.frameNumber - g_Level.Anims[Lara.rightArm.animNumber].frameBase];
			updateAnimation(item, laraObj, &revolverFramePtr, 0, 1, mask);
		}
			break;

		case WEAPON_PISTOLS:
		case WEAPON_UZI:
		default:
		{
			ANIM_FRAME* pistolFramePtr;

			// Left arm
			int upperArmMask = MESH_BITS(LM_LINARM);
			mask = MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			pistolFramePtr = &g_Level.Frames[Lara.leftArm.frameBase + Lara.leftArm.frameNumber - g_Level.Anims[Lara.leftArm.animNumber].frameBase];
			updateAnimation(item, laraObj, &pistolFramePtr, 0, 1, upperArmMask, true);
			updateAnimation(item, laraObj, &pistolFramePtr, 0, 1, mask);

			// Right arm
			upperArmMask = MESH_BITS(LM_RINARM);
			mask = MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			pistolFramePtr = &g_Level.Frames[Lara.rightArm.frameBase + Lara.rightArm.frameNumber - g_Level.Anims[Lara.rightArm.animNumber].frameBase];
			updateAnimation(item, laraObj, &pistolFramePtr, 0, 1, upperArmMask, true);
			updateAnimation(item, laraObj, &pistolFramePtr, 0, 1, mask);
		}

		break;

		case WEAPON_FLARE:
		case WEAPON_TORCH:
			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			frac = getFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
			updateAnimation(item, laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			frac = GetFrame(LaraItem, framePtr, &rate);
			updateAnimation(item, laraObj, framePtr, frac, rate, mask);
			break;
		}
	}

	// Copy matrices in Lara object
	for (int m = 0; m < 15; m++)
		laraObj.AnimationTransforms[m] = item->AnimationTransforms[m];

	m_items[Lara.itemNumber].DoneAnimations = true;
}

void TEN::Renderer::Renderer11::drawLara(RenderView& view, bool transparent, bool shadowMap)
{
	if (transparent && shadowMap)
		return;

	// Don't draw Lara if binoculars or sniper
	if (BinocularRange || SpotcamOverlay || SpotcamDontDrawLara || CurrentLevel == 0)
		return;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	int firstBucket = (transparent ? 1 : 0);
	int lastBucket = (transparent ? 2 : 1);

	m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	RendererItem *item = &m_items[Lara.itemNumber];

	// Set shaders
	if (shadowMap)
	{
		m_context->VSSetShader(m_vsShadowMap.Get(), NULL, 0);
		m_context->PSSetShader(m_psShadowMap.Get(), NULL, 0);
	}
	else
	{
		m_context->VSSetShader(m_vsItems.Get(), NULL, 0);
		m_context->PSSetShader(m_psItems.Get(), NULL, 0);
	}

	// Set texture
	m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
	m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
	m_context->PSSetShaderResources(1, 1, m_reflectionCubemap.ShaderResourceView.GetAddressOf());
	ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_stMisc.AlphaTest = !transparent;
	m_cbMisc.updateData(m_stMisc, m_context.Get());
	m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

	RendererObject& laraObj = *m_moveableObjects[ID_LARA];
	RendererObject& laraSkin = *m_moveableObjects[ID_LARA_SKIN];
	RendererRoom& room = m_rooms[LaraItem->roomNumber];

	m_stItem.World = m_LaraWorldMatrix;
	m_stItem.Position = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room.AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj.AnimationTransforms.data(), sizeof(Matrix) * 32);
	m_cbItem.updateData(m_stItem, m_context.Get());
	m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
	m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

	if (!shadowMap)
	{
		m_stLights.NumLights = item->Lights.size();
		for (int j = 0; j < item->Lights.size(); j++)
			memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
		m_cbLights.updateData(m_stLights, m_context.Get());
		m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());
	}

	for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
	{
		RendererMesh *mesh = getMesh(Lara.meshPtrs[k]);
		drawLaraMesh(mesh, transparent);
	}
	drawLaraHolsters(transparent);
	if (m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
	{
		RendererObject &laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];
		RendererObject& laraSkin = *m_moveableObjects[ID_LARA_SKIN];

		for (int k = 1; k < laraSkinJoints.ObjectMeshes.size(); k++)
		{
			RendererMesh *mesh = laraSkinJoints.ObjectMeshes[k];
			drawLaraMesh(mesh, transparent);
		}
	}

	if (!transparent && Objects[ID_LARA_HAIR].loaded)
	{
		RendererObject& hairsObj = *m_moveableObjects[ID_LARA_HAIR];

		// First matrix is Lara's head matrix, then all 6 hairs matrices. Bones are adjusted at load time for accounting this.
		m_stItem.World = Matrix::Identity;
		Matrix matrices[7];
		matrices[0] = laraObj.AnimationTransforms[LM_HEAD] * m_LaraWorldMatrix;
		for (int i = 0; i < hairsObj.BindPoseTransforms.size(); i++)
		{
			HAIR_STRUCT* hairs = &Hairs[0][i];
			Matrix world = Matrix::CreateFromYawPitchRoll(TO_RAD(hairs->pos.yRot), TO_RAD(hairs->pos.xRot), 0) * Matrix::CreateTranslation(hairs->pos.xPos, hairs->pos.yPos, hairs->pos.zPos);
			matrices[i + 1] = world;
		}
		memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 7);
		m_cbItem.updateData(m_stItem,m_context.Get());
		m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
		m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

		for (int k = 0; k < hairsObj.ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = hairsObj.ObjectMeshes[k];
			drawLaraMesh(mesh, transparent);
		}	
	}
}

void Renderer11::drawLaraHolsters(bool transparent)
{
	int firstBucket = (transparent ? 2 : 0);
	int lastBucket = (transparent ? 4 : 2);
	HOLSTER_SLOT leftHolsterID = Lara.holsterInfo.leftHolster;
	HOLSTER_SLOT rightHolsterID = Lara.holsterInfo.rightHolster;
	HOLSTER_SLOT backHolsterID = Lara.holsterInfo.backHolster;

	
	if(m_moveableObjects[static_cast<int>(leftHolsterID)]){
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(leftHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_LTHIGH];
		drawLaraMesh(mesh, transparent);
	}
	if(m_moveableObjects[static_cast<int>(rightHolsterID)]){
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(rightHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_RTHIGH];
		drawLaraMesh(mesh, transparent);
	}
	if(backHolsterID != HOLSTER_SLOT::Empty && m_moveableObjects[static_cast<int>(backHolsterID)]){
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(backHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_TORSO];
		drawLaraMesh(mesh, transparent);
	}
}

void Renderer11::drawLaraMesh(RendererMesh* mesh, bool transparent)
{
	for (auto& bucket : mesh->buckets)
	{
		if (bucket.Vertices.size() == 0)
			continue;

		if (transparent && bucket.blendMode == BLENDMODE_OPAQUE)
			continue;

		setBlendMode(bucket.blendMode);

		// Draw vertices
		m_context->DrawIndexed(bucket.Indices.size(), bucket.StartIndex, 0);
		m_numDrawCalls++;
	}
}


