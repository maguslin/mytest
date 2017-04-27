/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx11ApiAsset.h"

#include <Nv/HairWorks/Internal/NvHairAsset.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include <float.h>

#include "NvHairDx11Util.h"

namespace nvidia {
namespace HairWorks { 

Dx11ApiAsset::Dx11ApiAsset():
	m_apiGlobal(NV_NULL),
	m_asset(NV_NULL)
{
}

Result Dx11ApiAsset::init(Dx11ApiGlobal* apiGlobal, Asset* asset)
{
	m_apiGlobal = apiGlobal;
	m_asset = asset;

	NV_RETURN_ON_FAIL(_createMasterStrandBuffers());

	NV_RETURN_ON_FAIL(_createTessellatedFaceBuffers());

	NV_RETURN_ON_FAIL(_createFaceTexCoords());
	NV_RETURN_ON_FAIL(_createHairTexCoords());

	NV_RETURN_ON_FAIL(_createHairInteractionBuffers());

	NV_RETURN_ON_FAIL(_createSkinningBuffers());

	NV_RETURN_ON_FAIL(_createGrowthMeshBuffers());
	NV_RETURN_ON_FAIL(_createDebugRenderBuffers());

	return NV_OK;
}

Result Dx11ApiAsset::_createTessellatedFaceBuffers()
{
	const UINT* indices = m_asset->m_faceIndices;
	// triangle indices in gfsdk_float3 for shader binding
	m_faceIndices.setSize(m_asset->m_numFaces);

	// root vertex index and cv counts per each vertex of the faces
	const UINT numMasterStrands = UINT(m_asset->m_numMasterStrands);
	for (Int f = 0; f < m_asset->m_numFaces; f++, indices += 3)
	{
		if (indices[0] < numMasterStrands && indices[1] < numMasterStrands && indices[2] < numMasterStrands)
		{
			m_faceIndices[f] = gfsdk_makeFloat3(float(indices[0]), float(indices[1]), float(indices[2]));
		}
	}
	// create GPU buffer for original (untessellated) face->hair indices
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(m_apiGlobal->getDevice(), DXGI_FORMAT_R32G32B32_FLOAT, sizeof(gfsdk_float3), m_asset->m_numFaces, m_faceIndices,
		m_faceHairIndicesBuffer.writeRef(), m_faceHairIndicesSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiAsset::_createFaceTexCoords()
{
	// per face texture coord to sample per hair attribute (e.g. stiffness) during render
	return Dx11Util::createReadOnlyBuffer(m_apiGlobal->getDevice(), DXGI_FORMAT_R32G32_FLOAT, sizeof(gfsdk_float2), m_asset->m_numFaces * 3, m_asset->m_faceTexCoords,
		m_faceTexCoordsBuffer.writeRef(), m_faceTexCoordsSrv.writeRef());
}

Result Dx11ApiAsset::_createHairTexCoords()
{
	// Function to calculate per hair texture coord to sample per hair attribute (e.g. stiffness) during compute
	NvCo::PodBuffer<gfsdk_float2> texCoordsBuf;
	m_asset->calcTexCoordBuffer(texCoordsBuf);
	// create GPU resources for hair textures
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(m_apiGlobal->getDevice(), DXGI_FORMAT_R32G32_FLOAT, sizeof(gfsdk_float2), UINT(texCoordsBuf.getSize()), texCoordsBuf,
		m_hairTexCoordsBuffer.writeRef(), m_hairTexCoordsSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiAsset::_createHairInteractionBuffers()
{
	ID3D11Device* device = m_apiGlobal->getDevice();

	NvCo::PodBuffer<Int32> offsets, indices;
	NvCo::PodBuffer<Float> lengths;
	m_asset->calcHairInteractionBuffers(offsets, lengths, indices);

	NV_CORE_ASSERT(offsets.getSize() == m_asset->m_numMasterStrandControlVertices);
	NV_CORE_ASSERT(lengths.getSize() == indices.getSize());

	// TODO - compact array to avoid duplicate edges
	// create buffer for indices
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_SINT, sizeof(int), UINT(indices.getSize()), indices,
		m_masterStrandInteractionIndicesBuffer.writeRef(), m_masterStrandInteractionIndicesSrv.writeRef()));
	// create buffer for lengths
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_FLOAT, sizeof(float), UINT(lengths.getSize()), lengths,
		m_masterStrandInteractionLengthBuffer.writeRef(), m_masterStrandInteractionLengthSrv.writeRef()));
	// create buffer for offset array
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_SINT, sizeof(int), UINT(offsets.getSize()), offsets,
		m_masterStrandInteractionOffsetBuffer.writeRef(), m_masterStrandInteractionOffsetSrv.writeRef()));

