/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_API_GLOBAL_H
#define NV_HAIR_DX12_API_GLOBAL_H

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoPodBuffer.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12CounterFence.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12CircularResourceHeap.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12ResourceScopeManager.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12DescriptorCache.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12AsyncManager.h>

#include <Nv/HairWorks/Internal/NvHairApiGlobal.h>

#include <Nv/HairWorks/Platform/Dx12/NvHairDx12SdkHandle.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12DescriptorHeap.h>

#include "NvHairDx12Buffer.h"

#include "NvHairDx12ShaderPass.h"
#include "NvHairDx12AsyncTypes.h"

namespace nvidia {
namespace HairWorks { 

class SdkImpl;

class Dx12ApiGlobal: public ApiGlobal
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12ApiGlobal, ApiGlobal);

	enum { STRAND_NOISE_TABLE_SIZE = 1024 };
		/// The total size of the stream out _position/offset_ buffer
	enum { STREAM_OUT_POSITION_BUFFER_SIZE = sizeof(UInt64) * 3 * 64 };

	enum PassType
	{
		PASS_SPLINE,

		PASS_INTERPOLATE,
		
		PASS_DEBUG_GUIDE,
		PASS_DEBUG_CV,
		PASS_DEBUG_INTERACTION,
		PASS_BODY_DEBUG,

		PASS_VISUALIZE_FRAME,
		PASS_VISUALIZE_LOCAL_POS,
		PASS_VISUALIZE_NORMAL,
		PASS_VISUALIZE_PIN,
		PASS_HAIR_PARTICLE,  // NUTT

		PASS_COUNT_OF,
	};

		/// ApiGlobal
	virtual Result bindAsset(Asset& asset) NV_OVERRIDE;
	virtual Result bindInstance(Instance& inst) NV_OVERRIDE;
	virtual Result setContext(const NvCo::ApiContext& context) NV_OVERRIDE;
	virtual MsaaComposer* createMsaaComposer() NV_OVERRIDE;
	virtual Result init(const NvCo::ApiDevice& device, const NvCo::ApiContext& context, const NvCo::ConstApiPtr& other) NV_OVERRIDE;
	virtual Void saveRenderState() NV_OVERRIDE;
	virtual Void restoreRenderState() NV_OVERRIDE;
	virtual Void setRasterizerMode(RasterizerMode mode) NV_OVERRIDE;
	virtual Void setDepthOp(EDepthOp depthOp) NV_OVERRIDE;
	virtual Void applySamplers(bool useVs, bool useHs, bool useDs, bool useGs, bool usePs) NV_OVERRIDE;
	virtual Bool canRender() NV_OVERRIDE;
	virtual Result calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut) NV_OVERRIDE;
	virtual Bool areEqual(const NvCo::ApiHandle& a, const NvCo::ApiHandle& b) NV_OVERRIDE;
	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) NV_OVERRIDE;
	virtual Result setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo) NV_OVERRIDE;
	virtual Result getPixelShader(Int shaderIndex, const NvCo::ApiPtr& pixelShaderInfoOut) NV_OVERRIDE;
	virtual Int cancelAsync(const NvCo::HandleMapHandle* handles, Int numHandles, Bool allReferences) NV_OVERRIDE;
	virtual Int cancelAsync(InstanceId instId, Bool allReferences) NV_OVERRIDE;
	virtual Void updateCompleted() NV_OVERRIDE;

		/// Get the device
	NV_FORCE_INLINE ID3D12Device* getDevice() const { return m_device; }
		/// Get the context
	NV_FORCE_INLINE ID3D12GraphicsCommandList* getCommandList() const { return m_commandList; }

	Dx12Info getInfo(NvCo::Dx12DescriptorHeap* heap)
	{
		Dx12Info info;
		info.m_viewHeap = heap;
		info.m_commandList = m_commandList;
		info.m_device = m_device;
		info.m_resourceScopeManager = &m_resourceScopeManager;
		return info;
	}
		/// Called when an instance is destroyed
	Void onInstanceDestroyed(InstanceId instanceId);
	Void onAsyncCompleted(NvCo::Dx12Async* async);

		/// Ctor
	Dx12ApiGlobal();
		/// Dtor
	virtual ~Dx12ApiGlobal();

		/// Initialize as interpolate (PASS_INTERPOLATE like), which is used for rendering
	static Void initInterpolate(Dx12ShaderPass::Desc& desc, Bool useCubeMap, Dx12ShaderPass::DescriptorDesc& descriptorDesc);

