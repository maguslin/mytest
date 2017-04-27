/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairSdkImpl.h"

#include <Nv/HairWorks/Internal/Util/NvHairAssetDescriptorUtil.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/Internal/Util/NvHairVersionUtil.h>

#if defined(USE_APEX_HAIR_PARAMS)
#	include <Nv/HairWorks/Internal/Serialize/NvHairParamsUtil.h>

#	include <Nv/Common/Platform/StdC/NvCoStdCFileReadStream.h>
#	include <Nv/Common/Platform/StdC/NvCoStdCFileWriteStream.h>
#	include <Nv/Common/NvCoMemoryReadStream.h>
#	include <Nv/Common/NvCoMemoryWriteStream.h>
#endif

// Foundation types
#include <NvErrorCallback.h>
#include <NvAllocatorCallback.h>


#include <cstdio>

namespace nvidia {

void HandleError(Result res)
{
	NV_UNUSED(res);
	__debugbreak();
}

// If this fails the CacheSettings needs to have it's capacity expanded
NV_COMPILE_TIME_ASSERT(NvHair::TextureType::COUNT_OF <= 16);

} // namespace nvidia

extern const char* NvHair_buildString;

namespace nvidia { 
namespace HairWorks { 


// Adaptor from NvCo::MemoryAllocator to nvidia::NvAllocatorCallback
class AllocatorAdaptor : public nvidia::NvAllocatorCallback
{
public:
	virtual void* allocate(size_t size, const char* /*typeName*/, const char* /*filename*/, int /*line*/)
	{
		return NvCo::MemoryAllocator::getInstance()->simpleAllocate(size);
	}
	virtual void deallocate(void* ptr)
	{
		NvCo::MemoryAllocator::getInstance()->simpleDeallocate(ptr);
	}
};

// Adaptor from NvCo::Logger to nvidia::NvErrorCallback
class ErrorHandlerAdaptor : public nvidia::NvErrorCallback
{
public:
	virtual ~ErrorHandlerAdaptor() {}

