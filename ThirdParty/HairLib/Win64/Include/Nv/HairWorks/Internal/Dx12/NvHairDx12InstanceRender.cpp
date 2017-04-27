// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright (c) 2013-2015 NVIDIA Corporation. All rights reserved.
//
// NVIDIA Corporation and its licensors retain all intellectual property and proprietary
// rights in and to this software and related documentation and any modifications thereto.
// Any use, reproduction, disclosure or distribution of this software and related
// documentation without an express license agreement from NVIDIA Corporation is
// strictly prohibited.
//

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12InstanceRender.h>

//#include <NvFoundation.h>

// constant buffer definition shared with tessellation shaders
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>
#include <Nv/Common/NvCoString.h>
#include <Nv/Common/NvCoLogger.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ApiInstance.h>

#include <Nv/HairWorks/Internal/NvHairShaderCache.h>
#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

// constant buffer definition shared with pixel shaders
#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ShaderCacheEntry.h>
#include <Nv/HairWorks/Internal/NvHairInstance.h>
#include <Nv/HairWorks/Internal/Dx/NvHairDxRenderUtil.h>

#include <Nv/HairWorks/Internal/Dx/NvHairDxSimulateUtil.h>

#include "d3dx12.h"

namespace nvidia {
namespace HairWorks { 

Void Dx12InstanceRender::calcPixelConstantBuffer(Dx12ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& extConstantBuffer)
{
	DxRenderUtil::calcPixelConstantBuffer(apiInst->m_apiGlobal, apiInst->m_instance, frameViewInfo, extConstantBuffer);
}

/* static */Result Dx12InstanceRender::renderHairShading(Dx12ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other)
{
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	Dx12ApiGlobal* glob = apiAsset->m_apiGlobal;
	ShaderCache* shaderCache = glob->getShaderCache();

	const Dx12RenderInfo* renderInfo = NV_NULL;
	if (!other.isNull())
	{
		renderInfo = Dx12SdkType::cast<Dx12RenderInfo>(other);
	}

	Instance* inst = apiInst->m_instance;
	const Asset* asset = inst->m_asset;

	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();

	// apply the shader pass 
	const Bool useSimpleShader = defaultMaterial.m_colorizeMode == ColorizeMode::RED;

	// Set the defaults
	const Dx12PixelShaderInfo* pixelShader = &glob->m_simplePixelShader;
	Dx12ShaderPass* passSelector = &glob->m_passes[Dx12ApiGlobal::PASS_INTERPOLATE];
	if (!useSimpleShader)
	{
		// Get the info on the pixel shader at that index
		const Int pixelShaderIndex = shaderSettings.m_shaderIndex; 
		pixelShader = (pixelShaderIndex >= 0 || pixelShaderIndex < glob->m_pixelShaders.getSize()) ? &glob->m_pixelShaders[pixelShaderIndex] : NV_NULL;
		if (pixelShader == NV_NULL || !pixelShader->isInitialized())
		{
			NvCo::String msg;
			msg << "Pixel shader index is invalid :" << pixelShaderIndex;
			NV_CO_LOG_ERROR(msg.getCstr());
			NV_CORE_ASSERT("Pixel shader index is invalid");
			return NV_FAIL;
		}

		// Find the shader in the cache
		ShaderCacheSettings cacheSettings;
		ShaderCache::calcSettings(inst, cacheSettings);

		if (frameViewInfo.m_useCubeMap)
		{
			cacheSettings.m_useFlags &= ~ShaderCacheSettings::FLAG_SINGLE_TARGET;
		}
		else 
		{
			cacheSettings.m_useFlags |= ShaderCacheSettings::FLAG_SINGLE_TARGET;
		}

		cacheSettings = shaderCache->calcUniqueSettings(cacheSettings);

		ShaderCacheEntry* cacheEntry = inst->m_cacheEntry;
		// shader cache was already created and signature matches
		if (cacheEntry == NV_NULL || cacheEntry->getSettings() != cacheSettings)
		{
			// Create it
			cacheEntry = shaderCache->findOrCreate(cacheSettings);
			if (!cacheEntry)
			{
				NV_CO_LOG_ERROR("Unable to create shader entry");
				return NV_FAIL;
			}
			// Set the entry
			inst->m_cacheEntry = cacheEntry;
		}
		// Okay when I hit here.. I know I have a cache entry
		Dx12ShaderCacheEntry* dxEntry = static_cast<Dx12ShaderCacheEntry*>(cacheEntry);
		if (pixelShaderIndex >= dxEntry->m_passes.getSize() || !dxEntry->m_passes[pixelShaderIndex].isInitialized())
		{
			// We don't have the pass for the currently set shader index
			if (pixelShaderIndex >= dxEntry->m_passes.getSize())
			{
				dxEntry->m_passes.setSize(pixelShaderIndex + 1);
			}
			Dx12ShaderPass& pass = dxEntry->m_passes[pixelShaderIndex];

			Dx12ShaderPass::Desc desc;
			Dx12ShaderPass::DescriptorDesc descriptorDesc;

			// Set up as the interpolate shader
			Dx12ApiGlobal::initInterpolate(desc, frameViewInfo.m_useCubeMap, descriptorDesc);
			
			// Change the pixel shader
			desc.m_pixelBlob.pShaderBytecode = pixelShader->m_pixelBlob;
			desc.m_pixelBlob.BytecodeLength = pixelShader->m_pixelBlobSize;
			// Set the wanted format
			desc.m_targetInfo = pixelShader->m_targetInfo;
			
			Dx12ShaderPass::DescriptorDesc optionalDesc;
			optionalDesc.m_hasDynamicConstantBuffer = pixelShader->m_hasDynamicConstantBuffer;
			optionalDesc.m_numCbvs = pixelShader->m_numCbvs;
			optionalDesc.m_numSrvs = pixelShader->m_numSrvs;

			// Create the pass
			NV_RETURN_ON_FAIL(pass.initPatch(glob->getDevice(), descriptorDesc, desc, &optionalDesc));

			passSelector = &pass;
		}
		else
		{
			passSelector = &dxEntry->m_passes[pixelShaderIndex];
		}
	}

	// Set up initially pixel shader dynamic constant buffer 
	Heap::Cursor constantBufferCursor = {};
	NV_CORE_ASSERT(!constantBufferCursor.isValid());

	// set shader resources
	{
		// set shader resources for HS/DS/GS
		if (!shaderSettings.m_useCustomConstantBuffer)
		{
			constantBufferCursor = glob->m_uploadHeap.allocateConstantBuffer(sizeof(ShaderConstantBuffer));
			ShaderConstantBuffer* constantBuffer = (ShaderConstantBuffer*)constantBufferCursor.m_position;
			calcPixelConstantBuffer(apiInst, frameViewInfo, *constantBuffer);
		}
		else
		{
			//NV_CORE_ASSERT(renderInfo && renderInfo->m_constantBuffer);
			if (pixelShader->m_hasDynamicConstantBuffer)
			{
				NV_CORE_ASSERT(renderInfo && renderInfo->m_constantBufferData);
				constantBufferCursor = glob->m_uploadHeap.newConstantBuffer(renderInfo->m_constantBufferData, renderInfo->m_constantBufferSize);
			}
		}
	}

	const LodInfo& lodInfo = inst->m_lodInfo;
	float density, width;
	lodInfo.calcDensityAndWidth(defaultMaterial, &density, &width);

	// scale width based on unit
	float unitScale = 0.1f / asset->getUnitInCentimeters();
	width *= unitScale;

	if (shaderSettings.m_shadowPass)
	{
		float shadowDensityScale = defaultMaterial.m_shadowDensityScale;
		float baseWidthShadowScale = 2.0f * min(10.0f, 1.0f / (shadowDensityScale + 0.0001f));

		width *= baseWidthShadowScale;
		density *= shadowDensityScale;
	}

	float maxDensity = density;

	Dx12ShaderPass& pass = *passSelector;

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();

	Dx12StreamOutBuffer& tessellatedMasterStrand = apiInst->getCurrentTessellatedMasterStrand();
	Dx12StreamOutBuffer& tessellatedMasterStrandPrev = apiInst->getPreviousTessellatedMasterStrand();

	{
		Dx12BarrierSubmitter barriers(commandList);
		tessellatedMasterStrand.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		tessellatedMasterStrandPrev.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_tessellatedMasterStrandTangentsBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_tessellatedMasterStrandNormalsBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		apiInst->m_masterStrandInterpolationDeltaBuffer.transition(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
	}

	// draw each tessellated hair pass

	// Work out the descriptors in the cache
	ID3D12DescriptorHeap* viewHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[3] = { 0 };
	{		
		// build SRV
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[] = 
		{
			tessellatedMasterStrand.getSrvCpuHandle(),
			tessellatedMasterStrandPrev.getSrvCpuHandle(),

			apiAsset->m_origMasterStrandBuffer.getSrvCpuHandle(),
			apiAsset->m_faceTexCoordsBuffer.getSrvCpuHandle(),

			apiAsset->m_faceHairIndicesBuffer.getSrvCpuHandle(),

			glob->m_strandCoordinatesBuffer.getSrvCpuHandle(),
			glob->m_scalarNoiseBuffer.getSrvCpuHandle(),

			apiInst->getTextureSrv(TextureType::DENSITY),
			apiInst->getTextureSrv(TextureType::WIDTH),
			apiInst->getTextureSrv(TextureType::CLUMP_SCALE),
			apiInst->getTextureSrv(TextureType::CLUMP_ROUNDNESS),
			apiInst->getTextureSrv(TextureType::WAVE_SCALE),
			apiInst->getTextureSrv(TextureType::WAVE_FREQ),
			apiInst->getTextureSrv(TextureType::LENGTH),
			apiInst->getTextureSrv(TextureType::WEIGHTS),

			apiInst->m_tessellatedMasterStrandTangentsBuffer.getSrvCpuHandle(),
			apiInst->m_tessellatedMasterStrandNormalsBuffer.getSrvCpuHandle(),
		};

		if (renderInfo)
		{
			DescriptorSet sets[] =
			{
				DescriptorSet(DescriptorSet::TYPE_SRV, srvs, NV_COUNT_OF(srvs)),						// Main srvs
				DescriptorSet(DescriptorSet::TYPE_CBV, renderInfo->m_cbvs, pixelShader->m_numCbvs),	// Optional cbvs
				DescriptorSet(DescriptorSet::TYPE_SRV, renderInfo->m_srvs, pixelShader->m_numSrvs),	// Optional srvs
			};
			// Works out the changed flags
			Int hasChangedFlags = 0;
			hasChangedFlags |= (pixelShader->m_numCbvs > 0 && renderInfo->m_descriptorContentsChangedFlags & Dx12DescriptorFlag::CBV) ? 2 : 0;
			hasChangedFlags |= (pixelShader->m_numSrvs > 0 && renderInfo->m_descriptorContentsChangedFlags & Dx12DescriptorFlag::SRV) ? 4 : 0;
			
			viewHeap = glob->m_viewCache.put(sets, hasChangedFlags, viewHandles);
		}
		else
		{
			NV_CORE_ASSERT(pixelShader->m_numSrvs == 0 && pixelShader->m_numCbvs == 0);
			viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, &viewHandles[0]);
		}
	}

	ID3D12DescriptorHeap* heaps[] = { viewHeap, glob->m_samplerHeap.getHeap() };

	if (false)
	{
		Dx12Bundle& bundle = apiInst->m_bundles[Dx12ApiInstance::BUNDLE_RENDER];
		ID3D12GraphicsCommandList* bundleList = bundle.getCommandList();
		if (bundle.start(heaps, NV_COUNT_OF(heaps), viewHandles, NV_COUNT_OF(viewHandles), glob->m_sampleCountMsaa))
		{
			bundleList->SetGraphicsRootSignature(pass.m_rootSignature);
			bundleList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

			// Slots 0-3 are written later for each of the tessellation passes
			// NOTE that Slots 0-3 are used for tessellation (with reg b0 - on everything but pixel)

			{
				const D3D12_GPU_DESCRIPTOR_HANDLE srvBase = viewHandles[0];
				const D3D12_GPU_DESCRIPTOR_HANDLE samplerBase = glob->m_samplerHeap.getGpuStart();

				bundleList->SetGraphicsRootDescriptorTable(4, srvBase);
				bundleList->SetGraphicsRootDescriptorTable(5, srvBase);
				bundleList->SetGraphicsRootDescriptorTable(6, srvBase);
				bundleList->SetGraphicsRootDescriptorTable(7, srvBase);
				bundleList->SetGraphicsRootDescriptorTable(8, samplerBase);
				bundleList->SetGraphicsRootDescriptorTable(9, samplerBase);
			}

			Int rootTableIndex = 10;

			// Set the constant buffer (currently just supporting one)...
			if (constantBufferCursor.isValid())
			{
				bundleList->SetGraphicsRootConstantBufferView(rootTableIndex++, constantBufferCursor.getGpuHandle());
			}
			//
			{
				if (pixelShader->m_numCbvs > 0)
				{
					bundleList->SetGraphicsRootDescriptorTable(rootTableIndex++, viewHandles[1]);
				}
				// Custom srv
				if (pixelShader->m_numSrvs > 0)
				{
					bundleList->SetGraphicsRootDescriptorTable(rootTableIndex++, viewHandles[2]);
				}
			}

			if (glob->m_sampleCountMsaa == 0)
			{
				bundleList->SetPipelineState(pass.m_pipelineState);
			}
			else
			{
				DWORD index;
				_BitScanForward(&index, glob->m_sampleCountMsaa);
				bundleList->SetPipelineState(pass.m_pipelineStateMsaa[index]);
			}
			bundleList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

			bundleList->DrawInstanced(asset->getDesc().m_numFaces, 1, 0, 0);

			bundle.end();
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		// draw each tessellated hair pass
		for (float densityPass = 0; densityPass < maxDensity; densityPass += 1.0f)
		{
			NV_CORE_ASSERT(int(densityPass) < Dx12ApiInstance::MAX_NUM_TESS_PASSES);

			Heap::Cursor tessCursor = glob->m_uploadHeap.allocateConstantBuffer(sizeof(NvHair_TessellationConstantBuffer));
			NvHair_TessellationConstantBuffer* tessConstantBuffer = (NvHair_TessellationConstantBuffer*)tessCursor.m_position;

			DxRenderUtil::calcTessellationConstantBuffer(apiInst->m_instance, frameViewInfo, density, width, densityPass, *tessConstantBuffer);

			D3D12_GPU_VIRTUAL_ADDRESS tessConstBufGpuAddr = tessCursor.getGpuHandle();
			
			commandList->SetGraphicsRootConstantBufferView(0, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(1, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(2, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(3, tessConstBufGpuAddr);

			commandList->ExecuteBundle(bundleList);
		}
	}
	else
	{
		{
			commandList->SetGraphicsRootSignature(pass.m_rootSignature);

			if (glob->m_sampleCountMsaa == 0)
			{
				commandList->SetPipelineState(pass.m_pipelineState);
			}
			else
			{
				DWORD index;
				_BitScanForward(&index, glob->m_sampleCountMsaa);
				commandList->SetPipelineState(pass.m_pipelineStateMsaa[index]);
			}

			commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

			// Slots 0-3 are written later for each of the tessellation passes
			// NOTE that Slots 0-3 are used for tessellation (with reg b0 - on everything but pixel)

			{
				const D3D12_GPU_DESCRIPTOR_HANDLE srvBase = viewHandles[0];
				const D3D12_GPU_DESCRIPTOR_HANDLE samplerBase = glob->m_samplerHeap.getGpuStart();

				commandList->SetGraphicsRootDescriptorTable(4, srvBase);
				commandList->SetGraphicsRootDescriptorTable(5, srvBase);
				commandList->SetGraphicsRootDescriptorTable(6, srvBase);
				commandList->SetGraphicsRootDescriptorTable(7, srvBase);
				commandList->SetGraphicsRootDescriptorTable(8, samplerBase);
				commandList->SetGraphicsRootDescriptorTable(9, samplerBase);
				commandList->SetGraphicsRootDescriptorTable(10, samplerBase);
			}

			Int rootTableIndex = 11;

			// Set the constant buffer (currently just supporting one)...
			if (constantBufferCursor.isValid())
			{
				commandList->SetGraphicsRootConstantBufferView(rootTableIndex++, constantBufferCursor.getGpuHandle());
			}
			//
			{
				if (pixelShader->m_numCbvs > 0)
				{
					commandList->SetGraphicsRootDescriptorTable(rootTableIndex++, viewHandles[1]);
				}
				// Custom srv
				if (pixelShader->m_numSrvs > 0)
				{
					commandList->SetGraphicsRootDescriptorTable(rootTableIndex++, viewHandles[2]);
				}
			}
		}

		commandList->SetGraphicsRootSignature(pass.m_rootSignature);
		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		// draw each tessellated hair pass
		for (float densityPass = 0; densityPass < maxDensity; densityPass += 1.0f)
		{
			NV_CORE_ASSERT(int(densityPass) < Dx12ApiInstance::MAX_NUM_TESS_PASSES);

			Heap::Cursor tessCursor = glob->m_uploadHeap.allocateConstantBuffer(sizeof(NvHair_TessellationConstantBuffer));
			NvHair_TessellationConstantBuffer* tessConstantBuffer = (NvHair_TessellationConstantBuffer*)tessCursor.m_position;

			DxRenderUtil::calcTessellationConstantBuffer(apiInst->m_instance, frameViewInfo, density, width, densityPass, *tessConstantBuffer);

			D3D12_GPU_VIRTUAL_ADDRESS tessConstBufGpuAddr = tessCursor.getGpuHandle();

			commandList->SetGraphicsRootConstantBufferView(0, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(1, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(2, tessConstBufGpuAddr);
			commandList->SetGraphicsRootConstantBufferView(3, tessConstBufGpuAddr);

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
			commandList->DrawInstanced(asset->getDesc().m_numFaces, 1, 0, 0);
		}
	}

	{
		Dx12BarrierSubmitter barriers(commandList);
		tessellatedMasterStrand.restore(barriers);
		tessellatedMasterStrandPrev.restore(barriers);
		apiInst->m_tessellatedMasterStrandTangentsBuffer.restore(barriers);
		apiInst->m_tessellatedMasterStrandNormalsBuffer.restore(barriers);
	}

	return NV_OK;
}

/* static */Result Dx12InstanceRender::dispatchCalcInterpolationDelta(Dx12ApiInstance* apiInst, Float simulationInterp)
{
	Dx12ApiGlobal* glob = apiInst->m_apiGlobal;
	Instance* inst = apiInst->m_instance;
	const Dx12ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	Heap::Cursor constantCursor = glob->m_uploadHeap.allocateConstantBuffer(sizeof(NvHair_SimulateConstantBuffer));

	DxSimulateUtil::calcConstantBuffer(inst, 0.0f, simulationInterp, &inst->m_modelToWorld, (NvHair_SimulateConstantBuffer*)constantCursor.m_position);

	ID3D12DescriptorHeap* viewHeap;
	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[2] = { 0 };
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			apiAsset->m_masterStrandOffsetBuffer.getSrvCpuHandle(),
			apiAsset->m_origMasterStrandBuffer.getSrvCpuHandle(),
			apiAsset->m_boneIndicesBuffer.getSrvCpuHandle(),
			apiAsset->m_boneWeightsBuffer.getSrvCpuHandle(),
		};
		//set the unordered access views - this is what the CS shader will read from and write to
		const D3D12_CPU_DESCRIPTOR_HANDLE uavs[] =
		{
			apiInst->m_masterStrandBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandInterpBuffer.getUavCpuHandle(),
			apiInst->m_masterStrandInterpolationDeltaBuffer.getUavCpuHandle(),
		};
		DescriptorSet sets[] =
		{
			DescriptorSet(DescriptorSet::TYPE_SRV, srvs),
			DescriptorSet(DescriptorSet::TYPE_UAV, uavs),
		};
		viewHeap = glob->m_viewCache.put(sets, 0, viewHandles);
	}

	ID3D12GraphicsCommandList* commandList = glob->getCommandList();
	{
		Dx12BarrierSubmitter barriers(commandList);
		// Make sure any writes to these have completed
		apiInst->m_masterStrandBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		apiInst->m_masterStrandPrevBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
		
		apiInst->m_masterStrandInterpolationDeltaBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	{
		const Dx12ShaderPass& pass = glob->m_hairPrepareInterpolate;
		ID3D12DescriptorHeap* heaps[] = { viewHeap };

		commandList->SetComputeRootSignature(pass.m_rootSignature);
		commandList->SetPipelineState(pass.m_pipelineState);

		commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);

		commandList->SetComputeRootConstantBufferView(0, constantCursor.getGpuHandle());
		commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
		commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
	
		commandList->Dispatch(asset->m_numMasterStrands, 1, 1);
	}

	{
		Dx12BarrierSubmitter barriers(commandList);
		apiInst->m_masterStrandInterpolationDeltaBuffer.transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barriers);
	}

	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia 