/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_API_ASSET_H
#define NV_HAIR_API_ASSET_H

#include "NvHairApiGlobal.h"

namespace nvidia {
namespace HairWorks { 

class ApiAsset
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ApiAsset);

	virtual ~ApiAsset() {}
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_API_ASSET_H
