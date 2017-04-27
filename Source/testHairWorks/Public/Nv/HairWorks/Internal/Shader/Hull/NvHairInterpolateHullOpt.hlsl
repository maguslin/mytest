/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifdef OPTIMIZE_RUNTIME

#define	USE_DENSITY_TEXTURE			_USE_DENSITY_TEXTURE_
#define USE_PIXEL_DENSITY			_USE_PIXEL_DENSITY_

#define SAMPLE_DENSITY				_SAMPLE_DENSITY_

#endif

#include "NvHairInterpolateHull.hlsl"
