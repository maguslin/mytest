/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvParameterized.h"
#include "XmlSerializer.h"
#include "BinSerializer.h"
#include "NsFileBuffer.h"
#include "NvTraits.h"
#include "NsMemoryBuffer.h"

#include "NvAssert.h"

#include "NvErrorCallback.h"

#include "NsGlobals.h"
#include "NsVersionNumber.h"

// legacy version
#include "Params/NvHairAssetDescriptor1p0.h"
#include "Params/NvHairInstanceDescriptor1p0.h"

#include "Params/NvHairAssetHeaderInfo.h"
#include "Params/NvHairSceneDescriptor.h"
#include "Params/NvHairAssetDescriptor.h"
#include "Params/NvHairInstanceDescriptor.h"

#include "NvHairParamsUtil.h"

#include <Nv/Common/NvCoLogger.h>
#include <Nv/Common/NvCoString.h>

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>
#include <Nv/HairWorks/Internal/Serialize/NvHairFileBufReadAdapter.h>
#include <Nv/HairWorks/Internal/Serialize/NvHairFileBufWriteAdapter.h>

#include <Nv/HairWorks/Internal/Util/NvHairVersionUtil.h>

using namespace nvidia;
using namespace nvidia::parameterized;

#include <stdio.h>

namespace nvidia {
namespace HairWorks {

#if 0
static void _logFormat(nvidia::NvErrorCallback& errorCallback, const char* format, ...)
{
	va_list va;
	va_start(va, format);
	char msg[1024];
	vsprintf(msg, format, va);
	va_end(va);
	errorCallback.reportError(NvErrorCode::eDEBUG_INFO, msg, 0, 0);
}

#define PARAM_LOG(format, ...) { _logFormat(context->mFoundation->getErrorCallback(), format, __VA_ARGS__); }
#endif

struct ParamsContext
{
	//NvFoundation* mFoundation;
	NvParameterized::Traits* m_traits;
	HairWorksInfoFactory* m_infoFactory;
	HairSceneDescriptorFactory* m_sceneDescriptorFactory;
	HairAssetDescriptorFactory* m_assetDescriptorFactory;
	HairInstanceDescriptorFactory* m_instanceDescriptorFactory;
	legacy::ver1p0::HairAssetDescriptorFactory* m_assetDescriptor10Factory;
	legacy::ver1p0::HairInstanceDescriptorFactory* m_instanceDescriptor10Factory;
};

class DefaultErrorCallback : public nvidia::NvErrorCallback
{
public:
	DefaultErrorCallback(void)
	{
	}

	virtual void reportError(nvidia::NvErrorCode::Enum code, const char* message, const char* file, int line)
	{
		NV_UNUSED(code);
		NV_UNUSED(file);
		NV_UNUSED(line);
		NV_UNUSED(message);
	}
private:
};

class DefaultAllocator : public nvidia::NvAllocatorCallback
{
public:
	DefaultAllocator(void)
	{
	}

	~DefaultAllocator(void)
	{
	}

	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		NV_UNUSED(typeName);
		NV_UNUSED(filename);
		NV_UNUSED(line);
		void *ret = ::_aligned_malloc(size,16);
		return ret;
	}

	virtual void deallocate(void* ptr)
	{
		::_aligned_free(ptr);
	}
private:
};

// Convert HairInstanceDescriptor from 1.0 to current(1.1)
class InstanceDescriptorConversion_1p0 : public NvParameterized::Conversion
{
	NvParameterized::Traits* m_traits;

	//Default conversion which simply copies unchanged fields
	NvParameterized::Conversion* m_defaultConversion;

public:

	InstanceDescriptorConversion_1p0(NvParameterized::Traits *traits): m_traits(traits)
	{
		m_defaultConversion = internalCreateDefaultConversion(m_traits);
	}

	bool operator()(NvParameterized::Interface &legacyObj, NvParameterized::Interface &obj)
	{
		//Copy unchanged fields
		if( !(*m_defaultConversion)(legacyObj, obj) )
			return false;

		//legacy::ver1p0::HairInstanceDescriptor &legacyData = (legacy::ver1p0::HairInstanceDescriptor &)legacyObj;
		//HairInstanceDescriptor &data = (HairInstanceDescriptor &)obj;

		// For now, nothing to be added here as we just added new parameters
		//Custom conversion code goes here
		//data.shockwaveParams.width = legacyData.shockwaveWidth;
		//data.shockwaveParams.travelVelocity = legacyData.travelVelocity;

		return true;
	}

	~InstanceDescriptorConversion_1p0()
	{
		m_defaultConversion->release();
	}

	void release()
	{
		this->~InstanceDescriptorConversion_1p0();
		m_traits->free(this);
	}

	static InstanceDescriptorConversion_1p0 *Create(NvParameterized::Traits *traits)
	{
		void *buf = traits->alloc(sizeof(InstanceDescriptorConversion_1p0));
		return NV_PLACEMENT_NEW(buf, InstanceDescriptorConversion_1p0)(traits);
	}
};

// Convert HairAssetDescriptor from 1.0 to current(1.1)
class AssetDescriptorConversion_1p0 : public NvParameterized::Conversion
{
	NvParameterized::Traits* m_traits;

	//Default conversion which simply copies unchanged fields
	NvParameterized::Conversion* m_defaultConversion;

public:

	AssetDescriptorConversion_1p0(NvParameterized::Traits *traits): m_traits(traits)
	{
		m_defaultConversion = internalCreateDefaultConversion(m_traits);
	}

	bool operator()(NvParameterized::Interface &legacyObj, NvParameterized::Interface &obj)
	{
		//Copy unchanged fields
		if( !(*m_defaultConversion)(legacyObj, obj) )
			return false;

		//legacy::ver1p0::HairAssetDescriptor &legacyData = (legacy::ver1p0::HairAssetDescriptor &)legacyObj;
		//HairAssetDescriptor &data = (HairAssetDescriptor &)obj;

		// For now, nothing to be added here as we just added new parameters
		//Custom conversion code goes here
		//data.shockwaveParams.width = legacyData.shockwaveWidth;
		//data.shockwaveParams.travelVelocity = legacyData.travelVelocity;

		return true;
	}

	~AssetDescriptorConversion_1p0()
	{
		m_defaultConversion->release();
	}

	void release()
	{
		this->~AssetDescriptorConversion_1p0();
		m_traits->free(this);
	}

	static AssetDescriptorConversion_1p0* Create(NvParameterized::Traits *traits)
	{
		void *buf = traits->alloc(sizeof(AssetDescriptorConversion_1p0));
		return NV_PLACEMENT_NEW(buf, AssetDescriptorConversion_1p0)(traits);
	}
};

template<typename T>
static bool LoadParamArrayT(ParamsContext*, NvParameterized::Interface* iface, const char* paramName, T** outValue)
{
	NvParameterized::Handle handle(iface);
	if (iface->getParameterHandle(paramName, handle) == NvParameterized::ERROR_NONE)
	{
		int arraySize;
		handle.getArraySize(arraySize);
		if (arraySize == 0)
		{
			*outValue = NV_NULL;
			return false;
		}
		*outValue = NvHair::allocatePodArray<T>(arraySize);
		handle.getParamArray<T>((T*)*outValue, arraySize);
		return true;
	}
	NV_CO_LOG_ERROR_FORMAT("load parameter failed. paramName=\"%s\")", paramName);
	return false;
}

static bool LoadParamString(ParamsContext* , NvParameterized::Interface* iface, const char* paramName, char* outString)
{
	if (outString == NULL)
	{
		return false;
	}
	NvParameterized::Handle handle(iface);
	if (iface->getParameterHandle(paramName, handle) == NvParameterized::ERROR_NONE)
	{
		const char* var;
		handle.getParamString(var);
		if (var)
		{
			strcpy(outString, var);
		}
		else
		{
			outString[0] = '\0';
		}
		return true;
	}
	NV_CO_LOG_ERROR_FORMAT("load string failed. paramName=\"%s\")", paramName);
	return false;
}

template<typename T>
static bool SaveParamArrayT(ParamsContext*, NvParameterized::Interface* iface, const char* paramName, const T* value, int arraySize)
{
	if (value == NV_NULL)
		return false;

	if (arraySize == 0) 
		return false;

	NvParameterized::Handle handle(iface);
	if (iface->getParameterHandle(paramName, handle) == NvParameterized::ERROR_NONE)
	{
		handle.resizeArray(arraySize);
		handle.setParamArray((T*)value, arraySize);
		return true;
	}
	NV_CO_LOG_ERROR_FORMAT("save array parameter failed. paramName=\"%s\")", paramName);
	return false;
}

static bool SaveParamString(ParamsContext*, NvParameterized::Interface* iface, const char* paramName, const char* inString)
{
	NvParameterized::Handle handle(iface);
	if (iface->getParameterHandle(paramName, handle) == NvParameterized::ERROR_NONE)
	{
		handle.setParamString(inString);
		return true;
	}
	NV_CO_LOG_ERROR_FORMAT("save string failed. paramName=\"%s\")", paramName);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename TARGET_DESC>
static void _writeInstanceDescriptor(TARGET_DESC& targetDesc, const NvHair::InstanceDescriptor* instanceDesc)
{
	using namespace NvHair;
	using namespace nvidia;

	targetDesc.densityTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::DENSITY]);

