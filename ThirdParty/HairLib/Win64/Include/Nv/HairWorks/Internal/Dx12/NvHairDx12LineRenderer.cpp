/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>

#include "NvHairDx12LineRenderer.h"

#include <Nv/HairWorks/Internal/NvHairSphereCalculator.h>

#include "d3dx12.h"

namespace nvidia {
namespace HairWorks {

namespace DebugLineBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugLine_Ps.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugLine_Vs.h>
}

// Example of fence based resource management
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn899125%28v=vs.85%29.aspx

Dx12LineRenderer::~Dx12LineRenderer() 
{
}

static D3D12_DEPTH_WRITE_MASK _getWriteMask(EDepthOp op)
{
	switch (op)
	{
		case DepthOp::COUNT_OF:
		case DepthOp::UNKNOWN:
		case DepthOp::WRITE_LESS:
		case DepthOp::WRITE_GREATER:
		case DepthOp::ALWAYS:		
		{
			return D3D12_DEPTH_WRITE_MASK_ALL;
		}
		case DepthOp::TEST_LESS:
		case DepthOp::TEST_GREATER:
		{
			return D3D12_DEPTH_WRITE_MASK_ZERO;
		}
	}
	return D3D12_DEPTH_WRITE_MASK_ALL;
}

static D3D12_COMPARISON_FUNC _getComparisonFunc(EDepthOp op)
{
	switch (op)
	{
		case DepthOp::COUNT_OF:
		case DepthOp::ALWAYS:
		case DepthOp::UNKNOWN:
		{
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}
		case DepthOp::TEST_LESS:
		case DepthOp::WRITE_LESS:
		{
			return D3D12_COMPARISON_FUNC_LESS;
		}
		case DepthOp::TEST_GREATER:
		case DepthOp::WRITE_GREATER:
		{
			return D3D12_COMPARISON_FUNC_GREATER;
		}
	}
	return D3D12_COMPARISON_FUNC_ALWAYS;
}

Result Dx12LineRenderer::init(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& context, const NvCo::Dx12TargetInfo& targetInfo)
{
	ID3D12Device* device = NvCo::Dx12Type::cast<ID3D12Device>(deviceIn);
	ID3D12GraphicsCommandList* commandList = NvCo::Dx12Type::cast<ID3D12GraphicsCommandList>(context);
	NV_CORE_ASSERT(device);
	if (!device) return NV_FAIL;

	m_commandList = commandList;

	NV_RETURN_ON_FAIL(m_viewHeap.init(device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
	NV_RETURN_ON_FAIL(m_fence.init(device));
	NV_RETURN_ON_FAIL(m_resourceScopeManager.init(device, &m_fence));

	{
		NvCo::Dx12CircularResourceHeap::Desc desc;
		desc.init();
		// Define size
		desc.m_blockSize = sizeof(Vertex) * MAX_VERTICES * 2;
		// Set up the heap
		m_circularResourceHeap.init(device, desc, &m_fence);
	}
	
	Dx12Info info;
	info.m_commandList = commandList;
	info.m_device = device;
	info.m_viewHeap = &m_viewHeap;
	info.m_resourceScopeManager = &m_resourceScopeManager;

	{
		Dx12ShaderPass::Desc desc;
		desc.m_vertexBlob = DebugLineBlobs::g_vs_main;
		desc.m_pixelBlob = DebugLineBlobs::g_ps_main;
		desc.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		desc.m_targetInfo = targetInfo;
		
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		desc.m_inputElementDescs = inputElementDescs;
		desc.m_numInputElementDescs = NV_COUNT_OF(inputElementDescs);

		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		desc.m_depthStencilDesc = &depthStencilDesc;

		for (Int i = DepthOp::UNKNOWN + 1; i < DepthOp::COUNT_OF; i++)
		{
			Dx12ShaderPass& pass = m_passes[i];

			depthStencilDesc.DepthFunc = _getComparisonFunc(EDepthOp(i));
			depthStencilDesc.DepthWriteMask = _getWriteMask(EDepthOp(i));
			
			NV_RETURN_ON_FAIL(pass.init(device, NV_NULL, 0, desc));
		}
	}

	{
		SphereCalculator calc;

		calc.calc(SphereCalculator::TYPE_LINE, 1.0f, 10);
		
		const Int numVerts = Int(calc.m_indices.getSize());
		const Int* indices = calc.m_indices.begin();
		const gfsdk_float3* srcVerts = calc.m_points.begin();

		NvCo::PodBuffer<Vertex> buf(numVerts);
		for (Int i = 0; i < numVerts; i++)
		{
			Vertex& vert = buf[i];
			vert.m_position = srcVerts[indices[i]];
			vert.m_color = 0xffffffff;
		}
		m_numSphereLines = numVerts / 2;
		NV_RETURN_ON_FAIL(m_sphereVertexBuffer.init(info, sizeof(Vertex), m_numSphereLines * 2, buf));
	}

	// Allocate enough space for all vertices
	m_buffer.setSize(MAX_VERTICES);
	m_buffer.clear();

	// Set the color
	m_packedColor = _getPackedColor(m_color);
	m_cpuConstantBuffer.color = m_globalColor;
	return NV_OK;
}

Result Dx12LineRenderer::start(const NvCo::ApiContext& contextIn, const gfsdk_float4x4& projection)
{
	NV_UNUSED(projection)

	ID3D12GraphicsCommandList* commandList = NvCo::Dx12Type::cast<ID3D12GraphicsCommandList>(contextIn);
	if (!commandList) return NV_FAIL;

	m_circularResourceHeap.updateCompleted();
	m_resourceScopeManager.updateCompleted();

	// There are no vertices initially
	m_buffer.clear();

	// set constant buffer
	m_cpuConstantBuffer.modelToWorld = m_modelToWorld;
	m_cpuConstantBuffer.viewProjection = projection;
	m_cpuConstantBuffer.color = m_globalColor;
	// Force it to sync the constant buffer if anything is drawn
	m_constantBufferChanged = true;

	// Convert the set color
	m_packedColor = _getPackedColor(m_color);	
	m_mode = MODE_NONE;

	Dx12ShaderPass& pass = m_passes[m_depthOp];
	commandList->SetGraphicsRootSignature(pass.m_rootSignature);
	commandList->SetPipelineState(pass.m_pipelineState);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->IASetIndexBuffer(NV_NULL);

	m_commandList = commandList;

	m_isStarted = true;
	return NV_OK;
}

Void Dx12LineRenderer::end()
{
	NV_CORE_ASSERT(m_isStarted);
	m_isStarted = false;

	// Flush anything remaining
	_flush();

	m_commandList = NV_NULL;
}

Void Dx12LineRenderer::onGpuWorkSubmitted(const NvCo::ApiHandle& handle)
{
	ID3D12CommandQueue* commandQueue = NvCo::Dx12Type::cast<ID3D12CommandQueue>(handle);
	NV_CORE_ASSERT(commandQueue);
	if (!commandQueue)
	{
		NV_CO_LOG_ERROR("Requires ID3D12CommandQueue passed to onGpuWorkSubmitted");
		return;
	}

	const UInt64 signalValue = m_fence.nextSignal(commandQueue);

	m_circularResourceHeap.addSync(signalValue);	
	m_resourceScopeManager.addSync(signalValue);
}

Void Dx12LineRenderer::_setMode(Mode mode)
{
	if (mode == m_mode)
	{
		return;
	}
	m_mode = mode;

	switch (mode)
	{
		case MODE_LINE:
		{
			break;
		}
		case MODE_SPHERE:
		{
			// Set vertices
			m_commandList->IASetVertexBuffers(0, 1, &m_sphereVertexBuffer.m_vertexBufferView);
			break;
		}
	}
}

Result Dx12LineRenderer::_syncConstantBuffer(ID3D12GraphicsCommandList* commandList)
{
	if (m_constantBufferChanged)
	{
		NvCo::Dx12CircularResourceHeap::Cursor cursor = m_circularResourceHeap.allocateConstantBuffer(sizeof(m_cpuConstantBuffer));
		NvCo::Memory::copy(cursor.m_position, &m_cpuConstantBuffer, sizeof(m_cpuConstantBuffer));
		// Set the constant buffer
		commandList->SetGraphicsRootConstantBufferView(0, m_circularResourceHeap.getGpuHandle(cursor));
		// It's been set
		m_constantBufferChanged = false;
	}
	return NV_OK;
}

Result Dx12LineRenderer::_flush()
{
	const Int numVerts = Int(m_buffer.getSize());
	if (numVerts > 0)
	{
		_setMode(MODE_LINE);

		NV_RETURN_ON_FAIL(_syncConstantBuffer(m_commandList));	

		{
			// Map the vertex buffer
			SizeT bufferSize = sizeof(Vertex) * m_buffer.getSize();
			NvCo::Dx12CircularResourceHeap::Cursor cursor = m_circularResourceHeap.allocateVertexBuffer(bufferSize);
			NvCo::Memory::copy(cursor.m_position, m_buffer.begin(), bufferSize);
			// Clear the buffer
			m_buffer.clear();

			// Set the view
			D3D12_VERTEX_BUFFER_VIEW view = {};
			view.BufferLocation = m_circularResourceHeap.getGpuHandle(cursor);
			view.SizeInBytes = UINT(bufferSize);
			view.StrideInBytes = sizeof(Vertex);

			// Set vertices
			m_commandList->IASetVertexBuffers(0, 1, &view);
		}

		// Can now do the render call
		m_commandList->DrawInstanced(numVerts, 1, 0, 0);	
	}
	return NV_OK;
}

Void Dx12LineRenderer::drawLine(const gfsdk_float3& from, const gfsdk_float3& to)
{
	// Can only draw if started
	NV_CORE_ASSERT(m_isStarted);

	if (_needsFlush(2))
	{
		_flush();
	}
	Vertex* verts = m_buffer.expandBy(2);
	verts[0].m_position = from;
	verts[0].m_color = m_packedColor;
	verts[1].m_position = to;
	verts[1].m_color = m_packedColor;
}

Void Dx12LineRenderer::onGlobalColorChange()
{
	_flush();
	m_cpuConstantBuffer.color = m_globalColor;
	m_constantBufferChanged = true;
}

Void Dx12LineRenderer::onColorChange()
{
	m_packedColor = _getPackedColor(m_color);
}

Void Dx12LineRenderer::onModelToWorldChange()
{
	_flush();
	m_cpuConstantBuffer.modelToWorld = m_modelToWorld;
	m_constantBufferChanged = true;
}

Void Dx12LineRenderer::onDepthOpChange()
{
	_flush();
	// Use the appropriate pass type
	Dx12ShaderPass& pass = m_passes[m_depthOp];
	m_commandList->SetGraphicsRootSignature(pass.m_rootSignature);
	m_commandList->SetPipelineState(pass.m_pipelineState);
}

Void Dx12LineRenderer::drawSphere()
{
	if (!m_isStarted)
	{
		return;
	}
	_setMode(MODE_SPHERE);
	_syncConstantBuffer(m_commandList);
	// Can now do the render call
	m_commandList->DrawInstanced(m_numSphereLines * 2, 1, 0, 0);
}

} // namespace HairWorks 
} // namespace nvidia 