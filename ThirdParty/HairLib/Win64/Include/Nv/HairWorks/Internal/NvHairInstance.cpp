/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairInstance.h"

#include <Nv/HairWorks/Internal/NvHairLineRenderer.h>

namespace nvidia {
namespace HairWorks { 

Instance::Instance(Asset* asset) :
	m_isActive(false),
	m_instanceId(INSTANCE_ID_NULL),
	m_teleportMode(TeleportMode::NONE),
	m_cacheEntry(NV_NULL),
	m_asset(asset),
	m_isSimulationStarted(false),
	m_isTessellated(false),
	m_simulationInterp(0.0f)
{
	setNumVertsPerSegment(MAX_VERTS_PER_SEGMENT);

	// set matrices
	gfsdk_makeIdentity(m_modelToWorld);

	// Set up the bind pose as initial configuration and assume using matrix skinning
	{
		m_useDqSkinning = false;
		const gfsdk_float4x4* bindPoseMatrices = asset->getBindPoseMatrices();
		Int numBones = asset->getNumBones();
		if (bindPoseMatrices && numBones > 0)
		{
			m_boneMatrices.set(bindPoseMatrices, numBones);
			m_skinningMatrices.setSize(numBones);
			// Initialize skinning matrices
			for (IndexT i = 0; i < numBones; i ++)
			{
				gfsdk_makeIdentity(m_skinningMatrices[i]);
			}
		}

		{
			// Make sure there is space for the spheres
			const AssetDescriptor& desc = asset->getDesc();
			m_collisionSpheres.setSize(desc.m_numBoneSpheres);
		}
		// We'll use the bind pose/matrices to work out initial collision 
		_updateMatrixCollision();
	}

	// Set the reasonable defaults
	m_skinnedBounds[0] = asset->m_initialModelBounds[0];
	m_skinnedBounds[1] = asset->m_initialModelBounds[1];
	m_skinnedCenter = asset->m_initialModelCenter;
	m_prevSkinnedCenter = m_skinnedCenter;

	// Some reasonable defaults?
	m_shadingCenter = m_skinnedCenter;
	m_shadingCenterRest = m_skinnedCenter;

	// Add one to the amount of instances referencing this asset
	asset->m_numInstances++;
}


Instance::~Instance()
{
	// There is one less instance referencing the asset
	if (m_asset)
	{
		m_asset->m_numInstances--;
	}
}

Int Instance::getStrandPointCount() const
{
	int numVertsPerSegments = m_numVertsPerSegment;
	int numMasterSegments = m_asset->m_numMasterSegmentsPerHair;
	return numVertsPerSegments * numMasterSegments + 1;
}

Int Instance::getMaxStrandPointCount() const
{
	int numVertsPerSegments = MAX_VERTS_PER_SEGMENT;
	int numMasterSegments = m_asset->m_numMasterSegmentsPerHair;
	return numVertsPerSegments * numMasterSegments + 1;
}

Int Instance::getNumTessellatedPoints() const
{
	int strandPointCount = getStrandPointCount();
	return strandPointCount * m_asset->m_numMasterStrands;
}

Int Instance::getMaxTessellatedPoints() const
{
	int strandPointCount = getMaxStrandPointCount();
	return strandPointCount * m_asset->m_numMasterStrands;
}

void Instance::setNumVertsPerSegment(Int v)
{
	if (v >= 1 && v <= MAX_VERTS_PER_SEGMENT)
	{
		if (v != m_numVertsPerSegment)
		{
			m_numVertsPerSegment = v;

			if (m_apiInstance)
			{
				m_apiInstance->onNumVertsPerSegmentChanged();
			}
		}
	}
}

Void Instance::_updateShadingCenter()
{
	// shading bones
	const Int numBones = m_asset->getNumBones();
	const gfsdk_float4x4* bindPoseMatrices = m_asset->getBindPoseMatrices();

	const InstanceDescriptor& params = getDefaultMaterial();
	const Int index = Int(params.m_hairNormalBoneIndex);
	if (index >= 0 && index < numBones)
	{
		gfsdk_float3 offset = gfsdk_makeFloat3(0, 0, 0);

		const gfsdk_float4x4& bone = m_boneMatrices[index];
		const gfsdk_float4x4& bindPose = bindPoseMatrices[index];

		m_shadingCenter = gfsdk_transformCoord(bone, offset);
		m_shadingCenterRest = gfsdk_transformCoord(bindPose, offset);
	}
	else
	{
		m_shadingCenter = m_skinnedCenter;
	}
}

Void Instance::updateSkinningMatrices(const gfsdk_float4x4* boneMatrices)
{
	m_useDqSkinning = false;

	const Int numBones = m_asset->getNumBones();

	// copy skinning matrices
	if (numBones > 0)
	{
		m_skinningMatrices.setSize(numBones);
		NvCo::Memory::copy(m_skinningMatrices, boneMatrices, sizeof(gfsdk_float4x4) * numBones);
	}

	// compute bounding box
	_updateSkinnedBounds();

	const gfsdk_float4x4* bindPoseMatrices = m_asset->getBindPoseMatrices();

	// update bone matrices for debug rendering
	for (Int i = 0; i < numBones; ++i)
	{
		const gfsdk_float4x4& skin = m_skinningMatrices[i];
		const gfsdk_float4x4& pose = bindPoseMatrices[i];
		
		m_boneMatrices[i] = pose * skin;
	}

	_updateMatrixCollision();
	_updateShadingCenter();
}

Void Instance::_updateMatrixCollision()
{
	const AssetDescriptor& desc = m_asset->getDesc();
	const Int numSpheres = desc.m_numBoneSpheres;
	const gfsdk_float4x4* bindPoseMatrices = m_asset->getBindPoseMatrices();
	const Int numBones = m_asset->getNumBones();

	// compute position of collision spheres
	for (Int i = 0; i < numSpheres; i++)
	{
		const BoneSphere& srcSphere = desc.m_boneSpheres[i];

		UInt32 index = srcSphere.m_boneSphereIndex;
		NV_CORE_ASSERT(index < UInt32(numBones));

		Float radius = srcSphere.m_boneSphereRadius;

		const gfsdk_float4x4& skin = m_skinningMatrices[index];
		const gfsdk_float4x4& pose = bindPoseMatrices[index];

		gfsdk_float3 poseP = gfsdk_transformCoord(pose, srcSphere.m_boneSphereLocalPos);
		gfsdk_float3 sphereP = gfsdk_transformCoord(skin, poseP);
		m_collisionSpheres[i] = gfsdk_makeFloat4(sphereP, radius);
	}	
}

Void Instance::updateSkinningDqs(const gfsdk_dualquaternion* dqs)
{
	const Int numBones = m_asset->getNumBones();
	m_useDqSkinning = true;

	if (numBones > 0)
	{
		m_boneDqs.setSize(numBones);
		NvCo::Memory::copy(m_boneDqs, dqs, sizeof(gfsdk_dualquaternion) * numBones);
	}

	const gfsdk_float4x4* bindPoseMatrices = m_asset->getBindPoseMatrices();

	// update bone matrices for debug rendering
	for (Int i = 0; i < numBones; i++)
	{
		const gfsdk_float4x4& pose = bindPoseMatrices[i];
		const gfsdk_dualquaternion& skinDq = m_boneDqs[i];
		m_boneMatrices[i] = pose * gfsdk_makeTransform(skinDq);
	}

	_updateDqCollision();
	_updateSkinnedBounds();
	_updateShadingCenter();
}

Void Instance::_updateDqCollision()
{
	const AssetDescriptor& desc = m_asset->getDesc();
	const Int numSpheres = desc.m_numBoneSpheres;
	const gfsdk_float4x4* bindPoseMatrices = m_asset->getBindPoseMatrices();
	const Int numBones = m_asset->getNumBones();

	for (Int i = 0; i < numSpheres; i++)
	{
		const BoneSphere& srcSphere = desc.m_boneSpheres[i];

		const UInt32 index = srcSphere.m_boneSphereIndex;
		NV_CORE_ASSERT(index < UInt32(numBones));

		const Float radius = srcSphere.m_boneSphereRadius;

		const gfsdk_dualquaternion& skinDq = m_boneDqs[index];
		const gfsdk_float4x4& pose = bindPoseMatrices[index];

		gfsdk_float3 poseP = gfsdk_transformCoord(pose, srcSphere.m_boneSphereLocalPos);
		gfsdk_float3 sphereP = gfsdk_transformCoord(skinDq, poseP);

		m_collisionSpheres[i] = gfsdk_makeFloat4(sphereP, radius);
	}
}

Void Instance::_updateSkinnedBounds()
{
	const Asset* asset = m_asset;
	gfsdk_float3& bbMin = m_skinnedBounds[0];
	gfsdk_float3& bbMax = m_skinnedBounds[1];

	if(m_useDqSkinning)
		asset->calcSkinnedGrowthMeshBounds(m_boneDqs, bbMin, bbMax);
	else
		asset->calcSkinnedGrowthMeshBounds(m_skinningMatrices, bbMin, bbMax);

	m_prevSkinnedCenter = m_skinnedCenter;
	m_skinnedCenter = 0.5f * (bbMin + bbMax);
}

Void Instance::drawDebugBones(LineRenderer* renderer) const
{	
	float sceneUnit = m_asset->getUnitInCentimeters();

	const gfsdk_float4x4& modelToWorld = m_modelToWorld;

	LineRenderer::Scope scope(renderer);
	
	renderer->setDepthOp(DepthOp::ALWAYS);

	renderer->setColor(gfsdk_makeFloat3(1, 1, 1));
	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 0, 1));

