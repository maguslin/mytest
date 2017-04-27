/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx12ApiAsset.h"

#include <Nv/HairWorks/Internal/NvHairAsset.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>

#include <float.h>

namespace nvidia {
namespace HairWorks { 

Dx12ApiAsset::Dx12ApiAsset():
	m_apiGlobal(NV_NULL),
	m_asset(NV_NULL)
{
}

Dx12ApiAsset::~Dx12ApiAsset()
{
	if (m_apiGlobal)
	{
		NvCo::Dx12DescriptorSet set(m_viewHeap.getCpuStart(), m_viewHeap.getSize());
		m_apiGlobal->m_viewCache.evictIntersects(set);
	}
}


Result Dx12ApiAsset::init(Dx12ApiGlobal* apiGlobal, Asset* asset)
{
	m_apiGlobal = apiGlobal;
	m_asset = asset;

	// shared heap
	NV_RETURN_ON_FAIL(m_viewHeap.init(apiGlobal->getDevice(), 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

	Dx12Info info = m_apiGlobal->getInfo(&m_viewHeap);

	NV_RETURN_ON_FAIL(_createMasterStrandBuffers(info));
	NV_RETURN_ON_FAIL(_createTessellatedFaceBuffers(info));
	NV_RETURN_ON_FAIL(_createFaceTexCoords(info));
	NV_RETURN_ON_FAIL(_createHairTexCoords(info));
	NV_RETURN_ON_FAIL(_createHairInteractionBuffers(info));
	NV_RETURN_ON_FAIL(_createSkinningBuffers(info));
	NV_RETURN_ON_FAIL(_createGrowthMeshBuffers(info));
	NV_RETURN_ON_FAIL(_createDebugRenderBuffers(info));

	return NV_OK;
}

Result Dx12ApiAsset::_createTessellatedFaceBuffers(const Dx12Info& info)
{
	const UINT* indices = m_asset->m_faceIndices;

	// counters computed during buffer creation time
	NvCo::PodBuffer<gfsdk_float3> faceIndices;

	// triangle indices in gfsdk_float3 for shader binding
	faceIndices.setSize(m_asset->m_numFaces);

	// root vertex index and cv counts per each vertex of the faces
	const UINT numMasterStrands = UINT(m_asset->m_numMasterStrands);
	for (Int f = 0; f < m_asset->m_numFaces; f++, indices += 3)
	{
		if (indices[0] < numMasterStrands && indices[1] < numMasterStrands && indices[2] < numMasterStrands)
		{
			faceIndices[f] = gfsdk_makeFloat3(float(indices[0]), float(indices[1]), float(indices[2]));
		}
	}
	NV_RETURN_ON_FAIL(m_faceHairIndicesBuffer.init(info, m_asset->m_numFaces, faceIndices.begin()));
	return NV_OK;
}

Result Dx12ApiAsset::_createFaceTexCoords(const Dx12Info& info)
{
	NV_RETURN_ON_FAIL(m_faceTexCoordsBuffer.init(info, m_asset->m_numFaces * 3, m_asset->m_faceTexCoords));
	return NV_OK;
}

Result Dx12ApiAsset::_createHairTexCoords(const Dx12Info& info)
{
	// Function to calculate per hair texture coord to sample per hair attribute (e.g. stiffness) during compute
	NvCo::PodBuffer<gfsdk_float2> texCoordsBuf;
	m_asset->calcTexCoordBuffer(texCoordsBuf);
	// create GPU resources for hair textures
	NV_RETURN_ON_FAIL(m_hairTexCoordsBuffer.init(info, texCoordsBuf.getSize(), texCoordsBuf));
	return NV_OK;
}

Result Dx12ApiAsset::_createHairInteractionBuffers(const Dx12Info& info)
{
	NvCo::PodBuffer<Int32> offsets, indices;
	NvCo::PodBuffer<Float> lengths;
	m_asset->calcHairInteractionBuffers(offsets, lengths, indices);

	NV_CORE_ASSERT(offsets.getSize() == m_asset->m_numMasterStrandControlVertices);
	NV_CORE_ASSERT(lengths.getSize() == indices.getSize());

	// TODO - compact array to avoid duplicate edges
	// create buffer for indices
	NV_RETURN_ON_FAIL(m_masterStrandInteractionIndicesBuffer.init(info, indices.getSize(), indices));
	// create buffer for lengths
	NV_RETURN_ON_FAIL(m_masterStrandInteractionLengthBuffer.init(info, lengths.getSize(), lengths));
	// create buffer for offset array
	NV_RETURN_ON_FAIL(m_masterStrandInteractionOffsetBuffer.init(info, UINT(offsets.getSize()), offsets));

	m_totalNumHairInteractions = Int(indices.getSize());
	return NV_OK;
}

Result Dx12ApiAsset::_createMasterStrandBuffers(const Dx12Info& info)
{
	const Asset* asset = m_asset;

	{
		// create buffers for linear spring 
		// create CPU buffer for spring rest lengths
		NvCo::PodBuffer<Float32> linearSpringLengthsBuf(asset->m_numMasterStrandControlVertices);
		Float32* linearSpringLengths = linearSpringLengthsBuf;
		for (IndexT i = 0; i < (int)asset->m_numMasterStrandControlVertices - 1; i++)
		{
			linearSpringLengths[i] = gfsdk_length3(asset->m_masterStrandControlVertices[i] - asset->m_masterStrandControlVertices[i + 1]);
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL(m_masterStrandLinearSpringLengthsBuffer.init(info, asset->m_numMasterStrandControlVertices, linearSpringLengths));
	}

	// create GPU buffer for simulation hair indices
	NV_RETURN_ON_FAIL(m_masterStrandOffsetBuffer.init(info, asset->m_numMasterStrands, asset->m_masterStrandControlVertexOffsets));
	NV_RETURN_ON_FAIL(m_masterStrandNormalizedLengthToRootBuffer.init(info,  asset->m_numMasterStrandControlVertices, asset->m_masterStrandNormalizedDistances));

	{
		// create buffers for bend spring 
		// create CPU buffer for spring rest lengths
		NvCo::PodBuffer<Float32> bendSpringLengthsBuf(asset->m_numMasterStrandControlVertices);
		Float32* bendSpringLengths = bendSpringLengthsBuf;
		for (int i = 0; i < (int)asset->m_numMasterStrandControlVertices - 2; i++)
		{
			bendSpringLengths[i] = gfsdk_length3(asset->m_masterStrandControlVertices[i + 2] - asset->m_masterStrandControlVertices[i]);
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL(m_masterStrandBendSpringLengthsBuffer.init(info, asset->m_numMasterStrandControlVertices, bendSpringLengths));
	}

	{
		NvCo::PodBuffer<Float32> rootDistances;
		asset->calcMasterStrandRootDistances(rootDistances);
		// create GPU buffer
		NV_RETURN_ON_FAIL(m_masterStrandRootDistancesBuffer.init(info, rootDistances.getSize(), rootDistances));
	}

	// create gpu buffer for original master strand
	NV_RETURN_ON_FAIL(m_origMasterStrandBuffer.init(info, asset->m_numMasterStrandControlVertices, asset->m_masterStrandOriginalControlVertices));

	{
		NvCo::PodBuffer<gfsdk_float4> frames, localPosPrev, localPosNext;
		asset->calcFrameBuffers(frames, localPosPrev, localPosNext);

		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(m_origMasterFramesBuffer.init(info, frames.getSize(), frames));

		//Int numControlVertices = asset->m_numMasterStrandControlVertices;
		// NUTT
		//NV_RETURN_ON_FAIL(m_origMasterPosBuffer.init(info, sizeof(gfsdk_float4), numControlVertices, asset->m_masterStrandControlVertices));
		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(m_prevMasterLocalPosBuffer.init(info, localPosPrev.getSize(), localPosPrev));
		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(m_nextMasterLocalPosBuffer.init(info, localPosNext.getSize(), localPosNext));
	}

	return NV_OK;
}

Result Dx12ApiAsset::_createSkinningBuffers(const Dx12Info& info)
{
	const Asset* asset = m_asset;
	// create GPU buffer for bone indices
	NV_RETURN_ON_FAIL(m_boneIndicesBuffer.init(info, asset->m_numMasterStrands, asset->m_boneIndices));
	// create GPU buffer for bone weights
	NV_RETURN_ON_FAIL(m_boneWeightsBuffer.init(info, asset->m_numMasterStrands, asset->m_boneWeights));
	return NV_OK;
}

Result Dx12ApiAsset::_createGrowthMeshBuffers(const Dx12Info& info)
{
	const Asset* asset = m_asset;
	// create GPU index buffer for growth mesh
	NV_RETURN_ON_FAIL(m_growthMeshIndexBuffer.init(info, asset->m_faceIndices.getSize(), asset->m_faceIndices));
	NV_RETURN_ON_FAIL(m_growthMeshRestNormalBuffer.init(info, asset->m_growthMeshNormals.getSize(), asset->m_growthMeshNormals));
	NV_RETURN_ON_FAIL(m_growthMeshRestTangentBuffer.init(info, asset->m_growthMeshTangents.getSize(), asset->m_growthMeshTangents));
	return NV_OK;
}

Result Dx12ApiAsset::_createDebugRenderBuffers(const Dx12Info& info)
{
	const Asset* asset = m_asset;
	// create buffer for vertex to hair map
	{
		NvCo::PodBuffer<Int32> vertexToHairBuffer;
		asset->calcVertexToHairMap(vertexToHairBuffer);
		NV_RETURN_ON_FAIL(m_debugVertexToHairBuffer.init(info, vertexToHairBuffer.getSize(), vertexToHairBuffer));
	}

	// create index buffer for master strand
	{
		NvCo::PodBuffer<UInt32> indices;
		asset->calcMasterStrandIndices(indices);
		NV_RETURN_ON_FAIL(m_debugMasterIndexBuffer.init(info, indices.getSize() /* cnt * 2*/, indices));
	}
	// create index buffer for frame visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		asset->calcFrameVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(m_debugFrameIndexBuffer.init(info, indices.getSize(), indices));
	}
	// create index buffer for local pos visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		asset->calcLocalPosVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(m_debugLocalPosIndexBuffer.init(info, indices.getSize(), indices));
	}

	// create index buffer for normal visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		asset->calcNormalVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(m_debugNormalIndexBuffer.init(info, indices.getSize(), indices));
	}
	return NV_OK;
}

} // namespace HairWorks 
} // namespace nvidia 