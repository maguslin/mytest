/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11MsaaComposer.h"

#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>
#include <Nv/Common/NvCoMemory.h>

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11Util.h>

namespace nvidia {
namespace HairWorks {


namespace ResolveBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleResolve_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleResolve_Ps.h>
}

namespace DepthCopyBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleDepthCopy_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleDepthCopy_Ps.h>
}

namespace UpsampleBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleUpsample_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSampleUpsample_Ps.h>
}

namespace PostBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePost_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePost_Ps.h>
}

namespace PostColorBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePostColor_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePostColor_Ps.h>
}

namespace PostDepthBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePostDepth_Vs.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairSuperSamplePostDepth_Ps.h>
}

Dx11MsaaComposer::Dx11MsaaComposer()
{
	m_viewportMsaa.Width = 0;
	m_viewportMsaa.Height = 0;
	m_viewportMsaa.TopLeftX = 0;
	m_viewportMsaa.TopLeftY = 0;

	m_userViewport.Width = 0;
	m_userViewport.Height = 0;
	m_userViewport.TopLeftX = 0;
	m_userViewport.TopLeftY = 0;

	m_msaaBufferWidth = 0;
	m_msaaBufferHeight = 0;

	m_sampleCountUser = 0;
	m_sampleCountMsaa = 0;

	m_depthComparisonLess = true;
	m_numBlurs = 0;

	m_device = NV_NULL;
	m_context = NV_NULL;
}

Dx11MsaaComposer::~Dx11MsaaComposer()
{
}

void Dx11MsaaComposer::setSampleCount(Int sampleCount)  
{ 
	m_sampleCountMsaa = sampleCount; 
}

Int Dx11MsaaComposer::getSampleCount() const 
{ 
	return m_sampleCountMsaa; 
}

void Dx11MsaaComposer::setBlendState()
{
	NV_CORE_ASSERT(m_context);

	m_context->OMGetBlendState(m_blendStatePrev.writeRef(), m_blendFactorPrev, &m_sampleMaskPrev);
	float zero[] = { 0,0,0,0 };
	m_context->OMSetBlendState(m_blendStateAlphaBlendingDeferred, zero, 0xffffffff);
}

void Dx11MsaaComposer::restoreBlendState()
{
	NV_CORE_ASSERT(m_context);
	m_context->OMSetBlendState(m_blendStatePrev, m_blendFactorPrev, m_sampleMaskPrev);
}

D3D11_TEXTURE2D_DESC Dx11MsaaComposer::_getColorBufferDesc(UINT w, UINT h, UINT sampleCount)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = UINT(w);
	desc.Height = UINT(h);
	desc.ArraySize = 1;
	desc.MiscFlags = 0;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = sampleCount;
	desc.SampleDesc.Quality = 0;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	return desc;
}

/* static */D3D11_TEXTURE2D_DESC Dx11MsaaComposer::_getDepthBufferDesc(UINT w, UINT h, DXGI_FORMAT format, UINT sampleCount)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = UINT(w);
	desc.Height = UINT(h);
	desc.MipLevels = 1;
	desc.MiscFlags = NULL;
	desc.SampleDesc.Count = sampleCount;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	return desc;
}

