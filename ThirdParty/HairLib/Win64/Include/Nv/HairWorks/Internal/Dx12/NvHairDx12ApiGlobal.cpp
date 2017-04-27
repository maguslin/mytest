/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx12ApiGlobal.h"

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/NvCoUniquePtr.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>
#include <Nv/Common/NvCoLogger.h>
#include <Nv/Common/NvCoString.h>

#include <float.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include "NvHairDx12LineRenderer.h"

#include <Nv/HairWorks/Internal/Dx/NvHairDxUtil.h>

#include "NvHairDx12ApiAsset.h"
#include "NvHairDx12ApiInstance.h"

//#include "NvHairDx12SliSystem.h"
#include "NvHairDx12MsaaComposer.h"

#include <Nv/HairWorks/Internal/NvHairSphereCalculator.h>

#include <Nv/HairWorks/Internal/NvHairSdkImpl.h>
#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ShaderCacheEntry.h>

#include <Nv/HairWorks/Internal/NvHairSdkImpl.h>

#include "d3dx12.h"

#pragma comment(lib, "d3d12.lib")

//////////////////////////////////////////////////////////////////////////////////////////////////////////

NV_EXTERN_C NV_DLL_EXPORT NvHair::Sdk* NvHairWorks_Create(NvUInt32 version, NvCo::MemoryAllocator* allocator, NvCo::Logger* logger, int debugMode)
{
	using namespace NvHair;
	if (version != NV_HAIR_VERSION)
	{
		return NV_NULL;
	}
	if (allocator)
	{
		NvCo::MemoryAllocator::setInstance(allocator);
	}
	if (logger)
	{
		NvCo::Logger::setInstance(logger);
	}
	Dx12ApiGlobal* apiGlobal = new NvHair::Dx12ApiGlobal;
	SdkImpl* sdk = new SdkImpl(apiGlobal, debugMode);

	apiGlobal->m_sdk = sdk;
	return sdk;
}

namespace nvidia {
namespace HairWorks { 

Dx12ApiGlobal::Dx12ApiGlobal()
{
	m_device = NV_NULL;
	m_commandList = NV_NULL;
	m_hasRenderState = false;
	m_shaderCacheFactory = new Dx12ShaderCacheFactory;
	m_shaderCache = new ShaderCache(m_shaderCacheFactory);

	m_depthOp = DepthOp::UNKNOWN;
	m_sdk = NV_NULL;
}

Dx12ApiGlobal::~Dx12ApiGlobal()
{
	NV_CORE_ASSERT(m_hasRenderState == false);
	if (m_hasRenderState)
	{
		restoreRenderState();
	}
}

Result Dx12ApiGlobal::init(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(contextIn)

	m_nullTextureSrvCpuHandle.ptr = 0;
	
	ID3D12Device* device = NvCo::Dx12Type::cast<ID3D12Device>(deviceIn);
	ID3D12GraphicsCommandList* commandList = NvCo::Dx12Type::cast<ID3D12GraphicsCommandList>(contextIn);
	const Dx12InitInfo* initInfo = Dx12SdkType::cast<Dx12InitInfo>(other);

	if (!device || !commandList || !initInfo)
	{
		NV_CO_LOG_ERROR("Expecting Dx12 device/command list, and other set to NvHair::Dx12InitInfo");
		return NV_FAIL;
	}

	m_initInfo = *static_cast<const Dx12InitInfo*>(initInfo);

	m_device = device;
	m_commandList = commandList;

	m_apiDevice = deviceIn;
	m_apiContext = contextIn;

	m_sampleCountMsaa = 0;

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS options;
		device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
		m_resourceBindingTier = options.ResourceBindingTier;
	}

	NV_RETURN_ON_FAIL(m_fence.init(device));

