/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairAssetDescriptorUtil.h"

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/NvCoLogger.h>

namespace nvidia {
namespace HairWorks {

/* static */Result AssetDescriptorUtil::check(const Descriptor& desc, NvCo::Logger* log)
{
	log = log ? log : NvCo::Logger::getIgnoreLogger();
	if (desc.m_numGuideHairs == 0)
	{
		log->logError("hair asset does not have any guide hair.");
		return NV_FAIL;
	}
	if (desc.m_numVertices == 0)
	{
		log->logError("hair asset does not have any control vertices.");
		return NV_FAIL;
	}
	if (!desc.m_vertices)
	{
		log->logError("hair asset does not have valid pointer for control vertices.");
		return NV_FAIL;
	}
	if (!desc.m_numFaces)
	{
		log->logError("hair asset does not have growth mesh face.  At least one triangle is needed.");
		return NV_FAIL;
	}
	if (!desc.m_faceIndices)
	{
		log->logError("hair asset does not have valid pointer for face indices.");
		return NV_FAIL;
	}
	if (!desc.m_faceUvs)
	{
		log->logError("hair asset does not have valid pointer for face uvs.");
		return NV_FAIL;
	}
	if (desc.m_numBones > 0)
	{
		if (!desc.m_boneIndices)
		{
			log->logError("hair asset has non-zero bones, but does not have valid pointer for bone indices.");
			return NV_FAIL;
		}
		if (!desc.m_boneWeights)
		{
			log->logError("hair asset has non-zero bones, but does not have valid pointer for bone weights.");
			return NV_FAIL;
		}
	}
	if (desc.m_numBoneSpheres > 0)
	{
		if (!desc.m_boneSpheres)
		{
			log->logError("hair asset has non-zero bone spheres, but does not have valid pointer.");
			return NV_FAIL;
		}
	}
	if (desc.m_numBoneCapsules > 0)
	{
		if (!desc.m_boneCapsuleIndices)
		{
			log->logError("hair asset has non-zero bone capsules, but does not have valid pointer.");
			return NV_FAIL;
		}
	}
	if (desc.m_numPins > 0)
	{
		if (!desc.m_pins)
		{
			log->logError("hair asset has non-zero pin constraints, but does not have valid pointer.");
			return NV_FAIL;
		}
	}

	// check if indices have same number of cvs
	Int prevCvs, currentCvs;
	prevCvs = currentCvs = desc.m_endIndices[0] + 1;
	for (Int i = 1; i < Int(desc.m_numGuideHairs); i++)
	{
		currentCvs = desc.m_endIndices[i] - desc.m_endIndices[i - 1];
		if (prevCvs != currentCvs)
		{
			log->logError("number of cvs are different in the same asset. This is not allowed. Please resample hairs in DCC.");
			return NV_FAIL;
		}
	}
	return NV_OK;
}

/* static */Result AssetDescriptorUtil::convertUnits(Descriptor& desc, Float sceneUnit)
{
	if (sceneUnit == 0.0f) // unit conversion is disabled
		return NV_OK;

	float assetUnit = desc.m_sceneUnit;
	float scale = assetUnit / sceneUnit;

	if (scale == 1.0f)
		return NV_OK; // no need for unit conversion

	// convert vertex data
	for (Int i = 0; i < (Int)desc.m_numVertices; i++)
	{
		desc.m_vertices[i] = scale * desc.m_vertices[i];
	}

	// convert bind pose data
	if (desc.m_bindPoses)
	{
		gfsdk_float4x4 scaleMatrix;
		gfsdk_makeScale(scaleMatrix, gfsdk_makeFloat3(scale, scale, scale));

		for (Int i = 0; i < (Int)desc.m_numBones; i++)
		{
			gfsdk_float4x4& bindPose = desc.m_bindPoses[i];
			bindPose._41 *= scale;
			bindPose._42 *= scale;
			bindPose._43 *= scale;
		}
	}

	// convert collision capsule data
	if ((desc.m_numBoneSpheres > 0) &&  (desc.m_boneSpheres))
	{
		for (int i = 0; i < (int)desc.m_numBoneSpheres; ++i)
		{
			BoneSphere& sphere = desc.m_boneSpheres[i];
			sphere.m_boneSphereLocalPos = scale * sphere.m_boneSphereLocalPos;
			sphere.m_boneSphereRadius *= scale;
		}
	}

	// convert pin constraint data
	if ((desc.m_numPins > 0) &&  (desc.m_pins))
	{
		for (Int i = 0; i < (Int)desc.m_numPins; ++i)
		{
			Pin& pin = desc.m_pins[i];
			pin.m_localPos = scale * pin.m_localPos;
			pin.m_radius *= scale;
		}
	}
	// write new scene unit
	desc.m_sceneUnit = sceneUnit;
	return NV_OK;
}

/* static */Void AssetDescriptorUtil::calcConversionMatrix(EAxisHint srcAxisHint, EHandednessHint srcHandedness, EAxisHint dstAxisHint, EHandednessHint dstHandedness, gfsdk_float4x4& matOut)
{
	gfsdk_makeIdentity(matOut);

	if (srcAxisHint == AxisHint::UNKNOWN ||
		dstAxisHint == AxisHint::UNKNOWN ||
		srcHandedness == HandednessHint::UNKNOWN ||
		dstHandedness == HandednessHint::UNKNOWN)
	{
		return;
	}

	Int sourceCode = 0, targetCode = 0;

	if ((srcAxisHint == AxisHint::Y_UP) && (srcHandedness == HandednessHint::RIGHT))
		sourceCode = 0;
	else if ((srcAxisHint == AxisHint::Z_UP) && (srcHandedness == HandednessHint::RIGHT))
		sourceCode = 1;
	else if ((srcAxisHint == AxisHint::Y_UP) && (srcHandedness == HandednessHint::LEFT))
		sourceCode = 2;
	else if ((srcAxisHint == AxisHint::Z_UP) && (srcHandedness == HandednessHint::LEFT))
		sourceCode = 3;

	if ((dstAxisHint == AxisHint::Y_UP) && (dstHandedness == HandednessHint::RIGHT))
		targetCode = 0;
	else if ((dstAxisHint == AxisHint::Z_UP) && (dstHandedness == HandednessHint::RIGHT))
		targetCode = 1;
	else if ((dstAxisHint == AxisHint::Y_UP) && (dstHandedness == HandednessHint::LEFT))
		targetCode = 2;
	else if ((dstAxisHint == AxisHint::Z_UP) && (dstHandedness == HandednessHint::LEFT))
		targetCode = 3;

	static const Float table[][16] = 
	{
		// from Y-up r.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to y-up r.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0,-1, 0, 0,		0, 0, 0, 1}, // to z-up r.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0,-1, 0,		0, 0, 0, 1}, // to y-up l.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to z-up l.h.s

		// from Z-up r.h.s
		{ 1, 0, 0, 0,	0, 0,-1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to y-up r.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to z-up r.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to y-up l.h.s
		{ 1, 0, 0, 0,	0,-1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to z-up l.h.s

		// from Y-up l.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0,-1, 0,		0, 0, 0, 1}, // to y-up r.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to z-up r.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to y-up l.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0,-1, 0, 0,		0, 0, 0, 1}, // to z-up l.h.s

		// from Z-up l.h.s
		{ 1, 0, 0, 0,	0, 0, 1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to y-up r.h.s
		{ 1, 0, 0, 0,	0,-1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to z-up r.h.s
		{ 1, 0, 0, 0,	0, 0,-1, 0,		0, 1, 0, 0,		0, 0, 0, 1}, // to y-up l.h.s
		{ 1, 0, 0, 0,	0, 1, 0, 0,		0, 0, 1, 0,		0, 0, 0, 1}, // to z-up l.h.s
	};
	Int matrixCode = sourceCode * 4 + targetCode;
	gfsdk_makeFloat4x4(matOut, table[matrixCode]);
}

Void AssetDescriptorUtil::transform(const gfsdk_float4x4& mat, Bool flipWinding, Descriptor& desc)
{
	// convert vertex data
	for (Int i = 0; i < (Int)desc.m_numVertices; i++)
	{
		desc.m_vertices[i] = gfsdk_transformCoord(mat, desc.m_vertices[i]);
	}

	// convert bind pose data
	if (desc.m_bindPoses)
	{
		for (Int i = 0; i < (Int)desc.m_numBones; i++)
		{
			gfsdk_float4x4 bindPoseIn = desc.m_bindPoses[i];
			desc.m_bindPoses[i] = bindPoseIn * mat;
		}
	}

	// if handedness changes, winding should change for correct normal etc.
	if (flipWinding)
	{
		for (Int i = 0; i < (Int)desc.m_numFaces; i++)
		{
			UInt32 v1 = desc.m_faceIndices[i * 3 + 1];
			UInt32 v2 = desc.m_faceIndices[i * 3 + 2];

			// flip indices
			desc.m_faceIndices[i * 3 + 1] = v2;
			desc.m_faceIndices[i * 3 + 2] = v1;

			gfsdk_float2 uv1 = desc.m_faceUvs[i * 3 + 1];
			gfsdk_float2 uv2 = desc.m_faceUvs[i * 3 + 2];

			// flip uvs
			desc.m_faceUvs[i * 3 + 1] = uv2;
			desc.m_faceUvs[i * 3 + 2] = uv1;
		}
	}
}

/* static */Void AssetDescriptorUtil::convert(const ConversionSettings& settings, Descriptor& desc)
{
	const EAxisHint sourceUpAxisHint = desc.m_upAxis;
	const EHandednessHint sourceHandednessHint = desc.m_handedness;
	const EAxisHint targetUpAxisHint = settings.m_targetUpAxisHint;
	const EHandednessHint targetHandednessHint = settings.m_targetHandednessHint;

	gfsdk_float4x4 mat;

	if (settings.m_conversionMatrix)
		mat = *settings.m_conversionMatrix;
	else
		calcConversionMatrix(sourceUpAxisHint, sourceHandednessHint, targetUpAxisHint, targetHandednessHint, mat);

	const Bool flipWinding = (sourceHandednessHint != targetHandednessHint) &&
		(sourceHandednessHint != HandednessHint::UNKNOWN) &&
		(targetHandednessHint != HandednessHint::UNKNOWN);
	
	// Transform
	transform(mat, flipWinding, desc);

	// mark new coordinate systems
	if (targetHandednessHint != HandednessHint::UNKNOWN)
		desc.m_handedness = targetHandednessHint;

	if (targetUpAxisHint != AxisHint::UNKNOWN)
		desc.m_upAxis = targetUpAxisHint;
}

/* static */void AssetDescriptorUtil::resampleCurve(const gfsdk_float3* srcPoints, Int numSrcPoints, Float* normalizedSrcLengthsOut, gfsdk_float3* targetPoints, Int numTargetPoints)
{
	Float* accLengths = normalizedSrcLengthsOut;

	Float totalLength = 0.0f;
	for (Int i = 0; i < numSrcPoints; i++)
	{
		float length = (i == 0) ? 0.0f : gfsdk_length(srcPoints[i] - srcPoints[i - 1]);
		totalLength += length;
		accLengths[i] = totalLength;
	}

	// Normalize the lengths
	{
		// Special case if the length is close to zero
		Float recipTotalLength = (totalLength < FLT_EPSILON) ? 0.0f : (1.0f / totalLength);
		for (Int i = 0; i < numSrcPoints; i++)
			accLengths[i] *= recipTotalLength;
		accLengths[numSrcPoints] = 1.0f;
	}

	// If the totalLength is small -> treat as if in the same position
	if (totalLength < FLT_EPSILON)
	{
		for (Int i = 0; i < numTargetPoints; i++)
			targetPoints[i] = srcPoints[0];
		return;
	}

	const Float recipNumTargetPointsMinusOne = 1.0f / ( numTargetPoints - 1);
	Float t = 0.0f;
	for (Int j = 0, i = 0; i < numTargetPoints; i++, t += recipNumTargetPointsMinusOne)
	{
		// Step j if t has moved enough
		while (j < numSrcPoints && t >= accLengths[j])
		{
			j++;
		}
		if (j >= numSrcPoints)
		{
			// If past end, just copy the end point from the source
			for (; i < numTargetPoints; i++)
			{
				targetPoints[i] = srcPoints[numSrcPoints - 1];
			}
			break;
		}
		// Lerp
		gfsdk_float3 p0 = srcPoints[j - 1];
		gfsdk_float3 p1 = srcPoints[j];
		float delta = (t - accLengths[j - 1]) / (accLengths[j] - accLengths[j - 1] + FLT_EPSILON);

		targetPoints[i] = p0 + delta * (p1 - p0);
	}
}

/* static */Int AssetDescriptorUtil::calcMaxNumPointsPerHair(const UInt32* endIndices, Int numGuideHairs)
{
	Int maxSrcPoints = 0;
	for (Int i = 0; i < numGuideHairs; i++)
	{
		const Int start = Int((i == 0) ? 0 : endIndices[i - 1] + 1);
		const Int end = Int(endIndices[i]);
		const Int numSrcPoints = end - start + 1;
		maxSrcPoints = maxSrcPoints < numSrcPoints ? numSrcPoints : maxSrcPoints;
	}
	return maxSrcPoints;
}

/* static */void AssetDescriptorUtil::resampleGuideHairs(const gfsdk_float3* vertices, Int numGuideHairs, Int numPointsPerHair, UInt32* endIndicesInOut, gfsdk_float3* verticesOut)
{
	// The number of output vertices needed
	//Int numTargetVertices = numPointsPerHair * numGuideHairs;

	const UInt32* endIndices = endIndicesInOut;
	const Int maxSrcPointsPerHair = calcMaxNumPointsPerHair(endIndices, numGuideHairs);

	// Buffer to store normalized lengths
	NvCo::PodBuffer<Float> normalizedLengths(maxSrcPointsPerHair + 1);
	for (Int i = 0; i < numGuideHairs; i++)
	{
		Int start = Int((i == 0) ? 0 : endIndices[i - 1] + 1);
		Int end = Int(endIndices[i]);

		gfsdk_float3* targetPoints = verticesOut + i * numPointsPerHair;
		const gfsdk_float3* srcPoints = vertices + start;

		const Int numSrcPoints = end - start + 1;
		
		// fill in resampled vertex data
		resampleCurve(srcPoints, numSrcPoints, normalizedLengths, targetPoints, numPointsPerHair);
	}

	for (Int i = 0; i < numGuideHairs; i++)
	{
		endIndicesInOut[i] = (i + 1) * numPointsPerHair - 1;
	}
}


Result AssetDescriptorUtil::remapBones(Descriptor& desc, const Char*const* boneNames, Int numNewBones)
{
	// cannot find bone names
	if (!desc.m_boneNames)
		return NV_FAIL;
	const IndexT numOldBones = IndexT(desc.m_numBones);
	// cannot have more bones than original
	if (numNewBones > numOldBones)
		return NV_FAIL;

	// create bone remapping table
	NvCo::PodBuffer<Int> indexOldToNewBuf(numOldBones);
	NvCo::PodBuffer<Int> indexNewToOldBuf(numNewBones);

	Int* indexOldToNew = indexOldToNewBuf;
	Int* indexNewToOld = indexNewToOldBuf;

	// Mark all as -1
	NvCo::Memory::set(indexOldToNew, -1, sizeof(Int) * numOldBones);
	NvCo::Memory::set(indexNewToOld, -1, sizeof(Int) * numNewBones);
	
	// may be we can use std::map?
	for (IndexT i = 0; i < numOldBones; i++)
	{
		const char* oldBoneName = desc.m_boneNames + i * NV_HAIR_MAX_STRING;
		for (IndexT j = 0; j < numNewBones; j++)
		{
			if (!strcmp(boneNames[j], oldBoneName))
			{
				indexOldToNew[i] = Int32(j);
				indexNewToOld[j] = Int32(i);
				break;
			}
		}
	}

	{
		// This can all be done inplace
		// copy bone index
		for (IndexT i = 0; i < Int(desc.m_numGuideHairs); i++)
		{
			for (IndexT j = 0; j < 4; j++)
			{
				float& oldBoneIndex = *((float*)(&desc.m_boneIndices[i].x) + j);
				float& newBoneIndex = *((float*)(&desc.m_boneIndices[i].x) + j);
				float& newBoneWeight = *((float*)(&desc.m_boneWeights[i].x) + j);
				Int mappedIndex = indexOldToNew[Int(oldBoneIndex)];
				if (mappedIndex < 0) // not found
				{
					newBoneIndex = 0;
					newBoneWeight = 0;
				}
				else
				{
					newBoneIndex = (float)mappedIndex;
				}
			}
		}
	}

	desc.m_numBones = numNewBones;

	// copy bone names 
	if (desc.m_boneNames)
	{
		for (IndexT i = 0; i < numNewBones; i++)
		{
			const Char* srcBoneName = boneNames[i];
			// Make sure the boneName pointers are not aliased over the destination
			NV_CORE_ASSERT(!(srcBoneName > desc.m_boneNames && srcBoneName < desc.m_boneNames + NV_HAIR_MAX_STRING * numOldBones));
			// Copy to destination
			char* newBoneName = desc.m_boneNames + i * NV_HAIR_MAX_STRING;
			strcpy(newBoneName, srcBoneName);
		}
	}

	bool useBindPose = desc.m_bindPoses != NV_NULL;
	bool useBoneParent = desc.m_boneParents != NV_NULL;

	// copy bind poses and parent index
	{
		NvCo::PodBuffer<Int32> oldBoneParents;
		if (useBoneParent)
		{
			oldBoneParents.set(desc.m_boneParents, numOldBones);
		}
		NvCo::PodBuffer<gfsdk_float4x4> oldBindPoses;
		if (useBindPose)
		{
			oldBindPoses.set(desc.m_bindPoses, numOldBones);
		}
		for (Int i = 0; i < numNewBones; i++)
		{
			gfsdk_float4x4& dstBindPose = desc.m_bindPoses[i];
			Int mappedIndex = indexNewToOld[i];
			if (mappedIndex < 0) // not found
			{
				if (useBindPose)
				{
					gfsdk_makeIdentity(dstBindPose);
				}
				if (useBoneParent)
				{
					desc.m_boneParents[i] = -1;
				}
			}
			else
			{
				if (useBindPose)
					dstBindPose = oldBindPoses[mappedIndex];
				if (useBoneParent)
				{
					Int32 oldParent = oldBoneParents[mappedIndex];
					Int32 newParent = oldParent;

					if ((oldParent >= 0) && (oldParent < numOldBones))
						newParent = indexOldToNew[oldParent];

					desc.m_boneParents[i] = newParent;
				}
			}
		}
	}
	return NV_OK;
}

Void AssetDescriptorUtil::calcCopy(const Descriptor& srcDesc, const CopySettings& settings, Descriptor& dstDesc)
{
	NV_CORE_ASSERT(NV_SUCCEEDED(check(srcDesc, NV_NULL)));

	if (settings.m_copyAll || settings.m_copyGroom)
	{
		dstDesc.m_numGuideHairs = srcDesc.m_numGuideHairs;
		dstDesc.m_numVertices = srcDesc.m_numVertices;
		dstDesc.m_numFaces = srcDesc.m_numFaces;
		dstDesc.m_numBones = srcDesc.m_numBones;

		// mandatory buffers 
		dstDesc.m_vertices = srcDesc.m_vertices;
		dstDesc.m_endIndices = srcDesc.m_endIndices;
		dstDesc.m_faceIndices = srcDesc.m_faceIndices;
		dstDesc.m_faceUvs = srcDesc.m_faceUvs;

		// bone indices and weights only there is a bone
		if (dstDesc.m_numBones > 0)
		{
			dstDesc.m_boneIndices = srcDesc.m_boneIndices;
			dstDesc.m_boneWeights = srcDesc.m_boneWeights; 
		
			if (srcDesc.m_boneNames)
				dstDesc.m_boneNames = srcDesc.m_boneNames;
			if (srcDesc.m_bindPoses)
				dstDesc.m_bindPoses = srcDesc.m_bindPoses;
			if (srcDesc.m_boneParents)
				dstDesc.m_boneParents = srcDesc.m_boneParents;
		}
	}

	if (settings.m_copyConstraints || settings.m_copyAll)
	{
		dstDesc.m_numPins = srcDesc.m_numPins;
		dstDesc.m_pins = srcDesc.m_pins;
	}

	if (settings.m_copyCollision || settings.m_copyAll)
	{
		dstDesc.m_numBoneSpheres = srcDesc.m_numBoneSpheres;
		dstDesc.m_numBoneCapsules = srcDesc.m_numBoneCapsules;
		dstDesc.m_boneSpheres = srcDesc.m_boneSpheres;
		dstDesc.m_boneCapsuleIndices = srcDesc.m_boneCapsuleIndices;
	}

	if (settings.m_copyTextures || settings.m_copyAll)
	{
		dstDesc.m_textureNames = srcDesc.m_textureNames;
	}

	if (settings.m_copyAll)
	{
		dstDesc.m_handedness = srcDesc.m_handedness;
		dstDesc.m_sceneUnit = srcDesc.m_sceneUnit;
		dstDesc.m_upAxis = srcDesc.m_upAxis;
	}
}

/* static */Void AssetDescriptorUtil::deallocate(Descriptor& desc)
{
	safeDeallocatePodArray(desc.m_vertices);
	safeDeallocatePodArray(desc.m_endIndices);
	safeDeallocatePodArray(desc.m_faceIndices);
	safeDeallocatePodArray(desc.m_faceUvs);

	safeDeallocatePodArray(desc.m_boneIndices);
	safeDeallocatePodArray(desc.m_boneWeights);

	safeDeallocatePodArray(desc.m_boneNames);
	safeDeallocatePodArray(desc.m_bindPoses);
	safeDeallocatePodArray(desc.m_boneParents);
}

} // namespace HairWorks
} // namespace nvidia
