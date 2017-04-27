/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_DEPTH_OP_STATE_H
#define NV_HAIR_DX11_DEPTH_OP_STATE_H

#include <dxgi.h>
#include <d3d11.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoComPtr.h>

namespace nvidia {
namespace HairWorks {

class Dx11DepthOpState
{
	NV_CO_DECLARE_CLASS_BASE(Dx11DepthOpState);

	Result init(ID3D11Device* device);

		/// Get the depth stencil state for an op (or NV_NULL if invalid)
	ID3D11DepthStencilState* getDepthStencilState(EDepthOp op) { return m_depthStencilState[op]; }
	
		/// Can only be set after start. If the depth op is the same as is already set, it's a no-op
	Void setDepthOp(ID3D11DeviceContext* context, EDepthOp depthOp);
		/// Get the currently set depth op
	NV_FORCE_INLINE EDepthOp getDepthOp() const { return m_depthOp; }
	
		/// Ctor
	Dx11DepthOpState();

	protected:
	// Current state
	EDepthOp m_depthOp;

	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilState[DepthOp::COUNT_OF];
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_DEPTH_OP_STATE_H
