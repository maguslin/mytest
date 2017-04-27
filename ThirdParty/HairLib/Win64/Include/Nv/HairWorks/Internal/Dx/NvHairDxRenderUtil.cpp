/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairDxRenderUtil.h"

#include <Nv/HairWorks/Internal/NvHairInstance.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/Common/NvCoComPtr.h>

namespace nvidia { 
namespace HairWorks {

/* static */void DxRenderUtil::_calcTessellationMaterial(const InstanceDescriptor& desc, float density, float width, float scale, NvHair_TessellationMaterial& matOut)
{
	// grooming controls
	matOut.density = density;
	matOut.width = width;

	matOut.lengthNoise = desc.m_lengthNoise;
	matOut.lengthScale = desc.m_lengthScale;

	matOut.clumpScale = desc.m_clumpScale;
	matOut.clumpNoise = desc.m_clumpNoise;
	matOut.clumpRoundness = max(desc.m_clumpRoundness, 0.001f);

	matOut.waveScale = scale * desc.m_waveScale;
	matOut.waveScaleStrand = desc.m_waveScaleStrand;
	matOut.waveScaleClump = desc.m_waveScaleClump;
	matOut.waveScaleNoise = desc.m_waveScaleNoise;

	matOut.waveFreq = desc.m_waveFreq;
	matOut.waveFreqNoise = desc.m_waveFreqNoise;
	matOut.waveCutoff = desc.m_waveRootStraighten;

	matOut.rootWidthScale = desc.m_widthRootScale;
	matOut.tipWidthScale = desc.m_widthTipScale;
	matOut.widthNoiseScale = desc.m_widthNoise;
}

/* static */void DxRenderUtil::_calcMaterial(const InstanceDescriptor& desc, NvHair_Material& matOut)
{
	matOut.rootColor = desc.m_rootColor;
	matOut.tipColor = desc.m_tipColor;
	matOut.specularColor = desc.m_specularColor;

	matOut.diffuseBlend = desc.m_diffuseBlend;
	matOut.diffuseScale = 1.0f;
	matOut.diffuseHairNormalWeight = desc.m_hairNormalWeight;

	matOut.specularPrimaryScale = desc.m_specularPrimary;
	matOut.specularPrimaryBreakup = desc.m_specularPrimaryBreakup;
	matOut.specularSecondaryScale = desc.m_specularSecondary;
	matOut.specularPrimaryPower = desc.m_specularPowerPrimary;

	matOut.specularSecondaryPower = desc.m_specularPowerSecondary;
	matOut.specularNoiseScale = desc.m_specularNoiseScale;
	matOut.specularSecondaryOffset = desc.m_specularSecondaryOffset;

	matOut.rootTipColorWeight = desc.m_rootTipColorWeight;
	matOut.rootTipColorFalloff = desc.m_rootTipColorFalloff;
	matOut.rootAlphaFalloff = desc.m_rootAlphaFalloff;

	matOut.shadowSigma = desc.m_shadowSigma;
	matOut.strandBlendScale = desc.m_strandBlendScale;

	matOut.glintStrength = desc.m_glintStrength;
	matOut.glintCount = desc.m_glintCount;
	matOut.glintExponent = desc.m_glintExponent;
}

Void DxRenderUtil::calcPixelConstantBuffer(ApiGlobal* glob, Instance* inst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& extConstantBuffer)
{
	ApiInstance* apiInst = inst->m_apiInstance;
	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();
	
	NvHair_ConstantBuffer buf;

	// Current
	{
		const ViewInfo& view = frameViewInfo.m_currentView;
		buf.inverseViewProjection = view.m_inverseViewProjectionMatrix;
		buf.inverseProjection = view.m_inverseProjectionMatrix;
		buf.inverseViewport = view.m_inverseViewportMatrix;
		buf.inverseViewProjectionViewport = view.m_inverseViewProjectionViewportMatrix;

		buf.viewProjection = view.m_viewProjectionMatrix;
		buf.viewport = view.m_viewportMatrix;
		buf.camPosition = (float4&)view.m_eyePosition;
	}
	// Previous
	{
		const ViewInfo& view = frameViewInfo.m_previousView;

		buf.prevViewProjection = view.m_viewProjectionMatrix;
		buf.prevViewport = view.m_viewportMatrix;

	}

	// model
	{
		const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;

		gfsdk_float3 p = gfsdk_transformCoord(modelToWorld, inst->m_shadingCenter);
		buf.modelCenter = gfsdk_makeFloat4(p, 1);
	}

	// others constant values
	{
		buf.strandBlendMode = defaultMaterial.m_strandBlendMode;

		buf.useRootColorTexture = apiInst->isApiTextureUsed(TextureType::ROOT_COLOR);
		buf.useTipColorTexture = apiInst->isApiTextureUsed(TextureType::TIP_COLOR);
		buf.useSpecularTexture = apiInst->isApiTextureUsed(TextureType::SPECULAR);
		buf.useStrandTexture = apiInst->isApiTextureUsed(TextureType::STRAND);

		buf.receiveShadows = defaultMaterial.m_receiveShadows;
		buf.shadowUseLeftHanded = false;

		buf.colorizeMode = defaultMaterial.m_colorizeMode;

		buf.strandPointCount = (int)inst->getStrandPointCount();
	}

	// lod visualization factors
	{
		const LodInfo& lodInfo = inst->m_lodInfo;

		buf.lodDistanceFactor = lodInfo.m_distanceFactor;
		buf.lodDetailFactor = lodInfo.m_detailFactor;
		buf.lodAlphaFactor = lodInfo.m_alphaFactor;
	}

	// copy materials
	{
		_calcMaterial(defaultMaterial, buf.defaultMaterial);
	}

	// Check the strand NOISE_TABLE is big enough
	NV_COMPILE_TIME_ASSERT((sizeof(gfsdk_float4) * NvHair_ConstantBuffer::NOISE_TABLE_SIZE / sizeof(float)) <= ApiGlobal::STRAND_NOISE_TABLE_SIZE);

	// copy noise table
	NvCo::Memory::copy(buf.noiseTable, glob->getScalarNoise(), sizeof(gfsdk_float4) * NvHair_ConstantBuffer::NOISE_TABLE_SIZE);

	// be safe on copying into external constant buffer
	size_t internalBufferSize = sizeof(NvHair_ConstantBuffer);
	size_t externalBufferSize = sizeof(ShaderConstantBuffer);
	size_t copyBufferSize = (internalBufferSize < externalBufferSize) ? internalBufferSize : externalBufferSize;

	NvCo::Memory::copy(&extConstantBuffer, &buf, copyBufferSize);
}

/* static */Void DxRenderUtil::calcTessellationConstantBuffer(Instance* inst, const RenderViewInfo& frameViewInfo, float density, float width, float densityPass, NvHair_TessellationConstantBuffer& constantBufferOut)
{
	const Asset* asset = inst->m_asset;
	ApiInstance* apiInst = inst->m_apiInstance;

	NvHair_TessellationConstantBuffer* constantBuffer = &constantBufferOut;

	float sceneUnit = asset->getUnitInCentimeters();
	float scale = 1.0f / sceneUnit;

	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();

	{
		// copy transforms
		{
			const ViewInfo& view = frameViewInfo.m_currentView;
			constantBuffer->viewProjection = view.m_viewProjectionMatrix;
			constantBuffer->viewMatrix = view.m_viewMatrix;
			constantBuffer->inverseViewMatrix = view.m_inverseViewMatrix;
			constantBuffer->modelToWorld = inst->m_modelToWorld;
			constantBuffer->camPosition = (float4&)view.m_eyePosition;
		}
		// copy cube map transforms
		{
			constantBuffer->cubeMapActive[0] = frameViewInfo.m_useCubeMap;
			if (frameViewInfo.m_useCubeMap)
			{
				for (int i = 0; i < 6; ++i)
				{
					constantBuffer->cubeMapViewProjMatrix[i] = frameViewInfo.m_cubeViews[i].m_viewProjectionMatrix;
					constantBuffer->cubeMapInvViewMatrix[i] = frameViewInfo.m_cubeViews[i].m_inverseViewMatrix;
					constantBuffer->cubeMapVisible[i][0] = frameViewInfo.m_isCubeFaceVisible[i];
				}
			}
		}
		// culling
		{
			constantBuffer->useBackfaceCulling = defaultMaterial.m_useBackfaceCulling;
			constantBuffer->backfaceCullingThreshold = defaultMaterial.m_backfaceCullingThreshold;
			constantBuffer->useViewfrustrumCulling = defaultMaterial.m_useViewfrustrumCulling;
			constantBuffer->useCullSphere = defaultMaterial.m_useCullSphere;
			constantBuffer->cullSphereInvTransform = defaultMaterial.m_cullSphereInvTransform;
		}

		// texture bit
		{
			constantBuffer->useDensityTexture = apiInst->isApiTextureUsed(TextureType::DENSITY);
			constantBuffer->useWidthTexture = apiInst->isApiTextureUsed(TextureType::WIDTH);
			constantBuffer->useClumpScaleTexture = apiInst->isApiTextureUsed(TextureType::CLUMP_SCALE);
			constantBuffer->useClumpRoundnessTexture = apiInst->isApiTextureUsed(TextureType::CLUMP_ROUNDNESS);
			constantBuffer->useWaveScaleTexture = apiInst->isApiTextureUsed(TextureType::WAVE_SCALE);
			constantBuffer->useWaveFreqTexture = apiInst->isApiTextureUsed(TextureType::WAVE_FREQ);
			constantBuffer->useLengthTexture = apiInst->isApiTextureUsed(TextureType::LENGTH);
			constantBuffer->useWeightTexture = apiInst->isApiTextureUsed(TextureType::WEIGHTS);
		}

		// shader settings
		{
			constantBuffer->shaderMask = 0xffff;
			constantBuffer->leftHanded = frameViewInfo.m_useLeftHanded;
			constantBuffer->strandPointCount = (unsigned int)inst->getStrandPointCount();

			constantBuffer->densityPass = densityPass;
			constantBuffer->usePixelDensity = defaultMaterial.m_usePixelDensity;
			constantBuffer->vertexClumping = true;

			constantBuffer->materialWeight = 0.0f;
		}

		// set default and target material
		_calcTessellationMaterial(defaultMaterial, density, width, scale, constantBuffer->defaultMaterial);

		// texture channel

#define NV_HAIR_PACK_TEXTURE_CHANNEL( TARGET, CHANNEL) \
		constantBuffer->TARGET[0] = Int(inst->getDefaultTextureChannel(CHANNEL)); \
		constantBuffer->TARGET[1] = Int(inst->getCurrentTextureChannel(CHANNEL));

		NV_HAIR_PACK_TEXTURE_CHANNEL(densityTextureChan, TextureType::DENSITY);
		NV_HAIR_PACK_TEXTURE_CHANNEL(widthTextureChan, TextureType::WIDTH);
		NV_HAIR_PACK_TEXTURE_CHANNEL(clumpScaleTextureChan, TextureType::CLUMP_SCALE);

		NV_HAIR_PACK_TEXTURE_CHANNEL(clumpRoundnessTextureChan, TextureType::CLUMP_ROUNDNESS);
		NV_HAIR_PACK_TEXTURE_CHANNEL(waveScaleTextureChan, TextureType::WAVE_SCALE);
		NV_HAIR_PACK_TEXTURE_CHANNEL(waveFreqTextureChan, TextureType::WAVE_FREQ);
		NV_HAIR_PACK_TEXTURE_CHANNEL(lengthTextureChan, TextureType::LENGTH);

		NV_HAIR_PACK_TEXTURE_CHANNEL(weightTextureChan, TextureType::WEIGHTS);

#undef NV_HAIR_PACK_TEXTURE_CHANNEL
	}
}

} // namespace HairWorks
} // namespace nvidia

