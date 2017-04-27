/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_SHADER_CACHE_ENTRY_H
#define NV_HAIR_DX11_SHADER_CACHE_ENTRY_H

#include <Nv/HairWorks/Internal/Dx/NvHairDxShaderCacheEntry.h>

#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

namespace nvidia {
namespace HairWorks {

/*! Dx11 Shader Cache entry. */
class Dx11ShaderCacheEntry: public DxShaderCacheEntry
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11ShaderCacheEntry, DxShaderCacheEntry);

	// ShaderCacheEntry interface
	Bool hasShaders() const NV_OVERRIDE;
	Result createShaders(const NvCo::ApiDevice& device) NV_OVERRIDE;
	Bool applyShaders(const NvCo::ApiDevice& device, const NvCo::ApiContext& context) NV_OVERRIDE;

protected:
	NvCo::ComPtr<ID3D11HullShader> m_hullShader;
	NvCo::ComPtr<ID3D11DomainShader> m_domainShader;
	NvCo::ComPtr<ID3D11GeometryShader> m_geometryShader;
};

class Dx11ShaderCacheFactory: public DxShaderCacheFactory
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11ShaderCacheFactory, DxShaderCacheFactory);

	// ShaderCacheFactory
	ShaderCacheSettings calcUniqueSettings(const ShaderCacheSettings& settings) NV_OVERRIDE;
	Dx11ShaderCacheEntry* createEntry() NV_OVERRIDE;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX12_SHADER_CACHE_ENTRY_H
