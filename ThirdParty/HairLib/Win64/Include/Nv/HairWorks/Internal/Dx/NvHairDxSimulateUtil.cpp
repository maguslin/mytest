/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairDxSimulateUtil.h"

#include <Nv/HairWorks/Internal/NvHairInstance.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/Common/NvCoComPtr.h>

namespace nvidia { 
namespace HairWorks {

/*static*/ Void DxSimulateUtil::calcMaterial(const InstanceDescriptor& desc, float unitScale, NvHair_SimulationMaterial& matOut)
{
	matOut.massScale = unitScale * desc.m_massScale;

	matOut.stiffness = desc.m_stiffness;
	matOut.damping = desc.m_damping;
	matOut.stiffnessStrength = desc.m_stiffnessStrength;
	matOut.stiffnessDamping = desc.m_stiffnessDamping;

	matOut.rootStiffness = desc.m_rootStiffness;
	matOut.tipStiffness = desc.m_tipStiffness;
	matOut.bendStiffness = desc.m_bendStiffness;
	matOut.interactionStiffness = desc.m_interactionStiffness;

	matOut.pinStiffness = desc.m_pinStiffness;
	matOut.inertiaScale = desc.m_inertiaScale;
	matOut.backStopRadius = desc.m_backStopRadius;
	matOut.friction = unitScale * desc.m_friction;

	matOut.hairNormalWeight = desc.m_hairNormalWeight;

	matOut.stiffnessCurve = desc.m_stiffnessCurve;
	matOut.stiffnessStrengthCurve = desc.m_stiffnessStrengthCurve;
	matOut.stiffnessDampingCurve = desc.m_stiffnessDampingCurve;
	matOut.bendStiffnessCurve = desc.m_bendStiffnessCurve;
	matOut.interactionStiffnessCurve = desc.m_interactionStiffnessCurve;
}

/* static */Result DxSimulateUtil::calcConstantBuffer(Instance* inst, float timeStep, Float simulationInterp, const gfsdk_float4x4* mat, NvHair_SimulateConstantBuffer* constantBuffer)
{
	NV_UNUSED(mat);

	ApiInstance* apiInst = inst->m_apiInstance;
	const Asset* asset = inst->m_asset;

	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();

	const float unitInCentimeters = asset->getUnitInCentimeters();
	const float unitInMeters = unitInCentimeters * 0.01f;
	const float unitScale = 1.0f / unitInMeters;

	// compute inertia scale based on the speed of hair body
	const float bodySpeed = gfsdk_length(inst->m_skinnedCenter - inst->m_prevSkinnedCenter);
	const float inertiaLimit = defaultMaterial.m_inertiaLimit;

	const bool lock = (bodySpeed > inertiaLimit);

	const bool useDqs = inst->m_useDqSkinning;
	const bool teleport = (inst->m_teleportMode != TeleportMode::NONE);

	constantBuffer->simulationInterp = simulationInterp;

	// copy materials
	{
		constantBuffer->materialWeight = 0.0f;

		calcMaterial(defaultMaterial, unitScale, constantBuffer->defaultMaterial);
		// Target material is still used in shaders, so we must initialize it.
		calcMaterial(defaultMaterial, unitScale, constantBuffer->targetMaterial);
	}

	// gravity and wind
	{
		float3 gravity = gfsdk_getNormalized((float3&)defaultMaterial.m_gravityDir);
		constantBuffer->gravity = gfsdk_makeFloat4(gravity.x, gravity.y, gravity.z, 0);

		constantBuffer->windNoise = defaultMaterial.m_windNoise;
		constantBuffer->wind = unitScale * defaultMaterial.m_wind;
	}

	// book keeping variables
	{
		if(mat)
			constantBuffer->modelToWorld = *mat;
		else
			gfsdk_makeIdentity(constantBuffer->modelToWorld);

		constantBuffer->modelCenter = gfsdk_makeFloat4(inst->m_shadingCenter);
		constantBuffer->modelCenterRest = gfsdk_makeFloat4(inst->m_shadingCenterRest);

		constantBuffer->numConstraintIterations = 5;
		constantBuffer->numTotalCvs = asset->m_numMasterStrandControlVertices;
		constantBuffer->maxHairLength = asset->m_maxHairLength;

		constantBuffer->timeStep = timeStep;
		constantBuffer->simulate = defaultMaterial.m_simulate && inst->m_isSimulationStarted && !teleport;

		constantBuffer->lockInertia = lock;
		constantBuffer->useDualQuaterinon = useDqs;
	}

	// textures
	{
		constantBuffer->stiffnessChannel = Int(inst->getDefaultTextureChannel(TextureType::STIFFNESS));
		constantBuffer->rootStiffnessChannel = Int(inst->getDefaultTextureChannel(TextureType::ROOT_STIFFNESS));

		constantBuffer->useStiffnessTexture = apiInst->isApiTextureUsed(TextureType::STIFFNESS);
		constantBuffer->useRootStiffnessTexture = apiInst->isApiTextureUsed(TextureType::ROOT_STIFFNESS);
		constantBuffer->useWeightTexture = apiInst->isApiTextureUsed(TextureType::WEIGHTS);
	}

	{
		NvInt numBones = asset->getNumBones();

		// copy skinning data
		numBones = (numBones > NV_HAIR_MAX_BONE_MATRICES) ? NV_HAIR_MAX_BONE_MATRICES : numBones;

		constantBuffer->numBones = numBones;
		if (numBones > 0)
		{
			if (useDqs)
				NvCo::Memory::copy(constantBuffer->skinDqs, inst->m_boneDqs, sizeof(gfsdk_dualquaternion) * numBones);
			else
				NvCo::Memory::copy(constantBuffer->skinMatrices, inst->m_skinningMatrices, sizeof(matrix4) * numBones);

			NvCo::Memory::copy(constantBuffer->boneMatrices, inst->m_boneMatrices, sizeof(gfsdk_float4x4) * numBones);
		}
	}

	{
		// copy collision data
		
		Int numCaps = (Int)inst->m_asset->m_collisionCapsuleIndices.getSize();
		Int numSpheres = (Int)inst->m_collisionSpheres.getSize();

		constantBuffer->numCollisionSpheres = numSpheres;
		constantBuffer->numCollisionCapsules = numCaps;
		constantBuffer->useCollision = defaultMaterial.m_useCollision;

		if (numSpheres > 0)
		{
			NvCo::Memory::copy(constantBuffer->collisionSpheres, inst->m_collisionSpheres, sizeof(float4) * numSpheres);
		}
		if (numCaps > 0)
		{
			NvCo::Memory::copy(constantBuffer->collisionCapsuleIndex, inst->m_asset->m_collisionCapsuleIndices, sizeof(float4) * numCaps);
		}
	}

	// copy pin constraint data
	{
		constantBuffer->numPinConstraints = inst->getNumPins();
		constantBuffer->useDynamicPin = defaultMaterial.m_useDynamicPin;
		//constantBuffer->useDynamicPin		= true; // disable for 1.1
	}

	return NV_OK;
}


} // namespace HairWorks
} // namespace nvidia

