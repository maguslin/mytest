/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_BUFFER_H
#define NV_HAIR_DX12_BUFFER_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/Container/NvCoArray.h>
#include <Nv/Common/NvCoMemory.h>
#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoPodBuffer.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12ResourceScopeManager.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12DescriptorHeap.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12CircularResourceHeap.h>

#include <dxgi.h>
#include <d3d12.h>

#include <Nv/HairWorks/NvHairCommon.h>

namespace nvidia {
namespace HairWorks {

template <typename T>
struct GetDxgiFormat { enum { Value = DXGI_FORMAT_UNKNOWN }; };

#define NV_HAIR_GET_DXGI_FORMAT(from, to) template <> struct GetDxgiFormat<from> { enum { Value = DXGI_FORMAT_##to }; };
NV_HAIR_GET_DXGI_FORMAT(Float32, R32_FLOAT)
NV_HAIR_GET_DXGI_FORMAT(gfsdk_float2, R32G32_FLOAT)
NV_HAIR_GET_DXGI_FORMAT(gfsdk_float3, R32G32B32_FLOAT)
NV_HAIR_GET_DXGI_FORMAT(gfsdk_float4, R32G32B32A32_FLOAT)

NV_HAIR_GET_DXGI_FORMAT(Int32, R32_SINT)
NV_HAIR_GET_DXGI_FORMAT(UInt32, R32_UINT)

struct Dx12Info
{
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_commandList;
	NvCo::Dx12DescriptorHeap* m_viewHeap;
	NvCo::Dx12ResourceScopeManager* m_resourceScopeManager;
};

// Enables more conservative barriers - restoring the state of resources after they are used.
// Should not need to be enabled in normal builds, as the barriers should correctly sync resources
// If enabling fixes an issue it implies regular barriers are not correctly used. 
#define NV_HAIR_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS 0

struct Dx12BarrierSubmitter
{
	enum { MAX_BARRIERS = 8 };

		/// Expand one space to hold a barrier
	NV_FORCE_INLINE D3D12_RESOURCE_BARRIER& expandOne() { return (m_numBarriers < MAX_BARRIERS) ? m_barriers[m_numBarriers++] : _expandOne(); }
		/// Flush barriers to command list 
	NV_FORCE_INLINE Void flush() { if (m_numBarriers > 0) _flush(); }

		/// Ctor
	NV_FORCE_INLINE Dx12BarrierSubmitter(ID3D12GraphicsCommandList* commandList):m_numBarriers(0), m_commandList(commandList) { }
		/// Dtor
	NV_FORCE_INLINE ~Dx12BarrierSubmitter() { flush(); }

	protected:
	D3D12_RESOURCE_BARRIER& _expandOne();
	Void _flush();

	ID3D12GraphicsCommandList* m_commandList;
	IndexT m_numBarriers;
	D3D12_RESOURCE_BARRIER m_barriers[MAX_BARRIERS];
};

struct Dx12Resource
{
		/// Ctor
	NV_FORCE_INLINE Dx12Resource():m_state(D3D12_RESOURCE_STATE_COMMON)
#if NV_HAIR_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS
		, m_prevState(D3D12_RESOURCE_STATE_COMMON)
#endif
	{}
		/// Add a transition if necessary to the list
	Void transition(D3D12_RESOURCE_STATES nextState, Dx12BarrierSubmitter& submitter);
		/// get the current state
	NV_FORCE_INLINE D3D12_RESOURCE_STATES getState() const { return m_state; }

		/// Initialize			
	Result init(const Dx12Info& info, const D3D12_HEAP_PROPERTIES& heapProps, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initialState, const void* sysMem);

		/// restore previous state
#if NV_HAIR_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS
	NV_FORCE_INLINE Void restore(Dx12BarrierSubmitter& submitter) { transition(m_prevState, submitter); } 
#else
	NV_FORCE_INLINE Void restore(Dx12BarrierSubmitter& submitter) { NV_UNUSED(submitter) }
#endif