	m_totalNumHairInteractions = Int(indices.getSize());
	return NV_OK;
}

Result Dx11ApiAsset::_createMasterStrandBuffers()
{
	ID3D11Device* device = m_apiGlobal->getDevice();
	const Asset* asset = m_asset;

	{
		// create buffers for linear spring 
		// create CPU buffer for spring rest lengths
		NvCo::PodBuffer<float> linearSpringLengthsBuf(asset->m_numMasterStrandControlVertices);
		float* linearSpringLengths = linearSpringLengthsBuf;
		for (int i = 0; i < (int)asset->m_numMasterStrandControlVertices - 1; i++)
		{
			linearSpringLengths[i] = gfsdk_length3(asset->m_masterStrandControlVertices[i] - asset->m_masterStrandControlVertices[i + 1]);
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_FLOAT, sizeof(float), asset->m_numMasterStrandControlVertices, linearSpringLengths,
			m_masterStrandLinearSpringLengthsBuffer.writeRef(), m_masterStrandLinearSpringLengthsSrv.writeRef()));
	}

	// create GPU buffer for simulation hair indices
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_SINT, sizeof(int), asset->m_numMasterStrands, asset->m_masterStrandControlVertexOffsets,
		m_masterStrandOffsetBuffer.writeRef(), m_masterStrandOffsetSrv.writeRef()));
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_FLOAT, sizeof(float), asset->m_numMasterStrandControlVertices, asset->m_masterStrandNormalizedDistances,
		m_masterStrandNormalizedLengthToRootBuffer.writeRef(), m_masterStrandNormalizedLengthToRootSrv.writeRef()));

	{
		// create buffers for bend spring 
		// create CPU buffer for spring rest lengths
		NvCo::PodBuffer<float> bendSpringLengthsBuf(asset->m_numMasterStrandControlVertices);
		float* bendSpringLengths = bendSpringLengthsBuf;
		for (int i = 0; i < (int)asset->m_numMasterStrandControlVertices - 2; i++)
		{
			bendSpringLengths[i] = gfsdk_length3(asset->m_masterStrandControlVertices[i + 2] - asset->m_masterStrandControlVertices[i]);
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_FLOAT, sizeof(float), asset->m_numMasterStrandControlVertices, bendSpringLengths,
			m_masterStrandBendSpringLengthsBuffer.writeRef(), m_masterStrandBendSpringLengthsSrv.writeRef()));
	}

	{
		NvCo::PodBuffer<float> rootDistances;
		asset->calcMasterStrandRootDistances(rootDistances);
		// create GPU buffer
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_FLOAT, sizeof(float), UINT(rootDistances.getSize()), rootDistances,
			m_masterStrandRootDistancesBuffer.writeRef(), m_masterStrandRootDistancesSrv.writeRef()));
	}

	// create gpu buffer for original master strand
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), asset->m_numMasterStrandControlVertices, asset->m_masterStrandOriginalControlVertices,
		m_origMasterStrandBuffer.writeRef(), m_origMasterStrandSrv.writeRef()));

	{
		NvCo::PodBuffer<gfsdk_float4> frames, localPosPrev, localPosNext;
		asset->calcFrameBuffers(frames, localPosPrev, localPosNext);

		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), UINT(frames.getSize()), frames,
			m_origMasterFramesBuffer.writeRef(), m_origMasterFramesSrv.writeRef()));

		UINT numControlVertices = asset->m_numMasterStrandControlVertices;
		// NUTT
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), numControlVertices, asset->m_masterStrandControlVertices,
			m_origMasterPosBuffer.writeRef(), m_origMasterPosSrv.writeRef()));
		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), UINT(localPosPrev.getSize()), localPosPrev,
			m_prevMasterLocalPosBuffer.writeRef(), m_prevMasterLocalPosSrv.writeRef()));
		// create GPU buffer for original frames
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), UINT(localPosNext.getSize()), localPosNext,
			m_nextMasterLocalPosBuffer.writeRef(), m_nextMasterLocalPosSrv.writeRef()));

	}

	return NV_OK;
}

