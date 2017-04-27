/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains wrapper functions to make hair lib easy to setup and use
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "GfsdkMathUtil.h"

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeFloat4x4(gfsdk_float4x4& m, const float* p)
{
	memcpy(&m, p, sizeof(float) * 16);
}

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeIdentity(gfsdk_float4x4& transform)
{
	memset(&transform, 0, sizeof(gfsdk_float4x4));
	transform._11 = 1.0f;
	transform._22 = 1.0f;
	transform._33 = 1.0f;
	transform._44 = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeDiagonal(gfsdk_float4x4& transform, const gfsdk_float3& d)
{
	memset(&transform, 0, sizeof(gfsdk_float4x4));
	transform._11 = d.x;
	transform._22 = d.y;
	transform._33 = d.z;
	transform._44 = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeTranslation(gfsdk_float4x4& m, const gfsdk_float3& t)
{
	gfsdk_makeIdentity(m);
	m._41 = t.x;
	m._42 = t.y;
	m._43 = t.z;
}

////////////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeScale(gfsdk_float4x4& m, const gfsdk_float3& s)
{
	gfsdk_makeIdentity(m);
	m._11 = s.x;
	m._22 = s.y;
	m._33 = s.z;
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeRotation(gfsdk_float4x4& m, const gfsdk_float4& q)
{
	gfsdk_makeIdentity(m);

	const float x = q.x;
	const float y = q.y;
	const float z = q.z;
	const float w = q.w;

	const float x2 = x + x;
	const float y2 = y + y;
	const float z2 = z + z;

	const float xx = x2*x;
	const float yy = y2*y;
	const float zz = z2*z;

	const float xy = x2*y;
	const float xz = x2*z;
	const float xw = x2*w;

	const float yz = y2*z;
	const float yw = y2*w;
	const float zw = z2*w;

	m._11 = 1.0f - yy - zz;
	m._12 = xy + zw;
	m._13 = xz - yw;

	m._21 = xy - zw;
	m._22 = 1.0f - xx - zz;
	m._23 = yz + xw;

	m._31 = xz + yw;
	m._32 = yz - xw;
	m._33 = 1.0f - xx - yy;
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeRotation(gfsdk_float4x4&m, const gfsdk_float3& xaxis, const gfsdk_float3& yaxis, const gfsdk_float3& zaxis)
{
	gfsdk_makeIdentity(m);

	m._11 = xaxis.x;  m._12 = xaxis.y; m._13 = xaxis.z;
	m._21 = yaxis.x;  m._22 = yaxis.y; m._23 = yaxis.z;
	m._31 = zaxis.x;  m._32 = zaxis.y; m._33 = zaxis.z;
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeRotation(gfsdk_float4x4& R, const gfsdk_float3& from, const gfsdk_float3& to)
{
	gfsdk_makeIdentity(R);

	// Early exit if to = from
	if( gfsdk_lengthSquared(from - to) < 1e-4f )
	{
		return;
	}
		
	// Early exit if to = -from
	if( gfsdk_lengthSquared(from + to) < 1e-4f )
	{
		gfsdk_makeDiagonal(R, gfsdk_makeFloat3(1.0f, -1.0f, -1.0f));
		return;
	}

	gfsdk_float3 n = gfsdk_cross(from, to);

	float C = gfsdk_dot(from, to);
	float S = gfsdk_sqrt(1.0f - C * C);
	float CC = 1.0f - C;

	float xx = n.x * n.x;
	float yy = n.y * n.y;
	float zz = n.z * n.z;
	float xy = n.x * n.y;
	float yz = n.y * n.z;
	float xz = n.x * n.z;
	 
	R._11 =  1 + CC * (xx - 1);
	R._21 = -n.z * S + CC * xy;
	R._31 =  n.y * S + CC * xz;

	R._12 =  n.z * S + CC * xy;
	R._22 =  1 + CC * (yy - 1);
	R._32 = -n.x * S + CC * yz;

	R._13 = -n.y * S + CC * xz;
	R._23 =  n.x * S + CC * yz;
	R._33 =  1 + CC * (zz - 1);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 gfsdk_makeTransform(const gfsdk_float4& q, const gfsdk_float3& t, const gfsdk_float3 &s)
{
	gfsdk_float4x4 m;

	gfsdk_makeRotation(m, q);

	m._11 *= s.x; m._12 *= s.x; m._13 *= s.x;
	m._21 *= s.y; m._22 *= s.y; m._23 *= s.y;
	m._31 *= s.z; m._32 *= s.z; m._33 *= s.z;

	m._41 = t.x; 
	m._42 = t.y;
	m._43 = t.z;

	return m;
}

////////////////////////////////////////////////////////////////////////////////
// inverse of a projection matrix
bool gfsdk_inverseProjection(gfsdk_float4x4& out, const gfsdk_float4x4& in)
{
	gfsdk_makeIdentity(out);

	// perspective projection requires separate treatment
	// | A 0 0 0 |
	// | 0 B 0 0 |
	// | 0 0 C D |
	// | 0 0 E F |

	// RH: A = xScale, B = yScale, C = zf/(zn-zf),  D = -1, E = zn*zf/(zn-zf)
	// LH: A = xScale, B = yScale, C = -zf/(zn-zf), D = 1,  E = zn*zf/(zn-zf)

	float A = in._11;
	float B = in._22;
	float C = in._33;
	float D = in._34;
	float E = in._43;
	float F = in._44;

	if (F != 0.0f) // non-perspective, non-degenerate
	{
		out = gfsdk_inverse(in);
		return true;
	}

	if ( (D == 0.0f) || (E == 0.0f)) // can't be zero in normal perspective projection matrix;
		return false;

	/* UE4 test matrix
	gfsdk_float4x4 in;
	in._11 = 1.0f, in._12 = 0.0f,			in._13 = 0.0f,	in._14 = 0.0f;
	in._21 = 0.0f, in._22 = 1.12275445f,	in._23 = 0.0f,	in._24 = 0.0f;
	in._31 = 0.0f, in._32 = 0.0f,			in._33 = 0.0f,	in._34 = 1.0f;
	in._41 = 0.0f, in._42 = 0.0f,			in._43 = 10.0f, in._44 = 0.0f;
	*/
	
	// x' = Ax, y' = By, z' = Cz + Ew, w' = D * z
	// x = x' / A
	// y = y' / B, 
	// z = w' / D
	// w = (z' - Cz) / E = (z' - C * (w' / D) ) / E
	//  = (1/E) * z' - C / (D * E) * w'
	
	// Inverse = 
	// | 1/A 0		0		0		|
	// | 0   1/B	0		0		|
	// | 0   0		0		1/E		|
	// | 0   0		1/D		-C/(D*E)|

	out._11 = 1.0f / A;		out._12 = 0.0f;			out._13 = 0.0f;		out._14 = 0.0f;
	out._21 = 0.0f;			out._22 = 1.0f / B;		out._23 = 0.0f;		out._24 = 0.0f;
	out._31 = 0.0f;			out._32 = 0.0f;			out._33 = 0.0f;		out._34 = 1.0f / E;
	out._41 = 0.0f;			out._42 = 0.0f;			out._43 = 1.0f / D; out._44 = -1.0f * C / (D * E);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_rotate(const gfsdk_quaternion &q, const gfsdk_float3 &v)
{
	const float vx = 2.0f * v.x;
	const float vy = 2.0f * v.y;
	const float vz = 2.0f * v.z;
	const float w2 = q.w * q.w - 0.5f;
	const float dot2 = (q.x * vx + q.y * vy + q.z * vz);

	gfsdk_float3 ret;

	ret.x = vx * w2 + (q.y * vz - q.z * vy) * q.w + q.x * dot2;
	ret.y = vy * w2 + (q.z * vx - q.x * vz) * q.w + q.y * dot2;
	ret.z = vz * w2 + (q.x * vy - q.y * vx) * q.w + q.z * dot2;

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_rotateInv(const gfsdk_quaternion &q, const gfsdk_float3 &v)
{
	const float vx = 2.0f * v.x;
	const float vy = 2.0f * v.y;
	const float vz = 2.0f * v.z;
	const float w2 = q.w * q.w - 0.5f;
	const float dot2 = (q.x * vx + q.y * vy + q.z * vz);

	gfsdk_float3 ret;

	ret.x = vx * w2 - (q.y * vz - q.z * vy) * q.w + q.x * dot2;
	ret.y = vy * w2 - (q.z * vx - q.x * vz) * q.w + q.y * dot2;
	ret.z = vz * w2 - (q.x * vy - q.y * vx) * q.w + q.z * dot2;

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getBasisX(const gfsdk_quaternion &q)
{
	const float x2 = q.x * 2.0f;
	const float w2 = q.w * 2.0f;
	return gfsdk_makeFloat3(
		(q.w * w2) - 1.0f + q.x * x2,
		(q.z * w2)        + q.y * x2,
		(-q.y * w2)       + q.z * x2);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getBasisY(const gfsdk_quaternion &q)
{
	const float y2 = q.y * 2.0f;
	const float w2 = q.w * 2.0f;
	return gfsdk_makeFloat3(
		(-q.z * w2)			+ q.x * y2,
		( q.w * w2) - 1.0f	+ q.y * y2,
		( q.x * w2)			+ q.z * y2);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getBasisZ(const gfsdk_quaternion &q)
{
	const float z2 = q.z * 2.0f;
	const float w2 = q.w * 2.0f;
	
	return gfsdk_makeFloat3(	
		( q.y * w2)			+ q.x * z2,
		(-q.x * w2)			+ q.y * z2,
		( q.w * w2) - 1.0f	+ q.z * z2);
}

///////////////////////////////////////////////////////////////////////////////
gfsdk_float4 gfsdk_quaternionConjugate(const gfsdk_quaternion& in)
{
	return gfsdk_makeFloat4(-in.x, -in.y, -in.z, in.w);
}

///////////////////////////////////////////////////////////////////////////////
void gfsdk_makeIdentity(gfsdk_quaternion& q)
{
	q = gfsdk_makeFloat4(0, 0, 0, 1);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_quaternion	gfsdk_makeRotation(const gfsdk_float3 &axis, float radian)
{
	float a = radian * 0.5f;
	float s = sinf(a);
	
	gfsdk_quaternion q;
	q.w = cosf(a);
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;

	return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
gfsdk_quaternion gfsdk_quatFromAxis(const gfsdk_float3 n, const gfsdk_float3 t)
{
	gfsdk_quaternion q;

	gfsdk_float3 az = n;

	gfsdk_float3 ay = gfsdk_getNormalized(gfsdk_cross(az, t));
	gfsdk_float3 ax = gfsdk_getNormalized(gfsdk_cross(ay,az));

	gfsdk_makeRotation(q, ax, ay, az);

	return q;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_quaternion	gfsdk_rotateBetween(const gfsdk_float3 from, const gfsdk_float3 to)
{
	gfsdk_float3 axis = gfsdk_cross(from, to);

	float axisLength = gfsdk_length(axis);
	if (axisLength < 1e-2)
		return gfsdk_makeFloat4(0,0,0,1);

	axis = (1.0f / axisLength) * axis;

	float dotT = gfsdk_clamp(gfsdk_dot(from, to), -1.0f, 1.0f);
	float angle = acosf( dotT );

	return gfsdk_makeRotation(axis, angle);
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeRotation(gfsdk_quaternion&q, const gfsdk_float3& ax, const gfsdk_float3& ay, const gfsdk_float3& az)
{
	float tr = ax.x + ay.y + az.z;
	float h;
	if(tr >= 0)
	{
		h = sqrtf(tr +1);
		q.w = float(0.5) * h;
		h = float(0.5) / h;

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
			h = gfsdk_sqrt((ax.x - (ay.y + az.z)) + 1);
			q.x = float(0.5) * h;
			h = float(0.5) / h;

			q.y = (ay.x + ax.y) * h; 
			q.z = (ax.z + az.x) * h;
			q.w = (ay.z - az.y) * h;
			break;
		case 1:
			h = gfsdk_sqrt((ay.y - (az.z + ax.x)) + 1);
			q.y = float(0.5) * h;
			h = float(0.5) / h;

			q.z = (az.y + ay.z) * h;
			q.x = (ay.x + ax.y) * h;
			q.w = (az.x - ax.z) * h;
			break;
		case 2:
			h = gfsdk_sqrt((az.z - (ax.x + ay.y)) + 1);
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

////////////////////////////////////////////////////////////////////////////////
gfsdk_quaternion gfsdk_quaternionMultiply(const gfsdk_quaternion& q0, const gfsdk_quaternion& q1)
{
	gfsdk_quaternion q;

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
gfsdk_quaternion gfsdk_slerp(const gfsdk_quaternion& q0, const gfsdk_quaternion& q1, float t)
{
	const float quatEpsilon = (float(1.0e-8f));

	float cosine = gfsdk_dot(q0, q1);
	float sign = float(1);
	if (cosine < 0)
	{
		cosine = -cosine;
		sign = float(-1);
	}

	float sine = float(1) - cosine*cosine;

	if(sine>=quatEpsilon*quatEpsilon)	
	{
		sine = gfsdk_sqrt(sine);
		const float angle = atan2f(sine, cosine);
		const float i_sin_angle = float(1) / sine;

		const float leftw	= sinf(angle * (float(1)-t)) * i_sin_angle;
		const float rightw	= sinf(angle * t) * i_sin_angle * sign;

		return q0 * leftw + q1 * rightw;
	}

	return q0;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float4 gfsdk_getRotation(const gfsdk_float4x4& sm)
{
	gfsdk_float4x4 m = sm;

	gfsdk_orthonormalize(m);

	float x,y,z,w;

	float tr = m._11 + m._22 + m._33, h;
	if(tr >= 0)
	{
		h = gfsdk_sqrt(tr +1);
		w = 0.5f * h;
		h = 0.5f / h;

		x = (m._23 - m._32) * h;
		y = (m._31 - m._13) * h;
		z = (m._12 - m._21) * h;
	}
	else
	{
		float max = m._11;
		int i = 0; 
		if (m._22 > m._11)
		{
			i = 1; 
			max = m._22;
		}
		if (m._33 > max)
			i = 2; 
		switch (i)
		{
		case 0:
			h = gfsdk_sqrt((m._11 - (m._22 + m._33)) + 1);
			x = 0.5f * h;
			h = 0.5f / h;

			y = (m._21 + m._12) * h; 
			z = (m._13 + m._31) * h;
			w = (m._23 - m._32) * h;
			break;
		case 1:
			h = gfsdk_sqrt((m._22 - (m._33 + m._11)) + 1);
			y = 0.5f * h;
			h = 0.5f / h;

			z = (m._32 + m._23) * h;
			x = (m._21 + m._12) * h;
			w = (m._31 - m._13) * h;
			break;
		case 2:
			h = gfsdk_sqrt((m._33 - (m._11 + m._22)) + 1);
			z = 0.5f * h;
			h = 0.5f / h;

			x = (m._13 + m._31) * h;
			y = (m._32 + m._23) * h;
			w = (m._12 - m._21) * h;
			break;
		default: // Make compiler happy
			x = y = z = w = 0.0f;
			break;
		}
	}	

	return gfsdk_makeFloat4(x,y,z,w);
}

////////////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 gfsdk_lerp(gfsdk_float4x4& start, gfsdk_float4x4& end, float t)
{
	gfsdk_float4 sq = gfsdk_getRotation(start);
	gfsdk_float4 eq = gfsdk_getRotation(end);
	gfsdk_float3 st = gfsdk_getTranslation(start);
	gfsdk_float3 et = gfsdk_getTranslation(end);

	gfsdk_float3 ss = gfsdk_getScale(start);
	gfsdk_float3 es = gfsdk_getScale(end);
	gfsdk_float3 s = gfsdk_lerp(ss, es, t);

	gfsdk_dualquaternion sdq = gfsdk_makeDQ(sq, st);
	gfsdk_dualquaternion edq = gfsdk_makeDQ(eq, et);

	gfsdk_dualquaternion dq = gfsdk_lerp(sdq, edq, t);

	gfsdk_float4 gr = gfsdk_getRotation(dq);
	gfsdk_float3 gt = gfsdk_getTranslation(dq);

	return gfsdk_makeTransform(gr, gt, s);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 operator*(const gfsdk_float4x4& in1, const gfsdk_float4x4& in2)
{
#define MATRIX_SUM(OUT, IN1, IN2, ROW, COL) OUT._##ROW##COL = IN1._##ROW##1 * IN2._1##COL + IN1._##ROW##2 * IN2._2##COL + IN1._##ROW##3 * IN2._3##COL + IN1._##ROW##4 * IN2._4##COL;

	gfsdk_float4x4 out;

	MATRIX_SUM(out, in1, in2, 1, 1); 
	MATRIX_SUM(out, in1, in2, 1, 2);
	MATRIX_SUM(out, in1, in2, 1, 3);
	MATRIX_SUM(out, in1, in2, 1, 4);

	MATRIX_SUM(out, in1, in2, 2, 1); 
	MATRIX_SUM(out, in1, in2, 2, 2);
	MATRIX_SUM(out, in1, in2, 2, 3);
	MATRIX_SUM(out, in1, in2, 2, 4);

	MATRIX_SUM(out, in1, in2, 3, 1); 
	MATRIX_SUM(out, in1, in2, 3, 2);
	MATRIX_SUM(out, in1, in2, 3, 3);
	MATRIX_SUM(out, in1, in2, 3, 4);

	MATRIX_SUM(out, in1, in2, 4, 1); 
	MATRIX_SUM(out, in1, in2, 4, 2);
	MATRIX_SUM(out, in1, in2, 4, 3);
	MATRIX_SUM(out, in1, in2, 4, 4);

#undef MATRIX_SUM

	return out;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getScale(const gfsdk_float4x4& m)
{
	gfsdk_float3 ax = gfsdk_makeFloat3(m._11, m._12, m._13);
	gfsdk_float3 ay = gfsdk_makeFloat3(m._21, m._22, m._23);
	gfsdk_float3 az = gfsdk_makeFloat3(m._31, m._32, m._33);

	return gfsdk_makeFloat3(gfsdk_length(ax), gfsdk_length(ay), gfsdk_length(az));
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getTranslation(const gfsdk_float4x4& m)
{
	return gfsdk_makeFloat3(m._41, m._42, m._43);
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_setTranslation(gfsdk_float4x4& m, const gfsdk_float3 &v)
{
	m._41 = v.x;
	m._42 = v.y;
	m._43 = v.z;
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_orthonormalize(gfsdk_float4x4& m)
{
	gfsdk_float3 ax = gfsdk_getNormalized(gfsdk_makeFloat3(m._11, m._12, m._13));
	gfsdk_float3 ay = gfsdk_getNormalized(gfsdk_makeFloat3(m._21, m._22, m._23));
	gfsdk_float3 az = gfsdk_getNormalized(gfsdk_makeFloat3(m._31, m._32, m._33));

	m._11 = ax.x; m._12 = ax.y; m._13 = ax.z;
	m._21 = ay.x; m._22 = ay.y; m._23 = ay.z;
	m._31 = az.x; m._32 = az.y; m._33 = az.z;

}

////////////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4& operator*=(gfsdk_float4x4& m, float s)
{
	float* data = (float*)&m;
	for (int i = 0; i < 16; i++)
		data[i] *= s;

	return m;
}

////////////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4& operator+=(gfsdk_float4x4& m1, const gfsdk_float4x4& m2)
{
	float* data1 = (float*)&m1;
	float* data2 = (float*)&m2;

	for (int i = 0; i < 16; i++)
		data1[i] += data2[i];

	return m1;
}

////////////////////////////////////////////////////////////////////////////////
float 
gfsdk_getDeterminant(const gfsdk_float4x4& m)
{
	const float* matrix = (const float*)&m;
	
	gfsdk_float3 p0 = gfsdk_makeFloat3(matrix[0*4+0], matrix[0*4+1], matrix[0*4+2]);
	gfsdk_float3 p1 = gfsdk_makeFloat3(matrix[1*4+0], matrix[1*4+1], matrix[1*4+2]);
	gfsdk_float3 p2 = gfsdk_makeFloat3(matrix[2*4+0], matrix[2*4+1], matrix[2*4+2]);

	gfsdk_float3 tempv = gfsdk_cross(p1,p2);

	return gfsdk_dot(p0, tempv);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_transformCoord(const gfsdk_float4x4 &m, gfsdk_float3 op)
{
	gfsdk_float3 p;
	
	p.x = op.x * m._11 + op.y * m._21 + op.z * m._31 + m._41;
	p.y = op.x * m._12 + op.y * m._22 + op.z * m._32 + m._42;
	p.z = op.x * m._13 + op.y * m._23 + op.z * m._33 + m._43;

	return p;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_transformVector(const gfsdk_float4x4 &m, gfsdk_float3 op)
{
	gfsdk_float3 p;
	
	p.x = op.x * m._11 + op.y * m._21 + op.z * m._31;
	p.y = op.x * m._12 + op.y * m._22 + op.z * m._32;
	p.z = op.x * m._13 + op.y * m._23 + op.z * m._33;

	return p;
}


////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4
gfsdk_getSubMatrix(int ki,int kj, const gfsdk_float4x4& m)
{
	gfsdk_float4x4 out;
	gfsdk_makeIdentity(out);

	float* pDst = (float*)&out;
	const float* matrix = (const float*)&m;

	int row, col;
	int dstCol = 0, dstRow = 0;

	for ( col = 0; col < 4; col++ )
	{
		if ( col == kj )
		{
			continue;
		}
		for ( dstRow = 0, row = 0; row < 4; row++ )
		{
			if ( row == ki )
			{
				continue;
			}
			pDst[dstCol*4+dstRow] = matrix[col*4+row];
			dstRow++;
		}
		dstCol++;
	}

	return out;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 gfsdk_inverse(const gfsdk_float4x4& m)
{
	gfsdk_float4x4 im;

	float* inverse_matrix = (float*)&im;

	float det = gfsdk_getDeterminant(m);
	det = 1.0f / det;
	for (int i = 0; i < 4; i++ )
	{
		for (int j = 0; j < 4; j++ )
		{
			int sign = 1 - ( ( i + j ) % 2 ) * 2;

			gfsdk_float4x4 subMat = gfsdk_getSubMatrix(i, j, m);
			float subDeterminant = gfsdk_getDeterminant(subMat);
	
			inverse_matrix[i*4+j] = ( subDeterminant * sign ) * det;
		}
	}

	return im;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion gfsdk_makeDQ(const gfsdk_quaternion& q, const gfsdk_float3& t)
{
	gfsdk_dualquaternion dq;

	dq.q0 = gfsdk_getNormalized(q);
	dq.q1 = gfsdk_quaternionMultiply(gfsdk_makeFloat4(t, 0), dq.q0) * 0.5f;

	return dq;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion gfsdk_getNormalized(const gfsdk_dualquaternion & dq)
{
	float mag = gfsdk_dot( dq.q0, dq.q0);
	float deLen = 1.0f / gfsdk_sqrt(mag+FLT_EPSILON);
	gfsdk_dualquaternion dqOut;
	dqOut.q0 = dq.q0 * deLen;
	dqOut.q1 = dq.q1 * deLen;
	return dqOut;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion operator*(float s, const gfsdk_dualquaternion & dq)
{
	return gfsdk_dualquaternion(s * dq.q0, s * dq.q1);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion operator*(const gfsdk_dualquaternion & dq, float s)
{
	return gfsdk_dualquaternion(s * dq.q0, s * dq.q1);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion& operator+=(gfsdk_dualquaternion &dq, const gfsdk_dualquaternion & dq2)
{
	// hemispherization
	float sign = (gfsdk_dot(dq.q0, dq2.q0) < -FLT_EPSILON) ? -1.0f: 1.0f;

	dq.q0 += sign * dq2.q0;
	dq.q1 += sign * dq2.q1;

	return dq;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion gfsdk_lerp(const gfsdk_dualquaternion &dq1, const gfsdk_dualquaternion & dq2, float t)
{
	gfsdk_dualquaternion dq = dq1 * (1.0f - t);
	float sign = (gfsdk_dot(dq1.q0, dq2.q0) < -FLT_EPSILON) ? -1.0f: 1.0f;
	dq += (t * sign) * dq2;
	return gfsdk_getNormalized(dq);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_transformCoord(const gfsdk_dualquaternion& dq, const gfsdk_float3 &vecIn) 
{
	gfsdk_float3 d0 = gfsdk_makeFloat3(dq.q0.x, dq.q0.y, dq.q0.z);
	gfsdk_float3 de = gfsdk_makeFloat3(dq.q1.x, dq.q1.y, dq.q1.z);
	float a0 = dq.q0.w;
	float ae = dq.q1.w;

	gfsdk_float3 temp = gfsdk_cross( d0, vecIn ) + a0 * vecIn;
	gfsdk_float3 temp2 = 2.0f * (a0 * de - ae * d0 + gfsdk_cross(d0, de));

	return vecIn + temp2 + 2.0f * gfsdk_cross( d0, temp);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_transformVector(const gfsdk_dualquaternion& dq, const gfsdk_float3 &vecIn) 
{
	gfsdk_float3 d0 = gfsdk_makeFloat3(dq.q0.x, dq.q0.y, dq.q0.z);
	float a0 = dq.q0.w;

	gfsdk_float3 temp = gfsdk_cross( d0, vecIn ) + a0 * vecIn;
	return vecIn + 2.0f * gfsdk_cross( d0, temp);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_quaternion gfsdk_getRotation(const gfsdk_dualquaternion& dq)
{
	return dq.q0;
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getTranslation(const gfsdk_dualquaternion& dq)
{
	gfsdk_float4 dual = 2.0f * dq.q1;
	gfsdk_float4 t = gfsdk_quaternionMultiply(dual, gfsdk_quaternionConjugate( dq.q0 ));

	return gfsdk_makeFloat3(t.x, t.y, t.z);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_dualquaternion gfsdk_makeDQ(const gfsdk_float4x4& m)
{
	gfsdk_float4 q = gfsdk_getRotation(m);
	gfsdk_float3 t = gfsdk_getTranslation(m);

	return gfsdk_makeDQ(q, t);
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeIdentity(gfsdk_dualquaternion& dq)
{
	dq.q0 = gfsdk_makeFloat4(0.0f, 0.0f, 0.0f, 1.0f);
	dq.q1 = gfsdk_makeFloat4(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_makeZero(gfsdk_dualquaternion& dq)
{
	dq.q0 = gfsdk_makeFloat4(0.0f, 0.0f, 0.0f, 0.0f);
	dq.q1 = gfsdk_makeFloat4(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4	gfsdk_makeTransform(const gfsdk_dualquaternion& dq)
{
	gfsdk_float3 t = gfsdk_getTranslation(dq);
	gfsdk_float3 s = gfsdk_makeFloat3(1.0f, 1.0f, 1.0f);
	return gfsdk_makeTransform(dq.q0, t, s);
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getColumn(const gfsdk_float4x4&m, int col)
{
	float* base = (float*)(&m);
	return gfsdk_makeFloat3( base[col], base[ 4 + col], base [ 8 + col] );
}

////////////////////////////////////////////////////////////////////////////////
gfsdk_float3 gfsdk_getRow(const gfsdk_float4x4&m, int row)
{
	float* base = (float*)(&m) + row * 4;
	return gfsdk_makeFloat3( base[0], base[1], base[2] );
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_setColumn(const gfsdk_float4x4&m, int col, const gfsdk_float3& v)
{
	float* base = (float*)(&m);
	base[0 + col] = v.x;
	base[4 + col] = v.y;
	base[8 + col] = v.z;

}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_setRow(const gfsdk_float4x4&m, int row, const gfsdk_float3& v)
{
	float* base = (float*)(&m) + row * 4;
	base[0] = v.x;
	base[1] = v.y;
	base[2] = v.z;
}

////////////////////////////////////////////////////////////////////////////////
float& gfsdk_ref(gfsdk_float4x4& m, int row, int col)
{
	float* base = (float*)(&m) + row * 4;
	return base[col];
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_jacobiRotate(gfsdk_float4x4 &A, gfsdk_float4x4 &R, int p, int q)
{
	float& Apq = gfsdk_ref(A, p, q);
	float& Aqp = gfsdk_ref(A, q, p);
	float& App = gfsdk_ref(A, p, p);
	float& Aqq = gfsdk_ref(A, q, q);

	// rotates A through phi in pq-plane to set A(p,q) = 0
	// rotation stored in R whose columns are eigenvectors of A
	if (Apq == 0.0f)
		return;

	float d = (App - Aqq) / (2.0f * Apq);
	float t = 1.0f / (fabsf(d) + gfsdk_sqrt(d*d + 1.0f));
	if (d < 0.0f) 
		t = -t;

	float c = 1.0f / gfsdk_sqrt(t*t + 1);
	float s = t*c;

	App += t * Apq;
	Aqq -= t * Apq;
	Apq = Aqp = 0.0f;

	// transform A
	int k;
	for (k = 0; k < 3; k++) {
		if (k != p && k != q) {

			float &Akp = gfsdk_ref(A, k, p);
			float &Akq = gfsdk_ref(A, k, q);
			float &Apk = gfsdk_ref(A, p, k);
			float &Aqk = gfsdk_ref(A, q, k);

			float R1 = c * Akp + s * Akq;
			float R2 =-s * Akp + c * Akq;
			Akp = Apk = R1;
			Akq = Aqk = R2;
		}
	}

	// store rotation in R
	for (k = 0; k < 3; k++) {
		float& Rkp = gfsdk_ref(R, k, p);
		float& Rkq = gfsdk_ref(R, k, q);

		float R1 =  c * Rkp + s * Rkq;
		float R2 = -s * Rkp + c * Rkq;
		Rkp = R1;
		Rkq = R2;
	}
}

////////////////////////////////////////////////////////////////////////////////
void gfsdk_eigenDecomposition(gfsdk_float4x4 &A, gfsdk_float4x4 &R)
{
	const int numJacobiIterations = 10;
	const float epsilon = 1e-15f;

	// only for symmetric matrices!
	gfsdk_makeIdentity(R);

	int iter = 0;
	while (iter < numJacobiIterations) {	// 3 off diagonal elements
		// find off diagonal element with maximum modulus
		
		float max = fabsf(gfsdk_ref(A, 0, 1));
		int p = 0; 
		int q = 1;

		float a = fabsf(gfsdk_ref(A, 0, 2));
		if (a > max) { 
			p = 0; q = 2; max = a; 
		}
		
		a = fabsf(gfsdk_ref(A, 1, 2));
		if (a > max) { 
			p = 1; q = 2; max = a; 
		}

		// all small enough -> done
		if (max < epsilon) 
			break;

		// rotate matrix with respect to that element
		gfsdk_jacobiRotate(A, R, p,q);
		iter++;
	}
}

////////////////////////////////////////////////////////////////////////////////
void				
gfsdk_polarDecomposition(const gfsdk_float4x4 &A, gfsdk_float4x4 &R)
{
	// A = SR, where S is symmetric and R is orthonormal
	// -> S = (A A^T)^(1/2)

	gfsdk_float4x4 AAT;

	AAT._11 = A._11 * A._11 + A._12 * A._12 + A._13 * A._13;
	AAT._22 = A._21 * A._21 + A._22 * A._22 + A._23 * A._23;
	AAT._33 = A._31 * A._31 + A._32 * A._32 + A._33 * A._33;

	AAT._12 = A._11 * A._21 + A._12 * A._22 + A._13 * A._23;
	AAT._13 = A._11 * A._31 + A._12 * A._32 + A._13 * A._33;
	AAT._23 = A._21 * A._31 + A._22 * A._32 + A._23 * A._33;

	AAT._21 = AAT._12;
	AAT._31 = AAT._13;
	AAT._32 = AAT._23;

	gfsdk_float4x4 U;

	gfsdk_makeIdentity(R);

	gfsdk_eigenDecomposition(AAT, U);

	const float eps = 1e-15f;

	float l0 = AAT._11; if (l0 <= eps) l0 = 0.0f; else l0 = 1.0f / gfsdk_sqrt(l0);
	float l1 = AAT._22; if (l1 <= eps) l1 = 0.0f; else l1 = 1.0f / gfsdk_sqrt(l1);
	float l2 = AAT._33; if (l2 <= eps) l2 = 0.0f; else l2 = 1.0f / gfsdk_sqrt(l2);

	gfsdk_float4x4 S1;

	S1._11 = l0 * U._11 * U._11 + l1 * U._12 * U._12 + l2 * U._13 * U._13;
	S1._22 = l0 * U._21 * U._21 + l1 * U._22 * U._22 + l2 * U._23 * U._23;
	S1._33 = l0 * U._31 * U._31 + l1 * U._32 * U._32 + l2 * U._33 * U._33;

	S1._12 = l0 * U._11 * U._21 + l1 * U._12 * U._22 + l2 * U._13 * U._23;
	S1._13 = l0 * U._11 * U._31 + l1 * U._12 * U._32 + l2 * U._13 * U._33;
	S1._23 = l0 * U._21 * U._31 + l1 * U._22 * U._32 + l2 * U._23 * U._33;

	S1._21 = S1._12;
	S1._31 = S1._13;
	S1._32 = S1._23;

	R = S1 * A;

	// stabilize
	gfsdk_float3 c0 = gfsdk_getColumn(R, 0);
	gfsdk_float3 c1 = gfsdk_getColumn(R, 1);
	gfsdk_float3 c2 = gfsdk_getColumn(R, 2);

	if (gfsdk_lengthSquared(c0) < eps)
		c0 = gfsdk_cross(c1,c2);
	else if (gfsdk_lengthSquared(c1) < eps)
		c1 = gfsdk_cross(c2,c0);
	else 
		c2 = gfsdk_cross(c0,c1);

	gfsdk_setColumn(R, 0, c0);
	gfsdk_setColumn(R, 1, c1);
	gfsdk_setColumn(R, 2, c2);
}

///////////////////////////////////////////////////////////////////////////////
bool gfsdk_isLeftHanded(const gfsdk_float4x4 &m)
{
	gfsdk_float3 x = gfsdk_getColumn(m, 0);
	gfsdk_float3 y = gfsdk_getColumn(m, 1);
	gfsdk_float3 z = gfsdk_getColumn(m, 2);

	gfsdk_float3 xcrossy = gfsdk_cross(x,y);

	float dot = gfsdk_dot(xcrossy, z);

	return dot < 0;
}

///////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 gfsdk_makeOrthoLH(float orthoW, float orthoH, float zNear, float zFar)
{
	gfsdk_float4x4 out;
	gfsdk_makeIdentity(out);

	out._11 = 2.0f / orthoW;
	out._22 = 2.0f / orthoH;
	out._33 = 1.0f / (zFar - zNear);
	out._43 = zNear / (zNear - zFar);

	return out;
}

///////////////////////////////////////////////////////////////////////////////
gfsdk_float4x4 gfsdk_makeOrthoRH(float orthoW, float orthoH, float zNear, float zFar)
{
	gfsdk_float4x4 out;
	gfsdk_makeIdentity(out);

	out._11 = 2.0f / orthoW;
	out._22 = 2.0f / orthoH;
	out._33 = 1.0f / (zNear - zFar);
	out._43 = zNear / (zNear - zFar);

	return out;
}

bool gfsdk_equal(const gfsdk_float4x4& inA, const gfsdk_float4x4& inB)
{
	if (&inA == &inB)
	{
		return true;
	}
	const int size = int(sizeof(gfsdk_float4x4) / sizeof(float));
	const float* a = &inA._11;
	const float* b = &inB._11;
	for (int i = 0; i < size; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}