	virtual void reportError(nvidia::NvErrorCode::Enum code, const char* message, const char* file, int line)
	{
		NvCo::Logger* logger = NvCo::Logger::getInstance();
		typedef nvidia::NvErrorCode Code;

		if (logger)
		{
			NvCo::ELogSeverity severity;
			switch (code)
			{
				case Code::eNO_ERROR:			severity = NvCo::LogSeverity::INFO; break;
				case Code::eDEBUG_INFO:			severity = NvCo::LogSeverity::DEBUG_INFO; break;
				case Code::ePERF_WARNING:
				case Code::eDEBUG_WARNING:		severity = NvCo::LogSeverity::WARNING; break;
				case Code::eINVALID_OPERATION:
				case Code::eINTERNAL_ERROR:
				case Code::eINVALID_PARAMETER:	severity = NvCo::LogSeverity::NON_FATAL_ERROR; break;
				default:
				case Code::eOUT_OF_MEMORY:
				case Code::eABORT:				severity = NvCo::LogSeverity::FATAL_ERROR; break;
			}
			logger->log(severity, message, NV_FUNCTION_NAME, file, line);
		}
	}
};

NV_COMPILE_TIME_ASSERT(sizeof(NvCo::HandleMapHandle) == sizeof(AsyncHandle));

AllocatorAdaptor g_allocatorAdaptor;
ErrorHandlerAdaptor g_errorHandlerAdaptor;

SdkImpl::SdkImpl(ApiGlobal* apiGlobal, Int backDoorMode):
	m_apiGlobal(apiGlobal),
	m_backDoorMode(backDoorMode)
{
	// There are no assets initially
	m_numAssets = 0;
	m_numInstances = 0;
	
#if defined(USE_APEX_HAIR_PARAMS)
	m_paramsContext = ParamsUtil::init(&g_allocatorAdaptor, &g_errorHandlerAdaptor);
#endif

	m_frameViewInfo.init();

	{
		BuildInfo& info = m_buildInfo;

		info.m_buildString = NvHair_buildString;
		info.m_releaseVersion = NV_HAIR_RELEASE_VERSION;
		info.m_version = NV_HAIR_VERSION;
		info.m_serialVersion = NV_HAIR_SERIAL_VERSION;
		info.m_stringToVersionFunc = &VersionUtil::stringToVersion;
		info.m_versionToStringFunc = &VersionUtil::versionToString;
	}

	NV_CO_LOG_INFO("HairWorks SDK initialized OK");
}

SdkImpl::~SdkImpl()
{
#if defined(USE_APEX_HAIR_PARAMS)
	if (m_paramsContext)
		ParamsUtil::destroy(m_paramsContext);
#endif

	// Delete instances
	for (IndexT i = 0; i < m_instances.getSize(); i++)
	{
		Instance* inst = m_instances[i];
		if (inst)
		{
			delete inst;
		}
	}

	// Delete assets
	for (NvInt i = 0; i < m_assets.getSize(); i++)
	{
		Asset* asset = m_assets[i];
		if (asset)
		{
			delete asset;
		}
	}
}

void SdkImpl::release()
{
	delete this;
	NV_CO_LOG_INFO("HairWorks SDK released");
}

const BuildInfo& SdkImpl::getBuildInfo()
{
	return m_buildInfo;
}

Result SdkImpl::setBoneRemapping( AssetId assetId, const Char*const* boneNames, Int numNewBones )
{
	if (assetId == ASSET_ID_NULL)
	{
		NV_CO_LOG_ERROR("Invalid asset ID.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (assetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("Invalid asset ID.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = m_assets[assetId];
	if (!asset)
	{
		NV_CO_LOG_ERROR("specified asset does not exist.");
		return NV_FAIL;
	}
	// Just do it inplace
	NvResult res = AssetDescriptorUtil::remapBones(asset->getWriteDesc(), boneNames, numNewBones);
	asset->updateDescChanged(m_apiGlobal);
	return res;
}

IndexT SdkImpl::_getNewAssetSlot()
{
	IndexT index = m_assets.indexOf(NV_NULL);
	if (index >= 0)
	{
		return index;
	}
	index = m_assets.getSize();
	m_assets.pushBack(NV_NULL);
	return index;
}

ESerializeFormat SdkImpl::getSerializeFormatFromExtension(const char* ext)
{
	if (ext)
	{
		if (*ext == '.') 
		{
			ext++;
		}	
		if (!_stricmp(ext, "apx"))
		{
			return SerializeFormat::XML;
		}
		else if (!_stricmp(ext, "apb"))
		{
			return SerializeFormat::BINARY;
		}
	}
	return SerializeFormat::UNKNOWN;
}

static void _getTextureNames(const Char*const* textureNamesIn, const AssetDescriptor* assetDesc, const Char* textureNamesOut[Int(TextureType::COUNT_OF)])
{
	if (textureNamesIn)
	{
		for (Int i = 0; i < Int(TextureType::COUNT_OF); i++)
			textureNamesOut[i] = textureNamesIn[i];
	}
	else if (assetDesc->m_textureNames)
	{
		for (Int i = 0; i < Int(TextureType::COUNT_OF); i++)
			textureNamesOut[i] = assetDesc->m_textureNames + i * NV_HAIR_MAX_STRING;
	}
	else
	{
		for (Int i = 0; i < Int(TextureType::COUNT_OF); i++)
			textureNamesOut[i] = NV_NULL;
	}
}

Result SdkImpl::loadAsset(NvCo::ReadStream* stream, AssetId& assetIdOut, AssetHeaderInfo* infoOut, const ConversionSettings* settings)
{
	assetIdOut = ASSET_ID_NULL;
	if (NV_NULL == m_paramsContext)
	{
		NV_CO_LOG_ERROR("Hair parameter context does not exist.");
		return NV_FAIL;
	}

	Result res = NV_OK;
	IndexT newSlot = _getNewAssetSlot();
	if (newSlot < 0)
	{
		NV_CO_LOG_ERROR("No asset slot was found. check if there are too many assets.");
		return NV_FAIL;
	}

	// allocate data containers
	AssetHeaderInfo info;
	NvCo::UniquePtr<Asset> asset(new Asset);

	{
		AssetDescriptor assetDescriptor;

		// prepare texture file name array
		assetDescriptor.m_textureNames = allocatePodArray<Char>(NV_HAIR_MAX_STRING * Int(TextureType::COUNT_OF));
		char* textureNames[Int(TextureType::COUNT_OF)];
		for (int i = 0; i < Int(TextureType::COUNT_OF); i++)
		{
			textureNames[i] = assetDescriptor.m_textureNames + i * NV_HAIR_MAX_STRING;
			*textureNames[i] = '\0';
		}

		// do actual load
		res = ParamsUtil::load(m_paramsContext, stream, &info, &assetDescriptor, asset->m_materials, textureNames);
		if (NV_FAILED(res))
		{
			NV_CO_LOG_ERROR("Hair descriptor failed to load. Please check version if .apx was loaded.");
			return res;
		}
		if (infoOut)
		{
			*infoOut = info;
		}

		// Allocate in asset
		asset->getWriteDesc() = assetDescriptor;
		// Free the read in one
		AssetDescriptorUtil::deallocate(assetDescriptor);
	}

	res = AssetDescriptorUtil::check(asset->getDesc(), NvCo::Logger::getInstance());
	if (NV_FAILED(res))
	{
		NV_CO_LOG_ERROR("Hair asset descriptor is not valid.");
		return res;
	}
	// convert unit
	const float sceneUnit = settings ? settings->m_targetSceneUnit : 0.0f;
	res = AssetDescriptorUtil::convertUnits(asset->getWriteDesc(), sceneUnit);
	if (NV_FAILED(res))
	{
		NV_CO_LOG_ERROR("Hair unit conversion failed.");
		return res;
	}
	// convert coords
	if (settings)
	{
		AssetDescriptorUtil::convert(*settings, asset->getWriteDesc());
	}

	res = asset->updateDescChanged(m_apiGlobal);
	if (NV_FAILED(res)) return res;

	// register this asset and its default instance descriptor
	assetIdOut = AssetId(newSlot);
	m_assets[newSlot] = asset.detach();
	m_numAssets++;
	return NV_OK;
}

Result SdkImpl::saveAsset(NvCo::WriteStream* stream, ESerializeFormat format, AssetId assetId, const InstanceDescriptor* descriptor, const AssetHeaderInfo* info, const Char*const* textureNamesIn)
{
	NV_HAIR_THREAD_LOCK
	if (assetId == ASSET_ID_NULL || assetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("SaveHairAssetToFile() - asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = m_assets[assetId];
	if (!asset)
	{
		NV_CO_LOG_ERROR("SaveHairAssetToFile() - asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	const AssetDescriptor& assetDesc = asset->getDesc();
	const InstanceDescriptor* instanceDesc = descriptor ? descriptor : &asset->m_materials[0];

	Int numMaterials = Asset::MAX_MATERIALS;

	if (instanceDesc == NV_NULL)
	{
		NV_CO_LOG_ERROR("Instance descriptor is null.");
		return NV_FAIL;
	}

	const char* textureNames[Int(TextureType::COUNT_OF)];
	_getTextureNames(textureNamesIn, &assetDesc, textureNames);

#if defined(USE_APEX_HAIR_PARAMS)
	if (NV_NULL == m_paramsContext)
	{
		NV_CO_LOG_ERROR("Hair parameter context does not exist.");
		return NV_FAIL;
	}
	return ParamsUtil::save(m_paramsContext, stream, format, info, &assetDesc, instanceDesc, numMaterials, textureNames);
#else
	return NV_FAIL;
#endif
}

Result SdkImpl::saveInstance(NvCo::WriteStream* stream, ESerializeFormat format, InstanceId instanceId, const AssetHeaderInfo* info, const Char*const* textureNamesIn)
{
	NV_HAIR_THREAD_LOCK

	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("No hair resource for the specified instance id is found");
		return NV_FAIL;
	}

	const Asset* asset = inst->m_asset;
	const AssetDescriptor& assetDesc = asset->getDesc();

	const char* textureNames[Int(TextureType::COUNT_OF)];
	_getTextureNames(textureNamesIn, &assetDesc, textureNames);

#if defined(USE_APEX_HAIR_PARAMS)
	if (NV_NULL == m_paramsContext)
	{
		NV_CO_LOG_ERROR("No parameter context exists.");
		return NV_FAIL;
	}
	return ParamsUtil::save(m_paramsContext, stream, format, info, &assetDesc, inst->m_materials, Asset::MAX_MATERIALS, textureNames);
#else
	return NV_FAIL;
#endif
}


Result SdkImpl::copyAsset(AssetId fromAssetId, AssetId toAssetId, const AssetCopySettings& settings	)
{
	NV_HAIR_THREAD_LOCK
	if (fromAssetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("fromAssetID is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (toAssetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("toAssetId is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (toAssetId == fromAssetId)
	{
		NV_CO_LOG_ERROR("Cannot copy asset to itself.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* fromAsset = m_assets[fromAssetId];
	Asset* toAsset = m_assets[toAssetId];

	if (!fromAsset || !toAsset)
	{
		NV_CO_LOG_ERROR("Asset descriptor does not exist for the specified asset id.");
		return NV_FAIL;
	}
	toAsset->getWriteDesc().setCopy(fromAsset->getDesc(), settings);
	toAsset->updateDescChanged(m_apiGlobal);
	return NV_OK;
}

Result SdkImpl::createAsset(const AssetDescriptor& assetDescIn, AssetId& assetIdOut)
{
	NV_HAIR_THREAD_LOCK

	IndexT newSlot = _getNewAssetSlot();
	if (newSlot < 0)
	{
		NV_CO_LOG_ERROR("Could not find new asset slot.  Too many assets were already created.");
		return NV_FAIL;
	}
	if (NV_FAILED(AssetDescriptorUtil::check(assetDescIn, NvCo::Logger::getInstance())))
	{
		NV_CO_LOG_ERROR("Asset descriptor is invalid.");
		return NV_FAIL;
	}

	assetIdOut = AssetId(newSlot);

	Asset* asset = new Asset;
	asset->getWriteDesc() = assetDescIn;
	asset->updateDescChanged(m_apiGlobal);

	m_assets[newSlot] = asset;
	m_numAssets++;
	return NV_OK;
}

Void SdkImpl::freeAsset(AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	if (assetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return;
	}
	Asset* asset = m_assets[assetId];
	if (asset)
	{
		if (asset->m_numInstances > 0)
		{
			NV_CO_LOG_ERROR("Asset is being referenced by instance/s - cannot free");
			return;
		}

		m_assets[assetId] = NV_NULL;
		delete asset;
		m_numAssets--;
	}
}

Result SdkImpl::resampleGuideHairs(AssetId assetId, Int numTargetPointsPerHair)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	asset->getWriteDesc().resampleGuideHairs(numTargetPointsPerHair);
	asset->updateDescChanged(m_apiGlobal);
	return NV_OK;
}

Int SdkImpl::getNumBones(AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return 0;
	}
	return asset->getDesc().m_numBones;
}

Result SdkImpl::getBoneName(AssetId assetId, Int boneId, Char* boneNameOut)
{
	NV_HAIR_THREAD_LOCK
	if (!boneNameOut)
	{
		NV_CO_LOG_ERROR("boneNamePtr is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& desc = asset->getDesc();
	if (desc.m_boneNames == NV_NULL)
	{
		NV_CO_LOG_ERROR("asset does not have bone names.");
		return NV_FAIL; 
	}
	if (boneId >= desc.m_numBones)
	{
		NV_CO_LOG_ERROR("invalid bone id.");
		return NV_FAIL; 
	}
	const char* str = desc.m_boneNames + boneId * NV_HAIR_MAX_STRING;
	strncpy(boneNameOut, str, NV_HAIR_MAX_STRING);
	return NV_OK;
}

Result SdkImpl::getBindPose( AssetId assetId, Int boneId, gfsdk_float4x4* bindPoseOut)
{
	NV_HAIR_THREAD_LOCK
	if (!bindPoseOut)
	{
		NV_CO_LOG_ERROR("bind pose pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	if (boneId >= assetDesc.m_numBones)
	{
		NV_CO_LOG_ERROR("bone id is invalid.");
		return NV_FAIL; 
	}
	if (assetDesc.m_bindPoses == NV_NULL)
	{
		NV_CO_LOG_ERROR("asset does not have bind poses.");
		return NV_FAIL;
	}
	*bindPoseOut = assetDesc.m_bindPoses[boneId];
	return NV_OK;
}

Int SdkImpl::getNumGuideHairs( AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return 0;
	}
	return asset->getDesc().m_numGuideHairs;
}

Int SdkImpl::getNumHairVertices( AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return 0;
	}
	return asset->getDesc().m_numVertices;
}

Int SdkImpl::getNumFaces( AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return 0;
	}
	return asset->getDesc().m_numFaces;
}

Result SdkImpl::getHairVertices( AssetId assetId, gfsdk_float3* verticesOut)
{
	NV_HAIR_THREAD_LOCK
	if (!verticesOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(verticesOut, assetDesc.m_vertices, sizeof(gfsdk_float3) * assetDesc.m_numVertices);
	return NV_OK;
}

Result SdkImpl::getRootVertices(AssetId assetId, gfsdk_float3* verticesOut)
{
	NV_HAIR_THREAD_LOCK
	if (!verticesOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	for (Int i = 0; i < assetDesc.m_numGuideHairs; i++)
	{
		UInt32 index = (i == 0) ? 0 : assetDesc.m_endIndices[i-1] + 1;
		verticesOut[i] = assetDesc.m_vertices[index];
	}
	return NV_OK;
}

Result SdkImpl::getEndIndices( AssetId assetId, UInt32* indicesOut)
{
	NV_HAIR_THREAD_LOCK
	if (!indicesOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(indicesOut, assetDesc.m_endIndices, sizeof(UInt32) * assetDesc.m_numGuideHairs);
	return NV_OK;
}

Result SdkImpl::getFaceIndices( AssetId assetId, UInt32* indicesOut)
{
	NV_HAIR_THREAD_LOCK
	if (!indicesOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(indicesOut, assetDesc.m_faceIndices, sizeof(UInt32) * assetDesc.m_numFaces * 3);
	return NV_OK;
}

Result SdkImpl::getFaceUvs( AssetId assetId, gfsdk_float2* uvsOut)
{
	NV_HAIR_THREAD_LOCK
	if (!uvsOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(uvsOut, assetDesc.m_faceUvs, sizeof(gfsdk_float2) * assetDesc.m_numFaces * 3);
	return NV_OK;
}

Result SdkImpl::getBoneIndices( AssetId assetId, gfsdk_float4* boneIndicesOut)
{
	NV_HAIR_THREAD_LOCK
	if (!boneIndicesOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(boneIndicesOut, assetDesc.m_boneIndices, sizeof(gfsdk_float4) * assetDesc.m_numGuideHairs);
	return NV_OK;
}

Result SdkImpl::getBoneWeights( AssetId	assetId, gfsdk_float4* boneWeightsOut)
{
	NV_HAIR_THREAD_LOCK
	if (!boneWeightsOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	NvCo::Memory::copy(boneWeightsOut, assetDesc.m_boneWeights, sizeof(gfsdk_float4) * assetDesc.m_numGuideHairs);
	return NV_OK;
}

Result SdkImpl::getTextureName( AssetId assetId, ETextureType texType, Char* textureNameOut)
{
	NV_HAIR_THREAD_LOCK
	if (!textureNameOut)
	{
		NV_CO_LOG_ERROR("Input pointer is null.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (texType >= TextureType::COUNT_OF)
	{
		NV_CO_LOG_ERROR("Texture id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const AssetDescriptor& assetDesc = asset->getDesc();
	if (assetDesc.m_textureNames == NV_NULL)
	{
		NV_CO_LOG_ERROR("Texture names not set.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const char* str = assetDesc.m_textureNames + Int(texType) * NV_HAIR_MAX_STRING;
	strncpy(textureNameOut, str, NV_HAIR_MAX_STRING);
	return NV_OK;
}

Int SdkImpl::getNumPins(AssetId assetId)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return 0;
	}
	return asset->getDesc().m_numPins;
}

void SdkImpl::getPins(AssetId assetId, Int startIndex, Int numPins, Pin* pinsOut)
{
	NV_HAIR_THREAD_LOCK
	if (!pinsOut)
	{
		NV_CO_LOG_ERROR("ERROR: getPins() - pinOut Ptr is invalid.");
		return;
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("ERROR: getPins() - Asset id is invalid.");
		return;
	}
	const AssetDescriptor& desc = asset->getDesc();
	if (desc.m_pins == NV_NULL)
	{
		NV_CO_LOG_ERROR("ERROR: getPins() - asset does not have pins.");
		return;
	}
	if (startIndex < 0 || numPins <= 0 || startIndex + numPins > (Int)desc.m_numPins)
	{
		NV_CO_LOG_ERROR("ERROR: getPins() - invalid startIndex and numPins.");
		return;
	}
	for (int i = 0; i < numPins; ++i)
		pinsOut[i] = desc.m_pins[i + startIndex];
	return;
}

void SdkImpl::setPins(AssetId assetId, Int startIndex, Int numPins, const Pin* pinsIn)
{
	NV_HAIR_THREAD_LOCK
	if (!pinsIn)
	{
		NV_CO_LOG_ERROR("ERROR: setPins() - pinsIn Ptr is invalid.");
		return;
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("ERROR: setPins() - Asset id is invalid.");
		return;
	}
	const AssetDescriptor& desc = asset->getDesc();
	if (desc.m_pins == NV_NULL)
	{
		NV_CO_LOG_ERROR("ERROR: setPins() - asset does not have pins.");
		return;
	}
	if (startIndex < 0 || numPins <= 0 || (startIndex + numPins) > (Int)desc.m_numPins)
	{
		NV_CO_LOG_ERROR("ERROR: setPins() - invalid startIndex and numPins.");
		return;
	}
	for (int i = 0; i < numPins; ++i)
		desc.m_pins[i + startIndex] = pinsIn[i];

	for (IndexT i = 0; i < m_instances.getSize(); i++)
	{
		Instance* inst = m_instances[i];
		if (inst && inst->m_apiInstance && assetId == inst->m_assetId)
		{
			if (NV_FAILED(inst->m_apiInstance->updatePinConstraintBuffer()))
			{
				NV_CO_LOG_ERROR("ERROR: setPins() - Hair update pin buffer failed.");
			}
		}
	}
	return;
}

Result SdkImpl::clearShaderCache()
{
	NV_HAIR_THREAD_LOCK
	m_apiGlobal->getShaderCache()->clear();
	// avoid dangling pointers, created from doing the clear on the cache
	for (int i = 0; i < m_instances.getSize(); i++)
	{
		if (m_instances[i])
			m_instances[i]->m_cacheEntry = NV_NULL;
	}
	return NV_OK;
}

Result SdkImpl::addToShaderCache(const ShaderCacheSettings& settingsIn)
{
	NV_HAIR_THREAD_LOCK

	ShaderCache* shaderCache = m_apiGlobal->getShaderCache();

	const ShaderCacheSettings settings = shaderCache->calcUniqueSettings(settingsIn);
	if (shaderCache->findOrCreate(settings))
		return NV_OK;
	NV_CO_LOG_ERROR("Failed to add shader settings to shader cache.");
	return NV_OK;
}

Result SdkImpl::saveShaderCache(NvCo::WriteStream* stream)
{
	NV_HAIR_THREAD_LOCK
	ShaderCache* shaderCache = m_apiGlobal->getShaderCache();
	return shaderCache->write(stream);
}

Result SdkImpl::loadShaderCache(NvCo::ReadStream* stream)
{
	NV_HAIR_THREAD_LOCK

	ShaderCache* shaderCache = m_apiGlobal->getShaderCache();
	Result res = shaderCache->read(stream);
	if (NV_FAILED(res))
	{
		NV_CO_LOG_ERROR("Failed load shader cache.");
		return res;
	}
	if (!m_apiGlobal->getApiDevice().isNull())
	{
		res = shaderCache->createShaders(m_apiGlobal->getApiDevice());
		if (NV_FAILED(res))
		{
			NV_CO_LOG_ERROR("Could not create shaders from shader cache.");
			return res;
		}
	}
	return NV_OK;
}

Result SdkImpl::initRenderResources(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn, const NvCo::ConstApiPtr& other)
{
	NV_HAIR_THREAD_LOCK

	NvResult res;

	const Bool deviceChanged = !m_apiGlobal->areEqual(m_apiGlobal->getApiDevice(), deviceIn);

	res = m_apiGlobal->init(deviceIn, contextIn, other);
	if (NV_FAILED(res)) return NV_FAIL;
	
	if (deviceChanged && deviceIn.isSet())
	{
		// We need to rebind
		for (IndexT i = 0; i < m_assets.getSize(); i++)
		{
			Asset* asset = m_assets[i];
			if (asset)
			{
				m_apiGlobal->bindAsset(*asset);
			}
		}
		for (IndexT i = 0; i < m_instances.getSize(); i++)
		{
			Instance* inst = m_instances[i];
			if (inst)
			{
				m_apiGlobal->bindInstance(*inst);
			}
		}
	}

	m_msaaComposer = m_apiGlobal->createMsaaComposer();
	
	const char *env = ::getenv("NVHAIR_BACKDOOR_ENABLED");
	if (( m_backDoorMode == 0) && (env && _stricmp(env,"true") == 0 ))
	{
		m_backDoorMode = 2;
	}
	if (m_backDoorMode == 2) // slave mode
	{
		NvCo::UniquePtr<HairChannel> channel(new HairChannel);
		res = channel->init();
		if (NV_SUCCEEDED(res))
		{
			m_backDoorChannel = channel;
		}
	}
	
	return NV_OK;
}

void SdkImpl::freeRenderResources()
{
	NV_HAIR_THREAD_LOCK

	m_msaaComposer.setNull();
	
	// Destroy the instances
	{
		for (IndexT i = 0; i < m_instances.getSize(); ++i)
		{
			Instance* instance = m_instances[i];
			if (instance)
			{
				instance->m_apiInstance.setNull();
			}
		}	
	}
	// Now all the assets
	{
		for (IndexT i = 0; i < m_assets.getSize(); ++i)
		{
			Asset* asset = m_assets[i];
			if (asset)
			{
				asset->m_apiAsset.setNull();
			}
		}
	}

	if (m_backDoorMode == 2)
	{
		if (m_backDoorChannel)
		{
			m_backDoorChannel->getChannel()->send("HairWorks shutting down");
		}	
	}
}

Result SdkImpl::setCurrentContext(const NvCo::ApiContext& contextIn)
{
	NV_HAIR_THREAD_LOCK
	NV_RETURN_ON_FAIL(m_apiGlobal->setContext(contextIn));
	return NV_OK;
}

Result SdkImpl::getInstanceDescriptorFromAsset(AssetId assetId, InstanceDescriptor& descriptorOut)
{
	NV_HAIR_THREAD_LOCK
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	descriptorOut = asset->m_materials[0];
	return NV_OK;
}

Result SdkImpl::getInstanceDescriptor(InstanceId instanceId, InstanceDescriptor& descriptorOut)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_FAIL;
	}
	descriptorOut = inst->m_materials[0];
	return NV_OK;
}

Result SdkImpl::createInstance(AssetId assetId, InstanceId& instanceIdOut)
{
	NV_HAIR_THREAD_LOCK
	if (m_apiGlobal->getApiDevice().isNull())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	Asset* asset = _getAsset(assetId);
	if (!asset)
	{
		NV_CO_LOG_ERROR("Asset id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	//const HairAssetDescriptor& hairDesc = asset->getDesc();
	InstanceDescriptor* descriptor = asset->m_materials;
	
	// find next available slot for instance resource
	IndexT freeSlot = m_instances.indexOf(NV_NULL);
	if (freeSlot < 0)
	{
		freeSlot = m_instances.getSize();
		m_instances.pushBack(NV_NULL);
	}

	NvCo::UniquePtr<Instance> inst(new Instance(asset));

	Int numMaterials = Asset::MAX_MATERIALS;
	// copy parameters here
	if (descriptor)
	{
		memcpy(inst->m_materials, descriptor, numMaterials * sizeof(InstanceDescriptor));
	}

	Result res = m_apiGlobal->bindInstance(*inst.get());
	if (NV_FAILED(res)) return NV_FAIL;

	inst->m_instanceId = InstanceId(freeSlot);
	inst->m_assetId = assetId;

	m_instances[freeSlot] = inst.detach();

	instanceIdOut = InstanceId(freeSlot);
	if (m_backDoorMode == 2 && m_backDoorChannel)
	{
		m_backDoorChannel->getChannel()->send("HAIR_CREATED");
	}
	return NV_OK;
}

Result SdkImpl::freeInstance(InstanceId instanceId)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	delete inst;
	m_instances[instanceId] = NV_NULL;
	return NV_OK;
}

Result SdkImpl::updateInstanceDescriptor( InstanceId instanceId, const InstanceDescriptor& descriptor)
{
	NV_HAIR_THREAD_LOCK

	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	InstanceDescriptor& material = inst->m_materials[0];

	material = descriptor;
	
	inst->setNumVertsPerSegment(material.m_splineMultiplier);

	if (m_backDoorMode == 2 && m_backDoorChannel)
	{
		BackDoorChannel* channel = m_backDoorChannel->getChannel();
		if (channel->checkMessage("HAIR_DESCRIPTOR"))
		{
			channel->sendData((char*)&material, sizeof(InstanceDescriptor), 1);
		}
		else
		{			
			m_backDoorChannel->updateDescriptor(material);
		}
	}

	// to do, make gravity and wind global parameters
	if (!gfsdk_equal(inst->m_modelToWorld, descriptor.m_modelToWorld))
	{
		inst->m_modelToWorld = descriptor.m_modelToWorld;
		// wind and gravity are passed in world, but converted to be in local
		gfsdk_float4x4 worldToLocal = gfsdk_inverse(descriptor.m_modelToWorld);
		material.m_wind = gfsdk_transformVector( worldToLocal, material.m_wind);
		material.m_gravityDir = gfsdk_transformVector( worldToLocal, material.m_gravityDir);

		// The current tessellation is wrong if the model to world matrix has changed.
		inst->m_isTessellated = false;
	}

	return NV_OK;
}

Result SdkImpl::updateSkinningMatrices(InstanceId instanceId, Int numBones, const gfsdk_float4x4* boneMatrices, ETeleportMode teleportMode)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (inst->m_asset->getNumBones() != Int(numBones))
	{
		NV_CO_LOG_ERROR("Number of bones does not match asset.");
		return NV_FAIL;
	}
	inst->updateSkinningMatrices(boneMatrices);
	inst->m_lodInfo.setLodFactor(inst, m_frameViewInfo.m_currentView);
	inst->m_isTessellated = false;

	inst->m_teleportMode = teleportMode;
	return NV_OK;
}

Result SdkImpl::updateSkinningDqs(InstanceId instanceId, Int numBones, const gfsdk_dualquaternion* dqs, ETeleportMode teleportMode)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (inst->m_asset->getNumBones() != Int(numBones))
	{
		NV_CO_LOG_ERROR("Number of bones does not match asset.");
		return NV_FAIL;
	}
	inst->updateSkinningDqs(dqs);

	inst->m_lodInfo.setLodFactor(inst, m_frameViewInfo.m_currentView);
	inst->m_teleportMode = teleportMode;
	inst->m_isTessellated = false;
	return NV_OK;
}

Result SdkImpl::stepSimulation(Float timeStep, const gfsdk_float4x4* worldReference, Bool simulateOnly)
{
	// uncomment to show fps indicator
	//NvAPI_D3D_SetFPSIndicatorState(m_pD3D11Device, true);

	NV_HAIR_THREAD_LOCK

	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}

	// restore all the render state before/after hair rendering
	ScopeRenderState scope(m_apiGlobal);
	
	for (IndexT i = 0; i < m_instances.getSize(); ++i)
	{
		Instance* inst = m_instances[i];
		if (!inst)
			continue;

		const InstanceDescriptor& params = inst->getDefaultMaterial();
		if (params.m_enable == false)
			continue;

		//// skip hair rendering for hairs that have been LOD'ed out
		LodInfo& lodInfo = inst->m_lodInfo;
		lodInfo.setLodFactor(inst, m_frameViewInfo.m_currentView);
		// early out on LOD factor, but if has never stepped allow a single step
		if (lodInfo.m_distanceFactor >= 1.0f)
		{
			inst->m_isSimulationStarted = false;
			continue;
		}

		ApiInstance* apiInst = inst->m_apiInstance;

		// always enable simulation
		if (NV_FAILED(apiInst->stepSimulation(timeStep, worldReference)))
		{
			NV_CO_LOG_ERROR("Hair simulation computation failed.");
			return NV_FAIL;
		}
		
		inst->m_isTessellated = false;
		if (!simulateOnly)
		{
			apiInst->preRender(1.0f);
			inst->m_isTessellated = true;
		}

		inst->m_isSimulationStarted = true;
	}

	return NV_OK;
}

Result SdkImpl::stepInstanceSimulation(InstanceId instanceId, Float timeStep, const gfsdk_float4x4* worldReference, Bool simulateOnly)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}

	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	// restore all the render state before/after hair rendering
	ScopeRenderState scope(m_apiGlobal);

	const InstanceDescriptor& params = inst->getDefaultMaterial();
	if (params.m_enable)
	{
		ApiInstance* apiInst = inst->m_apiInstance;
		// always enable simulation
		if (NV_FAILED(apiInst->stepSimulation(timeStep, worldReference)))
		{
			NV_CO_LOG_ERROR("Hair simulation computation failed.");
			return NV_FAIL;
		}
		inst->m_isTessellated = false;
		if (!simulateOnly)
		{
			apiInst->preRender(1.0f);
			inst->m_isTessellated = true;
		}
		inst->m_isSimulationStarted = true;
	}

	return NV_OK;
}

Result SdkImpl::getBounds(AssetId assetId, const gfsdk_float4x4 * skinningMatrices, gfsdk_float3 & bbMinOut, gfsdk_float3 & bbMaxOut, Bool growthMeshOnly)
{
	NV_HAIR_THREAD_LOCK

	if (assetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("fromAssetID is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	Asset* asset = m_assets[assetId];
	if(!asset)
	{
		NV_CO_LOG_ERROR("specified asset does not exist.");
		return NV_FAIL;
	}

	gfsdk_float3 bbMinGrowthMesh, bbMaxGrowthMesh;
	asset->calcSkinnedGrowthMeshBounds(skinningMatrices, bbMinGrowthMesh, bbMaxGrowthMesh);

	if(!growthMeshOnly)
	{
		// Add the maximum hair distance from the center to the current center for bounding size
		gfsdk_float3 skinnedCenter = (bbMinGrowthMesh + bbMaxGrowthMesh) * 0.5f;
		float d = asset->m_maxHairCenterDist;
		bbMinOut = skinnedCenter - gfsdk_makeFloat3(d, d, d);
		bbMaxOut = skinnedCenter + gfsdk_makeFloat3(d, d, d);
	}
	else
	{
		bbMinOut = bbMinGrowthMesh;
		bbMaxOut = bbMaxGrowthMesh;
	}

	return NV_OK;
}

Result SdkImpl::getBounds(AssetId assetId, const gfsdk_dualquaternion * skinningDqs, gfsdk_float3 & bbMinOut, gfsdk_float3 & bbMaxOut, Bool growthMeshOnly)
{
	NV_HAIR_THREAD_LOCK

	if (assetId >= m_assets.getSize())
	{
		NV_CO_LOG_ERROR("fromAssetID is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	Asset* asset = m_assets[assetId];
	if(!asset)
	{
		NV_CO_LOG_ERROR("specified asset does not exist.");
		return NV_FAIL;
	}

	gfsdk_float3 bbMinGrowthMesh, bbMaxGrowthMesh;
	asset->calcSkinnedGrowthMeshBounds(skinningDqs, bbMinGrowthMesh, bbMaxGrowthMesh);

	if(!growthMeshOnly)
	{
		// Add the maximum hair distance from the center to the current center for bounding size
		gfsdk_float3 skinnedCenter = (bbMinGrowthMesh + bbMaxGrowthMesh) * 0.5f;
		float d = asset->m_maxHairCenterDist;
		bbMinOut = skinnedCenter - gfsdk_makeFloat3(d, d, d);
		bbMaxOut = skinnedCenter + gfsdk_makeFloat3(d, d, d);
	}
	else
	{
		bbMinOut = bbMinGrowthMesh;
		bbMaxOut = bbMaxGrowthMesh;
	}

	return NV_OK;
}

Result SdkImpl::getBounds(InstanceId instanceId, gfsdk_float3& minOut, gfsdk_float3& maxOut, Bool growthMeshOnly)
{
	NV_HAIR_THREAD_LOCK

	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	inst->getBounds(minOut, maxOut, growthMeshOnly);
	return NV_OK;
}

Result SdkImpl::setViewProjection(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, EHandednessHint handedness, Float fov)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	m_frameViewInfo.m_useCubeMap = false;
	m_frameViewInfo.m_useLeftHanded = (handedness == HandednessHint::LEFT);

	return m_apiGlobal->calcViewInfo(viewport, view, projection, fov, m_frameViewInfo.m_currentView);
}

void SdkImpl::setCubeMapViewProjection(const Viewport viewports[6], const gfsdk_float4x4 view[6], const gfsdk_float4x4 proj[6], const Bool faceVisibile[6], EHandednessHint handedness)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return;
	}
	// The fov doesn't really matter - it is used in LOD calculations, but the proj matrices actually hold the projection
	const Float fov = 90.0f;
	for (int i = 0; i < 6; ++i)
	{
		m_apiGlobal->calcViewInfo(viewports[i], view[i], proj[i], fov, m_frameViewInfo.m_cubeViews[i]);
		m_frameViewInfo.m_isCubeFaceVisible[i] = faceVisibile[i];
	}
	m_frameViewInfo.m_useLeftHanded = (handedness == HandednessHint::LEFT);
	m_frameViewInfo.m_useCubeMap = true;
}

Result SdkImpl::setPrevViewProjection(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, Float fov)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	m_frameViewInfo.m_useCubeMap = false;
	return m_apiGlobal->calcViewInfo(viewport, view, projection, fov, m_frameViewInfo.m_previousView);
}

Result SdkImpl::startMsaaRendering(Int sampleCount, Bool depthCompareLess, const NvCo::ConstApiPtr& other)
{
	NV_HAIR_THREAD_LOCK

	if (!m_msaaComposer)
	{
		return NV_FAIL;
	}

	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	if (sampleCount == 0)
	{
		m_msaaComposer->setSampleCount(0);
		return NV_OK;
	}

	bool validCount = (sampleCount == 1) || (sampleCount == 2) || (sampleCount == 4) || (sampleCount == 8);
	if (!validCount)
	{
		NV_CO_LOG_ERROR("Invalid sample count. Only 0, 1, 2, 4, 8 are currently allowed.");
		return NV_FAIL;
	}

	{
		ScopeRenderState scope(m_apiGlobal);
		
		if (NV_FAILED(m_msaaComposer->prepare(m_apiGlobal->getApiContext(), sampleCount, depthCompareLess, other)))
		{
			NV_CO_LOG_ERROR("Internal initialization failure.");
			return NV_FAIL;
		}
		if (NV_FAILED(m_msaaComposer->upsampleDepthBuffer()))
		{
			NV_CO_LOG_ERROR("Internal initialization failure.");
			return NV_FAIL;
		}
		if (NV_FAILED(m_msaaComposer->startRendering()))
		{
			NV_CO_LOG_ERROR("Internal initialization failure.");
			return NV_FAIL;
		}
	}
	m_msaaComposer->setBlendState();
	return NV_OK;
}

Result SdkImpl::finishMsaaRendering()
{
	NV_HAIR_THREAD_LOCK

	if (!m_msaaComposer)
	{
		return NV_FAIL;
	}

	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	if (m_msaaComposer->getSampleCount() == 0)
		return NV_OK;

	{
		ScopeRenderState scope(m_apiGlobal);

		if (NV_FAILED(m_msaaComposer->finishRendering()))
		{
			NV_CO_LOG_ERROR("Internal MSAA resolve failed.");
			return NV_FAIL;
		}
	}
	m_msaaComposer->restoreBlendState();
	return NV_OK;
}

Result SdkImpl::drawMsaaColor()
{
	NV_HAIR_THREAD_LOCK
	if (!m_msaaComposer)
	{
		return NV_FAIL;
	}
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}

	if (m_msaaComposer->getSampleCount() == 0)
		return NV_OK;

	ScopeRenderState scope(m_apiGlobal);
	
	if (NV_FAILED(m_msaaComposer->drawColor()))
	{
		NV_CO_LOG_ERROR("Internal MSAA draw call failed.");
		return NV_FAIL;
	}
	return NV_OK;
}

Result SdkImpl::drawMsaaPostDepth(bool emitPartialFramgment)
{
	NV_HAIR_THREAD_LOCK

	if (!m_msaaComposer)
	{
		return NV_FAIL;
	}
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	if (m_msaaComposer->getSampleCount() == 0)
		return NV_OK;

	ScopeRenderState scope(m_apiGlobal);
	if (NV_FAILED(m_msaaComposer->drawPostDepth(emitPartialFramgment)))
	{
		NV_CO_LOG_ERROR("Internal MSAA draw call failed.");
		return NV_FAIL;
	}
	return NV_OK;
}

Result SdkImpl::preRender(Float simulationInterp)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}

	{
		// restore all the render state before/after hair rendering
		ScopeRenderState scope(m_apiGlobal);
		for (IndexT i = 0; i < m_instances.getSize(); ++i)
		{
			Instance* inst = m_instances[i];
			if (inst)
			{
				const InstanceDescriptor& params = inst->getDefaultMaterial();
				if (params.m_enable)
				{
					NV_RETURN_ON_FAIL(inst->preRender(simulationInterp));
				}
			}
		}
	}
	return NV_OK;
}

Result SdkImpl::preRenderInstance(InstanceId instanceId, Float simulationInterp)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	const InstanceDescriptor& params = inst->getDefaultMaterial();
	if (params.m_enable && inst->needsPreRender(simulationInterp))
	{
		// restore all the render state before/after hair rendering
		ScopeRenderState scope(m_apiGlobal);
		NV_RETURN_ON_FAIL(inst->preRender(simulationInterp));
	}
	return NV_OK;
}

Result SdkImpl::renderHairs(InstanceId instanceId, const ShaderSettings* settingsIn, const NvCo::ConstApiPtr& other)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	if (!inst->m_isTessellated)
	{
		NV_CO_LOG_ERROR("preRender not performed either in call to stepSimulate or with explicit preRender call.");
		return NV_FAIL;
	}

	ApiInstance* apiInst = inst->m_apiInstance;
	if (!apiInst)
	{
		NV_CO_LOG_ERROR("Internal buffer was not properly initialized.");
		return NV_FAIL;
	}

	const InstanceDescriptor& params = inst->getDefaultMaterial();
	if (params.m_enable == false)
		return NV_OK;
	if (params.m_drawRenderHairs == false)
		return NV_OK;

	inst->setActive(false);

	ShaderSettings settings;
	if (settingsIn)
		settings = *settingsIn;

	// do not compute LOD for shadow pass (different view matrix will pollute LOD)
	if (settings.m_shadowPass)
	{
		LodInfo shadowLodData; //do not compute LOD for shadow pass (different view matrix will pollute LOD)
		shadowLodData.setLodFactor(inst, m_frameViewInfo.m_currentView);
		if (shadowLodData.m_distanceFactor >= 1.0f)
			return NV_OK; // light camera is too far away from hair
	}
	else
	{
		LodInfo& lodInfo = inst->m_lodInfo;
		lodInfo.setLodFactor(inst, m_frameViewInfo.m_currentView);
		// regardless of pass, we discard if scene lod is out
		if (lodInfo.m_distanceFactor >= 1.0f)
			return NV_OK;
	}

	{
		ScopeRenderState scope(m_apiGlobal);

		m_apiGlobal->setRasterizerMode(ApiGlobal::RASTERIZER_MODE_SOLID);
		m_apiGlobal->applySamplers(false, true, true, true, false);

		// do actual render
		{
			const Result res = inst->m_apiInstance->renderHairShading(m_frameViewInfo, settings, other);
			if (NV_FAILED(res))
			{
				NV_CO_LOG_ERROR("Internal draw call failed.");
				return res;	
			}
		}
		inst->setActive(true);
	}
	return NV_OK;
}

Result SdkImpl::_drawVisualization(Instance* inst, ApiInstance::EDebugDraw drawType)
{
	ApiInstance* apiInst = inst->m_apiInstance;
	if (apiInst)
	{
		Result res = apiInst->debugDraw(drawType, m_frameViewInfo.m_currentView);
		if (NV_FAILED(res))
		{
			char buffer[1024];
			sprintf(buffer, "Draw of '%s' failed", ApiInstance::getText(drawType));
			NV_CO_LOG_ERROR(buffer);
		}
		return res;
	}
	return NV_OK;
}

Result SdkImpl::renderVisualization(InstanceId instanceId, const VisualizationSettings* settings)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_FAIL;
	}
	
	if (!inst->m_isTessellated)
	{
		NV_CO_LOG_ERROR("preRender not performed either in call to stepSimulate or with explicit preRender call.");
		return NV_FAIL;
	}

	ApiInstance* apiInst = inst->m_apiInstance;
	if (!apiInst)
	{
		NV_CO_LOG_ERROR("Internal buffer was not properly initialized.");
		return NV_FAIL;
	}

	const InstanceDescriptor& params = inst->getDefaultMaterial();
	if (params.m_enable == false)
		return NV_OK;

	inst->m_lodInfo.setLodFactor(inst, m_frameViewInfo.m_currentView);

	// Render without line renderer
	{
		ScopeRenderState scope(m_apiGlobal);

		const EDepthOp depthOp = settings ? settings->m_depthOp : DepthOp::WRITE_LESS;

		m_apiGlobal->setRasterizerMode(ApiGlobal::RASTERIZER_MODE_WIRE_FRAME);
		m_apiGlobal->setDepthOp(depthOp);
	
		if (params.m_visualizeGuideHairs)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::GUIDE_HAIRS);
		}
		if (params.m_visualizeSkinnedGuideHairs)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::SKINNED_GUIDE_HAIRS);
		}
		if (params.m_visualizeFrames)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::FRAMES);
		}
		if (params.m_visualizeShadingNormals)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::NORMALS);
		}
		if (params.m_visualizeLocalPos)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::LOCAL_POS);
		}
		if (params.m_visualizeHairInteractions)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::HAIR_INTERACTION);
		}
		if (params.m_visualizeControlVertices)
		{	
			_drawVisualization(inst, ApiInstance::DebugDraw::GUIDE_HAIR_CONTROL_VERTICES);
		}
		if (params.m_visualizeGrowthMesh)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::GROWTH_MESH);
		}
		if (params.m_visualizePinConstraints)
		{
			_drawVisualization(inst, ApiInstance::DebugDraw::PIN_CONSTRAINTS);
		}
	
		LineRenderer* lineRenderer = m_apiGlobal->getLineRenderer();
		if (lineRenderer)
		{
			// Set the depth op on the line renderer
			lineRenderer->setDepthOp(depthOp);
			// Start line rendering
			lineRenderer->start(m_apiGlobal->getApiContext(),  m_frameViewInfo.m_currentView.m_viewProjectionMatrix);
			if (params.m_visualizeBones)
				inst->drawDebugBones(lineRenderer);
			if (params.m_visualizeCapsules)
				inst->drawDebugCapsules(lineRenderer);
			if (params.m_visualizeBoundingBox)
				inst->drawDebugBoundingBox(lineRenderer);
			if (params.m_visualizeShadingNormalBone)
				inst->drawDebugShadingBone(lineRenderer);
			if (params.m_visualizeCullSphere)
				inst->drawDebugCullSphere(lineRenderer);
			// Line rendering is done
			lineRenderer->end();
		}
	}

	return NV_OK;
}

