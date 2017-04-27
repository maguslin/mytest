/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12ShaderPass.h"

#include <Nv/Common/NvCoMemory.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12MsaaComposer.h>

#include "d3dx12.h"

namespace nvidia {
namespace HairWorks { 

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Dx12ShaderPass::DescriptorDesc::DescriptorDesc()
{
	m_hasDynamicConstantBuffer = false;
	m_numCbvs = 0;
	m_numSrvs = 0;
	m_numUavs = 0;
	m_numSamplers = 0;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Dx12ShaderPass::Desc::Desc()
{
	m_inputElementDescs = NV_NULL;
	m_numInputElementDescs = 0;
	
	NvCo::Memory::zero(m_streamOutputDesc);

	m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	m_rasterizerDesc = NV_NULL;
	m_blendDesc = NV_NULL;
	m_depthStencilDesc = NV_NULL;

	m_targetInfo.init();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Dx12ShaderPass !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#if 0
Result Dx12ShaderPass::createGraphicsPipelineState(ID3D12Device* device, const Blob& vertexBlob, const Blob& pixelBlob)
{
	CD3DX12_ROOT_PARAMETER rootParameters[32];

	// 1 CBV
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// 1 SRV
	CD3DX12_DESCRIPTOR_RANGE ranges[1];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	{
		// Allow input layout and deny unnecessary access to certain pipeline stages
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(2, rootParameters, 0, NV_NULL, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		NV_RETURN_ON_FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	{
		// Describe and create the graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { m_inputElementDescs, m_numInputElementDescs };
		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.VS = vertexBlob;
		psoDesc.PS = pixelBlob;
	
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.RasterizerState.MultisampleEnable = TRUE;

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;

		NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
	}
	return NV_OK;
}
#endif

/* static */Void Dx12ShaderPass::setGraphicsPipelineStateDesc(const Desc& desc, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	psoDesc.InputLayout = { desc.m_inputElementDescs, UINT(desc.m_numInputElementDescs) };

	psoDesc.PrimitiveTopologyType = desc.m_topologyType;

	
	{
		const NvCo::Dx12TargetInfo& targetInfo = desc.m_targetInfo;

		psoDesc.DSVFormat = targetInfo.m_depthStencilFormat;
		psoDesc.NumRenderTargets = targetInfo.m_numRenderTargets;
		for (Int i = 0; i < targetInfo.m_numRenderTargets; i++)
		{
			psoDesc.RTVFormats[i] = targetInfo.m_renderTargetFormats[i];
		}
		psoDesc.SampleDesc.Count = targetInfo.m_numSamples;
		psoDesc.SampleDesc.Quality = targetInfo.m_sampleQuality;

		psoDesc.SampleMask = targetInfo.m_sampleMask;
	}

	if (desc.m_rasterizerDesc)
	{
		psoDesc.RasterizerState = *desc.m_rasterizerDesc;
	}
	else
	{
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	}
	if (desc.m_blendDesc)
	{
		psoDesc.BlendState = *desc.m_blendDesc;
	}
	else
	{
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	if (desc.m_depthStencilDesc)
	{
		psoDesc.DepthStencilState = *desc.m_depthStencilDesc;
	}
	else
	{
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}
}


Result Dx12ShaderPass::init(ID3D12Device* device, const D3D12_DESCRIPTOR_RANGE* ranges, Int numRanges, const Desc& desc)
{
	CD3DX12_ROOT_PARAMETER rootParameters[32];
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	for (Int i = 0; i < numRanges; ++i)
	{
		rootParameters[i + 1].InitAsDescriptorTable(1, &ranges[i], D3D12_SHADER_VISIBILITY_ALL);
	}

	{
		// Allow input layout and deny unnecessary access to certain pipeline stages
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(numRanges + 1, rootParameters, 0, NV_NULL, rootSignatureFlags);

		NvCo::ComPtr<ID3DBlob> signature;
		NvCo::ComPtr<ID3DBlob> error;
		NV_RETURN_ON_FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	{
		// Describe and create the graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.VS = desc.m_vertexBlob;
		psoDesc.PS = desc.m_pixelBlob;
		psoDesc.GS = desc.m_geometryBlob;
		psoDesc.DS = desc.m_domainBlob;
		
		setGraphicsPipelineStateDesc(desc, psoDesc);

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		//psoDesc.RasterizerState.MultisampleEnable = TRUE;

		NV_RETURN_ON_FAIL( device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
	}
	return NV_OK;
}

Result Dx12ShaderPass::initPatch(ID3D12Device* device, const DescriptorDesc& descripDesc, const Desc& desc, const DescriptorDesc* optionalDesc)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[8];
	CD3DX12_ROOT_PARAMETER rootParameters[32];

	UINT rootIndex = 0;

	// CBV
	for (Int i = 0; i < descripDesc.m_numCbvs; ++i)
	{
		rootParameters[rootIndex++].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[rootIndex++].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_HULL);
		rootParameters[rootIndex++].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_DOMAIN);
		rootParameters[rootIndex++].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_GEOMETRY);
	}

	// SRV
	if (descripDesc.m_numSrvs > 0)
	{
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, descripDesc.m_numSrvs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_HULL);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_DOMAIN);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_GEOMETRY);
	}

	// UAV
	if (descripDesc.m_numUavs > 0)
	{
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, descripDesc.m_numUavs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	}

	// Sampler
	if (descripDesc.m_numSamplers > 0)
	{
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, descripDesc.m_numSamplers, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_HULL);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_DOMAIN);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
	}

