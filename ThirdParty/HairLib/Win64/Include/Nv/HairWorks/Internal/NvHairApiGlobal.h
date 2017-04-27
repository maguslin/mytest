/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_API_GLOBAL_H
#define NV_HAIR_API_GLOBAL_H

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/NvCoApiHandle.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Container/NvCoHandleMap.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairLineRenderer.h>
#include <Nv/HairWorks/Internal/NvHairShaderCache.h>

namespace nvidia {
namespace HairWorks { 

class Asset;
class Instance;
class SliSystem;
class MsaaComposer;
class ShaderCache;
struct ViewInfo;

/*! Class for holding global API state and functionality. There will be one of these for the Sdk class implementation */
class ApiGlobal
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ApiGlobal);
		
	enum { STRAND_NOISE_TABLE_SIZE = 1024 };

	enum RasterizerMode
	{
		RASTERIZER_MODE_WIRE_FRAME,
		RASTERIZER_MODE_SOLID,
	};

		/// Must be called before being used
	virtual Result init(const NvCo::ApiDevice& device, const NvCo::ApiContext& context, const NvCo::ConstApiPtr& other) = 0;

		/// Create the asset
	virtual Result bindAsset(Asset& asset) = 0;
		/// Bind api specific instance
	virtual Result bindInstance(Instance& inst) = 0;
		/// By default doesn't support Msaa Composer
	virtual MsaaComposer* createMsaaComposer() { return NV_NULL; }

		/// Change the context
	virtual Result setContext(const NvCo::ApiContext& context) = 0;

		/// Saves all current render state
	virtual Void saveRenderState() = 0;
		/// Restores all current render state
	virtual Void restoreRenderState() = 0;

		/// Set the depth op
	virtual Void setDepthOp(EDepthOp depthOp) = 0;
		/// Set the rasterizer mode
	virtual Void setRasterizerMode(RasterizerMode mode) = 0;
		/// Apply sampler to the specified parts of pipeline
	virtual void applySamplers(bool useVs, bool useHs, bool useDs, bool useGs, bool usePs) = 0;
		/// True if is in a state where can render
	virtual Bool canRender() = 0;
		/// Fills in viewInfoOut with the correct values for rendering
	virtual Result calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut) = 0;
		/// True if the contents of a and b are equivalent
	virtual Bool areEqual(const NvCo::ApiHandle& a, const NvCo::ApiHandle& b) = 0;
		/// Called regularly when gpu work is submitted for resource management
	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) = 0;
		/// Set a shader
	virtual Result setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo) = 0;
		/// Get pixel shader	
	virtual Result getPixelShader(Int shaderIndex, const NvCo::ApiPtr& pixelShaderInfoOut) { NV_UNUSED(shaderIndex) NV_UNUSED(pixelShaderInfoOut) return NV_FAIL; }

	virtual Int cancelAsync(const NvCo::HandleMapHandle* handles, Int numHandles, Bool allReferences);

	virtual Int cancelAsync(InstanceId instId, Bool allReferences);

	virtual Void updateCompleted() {}

		/// Get the SLI system
	NV_FORCE_INLINE SliSystem* getSliSystem() const { return m_sliSystem; }
		/// Get the line renderer
	NV_FORCE_INLINE LineRenderer* getLineRenderer() const { return m_lineRenderer; }
		/// Get the shader cache
	NV_FORCE_INLINE ShaderCache* getShaderCache() const { return m_shaderCache; }

	NV_FORCE_INLINE const NvCo::ApiDevice& getApiDevice() const { return m_apiDevice; }
	NV_FORCE_INLINE const NvCo::ApiContext& getApiContext() const { return m_apiContext; }

	ApiGlobal();

		/// Get scalar noise
	NV_FORCE_INLINE const NvCo::PodBuffer<float>& getScalarNoise() const { return m_scalarNoiseLut; }
		/// Get the vector noise
	NV_FORCE_INLINE const NvCo::PodBuffer<gfsdk_float3>& getVectorNoise() const { return m_vectorNoise; }
		/// Get the barycentric noise
	NV_FORCE_INLINE const NvCo::PodBuffer<gfsdk_float2>& getBarycentricNoise() const { return m_barycentricNoise; }

		/// Ctor
	virtual ~ApiGlobal() {}

protected:
	
	NvCo::ApiDevice m_apiDevice;
	NvCo::ApiContext m_apiContext;

	NvCo::UniquePtr<SliSystem> m_sliSystem;
	NvCo::UniquePtr<LineRenderer> m_lineRenderer;

	NvCo::UniquePtr<ShaderCache> m_shaderCache;

private:
	NvCo::PodBuffer<float> m_scalarNoiseLut;
	NvCo::PodBuffer<gfsdk_float3> m_vectorNoise;
	NvCo::PodBuffer<gfsdk_float2> m_barycentricNoise;
};

/*! Helper class that will save and restore render state on the ApiGlobal */
class ScopeRenderState
{
public:
	ScopeRenderState(ApiGlobal* glob):m_glob(glob) {glob->saveRenderState(); }
	~ScopeRenderState() { m_glob->restoreRenderState(); }
	ApiGlobal* m_glob;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_API_GLOBAL_H
