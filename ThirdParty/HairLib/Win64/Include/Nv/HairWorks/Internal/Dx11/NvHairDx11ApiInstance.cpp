/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx11ApiInstance.h"

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include <float.h>

#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include "NvHairDx11Util.h"

#include "NvHairDx11InstanceSimulate.h"
#include "NvHairDx11InstanceRender.h"
#include "NvHairDx11DebugRender.h"

namespace nvidia {
namespace HairWorks { 

#define NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING 1

Dx11ApiInstance::Dx11ApiInstance()
{
	// tessellated vertices
	m_tessellatedMasterStrandDoubleBufferStatus = 0;

	// Sli 
	m_growthMeshVertexSliHandle = NV_NULL;
	m_statsSliHandle = NV_NULL;
	m_skinnedMasterStrandSliHandle = NV_NULL;
	m_masterFramesSliHandle = NV_NULL;
	m_masterStrandTangentSliHandle = NV_NULL;
	m_masterStrandNormalSliHandle = NV_NULL;
	m_masterStrandLuminanceSliHandle = NV_NULL;
	m_pinConstraintsSliHandle = NV_NULL;
	m_scratchSliHandle = NV_NULL;

	m_masterStrandSliHandle = NV_NULL;
	m_masterStrandPrevSliHandle = NV_NULL;

	m_tessellatedMasterStrandSliHandle = NV_NULL;
	m_tessellatedMasterStrandPrevSliHandle = NV_NULL;
	m_tessellatedMasterStrandTangentSliHandle = NV_NULL;

	// Initiallly debug resources are not set up -> set to uninitialized, and store the actual result
	// when the attempt is made to create
	m_debugResourcesResult = NV_E_MISC_UNINITIALIZED;
}

Result Dx11ApiInstance::init(Dx11ApiGlobal* apiGlobal, Instance& inst)
{
	const Asset* asset = inst.m_asset;
	m_apiAsset = static_cast<const Dx11ApiAsset*>(asset->m_apiAsset.get());
	m_apiGlobal = apiGlobal;
	m_instance = &inst;

	const AssetDescriptor& assetDesc = asset->getDesc();

	ID3D11Device* device = apiGlobal->getDevice();

	m_numMasterStrands = asset->m_numMasterStrands;
	m_numMasterStrandControlVertices = asset->m_numMasterStrandControlVertices;
	m_numFaces = asset->m_numFaces;

	NV_RETURN_ON_FAIL(_createMasterStrandVertexBuffers(device, inst));
	NV_RETURN_ON_FAIL(_createMasterStrandTangentBuffers(device, inst));

	NV_RETURN_ON_FAIL(_createStreamOutBuffers(device, inst));

	NV_RETURN_ON_FAIL(_createComputeShaderConstantBuffers(device));
	NV_RETURN_ON_FAIL(_createShadingConstantBuffers(device));

	NV_RETURN_ON_FAIL(_createGrowthMeshBuffers(device, inst));

	if (assetDesc.m_numPins > 0)
	{
		NV_RETURN_ON_FAIL(_createPinConstraintBuffers(device, inst, assetDesc));
	}

	NV_RETURN_ON_FAIL(_createMasterStrandNormals(device, inst));

	return NV_OK;
}

Result Dx11ApiInstance::_requireDebugResources()
{
	if(m_debugResourcesResult == NV_E_MISC_UNINITIALIZED)
	{
		m_debugResourcesResult = _createDebugResources();
	}
	return m_debugResourcesResult;
}

Result Dx11ApiInstance::_createDebugResources()
{
	ID3D11Device* device = m_apiGlobal->getDevice();

	NV_RETURN_ON_FAIL(_createDebugTessellatedIndexBuffer(device, *m_instance));
	NV_RETURN_ON_FAIL(_createDebugConstantBuffers(device));
	NV_RETURN_ON_FAIL(_createComputeStatsBuffers(device, *m_instance));

	NV_RETURN_ON_FAIL(_createMasterStrandFrames(device, *m_instance));
	NV_RETURN_ON_FAIL(_createMasterStrandLuminances(device, *m_instance));

	return NV_OK;
}

Bool Dx11ApiInstance::isApiTextureUsed(ETextureType type)  
{ 
	return m_textures[IndexT(type)].get() != NV_NULL; 
}

Result Dx11ApiInstance::startEarlyPushSimulationBuffers(SliSystem* system)
{
	// apply early push for UAVs
	// set SRH for other buffers
	if (system)
	{
		// set SRH for buffers that do not need synchronization for SLI
		system->setResourceHint(m_growthMeshVertexSliHandle, NvCo::Dx11Type::wrap(m_growthMeshVertexBuffer), "GrowthMeshVertexBuffer");
		system->setResourceHint(m_statsSliHandle, NvCo::Dx11Type::wrap(m_statsBuffer), "StatsBuffer");
		system->setResourceHint(m_skinnedMasterStrandSliHandle, NvCo::Dx11Type::wrap(m_skinnedMasterStrandBuffer), "SkinnedMasterStrandBuffer");
		system->setResourceHint(m_masterFramesSliHandle, NvCo::Dx11Type::wrap(m_masterFramesBuffer), "MasterFramesBuffer");
		system->setResourceHint(m_masterStrandTangentSliHandle, NvCo::Dx11Type::wrap(m_masterStrandTangentBuffer), "MasterStrandTangentBuffer");
		system->setResourceHint(m_masterStrandNormalSliHandle, NvCo::Dx11Type::wrap(m_masterStrandNormalBuffer), "MasterStrandNormalBuffer");
		system->setResourceHint(m_masterStrandLuminanceSliHandle, NvCo::Dx11Type::wrap(m_masterStrandLuminanceBuffer), "MasterStrandLuminanceBuffer");
		system->setResourceHint(m_pinConstraintsSliHandle, NvCo::Dx11Type::wrap(m_pinsBuffer), "PinsBuffer");
		system->setResourceHint(m_scratchSliHandle, NvCo::Dx11Type::wrap(m_pinScratchBuffer), "ScratchBuffer");

		// early push UAVs that needed to be kept in AFR mode
		system->earlyPushStart(m_masterStrandSliHandle, NvCo::Dx11Type::wrap(m_masterStrandBuffer), "MasterStrandBuffer");
		system->earlyPushStart(m_masterStrandPrevSliHandle, NvCo::Dx11Type::wrap(m_masterStrandPrevBuffer), "MasterStrandPrevBuffer");
	}
	return NV_OK;
}

Result Dx11ApiInstance::finishEarlyPushSimulationBuffers(SliSystem* system)
{
	// finish early push for UAVs
	// set SRH for other buffers
	if (system)
	{
		system->earlyPushFinish(m_masterStrandPrevSliHandle, "MasterStrandPrevBuffer");
		system->earlyPushFinish(m_masterStrandSliHandle, "MasterStrandBuffer");
		system->flushLog();
	}
	return NV_OK;
}

Result Dx11ApiInstance::setHintTessellationBuffers(SliSystem* system)
{
	if (system)
	{
		// Mark stream out buffer for GS tessellation as not kept in AFR
		system->setResourceHint(m_tessellatedMasterStrandSliHandle, NvCo::Dx11Type::wrap(m_tessellatedMasterStrandBuffer), "TessellatedMasterStrandBuffer-SO");
		system->setResourceHint(m_tessellatedMasterStrandPrevSliHandle, NvCo::Dx11Type::wrap(m_tessellatedMasterStrandBuffer), "TessellatedMasterStrandPrevBuffer-SO");
		system->setResourceHint(m_tessellatedMasterStrandTangentSliHandle, NvCo::Dx11Type::wrap(m_tessellatedMasterStrandTangentsBuffer), "TessellatedMasterStrandTangentBuffer-SO");
		system->setResourceHint(m_tessellatedMasterStrandNormalSliHandle, NvCo::Dx11Type::wrap(m_tessellatedMasterStrandNormalsBuffer), "TessellatedMasterStrandNormalBuffer-SO");
		system->flushLog();
	}
	return NV_OK;
}

Result Dx11ApiInstance::setApiTexture(ETextureType type, const NvCo::ApiHandle& texture)
{
	m_textures[Int(type)] = NvCo::Dx11Type::cast<ID3D11ShaderResourceView>(texture);
	return NV_OK;
}

Result Dx11ApiInstance::getApiTextures(const ETextureType* types, Int numTextures, const NvCo::ApiPtr& textureOut)
{
	ID3D11ShaderResourceView** dst = NvCo::Dx11Type::cast<ID3D11ShaderResourceView*>(textureOut);
	if (!dst)
	{
		return NV_FAIL;
	}
	if (types == NV_NULL)
	{
		if (numTextures != Int(TextureType::COUNT_OF))
		{
			NV_CO_LOG_ERROR("Wrong amount of textures");
			return NV_FAIL;
		}
		for (Int i = 0; i < numTextures; i++)
		{
			dst[i] = m_textures[i];
		}
	}
	else
	{
		for (Int i = 0; i < numTextures; i++)
		{
			dst[i] = m_textures[Int(types[i])];
		}
	}
	return NV_OK;
}

ID3D11ShaderResourceView* Dx11ApiInstance::_getResource(EShaderResourceType t) const
{
	switch (t)
	{
		case ShaderResourceType::TANGENTS:				return m_tessellatedMasterStrandTangentsSrv;
		case ShaderResourceType::NORMALS:				return m_tessellatedMasterStrandNormalsSrv;
		case ShaderResourceType::HAIR_INDICES:			return m_apiAsset->m_faceHairIndicesSrv;
		case ShaderResourceType::MASTER_POSITIONS:		return getCurrentTessellatedMasterStrandSrv();
		case ShaderResourceType::PREV_MASTER_POSITIONS:	return getPreviousTessellatedMasterStrandSrv();

		//case ShaderResourceType::MASTER_POSITIONS:		return m_tessellatedMasterStrandSrv;
		//case ShaderResourceType::PREV_MASTER_POSITIONS:	return m_tessellatedMasterStrandPrevSrv;
		default: NV_CORE_ASSERT(!"Unknown resource");
	}
	return NV_NULL;
}

Result Dx11ApiInstance::getApiResources(const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& resourceOut)
{
	ID3D11ShaderResourceView** dst = NvCo::Dx11Type::cast<ID3D11ShaderResourceView*>(resourceOut);
	if (!dst)
	{
		return NV_FAIL;
	}
	if (types == NV_NULL)
	{
		if (numResources != Int(ShaderResourceType::COUNT_OF))
		{
			NV_CO_LOG_ERROR("numResources is the wrong size");
			return NV_FAIL;
		}
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

Result Dx11ApiInstance::_createDebugConstantBuffers(ID3D11Device* device)
{
	return Dx11Util::createConstantBuffer(device, sizeof(NvHair_VisualizeConstantBuffer), m_debugVsConstantBuffer.writeRef());
}

Result Dx11ApiInstance::_createComputeShaderConstantBuffers(ID3D11Device* device)
{
	return Dx11Util::createConstantBuffer(device, sizeof(NvHair_SimulateConstantBuffer), m_csConstantBuffer.writeRef());
}

Result Dx11ApiInstance::_createComputeStatsBuffers(ID3D11Device* device, const Instance& inst)
{
	const Asset* asset = inst.m_asset;

	// CPU buffers for stats 
	{
		m_stats.setSize(asset->m_numFaces);
		const gfsdk_float4 initialValue = gfsdk_makeFloat4(1, 0, 0, 0);
		for (Int i = 0; i < asset->m_numFaces; ++i)
			m_stats[i] = initialValue;
	}

	// constant buffer for stats computation
	NV_RETURN_ON_FAIL(Dx11Util::createConstantBuffer(device, sizeof(NvHair_StatsPerFrameConstantBuffer), m_statsConstantBuffer.writeRef()));
	// create GPU vertex buffer for sampled density buffer
	NV_RETURN_ON_FAIL(Dx11Util::createVertexBuffer(device, sizeof(gfsdk_float4), asset->m_numFaces, m_stats, m_statsBuffer.writeRef(), m_statsUav.writeRef()));
	// create GPU buffer to stage output from compute 
	NV_RETURN_ON_FAIL(Dx11Util::createStagingBuffer(device, sizeof(gfsdk_float4), asset->m_numFaces, m_stats, D3D11_CPU_ACCESS_READ, m_statsStagingBuffer.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createShadingConstantBuffers(ID3D11Device* device)
{
	// create constant buffer for tessellation shader (HS/DS/GS)
	NV_RETURN_ON_FAIL(Dx11Util::createConstantBuffer(device, sizeof( NvHair_TessellationConstantBuffer ), m_hairTessellationConstantBuffer.writeRef() ));
	// create constant buffer for hair rendering pixel shader
	NV_RETURN_ON_FAIL(Dx11Util::createConstantBuffer(device, sizeof( NvHair_ConstantBuffer ), m_hairPixelShaderConstantBuffer.writeRef() ));
	// create constant buffer for hair rendering pixel shader
	NV_RETURN_ON_FAIL(Dx11Util::createConstantBuffer(device, sizeof( NvHair_SplineConstantBuffer ), m_hairSplineConstantBuffer.writeRef() ));
	return NV_OK;
}

Result Dx11ApiInstance::_createMasterStrandVertexBuffers(ID3D11Device* device, const Instance& inst)
{
	// create vb buffer for initial (and previous) positions of the master strands
	const Asset* asset = inst.m_asset;

	UINT numVerts = asset->m_numMasterStrandControlVertices;

	//create structured GPU buffers for current particle positions
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices,
		m_masterStrandBuffer.writeRef(), m_masterStrandSrv.writeRef(), m_masterStrandUav.writeRef()));

	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices,
		m_masterStrandInterpBuffer.writeRef(), m_masterStrandInterpSrv.writeRef(), m_masterStrandInterpUav.writeRef()));

	//create structured GPU buffers for previous particle positions
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices,
		m_masterStrandPrevBuffer.writeRef(), m_masterStrandPrevSrv.writeRef(), m_masterStrandPrevUav.writeRef()));

	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices,
		m_masterStrandInterpolationDeltaBuffer.writeRef(), m_masterStrandInterpolationDeltaSrv.writeRef(), m_masterStrandInterpolationDeltaUav.writeRef()));
	
	//create structured GPU buffers for skinned particle positions
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numVerts, asset->m_masterStrandControlVertices,
		m_skinnedMasterStrandBuffer.writeRef(), m_skinnedMasterStrandSrv.writeRef(), m_skinnedMasterStrandUav.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createMasterStrandTangentBuffers(ID3D11Device* device, const Instance& inst)
{
	// create vb buffer for initial (and previous) positions of the master strands
	const Asset* asset = inst.m_asset;
	//create structured GPU buffers for tangents
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), UINT(asset->m_masterStrandTangents.getSize()), asset->m_masterStrandTangents,
			m_masterStrandTangentBuffer.writeRef(), m_masterStrandTangentSrv.writeRef(), m_masterStrandTangentUav.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createPinConstraintBuffers(ID3D11Device* device, const Instance& inst, const AssetDescriptor& hairDesc)
{
	const Asset* asset = inst.m_asset;
	Int numPins = asset->getDesc().m_numPins;
	if (numPins <= 0)
		return NV_OK;

	asset->calcPins(m_pins);

	// create scratch buffer
	const int scratchSize = numPins * NV_HAIR_SCRATCH_SIZE_PER_PIN;
	
	//create structured GPU buffers for global scratch mem
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(NvHair_PinScratchData), scratchSize, NV_NULL,
		m_pinScratchBuffer.writeRef(), m_pinScratchSrv.writeRef(), m_pinScratchUav.writeRef()));
	// create duplicate GPU buffer to stage output from compute 
	NV_RETURN_ON_FAIL(Dx11Util::createStagingBuffer(device, sizeof(NvHair_PinScratchData), scratchSize, 
		NV_NULL, D3D11_CPU_ACCESS_READ, m_pinScratchStagingBuffer.writeRef()));

	//create structured GPU buffers for global scratch mem
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(NvHair_Pin), hairDesc.m_numPins, m_pins,
		m_pinsBuffer.writeRef(), m_pinsSrv.writeRef(), m_pinsUav.writeRef()));
	// create duplicate GPU buffer to stage output from compute 
	NV_RETURN_ON_FAIL(Dx11Util::createStagingBuffer(device, sizeof(NvHair_Pin), hairDesc.m_numPins, NV_NULL,
		D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, m_pinsStaging.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createGrowthMeshBuffers(ID3D11Device* device, const Instance& inst)
{
	const Asset* asset = inst.m_asset;
	
	// create GPU vertex buffer for growth mesh
	//NV_RETURN_ON_FAIL(Dx11Util::createVertexBuffer(device, sizeof(gfsdk_float4), UINT(asset->m_growthMeshVertices.getSize()), asset->m_growthMeshVertices,
	//		m_growthMeshVertexBuffer.writeRef(), m_growthMeshVertexUav.writeRef()));

	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), UINT(asset->m_growthMeshVertices.getSize()), asset->m_growthMeshVertices,
			m_growthMeshVertexBuffer.writeRef(), m_growthMeshVertexSrv.writeRef(), m_growthMeshVertexUav.writeRef()));

	return NV_OK;
}