/* static */Result Dx11MsaaComposer::_createDepthBufferSrv(ID3D11Device* device, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView** depthBufferSrvOut, UINT& sampleCountUser)
{
	// create srv from current depth buffer 
	if (!dsv)
		return NV_FAIL;

	sampleCountUser = 0;

	{
		NvCo::ComPtr<ID3D11Resource> dsvResource;
		dsv->GetResource(dsvResource.writeRef());

		if (!dsvResource)
			return NV_FAIL;

		NvCo::ComPtr<ID3D11Texture2D> depthBuffer;
		NV_RETURN_ON_FAIL(dsvResource->QueryInterface(depthBuffer.writeRef()));

		D3D11_TEXTURE2D_DESC dsvDesc;
		depthBuffer->GetDesc(&dsvDesc);

		// depth target must be able to bound as shader resource
		if (!(dsvDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
		{
			return NV_FAIL;
		}
		sampleCountUser = dsvDesc.SampleDesc.Count;

		// create SRV for the user depth buffer
		bool msaaBuffer = sampleCountUser > 0;

		D3D11_SRV_DIMENSION dimension = msaaBuffer ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		DXGI_FORMAT format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = Dx11Util::createSrvDesc(format, dimension, 0);
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		device->CreateShaderResourceView(depthBuffer, &srvDesc, depthBufferSrvOut);
	}

	return NV_OK;
}

/* static */Result Dx11MsaaComposer::_createDepthBufferSrv(ID3D11Device* device, ID3D11Texture2D* depthBuffer, ID3D11ShaderResourceView** srvOut)
{
	// create srv from current depth buffer for upsample pass
	if (!depthBuffer)
		return NV_FAIL;

	D3D11_TEXTURE2D_DESC desc;
	depthBuffer->GetDesc(&desc);

	// depth target must be able to bound as shader resource
	if (!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
		return NV_FAIL;

	UINT sampleCount = desc.SampleDesc.Count;

	// create SRV for the user depth buffer
	bool msaaBuffer = sampleCount > 1;

	D3D11_SRV_DIMENSION dimension = msaaBuffer ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	DXGI_FORMAT format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = Dx11Util::createSrvDesc(format, dimension, 0);
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	return device->CreateShaderResourceView(depthBuffer, &srvDesc, srvOut);
}

/* static */Result Dx11MsaaComposer::_createDepthBufferDsv(ID3D11Device* device, ID3D11Texture2D* depthBuffer, ID3D11DepthStencilView** dsvOut)
{
	// create dsv from current depth buffer for depth copy pass
	if (!depthBuffer)
		return NV_FAIL;
	// create DSV out of target
	D3D11_TEXTURE2D_DESC desc;
	depthBuffer->GetDesc(&desc);
	if (!(desc.BindFlags & D3D11_BIND_DEPTH_STENCIL))
		return NV_FAIL;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = Dx11Util::createDsvDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D, 0, 0);
	return device->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvOut);
}

/* static */void  Dx11MsaaComposer::_clearShaderResources(ID3D11DeviceContext* context)
{
	// prepare SRVs and constant buffer
	const int numSrvs = 9;
	ID3D11ShaderResourceView* srvs[numSrvs] = { NV_NULL };
	context->PSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);
}

void Dx11MsaaComposer::_setRenderState(const Dx11ShaderPass& pass)
{
	m_context->RSSetState(m_rasterizerStateSolid);
	m_context->IASetInputLayout(NV_NULL);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_context->VSSetShader(pass.m_vertexShader, NV_NULL, 0);
	m_context->PSSetShader(pass.m_pixelShader, NV_NULL, 0);
	m_context->HSSetShader(NV_NULL, NV_NULL, 0);
	m_context->DSSetShader(NV_NULL, NV_NULL, 0);
	m_context->GSSetShader(NV_NULL, NV_NULL, 0);
}