	if (optionalDesc)
	{
		Int cbReg = 0;
		Int svReg = 0;
		// Dynamic constant buffer
		if (optionalDesc->m_hasDynamicConstantBuffer)
		{
			// Custom CBV
			//for (Int i = 0; i < descripDesc.m_numCustomCbv; ++i)
			{
				rootParameters[rootIndex++].InitAsConstantBufferView(cbReg++, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			}
		}
		// Custom Cbvs
		if (optionalDesc->m_numCbvs)
		{
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, optionalDesc->m_numCbvs, cbReg);
			rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
			cbReg += optionalDesc->m_numCbvs;
		}
		// Custom SRV
		if (optionalDesc->m_numSrvs > 0)
		{
			ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, optionalDesc->m_numSrvs, svReg);
			rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_PIXEL);
			svReg += optionalDesc->m_numSrvs;
		}
	}

	{
		// Deny unnecessary access to certain pipeline stages
		const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(rootIndex, rootParameters, 0, NV_NULL, rootSignatureFlags);

		NvCo::ComPtr<ID3DBlob> signature;
		NvCo::ComPtr<ID3DBlob> error;
		NV_RETURN_ON_FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	{
		// Describe and create the graphics pipeline state object (PSO)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		psoDesc.pRootSignature = m_rootSignature;

		psoDesc.VS = desc.m_vertexBlob;
		psoDesc.HS = desc.m_hullBlob;
		psoDesc.DS = desc.m_domainBlob;
		psoDesc.GS = desc.m_geometryBlob;
		psoDesc.PS = desc.m_pixelBlob;

		setGraphicsPipelineStateDesc(desc, psoDesc);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		//psoDesc.RasterizerState.MultisampleEnable = TRUE;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

		NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));

		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		{
			TRUE, FALSE,
			D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_CLEAR,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

		psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		for (int i = 0; i < 4; ++i)
		{
			//psoDesc.RasterizerState.ForcedSampleCount = psoDesc.SampleDesc.Count;
			NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStateMsaa[i].writeRef())));
			psoDesc.SampleDesc.Count <<= 1;
		}
	}

	return NV_OK;
}

Result Dx12ShaderPass::initStreamOut(ID3D12Device* device, const DescriptorDesc& descripDesc, const Desc& desc)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[3];
	CD3DX12_ROOT_PARAMETER rootParameters[32];

	UINT rootIndex = 0;

	// CBV
	for (Int i = 0; i < descripDesc.m_numCbvs; ++i)
	{
		rootParameters[rootIndex].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_GEOMETRY);
		rootIndex++;
	}

	// SRV
	if (descripDesc.m_numSrvs > 0)
	{
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, descripDesc.m_numSrvs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_GEOMETRY);
	}

	// UAV
	if (descripDesc.m_numUavs > 0)
	{
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, descripDesc.m_numUavs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_GEOMETRY);
	}

	{
		// Allow input layout and deny unnecessary access to certain pipeline stages
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(rootIndex, rootParameters, 0, NV_NULL, rootSignatureFlags);

		NvCo::ComPtr<ID3DBlob> signature;
		NvCo::ComPtr<ID3DBlob> error;
		NV_RETURN_ON_FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	// Describe and create the graphics pipeline state object (PSO)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { desc.m_inputElementDescs, UINT(desc.m_numInputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.VS = desc.m_vertexBlob;
		psoDesc.GS = desc.m_geometryBlob;
	
		psoDesc.StreamOutput = desc.m_streamOutputDesc;

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		//psoDesc.RasterizerState.MultisampleEnable = TRUE;

		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
	}
	return NV_OK;
}

