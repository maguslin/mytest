///////////////////////////////////////////////////////////////////////////////////////////////
// This file was generated by <HairWorks>/external/shader_preprocess/bin/shader_preprocess.exe 
// Source file:			NvHairInterpolateHullOpt.hlsl
// Include path:		..\..\..\Nv\HairWorks\Internal\Shader\Hull\
// Output file:			..\..\..\Nv\HairWorks\Internal\Shader\Generated\NvHairInterpolateHullOpt.cpp
// Generated at 2017-04-16.23:28:08
///////////////////////////////////////////////////////////////////////////////////////////////
const char* shaderVar =
"#define OPTIMIZE_RUNTIME\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifdef OPTIMIZE_RUNTIME\n"
"\n"
"#define	USE_DENSITY_TEXTURE			_USE_DENSITY_TEXTURE_\n"
"#define USE_PIXEL_DENSITY			_USE_PIXEL_DENSITY_\n"
"\n"
"#define SAMPLE_DENSITY				_SAMPLE_DENSITY_\n"
"\n"
"#endif\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef OPTIMIZE_RUNTIME\n"
"\n"
"#define USE_PIXEL_DENSITY		1\n"
"#define USE_DENSITY_TEXTURE		1\n"
"#define USE_WEIGHT_TEXTURE		1\n"
"\n"
"#define SAMPLE_DENSITY			SAMPLE_RED\n"
"\n"
"#endif\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"#define NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"#define NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE 1023\n"
"#define NUM_INTERPOLATED_ATTRIBUTES 1024\n"
"#define LUT_SIZE_MINUS_ONE 1023\n"
"\n"
"#define NHAIRS_PER_PATCH 64\n"
"#define NSEGMENTS_PER_PATCH 256 \n"
"\n"
"#define TWO_PI 3.141592 * 2.0f\n"
"\n"
"#ifndef FLT_EPSILON\n"
"#define FLT_EPSILON	0.0000001f\n"
"#endif\n"
"\n"
"#ifdef _CPP // C++ code\n"
"\n"
"#define float4			gfsdk_float4\n"
"#define float3			gfsdk_float3\n"
"#define float2			gfsdk_float2\n"
"#define float4x4		gfsdk_float4x4\n"
"#define matrix4			gfsdk_float4x4\n"
"#define DualQuaternion	gfsdk_dualquaternion\n"
"\n"
"typedef int				int2[2];\n"
"typedef int				int4[4];\n"
"\n"
"#define lerp			gfsdk_lerp\n"
"\n"
"#else // shader\n"
"\n"
"struct DualQuaternion\n"
"{\n"
"	float4 q0;\n"
"	float4 q1;\n"
"};\n"
"\n"
"#define matrix4			row_major float4x4\n"
"#define MATERIAL_CHANNELS	float2\n"
"\n"
"inline float NvHair_LerpChannel(float2 channel, float s)\n"
"{\n"
"	return channel.x;\n"
"}\n"
"\n"
"#define SAMPLE_RED(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r;\n"
"#define SAMPLE_GREEN(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g;\n"
"#define SAMPLE_BLUE(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b;\n"
"#define SAMPLE_ALPHA(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"#define NV_HAIR_SAMPLE_CHANNEL(TEX, SAMPLER, TEXCOORD, MIPLEVEL, CHANNEL, SAMPLE) \\\n"
"	if (CHANNEL == 0) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r; \\\n"
"	else if (CHANNEL == 1) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g; \\\n"
"	else if (CHANNEL == 2) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b; \\\n"
"	else if (CHANNEL == 3) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"inline float NvHair_SampleChannel( Texture2D tex, SamplerState texSampler, float2 texcoords, int channel, float weight = 1.0f)\n"
"{\n"
"	float sample = 1.0f;\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, texSampler, texcoords, 0, channel, sample);\n"
"	return sample;\n"
"}\n"
"\n"
"#endif // _CPP \n"
"\n"
"#endif  // NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef NV_HAIR_INTERPOLATE_H\n"
"#define NV_HAIR_INTERPOLATE_H\n"
"\n"
"#ifdef _CPP\n"
"#	error \"Can only be included in HLSL code\"\n"
"#endif\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"#define NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"#define NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE 1023\n"
"#define NUM_INTERPOLATED_ATTRIBUTES 1024\n"
"#define LUT_SIZE_MINUS_ONE 1023\n"
"\n"
"#define NHAIRS_PER_PATCH 64\n"
"#define NSEGMENTS_PER_PATCH 256 \n"
"\n"
"#define TWO_PI 3.141592 * 2.0f\n"
"\n"
"#ifndef FLT_EPSILON\n"
"#define FLT_EPSILON	0.0000001f\n"
"#endif\n"
"\n"
"#ifdef _CPP // C++ code\n"
"\n"
"#define float4			gfsdk_float4\n"
"#define float3			gfsdk_float3\n"
"#define float2			gfsdk_float2\n"
"#define float4x4		gfsdk_float4x4\n"
"#define matrix4			gfsdk_float4x4\n"
"#define DualQuaternion	gfsdk_dualquaternion\n"
"\n"
"typedef int				int2[2];\n"
"typedef int				int4[4];\n"
"\n"
"#define lerp			gfsdk_lerp\n"
"\n"
"#else // shader\n"
"\n"
"struct DualQuaternion\n"
"{\n"
"	float4 q0;\n"
"	float4 q1;\n"
"};\n"
"\n"
"#define matrix4			row_major float4x4\n"
"#define MATERIAL_CHANNELS	float2\n"
"\n"
"inline float NvHair_LerpChannel(float2 channel, float s)\n"
"{\n"
"	return channel.x;\n"
"}\n"
"\n"
"#define SAMPLE_RED(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r;\n"
"#define SAMPLE_GREEN(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g;\n"
"#define SAMPLE_BLUE(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b;\n"
"#define SAMPLE_ALPHA(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"#define NV_HAIR_SAMPLE_CHANNEL(TEX, SAMPLER, TEXCOORD, MIPLEVEL, CHANNEL, SAMPLE) \\\n"
"	if (CHANNEL == 0) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r; \\\n"
"	else if (CHANNEL == 1) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g; \\\n"
"	else if (CHANNEL == 2) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b; \\\n"
"	else if (CHANNEL == 3) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"inline float NvHair_SampleChannel( Texture2D tex, SamplerState texSampler, float2 texcoords, int channel, float weight = 1.0f)\n"
"{\n"
"	float sample = 1.0f;\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, texSampler, texcoords, 0, channel, sample);\n"
"	return sample;\n"
"}\n"
"\n"
"#endif // _CPP \n"
"\n"
"#endif  // NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef NV_HAIR_INTERNAL_SHADER_TYPES_H\n"
"#define NV_HAIR_INTERNAL_SHADER_TYPES_H\n"
"\n"
"/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited. */\n"
"\n"
"#ifndef NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"#define NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"#define NUM_INTERPOLATED_ATTRIBUTES_MINUS_ONE 1023\n"
"#define NUM_INTERPOLATED_ATTRIBUTES 1024\n"
"#define LUT_SIZE_MINUS_ONE 1023\n"
"\n"
"#define NHAIRS_PER_PATCH 64\n"
"#define NSEGMENTS_PER_PATCH 256 \n"
"\n"
"#define TWO_PI 3.141592 * 2.0f\n"
"\n"
"#ifndef FLT_EPSILON\n"
"#define FLT_EPSILON	0.0000001f\n"
"#endif\n"
"\n"
"#ifdef _CPP // C++ code\n"
"\n"
"#define float4			gfsdk_float4\n"
"#define float3			gfsdk_float3\n"
"#define float2			gfsdk_float2\n"
"#define float4x4		gfsdk_float4x4\n"
"#define matrix4			gfsdk_float4x4\n"
"#define DualQuaternion	gfsdk_dualquaternion\n"
"\n"
"typedef int				int2[2];\n"
"typedef int				int4[4];\n"
"\n"
"#define lerp			gfsdk_lerp\n"
"\n"
"#else // shader\n"
"\n"
"struct DualQuaternion\n"
"{\n"
"	float4 q0;\n"
"	float4 q1;\n"
"};\n"
"\n"
"#define matrix4			row_major float4x4\n"
"#define MATERIAL_CHANNELS	float2\n"
"\n"
"inline float NvHair_LerpChannel(float2 channel, float s)\n"
"{\n"
"	return channel.x;\n"
"}\n"
"\n"
"#define SAMPLE_RED(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r;\n"
"#define SAMPLE_GREEN(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g;\n"
"#define SAMPLE_BLUE(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b;\n"
"#define SAMPLE_ALPHA(TEX, SAMPLER, TEXCOORD, MIPLEVEL) TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"#define NV_HAIR_SAMPLE_CHANNEL(TEX, SAMPLER, TEXCOORD, MIPLEVEL, CHANNEL, SAMPLE) \\\n"
"	if (CHANNEL == 0) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).r; \\\n"
"	else if (CHANNEL == 1) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).g; \\\n"
"	else if (CHANNEL == 2) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).b; \\\n"
"	else if (CHANNEL == 3) \\\n"
"		SAMPLE = TEX.SampleLevel(SAMPLER, TEXCOORD, MIPLEVEL).a;\n"
"\n"
"inline float NvHair_SampleChannel( Texture2D tex, SamplerState texSampler, float2 texcoords, int channel, float weight = 1.0f)\n"
"{\n"
"	float sample = 1.0f;\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, texSampler, texcoords, 0, channel, sample);\n"
"	return sample;\n"
"}\n"
"\n"
"#endif // _CPP \n"
"\n"
"#endif  // NV_HAIR_INTERNAL_SHADER_COMMON_H\n"
"\n"
"\n"
"#define NV_HAIR_MAX_BONE_MATRICES 256\n"
"\n"
"#define	NV_HAIR_MAX_COLLISION_SPHERES 128\n"
"#define NV_HAIR_MAX_COLLISION_CAPSULES 128\n"
"\n"
"#define	NV_HAIR_MAX_PINS 32\n"
"\n"
"#define NV_HAIR_BLOCK_SIZE_SIMULATE 64\n"
"#define NV_HAIR_BLOCK_SIZE_PIN_COM 1024\n"
"\n"
"\n"
"struct NvHair_SimulationMaterial\n"
"{\n"
"	float		stiffness;\n"
"	float		damping;\n"
"	float		stiffnessStrength;\n"
"	float		stiffnessDamping;\n"
"\n"
"	float		rootStiffness;\n"
"	float		tipStiffness;\n"
"	float		bendStiffness;\n"
"	float		interactionStiffness;\n"
"\n"
"	float		pinStiffness;\n"
"	float		inertiaScale;\n"
"	float		backStopRadius;\n"
"	float		friction;\n"
"\n"
"	float		hairNormalWeight;\n"
"	float		massScale;\n"
"	float		_reserved2_;\n"
"	float		_reserved3_;\n"
"\n"
"	float4		stiffnessCurve;\n"
"	float4		stiffnessStrengthCurve;\n"
"	float4		stiffnessDampingCurve;\n"
"	float4		bendStiffnessCurve;\n"
"	float4		interactionStiffnessCurve;\n"
"};\n"
"\n"
"#define NV_HAIR_LERP_SIMULATION_PARAM(BUFFER, PARAM) \\\n"
"	PARAM	= lerp(BUFFER.defaultMaterial.PARAM, BUFFER.targetMaterial.PARAM, BUFFER.materialWeight);\n"
"\n"
"struct NvHair_SimulateConstantBuffer\n"
"{\n"
"	matrix4 boneMatrices[NV_HAIR_MAX_BONE_MATRICES];\n"
"	matrix4 skinMatrices[NV_HAIR_MAX_BONE_MATRICES];\n"
"	DualQuaternion skinDqs[NV_HAIR_MAX_BONE_MATRICES];\n"
"\n"
"	matrix4 modelToWorld;\n"
"\n"
"	float4 modelCenter;\n"
"	float4 modelCenterRest;\n"
"\n"
"	float4 gravity;\n"
"\n"
"	float3 wind;\n"
"	float windNoise;\n"
"\n"
"	int numBones;\n"
"	int numTotalCvs;\n"
"	int numCollisionSpheres;\n"
"	int numCollisionCapsules;\n"
"\n"
"	int numPinConstraints;\n"
"	int useCollision;\n"
"	int useDynamicPin;\n"
"	int useDualQuaterinon;\n"
"\n"
"	int numTubeVertices; // NUTT\n"
"	int numStrands; // NUTT\n"
"	int numParsPerStrand; // NUTT\n"
"	int numSubSegments; // NUTT\n"
"\n"
"	float kInterpolation; // NUTT\n"
"	float simulationInterp;					///< Used on prepare interp to calculate the simulation interpolation\n"
"	int tmp2;\n"
"	int tmp3;\n"
"\n"
"	int numConstraintIterations;\n"
"	float timeStep;\n"
"	int simulate;\n"
"	int lockInertia;\n"
"\n"
"	float maxHairLength;\n"
"	int stiffnessChannel;\n"
"	int rootStiffnessChannel;\n"
"	float _reserved3_;\n"
"\n"
"	int useStiffnessTexture;\n"
"	int useRootStiffnessTexture;\n"
"	int useWeightTexture;\n"
"	float materialWeight;\n"
"\n"
"	NvHair_SimulationMaterial defaultMaterial;\n"
"	NvHair_SimulationMaterial targetMaterial;\n"
"\n"
"	float4 collisionSpheres[NV_HAIR_MAX_COLLISION_SPHERES];\n"
"	float4 collisionCapsuleIndex[NV_HAIR_MAX_COLLISION_CAPSULES];\n"
"};\n"
"\n"
"\n"
"struct NvHair_TessellationMaterial // packed to 2 channel for faster sampling/interpolation\n"
"{\n"
"	float		width;\n"
"	float		widthNoiseScale;\n"
"	float		rootWidthScale;\n"
"	float		tipWidthScale;\n"
"\n"
"	float		density;\n"
"	float		clumpScale;\n"
"	float		clumpNoise;\n"
"	float		clumpRoundness;\n"
"\n"
"	float		lengthNoise;\n"
"	float		lengthScale;\n"
"	float		__reserved1__;\n"
"	float		__reserved2__;\n"
"\n"
"	float		waveScale;\n"
"	float		waveScaleNoise;\n"
"	float		waveScaleStrand;\n"
"	float		waveScaleClump;\n"
"\n"
"	float		waveFreq;\n"
"	float		waveFreqNoise;\n"
"	float		waveCutoff;\n"
"	float		__reserved3__;\n"
"};\n"
"\n"
"struct NvHair_TessellationConstantBuffer\n"
"{\n"
"	// transforms\n"
"	matrix4	viewProjection;\n"
"	matrix4	viewMatrix;\n"
"	matrix4	inverseViewMatrix;\n"
"	matrix4	modelToWorld;\n"
"	matrix4	cullSphereInvTransform;\n"
"	float4 camPosition;\n"
"\n"
"	// tessellation materials\n"
"	NvHair_TessellationMaterial defaultMaterial;\n"
"	NvHair_TessellationMaterial targetMaterial;\n"
"\n"
"	// texture bit\n"
"	int useDensityTexture;\n"
"	int	useWidthTexture;\n"
"	int	useClumpScaleTexture;\n"
"	int useClumpRoundnessTexture;\n"
"\n"
"	int useClumpNoiseTexture;\n"
"	int useWaveScaleTexture;\n"
"	int useWaveFreqTexture;\n"
"	int useLengthTexture;\n"
"\n"
"	// material control\n"
"	float materialWeight;\n"
"	int useWeightTexture;\n"
"	unsigned int strandPointCount;\n"
"	float __reserved1__;\n"
"\n"
"	// shader settings\n"
"	unsigned int shaderMask;\n"
"	int leftHanded;\n"
"	int vertexClumping;\n"
"	float __reserved2_;\n"
"\n"
"	// density option\n"
"	int usePixelDensity;\n"
"	float densityPass;\n"
"	float __reserved3__;\n"
"	float __reserved2__;\n"
"\n"
"	// culling\n"
"	int useViewfrustrumCulling;\n"
"	float useBackfaceCulling;\n"
"	float backfaceCullingThreshold;\n"
"	int useCullSphere;\n"
"\n"
"	// channels\n"
"	int2 densityTextureChan;\n"
"	int2 widthTextureChan;\n"
"\n"
"	int2 clumpScaleTextureChan;\n"
"	int2 clumpRoundnessTextureChan;\n"
"\n"
"	int2 waveScaleTextureChan;\n"
"	int2 waveFreqTextureChan;\n"
"\n"
"	int2 lengthTextureChan;\n"
"	int2 weightTextureChan;\n"
"\n"
"	// Cubemap rendering info\n"
"	int4 cubeMapActive;\n"
"	matrix4	cubeMapViewProjMatrix[6];\n"
"	matrix4	cubeMapInvViewMatrix[6];\n"
"	int4 cubeMapVisible[6];\n"
"};\n"
"\n"
"\n"
"struct NvHair_Pin\n"
"{\n"
"	matrix4 invHairPoseMatrix;\n"
"	matrix4 currentHairMatrix;\n"
"\n"
"	matrix4 invPinPoseMatrix; // = inverse pose\n"
"	matrix4 currentPinMatrix;\n"
"	matrix4 shapeMatrix;\n"
"\n"
"	float radius;\n"
"	float3 localPos;\n"
"\n"
"	int boneIndex;\n"
"	float3 restComShift;\n"
"\n"
"	int rootBoneIndex;\n"
"	float rootBoneDis;\n"
"	int stiffPin;\n"
"	int doLra;\n"
"\n"
"	float3 restTangent;\n"
"	int useDynamicPin;\n"
"\n"
"	float stiffness;\n"
"	float influenceFallOff;\n"
"	int selected;\n"
"	float __align__;\n"
"\n"
"	float4 influenceFallOffCurve;\n"
"};\n"
"\n"
"#define NV_HAIR_SCRATCH_SIZE_PER_PIN 128 // = 128 * 1024 = 130K simulated CVs\n"
"struct NvHair_PinScratchData\n"
"{\n"
"	float4 com;\n"
"	float4 tangent;\n"
"};\n"
"\n"
"\n"
"struct NvHair_SplineConstantBuffer\n"
"{\n"
"	matrix4 modelToWorld;\n"
"\n"
"	unsigned int numVertsPerSegments;\n"
"	unsigned int numMasterSegments;\n"
"	unsigned int numTessellatedPoints;\n"
"	unsigned int strandPointCounts;\n"
"\n"
"	float simulationInterp;\n"
"	float __unused__[3];\n"
"};\n"
"\n"
"\n"
"struct NvHair_VisualizeConstantBuffer\n"
"{\n"
"	matrix4 viewProjection;\n"
"	matrix4 modelToWorld;\n"
"\n"
"	float4 color;\n"
"\n"
"	int hairMin;\n"
"	int hairMax;\n"
"	int hairSkip;\n"
"	int hairDummy;\n"
"\n"
"	float hairWidth;\n"
"	float aspect;\n"
"	float scale;\n"
"	int pinId;\n"
"};\n"
"\n"
"\n"
"struct NvHair_StatsPerFrameConstantBuffer\n"
"{\n"
"	int		numFaces;\n"
"	int		usePixelDensity;\n"
"	float	density;\n"
"	int		_dummy;\n"
"};\n"
"\n"
"\n"
"#endif  // NV_HAIR_INTERNAL_SHADER_TYPES_H\n"
"\n"
"\n"
"void NvHair_BlendMaterial(NvHair_TessellationMaterial defaultMaterial, in NvHair_TessellationMaterial targetMaterial, float weight, out NvHair_TessellationMaterial dst)\n"
"{\n"
"#define LERP_MATERIAL(PARAM) \\\n"
"	dst.PARAM	= lerp(defaultMaterial.PARAM, targetMaterial.PARAM, weight);\n"
"\n"
"	LERP_MATERIAL(width);\n"
"	LERP_MATERIAL(widthNoiseScale);\n"
"	LERP_MATERIAL(rootWidthScale);\n"
"	LERP_MATERIAL(tipWidthScale);\n"
"\n"
"	LERP_MATERIAL(density);\n"
"	LERP_MATERIAL(clumpScale);\n"
"	LERP_MATERIAL(clumpNoise);\n"
"	LERP_MATERIAL(clumpRoundness);\n"
"\n"
"	LERP_MATERIAL(lengthNoise);\n"
"	LERP_MATERIAL(lengthScale);\n"
"	LERP_MATERIAL(waveScale);\n"
"	LERP_MATERIAL(waveFreq);\n"
"\n"
"	LERP_MATERIAL(waveScaleNoise);\n"
"	LERP_MATERIAL(waveFreqNoise);\n"
"	LERP_MATERIAL(waveCutoff);\n"
"	LERP_MATERIAL(waveScaleClump);\n"
"	LERP_MATERIAL(waveScaleStrand);\n"
"\n"
"#undef LERP_MATERIAL\n"
"}\n"
"\n"
"Buffer<float4>				g_tessellatedMasterStrand		: register(t0);\n"
"Buffer<float4>				g_tessellatedMasterStrandPrev	: register(t1);\n"
"\n"
"Buffer<float4>				g_restMasterStrand				: register(t2);\n"
"Buffer<float2>				g_texCoords						: register(t3);\n"
"\n"
"Buffer<float3>				g_faceHairIndices				: register(t4);\n"
"Buffer<float2>				g_strandCoordinatesLut			: register(t5);\n"
"Buffer<float>				g_noiseLut						: register(t6);\n"
"\n"
"Texture2D					g_densityTexture				: register(t7);\n"
"Texture2D					g_widthTexture					: register(t8);\n"
"\n"
"Texture2D					g_clumpScaleTexture				: register(t9);\n"
"Texture2D					g_clumpRoundnessTexture			: register(t10);\n"
"\n"
"Texture2D					g_waveScaleTexture				: register(t11);\n"
"Texture2D					g_waveFreqTexture				: register(t12);\n"
"Texture2D					g_lengthTexture					: register(t13);\n"
"Texture2D					g_weightTexture					: register(t14);\n"
"\n"
"Buffer<float4>				g_tessellatedTangents			: register(t15);\n"
"Buffer<float4>				g_tessellatedNormals			: register(t16);\n"
" \n"
"SamplerState samLinear : register(s0);\n"
"SamplerState samPointClamp : register(s1);\n"
"\n"
"cbuffer cbPerFrame : register( b0 )\n"
"{\n"
"	NvHair_TessellationConstantBuffer g_buffer;\n"
"}\n"
"\n"
"struct VSOut\n"
"{\n"
"	float dummy : DUMMY;\n"
"};\n"
"\n"
"struct HSOut\n"
"{\n"
"    float		edges[2]			: SV_TessFactor;\n"
"	float3		rootIndices			: ROOT_INDICES;\n"
"	float3x2	texcoords			: TEX_COORDS;\n"
"	uint		primitiveId			: PRIMITIVE_ID;\n"
"};\n"
"\n"
"struct DSOut // 14 floats\n"
"{\n"
"   float3	position	: Position;\n"
"   float2	texcoords	: SCALPTEX;\n"
"   float3	normal		: Normal;\n"
"   float3	tangent		: Tangent;\n"
"   float	width		: Width;\n"
"   float	tex			: TEXALONGLENGTH;\n"
"\n"
"   uint		primitiveId : PRIMITIVE_ID;\n"
"   float2	coords		: COORDS;\n"
"};\n"
"\n"
"inline float NvHair_SampleMultiChannel( Texture2D tex, float2 texcoords, int2 channels, float weight)\n"
"{\n"
"#if MULTI_CHANNEL_SUPPORT\n"
"	float sampleDefault = 1.0f;\n"
"	float sampleTarget = 1.0f;\n"
"	float sample = 1.0f;\n"
"\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[0], sampleDefault);\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[1], sampleTarget); // optimize?\n"
"\n"
"	if (weight == 0.0f)\n"
"		sample = sampleDefault;\n"
"	else\n"
"	{\n"
"		sample = lerp(sampleDefault, sampleTarget, weight);\n"
"	}\n"
"#else\n"
"	float sample = 1.0f;\n"
"\n"
"	NV_HAIR_SAMPLE_CHANNEL(tex, samLinear, texcoords, 0, channels[0], sample);\n"
"#endif\n"
"\n"
"	return sample;\n"
"}\n"
"\n"
"inline void NvHair_BlendMaterial(out NvHair_TessellationMaterial material, float weight = 0.0f)\n"
"{\n"
"	material = g_buffer.defaultMaterial;\n"
"}\n"
"\n"
"\n"
"inline float3 NvHair_InterpolateBary(float3 v0, float3 v1, float3 v2, float3 coords)\n"
"{\n"
"	return coords.x  * v0.xyz + coords.y * v1.xyz + coords.z * v2.xyz; \n"
"}\n"
"\n"
"inline float4 NvHair_InterpolateBary(float4 v0, float4 v1, float4 v2, float3 coords)\n"
"{\n"
"	return coords.x  * v0 + coords.y * v1 + coords.z * v2; \n"
"}\n"
"\n"
"float packFloat2(float x, float y)\n"
"{\n"
"	const float base = 2048;\n"
"\n"
"	float basey = floor(base * y);\n"
"	float packed = basey + x;\n"
"\n"
"	return packed;\n"
"}\n"
"\n"
"float packSignedFloat(float x)\n"
"{\n"
"	return 0.5f + 0.5f * clamp(x, -1.0, 1.0);\n"
"}\n"
"\n"
"float packSignedFloat2(float x, float y)\n"
"{\n"
"	float sx = packSignedFloat(x);\n"
"	float sy = packSignedFloat(y);\n"
"\n"
"	return packFloat2(sx,sy);\n"
"}\n"
"\n"
"\n"
"inline float2 unpackFloat2(float packed)\n"
"{\n"
"	const float inv_base = 1.0f / 2048.0f;\n"
"\n"
"	float ubase = floor(packed);\n"
"	float unpackedy = ubase * inv_base;\n"
"	float unpackedx = packed - ubase;\n"
"\n"
"	return float2(unpackedx, unpackedy);\n"
"}\n"
"\n"
"inline float unpackSignedFloat(float x)\n"
"{\n"
"	return clamp(2.0f * (x - 0.5f), -1.0f, 1.0f);\n"
"}\n"
"\n"
"inline float2 unpackSignedFloat2(float x)\n"
"{\n"
"	float2 unpacked = unpackFloat2(x);\n"
"	float sx = unpackSignedFloat(unpacked.x);\n"
"	float sy = unpackSignedFloat(unpacked.y);\n"
"\n"
"	return float2(sx, sy);\n"
"}\n"
"\n"
"float3 NvHair_GetCameraDirection()\n"
"{\n"
"	//float3 dir = gfsdk_getBasisZ(g_buffer.inverseViewMatrix);\n"
"	float3 dir = float3(\n"
"		g_buffer.inverseViewMatrix._31,\n"
"		g_buffer.inverseViewMatrix._32,\n"
"		g_buffer.inverseViewMatrix._33);\n"
"\n"
"	if (g_buffer.leftHanded)\n"
"		dir *= -1;\n"
"\n"
"	return dir;\n"
"}\n"
"\n"
"#endif  // NV_HAIR_INTERPOLATE_H\n"
"\n"
"inline float2 NvHair_getClipSpaceCoord(float3 p)\n"
"{\n"
"	// convert to view space (clip space)\n"
"	float4 screenPos = mul(float4(p.xyz, 1), g_buffer.viewProjection);\n"
"	screenPos.xy /= screenPos.w;\n"
"\n"
"	return screenPos.xy;\n"
"}\n"
"\n"
"inline bool NvHair_checkClipping(float2 pos, float size)\n"
"{\n"
"	if ((pos.x < -size) || (pos.x > size))\n"
"		return true;\n"
"\n"
"	if ((pos.y < -size) || (pos.y > size))\n"
"		return true;\n"
"\n"
"	return false;\n"
"}\n"
"\n"
"inline float NvHair_viewFrustrumClipDensity(float3 rootIndices, int numSegments)\n"
"{\n"
"	int rootIndex0 = floor(rootIndices.x);\n"
"	int rootIndex1 = floor(rootIndices.y);\n"
"	int rootIndex2 = floor(rootIndices.z);\n"
"	\n"
"	int tipIndex0 = rootIndex0 + numSegments;\n"
"	int tipIndex1 = rootIndex1 + numSegments;\n"
"	int tipIndex2 = rootIndex2 + numSegments;\n"
"\n"
"	// root position for early culling\n"
"	float3 r0 = g_tessellatedMasterStrand.Load(rootIndex0).xyz;\n"
"	float3 r1 = g_tessellatedMasterStrand.Load(rootIndex1).xyz;\n"
"	float3 r2 = g_tessellatedMasterStrand.Load(rootIndex2).xyz;\n"
"\n"
"	float2 screenPos0 = NvHair_getClipSpaceCoord(r0);\n"
"	float2 screenPos1 = NvHair_getClipSpaceCoord(r1);\n"
"	float2 screenPos2 = NvHair_getClipSpaceCoord(r2);\n"
"\n"
"	float3 t0 = g_tessellatedMasterStrand.Load(tipIndex0).xyz;\n"
"	float3 t1 = g_tessellatedMasterStrand.Load(tipIndex1).xyz;\n"
"	float3 t2 = g_tessellatedMasterStrand.Load(tipIndex2).xyz;\n"
"\n"
"	float threshold = 1.05f;\n"
"\n"
"	bool rootHidden0 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r0), threshold);\n"
"	bool rootHidden1 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r1), threshold);\n"
"	bool rootHidden2 = NvHair_checkClipping(NvHair_getClipSpaceCoord(r2), threshold);\n"
"\n"
"	bool tipHidden0 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t0), threshold);\n"
"	bool tipHidden1 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t1), threshold);\n"
"	bool tipHidden2 = NvHair_checkClipping(NvHair_getClipSpaceCoord(t2), threshold);\n"
"\n"
"	if (rootHidden0 && rootHidden1 && rootHidden2 &&\n"
"		tipHidden0 && tipHidden1 && tipHidden2)\n"
"		return 0.0f;\n"
"\n"
"	return 1.0f;\n"
"}\n"
"\n"
"inline float NvHair_backfaceCulling(float3 rootIndices)\n"
"{\n"
"	float3 n0 = g_tessellatedNormals.Load( rootIndices ).xyz;\n"
"	float3 n1 = g_tessellatedNormals.Load( rootIndices ).xyz;\n"
"	float3 n2 = g_tessellatedNormals.Load( rootIndices ).xyz; \n"
"		\n"
"	float3 d =  NvHair_GetCameraDirection();\n"
"\n"
"	float eps = g_buffer.backfaceCullingThreshold;\n"
"	bool cull0 = dot(n0, d) < eps;\n"
"	bool cull1 = dot(n1, d) < eps;\n"
"	bool cull2 = dot(n2, d) < eps;\n"
"\n"
"	if (cull0 && cull1 && cull2)\n"
"		return 0.0f;\n"
"\n"
"	return 1.0f;\n"
"}\n"
"\n"
"HSOut InterpolateHSConst(uint iWisp : SV_PrimitiveID)\n"
"{    \n"
"    HSOut output = (HSOut)0;\n"
"\n"
"	int numSegments = g_buffer.strandPointCount - 1;\n"
"\n"
"	float3 hairIndices = g_faceHairIndices.Load(iWisp);\n"
"\n"
"	output.rootIndices = floor(\n"
"		float3(\n"
"			hairIndices.x * g_buffer.strandPointCount + 0.5f, \n"
"			hairIndices.y * g_buffer.strandPointCount + 0.5f,\n"
"			hairIndices.z * g_buffer.strandPointCount + 0.5f\n"
"			)\n"
"		);\n"
"\n"
"	output.texcoords[0] = g_texCoords.Load( iWisp * 3);\n"
"	output.texcoords[1] = g_texCoords.Load( iWisp * 3 + 1);\n"
"	output.texcoords[2] = g_texCoords.Load( iWisp * 3 + 2);\n"
"\n"
"	output.primitiveId = iWisp;\n"
"\n"
"	float2 texcoords = 1.0 / 3.0 * (output.texcoords[0] + output.texcoords[1] + output.texcoords[2]);\n"
"\n"
"	float weight = g_buffer.materialWeight;\n"
"\n"
"	NvHair_TessellationMaterial material = g_buffer.defaultMaterial;\n"
"\n"
"	// density is pre-interpolated from LOD, etc.\n"
"	float density = NvHair_LerpChannel(material.density, weight);\n"
"		  density = min(1.0f, density - g_buffer.densityPass);\n"
"	\n"
"	// set density to zero for hairs completely outside view frustrum (both tip and root)\n"
"	if (g_buffer.useViewfrustrumCulling)\n"
"		density *= NvHair_viewFrustrumClipDensity(output.rootIndices, numSegments);\n"
"\n"
"	// apply backface culling \n"
"	if (g_buffer.useBackfaceCulling)\n"
"		density *= NvHair_backfaceCulling(output.rootIndices);\n"
"\n"
"#ifndef OPTIMIZE_RUNTIME\n"
"	if (!g_buffer.usePixelDensity && g_buffer.useDensityTexture)\n"
"		density *= NvHair_SampleChannel(g_densityTexture, samLinear, texcoords, g_buffer.densityTextureChan);\n"
"#else\n"
"\n"
"	#if USE_DENSITY_TEXTURE && !(USE_PIXEL_DENSITY)\n"
"	density *= SAMPLE_DENSITY(g_densityTexture, samLinear, texcoords, 0);\n"
"	#endif\n"
"#endif\n"
"\n"
"	output.edges[0] = max(NHAIRS_PER_PATCH * density, 0);\n"
"	output.edges[1] = min(numSegments, NSEGMENTS_PER_PATCH);\n"
"\n"
"    return output;\n"
"}\n"
"\n"
"[domain(\"isoline\")]\n"
"[partitioning(\"integer\")]\n"
"[outputtopology(\"line\")]\n"
"[outputcontrolpoints(1)]\n"
"[patchconstantfunc(\"InterpolateHSConst\")]\n"
"void hs_main(InputPatch<VSOut, 1> inputPatch)\n"
"{\n"
"}\n"
"\n"
;