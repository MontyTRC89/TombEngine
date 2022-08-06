#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/animation.h"
#include "Game/effects/hair.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/control/control.h"
#include "Game/spotcam.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Flow/ScriptInterfaceFlowHandler.h"

using namespace TEN::Renderer;

extern ScriptInterfaceFlowHandler *g_GameFlow;

bool shouldAnimateUpperBody(const LaraWeaponType& weapon)
{
	ItemInfo& laraItem = *LaraItem;
	LaraInfo& laraInfo = Lara;

	switch (weapon)
	{
	case LaraWeaponType::RocketLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::Crossbow:
	case LaraWeaponType::Shotgun:
		if (LaraItem->Animation.ActiveState == LS_IDLE ||
			LaraItem->Animation.ActiveState == LS_TURN_LEFT_FAST ||
			LaraItem->Animation.ActiveState == LS_TURN_RIGHT_FAST ||
			LaraItem->Animation.ActiveState == LS_TURN_LEFT_SLOW ||
			LaraItem->Animation.ActiveState == LS_TURN_RIGHT_SLOW)
		{
			return true;
		}

		return false;

	case LaraWeaponType::HK:
	{
		// Animate upper body if Lara is shooting from shoulder OR if Lara is standing still/turning
		int baseAnim = Objects[WeaponObject(weapon)].animIndex;
		if (laraInfo.RightArm.AnimNumber - baseAnim == 0 ||
			laraInfo.RightArm.AnimNumber - baseAnim == 2 ||
			laraInfo.RightArm.AnimNumber - baseAnim == 4)
		{
			return true;
		}
		else
		{
			if (LaraItem->Animation.ActiveState == LS_IDLE ||
				LaraItem->Animation.ActiveState == LS_TURN_LEFT_FAST ||
				LaraItem->Animation.ActiveState == LS_TURN_RIGHT_FAST ||
				LaraItem->Animation.ActiveState == LS_TURN_LEFT_SLOW ||
				LaraItem->Animation.ActiveState == LS_TURN_RIGHT_SLOW)
			{
				return true;
			}

			return false;
		}
	}
	break;

	default:
		return false;
		break;
	}
}

