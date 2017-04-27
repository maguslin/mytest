/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12InstanceSimulate.h"

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ApiInstance.h>

#include <Nv/HairWorks/Internal/Dx/NvHairDxSimulateUtil.h>

#include "d3dx12.h"

#include "NvHairDx12ApiGlobal.h"

namespace nvidia {
namespace HairWorks { 

/* static */Result Dx12InstanceSimulate::stepSimulation(Dx12ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* worldReference)
{
	//Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	
	Instance* inst = apiInst->m_instance;
	const InstanceDescriptor& params = inst->getDefaultMaterial();

	ResourceHeap::Cursor cbCursor;

	NV_RETURN_ON_FAIL(_prepareSimulationConstantBuffer(apiInst, timeStep, worldReference, cbCursor));

	NV_RETURN_ON_FAIL(_dispatchSimulate(apiInst, cbCursor));
	if (params.m_interactionStiffness > 0)
	{
		NV_RETURN_ON_FAIL(_dispatchInteraction(apiInst, cbCursor));
	}

	if ((apiInst->m_pins.getSize() > 0) && (params.m_pinStiffness > 0))
	{		
		NV_RETURN_ON_FAIL(_dispatchSimulatePinCom(apiInst, cbCursor));
		NV_RETURN_ON_FAIL(_dispatchSimulatePinComGather(apiInst, cbCursor));
		NV_RETURN_ON_FAIL(_dispatchSimulatePin(apiInst, cbCursor));
	}

	// now simulation is setup properly for subsequent uses
	inst->m_isSimulationStarted = true;
	return NV_OK;
}

/* static */Result Dx12InstanceSimulate::_prepareSimulationConstantBuffer(Dx12ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* mat, ResourceHeap::Cursor& cursorOut)
{
	NV_UNUSED(mat);
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	
	cursorOut = glob->m_uploadHeap.allocateConstantBuffer(sizeof(NvHair_SimulateConstantBuffer));
	NvHair_SimulateConstantBuffer* constantBuffer = (NvHair_SimulateConstantBuffer*)cursorOut.m_position;
	DxSimulateUtil::calcConstantBuffer(apiInst->m_instance, timeStep, 0.0f, mat, constantBuffer);
	return NV_OK;
}

/* static */Result Dx12InstanceSimulate::_dispatchSimulate(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	Instance* inst = apiInst->m_instance;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandPrevBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandInterpBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterFramesBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_growthMeshVertexBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_skinnedMasterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandNormalBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandTangentBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	ID3D12DescriptorHeap* viewHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2];
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			apiAsset->m_masterStrandOffsetBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandLinearSpringLengthsBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandBendSpringLengthsBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandRootDistancesBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandNormalizedLengthToRootBuffer.getSrvCpuHandle(),

			apiAsset->m_origMasterStrandBuffer.getSrvCpuHandle(),
			apiAsset->m_origMasterFramesBuffer.getSrvCpuHandle(),

			apiAsset->m_prevMasterLocalPosBuffer.getSrvCpuHandle(),
			apiAsset->m_nextMasterLocalPosBuffer.getSrvCpuHandle(),

			apiAsset->m_boneIndicesBuffer.getSrvCpuHandle(),
			apiAsset->m_boneWeightsBuffer.getSrvCpuHandle(),

			apiAsset->m_hairTexCoordsBuffer.getSrvCpuHandle(),

			apiAsset->m_growthMeshRestNormalBuffer.getSrvCpuHandle(),
			apiAsset->m_growthMeshRestTangentBuffer.getSrvCpuHandle(),

			glob->m_vectorNoiseBuffer.getSrvCpuHandle(),