	NvCo::ComPtr<ID3D12Resource> m_buffer;
	protected:
	D3D12_RESOURCE_STATES m_state;
#if NV_HAIR_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS
	D3D12_RESOURCE_STATES m_prevState;
#endif
};

struct Dx12IndexBuffer: public Dx12Resource
{
	Result init(const Dx12Info& info, IndexT numIndices, const void* sysMem);
		/// Reset
	Void reset() { m_buffer.setNull(); }

	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};

struct Dx12VertexBuffer: public Dx12Resource
{
		/// 
	Result init(const Dx12Info& info, Int stride, IndexT numElements, const Void* sysMem);
		/// Get the cpu handle
	D3D12_CPU_DESCRIPTOR_HANDLE getUavCpuHandle() const { return m_viewHeap->getCpuHandle(m_uavIndex); }

		/// Ctor
	Dx12VertexBuffer() :m_viewHeap(NV_NULL), m_uavIndex(-1) {}

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_uavIndex;
};

struct Dx12ReadOnlyBuffer: public Dx12Resource
{
	template <typename T>
	NV_FORCE_INLINE Result init(const Dx12Info& info, IndexT numElements, const T* sysMem)
	{
		DXGI_FORMAT format = DXGI_FORMAT(GetDxgiFormat<T>::Value);
		NV_CORE_ASSERT(format != DXGI_FORMAT_UNKNOWN);
		return init(info, format, sizeof(T), numElements, sysMem);
	}
	template <typename T>
	NV_FORCE_INLINE Result init(const Dx12Info& info, IndexT numElements, const NvCo::PodBuffer<T>& sysMem)
	{
		// Pass the stride as 0, as not structured buffer
		DXGI_FORMAT format = DXGI_FORMAT(GetDxgiFormat<T>::Value);
		NV_CORE_ASSERT(format != DXGI_FORMAT_UNKNOWN);
		return init(info, format, sizeof(T), numElements, sysMem);
	}

		/// Stride must always be set, to calculate the buffer size correctly. 
		/// If the BUT if format != DXGI_FORMAT_UNKNOWN, it will be set to 0 in the view (as it will not be a structred buffer)
	Result init(const Dx12Info& info, DXGI_FORMAT format, Int stride, IndexT numElements, const void* sysMem);

	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_viewHeap->getCpuHandle(m_srvIndex); }

		/// Ctor
	Dx12ReadOnlyBuffer() :m_viewHeap(NV_NULL), m_srvIndex(-1) {}

	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
};

struct Dx12StructuredBuffer: public Dx12Resource
{	
		/// Initialize - must be called before any other methods
	Result init(const Dx12Info& info, Int stride, IndexT numElements, const void* sysMem, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);
		/// Can only be called after init, recreates the resources and reset the descriptors
	Result update(const Dx12Info& info, Int stride, IndexT numElements, const void* sysMem, D3D12_RESOURCE_STATES initialState);

	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_viewHeap ? m_viewHeap->getCpuHandle(m_srvIndex) : D3D12_CPU_DESCRIPTOR_HANDLE(); }
		
	D3D12_CPU_DESCRIPTOR_HANDLE getUavCpuHandle() const { return m_viewHeap ? m_viewHeap->getCpuHandle(m_uavIndex) : D3D12_CPU_DESCRIPTOR_HANDLE(); }

		/// Ctor
	Dx12StructuredBuffer() :m_viewHeap(NV_NULL), m_srvIndex(-1), m_uavIndex(-1) {}

	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
	Int m_uavIndex;
};

struct Dx12StreamOutBuffer: public Dx12Resource
{
	Result init(const Dx12Info& info, DXGI_FORMAT format, Int stride, IndexT numElements);

		/// Get the srv CPU handle
	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_viewHeap->getCpuHandle(m_srvIndex); }

		/// Get the next stream out view
		/// Note! Allocates space on the (should be upload) heap to store size of a stream out result
		/// and zeros it. 
	D3D12_STREAM_OUTPUT_BUFFER_VIEW nextStreamOutView(D3D12_GPU_VIRTUAL_ADDRESS sizeLocation) const; 

		/// Copy to cpu array (copys from readbackBuffer if set)
	Void copyToCpu(NvCo::Array<gfsdk_float4>& out) const;

		/// Ctor
	Dx12StreamOutBuffer() :m_viewHeap(NV_NULL), m_srvIndex(-1), m_sizeInBytes(0) {}

	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
	Int m_sizeInBytes;					///< Total buffer size in bytes

	NvCo::ComPtr<ID3D12Resource> m_readBackBuffer;
};

