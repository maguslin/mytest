/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11ShaderPass.h"

namespace nvidia {
namespace HairWorks { 

Result Dx11ShaderPass::createVertexShader( ID3D11Device* device, const Blob& blob)
{
	return device->CreateVertexShader( blob.m_data, blob.m_size, NV_NULL, m_vertexShader.writeRef());
}

Result Dx11ShaderPass::createPixelShader( ID3D11Device* device, const Blob& blob)
{
	return device->CreatePixelShader( blob.m_data, blob.m_size, NV_NULL, m_pixelShader.writeRef());
}

Result Dx11ShaderPass::createHullShader( ID3D11Device* device, const Blob& blob)
{
	return device->CreateHullShader( blob.m_data, blob.m_size, NV_NULL, m_hullShader.writeRef());
}

Result Dx11ShaderPass::createDomainShader( ID3D11Device* device, const Blob& blob)
{
	return device->CreateDomainShader( blob.m_data, blob.m_size, NV_NULL, m_domainShader.writeRef());
}

Result Dx11ShaderPass::createGeometryShader( ID3D11Device* device, const Blob& blob)
{
	return device->CreateGeometryShader( blob.m_data, blob.m_size, NV_NULL, m_geometryShader.writeRef());
}

Result Dx11ShaderPass::createGeometryShaderWithStreamOut(ID3D11Device* device, const Blob& blob, D3D11_SO_DECLARATION_ENTRY* so_decl, UINT ndecl)
{
	return device->CreateGeometryShaderWithStreamOutput(blob.m_data, blob.m_size, so_decl, ndecl, NV_NULL, 0, D3D11_SO_NO_RASTERIZED_STREAM, NV_NULL, m_geometryShader.writeRef());
}

Result Dx11ShaderPass::createInputLayout( ID3D11Device* device, const Blob& blob, const D3D11_INPUT_ELEMENT_DESC* elementDescs, UINT numElements)
{
	return device->CreateInputLayout( elementDescs, numElements, blob.m_data, blob.m_size, m_inputLayout.writeRef());
}

void Dx11ShaderPass::enable( ID3D11DeviceContext* context, Bool useUserPixelShader) const
{
	// set shaders
	context->VSSetShader(m_vertexShader, NV_NULL, 0);
	context->DSSetShader(m_domainShader, NV_NULL, 0);
	context->HSSetShader(m_hullShader, NV_NULL, 0);
	context->GSSetShader(m_geometryShader, NV_NULL, 0);

	context->IASetInputLayout( m_inputLayout);

	if (!useUserPixelShader)
	{
		context->PSSetShader(m_pixelShader, NV_NULL, 0);
	}
}

void Dx11ShaderPass::disable( ID3D11DeviceContext* context, Bool useUserPixelShader) const
{
	// disable all the render state that this shader might have touched
	context->VSSetShader(NV_NULL, NV_NULL, 0);
	context->DSSetShader(NV_NULL, NV_NULL, 0);
	context->HSSetShader(NV_NULL, NV_NULL, 0);
	context->GSSetShader(NV_NULL, NV_NULL, 0);

	if (!useUserPixelShader)
	{
		context->PSSetShader(NV_NULL, NV_NULL, 0);
	}
}

void Dx11ShaderPass::clear()
{
	m_vertexShader.setNull();
	m_pixelShader.setNull();
	m_geometryShader.setNull();
	m_hullShader.setNull();
	m_domainShader.setNull();
	m_computeShader.setNull();
	m_inputLayout.setNull();
}

} // namespace HairWorks
} // namespace nvidia
