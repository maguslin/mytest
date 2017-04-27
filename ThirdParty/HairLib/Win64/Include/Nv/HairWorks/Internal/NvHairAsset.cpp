/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairAsset.h"

#include <Nv/HairWorks/Internal/Util/NvHairAssetDescriptorUtil.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

#include <Nv/HairWorks/Internal/Util/NvHairNormalUtil.h>

namespace nvidia {
namespace HairWorks { 

Result Asset::updateDescChanged(ApiGlobal* apiGlobal)
{
	NV_RETURN_ON_FAIL(_updateMain());
	_updateBounds();
	_calcNormalizedStrandLengths();
	_initCollisionCapsules();

	_calcGrowthBuffers();
	_calcMasterStrandTangentBuffer();

	// Set the api asset
	NV_RETURN_ON_FAIL(apiGlobal->bindAsset(*this));

	return NV_OK;
}

Result Asset::_updateMain()
{
	const AssetDescriptor& desc = m_assetDesc;
	Int numGuideHairs = desc.m_numGuideHairs;

	m_masterStrandOriginalControlVertices.setSize(desc.m_numVertices);
	m_masterStrandControlVertices.setSize(desc.m_numVertices);
	m_masterStrandControlVertexOffsets.setSize(numGuideHairs);

	m_rootIndices.setSize(numGuideHairs);

	if (desc.m_numBones > 0)
	{
		m_boneIndices.setSize(numGuideHairs);
		m_boneWeights.setSize(numGuideHairs);
	}

	NvCo::PodBuffer<int> originalToValidHairsBuf(numGuideHairs);
	int* originalToValidHairs = originalToValidHairsBuf;

	m_maxHairLength = 0.0f;

	int numAnchors = 1;

	int index = 0;
	float currentHairLength = 0;
	float averageHairLength = 0;
	int hairCount = 0;
	int hairVertexCount = 0;

	//
	for (Int i = 0; i < numGuideHairs; i++)
	{
		currentHairLength = 0;

		Int start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		Int end = desc.m_endIndices[i];

		int prevIndex = index;
		int numCvs = end - start + 1;

		originalToValidHairs[i] = -1;

		for (Int j = start; j <= end; j++)
		{
			gfsdk_float4 v;
			v.x = desc.m_vertices[j].x;
			v.y = desc.m_vertices[j].y;
			v.z = desc.m_vertices[j].z;

			int offset = j - start;
			if (offset < numAnchors)
				v.w = 0.0;
			else
				v.w = 1.0;

			m_masterStrandOriginalControlVertices[index] = v;
			m_masterStrandControlVertices[index] = v;

			if (j != start)
				currentHairLength += gfsdk_length3(m_masterStrandOriginalControlVertices[index] - m_masterStrandOriginalControlVertices[index - 1]);
			index++;
		}

		const float lengthEps = 0.00001f;
		bool validHairs = (currentHairLength > lengthEps);

		if (!validHairs)
		{
			index = prevIndex;
			continue;
		}

		if (currentHairLength > m_maxHairLength)
			m_maxHairLength = currentHairLength;

		averageHairLength += currentHairLength;

		m_masterStrandControlVertexOffsets[hairCount] = index;
		m_rootIndices[hairCount] = (hairCount == 0) ? 0
			: m_masterStrandControlVertexOffsets[hairCount - 1];

		if (desc.m_boneIndices && m_boneIndices)
			m_boneIndices[hairCount] = desc.m_boneIndices[i];

		if (desc.m_boneWeights && m_boneWeights)
			m_boneWeights[hairCount] = desc.m_boneWeights[i];

		originalToValidHairs[i] = hairCount;
		hairCount++;
		hairVertexCount += numCvs;
	}

	if (hairCount == 0)
		return NV_FAIL;

	averageHairLength /= hairCount;

	m_numMasterStrands = hairCount;
	m_numMasterStrandControlVertices = hairVertexCount;

	// count number of segments
	UInt32 maxSegments = 0;
	UInt32 minSegments = 100000;
	for (int i = 0; i < (int)m_numMasterStrands; ++i)
	{
		int beginOffset = (i == 0) ? 0 : m_masterStrandControlVertexOffsets[i - 1];
		int endOffset = m_masterStrandControlVertexOffsets[i];

		UInt32 numSegments = (endOffset - beginOffset) - 1;

		minSegments = gfsdk_min(minSegments, numSegments);
		maxSegments = gfsdk_max(maxSegments, numSegments);
	}

	// Compute tessellated master strand vertex offsets
	m_numMasterSegmentsPerHair = maxSegments;
	m_totalNumMasterSegments = maxSegments * m_numMasterStrands;

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	Int numFaces = Int(desc.m_numFaces);

	m_faceIndices.setSize(numFaces * 3);
	m_faceTexCoords.setSize(numFaces * 3);
	m_faceHairIndices.setSize(numFaces);

	// we check validity of each hair and disable faces adjoining zero-length hair
	Int validFaces = 0;
	for (Int i = 0; i < numFaces; i++)
	{
		UInt32 i0 = desc.m_faceIndices[i * 3];
		UInt32 i1 = desc.m_faceIndices[i * 3 + 1];
		UInt32 i2 = desc.m_faceIndices[i * 3 + 2];

		int v0 = originalToValidHairs[i0];
		int v1 = originalToValidHairs[i1];
		int v2 = originalToValidHairs[i2];

		Bool valid0 = v0 >= 0;
		Bool valid1 = v1 >= 0;
		Bool valid2 = v2 >= 0;

		if (!valid0 || !valid1 || !valid2)
			continue;

		m_faceHairIndices[validFaces] = gfsdk_makeFloat3(float(v0), float(v1), float(v2));

		m_faceTexCoords[validFaces * 3] = desc.m_faceUvs[i * 3];
		m_faceTexCoords[validFaces * 3 + 1] = desc.m_faceUvs[i * 3 + 1];
		m_faceTexCoords[validFaces * 3 + 2] = desc.m_faceUvs[i * 3 + 2];

		m_faceIndices[validFaces * 3] = v0;
		m_faceIndices[validFaces * 3 + 1] = v1;
		m_faceIndices[validFaces * 3 + 2] = v2;

		validFaces++;
	}

	m_numFaces = validFaces;
	return NV_OK;
}

Void Asset::_updateBounds()
{
	gfsdk_float3 bbMin = gfsdk_makeFloat3(FLT_MAX, FLT_MAX, FLT_MAX);
	gfsdk_float3 bbMax = gfsdk_makeFloat3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	NvCo::Memory::zero(m_initialBoundIndices);
	Int* indices = m_initialBoundIndices;

	// bound indices for growth mesh skinning
	const Int numMasterStrands = m_numMasterStrands;
	for (Int i = 0; i < numMasterStrands; i++)
	{
		UInt32 rootIdx = m_rootIndices[i];
		const gfsdk_float3 p = (const gfsdk_float3&)m_masterStrandControlVertices[rootIdx];

		if (p.x < bbMin.x) { bbMin.x = p.x; indices[0] = i; }
		if (p.y < bbMin.y) { bbMin.y = p.y; indices[1] = i; }
		if (p.z < bbMin.z) { bbMin.z = p.z; indices[2] = i; }
		if (p.x > bbMax.x) { bbMax.x = p.x; indices[3] = i; }
		if (p.y > bbMax.y) { bbMax.y = p.y; indices[4] = i; }
		if (p.z > bbMax.z) { bbMax.z = p.z; indices[5] = i; }
	}

	// recompute bounds on 6 extrema vertices
	bbMin = gfsdk_makeFloat3(FLT_MAX, FLT_MAX, FLT_MAX);
	bbMax = gfsdk_makeFloat3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (IndexT i = 0; i < 6; ++i)
	{
		Int index = indices[i];
		Int rootIdx = (index == 0) ? 0 : m_masterStrandControlVertexOffsets[index - 1];
		gfsdk_float3 p = (gfsdk_float3&)m_masterStrandControlVertices[rootIdx];

		bbMin = gfsdk_min(bbMin, p);
		bbMax = gfsdk_max(bbMax, p);
	}

	m_initialModelCenter = 0.5f * (bbMin + bbMax);

	{
		gfsdk_float3 bbSize = bbMax - bbMin;
		m_modelSize = gfsdk_max(gfsdk_max(bbSize.x, bbSize.y), bbSize.z);
	}

	m_initialModelBounds[0] = bbMin;
	m_initialModelBounds[1] = bbMax;
}

Void Asset::_calcNormalizedStrandLengths()
{
	// fill CPU buffer for length of each hair, and normalized distance to root for each CV.
	
	m_masterStrandNormalizedDistances.setSize(m_numMasterStrandControlVertices);
	float* masterStrandNormalizedDistanceToRoot = m_masterStrandNormalizedDistances; 

	int ind = 0;
	int v = 0;

	gfsdk_float3 modelCenter = m_initialModelCenter;

	//m_maxHairLength = 0.0f;
	Float maxHairCenterDist = 0.0f;
	//Float maxHairLength = 0.0f;

	for (IndexT s = 0; s < m_numMasterStrands; ++s)
	{
		Float hairLength = 0.0f;
		int originalInd = ind;

		masterStrandNormalizedDistanceToRoot[ind++] = 0;
		v++;

		//get the actual hair lengths 
		for (; v < (Int)m_masterStrandControlVertexOffsets[s]; ++ind, v++)
		{
			hairLength += gfsdk_length3(m_masterStrandControlVertices[v] - m_masterStrandControlVertices[v - 1]);
			masterStrandNormalizedDistanceToRoot[ind] = hairLength;
		}

		for (v = 0; v < (Int)m_masterStrandControlVertexOffsets[s]; v++)
		{
			Float distFromCenter = gfsdk_length((const gfsdk_float3&)m_masterStrandControlVertices[v] - modelCenter);
			maxHairCenterDist = gfsdk_max(distFromCenter, maxHairCenterDist);
		}

		//create the fractional hair lengths
		for (; originalInd < ind; ++originalInd)
			masterStrandNormalizedDistanceToRoot[originalInd] /= hairLength;

		// set maximum hair length
		//maxHairLength = gfsdk_max(maxHairLength, hairLength);

		// Next
		v = m_masterStrandControlVertexOffsets[s];
	}

	m_maxHairCenterDist = maxHairCenterDist;
	//m_maxHairLength = maxHairLength;
}

Void Asset::_initCollisionCapsules()
{
	const AssetDescriptor& desc = m_assetDesc;
	const IndexT numCapsules = desc.m_numBoneCapsules;
	m_collisionCapsuleIndices.setSize(numCapsules);

	const UInt32* indices = desc.m_boneCapsuleIndices;

	for (IndexT i = 0; i < numCapsules; i++)
	{
		m_collisionCapsuleIndices[i] = gfsdk_makeFloat4(Float(indices[i * 2]), Float(indices[i * 2 + 1]), 0.0f, 0.0f);
	}
}

Void Asset::_calcGrowthBuffers()
{
	/* NV_RETURN_ON_FAIL(_createGrowthMeshVertexBuffers(device, inst));
	NV_RETURN_ON_FAIL(_createGrowthMeshNormalBuffers(device, inst));
	NV_RETURN_ON_FAIL(_createGrowthMeshTangentBuffers(device, inst)); */

	{
		// create CPU buffer for mesh vertex 
		m_growthMeshVertices.setSize(m_numMasterStrands);
		// copy growth mesh vertex from hair root
		const gfsdk_float4* vertices = m_masterStrandControlVertices;
		for (Int i = 0; i < m_numMasterStrands; i++)
		{
			UInt32 rootIdx = m_rootIndices[i];
			m_growthMeshVertices[i] = gfsdk_makeFloat4(vertices[rootIdx].x, vertices[rootIdx].y, vertices[rootIdx].z, 1);
		}
	}
	{
		m_growthMeshNormals.setSize(m_numMasterStrands);
		NormalUtil::calcNormals(m_numMasterStrands, m_numFaces, m_faceIndices, m_growthMeshVertices, m_growthMeshNormals);
	}
	{
		m_growthMeshTangents.setSize(m_numMasterStrands);
		//NormalUtil::calcTangents(m_numMasterStrands, m_numFaces, m_faceIndices, m_growthMeshVertices, m_growthMeshTangents);
		NormalUtil::calcTangents(m_growthMeshNormals, m_numMasterStrands, m_growthMeshTangents);
	}
}


Void Asset::calcHairInteractionBuffers(NvCo::PodBuffer<Int32>& offsetsOut, NvCo::PodBuffer<Float>& lengthsOut, NvCo::PodBuffer<Int32>& indicesOut) const
{
	int numCvs = m_numMasterStrandControlVertices;

	NvCo::PodBuffer<gfsdk_float3> rootIndices(m_numFaces);
	NvCo::PodBuffer<gfsdk_float4> masterStrandCvCounts(m_numFaces);

	{
		// root vertex index and cv counts per each vertex of the faces
		const UInt32* indices = m_faceIndices;
		for (Int f = 0; f < m_numFaces; f++, indices += 3)
		{
			int offset[3];
			int num[3];

			for (int i = 0; i < 3; ++i)
			{
				int index = indices[i];
				offset[i] = index == 0 ? 0 : m_masterStrandControlVertexOffsets[index - 1];
				num[i] = m_masterStrandControlVertexOffsets[index] - offset[i];
			}

			rootIndices[f] = gfsdk_makeFloat3(float(offset[0]), float(offset[1]), float(offset[2]));
			masterStrandCvCounts[f] = gfsdk_makeFloat4(float(num[0]), float(num[1]), float(num[2]), float(min(min(num[0], num[1]), num[2])));
		}
	}

	// compute valence 
	NvCo::PodBuffer<int> valences(numCvs);
	valences.zero();
	offsetsOut.setSize(numCvs);
	offsetsOut.zero();
	int* offsets = offsetsOut;

	int totalCnt = 0;
	for (Int f = 0; f < m_numFaces; f++)
	{
		const gfsdk_float3& curIndices = rootIndices[f];
		const gfsdk_float4& cvCounts = masterStrandCvCounts[f];

		int root0 = int(curIndices.x);
		int root1 = int(curIndices.y);
		int root2 = int(curIndices.z);
		int minCv = int(cvCounts.w);

		for (int i = 0; i < minCv; i++)
		{
			valences[root0 + i] += 2;
			valences[root1 + i] += 2;
			valences[root2 + i] += 2;

			totalCnt += 6;
		}
	}

	// do prefix sum on valence
	int sum = 0;
	for (int i = 0; i < numCvs; i++)
	{
		sum += valences[i];
		valences[i] = sum;
	}

	// TODO: Check if this is what is wanted
	// Copies floats to offset ints!!!
	memcpy(offsets, valences, sizeof(int) * numCvs);

	// allocate total array
	indicesOut.setSize(totalCnt);
	Int32* indices = indicesOut;

	lengthsOut.setSize(totalCnt);
	Float* lengths = lengthsOut;

	// addTriangle
	for (Int f = 0; f < m_numFaces; f++)
	{
		const gfsdk_float3& curIndices = rootIndices[f];
		const gfsdk_float4& cvCounts = masterStrandCvCounts[f];

		int root0 = int(curIndices.x);
		int root1 = int(curIndices.y);
		int root2 = int(curIndices.z);
		int minCv = int(cvCounts.w);

		for (int i = 0; i < minCv; i++)
		{
			int vid0 = root0 + i;
			int vid1 = root1 + i;
			int vid2 = root2 + i;

			gfsdk_float3 p0 = (gfsdk_float3&)m_masterStrandControlVertices[vid0];
			gfsdk_float3 p1 = (gfsdk_float3&)m_masterStrandControlVertices[vid1];
			gfsdk_float3 p2 = (gfsdk_float3&)m_masterStrandControlVertices[vid2];

			float l01 = gfsdk_length(p0 - p1);
			float l02 = gfsdk_length(p0 - p2);
			float l12 = gfsdk_length(p1 - p2);

			{
				indices[--valences[vid0]] = vid1;
				lengths[valences[vid0]] = l01;

				indices[--valences[vid0]] = vid2;
				lengths[valences[vid0]] = l02;
			}

			{
				indices[--valences[vid1]] = vid2;
				lengths[valences[vid1]] = l12;

				indices[--valences[vid1]] = vid0;
				lengths[valences[vid1]] = l01;
			}

			{
				indices[--valences[vid2]] = vid0;
				lengths[valences[vid2]] = l02;

				indices[--valences[vid2]] = vid1;
				lengths[valences[vid2]] = l12;
			}
		}
	}
}

Void Asset::calcTexCoordBuffer(NvCo::PodBuffer<gfsdk_float2>& texCoordsOut) const
{
	// Function to calculate per hair texture coord to sample per hair attribute (e.g. stiffness) during compute
	texCoordsOut.setSize(m_numMasterStrands);
	texCoordsOut.zero();
	gfsdk_float2* texCoords = texCoordsOut;

	const UInt32 numMasterStrands = UInt32(m_numMasterStrands);
	// copy hair textures from face textures
	for (Int i = 0; i < m_numFaces; i++)
	{
		UInt32 v0 = m_faceIndices[i * 3];
		UInt32 v1 = m_faceIndices[i * 3 + 1];
		UInt32 v2 = m_faceIndices[i * 3 + 2];

		if (v0 < numMasterStrands && v1 < numMasterStrands && v2 < numMasterStrands)
		{
			gfsdk_float2 t0 = m_faceTexCoords[i * 3];
			gfsdk_float2 t1 = m_faceTexCoords[i * 3 + 1];
			gfsdk_float2 t2 = m_faceTexCoords[i * 3 + 2];

			texCoords[v0] = t0;
			texCoords[v1] = t1;
			texCoords[v2] = t2;
		}
	}
}

Void Asset::calcFrameBuffers(NvCo::PodBuffer<gfsdk_float4>& framesOut, NvCo::PodBuffer<gfsdk_float4>& localPosPrevOut, NvCo::PodBuffer<gfsdk_float4>& localPosNextOut) const
{
	Int numControlVertices = m_numMasterStrandControlVertices;

	framesOut.setSize(numControlVertices);
	localPosPrevOut.setSize(numControlVertices);
	localPosNextOut.setSize(numControlVertices);

	gfsdk_float4* frames = framesOut;
	gfsdk_float4* localPosPrev = localPosPrevOut;
	gfsdk_float4* localPosNext = localPosNextOut;

	// for each cv, compute accumulated distance from the root
	for (Int v = 0, s = 0; s < m_numMasterStrands; ++s)
	{
		gfsdk_quaternion prevq = gfsdk_quatFromAxis((gfsdk_float3&)m_growthMeshNormals[s], (gfsdk_float3&)m_growthMeshTangents[s]);

		int cnt = 0;
		int numCvs = (int)m_masterStrandControlVertexOffsets[s] - v;

		for (cnt = 0; cnt < numCvs; ++v, ++cnt)
		{
			gfsdk_float3 p = (gfsdk_float3&)m_masterStrandControlVertices[v];
			gfsdk_float3 prevp = (gfsdk_float3&)m_masterStrandControlVertices[v];
			gfsdk_float3 nextp = (gfsdk_float3&)m_masterStrandControlVertices[v];

			if (cnt > 0)
				prevp = (gfsdk_float3&)m_masterStrandControlVertices[v - 1];

			if (cnt < (numCvs - 1))
				nextp = (gfsdk_float3&)m_masterStrandControlVertices[v + 1];

			gfsdk_float3 prevDir = prevp - p;
			gfsdk_float3 nextDir = nextp - p;

			gfsdk_float3 z = gfsdk_getBasisZ(prevq);

			gfsdk_float3 prevz = z, newz = z;

			if (cnt > 0)
			{
				// use central difference
				newz = gfsdk_getNormalized(nextp - prevp);
			}

			gfsdk_quaternion rot = gfsdk_rotateBetween(prevz, newz);
			gfsdk_quaternion q = gfsdk_quaternionMultiply(rot, prevq);

			localPosPrev[v] = gfsdk_makeFloat4(gfsdk_rotateInv(q, prevDir));
			localPosNext[v] = gfsdk_makeFloat4(gfsdk_rotateInv(q, nextDir));

			frames[v] = q;
			prevq = q;
		}
	}
}

Void Asset::_calcMasterStrandTangentBuffer()
{
	m_masterStrandTangents.setSize(m_numMasterStrandControlVertices);

	// for each cv, compute accumulated distance from the root
	for (Int v = 0, s = 0; s < m_numMasterStrands; ++s)
	{
		Int offset = (int)m_masterStrandControlVertexOffsets[s];
		for (Int cnt = 0; v < offset; ++v, ++cnt)
		{
			const Bool firstCv = (cnt == 0);
			const Bool lastCv = (v == (offset - 1));

			gfsdk_float4 pp = firstCv ? m_masterStrandControlVertices[v] : m_masterStrandControlVertices[v - 1];
			gfsdk_float4 np = lastCv ? m_masterStrandControlVertices[v] : m_masterStrandControlVertices[v + 1];

			gfsdk_float3 tangent = gfsdk_getNormalized((gfsdk_float3&)np - (gfsdk_float3&)pp);

			m_masterStrandTangents[v] = gfsdk_makeFloat4(tangent, 0);
		}
	}
}

Void Asset::calcMasterStrandRootDistances(NvCo::PodBuffer<Float>& distancesOut) const
{
	// create buffers for LRA
	//LRA root distance
	distancesOut.setSize(m_numMasterStrandControlVertices);
	float* rootDistances = distancesOut;
	// for each cv, compute accumulated distance from the root
	for (int v = 0, s = 0; s < (int)m_numMasterStrands; ++s)
	{
		float length = 0.0f;
		for (int cnt = 0; v < (int)m_masterStrandControlVertexOffsets[s]; ++v, ++cnt)
		{
			if (cnt == 0)
				length = 0.0f;
			else
				length += gfsdk_length3(m_masterStrandControlVertices[v] - m_masterStrandControlVertices[v - 1]);
			rootDistances[v] = length;
		}
	}
}

Void Asset::calcVertexToHairMap(NvCo::PodBuffer<Int32>& vertexToHairMapOut) const
{
	const AssetDescriptor& desc = getDesc();

	const int numCvs = m_numMasterStrandControlVertices;
	vertexToHairMapOut.setSize(numCvs);

	for (Int i = 0; i < m_numMasterStrands; i++)
	{
		UInt32 start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		UInt32 end = desc.m_endIndices[i];
		for (UInt32 j = start; j <= end; j++)
			vertexToHairMapOut[j] = i;
	}
}
Void Asset::calcMasterStrandIndices(NvCo::PodBuffer<UInt32>& indicesOut) const
{
	const AssetDescriptor& desc = getDesc();

	// create index buffer for master strand
	indicesOut.setSize(m_numMasterStrandControlVertices * 2);
	Int cnt = 0;
	for (Int i = 0; i < m_numMasterStrands; i++)
	{
		UInt32 start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		UInt32 end = desc.m_endIndices[i];

		for (UInt32 j = start; j < end; j++)
		{
			indicesOut[cnt * 2] = j;
			indicesOut[cnt * 2 + 1] = j + 1;
			cnt++;
		}
	}
	indicesOut.resize(Int(cnt * 2));
	//NV_CORE_ASSERT(cnt * 2 == m_numMasterStrandControlVertices * 2);
}

Void Asset::calcFrameVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const
{
	const AssetDescriptor& desc = getDesc();

	indicesOut.setSize(m_numMasterStrandControlVertices * 6);
	UInt32 vcnt = 0;
	for (Int i = 0; i < m_numMasterStrands; i++)
	{
		UInt32 start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		UInt32 end = desc.m_endIndices[i];
		for (UInt32 j = start; j <= end; j++, vcnt++)
		{
			indicesOut[vcnt * 6    ] = vcnt * 4;
			indicesOut[vcnt * 6 + 1] = vcnt * 4 + 1;

			indicesOut[vcnt * 6 + 2] = vcnt * 4;
			indicesOut[vcnt * 6 + 3] = vcnt * 4 + 2;

			indicesOut[vcnt * 6 + 4] = vcnt * 4;
			indicesOut[vcnt * 6 + 5] = vcnt * 4 + 3;
		}
	}
	indicesOut.resize(Int(vcnt * 6));
	//NV_CORE_ASSERT(Int(vcnt * 6) == m_numMasterStrandControlVertices * 6);
}

Void Asset::calcLocalPosVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const
{
	const AssetDescriptor& desc = getDesc();

	indicesOut.setSize(m_numMasterStrandControlVertices * 4);
	UInt32 vcnt = 0;
	for (Int i = 0; i < m_numMasterStrands; i++)
	{
		UInt32 start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		UInt32 end = desc.m_endIndices[i];
		for (UInt32 j = start; j <= end; j++, vcnt++)
		{
			indicesOut[vcnt * 4] = vcnt * 3;
			indicesOut[vcnt * 4 + 1] = vcnt * 3 + 1;

			indicesOut[vcnt * 4 + 2] = vcnt * 3;
			indicesOut[vcnt * 4 + 3] = vcnt * 3 + 2;
		}
	}
	indicesOut.resize(Int(vcnt * 4));
	//NV_CORE_ASSERT(Int(vcnt * 4) == m_numMasterStrandControlVertices * 4);
}

Void Asset::calcNormalVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const
{
	const AssetDescriptor& desc = getDesc();

	indicesOut.setSize(m_numMasterStrandControlVertices * 2);
	UInt32 vcnt = 0;
	for (Int i = 0; i < m_numMasterStrands; i++)
	{
		UInt32 start = (i == 0) ? 0 : desc.m_endIndices[i - 1] + 1;
		UInt32 end = desc.m_endIndices[i];
		for (UInt32 j = start; j <= end; j++, vcnt++)
		{
			indicesOut[vcnt * 2] = vcnt * 2;
			indicesOut[vcnt * 2 + 1] = vcnt * 2 + 1;
		}
	}
	indicesOut.resize(Int(vcnt * 2));

	//NV_CORE_ASSERT(Int(vcnt * 2) == m_numMasterStrandControlVertices * 2);
}

Void Asset::calcPins(NvCo::PodBuffer<NvHair_Pin>& pinsOut) const
{
	const AssetDescriptor& desc = getDesc();

	const Int numPins = desc.m_numPins;
	pinsOut.setSize(numPins);

	const gfsdk_float4x4* bindPoseMatrices = getBindPoseMatrices();
	const Int numBones = getNumBones();
	const Int numCvs = m_numMasterStrandControlVertices;

	float maxPinZ = -1e10;
	int maxPinZIndex = -1;

	for (Int i = 0; i < Int(desc.m_numPins); i++)
	{
		const Pin& pinAsset = desc.m_pins[i];

		NvHair_Pin& pin = pinsOut[i];

		pin.boneIndex = pinAsset.m_boneIndex;
		pin.radius = pinAsset.m_radius;
		pin.localPos = pinAsset.m_localPos;

		gfsdk_makeIdentity(pin.invHairPoseMatrix);
		gfsdk_makeIdentity(pin.currentHairMatrix);
		gfsdk_makeIdentity(pin.invPinPoseMatrix);
		gfsdk_makeIdentity(pin.currentPinMatrix);

		if (pin.boneIndex >= 0)
		{
			const gfsdk_float4x4& bonePoseMatrix = bindPoseMatrices[pin.boneIndex];

			gfsdk_float4x4 offsetMatrix;  gfsdk_makeTranslation(offsetMatrix, pin.localPos);

			// TODO: dummy shape matrix
			gfsdk_float3 shapeRotAxis = gfsdk_getNormalized(gfsdk_makeFloat3(1.0f, 1.0f, 1.0f));
			gfsdk_quaternion shapeQuaternion = gfsdk_makeRotation(shapeRotAxis, 0.78539816339744830961566084581988f);
			gfsdk_float4x4 shapeRotationMatrix;
			gfsdk_makeRotation(shapeRotationMatrix, shapeQuaternion);
			gfsdk_float4x4 shapeScaleMatrix;
			gfsdk_makeScale(shapeScaleMatrix, gfsdk_makeFloat3(0.99f, 1.04f, 1.02f));
			pin.shapeMatrix = shapeScaleMatrix*shapeRotationMatrix;

			gfsdk_float4x4 pinPose = pin.shapeMatrix * offsetMatrix * bonePoseMatrix;
			gfsdk_float4x4 invPose = gfsdk_inverse(pinPose);

			// compute rest pose center of mass
			gfsdk_float4 restCom = gfsdk_makeFloat4(0, 0, 0, 0);
			gfsdk_float3 restTangent = gfsdk_makeFloat3(0, 0, 0);

			for (Int j = 0; j < numCvs; ++j)
			{
				gfsdk_float3 rp = (gfsdk_float3&)m_masterStrandControlVertices[j];
				gfsdk_float3 rt = (gfsdk_float3&)m_masterStrandTangents[j];
				gfsdk_float3 lp = gfsdk_transformCoord(invPose, rp);

				float l = gfsdk_length(lp);
				float w = 1.0f - gfsdk_saturate(l / pin.radius);

				(gfsdk_float3&)restCom += w * rp;
				restCom.w += w;

				restTangent += w * rt;
			}

			restCom = (restCom * (1.0f / (restCom.w + FLT_EPSILON)));
			gfsdk_float3 com = (const gfsdk_float3&)(restCom);

			gfsdk_float4x4 hairPoseMatrix = bonePoseMatrix;
			gfsdk_setTranslation(hairPoseMatrix, com);
			pin.invHairPoseMatrix = gfsdk_inverse(hairPoseMatrix);
			pin.currentHairMatrix = hairPoseMatrix;

			pin.restTangent = gfsdk_getNormalized(restTangent);

			gfsdk_float3 pinCenter = gfsdk_getTranslation(pinPose);
			pin.restComShift = com - pinCenter;

			pin.invPinPoseMatrix = invPose;
			pin.currentPinMatrix = pinPose;

			gfsdk_float4x4 currentHairMatrix = bonePoseMatrix;
			currentHairMatrix._41 = com.x;
			currentHairMatrix._42 = com.y;
			currentHairMatrix._43 = com.z;

			gfsdk_float4x4 offset; gfsdk_makeTranslation(offset, -1.0f * pin.restComShift);
			gfsdk_float4x4 pinMatrix = currentHairMatrix * offset;

			if (pinCenter.z > maxPinZ) 
			{
				maxPinZ = pinCenter.z;
				maxPinZIndex = i;
			}

			if (pinMatrix._11 != pinPose._11)
				offset._11 = offset._11;
			// is pin same as pinPose?
		}
	}

	gfsdk_float3 rootBonePos = gfsdk_getTranslation(bindPoseMatrices[desc.m_pins[maxPinZIndex].m_boneIndex]);
	for (UInt i = 0; i < desc.m_numPins; i++)
	{
		//const gfsdk_boneSphere& boneSphere = hairDesc.m_pPinConstraints[i];

		const nvidia::HairWorks::Pin& pinSrc = desc.m_pins[i];
		NvHair_Pin& pin = pinsOut[i];
		gfsdk_float3 pinCenter = gfsdk_getTranslation(pin.currentPinMatrix);

		// TODO: Configure these from authoring tool
		pin.rootBoneIndex = desc.m_pins[maxPinZIndex].m_boneIndex;
		pin.rootBoneDis = gfsdk_length(pinCenter - rootBonePos);
		pin.stiffness = pinSrc.m_pinStiffness;
		pin.influenceFallOff = pinSrc.m_influenceFallOff;
		pin.influenceFallOffCurve = pinSrc.m_influenceFallOffCurve;
		pin.useDynamicPin = pinSrc.m_useDynamicPin;
		pin.doLra = pinSrc.m_doLra;
		pin.stiffPin = true; // pinSrc.m_useStiffnessPin;  // hardcode it as true for GWDCC-302.
		pin.selected = pinSrc.m_selected;
	}
}

Void Asset::calcSkinnedGrowthMeshBounds(const gfsdk_float4x4* skinningMatrices, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut) const
{
	const Int numBones = getNumBones();

	gfsdk_float3 bbMin = gfsdk_makeFloat3(FLT_MAX, FLT_MAX, FLT_MAX);
	gfsdk_float3 bbMax = gfsdk_makeFloat3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for(Int i = 0; i < 6; ++i)
	{
		UInt index = m_initialBoundIndices[i];

		UInt rootIndex = (index == 0) ? 0 : m_masterStrandControlVertexOffsets[index - 1];

		gfsdk_float3 hp = (gfsdk_float3&)m_masterStrandOriginalControlVertices[rootIndex];
		gfsdk_float3 tp = gfsdk_makeFloat3(0.0f, 0.0f, 0.0f);

		if(skinningMatrices && m_boneIndices && m_boneWeights)
		{
			const float* boneIndices = &m_boneIndices[index].x;
			const float* boneWeights = &m_boneWeights[index].x;

			float weightSum = boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3] + FLT_EPSILON;

			for(Int b = 0; b < 4; b++)
			{
				float w = boneWeights[b];
				if(w == 0.0)
					continue;

				const Int boneIndex = Int(boneIndices[b]);
				NV_CORE_ASSERT(boneIndex >= 0 && boneIndex < numBones);

				const gfsdk_float4x4& bone = skinningMatrices[boneIndex];

				gfsdk_float3 p = gfsdk_transformCoord(bone, hp);
				tp += boneWeights[b] * p;
			}
			tp = (1.0f / weightSum) * tp;
		}
		else
		{
			tp = hp;
		}

		bbMin = gfsdk_min(bbMin, tp);
		bbMax = gfsdk_max(bbMax, tp);
	}

