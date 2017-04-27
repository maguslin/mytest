/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11InstanceSimulate.h"

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11Util.h>
#include <Nv/HairWorks/Internal/Dx11/NvHairDx11ApiInstance.h>

#include <Nv/HairWorks/Internal/Dx/NvHairDxSimulateUtil.h>

#include "NvHairDx11ApiGlobal.h"

namespace nvidia {
namespace HairWorks { 

static void* const g_nulls[128] = { NV_NULL };

/* static */Void Dx11InstanceSimulate::_cleanComputeResources(ID3D11DeviceContext* context, int numUavs, int numSrvs)
{
	// Unbind resources for CS
	const int maxNull = int(NV_COUNT_OF(g_nulls));
	UINT initCounts = 0;
	if (numSrvs > 0)
	{
		NV_CORE_ASSERT(numSrvs <= maxNull);
		numSrvs = numSrvs <= maxNull ? numSrvs : maxNull;
		context->CSSetShaderResources( 0, numSrvs, (ID3D11ShaderResourceView*const*)g_nulls);
	}
	if (numUavs > 0)
	{
		NV_CORE_ASSERT(numUavs <= maxNull);
		numUavs = numUavs <= maxNull ? numUavs : maxNull;
		context->CSSetUnorderedAccessViews( 0, numUavs, (ID3D11UnorderedAccessView*const*)g_nulls, &initCounts);
	}
}

/* static */Result Dx11InstanceSimulate::_prepareSimulationConstantBuffer(Dx11ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* mat)
{
	NV_UNUSED(mat);
	
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();

	Instance* inst = apiInst->m_instance;
	
	// map constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map( apiInst->m_csConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource ));
	DxSimulateUtil::calcConstantBuffer(inst, timeStep, 0.0f, mat, (NvHair_SimulateConstantBuffer*)mappedResource.pData);
	context->Unmap( apiInst->m_csConstantBuffer, 0 );
	context->CSSetConstantBuffers( 0, 1, apiInst->m_csConstantBuffer.readRef() );
	return NV_OK;
}

/* static */Result Dx11InstanceSimulate::_dispatchSimulate(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	Instance* inst = apiInst->m_instance;

	ID3D11DeviceContext* context = glob->getContext();

	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	ID3D11ComputeShader* hairSimulateShader = glob->m_hairSimulate;

	if (!hairSimulateShader)
		return NV_FAIL;

	context->CSSetShader( hairSimulateShader, NV_NULL, 0 );

	ID3D11ShaderResourceView*const srvs[] = 
	{ 
		apiAsset->m_masterStrandOffsetSrv,
		apiAsset->m_masterStrandLinearSpringLengthsSrv, 
		apiAsset->m_masterStrandBendSpringLengthsSrv, 
		apiAsset->m_masterStrandRootDistancesSrv,
		apiAsset->m_masterStrandNormalizedLengthToRootSrv,

		apiAsset->m_origMasterStrandSrv,
		apiAsset->m_origMasterFramesSrv,

		apiAsset->m_prevMasterLocalPosSrv,
		apiAsset->m_nextMasterLocalPosSrv,

		apiAsset->m_boneIndicesSrv,
		apiAsset->m_boneWeightsSrv,

		apiAsset->m_hairTexCoordsSrv,

		apiAsset->m_growthMeshRestNormalSrv,
		apiAsset->m_growthMeshRestTangentSrv,

		glob->m_vectorNoiseSrv,

		apiInst->getTexture(TextureType::STIFFNESS),
		apiInst->getTexture(TextureType::ROOT_STIFFNESS),
		apiInst->getTexture(TextureType::WEIGHTS),

		//apiInst->m_masterStrandRadiusSrv
	};
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the sampler
	ID3D11SamplerState* const states[] = { glob->m_linearSampler };
	context->CSSetSamplers( 0, NV_COUNT_OF(states), states );

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] = 
	{ 
		apiInst->m_masterStrandUav, 
		apiInst->m_masterStrandPrevUav,
		apiInst->m_masterStrandInterpUav,
		apiInst->m_masterFramesUav,

		apiInst->m_growthMeshVertexUav,
		apiInst->m_skinnedMasterStrandUav,
		apiInst->m_masterStrandNormalUav,
		apiInst->m_masterStrandTangentUav,
	};

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews( 0, NV_COUNT_OF(uavs), uavs, &initCounts );
	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch( asset->m_numMasterStrands, 1, 1 );
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), NV_COUNT_OF(srvs));
	return NV_OK;
}