Result Dx11ApiAsset::_createSkinningBuffers()
{
	ID3D11Device* device = m_apiGlobal->getDevice();
	const Asset* asset = m_asset;

	// create GPU buffer for bone indices
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), asset->m_numMasterStrands, asset->m_boneIndices,
		m_boneIndicesBuffer.writeRef(), m_boneIndicesSrv.writeRef()));
	// create GPU buffer for bone weights
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), asset->m_numMasterStrands, asset->m_boneWeights,
		m_boneWeightsBuffer.writeRef(), m_boneWeightsSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiAsset::_createGrowthMeshBuffers()
{
	ID3D11Device* device = m_apiGlobal->getDevice();
	const Asset* asset = m_asset;

	// create GPU index buffer for growth mesh
	NV_RETURN_ON_FAIL(Dx11Util::createIndexBuffer(device, UINT(asset->m_faceIndices.getSize()), asset->m_faceIndices, m_growthMeshIndexBuffer.writeRef()));

	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), UINT(asset->m_growthMeshNormals.getSize()), asset->m_growthMeshNormals,
		m_growthMeshRestNormalBuffer.writeRef(), m_growthMeshRestNormalSrv.writeRef()));
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), UINT(asset->m_growthMeshTangents.getSize()), asset->m_growthMeshTangents,
		m_growthMeshRestTangentBuffer.writeRef(), m_growthMeshRestTangentSrv.writeRef()));

	return NV_OK;
}

Result Dx11ApiAsset::_createDebugRenderBuffers()
{
	ID3D11Device* device = m_apiGlobal->getDevice();

	// create buffer for vertex to hair map
	{
		NvCo::PodBuffer<Int32> vertexToHairBuffer;
		m_asset->calcVertexToHairMap(vertexToHairBuffer);
		NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(device, DXGI_FORMAT_R32_SINT, sizeof(int), Int(vertexToHairBuffer.getSize()), vertexToHairBuffer,
			m_debugVertexToHairBuffer.writeRef(), m_debugVertexToHairSrv.writeRef()));
	}

	// create index buffer for master strand
	{
		NvCo::PodBuffer<UInt32> indices;
		m_asset->calcMasterStrandIndices(indices);
		NV_RETURN_ON_FAIL(Dx11Util::createIndexBuffer(device, UINT(indices.getSize()) /* cnt * 2*/, indices, m_debugMasterIndexBuffer.writeRef()));
	}

	// create index buffer for frame visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		m_asset->calcFrameVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(Dx11Util::createIndexBuffer(device, Int(indices.getSize()), indices, m_debugFrameIndexBuffer.writeRef()));
	}
	// create index buffer for local pos visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		m_asset->calcLocalPosVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(Dx11Util::createIndexBuffer(device, Int(indices.getSize()), indices, m_debugLocalPosIndexBuffer.writeRef()));
	}
	// create index buffer for normal visualization
	{
		NvCo::PodBuffer<UInt32> indices;
		m_asset->calcNormalVisualizationIndices(indices);
		NV_RETURN_ON_FAIL(Dx11Util::createIndexBuffer(device, Int(indices.getSize()), indices, m_debugNormalIndexBuffer.writeRef()));
	}
	return NV_OK;
}

} // namespace HairWorks 
} // namespace nvidia 