	// shared cbv, srv, uav heap
	NV_RETURN_ON_FAIL(m_viewHeap.init(m_device, 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
	// Initialize the sampler descriptor heap
	NV_RETURN_ON_FAIL(m_samplerHeap.init(device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
	// Initialize the upload heap
	NV_RETURN_ON_FAIL(m_resourceScopeManager.init(device, &m_fence));

	const Dx12Info info = getInfo(&m_viewHeap);

	// Initialize the dynamic descriptor heap
	NV_RETURN_ON_FAIL(m_viewCache.init(device, 1024, 20, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, &m_fence));

	if (m_resourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.PlaneSlice = 0;
		desc.Texture2D.ResourceMinLODClamp = 0.0f;

		const Int srvIndex = m_viewHeap.allocate();

		m_nullTextureSrvCpuHandle = m_viewHeap.getCpuHandle(srvIndex);
		// Copy to the descriptor
		device->CreateShaderResourceView(NV_NULL, &desc, m_nullTextureSrvCpuHandle);
	}

	{
		NvCo::Dx12CircularResourceHeap::Desc desc;
		desc.init();
		desc.m_blockSize = 256 * 1024;
		NV_RETURN_ON_FAIL(m_uploadHeap.init(device, desc, &m_fence));
	}

	{
		NvCo::Dx12CircularResourceHeap::Desc desc;
		desc.init();
		desc.m_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		desc.m_blockSize = 32 * 1024;
		// Readback can only be in this state
		desc.m_initialState = D3D12_RESOURCE_STATE_COPY_DEST;		
		NV_RETURN_ON_FAIL(m_readBackHeap.init(device, desc, &m_fence));
	}

#if 0
	m_sliSystem = Dx11SliSystem::create(m_device);
#endif

	// Set up the line renderer
	{
		Dx12LineRenderer* lineRenderer = new Dx12LineRenderer;
		m_lineRenderer = lineRenderer;

		Result res = lineRenderer->init(m_apiDevice, m_apiContext, m_initInfo.m_targetInfo);
		if (NV_FAILED(res))
		{
			NV_CO_LOG_ERROR("Unable to init line renderer");
			return res;
		}
	}

	NV_RETURN_ON_FAIL(_createShaders());

	{
		NV_RETURN_ON_FAIL(_createVisualizationVertexBuffers(info));
		NV_RETURN_ON_FAIL(_createNoiseBuffers(info));
	}

	{
		NvCo::PodBuffer<UInt64> zeroed(STREAM_OUT_POSITION_BUFFER_SIZE / sizeof(UInt64));
		zeroed.zero();
		{
			CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
			D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(STREAM_OUT_POSITION_BUFFER_SIZE));
			m_streamOutClearBuffer.init(info, defaultHeapProps, resourceDesc, D3D12_RESOURCE_STATE_COPY_SOURCE, zeroed.begin());
		}
	}

	return NV_OK;
}

Bool Dx12ApiGlobal::canRender() 
{
	return m_commandList && m_device;
}

Result Dx12ApiGlobal::bindAsset(Asset& asset)
{
	if (m_device == NV_NULL)
	{
		// If there is no device -> then I can't bind.
		asset.m_apiAsset.setNull();
		return NV_OK;
	}
	Dx12ApiAsset* apiAsset = static_cast<Dx12ApiAsset*>(asset.m_apiAsset.get());
	if (!apiAsset)
	{
		apiAsset = new Dx12ApiAsset;
		asset.m_apiAsset = apiAsset;
	}
	// Initialize it
	Result res = apiAsset->init(this, &asset);
	if (NV_FAILED(res))
	{
		asset.m_apiAsset.setNull();
	}
	return res;
}

Result Dx12ApiGlobal::bindInstance(Instance& inst)
{
	if (m_device == NV_NULL)
	{
		// If there is no device -> then I can't bind.
		inst.m_apiInstance.setNull();
		return NV_OK;
	}
	// Set up Dx11 bit
	Dx12ApiInstance* apiInst = static_cast<Dx12ApiInstance*>(inst.m_apiInstance.get());
	if (!apiInst)
	{
		apiInst = new Dx12ApiInstance;
		inst.m_apiInstance = apiInst;
	}	
	return apiInst->init(this, inst);
}

Result Dx12ApiGlobal::setContext(const NvCo::ApiContext& contextIn)
{
	ID3D12GraphicsCommandList* commandList = NvCo::Dx12Type::cast<ID3D12GraphicsCommandList>(contextIn);
	if (!commandList)
	{
		return NV_FAIL;
	}
	
	NV_CORE_ASSERT(commandList);

	m_apiContext = contextIn;
	m_commandList = commandList;
	return NV_OK;
}

MsaaComposer* Dx12ApiGlobal::createMsaaComposer()
{
	NvCo::UniquePtr<Dx12MsaaComposer> composer(new Dx12MsaaComposer);
	if (NV_FAILED(composer->init(m_device, this)))
	{
		return NV_NULL;
	}
	return composer.detach();
}

// These are the result of running fxc with the /Fh option.  The NVHairLibShaders project runs fxc in custom build steps.
namespace HairPrepareInterpolateBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairPrepareInterpolate_Cs.h>
}

namespace HairSimulateBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSimulate_Cs.h>
}

namespace HairSimulateInteractionBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSimulateInteraction_Cs.h>
}

namespace HairSimulatePinBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSimulatePin_Cs.h>
}

