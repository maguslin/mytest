/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11InstanceRender.h>

// constant buffer definition shared with tessellation shaders
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11ApiInstance.h>

#include <Nv/HairWorks/Internal/NvHairShaderCache.h>
#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

#include <Nv/HairWorks/Internal/Dx/NvHairDxRenderUtil.h>

// constant buffer definition shared with pixel shaders
#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>
#include <Nv/HairWorks/Internal/Dx/NvHairDxSimulateUtil.h>

namespace nvidia {
namespace HairWorks { 

/* static */Result Dx11InstanceRender::_prepareTessellationConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& settings, 
	float density, float width, float densityPass)
{
	NV_UNUSED(settings)

	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();

	Instance* inst = apiInst->m_instance;

	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		NV_RETURN_ON_FAIL(context->Map( apiInst->m_hairTessellationConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource ));
		NvHair_TessellationConstantBuffer* constantBuffer = ( NvHair_TessellationConstantBuffer* )mappedResource.pData;
		DxRenderUtil::calcTessellationConstantBuffer(inst, frameViewInfo, density, width, densityPass, *constantBuffer);
		context->Unmap(apiInst->m_hairTessellationConstantBuffer,0);
	}

	context->VSSetConstantBuffers(0, 1, apiInst->m_hairTessellationConstantBuffer.readRef());
	context->HSSetConstantBuffers(0, 1, apiInst->m_hairTessellationConstantBuffer.readRef());
	context->DSSetConstantBuffers(0, 1, apiInst->m_hairTessellationConstantBuffer.readRef());
	context->GSSetConstantBuffers(0, 1, apiInst->m_hairTessellationConstantBuffer.readRef());
	return NV_OK;
}

Void Dx11InstanceRender::calcPixelConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& extConstantBuffer)
{
	DxRenderUtil::calcPixelConstantBuffer(apiInst->m_apiGlobal, apiInst->m_instance, frameViewInfo, extConstantBuffer);
}

/* static */Result Dx11InstanceRender::_preparePixelConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	//HairInstance* inst = apiInst->m_instance;
	ID3D11DeviceContext* context = glob->getContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map( apiInst->m_hairPixelShaderConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource ));
	ShaderConstantBuffer& perFrame = *( ShaderConstantBuffer*)mappedResource.pData;
	calcPixelConstantBuffer(apiInst, frameViewInfo, perFrame);
	context->Unmap(apiInst->m_hairPixelShaderConstantBuffer, 0);
	context->PSSetConstantBuffers(0, 1, apiInst->m_hairPixelShaderConstantBuffer.readRef());

	return NV_OK;
}