Result Dx11MsaaComposer::init(ID3D11Device*device, ID3D11DeviceContext* context)
{	
	m_device = device;
	if (!device)
	{
		return NV_FAIL;
	}
	m_context = context;
	
	// shader for super-sample resolve
	{
		Dx11ShaderPass& pass = m_passes[PASS_RESOLVE];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, ResolveBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, ResolveBlobs::g_ps_main));
	}
	// shader for super-sample depth copy
	{
		Dx11ShaderPass& pass = m_passes[PASS_DEPTH_COPY];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, DepthCopyBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, DepthCopyBlobs::g_ps_main));
	}
	// shader for super-sample post processing
	{
		Dx11ShaderPass& pass = m_passes[PASS_POST];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, PostBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, PostBlobs::g_ps_main));
	}
	// shader for super-sample post processing color
	{
		Dx11ShaderPass& pass = m_passes[PASS_POST_COLOR];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, PostColorBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, PostColorBlobs::g_ps_main));
	}
	// shader for super-sample post processing depth
	{
		Dx11ShaderPass& pass = m_passes[PASS_POST_DEPTH];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, PostDepthBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, PostDepthBlobs::g_ps_main));
	}
	// shader for super-sample up-sample
	{
		Dx11ShaderPass& pass = m_passes[PASS_UPSAMPLE];
		NV_RETURN_ON_FAIL(pass.createVertexShader(device, UpsampleBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(pass.createPixelShader(device, UpsampleBlobs::g_ps_main));
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;
		NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilStateWriteOnly.writeRef()));
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
	// no alpha blending
	{
		D3D11_BLEND_DESC desc;
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		D3D11_RENDER_TARGET_BLEND_DESC& rtDesc = desc.RenderTarget[0];
		{
			rtDesc.BlendEnable = false;
			rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
			rtDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
			rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtDesc.RenderTargetWriteMask = 0x0f;
		}
		NV_RETURN_ON_FAIL(device->CreateBlendState(&desc, m_blendStateNoBlending.writeRef()));
	}
	// alpha blending enabled
	{
		D3D11_BLEND_DESC desc;
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		D3D11_RENDER_TARGET_BLEND_DESC& rtDesc = desc.RenderTarget[0];
		{
			rtDesc.BlendEnable = true;
			rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
			rtDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
			rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtDesc.RenderTargetWriteMask = 0x0f;
		}
		NV_RETURN_ON_FAIL(device->CreateBlendState(&desc, m_blendStateAlphaBlending.writeRef()));
	}
	// alpha blending for deferred MSAA
	{
		D3D11_BLEND_DESC desc;
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;

		D3D11_RENDER_TARGET_BLEND_DESC& rtDesc = desc.RenderTarget[0];
		{
			rtDesc.BlendEnable = true;
			rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
			rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			rtDesc.RenderTargetWriteMask = 0x0f;
		}
		NV_RETURN_ON_FAIL(device->CreateBlendState(&desc, m_blendStateAlphaBlendingDeferred.writeRef()));
	}

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
		NV_RETURN_ON_FAIL(device->CreateRasterizerState(&solidNoCullDesc, m_rasterizerStateSolid.writeRef()));
	}

	D3D11_BUFFER_DESC desc = Dx11Util::createBufferDesc(D3D11_USAGE_DYNAMIC, sizeof(ConstantBuffer), 0, D3D11_BIND_CONSTANT_BUFFER, 0, D3D11_CPU_ACCESS_WRITE);
	return Dx11Util::createBuffer(device, desc, NV_NULL, m_constantBuffer.writeRef());
}