namespace HairSimulatePinComBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSimulatePinCom_Cs.h>
}

namespace HairSimulatePinComGatherBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSimulatePinComGather_Cs.h>
}

namespace ComputeStatsBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairComputeStats_Cs.h>
}

namespace HairDebugBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebug_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebug_Ps.h>
}

namespace HairDebugCvBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugCv_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugCv_Ps.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugCv_Gs.h>
}

namespace HairDebugInteractionBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugInteraction_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugInteraction_Ps.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugInteraction_Gs.h>
}

namespace HairSplineBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSplineVertex_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSplineGeometry_Gs.h>
}

namespace InterpolateBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateVertex_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateDomain_Ds.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateHull_Hs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateGeometry_Gs.h>
}

namespace InterpolateCubeMapBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateCubeMapGeometry_Gs.h>
}

namespace HairWorksShaderBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairShaderPixel_Ps.h>
}

namespace HairWorksShadowBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairShadowPixel_Ps.h>
}

namespace BodyDebugBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairBodyDebug_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairBodyDebug_Ps.h>
}

namespace VisualizeFrameBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeFrame_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeFrame_Ps.h>
}

namespace VisualizeLocalPosBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeLocalPos_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeLocalPos_Ps.h>
}

namespace VisualizeNormalBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeNormal_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizeNormal_Ps.h>
}

namespace VisualizePinBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizePin_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairVisualizePin_Ps.h>
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Shader source (compiled runtime)
//////////////////////////////////////////////////////////////////////////////////////////////
namespace HairInterpolateDomainShader
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateDomain.cpp>
}

