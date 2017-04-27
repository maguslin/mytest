/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_RENDER_STATE_H
#define NV_HAIR_DX11_RENDER_STATE_H

#include <dxgi.h>
#include <d3d11.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoWeakComPtr.h>

namespace nvidia {
namespace HairWorks { 

/*! Structure for holding rendering state. 
NOTE! It is not totally symmetric - because it will only restore a vertex/index buffer if one was previously 
set. If one wasn't set, it leaves what was ever previously bound on the InputAssembler(IA) */
class Dx11RenderState
{
	NV_CO_DECLARE_CLASS_BASE(Dx11RenderState);

	enum StageType
	{
		STAGE_VS,
		STAGE_PS,
		STAGE_HS,
		STAGE_DS,
		STAGE_GS,
		STAGE_CS,
		STAGE_COUNT_OF,
	};

	struct ConstantBufferState
	{
		ConstantBufferState();
		void save(ID3D11DeviceContext* context);
		void restore(ID3D11DeviceContext* context);
	protected:
		ID3D11Buffer* m_buffers[STAGE_COUNT_OF];
	};
	struct ShaderResourceViewState
	{
		ShaderResourceViewState();
		void save(ID3D11DeviceContext* context);
		void restore(ID3D11DeviceContext* context);
	protected:
		enum { MAX_STATES = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT };
		ID3D11ShaderResourceView* m_srvs[STAGE_COUNT_OF * MAX_STATES];
	};
	struct SamplerState
	{
		SamplerState();
		void save(ID3D11DeviceContext* context);
		void restore(ID3D11DeviceContext* context);
	protected:
		enum { MAX_STATES = 16 };
		ID3D11SamplerState* m_states[STAGE_COUNT_OF * MAX_STATES];
	};
	struct ShaderState
	{
		void save(ID3D11DeviceContext* context);
		void restore(ID3D11DeviceContext* context);
	protected:
		// render state management
		D3D11_PRIMITIVE_TOPOLOGY m_topology;
		UINT m_sampleMask;
		float m_blendFactors[4];
		UINT m_stencilRef;

		// shader management
		NvCo::WeakComPtr<ID3D11VertexShader>	m_vertexShader;
		NvCo::WeakComPtr<ID3D11PixelShader> m_pixelShader;
		NvCo::WeakComPtr<ID3D11GeometryShader> m_geometryShader;
		NvCo::WeakComPtr<ID3D11HullShader> m_hullShader;
		NvCo::WeakComPtr<ID3D11DomainShader>	m_domainShader;
		NvCo::WeakComPtr<ID3D11ComputeShader> m_computeShader;

		// other state (IA, OM, RS, ..)
		NvCo::WeakComPtr<ID3D11InputLayout> m_inputLayout;

		NvCo::WeakComPtr<ID3D11Buffer> m_vertexBuffer;
		UINT m_vertexBufferStride;
		UINT m_vertexBufferOffsets;

		NvCo::WeakComPtr<ID3D11Buffer> m_indexBuffer;
		DXGI_FORMAT	m_indexBufferFormat;
		UINT m_indexBufferOffset;

		NvCo::WeakComPtr<ID3D11BlendState> m_blendState;
		NvCo::WeakComPtr<ID3D11DepthStencilState> m_depthStencilState;
		NvCo::WeakComPtr<ID3D11RasterizerState> m_rasterizerState;
	};

	void save(ID3D11DeviceContext* context)
	{
		m_constantBufferState.save(context);
		m_samplerState.save(context);
		m_shaderResourceViewState.save(context);
		m_shaderState.save(context);
	}
	void restore(ID3D11DeviceContext* context)
	{
		m_constantBufferState.restore(context);
		m_samplerState.restore(context);
		m_shaderResourceViewState.restore(context);
		m_shaderState.restore(context);
	}

	ConstantBufferState	m_constantBufferState;
	ShaderResourceViewState	m_shaderResourceViewState;
	SamplerState m_samplerState;
	ShaderState	m_shaderState;
};

#if 0
class Dx11ScopeRenderState
{
public:
	Dx11ScopeRenderState::Dx11ScopeRenderState(Dx11RenderState& state, ID3D11DeviceContext* context) :
		m_context(context),
		m_state(state)
	{
		state.save(context);
	}
	~Dx11ScopeRenderState()
	{
		m_state.restore(m_context);
	}
	Dx11RenderState& m_state;
	ID3D11DeviceContext* m_context;
};
#endif

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_RENDER_STATE_H