Result Dx11MsaaComposer::prepare(const NvCo::ApiContext& contextIn, Int sampleCount, Bool depthComparisonLess, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(other);

	m_context = NvCo::Dx11Type::cast<ID3D11DeviceContext>(contextIn);
	if (!m_device || !m_context)
	{
		return NV_FAIL;
	}

	m_depthComparisonLess = depthComparisonLess;

	UINT numViewports = 1;
	//D3D11_VIEWPORT	viewport;
	m_context->RSGetViewports(&numViewports, &m_userViewport);

	m_viewportMsaa = m_userViewport;
	m_viewportMsaa.TopLeftX = 0;
	m_viewportMsaa.TopLeftY = 0;

	// check if msaa buffer needs to be recreated
	bool msaaBufferLarge = (m_msaaBufferWidth >= m_userViewport.Width) && (m_msaaBufferHeight >= m_userViewport.Height);
	bool msaaSampleCountMatch = m_sampleCountMsaa == sampleCount;

	if (!msaaBufferLarge || !msaaSampleCountMatch)
	{
		UINT w = (UINT)m_userViewport.Width;
		UINT h = (UINT)m_userViewport.Height;

		m_msaaBufferWidth = w;
		m_msaaBufferHeight = h;
		m_sampleCountMsaa = sampleCount;

		// create MSAA color buffer
		{
			D3D11_TEXTURE2D_DESC desc = _getColorBufferDesc(w, h, sampleCount);
			NV_RETURN_ON_FAIL(m_device->CreateTexture2D(&desc, NV_NULL, m_colorBufferMsPost.writeRef()));

			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = Dx11Util::createRtvDesc(desc.Format, D3D11_RTV_DIMENSION_TEXTURE2DMS);
				NV_RETURN_ON_FAIL(m_device->CreateRenderTargetView(m_colorBufferMsPost, &rtvDesc, m_colorBufferMsPostRtv.writeRef()));
			}
		}

		// create non-MSAA color buffer
		{
			D3D11_TEXTURE2D_DESC desc = _getColorBufferDesc(w, h, 1);
			NV_RETURN_ON_FAIL(m_device->CreateTexture2D(&desc, NV_NULL, m_colorBufferPost.writeRef()));
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc = Dx11Util::createSrvDesc(desc.Format, D3D11_SRV_DIMENSION_TEXTURE2D, 0);
				srcDesc.Texture2D.MostDetailedMip = 0;
				srcDesc.Texture2D.MipLevels = 1;

				NV_RETURN_ON_FAIL(m_device->CreateShaderResourceView(m_colorBufferPost, &srcDesc, m_colorBufferPostSrv.writeRef()));
			}
			{
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = Dx11Util::createRtvDesc(desc.Format, D3D11_RTV_DIMENSION_TEXTURE2D);
				NV_RETURN_ON_FAIL(m_device->CreateRenderTargetView(m_colorBufferPost, &rtvDesc, m_colorBufferPostRtv.writeRef()));
			}
		}

		// create non-MSAA depth buffer
		{
			// 24bit depth, 8bit stencil
			D3D11_TEXTURE2D_DESC desc = _getDepthBufferDesc(w, h, DXGI_FORMAT_R24G8_TYPELESS, 1);
			NV_RETURN_ON_FAIL(m_device->CreateTexture2D(&desc, NULL, m_depthBufferPost.writeRef()));
			// dsv
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = Dx11Util::createDsvDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D, 0, 0);
				NV_RETURN_ON_FAIL(m_device->CreateDepthStencilView(m_depthBufferPost, &dsvDesc, m_depthBufferPostDsv.writeRef()));
			}
			// srv
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = Dx11Util::createSrvDesc(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, D3D11_SRV_DIMENSION_TEXTURE2D, 0);
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				NV_RETURN_ON_FAIL(m_device->CreateShaderResourceView(m_depthBufferPost, &srvDesc, m_depthBufferPostSrv.writeRef()));
			}
		}

		// create MSAA depth buffer
		{
			// 24bit depth, 8bit stencil
			{
				D3D11_TEXTURE2D_DESC desc = _getDepthBufferDesc(w, h, DXGI_FORMAT_R24G8_TYPELESS, sampleCount);
				NV_RETURN_ON_FAIL(m_device->CreateTexture2D(&desc, NULL, m_depthBufferMsPost.writeRef()));
			}
			// dsv
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = Dx11Util::createDsvDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2DMS, 0, 0);
				NV_RETURN_ON_FAIL(m_device->CreateDepthStencilView(m_depthBufferMsPost, &dsvDesc, m_depthBufferMsPostDsv.writeRef()));
			}
			// srv
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = Dx11Util::createSrvDesc(DXGI_FORMAT_R24_UNORM_X8_TYPELESS, D3D11_SRV_DIMENSION_TEXTURE2DMS, 0);
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				NV_RETURN_ON_FAIL(m_device->CreateShaderResourceView(m_depthBufferMsPost, &srvDesc, m_depthBufferMsPostSrv.writeRef()));
			}
		}
	}
	return NV_OK;
}

