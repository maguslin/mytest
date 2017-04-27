/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12MsaaComposer.h"

#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>
#include <Nv/Common/NvCoMemory.h>

#include <Nv/HairWorks/Internal/Dx12/d3dx12.h>

namespace nvidia {
namespace HairWorks {

Dx12MsaaComposer::Dx12MsaaComposer()
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
	m_commandList = NV_NULL;
}

Dx12MsaaComposer::~Dx12MsaaComposer()
{
}

void Dx12MsaaComposer::setBlendState()
{
}

void Dx12MsaaComposer::restoreBlendState()
{
}

void Dx12MsaaComposer::setSampleCount(Int sampleCount)  
{ 
	m_sampleCountMsaa = sampleCount; 
}

Int Dx12MsaaComposer::getSampleCount() const 
{ 
	return m_sampleCountMsaa; 
}

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

Result Dx12MsaaComposer::init(ID3D12Device* device, Dx12ApiGlobal* glob)
{
	m_device = device;
	if (!device)
	{
		return NV_FAIL;
	}

	m_apiGlobal = glob;
	const NvCo::Dx12TargetInfo& targetInfo = glob->m_initInfo.m_targetInfo;

	// shader for super-sample resolve
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = ResolveBlobs::g_vs_main;
		desc.m_pixelBlob = ResolveBlobs::g_ps_main;

		NV_RETURN_ON_FAIL(m_passes[PASS_RESOLVE].initSuperSample(device, desc, PASS_RESOLVE));
	}
	// shader for supersample depth copy
	{
#if 0
		NVHairINTShaderPass& shaderPass = pINTShader->m_pShaderPasses[NVHAIR_INT_SHADER_SUPERSAMPLE_DEPTH_COPY_PASS];

		shaderPass.createVertexShader(pd3dDevice, BLOB_POINTER_AND_SIZE(SupersampleDepthCopyBlobs::g_vs_main));
		shaderPass.createPixelShader(pd3dDevice, BLOB_POINTER_AND_SIZE(SupersampleDepthCopyBlobs::g_ps_main));
#endif
	}
	// shader for supersample post processing
	{
#if 0
		NVHairINTShaderPass& shaderPass = pINTShader->m_pShaderPasses[NVHAIR_INT_SHADER_SUPERSAMPLE_POST_PASS];

		shaderPass.createVertexShader(pd3dDevice, BLOB_POINTER_AND_SIZE(SupersamplePostBlobs::g_vs_main));
		shaderPass.createPixelShader(pd3dDevice, BLOB_POINTER_AND_SIZE(SupersamplePostBlobs::g_ps_main));
#endif
	}
	// shader for supersample post processing color
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = PostColorBlobs::g_vs_main;
		desc.m_pixelBlob = PostColorBlobs::g_ps_main;
		desc.m_targetInfo = targetInfo;

		NV_RETURN_ON_FAIL(m_passes[PASS_POST_COLOR].initSuperSample(device, desc, PASS_POST_COLOR));
	}
	// shader for supersample post processing depth
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = PostDepthBlobs::g_vs_main;
		desc.m_pixelBlob = PostDepthBlobs::g_ps_main;
		desc.m_targetInfo = targetInfo;

		NV_RETURN_ON_FAIL(m_passes[PASS_POST_DEPTH].initSuperSample(device, desc, PASS_POST_DEPTH));
	}
	// shader for supersample updample
	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = UpsampleBlobs::g_vs_main;
		desc.m_pixelBlob = UpsampleBlobs::g_ps_main;

		NV_RETURN_ON_FAIL(m_passes[PASS_UPSAMPLE].initSuperSample(device, desc, PASS_UPSAMPLE));
	}

	m_msaaInfo = NV_NULL;

	return NV_OK;
}

