/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

//#define DEBUG_HAIR_SKIP 20

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12DebugRender.h"

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ApiInstance.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

// constant buffer definition shared with shaders
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>
#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

#include "d3dx12.h"

namespace nvidia { 
namespace HairWorks { 

static Void _setMatrices(const Instance* inst, const ViewInfo& viewInfo, NvHair_VisualizeConstantBuffer& dst)
{
	dst.modelToWorld = inst->m_modelToWorld;
	dst.viewProjection = viewInfo.m_viewProjectionMatrix;
}

static void _setHairMinMax(Dx12ApiInstance* apiInst, NvHair_VisualizeConstantBuffer& dst)
{
	const InstanceDescriptor& params = apiInst->m_instance->getDefaultMaterial();
	dst.hairMin = 0;
	dst.hairMax = apiInst->m_numMasterStrands;
	dst.hairSkip = params.m_visualizeHairSkips;
}

static void _setColor(float r, float g, float b, float a, NvHair_VisualizeConstantBuffer& dst)
{
	dst.color = gfsdk_makeFloat4(r, g, b, a);
}

/* static */Result Dx12DebugRender::drawGuideHairs(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	Instance* inst = apiInst->m_instance;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}

	Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_DEBUG_GUIDE];

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{
			apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
			apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}

	commandList->SetGraphicsRootSignature(pass.m_rootSignature);
	commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());
	
	{
		ID3D12DescriptorHeap* heaps[] = { viewHeap };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
	}
	commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);
	
	commandList->SetPipelineState(pass.m_pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->IASetIndexBuffer(&apiAsset->m_debugMasterIndexBuffer.m_indexBufferView);

	Int lineCount = inst->m_asset->m_totalNumMasterSegments;
	commandList->DrawIndexedInstanced(lineCount * 2, 1, 0, 0, 0);

	return NV_OK;
}

/* static */Result Dx12DebugRender::drawGrowthMesh(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_growthMeshVertexBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 1, 0, 1, dst);
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}

	// draw growth mesh
	Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_BODY_DEBUG];

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			apiInst->m_growthMeshVertexBuffer.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}

	commandList->SetGraphicsRootSignature(pass.m_rootSignature);

	commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());
	
	commandList->SetPipelineState(pass.m_pipelineState);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

	commandList->IASetVertexBuffers(0, 0, NV_NULL);
	commandList->IASetIndexBuffer(&apiAsset->m_growthMeshIndexBuffer.m_indexBufferView);

	commandList->DrawIndexedInstanced(asset->m_numFaces * 3, 1, 0, 0, 0);
	return NV_OK;
}


/* static */Result Dx12DebugRender::drawSkinnedGuideHairs(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_skinnedMasterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 1, 0, 1, dst);
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}
	// draw lines
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_DEBUG_GUIDE];
		
		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap;
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_skinnedMasterStrandBuffer.getSrvCpuHandle(),
				apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}
		
		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);
		
		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		commandList->IASetIndexBuffer(&apiAsset->m_debugMasterIndexBuffer.m_indexBufferView);

		int cvCount = asset->m_numMasterStrandControlVertices;
		commandList->DrawIndexedInstanced(cvCount * 2, 1, 0, 0, 0);
	}

	return NV_OK;
}

/* static */Result Dx12DebugRender::drawFrames(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_masterFramesBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		const float sceneUnit = asset->getUnitInCentimeters();
		const float boneDisplayScale = 1.0f / sceneUnit;

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		dst.scale = boneDisplayScale;
		_setColor(1, 0, 0, 1, dst);
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}

	// draw lines
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_VISUALIZE_FRAME];

		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap;
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
				apiInst->m_masterFramesBuffer.getSrvCpuHandle(),
				apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		commandList->IASetIndexBuffer(&apiAsset->m_debugFrameIndexBuffer.m_indexBufferView);

		int cvCount = asset->m_numMasterStrandControlVertices;
		commandList->DrawIndexedInstanced(cvCount * 6, 1, 0, 0, 0);
	}

	return NV_OK;
}

