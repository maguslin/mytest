/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX_UTIL_H
#define NV_HAIR_DX_UTIL_H

#include <Nv/Common/NvCoCommon.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

#include <dxgi.h>

namespace nvidia {
namespace HairWorks {

struct DxUtil
{
		/// Given a stride in floats return the format
	static DXGI_FORMAT getFloatFormatFromStride(UINT stride);
		/// The number of float channels held in the format
	static Int calcNumFloatChannels(DXGI_FORMAT format);

		/// Calculate the viewport matrix from the viewport
	static void calcViewportMatrix(const Viewport& viewport, gfsdk_float4x4& out);
		/// Calc view info, using the render targets viewports
	static Void calcViewInfo(const Viewport& viewport , const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewOut);
};


} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX_UTIL_H