Result Dx12ShaderPass::initCompute(ID3D12Device* device, const DescriptorDesc& descripDesc, const Blob& computeBlob) // , Int numCbv, Int numSrv, Int numUav, Int numSampler)
{
	CD3DX12_DESCRIPTOR_RANGE ranges[3];
	CD3DX12_ROOT_PARAMETER rootParameters[32];

	Int rootIndex = 0;

	// CBV
	for (Int i = 0; i < descripDesc.m_numCbvs; ++i)
	{
		rootParameters[rootIndex].InitAsConstantBufferView(i, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootIndex++;
	}

	// SRV
	if (descripDesc.m_numSrvs > 0)
	{
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, descripDesc.m_numSrvs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	}

	// UAV
	if (descripDesc.m_numUavs > 0)
	{
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, descripDesc.m_numUavs, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	}

	// Sampler
	if (descripDesc.m_numSamplers > 0)
	{
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, descripDesc.m_numSamplers, 0);
		rootParameters[rootIndex++].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);
	}
	
	{
		NvCo::ComPtr<ID3DBlob> signature;
		NvCo::ComPtr<ID3DBlob> error;

		// Create compute signature
		CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc(rootIndex, rootParameters);
		D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef());
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	{
		// Describe and create the compute pipeline state object (PSO)
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
		computePsoDesc.pRootSignature = m_rootSignature;
		computePsoDesc.CS = computeBlob;
	
		NV_RETURN_ON_FAIL(device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
	}

	return NV_OK;
}

Result Dx12ShaderPass::initSuperSample(ID3D12Device* device, const Desc& desc, const UInt type)
{
	const Dx12MsaaComposer::PassType shaderId = (Dx12MsaaComposer::PassType)type;

	CD3DX12_ROOT_PARAMETER rootParameters[32];

	// 1 CBV
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

	// 1 SRV
	CD3DX12_DESCRIPTOR_RANGE ranges[1];
	if (shaderId == Dx12MsaaComposer::PassType::PASS_RESOLVE || shaderId == Dx12MsaaComposer::PassType::PASS_POST_DEPTH)
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
	else
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	{
		// Allow input layout and deny uneccessary access to certain pipeline stages
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(2, rootParameters, 0, NV_NULL, rootSignatureFlags);

		NvCo::ComPtr<ID3DBlob> signature;
		NvCo::ComPtr<ID3DBlob> error;
		NV_RETURN_ON_FAIL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
		NV_RETURN_ON_FAIL(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.writeRef())));
	}

	// Describe and create the graphics pipeline state object (PSO)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { desc.m_inputElementDescs, UINT(desc.m_numInputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature;
		psoDesc.VS = desc.m_vertexBlob;
		psoDesc.PS = desc.m_pixelBlob;

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.RasterizerState.MultisampleEnable = TRUE;

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;

		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;

		switch (shaderId)
		{
		case Dx12MsaaComposer::PassType::PASS_UPSAMPLE:
		{
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				psoDesc.BlendState.RenderTarget[i].BlendEnable = true;
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			psoDesc.SampleDesc.Count = 1;
			for (int i = 0; i < 4; ++i)
			{
				NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineStateMsaa[i].writeRef())));
				psoDesc.SampleDesc.Count <<= 1;
			}
			psoDesc.SampleDesc.Count = 8;
		}
		break;

		case Dx12MsaaComposer::PassType::PASS_RESOLVE:
		{
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			psoDesc.SampleDesc.Count = 1;
		}
		break;

		case Dx12MsaaComposer::PassType::PASS_POST_COLOR:
		{
			const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				TRUE, FALSE,
				D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ZERO, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_CLEAR,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

			psoDesc.DepthStencilState.DepthEnable = false;

			psoDesc.SampleDesc.Count = 1;
			psoDesc.RTVFormats[0] = desc.m_targetInfo.m_renderTargetFormats[0];// m_formatRTV;
			psoDesc.DSVFormat = desc.m_targetInfo.m_depthStencilFormat;// m_formatDSV;
		}
		break;

		case Dx12MsaaComposer::PassType::PASS_POST_DEPTH:
		{
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			psoDesc.SampleDesc.Count = 1;
			psoDesc.RTVFormats[0] = desc.m_targetInfo.m_renderTargetFormats[0];// m_formatRTV;
			psoDesc.DSVFormat = desc.m_targetInfo.m_depthStencilFormat;// m_formatDSV;
		}
		break;
		}

		NV_RETURN_ON_FAIL(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
	}

	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia
