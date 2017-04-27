/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDxShaderCacheEntry.h"
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

#include "D3Dcompiler.h"
#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/NvCoMemory.h>

namespace nvidia {
namespace HairWorks {

namespace InterpolateDomainOptShader
{
	#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateDomainOpt.cpp>
}

namespace InterpolateGeometryOptShader
{
	#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateGeometryOpt.cpp>
}

namespace InterpolateHullOptShader
{
	#include <Nv/HairWorks/Internal/Shader/Generated/NvHairInterpolateHullOpt.cpp>
}

/* static */Result DxShaderCacheEntry::compileShader(const Settings& settings, const char* sourceCode, const char* entryPoint, const char* shaderModel, NvCo::Array<UInt8>& blobOut)
{
	blobOut.clear();

	NvCo::ComPtr<ID3D10Blob> errorBlob, blob;
	NvCo::Array<Char> shaderSource;

	ShaderCache::createShaderSource(settings, sourceCode, shaderSource);

	Result res = D3DCompile(shaderSource.begin(), shaderSource.getSize(), NV_NULL, 0, 0, entryPoint, shaderModel, D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, blob.writeRef(), errorBlob.writeRef());
	if (blob)
	{
		SizeT bufferSize = blob->GetBufferSize();
		blobOut.setSize(IndexT(bufferSize));
		NvCo::Memory::copy(blobOut.begin(), blob->GetBufferPointer(), bufferSize);
	}
	return res;
}

Bool DxShaderCacheEntry::isValid() const
{
	return m_hullBlob.getSize() > 0 && m_domainBlob.getSize() > 0 && m_geometryBlob.getSize() > 0;
}

SizeT DxShaderCacheEntry::calcSize() const
{
	if (!isValid())
		return 0;
	SizeT size = 0;
	size += sizeof(Header); // header size
	size += sizeof(Settings);
	size += m_hullBlob.getSize();
	size += m_domainBlob.getSize();
	size += m_geometryBlob.getSize();
	return size;
}

Result DxShaderCacheEntry::write(NvCo::WriteStream* stream) const
{
	if (!isValid())
		return NV_FAIL;

	Header header;
	header.m_hullShaderLen = UInt32(m_hullBlob.getSize());
	header.m_domainShaderLen = UInt32(m_domainBlob.getSize());
	header.m_geometryShaderLen	= UInt32(m_geometryBlob.getSize());
	header.m_totalLen = UInt32(calcSize());

	if (stream->write(&header, sizeof(Header)) != sizeof(Header))
	{
		NV_CO_LOG_ERROR("Unable to write shader cache entry header");
		return NV_FAIL;
	} 

	Bool isOk = (stream->write(m_hullBlob.begin(), m_hullBlob.getSize()) == m_hullBlob.getSize());
	isOk = isOk && (stream->write(m_domainBlob.begin(), m_domainBlob.getSize()) == m_domainBlob.getSize());
	isOk = isOk && (stream->write(m_geometryBlob.begin(), m_geometryBlob.getSize()) != m_geometryBlob.getSize());
	isOk = isOk && (stream->write(&m_settings, sizeof(m_settings)) != sizeof(m_settings));
	if (!isOk)
	{
		NV_CO_LOG_ERROR("Unable to write shader cache entry contents");
		return NV_FAIL;
	}

	return NV_OK;
}

Result DxShaderCacheEntry::read(NvCo::ReadStream* stream)
{
	Header header;
	if (stream->read(&header, sizeof(Header)) != sizeof(Header))
	{
		NV_CO_LOG_ERROR("Unable to read shader cache entry header");
		return NV_FAIL;
	}

	m_hullBlob.setSize(header.m_hullShaderLen);
	m_domainBlob.setSize(header.m_domainShaderLen);
	m_geometryBlob.setSize(header.m_geometryShaderLen);

	Bool isOk = (stream->read(m_hullBlob.begin(), m_hullBlob.getSize()) == m_hullBlob.getSize());
	isOk = isOk && (stream->read(m_domainBlob.begin(), m_domainBlob.getSize()) == m_domainBlob.getSize());
	isOk = isOk && (stream->read(m_geometryBlob.begin(), m_geometryBlob.getSize()) == m_geometryBlob.getSize());
	isOk = isOk && (stream->read(&m_settings, sizeof(m_settings)) == sizeof(m_settings));
	if (!isOk)
	{
		NV_CO_LOG_ERROR("Unable to read shader cache entry contents");
		return NV_FAIL;
	}

	return NV_OK;
}

Result DxShaderCacheEntry::compile(const Settings& settings)
{
	m_settings = settings;
	// modify hull shader	
	NV_RETURN_ON_FAIL(compileShader(settings, InterpolateHullOptShader::shaderVar, "hs_main", "hs_5_0", m_hullBlob));
	// modify domain shader
	NV_RETURN_ON_FAIL(compileShader(settings, InterpolateDomainOptShader::shaderVar, "ds_main", "ds_5_0", m_domainBlob));
	// modify geometry shader
	NV_RETURN_ON_FAIL(compileShader(settings, InterpolateGeometryOptShader::shaderVar, "gs_main", "gs_5_0", m_geometryBlob));
	return NV_OK;		
}

Result DxShaderCacheEntry::createShaders(const NvCo::ApiDevice& deviceIn)
{
	NV_UNUSED(deviceIn)
	NV_CORE_ALWAYS_ASSERT("Not implemented on this platform");
	return NV_FAIL;
}

bool DxShaderCacheEntry::applyShaders(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn)
{
	NV_UNUSED(deviceIn)
	NV_UNUSED(contextIn)
	return false;
}

Bool DxShaderCacheEntry::hasShaders() const
{ 
	return false;
}

/* static */ShaderCacheEntry* DxShaderCacheEntry::create()
{
	return new ThisType;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DxShaderCacheFactory !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


} // namespace HairWorks
} // namespace nvidia