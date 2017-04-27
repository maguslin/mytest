/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairDx12Buffer.h"

#include <Nv/Common/Platform/Dx12/NvCoDx12HelperUtil.h>
#include <Nv/HairWorks/Internal/Dx/NvHairDxUtil.h>

#include "d3dx12.h"

namespace nvidia { 
namespace HairWorks {

Void Dx12BarrierSubmitter::_flush()
{
	NV_CORE_ASSERT(m_numBarriers > 0);

	if (m_commandList)
	{
		m_commandList->ResourceBarrier(UINT(m_numBarriers), m_barriers);
	}
	m_numBarriers = 0;
}

D3D12_RESOURCE_BARRIER& Dx12BarrierSubmitter::_expandOne()
{
	_flush();
	return m_barriers[m_numBarriers++];
}

Void Dx12Resource::transition(D3D12_RESOURCE_STATES nextState, Dx12BarrierSubmitter& submitter)
{
	if (nextState != m_state)
	{
		D3D12_RESOURCE_BARRIER& barrier = submitter.expandOne();
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_buffer, m_state, nextState);
#if NV_HAIR_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS
		m_prevState = m_state;
#endif
		m_state = nextState;
	}
	else
	{
		if (nextState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			D3D12_RESOURCE_BARRIER& barrier = submitter.expandOne();
			barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_buffer);
			m_state = nextState;
		}
	}
}

Result Dx12StreamOutBuffer::init(const Dx12Info& info, DXGI_FORMAT format, Int stride, IndexT numElements)
{
	m_viewHeap = info.m_viewHeap;
	m_srvIndex = info.m_viewHeap->allocate();

	const SizeT bufferSize = SizeT(numElements * stride);
	m_sizeInBytes = Int(bufferSize);

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
		NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, NV_NULL,
			IID_PPV_ARGS(m_buffer.writeRef())));

		m_state = D3D12_RESOURCE_STATE_COMMON;
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Format = format;  // DXGI_FORMAT_UNKNOWN;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Buffer.FirstElement = 0;
		viewDesc.Buffer.NumElements = UINT(numElements);
		// If the format is set, then it's a regular 'buffer', so the stride should be 0
		// If it's a structured buffer (format = DXGI_FORMAT_UNKNOWN), the stride should be set
		viewDesc.Buffer.StructureByteStride = (format == DXGI_FORMAT_UNKNOWN) ? stride : 0;
		viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		
		info.m_device->CreateShaderResourceView(m_buffer, &viewDesc, info.m_viewHeap->getCpuHandle(m_srvIndex));
	}

#ifdef NV_DEBUG
	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_READBACK);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
		NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL,
			IID_PPV_ARGS(m_readBackBuffer.writeRef())));
	}
#endif

	return NV_OK;
}

D3D12_STREAM_OUTPUT_BUFFER_VIEW Dx12StreamOutBuffer::nextStreamOutView(D3D12_GPU_VIRTUAL_ADDRESS sizeLocation) const
{
	// Note: It's not 100% clear what the alignment is needed from the docs. 
	// In other engines I've seen D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT used, but I don't think that applies as thats used for the 
	// D3D12_BUFFER_UAV structures CounterOffsetInBytes member it seems. 
	// Unless other information comes to light - 16 seems like a reasonable alignment
	//Dx12CircularResourceHeap::Cursor cursor = heap.allocate(sizeof(UInt64), sizeof(16));

	//UInt64* dst = (UInt64*)cursor.m_position;
	//*dst = 0;

	D3D12_STREAM_OUTPUT_BUFFER_VIEW view;
	view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	view.SizeInBytes = m_sizeInBytes;
	view.BufferFilledSizeLocation = sizeLocation;
	return view;
}

