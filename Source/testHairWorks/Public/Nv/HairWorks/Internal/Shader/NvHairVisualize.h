/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_VISUALIZE_H
#define NV_HAIR_VISUALIZE_H

#ifdef _CPP
#	error "Can only be included in HLSL code"
#endif

#include "NvHairInternalShaderCommon.h"
#include "NvHairInternalShaderTypes.h"

bool NvHair_CheckHairToShow(in NvHair_VisualizeConstantBuffer cb, int hairId)
{
	if (hairId < cb.hairMin)
		return false;
	if (hairId > cb.hairMax)
		return false;
	if ((cb.hairSkip > 0) && ((hairId % cb.hairSkip) != 0))
		return false;

	return true;
}

cbuffer cbPerFrame : register(b0)
{
	NvHair_VisualizeConstantBuffer g_buffer;
};

#endif // NV_HAIR_VISUALIZE_H
