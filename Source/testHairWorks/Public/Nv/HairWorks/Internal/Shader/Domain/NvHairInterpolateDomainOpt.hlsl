/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifdef OPTIMIZE_RUNTIME

#define USE_PIXEL_DENSITY 		_USE_PIXEL_DENSITY_
#define USE_CULL_SPHERE 		_USE_CULL_SPHERE_

#define USE_CLUMPING			_USE_CLUMPING_
#define USE_WAVINESS			_USE_WAVINESS_
#define USE_WAVE_CLUMP			_USE_WAVE_CLUMP_

////////////////////////////////////////////////////////////////////

#define	USE_DENSITY_TEXTURE			_USE_DENSITY_TEXTURE_

#define	USE_WAVE_SCALE_TEXTURE		_USE_WAVE_SCALE_TEXTURE_
#define	USE_WAVE_FREQ_TEXTURE		_USE_WAVE_FREQ_TEXTURE_

#define	USE_CLUMP_SCALE_TEXTURE			_USE_CLUMP_SCALE_TEXTURE_
#define	USE_CLUMP_ROUNDNESS_TEXTURE		_USE_CLUMP_ROUNDNESS_TEXTURE_

#define	USE_WIDTH_TEXTURE			_USE_WIDTH_TEXTURE_

#define	USE_LENGTH_TEXTURE			_USE_LENGTH_TEXTURE_

#define SAMPLE_DENSITY				_SAMPLE_DENSITY_
#define SAMPLE_LENGTH				_SAMPLE_LENGTH_
#define SAMPLE_WIDTH				_SAMPLE_WIDTH_

#define SAMPLE_WAVE_SCALE			_SAMPLE_WAVE_SCALE_
#define SAMPLE_WAVE_FREQ			_SAMPLE_WAVE_FREQ_
#define SAMPLE_CLUMP_SCALE			_SAMPLE_CLUMP_SCALE_
#define SAMPLE_CLUMP_ROUNDNESS		_SAMPLE_CLUMP_ROUNDNESS_

#endif

#include "NvHairInterpolateDomain.hlsl"