Result Dx12MsaaComposer::prepare(const NvCo::ApiContext& contextIn, Int sampleCount, Bool depthComparisonLess, const NvCo::ConstApiPtr& other)
{
	NV_UNUSED(contextIn);

	m_msaaInfo = NV_NULL;
	if (!other.isNull())
	{
		m_msaaInfo = Dx12SdkType::cast<Dx12MsaaInfo>(other);
	}

	m_apiGlobal->m_sampleCountMsaa = sampleCount;	// Important to set the MSAA sample count so that the instance renderer uses the right PSO

	m_commandList = m_apiGlobal->getCommandList();

	m_depthComparisonLess = depthComparisonLess;

	m_userViewport = (D3D12_VIEWPORT&)m_apiGlobal->m_viewPort;

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

#if 0
		// create msaa shader heap
		for (UINT i = 0; i < NVHAIR_NUM_MSAA_SHADER_HEAPS; ++i)
		{
			buffers.m_shaderHeap[i].Release();
			buffers.m_shaderHeap[i].Init(pd3dDevice, 32);
		}

		// create SRV heap
		buffers.m_srvHeap.Release();
		buffers.m_srvHeap.Init(pd3dDevice, 32, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		buffers.m_colorMSPost.Release();
		buffers.m_colorPost.Release();
		buffers.m_colorMSPost.Init(&buffers.m_srvHeap);
		buffers.m_colorPost.Init(&buffers.m_srvHeap);

		buffers.m_depthMSPost.Release();
		buffers.m_depthPost.Release();
		buffers.m_depthMSPost.Init(&buffers.m_srvHeap);
		buffers.m_depthPost.Init(&buffers.m_srvHeap);

		buffers.m_srvIndexDSVUser = (UINT)-1;
#endif
		const Dx12Info& info = m_apiGlobal->getInfo(&m_apiGlobal->m_viewHeap);

		// create MSAA color buffer
		{
			D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, UINT(w), UINT(h), 1, 1, sampleCount, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_colorMsPost.init(info, &resourceDesc, &rtvDesc, &srvDesc);
		}

		// create non-MSAA color buffer
		{
			D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, UINT(w), UINT(h), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_colorPost.init(info, &resourceDesc, &rtvDesc, &srvDesc);
		}

		// create non-MSAA depth buffer
		{
			D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R24G8_TYPELESS, UINT(w), UINT(h), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_depthPost.init(info, &resourceDesc, &dsvDesc, &srvDesc);
		}

		// create MSAA depth buffer
		{
			// 24bit depth, 8bit stencil
			D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R24G8_TYPELESS, UINT(w), UINT(h), 1, 1, sampleCount, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_depthMsPost.init(info, &resourceDesc, &dsvDesc, &srvDesc);
		}

		// create SRV for user DSV
		{
			Int index = info.m_viewHeap->allocate();
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = info.m_viewHeap->getCpuHandle(index);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			info.m_device->CreateShaderResourceView(m_msaaInfo->m_dsvBuffer, &srvDesc, srvHandle);

			m_dsvUser.m_buffer = m_msaaInfo->m_dsvBuffer;
			m_dsvUser.m_viewHeap = info.m_viewHeap;
			m_dsvUser.m_srvIndex = index;
		}

#if 0
		{
			D3D12_RESOURCE_BARRIER barriers[] =
			{
				CD3DX12_RESOURCE_BARRIER::Transition(buffers.m_colorMSPost.m_pBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(buffers.m_colorPost.m_pBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(buffers.m_depthMSPost.m_pBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(buffers.m_depthPost.m_pBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			};
			pCommandList->ResourceBarrier(_countof(barriers), barriers);
		}

		// create bundle command allocators and commandlists
		for (UINT i = 0; i < NVHAIR_NUM_MSAA_SHADER_HEAPS; ++i)
		{
			pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(buffers.m_bundleAllocator[i].ReleaseAndGetAddressOf()));
			pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, buffers.m_bundleAllocator[i].Get(), nullptr, IID_PPV_ARGS(buffers.m_bundle[i].ReleaseAndGetAddressOf()));
			buffers.m_useBundle[i] = false;
		}
#endif
	}

	return NV_OK;
}

Result Dx12MsaaComposer::bindConstantBuffer(bool emitPartialFragment)
{
	Dx12ApiGlobal* glob = m_apiGlobal;
	typedef NvCo::Dx12CircularResourceHeap Heap;
	m_cursorOut = glob->m_uploadHeap.allocateConstantBuffer(sizeof(ConstantBuffer));
	ConstantBuffer* constantBuffer = (ConstantBuffer*)m_cursorOut.m_position;

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

	return NV_OK;
}

Result Dx12MsaaComposer::upsampleDepthBuffer()
{
	if (!m_depthMsPost.m_buffer)
		return NV_FAIL;

	Dx12ApiGlobal* glob = m_apiGlobal;

	{
		Dx12BarrierSubmitter barriers(m_commandList);
		m_depthMsPost.transition(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers);
		m_dsvUser.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthMsPost.getDsvCpuHandle();

	m_commandList->OMSetRenderTargets(0, &rtvHandle, FALSE, &dsvHandle);
	m_commandList->RSSetViewports(1, &m_viewportMsaa);

	Dx12ShaderPass& shaderPass = m_passes[PASS_UPSAMPLE];

	m_sampleCountUser = 1;
	bindConstantBuffer();

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			m_dsvUser.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}
	ID3D12DescriptorHeap* heaps[] = { viewHeap };

	{
		m_commandList->SetGraphicsRootSignature(shaderPass.m_rootSignature);

		m_commandList->SetGraphicsRootConstantBufferView(0, m_cursorOut.getGpuHandle());

		m_commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		m_commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		DWORD index;
		_BitScanForward(&index, m_sampleCountMsaa);
		m_commandList->SetPipelineState(shaderPass.m_pipelineStateMsaa[index]);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		m_commandList->DrawInstanced(3, 1, 0, 0);
	}

	return NV_OK;
}

Result Dx12MsaaComposer::startRendering()
{
	if (!m_colorMsPost.m_buffer)
		return NV_FAIL;

	if (!m_depthMsPost.m_buffer)
		return NV_FAIL;

	{
		Dx12BarrierSubmitter barriers(m_commandList);
		m_colorMsPost.transition(D3D12_RESOURCE_STATE_RENDER_TARGET, barriers);
		m_dsvUser.transition(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_colorMsPost.getRtvCpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthMsPost.getDsvCpuHandle();

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	m_commandList->RSSetViewports(1, &m_viewportMsaa);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	D3D12_RECT rect = { (LONG)m_viewportMsaa.TopLeftX, (LONG)m_viewportMsaa.TopLeftY, (LONG)m_viewportMsaa.Width, (LONG)m_viewportMsaa.Height };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 1, &rect);

	return NV_OK;
}

Result Dx12MsaaComposer::resolveDepth(const NvCo::ApiHandle& sourceSrvIn, const NvCo::ApiHandle& targetDsvIn)
{
	NV_UNUSED(sourceSrvIn);
	NV_UNUSED(targetDsvIn);

	Dx12ApiGlobal* glob = m_apiGlobal;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_colorPost.getRtvCpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthPost.getDsvCpuHandle();

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	m_commandList->RSSetViewports(1, &m_viewportMsaa);

	Dx12ShaderPass& shaderPass = m_passes[PASS_RESOLVE];

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			m_depthMsPost.getSrvCpuHandle(),
			m_colorMsPost.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}
	ID3D12DescriptorHeap* heaps[] = { viewHeap };

	{
		m_commandList->SetGraphicsRootSignature(shaderPass.m_rootSignature);

		m_commandList->SetGraphicsRootConstantBufferView(0, m_cursorOut.getGpuHandle());

		m_commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		m_commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		m_commandList->SetPipelineState(shaderPass.m_pipelineState);
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		m_commandList->DrawInstanced(3, 1, 0, 0);
	}

	return NV_OK;
}

Result Dx12MsaaComposer::finishRendering()
{
	m_apiGlobal->m_sampleCountMsaa = 0;	// Reset MSAA sample count

	if (!m_colorPost.m_buffer || !m_colorMsPost.m_buffer)
		return NV_FAIL;

	{
		Dx12BarrierSubmitter barriers(m_commandList);
		m_colorMsPost.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		m_colorPost.transition(D3D12_RESOURCE_STATE_RENDER_TARGET, barriers);
		m_depthMsPost.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		m_depthPost.transition(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers);
	}

	NvCo::ApiHandle ptr;
	resolveDepth(ptr, ptr);

	m_commandList->OMSetRenderTargets(1, &m_msaaInfo->m_rtvCpuHandle, FALSE, &m_msaaInfo->m_dsvCpuHandle);
	m_commandList->RSSetViewports(1, &m_userViewport);

	return NV_OK;
}

Result Dx12MsaaComposer::drawColor()
{
	if (!m_colorPost.m_buffer)
		return NV_FAIL;

	Dx12ApiGlobal* glob = m_apiGlobal;

	{
		Dx12BarrierSubmitter barriers(m_commandList);
		m_colorPost.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
	}

	Dx12ShaderPass& shaderPass = m_passes[PASS_POST_COLOR];

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			m_colorPost.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}
	ID3D12DescriptorHeap* heaps[] = { viewHeap };

	{
		m_commandList->SetGraphicsRootSignature(shaderPass.m_rootSignature);

		m_commandList->SetGraphicsRootConstantBufferView(0, m_cursorOut.getGpuHandle());

		m_commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		m_commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		m_commandList->SetPipelineState(shaderPass.m_pipelineState);
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		m_commandList->DrawInstanced(3, 1, 0, 0);
	}

	return NV_OK;
}

Result Dx12MsaaComposer::drawPostDepth(bool emitPartialFramgment)
{
	if (!m_depthPost.m_buffer)
		return NV_FAIL;

	Dx12ApiGlobal* glob = m_apiGlobal;

	{
		Dx12BarrierSubmitter barriers(m_commandList);
		m_colorPost.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
		m_depthPost.transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, barriers);
	}

	Dx12ShaderPass& shaderPass = m_passes[PASS_POST_DEPTH];

	bindConstantBuffer(emitPartialFramgment);

	D3D12_GPU_DESCRIPTOR_HANDLE viewHandles[1];
	ID3D12DescriptorHeap* viewHeap;
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE srvs[] =
		{
			m_colorPost.getSrvCpuHandle(),
			m_depthPost.getSrvCpuHandle(),
		};
		viewHeap = glob->m_viewCache.put(DescriptorSet(DescriptorSet::TYPE_SRV, srvs), false, viewHandles);
	}
	ID3D12DescriptorHeap* heaps[] = { viewHeap };

	{
		m_commandList->SetGraphicsRootSignature(shaderPass.m_rootSignature);

		m_commandList->SetGraphicsRootConstantBufferView(0, m_cursorOut.getGpuHandle());

		m_commandList->SetDescriptorHeaps(NV_COUNT_OF(heaps), heaps);
		m_commandList->SetGraphicsRootDescriptorTable(1, viewHandles[0]);

		m_commandList->SetPipelineState(shaderPass.m_pipelineState);
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		m_commandList->DrawInstanced(3, 1, 0, 0);
	}

	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia
