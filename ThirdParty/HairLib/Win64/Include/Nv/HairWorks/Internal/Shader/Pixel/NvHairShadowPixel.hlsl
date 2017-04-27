/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

//////////////////////////////////////////////////////////////////////////////
// do your definitions here
//////////////////////////////////////////////////////////////////////////////

#include <Nv/HairWorks/Shader/NvHairShaderCommon.h>

//////////////////////////////////////////////////////////////////////////////
// constant buffer for render parameters
//////////////////////////////////////////////////////////////////////////////
cbuffer cbPerFrame : register(b0)
{
	// .... Add other cbuffer variables to get your own data

	NvHair_ConstantBuffer	g_hairConstantBuffer; // hairworks portion of constant buffer data

	// .... Add other cbuffer variables to get your own data
}

//////////////////////////////////////////////////////////////////////////////////
// Pixel shader for shadow rendering pass
//////////////////////////////////////////////////////////////////////////////////
float ps_main(NvHair_PixelShaderInput input) : SV_Target0
{
	float depth = NvHair_ScreenToView(input.position, g_hairConstantBuffer).z;
	return depth;
}