	const Int numBones = m_asset->getNumBones();
	for (Int i = 0; i < numBones; i++)
	{
		float boneDisplayScale = 4.0f / sceneUnit;

		const gfsdk_float4x4& bone = m_boneMatrices[i];
		gfsdk_float3 boneMatrixScale = gfsdk_getScale(bone);
		gfsdk_float3 boneScale = gfsdk_makeFloat3(boneDisplayScale / boneMatrixScale.x, boneDisplayScale / boneMatrixScale.y, boneDisplayScale / boneMatrixScale.z);
		gfsdk_float4x4 scale, transform, model;

		// draw sphere
		const float boneSphereScale = 0.1f;
		boneScale = boneSphereScale * boneScale;

		gfsdk_makeScale(scale, boneScale);
		transform = scale * bone;
		model = transform * modelToWorld;

		renderer->setModelToWorldMatrix(model);
		renderer->drawSphere();
	}

	// Draw the parent lines, and the axis
	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 1, 1));
	renderer->setColor(gfsdk_makeFloat3(1, 0, 1));
	renderer->setModelToWorldMatrix(modelToWorld);
	for (Int i = 0; i < numBones; i++)
	{
		const gfsdk_float4x4& bone = m_boneMatrices[i];
		float boneDisplayScale = 4.0f / sceneUnit;
		const gfsdk_float3 boneMatrixScale = gfsdk_getScale(bone);
		gfsdk_float3 boneScale = gfsdk_makeFloat3(boneDisplayScale / boneMatrixScale.x, boneDisplayScale / boneMatrixScale.y, boneDisplayScale / boneMatrixScale.z);
		
		renderer->drawAxis(bone, boneScale);

		// draw parent
		const Int* boneParents = m_asset->getBindPoseParents();
		Int parent = boneParents[i];
		if ((parent < 0) || (parent >= numBones))
			continue;

		const gfsdk_float4x4& parentBone = m_boneMatrices[parent];
		gfsdk_float3 from = gfsdk_makeFloat3(bone._41, bone._42, bone._43);
		gfsdk_float3 to = gfsdk_makeFloat3(parentBone._41, parentBone._42, parentBone._43);

		renderer->drawLine(from, to);
	}
}

