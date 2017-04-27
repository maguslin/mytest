/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairSimulate.h>
#include <Nv/HairWorks/Internal/Shader/NvHairShaderMath.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// UAVs
RWStructuredBuffer<NvHair_PinScratchData>	g_globalScratchMem : register(u0); // global scratch mem
RWStructuredBuffer<NvHair_Pin>	g_pinBuffer : register(u1);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_SimulateConstantBuffer g_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_PIN_COM,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{
	// number of particles in this hair
    int n = g_buffer.numTotalCvs; // need total number of cvs

	int numBlocks = n / NV_HAIR_BLOCK_SIZE_PIN_COM + 1;
	int numPins = g_buffer.numPinConstraints;
	int pinId = globalThreadId.x;

	if (pinId < numPins)
	{
		float4 com = float4(0,0,0,0);
		float4 tangentSum = float4(0,0,0,0);

		for (int i = 0; i < numBlocks; i++)
		{
			NvHair_PinScratchData p = g_globalScratchMem[pinId * NV_HAIR_SCRATCH_SIZE_PER_PIN + i];
			com += p.com;
			tangentSum += p.tangent;
		}

		com /= (com.w + FLT_EPSILON);

		float3 tangent = normalize(tangentSum.xyz);

		NvHair_Pin pin = g_pinBuffer[pinId];

		int index = pin.boneIndex;

		if (index >= 0)
		{
			matrix4 bone = g_buffer.boneMatrices[index];

			if (pin.useDynamicPin)
			{
				// copy bone matrix and set translation to COM
				pin.currentHairMatrix = bone;
				gfsdk_setTranslation(pin.currentHairMatrix, com.xyz);

				// compute transfer matrix from skinned tangent to simulated tangent
				float3 restLocalTangent = mul(float4(pin.restTangent,0), pin.invHairPoseMatrix).xyz;
				float3 skinnedTangent = mul(float4(restLocalTangent,0), bone).xyz;
				float4 q = gfsdk_rotateBetween(skinnedTangent, tangent);


				// now hair matrix is aligned with tangent
				pin.currentHairMatrix = gfsdk_changeBasis(q, pin.currentHairMatrix);
				if (pin.doLra) {
					float3 rootBonePos = gfsdk_getTranslation(g_buffer.boneMatrices[pin.rootBoneIndex]);
					float3 pos = gfsdk_getTranslation(pin.currentHairMatrix);
					float len = length(pos-rootBonePos);
					if (len > pin.rootBoneDis) {
						float3 dir = (pos - rootBonePos)/len;
						pos = rootBonePos + dir*pin.rootBoneDis;
						gfsdk_setTranslation(pin.currentHairMatrix,pos);
					}
				}
				//gfsdk_setTranslation(pin.currentHairMatrix,float3(0.0f,0.0f,0.0f));
				// apply offset matrix to properly set display matrix for the pin
				matrix4 offsetMatrix = gfsdk_makeTranslation(-pin.restComShift.xyz);
				pin.currentPinMatrix = mul(pin.currentHairMatrix, offsetMatrix) ;
			}	
			else
			{
				matrix4 offsetMatrix = gfsdk_makeTranslation( pin.localPos);
				pin.currentPinMatrix = mul(offsetMatrix, bone);
			}
		}

		g_pinBuffer[pinId] = pin;
	}
}


