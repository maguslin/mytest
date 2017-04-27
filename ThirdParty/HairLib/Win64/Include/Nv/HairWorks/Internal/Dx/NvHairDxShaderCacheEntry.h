/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX_SHADER_CACHE_ENTRY_H
#define NV_HAIR_DX_SHADER_CACHE_ENTRY_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/NvHairShaderCache.h>

namespace nvidia {
namespace HairWorks {

/*! Dx Shader Cache entry. */
class DxShaderCacheEntry: public ShaderCacheEntry
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(DxShaderCacheEntry, ShaderCacheEntry);

	virtual Bool isValid() const NV_OVERRIDE;
	virtual SizeT calcSize() const NV_OVERRIDE;
	virtual Bool hasShaders() const NV_OVERRIDE;
	virtual Result write(NvCo::WriteStream* stream) const NV_OVERRIDE;
	virtual Result read(NvCo::ReadStream* stream) NV_OVERRIDE;
	virtual Result compile(const Settings& cacheSettings) NV_OVERRIDE;
	virtual Result createShaders(const NvCo::ApiDevice& device) NV_OVERRIDE;
	virtual Bool applyShaders(const NvCo::ApiDevice& device, const NvCo::ApiContext& context) NV_OVERRIDE;

	static Result compileShader(const Settings& settings, const char* sourceCode, const char* entryPoint, const char* shaderModel, NvCo::Array<UInt8>& blobOut);

		/// Method for construction
	static ShaderCacheEntry* create();

protected:

	NvCo::Array<UInt8> m_hullBlob;
	NvCo::Array<UInt8> m_domainBlob;
	NvCo::Array<UInt8> m_geometryBlob;
};

class DxShaderCacheFactory: public ShaderCacheFactory
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(DxShaderCacheEntry, ShaderCacheEntry);

	// ShaderCacheFactory
	ShaderCacheEntry* createEntry() NV_OVERRIDE { return new DxShaderCacheEntry; }
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX_SHADER_CACHE_ENTRY_H
