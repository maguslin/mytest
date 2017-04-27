/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

// this
#include "NvHairDx11ApiGlobal.h"

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include <float.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

#include "NvHairDx11LineRenderer.h"

#include "NvHairDx11Util.h"

#include "NvHairDx11ApiAsset.h"
#include "NvHairDx11ApiInstance.h"

#include "NvHairDx11SliSystem.h"
#include "NvHairDx11MsaaComposer.h"

#include <Nv/HairWorks/Internal/NvHairSphereCalculator.h>

#include <Nv/HairWorks/Internal/NvHairSdkImpl.h>
#include <Nv/HairWorks/Internal/Dx11/NvHairDx11ShaderCacheEntry.h>
#include <Nv/HairWorks/Internal/Dx/NvHairDxUtil.h>

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
	Dx11ApiGlobal* apiGlobal = new NvHair::Dx11ApiGlobal;
	return new SdkImpl(apiGlobal, debugMode);
}

namespace nvidia {
namespace HairWorks { 

Dx11ApiGlobal::Dx11ApiGlobal():
	m_device(NV_NULL),
	m_context(NV_NULL)
{
	m_hasRenderState = false;
	m_shaderCacheFactory = new Dx11ShaderCacheFactory;
	m_shaderCache = new ShaderCache(m_shaderCacheFactory);
}

Dx11ApiGlobal::~Dx11ApiGlobal()
{
	NV_CORE_ASSERT(m_hasRenderState == false);
	if (m_hasRenderState)
	{
		restoreRenderState();
	}
}

Result Dx11ApiGlobal::init(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(other)

	ID3D11Device* device = NvCo::Dx11Type::cast<ID3D11Device>(deviceIn);
	ID3D11DeviceContext* context = NvCo::Dx11Type::cast<ID3D11DeviceContext>(contextIn);

	if (!device)
	{
		NV_CO_LOG_ERROR("Expecting Dx11 device");
		return NV_FAIL;
	}

	m_device = device;
	m_context = context;

	m_apiDevice = deviceIn;
	m_apiContext = contextIn;

	m_sliSystem = Dx11SliSystem::create(m_device);

	// Set up depthOp state
	NV_RETURN_ON_FAIL(m_depthOpState.init(device));

	// Set up the line renderer
	{
		Dx11LineRenderer* lineRenderer = new Dx11LineRenderer(m_depthOpState);
		m_lineRenderer = lineRenderer;

		Result res = lineRenderer->init(m_apiDevice, m_apiContext);
		if (NV_FAILED(res))
		{
			NV_CO_LOG_ERROR("Unable to init line renderer");
			return res;
		}
	}

	NV_RETURN_ON_FAIL(_createShaders());
	NV_RETURN_ON_FAIL(_createVisualizationVertexBuffers());
	NV_RETURN_ON_FAIL(_createScalarNoiseLut());
	NV_RETURN_ON_FAIL(_createStrandBarycentricCoordinates());
	NV_RETURN_ON_FAIL(_createVectorNoiseLut());

	return NV_OK;
}

Bool Dx11ApiGlobal::canRender() 
{
	return m_device && m_context; 
}

Result Dx11ApiGlobal::bindAsset(Asset& asset) 
{
	if (m_device == NV_NULL)
	{
		// If there is no device -> then I can't bind.
		asset.m_apiAsset.setNull();
		return NV_OK;
	}

	Dx11ApiAsset* apiAsset = static_cast<Dx11ApiAsset*>(asset.m_apiAsset.get());
	if (!apiAsset)
	{
		apiAsset = new Dx11ApiAsset;
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

Result Dx11ApiGlobal::bindInstance(Instance& inst)
{
	if (m_device == NV_NULL)
	{
		// If there is no device -> then I can't bind.
		inst.m_apiInstance.setNull();
		return NV_OK;
	}
	// Set up Dx11 bit
	Dx11ApiInstance* apiInst = static_cast<Dx11ApiInstance*>(inst.m_apiInstance.get());
	if (!apiInst)
	{
		apiInst = new Dx11ApiInstance;
		inst.m_apiInstance = apiInst;
	}	
	return apiInst->init(this, inst);
}

Result Dx11ApiGlobal::setContext(const NvCo::ApiContext& contextIn)
{
	ID3D11DeviceContext* context = NvCo::Dx11Type::cast<ID3D11DeviceContext>(contextIn);
	if (!context)
	{
		return NV_FAIL;
	}
	m_apiContext = contextIn;
	m_context = context;
	return NV_OK;
}

MsaaComposer* Dx11ApiGlobal::createMsaaComposer()
{
	NvCo::UniquePtr<Dx11MsaaComposer> composer(new Dx11MsaaComposer);
	if (NV_FAILED(composer->init(m_device, m_context)))
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

static HRESULT _createComputeShader(ID3D11Device* device, const Dx11ShaderPass::Blob& blob, ID3D11ComputeShader** shaderOut)
{
	return device->CreateComputeShader(blob.m_data, blob.m_size, NV_NULL, shaderOut);
}

static HRESULT _createPixelShader(ID3D11Device* device, const Dx11ShaderPass::Blob& blob, ID3D11PixelShader** shaderOut)
{
	return device->CreatePixelShader(blob.m_data, blob.m_size, NV_NULL, shaderOut);
}

Result Dx11ApiGlobal::_createShaders()
{
	ID3D11Device* device = m_device;

	// compute shaders
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairPrepareInterpolateBlobs::g_cs_main, m_hairPrepareInterpolate.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairSimulateBlobs::g_cs_main, m_hairSimulate.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairSimulateInteractionBlobs::g_cs_main, m_hairSimulateInteraction.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairSimulatePinBlobs::g_cs_main, m_hairSimulatePinPass.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairSimulatePinComBlobs::g_cs_main, m_hairSimulatePinComPass.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, HairSimulatePinComGatherBlobs::g_cs_main, m_hairSimulatePinComGatherPass.writeRef()));
	NV_RETURN_ON_FAIL(_createComputeShader(device, ComputeStatsBlobs::g_cs_main, m_computeStats.writeRef()));

	// geometry shader for splining control hairs
	{
		Dx11ShaderPass& pass = m_passes[PASS_SPLINE];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, HairSplineBlobs::g_vs_main));
		D3D11_SO_DECLARATION_ENTRY so_decl[] =
		{
			{ 0, "POSITION", 0, 0, 4, 0},
			{ 0, "TANGENT", 0, 0, 4, 1},
			{ 0, "NORMAL", 0, 0, 4, 2},
		};
		NV_RETURN_ON_FAIL(pass.createGeometryShaderWithStreamOut(device, HairSplineBlobs::g_gs_main, so_decl, NV_COUNT_OF(so_decl)));
	}
	// shaders for hair interpolation
	{
		Dx11ShaderPass& pass = m_passes[PASS_INTERPOLATE];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, InterpolateBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createDomainShader(device, InterpolateBlobs::g_ds_main));
		NV_RETURN_ON_FAIL(pass.createHullShader(device, InterpolateBlobs::g_hs_main));
		NV_RETURN_ON_FAIL(pass.createGeometryShader(device, InterpolateBlobs::g_gs_main));
	}
	{
		Dx11ShaderPass& pass = m_passes[PASS_CUBE_MAP_INTERPOLATE];
		pass = m_passes[PASS_INTERPOLATE];
		NV_RETURN_ON_FAIL(pass.createGeometryShader(device, InterpolateCubeMapBlobs::g_gs_main));	
	}
		
