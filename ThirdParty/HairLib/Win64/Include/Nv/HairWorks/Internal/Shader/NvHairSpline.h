/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SPLINE_H
#define NV_HAIR_SPLINE_H

#ifdef _CPP
#	error "Can only be included in HLSL code"
#endif

#include "NvHairInternalShaderCommon.h"
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

struct VSOut
{
    uint	vertexId : vertexID;
};

#endif // NV_HAIR_SPLINE_H