Result Dx11ApiInstance::_createStreamOutBuffers(ID3D11Device* device, const Instance& inst)
{
	// Function to create output buffers for splining stream-out
	UINT numElem = inst.getMaxTessellatedPoints();
	// create the splined positions buffers and SRV
	NV_RETURN_ON_FAIL( Dx11Util::createStreamOutBuffer(device, sizeof(gfsdk_float4), numElem, NV_NULL, m_tessellatedMasterStrandBuffer.writeRef(), m_tessellatedMasterStrandSrv.writeRef()));
	// create the splined previous position buffers and SRV
	NV_RETURN_ON_FAIL( Dx11Util::createStreamOutBuffer(device, sizeof(gfsdk_float4), numElem, NV_NULL, m_tessellatedMasterStrandPrevBuffer.writeRef(), m_tessellatedMasterStrandPrevSrv.writeRef()));
	// create the splined tangents buffers and SRV
	NV_RETURN_ON_FAIL( Dx11Util::createStreamOutBuffer(device, sizeof(gfsdk_float4), numElem, NV_NULL, m_tessellatedMasterStrandTangentsBuffer.writeRef(), m_tessellatedMasterStrandTangentsSrv.writeRef()));
	// create the splined normals buffers and SRV
	NV_RETURN_ON_FAIL( Dx11Util::createStreamOutBuffer(device, sizeof(gfsdk_float4), numElem, NV_NULL, m_tessellatedMasterStrandNormalsBuffer.writeRef(), m_tessellatedMasterStrandNormalsSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createMasterStrandNormals(ID3D11Device* device, const Instance& inst)
{
	const Asset* asset = inst.m_asset;

	UINT numControlVertices = asset->m_numMasterStrandControlVertices;
	NvCo::PodBuffer<gfsdk_float4> normalsBuf(numControlVertices);
	gfsdk_float4* normals = normalsBuf;
	for (UINT i = 0; i < numControlVertices; i++)
	{
		normals[i] = gfsdk_makeFloat4(0, 0, 1, 0);
	}
	// create GPU buffer for simulated normals
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), numControlVertices, normals,
		m_masterStrandNormalBuffer.writeRef(), m_masterStrandNormalSrv.writeRef(), m_masterStrandNormalUav.writeRef()));

	return NV_OK;
}

