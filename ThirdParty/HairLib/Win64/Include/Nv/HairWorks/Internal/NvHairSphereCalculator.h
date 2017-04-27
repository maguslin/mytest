/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SPHERE_CALCULATOR_H
#define NV_HAIR_SPHERE_CALCULATOR_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

namespace nvidia {
namespace HairWorks { 

/*! Class to generate spheres. Set the m_type member to control what kind of triangle is produced */
class SphereCalculator
{
	NV_CO_DECLARE_CLASS_BASE(SphereCalculator);

	enum Type
	{
		TYPE_LINE,
		TYPE_TRIANGULATED_LINE,
		TYPE_TRIANGLE,
	};

		/// Clears the contents
	void clear();
		/// Do the calculation with specifed radius and number of rows
	void calc(Type type, Float radius, Int numRowsIn);

	SphereCalculator():m_type(TYPE_TRIANGULATED_LINE) {}

	NvCo::Array<gfsdk_float3> m_points;
	NvCo::Array<Int> m_indices;

	protected:
	void _addLine(Int v0, Int v1);
	void _addTri(Int v0, Int v1, Int v2);
	void _addQuad(Int v0, Int v1, Int v2, Int v3);

	Type m_type;				//< Output type
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_SPHERE_CALCULATOR_H