Result Dx12ApiGlobal::_createShaders()
{
	ID3D12Device* device = m_device;

	const NvCo::Dx12TargetInfo& targetInfo = m_initInfo.m_targetInfo;

	// compute shaders
	{
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 18, 8, 1);
		NV_RETURN_ON_FAIL(m_hairSimulate.initCompute(device, descripDesc, HairSimulateBlobs::g_cs_main));
	}

	{
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 4, 1, 0);
		NV_RETURN_ON_FAIL(m_hairSimulateInteraction.initCompute(device, descripDesc, HairSimulateInteractionBlobs::g_cs_main));
	}

	{
		// 3 uavs, 4 srvs
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 4, 3, 0);

		NV_RETURN_ON_FAIL(m_hairPrepareInterpolate.initCompute(device, descripDesc, HairPrepareInterpolateBlobs::g_cs_main));
	}

	{
		// cbuffer cbPerFrame								: register(b0)
		// Buffer<float4>	g_restMasterStrand				: register(t0);
		// RWStructuredBuffer<float4>	g_particlePositions	: register(u0); // particle positions, for the  solver 
		// RWStructuredBuffer<NvHair_Pin> g_pinBuffer		: register(u1);
		// RWStructuredBuffer<float> g_particleLuminances	: register(u2);
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 1, 3, 0);
		NV_RETURN_ON_FAIL(m_hairSimulatePinPass.initCompute(device, descripDesc, HairSimulatePinBlobs::g_cs_main));
	}

	{
		// cbuffer cbPerFrame : register(b0)
		// Buffer<float4>	g_restMasterStrand				: register(t0);
		// RWStructuredBuffer<float4>	g_particlePositions	: register(u0); // particle positions, for the  solver 
		// RWStructuredBuffer<NvHair_PinScratchData>	g_globalScratchMem : register(u1); // global scratch mem
		// RWStructuredBuffer<NvHair_Pin> g_pinBuffer : register(u2);
		// RWStructuredBuffer<float4>	g_masterStrandTangents: register(u3);

		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 1, 4, 0);
		NV_RETURN_ON_FAIL(m_hairSimulatePinComPass.initCompute(device, descripDesc, HairSimulatePinComBlobs::g_cs_main));
	}

	{
		// cbuffer cbPerFrame : register(b0)
		// RWStructuredBuffer<NvHair_PinScratchData>	g_globalScratchMem : register(u0); // global scratch mem
		// RWStructuredBuffer<NvHair_Pin>	g_pinBuffer : register(u1);

		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 0, 2, 0);
		NV_RETURN_ON_FAIL(m_hairSimulatePinComGatherPass.initCompute(device, descripDesc, HairSimulatePinComGatherBlobs::g_cs_main));
	}

	{
		// cbuffer cbPerFrame : register(b0)		
		// Buffer<float2>				g_faceTexCoords : register(t0);
		// Texture2D		            g_densityTexture : register(t1);
		// Buffer<float>				g_noiseLut : register(t2);
		// Buffer<float2>				g_strandCoordinatesLut : register(t3);
		// RWBuffer<float4> g_stats : register(u0);
		// SamplerState samLinear;

		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 4, 1, 1);

		NV_RETURN_ON_FAIL(m_computeStats.initCompute(device, descripDesc, ComputeStatsBlobs::g_cs_main));
	}

	// geometry shader for spline control hairs
	{
		Dx12ShaderPass& pass = m_passes[PASS_SPLINE];
		
		Dx12ShaderPass::Desc desc;

		desc.m_vertexBlob = Dx12ShaderPass::Blob(HairSplineBlobs::g_vs_main);
		desc.m_geometryBlob = Dx12ShaderPass::Blob(HairSplineBlobs::g_gs_main);

		const UINT soStrides[] = 
		{
			UINT(sizeof(float) * 4),
			UINT(sizeof(float) * 4),
			UINT(sizeof(float) * 4),
		};

		D3D12_SO_DECLARATION_ENTRY soDecls[] =
		{
			{ 0, "POSITION", 0, 0, 4, 0},
			{ 0, "TANGENT", 0, 0, 4, 1},
			{ 0, "NORMAL", 0, 0, 4, 2},
		};
		D3D12_STREAM_OUTPUT_DESC streamOutputDesc = { soDecls, NV_COUNT_OF(soDecls), soStrides, NV_COUNT_OF(soStrides), D3D12_SO_NO_RASTERIZED_STREAM };
		desc.m_streamOutputDesc = streamOutputDesc;
	
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 4, 0, 0);

		NV_RETURN_ON_FAIL(pass.initStreamOut(device, descripDesc, desc));
	}
	// shaders for hair interpolation
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_vs_main);
		desc.m_hullBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_hs_main);
		desc.m_domainBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_ds_main);
		desc.m_geometryBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_gs_main);
		desc.m_pixelBlob = Dx12ShaderPass::Blob(HairWorksShaderBlobs::g_ps_main);
		desc.m_targetInfo = targetInfo;
		
		Dx12ShaderPass::DescriptorDesc descripDesc;
		descripDesc.setNumCbvSrvUavSamplers(1, 17, 0, 2);

		Dx12ShaderPass::DescriptorDesc optionalDesc;
		optionalDesc.m_hasDynamicConstantBuffer = true;

		NV_RETURN_ON_FAIL(m_passes[PASS_INTERPOLATE].initPatch(device, descripDesc, desc, &optionalDesc));
	}
		
	// Set pixel shader 0 to be the default
	{
		Dx12PixelShaderInfo& pixelShader = m_simplePixelShader;

		pixelShader.m_targetInfo = targetInfo;
		
		pixelShader.m_hasDynamicConstantBuffer = true;				// Has a dynamic constant buffer
		pixelShader.m_numSrvs = 0;
		pixelShader.m_pixelBlob = HairWorksShaderBlobs::g_ps_main;
		pixelShader.m_pixelBlobSize = NV_COUNT_OF(HairWorksShaderBlobs::g_ps_main);

		m_pixelShaders.setSize(1);
		m_pixelShaders[0] = pixelShader;
	}

	// shader for guide rendering
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = HairDebugBlobs::g_vs_main;
		desc.m_pixelBlob = HairDebugBlobs::g_ps_main;
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		desc.m_targetInfo = targetInfo;

		/* 	const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		}; */

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
		NV_RETURN_ON_FAIL(m_passes[PASS_DEBUG_GUIDE].init(device, &range, 1, desc));
	}
	// shader for cv rendering
	{
		Dx12ShaderPass::Desc desc;

		desc.m_vertexBlob = HairDebugCvBlobs::g_vs_main;
		desc.m_pixelBlob = HairDebugCvBlobs::g_ps_main;
		desc.m_geometryBlob = HairDebugCvBlobs::g_gs_main;
		desc.m_targetInfo = targetInfo;
		
		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_DEBUG_CV].init(device, &range, 1, desc));
	}
	// shader for pin constraint rendering
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = VisualizePinBlobs::g_vs_main;
		desc.m_pixelBlob = VisualizePinBlobs::g_ps_main;

		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		desc.m_targetInfo = targetInfo;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_VISUALIZE_PIN].init(device, &range, 1, desc));
	}
	// shader for hair interaction rendering
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = HairDebugInteractionBlobs::g_vs_main;
		desc.m_pixelBlob = HairDebugInteractionBlobs::g_ps_main;
		desc.m_geometryBlob = HairDebugInteractionBlobs::g_gs_main;
		desc.m_targetInfo = targetInfo;
		
		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
	
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_DEBUG_INTERACTION].init(device, &range, 1, desc));
	}
	// shader for growth mesh debug rendering
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = BodyDebugBlobs::g_vs_main;
		//desc.m_geometryBlob = BodyDebugBlobs::g_gs_main;
		desc.m_pixelBlob = BodyDebugBlobs::g_ps_main;
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.m_targetInfo = targetInfo;
		
		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		NV_RETURN_ON_FAIL(m_passes[PASS_BODY_DEBUG].init(device, &range, 1, desc));
	}
	// shader for frame visualization
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = VisualizeFrameBlobs::g_vs_main;
		desc.m_pixelBlob = VisualizeFrameBlobs::g_ps_main;
		desc.m_targetInfo = targetInfo;
		
		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_VISUALIZE_FRAME].init(device, &range, 1, desc));
	}
	// shader for local pos visualization
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = VisualizeLocalPosBlobs::g_vs_main;
		desc.m_pixelBlob = VisualizeLocalPosBlobs::g_ps_main;
		desc.m_targetInfo = targetInfo;
		
		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		desc.m_targetInfo = targetInfo;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_VISUALIZE_LOCAL_POS].init(device, &range, 1, desc));
	}
	// shader for normal visualization
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = VisualizeNormalBlobs::g_vs_main;
		desc.m_pixelBlob = VisualizeNormalBlobs::g_ps_main;
		desc.m_targetInfo = targetInfo;

		D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		desc.m_inputElementDescs = layout;
		desc.m_numInputElementDescs = NV_COUNT_OF(layout);
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

		CD3DX12_DESCRIPTOR_RANGE range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);

		NV_RETURN_ON_FAIL(m_passes[PASS_VISUALIZE_NORMAL].init(device, &range, 1, desc));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// create samplers

	// linear sampler
	{
		m_linearSamplerIndex = m_samplerHeap.allocate();
		D3D12_SAMPLER_DESC desc =
		{
			D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			0.0f, 0, D3D12_COMPARISON_FUNC_NEVER,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, D3D12_FLOAT32_MAX,
		};
		device->CreateSampler(&desc, m_samplerHeap.getCpuHandle(m_linearSamplerIndex));
	}
	// Point clamp sampler
	{
		m_pointClampSamplerIndex = m_samplerHeap.allocate();
		D3D12_SAMPLER_DESC desc =
		{
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			0.0f, 0, D3D12_COMPARISON_FUNC_NEVER,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, D3D12_FLOAT32_MAX,
		};
		device->CreateSampler(&desc, m_samplerHeap.getCpuHandle(m_pointClampSamplerIndex));
	}

	return NV_OK;
}

