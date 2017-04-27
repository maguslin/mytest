/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairDx11Util.h"

#include <Nv/HairWorks/Internal/Dx/NvHairDxUtil.h>

#include <Nv/Common/NvCoComPtr.h>

namespace nvidia { 
namespace HairWorks {

/* static */D3D11_BUFFER_DESC Dx11Util::createBufferDesc(D3D11_USAGE usage, UINT byteWidth, UINT structureByteStride, UINT bindFlags, UINT miscFlags, UINT cpuAccessFlags)
{
	D3D11_BUFFER_DESC desc;
	desc.Usage					= usage;
	desc.ByteWidth				= byteWidth;
	desc.StructureByteStride	= structureByteStride;
	desc.BindFlags				= bindFlags;
	desc.MiscFlags				= miscFlags;
	desc.CPUAccessFlags			= cpuAccessFlags;
	return desc;
}

/* static */D3D11_UNORDERED_ACCESS_VIEW_DESC Dx11Util::createUavDesc(DXGI_FORMAT format, D3D11_UAV_DIMENSION viewDimension, UINT numElements, UINT firstElement, UINT flags)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format                    = format;
	desc.ViewDimension             = viewDimension;
	desc.Buffer.NumElements        = numElements;
	desc.Buffer.FirstElement       = firstElement;
	desc.Buffer.Flags              = flags;
	return desc;
}
/* static */D3D11_SHADER_RESOURCE_VIEW_DESC Dx11Util::createSrvDesc(DXGI_FORMAT format, D3D11_SRV_DIMENSION viewDimension, UINT numElements, UINT firstElement)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format                    = format;
	desc.ViewDimension             = viewDimension;
	desc.Buffer.FirstElement	   = firstElement; // MostDetailedMip
	desc.Buffer.NumElements        = numElements; // MipLevels
	return desc;
}

/* static */D3D11_DEPTH_STENCIL_VIEW_DESC Dx11Util::createDsvDesc(DXGI_FORMAT format, D3D11_DSV_DIMENSION viewDimension, UINT flags, UINT mipSlice)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.Format                    = format;
	desc.ViewDimension             = viewDimension;
	desc.Flags					   = flags;
	desc.Texture2D.MipSlice        = mipSlice;
	return desc;
}

/* static */D3D11_RENDER_TARGET_VIEW_DESC Dx11Util::createRtvDesc(DXGI_FORMAT format, D3D11_RTV_DIMENSION viewDimension, UINT mipSlice)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format                    = format;
	desc.ViewDimension             = viewDimension;
	desc.Texture2D.MipSlice        = mipSlice;
	return desc;
}

/* static */D3D11_TEXTURE2D_DESC Dx11Util::createTextureDesc(DXGI_FORMAT format, UINT width, UINT height, UINT bindFlags, 
	UINT sampleCount, D3D11_USAGE usage, UINT cpuAccessFlags, UINT miscFlags, UINT arraySize, UINT mipLevels)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Format                 = format;
	desc.Width					= width;
	desc.Height					= height;
	desc.ArraySize				= arraySize;
	desc.MiscFlags				= miscFlags;
	desc.MipLevels				= mipLevels;
	desc.SampleDesc.Count		= sampleCount;
	desc.SampleDesc.Quality		= 0;
	desc.BindFlags				= bindFlags;
	desc.Usage					= usage;
	desc.CPUAccessFlags			= cpuAccessFlags;
	return desc;
}

Result Dx11Util::createBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* sysMem, ID3D11Buffer** bufferOut)
{
	// assert to prevent failure when coding
	// If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16, 
	// and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
	NV_CORE_ASSERT(desc.BindFlags != D3D11_BIND_CONSTANT_BUFFER || (desc.ByteWidth & 0xf) == 0);

	NV_CORE_ASSERT(bufferOut);
	if (sysMem)
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = sysMem;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		NV_RETURN_ON_FAIL(device->CreateBuffer(&desc, &data, bufferOut));
	}
	else
	{
		NV_RETURN_ON_FAIL(device->CreateBuffer(&desc, NV_NULL, bufferOut));
	}
	return NV_OK;
}

Result Dx11Util::createIndexBuffer(ID3D11Device* device, UINT indexCounts, const void* sysMem, ID3D11Buffer** bufferOut)
{
	NV_CORE_ASSERT(bufferOut);
	D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_DEFAULT, sizeof(UINT) * indexCounts, 0, D3D11_BIND_INDEX_BUFFER, 0, 0);
	return createBuffer(device, bd, sysMem, bufferOut);
}


Result Dx11Util::createStreamOutBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, 
	ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut)
{
	NV_CORE_ASSERT(bufferOut);
	UINT bufferSize	= numElements * stride;
	D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_DEFAULT, bufferSize, 0, D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_SHADER_RESOURCE, 0, 0);

	NvCo::ComPtr<ID3D11Buffer> buf;
	NV_RETURN_ON_FAIL(createBuffer(device, bd, sysMem, buf.writeRef()));

	DXGI_FORMAT format = DxUtil::getFloatFormatFromStride(stride);
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = createSrvDesc(format, D3D11_SRV_DIMENSION_BUFFER, numElements);
	NV_RETURN_ON_FAIL(createShaderResourceView(device, buf, &desc, srvOut));
	*bufferOut = buf.detach();
	return NV_OK;
}

