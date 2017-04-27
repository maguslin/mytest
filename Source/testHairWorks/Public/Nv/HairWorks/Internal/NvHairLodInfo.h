/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_LOD_UTIL_H
#define NV_HAIR_LOD_UTIL_H

#include <Nv/HairWorks/Internal/NvHairInternal.h>

namespace nvidia {
namespace HairWorks {

struct ViewInfo;
class Instance;

struct LodInfo
{
	NV_CO_DECLARE_CLASS_BASE(LodInfo);

	Float m_distance;
	Float m_distanceFactor;
	Float m_alphaFactor;
	Float m_detailFactor;

		/// Set the lod factor from resources/view
	Void setLodFactor(Instance* inst, const ViewInfo& viewInfo);

		// Computes density and width after LOD processing
	Void calcDensityAndWidth(const InstanceDescriptor& material, Float* densityOut, Float* widthOut) const;
public:
	LodInfo():
		m_distance(0.0f),
		m_distanceFactor(0.0f),
		m_detailFactor(0.0f),
		m_alphaFactor(0.0f)
	{
	}
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_LOD_UTIL_H
