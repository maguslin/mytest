/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11ShaderCacheEntry.h"

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/NvCoMemory.h>

namespace nvidia {
namespace HairWorks {

Result Dx11ShaderCacheEntry::createShaders(const NvCo::ApiDevice& deviceIn)
{
	if (!isValid() || !hasShaders())
	{
		return NV_FAIL;
	}

	ID3D11Device* device = NvCo::Dx11Type::cast<ID3D11Device>(deviceIn);
	if (!device)
	{
		NV_CORE_ASSERT(!"Device is not set");
		return NV_FAIL;
	}

	m_hullShader.setNull();
	m_domainShader.setNull();
	m_geometryShader.setNull();

	NV_RETURN_ON_FAIL(device->CreateHullShader(m_hullBlob.begin(), m_hullBlob.getSize(), NV_NULL, m_hullShader.writeRef()));
	NV_RETURN_ON_FAIL(device->CreateDomainShader(m_domainBlob.begin(), m_domainBlob.getSize(), NV_NULL, m_domainShader.writeRef()));
	NV_RETURN_ON_FAIL(device->CreateGeometryShader(m_geometryBlob.begin(), m_geometryBlob.getSize(), NV_NULL, m_geometryShader.writeRef()));
	return NV_OK;
}

bool Dx11ShaderCacheEntry::applyShaders(const NvCo::ApiDevice& deviceIn, const NvCo::ApiContext& contextIn)
{
	NV_UNUSED(deviceIn);

	if (!hasShaders())
		return false;

	ID3D11DeviceContext* context = NvCo::Dx11Type::cast<ID3D11DeviceContext>(contextIn);
	if (!context)
	{
		NV_CORE_ASSERT(!"Context is not set");
		return false;
	}

	context->DSSetShader(m_domainShader, NV_NULL, 0);
	context->HSSetShader(m_hullShader, NV_NULL, 0);
	context->GSSetShader(m_geometryShader, NV_NULL, 0);
	return true;
}

Bool Dx11ShaderCacheEntry::hasShaders() const 
{ 
	return m_hullShader && m_domainShader && m_geometryShader; 
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! Dx11ShaderCacheFactory !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static const ShaderCacheSettings _calcSettingsMask()
{
	ShaderCacheSettings settings = ShaderCache::getDefaultSettingsMask();
	// We don't need unique settings for these options
	settings.m_useFlags &= ~ShaderCacheSettings::FLAG_SINGLE_TARGET;
	return settings;
}

static const ShaderCacheSettings s_mask = _calcSettingsMask();

ShaderCacheSettings Dx11ShaderCacheFactory::calcUniqueSettings(const ShaderCacheSettings& settingsIn)
{
	ShaderCacheSettings settings(settingsIn);
	settings.applyMask(s_mask);
	return settings;
}

Dx11ShaderCacheEntry* Dx11ShaderCacheFactory::createEntry() 
{ 
	return new Dx11ShaderCacheEntry; 
}


} // namespace HairWorks
} // namespace nvidia