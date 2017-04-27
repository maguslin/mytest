/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInterpolate.h>

//////////////////////////////////////////////////////////////////////////////
// Dummy vertex shader (we don't need vs since HS and GS emits new geometry from shader resources)
//////////////////////////////////////////////////////////////////////////////
VSOut vs_main()
{
	VSOut dummy;
	dummy.dummy = 0;
	return dummy;
}

