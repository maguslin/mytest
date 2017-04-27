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

//////////////////////////////////////////////////////////////////////////////////////////////
// Pixel shader for interpolated hair rendering
//////////////////////////////////////////////////////////////////////////////////////////////
[earlydepthstencil] float4 ps_main(NvHair_PixelShaderInput input) : SV_Target
{        
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

