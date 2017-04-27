/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairInternal.h"

#include "NvHairViewInfo.h"

namespace nvidia {
namespace HairWorks { 

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ViewInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void ViewInfo::setProjection(const gfsdk_float4x4& viewportMatrix, float aspect, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov)
{
	// viewport matrix and inverse
	{
		m_viewportMatrix = viewportMatrix;
		m_inverseViewportMatrix = gfsdk_inverse(m_viewportMatrix);
		// aspect ratio
		m_aspect = aspect;
		m_fov = fov;
	}

	// view matrix and inverse
	{
		m_viewMatrix = view;
		m_inverseViewMatrix = gfsdk_inverse(view);
	}

	// camera position from inverse view matrix
	{
		gfsdk_float4x4 iview = m_inverseViewMatrix;
		m_eyePosition.x = iview._41;
		m_eyePosition.y = iview._42;
		m_eyePosition.z = iview._43;
	}

	// projection matrix and inverse
	{
		m_projectionMatrix = projection;
		
		bool valid = gfsdk_inverseProjection( m_inverseProjectionMatrix, m_projectionMatrix);
		if (!valid)
			NV_CO_LOG_ERROR("Projection matrix is not a valid projection.");
	}

	// view projection matrix and inverse
	m_viewProjectionMatrix = view * projection;
	m_inverseViewProjectionMatrix = m_inverseProjectionMatrix * m_inverseViewMatrix;
	
	// view projection + viewport
	m_inverseViewProjectionViewportMatrix = m_inverseViewportMatrix * m_inverseViewProjectionMatrix;
	
	// world to screen space
	{
		static const float clip2Tex[] = 
		{ 
			0.5,    0,    0,   0,
			0,	   -0.5,  0,   0,
			0,     0,     1,   0,
			0.5,   0.5,   0,   1 
		};
		m_worldToScreenMatrix = m_viewProjectionMatrix * (gfsdk_float4x4&)clip2Tex;
	}
}

void ViewInfo::init()
{
	gfsdk_makeIdentity(m_viewMatrix);
	gfsdk_makeIdentity(m_inverseViewMatrix);

	gfsdk_makeIdentity(m_projectionMatrix);
	gfsdk_makeIdentity(m_inverseProjectionMatrix);

	gfsdk_makeIdentity(m_viewProjectionMatrix);
	gfsdk_makeIdentity(m_inverseViewProjectionMatrix);


	gfsdk_makeIdentity(m_viewportMatrix);
	gfsdk_makeIdentity(m_inverseViewportMatrix);

	gfsdk_makeIdentity(m_inverseViewProjectionViewportMatrix);

	gfsdk_makeIdentity(m_worldToScreenMatrix);

	m_eyePosition = gfsdk_makeFloat3(0, 0, 0);

	m_aspect = 0.0f;
	m_fov = 70.0f;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FrameViewInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void RenderViewInfo::init()
{
	m_currentView.init();
	m_previousView.init();

	m_useLeftHanded = false;

	// Set defaults for cubemap
	{
		for (Int i = 0; i < NV_COUNT_OF(m_cubeViews); i++)
		{
			m_cubeViews[i].init();
			m_isCubeFaceVisible[i] = false;
		}
		m_useCubeMap = false;
	}
}

} // namespace HairWorks
} // namespace nvidia