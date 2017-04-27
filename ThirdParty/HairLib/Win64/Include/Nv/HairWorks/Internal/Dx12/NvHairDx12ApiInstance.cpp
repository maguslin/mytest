/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx12ApiInstance.h"

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>


#include <float.h>

#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include "NvHairDx12InstanceSimulate.h"

#include "NvHairDx12InstanceRender.h"
#include "NvHairDx12DebugRender.h"

#include "d3dx12.h"

namespace nvidia {
namespace HairWorks { 

#define NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING 1

Dx12ApiInstance::Dx12ApiInstance()
{
	// tessellated vertices
	m_tessellatedMasterStrandDoubleBufferStatus = 0;

	m_simulationStep = 0;

	// Initiallly debug resources are not set up -> set to uninitialized, and store the actual result
	// when the attempt is made to create
	//m_debugResourcesResult = NV_E_MISC_UNINITIALIZED;
}

Dx12ApiInstance::~Dx12ApiInstance()
{
	if (m_apiGlobal)
	{
		// Evict any descriptor handles that this used
		DescriptorSet set(m_viewHeap.getCpuStart(), m_viewHeap.getSize());
		m_apiGlobal->m_viewCache.evictIntersects(set);

		// Inform that the instance is being destroyed
		m_apiGlobal->onInstanceDestroyed(m_instance->m_instanceId);
	}
}

Result Dx12ApiInstance::init(Dx12ApiGlobal* apiGlobal, Instance& inst)
{
	const Asset* asset = inst.m_asset;
	m_apiAsset = static_cast<const Dx12ApiAsset*>(asset->m_apiAsset.get());
	m_apiGlobal = apiGlobal;
	m_instance = &inst;

	m_nullTextureSrvCpuHandle = apiGlobal->m_nullTextureSrvCpuHandle;

	m_numMasterStrands = asset->m_numMasterStrands;
	m_numMasterStrandControlVertices = asset->m_numMasterStrandControlVertices;
	m_numFaces = asset->m_numFaces;

	// shared heap
	NV_RETURN_ON_FAIL(m_viewHeap.init(apiGlobal->getDevice(), 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

	Dx12Info info = apiGlobal->getInfo(&m_viewHeap);

	NV_RETURN_ON_FAIL(_createMasterStrandVertexBuffers(info));
	NV_RETURN_ON_FAIL(_createMasterStrandTangentBuffers(info));

	NV_RETURN_ON_FAIL(_createStreamOutBuffers(info));

	NV_RETURN_ON_FAIL(_createGrowthMeshBuffers(info));

	if (asset->getDesc().m_numPins > 0)
	{
		NV_RETURN_ON_FAIL(_createPinConstraintBuffers(info, asset->getDesc()));
	}

	NV_RETURN_ON_FAIL(_createMasterStrandNormals(info));
	NV_RETURN_ON_FAIL(_createMasterStrandLuminancesBuffer(info));

	// Initialize all of the textures
	{
		for (IndexT i = 0; i < NV_COUNT_OF(m_textures); i++)
		{
			m_textures[i].init(&m_viewHeap);
		}
	}

	{
		for (IndexT i = 0; i < NV_COUNT_OF(m_bundles); i++)
		{
			NV_RETURN_ON_FAIL(m_bundles[i].init(apiGlobal->getDevice()));
		}
	}

	NV_RETURN_ON_FAIL(_createDebugResources());

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(Dx12ApiGlobal::STREAM_OUT_POSITION_BUFFER_SIZE));
		m_streamOutPositionBuffer.init(info, defaultHeapProps, resourceDesc, D3D12_RESOURCE_STATE_STREAM_OUT, NV_NULL);
		// Set at end, so on first iteration the position buffer will be initialized
		m_streamOutPositionGpuAddr = m_streamOutPositionBuffer.m_buffer->GetGPUVirtualAddress() + Dx12ApiGlobal::STREAM_OUT_POSITION_BUFFER_SIZE;
	}

	return NV_OK;
}

Result Dx12ApiInstance::_requireDebugResources()
{
	return NV_OK;

	/* if (m_debugResourcesResult == NV_E_MISC_UNINITIALIZED)
	{
		m_debugResourcesResult = _createDebugResources();
	}
	return m_debugResourcesResult; */
}

Result Dx12ApiInstance::_createDebugResources()
{
	Dx12Info info = m_apiGlobal->getInfo(&m_viewHeap);

	NV_RETURN_ON_FAIL(_createDebugTessellatedIndexBuffer(info));
	NV_RETURN_ON_FAIL(_createComputeStatsBuffers(info));
	NV_RETURN_ON_FAIL(_createMasterStrandFrames(info));
	
	return NV_OK;
}

Bool Dx12ApiInstance::isApiTextureUsed(ETextureType type)
{ 
	return m_textures[type].isSet();
}

Result Dx12ApiInstance::startEarlyPushSimulationBuffers(SliSystem*)
{
	return NV_OK;
}

Result Dx12ApiInstance::finishEarlyPushSimulationBuffers(SliSystem*)
{
	return NV_OK;
}

Result Dx12ApiInstance::setHintTessellationBuffers(SliSystem*)
{
	return NV_OK;
}

Result Dx12ApiInstance::setApiTexture(ETextureType type, const NvCo::ApiHandle& textureIn)
{
	ID3D12Resource* texture = NvCo::Dx12Type::cast<ID3D12Resource>(textureIn);

	Dx12Texture& dst = m_textures[type];

	if (texture != dst.m_texture)
	{
		// Tell the global cache, that the view may be wrong
		m_apiGlobal->m_viewCache.evict(dst.getSrvCpuHandle());
	}

	if (texture)
	{
		D3D12_RESOURCE_DESC desc = texture->GetDesc();
		// Set the texture
		dst.set(m_apiGlobal->getDevice(), texture, desc.Format);
	}
	else
	{
		dst.reset();
	}
	return NV_OK;
}

Result Dx12ApiInstance::getApiTextures(const ETextureType* types, Int numTextures, const NvCo::ApiPtr& textureOut)
{
	/// Check if inputs seem valid
	if (types == NV_NULL && numTextures != TextureType::COUNT_OF)
	{
		NV_CO_LOG_ERROR("numResources is the wrong size");
		return NV_FAIL;
	}

	// Access as resources
	{
		ID3D12Resource** dst = NvCo::Dx12Type::cast<ID3D12Resource*>(textureOut);
		if (dst)
		{
			if (types == NV_NULL)
			{
				for (Int i = 0; i < numTextures; i++)
				{
					dst[i] = m_textures[i].m_texture;
				}
			}
			else
			{
				for (Int i = 0; i < numTextures; i++)
				{
					dst[i] = m_textures[Int(types[i])].m_texture;
				}
			}
			return NV_OK;
		}
	}

	{
		// Want to access as descriptor handles
		D3D12_CPU_DESCRIPTOR_HANDLE* dst = NvCo::Dx12Type::cast<D3D12_CPU_DESCRIPTOR_HANDLE>(textureOut);
		if (dst)
		{
			getTextureSrvs(types, numTextures, dst);
			return NV_OK;
		}
	}
	return NV_OK;
}


Void Dx12ApiInstance::getTextureSrvs(const ETextureType* types, Int numTypes, D3D12_CPU_DESCRIPTOR_HANDLE* dst)
{
	if (types == NV_NULL)
	{
		for (Int i = 0; i < numTypes; i++)
		{
			dst[i] = m_textures[i].getSrvCpuHandle();
		}
	}
	else
	{
		for (Int i = 0; i < numTypes; i++)
		{
			dst[i] = m_textures[Int(types[i])].getSrvCpuHandle();
		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12ApiInstance::_getDescriptorHandle(EShaderResourceType t) const
{
	switch (t)
	{
		case ShaderResourceType::TANGENTS:				return m_tessellatedMasterStrandTangentsBuffer.getSrvCpuHandle();
		case ShaderResourceType::NORMALS:				return m_tessellatedMasterStrandNormalsBuffer.getSrvCpuHandle();
		case ShaderResourceType::HAIR_INDICES:			return m_apiAsset->m_faceHairIndicesBuffer.getSrvCpuHandle();
		case ShaderResourceType::MASTER_POSITIONS:		return getCurrentTessellatedMasterStrand().getSrvCpuHandle();
		case ShaderResourceType::PREV_MASTER_POSITIONS:	return getPreviousTessellatedMasterStrand().getSrvCpuHandle();
		default: NV_CORE_ASSERT(!"Unknown resource");
	}
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

ID3D12Resource* Dx12ApiInstance::_getResource(EShaderResourceType t) const
{
	switch (t)
	{
		case ShaderResourceType::TANGENTS:				return m_tessellatedMasterStrandTangentsBuffer.m_buffer;
		case ShaderResourceType::NORMALS:				return m_tessellatedMasterStrandNormalsBuffer.m_buffer;
		case ShaderResourceType::HAIR_INDICES:			return m_apiAsset->m_faceHairIndicesBuffer.m_buffer;
		case ShaderResourceType::MASTER_POSITIONS:		return getCurrentTessellatedMasterStrand().m_buffer;
		case ShaderResourceType::PREV_MASTER_POSITIONS:	return getPreviousTessellatedMasterStrand().m_buffer;
		default: NV_CORE_ASSERT(!"Unknown resource");
	}
	return NV_NULL;
}

Result Dx12ApiInstance::getApiResources(const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& resourceOut)
{
	/// Check if inputs seem valid
	if (types == NV_NULL && numResources != ShaderResourceType::COUNT_OF)
	{
		NV_CO_LOG_ERROR("numResources is the wrong size");
		return NV_FAIL;
	}

	// Check it we want the resources
	{
		ID3D12Resource** dst = NvCo::Dx12Type::cast<ID3D12Resource*>(resourceOut);
		if (dst)
		{
			if (types == NV_NULL)
			{
				for (Int i = 0; i < numResources; i++)
				{
					dst[i] = _getResource(EShaderResourceType(i));
				}
			}
			else
			{
				for (Int i = 0; i < numResources; i++)
				{
					dst[i] = _getResource(types[i]);
				}
			}
			return NV_OK;
		}
	}

	// Check if we want the shader resource views
	{
		D3D12_CPU_DESCRIPTOR_HANDLE* dst = NvCo::Dx12Type::cast<D3D12_CPU_DESCRIPTOR_HANDLE>(resourceOut);
		if (dst)
		{
			if (types == NV_NULL)
			{
				for (Int i = 0; i < numResources; i++)
				{
					dst[i] = _getDescriptorHandle(EShaderResourceType(i));
				}
			}
			else
			{
				for (Int i = 0; i < numResources; i++)
				{
					dst[i] = _getDescriptorHandle(types[i]);
				}
			}
			return NV_OK;
		}
	}

	// Couldn't handle
	return NV_FAIL;
}

Result Dx12ApiInstance::_createComputeStatsBuffers(const Dx12Info& info)
{
	const Asset* asset = m_instance->m_asset;	
	// CPU buffers for stats 
	{
		m_stats.setSize(asset->m_numFaces);
		const gfsdk_float4 initialValue = gfsdk_makeFloat4(1, 0, 0, 0);
		for (Int i = 0; i < asset->m_numFaces; ++i)
			m_stats[i] = initialValue;
	}
	// create GPU vertex buffer for sampled density buffer
	NV_RETURN_ON_FAIL(m_statsBuffer.init(info, sizeof(gfsdk_float4), asset->m_numFaces, m_stats));
	return NV_OK;
}

Result Dx12ApiInstance::_createMasterStrandVertexBuffers(const Dx12Info& info)
{
	// create vb buffer for initial (and previous) positions of the master strands
	const Asset* asset = m_instance->m_asset;
	UINT numVerts = asset->m_numMasterStrandControlVertices;

	//create structured GPU buffers for current particle positions
	NV_RETURN_ON_FAIL(m_masterStrandBuffer.init(info, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices));
	// GPU buffer to hold exact copy of last frames m_masterStrandBuffer, such that can be interpolated between for FIR.
	// m_masterStrandPrevBuffer is NOT an exact copy of previous frames positions
	NV_RETURN_ON_FAIL(m_masterStrandInterpBuffer.init(info, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices));

	//create structured GPU buffers for previous particle positions
	NV_RETURN_ON_FAIL(m_masterStrandPrevBuffer.init(info, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices));
	//create structured GPU buffers for skinned particle positions
	NV_RETURN_ON_FAIL(m_skinnedMasterStrandBuffer.init(info, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices));

	// GPU buffer to hold delta to fix hairs to skinning during interpolation
	NV_RETURN_ON_FAIL(m_masterStrandInterpolationDeltaBuffer.init(info, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices));

	return NV_OK;
}

Result Dx12ApiInstance::_createMasterStrandTangentBuffers(const Dx12Info& info)
{
	// create vb buffer for initial (and previous) positions of the master strands
	const Asset* asset = m_instance->m_asset;
	//create structured GPU buffers for tangents
	NV_RETURN_ON_FAIL(m_masterStrandTangentBuffer.init(info, sizeof(gfsdk_float4), asset->m_masterStrandTangents.getSize(), asset->m_masterStrandTangents));
	return NV_OK;
}

Result Dx12ApiInstance::_createPinConstraintBuffers(const Dx12Info& info, const AssetDescriptor& hairDesc)
{
	const Asset* asset = m_instance->m_asset;
	
	Int numPins = asset->getDesc().m_numPins;
	if (numPins <= 0)
		return NV_OK;

	asset->calcPins(m_pins);

	// create scratch buffer
	const int scratchSize = numPins * NV_HAIR_SCRATCH_SIZE_PER_PIN;
	
	//create structured GPU buffers for global scratch mem
	NV_RETURN_ON_FAIL(m_pinScratchBuffer.init(info, sizeof(NvHair_PinScratchData), scratchSize, NV_NULL, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	//create structured GPU buffers for global scratch mem
	NV_RETURN_ON_FAIL(m_pinsBuffer.init(info, sizeof(NvHair_Pin), hairDesc.m_numPins, m_pins, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	return NV_OK;
}

Result Dx12ApiInstance::_createGrowthMeshBuffers(const Dx12Info& info)
{
	const Asset* asset = m_instance->m_asset;
	// create GPU vertex buffer for growth mesh
	NV_RETURN_ON_FAIL(m_growthMeshVertexBuffer.init(info, sizeof(gfsdk_float4), asset->m_growthMeshVertices.getSize(), asset->m_growthMeshVertices));
	return NV_OK;
}

Result Dx12ApiInstance::_createStreamOutBuffers(const Dx12Info& info)
{
	// Function to create output buffers for splining stream-out
	Int numElem = m_instance->getMaxTessellatedPoints();
	// create the splined positions buffers and SRV
	NV_RETURN_ON_FAIL(m_tessellatedMasterStrandBuffer.init(info, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), numElem));
	// create the splined previous position buffers and SRV
	NV_RETURN_ON_FAIL(m_tessellatedMasterStrandPrevBuffer.init(info, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), numElem));
	// create the splined tangents buffers and SRV
	NV_RETURN_ON_FAIL(m_tessellatedMasterStrandTangentsBuffer.init(info, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), numElem));
	// create the splined normals buffers and SRV
	NV_RETURN_ON_FAIL(m_tessellatedMasterStrandNormalsBuffer.init(info, DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(gfsdk_float4), numElem));
	return NV_OK;
}

Result Dx12ApiInstance::_createMasterStrandNormals(const Dx12Info& info)
{
	const Asset* asset = m_instance->m_asset;
	Int numControlVertices = asset->m_numMasterStrandControlVertices;
	NvCo::PodBuffer<gfsdk_float4> normalsBuf(numControlVertices);
	gfsdk_float4* normals = normalsBuf;
	for (Int i = 0; i < numControlVertices; i++)
	{
		normals[i] = gfsdk_makeFloat4(0, 0, 1, 0);
	}
	// create GPU buffer for simulated normals
	NV_RETURN_ON_FAIL(m_masterStrandNormalBuffer.init(info, sizeof(gfsdk_float4), numControlVertices, normals));
	return NV_OK;
}

Result Dx12ApiInstance::_createMasterStrandFrames(const Dx12Info& info)
{
	NvCo::PodBuffer<gfsdk_float4> frames, localPosPrev, localPosNext;
	m_instance->m_asset->calcFrameBuffers(frames, localPosPrev, localPosNext);
	// create GPU buffer for current frames
	NV_RETURN_ON_FAIL(m_masterFramesBuffer.init(info, sizeof(gfsdk_float4), frames.getSize(), frames));
	return NV_OK;
}

Result Dx12ApiInstance::_createDebugTessellatedIndexBuffer(const Dx12Info& info)
{
	// create index buffer for tessellated strand
	int numTesselatedPoints = m_instance->getNumTessellatedPoints();
	int strandPointCount = m_instance->getStrandPointCount();
	{
		NvCo::PodBuffer<UInt32> indices(numTesselatedPoints * 2);

		Int cnt = 0;
		for (Int i = 0; i < m_numMasterStrands; i++)
		{
			Int start = i * strandPointCount;
			Int end = (i + 1) * strandPointCount - 1;
			for (Int j = start; j < end; j++)
			{
				indices[cnt * 2] = UInt32(j);
				indices[cnt * 2 + 1] = UInt32(j + 1);
				cnt++;
			}
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL(m_debugTessellatedIndexBuffer.init(info, cnt * 2, indices));
	}

	return NV_OK;
}

Result Dx12ApiInstance::_createMasterStrandLuminancesBuffer(const Dx12Info& info)
{
	// Set the default luminosity
	const UINT numControlVertices = m_apiAsset->m_asset->m_numMasterStrandControlVertices;

	NvCo::PodBuffer<Float32> luminancesBuf(numControlVertices);
	Float32* luminances = luminancesBuf;
	for (UINT i = 0; i < numControlVertices; i++)
	{
		// Negative numbers just display as the default color
		luminances[i] = -1.0f;
	}
	// create GPU buffer for simulated luminances
	NV_RETURN_ON_FAIL(m_masterStrandLuminanceBuffer.init(info, sizeof(Float32), numControlVertices, luminances, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	return NV_OK;
}

Dx12StreamOutBuffer& Dx12ApiInstance::nextTessellatedMasterStrandBuffer()
{
	Dx12StreamOutBuffer* positionSoBuffer = &m_tessellatedMasterStrandBuffer;
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	int dbStatus = m_tessellatedMasterStrandDoubleBufferStatus;
	positionSoBuffer = (dbStatus & 1) ? &m_tessellatedMasterStrandBuffer : &m_tessellatedMasterStrandPrevBuffer;
	// 0->1, 1->2, 2->3, 3->2
	dbStatus = (dbStatus + 1);
	dbStatus -= (dbStatus & 4) >> 1;
	m_tessellatedMasterStrandDoubleBufferStatus = dbStatus;
#endif
	return *positionSoBuffer;
}

Dx12StreamOutBuffer& Dx12ApiInstance::getPreviousTessellatedMasterStrand()
{
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	switch (m_tessellatedMasterStrandDoubleBufferStatus)
	{
		default:
		case 0:		return m_tessellatedMasterStrandPrevBuffer;
		// TODO JS: This looks suspicious... wouldn't you expect it to be m_tessellatedMasterStrandBuffer?
		case 1:		return m_tessellatedMasterStrandPrevBuffer;
		case 2:		return m_tessellatedMasterStrandPrevBuffer;
		case 3:		return m_tessellatedMasterStrandBuffer;
	}
#else
	return m_tessellatedMasterStrandPrevBuffer;
#endif
}

Dx12StreamOutBuffer&  Dx12ApiInstance::getCurrentTessellatedMasterStrand()
{
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	switch (m_tessellatedMasterStrandDoubleBufferStatus)
	{
		default:
		case 0:		return m_tessellatedMasterStrandBuffer;
		case 1:		return m_tessellatedMasterStrandPrevBuffer;
		case 2:		return m_tessellatedMasterStrandBuffer;
		case 3:		return m_tessellatedMasterStrandPrevBuffer;
	}
#else 
	return m_tessellatedMasterStrandBuffer;
#endif
}

Result Dx12ApiInstance::preRender(Float simulationInterp)
{
	typedef NvCo::Dx12CircularResourceHeap ResourceHeap;

	SliSystem* sliSystem = m_apiGlobal->getSliSystem();
	ID3D12GraphicsCommandList* commandList = m_apiGlobal->getCommandList();

	//------------------------NUT-----------------------------------------
	// check if buffers and resources are ready
	if (simulationInterp < 1.0f)
	{
		// Need to interpolate 
		NV_RETURN_ON_FAIL(Dx12InstanceRender::dispatchCalcInterpolationDelta(this, simulationInterp));
	}
	//------------------------------------------------------------------------

	if (sliSystem)
	{
		setHintTessellationBuffers(sliSystem);
	}

	Dx12ShaderPass& pass = m_apiGlobal->m_passes[Dx12ApiGlobal::PASS_SPLINE];

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{
			m_masterStrandBuffer.getSrvCpuHandle(),
			m_masterStrandInterpBuffer.getSrvCpuHandle(),
			m_masterStrandInterpolationDeltaBuffer.getSrvCpuHandle(),
			m_masterStrandNormalBuffer.getSrvCpuHandle(),
		};
		viewHeap = m_apiGlobal->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}

	ResourceHeap::Cursor splineHeapCursor;
	{
		splineHeapCursor = m_apiGlobal->m_uploadHeap.allocateConstantBuffer(sizeof(NvHair_SplineConstantBuffer));
		const Instance* inst = m_instance;
		const Asset* asset = inst->m_asset;

		NvHair_SplineConstantBuffer* dst = (NvHair_SplineConstantBuffer*)splineHeapCursor.m_position;
		// shared constant values regardless of material
		dst->modelToWorld = inst->m_modelToWorld;
		dst->numVertsPerSegments = inst->m_numVertsPerSegment;
		dst->numMasterSegments = asset->m_numMasterSegmentsPerHair;
		dst->strandPointCounts = inst->getStrandPointCount();
		dst->numTessellatedPoints = inst->getNumTessellatedPoints();
		dst->simulationInterp = simulationInterp;
	}

	Dx12StreamOutBuffer& tessellatedMasterStrand = nextTessellatedMasterStrandBuffer();

	// Check if run out of space on streamOutPositionBuffer, if so reset by copying zeros over the top
	if (m_streamOutPositionGpuAddr + sizeof(UInt64) * 3 > m_streamOutPositionBuffer.m_buffer->GetGPUVirtualAddress() + Dx12ApiGlobal::STREAM_OUT_POSITION_BUFFER_SIZE)
	{
		{
			Dx12BarrierSubmitter barriers(commandList);
			m_streamOutPositionBuffer.transition(D3D12_RESOURCE_STATE_COPY_DEST, barriers);
		}
		commandList->CopyResource(m_streamOutPositionBuffer.m_buffer, m_apiGlobal->m_streamOutClearBuffer.m_buffer);
		{
			Dx12BarrierSubmitter barriers(commandList);
			m_streamOutPositionBuffer.transition(D3D12_RESOURCE_STATE_STREAM_OUT, barriers);
		}
		m_streamOutPositionGpuAddr = m_streamOutPositionBuffer.m_buffer->GetGPUVirtualAddress();
	}

	{
		Dx12BarrierSubmitter barriers(commandList);
		m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		m_masterStrandInterpBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		m_masterStrandNormalBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		tessellatedMasterStrand.transition(D3D12_RESOURCE_STATE_STREAM_OUT, barriers);
		m_tessellatedMasterStrandTangentsBuffer.transition(D3D12_RESOURCE_STATE_STREAM_OUT, barriers);
		m_tessellatedMasterStrandNormalsBuffer.transition(D3D12_RESOURCE_STATE_STREAM_OUT, barriers);
	}

	
	{
		D3D12_GPU_VIRTUAL_ADDRESS streamOutPositionGpuAddr = m_streamOutPositionGpuAddr;
		m_streamOutPositionGpuAddr += sizeof(UInt64) * 3;
		D3D12_STREAM_OUTPUT_BUFFER_VIEW views[] = 
		{ 
			tessellatedMasterStrand.nextStreamOutView(streamOutPositionGpuAddr),
			m_tessellatedMasterStrandTangentsBuffer.nextStreamOutView(streamOutPositionGpuAddr + sizeof(UInt64)),
			m_tessellatedMasterStrandNormalsBuffer.nextStreamOutView(streamOutPositionGpuAddr + 2 * sizeof(UInt64))
		};
		commandList->SOSetTargets(0, NV_COUNT_OF(views), views);
	}

	ID3D12DescriptorHeap* heaps[] = { viewHeap };
	
	if (false)
	{
		Dx12Bundle& bundle = m_bundles[BUNDLE_TESSELATE];
		ID3D12GraphicsCommandList* bundleList = bundle.getCommandList();
		if (bundle.start(heaps, NV_COUNT_OF(heaps), viewHandles, NV_COUNT_OF(viewHandles)))
		{
			bundleList->SetPipelineState(pass.m_pipelineState);
			bundleList->SetGraphicsRootSignature(pass.m_rootSignature);
			bundleList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
			bundleList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);
			bundleList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			bundleList->DrawInstanced(m_instance->getNumTessellatedPoints(), 1, 0, 0);
			bundle.end();
		}
		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		commandList->SetGraphicsRootConstantBufferView(0, splineHeapCursor.getGpuHandle());
		commandList->ExecuteBundle(bundleList);
	}
	else
	{
		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->SetGraphicsRootSignature(pass.m_rootSignature);

		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		commandList->SetGraphicsRootConstantBufferView(0, splineHeapCursor.getGpuHandle());
		commandList->DrawInstanced(m_instance->getNumTessellatedPoints(), 1, 0, 0);
	}

	// Reset stream out targets
	commandList->SOSetTargets(0, 0, NV_NULL);
	
	{
		Dx12BarrierSubmitter barriers(commandList);
		m_masterStrandBuffer.restore(barriers);
		m_masterStrandInterpBuffer.restore(barriers);
		m_masterStrandNormalBuffer.restore(barriers);
		tessellatedMasterStrand.restore(barriers);
		m_tessellatedMasterStrandTangentsBuffer.restore(barriers);
		m_tessellatedMasterStrandNormalsBuffer.restore(barriers);
	}

	return NV_OK;
}

Result Dx12ApiInstance::updatePinConstraintBuffer()
{
	// This is a little different from Dx11 in that it doesn't map the buffer, set the pin data and then remap.
	// It just recalcs from scatch and uploads.

	Instance* inst = m_instance;
	const Asset* asset = inst->m_asset;
	ID3D12GraphicsCommandList* commandList = m_apiGlobal->getCommandList();
	Dx12ApiGlobal* glob = m_apiGlobal;

	const Dx12Info info = glob->getInfo(NV_NULL);

	const Int prevNumPins = Int(m_pins.getSize());
	asset->calcPins(m_pins);
	const Int numPins = Int(m_pins.getSize());

	if (prevNumPins != numPins)
	{
		// Free up any previous resources
		if (prevNumPins > 0)
		{
			// Free this...
			glob->m_resourceScopeManager.add(m_pinsBuffer.m_buffer.detach());
			glob->m_resourceScopeManager.add(m_pinScratchBuffer.m_buffer.detach());
			// Evict all the handles
			const D3D12_CPU_DESCRIPTOR_HANDLE handles[] =
			{
				m_pinsBuffer.getSrvCpuHandle(),
				m_pinsBuffer.getUavCpuHandle(),
				m_pinScratchBuffer.getSrvCpuHandle(),
				m_pinScratchBuffer.getUavCpuHandle(),
			};
			// Tell the cache these have changed
			glob->m_viewCache.evictIntersects(DescriptorSet(DescriptorSet::TYPE_UNKNOWN, handles));
		}
		// Set up new buffers
		if (numPins > 0)
		{
			// create scratch buffer
			const int scratchSize = numPins * NV_HAIR_SCRATCH_SIZE_PER_PIN;
			//create structured GPU buffers for global scratch mem
			NV_RETURN_ON_FAIL(m_pinScratchBuffer.update(info, sizeof(NvHair_PinScratchData), scratchSize, NV_NULL, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			//create structured GPU buffers for global scratch mem
			NV_RETURN_ON_FAIL(m_pinsBuffer.update(info, sizeof(NvHair_Pin), numPins, m_pins, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		}
		return NV_OK;
	}
	if (numPins == 0)
	{
		return NV_OK;
	}

	// Okay.. all we need to do is upload the data
	{
		{
			Dx12BarrierSubmitter barriers(commandList);
			m_pinsBuffer.transition(D3D12_RESOURCE_STATE_COPY_DEST, barriers);
		}

		const SizeT bufferSize = sizeof(NvHair_Pin) * numPins;
		NvCo::Dx12CircularResourceHeap::Cursor cursor = glob->m_uploadHeap.allocate(bufferSize, 16);
		NvCo::Memory::copy(cursor.m_position, m_pins.begin(), bufferSize);
		commandList->CopyBufferRegion(m_pinsBuffer.m_buffer, 0, cursor.getResource(), cursor.getOffset(), bufferSize);

		{
			Dx12BarrierSubmitter barriers(commandList);
			m_pinsBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		}
	}

	return NV_OK;
}

Result Dx12ApiInstance::stepSimulation(float timeStep, const gfsdk_float4x4* worldReference)
{
	m_simulationStep++;

	NV_RETURN_ON_FAIL(Dx12InstanceSimulate::stepSimulation(this, timeStep, worldReference));
	return NV_OK;
}

Result Dx12ApiInstance::renderHairShading(const RenderViewInfo& frameViewInfo, ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(other)

	NV_RETURN_ON_FAIL(Dx12InstanceRender::renderHairShading(this, frameViewInfo, shaderSettings, other));
	return NV_OK;
}

Void Dx12ApiInstance::calcPixelConstantBuffer(const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut)
{
	Dx12InstanceRender::calcPixelConstantBuffer(this, frameViewInfo, constantBufferOut);
}

Result Dx12ApiInstance::_computeStats(NvCo::Dx12CircularResourceHeap::Cursor& cursorOut)
{
	Instance* inst = m_instance;
	const Asset* asset = inst->m_asset;
	Dx12ApiGlobal* glob = m_apiGlobal;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	if (!commandList)
	{	
		return NV_FAIL;
	}

	Dx12StructuredBuffer& buffer = m_statsBuffer;

	{
		Dx12BarrierSubmitter barriers(commandList);
		buffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	const Dx12ShaderPass& pass = glob->m_computeStats;
	
	const InstanceDescriptor& params = m_instance->getDefaultMaterial();
	
	//	constants for compute shader
	const int cbpsPerFrameBind = 0;
	//	fill in the constant buffer
	NvCo::Dx12CircularResourceHeap::Cursor cbCursor;
	{
		NvHair_StatsPerFrameConstantBuffer buf;
		buf.numFaces = asset->m_numFaces;
		buf.density = params.m_density;
		buf.usePixelDensity = params.m_usePixelDensity;
		cbCursor = glob->m_uploadHeap.newConstantBuffer(buf);
	}

	ID3D12DescriptorHeap* viewHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2];
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			m_apiAsset->m_faceTexCoordsBuffer.getSrvCpuHandle(),
			getTextureSrv(TextureType::DENSITY),
			glob->m_scalarNoiseBuffer.getSrvCpuHandle(),
			glob->m_strandCoordinatesBuffer.getSrvCpuHandle(),
		};
		//set the unordered access views - this is what the CS shader will read from and write to
		//we bind two buffers, one containing the particle positions, and the other containing particle positions from the last frame
		D3D12_CPU_DESCRIPTOR_HANDLE uavs[] = 
		{
			buffer.getUavCpuHandle(),
		};
		const DescriptorSet sets[] =
		{
			DescriptorSet(DescriptorSet::TYPE_SRV, srvs),
			DescriptorSet(DescriptorSet::TYPE_UAV, uavs),
		};
		viewHeap = glob->m_viewCache.put(sets, 0, viewHandles);
	}	

	{
		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->SetComputeRootSignature(pass.m_rootSignature);

		ID3D12DescriptorHeap* heaps[] = { viewHeap, glob->m_samplerHeap.getHeap() };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());
		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
		commandList->SetComputeRootDescriptorTable(3, glob->m_samplerHeap.getGpuStart());

		// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
		commandList->Dispatch(asset->m_numFaces, 1, 1);
	}

	// Copy to the the readback heap
	{
		const SizeT bufferSize = sizeof(gfsdk_float4) * asset->m_numFaces;
		NvCo::Dx12CircularResourceHeap::Cursor readBackCursor = glob->m_readBackHeap.allocate(bufferSize, 16);
		// Wait til buffer is a source
		{
			Dx12BarrierSubmitter barriers(commandList);
			buffer.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, barriers);
		}
		commandList->CopyBufferRegion(readBackCursor.getResource(), readBackCursor.getOffset(), buffer.m_buffer, 0, bufferSize);
		{
			// Wait for the copy to complete
			Dx12BarrierSubmitter barriers(commandList);
			buffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		}
		cursorOut = readBackCursor;
	}

	return NV_OK;
}

Void Dx12ApiInstance::_completeComputeStats(Dx12ComputeStatsAsync& async)
{
	ID3D12Resource* resource = async.m_readBackCursor.getResource();
	SizeT offset = async.m_readBackCursor.getOffset();

	const Int numFaces = m_apiAsset->m_asset->m_numFaces;
	SizeT size = sizeof(gfsdk_float4) * numFaces;

	UInt8* start = NV_NULL;	
	D3D12_RANGE range = { offset, offset + size };
	resource->Map(0, &range, (Void**)&start);

	const gfsdk_float4* stats = (const gfsdk_float4*)(start + offset);
	Float densitySum = 0.0f;
	for (Int i = 0; i < numFaces; i++)
	{
		densitySum += stats[i].x;
	}
	// Calculate and store result
	m_instance->computeStats(densitySum, async.m_stats);

	// Unmap it
	resource->Unmap(0, NV_NULL);
}

Result Dx12ApiInstance::computeStats(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Stats& statsOut)
{
	NvCo::Dx12AsyncManager& asyncMan = m_apiGlobal->m_asyncManager;

	// Handle completion
	{
		Dx12ComputeStatsAsync* async;
		NV_RETURN_ON_FAIL(asyncMan.complete(asyncInOut, asyncRepeat, &async));
		if (async)
		{
			statsOut = async->m_stats;
			asyncMan.releaseCompleted(async);	
			if (asyncRepeat)
			{
				// NOTE! Correctly ignores the repeat calls result
				computeStats(asyncInOut, false, statsOut);
			}
			return NV_OK;
		}
	}

	if (isTextureUsed(TextureType::DENSITY))
	{
		if (asyncInOut == NV_NULL)
		{
			// Cannot block
			return NV_FAIL;
		}
		// See if it's was requested and nothing has changed
		{
			Dx12ComputeStatsAsync* async = asyncMan.findAndRef<Dx12ComputeStatsAsync>(NvCo::Dx12Async::STATE_NEW, (Void*)m_instance->m_instanceId, 0);
			if (async)
			{
				*asyncInOut = asyncMan.getHandle(async);
				return NV_E_MISC_PENDING;
			}
		}

		NV_RETURN_ON_FAIL(_requireDebugResources());

		Dx12ComputeStatsAsync* async = asyncMan.create<Dx12ComputeStatsAsync>((Void*)m_instance->m_instanceId, 0);
		NV_RETURN_ON_FAIL(_computeStats(async->m_readBackCursor));
		*asyncInOut = asyncMan.getHandle(async);
		return NV_E_MISC_PENDING;
	}
	else
	{
		Float densitySum = m_instance->getDefaultMaterial().m_density * float(m_apiAsset->m_asset->m_numFaces);
		m_instance->computeStats(densitySum, statsOut);
		return NV_OK;
	}
}

Void Dx12ApiInstance::_completeGetPinMatrix(Dx12GetPinMatrixAsync& async)
{
	// Copy all of the matrices
	ID3D12Resource* resource = async.m_readBackCursor.getResource();
	SizeT offset = async.m_readBackCursor.getOffset();
	const SizeT size = sizeof(NvHair_Pin) * async.m_numPins;

	D3D12_RANGE range;
	range.Begin = offset;
	range.End = offset + size;

	UInt8* start = NV_NULL;
	Result res = resource->Map(0, &range, (Void**)&start);
	NV_CORE_ASSERT(NV_SUCCEEDED(res));

	NvHair_Pin* pins = (NvHair_Pin*)(start + offset);

	gfsdk_float4x4* dst = async.m_pinMatrices;
	for (Int i = 0; i < async.m_numPins; i++)
	{
		dst[i] = pins[i].currentPinMatrix;
	}
	// Unmap it
	resource->Unmap(0, NV_NULL);
}

Result Dx12ApiInstance::getPinMatrix(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int pinId, gfsdk_float4x4& matrixOut)
{
	return getPinMatrices(asyncInOut, asyncRepeat, pinId, 1, &matrixOut);
}

Result Dx12ApiInstance::getPinMatrices(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int startIndex, Int size, gfsdk_float4x4* matricesOut)
{
	Dx12ApiGlobal* glob = m_apiGlobal;
	const Int numPins = Int(m_pins.getSize());
	NvCo::Dx12AsyncManager& asyncMan = glob->m_asyncManager;
	
	// Handle completion
	{
		Dx12GetPinMatrixAsync* async;
		NV_RETURN_ON_FAIL(asyncMan.complete(asyncInOut, asyncRepeat, &async));
		if (async)
		{
			NV_CORE_ASSERT(numPins == async->m_numPins);
			for (Int i = 0; i < size; i++)
			{
				const gfsdk_float4x4 pinMatrix = async->m_pinMatrices[i + startIndex];
				matricesOut[i] = pinMatrix * async->m_modelToWorld;
			}
			asyncMan.releaseCompleted(async);
			if (asyncRepeat)
			{
				// NOTE! Correctly ignores the repeat calls result
				getPinMatrices(asyncInOut, false, startIndex, size, matricesOut);
			}
			return NV_OK;
		}
	}

	if (numPins <= 0)
	{
		return NV_OK;
	}
	NV_CORE_ASSERT(startIndex >= 0 && startIndex + size <= numPins);
	if (startIndex < 0 || size < 0 || startIndex + size > numPins)
	{
		return NV_FAIL;
	}

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	if (commandList == NV_NULL)
		return NV_FAIL;
	if (asyncInOut == NV_NULL)
	{
		// Cannot block
		return NV_FAIL;
	}

	// See if it's was requested and nothing has changed
	{
		Dx12GetPinMatrixAsync* async = asyncMan.findAndRef<Dx12GetPinMatrixAsync>(NvCo::Dx12Async::STATE_NEW, (Void*)m_instance->m_instanceId, Int32(m_simulationStep));
		if (async)
		{
			*asyncInOut = asyncMan.getHandle(async);
			return NV_E_MISC_PENDING;
		}
	}

	{
		SizeT totalSize = sizeof(Dx12GetPinMatrixAsync) + sizeof(gfsdk_float4x4) * (numPins - 1);
		Dx12GetPinMatrixAsync* async = asyncMan.create<Dx12GetPinMatrixAsync>((Void*)m_instance->m_instanceId, m_simulationStep, totalSize);

		async->m_numPins = numPins;
		async->m_modelToWorld = m_instance->m_modelToWorld;

		Dx12StructuredBuffer& buffer = m_pinsBuffer;

		const SizeT bufferSize = sizeof(NvHair_Pin) * numPins;
		NvCo::Dx12CircularResourceHeap::Cursor readBackCursor = glob->m_readBackHeap.allocate(bufferSize, 16);
		// Wait until buffer is a source
		{
			Dx12BarrierSubmitter barriers(commandList);
			buffer.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, barriers);
		}
		// Copy 
		commandList->CopyBufferRegion(readBackCursor.getResource(), readBackCursor.getOffset(), buffer.m_buffer, 0, bufferSize);
		{
			// Wait for the copy to complete
			Dx12BarrierSubmitter barriers(commandList);
			buffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		}
		async->m_readBackCursor = readBackCursor;

		*asyncInOut = asyncMan.getHandle(async);
		return NV_E_MISC_PENDING;
	}
}

Result Dx12ApiInstance::debugDraw(EDebugDraw drawType, const ViewInfo& viewInfo)
{
	NV_RETURN_ON_FAIL(_requireDebugResources());

	switch (drawType)
	{
		case DebugDraw::GUIDE_HAIRS:					return Dx12DebugRender::drawGuideHairs(this, viewInfo);
		case DebugDraw::GROWTH_MESH:					return Dx12DebugRender::drawGrowthMesh(this, viewInfo);
		case DebugDraw::SKINNED_GUIDE_HAIRS:			return Dx12DebugRender::drawSkinnedGuideHairs(this, viewInfo);
		case DebugDraw::FRAMES:							return Dx12DebugRender::drawFrames(this, viewInfo);
		case DebugDraw::LOCAL_POS:						return Dx12DebugRender::drawLocalPos(this, viewInfo);
		case DebugDraw::NORMALS:						return Dx12DebugRender::drawNormals(this, viewInfo);
		case DebugDraw::HAIR_INTERACTION:				return Dx12DebugRender::drawHairInteraction(this, viewInfo);
		case DebugDraw::GUIDE_HAIR_CONTROL_VERTICES:	return Dx12DebugRender::drawGuideHairControlVertices(this, viewInfo);
		case DebugDraw::PIN_CONSTRAINTS:				return Dx12DebugRender::drawPinConstraints(this, viewInfo);
		default: break;
	}
	return NV_FAIL;
}

Result Dx12ApiInstance::onNumVertsPerSegmentChanged()
{
	// Have to recreate the debug buffers...
	if (m_debugTessellatedIndexBuffer.m_buffer)
	{
		m_apiGlobal->m_resourceScopeManager.add(m_debugTessellatedIndexBuffer.m_buffer);
		m_debugTessellatedIndexBuffer.reset();
	}
	// Initialize it again
	NV_CORE_ASSERT_ON_FAIL(_createDebugTessellatedIndexBuffer(m_apiGlobal->getInfo(NV_NULL)));

	// Invalidate all bundles
	for (IndexT i = 0; i < NV_COUNT_OF(m_bundles); i++)
	{
		m_bundles[i].reset();
	}

	return NV_OK;
}

} // namespace HairWorks 
} // namespace nvidia 