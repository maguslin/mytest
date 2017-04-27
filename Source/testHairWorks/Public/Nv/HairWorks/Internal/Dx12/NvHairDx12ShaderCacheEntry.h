/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_SHADER_CACHE_ENTRY_H
#define NV_HAIR_DX12_SHADER_CACHE_ENTRY_H

#include <Nv/HairWorks/Internal/Dx/NvHairDxShaderCacheEntry.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>
#include "NvHairDx12ShaderPass.h"

namespace nvidia {
namespace HairWorks {

/*! Dx12 Shader Cache entry. */
class Dx12ShaderCacheEntry: public DxShaderCacheEntry
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12ShaderCacheEntry, DxShaderCacheEntry);

	// ShaderCacheEntry interface
	Result createShaders(const NvCo::ApiDevice& device) NV_OVERRIDE;

	NvCo::Array<Dx12ShaderPass> m_passes;				///< Has one for each shader index. 
protected:
};

class Dx12ShaderCacheFactory: public DxShaderCacheFactory
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12ShaderCacheFactory, DxShaderCacheFactory);

	// ShaderCacheFactory
	Dx12ShaderCacheEntry* createEntry() NV_OVERRIDE { return new Dx12ShaderCacheEntry; }
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_SHADER_CACHE_ENTRY_H