Result Dx11Util::createReadOnlyBuffer(ID3D11Device* device, DXGI_FORMAT format, UINT stride, UINT numElements, const void* sysMem, 
	ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut)
{
	UINT bufferSize	= numElements * stride;
	NvCo::ComPtr<ID3D11Buffer> buf;
	// create buffer 
	if (bufferOut)
	{
		D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_IMMUTABLE, bufferSize, 0, D3D11_BIND_SHADER_RESOURCE, 0, 0);
		NV_RETURN_ON_FAIL( createBuffer(device, bd, sysMem, buf.writeRef()));
	}
	// create SRV for updated frames
	if (srvOut)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc = createSrvDesc(format, D3D11_SRV_DIMENSION_BUFFER, numElements);
		NV_RETURN_ON_FAIL( createShaderResourceView(device, buf, &desc, srvOut));
	}
	if (bufferOut)
	{
		*bufferOut = buf.detach();
	}
	return NV_OK;
}

Result Dx11Util::createVertexBuffer(ID3D11Device*	device, UINT stride, UINT numElements, const void* sysMem, 
	ID3D11Buffer** bufferOut, ID3D11UnorderedAccessView** uavOut)
{
	UINT bufferSize	= numElements * stride;

	NvCo::ComPtr<ID3D11Buffer> buf;
	// create buffer 
	if (bufferOut)
	{
		const UINT bindFlags = uavOut ? (D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_VERTEX_BUFFER) : D3D11_BIND_VERTEX_BUFFER;
		D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_DEFAULT, bufferSize, stride, bindFlags, 0, 0);
		NV_RETURN_ON_FAIL( createBuffer(device, bd, sysMem, buf.writeRef()));
	}
	// create UAV for updated frames
	if (uavOut)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = createUavDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_UAV_DIMENSION_BUFFER, numElements);
		NV_RETURN_ON_FAIL( createUnorderedAccessView(device, buf, &desc, uavOut) );
	}
	if (bufferOut)
	{
		*bufferOut = buf.detach();
	}
	return NV_OK;
}

Result Dx11Util::createStagingBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, UINT cpuAccessFlags, ID3D11Buffer** bufferOut)
{
	NV_CORE_ASSERT(bufferOut);
	UINT bufferSize	= numElements * stride;
	D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_STAGING, bufferSize, stride, 0, 0, cpuAccessFlags);
	return createBuffer(device, bd, sysMem, bufferOut);
}

Result Dx11Util::createConstantBuffer(ID3D11Device* device, UINT bufferSize, ID3D11Buffer** bufferOut)
{
	D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_DYNAMIC, bufferSize, 0, D3D11_BIND_CONSTANT_BUFFER, 0, D3D11_CPU_ACCESS_WRITE);
	return createBuffer( device, bd, NV_NULL, bufferOut);
}

Result Dx11Util::createStructuredBuffer(ID3D11Device* device, UINT stride, UINT numElements, const void* sysMem, 
	ID3D11Buffer** bufferOut, ID3D11ShaderResourceView** srvOut, ID3D11UnorderedAccessView** uavOut)
{
	NvCo::ComPtr<ID3D11Buffer> buf;
	NvCo::ComPtr<ID3D11ShaderResourceView> srv;
	// create buffer 
	if (bufferOut)
	{
		const D3D11_BIND_FLAG bindFlags = D3D11_BIND_FLAG(D3D11_BIND_UNORDERED_ACCESS | (srvOut ? D3D11_BIND_SHADER_RESOURCE : 0));
		const UINT bufferSize = numElements * stride;
		const D3D11_BUFFER_DESC bd = createBufferDesc(D3D11_USAGE_DEFAULT, bufferSize, stride, bindFlags, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, 0);
		NV_RETURN_ON_FAIL( createBuffer(device, bd, sysMem, buf.writeRef()));
	}
	// create SRV for updated frames
	if (srvOut)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc = createSrvDesc(DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFER, numElements);
		NV_RETURN_ON_FAIL( createShaderResourceView( device, buf, &desc, srv.writeRef()));
	}
	// create UAV for updated frames
	if (uavOut)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = createUavDesc(DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, numElements);
		NV_RETURN_ON_FAIL( createUnorderedAccessView(device, buf, &desc, uavOut) );
	}
	if (bufferOut)
	{
		*bufferOut = buf.detach();
	}
	if (srvOut)
	{
		*srvOut = srv.detach();
	}
	return NV_OK;
}

/* static */ UINT Dx11Util::calcTextureSize(const D3D11_TEXTURE2D_DESC& desc)
{
	Int numChans = DxUtil::calcNumFloatChannels(desc.Format);
	NV_CORE_ASSERT(numChans > 0);
	return numChans * sizeof(float) * desc.Width * desc.Height * desc.SampleDesc.Count;
}

Result Dx11Util::createTexture2D(ID3D11Device* device, D3D11_TEXTURE2D_DESC& desc, ID3D11Texture2D** textureOut)
{
	return device->CreateTexture2D( &desc, NV_NULL, textureOut);
}

Result Dx11Util::createShaderResourceView(ID3D11Device* device, ID3D11Resource* resource, D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** srvOut)
{
	return device->CreateShaderResourceView( resource, desc, srvOut);
}

Result Dx11Util::createRenderTargetView(ID3D11Device* device, ID3D11Resource* resource, D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** rtvOut)
{
	return device->CreateRenderTargetView(resource, desc, rtvOut);
}

Result Dx11Util::createDepthStencilView(ID3D11Device* device, ID3D11Resource* resource, D3D11_DEPTH_STENCIL_VIEW_DESC* desc, ID3D11DepthStencilView** dsvOut)
{
	return device->CreateDepthStencilView(resource, desc, dsvOut);
}

Result Dx11Util::createUnorderedAccessView(ID3D11Device* device, ID3D11Resource* resource, D3D11_UNORDERED_ACCESS_VIEW_DESC* desc, ID3D11UnorderedAccessView** uavOut)
{
	return device->CreateUnorderedAccessView(resource, desc, uavOut);
}

} // namespace HairWorks
} // namespace nvidia