	targetDesc.widthTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::WIDTH]);

	targetDesc.lengthTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::LENGTH]);

	targetDesc.clumpScaleTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::CLUMP_SCALE]);
	targetDesc.clumpRoundnessTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::CLUMP_ROUNDNESS]);
	targetDesc.waveScaleTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::WAVE_SCALE]);
	targetDesc.waveFreqTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::WAVE_FREQ]);

	targetDesc.stiffnessTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::STIFFNESS]);
	targetDesc.rootStiffnessTextureChan = UInt32(instanceDesc->m_textureChannels[TextureType::ROOT_STIFFNESS]);

	targetDesc.width = instanceDesc->m_width;
	targetDesc.widthNoise = instanceDesc->m_widthNoise;
	targetDesc.clumpNoise = instanceDesc->m_clumpNoise;
	targetDesc.clumpRoundness = instanceDesc->m_clumpRoundness;
	targetDesc.clumpScale = instanceDesc->m_clumpScale;

	targetDesc.density = instanceDesc->m_density;
	targetDesc.lengthNoise = instanceDesc->m_lengthNoise;
	targetDesc.lengthScale = instanceDesc->m_lengthScale;
	targetDesc.widthRootScale = instanceDesc->m_widthRootScale;
	targetDesc.widthTipScale = instanceDesc->m_widthTipScale;
		
	targetDesc.waveRootStraighten = instanceDesc->m_waveRootStraighten;
	targetDesc.waveScale = instanceDesc->m_waveScale;
	targetDesc.waveScaleNoise = instanceDesc->m_waveScaleNoise;
	targetDesc.waveFreq = instanceDesc->m_waveFreq;
	targetDesc.waveFreqNoise = instanceDesc->m_waveFreqNoise;
	targetDesc.waveScaleClump = instanceDesc->m_waveScaleClump;
	targetDesc.waveScaleStrand = instanceDesc->m_waveScaleStrand;

	targetDesc.enableDistanceLOD = instanceDesc->m_enableDistanceLod;
	targetDesc.distanceLODStart = instanceDesc->m_distanceLodStart;
	targetDesc.distanceLODEnd= instanceDesc->m_distanceLodEnd;
	targetDesc.distanceLODFadeStart = instanceDesc->m_distanceLodFadeStart;
	targetDesc.distanceLODDensity = instanceDesc->m_distanceLodDensity;
	targetDesc.distanceLODWidth = instanceDesc->m_distanceLodWidth;

	targetDesc.enableDetailLOD = instanceDesc->m_enableDetailLod;
	targetDesc.detailLODStart = instanceDesc->m_detailLodStart;
	targetDesc.detailLODEnd= instanceDesc->m_detailLodEnd;
	targetDesc.detailLODDensity = instanceDesc->m_detailLodDensity;
	targetDesc.detailLODWidth = instanceDesc->m_detailLodWidth;

	targetDesc.splineMultiplier = instanceDesc->m_splineMultiplier;

	targetDesc.assetType = instanceDesc->m_assetType;
	targetDesc.assetPriority = instanceDesc->m_assetPriority;
	targetDesc.assetGroup = instanceDesc->m_assetGroup;

	targetDesc.usePixelDensity = instanceDesc->m_usePixelDensity;
	targetDesc.useViewfrustrumCulling = instanceDesc->m_useViewfrustrumCulling;
	targetDesc.useBackfaceCulling = instanceDesc->m_useBackfaceCulling;
	targetDesc.backfaceCullingThreshold = instanceDesc->m_backfaceCullingThreshold;

	targetDesc.diffuseBlend = instanceDesc->m_diffuseBlend;
	targetDesc.diffuseHairNormalWeight = instanceDesc->m_hairNormalWeight;
	targetDesc.diffuseBoneIndex = instanceDesc->m_hairNormalBoneIndex;
		
	memcpy(&targetDesc.rootColor, &instanceDesc->m_rootColor, sizeof(instanceDesc->m_rootColor));
	memcpy(&targetDesc.tipColor, &instanceDesc->m_tipColor, sizeof(instanceDesc->m_tipColor));
	memcpy(&targetDesc.specularColor, &instanceDesc->m_specularColor, sizeof(instanceDesc->m_specularColor));
	targetDesc.shadowSigma = instanceDesc->m_shadowSigma;

	targetDesc.glintStrength = instanceDesc->m_glintStrength;
	targetDesc.glintCount = instanceDesc->m_glintCount;
	targetDesc.glintExponent = instanceDesc->m_glintExponent;

	targetDesc.specularNoiseScale = instanceDesc->m_specularNoiseScale;
	targetDesc.specularEnvScale = instanceDesc->m_specularEnvScale;
	targetDesc.specularPrimary = instanceDesc->m_specularPrimary;
	targetDesc.specularPrimaryBreakup = instanceDesc->m_specularPrimaryBreakup;
	targetDesc.specularSecondary = instanceDesc->m_specularSecondary;
	targetDesc.specularSecondaryOffset = instanceDesc->m_specularSecondaryOffset;
	targetDesc.specularPowerPrimary = instanceDesc->m_specularPowerPrimary;
	targetDesc.specularPowerSecondary = instanceDesc->m_specularPowerSecondary;
	targetDesc.strandBlendScale = instanceDesc->m_strandBlendScale;
	targetDesc.strandBlendMode = instanceDesc->m_strandBlendMode;
	targetDesc.castShadows = instanceDesc->m_castShadows;
	targetDesc.receiveShadows = instanceDesc->m_receiveShadows;
	targetDesc.shadowDensityScale = instanceDesc->m_shadowDensityScale;

	targetDesc.backStopRadius = instanceDesc->m_backStopRadius;
	targetDesc.bendStiffness = instanceDesc->m_bendStiffness;
	targetDesc.pinStiffness = instanceDesc->m_pinStiffness;
	targetDesc.useCollision = instanceDesc->m_useCollision;
	targetDesc.useDynamicPin = instanceDesc->m_useDynamicPin;
	targetDesc.damping = instanceDesc->m_damping;
	targetDesc.friction = instanceDesc->m_friction;
	targetDesc.massScale = instanceDesc->m_massScale;
	memcpy(&targetDesc.gravity, &instanceDesc->m_gravityDir, sizeof(instanceDesc->m_gravityDir));
	targetDesc.simulate = instanceDesc->m_simulate;
	targetDesc.inertiaScale = instanceDesc->m_inertiaScale;
	targetDesc.inertiaLimit = instanceDesc->m_inertiaLimit;
	targetDesc.stiffness = instanceDesc->m_stiffness;
	targetDesc.stiffnessStrength = instanceDesc->m_stiffnessStrength;
	targetDesc.stiffnessDamping = instanceDesc->m_stiffnessDamping;
	targetDesc.rootStiffness = instanceDesc->m_rootStiffness;
	targetDesc.tipStiffness = instanceDesc->m_tipStiffness;

	targetDesc.interactionStiffness = instanceDesc->m_interactionStiffness;

	memcpy(&targetDesc.wind, &instanceDesc->m_wind, sizeof(instanceDesc->m_wind));
	targetDesc.windNoise = instanceDesc->m_windNoise;

	memcpy(&targetDesc.stiffnessCurve, &instanceDesc->m_stiffnessCurve, sizeof(instanceDesc->m_stiffnessCurve));
	memcpy(&targetDesc.stiffnessStrengthCurve, &instanceDesc->m_stiffnessStrengthCurve, sizeof(instanceDesc->m_stiffnessStrengthCurve));
	memcpy(&targetDesc.stiffnessDampingCurve, &instanceDesc->m_stiffnessDampingCurve, sizeof(instanceDesc->m_stiffnessDampingCurve));
	memcpy(&targetDesc.bendStiffnessCurve, &instanceDesc->m_bendStiffnessCurve, sizeof(instanceDesc->m_bendStiffnessCurve));
	memcpy(&targetDesc.interactionStiffnessCurve, &instanceDesc->m_interactionStiffnessCurve, sizeof(instanceDesc->m_interactionStiffnessCurve));

	targetDesc.visualizeBones = instanceDesc->m_visualizeBones;
	targetDesc.visualizeBoundingBox = instanceDesc->m_visualizeBoundingBox;
	targetDesc.visualizeCapsules = instanceDesc->m_visualizeCapsules;
	targetDesc.visualizeControlVertices = instanceDesc->m_visualizeControlVertices;
	targetDesc.visualizeCullSphere = instanceDesc->m_visualizeCullSphere;
	targetDesc.visualizeDiffuseBone = instanceDesc->m_visualizeShadingNormalBone;
	targetDesc.visualizeFrames	= instanceDesc->m_visualizeFrames;
	targetDesc.visualizeGrowthMesh = instanceDesc->m_visualizeGrowthMesh;
	targetDesc.visualizeGuideHairs = instanceDesc->m_visualizeGuideHairs;
	targetDesc.visualizeHairInteractions = instanceDesc->m_visualizeHairInteractions;
	targetDesc.visualizeHairSkips = instanceDesc->m_visualizeHairSkips;
	targetDesc.visualizeLocalPos	= instanceDesc->m_visualizeLocalPos;
	targetDesc.visualizePinConstraints	= instanceDesc->m_visualizePinConstraints;
	targetDesc.visualizeShadingNormals = instanceDesc->m_visualizeShadingNormals;
	targetDesc.visualizeSkinnedGuideHairs = instanceDesc->m_visualizeSkinnedGuideHairs;

	targetDesc.drawRenderHairs = instanceDesc->m_drawRenderHairs;
	targetDesc.enable = instanceDesc->m_enable;

	targetDesc.rootAlphaFalloff = instanceDesc->m_rootAlphaFalloff;
	targetDesc.rootTipColorWeight = instanceDesc->m_rootTipColorWeight;
	targetDesc.rootTipColorFalloff = instanceDesc->m_rootTipColorFalloff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename SOURCE_DESC>