Result Dx11ApiInstance::_createMasterStrandLuminances(ID3D11Device* device, const Instance& inst)
{
	const Asset* asset = inst.m_asset;
	// Set the default luminosity
	UINT numControlVertices = asset->m_numMasterStrandControlVertices;
	NvCo::PodBuffer<Float32> luminancesBuf(numControlVertices);
	Float32* luminances = luminancesBuf;
	for (UINT i = 0; i < numControlVertices; i++)
	{
		// Negative numbers just display as the default color
		luminances[i] = -1.0f; 
	}
	// create GPU buffer for simulated luminances
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(Float32), numControlVertices, luminances,
		m_masterStrandLuminanceBuffer.writeRef(), m_masterStrandLuminanceSrv.writeRef(), m_masterStrandLuminanceUav.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createMasterStrandFrames(ID3D11Device* device, const Instance& inst)
{
	NvCo::PodBuffer<gfsdk_float4> frames, localPosPrev, localPosNext;
	inst.m_asset->calcFrameBuffers(frames, localPosPrev, localPosNext);
	
	// create GPU buffer for current frames
	NV_RETURN_ON_FAIL(Dx11Util::createStructuredBuffer(device, sizeof(gfsdk_float4), UINT(frames.getSize()), frames, 
		m_masterFramesBuffer.writeRef(), m_masterFramesSrv.writeRef(), m_masterFramesUav.writeRef()));
	return NV_OK;
}

Result Dx11ApiInstance::_createDebugTessellatedIndexBuffer(ID3D11Device* device, const Instance& inst)
{
	// create index buffer for tessellated strand
	int numTesselatedPoints = inst.getNumTessellatedPoints();
	int strandPointCount = inst.getStrandPointCount();
	{
		NvCo::PodBuffer<UInt32> indices(numTesselatedPoints * 2);

		Int cnt = 0;
		for (Int i = 0; i < m_numMasterStrands; i++)
		{
			Int start = i * strandPointCount;
			Int end = (i + 1) * strandPointCount - 1;
			for (Int j = start; j < end ; j++)
			{
				indices[cnt * 2] = UInt32(j);
				indices[cnt * 2 + 1] = UInt32(j + 1);
				cnt++;
			}
		}
		// create GPU buffer
		NV_RETURN_ON_FAIL( Dx11Util::createIndexBuffer(device, cnt * 2, indices, m_debugTessellatedIndexBuffer.writeRef()));
	}
	return NV_OK;
}

ID3D11Buffer* Dx11ApiInstance::nextTessellatedMasterStrandBuffer()
{
	ID3D11Buffer* positionSoBuffer = m_tessellatedMasterStrandBuffer;
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	int dbStatus = m_tessellatedMasterStrandDoubleBufferStatus;
	positionSoBuffer = (dbStatus & 1) ? m_tessellatedMasterStrandBuffer : m_tessellatedMasterStrandPrevBuffer;
	// 0->1, 1->2, 2->3, 3->2
	dbStatus = (dbStatus + 1);
	dbStatus -= (dbStatus & 4) >> 1;
	m_tessellatedMasterStrandDoubleBufferStatus = dbStatus;
#endif
	return positionSoBuffer;
}

ID3D11ShaderResourceView* Dx11ApiInstance::getPreviousTessellatedMasterStrandSrv() const
{
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	switch (m_tessellatedMasterStrandDoubleBufferStatus)
	{
		default:
		case 0:		return m_tessellatedMasterStrandPrevSrv;
		// TODO JS: This looks suspicious... wouldn't you expect it to be m_tessellatedMasterStrandSrv?
		case 1:		return m_tessellatedMasterStrandPrevSrv;
		case 2:		return m_tessellatedMasterStrandPrevSrv;
		case 3:		return m_tessellatedMasterStrandSrv;
	}
#else 
	return m_tessellatedMasterStrandPrevSrv;
#endif
}

ID3D11ShaderResourceView* Dx11ApiInstance::getCurrentTessellatedMasterStrandSrv() const
{
#if NV_HAIR_USE_TESSELLATION_DOUBLE_BUFFERING
	switch (m_tessellatedMasterStrandDoubleBufferStatus)
	{
		default:
		case 0:		return m_tessellatedMasterStrandSrv;
		case 1:		return m_tessellatedMasterStrandPrevSrv;
		case 2:		return m_tessellatedMasterStrandSrv;
		case 3:		return m_tessellatedMasterStrandPrevSrv;
	}
#else 
	return m_tessellatedMasterStrandSrv;
#endif
}

Result Dx11ApiInstance::_prepareTesselateConstantBuffer(ID3D11DeviceContext* context, Instance* inst, Float simulationInterp)
{
	const Asset* asset = inst->m_asset;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map(m_hairSplineConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	NvHair_SplineConstantBuffer* constantBuffer = (NvHair_SplineConstantBuffer*)mappedResource.pData;

	// shared constant values regardless of material
	constantBuffer->modelToWorld = inst->m_modelToWorld;
	constantBuffer->numVertsPerSegments = inst->m_numVertsPerSegment;
	constantBuffer->numMasterSegments = asset->m_numMasterSegmentsPerHair;
	constantBuffer->strandPointCounts = inst->getStrandPointCount();
	constantBuffer->numTessellatedPoints = inst->getNumTessellatedPoints();
	constantBuffer->simulationInterp = simulationInterp;

	context->Unmap(m_hairSplineConstantBuffer, 0);
	context->GSSetConstantBuffers(0, 1, m_hairSplineConstantBuffer.readRef());
	return NV_OK;
}

Result Dx11ApiInstance::preRender(Float simulationInterp)
{
	// Function to Compute tessellated master strands with spline interpolation using geometry shaders
	Dx11ApiGlobal* glob = m_apiGlobal;
	SliSystem* sliSystem = glob->getSliSystem();

	ID3D11DeviceContext* context = glob->getContext();

	//------------------------NUT-----------------------------------------
	// check if buffers and resources are ready
	if (simulationInterp < 1.0f)
	{
		// Need to interpolate 
		NV_RETURN_ON_FAIL(Dx11InstanceRender::prepareSimulationConstantBuffer(this, 0.0f, simulationInterp, &m_instance->m_modelToWorld)); //?? HACK
		if (sliSystem)
		{
			startEarlyPushSimulationBuffers(sliSystem);
		}
		NV_RETURN_ON_FAIL(Dx11InstanceRender::dispatchPrepareInterpolate(this));
	}
	//------------------------------------------------------------------------

	if (sliSystem)
	{
		setHintTessellationBuffers(sliSystem);
	}

	//Tessellate master strands with spline interpolation
	NvHair::Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_SPLINE], context);

	ID3D11ShaderResourceView* const srvs[] =
	{
		m_masterStrandSrv,
		m_masterStrandInterpSrv, 
		m_masterStrandInterpolationDeltaSrv,
		m_masterStrandNormalSrv
	};
	context->GSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	// change double buffering status
	// double buffering between current and prev buffer
	ID3D11Buffer* positionSoBuffer = nextTessellatedMasterStrandBuffer();
	
	// set stream out buffers
	ID3D11Buffer* const soBuffers[] =
	{
		positionSoBuffer,
		m_tessellatedMasterStrandTangentsBuffer,
		m_tessellatedMasterStrandNormalsBuffer,
	};
	UINT offsets[] = { 0, 0, 0, 0 };
	context->SOSetTargets(3, soBuffers, offsets);

	NV_RETURN_ON_FAIL(_prepareTesselateConstantBuffer(context, m_instance, simulationInterp));

	// draw
	int numTesselatedPoints = m_instance->getNumTessellatedPoints();

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(numTesselatedPoints, 0);

	// cleanup	
	{
		ID3D11ShaderResourceView* const nullSrvs[NV_COUNT_OF(srvs)] = { NV_NULL };
		// TODO: Not clear this needs to be done, here as not set here?
		context->VSSetShaderResources(0, NV_COUNT_OF(nullSrvs), nullSrvs);
		context->GSSetShaderResources(0, NV_COUNT_OF(nullSrvs), nullSrvs);

		ID3D11Buffer* const nullSoBuffers[NV_COUNT_OF(soBuffers)] = { NV_NULL };
		context->SOSetTargets(NV_COUNT_OF(nullSoBuffers), nullSoBuffers, NV_NULL);
	}
	return NV_OK;
}