Void Dx12StreamOutBuffer::copyToCpu(NvCo::Array<gfsdk_float4>& out) const
{
	NV_CORE_ASSERT(m_readBackBuffer);
	if (m_readBackBuffer)
	{
		NV_CORE_ASSERT(m_sizeInBytes == out.getSize() * sizeof(gfsdk_float4));
		// Do the copy... 
		D3D12_RANGE range;
		range.Begin = 0;
		range.End = m_sizeInBytes;

		const Void* src;
		m_readBackBuffer->Map(0, &range, (void**)&src);
		NvCo::Memory::copy(out.begin(), src, m_sizeInBytes);
		m_readBackBuffer->Unmap(0, NV_NULL);
	}
}

Result Dx12IndexBuffer::init(const Dx12Info& info, IndexT numIndices, const void* sysMem)
{
	NV_CORE_ASSERT(sysMem);
	const SizeT bufferSize = sizeof(UInt32) * numIndices;

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
		if (sysMem)
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL, 
				IID_PPV_ARGS(m_buffer.writeRef())));
			NV_RETURN_ON_FAIL(info.m_resourceScopeManager->upload(info.m_commandList, sysMem, m_buffer, D3D12_RESOURCE_STATE_COMMON));
		}
		else
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, NV_NULL,
				IID_PPV_ARGS(m_buffer.writeRef())));
			
		}
	}
	m_state = D3D12_RESOURCE_STATE_COMMON;
	
	NvCo::Memory::zero(m_indexBufferView);
	m_indexBufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = UINT(bufferSize);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	
	return NV_OK;
}

Result Dx12ReadOnlyBuffer::init(const Dx12Info& info, DXGI_FORMAT format, Int stride, IndexT numElements, const void* sysMem)
{
	NV_CORE_ASSERT(sysMem);
	NV_CORE_ASSERT(stride > 0);

	m_viewHeap = info.m_viewHeap;
	m_srvIndex = info.m_viewHeap->allocate();
	
	const SizeT bufferSize = SizeT(numElements * stride);

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize));
		NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL,
			IID_PPV_ARGS(m_buffer.writeRef())));
	}

	NV_RETURN_ON_FAIL(info.m_resourceScopeManager->upload(info.m_commandList, sysMem, m_buffer, D3D12_RESOURCE_STATE_GENERIC_READ));
	m_state = D3D12_RESOURCE_STATE_GENERIC_READ;

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = format;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = UINT(numElements);
	
		// If the format is set, then it's a regular 'buffer', so the stride should be 0
		// If it's a structured buffer (format = DXGI_FORMAT_UNKNOWN), the stride should be set
		srvDesc.Buffer.StructureByteStride = (format == DXGI_FORMAT_UNKNOWN) ? stride : 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		info.m_device->CreateShaderResourceView(m_buffer, &srvDesc, m_viewHeap->getCpuHandle(m_srvIndex));
	}

	return NV_OK;
}

Result Dx12Resource::init(const Dx12Info& info, const D3D12_HEAP_PROPERTIES& heapProps, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initialState, const void* sysMem)
{
	ID3D12Device* device = info.m_device;
	if (sysMem)
	{
		NV_RETURN_ON_FAIL(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL,
			IID_PPV_ARGS(m_buffer.writeRef())));
		NV_RETURN_ON_FAIL(info.m_resourceScopeManager->upload(info.m_commandList, sysMem, m_buffer, initialState));
	}
	else
	{
		NV_RETURN_ON_FAIL(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, NV_NULL,
			IID_PPV_ARGS(m_buffer.writeRef())));
	}
	m_state = initialState;
	return NV_OK;
}

