/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairDxUtil.h"

#include <Nv/Common/NvCoComPtr.h>

namespace nvidia { 
namespace HairWorks {

/* static */DXGI_FORMAT DxUtil::getFloatFormatFromStride(UINT stride)
{
	switch (stride)
	{
		case sizeof(float) :		return DXGI_FORMAT_R32_FLOAT;
		case 2 * sizeof(float) :	return DXGI_FORMAT_R32G32_FLOAT;
		case 3 * sizeof(float) :	return DXGI_FORMAT_R32G32B32_FLOAT;
		case 4 * sizeof(float) :	return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:					return DXGI_FORMAT_UNKNOWN;
	}
}

/* static */Int DxUtil::calcNumFloatChannels(DXGI_FORMAT format)
{
	if ((DXGI_FORMAT_UNKNOWN <= format) && (DXGI_FORMAT_R32G32B32A32_SINT >= format))
		return 4;
	else if ((DXGI_FORMAT_R32G32B32_TYPELESS <= format) && (DXGI_FORMAT_R32G32B32_SINT >= format))
		return 3;
	else if ((DXGI_FORMAT_R32G32B32_TYPELESS <= format) && (DXGI_FORMAT_R32G32B32_SINT >= format))
		return 3;
	else if ((DXGI_FORMAT_R16G16B16A16_TYPELESS <= format) && (DXGI_FORMAT_R16G16B16A16_SINT >= format))
		return 2;
	else if ((DXGI_FORMAT_R32G32_TYPELESS <= format) && (DXGI_FORMAT_R32G32_SINT >= format))
		return 2;
	else if ((DXGI_FORMAT_R16G16_TYPELESS <= format) && (DXGI_FORMAT_X24_TYPELESS_G8_UINT >= format))
		return 1;
	return 0;
}

/* static */void DxUtil::calcViewportMatrix(const Viewport& viewport, gfsdk_float4x4& out)
{
	float halfw = 0.5f * viewport.m_width;
	float halfh = 0.5f * viewport.m_height;
	float l = viewport.m_topLeftX;
	float t = viewport.m_topLeftY;

	// float sx = l + w * 0.5 * (1.0 + ndcx);
	// float sy = t + h * 0.5 * (1.0 - ndcy);
	// sx = w * 0.5 * ndcx + l + w * 0.5;
	// sy = -h * 0.5 * ndcy + t + h * 0.5;

	out._11 = halfw;		out._12 = 0;			out._13 = 0;  out._14 = 0;
	out._21 = 0;			out._22 = -halfh;		out._23 = 0;  out._24 = 0;
	out._31 = 0;			out._32 = 0;			out._33 = 1;  out._34 = 0;
	out._41 = l + halfw;	out._42 = t + halfh;	out._43 = 0;  out._44 = 1;
}

/* static */Void DxUtil::calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut)
{
	gfsdk_float4x4 viewportMatrix;
	calcViewportMatrix(viewport, viewportMatrix);

	Float aspectRatio = viewport.m_width / viewport.m_height;

	viewInfoOut.setProjection(viewportMatrix, aspectRatio, view, projection, fov);
}

} // namespace HairWorks
} // namespace nvidia