Result Dx11ApiInstance::updatePinConstraintBuffer()
{
	if (m_pinsStaging == NULL)
		return NV_OK;

	ID3D11DeviceContext* context = m_apiGlobal->getContext();
	const Asset* asset = m_instance->m_asset;
	const AssetDescriptor& hairAssetDesc = asset->getDesc();

	context->CopyResource(m_pinsStaging, m_pinsBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map(m_pinsStaging, 0, D3D11_MAP_WRITE, 0, &mappedResource));

	NvHair_Pin* pinBuffer = (NvHair_Pin*)mappedResource.pData;

	// copy pin constraint data
	for (UInt i = 0; i < hairAssetDesc.m_numPins; i++)
	{
		const NvHair::Pin& pinSrc = hairAssetDesc.m_pins[i];
		NvHair_Pin& pinTarget = pinBuffer[i];

		pinTarget.stiffness = pinSrc.m_pinStiffness;
		pinTarget.influenceFallOff = pinSrc.m_influenceFallOff;
		pinTarget.useDynamicPin = pinSrc.m_useDynamicPin;
		pinTarget.doLra = pinSrc.m_doLra;
		pinTarget.stiffPin = true; // pinSrc.m_useStiffnessPin;  // hardcode it as true for GWDCC-302.
		pinTarget.selected = pinSrc.m_selected;

		pinTarget.influenceFallOffCurve = pinSrc.m_influenceFallOffCurve;
	}

	context->Unmap(m_pinsStaging, 0);
	context->CopyResource(m_pinsBuffer, m_pinsStaging);
	return NV_OK;
}