Result Dx12VertexBuffer::init(const Dx12Info& info, Int stride, IndexT numElements, const Void* sysMem)
{
	NV_CORE_ASSERT(sysMem);

	m_viewHeap = info.m_viewHeap;
	m_uavIndex = info.m_viewHeap->allocate();

	SizeT bufferSize = SizeT(numElements * stride);

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

		if (sysMem)
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL, 
				IID_PPV_ARGS(m_buffer.writeRef())));
			NV_RETURN_ON_FAIL(info.m_resourceScopeManager->upload(info.m_commandList, sysMem, m_buffer, D3D12_RESOURCE_STATE_COMMON));
		}
		else
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, NV_NULL,
				IID_PPV_ARGS(m_buffer.writeRef())));
		}
		m_state = D3D12_RESOURCE_STATE_COMMON;
	}

	{
		NvCo::Memory::zero(&m_vertexBufferView, sizeof(m_vertexBufferView));
		m_vertexBufferView.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = UINT(bufferSize);
		m_vertexBufferView.StrideInBytes = stride;
	}

	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = UINT(numElements);
		uavDesc.Buffer.StructureByteStride = stride;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		info.m_device->CreateUnorderedAccessView(m_buffer, NV_NULL, &uavDesc, m_viewHeap->getCpuHandle(m_uavIndex));
	}

	return NV_OK;
}

Result Dx12StructuredBuffer::init(const Dx12Info& info, Int stride, IndexT numElements, const void* sysMem, D3D12_RESOURCE_STATES initialState)
{
	m_viewHeap = info.m_viewHeap;

	m_srvIndex = m_viewHeap->allocate();
	m_uavIndex = m_viewHeap->allocate();

	return update(info, stride, numElements, sysMem, initialState);
}

Result Dx12StructuredBuffer::update(const Dx12Info& info, Int stride, IndexT numElements, const void* sysMem, D3D12_RESOURCE_STATES initialState)
{
	const SizeT bufferSize = SizeT(numElements * stride);

	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc(CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

		if (sysMem)
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NV_NULL,
				IID_PPV_ARGS(m_buffer.writeRef())));
			NV_RETURN_ON_FAIL(info.m_resourceScopeManager->upload(info.m_commandList, sysMem, m_buffer, initialState));
		}
		else
		{
			NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, NV_NULL,
				IID_PPV_ARGS(m_buffer.writeRef())));
		}
		m_state = initialState;
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = UINT(numElements);
		srvDesc.Buffer.StructureByteStride = stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		info.m_device->CreateShaderResourceView(m_buffer, &srvDesc, m_viewHeap->getCpuHandle(m_srvIndex));
	}

	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = UINT(numElements);
		uavDesc.Buffer.StructureByteStride = stride;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		info.m_device->CreateUnorderedAccessView(m_buffer, NV_NULL, &uavDesc, m_viewHeap->getCpuHandle(m_uavIndex));
	}

	return NV_OK;
}

Result Dx12RenderTargetView::init(const Dx12Info& info, const D3D12_RESOURCE_DESC* pResourceDesc, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc)
{
	m_rtvHeap.init(info.m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	m_rtvHeap.allocate();
	m_viewHeap = info.m_viewHeap;
	m_srvIndex = info.m_viewHeap->allocate();

	{
		D3D12_CLEAR_VALUE rtvClearValue;
		rtvClearValue.Format = pRtvDesc->Format;
		rtvClearValue.Color[0] = 0.0f;
		rtvClearValue.Color[1] = 0.0f;
		rtvClearValue.Color[2] = 0.0f;
		rtvClearValue.Color[3] = 0.0f;

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, pResourceDesc, D3D12_RESOURCE_STATE_COMMON, &rtvClearValue,
			IID_PPV_ARGS(m_buffer.writeRef())));

		info.m_device->CreateRenderTargetView(m_buffer, pRtvDesc, m_rtvHeap.getCpuHandle(0));

		m_state = D3D12_RESOURCE_STATE_COMMON;
	}

	if (pSrvDesc)
	{
		info.m_device->CreateShaderResourceView(m_buffer, pSrvDesc, m_viewHeap->getCpuHandle(m_srvIndex));
	}

	return NV_OK;
}