Void Instance::drawDebugCapsules(LineRenderer* renderer) const
{
	typedef BoneSphere BoneSphere;

	LineRenderer::Scope scope(renderer);
	
	const gfsdk_float4x4& modelToWorld = m_modelToWorld;

	gfsdk_float4x4 sphereMatrices[1024];

	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 0, 1));
	renderer->setColor(gfsdk_makeFloat3(1, 1, 1));

	// Draw a sphere at all the bones
	{
		const AssetDescriptor& desc = m_asset->getDesc();
		const Int numSpheres = desc.m_numBoneSpheres;

		for (Int i = 0; i < numSpheres; i++)
		{
			const BoneSphere& srcSphere = desc.m_boneSpheres[i];
			const Int boneIndex = Int(srcSphere.m_boneSphereIndex);

			float radius = srcSphere.m_boneSphereRadius;
			gfsdk_float3 offset = srcSphere.m_boneSphereLocalPos;

			const gfsdk_float4x4& bone = m_boneMatrices[boneIndex];

			gfsdk_float4x4 scale, trans, transform, model;

			gfsdk_makeScale(scale, gfsdk_makeFloat3(radius, radius, radius));
			gfsdk_makeTranslation(trans, offset);

			transform = (scale * trans) * bone;
			model = transform * modelToWorld;

			sphereMatrices[i] = transform;

			renderer->setModelToWorldMatrix(model);
			renderer->drawSphere();
		}
	}

	renderer->setModelToWorldMatrix(m_modelToWorld);
	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 1, 1));
	renderer->setColor(gfsdk_makeFloat3(1, 0, 1));
	{
		const int numSegments = 16;
		float cx[16];
		float cy[16];
		for (int i = 0; i < numSegments; i++)
		{
			float theta = 2.0f * 3.1415923f * i / float(numSegments);
			cx[i] = cosf(theta);
			cy[i] = sinf(theta);
		}

		gfsdk_float4x4 identity;
		gfsdk_makeIdentity(identity);

		const Asset* asset = m_asset;
		const Int numCaps = NvInt(asset->m_collisionCapsuleIndices.getSize());

		const AssetDescriptor& desc = m_asset->getDesc();
		const Int numSpheres = desc.m_numBoneSpheres;
		const BoneSphere* srcSpheres = desc.m_boneSpheres;

		for (Int i = 0; i < numCaps; i++)
		{
			const gfsdk_float4& indices = asset->m_collisionCapsuleIndices[i];
			Int startIndex = Int(indices.x);
			Int endIndex = Int(indices.y);

			if (startIndex >= numSpheres || endIndex >= numSpheres)
				continue;

			const BoneSphere& startSphere = srcSpheres[startIndex];
			const BoneSphere& endSphere = srcSpheres[endIndex];

			const gfsdk_float4x4& startMat = sphereMatrices[startIndex];
			const gfsdk_float4x4& endMat = sphereMatrices[endIndex];

			gfsdk_float3 start = gfsdk_makeFloat3(startMat._41, startMat._42, startMat._43);
			gfsdk_float3 end = gfsdk_makeFloat3(endMat._41, endMat._42, endMat._43);
			gfsdk_float3 dir = end - start;

			float d = gfsdk_length(dir);
			// Normalize
			if (d > 0.0f)
			{
				dir = dir * (1.0f / d);
			}

			gfsdk_float3 xAxis = gfsdk_makeFloat3(1, 0, 0);
			gfsdk_float3 yAxis = gfsdk_makeFloat3(0, 1, 0);
			gfsdk_float3 axis1, axis2;
			if (fabs(gfsdk_dot(dir, xAxis)) < fabs(gfsdk_dot(dir, yAxis)))
				axis1 = xAxis;
			else
				axis1 = yAxis;

			axis1 = gfsdk_getNormalized(gfsdk_getNormalized(gfsdk_cross(dir, axis1)));
			axis2 = gfsdk_getNormalized(gfsdk_cross(dir, axis1));

			const

			float startRadius = startSphere.m_boneSphereRadius;
			float endRadius = endSphere.m_boneSphereRadius;
			float deltaRadius = startRadius - endRadius;

			float slope = gfsdk_sqrt(d * d + deltaRadius * deltaRadius);
			float startShift = startRadius * deltaRadius / slope;
			float endShift = endRadius * deltaRadius / slope;

			float startRadiusScaled = gfsdk_sqrt(startRadius * startRadius - startShift * startShift);
			float endRadiusScaled = gfsdk_sqrt(endRadius * endRadius - endShift * endShift);

			for (int j = 0; j < numSegments; j++)
			{
				gfsdk_float3 startPos = start + startRadiusScaled * (cx[j] * axis1 + cy[j] * axis2) + startShift * dir;
				gfsdk_float3 endPos = end + endRadiusScaled * (cx[j] * axis1 + cy[j] * axis2) + endShift * dir;

				renderer->drawLine(startPos, endPos);
			}
		}
	}
}