Result Dx11MsaaComposer::bindConstantBuffer(bool emitPartialFragment)
{
	// bind constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	NV_RETURN_ON_FAIL(m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ConstantBuffer* constantBuffer = (ConstantBuffer*)mappedResource.pData;

	// User viewport
	constantBuffer->m_userWidth = m_userViewport.Width;
	constantBuffer->m_userHeight = m_userViewport.Height;
	constantBuffer->m_userTopLeftX = m_userViewport.TopLeftX;
	constantBuffer->m_userTopLeftY = m_userViewport.TopLeftY;

	// Msaa viewport
	constantBuffer->m_msaaWidth = m_viewportMsaa.Width;
	constantBuffer->m_msaaHeight = m_viewportMsaa.Height;
	constantBuffer->m_msaaTopLeftX = m_viewportMsaa.TopLeftX;
	constantBuffer->m_msaaTopLeftY = m_viewportMsaa.TopLeftY;

	constantBuffer->m_sampleCountUser = m_sampleCountUser;
	constantBuffer->m_sampleCountMsaa = m_sampleCountMsaa;

	constantBuffer->m_depthComparisonLess = m_depthComparisonLess;

	constantBuffer->m_emitPartialFragment = emitPartialFragment;

	m_context->Unmap(m_constantBuffer, 0);
	m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.readRef());
	return NV_OK;
}

Result Dx11MsaaComposer::upsampleDepthBuffer()
{
	if (!m_depthBufferMsPostDsv)
		return NV_FAIL;

	// get current render target DSV
	NvCo::ComPtr<ID3D11DepthStencilView> userDsv;
	NvCo::ComPtr<ID3D11RenderTargetView> userRtv;
	m_context->OMGetRenderTargets(1, userRtv.writeRef(), userDsv.writeRef());

	if (userDsv == NV_NULL)
		return NV_FAIL;

	// set render target to custom MSAA buffers
	ID3D11RenderTargetView* noRtv = NV_NULL;
	m_context->OMSetRenderTargets(1, &noRtv, m_depthBufferMsPostDsv);
	m_context->RSSetViewports(1, &m_viewportMsaa);

	// clear msaa depth buffer
	bool depthComparisonLess = m_depthComparisonLess;

	float clearDepth = (depthComparisonLess) ? 1.0f : 0.0f;
	m_context->ClearDepthStencilView(m_depthBufferMsPostDsv, D3D11_CLEAR_DEPTH, clearDepth, 0);

	// create SRV out of current DSV
	NvCo::ComPtr<ID3D11ShaderResourceView> depthBufferSrv;
	UINT sampleCountUser = 0;
	NV_RETURN_ON_FAIL(_createDepthBufferSrv(m_device, userDsv, depthBufferSrv.writeRef(), sampleCountUser));

	m_sampleCountUser = sampleCountUser;
	m_context->OMSetDepthStencilState(m_depthStencilStateWriteOnly, 0);

	// copy current depth values into MSAA depth buffer by upsampling pass
	{
		// prepare SRVs 
		ID3D11ShaderResourceView* srvs[] = { depthBufferSrv };
		m_context->PSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);
		// bind constant buffer
		NV_RETURN_ON_FAIL(bindConstantBuffer());
		// set other render states
		_setRenderState(m_passes[PASS_UPSAMPLE]);
		// draw the quad with depth resolve shader
		m_context->Draw(3, 0);
		// clear shader resources
		_clearShaderResources(m_context);
	}
	// restore current render target
	m_context->OMSetRenderTargets(1, userRtv.readRef(), userDsv);
	return NV_OK;
}

Result Dx11MsaaComposer::startRendering()
{
	if (!m_colorBufferMsPostRtv || !m_depthBufferMsPostDsv)
		return NV_FAIL;
	// get current render target and dsv
	m_context->OMGetRenderTargets(1, m_userRtv.writeRef(), m_userDsv.writeRef());
	// set render target to custom MSAA buffers
	m_context->OMSetRenderTargets(1, m_colorBufferMsPostRtv.readRef(), m_depthBufferMsPostDsv);
	m_context->RSSetViewports(1, &m_viewportMsaa);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_context->ClearRenderTargetView(m_colorBufferMsPostRtv, clearColor);
	return NV_OK;
}