Result Dx12DepthStencilView::init(const Dx12Info& info, const D3D12_RESOURCE_DESC* pResourceDesc, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc)
{
	m_dsvHeap.init(info.m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	m_dsvHeap.allocate();
	m_viewHeap = info.m_viewHeap;
	m_srvIndex = info.m_viewHeap->allocate();

	{
		D3D12_CLEAR_VALUE dsvClearValue;
		dsvClearValue.Format = pDsvDesc->Format;
		dsvClearValue.DepthStencil.Depth = 1.0f;
		dsvClearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		NV_RETURN_ON_FAIL(info.m_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, pResourceDesc, D3D12_RESOURCE_STATE_COMMON, &dsvClearValue,
			IID_PPV_ARGS(m_buffer.writeRef())));

		info.m_device->CreateDepthStencilView(m_buffer, pDsvDesc, m_dsvHeap.getCpuHandle(0));

		m_state = D3D12_RESOURCE_STATE_COMMON;
	}

	if (pSrvDesc)
	{
		info.m_device->CreateShaderResourceView(m_buffer, pSrvDesc, m_viewHeap->getCpuHandle(m_srvIndex));
	}

	return NV_OK;
}

Result Dx12Texture::init(NvCo::Dx12DescriptorHeap* heap)
{
	m_viewHeap = heap;
	m_srvIndex = heap->allocate();
	return NV_OK;
}

Void Dx12Texture::set(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT pixelFormat)
{
	if (!resource)
	{
		reset();
		return;
	}

	// allocate heap
	m_texture = resource;	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	NvCo::Dx12HelperUtil::calcSrvDesc(resource, pixelFormat, srvDesc);

	// Copy to the descriptor
	device->CreateShaderResourceView(m_texture, &srvDesc, m_viewHeap->getCpuHandle(m_srvIndex));
}

Void Dx12Texture::reset()
{
	m_texture.setNull();
}

Result Dx12Bundle::init(ID3D12Device* device)
{
	NV_RETURN_ON_FAIL(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(m_allocator.writeRef())));
	NV_RETURN_ON_FAIL(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, m_allocator, NV_NULL, IID_PPV_ARGS(m_bundle.writeRef())));

	m_bundle->Close();
	return NV_OK;
}

Bool Dx12Bundle::start(Hash hash)
{
	if (hash != m_hash)
	{
		NV_CORE_ASSERT(m_bundle);
		m_allocator->Reset();
		m_bundle->Reset(m_allocator, NV_NULL);
		m_hash = hash;
		// Leave it open
		return true;
	}
	else
	{
		return false;
	}
}

Bool Dx12Bundle::forceStart(Hash hash)
{
	NV_CORE_ASSERT(m_bundle);
	m_allocator->Reset();
	m_bundle->Reset(m_allocator, NV_NULL);
	m_hash = hash;
	// Leave it open
	return true;
}

Void Dx12Bundle::end()
{
	m_bundle->Close();
}

SizeT Dx12Bundle::calcHash(ID3D12DescriptorHeap*const* heaps, Int numHeaps, const D3D12_GPU_DESCRIPTOR_HANDLE* handles, Int numHandles, Int optionalSeed)
{
	Hash hash = numHeaps + (numHandles << 4) + (optionalSeed << 12);
	{
		const SizeT* src = (const SizeT*)heaps;
		// Rabin-Karp hash
		for (Int i = 0; i < numHeaps; i++)
		{
			hash = hash * 31 + src[i];
		}
	}
	{
		const SizeT* src = (const SizeT*)handles;
		const Int size = (sizeof(D3D12_GPU_DESCRIPTOR_HANDLE) * numHandles) / sizeof(SizeT);

		// Rabin-Karp hash
		for (Int i = 0; i < size; i++)
		{
			hash = hash * 31 + src[i];
		}
	}
	// 0 isn't a valid value
	hash += (hash == 0);
	return hash;
}


} // namespace HairWorks
} // namespace nvidia