Result Dx11ApiInstance::stepSimulation(float timeStep, const gfsdk_float4x4* worldReference)
{
	return Dx11InstanceSimulate::stepSimulation(this, timeStep, worldReference);
}

Result Dx11ApiInstance::renderHairShading(const RenderViewInfo& frameViewInfo, ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(other)
	return Dx11InstanceRender::renderHairShading(this, frameViewInfo, shaderSettings);
}

Void Dx11ApiInstance::calcPixelConstantBuffer(const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut)
{
	Dx11InstanceRender::calcPixelConstantBuffer(this, frameViewInfo, constantBufferOut);
}

Result Dx11ApiInstance::_computeStats()
{
	Dx11ApiGlobal* glob = m_apiGlobal;

	UINT initCounts = 0;

	ID3D11DeviceContext* context = glob->getContext();
	ID3D11ComputeShader* cs = glob->m_computeStats;
	if (!cs)
		return NV_FAIL;

	const Asset* asset = m_instance->m_asset; 
	const Dx11ApiAsset* apiAsset = m_apiAsset;

	const InstanceDescriptor& params = m_instance->getDefaultMaterial();
	
	//	constants for compute shader
	const int cbpsPerFrameBind = 0;

	context->CSSetShader(cs, NV_NULL, 0);

	//	fill in the constant buffer
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// set CB_CS_PER_FRAME constant data
		NV_RETURN_ON_FAIL(context->Map(m_statsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

		NvHair_StatsPerFrameConstantBuffer* constantBuffer = (NvHair_StatsPerFrameConstantBuffer*)mappedResource.pData;

		constantBuffer->numFaces = asset->m_numFaces;
		constantBuffer->density = params.m_density;
		constantBuffer->usePixelDensity = params.m_usePixelDensity;

		context->Unmap(m_statsConstantBuffer, 0);
		context->CSSetConstantBuffers(cbpsPerFrameBind, 1, m_statsConstantBuffer.readRef());
	}

	ID3D11ShaderResourceView* densitySrv = getTexture(TextureType::DENSITY);

	// create SRV
	ID3D11ShaderResourceView* const srvs[] =
	{
		apiAsset->m_faceTexCoordsSrv,
		densitySrv,
		glob->m_scalarNoiseSrv,
		glob->m_strandCoordinatesSrv
	};
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the unordered access views - this is what the CS shader will read from and write to
	//we bind two buffers, one containing the particle positions, and the other containing particle positions from the last frame
	ID3D11UnorderedAccessView* uavs[] = { m_statsUav };
	context->CSSetUnorderedAccessViews(0, NV_COUNT_OF(uavs), uavs, &initCounts);

	//set the sampler
	ID3D11SamplerState* const states[] = { glob->m_linearSampler };
	context->CSSetSamplers(0, NV_COUNT_OF(states), states);

	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch(asset->m_numFaces, 1, 1);

	// Unbind resources for CS
	ID3D11UnorderedAccessView* const nullUavs[NV_COUNT_OF(uavs)] = { NV_NULL };
	context->CSSetUnorderedAccessViews(0, NV_COUNT_OF(nullUavs), nullUavs, &initCounts);
	ID3D11ShaderResourceView* const nullSrvs[NV_COUNT_OF(srvs)] = { NV_NULL };
	context->CSSetShaderResources(0, NV_COUNT_OF(nullSrvs), nullSrvs);
	return NV_OK;
}

Result Dx11ApiInstance::computeStats(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Stats& statsOut)
{
	NV_UNUSED(asyncRepeat)
	if (asyncInOut && *asyncInOut)
	{
		// This us blocking so asyncInOut must hold NV_NULL if set
		return NV_FAIL;
	}

	NV_RETURN_ON_FAIL(_requireDebugResources());

	const Asset* asset = m_instance->m_asset;
	const InstanceDescriptor& params = m_instance->getDefaultMaterial();

	float densitySum = 0.0f;
	if (getTexture(TextureType::DENSITY))
	{
		NV_RETURN_ON_FAIL(_computeStats());

		ID3D11DeviceContext* context = m_apiGlobal->getContext();

		// copy compute results back to CPU
		context->CopyResource(m_statsStagingBuffer, m_statsBuffer);

		// map staging buffer for copy
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		NV_RETURN_ON_FAIL(context->Map(m_statsStagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource));
		gfsdk_float4* mappedStats = (gfsdk_float4*)mappedResource.pData;
		memcpy(m_stats, mappedStats, sizeof(gfsdk_float4) * asset->m_numFaces);
		context->Unmap(m_statsStagingBuffer, 0);
		for (Int i = 0; i < asset->m_numFaces; i++)
			densitySum += m_stats[i].x;
	}
	else
	{
		densitySum = params.m_density * float(asset->m_numFaces);
	}
	m_instance->computeStats(densitySum, statsOut);
	return NV_OK;
}

Result Dx11ApiInstance::debugDraw(EDebugDraw drawType, const ViewInfo& viewInfo)
{
	NV_RETURN_ON_FAIL(_requireDebugResources());

	switch (drawType)
	{ 
		case DebugDraw::GUIDE_HAIRS:					return Dx11DebugRender::drawGuideHairs(this, viewInfo);
		case DebugDraw::SKINNED_GUIDE_HAIRS:			return Dx11DebugRender::drawSkinnedGuideHairs(this, viewInfo);
		case DebugDraw::FRAMES:							return Dx11DebugRender::drawFrames(this, viewInfo);
		case DebugDraw::NORMALS:						return Dx11DebugRender::drawNormals(this, viewInfo);
		case DebugDraw::LOCAL_POS:						return Dx11DebugRender::drawLocalPos(this, viewInfo);
		case DebugDraw::HAIR_INTERACTION:				return Dx11DebugRender::drawHairInteraction(this, viewInfo);
		case DebugDraw::GUIDE_HAIR_CONTROL_VERTICES:	return Dx11DebugRender::drawGuideHairControlVertices(this, viewInfo);
		case DebugDraw::GROWTH_MESH:					return Dx11DebugRender::drawGrowthMesh(this, viewInfo);
		case DebugDraw::PIN_CONSTRAINTS:				return Dx11DebugRender::drawPinConstraints(this, viewInfo);
		default: return NV_FAIL;
	}
}

Result Dx11ApiInstance::getPinMatrix(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int pinId, gfsdk_float4x4& matrixOut)
{
	return getPinMatrices(asyncInOut, asyncRepeat, pinId, 1, &matrixOut);
}

Result Dx11ApiInstance::getPinMatrices(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int startIndex, Int size, gfsdk_float4x4* matricesOut)
{
	NV_UNUSED(asyncRepeat)

	if (asyncInOut && *asyncInOut)
	{
		// This us blocking so asyncInOut must hold NV_NULL if set
		return NV_FAIL;
	}

	const Int numPins = Int(m_pins.getSize());
	if (startIndex < 0 || numPins < 0 || startIndex + size > numPins)
	{
		return NV_FAIL;
	}
	if (size == 0)
	{
		return NV_OK;
	}

	ID3D11DeviceContext* context = m_apiGlobal->getContext();
	context->CopyResource(m_pinsStaging, m_pinsBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map(m_pinsStaging, 0, D3D11_MAP_READ, 0, &mappedResource));
	NvHair_Pin* pins = (NvHair_Pin*)mappedResource.pData;
	for (Int i = 0; i < size; i++)
	{
		const gfsdk_float4x4 pinMatrix = pins[i + startIndex].currentPinMatrix;
		matricesOut[i] = pinMatrix * m_instance->m_modelToWorld;
	}
	context->Unmap(m_pinsStaging, 0);
	return NV_OK;
}

Result Dx11ApiInstance::onNumVertsPerSegmentChanged()
{
	if (m_debugTessellatedIndexBuffer)
	{
		m_debugTessellatedIndexBuffer.setNull();
		NV_RETURN_ON_FAIL(_createDebugTessellatedIndexBuffer(m_apiGlobal->getDevice(), *m_instance));
	}
	return NV_OK;
}

} // namespace HairWorks 
} // namespace nvidia 