	// shaders for simple hair shading
	{
		NV_RETURN_ON_FAIL(_createPixelShader(device, HairWorksShaderBlobs::g_ps_main, m_hairSimpleShader.writeRef()));
	}
	// shaders for high resolution hair shadow
	{
		NV_RETURN_ON_FAIL(_createPixelShader(device, HairWorksShadowBlobs::g_ps_main, m_hairShadowShader.writeRef()));
	}

	// shader for guide rendering
	{
		Dx11ShaderPass& pass = m_passes[PASS_DEBUG_GUIDE];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, HairDebugBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, HairDebugBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, HairDebugBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for cv rendering
	{
		Dx11ShaderPass& pass = m_passes[PASS_DEBUG_CV];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, HairDebugCvBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, HairDebugCvBlobs::g_ps_main));
		NV_RETURN_ON_FAIL(pass.createGeometryShader(device, HairDebugCvBlobs::g_gs_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, HairDebugCvBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for pin constraint rendering
	{
		Dx11ShaderPass& pass = m_passes[PASS_VISUALIZE_PIN];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, VisualizePinBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, VisualizePinBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, VisualizePinBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for hair interaction rendering
	{
		Dx11ShaderPass& pass = m_passes[PASS_DEBUG_INTERACTION];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, HairDebugInteractionBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, HairDebugInteractionBlobs::g_ps_main));
		NV_RETURN_ON_FAIL(pass.createGeometryShader(device, HairDebugInteractionBlobs::g_gs_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, HairDebugInteractionBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for growth mesh debug rendering
	{
		Dx11ShaderPass& pass = m_passes[PASS_BODY_DEBUG];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, BodyDebugBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, BodyDebugBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutBodyDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, BodyDebugBlobs::g_vs_main, layoutBodyDebug, NV_COUNT_OF(layoutBodyDebug)));
	}
	// shader for frame visualization
	{
		Dx11ShaderPass& pass = m_passes[PASS_VISUALIZE_FRAME];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, VisualizeFrameBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, VisualizeFrameBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, VisualizeFrameBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for local pos visualization
	{
		Dx11ShaderPass& pass = m_passes[PASS_VISUALIZE_LOCAL_POS];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, VisualizeLocalPosBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, VisualizeLocalPosBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, VisualizeLocalPosBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// shader for normal visualization
	{
		Dx11ShaderPass& pass = m_passes[PASS_VISUALIZE_NORMAL];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, VisualizeNormalBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, VisualizeNormalBlobs::g_ps_main));
		const D3D11_INPUT_ELEMENT_DESC layoutDebug[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(pass.createInputLayout(device, VisualizeNormalBlobs::g_vs_main, layoutDebug, NV_COUNT_OF(layoutDebug)));
	}
	// depth stencil state descriptors
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateLess.writeRef()));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateGreater.writeRef()));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateNone.writeRef()));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateTestOnlyLess.writeRef()));
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_GREATER;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateTestOnlyGreater.writeRef()));
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// rasterizer state descriptors
	{
		D3D11_RASTERIZER_DESC solidNoCullDesc;
		{
			solidNoCullDesc.FillMode = D3D11_FILL_SOLID;
			solidNoCullDesc.CullMode = D3D11_CULL_NONE;
			solidNoCullDesc.AntialiasedLineEnable = false;
			solidNoCullDesc.MultisampleEnable = true;
			solidNoCullDesc.FrontCounterClockwise = 0;
			solidNoCullDesc.DepthBias = 0;
			solidNoCullDesc.DepthBiasClamp = 0;
			solidNoCullDesc.SlopeScaledDepthBias = 0;
			solidNoCullDesc.DepthClipEnable = true;
			solidNoCullDesc.ScissorEnable = 0;
		};
		D3D11_RASTERIZER_DESC wireNoCullDesc;
		{
			wireNoCullDesc.FillMode = D3D11_FILL_WIREFRAME;
			wireNoCullDesc.CullMode = D3D11_CULL_NONE;
			wireNoCullDesc.AntialiasedLineEnable = false;
			wireNoCullDesc.MultisampleEnable = true;
			wireNoCullDesc.FrontCounterClockwise = 0;
			wireNoCullDesc.DepthBias = 0;
			wireNoCullDesc.DepthBiasClamp = 0;
			wireNoCullDesc.SlopeScaledDepthBias = 0;
			wireNoCullDesc.DepthClipEnable = true;
			wireNoCullDesc.ScissorEnable = 0;
		};
		NV_RETURN_ON_FAIL(device->CreateRasterizerState(&solidNoCullDesc, m_rasterizerStateSolid.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRasterizerState(&wireNoCullDesc, m_rasterizerStateWireFrame.writeRef()));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// create linear sampler
	{
		D3D11_SAMPLER_DESC linearSamplerDesc[1] =
		{
			D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			0.0, 0, D3D11_COMPARISON_NEVER, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, D3D11_FLOAT32_MAX,
		};
		NV_RETURN_ON_FAIL(device->CreateSamplerState(linearSamplerDesc, m_linearSampler.writeRef()));
		// create comparison sampler for PCF shadow map
		D3D11_SAMPLER_DESC comparisonSamplerDesc[1] = {
			D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_BORDER,
			D3D11_TEXTURE_ADDRESS_BORDER,
			D3D11_TEXTURE_ADDRESS_BORDER,
			0.0, 0, D3D11_COMPARISON_LESS_EQUAL,
			1e10, 1e10, 1e10, 1e10,
			0.0f, D3D11_FLOAT32_MAX,
		};
		NV_RETURN_ON_FAIL(device->CreateSamplerState(comparisonSamplerDesc, m_comparisonSampler.writeRef()));

		// create point clamp sampler for PCF sampling for hair
		D3D11_SAMPLER_DESC pointClampSamplerDesc[1] = {
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			0.0, 0, D3D11_COMPARISON_NEVER,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, D3D11_FLOAT32_MAX,
		};
		NV_RETURN_ON_FAIL(device->CreateSamplerState(pointClampSamplerDesc, m_pointClampSampler.writeRef()));
	}
	return NV_OK;
}



Void Dx11ApiGlobal::saveRenderState()
{
	NV_CORE_ASSERT(m_hasRenderState == false && m_context);
	m_renderState.save(m_context);
	// Set that we don't know what the current depth op is
	m_depthOpState.setDepthOp(NV_NULL, DepthOp::UNKNOWN);
	m_hasRenderState = true;
}
Void Dx11ApiGlobal::restoreRenderState()
{
	NV_CORE_ASSERT(m_hasRenderState == true && m_context);
	m_renderState.restore(m_context);
	m_hasRenderState = false;
}

Void Dx11ApiGlobal::setRasterizerMode(RasterizerMode mode)
{
	switch (mode)
	{
		case RASTERIZER_MODE_SOLID:
		{
			m_context->RSSetState(m_rasterizerStateSolid);
			break;
		}
		case RASTERIZER_MODE_WIRE_FRAME:
		{
			m_context->RSSetState(m_rasterizerStateWireFrame);
			break;
		}
		default: break;
	}
}

Void Dx11ApiGlobal::setDepthOp(EDepthOp depthOp)
{
	NV_CORE_ASSERT(m_hasRenderState == true && m_context);
	m_depthOpState.setDepthOp(m_context, depthOp);
}

void Dx11ApiGlobal::applySamplers(bool useVs, bool useHs, bool useDs, bool useGs, bool usePs)
{
	// set sampler states for the texture sampling
	ID3D11SamplerState* const states[] =
	{
		m_linearSampler,
		m_pointClampSampler
	};
	if (useVs) m_context->VSSetSamplers(0, NV_COUNT_OF(states), states);
	if (useHs) m_context->HSSetSamplers(0, NV_COUNT_OF(states), states);
	if (useDs) m_context->DSSetSamplers(0, NV_COUNT_OF(states), states);
	if (useGs) m_context->GSSetSamplers(0, NV_COUNT_OF(states), states);
	if (usePs) m_context->PSSetSamplers(0, NV_COUNT_OF(states), states);
}

Result Dx11ApiGlobal::_createVisualizationVertexBuffers()
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
	NV_RETURN_ON_FAIL(Dx11Util::createVertexBuffer(m_device, sizeof(gfsdk_float4), m_numSphereLines * 2, buf, m_sphereVertexBuffer.writeRef(), NV_NULL));
	return NV_OK;
}