/*static*/ Result Dx11InstanceSimulate::_dispatchInteraction(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiInst->m_instance->m_asset;
	
	ID3D11ComputeShader* hairSimulateShader = glob->m_hairSimulateInteraction;
	if (!hairSimulateShader)
		return NV_FAIL;

	context->CSSetShader( hairSimulateShader, NV_NULL, 0 );

	// create SRV
	ID3D11ShaderResourceView*const srvs[] = 
	{ 
		apiAsset->m_masterStrandOffsetSrv,
		apiAsset->m_masterStrandInteractionIndicesSrv,
		apiAsset->m_masterStrandInteractionOffsetSrv,
		apiAsset->m_masterStrandInteractionLengthSrv,
		//apiInst->m_masterStrandRadiusSrv
	};
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the sampler
	ID3D11SamplerState* const states[] = { glob->m_linearSampler };
	context->CSSetSamplers(0, NV_COUNT_OF(states), states);

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] = {  apiInst->m_masterStrandUav };

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews( 0, NV_COUNT_OF(uavs), uavs, &initCounts );
	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch( asset->m_numMasterStrands, 1, 1 );
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), NV_COUNT_OF(srvs));
	return NV_OK;
}

/*static*/ Result Dx11InstanceSimulate::_dispatchSimulatePin(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;

	ID3D11DeviceContext* context = glob->getContext();
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;

	ID3D11ComputeShader* computeShader = glob->m_hairSimulatePinPass;
	if (!computeShader)
		return NV_FAIL;

	context->CSSetShader( computeShader, NV_NULL, 0 );

	// create SRV
	ID3D11ShaderResourceView* const srvs[] = { apiAsset->m_origMasterStrandSrv };
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] = 
	{ 
		apiInst->m_masterStrandUav, 
		apiInst->m_pinsUav,
		apiInst->m_masterStrandLuminanceUav,
	};

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews( 0, NV_COUNT_OF(uavs), uavs, &initCounts );

	int numCvs = asset->m_numMasterStrandControlVertices;
	int numBlocks = numCvs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch( numBlocks, 1, 1 );
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), NV_COUNT_OF(srvs));
	return NV_OK;
}

/* static */Result Dx11InstanceSimulate::_dispatchSimulatePinCom(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	ID3D11DeviceContext* context = glob->getContext();

	ID3D11ComputeShader* computeShader = glob->m_hairSimulatePinComPass;
	if (!computeShader)
		return NV_FAIL;

	context->CSSetShader( computeShader, NV_NULL, 0 );

	ID3D11ShaderResourceView* const srvs[] = { apiAsset->m_origMasterStrandSrv };
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] = 
	{ 
		apiInst->m_masterStrandUav, 
		apiInst->m_pinScratchUav,
		apiInst->m_pinsUav,
		apiInst->m_masterStrandTangentUav,
	};

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews( 0, NV_COUNT_OF(uavs), uavs, &initCounts );

	int numCvs = asset->m_numMasterStrandControlVertices;
	int numBlocks = numCvs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch( numBlocks, 1, 1 );
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), NV_COUNT_OF(srvs));
	return NV_OK;
}

/* static */Result Dx11InstanceSimulate::_dispatchSimulatePinComGather(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	const Asset* asset = apiInst->m_apiAsset->m_asset;
	ID3D11DeviceContext* context = glob->getContext();

	ID3D11ComputeShader* computeShader = glob->m_hairSimulatePinComGatherPass;
	if (!computeShader)
		return NV_FAIL;
	context->CSSetShader( computeShader, NV_NULL, 0 );

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] = 
	{ 
		apiInst->m_pinScratchUav,
		apiInst->m_pinsUav,
	};

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews( 0, NV_COUNT_OF(uavs), uavs, &initCounts );

	int numCVs = asset->m_numMasterStrandControlVertices;
	int numBlocks = numCVs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;
	int numGrids = numBlocks / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch( numGrids, 1, 1 );
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), 0);
	return NV_OK;
}

/* static */Result Dx11InstanceSimulate::stepSimulation(Dx11ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* worldReference)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	// check if buffers and resources are ready
	SliSystem* sliSystem = glob->getSliSystem();

	Instance* inst = apiInst->m_instance;
	const InstanceDescriptor& params = inst->getDefaultMaterial();

	NV_RETURN_ON_FAIL(_prepareSimulationConstantBuffer(apiInst, timeStep, worldReference));
	if (sliSystem)
	{
		apiInst->startEarlyPushSimulationBuffers(sliSystem);
	}
	NV_RETURN_ON_FAIL(_dispatchSimulate(apiInst));

	if (params.m_interactionStiffness > 0)
	{
		NV_RETURN_ON_FAIL(_dispatchInteraction(apiInst));
	}

	if ((apiInst->m_pins.getSize() > 0) && (params.m_pinStiffness > 0) )
	{
		NV_RETURN_ON_FAIL(_dispatchSimulatePinCom(apiInst));
		NV_RETURN_ON_FAIL(_dispatchSimulatePinComGather(apiInst));
		NV_RETURN_ON_FAIL(_dispatchSimulatePin(apiInst));
	}

	if (sliSystem)
	{
		apiInst->finishEarlyPushSimulationBuffers(sliSystem);
	}
	// now simulation is setup properly for subsequent uses
	inst->m_isSimulationStarted = true;
	return NV_OK;
}

} // namespace HairWorks 
} // namespace nvidia 


