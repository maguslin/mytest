/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_UTIL_H
#define NV_HAIR_DX11_UTIL_H

#include <Nv/Common/NvCoCommon.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

#include <dxgi.h>
#include <d3d11.h>

namespace nvidia {
namespace HairWorks {

struct Dx11Util
{
		/// Calculate the texture size in bytes
	static UINT calcTextureSize(const D3D11_TEXTURE2D_DESC& desc);

		/// Helpers
	static D3D11_BUFFER_DESC createBufferDesc(D3D11_USAGE usage, UINT byteWidth, UINT structureByteStride, UINT bindFlags, UINT miscFlags, UINT cpuAccessFlags);
	static D3D11_UNORDERED_ACCESS_VIEW_DESC createUavDesc(DXGI_FORMAT format, D3D11_UAV_DIMENSION viewDimension, UINT numElements, UINT firstElement = 0, UINT flags = 0);
	static D3D11_SHADER_RESOURCE_VIEW_DESC createSrvDesc(DXGI_FORMAT format, D3D11_SRV_DIMENSION viewDimension, UINT numElements, UINT firstElement = 0);
	static D3D11_DEPTH_STENCIL_VIEW_DESC createDsvDesc(DXGI_FORMAT format, D3D11_DSV_DIMENSION viewDimension, UINT flags = 0, UINT mipSlice = 0);
	static D3D11_RENDER_TARGET_VIEW_DESC createRtvDesc(DXGI_FORMAT format, D3D11_RTV_DIMENSION viewDimension, UINT mipSlice = 0);
	static D3D11_TEXTURE2D_DESC createTextureDesc(DXGI_FORMAT format, UINT width, UINT height, UINT bindFlags, UINT sampleCount = 1, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, 
													UINT cpuAccessFlags = 0, UINT miscFlags = 0, UINT arraySize = 1, UINT mipLevels = 1);

		/// Buffer creation
	static Result createBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* sysMem, ID3D11Buffer** bufferOut);
	static Result createStreamOutBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut);
	static Result createReadOnlyBuffer(ID3D11Device* device, DXGI_FORMAT format, UINT stride, UINT numElements, const void* sysMem, ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut);
	static Result createVertexBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, ID3D11Buffer** bufferOut, ID3D11UnorderedAccessView** uavOut);
	static Result createIndexBuffer(ID3D11Device* device, UINT indexCounts, const void* sysMem, ID3D11Buffer** bufferOut);
	static Result createStagingBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, UINT cpuAccessFlags, ID3D11Buffer** bufferOut);
	static Result createConstantBuffer(ID3D11Device* device, UINT bufferSize, ID3D11Buffer** bufferOut);
	static Result createStructuredBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut, ID3D11UnorderedAccessView** uavOut);
		/// Texture creation
	static Result createTexture2D(ID3D11Device* device, D3D11_TEXTURE2D_DESC& desc, ID3D11Texture2D** textureOut);
		/// View creation
	static Result createShaderResourceView(ID3D11Device* device, ID3D11Resource* resource, D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** srvOut);
	static Result createRenderTargetView(ID3D11Device* device, ID3D11Resource* resource, D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** rtvOut);
	static Result createDepthStencilView(ID3D11Device* device, ID3D11Resource* resource, D3D11_DEPTH_STENCIL_VIEW_DESC* desc, ID3D11DepthStencilView** dsvOut);
	static Result createUnorderedAccessView(ID3D11Device* device, ID3D11Resource* resource, D3D11_UNORDERED_ACCESS_VIEW_DESC* desc, ID3D11UnorderedAccessView** uavOut);
};


} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_UTIL_H