void Renderer11::UpdateLaraAnimations(bool force)
{
	Matrix translation;
	Matrix rotation;
	Matrix lastMatrix;
	Matrix hairMatrix;
	Matrix identity;
	Matrix world;

	RendererItem* item = &m_items[Lara.ItemNumber];
	item->ItemNumber = Lara.ItemNumber;

	if (!force && item->DoneAnimations)
		return;

	RendererObject& laraObj = *m_moveableObjects[ID_LARA];

	// Clear extra rotations
	for (int i = 0; i < laraObj.LinearizedBones.size(); i++)
		laraObj.LinearizedBones[i]->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);

	// Lara world matrix
	translation = Matrix::CreateTranslation(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y, LaraItem->Pose.Position.z);
	rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(LaraItem->Pose.Orientation.y), TO_RAD(LaraItem->Pose.Orientation.x), TO_RAD(LaraItem->Pose.Orientation.z));

	m_LaraWorldMatrix = rotation * translation;
	item->World = m_LaraWorldMatrix;

	// Update first Lara's animations
	laraObj.LinearizedBones[LM_TORSO]->ExtraRotation = Vector3(TO_RAD(Lara.ExtraTorsoRot.x), TO_RAD(Lara.ExtraTorsoRot.y), TO_RAD(Lara.ExtraTorsoRot.z));
	laraObj.LinearizedBones[LM_HEAD]->ExtraRotation = Vector3(TO_RAD(Lara.ExtraHeadRot.x), TO_RAD(Lara.ExtraHeadRot.y), TO_RAD(Lara.ExtraHeadRot.z));

	// First calculate matrices for legs, hips, head and torso
	int mask = MESH_BITS(LM_HIPS) | MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT) | MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
	ANIM_FRAME* framePtr[2];
	int rate, frac;

	frac = GetFrame(LaraItem, framePtr, &rate);
	UpdateAnimation(item, laraObj, framePtr, frac, rate, mask);

	// Then the arms, based on current weapon status
	if (Lara.Control.Weapon.GunType != LaraWeaponType::Flare &&
		(Lara.Control.HandStatus == HandStatus::Free || Lara.Control.HandStatus == HandStatus::Busy) ||
		Lara.Control.Weapon.GunType == LaraWeaponType::Flare && !Lara.Flare.ControlLeft)
	{
		// Both arms
		mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND) | MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
		frac = GetFrame(LaraItem, framePtr, &rate);
		UpdateAnimation(item, laraObj, framePtr, frac, rate, mask);
	}
	else
	{
		// While handling weapon some extra rotation could be applied to arms
		if (Lara.Control.Weapon.GunType == LaraWeaponType::Pistol || Lara.Control.Weapon.GunType == LaraWeaponType::Uzi)
		{
			laraObj.LinearizedBones[LM_LINARM]->ExtraRotation += Vector3(TO_RAD(Lara.LeftArm.Orientation.x), TO_RAD(Lara.LeftArm.Orientation.y), TO_RAD(Lara.LeftArm.Orientation.z));
			laraObj.LinearizedBones[LM_RINARM]->ExtraRotation += Vector3(TO_RAD(Lara.RightArm.Orientation.x), TO_RAD(Lara.RightArm.Orientation.y), TO_RAD(Lara.RightArm.Orientation.z));
		}
		else
		{
			laraObj.LinearizedBones[LM_RINARM]->ExtraRotation += Vector3(TO_RAD(Lara.RightArm.Orientation.x), TO_RAD(Lara.RightArm.Orientation.y), TO_RAD(Lara.RightArm.Orientation.z));
			laraObj.LinearizedBones[LM_LINARM]->ExtraRotation = laraObj.LinearizedBones[LM_RINARM]->ExtraRotation;
		}

		ArmInfo* leftArm = &Lara.LeftArm;
		ArmInfo* rightArm = &Lara.RightArm;

		// HACK: Back guns handle differently. TokyoSU: Not really a hack since it's the original way to do it.
		switch (Lara.Control.Weapon.GunType)
		{
		case LaraWeaponType::Shotgun:
		case LaraWeaponType::HK:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		{
			ANIM_FRAME* shotgunFramePtr;

			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);

			if (shouldAnimateUpperBody(Lara.Control.Weapon.GunType))
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);

			shotgunFramePtr = &g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber];
			UpdateAnimation(item, laraObj, &shotgunFramePtr, 0, 1, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			if (shouldAnimateUpperBody(Lara.Control.Weapon.GunType))
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);

			shotgunFramePtr = &g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber];
			UpdateAnimation(item, laraObj, &shotgunFramePtr, 0, 1, mask);
		}

		break;

		case LaraWeaponType::Revolver:
		{
			ANIM_FRAME* revolverFramePtr;

			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			revolverFramePtr = &g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - g_Level.Anims[Lara.LeftArm.AnimNumber].frameBase];
			UpdateAnimation(item, laraObj, &revolverFramePtr, 0, 1, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			revolverFramePtr = &g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - g_Level.Anims[Lara.RightArm.AnimNumber].frameBase];
			UpdateAnimation(item, laraObj, &revolverFramePtr, 0, 1, mask);
		}

		break;

		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
		default:
		{
			ANIM_FRAME* pistolFramePtr;

			// Left arm
			int upperArmMask = MESH_BITS(LM_LINARM);
			mask = MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			pistolFramePtr = &g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - g_Level.Anims[Lara.LeftArm.AnimNumber].frameBase];
			UpdateAnimation(item, laraObj, &pistolFramePtr, 0, 1, upperArmMask, true);
			UpdateAnimation(item, laraObj, &pistolFramePtr, 0, 1, mask);

			// Right arm
			upperArmMask = MESH_BITS(LM_RINARM);
			mask = MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			pistolFramePtr = &g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - g_Level.Anims[Lara.RightArm.AnimNumber].frameBase];
			UpdateAnimation(item, laraObj, &pistolFramePtr, 0, 1, upperArmMask, true);
			UpdateAnimation(item, laraObj, &pistolFramePtr, 0, 1, mask);
		}

		break;

		case LaraWeaponType::Flare:
		case LaraWeaponType::Torch:
			// Left arm
			ItemInfo tempItem;
			tempItem.Animation.AnimNumber = Lara.LeftArm.AnimNumber;
			tempItem.Animation.FrameNumber = Lara.LeftArm.FrameNumber;

			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);

			// HACK: Mask head and torso only when pulling out the flare.
			if (!Lara.Control.IsLow &&
				tempItem.Animation.AnimNumber > Objects[ID_LARA_FLARE_ANIM].animIndex + 1 &&
				tempItem.Animation.AnimNumber < Objects[ID_LARA_FLARE_ANIM].animIndex + 4)
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);

			frac = GetFrame(&tempItem, framePtr, &rate);
			UpdateAnimation(item, laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			frac = GetFrame(LaraItem, framePtr, &rate);
			UpdateAnimation(item, laraObj, framePtr, frac, rate, mask);
			break;
		}
	}

	// Copy matrices in Lara object
	for (int m = 0; m < 15; m++)
		laraObj.AnimationTransforms[m] = item->AnimationTransforms[m];

	m_items[Lara.ItemNumber].DoneAnimations = true;
}