/* static */Void Dx12ApiGlobal::initInterpolate(Dx12ShaderPass::Desc& desc, Bool useCubeMap, Dx12ShaderPass::DescriptorDesc& descripDesc)
{
	desc.m_vertexBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_vs_main);
	desc.m_hullBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_hs_main);
	desc.m_domainBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_ds_main);
	if (useCubeMap)
	{
		desc.m_geometryBlob = Dx12ShaderPass::Blob(InterpolateCubeMapBlobs::g_gs_main);
	}
	else
	{
		desc.m_geometryBlob = Dx12ShaderPass::Blob(InterpolateBlobs::g_gs_main);
	}
	desc.m_pixelBlob = Dx12ShaderPass::Blob(HairWorksShaderBlobs::g_ps_main);
	
	descripDesc.setNumCbvSrvUavSamplers(1, 18, 0, 2);
	//? is this right?
	//descripDesc.setNumCustomCbvSrv(1, 0);
}



Void Dx12ApiGlobal::saveRenderState()
{
	NV_CORE_ASSERT(m_hasRenderState == false);
	m_hasRenderState = true;
	m_depthOp = DepthOp::UNKNOWN;

	updateCompleted();
}

Void Dx12ApiGlobal::restoreRenderState()
{
	NV_CORE_ASSERT(m_hasRenderState == true);
	m_hasRenderState = false;
	m_depthOp = DepthOp::UNKNOWN;
}

