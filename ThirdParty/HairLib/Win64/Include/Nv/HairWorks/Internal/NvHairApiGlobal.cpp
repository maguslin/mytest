/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairApiGlobal.h"

#include <Nv/Common/Random/NvCoFogRandomGenerator.h>

namespace nvidia {
namespace HairWorks { 

ApiGlobal::ApiGlobal()
{
	// create LUT for general scalar noise
	NvCo::FogRandomGenerator rand(52542);
	// fill CPU buffer
	m_scalarNoiseLut.setSize(STRAND_NOISE_TABLE_SIZE);
	rand.nextFloats(m_scalarNoiseLut.begin(), STRAND_NOISE_TABLE_SIZE);

	m_vectorNoise.setSize(STRAND_NOISE_TABLE_SIZE);
	rand.nextFloatsModOne(&m_vectorNoise.begin()->x, IndexT(STRAND_NOISE_TABLE_SIZE * sizeof(gfsdk_float3) / sizeof(Float32)));

	// fill CPU buffer
	{
		m_barycentricNoise.setSize(STRAND_NOISE_TABLE_SIZE);
		gfsdk_float2* barycentricCoordinates = m_barycentricNoise;
		for (int c = 0; c < (int)STRAND_NOISE_TABLE_SIZE; ++c)
		{
			float coord1 = rand.nextFloat();
			float coord2 = rand.nextFloat();
			if (coord1 + coord2 > 1)
			{
				coord1 = 1.0f - coord1;
				coord2 = 1.0f - coord2;
			}
			barycentricCoordinates[c] = gfsdk_makeFloat2(coord1, coord2);
		}
	}
}


Int ApiGlobal::cancelAsync(const NvCo::HandleMapHandle* handles, Int numHandles, Bool allReferences)
{
	NV_UNUSED(handles)
	NV_UNUSED(numHandles)
	NV_UNUSED(allReferences)
	return 0;
}

Int ApiGlobal::cancelAsync(InstanceId instId, Bool allReferences)
{
	NV_UNUSED(instId)
	NV_UNUSED(allReferences)
	return 0;
}

} // namespace HairWorks
} // namespace nvidia