void _readInstanceDescriptor(SOURCE_DESC& srcDesc, NvHair::InstanceDescriptor* outInstanceDesc)
{
	using namespace nvidia;
	using namespace NvHair;

	if (!outInstanceDesc)
		return;

	outInstanceDesc->m_textureChannels[TextureType::DENSITY] = (ETextureChannel)srcDesc.densityTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::WIDTH] = (ETextureChannel)srcDesc.widthTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::LENGTH] = (ETextureChannel)srcDesc.lengthTextureChan;

	outInstanceDesc->m_textureChannels[TextureType::CLUMP_SCALE] = (ETextureChannel)srcDesc.clumpScaleTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::CLUMP_ROUNDNESS] = (ETextureChannel)srcDesc.clumpRoundnessTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::WAVE_SCALE] = (ETextureChannel)srcDesc.waveScaleTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::WAVE_FREQ] = (ETextureChannel)srcDesc.waveFreqTextureChan;

	outInstanceDesc->m_textureChannels[TextureType::STIFFNESS] = (ETextureChannel)srcDesc.stiffnessTextureChan;
	outInstanceDesc->m_textureChannels[TextureType::ROOT_STIFFNESS] = (ETextureChannel)srcDesc.rootStiffnessTextureChan;

	outInstanceDesc->m_width = srcDesc.width;
	outInstanceDesc->m_widthNoise = srcDesc.widthNoise;
	outInstanceDesc->m_widthRootScale = srcDesc.widthRootScale;
	outInstanceDesc->m_widthTipScale = srcDesc.widthTipScale;

	outInstanceDesc->m_clumpNoise = srcDesc.clumpNoise;
	outInstanceDesc->m_clumpRoundness = srcDesc.clumpRoundness;
	outInstanceDesc->m_clumpScale = srcDesc.clumpScale;

	outInstanceDesc->m_density = srcDesc.density;
	outInstanceDesc->m_lengthNoise = srcDesc.lengthNoise;
	outInstanceDesc->m_lengthScale = srcDesc.lengthScale;

	outInstanceDesc->m_waveRootStraighten = srcDesc.waveRootStraighten;
	outInstanceDesc->m_waveScale = srcDesc.waveScale;
	outInstanceDesc->m_waveScaleNoise = srcDesc.waveScaleNoise;
	outInstanceDesc->m_waveFreq = srcDesc.waveFreq;
	outInstanceDesc->m_waveFreqNoise = srcDesc.waveFreqNoise;
	outInstanceDesc->m_waveScaleStrand = srcDesc.waveScaleStrand;
	outInstanceDesc->m_waveScaleClump = srcDesc.waveScaleClump;

	outInstanceDesc->m_enableDistanceLod = srcDesc.enableDistanceLOD;
	outInstanceDesc->m_distanceLodStart = srcDesc.distanceLODStart;
	outInstanceDesc->m_distanceLodEnd = srcDesc.distanceLODEnd;
	outInstanceDesc->m_distanceLodFadeStart = srcDesc.distanceLODFadeStart;
	outInstanceDesc->m_distanceLodDensity = srcDesc.distanceLODDensity;
	outInstanceDesc->m_distanceLodWidth = srcDesc.distanceLODWidth;

	outInstanceDesc->m_enableDetailLod = srcDesc.enableDetailLOD;
	outInstanceDesc->m_detailLodStart = srcDesc.detailLODStart;
	outInstanceDesc->m_detailLodEnd = srcDesc.detailLODEnd;
	outInstanceDesc->m_detailLodDensity = srcDesc.detailLODDensity;
	outInstanceDesc->m_detailLodWidth = srcDesc.detailLODWidth;

	outInstanceDesc->m_splineMultiplier = srcDesc.splineMultiplier;

	outInstanceDesc->m_assetType = srcDesc.assetType;
	outInstanceDesc->m_assetPriority = srcDesc.assetPriority;
	outInstanceDesc->m_assetGroup = srcDesc.assetGroup;

	outInstanceDesc->m_usePixelDensity = srcDesc.usePixelDensity;
	outInstanceDesc->m_useViewfrustrumCulling = srcDesc.useViewfrustrumCulling;
	outInstanceDesc->m_useBackfaceCulling = srcDesc.useBackfaceCulling;
	outInstanceDesc->m_backfaceCullingThreshold = srcDesc.backfaceCullingThreshold;

	outInstanceDesc->m_diffuseBlend = srcDesc.diffuseBlend;
	outInstanceDesc->m_hairNormalWeight = srcDesc.diffuseHairNormalWeight;
	outInstanceDesc->m_hairNormalBoneIndex = srcDesc.diffuseBoneIndex;

	memcpy(&outInstanceDesc->m_rootColor, &srcDesc.rootColor, sizeof(srcDesc.rootColor));
	memcpy(&outInstanceDesc->m_tipColor, &srcDesc.tipColor, sizeof(srcDesc.tipColor));
	memcpy(&outInstanceDesc->m_specularColor, &srcDesc.specularColor, sizeof(srcDesc.specularColor));
	outInstanceDesc->m_shadowSigma = srcDesc.shadowSigma;

	outInstanceDesc->m_glintStrength = srcDesc.glintStrength;
	outInstanceDesc->m_glintCount = srcDesc.glintCount;
	outInstanceDesc->m_glintExponent = srcDesc.glintExponent;

	outInstanceDesc->m_specularNoiseScale = srcDesc.specularNoiseScale;
	outInstanceDesc->m_specularEnvScale = srcDesc.specularEnvScale;
	outInstanceDesc->m_specularPrimary = srcDesc.specularPrimary;
	outInstanceDesc->m_specularPrimaryBreakup = srcDesc.specularPrimaryBreakup;
	outInstanceDesc->m_specularSecondary = srcDesc.specularSecondary;
	outInstanceDesc->m_specularSecondaryOffset = srcDesc.specularSecondaryOffset;
	outInstanceDesc->m_specularPowerPrimary = srcDesc.specularPowerPrimary;
	outInstanceDesc->m_specularPowerSecondary = srcDesc.specularPowerSecondary;
	outInstanceDesc->m_strandBlendScale = srcDesc.strandBlendScale;
	outInstanceDesc->m_strandBlendMode = srcDesc.strandBlendMode;
	outInstanceDesc->m_castShadows = srcDesc.castShadows;
	outInstanceDesc->m_receiveShadows = srcDesc.receiveShadows;
	outInstanceDesc->m_shadowDensityScale = srcDesc.shadowDensityScale;

	outInstanceDesc->m_backStopRadius = srcDesc.backStopRadius;
	outInstanceDesc->m_bendStiffness = srcDesc.bendStiffness;					
	outInstanceDesc->m_useCollision = srcDesc.useCollision;
	outInstanceDesc->m_useDynamicPin = srcDesc.useDynamicPin;
	outInstanceDesc->m_damping = srcDesc.damping;
	outInstanceDesc->m_friction = srcDesc.friction;
	outInstanceDesc->m_massScale = srcDesc.massScale;
	memcpy(&outInstanceDesc->m_gravityDir, &srcDesc.gravity, sizeof(srcDesc.gravity));
	outInstanceDesc->m_rootStiffness = srcDesc.rootStiffness;
	outInstanceDesc->m_pinStiffness = srcDesc.pinStiffness;
	outInstanceDesc->m_tipStiffness = srcDesc.tipStiffness;
	outInstanceDesc->m_inertiaScale = srcDesc.inertiaScale;
	outInstanceDesc->m_inertiaLimit = srcDesc.inertiaLimit;

	outInstanceDesc->m_simulate = srcDesc.simulate;
	outInstanceDesc->m_stiffness = srcDesc.stiffness;
	outInstanceDesc->m_stiffnessStrength = srcDesc.stiffnessStrength;
	outInstanceDesc->m_stiffnessDamping = srcDesc.stiffnessDamping;
	outInstanceDesc->m_interactionStiffness = srcDesc.interactionStiffness;

	memcpy(&outInstanceDesc->m_wind, &srcDesc.wind, sizeof(srcDesc.wind));
	outInstanceDesc->m_windNoise = srcDesc.windNoise;

	memcpy(&outInstanceDesc->m_stiffnessCurve, &srcDesc.stiffnessCurve, sizeof(srcDesc.stiffnessCurve));
	memcpy(&outInstanceDesc->m_stiffnessStrengthCurve, &srcDesc.stiffnessStrengthCurve, sizeof(srcDesc.stiffnessStrengthCurve));
	memcpy(&outInstanceDesc->m_stiffnessDampingCurve, &srcDesc.stiffnessDampingCurve, sizeof(srcDesc.stiffnessDampingCurve));
	memcpy(&outInstanceDesc->m_bendStiffnessCurve, &srcDesc.bendStiffnessCurve, sizeof(srcDesc.bendStiffnessCurve));
	memcpy(&outInstanceDesc->m_interactionStiffnessCurve, &srcDesc.interactionStiffnessCurve, sizeof(srcDesc.interactionStiffnessCurve));

	outInstanceDesc->m_visualizeBones = srcDesc.visualizeBones;
	outInstanceDesc->m_visualizeBoundingBox = srcDesc.visualizeBoundingBox;
	outInstanceDesc->m_visualizeCapsules = srcDesc.visualizeCapsules;
	outInstanceDesc->m_visualizeControlVertices = srcDesc.visualizeControlVertices;
	outInstanceDesc->m_visualizeCullSphere = srcDesc.visualizeCullSphere;
	outInstanceDesc->m_visualizeShadingNormalBone = srcDesc.visualizeDiffuseBone;
	outInstanceDesc->m_visualizeFrames = srcDesc.visualizeFrames;
	outInstanceDesc->m_visualizeGrowthMesh = srcDesc.visualizeGrowthMesh;
	outInstanceDesc->m_visualizeGuideHairs = srcDesc.visualizeGuideHairs;
	outInstanceDesc->m_visualizeHairInteractions= srcDesc.visualizeHairInteractions;
	outInstanceDesc->m_visualizeHairSkips= srcDesc.visualizeHairSkips;
	outInstanceDesc->m_visualizeLocalPos = srcDesc.visualizeLocalPos;
	outInstanceDesc->m_visualizePinConstraints = srcDesc.visualizePinConstraints;
	outInstanceDesc->m_visualizeShadingNormals = srcDesc.visualizeShadingNormals;
	outInstanceDesc->m_visualizeSkinnedGuideHairs = srcDesc.visualizeSkinnedGuideHairs;

	outInstanceDesc->m_enable = srcDesc.enable;
	outInstanceDesc->m_drawRenderHairs = srcDesc.drawRenderHairs;

	outInstanceDesc->m_rootAlphaFalloff = srcDesc.rootAlphaFalloff;
	outInstanceDesc->m_rootTipColorWeight = srcDesc.rootTipColorWeight;
	outInstanceDesc->m_rootTipColorFalloff = srcDesc.rootTipColorFalloff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/* static */NvResult ParamsUtil::load(ParamsContext* context, NvCo::ReadStream* streamIn, AssetHeaderInfo* outInfo, AssetDescriptor* outAssetDesc, 
			InstanceDescriptor* outMaterials, char* textureNames[])
{
	NvParameterized::XmlSerializer serializerXml(context->m_traits);
	NvParameterized::BinSerializer serializerBin(context->m_traits);

	NvHair::FileBufReadAdapter stream(streamIn);

	serializerXml.setAutoUpdate(false);
	serializerBin.setAutoUpdate(false);

	NvParameterized::Serializer::DeserializedData data;
	bool isUpdated;
	NvParameterized::Serializer::SerializeType serializeType = NvParameterized::Serializer::peekSerializeType(stream);
	NvParameterized::Serializer::ErrorType serError = NvParameterized::Serializer::ERROR_NONE;
	if (serializeType == NvParameterized::Serializer::NST_XML)
	{
		serError = serializerXml.deserialize(stream, data, isUpdated);
	}
	else if (serializeType == NvParameterized::Serializer::NST_BINARY)
	{
		serError = serializerBin.deserialize(stream, data, isUpdated);
	}
	else
	{
		NV_CO_LOG_ERROR("not supported serialization type");
		// Not supported type
		return NV_HAIR_RESULT(INVALID_FORMAT);
	}

	if (data.size() < 1)
	{
		return NV_HAIR_RESULT(INVALID_FORMAT);
	}

	bool isFileInfoExist = false;
	for (int dataIdx = 0; dataIdx < (int)data.size(); ++dataIdx)
	{
		NvParameterized::Interface* iface = data[dataIdx];
		if (::strcmp(iface->className(), HairWorksInfo::staticClassName()) == 0)
		{
			isFileInfoExist = true;
			if (outInfo)
			{
				HairWorksInfo* params = static_cast<HairWorksInfo*>(iface);

				// 1.1 or 1.2
				const int requiredVersions[] = { NV_HAIR_MAKE_VERSION(1, 1, 0), NV_HAIR_MAKE_VERSION(1, 2, 0) };
				HairWorksInfoNS::ParametersStruct& srcDesc = params->parameters();
				const Int apxVersion = VersionUtil::stringToVersion(srcDesc.fileVersion);

				Bool isVersionOk = false;
				for (int i = 0; i < NV_COUNT_OF(requiredVersions); i++)
				{
					if (apxVersion == requiredVersions[i])
					{
						isVersionOk = true;
						break;
					}
				}

				if (!isVersionOk)
				{
					Char buf[BuildInfo::VERSION_BUFFER_SIZE];
					// Version mismatch!
					NvCo::String requiredVersionBuf;
					for (int i = 0; i < NV_COUNT_OF(requiredVersions); i++)
					{
						int version = requiredVersions[i];
						if (i > 0)
						{
							requiredVersionBuf.concat(",");
						}
						const Int len = VersionUtil::versionToString(version, buf);
						requiredVersionBuf << NvCo::SubString(buf, len);
					}
					NV_CO_LOG_INFO_FORMAT("version mismatched. required:%s current:%s\n", requiredVersionBuf.getCstr(), (const char *)srcDesc.fileVersion);
					if (apxVersion > requiredVersions[NV_COUNT_OF(requiredVersions) - 1])
					{
						return NV_HAIR_RESULT(VERSION_MISMATCH);
					}
					NV_CO_LOG_INFO("trying to convert from old version\n");
				}

				//LoadParamString(context, iface, "fileVersion", versionBuf);
				outInfo->m_serialVersion = apxVersion;

				LoadParamString(context, iface, "toolVersion", outInfo->m_toolVersion);
				LoadParamString(context, iface, "sourcePath", outInfo->m_sourcePath);
				LoadParamString(context, iface, "authorName", outInfo->m_authorName);
				LoadParamString(context, iface, "lastModified", outInfo->m_lastModified);
			}
		}
		else if (::strcmp(iface->className(), HairSceneDescriptor::staticClassName()) == 0)
		{
			if (textureNames)
			{
				for (NvInt i = 0; i < NvHair::TextureType::COUNT_OF; i++)
					strcpy(textureNames[i], "");

				LoadParamString(context, iface, "densityTexture", textureNames[NvHair::TextureType::DENSITY]);
				LoadParamString(context, iface, "rootColorTexture", textureNames[NvHair::TextureType::ROOT_COLOR]);
				LoadParamString(context, iface, "tipColorTexture", textureNames[NvHair::TextureType::TIP_COLOR]);
				LoadParamString(context, iface, "widthTexture", textureNames[NvHair::TextureType::WIDTH]);
				LoadParamString(context, iface, "stiffnessTexture", textureNames[NvHair::TextureType::STIFFNESS]);
				LoadParamString(context, iface, "rootStiffnessTexture", textureNames[NvHair::TextureType::ROOT_STIFFNESS]);
				LoadParamString(context, iface, "clumpScaleTexture", textureNames[NvHair::TextureType::CLUMP_SCALE]);
				LoadParamString(context, iface, "clumpRoundnessTexture", textureNames[NvHair::TextureType::CLUMP_ROUNDNESS]);
				LoadParamString(context, iface, "waveScaletexture", textureNames[NvHair::TextureType::WAVE_SCALE]);
				LoadParamString(context, iface, "waveFreqTexture", textureNames[NvHair::TextureType::WAVE_FREQ]);
				LoadParamString(context, iface, "strandTexture", textureNames[NvHair::TextureType::STRAND]);
				LoadParamString(context, iface, "lengthTexture", textureNames[NvHair::TextureType::LENGTH]);
				LoadParamString(context, iface, "specularTexture", textureNames[NvHair::TextureType::SPECULAR]);
			}
		}
		else if (::strcmp(iface->className(), HairAssetDescriptor::staticClassName()) == 0)
		{
			// copy asset descriptor
			if (outAssetDesc)
			{
				if (HairAssetDescriptor::staticVersion() != iface->version())
				{
					NV_CO_LOG_ERROR("HairAssetDescriptor version mismatch!");
					// Not supported type
					return NV_HAIR_RESULT(INVALID_FORMAT);
				}

				HairAssetDescriptor* params = static_cast<HairAssetDescriptor*>(iface);
				HairAssetDescriptorNS::ParametersStruct& srcDesc = params->parameters();
				outAssetDesc->m_numGuideHairs = srcDesc.numGuideHairs;
				outAssetDesc->m_numVertices = srcDesc.numVertices;
				outAssetDesc->m_numFaces = srcDesc.numFaces;
				outAssetDesc->m_numBones = srcDesc.numBones;
				outAssetDesc->m_sceneUnit = srcDesc.sceneUnit;
				outAssetDesc->m_upAxis = (NvHair::AxisHint::Enum)srcDesc.upAxis;
				outAssetDesc->m_handedness = (NvHair::HandednessHint::Enum)srcDesc.handedness;

				LoadParamArrayT(context, iface, "vertices", (NvVec3**)&outAssetDesc->m_vertices);
				LoadParamArrayT(context, iface, "endIndices", (NvUInt32**)&outAssetDesc->m_endIndices);
				LoadParamArrayT(context, iface, "faceIndices", (NvUInt32**)&outAssetDesc->m_faceIndices);
				LoadParamArrayT(context, iface, "faceUVs", (NvVec2**)&outAssetDesc->m_faceUvs);
				LoadParamArrayT(context, iface, "boneIndices", (NvVec4**)&outAssetDesc->m_boneIndices);
				LoadParamArrayT(context, iface, "boneWeights", (NvVec4**)&outAssetDesc->m_boneWeights);
				// Load boneNameList or boneNames
				{
					bool boneNameListLoaded = false;
					NvParameterized::Handle handle(iface);
					if (iface->getParameterHandle("boneNameList", handle) == NvParameterized::ERROR_NONE)
					{
						int numBoneNameList = 0;
						handle.getArraySize(numBoneNameList);
						if (numBoneNameList > 0)
						{
							char** boneNameList = NvHair::allocatePodArray<char*>(numBoneNameList);
							if ((numBoneNameList > 0) && (handle.getParamStringArray(boneNameList, numBoneNameList) == NvParameterized::ERROR_NONE))
							{
								boneNameListLoaded = true;
								outAssetDesc->m_boneNames = NvHair::allocatePodArray<NvChar>(outAssetDesc->m_numBones * NV_HAIR_MAX_STRING);
								for (int i = 0; i < (int)srcDesc.numBones; i++)
								{
									char* boneName = outAssetDesc->m_boneNames + i * NV_HAIR_MAX_STRING;
									strcpy(boneName, boneNameList[i]);
								}
							}
							NvHair::deallocatePodArray(boneNameList);
						}
					}
					// Try boneNames if there is no boneNameList. Legacy format
					if (!boneNameListLoaded)
					{
						char *pBuffer = 0;
						if (LoadParamArrayT(context, iface, "boneNames", (NvUInt8**)&pBuffer))
						{
							char *ptr = pBuffer;

							outAssetDesc->m_boneNames = NvHair::allocatePodArray<NvChar>(outAssetDesc->m_numBones * NV_HAIR_MAX_STRING);
							for (int i = 0; i < (int)srcDesc.numBones; i++)
							{
								char* boneName = outAssetDesc->m_boneNames + i * NV_HAIR_MAX_STRING;
								strcpy(boneName, ptr);
								ptr += strlen(boneName) + 1;
							}
						}
					}
				}
				LoadParamArrayT(context, iface, "bindPoses", (NvMat44**)&outAssetDesc->m_bindPoses);
				LoadParamArrayT(context, iface, "boneParents", (NvInt32**)&outAssetDesc->m_boneParents);

				//NEEDS ERROR CHECK ON EXPECTED ARRAY SIZE!!
				// bone spheres and bone capsules

				bool validBoneSpheres = srcDesc.numBoneSpheres > 0;
				validBoneSpheres &= (srcDesc.boneSpheres.buf != NV_NULL);
				validBoneSpheres &= (srcDesc.boneSpheres.arraySizes[0] == (int)srcDesc.numBoneSpheres);

				outAssetDesc->m_numBoneSpheres = validBoneSpheres ? srcDesc.numBoneSpheres : 0;
				outAssetDesc->m_boneSpheres = NV_NULL;

				if (validBoneSpheres)
				{
					outAssetDesc->m_boneSpheres = NvHair::allocatePodArray<NvHair::BoneSphere>(srcDesc.numBoneSpheres);
					for (unsigned int boneIdx = 0; boneIdx < srcDesc.numBoneSpheres; ++boneIdx)
					{
						outAssetDesc->m_boneSpheres[boneIdx].m_boneSphereIndex = srcDesc.boneSpheres.buf[boneIdx].boneSphereIndex;
						memcpy(&outAssetDesc->m_boneSpheres[boneIdx].m_boneSphereLocalPos, &srcDesc.boneSpheres.buf[boneIdx].boneSphereLocalPos, sizeof(srcDesc.boneSpheres.buf[boneIdx].boneSphereLocalPos));
						outAssetDesc->m_boneSpheres[boneIdx].m_boneSphereRadius = srcDesc.boneSpheres.buf[boneIdx].boneSphereRadius;
					}
				}

				bool validBoneCapsules = srcDesc.numBoneCapsules > 0;
				validBoneCapsules &= (srcDesc.boneCapsuleIndices.buf != NV_NULL);
				validBoneCapsules &= (srcDesc.boneCapsuleIndices.arraySizes[0] == ( 2* (int)srcDesc.numBoneCapsules));

				outAssetDesc->m_numBoneCapsules = validBoneCapsules ? srcDesc.numBoneCapsules : 0;
				outAssetDesc->m_boneCapsuleIndices = NV_NULL;

				if (validBoneCapsules)
				{
					LoadParamArrayT(context, iface, "boneCapsuleIndices", (NvUInt32**)&outAssetDesc->m_boneCapsuleIndices);
				}

				bool validPinConstraint = srcDesc.numPinConstraints > 0;
				validPinConstraint &= (srcDesc.pinConstraints.buf != NV_NULL);
				validPinConstraint &= (srcDesc.pinConstraints.arraySizes[0] == (int)srcDesc.numPinConstraints);

				outAssetDesc->m_numPins = srcDesc.numPinConstraints;
				outAssetDesc->m_pins = NV_NULL;
				if (srcDesc.numPinConstraints)
				{
					assert(srcDesc.pinConstraints.arraySizes[0] == (int)(srcDesc.numPinConstraints));
					outAssetDesc->m_pins = NvHair::allocatePodArray<NvHair::Pin>(srcDesc.numPinConstraints);
					for (unsigned int idx = 0; idx < srcDesc.numPinConstraints; ++idx)
					{
						nvidia::parameterized::HairAssetDescriptorNS::Pin_Type& pinSrc = srcDesc.pinConstraints.buf[idx];
						NvHair::Pin& pinTarget = outAssetDesc->m_pins[idx];

						// set default values for pin as parameterized library doesn't set properties with default values described in schema file
						if (pinSrc.useDynamicPin == false
							&& pinSrc.useStiffnessPin == false
							&& pinSrc.doLra == false
							&& pinSrc.pinStiffness == 0.0
							&& pinSrc.influenceFallOff == 0.0
							&& pinSrc.influenceFallOffCurve.x == 0.0
							&& pinSrc.influenceFallOffCurve.y == 0.0
							&& pinSrc.influenceFallOffCurve.z == 0.0
							&& pinSrc.influenceFallOffCurve.w == 0.0)
						{
							pinSrc.useStiffnessPin = true;
							pinSrc.pinStiffness = 1.0;
							pinSrc.influenceFallOff = 1.0;
							pinSrc.influenceFallOffCurve.x = 1.0;
							pinSrc.influenceFallOffCurve.y = 1.0;
							pinSrc.influenceFallOffCurve.z = 1.0;
							pinSrc.influenceFallOffCurve.w = 1.0;
						}

						pinTarget.m_boneIndex = pinSrc.boneSphereIndex;
						memcpy(&pinTarget.m_localPos, &pinSrc.boneSphereLocalPos, sizeof(srcDesc.boneSpheres.buf[idx].boneSphereLocalPos));
						pinTarget.m_radius = pinSrc.boneSphereRadius;
						pinTarget.m_pinStiffness = pinSrc.pinStiffness;
						pinTarget.m_influenceFallOff = pinSrc.influenceFallOff;
						pinTarget.m_useDynamicPin = pinSrc.useDynamicPin;
						pinTarget.m_doLra = pinSrc.doLra;
						pinTarget.m_useStiffnessPin = pinSrc.useStiffnessPin;
						pinTarget.m_selected = false;
						memcpy(&pinTarget.m_influenceFallOffCurve, &pinSrc.influenceFallOffCurve, sizeof(pinSrc.influenceFallOffCurve));
					}
				}
			}
		}
		else if (::strcmp(iface->className(), HairInstanceDescriptor::staticClassName()) == 0)
		{
			// copy instance descriptor
			if (outMaterials)
			{
				if (HairInstanceDescriptor::staticVersion() != iface->version())
				{
					NV_CO_LOG_ERROR("HairInstanceDescriptor version mismatch!");
					// Not supported type
					return NV_HAIR_RESULT(INVALID_FORMAT);
				}

				HairInstanceDescriptor* params = static_cast<HairInstanceDescriptor*>(iface);
				HairInstanceDescriptorNS::ParametersStruct& srcDesc = params->parameters();

				// copy default material
				//GFDSK_HairReadInstanceDescriptor<HairInstanceDescriptorNS::ParametersStruct>(srcDesc, &outMaterials[0]);

				// check if there are additional materials
				unsigned int numMaterials = 0;
				NvParameterized::Handle handle(iface);
				if (iface->getParameterHandle("materials", handle) == NvParameterized::ERROR_NONE)
					numMaterials = srcDesc.materials.arraySizes[0];

				// copy additional materials
				if (numMaterials > 0)
				{
					for (unsigned int idx = 0; idx < numMaterials; ++idx)
					{
						HairInstanceDescriptorNS::Material_Type& matDesc = srcDesc.materials.buf[idx];
						NvHair::InstanceDescriptor* pTargetInstanceDesc = &outMaterials[idx];

						_readInstanceDescriptor<HairInstanceDescriptorNS::Material_Type>(matDesc, pTargetInstanceDesc);
					}
				}

				// if there is not enough material, copy it from default (first)
				for (unsigned int idx = numMaterials; idx < 4; ++idx)
					memcpy(&outMaterials[idx+1], &outMaterials[0], sizeof(HairInstanceDescriptor));
			}
		}
	}
	if (!isFileInfoExist && outInfo)
	{
		NvHair::AssetHeaderInfo defaultInfo;
		memcpy(outInfo, &defaultInfo, sizeof(*outInfo));
	}
	return NV_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* static */Result ParamsUtil::save(ParamsContext* context, NvCo::WriteStream* streamIn, ESerializeFormat	format, const AssetHeaderInfo* info,
	const AssetDescriptor* assetDesc, const InstanceDescriptor* materials, int numMaterials, const char* textureNames[])
{
	if (context == NV_NULL) 
	{
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	NvParameterized::XmlSerializer serializerXml(context->m_traits);
	NvParameterized::BinSerializer serializerBin(context->m_traits);
	NvParameterized::Serializer* serializer = NV_NULL;
	if (format == NvHair::SerializeFormat::XML)
	{
		serializer = &serializerXml;
	}
	else if (format == NvHair::SerializeFormat::BINARY)
	{
		serializer = &serializerBin;
	}
	else
	{
		// Couldn't find any suitable serializer for this extension
		NV_CO_LOG_ERROR("not supported stream type");
		return NV_HAIR_RESULT(OPEN_FAILED);
	}
		
	NvHair::FileBufWriteAdapter stream(streamIn);

	//NvFileBuf* stream = new NvFileBufferBase(filePath, NvFileBuf::OPEN_WRITE_ONLY);
	if (!stream.isOpen())
	{
		// file open error
		NV_CO_LOG_ERROR("file save error");
		return NV_HAIR_RESULT(OPEN_FAILED);
	}
	NvParameterized::Traits* traits = context->m_traits;
	int numObjects = 0;
	const int kMaxObjects = 4;
	NvParameterized::Interface* objects[kMaxObjects];

	// copy header info
	if (info)
	{
		HairWorksInfo * params = new HairWorksInfo(traits);
		objects[numObjects++] = params;
		NvParameterized::Interface* iface = static_cast<NvParameterized::Interface*>(params);

		Int serialVersion = info->m_serialVersion;
		if (serialVersion < 0)
		{
			serialVersion = NV_HAIR_SERIAL_VERSION;
		}

		if (serialVersion != NV_HAIR_SERIAL_VERSION)
		{
			// Can only save in current format
			NV_CO_LOG_ERROR("Can only save in current serial format");
			return NV_FAIL;
		}

		Char serialVersionBuf[BuildInfo::VERSION_BUFFER_SIZE];
		VersionUtil::versionToString(serialVersion, serialVersionBuf);

		SaveParamString(context, iface, "fileVersion", serialVersionBuf);
		SaveParamString(context, iface, "toolVersion", info->m_toolVersion);
		SaveParamString(context, iface, "sourcePath", info->m_sourcePath);
		SaveParamString(context, iface, "authorName", info->m_authorName);
		SaveParamString(context, iface, "lastModified", info->m_lastModified); 
	}

	// copy scene descriptor
	if (textureNames)
	{
		HairSceneDescriptor* params = new HairSceneDescriptor(traits);
		objects[numObjects++] = params;
		NvParameterized::Interface* iface = static_cast<NvParameterized::Interface*>(params);
		SaveParamString(context, iface, "densityTexture", textureNames[NvHair::TextureType::DENSITY]);
		SaveParamString(context, iface, "rootColorTexture", textureNames[NvHair::TextureType::ROOT_COLOR]);
		SaveParamString(context, iface, "tipColorTexture", textureNames[NvHair::TextureType::TIP_COLOR]);
		SaveParamString(context, iface, "widthTexture", textureNames[NvHair::TextureType::WIDTH]);
		//SaveParamString(context, iface, "rootWidthTexture", textureNames[NvHair::TextureType::ROOT_WIDTH]);
		//SaveParamString(context, iface, "tipWidthTexture", textureNames[NvHair::TextureType::TIP_WIDTH]);
		SaveParamString(context, iface, "stiffnessTexture", textureNames[NvHair::TextureType::STIFFNESS]);
		SaveParamString(context, iface, "rootStiffnessTexture", textureNames[NvHair::TextureType::ROOT_STIFFNESS]);
		SaveParamString(context, iface, "clumpScaleTexture", textureNames[NvHair::TextureType::CLUMP_SCALE]);
//		SaveParamString(context, iface, "clumpNoiseTexture", textureNames[NvHair::TextureType::CLUMP_NOISE]);
		SaveParamString(context, iface, "clumpRoundnessTexture", textureNames[NvHair::TextureType::CLUMP_ROUNDNESS]);
		SaveParamString(context, iface, "waveScaletexture", textureNames[NvHair::TextureType::WAVE_SCALE]);
		SaveParamString(context, iface, "waveFreqTexture", textureNames[NvHair::TextureType::WAVE_FREQ]);
		SaveParamString(context, iface, "strandTexture", textureNames[NvHair::TextureType::STRAND]);
		SaveParamString(context, iface, "lengthTexture", textureNames[NvHair::TextureType::LENGTH]);
		SaveParamString(context, iface, "specularTexture", textureNames[NvHair::TextureType::SPECULAR]);
	}

	// copy asset descriptor
	if (assetDesc)
	{
		HairAssetDescriptor* params = new HairAssetDescriptor(traits);
		NvParameterized::Interface* iface = static_cast<NvParameterized::Interface*>(params);
		objects[numObjects++] = params;
		HairAssetDescriptorNS::ParametersStruct& targetDesc = params->parameters();

		targetDesc.numGuideHairs = assetDesc->m_numGuideHairs;
		targetDesc.numVertices = assetDesc->m_numVertices;
		targetDesc.numFaces = assetDesc->m_numFaces;
		targetDesc.numBones = assetDesc->m_numBones;
		targetDesc.sceneUnit = assetDesc->m_sceneUnit;
		targetDesc.upAxis = uint32_t(assetDesc->m_upAxis);
		targetDesc.handedness = uint32_t(assetDesc->m_handedness);

		SaveParamArrayT(context, iface, "vertices", (NvVec3*)assetDesc->m_vertices, assetDesc->m_numVertices);
		SaveParamArrayT(context, iface, "endIndices", assetDesc->m_endIndices, assetDesc->m_numGuideHairs);
		SaveParamArrayT(context, iface, "faceIndices", assetDesc->m_faceIndices, assetDesc->m_numFaces * 3);
		SaveParamArrayT(context, iface, "faceUVs", (NvVec2*)assetDesc->m_faceUvs, assetDesc->m_numFaces * 3);
		SaveParamArrayT(context, iface, "boneIndices", (NvVec4*)assetDesc->m_boneIndices, assetDesc->m_numGuideHairs);
		SaveParamArrayT(context, iface, "boneWeights", (NvVec4*)assetDesc->m_boneWeights, assetDesc->m_numGuideHairs);
		{
			char* boneNames = NvHair::allocatePodArray<char>(assetDesc->m_numBones * NV_HAIR_MAX_STRING);
			char* ptr = boneNames;
			for (int i = 0; i < (int)assetDesc->m_numBones; i++)
			{
				char* boneName = assetDesc->m_boneNames + i * NV_HAIR_MAX_STRING;
				strcpy(ptr, boneName);
				ptr += strlen(boneName) + 1;
			}
			int bufferSize = (int)(ptr - boneNames);
			SaveParamArrayT(context, iface, "boneNames", (NvUInt8*)boneNames, bufferSize);
			// Save new format for bonenames as well
			{
				NvParameterized::Handle handle(iface);
				if (iface->getParameterHandle("boneNameList", handle) == NvParameterized::ERROR_NONE)
				{
					const char** boneNameList = NvHair::allocatePodArray<const char*>(assetDesc->m_numBones);
					for (int idx = 0; idx < (int)assetDesc->m_numBones; ++idx)
					{
						boneNameList[idx] = &assetDesc->m_boneNames[idx*NV_HAIR_MAX_STRING];
					}
					handle.resizeArray(assetDesc->m_numBones);
					handle.setParamStringArray(boneNameList, assetDesc->m_numBones);
					NvHair::deallocatePodArray(boneNameList);
				}
			}
			NvHair::deallocatePodArray(boneNames);
		}

		if (assetDesc->m_bindPoses)
		{
			SaveParamArrayT(context, iface, "bindPoses", (NvMat44*)assetDesc->m_bindPoses, assetDesc->m_numBones);
		}
		if (assetDesc->m_boneParents)
		{
			SaveParamArrayT(context, iface, "boneParents", (NvInt32*)assetDesc->m_boneParents, assetDesc->m_numBones);
		}

		targetDesc.numBoneSpheres = assetDesc->m_numBoneSpheres;
		targetDesc.numBoneCapsules = assetDesc->m_numBoneCapsules;

		// bone spheres and bone capsules
		if (assetDesc->m_numBoneSpheres)
		{
			NvParameterized::Handle handle(iface);
			if (iface->getParameterHandle("boneSpheres", handle) == NvParameterized::ERROR_NONE)
			{
				handle.resizeArray(assetDesc->m_numBoneSpheres);

				int numBoneSpheres = assetDesc->m_numBoneSpheres;
				for (int idx = 0; idx < numBoneSpheres; ++idx)
				{
					targetDesc.boneSpheres.buf[idx].boneSphereIndex = assetDesc->m_boneSpheres[idx].m_boneSphereIndex;
					memcpy(&targetDesc.boneSpheres.buf[idx].boneSphereLocalPos, &assetDesc->m_boneSpheres[idx].m_boneSphereLocalPos, sizeof(assetDesc->m_boneSpheres[idx].m_boneSphereLocalPos));
					targetDesc.boneSpheres.buf[idx].boneSphereRadius = assetDesc->m_boneSpheres[idx].m_boneSphereRadius;
				}
			}
			assert(targetDesc.boneSpheres.arraySizes[0] == (int)assetDesc->m_numBoneSpheres);

		}
		if (assetDesc->m_numBoneCapsules)
		{
			int numBoneCapsuleIndices = assetDesc->m_numBoneCapsules*2;
			SaveParamArrayT(context, iface, "boneCapsuleIndices", assetDesc->m_boneCapsuleIndices, numBoneCapsuleIndices);
		}

		targetDesc.numPinConstraints = assetDesc->m_numPins;
		if (assetDesc->m_numPins)
		{
			NvParameterized::Handle handle(iface);
			if (iface->getParameterHandle("pinConstraints", handle) == NvParameterized::ERROR_NONE)
			{
				handle.resizeArray(assetDesc->m_numPins);

				int numPinConstraints = assetDesc->m_numPins;
				for (int idx = 0; idx < numPinConstraints; ++idx)
				{
					const NvHair::Pin& pinSrc = assetDesc->m_pins[idx];
					nvidia::parameterized::HairAssetDescriptorNS::Pin_Type& pinTarget = targetDesc.pinConstraints.buf[idx];

					pinTarget.boneSphereIndex = pinSrc.m_boneIndex;
					memcpy(&pinTarget.boneSphereLocalPos, &pinSrc.m_localPos,
						sizeof(pinSrc.m_localPos));
					pinTarget.boneSphereRadius = pinSrc.m_radius;
					pinTarget.pinStiffness = pinSrc.m_pinStiffness;
					pinTarget.influenceFallOff = pinSrc.m_influenceFallOff;
					pinTarget.useDynamicPin = pinSrc.m_useDynamicPin;
					pinTarget.doLra = pinSrc.m_doLra;
					pinTarget.useStiffnessPin = pinSrc.m_useStiffnessPin;
					memcpy(&pinTarget.influenceFallOffCurve, &pinSrc.m_influenceFallOffCurve,
						sizeof(pinSrc.m_influenceFallOffCurve));
				}
			}
			assert(targetDesc.pinConstraints.arraySizes[0] == (int)assetDesc->m_numPins);
		}
	}
	else
	{
		HairAssetDescriptor* params = new HairAssetDescriptor(traits);
		objects[numObjects++] = params;
	}

	// copy instance descriptor
	if ((numMaterials > 0) && (materials))
	{
		HairInstanceDescriptor* params = new HairInstanceDescriptor(traits);
		objects[numObjects++] = params;

		HairInstanceDescriptorNS::ParametersStruct& targetDesc = params->parameters();
//		GFSDK_HairWriteInstanceDescriptor<HairInstanceDescriptorNS::ParametersStruct>(targetDesc, &materials[0]);

		{
			NvParameterized::Interface* iface = static_cast<NvParameterized::Interface*>(params);
			NvParameterized::Handle handle(iface);
			if (iface->getParameterHandle("materials", handle) == NvParameterized::ERROR_NONE)
			{
				int sizeArray = numMaterials;
				handle.resizeArray(sizeArray);
				for (int idx = 0; idx < sizeArray; ++idx)
				{
					HairInstanceDescriptorNS::Material_Type& matDesc = targetDesc.materials.buf[idx];
					const NvHair::InstanceDescriptor* pInstanceDesc = &materials[idx];
					_writeInstanceDescriptor<HairInstanceDescriptorNS::Material_Type>(matDesc, pInstanceDesc);
				}
			}
		}
		
	}
	else
	{
		HairInstanceDescriptor* params = new HairInstanceDescriptor(traits);
		objects[numObjects++] = params;
	}

	NV_ASSERT(numObjects <= kMaxObjects);
	NvParameterized::Serializer::ErrorType serError = NvParameterized::Serializer::ERROR_NONE;
	bool isUpdate = false;
	serError = serializer->serialize(stream, (const NvParameterized::Interface**)&objects[0], numObjects, isUpdate);
	for (int idx = 0; idx < numObjects; ++idx)
	{
		delete objects[idx];
	}

	return NV_OK;
}

/* static */ParamsContext* ParamsUtil::init(nvidia::NvAllocatorCallback* allocatorCallback, nvidia::NvErrorCallback* errorCallback)
{
	static DefaultAllocator sDefaultAllocator;
	static DefaultErrorCallback sDefaultErrorCallback;
	ParamsContext* context = new ParamsContext;

	{
		allocatorCallback = allocatorCallback ? allocatorCallback : &sDefaultAllocator;
		errorCallback = errorCallback ? errorCallback : &sDefaultErrorCallback;

		nvidia::initializeSharedFoundation(NV_FOUNDATION_VERSION, *allocatorCallback, *errorCallback);
	}
	context->m_traits = NvParameterized::createTraits();
	context->m_infoFactory = new HairWorksInfoFactory;
	context->m_sceneDescriptorFactory = new HairSceneDescriptorFactory;
	context->m_assetDescriptorFactory = new HairAssetDescriptorFactory;
	context->m_instanceDescriptorFactory = new HairInstanceDescriptorFactory;
	context->m_traits->registerFactory(*context->m_infoFactory);
	context->m_traits->registerFactory(*context->m_sceneDescriptorFactory);
	context->m_traits->registerFactory(*context->m_assetDescriptorFactory);
	context->m_traits->registerFactory(*context->m_instanceDescriptorFactory);

	// register legacy factories
	context->m_assetDescriptor10Factory = new legacy::ver1p0::HairAssetDescriptorFactory;
	context->m_instanceDescriptor10Factory = new legacy::ver1p0::HairInstanceDescriptorFactory;
	context->m_traits->registerFactory(*context->m_assetDescriptor10Factory);
	context->m_traits->registerFactory(*context->m_instanceDescriptor10Factory);

#if 0
	// register HairAssetDescriptorConversion_1p0
	HairAssetDescriptorConversion_1p0 * m_HairAssetDescriptorConversion_1p0 =
		HairAssetDescriptorConversion_1p0::Create(context->mTraits);
	context->mTraits->registerConversion(context->mHairAssetDescriptorFactory->getClassName(),
		legacy::ver1p0::HairAssetDescriptor::staticVersion(),
		HairAssetDescriptor::staticVersion(),
		*m_HairAssetDescriptorConversion_1p0);

	// register HairInstanceDescriptorConversion_1p0
	HairInstanceDescriptorConversion_1p0 * m_HairInstanceDescriptorConversion_1p0 =
		HairInstanceDescriptorConversion_1p0::Create(context->mTraits);
	context->mTraits->registerConversion(context->mHairInstanceDescriptorFactory->getClassName(),
		legacy::ver1p0::HairInstanceDescriptor::staticVersion(),
		HairInstanceDescriptor::staticVersion(),
		*m_HairInstanceDescriptorConversion_1p0);
#endif

	return context;
}

void ParamsUtil::destroy(ParamsContext* context)
{
	context->m_traits->release();
	//context->mFoundation->release();
	delete context->m_infoFactory;
	delete context->m_sceneDescriptorFactory;
	delete context->m_assetDescriptorFactory;
	delete context->m_instanceDescriptorFactory;

	// delete legacy factories
	delete context->m_assetDescriptor10Factory;
	delete context->m_instanceDescriptor10Factory;

	delete context;

	nvidia::terminateSharedFoundation();
}

} //namespace HairWorks 
} // namespace nvidia

