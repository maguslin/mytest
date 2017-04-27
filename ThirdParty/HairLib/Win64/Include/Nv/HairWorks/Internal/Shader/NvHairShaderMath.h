/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SHADER_MATH_H
#define NV_HAIR_SHADER_MATH_H

#include "NvHairInternalShaderCommon.h"

#ifndef _CPP // for shader use only

////////////////////////////////////////////////////////////////////////////////
// inverse of a quaternion
inline float4 gfsdk_inverse(const float4 q)
{
	return float4(-q.x, -q.y, -q.z, q.w);
}

////////////////////////////////////////////////////////////////////////////////
// rotation of a unit vector by a quaternion
inline float3 gfsdk_rotate(const float4 q, const float3 v)
{
	const float vx = 2.0f * v.x;
	const float vy = 2.0f * v.y;
	const float vz = 2.0f * v.z;
	const float w2 = q.w * q.w - 0.5f;
	const float dot2 = (q.x * vx + q.y * vy + q.z * vz);

	float3 ret;

	ret.x = vx * w2 + (q.y * vz - q.z * vy) * q.w + q.x * dot2;
	ret.y = vy * w2 + (q.z * vx - q.x * vz) * q.w + q.y * dot2;
	ret.z = vz * w2 + (q.x * vy - q.y * vx) * q.w + q.z * dot2;

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// multiply 
inline float4 gfsdk_multiply(const float4 q0, const float4 q1)
{
	float4 q;

	const float tx = q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y;
	const float ty = q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z;
	const float tz = q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x;

	q.w = q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z  * q1.z;
	q.x = tx;
	q.y = ty;
	q.z = tz;

	return q;
}

////////////////////////////////////////////////////////////////////////////////
// inverse rotation of a unit vector by a quaternion
inline float3 gfsdk_rotateInv(const float4 q, const float3 v)
{
	const float vx = 2.0f * v.x;
	const float vy = 2.0f * v.y;
	const float vz = 2.0f * v.z;
	const float w2 = q.w * q.w - 0.5f;
	const float dot2 = (q.x * vx + q.y * vy + q.z * vz);

	float3 ret;

	ret.x = vx * w2 - (q.y * vz - q.z * vy) * q.w + q.x * dot2;
	ret.y = vy * w2 - (q.z * vx - q.x * vz) * q.w + q.y * dot2;
	ret.z = vz * w2 - (q.x * vy - q.y * vx) * q.w + q.z * dot2;

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float4 gfsdk_makeRotation(const float3 axis, float radian)
{
	float a = radian * 0.5f;
	float s = sin(a);
	
	float4 q;
	q.w = cos(a);
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;

	return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float4 gfsdk_rotateBetween(const float3 from, const float3 to)
{
	float3 axis = cross(from, to);

	float axisLength = length(axis);
	if (axisLength < 1e-2)
		return float4(0,0,0,1);

	axis /= axisLength;

	float dotT = clamp(dot(from, to), -1.0f, 1.0f);
	float angle = acos( dotT );

	return gfsdk_makeRotation(axis, angle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
inline float4 gfsdk_transferFrame(const float4 from, const float4 to)
{
	return gfsdk_multiply( to, gfsdk_inverse(from));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get x basis vector from quaternion
float3 gfsdk_getBasisX(const float4 q)
{
	const float x2 = q.x * 2.0f;
	const float w2 = q.w * 2.0f;
	return float3(
		(q.w * w2) - 1.0f + q.x * x2,
		(q.z * w2)        + q.y * x2,
		(-q.y * w2)       + q.z * x2);
}

////////////////////////////////////////////////////////////////////////////////
// get y basis vector from quaternion
float3 gfsdk_getBasisY(const float4 q)
{
	const float y2 = q.y * 2.0f;
	const float w2 = q.w * 2.0f;
	return float3(
		(-q.z * w2)			+ q.x * y2,
		( q.w * w2) - 1.0f	+ q.y * y2,
		( q.x * w2)			+ q.z * y2);
}

////////////////////////////////////////////////////////////////////////////////
// get z basis vector from quaternion
float3 gfsdk_getBasisZ(const float4 q)
{
	const float z2 = q.z * 2.0f;
	const float w2 = q.w * 2.0f;
	
	return float3(	
		( q.y * w2)			+ q.x * z2,
		(-q.x * w2)			+ q.y * z2,
		( q.w * w2) - 1.0f	+ q.z * z2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get x basis vector from an orthonormal matrix
float3 gfsdk_getBasisX(const matrix4 m)
{
	return float3(m._11, m._12, m._13);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get y basis vector from an orthonormal matrix
float3 gfsdk_getBasisY(const matrix4 m)
{
	return float3(m._21, m._22, m._23);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get z basis vector from an orthonormal matrix
float3 gfsdk_getBasisZ(const matrix4 m)
{
	return float3(m._31, m._32, m._33);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set x basis vector for an orthonormal matrix
void gfsdk_setBasisX(inout matrix4 m, float3 a)
{
	m._11 = a.x;
	m._12 = a.y;
	m._13 = a.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set y basis vector for an orthonormal matrix
void gfsdk_setBasisY(inout matrix4 m, float3 a)
{
	m._21 = a.x;
	m._22 = a.y;
	m._23 = a.z;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set z basis vector for an orthonormal matrix
void gfsdk_setBasisZ(inout matrix4 m, float3 a)
{
	m._31 = a.x;
	m._32 = a.y;
	m._33 = a.z;
}
////////////////////////////////////////////////////////////////////////////////
// rotate only basis (3x3) part of a matrix with quaternion
////////////////////////////////////////////////////////////////////////////////
matrix4 gfsdk_changeBasis(const float4 q, const matrix4 in_m)
{
	float3 axisX	= gfsdk_getBasisX(in_m);
	float3 axisY	= gfsdk_getBasisY(in_m);
	float3 axisZ	= gfsdk_getBasisZ(in_m);

	axisX = gfsdk_rotate(q, axisX);
	axisY = gfsdk_rotate(q, axisY);
	axisZ = gfsdk_rotate(q, axisZ);
			
	matrix4 m = in_m;
	gfsdk_setBasisX(m, axisX);
	gfsdk_setBasisY(m, axisY);
	gfsdk_setBasisZ(m, axisZ);

	return m;
}

////////////////////////////////////////////////////////////////////////////////
// make quaternion from 3 orthogonal unit vectors
void gfsdk_makeRotation(inout float4 q, const float3 ax, const float3 ay, const float3 az)
{
	float tr = ax.x + ay.y + az.z;
	float h;

	if(tr >= 0)
	{
		h = sqrt(tr +1);
		q.w = 0.5f * h;
		h = 0.5f / h;

		q.x = (ay.z - az.y) * h;
		q.y = (az.x - ax.z) * h;
		q.z = (ax.y - ay.x) * h;
	}
	else
	{
		float max = ax.x;
		int i = 0; 
		if (ay.y > max)
		{
			i = 1; 
			max = ay.y;
		}
		if (az.z > max)
			i = 2; 

		switch (i)
		{
		case 0:
			h = sqrt((ax.x - (ay.y + az.z)) + 1);
			q.x = float(0.5) * h;
			h = float(0.5) / h;

			q.y = (ay.x + ax.y) * h; 
			q.z = (ax.z + az.x) * h;
			q.w = (ay.z - az.y) * h;
			break;
		case 1:
			h = sqrt((ay.y - (az.z + ax.x)) + 1);
			q.y = float(0.5) * h;
			h = float(0.5) / h;

			q.z = (az.y + ay.z) * h;
			q.x = (ay.x + ax.y) * h;
			q.w = (az.x - ax.z) * h;
			break;
		case 2:
			h = sqrt((az.z - (ax.x + ay.y)) + 1);
			q.z = float(0.5) * h;
			h = float(0.5) / h;

			q.x = (ax.z + az.x) * h;
			q.y = (az.y + ay.z) * h;
			q.w = (ax.y - ay.x) * h;
			break;
		default: // Make compiler happy
			q.x = q.y = q.z = q.w = 0;
			break;
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// build quaternion from 2 perpendicular axis (3rd from those two)
/////////////////////////////////////////////////////////////////////////////////////////////////
inline float4 gfsdk_quatFromAxis(const float3 n, const float3 t)
{
	float4 q;

	float3 az = n; 
	float3 ax = t;
	float3 ay = normalize(cross(az, ax));
	ax = normalize(cross(ay,az));
	gfsdk_makeRotation(q, ax, ay, az);

	return q;
}

//////////////////////////////////////////////////////////////////////////////
float gfsdk_splineInterpolate(float4 v, float u)
{
	float V0 = v.x, V1 = v.y, V2 = v.z, V3 = v.w;
	const float oneThird = 1.0f / 3.0f;
	const float twoThird = 2.0f / 3.0f;

	if ( u < oneThird)
	{
		V0 = v.x, V1 = v.x, V2 = v.y, V3 = v.z;
		u /= oneThird;
	}
	else if (u > twoThird)
	{
		V0 = v.y, V1 = v.z, V2 = v.w, V3 = v.w;
		u = (u - twoThird) / oneThird;
	}
	else
	{
		u = (u - oneThird ) / oneThird;
	}

	float4 basis;

	float u2 = u * u;
	float u3 = u2 * u;

	float B0 = -0.5f * u3 + 1.0f * u2 - 0.5f * u + 0.0f;
	float B1 = 1.5f  * u3 - 2.5f * u2 + 0.0f * u + 1.0f;
	float B2 = -1.5f * u3 + 2.0f * u2 + 0.5f * u + 0.0f;
	float B3 = 0.5f  * u3 - 0.5f * u2 + 0.0f * u + 0.0f;

	float val = B0 * V0 + B1 * V1 + B2 * V2 + B3 * V3;
	return saturate(val);
}

////////////////////////////////////////////////////////////////////////////////////////
matrix4 gfsdk_makeTranslation(float3 t)
{
	matrix4 m;

	m._11 = 1.0f; m._12 = 0.0f; m._13 = 0.0f; m._14 = 0.0f;
	m._21 = 0.0f; m._22 = 1.0f; m._23 = 0.0f; m._24 = 0.0f;
	m._31 = 0.0f; m._32 = 0.0f; m._33 = 1.0f; m._34 = 0.0f;
	m._41 = t.x; m._42 = t.y; m._43 = t.z; m._44 = 1.0f;

	return m;
}

////////////////////////////////////////////////////////////////////////////////////////
float3 gfsdk_getTranslation(const matrix4 m)
{
	return float3(m._41, m._42, m._43);
}

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_setTranslation(inout matrix4 m, float3 t)
{
	m._41 = t.x; m._42 = t.y; m._43 = t.z; 
}

////////////////////////////////////////////////////////////////////////////////
matrix4 mul(const matrix4 in1, const matrix4 in2)
{
#define MATRIX_SUM(OUT, IN1, IN2, ROW, COL) OUT._##ROW##COL = IN1._##ROW##1 * IN2._1##COL + IN1._##ROW##2 * IN2._2##COL + IN1._##ROW##3 * IN2._3##COL + IN1._##ROW##4 * IN2._4##COL;

	matrix4 m;

	MATRIX_SUM(m, in1, in2, 1, 1); 
	MATRIX_SUM(m, in1, in2, 1, 2);
	MATRIX_SUM(m, in1, in2, 1, 3);
	MATRIX_SUM(m, in1, in2, 1, 4);

	MATRIX_SUM(m, in1, in2, 2, 1); 
	MATRIX_SUM(m, in1, in2, 2, 2);
	MATRIX_SUM(m, in1, in2, 2, 3);
	MATRIX_SUM(m, in1, in2, 2, 4);

	MATRIX_SUM(m, in1, in2, 3, 1); 
	MATRIX_SUM(m, in1, in2, 3, 2);
	MATRIX_SUM(m, in1, in2, 3, 3);
	MATRIX_SUM(m, in1, in2, 3, 4);

	MATRIX_SUM(m, in1, in2, 4, 1); 
	MATRIX_SUM(m, in1, in2, 4, 2);
	MATRIX_SUM(m, in1, in2, 4, 3);
	MATRIX_SUM(m, in1, in2, 4, 4);

#undef MATRIX_SUM

	return m;
}
#endif

#endif  // NV_HAIR_SHADER_MATH_H