			apiInst->getTextureSrv(TextureType::STIFFNESS),
			apiInst->getTextureSrv(TextureType::ROOT_STIFFNESS),
			apiInst->getTextureSrv(TextureType::WEIGHTS),
			// apiInst->m_masterStrandRadiusBuffer.getSrvCpuHandle()
		};
		NV_COMPILE_TIME_ASSERT(NV_COUNT_OF(srvs) == 18);
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] = 
		{
			apiInst->m_masterStrandBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandPrevBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandInterpBuffer.getUavCpuHandle(),
			apiInst->m_masterFramesBuffer.getUavCpuHandle(),
			apiInst->m_growthMeshVertexBuffer.getUavCpuHandle(),
			apiInst->m_skinnedMasterStrandBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandNormalBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandTangentBuffer.getUavCpuHandle(),
		};
		const DescriptorSet sets[] =
		{
			DescriptorSet(DescriptorSet::TYPE_SRV, srvs),
			DescriptorSet(DescriptorSet::TYPE_UAV, uavs),
		};
		viewHeap = glob->m_viewCache.put(sets, 0, viewHandles);
	}

	ID3D12DescriptorHeap* heaps[] = { viewHeap, glob->m_samplerHeap.getHeap() };

	if (false)
	{		
		Dx12Bundle& bundle = apiInst->m_bundles[Dx12ApiInstance::BUNDLE_SIMULATE];
		ID3D12GraphicsCommandList* bundleList = bundle.getCommandList();
		if (bundle.start(heaps, NV_COUNT_OF(heaps), viewHandles, NV_COUNT_OF(viewHandles)))
		{
			bundleList->SetComputeRootSignature(glob->m_hairSimulate.m_rootSignature);

			bundleList->SetPipelineState(glob->m_hairSimulate.m_pipelineState);

			bundleList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

			bundleList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());

			bundleList->SetComputeRootDescriptorTable(1, viewHandles[0]);
			bundleList->SetComputeRootDescriptorTable(2, viewHandles[1]);
			bundleList->SetComputeRootDescriptorTable(3, glob->m_samplerHeap.getGpuStart());

			bundleList->Dispatch(asset->m_numMasterStrands, 1, 1);

			bundle.end();
		}

		commandList->SetComputeRootSignature(glob->m_hairSimulate.m_rootSignature);
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		commandList->ExecuteBundle(bundleList);
	}
	else
	{
		commandList->SetComputeRootSignature(glob->m_hairSimulate.m_rootSignature);
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		//commandList->SetComputeRootSignature(glob->m_hairSimulate.m_rootSignature);
		//commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetPipelineState(glob->m_hairSimulate.m_pipelineState);
		
		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());

		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
		commandList->SetComputeRootDescriptorTable(3, glob->m_samplerHeap.getGpuStart());

		commandList->Dispatch(asset->m_numMasterStrands, 1, 1);
	}
	
	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.restore(barriers);
		apiInst->m_masterStrandPrevBuffer.restore(barriers);
		apiInst->m_masterStrandInterpBuffer.restore(barriers);
		apiInst->m_masterFramesBuffer.restore(barriers);
		apiInst->m_growthMeshVertexBuffer.restore(barriers);
		apiInst->m_skinnedMasterStrandBuffer.restore(barriers);
		apiInst->m_masterStrandNormalBuffer.restore(barriers);
		apiInst->m_masterStrandTangentBuffer.restore(barriers);
	}

	return NV_OK;
}

/*static*/ Result Dx12InstanceSimulate::_dispatchInteraction(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiInst->m_instance->m_asset;
	
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			apiAsset->m_masterStrandOffsetBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandInteractionIndicesBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandInteractionOffsetBuffer.getSrvCpuHandle(),
			apiAsset->m_masterStrandInteractionLengthBuffer.getSrvCpuHandle(),
			//apiInst->m_masterStrandRadiusBuffer.getSrvCpuHandle(),
		};
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] =
		{
			apiInst->m_masterStrandBuffer.getUavCpuHandle(),
		};
		const DescriptorSet sets[] =
		{
			DescriptorSet(DescriptorSet::TYPE_SRV, srvs),
			DescriptorSet(DescriptorSet::TYPE_UAV, uavs),
		};
		viewHeap = glob->m_viewCache.put(sets, 0, viewHandles);
	}

	ID3D12DescriptorHeap* heaps[] = { viewHeap };
	if (false)
	{
		Dx12Bundle& bundle = apiInst->m_bundles[Dx12ApiInstance::BUNDLE_INTERACTION];
		ID3D12GraphicsCommandList* bundleList = bundle.getCommandList();

		if (bundle.start(heaps, NV_COUNT_OF(heaps), viewHandles, NV_COUNT_OF(viewHandles)))
		{
			bundleList->SetPipelineState(glob->m_hairSimulateInteraction.m_pipelineState);
			bundleList->SetComputeRootSignature(glob->m_hairSimulateInteraction.m_rootSignature);

			bundleList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

			bundleList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());
			bundleList->SetComputeRootDescriptorTable(1, viewHandles[0]);
			bundleList->SetComputeRootDescriptorTable(2, viewHandles[1]);

			bundleList->Dispatch(asset->m_numMasterStrands, 1, 1);
			bundle.end();
		}

		commandList->SetPipelineState(glob->m_hairSimulateInteraction.m_pipelineState);
		commandList->SetComputeRootSignature(glob->m_hairSimulateInteraction.m_rootSignature);

		commandList->ExecuteBundle(bundleList);
	}
	else
	{
		commandList->SetPipelineState(glob->m_hairSimulateInteraction.m_pipelineState);
		commandList->SetComputeRootSignature(glob->m_hairSimulateInteraction.m_rootSignature);

		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());
		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);

		commandList->Dispatch(asset->m_numMasterStrands, 1, 1);
	}

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.restore(barriers);
	}

	return NV_OK;
}