Void Dx12ApiGlobal::setDepthOp(EDepthOp depthOp)
{
	m_depthOp = depthOp;
}

Void Dx12ApiGlobal::updateCompleted()
{
	const UInt64 completedValue = m_fence.getCompletedValue();

	{
		NvCo::Dx12Async* end = m_asyncManager.updateCompleted(completedValue);
		NvCo::Dx12Async* async = m_asyncManager.getStart(NvCo::Dx12Async::STATE_COMPLETED);
		while (async != end)
		{
			onAsyncCompleted(async);
			async = async->m_next;
		}
	}

	m_uploadHeap.updateCompleted();
	m_resourceScopeManager.updateCompleted();
	m_viewCache.updateCompleted();
	m_readBackHeap.updateCompleted();
}

Void Dx12ApiGlobal::setRasterizerMode(RasterizerMode mode)
{
	NV_UNUSED(mode)
}

void Dx12ApiGlobal::applySamplers(bool useVs, bool useHs, bool useDs, bool useGs, bool usePs)
{
	NV_UNUSED(useVs)
	NV_UNUSED(useHs)
	NV_UNUSED(useDs)
	NV_UNUSED(useGs)
	NV_UNUSED(usePs)
}

Result Dx12ApiGlobal::_createVisualizationVertexBuffers(const Dx12Info& info)
{
	SphereCalculator calc;
	calc.calc(SphereCalculator::TYPE_LINE, 1.0f, 10);
	const Int numVerts = Int(calc.m_indices.getSize());
	const Int* indices = calc.m_indices.begin();
	const gfsdk_float3* srcVerts = calc.m_points.begin();
	// Buffer is float4s
	NvCo::PodBuffer<gfsdk_float4> buf(numVerts);
	for (Int i = 0; i < numVerts; i++) 
	{
		buf[i] = gfsdk_makeFloat4(srcVerts[indices[i]]);
	}
	m_numSphereLines = numVerts / 2;
	NV_RETURN_ON_FAIL(m_sphereVertexBuffer.init(info, sizeof(gfsdk_float4), m_numSphereLines * 2, buf));
	return NV_OK;
}

