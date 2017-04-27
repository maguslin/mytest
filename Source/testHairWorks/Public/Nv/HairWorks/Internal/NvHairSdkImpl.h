/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SDK_IMPL_H
#define NV_HAIR_SDK_IMPL_H

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/HairWorks/Internal/BackDoor/NvHairHairChannel.h>

#include <Nv/HairWorks/Internal/NvHairShaderCache.h>
#include <Nv/HairWorks/Internal/NvHairMsaaComposer.h>

#include <Nv/HairWorks/Internal/NvHairAsset.h>
#include <Nv/HairWorks/Internal/NvHairViewInfo.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/Common/NvCoUniquePtr.h>

#define USE_APEX_HAIR_PARAMS


namespace nvidia {
namespace HairWorks { 

#ifdef USE_APEX_HAIR_PARAMS
struct ParamsContext;
#endif

class SdkImpl : public Sdk
{
public:
	
	// Implement the interface HairSdk interface
	virtual void release() NV_OVERRIDE;

	virtual Result createAsset(const AssetDescriptor& assetDesc, AssetId& assetIdOut) NV_OVERRIDE;
	virtual Void freeAsset(const AssetId) NV_OVERRIDE;

	virtual ESerializeFormat getSerializeFormatFromExtension(const char* ext);

	virtual Result loadAsset(NvCo::ReadStream* stream, AssetId& assetIdOut, AssetHeaderInfo* infoOut = NV_NULL, const ConversionSettings* settings = NV_NULL) NV_OVERRIDE;
	virtual Result saveAsset(NvCo::WriteStream* stream, ESerializeFormat format, AssetId assetId, const InstanceDescriptor* descriptor = NV_NULL, const AssetHeaderInfo* info = NV_NULL, const Char*const* textureNames = NV_NULL) NV_OVERRIDE;
	virtual Result saveInstance(NvCo::WriteStream* stream, ESerializeFormat format, InstanceId instanceId, const AssetHeaderInfo* info = NV_NULL, const Char*const* textureNames = NV_NULL) NV_OVERRIDE;

	virtual Result copyAsset(AssetId fromAssetId, AssetId toAssetId, const AssetCopySettings& settings)  NV_OVERRIDE;
	virtual Result getInstanceDescriptorFromAsset(AssetId assetID, InstanceDescriptor& descriptor)  NV_OVERRIDE;

	virtual Result initRenderResources(const NvCo::ApiDevice& device, const NvCo::ApiContext& context, const NvCo::ConstApiPtr& other)  NV_OVERRIDE;
	virtual void freeRenderResources()  NV_OVERRIDE;
	virtual Result setCurrentContext(const NvCo::ApiContext& context)  NV_OVERRIDE;

	virtual Result createInstance(AssetId assetId, InstanceId& instanceIdOut)  NV_OVERRIDE;
	virtual Result freeInstance(InstanceId instId)  NV_OVERRIDE;
	virtual Result getInstanceDescriptor(InstanceId instId, InstanceDescriptor& descriptor)  NV_OVERRIDE;

	virtual Result updateInstanceDescriptor(InstanceId instId, const InstanceDescriptor& descriptor)  NV_OVERRIDE;

	virtual Result updateSkinningMatrices(InstanceId instId, Int numBones, const gfsdk_float4x4* boneMatrices, ETeleportMode teleMode)  NV_OVERRIDE;

	virtual Result updateSkinningDqs(InstanceId instId, Int numBones, const gfsdk_dualquaternion* boneMatrices, ETeleportMode teleMode)  NV_OVERRIDE;

	virtual Result stepSimulation(Float timeStep, const gfsdk_float4x4* worldReference, Bool simulateOnly)  NV_OVERRIDE;
	virtual Result stepInstanceSimulation(InstanceId instanceId, Float timeStepSize, const gfsdk_float4x4* worldReference, Bool simulateOnly) NV_OVERRIDE;

	virtual Result getBounds(InstanceId instId, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut, bool growthMeshOnly)  NV_OVERRIDE;

