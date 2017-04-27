/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/Common/NvCoCommon.h>

#include "NvHairSphereCalculator.h"

namespace nvidia {
namespace HairWorks {

namespace { // anonymous
struct Vec2
{
	NV_FORCE_INLINE Vec2() {}
	NV_FORCE_INLINE Vec2(Float x, Float y) :m_x(x), m_y(y) {}
	NV_FORCE_INLINE Void set(Float x, Float y) { m_x = x; m_y = y; }
	Float m_x, m_y;
};
struct Mat2
{
	Void setRotate(Float rad)
	{
		Float c = cosf(rad);
		Float s = sinf(rad);
		m_rows[0].set(c, s);
		m_rows[1].set(-s, c);
	}
	Vec2 transform(const Vec2& in) const
	{
		return Vec2(in.m_x * m_rows[0].m_x + in.m_y * m_rows[1].m_x, in.m_x * m_rows[0].m_y + in.m_y * m_rows[1].m_y);
	}
	Vec2 m_rows[2];
};

} // namespace anonymous

void SphereCalculator::_addLine(Int v0, Int v1)
{
	NV_CORE_ASSERT(v0 < m_points.getSize() && v1 < m_points.getSize());
	if (v0 < v1)
	{
		Int* dst = m_indices.expandBy(2);
		dst[0] = v0;
		dst[1] = v1;
	}
}

void SphereCalculator::_addTri(Int v0, Int v1, Int v2)
{
	if (m_type == TYPE_LINE)
	{ 
		_addLine(v0, v1);
		_addLine(v1, v2);
		_addLine(v2, v0);
	}
	else
	{
		Int* dst = m_indices.expandBy(3);
		dst[0] = v0;
		dst[1] = v1;
		dst[2] = v2;
	}
}

void SphereCalculator::_addQuad(Int v0, Int v1, Int v2, Int v3)
{
	switch (m_type)
	{
		case TYPE_TRIANGLE:
		case TYPE_TRIANGULATED_LINE:
		{
			_addTri(v0, v1, v2);
			_addTri(v0, v2, v3);
			break;
		}
		case TYPE_LINE:
		{ 
			_addLine(v0, v1);
			_addLine(v1, v2);
			_addLine(v2, v3);
			_addLine(v3, v0);
			break;
		}
	}
}

void SphereCalculator::clear()
{
	m_indices.clear();
	m_points.clear();
}

void SphereCalculator::calc(Type type, Float radius, Int numRowsIn)
{
	m_type = type;
	clear();

	const Int numMajor = numRowsIn;
	const Int numMinor = numRowsIn + 1;

	const Int rows = numMajor + 1;
	const Int cols = numMinor;
	{
		const Float pi = 3.141592654f;

		const Float majorStep = (pi / (Float)numMajor); // 180 degrees
		const Float minorStep = (2.0f * pi / (Float)(numMinor)); // 360 degrees

		Mat2 rowRotate; rowRotate.setRotate(majorStep);
		Mat2 colRotate; colRotate.setRotate(minorStep);
		
		Vec2 rowVec(radius, 0.0f);
			// first and last vertex are the two apexs
		for (Int i = 0; i < rows; ++i)
		{
			const Float r0 = rowVec.m_y; 
			const Float z0 = rowVec.m_x; 

			Vec2 colVec(1.0f, 0.0f);
			for (Int j = 0; j < cols; ++j)
			{
				const Float x = colVec.m_x; 
				const Float y = colVec.m_y; 

				m_points.pushBack(gfsdk_makeFloat3(x * r0, y * r0, z0));

				// Only output one top and one bottom vertex
				if (i == 0 || i == rows - 1)
				{
					break;
				}
				colVec = colRotate.transform(colVec);
			}
			rowVec = rowRotate.transform(rowVec);
		}
	}

	{
		const Int bottom = Int(m_points.getSize() - 1);
		const Int bottomStart = 1 + (numMajor - 2) * cols;

		NV_CORE_ASSERT(bottom - cols == bottomStart);

		const Int top = 0;
		const Int topStart = 1;

		for (Int j = 0; j < cols - 1; j++)
		{
			// top
			_addTri(top, j + topStart, 1 + j + topStart);
			// bottom
			_addTri(j + bottomStart, bottom, j + 1 + bottomStart);
		}
		// Last top
		_addTri(top, (cols - 1) + topStart, topStart);
		// Last bottom
		_addTri((cols - 1) + bottomStart, bottom, bottomStart);
	}
	{
		// middle
		for (Int i = 1; i < numMajor - 1; i++) 
		{
			Int start0 = cols * (i - 1) + 1;
			Int start1 = cols * (i - 1 + 1) + 1;
			for (Int j = 0; j < cols - 1; j++) 
			{
				_addQuad(start0 + j, start1 + j, start1 + j + 1, start0 + j + 1);
			}
			_addQuad(start0 + cols - 1, start1 + cols - 1, start1, start0);
		}
	}
#if 0
	// Verify uniqueness
	for (Int i = 0; i < m_indices.getSize(); i += 2)
	{
		Int a0 = m_indices[i];
		Int a1 = m_indices[i + 1];
		for (Int j = i + 2; j < m_indices.getSize(); j += 2)
		{
			Int b0 = m_indices[j];
			Int b1 = m_indices[j + 1];

			Bool found = (a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0);
			NV_CORE_ASSERT(!found);
		}
	}
#endif
}

} // namespace HairWorks 
} // namespace nvidia 