Void Dx11InstanceRender::_prepareTessellationResources(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;

	// we use double buffering for tessellated master strand buffer
	ID3D11ShaderResourceView* tessellatedMasterStrandSrv = apiInst->getCurrentTessellatedMasterStrandSrv();
	ID3D11ShaderResourceView* tessellatedMasterStrandPrevSrv = apiInst->getPreviousTessellatedMasterStrandSrv();

	// prepare all the shader resources
	{
		ID3D11ShaderResourceView* const srvs[] = 
		{ 
			tessellatedMasterStrandSrv,		// g_TessellatedMasterStrand
			tessellatedMasterStrandPrevSrv,		// g_TessellatedMasterStrandPrev

			apiAsset->m_origMasterStrandSrv,		
			apiAsset->m_faceTexCoordsSrv,		// g_TexCoords

			apiAsset->m_faceHairIndicesSrv,		// g_FaceRootIndices

			glob->m_strandCoordinatesSrv,	// strand barycentric coord
			glob->m_scalarNoiseSrv,			// 

			apiInst->getTexture(TextureType::DENSITY),
			apiInst->getTexture(TextureType::WIDTH),

			apiInst->getTexture(TextureType::CLUMP_SCALE),
			apiInst->getTexture(TextureType::CLUMP_ROUNDNESS),

			apiInst->getTexture(TextureType::WAVE_SCALE),
			apiInst->getTexture(TextureType::WAVE_FREQ),

			apiInst->getTexture(TextureType::LENGTH),
			apiInst->getTexture(TextureType::WEIGHTS),

			apiInst->m_tessellatedMasterStrandTangentsSrv, 
			apiInst->m_tessellatedMasterStrandNormalsSrv, 
		};
		// SRV for tessellation pipeline
		context->HSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
		context->DSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
		context->GSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
}

Void Dx11InstanceRender::_preparePixelResources(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();

	ID3D11ShaderResourceView* const srvs[] = 
	{ 
		apiInst->getTexture(TextureType::ROOT_COLOR),
		apiInst->getTexture(TextureType::TIP_COLOR),
	};
	// SRV for pixel shader
	context->PSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
}

/* static */Result Dx11InstanceRender::renderHairShading(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& shaderSettings)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	
	const Asset* asset = inst->m_asset;

	ShaderCache* shaderCache = glob->getShaderCache();

	const InstanceDescriptor& params = inst->getDefaultMaterial();
	
	// Use specialized shader pass set if cubemap is required
	const Dx11ShaderPass& pass = frameViewInfo.m_useCubeMap ? glob->m_passes[Dx11ApiGlobal::PASS_CUBE_MAP_INTERPOLATE] : glob->m_passes[Dx11ApiGlobal::PASS_INTERPOLATE];
	Dx11ScopeShaderPass scope(pass, context, true);
	

	// runtime optimize shader with caches
	{
		ShaderCacheSettings cacheSettings;
		ShaderCache::calcSettings(inst, cacheSettings);
		cacheSettings = shaderCache->calcUniqueSettings(cacheSettings);

		ShaderCacheEntry* cacheEntry = inst->m_cacheEntry;

		// shader cache was already created and signature matches
		if (cacheEntry == NV_NULL || cacheEntry->getSettings() != cacheSettings)
		{
			cacheEntry = shaderCache->findOrCreate(cacheSettings);
			if (!cacheEntry)
			{
				return NV_FAIL;
			}
			inst->m_cacheEntry = cacheEntry;
			cacheEntry->createShaders(glob->getApiDevice());			
		}

		// If here we must have a cache entry
		cacheEntry->applyShaders(glob->getApiDevice(), glob->getApiContext());
	}

	// apply the shader pass 
	bool useSimpleShader = (params.m_colorizeMode == Int(ColorizeMode::RED));
	if (useSimpleShader)
		context->PSSetShader(glob->m_hairSimpleShader, NV_NULL, 0);

	// set shader resources
	{
		// set shader resources for HS/DS/GS
		_prepareTessellationResources(apiInst);
		if (!shaderSettings.m_useCustomConstantBuffer)
		{
			NV_RETURN_ON_FAIL(_preparePixelConstantBuffer(apiInst, frameViewInfo));
		}
	}

	// we don't need a vertex buffer since we are only using vertexID and instanceID
	{
		unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* const nullBuffers[] = { NV_NULL };
		context->IASetVertexBuffers(0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	}

	const LodInfo& lodInfo = inst->m_lodInfo;
	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();

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

	// draw each tessellated hair pass
	for (float densityPass = 0; densityPass < maxDensity; densityPass += 1.0f)
	{
		NV_RETURN_ON_FAIL(_prepareTessellationConstantBuffer(apiInst, frameViewInfo, shaderSettings, density, width, densityPass));
		context->Draw(apiInst->m_numFaces, 0);
	}

	return NV_OK;
}

Void Dx11InstanceRender::_cleanComputeResources(ID3D11DeviceContext* context, int numUavs, int numSrvs)
{
	const int maxSize = 128;

	static void* const nulls[maxSize] = { NV_NULL };
	static const UINT initCounts[maxSize] = { 0 };

	// Unbind resources for CS
	NV_CORE_ASSERT(numUavs >= 0 && numUavs <= maxSize);
	NV_CORE_ASSERT(numSrvs >= 0 && numSrvs <= maxSize);

	if (numSrvs > 0)
	{
		context->CSSetShaderResources(0, numSrvs, (ID3D11ShaderResourceView*const*)nulls);
	}
	if (numUavs > 0)
	{
		context->CSSetUnorderedAccessViews(0, numUavs, (ID3D11UnorderedAccessView*const*)nulls, initCounts);
	}
}

Result Dx11InstanceRender::prepareSimulationConstantBuffer(Dx11ApiInstance* apiInst, float timeStep, Float simulationInterp, const gfsdk_float4x4* mat)
{
	NV_UNUSED(mat);

	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();

	Instance* inst = apiInst->m_instance;

	// map constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(context->Map(apiInst->m_csConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	DxSimulateUtil::calcConstantBuffer(inst, timeStep, simulationInterp, mat, (NvHair_SimulateConstantBuffer*)mappedResource.pData);
	context->Unmap(apiInst->m_csConstantBuffer, 0);
	context->CSSetConstantBuffers(0, 1, apiInst->m_csConstantBuffer.readRef());
	return NV_OK;
}

/* static */Result Dx11InstanceRender::dispatchPrepareInterpolate(Dx11ApiInstance* apiInst)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	Instance* inst = apiInst->m_instance;

	ID3D11DeviceContext* context = glob->getContext();

	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	ID3D11ComputeShader* hairPrepareInterpolateShader = glob->m_hairPrepareInterpolate;

	if (!hairPrepareInterpolateShader)
		return NV_FAIL;

	context->CSSetShader(hairPrepareInterpolateShader, NV_NULL, 0);

	ID3D11ShaderResourceView*const srvs[] =
	{
		apiAsset->m_masterStrandOffsetSrv,
		apiAsset->m_origMasterStrandSrv,		
		apiAsset->m_boneIndicesSrv,
		apiAsset->m_boneWeightsSrv,
	};
	context->CSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

	//set the sampler
	ID3D11SamplerState* const states[] = { glob->m_linearSampler };
	context->CSSetSamplers(0, NV_COUNT_OF(states), states);

	//set the unordered access views - this is what the CS shader will read from and write to
	ID3D11UnorderedAccessView* const uavs[] =
	{
		apiInst->m_masterStrandUav,
		apiInst->m_masterStrandInterpUav,
		apiInst->m_masterStrandInterpolationDeltaUav
	};

	UINT initCounts = 0;
	context->CSSetUnorderedAccessViews(0, NV_COUNT_OF(uavs), uavs, &initCounts);
	// Run the CS. we run one CTA for each strand. All the threads in a CTA will work co-operatively on a single strand
	context->Dispatch(asset->m_numMasterStrands, 1, 1);
	// clean up resources
	_cleanComputeResources(context, NV_COUNT_OF(uavs), NV_COUNT_OF(srvs));
	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia 