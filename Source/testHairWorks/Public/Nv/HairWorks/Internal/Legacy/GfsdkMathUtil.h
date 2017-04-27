/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */
#ifndef _MATH_UTIL_H_
#define _MATH_UTIL_H_

#include <Nv/HairWorks/NvHairCommon.h>
//#include "NvFoundation.h"

#include "math.h"
#include "float.h"

inline float gfsdk_sqrt(float v)
{
	return sqrtf(v);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_lerp(float v1, float v2, float t)
{
	return (1.0f - t) * v1 + t * v2;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_min(float x, float y) {
	return (x < y) ? x : y;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_max(float x, float y) {
	return (x > y) ? x : y;
}

////////////////////////////////////////////////////////////////////////////////////////
inline NvUInt32 gfsdk_min(NvUInt32 x, NvUInt32 y) {
	return (x < y) ? x : y;
}

////////////////////////////////////////////////////////////////////////////////////////
inline NvUInt32 gfsdk_max(NvUInt32 x, NvUInt32 y) {
	return (x > y) ? x : y;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_saturate(float x) {
	if (x < 0.0f) return 0.0f;
	if (x > 1.0f) return 1.0f;
	return x;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_clamp(float x, float a, float b) {
	if (x < a) return a;
	if (x > b) return b;
	return x;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float2 gfsdk_makeFloat2(float x, float y)
{
	gfsdk_float2 v;
	v.x = x;
	v.y = y;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float2& operator*=(gfsdk_float2& v, float s)
{
	v.x *= s;
	v.y *= s;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_makeFloat3(float x, float y, float z)
{
	gfsdk_float3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 operator+(const gfsdk_float3 &p0, const gfsdk_float3 &p1) 
{
	return gfsdk_makeFloat3(p0.x + p1.x, p0.y + p1.y, p0.z + p1.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3& operator+=(gfsdk_float3 &v, const gfsdk_float3 & v1)
{
	v.x += v1.x; v.y += v1.y; v.z += v1.z;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 operator-(const gfsdk_float3 &p0, const gfsdk_float3 &p1) 
{
	return gfsdk_makeFloat3(p0.x - p1.x, p0.y - p1.y, p0.z - p1.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3& operator-=(gfsdk_float3 &v, const gfsdk_float3 & v1)
{
	v.x -= v1.x; v.y -= v1.y; v.z -= v1.z;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 operator*(float s, const gfsdk_float3 &p) 
{
	return gfsdk_makeFloat3(s * p.x, s * p.y, s * p.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 operator*(const gfsdk_float3 &p, float s) 
{
	return gfsdk_makeFloat3(s * p.x, s * p.y, s * p.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_dot(const gfsdk_float3& v0, const gfsdk_float3 &v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_lengthSquared(const gfsdk_float3& v) {
	return gfsdk_dot(v,v);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_length(const gfsdk_float3 &v) {
	return gfsdk_sqrt(gfsdk_lengthSquared(v));
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_getNormalized(const gfsdk_float3 &v) {

	float l = gfsdk_length(v);
	if (l != 0.0f)
	{
		return v * (1.0f / l);
	}
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_cross(const gfsdk_float3& v1, const gfsdk_float3& v2)
{
	return gfsdk_makeFloat3(
		v1.y * v2.z - v1.z * v2.y, 
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_lerp(const gfsdk_float3& v1, const gfsdk_float3& v2, float t)
{
	return gfsdk_makeFloat3(gfsdk_lerp(v1.x, v2.x, t), gfsdk_lerp(v1.y, v2.y, t), gfsdk_lerp(v1.z, v2.z, t));
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_min(const gfsdk_float3& v1, const gfsdk_float3 &v2)
{
	return gfsdk_makeFloat3(
		gfsdk_min(v1.x, v2.x),
		gfsdk_min(v1.y, v2.y),
		gfsdk_min(v1.z, v2.z)
		);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float3 gfsdk_max(const gfsdk_float3& v1, const gfsdk_float3 &v2)
{
	return gfsdk_makeFloat3(
		gfsdk_max(v1.x, v2.x),
		gfsdk_max(v1.y, v2.y),
		gfsdk_max(v1.z, v2.z)
		);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_min(const gfsdk_float3 &v)
{
	return gfsdk_min(gfsdk_min(v.x, v.y), v.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_max(const gfsdk_float3 &v)
{
	return gfsdk_max(gfsdk_max(v.x, v.y), v.z);
}

////////////////////////////////////////////////////////////////////////////////////////
inline bool	gfsdk_equal(const gfsdk_float3& a, const gfsdk_float3& b)
{
	return (&a == &b) || (a.x == b.x && a.y == b.y && a.z == b.z);
}

////////////////////////////////////////////////////////////////////////////////
// float4
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 gfsdk_makeFloat4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
{
	gfsdk_float4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 gfsdk_makeFloat4(const gfsdk_float3& v, float w = 0.0f)
{
	return gfsdk_makeFloat4(v.x, v.y, v.z, w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline bool	gfsdk_equal(const gfsdk_float4& a, const gfsdk_float4& b)
{
	return (&a == &b) || (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 operator+(const gfsdk_float4 &p0, const gfsdk_float4 &p1) 
{
	return gfsdk_makeFloat4(p0.x + p1.x, p0.y + p1.y, p0.z + p1.z, p0.w + p1.w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4& operator+=(gfsdk_float4 &v, const gfsdk_float4 & v1)
{
	v.x += v1.x; v.y += v1.y; v.z += v1.z; v.w += v1.w;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 operator-(const gfsdk_float4 &p0, const gfsdk_float4 &p1) 
{
	return gfsdk_makeFloat4(p0.x - p1.x, p0.y - p1.y, p0.z - p1.z, p0.w - p1.w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4& operator-=(gfsdk_float4 &v, const gfsdk_float4 & v1)
{
	v.x -= v1.x; v.y -= v1.y; v.z -= v1.z; v.w -= v1.w;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 operator*(float s, const gfsdk_float4 &p) 
{
	return gfsdk_makeFloat4(s * p.x, s * p.y, s * p.z, s * p.w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 operator*(const gfsdk_float4 &p, float s) 
{
	return gfsdk_makeFloat4(s * p.x, s * p.y, s * p.z, s * p.w);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_dot(const gfsdk_float4& v0, const gfsdk_float4 &v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_lengthSquared(const gfsdk_float4& v) {
	return gfsdk_dot(v,v);
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_length(const gfsdk_float4 &v) {
	return gfsdk_sqrt(gfsdk_lengthSquared(v));
}

////////////////////////////////////////////////////////////////////////////////////////
inline float gfsdk_length3(const gfsdk_float4 &v) {
	return gfsdk_sqrt(gfsdk_lengthSquared((gfsdk_float3&)v));
}

////////////////////////////////////////////////////////////////////////////////////////
inline const gfsdk_float4 gfsdk_getNormalized(const gfsdk_float4 &v) {
	gfsdk_float4 nv = v;

	float l = gfsdk_length(nv);
	if (l > FLT_EPSILON)
	{
		const float s = 1.0f / l;

		nv.x *= s; 
		nv.y *= s; 
		nv.z *= s; 
		nv.w *= s;
	}

	return nv;
}

////////////////////////////////////////////////////////////////////////////////////////
inline gfsdk_float4 gfsdk_lerp(const gfsdk_float4& v1, const gfsdk_float4& v2, float t)
{
	return gfsdk_makeFloat4(
		gfsdk_lerp(v1.x, v2.x, t), 
		gfsdk_lerp(v1.y, v2.y, t), 
		gfsdk_lerp(v1.z, v2.z, t),
		gfsdk_lerp(v1.w, v2.w, t)
		);
}

////////////////////////////////////////////////////////////////////////////////
// quaternion
////////////////////////////////////////////////////////////////////////////////
typedef gfsdk_float4 gfsdk_quaternion;

gfsdk_float4		gfsdk_quaternionConjugate(const gfsdk_quaternion& in);
void				gfsdk_makeIdentity(gfsdk_quaternion& q);
gfsdk_quaternion	gfsdk_quaternionMultiply(const gfsdk_quaternion& q0, const gfsdk_quaternion& q1);
gfsdk_float3		gfsdk_rotate(const gfsdk_quaternion &q, const gfsdk_float3 &v);
gfsdk_float3		gfsdk_rotateInv(const gfsdk_quaternion &q, const gfsdk_float3 &v);
gfsdk_quaternion	gfsdk_slerp(const gfsdk_quaternion& q0, const gfsdk_quaternion& q1, float t);
gfsdk_float3		gfsdk_getBasisX(const gfsdk_quaternion &q);
gfsdk_float3		gfsdk_getBasisY(const gfsdk_quaternion &q);
gfsdk_float3		gfsdk_getBasisZ(const gfsdk_quaternion &q);
void				gfsdk_makeRotation(gfsdk_quaternion&m, const gfsdk_float3& xaxis, const gfsdk_float3& yaxis, const gfsdk_float3& zaxis);
gfsdk_quaternion	gfsdk_makeRotation(const gfsdk_float3 &axis, float radian);
gfsdk_quaternion	gfsdk_rotateBetween(const gfsdk_float3 from, const gfsdk_float3 to);
gfsdk_quaternion	gfsdk_quatFromAxis(const gfsdk_float3 n, const gfsdk_float3 t);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// 4x4 matrix
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void				gfsdk_makeIdentity(gfsdk_float4x4& transform);
void				gfsdk_makeDiagonal(gfsdk_float4x4& transform, const gfsdk_float3& d);
void				gfsdk_makeFloat4x4(gfsdk_float4x4& m, const float* p);
void				gfsdk_makeScale(gfsdk_float4x4& m, const gfsdk_float3& s);
void				gfsdk_makeRotation(gfsdk_float4x4&m, const gfsdk_float3& xaxis, const gfsdk_float3& yaxis, const gfsdk_float3& zaxis);
void				gfsdk_makeRotation(gfsdk_float4x4& m, const gfsdk_quaternion& q);
void				gfsdk_makeRotation(gfsdk_float4x4& m, const gfsdk_float3& from, const gfsdk_float3& to);
void				gfsdk_makeTranslation(gfsdk_float4x4& m, const gfsdk_float3& t);
gfsdk_float4x4		gfsdk_makeTransform(const gfsdk_quaternion& q, const gfsdk_float3& t, const gfsdk_float3 &s);
bool				gfsdk_equal(const gfsdk_float4x4& a, const gfsdk_float4x4& b);

gfsdk_float3		gfsdk_transformCoord(const gfsdk_float4x4 &m, gfsdk_float3 op);
gfsdk_float3		gfsdk_transformVector(const gfsdk_float4x4 &m, gfsdk_float3 op);

float				gfsdk_getDeterminant(const gfsdk_float4x4& m);
gfsdk_float4x4		gfsdk_inverse(const gfsdk_float4x4& m);
void				gfsdk_orthonormalize(gfsdk_float4x4& m);

gfsdk_float4x4		operator*(const gfsdk_float4x4& in1, const gfsdk_float4x4& in2);
gfsdk_float4x4&		operator*=(gfsdk_float4x4& m, float s);
gfsdk_float4x4&		operator+=(gfsdk_float4x4& m1, const gfsdk_float4x4& m2);

gfsdk_quaternion	gfsdk_getRotation(const gfsdk_float4x4& m);
gfsdk_float3		gfsdk_getScale(const gfsdk_float4x4& m);
gfsdk_float3		gfsdk_getTranslation(const gfsdk_float4x4& m);
void				gfsdk_setTranslation(gfsdk_float4x4& m, const gfsdk_float3 &v);

float&				gfsdk_ref(gfsdk_float4x4& m, int row, int col);
gfsdk_float3		gfsdk_getColumn(const gfsdk_float4x4&m, int col);
gfsdk_float3		gfsdk_getRow(const gfsdk_float4x4&m, int row);
void				gfsdk_setColumn(const gfsdk_float4x4&m, int col, const gfsdk_float3& v);
void				gfsdk_setRow(const gfsdk_float4x4&m, int row, const gfsdk_float3& v);

gfsdk_float4x4		gfsdk_lerp(gfsdk_float4x4& start, gfsdk_float4x4& end, float t);
bool				gfsdk_inverseProjection(gfsdk_float4x4& out, const gfsdk_float4x4& in);
void				gfsdk_polarDecomposition(const gfsdk_float4x4 &A, gfsdk_float4x4 &R);

bool				gfsdk_isLeftHanded(const gfsdk_float4x4 &m);
gfsdk_float4x4		gfsdk_makeOrthoLH(float orthoW, float orthoH, float zNear, float zFar);
gfsdk_float4x4		gfsdk_makeOrthoRH(float orthoW, float orthoH, float zNear, float zFar);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// dual quaternion 
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

gfsdk_dualquaternion	gfsdk_makeDQ(const gfsdk_quaternion& q, const gfsdk_float3& t);
gfsdk_dualquaternion	gfsdk_makeDQ(const gfsdk_float4x4& m);
gfsdk_float4x4			gfsdk_makeTransform(const gfsdk_dualquaternion& dq);
void					gfsdk_makeIdentity(gfsdk_dualquaternion& dq);
void					gfsdk_makeZero(gfsdk_dualquaternion& dq);

gfsdk_dualquaternion	gfsdk_getNormalized(const gfsdk_dualquaternion & dq);

gfsdk_dualquaternion	operator*(float s, const gfsdk_dualquaternion & dq);
gfsdk_dualquaternion	operator*(const gfsdk_dualquaternion & dq, float s);
gfsdk_dualquaternion&	operator+=(gfsdk_dualquaternion &dq, const gfsdk_dualquaternion & dq2);
gfsdk_dualquaternion	gfsdk_lerp(const gfsdk_dualquaternion &dq1, const gfsdk_dualquaternion & dq2, float t);
gfsdk_float3			gfsdk_transformCoord(const gfsdk_dualquaternion& dq, const gfsdk_float3 &vecIn);
gfsdk_float3			gfsdk_transformVector(const gfsdk_dualquaternion& dq, const gfsdk_float3 &vecIn);
gfsdk_quaternion		gfsdk_getRotation(const gfsdk_dualquaternion& dq);
gfsdk_float3			gfsdk_getTranslation(const gfsdk_dualquaternion& dq);

#endif 