Void Instance::getBounds(gfsdk_float3& minOut, gfsdk_float3& maxOut, Bool growthMeshOnly) const
{
	if (!growthMeshOnly)
	{
		// Add the maximum hair distance from the center to the current center for bounding size
		float d = m_asset->m_maxHairCenterDist;
		minOut = m_skinnedCenter - gfsdk_makeFloat3(d, d, d);
		maxOut = m_skinnedCenter + gfsdk_makeFloat3(d, d, d);
	}
	else
	{
		minOut = m_skinnedBounds[0];
		maxOut = m_skinnedBounds[1];
	}
}

Void Instance::drawDebugBoundingBox(LineRenderer* renderer) const
{
	LineRenderer::Scope scope(renderer);
	
	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 1, 1));
	renderer->setModelToWorldMatrix(m_modelToWorld);

	{
		gfsdk_float3 min, max;
		getBounds(min, max, true);

		renderer->setColor(gfsdk_makeFloat3(1, 1, 0));
		renderer->drawBoundingBox(min, max);
	}

	{
		gfsdk_float3 min, max;
		getBounds(min, max, false);

		renderer->setColor(gfsdk_makeFloat3(1, 0, 1));
		renderer->drawBoundingBox(min, max);
	}
	{
		gfsdk_float3 c = m_shadingCenter;
		const Float lineSize = m_asset->m_modelSize * 0.05f;
		gfsdk_float3 cmin = c - gfsdk_makeFloat3(lineSize, lineSize, lineSize);
		gfsdk_float3 cmax = c + gfsdk_makeFloat3(lineSize, lineSize, lineSize);

		renderer->setColor(gfsdk_makeFloat3(0, 1, 0));

		renderer->drawLine(gfsdk_makeFloat3(cmin.x, c.y, c.z), gfsdk_makeFloat3(cmax.x, c.y, c.z));
		renderer->drawLine(gfsdk_makeFloat3(c.x, cmin.y, c.z), gfsdk_makeFloat3(c.x, cmax.y, c.z));
		renderer->drawLine(gfsdk_makeFloat3(c.x, c.y, cmin.z), gfsdk_makeFloat3(c.x, c.y, cmax.z));
	}
}

