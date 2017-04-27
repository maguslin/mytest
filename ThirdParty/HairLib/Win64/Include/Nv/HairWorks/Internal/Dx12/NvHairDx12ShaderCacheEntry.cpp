/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12ShaderCacheEntry.h"

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/NvCoMemory.h>

namespace nvidia {
namespace HairWorks {

Result Dx12ShaderCacheEntry::createShaders(const NvCo::ApiDevice& deviceIn)
{
	NV_UNUSED(deviceIn)
	if (!isValid() || !hasShaders())
	{
		return NV_FAIL;
	}
	return NV_OK;
}

} // namespace HairWorks
} // namespace nvidia