struct Dx12RenderTargetView: public Dx12Resource
{
	Result init(const Dx12Info& info, const D3D12_RESOURCE_DESC* pResourceDesc, const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc);

	/// Get the rtv CPU handle
	D3D12_CPU_DESCRIPTOR_HANDLE getRtvCpuHandle() const { return m_rtvHeap.getCpuHandle(0); }
	/// Get the srv CPU handle
	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_viewHeap->getCpuHandle(m_srvIndex); }

	/// Ctor
	Dx12RenderTargetView() :m_viewHeap(NV_NULL), m_srvIndex(-1) {}

	NvCo::Dx12DescriptorHeap m_rtvHeap;
	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
};

struct Dx12DepthStencilView: public Dx12Resource
{
	Result init(const Dx12Info& info, const D3D12_RESOURCE_DESC* pResourceDesc, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc);

	/// Get the dsv CPU handle
	D3D12_CPU_DESCRIPTOR_HANDLE getDsvCpuHandle() const { return m_dsvHeap.getCpuHandle(0); }
	/// Get the srv CPU handle
	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_viewHeap->getCpuHandle(m_srvIndex); }

	/// Ctor
	Dx12DepthStencilView() :m_viewHeap(NV_NULL), m_srvIndex(-1) {}

	NvCo::Dx12DescriptorHeap m_dsvHeap;
	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
};

struct Dx12Texture
{
		/// Initialize	
	Result init(NvCo::Dx12DescriptorHeap* heap);

		/// True if it's set		
	Bool isSet() const { return m_texture != NV_NULL; }

		/// Set 
	Void set(ID3D12Device* device, ID3D12Resource* resource, DXGI_FORMAT pixelFormat);
		/// Reset its state
	Void reset();

		/// Get the handle
	D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpuHandle() const { return m_texture ? m_viewHeap->getCpuHandle(m_srvIndex) : D3D12_CPU_DESCRIPTOR_HANDLE(); }

		/// Ctor
	Dx12Texture() :m_viewHeap(NV_NULL), m_srvIndex(-1) {}

	NvCo::ComPtr<ID3D12Resource> m_texture;
	NvCo::Dx12DescriptorHeap* m_viewHeap;
	Int m_srvIndex;
};

struct Dx12Bundle
{
	typedef SizeT Hash;

		/// Initialize the batch
	Result init(ID3D12Device* device);
		/// If it returns true need to fill in the bundle
	Bool start(ID3D12DescriptorHeap*const* heaps, Int numHeaps, const D3D12_GPU_DESCRIPTOR_HANDLE* handles, Int numHandles, Int optionalSeed = 0) { return start(calcHash(heaps, numHeaps, handles, numHandles, optionalSeed)); }
	Bool forceStart(ID3D12DescriptorHeap*const* heaps, Int numHeaps, const D3D12_GPU_DESCRIPTOR_HANDLE* handles, Int numHandles, Int optionalSeed = 0) { return forceStart(calcHash(heaps, numHeaps, handles, numHandles, optionalSeed)); }
	Bool start(Hash hash);
	Bool forceStart(Hash hash);

	Void end();

		///  
	NV_FORCE_INLINE operator ID3D12GraphicsCommandList*() const { return m_bundle; }

	NV_FORCE_INLINE ID3D12GraphicsCommandList* getCommandList() const { return m_bundle; }

		/// Reset the state (invalidate the hash)
	NV_FORCE_INLINE Void reset() { m_hash = 0; }

		/// Calculate a hash
	static SizeT calcHash(ID3D12DescriptorHeap*const* heaps, Int numHeaps, const D3D12_GPU_DESCRIPTOR_HANDLE* handles, Int numHandles, Int optionalSeed = 0);

		/// Ctor
	Dx12Bundle():m_hash(0) {}

	protected:
	Hash m_hash;
	NvCo::ComPtr<ID3D12CommandAllocator> m_allocator;
	NvCo::ComPtr<ID3D12GraphicsCommandList>	m_bundle;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_BUFFER_H