Result Dx11MsaaComposer::resolveDepth(const NvCo::ApiHandle& sourceSrvIn, const NvCo::ApiHandle& targetDsvIn)
{
	ID3D11ShaderResourceView* sourceSrv = NvCo::Dx11Type::cast<ID3D11ShaderResourceView>(sourceSrvIn);
	ID3D11DepthStencilView* targetDsv = NvCo::Dx11Type::cast<ID3D11DepthStencilView>(targetDsvIn);

	// set render target for resolve buffer
	ID3D11RenderTargetView* noRtv = NV_NULL;
	m_context->OMSetRenderTargets(1, &noRtv, targetDsv);

	float zero[] = { 0,0,0,0 };
	m_context->OMSetBlendState(m_blendStateNoBlending, zero, 0xffffffff);
	m_context->OMSetDepthStencilState(m_depthStencilStateWriteOnly, 0);

	// custom msaa resolve pass
	{
		// bind constant buffer
		NV_RETURN_ON_FAIL(bindConstantBuffer());
		// prepare SRVs 
		ID3D11ShaderResourceView* const srvs[] = { sourceSrv };
		m_context->PSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);

		// set shader and other render states
		_setRenderState(m_passes[PASS_RESOLVE]);

		// draw the quad with custom resolve shader
		m_context->Draw(3, 0);
		// clear shader resources
		_clearShaderResources(m_context);
	}

	return NV_OK;
}

Result Dx11MsaaComposer::finishRendering()
{
	// restore previous rtv, dsv, viewport
	if (!m_colorBufferPost || !m_colorBufferMsPost || !m_userRtv || !m_userDsv)
		return NV_FAIL;

	// resolve MSAA targets into non-MSAA buffers
	m_context->ResolveSubresource(m_colorBufferPost, 0, m_colorBufferMsPost, 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

	// resolve post render depth buffer
	resolveDepth(NvCo::Dx11Type::wrap(m_depthBufferMsPostSrv), NvCo::Dx11Type::wrap(m_depthBufferPostDsv));

	// restore user's render target stored before msaa
	m_context->OMSetRenderTargets(1, m_userRtv.readRef(), m_userDsv);
	m_context->RSSetViewports(1, &m_userViewport);

	// clear render target ref counters
	m_userRtv.setNull();
	m_userDsv.setNull();
	return NV_OK;
}

Result Dx11MsaaComposer::drawColor()
{
	if (!m_colorBufferPost)
		return NV_FAIL;

	// render post (composite) pass
	{
		ID3D11ShaderResourceView* srvs[] = { m_colorBufferPostSrv };
		m_context->PSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);
		// bind constant buffer
		NV_RETURN_ON_FAIL(bindConstantBuffer());
		// set shader and other render states
		_setRenderState(m_passes[PASS_POST_COLOR]);
		// do not write depth
		m_context->OMSetDepthStencilState(m_depthStencilStateNone, 0);
		// blend state must be on
		const float zero[] = { 0,0,0,0 };
		m_context->OMSetBlendState(m_blendStateAlphaBlending, zero, 0xffffffff);
		// draw the quad with custom resolve shader
		m_context->Draw(3, 0);
		// clear shader resources
		_clearShaderResources(m_context);
	}
	return NV_OK;
}

Result Dx11MsaaComposer::drawPostDepth(bool emitPartialFramgment)
{
	if (!m_depthBufferPost)
		return NV_FAIL;
	// render post (composite) pass
	{
		ID3D11ShaderResourceView* const srvs[] =
		{
			m_colorBufferPostSrv,
			m_depthBufferPostSrv
		};
		m_context->PSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);
		// bind constant buffer
		NV_RETURN_ON_FAIL(bindConstantBuffer(emitPartialFramgment));
		// set shader and other render states
		_setRenderState(m_passes[PASS_POST_DEPTH]);
		// do not test depth
		m_context->OMSetDepthStencilState(m_depthStencilStateWriteOnly, 0);
		// draw the quad with custom resolve shader
		m_context->Draw(3, 0);
		// clear shader resources
		_clearShaderResources(m_context);
	}
	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia