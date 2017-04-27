/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairNormalUtil.h"

#include <Nv/Common/NvCoMemory.h>

namespace nvidia {
namespace HairWorks {

/* static */void NormalUtil::calcNormals(Int numVertices, Int numTris, const UInt32* triIndices, const gfsdk_float4* vertices, gfsdk_float4* normalsOut)
{
	// TODO JS: This is rather dodgy casting with Vec4 to Vec3 and back.

	// compute mesh normal of the growth mesh

	NvCo::Memory::zero(normalsOut, sizeof(gfsdk_float4) *  numVertices);

	const UInt32 maxVertexIndex(numVertices);

	//// sum triangle normals
	const UInt32* endTriIndices = triIndices + (3 * numTris);
	for (; triIndices < endTriIndices; triIndices += 3)
	{
		UInt32 id0 = triIndices[0];
		UInt32 id1 = triIndices[1];
		UInt32 id2 = triIndices[2];

		if(id0 < maxVertexIndex && id1 < maxVertexIndex && id2 < maxVertexIndex)
		{
			gfsdk_float3 p0 = gfsdk_makeFloat3(vertices[id0].x, vertices[id0].y, vertices[id0].z);
			gfsdk_float3 p1 = gfsdk_makeFloat3(vertices[id1].x, vertices[id1].y, vertices[id1].z);
			gfsdk_float3 p2 = gfsdk_makeFloat3(vertices[id2].x, vertices[id2].y, vertices[id2].z);
			gfsdk_float3 p01 = p1 - p0;
			gfsdk_float3 p02 = p2 - p0;

			gfsdk_float3 n;
			n.x = p01.y * p02.z - p01.z * p02.y;
			n.y = p01.z * p02.x - p01.x * p02.z;
			n.z = p01.x * p02.y - p01.y * p02.x;

			((gfsdk_float3&)normalsOut[id0]) += n;
			((gfsdk_float3&)normalsOut[id1]) += n;
			((gfsdk_float3&)normalsOut[id2]) += n;
		}
	}

	{
		const float eps = 1e-5f;
		for (Int i = 0; i < numVertices; i++)
		{
			gfsdk_float3& normal = (gfsdk_float3&)normalsOut[i];
			const float lenSq = gfsdk_lengthSquared(normal);
			if (lenSq > eps * eps)
			{
				// Normalize if the lenth is long enough
				float scale = 1.0f / gfsdk_sqrt(lenSq);
				normal = normal * scale;
			}
			else
			{
				// Set an arbitrary normal
				normal = gfsdk_makeFloat3(1, 0, 0);
			}
		}
	}
}

gfsdk_float3 NormalUtil::calcPerpendicular(const gfsdk_float3& in)
{
	// Square to remove sign. (In simd this would be better with abs). Ie abs(x) < abs(y) <=> x*x < y*y
	Float xx = in.x * in.x;
	Float yy = in.y * in.y;
	Float zz = in.z * in.z;

	// Maps flags to the axis with smallest value
	static const UInt8 smallestIndexTable[8] = { 0, 2, 1, 1, 0, 2, 0, 0 };
	// Has a bit for each condition. 3 bits means table is 8 in size
	const Int flags = (Int(xx < yy) << 2) + (Int(yy < zz) << 1) + (Int(zz < xx));
	const Int smallestIndex = smallestIndexTable[flags];

#if 0 && NV_DEBUG
	// To check the table
	{
		const float* values = &in.x;
		float smallest = values[smallestIndex];
		for (Int i = 0; i < 3; i++)
		{
			NV_CORE_ASSERT(smallest * smallest <= values[i] * values[i]);
		}
	}
#endif

	switch (smallestIndex)
	{
		// Result is cross(v1, in);
		// v1 is the vector with a 1 on the smallest index. 
		default:
		// v1 = (1, 0, 0)
		case 0: return gfsdk_makeFloat3(0.0f, -in.z, in.y);
		// v1 = (0, 1, 0)
		case 1:	return gfsdk_makeFloat3(in.z, 0.0f, -in.x);
		// v1 = (0, 0, 1)
		case 2: return gfsdk_makeFloat3(-in.y, in.x, 0.0f);
	}
}


/* static */void NormalUtil::calcTangents(const gfsdk_float4* normalsIn, Int numNormals, gfsdk_float4* tangentsOut)
{

	for (Int i = 0; i < numNormals; i++)
	{
		const gfsdk_float3& normal = (const gfsdk_float3&)normalsIn[i];

		gfsdk_float3 perp = calcPerpendicular(normal);
		gfsdk_float3 tangent = gfsdk_getNormalized(perp);

#if 0 && NV_DEBUG
		// Ensures it really is perpendicular
		{
			const float eps = 1e-4f;
			gfsdk_float3 v = gfsdk_cross(tangent, normal);
			float len = gfsdk_lengthSquared(v);
			NV_CORE_ASSERT(len > 1.0f - eps && len < 1.0f + eps);
		}
#endif

		tangentsOut[i] = gfsdk_makeFloat4(tangent, 0.0f);
	}
}

#if 0
/* static */void NormalUtil::calcTangents(Int numVertices, Int numTris, const UInt32* triIndices, const gfsdk_float4* vertices, gfsdk_float4* tangentsOut)
{
	// compute mesh tangents of the growth mesh
	NvCo::Memory::zero(tangentsOut, sizeof(gfsdk_float4) * numVertices);

	// TODO JS: This is doing very hairy stuff with casting vec4s to 3s etc. Who knows whats in w.

	const UInt32 maxVertexIndex(numVertices);
	//// sum triangle normals
	const UInt32* endTriIndices = triIndices + (3 * numTris);
	for (; triIndices < endTriIndices; triIndices += 3)
	{
		UInt32 id0 = triIndices[0];
		UInt32 id1 = triIndices[1];
		UInt32 id2 = triIndices[2];

		if(id0 < maxVertexIndex && id1 < maxVertexIndex && id2 < maxVertexIndex)
		{
			gfsdk_float3 p0 = gfsdk_makeFloat3(vertices[id0].x, vertices[id0].y, vertices[id0].z);
			gfsdk_float3 p1 = gfsdk_makeFloat3(vertices[id1].x, vertices[id1].y, vertices[id1].z);
			gfsdk_float3 p2 = gfsdk_makeFloat3(vertices[id2].x, vertices[id2].y, vertices[id2].z);

			gfsdk_float3 p01 = p1 - p0;
			gfsdk_float3 p02 = p2 - p0;
			gfsdk_float3 p12 = p2 - p1;

			((gfsdk_float3&)tangentsOut[id0]) += p01;
			((gfsdk_float3&)tangentsOut[id0]) += p02;

			((gfsdk_float3&)tangentsOut[id1]) -= p01;
			((gfsdk_float3&)tangentsOut[id1]) += p12;

			((gfsdk_float3&)tangentsOut[id2]) -= p02;
			((gfsdk_float3&)tangentsOut[id2]) -= p12;
		}
	}
}
#endif

} // namespace HairWorks 
} // namespace nvidia 

