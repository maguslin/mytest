/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include "NvHairDx11LineRenderer.h"

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11Util.h>
#include <Nv/HairWorks/Internal/NvHairSphereCalculator.h>

namespace nvidia {
namespace HairWorks {

namespace DebugLineBlobs
{
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugLine_Ps.h>
#include <Nv/HairWorks/Internal/Shader/Generated/NvHairDebugLine_Vs.h>
}

Dx11LineRenderer::Dx11LineRenderer(Dx11DepthOpState& depthOpState):
	m_depthOpState(depthOpState)
{
}

Dx11LineRenderer::~Dx11LineRenderer() 
{
}

Result Dx11LineRenderer::init(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn)
{
	NV_UNUSED(contextIn);

	ID3D11Device* device = NvCo::Dx11Type::cast<ID3D11Device>(deviceIn);
	NV_CORE_ASSERT(device);
	if (!device) return NV_FAIL;

	{
		NV_RETURN_ON_FAIL(m_pass.createVertexShader(device, DebugLineBlobs::g_vs_main));
		NV_RETURN_ON_FAIL(m_pass.createPixelShader(device, DebugLineBlobs::g_ps_main));

		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		NV_RETURN_ON_FAIL(m_pass.createInputLayout(device, DebugLineBlobs::g_vs_main, layout, NV_COUNT_OF(layout)));
	}

	const UINT bindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_BUFFER_DESC bd = Dx11Util::createBufferDesc(D3D11_USAGE_DYNAMIC, sizeof(Vertex) * MAX_VERTICES, sizeof(Vertex), bindFlags, 0, D3D11_CPU_ACCESS_WRITE);
	NV_RETURN_ON_FAIL(Dx11Util::createBuffer(device, bd, NV_NULL, m_vertexBuffer.writeRef()));
	NV_RETURN_ON_FAIL(Dx11Util::createConstantBuffer(device, sizeof(ConstantBuffer), m_constantBuffer.writeRef()));

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
		NV_RETURN_ON_FAIL(Dx11Util::createVertexBuffer(device, sizeof(Vertex), m_numSphereLines * 2, buf, m_sphereVertexBuffer.writeRef(), NV_NULL));
	}

	// Allocate enough space for all vertices
	m_buffer.setSize(MAX_VERTICES);
	m_buffer.clear();

	// Set the color
	m_packedColor = _getPackedColor(m_color);
	m_cpuConstantBuffer.color = m_globalColor;
	return NV_OK;
}

Result Dx11LineRenderer::start(const NvCo::ApiContext& contextIn, const gfsdk_float4x4& projection)
{
	ID3D11DeviceContext* context = NvCo::Dx11Type::cast<ID3D11DeviceContext>(contextIn);
	if (!context) return NV_FAIL;

	// There are no vertices initially
	m_buffer.clear();

	m_pass.enable(context, false);

	// set constant buffer
	m_cpuConstantBuffer.modelToWorld = m_modelToWorld;
	m_cpuConstantBuffer.viewProjection = projection;
	m_cpuConstantBuffer.color = m_globalColor;
	// Force it to sync the constant buffer if anything is drawn
	m_constantBufferChanged = true;

	// Convert the set color
	m_packedColor = _getPackedColor(m_color);

	context->VSSetConstantBuffers(0, 1, m_constantBuffer.readRef());
	context->PSSetConstantBuffers(0, 1, m_constantBuffer.readRef());

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers( 0, 1, m_vertexBuffer.readRef(), &stride, &offset );

	m_context = context;
	m_isStarted = true;
	return NV_OK;
}

Void Dx11LineRenderer::end()
{
	NV_CORE_ASSERT(m_isStarted);
	m_isStarted = false;

	// Flush anything remaining
	_flush();

	// Disable the pass
	m_pass.disable(m_context, false);

	// Detatch 
	{
		ID3D11Buffer* nullBuffer = NV_NULL;
		m_context->VSSetConstantBuffers(0, 1, &nullBuffer);
		m_context->PSSetConstantBuffers(0, 1, &nullBuffer);

		UINT stride = 0, offset = 0;
		m_context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	}

	m_context = NV_NULL;
}

Result Dx11LineRenderer::_syncConstantBuffer()
{
	if (m_constantBufferChanged)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		NV_RETURN_ON_FAIL(m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		memcpy(mappedResource.pData, &m_cpuConstantBuffer, sizeof(m_cpuConstantBuffer));
		m_context->Unmap(m_constantBuffer, 0);
		m_constantBufferChanged = false;
	}
	return NV_OK;
}

Result Dx11LineRenderer::_flush()
{
	const Int numVerts = Int(m_buffer.getSize());
	if (numVerts > 0)
	{
		NV_RETURN_ON_FAIL(_syncConstantBuffer());	
		// Map vertex buffer
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			// Might be better with D3D11_MAP_WRITE_NO_OVERWRITE, but this works for now
			NV_RETURN_ON_FAIL(m_context->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			NvCo::Memory::copy(mappedResource.pData, m_buffer.begin(), sizeof(Vertex) * m_buffer.getSize());
			m_context->Unmap(m_vertexBuffer, 0);
			m_buffer.clear();
		}

		// Can now do the render call
		m_context->Draw(numVerts, 0);
	}
	return NV_OK;
}

Void Dx11LineRenderer::drawLine(const gfsdk_float3& from, const gfsdk_float3& to)
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

Void Dx11LineRenderer::onGlobalColorChange()
{
	_flush();
	m_cpuConstantBuffer.color = m_globalColor;
	m_constantBufferChanged = true;
}

Void Dx11LineRenderer::onColorChange()
{
	m_packedColor = _getPackedColor(m_color);
}

Void Dx11LineRenderer::onModelToWorldChange()
{
	_flush();
	m_cpuConstantBuffer.modelToWorld = m_modelToWorld;
	m_constantBufferChanged = true;
}

Void Dx11LineRenderer::onDepthOpChange()
{
	_flush();
	m_depthOpState.setDepthOp(m_context, m_depthOp);
}

Void Dx11LineRenderer::drawSphere()
{
	if (!m_isStarted)
	{
		return;
	}

	_syncConstantBuffer();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, m_sphereVertexBuffer.readRef(), &stride, &offset);
	m_context->Draw(m_numSphereLines * 2, 0);
	// Reset to line drawing VB
	m_context->IASetVertexBuffers( 0, 1, m_vertexBuffer.readRef(), &stride, &offset );
}

Void Dx11LineRenderer::onGpuWorkSubmitted(const NvCo::ApiHandle& handle)
{
	// Work is submitted immediately to Dx11 so nothing to do here
	NV_UNUSED(handle);
}

} // namespace HairWorks 
} // namespace nvidia 