Void Instance::drawDebugShadingBone(LineRenderer* renderer) const
{
	const Int numBones = m_asset->getNumBones();
	const InstanceDescriptor& params = getDefaultMaterial();
	if (params.m_hairNormalBoneIndex < 0 || params.m_hairNormalBoneIndex >= numBones)
		return;

	LineRenderer::Scope scope(renderer);
	
	renderer->setDepthOp(DepthOp::ALWAYS);
	renderer->setColor(gfsdk_makeFloat3(1, 1, 1));
	renderer->setGlobalColor(gfsdk_makeFloat4(0.5, 0.5, 0.5, 1));

	float sceneUnit = m_asset->getUnitInCentimeters();
	const gfsdk_float4x4& modelToWorld = m_modelToWorld;
	const gfsdk_float4x4& bone = m_boneMatrices[params.m_hairNormalBoneIndex];
	
	gfsdk_float4x4 scale, transform, model;
	float boneScale = 2.0f / sceneUnit;

	gfsdk_makeScale(scale, gfsdk_makeFloat3(boneScale, boneScale, boneScale));

	transform = scale  * bone;
	model = transform * modelToWorld;
	
	renderer->setModelToWorldMatrix(model);
	renderer->drawSphere();
}

Void Instance::drawDebugCullSphere(LineRenderer* renderer) const
{
	const InstanceDescriptor& param = getDefaultMaterial();

	LineRenderer::Scope scope(renderer);
	
	gfsdk_float4x4 modelToWorld = gfsdk_inverse(param.m_cullSphereInvTransform);
	
	renderer->setModelToWorldMatrix(modelToWorld);
	renderer->setGlobalColor(gfsdk_makeFloat4(1, 1, 0, 1));
	renderer->drawSphere();
}

Void Instance::computeStats(Float densitySum, Stats& statsOut) const
{
	const LodInfo& lodInfo = m_lodInfo;
	const Asset* asset = m_asset;

	const InstanceDescriptor& params = getDefaultMaterial();

	float lodDistanceFactor = lodInfo.m_distanceFactor;
	float lodDetailFactor = lodInfo.m_detailFactor;

	if (lodDistanceFactor > 0.0f)
	{
		float lodDensity = gfsdk_lerp(params.m_density, params.m_distanceLodDensity, lodDistanceFactor);
		float densityScale = lodDensity / params.m_density;
		densitySum *= densityScale;
	}

	if (lodDetailFactor > 0.0f)
	{
		float lodDensity = gfsdk_lerp(params.m_density, params.m_detailLodDensity, lodDetailFactor);
		float densityScale = lodDensity / params.m_density;
		densitySum *= densityScale;
	}

	NvCo::Memory::zero(&statsOut, sizeof(Stats));

	statsOut.m_averageNumCvsPerHair = asset->m_numMasterStrandControlVertices / float(asset->m_numMasterStrands);
	statsOut.m_numHairs = int(densitySum * NV_HAIR_NUM_HAIRS_PER_FACE);
	statsOut.m_numFaces = asset->m_numFaces;
	statsOut.m_averageDensity = densitySum / float(asset->m_numFaces);
	statsOut.m_averageHairsPerFace = statsOut.m_numHairs / float(asset->m_numFaces);

	statsOut.m_distanceLodFactor = lodDistanceFactor;
	statsOut.m_detailLodFactor = lodDetailFactor;
	statsOut.m_camDistance = lodInfo.m_distance;
}


Result Instance::preRender(Float simulationInterp)
{
	ApiInstance* apiInst = m_apiInstance;
	if (!apiInst)
	{
		NV_CO_LOG_ERROR("Internal buffer was not properly initialized.");
		return NV_FAIL;
	}
	if (!needsPreRender(simulationInterp))
	{
		// Do nothing
		return NV_OK;
	}
		
	NV_RETURN_ON_FAIL( apiInst->preRender(simulationInterp));

	m_isTessellated = true;
	m_simulationInterp = simulationInterp;
	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia
