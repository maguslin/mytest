/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairLodInfo.h"
#include "NvHairViewInfo.h"

#include "NvHairInstance.h"

namespace nvidia {
namespace HairWorks {

Void LodInfo::setLodFactor(Instance* inst, const ViewInfo& viewInfo)
{
	gfsdk_float4x4 viewMatrix = viewInfo.m_viewMatrix;

	gfsdk_float3 hairOrigin = inst->m_skinnedCenter;
	const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;

	// take into account model matrix in LOD computation
	hairOrigin = gfsdk_transformCoord(modelToWorld, hairOrigin);

	gfsdk_float3 hairOriginCamera = gfsdk_transformCoord(viewMatrix, hairOrigin);
	
	const Float unitInMeter = inst->m_asset->getUnitInCentimeters() * (1.0f / 100.0f);

	const Float defaultFov = 75.0f;
	const Float targetFov = viewInfo.m_fov;

	// distance to the camera
	const Float distanceToCamera = fabsf(hairOriginCamera.z);
	const Float distance = (distanceToCamera * unitInMeter) * targetFov * ( 1.0f / defaultFov);

	const InstanceDescriptor& params = inst->getDefaultMaterial();

	const Float startDistance	= min(params.m_distanceLodStart, params.m_distanceLodEnd);
	const Float endDistance	= max(params.m_distanceLodStart, params.m_distanceLodEnd);
	const Float rangeDistance	= endDistance - startDistance + FLT_EPSILON;
	const Float fadeDistance	= params.m_distanceLodFadeStart;

	// compute distance factor
	Float distanceFactor = gfsdk_saturate((distance - startDistance) / rangeDistance);

	// compute alpha factor
	Float alphaRatio = gfsdk_saturate((distance - fadeDistance) / (endDistance - fadeDistance + FLT_EPSILON));
	
	if (fadeDistance > endDistance)
		alphaRatio = 0.0;  // no alpha when fade distance is farther than lod end (turn off alpha feature)

	Float alphaFactor = alphaRatio;

	// compute detail factor
	Float startDetail = max(params.m_detailLodStart, params.m_detailLodEnd);
	Float endDetail = min(params.m_detailLodStart, params.m_detailLodEnd);
	Float rangeCloseup = endDetail - startDetail + FLT_EPSILON;

	Float detailFactor = (distance - startDetail) / rangeCloseup;
	detailFactor = gfsdk_saturate(detailFactor);

	// if lod is turned off set those to 0
	if (!params.m_enableLod)
	{
		distanceFactor = 0.0f;
		detailFactor = 0.0f;
		alphaFactor = 0.0f;
	}
	else
	{
		if (!params.m_enableDistanceLod)
		{
			distanceFactor = 0.0f;
			alphaFactor = 0.0f;
		}
		if (!params.m_enableDetailLod)
			detailFactor = 0.0f;
	}

	// Write out the results
	m_distance = distance;
	m_detailFactor = detailFactor;
	m_distanceFactor = distanceFactor;
	m_alphaFactor = alphaFactor;
}

Void LodInfo::calcDensityAndWidth(const InstanceDescriptor& material, Float* densityOut, Float* widthOut) const
{
	Float distanceDensity	= material.m_distanceLodDensity;
	Float distanceWidth		= material.m_distanceLodWidth;
	Float detailDensity		= material.m_detailLodDensity;
	Float detailWidth		= material.m_detailLodWidth;

	// Defaults
	Float density = material.m_density;
	Float width = material.m_width;

	if (m_distanceFactor > 0.0f) 
	{
		density = gfsdk_lerp(density, distanceDensity, m_distanceFactor);
		width = gfsdk_lerp(width, distanceWidth, m_distanceFactor);
	}
	if (m_detailFactor > 0.0f)
	{
		density = gfsdk_lerp(density, detailDensity, m_detailFactor);
		width = gfsdk_lerp(width, detailWidth, m_detailFactor);
	}

	*densityOut = density;
	*widthOut = width;
}

} // namespace HairWorks
} // namespace nvidia