/* static */Result Dx12DebugRender::drawLocalPos(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_masterFramesBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		float sceneUnit = asset->getUnitInCentimeters();
		float boneDisplayScale = 1.0f / sceneUnit;

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		dst.scale = boneDisplayScale;
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}
	// draw local pos
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_VISUALIZE_LOCAL_POS];

		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap;
		{
			D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
				apiInst->m_masterFramesBuffer.getSrvCpuHandle(),
				apiAsset->m_prevMasterLocalPosBuffer.getSrvCpuHandle(),
				apiAsset->m_nextMasterLocalPosBuffer.getSrvCpuHandle(),
				apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		commandList->IASetIndexBuffer(&apiAsset->m_debugLocalPosIndexBuffer.m_indexBufferView);

		int cvCount = asset->m_numMasterStrandControlVertices;
		commandList->DrawIndexedInstanced(cvCount * 4, 1, 0, 0, 0);
	}
	
	return NV_OK;
}

/* static */Result Dx12DebugRender::drawNormals(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_masterStrandNormalBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		float sceneUnit = asset->getUnitInCentimeters();
		float boneDisplayScale = 1.0f / sceneUnit;

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		dst.scale = boneDisplayScale;
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}
	// draw normals
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_VISUALIZE_NORMAL];
		
		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap;
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
				apiInst->m_masterStrandNormalBuffer.getSrvCpuHandle(),
				apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		commandList->IASetIndexBuffer(&apiAsset->m_debugNormalIndexBuffer.m_indexBufferView);

		int numCvs = asset->m_numMasterStrandControlVertices;
		commandList->DrawIndexedInstanced(numCvs * 2, 1, 0, 0, 0);
	}
	return NV_OK;
}

/* static */Result Dx12DebugRender::drawHairInteraction(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// set constant buffer
	Heap::Cursor cursor;
	{
		const float sceneUnit = asset->getUnitInCentimeters();
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 1, 0, 1, dst);
		dst.hairWidth = 0.5f / sceneUnit;
		dst.aspect = viewInfo.m_aspect;
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}

	// draw interaction lines
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_DEBUG_INTERACTION];
		
		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap; 
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
				apiAsset->m_masterStrandInteractionIndicesBuffer.getSrvCpuHandle(),
				apiAsset->m_masterStrandInteractionOffsetBuffer.getSrvCpuHandle(),
				apiAsset->m_masterStrandInteractionLengthBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		commandList->IASetIndexBuffer(NV_NULL);
		
		commandList->DrawInstanced(asset->m_numMasterStrandControlVertices, 1, 0, 0);
	}
	return NV_OK;
}

/* static */Result Dx12DebugRender::drawGuideHairControlVertices(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	// set constant buffer
	Heap::Cursor cursor;
	{
		NvHair_VisualizeConstantBuffer dst;
		float sceneUnit = asset->getUnitInCentimeters();
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(0.1, 0.1, 1, 1, dst);
		dst.hairWidth = 0.5f / sceneUnit;
		dst.aspect = viewInfo.m_aspect;
		cursor = glob->m_uploadHeap.newConstantBuffer(dst);
	}

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandLuminanceBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}

	// draw cvs
	{
		Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_DEBUG_CV];

		D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
		ID3D12DescriptorHeap* viewHeap;
		{
			D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
			{
				apiInst->m_masterStrandBuffer.getSrvCpuHandle(),
				apiInst->m_masterStrandLuminanceBuffer.getSrvCpuHandle(),
				apiAsset->m_debugVertexToHairBuffer.getSrvCpuHandle(),
			};
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
		}
		
		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());

		{
			ID3D12DescriptorHeap* heaps[] = { viewHeap };
			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		}
		commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		commandList->SetPipelineState(pass.m_pipelineState);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		int cvCount = asset->m_numMasterStrandControlVertices;
		commandList->DrawInstanced(cvCount + 1, 1, 0, 0);
	}

	return NV_OK;
}

#if 0
static void _drawSphere(Dx12ApiGlobal* glob)
{
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		commandList->IASetVertexBuffers(0, 1, &glob->m_sphereVertexBuffer.m_vertexBufferView);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		commandList->DrawInstanced(glob->m_numSphereLines * 2, 1, 0, 0);
	}
}
#endif


