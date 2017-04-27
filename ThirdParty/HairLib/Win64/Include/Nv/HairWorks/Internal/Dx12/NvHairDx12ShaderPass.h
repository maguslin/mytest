/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_SHADER_PASS_H
#define NV_HAIR_DX12_SHADER_PASS_H

#include <dxgi.h>
#include <d3d12.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoComPtr.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>

namespace nvidia {
namespace HairWorks {

class Dx12ShaderPass
{
	NV_CO_DECLARE_CLASS_BASE(Dx12ShaderPass);

		/// Way to pass statically defined compiled blobs conveniently. 
	struct Blob: public D3D12_SHADER_BYTECODE
	{
		template <SizeT SIZE>
		NV_FORCE_INLINE Blob(const BYTE (&in)[SIZE]) { pShaderBytecode = in; BytecodeLength = SIZE; }
		NV_FORCE_INLINE Blob(const BYTE* in, SizeT size) { pShaderBytecode = in; BytecodeLength = size; }
		NV_FORCE_INLINE Blob() { pShaderBytecode = NV_NULL; BytecodeLength = 0; }
	};

	struct DescriptorDesc
	{
		DescriptorDesc();

		NV_FORCE_INLINE Void setNumCbvSrvUavSamplers(Int numCbv, Int numSrv, Int numUav, Int numSamplers) { m_numCbvs = numCbv; m_numSrvs = numSrv; m_numUavs = numUav; m_numSamplers = numSamplers; }

		Bool m_hasDynamicConstantBuffer;
		Int m_numCbvs;
		Int m_numSrvs;
		Int m_numUavs;
		Int m_numSamplers;
	};

	struct Desc
	{
		Desc();

		const D3D12_INPUT_ELEMENT_DESC* m_inputElementDescs;
		Int m_numInputElementDescs;
		
		D3D12_STREAM_OUTPUT_DESC m_streamOutputDesc;

		Blob m_vertexBlob;
		Blob m_hullBlob;
		Blob m_domainBlob;
		Blob m_geometryBlob;
		Blob m_pixelBlob;

		NvCo::Dx12TargetInfo m_targetInfo;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType;
	
		const D3D12_RASTERIZER_DESC* m_rasterizerDesc;
		const D3D12_BLEND_DESC* m_blendDesc;
		const D3D12_DEPTH_STENCIL_DESC* m_depthStencilDesc;
	};

		/// Set fields in pso based on descIn
	static Void setGraphicsPipelineStateDesc(const Desc& descIn, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

	//Result createGraphicsPipelineState(ID3D12Device* device, const Blob& vertexBlob, const Blob& pixelBlob);
		/// Set vertex and pixel
	Result init(ID3D12Device* device, const D3D12_DESCRIPTOR_RANGE* ranges, Int numRanges, const Desc& desc);

		/// Inits for stream out (vertex and geometry shaders)
	Result initStreamOut(ID3D12Device* device, const DescriptorDesc& descripDesc, const Desc& desc);
		/// 
	Result initPatch(ID3D12Device* device, const DescriptorDesc& descripDesc, const Desc& desc, const DescriptorDesc* optionalDesc);
		/// Init compute
	Result initCompute(ID3D12Device* device, const DescriptorDesc& descripDesc, const Blob& computeBlob);
		/// Init supersample
	Result initSuperSample(ID3D12Device* device, const Desc& desc, const UInt type);

		/// True if it's initialized (and presumably usable)
	NV_FORCE_INLINE Bool isInitialized() const { return m_rootSignature != NV_NULL; }

		/// Reset to initial state
	Void reset() { m_rootSignature.setNull(); m_pipelineState.setNull(); }

	NvCo::ComPtr<ID3D12RootSignature> m_rootSignature;
	NvCo::ComPtr<ID3D12PipelineState> m_pipelineState;
	NvCo::ComPtr<ID3D12PipelineState> m_pipelineStateMsaa[4];
};


} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX12_SHADER_PASS_H