Result SdkImpl::prepareShaderConstantBuffer(InstanceId instanceId, ShaderConstantBuffer& constantBuffer)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	inst->m_apiInstance->calcPixelConstantBuffer(m_frameViewInfo, constantBuffer);
	return NV_OK;
}

Result SdkImpl::setTexture(InstanceId instanceId, ETextureType t, const NvCo::ApiHandle& texture)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst || !inst->m_apiInstance)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	ApiInstance* apiInst = inst->m_apiInstance;
	apiInst->setApiTexture(t, texture);
	return NV_OK;
}

Result SdkImpl::getTextures(InstanceId instanceId, const ETextureType* types, Int numTextures, const NvCo::ApiPtr& texturesOut)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst || !inst->m_apiInstance)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	ApiInstance* apiInst = inst->m_apiInstance;
	return apiInst->getApiTextures(types, numTextures, texturesOut);
}

Result SdkImpl::getShaderResources(InstanceId instanceId, const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& resourcesOut)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}
	if (!inst->m_isTessellated)
	{
		NV_CO_LOG_ERROR("simulationStep not performed with simulationOnly = false and/or requires explicit preRender call");
		return NV_FAIL;
	}

	ApiInstance* apiInst = inst->m_apiInstance;
	if (!apiInst)
	{
		NV_CO_LOG_ERROR("Buffer is not properly initialized.");
		return NV_HAIR_RESULT(INVALID_PARAMETERS);
	}

	return apiInst->getApiResources(types, numResources, resourcesOut);
}