void TEN::Renderer::Renderer11::DrawLara(RenderView& view, bool transparent)
{
	// Don't draw Lara if binoculars or sniper
	if (BinocularRange || SpotcamDontDrawLara || CurrentLevel == 0)
		return;

	RendererItem* item = &m_items[Lara.ItemNumber];
	ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];

	if (nativeItem->Flags & IFLAG_INVISIBLE)
		return;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	m_context->VSSetShader(m_vsItems.Get(), nullptr, 0);
	m_context->PSSetShader(m_psItems.Get(), nullptr, 0);

	// Set texture
	BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[0]), SAMPLER_LINEAR_CLAMP);
	BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[0]), SAMPLER_NONE);

	SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

	RendererObject& laraObj = *m_moveableObjects[ID_LARA];
	RendererObject& laraSkin = *m_moveableObjects[ID_LARA_SKIN];

	RendererRoom* room = &m_rooms[LaraItem->RoomNumber];

	m_stItem.World = m_LaraWorldMatrix;
	m_stItem.Color = item->Color;
	m_stItem.AmbientLight = item->AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj.AnimationTransforms.data(), sizeof(Matrix) * MAX_BONES);
	for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
		m_stItem.BoneLightModes[k] = GetMesh(Lara.MeshPtrs[k])->LightMode;

	m_cbItem.updateData(m_stItem, m_context.Get());
	BindConstantBufferVS(CB_ITEM, m_cbItem.get());
	BindConstantBufferPS(CB_ITEM, m_cbItem.get());

	BindLights(item->LightsToDraw, item->RoomNumber, item->PrevRoomNumber, item->LightFade);

	for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
	{
		if (!nativeItem->TestBits(JointBitType::Mesh, k))
			continue;

		RendererMesh *mesh = GetMesh(Lara.MeshPtrs[k]);
		DrawMoveableMesh(item, mesh, room, k, transparent);
	}

	DrawLaraHolsters(transparent);

	if (m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
	{
		RendererObject& laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];
		RendererObject& laraSkin = *m_moveableObjects[ID_LARA_SKIN];

		for (int k = 1; k < laraSkinJoints.ObjectMeshes.size(); k++)
		{
			RendererMesh *mesh = laraSkinJoints.ObjectMeshes[k];
			DrawMoveableMesh(item, mesh, room, k, transparent);
		}
	}

	if (Objects[ID_LARA_HAIR].loaded)
	{
		RendererObject& hairsObj = *m_moveableObjects[ID_LARA_HAIR];

		// First matrix is Lara's head matrix, then all 6 hairs matrices. Bones are adjusted at load time for accounting this.
		m_stItem.World = Matrix::Identity;
		m_stItem.BonesMatrices[0] = laraObj.AnimationTransforms[LM_HEAD] * m_LaraWorldMatrix;

		for (int i = 0; i < hairsObj.BindPoseTransforms.size(); i++)
		{
			auto* hairs = &Hairs[0][i];
			Matrix world = Matrix::CreateFromYawPitchRoll(TO_RAD(hairs->pos.Orientation.y), TO_RAD(hairs->pos.Orientation.x), 0) *
				Matrix::CreateTranslation(hairs->pos.Position.x, hairs->pos.Position.y, hairs->pos.Position.z);
			m_stItem.BonesMatrices[i + 1] = world;
			m_stItem.BoneLightModes[i] = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
		}

		m_cbItem.updateData(m_stItem, m_context.Get());
		BindConstantBufferVS(CB_ITEM, m_cbItem.get());
		BindConstantBufferPS(CB_ITEM, m_cbItem.get());

		for (int k = 0; k < hairsObj.ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = hairsObj.ObjectMeshes[k];
			DrawMoveableMesh(item, mesh, room, k, transparent);
		}	
	}
}

void Renderer11::DrawLaraHolsters(bool transparent)
{
	RendererItem* item = &m_items[Lara.ItemNumber];
	RendererRoom* room = &m_rooms[LaraItem->RoomNumber];

	HolsterSlot leftHolsterID = Lara.Control.Weapon.HolsterInfo.LeftHolster;
	HolsterSlot rightHolsterID = Lara.Control.Weapon.HolsterInfo.RightHolster;
	HolsterSlot backHolsterID = Lara.Control.Weapon.HolsterInfo.BackHolster;

	if (m_moveableObjects[static_cast<int>(leftHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(leftHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_LTHIGH];
		DrawMoveableMesh(item, mesh, room, LM_LTHIGH, transparent);
	}

	if (m_moveableObjects[static_cast<int>(rightHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(rightHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_RTHIGH];
		DrawMoveableMesh(item, mesh, room, LM_RTHIGH, transparent);
	}

	if (backHolsterID != HolsterSlot::Empty && m_moveableObjects[static_cast<int>(backHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(backHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_TORSO];
		DrawMoveableMesh(item, mesh, room, LM_TORSO, transparent);
	}
}
