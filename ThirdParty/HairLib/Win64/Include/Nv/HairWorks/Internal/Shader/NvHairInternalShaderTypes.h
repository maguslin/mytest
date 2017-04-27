/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_INTERNAL_SHADER_TYPES_H
#define NV_HAIR_INTERNAL_SHADER_TYPES_H

#include "NvHairInternalShaderCommon.h"

///////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////
#define NV_HAIR_MAX_BONE_MATRICES 256

#define	NV_HAIR_MAX_COLLISION_SPHERES 128
#define NV_HAIR_MAX_COLLISION_CAPSULES 128

#define	NV_HAIR_MAX_PINS 32

#define NV_HAIR_BLOCK_SIZE_SIMULATE 64
#define NV_HAIR_BLOCK_SIZE_PIN_COM 1024


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// simulation materials
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct NvHair_SimulationMaterial
{
	float		stiffness;
	float		damping;
	float		stiffnessStrength;
	float		stiffnessDamping;

	float		rootStiffness;
	float		tipStiffness;
	float		bendStiffness;
	float		interactionStiffness;

	float		pinStiffness;
	float		inertiaScale;
	float		backStopRadius;
	float		friction;

	float		hairNormalWeight;
	float		massScale;
	float		_reserved2_;
	float		_reserved3_;

	float4		stiffnessCurve;
	float4		stiffnessStrengthCurve;
	float4		stiffnessDampingCurve;
	float4		bendStiffnessCurve;
	float4		interactionStiffnessCurve;
};

#define NV_HAIR_LERP_SIMULATION_PARAM(BUFFER, PARAM) \
	PARAM	= lerp(BUFFER.defaultMaterial.PARAM, BUFFER.targetMaterial.PARAM, BUFFER.materialWeight);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// constant buffer for simulation shaders
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct NvHair_SimulateConstantBuffer
{
	matrix4 boneMatrices[NV_HAIR_MAX_BONE_MATRICES];
	matrix4 skinMatrices[NV_HAIR_MAX_BONE_MATRICES];
	DualQuaternion skinDqs[NV_HAIR_MAX_BONE_MATRICES];

	matrix4 modelToWorld;

	float4 modelCenter;
	float4 modelCenterRest;

	float4 gravity;

	float3 wind;
	float windNoise;

	int numBones;
	int numTotalCvs;
	int numCollisionSpheres;
	int numCollisionCapsules;

	int numPinConstraints;
	int useCollision;
	int useDynamicPin;
	int useDualQuaterinon;

	int numTubeVertices; // NUTT
	int numStrands; // NUTT
	int numParsPerStrand; // NUTT
	int numSubSegments; // NUTT

	float kInterpolation; // NUTT
	float simulationInterp;					///< Used on prepare interp to calculate the simulation interpolation
	int tmp2;
	int tmp3;

	int numConstraintIterations;
	float timeStep;
	int simulate;
	int lockInertia;

	float maxHairLength;
	int stiffnessChannel;
	int rootStiffnessChannel;
	float _reserved3_;

	int useStiffnessTexture;
	int useRootStiffnessTexture;
	int useWeightTexture;
	float materialWeight;

	NvHair_SimulationMaterial defaultMaterial;
	NvHair_SimulationMaterial targetMaterial;

	float4 collisionSpheres[NV_HAIR_MAX_COLLISION_SPHERES];
	float4 collisionCapsuleIndex[NV_HAIR_MAX_COLLISION_CAPSULES];
};

//////////////////////////////////////////////////////////////////////////////
// interpolated material parameters
//////////////////////////////////////////////////////////////////////////////

struct NvHair_TessellationMaterial // packed to 2 channel for faster sampling/interpolation
{
	float		width;
	float		widthNoiseScale;
	float		rootWidthScale;
	float		tipWidthScale;

	float		density;
	float		clumpScale;
	float		clumpNoise;
	float		clumpRoundness;

	float		lengthNoise;
	float		lengthScale;
	float		__reserved1__;
	float		__reserved2__;

	float		waveScale;
	float		waveScaleNoise;
	float		waveScaleStrand;
	float		waveScaleClump;