protected:

	struct MemoryDx12PixelShaderInfo: public Dx12PixelShaderInfo
	{
		typedef Dx12PixelShaderInfo Parent;
		typedef MemoryDx12PixelShaderInfo ThisType;

			/// Default Ctor
		MemoryDx12PixelShaderInfo():Parent() {}
			/// Copy Ctor
		MemoryDx12PixelShaderInfo(const Parent& rhs):
			Parent(rhs)
		{
			m_pixelBlobArray.set(rhs.m_pixelBlob, IndexT(rhs.m_pixelBlobSize));
			m_pixelBlob = m_pixelBlobArray.begin();
		}
			/// Assignment
		ThisType& operator=(const Parent& rhs)
		{
			if (this != &rhs)
			{
				Parent::operator=(rhs);
				m_pixelBlobArray.set(rhs.m_pixelBlob, IndexT(rhs.m_pixelBlobSize));
				m_pixelBlob = m_pixelBlobArray.begin();
			}
			return *this;
		}
		// Memory storage for the blob (if needed)
		NvCo::Array<UInt8> m_pixelBlobArray;
	};

	Result _createShaders();

	Result _createVisualizationVertexBuffers(const Dx12Info& info);
	
	Result _createNoiseBuffers(const Dx12Info& info);
	
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_commandList;

public:
	SdkImpl* m_sdk;

	D3D12_RESOURCE_BINDING_TIER m_resourceBindingTier;
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullTextureSrvCpuHandle;

	NvCo::Dx12DescriptorHeap m_nullViewHeap;	///< Cbv, Srv, Uav 
	NvCo::Dx12DescriptorHeap m_viewHeap;		///< Cbv, Srv, Uav 
	NvCo::Dx12ResourceScopeManager m_resourceScopeManager;

	Dx12ShaderPass m_passes[PASS_COUNT_OF];

	Dx12ShaderPass m_hairSimulate;
	Dx12ShaderPass m_hairSimulateInteraction;
	Dx12ShaderPass m_hairPrepareInterpolate;
	Dx12ShaderPass m_hairSimulatePinPass;
	Dx12ShaderPass m_hairSimulatePinComPass;
	Dx12ShaderPass m_hairSimulatePinComGatherPass;
	Dx12ShaderPass m_computeStats;

	NvCo::Dx12DescriptorHeap m_samplerHeap;			///< 0 is linear sampler, 1 is point clamp
	// Allocated from the sampler heap
	Int m_linearSamplerIndex;			 
	Int m_pointClampSamplerIndex;
	
	NvCo::UniquePtr<ShaderCacheFactory> m_shaderCacheFactory;

	// Pixel shaders
	NvCo::Array<MemoryDx12PixelShaderInfo> m_pixelShaders;
	Dx12PixelShaderInfo m_simplePixelShader;

	Bool m_hasRenderState;							///< True if has stored render state
	
	// bone visualization data
	Dx12VertexBuffer m_sphereVertexBuffer;			///< The vertex buffer holding a unit sphere (float4s)
	Int m_numSphereLines;							///< Two vertices for every line

	Dx12ReadOnlyBuffer m_scalarNoiseBuffer;			///< 1d noise
	Dx12ReadOnlyBuffer m_strandCoordinatesBuffer;	///< barycentric coordinate LUT (2D noise)
	Dx12ReadOnlyBuffer m_vectorNoiseBuffer;			///< wind noise table (3D vector noise)

	NvCo::Dx12CircularResourceHeap m_uploadHeap;			///< Circular resource heap (can hold dynamic constant buffers, dynamic vertex buffers, index buffers etc)	
	NvCo::Dx12DescriptorCache m_viewCache;				///< Descriptor cache, used for holding uav, cbv, srv views
	NvCo::Dx12CircularResourceHeap m_readBackHeap;		///< Used as temporary storage for data read back

	NvCo::Dx12CounterFence m_fence;						///< Fence
	Dx12InitInfo m_initInfo;						///< Initialization info

	Viewport m_viewPort;							///< View port

	UInt m_sampleCountMsaa;							///< MSAA sample count

	Dx12Resource m_streamOutClearBuffer;

	EDepthOp m_depthOp;								///< Used for tracking depthOp for debugging purposes because Dx12 can't set 'depth stencil state'

	// Handling async 
	NvCo::Dx12AsyncManager m_asyncManager;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_API_GLOBAL_H
