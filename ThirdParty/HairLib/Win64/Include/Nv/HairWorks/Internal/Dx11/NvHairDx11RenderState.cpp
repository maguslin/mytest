/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoMemory.h>
#include "NvHairDx11RenderState.h"

namespace nvidia {
namespace HairWorks {

///////////////////////////////////////////////////////////////////////////////////////
Dx11RenderState::ConstantBufferState::ConstantBufferState()
{
	NvCo::Memory::zero(*this);
}

void Dx11RenderState::ConstantBufferState::save(ID3D11DeviceContext* context)
{
	// store buffer and resource states
	context->VSGetConstantBuffers(0, 1, &m_buffers[STAGE_VS]);
	context->PSGetConstantBuffers(0, 1, &m_buffers[STAGE_PS]);
	context->DSGetConstantBuffers(0, 1, &m_buffers[STAGE_DS]);
	context->GSGetConstantBuffers(0, 1, &m_buffers[STAGE_GS]);
	context->HSGetConstantBuffers(0, 1, &m_buffers[STAGE_HS]);
	context->CSGetConstantBuffers(0, 1, &m_buffers[STAGE_CS]);
}
void Dx11RenderState::ConstantBufferState::restore(ID3D11DeviceContext* context)
{
	context->VSSetConstantBuffers(0, 1, &m_buffers[STAGE_VS]);
	context->PSSetConstantBuffers(0, 1, &m_buffers[STAGE_PS]);
	context->DSSetConstantBuffers(0, 1, &m_buffers[STAGE_DS]);
	context->GSSetConstantBuffers(0, 1, &m_buffers[STAGE_GS]);
	context->HSSetConstantBuffers(0, 1, &m_buffers[STAGE_HS]);
	context->CSSetConstantBuffers(0, 1, &m_buffers[STAGE_CS]);
	for (int i = 0; i < STAGE_COUNT_OF; i++)
	{
		ID3D11Buffer* buf = m_buffers[i];
		if (buf)
		{
			buf->Release();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
Dx11RenderState::ShaderResourceViewState::ShaderResourceViewState()
{
	NvCo::Memory::zero(*this);
}
void Dx11RenderState::ShaderResourceViewState::save(ID3D11DeviceContext* context)
{
	context->VSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_VS * MAX_STATES);
	context->PSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_PS * MAX_STATES);
	context->HSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_HS * MAX_STATES);
	context->DSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_DS * MAX_STATES);
	context->GSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_GS * MAX_STATES);
	context->CSGetShaderResources( 0, MAX_STATES, m_srvs + STAGE_CS * MAX_STATES);
}
void Dx11RenderState::ShaderResourceViewState::restore(ID3D11DeviceContext* context)
{
	context->VSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_VS * MAX_STATES);
	context->PSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_PS * MAX_STATES);
	context->HSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_HS * MAX_STATES);
	context->DSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_DS * MAX_STATES);
	context->GSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_GS * MAX_STATES);
	context->CSSetShaderResources( 0, MAX_STATES, m_srvs + STAGE_CS * MAX_STATES);
	// *GetShaderResources() functions increment ref counter, need to clean up here.
	for (int i = 0; i < NV_COUNT_OF(m_srvs); i++)
	{
		ID3D11ShaderResourceView* srv = m_srvs[i];
		if (srv)
		{
			srv->Release();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
Dx11RenderState::SamplerState::SamplerState()
{
	NvCo::Memory::zero(*this);
}
void Dx11RenderState::SamplerState::save(ID3D11DeviceContext* context)
{
	context->VSGetSamplers(0, MAX_STATES, &m_states[STAGE_VS * MAX_STATES]);
	context->PSGetSamplers(0, MAX_STATES, &m_states[STAGE_PS * MAX_STATES]);
	context->HSGetSamplers(0, MAX_STATES, &m_states[STAGE_HS * MAX_STATES]);
	context->DSGetSamplers(0, MAX_STATES, &m_states[STAGE_DS * MAX_STATES]);
	context->GSGetSamplers(0, MAX_STATES, &m_states[STAGE_GS * MAX_STATES]);
	context->CSGetSamplers(0, MAX_STATES, &m_states[STAGE_CS * MAX_STATES]);
}
void Dx11RenderState::SamplerState::restore(ID3D11DeviceContext* context)
{
	context->VSSetSamplers(0, MAX_STATES, &m_states[STAGE_VS * MAX_STATES]);
	context->PSSetSamplers(0, MAX_STATES, &m_states[STAGE_PS * MAX_STATES]);
	context->HSSetSamplers(0, MAX_STATES, &m_states[STAGE_HS * MAX_STATES]);
	context->DSSetSamplers(0, MAX_STATES, &m_states[STAGE_DS * MAX_STATES]);
	context->GSSetSamplers(0, MAX_STATES, &m_states[STAGE_GS * MAX_STATES]);
	context->CSSetSamplers(0, MAX_STATES, &m_states[STAGE_CS * MAX_STATES]);
	for (int i = 0; i < NV_COUNT_OF(m_states); i++)
	{
		ID3D11SamplerState* state = m_states[i];
		if (state)
		{
			state->Release();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void Dx11RenderState::ShaderState::save(ID3D11DeviceContext* context)
{
	// Input Assembler properties
	context->IAGetPrimitiveTopology(&m_topology);
	context->IAGetInputLayout(&m_inputLayout);
		
	context->IAGetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexBufferStride, &m_vertexBufferOffsets);
	context->IAGetIndexBuffer(&m_indexBuffer, &m_indexBufferFormat, &m_indexBufferOffset);
	// OM
	context->OMGetBlendState(&m_blendState, m_blendFactors, &m_sampleMask);
	context->OMGetDepthStencilState(&m_depthStencilState, &m_stencilRef);
	// RS
	context->RSGetState(&m_rasterizerState);
	// store shader state
	context->VSGetShader(&m_vertexShader, NV_NULL, 0);
	context->PSGetShader(&m_pixelShader, NV_NULL, 0);
	context->GSGetShader(&m_geometryShader, NV_NULL, 0);
	context->HSGetShader(&m_hullShader, NV_NULL, 0);
	context->DSGetShader(&m_domainShader, NV_NULL, 0);
	context->CSGetShader(&m_computeShader, NV_NULL, 0);
}

void Dx11RenderState::ShaderState::restore(ID3D11DeviceContext* context)
{
	// if each state was ref counted, also do release (decrease the ref count) the state resource.
	context->IASetPrimitiveTopology(m_topology);
	if (m_vertexBuffer)
	{
		context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexBufferStride, &m_vertexBufferOffsets);
		m_vertexBuffer.release();
	}
	if (m_indexBuffer)
	{
		context->IASetIndexBuffer(m_indexBuffer, m_indexBufferFormat, m_indexBufferOffset);
		m_indexBuffer.release();
	}
	context->OMSetBlendState(m_blendState, m_blendFactors, m_sampleMask);
	m_blendState.release();
	context->OMSetDepthStencilState(m_depthStencilState, m_stencilRef);
	m_depthStencilState.release();
	context->RSSetState(m_rasterizerState);
	m_rasterizerState.release();
	context->VSSetShader(m_vertexShader, NV_NULL, 0);
	m_vertexShader.release();
	context->PSSetShader(m_pixelShader, NV_NULL, 0);
	m_pixelShader.release();
	context->HSSetShader(m_hullShader, NV_NULL, 0);
	m_hullShader.release();
	context->GSSetShader(m_geometryShader, NV_NULL, 0);
	m_geometryShader.release();
	context->DSSetShader(m_domainShader, NV_NULL, 0);
	m_domainShader.release();
	context->CSSetShader(m_computeShader, NV_NULL, 0);
	m_computeShader.release();
	context->IASetInputLayout(m_inputLayout);
	m_inputLayout.release();
}

} // namespace HairWorks 
} // namespace nvidia 