	virtual Result getBounds(AssetId assetId, const gfsdk_float4x4* skinningMatrices, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut, Bool growthMeshOnly = false) NV_OVERRIDE;

	virtual Result getBounds(AssetId assetId, const gfsdk_dualquaternion* skinningDqs, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut, Bool growthMeshOnly = false) NV_OVERRIDE;

	virtual void setCubeMapViewProjection(const Viewport viewports[6], const gfsdk_float4x4 view[6], const gfsdk_float4x4 proj[6], const bool visibility[6], EHandednessHint handedness) NV_OVERRIDE;

	virtual Result setViewProjection(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, EHandednessHint handedness, Float fov)  NV_OVERRIDE;
	virtual Result setPrevViewProjection(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, Float fov)  NV_OVERRIDE;

	virtual Result preRenderInstance(InstanceId instanceId, Float simulationInterp) NV_OVERRIDE;
	virtual Result preRender(Float simulationInterp) NV_OVERRIDE;

	virtual Result renderHairs(InstanceId instId, const ShaderSettings* settings, const NvCo::ConstApiPtr& other)  NV_OVERRIDE;
	virtual Result renderVisualization(InstanceId instId, const VisualizationSettings* settings)  NV_OVERRIDE;

	virtual Result prepareShaderConstantBuffer(InstanceId instId, ShaderConstantBuffer& constantBuffer)  NV_OVERRIDE;

	virtual Result setTexture(InstanceId instId, ETextureType t, const NvCo::ApiHandle& texture)  NV_OVERRIDE;
	virtual Result getTextures(InstanceId instId, const ETextureType* types, Int numTextures, const NvCo::ApiPtr& texturesOut)  NV_OVERRIDE;

	virtual Result getShaderResources(InstanceId instId, const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& resourcesOut)  NV_OVERRIDE;
	
	virtual Result startMsaaRendering(Int sampleCount, Bool depthCompareLess, const NvCo::ConstApiPtr& other)  NV_OVERRIDE;
	virtual Result finishMsaaRendering()  NV_OVERRIDE;
	virtual Result drawMsaaColor()  NV_OVERRIDE;
	virtual Result drawMsaaPostDepth(Bool emitPartialFramgment = true)  NV_OVERRIDE;

	virtual Result computeStats(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instId, Stats& stats)  NV_OVERRIDE;

	virtual const BuildInfo& getBuildInfo() NV_OVERRIDE;

	virtual Int getNumGuideHairs(AssetId assetId)  NV_OVERRIDE;
	virtual Int getNumHairVertices(AssetId assetId)  NV_OVERRIDE;
	virtual Int getNumFaces(AssetId assetId)  NV_OVERRIDE;
	virtual Result getHairVertices(AssetId assetId, gfsdk_float3* verticesOut)  NV_OVERRIDE;
	virtual Result getRootVertices(AssetId assetId, gfsdk_float3* verticesOut)  NV_OVERRIDE;
	virtual Result getEndIndices(AssetId assetId, UInt32* indicesOut)  NV_OVERRIDE;
	virtual Result getFaceIndices(AssetId assetId, UInt32* indicesOut)  NV_OVERRIDE;
	virtual Result getFaceUvs(AssetId assetId, gfsdk_float2* uvsOut)  NV_OVERRIDE;
	virtual Int getNumBones(AssetId assetId)  NV_OVERRIDE;
	virtual Result getBoneName(AssetId assetId, Int boneId, Char* boneNameOut)  NV_OVERRIDE;
	virtual Result getBindPose(AssetId assetId, Int boneId, gfsdk_float4x4* bindPose)  NV_OVERRIDE;
	virtual Result getBoneIndices(AssetId assetId, gfsdk_float4* boneIndicesOut)  NV_OVERRIDE;
	virtual Result getBoneWeights(AssetId assetId, gfsdk_float4* boneWeightsOut)  NV_OVERRIDE;