Result Dx12ApiGlobal::_createNoiseBuffers(const Dx12Info& info)
{
	NV_RETURN_ON_FAIL(m_scalarNoiseBuffer.init(info, STRAND_NOISE_TABLE_SIZE, getScalarNoise()));
	NV_RETURN_ON_FAIL(m_vectorNoiseBuffer.init(info, STRAND_NOISE_TABLE_SIZE, getVectorNoise()));
	// create resource for strand coord LUT
	NV_RETURN_ON_FAIL(m_strandCoordinatesBuffer.init(info, STRAND_NOISE_TABLE_SIZE, getBarycentricNoise()));
	return NV_OK;
}

Result Dx12ApiGlobal::calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut)
{
	m_viewPort = viewport;
	DxUtil::calcViewInfo(viewport, view, projection, fov, viewInfoOut);
	return NV_OK;
}

Bool Dx12ApiGlobal::areEqual(const NvCo::ApiHandle& a, const NvCo::ApiHandle& b)
{
	if (a.isNull() && b.isNull())
	{
		return true;
	}
	if ((a.isNull() || b.isNull()) || a.m_type != b.m_type)
	{
		return false;
	}
	if (a.getApiType(a.m_type) != NvCo::ApiType::DX12)
	{
		return false;
	}
	// We just need to compare the handles
	return a.m_handle == b.m_handle;
}

Void Dx12ApiGlobal::onGpuWorkSubmitted(const NvCo::ApiHandle& handle)
{
	// Update completed state
	updateCompleted();

	ID3D12CommandQueue* commandQueue = NvCo::Dx12Type::cast<ID3D12CommandQueue>(handle);
	NV_CORE_ASSERT(commandQueue);
	if (!commandQueue)
	{
		NV_CO_LOG_ERROR("Must pass a ID3D12CommandQueue to onGpuWorkSubmitted");
		return;
	}

	const UInt64 signalValue = m_fence.nextSignal(commandQueue);

	m_asyncManager.addSync(signalValue);

	// Sync on main circular resource heap
	m_uploadHeap.addSync(signalValue);
	m_resourceScopeManager.addSync(signalValue);
	m_viewCache.addSync(signalValue);
	m_readBackHeap.addSync(signalValue);

	// Sync on the line renderer
	if (m_lineRenderer)
	{
		m_lineRenderer->onGpuWorkSubmitted(handle);
	}
}

Void Dx12ApiGlobal::onInstanceDestroyed(InstanceId instanceId)
{
	IndexT numRemoved = m_asyncManager.onOwnerDestroyed((Void*)instanceId);
	if (numRemoved)
	{
		NvCo::String buf;
		buf << "There were " << Int(numRemoved) << " pending request(s) on instance [" << instanceId << "]";
		NV_CO_LOG_WARN(buf.getCstr());
	}
}

