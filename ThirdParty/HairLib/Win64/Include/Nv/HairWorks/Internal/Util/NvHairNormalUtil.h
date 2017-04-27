/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_NORMAL_UTIL_H
#define NV_HAIR_NORMAL_UTIL_H

#include <Nv/Common/NvCoCommon.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

namespace nvidia {
namespace HairWorks {

struct NormalUtil
{
		/*! Calculate normals at vertices. 
		@param normalsOut should be numVertices in size. */
	static void calcNormals(Int numVertices, Int numTris, const UInt32* triIndices, const gfsdk_float4* vertices, gfsdk_float4* normalsOut);
	
		/*! Given a vector calculates a vector perpendicular */
	static gfsdk_float3 calcPerpendicular(const gfsdk_float3& in);

		/*! Calculate tangents to the normals.
		@param perpsOut should be numNormals in size */
	static void calcTangents(const gfsdk_float4* normalsIn, Int numNormals, gfsdk_float4* tangentsOut);

#if 0
		/*! Calculate the tangents at each vertex from the specified triangles.
		@param tangentsOut should be numVertices in size 
		\note This algorithm is probab*/
	static void calcTangents(Int numVertices, Int numTris, const UInt32* triIndices, const gfsdk_float4* vertices, gfsdk_float4* tangentsOut);
#endif
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_NORMAL_UTIL_H