	virtual Result setBoneRemapping(AssetId assetId, const Char*const* boneNamesOut, Int numBones)  NV_OVERRIDE;
	virtual Result getTextureName(AssetId assetId, ETextureType textureId, Char* textureNameOut)  NV_OVERRIDE;
	virtual Result resampleGuideHairs(AssetId assetId, Int numTargetPointsPerHair)  NV_OVERRIDE;

	virtual Int getNumPins(AssetId assetId)  NV_OVERRIDE;
	virtual void getPins(AssetId assetId, Int startIndex, Int numPins, Pin* pinsOut)  NV_OVERRIDE;
	virtual void setPins(AssetId assetId, Int startIndex, Int numPins, const Pin* pinsIn)  NV_OVERRIDE;

	virtual Result clearShaderCache()  NV_OVERRIDE;
	virtual Result addToShaderCache(const ShaderCacheSettings& settings)  NV_OVERRIDE;
	virtual Result saveShaderCache(NvCo::WriteStream* stream)  NV_OVERRIDE;
	virtual Result loadShaderCache(NvCo::ReadStream* stream)  NV_OVERRIDE;

	virtual Result getPinMatrix(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instanceId, Int pinIndex, gfsdk_float4x4& matrixOut) NV_OVERRIDE;
	virtual Result getPinMatrices(AsyncHandle* asyncInOut, Bool asyncRepeat, InstanceId instanceId, Int startPinIndex, Int numPins, gfsdk_float4x4* matricesOut) NV_OVERRIDE;

	virtual Int cancelAsync(const AsyncHandle* handles, Int numHandles, Bool allReferences) NV_OVERRIDE;
	virtual Int cancelAsync(InstanceId instId, Bool allReferences) NV_OVERRIDE;

	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) NV_OVERRIDE;
	virtual Void updateCompleted() NV_OVERRIDE;

	virtual Result setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo) NV_OVERRIDE;
	virtual Result getPixelShader(Int shaderIndex, const NvCo::ApiPtr& pixelShaderInfoOut) NV_OVERRIDE;

		/// Get an instance
	Instance* getInstance(InstanceId instanceId) { IndexT index(instanceId); return index >= 0 && index < m_instances.getSize() ? m_instances[index] : NV_NULL; } 

		/// Ctor
	SdkImpl(ApiGlobal* apiGlobal, Int backDoorMode);
	~SdkImpl();

protected:
		/// Gets a new asset slot
	IndexT _getNewAssetSlot();
	/// Get an asset from assetId
	NV_FORCE_INLINE Asset* _getAsset(AssetId assetId) const
	{
		return (assetId < 0 || assetId >= m_assets.getSize() || assetId == ASSET_ID_NULL) ? NV_NULL : m_assets[assetId];
	}
	/// get hair resource from instance id
	NV_FORCE_INLINE Instance* _getInstance(const InstanceId instanceId) const
	{
		return (instanceId == INSTANCE_ID_NULL || instanceId >= m_instances.getSize()) ? NV_NULL : m_instances[instanceId];
	}
	Result _drawVisualization(Instance* inst, ApiInstance::EDebugDraw drawType);

protected:

	IndexT m_numAssets;			///< The amount of active assets
	NvCo::Array<Asset*> m_assets;	///< Maps handles to assets. NOTE slots can be NV_NULL. m_numAssets <= m_assets.getSize()

	IndexT m_numInstances;		///< The amount of active instances
	NvCo::Array<Instance*> m_instances;	///< Maps handles to instances. Note slots can be NV_NULL. m_numInstances <= m_instances.getSize()
	
	RenderViewInfo m_frameViewInfo;

	NvCo::UniquePtr<MsaaComposer> m_msaaComposer;
	NvCo::UniquePtr<ApiGlobal> m_apiGlobal;

#if defined(USE_APEX_HAIR_PARAMS)
	struct ParamsContext* m_paramsContext;
#endif

	BuildInfo m_buildInfo;

	NvCo::UniquePtr<HairChannel> m_backDoorChannel;
	Int m_backDoorMode;

	NvCo::CriticalSection m_criticalSection;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_SDK_IMPL_H