Result SdkImpl::computeStats(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instanceId, Stats& stats)
{
	NV_HAIR_THREAD_LOCK
	if (!m_apiGlobal->canRender())
	{
		NV_CO_LOG_ERROR("Not initialized.  Call initRenderResources() first.");
		return NV_FAIL;
	}
	Instance* inst = _getInstance(instanceId);
	if (!inst)
	{
		NV_CO_LOG_ERROR("Instance id is invalid.");
		return NV_FAIL;
	}
	const InstanceDescriptor& params = inst->getDefaultMaterial();
	if (params.m_enable == false)
		return NV_OK;

	NvCo::Memory::zero(stats);

	if (!inst->isActive())
	{
		NV_CO_LOG_ERROR("Specified instance is not active, cannot compute stats.");
		return NV_FAIL;
	}

	return inst->m_apiInstance->computeStats((NvCo::HandleMapHandle*)asyncInOut, asyncRepeat, stats);
}

Result SdkImpl::getPinMatrix(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instanceId, Int pinIndex, gfsdk_float4x4& matrixOut)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst && inst->m_apiInstance)
	{
		gfsdk_makeIdentity(matrixOut);
		NV_CO_LOG_ERROR("Hair instance is invalid");
		return NV_FAIL;
	}
	return inst->m_apiInstance->getPinMatrix((NvCo::HandleMapHandle*)asyncInOut, asyncRepeat, pinIndex, matrixOut);
}