/*static*/ Result Dx12InstanceSimulate::_dispatchSimulatePin(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	
	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_pinsBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandLuminanceBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	Dx12ShaderPass& pass = glob->m_hairSimulatePinPass;

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{
			apiAsset->m_origMasterStrandBuffer.getSrvCpuHandle(),
		};
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] = 
		{
			apiInst->m_masterStrandBuffer.getUavCpuHandle(),
			apiInst->m_pinsBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandLuminanceBuffer.getUavCpuHandle(),
		};
		const DescriptorSet sets[] =
		{
			DescriptorSet(DescriptorSet::TYPE_SRV, srvs),
			DescriptorSet(DescriptorSet::TYPE_UAV, uavs),
		};
		viewHeap = glob->m_viewCache.put(sets, 0, viewHandles);
	}

	int numCvs = asset->m_numMasterStrandControlVertices;
	int numBlocks = numCvs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

	{
		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->SetComputeRootSignature(pass.m_rootSignature);

		ID3D12DescriptorHeap* heaps[] = { viewHeap };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());

		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
		
		// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
		commandList->Dispatch(numBlocks, 1, 1);
	}

	return NV_OK;
}

/* static */Result Dx12InstanceSimulate::_dispatchSimulatePinCom(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;

	Dx12ShaderPass& pass = glob->m_hairSimulatePinComPass;

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_pinScratchBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_pinsBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandTangentBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		//apiInst->m_masterStrandLuminanceBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{
			apiAsset->m_origMasterStrandBuffer.getSrvCpuHandle(),
		};
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] = 
		{
			apiInst->m_masterStrandBuffer.getUavCpuHandle(),
			apiInst->m_pinScratchBuffer.getUavCpuHandle(),
			apiInst->m_pinsBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandTangentBuffer.getUavCpuHandle(),
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

		ID3D12DescriptorHeap* heaps[] = { viewHeap };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());

		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
	
		int numCvs = asset->m_numMasterStrandControlVertices;
		int numBlocks = numCvs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

		// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
		commandList->Dispatch( numBlocks, 1, 1 );
	}
	return NV_OK;
}

/* static */Result Dx12InstanceSimulate::_dispatchSimulatePinComGather(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;

	Dx12ShaderPass& pass = glob->m_hairSimulatePinComGatherPass;

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_pinScratchBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_pinsBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		//set the unordered access views - this is what the CS shader will read from and write to
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] =
		{
			apiInst->m_pinScratchBuffer.getUavCpuHandle(),
			apiInst->m_pinsBuffer.getUavCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_UAV, uavs), false, viewHandles);
	}

	{
		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->SetComputeRootSignature(pass.m_rootSignature);

		ID3D12DescriptorHeap* heaps[] = { viewHeap };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());
		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);

		int numCvs = asset->m_numMasterStrandControlVertices;
		int numBlocks = numCvs / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;
		int numGrids = numBlocks / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;

		// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
		commandList->Dispatch( numGrids, 1, 1 );
	}

	return NV_OK;
}


} // namespace HairWorks 
} // namespace nvidia 


