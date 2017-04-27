/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11DepthOpState.h"

namespace nvidia {
namespace HairWorks { 

Dx11DepthOpState::Dx11DepthOpState():
	m_depthOp(DepthOp::UNKNOWN)
{
}

Result Dx11DepthOpState::init(ID3D11Device* device)
{
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.StencilEnable = false;
		desc.StencilReadMask = 0xff;
		desc.StencilWriteMask = 0xff;

		for (Int i = 0; i < DepthOp::COUNT_OF; i++)
		{
			// It is only not enabled on ALWAYS
			desc.DepthEnable = (i != DepthOp::ALWAYS);
			// The write mask is only on with 'WRITE'
			desc.DepthWriteMask = (i == DepthOp::WRITE_GREATER || i == DepthOp::WRITE_LESS) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
			// Depth func
			desc.DepthFunc = (i == DepthOp::WRITE_GREATER || i == DepthOp::TEST_GREATER) ? D3D11_COMPARISON_GREATER : D3D11_COMPARISON_LESS;
			desc.DepthFunc = (i == DepthOp::ALWAYS) ? D3D11_COMPARISON_ALWAYS : desc.DepthFunc;
			NV_RETURN_ON_FAIL(device->CreateDepthStencilState(&desc, m_depthStencilState[i].writeRef()));
		}
	}

	return NV_OK;
}

Void Dx11DepthOpState::setDepthOp(ID3D11DeviceContext* context, EDepthOp depthOp)
{
	if (depthOp != m_depthOp)
	{
		ID3D11DepthStencilState* state = m_depthStencilState[depthOp];
		if (context && state)
		{
			context->OMSetDepthStencilState(state, 0);
		}
		m_depthOp = depthOp;
	}
}

} // namespace HairWorks
} // namespace nvidia