/* static */Result Dx12DebugRender::drawPinConstraints(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	const IndexT numPins = apiInst->m_pins.getSize();
	if (numPins <= 0)
	{
		return NV_OK;
	}

	Instance* inst = apiInst->m_instance;
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_pinScratchBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_pinsBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, barriers);
	}
	
	const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;
	// set shader resource
	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{ 
			apiInst->m_pinScratchBuffer.getSrvCpuHandle(),
			apiInst->m_pinsBuffer.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}

	Dx12ShaderPass& pass = glob->m_passes[Dx12ApiGlobal::PASS_VISUALIZE_PIN];
	const float colors[12][3] = 
	{
		{1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{1.0f,1.0f,0.0f},
		{1.0f,0.0f,1.0f},
		{0.0f,1.0f,1.0f},
		{0.5f,0.0f,0.0f},
		{0.0f,0.5f,0.0f},
		{0.0f,0.0f,0.5f},
		{1.0f,0.5f,0.0f},
		{1.0f,0.0f,0.5f},
		{0.0f,1.0f,0.5f}
	};

	commandList->SetGraphicsRootSignature(pass.m_rootSignature);
	{
		ID3D12DescriptorHeap* heaps[] = { viewHeap };
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
	}
	commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);
	commandList->SetPipelineState(pass.m_pipelineState);
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->IASetVertexBuffers(0, 1, &glob->m_sphereVertexBuffer.m_vertexBufferView);

	for (IndexT i = 0; i < numPins; i++)
	{
		// set constant buffer
		{
			NvHair_VisualizeConstantBuffer dst;
			dst.modelToWorld = modelToWorld;
			dst.viewProjection = viewInfo.m_viewProjectionMatrix;

			_setColor(colors[i%12][0], colors[i%12][1], colors[i%12][2], 1, dst);
			dst.pinId = int(i);
			dst.scale = apiInst->m_pins[i].radius;

			Heap::Cursor cursor = glob->m_uploadHeap.newConstantBuffer(dst);
			commandList->SetGraphicsRootConstantBufferView(0, cursor.getGpuHandle());
		}
		
		// Draw the sphere
		commandList->DrawInstanced(glob->m_numSphereLines * 2, 1, 0, 0);
	}
	return NV_OK;
}

#if 0
/* static */Result Dx12DebugRender::drawHairParticles(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	// NUT
	typedef Dx12CircularResourceHeap Heap;

	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = apiAsset->m_asset;
	Instance* inst = apiInst->m_instance;
	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;

	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] = 
		{ 
			apiInst->m_pinScratchSrv,
			apiInst->m_masterStrandSrv,
			apiInst->m_masterStrandRadiusSrv
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
	Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_HAIR_PARTICLE], context);
	const float colors[12][3] = 
	{
		{1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{1.0f,1.0f,0.0f},
		{1.0f,0.0f,1.0f},
		{0.0f,1.0f,1.0f},
		{0.5f,0.0f,0.0f},
		{0.0f,0.5f,0.0f},
		{0.0f,0.0f,0.5f},
		{1.0f,0.5f,0.0f},
		{1.0f,0.0f,0.5f},
		{0.0f,1.0f,0.5f}
	};

	for (Int i = 0; i < apiInst->m_numMasterStrandControlVertices; i++)
	{
		float radius		= 1.0f;
		gfsdk_float4x4 scale, model;

		gfsdk_makeScale(scale, gfsdk_makeFloat3(radius, radius, radius));
		model = scale * modelToWorld;

		// set constant buffer
		{
			NvHair_VisualizeConstantBuffer dst;
			dst.modelToWorld = model;
			dst.viewProjection = viewInfo.m_viewProjectionMatrix;
			//GFSDK_HAIR_SetVisualzeColor(cbuffer, 1, 0, 1, 1);
			_setColor(colors[i%12][0], colors[i%12][1], colors[i%12][2], 1, dst);
			dst.pinId = i;
			NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
		}
		_drawSphere(glob);
	}
	return NV_OK;
}

#endif

} // namespace HairWorks 
} // namespace nvidia 