	bbMinOut = bbMin;
	bbMaxOut = bbMax;
}

Void Asset::calcSkinnedGrowthMeshBounds(const gfsdk_dualquaternion* skinningDqs, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut) const
{
	const Int numBones = getNumBones();

	gfsdk_float3 bbMin = gfsdk_makeFloat3(FLT_MAX, FLT_MAX, FLT_MAX);
	gfsdk_float3 bbMax = gfsdk_makeFloat3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (Int i = 0; i < 6; ++i)
	{
		Int index = m_initialBoundIndices[i];
		Int rootIndex = (index == 0) ? 0 : m_masterStrandControlVertexOffsets[index - 1];

		gfsdk_float3 hp = (gfsdk_float3&)m_masterStrandOriginalControlVertices[rootIndex];
		gfsdk_float3 tp = gfsdk_makeFloat3(0.0f, 0.0f, 0.0f);

		if(skinningDqs && m_boneIndices && m_boneWeights)
		{
			const float* boneIndices = &m_boneIndices[index].x;
			const float* boneWeights = &m_boneWeights[index].x;

			gfsdk_dualquaternion tempDq;
			gfsdk_makeZero(tempDq);

			for(Int b = 0; b < 4; b++)
			{
				float w = boneWeights[b];
				if(w == 0.0)
					continue;

				Int boneIndex = Int(boneIndices[b]);
				NV_CORE_ASSERT(boneIndex >= 0 && boneIndex < numBones);

				const gfsdk_dualquaternion& bone = skinningDqs[boneIndex];
				tempDq += w * bone;
			}

			tempDq = gfsdk_getNormalized(tempDq);
			tp = gfsdk_transformCoord(tempDq, gfsdk_makeFloat3(hp.x, hp.y, hp.z));
		}
		else
		{
			tp = hp;
		}

		bbMin = gfsdk_min(bbMin, tp);
		bbMax = gfsdk_max(bbMax, tp);
	}

	bbMinOut = bbMin;
	bbMaxOut = bbMax;
}

} // namespace HairWorks
} // namespace nvidia
