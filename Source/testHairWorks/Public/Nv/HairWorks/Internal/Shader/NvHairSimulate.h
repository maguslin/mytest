/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SIMULATE_H
#define NV_HAIR_SIMULATE_H

#ifdef _CPP
#	error "Can only be included in HLSL code"
#endif

#include "NvHairInternalShaderCommon.h"
#include "NvHairInternalShaderTypes.h"
#include "NvHairShaderMath.h"

void NvHair_BlendMaterial(in NvHair_SimulationMaterial defaultMaterial, in NvHair_SimulationMaterial targetMaterial, float weight, out NvHair_SimulationMaterial dst)
{
#define LERP_MATERIAL(PARAM) \
	dst.PARAM	= lerp(defaultMaterial.PARAM, targetMaterial.PARAM, weight);

	LERP_MATERIAL(stiffness);
	LERP_MATERIAL(damping);
	LERP_MATERIAL(stiffnessStrength);
	LERP_MATERIAL(stiffnessDamping);

	LERP_MATERIAL(rootStiffness);
	LERP_MATERIAL(tipStiffness);
	LERP_MATERIAL(bendStiffness);
	LERP_MATERIAL(interactionStiffness);

	LERP_MATERIAL(pinStiffness);
	LERP_MATERIAL(inertiaScale);
	LERP_MATERIAL(backStopRadius);
	LERP_MATERIAL(friction);

	LERP_MATERIAL(massScale);
	LERP_MATERIAL(hairNormalWeight);

	LERP_MATERIAL(stiffnessCurve);
	LERP_MATERIAL(stiffnessStrengthCurve);
	LERP_MATERIAL(stiffnessDampingCurve);
	LERP_MATERIAL(bendStiffnessCurve);
	LERP_MATERIAL(interactionStiffnessCurve);
#undef LERP_MATERIAL

	dst._reserved2_ = 0;
	dst._reserved3_ = 0;
}

float NvHair_ComputeWeight(in NvHair_Pin pin, const float4 restPosition)
{
	float3 restDelta = mul(float4(restPosition.xyz, 1), pin.invPinPoseMatrix).xyz;
	float distance = length(restDelta);
	float ratio = 1.0f - saturate(distance / pin.radius);
	return ratio;
}

float NvHair_ComputeInfluence(in NvHair_Pin pin, const float4 restPosition)
{
	float3 restDelta = mul(float4(restPosition.xyz, 1), pin.invPinPoseMatrix).xyz;
	float distance = length(restDelta);
	float ratio = 1.0f - saturate(distance / pin.radius);
	ratio = pin.influenceFallOff * gfsdk_splineInterpolate(pin.influenceFallOffCurve, ratio);
	return ratio;
}
	
float3 NvHair_ComputeDynamicTarget(in NvHair_Pin pin, const float4 restPosition)
{
	float3 restDelta = mul(float4(restPosition.xyz, 1), pin.invHairPoseMatrix).xyz;
	float3 target = mul(float4(restDelta, 1), pin.currentHairMatrix).xyz;
	return target;
}

float3 NvHair_ComputeStaticTarget(in NvHair_Pin pin, const float4 restPosition)
{
	float3 restDelta = mul(float4(restPosition.xyz, 1), pin.invHairPoseMatrix).xyz;
	float3 target = mul(float4(restDelta, 1), pin.currentPinMatrix).xyz;
	return target;
}

#endif  // NV_HAIR_SIMULATE_H