	float		waveFreq;
	float		waveFreqNoise;
	float		waveCutoff;
	float		__reserved3__;
};

//////////////////////////////////////////////////////////////////////////////
// constant buffer for tessellation shaders
//////////////////////////////////////////////////////////////////////////////
struct NvHair_TessellationConstantBuffer
{
	// transforms
	matrix4	viewProjection;
	matrix4	viewMatrix;
	matrix4	inverseViewMatrix;
	matrix4	modelToWorld;
	matrix4	cullSphereInvTransform;
	float4 camPosition;

	// tessellation materials
	NvHair_TessellationMaterial defaultMaterial;
	NvHair_TessellationMaterial targetMaterial;

	// texture bit
	int useDensityTexture;
	int	useWidthTexture;
	int	useClumpScaleTexture;
	int useClumpRoundnessTexture;

	int useClumpNoiseTexture;
	int useWaveScaleTexture;
	int useWaveFreqTexture;
	int useLengthTexture;

	// material control
	float materialWeight;
	int useWeightTexture;
	unsigned int strandPointCount;
	float __reserved1__;

	// shader settings
	unsigned int shaderMask;
	int leftHanded;
	int vertexClumping;
	float __reserved2_;

	// density option
	int usePixelDensity;
	float densityPass;
	float __reserved3__;
	float __reserved2__;

	// culling
	int useViewfrustrumCulling;
	float useBackfaceCulling;
	float backfaceCullingThreshold;
	int useCullSphere;

	// channels
	int2 densityTextureChan;
	int2 widthTextureChan;

	int2 clumpScaleTextureChan;
	int2 clumpRoundnessTextureChan;

	int2 waveScaleTextureChan;
	int2 waveFreqTextureChan;

	int2 lengthTextureChan;
	int2 weightTextureChan;

	// Cubemap rendering info
	int4 cubeMapActive;
	matrix4	cubeMapViewProjMatrix[6];
	matrix4	cubeMapInvViewMatrix[6];
	int4 cubeMapVisible[6];
};

//////////////////////////////////////////////////////////////////////////////
// Pin
//////////////////////////////////////////////////////////////////////////////

struct NvHair_Pin
{
	matrix4 invHairPoseMatrix;
	matrix4 currentHairMatrix;

	matrix4 invPinPoseMatrix; // = inverse pose
	matrix4 currentPinMatrix;
	matrix4 shapeMatrix;

	float radius;
	float3 localPos;

	int boneIndex;
	float3 restComShift;

	int rootBoneIndex;
	float rootBoneDis;
	int stiffPin;
	int doLra;

	float3 restTangent;
	int useDynamicPin;

	float stiffness;
	float influenceFallOff;
	int selected;
	float __align__;

	float4 influenceFallOffCurve;
};

#define NV_HAIR_SCRATCH_SIZE_PER_PIN 128 // = 128 * 1024 = 130K simulated CVs
struct NvHair_PinScratchData
{
	float4 com;
	float4 tangent;
};

//////////////////////////////////////////////////////////////////////////////
// Spline.h
//////////////////////////////////////////////////////////////////////////////

struct NvHair_SplineConstantBuffer
{
	matrix4 modelToWorld;

	unsigned int numVertsPerSegments;
	unsigned int numMasterSegments;
	unsigned int numTessellatedPoints;
	unsigned int strandPointCounts;

	float simulationInterp;
	float __unused__[3];
};

//////////////////////////////////////////////////////////////////////////////
// Visualize
//////////////////////////////////////////////////////////////////////////////

struct NvHair_VisualizeConstantBuffer
{
	matrix4 viewProjection;
	matrix4 modelToWorld;

	float4 color;

	int hairMin;
	int hairMax;
	int hairSkip;
	int hairDummy;

	float hairWidth;
	float aspect;
	float scale;
	int pinId;
};

//////////////////////////////////////////////////////////////////////////////
// Stats 

struct NvHair_StatsPerFrameConstantBuffer
{
	int		numFaces;
	int		usePixelDensity;
	float	density;
	int		_dummy;
};


#endif  // NV_HAIR_INTERNAL_SHADER_TYPES_H