Result Dx12ApiGlobal::setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfoIn)
{
	NV_CORE_ASSERT(shaderIndex >= 0);

	if (shaderIndex >= m_pixelShaders.getSize())
	{
		m_pixelShaders.setSize(shaderIndex + 1);
	}

	MemoryDx12PixelShaderInfo& dstInfo = m_pixelShaders[shaderIndex];
	if (dstInfo.isInitialized())
	{
		// Okay.. we'll need to go through the cache and destroy all entries that use this index
		const NvCo::Array<ShaderCacheEntry*>& entries = m_shaderCache->getEntries();
		const IndexT numEntries = entries.getSize();
		for (IndexT i = 0; i < numEntries; i++)
		{
			ShaderCacheEntry* entry = entries[i];
			if (entry)
			{
				Dx12ShaderCacheEntry* dxEntry = static_cast<Dx12ShaderCacheEntry*>(entry);
				// If it has something in the cache for this, reset it so it will have to be reset
				if (dxEntry->m_passes.getSize() > shaderIndex)
				{
					Dx12ShaderPass& pass = dxEntry->m_passes[shaderIndex];
					pass.reset();
				}
			}
		}
	}

	// Okay we've removed it if it was there.. we are done
	if (pixelShaderInfoIn.isNull())
	{
		return NV_OK;
	}
	const Dx12PixelShaderInfo* pixelShader = Dx12SdkType::cast<Dx12PixelShaderInfo>(pixelShaderInfoIn);
	if (!pixelShader)
	{	
		NV_CORE_ASSERT("Must be passed a Dx12PixelShaderInfo");
		NV_CO_LOG_ERROR("setPixelShader must be passed a Dx12PixelShaderInfo");
		return NV_FAIL;
	}

	dstInfo = *pixelShader;
	return NV_OK;
}

Result Dx12ApiGlobal::getPixelShader(Int shaderIndex, const NvCo::ApiPtr& pixelShaderInfoOut)
{
	Dx12PixelShaderInfo* infoOut = Dx12SdkType::cast<Dx12PixelShaderInfo>(pixelShaderInfoOut);
	if (!infoOut)
	{
		NV_CORE_ASSERT("Must be passed a Dx12PixelShaderInfo");
		NV_CO_LOG_ERROR("setPixelShader must be passed a Dx12PixelShaderInfo");
		return NV_FAIL;
	}
	NV_CORE_ASSERT(shaderIndex >= 0);
	if (shaderIndex >= 0 && shaderIndex < m_pixelShaders.getSize())
	{
		const MemoryDx12PixelShaderInfo& info = m_pixelShaders[shaderIndex];
		if (info.isInitialized())
		{
			*infoOut = info;
			return NV_OK;
		}
	}
	return NV_E_MISC_UNINITIALIZED;
}

Void Dx12ApiGlobal::onAsyncCompleted(NvCo::Dx12Async* async)
{
	// Handle final completion
	InstanceId instId = InstanceId(SizeT(async->m_owner));
	Instance* inst = m_sdk->getInstance(instId);
	Dx12ApiInstance* apiInst = static_cast<Dx12ApiInstance*>(inst->m_apiInstance.get());
	switch (async->m_type)
	{
		case Dx12AsyncType::COMPUTE_STATS:
		{
			apiInst->_completeComputeStats(*static_cast<Dx12ComputeStatsAsync*>(async));
			break;
		}
		case Dx12AsyncType::GET_PIN_MATRICES:
		{
			apiInst->_completeGetPinMatrix(*static_cast<Dx12GetPinMatrixAsync*>(async));
			break;
		}
		default: break;
	}
}

Int Dx12ApiGlobal::cancelAsync(const NvCo::HandleMapHandle* handles, Int numHandles, Bool allReferences)
{
	return Int(m_asyncManager.cancel(handles, numHandles, allReferences));
}

Int Dx12ApiGlobal::cancelAsync(InstanceId instId, Bool allReferences)
{
	return Int(m_asyncManager.cancel((Void*)instId, allReferences));
}


} // namespace HairWorks 
} // namespace nvidia 