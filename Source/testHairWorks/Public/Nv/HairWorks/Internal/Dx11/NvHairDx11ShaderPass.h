/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_SHADER_PASS_H
#define NV_HAIR_DX11_SHADER_PASS_H

#include <dxgi.h>
#include <d3d11.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoComPtr.h>

namespace nvidia {
namespace HairWorks {

class Dx11ShaderPass
{
	NV_CO_DECLARE_CLASS_BASE(Dx11ShaderPass);

		/// Way to pass statically defined compiled blobs conveniently. 
	struct Blob
	{
		template <SizeT SIZE>
		NV_FORCE_INLINE Blob(const BYTE (&in)[SIZE]):m_data(in),m_size(SIZE) {}
		NV_FORCE_INLINE Blob(const BYTE* in, SizeT size):m_data(in), m_size(size) {}
		const BYTE* m_data;
		SizeT m_size;
	};

	Result createVertexShader(ID3D11Device* device, const Blob& blob);
	Result createPixelShader(ID3D11Device* device, const Blob& blob);
	Result createDomainShader(ID3D11Device* device, const Blob& blob);
	Result createHullShader(ID3D11Device* device, const Blob& blob);
	Result createGeometryShader(ID3D11Device* device, const Blob& blob);
	Result createGeometryShaderWithStreamOut(ID3D11Device* device, const Blob& blob, D3D11_SO_DECLARATION_ENTRY* decl, UINT ndecl);

	Result createInputLayout(ID3D11Device* device, const Blob& blob, const D3D11_INPUT_ELEMENT_DESC* elementDescs = NV_NULL, UINT numElements = 0);

	void enable(ID3D11DeviceContext* context, Bool useUserPixelShader = false) const;
	void disable(ID3D11DeviceContext* context, Bool useUserPixelShdaer = false) const;

		/// Sets all the shaders to NV_NULL
	void clear();

	NvCo::ComPtr<ID3D11VertexShader> m_vertexShader;
	NvCo::ComPtr<ID3D11PixelShader> m_pixelShader;
	NvCo::ComPtr<ID3D11GeometryShader> m_geometryShader;

	NvCo::ComPtr<ID3D11HullShader> m_hullShader;
	NvCo::ComPtr<ID3D11DomainShader> m_domainShader;

	NvCo::ComPtr<ID3D11ComputeShader> m_computeShader;

	NvCo::ComPtr<ID3D11InputLayout> m_inputLayout;
};

class Dx11ScopeShaderPass
{
	NV_CO_DECLARE_CLASS_BASE(Dx11ScopeShaderPass);

	Dx11ScopeShaderPass(const Dx11ShaderPass& pass, ID3D11DeviceContext* context, Bool userPixelShader = false) :
		m_context(context),
		m_userPixelShader(userPixelShader),
		m_pass(pass)
	{
		m_pass.enable(m_context, m_userPixelShader);
	}
	~Dx11ScopeShaderPass()
	{
		m_pass.disable(m_context, m_userPixelShader);
	}

private:
	// Disable
	ThisType& operator=(const ThisType& rhs); // { return *this; }
	Dx11ScopeShaderPass(const ThisType& rhs);

	// Members
	ID3D11DeviceContext* m_context;
	const Dx11ShaderPass& m_pass;
	Bool m_userPixelShader;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_SHADER_PASS_H