Result SdkImpl::getPinMatrices(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instanceId, Int startPinIndex, Int numPins, gfsdk_float4x4* matricesOut)
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instanceId);
	if (!inst && inst->m_apiInstance)
	{
		NV_CO_LOG_ERROR("Hair instance is invalid");
		return NV_FAIL;
	}
	return inst->m_apiInstance->getPinMatrices((NvCo::HandleMapHandle*)asyncInOut, asyncRepeat, startPinIndex, numPins, matricesOut);
}

Void SdkImpl::updateCompleted()
{
	NV_HAIR_THREAD_LOCK
	m_apiGlobal->updateCompleted();
}

Void SdkImpl::onGpuWorkSubmitted(const NvCo::ApiHandle& handle)
{
	NV_HAIR_THREAD_LOCK
	m_apiGlobal->onGpuWorkSubmitted(handle);
}

Result SdkImpl::setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo)
{
	NV_HAIR_THREAD_LOCK
	return m_apiGlobal->setPixelShader(shaderIndex, pixelShaderInfo);
}

Result SdkImpl::getPixelShader(Int shaderIndex, const NvCo::ApiPtr& pixelShaderInfoOut)
{
	NV_HAIR_THREAD_LOCK
	return m_apiGlobal->getPixelShader(shaderIndex, pixelShaderInfoOut);
}

Int SdkImpl::cancelAsync(const AsyncHandle* handles, Int numHandles, Bool allReferences)
{	
	NV_HAIR_THREAD_LOCK
	return m_apiGlobal->cancelAsync((const NvCo::HandleMapHandle*)handles, numHandles, allReferences);
}

Int SdkImpl::cancelAsync(InstanceId instId, Bool allReferences) 
{
	NV_HAIR_THREAD_LOCK
	Instance* inst = _getInstance(instId);
	if (!inst)
	{
		return 0;
	}
	return m_apiGlobal->cancelAsync(instId, allReferences);
}

} // namespace HairWorks 
} // namespace nvidia 