Result Dx11ApiGlobal::_createScalarNoiseLut()
{
	// create GPU buffer
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(m_device, DXGI_FORMAT_R32_FLOAT, sizeof(float), STRAND_NOISE_TABLE_SIZE, getScalarNoise(),
		m_scalarNoiseBuffer.writeRef(), m_scalarNoiseSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiGlobal::_createVectorNoiseLut()
{
	// create GPU buffer
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(m_device, DXGI_FORMAT_R32G32B32_FLOAT, sizeof(gfsdk_float3), STRAND_NOISE_TABLE_SIZE, getVectorNoise(),
		m_vectorNoiseBuffer.writeRef(), m_vectorNoiseSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiGlobal::_createStrandBarycentricCoordinates()
{
	// create resource for strand coord LUT
	NV_RETURN_ON_FAIL(Dx11Util::createReadOnlyBuffer(m_device, DXGI_FORMAT_R32G32_FLOAT, sizeof(gfsdk_float2), STRAND_NOISE_TABLE_SIZE,  getBarycentricNoise(),
		m_strandCoordinatesBuffer.writeRef(), m_strandCoordinatesSrv.writeRef()));
	return NV_OK;
}

Result Dx11ApiGlobal::calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut)
{
	if (!m_context)
	{
		return NV_FAIL;
	}
	DxUtil::calcViewInfo(viewport, view, projection, fov, viewInfoOut);
	return NV_OK;
}

Bool Dx11ApiGlobal::areEqual(const NvCo::ApiHandle& a, const NvCo::ApiHandle& b)
{
	if (a.isNull() && b.isNull())
	{
		return true;
	}
	if ((a.isNull() || b.isNull()) || a.m_type != b.m_type)
	{
		return false;
	}
	if (a.getApiType(a.m_type) != NvCo::ApiType::DX11)
	{
		return false;
	}
	// We just need to compare the handles
	return a.m_handle == b.m_handle;
}

Void Dx11ApiGlobal::onGpuWorkSubmitted(const NvCo::ApiHandle& handle)
{
	// Not needed on Dx11 as work is submitted immediately to Dx11 api
	NV_UNUSED(handle);
}

Result Dx11ApiGlobal::setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo)
{
	NV_UNUSED(shaderIndex)
	NV_UNUSED(pixelShaderInfo)
	NV_CO_LOG_ERROR("Not implemented");
	return NV_FAIL;
}

} // namespace HairWorks 
} // namespace nvidia 