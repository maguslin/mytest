/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_VIEW_INFO_H
#define NV_HAIR_VIEW_INFO_H

#include <Nv/HairWorks/Internal/NvHairInternal.h>

namespace nvidia {
namespace HairWorks {

/*! Internal structure for all the camera/view related stuffs */
struct ViewInfo
{
		/// Set the projection
		/// Aspect ratio is width/height
	void setProjection(const gfsdk_float4x4& viewportMatrix, float aspectRatio, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov);

	void init();

	gfsdk_float4x4  m_viewportMatrix;
	gfsdk_float4x4  m_inverseViewportMatrix;

	gfsdk_float4x4	m_viewMatrix;
	gfsdk_float4x4	m_inverseViewMatrix;

	gfsdk_float4x4	m_projectionMatrix;
	gfsdk_float4x4	m_inverseProjectionMatrix;

	gfsdk_float4x4	m_viewProjectionMatrix;
	gfsdk_float4x4	m_inverseViewProjectionMatrix;

	gfsdk_float4x4	m_inverseViewProjectionViewportMatrix;

	gfsdk_float4x4	m_worldToScreenMatrix;

	gfsdk_float3	m_eyePosition;

	float			m_aspect;
	float			m_fov;
};

/*! All of the view data needed to be able to render. 
 It consists of more than one ViewInfo - because it may be rendering to multiple 'views' (say with a cubemap)
or it may need additional view information to render a single 'view' (for example it needs the last frame 
and this frames view information to calculate pixel velocity */
struct RenderViewInfo
{
	void init();

	Bool m_useLeftHanded;

	ViewInfo m_currentView;
	ViewInfo m_previousView;

	ViewInfo m_cubeViews[6];
	Bool m_isCubeFaceVisible[6];
	Bool m_useCubeMap;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_VIEW_INFO_H
