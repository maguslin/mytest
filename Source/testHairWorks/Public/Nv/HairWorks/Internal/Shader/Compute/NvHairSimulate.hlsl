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
RWStructuredBuffer<float4>	g_particlePositions			: register(u0); //particle positions, for the  solver 
RWStructuredBuffer<float4>	g_previousParticlePositions	: register(u1); //the partice positions from the last frame
RWStructuredBuffer<float4>	g_interpParticlePositions	: register(u2); //the partice positions from the last frame
RWStructuredBuffer<float4>	g_frames					: register(u3); // coordinate frames for bending constraints
RWStructuredBuffer<float4>	g_growthMeshVertices		: register(u4); // skinning data for growth mesh used in shadows and collisions
RWStructuredBuffer<float4>	g_skinnedParticlePositions	: register(u5); // skinned particle position
RWStructuredBuffer<float4>	g_hairNormals				: register(u6); // computed hair normals
RWStructuredBuffer<float4>	g_hairTangents				: register(u7); // computed hair tangents

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shared resource views
Buffer<int>		g_simulationStrandOffsets			: register(t0); 
Buffer<float>	g_particleDistanceConstraintLengths : register(t1);
Buffer<float>	g_particleBendConstraintLengths		: register(t2);
Buffer<float>	g_particleRootDistances				: register(t3);
Buffer<float>	g_normalizedLength					: register(t4);

Buffer<float4>	g_restMasterStrand					: register(t5);
Buffer<float4>	g_origFrames						: register(t6);

Buffer<float4>	g_masterLocalPosPrev				: register(t7);
Buffer<float4>	g_masterLocalPosNext				: register(t8);

Buffer<float4>	g_boneIndices						: register(t9);
Buffer<float4>	g_boneWeights						: register(t10);

Buffer<float4>	g_texCoords							: register(t11);

Buffer<float4>	g_growthMeshRestNormals				: register(t12);
Buffer<float4>	g_growthMeshRestTangents			: register(t13);

Buffer<float3>	g_windNoiseLut						: register(t14);

Texture2D		g_stiffnessTexture					: register(t15);
Texture2D		g_rootStiffnessTexture				: register(t16);
Texture2D		g_weightTexture						: register(t17);

//Buffer<float>	g_particleRadius					: register(t18);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//constants that change frame to frame
cbuffer cbPerFrame : register( b0 )
{
	NvHair_SimulateConstantBuffer g_buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture samplers
SamplerState samLinear : register(s0);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// preserves distance between two particles
void DistanceConstraint(inout float4 particle0, inout float4 particle1, float targetDistance, float stiffness = 1.0)
{
    float3 delta = particle1.xyz - particle0.xyz;
    float distance = max(length(delta), 1e-7);
    float stretching = 1 - targetDistance / distance;
    delta = stretching * delta;

	float wsum = particle0.w + particle1.w;
	float invwsum = (wsum == 0.0) ? 0.0 : 1.0 / wsum;
	float scale = stiffness * invwsum;

    particle0.xyz += particle0.w * scale * delta;
    particle1.xyz -= particle1.w * scale * delta;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// preserve distance from one fixed anchor to another free particle
void LRAConstraint(inout float4 particle, float4 root, float targetDistance, float stiffness = 1.0)
{
    float3 delta = root.xyz - particle.xyz;
    float distance = max(length(delta), 1e-7);
    
    float residual = max(0.0, distance - targetDistance) / distance;

	residual *= stiffness;
    
	particle.xyz += particle.w * residual * delta;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply global shape preserving constraint
void ApplyGlobalConstraint(
	float3 rootDelta,
	inout float4 prevPos,
	inout float4 currentPos, 
	float4 targetPos, 
	float targetDistance, 
	float stiffness,
	float damping = 0.0f)
{
    float3 constraintDir = targetPos.xyz - currentPos.xyz; // toward target position
    float distance = max(length(constraintDir), 1e-7);

	constraintDir /= distance; // normalize
    
    float residual = max(0.0, distance - targetDistance) ; // how far are we from the target distance?

	float constraintScale = residual * stiffness * currentPos.w; // how much should we move?

	currentPos.xyz += constraintScale * constraintDir;

	// damping along the constraint direction
	{
		float3 v = currentPos.xyz - prevPos.xyz - rootDelta.xyz; // subtract global translation from damping velocity
		float vDot = dot(v, constraintDir);
		prevPos.xyz += damping * (vDot * constraintDir);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplySphereConstraint(inout float4 particle, float3 sphereCenter, float sphereRadius)
{
	float3 delta = particle.xyz - sphereCenter.xyz;
	float distance = max(length(delta), 1e-7);

	float residual = max(0.0f, sphereRadius - distance) / distance;

	particle.xyz += particle.w * residual * delta;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CheckSphereConstraint(float4 particle, float3 sphereCenter, float sphereRadius)
{
	float3 delta = particle.xyz - sphereCenter.xyz;
	float distanceSq = dot(delta, delta);
	float rSq = sphereRadius * sphereRadius;

	return (distanceSq < rSq);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply sphere collision
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplySphereCollision(inout float4 particle, float4 skinnedPosition)
{
	for (int i = 0; i < g_buffer.numCollisionSpheres; i++)
	{
		float4	sphere = g_buffer.collisionSpheres[i];
		ApplySphereConstraint(particle, sphere.xyz, sphere.w);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply sphere collision
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyCapsuleCollision(inout float4 particle)
{
	for (int i = 0; i < g_buffer.numCollisionCapsules; i++)
	{
		float4 index = g_buffer.collisionCapsuleIndex[i]; 
		int start	= index.x;
		int end		= index.y;

		float4	sphereStart = g_buffer.collisionSpheres[start];
		float4	sphereEnd   = g_buffer.collisionSpheres[end];

		float3 capsuleDir = sphereEnd.xyz - sphereStart.xyz;
		float capsuleSq = dot(capsuleDir, capsuleDir);

		float3 dir = particle.xyz - sphereStart.xyz;
		float t = dot(dir, capsuleDir) / capsuleSq;

		if (t < 0)
			continue;
		if (t > 1.0f) 
			continue;

		float4 sphere = (1.0f - t) * sphereStart + t * sphereEnd;

		ApplySphereConstraint(particle, sphere.xyz, sphere.w);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply friction
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyFriction(inout float4 prevParticle, float4 curParticle, float4 impulse, float frictionScale)
{
	float impulseLength = frictionScale * length(impulse.xyz);
	float friction = saturate(impulseLength);

	prevParticle.xyz  = lerp(prevParticle.xyz, curParticle.xyz, friction);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//add forces and do verlet integration
float4 addForcesAndIntegrate(
	int			hairId, 
	float		massScale,
	float		damping, 
	float4		restPosition, 
	float4		position, 
	float4		oldPosition)
{  
    float4 outputPos = position;   
	
    if(position.w == 0)
    { 
        //if this is a root, use animated position by the transform of the scalp
        outputPos.xyz = restPosition.xyz;
        
        return outputPos; 
    }       
     	
    float3 velocity = (1.0 - damping) * (position.xyz - oldPosition.xyz);
        
    //Gravity------------------------------------------------------------------------
    float3 force = massScale * g_buffer.gravity.xyz;
	
	// Wind --------------------------------------------------------------
	// just use large prime numbers to hash position into noise look up table
	float posHash = 10571.0f * position.x + 794811.0f * position.y + 30651 * position.z;
	float windStrength = length(g_buffer.wind);
	float3 wind =  g_buffer.wind + windStrength * g_buffer.windNoise * g_windNoiseLut.Load((int)posHash & LUT_SIZE_MINUS_ONE);

	force = force + wind;

    //verlet integration----------------------------------------------------------------
	float timeStep = g_buffer.timeStep;
    outputPos.xyz = position.xyz
                    + velocity.xyz
                    + force * timeStep * timeStep; 
    
    return outputPos;    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply dual quaternion skinning
void computeSkinningDq(uint hairId, inout float4 skinnedPosition, inout float3 skinnedNormal, inout float3 skinnedTangent, 
	float4 restPosition, float3 restNormal, float3 restTangent)
{
	float4 boneIndex  = g_boneIndices.Load(hairId);
	float4 boneWeight = g_boneWeights.Load(hairId);

	DualQuaternion dq;
	dq.q0 = float4(0,0,0,0);
	dq.q1 = float4(0,0,0,0);

	[unroll(4)]
	for (int b = 0; b < 4; b++)
	{
		float w = boneWeight[b];
		DualQuaternion skinDq = g_buffer.skinDqs[boneIndex[b]];

		skinDq.q0 *= w;
		skinDq.q1 *= w;

		// hemispherization
		float sign = (dot(dq.q0, skinDq.q0) < -FLT_EPSILON) ? -1.0f: 1.0f;

		dq.q0 += sign * skinDq.q0;
		dq.q1 += sign * skinDq.q1;
	}

	// normalize
	float mag = dot(dq.q0, dq.q0);
	float deLen = 1.0f / sqrt(mag+FLT_EPSILON);
	dq.q0 *= deLen;
	dq.q1 *= deLen;

	// transform
	float3 d0 = dq.q0.xyz;
	float3 de = dq.q1.xyz;
	float a0 = dq.q0.w;
	float ae = dq.q1.w;

	float3 tempPos		= cross(d0, restPosition.xyz) + a0 * restPosition.xyz;
	float3 tempPos2		= 2.0f * (de * a0 - d0 * ae + cross(d0, de));
	float3 tempNormal	= cross(d0, restNormal.xyz) + a0 * restNormal.xyz;
	float3 tempTangent	= cross(d0, restTangent.xyz) + a0 * restTangent.xyz;
			
	skinnedPosition.xyz = restPosition.xyz + tempPos2 + cross(2.0f * d0, tempPos);
	skinnedNormal.xyz	= restNormal.xyz + 2.0 * cross( d0, tempNormal);
	skinnedTangent.xyz	= restTangent.xyz + 2.0 * cross( d0, tempTangent);

	skinnedNormal		= normalize(skinnedNormal);
	skinnedTangent		= normalize(skinnedTangent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply linear blending based skinning
void computeSkinningLinear(uint hairId, inout float4 skinnedPosition, inout float3 skinnedNormal, inout float3 skinnedTangent, 
	float4 restPosition, float3 restNormal, float3 restTangent)
{
	float4 boneIndex  = g_boneIndices.Load(hairId);
	float4 boneWeight = g_boneWeights.Load(hairId);

	float weightSum = boneWeight.x + boneWeight.y + boneWeight.z + boneWeight.w ;
	float invWeightSum = 1.0f / (weightSum + FLT_EPSILON);

	skinnedPosition.xyz = float3(0,0,0);
	skinnedNormal.xyz = float3(0,0,0);
	skinnedTangent.xyz = float3(0,0,0);

	[unroll(4)]
	for (int b = 0; b < 4; b++)
	{
		row_major float4x4 skin = g_buffer.skinMatrices[boneIndex[b]];
		float3 p = boneWeight[b] * invWeightSum * (mul(float4(restPosition.xyz,1), skin)).xyz;
		skinnedPosition.xyz += p;
		float3 n = boneWeight[b] * invWeightSum * (mul(float4(restNormal.xyz,0), skin)).xyz;
		skinnedNormal.xyz += n;
		float3 t = boneWeight[b] * invWeightSum * (mul(float4(restTangent.xyz,0), skin)).xyz;
		skinnedTangent.xyz += t;
	}

	skinnedNormal		= normalize(skinnedNormal);
	skinnedTangent		= normalize(skinnedTangent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply intertia related constraints
void applyInertia(
	float inertiaScale,
	float3 rootPos, float3 prevRootPos,
	inout float4 prevPos, inout float4 currentPos, inout float4 nextPos)
{
	float3 rootDelta	= rootPos.xyz - prevRootPos.xyz;

	if (g_buffer.lockInertia)
	{
		currentPos.xyz += rootDelta.xyz;
		nextPos.xyz = currentPos.xyz;
		prevPos.xyz = currentPos.xyz;
	}
	else
	{
		rootDelta *= clamp(1.0f - inertiaScale, 0.0f, 1.0f);

		prevPos.xyz += rootDelta.xyz;
		currentPos.xyz += rootDelta.xyz;
		nextPos.xyz = currentPos.xyz;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// prepare target positions for bending constraints
void prepareHairTargetPos(
	uint globalCvIndex, 
	float4 currentHairFrame, 
	out float3 targetPosPrev, out float3 targetPosNext)
{
	// local position offset from nearby cvs w.r.t this cv
	float3 localPosPrev		= g_masterLocalPosPrev[globalCvIndex].xyz;
	float3 localPosNext		= g_masterLocalPosNext[globalCvIndex].xyz;
		
	// apply global transform to get positions in world space
	targetPosPrev			= gfsdk_rotate(currentHairFrame, localPosPrev);
	targetPosNext			= gfsdk_rotate(currentHairFrame, localPosNext);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// shared memory
groupshared float4 sharedPos[NV_HAIR_BLOCK_SIZE_SIMULATE];				// particle positions
groupshared float4 sharedRestPos[NV_HAIR_BLOCK_SIZE_SIMULATE];				// original particle positions
groupshared float sharedConstraintDistance[NV_HAIR_BLOCK_SIZE_SIMULATE]; // rest length for distance constraint
groupshared float sharedRootDistance[NV_HAIR_BLOCK_SIZE_SIMULATE];		// rest distance for LRA constraint
groupshared float sharedNormalizedLength[NV_HAIR_BLOCK_SIZE_SIMULATE];	// actual hair length up to each cv for stiffness scaling

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply bending constraint
void applyBendingConstraint(
	uint threadId, uint n,
	float3 targetPosPrev, float3 targetPosNext,
	float scale )
{
	bool firstCv = (threadId == 0);	// is this first (root) cv?
	bool lastCv  = (threadId == (n-1)); // is this last cv?

	int prevCv = firstCv ? threadId : threadId - 1;
	int nextCv = lastCv ? threadId : threadId + 1;

	float4 p		= sharedPos[threadId];
	float4 prevp	= sharedPos[prevCv];
	float4 nextp	= sharedPos[nextCv];

	float3 targetPrev = p + targetPosPrev;
	float3 targetNext = p + targetPosNext;

	float3 deltaPrev = 0.5f * (targetPrev - prevp.xyz);
	float3 deltaNext = 0.5f * (targetNext - nextp.xyz);
	float3 deltaP = -1.0f * (deltaPrev + deltaNext);

	p.xyz += p.w * scale * deltaP;
	prevp.xyz += prevp.w * scale * deltaPrev;
	nextp.xyz += nextp.w * scale * deltaNext;

	sharedPos[threadId] = p;
	sharedPos[prevCv] = prevp;
	sharedPos[nextCv] = nextp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// main compute shader for hair simulation
// each group computes position of cvs of a single hair strand.
// each thread mostly maps to each cv within the hair
// So, in many cases (but not all), threadId = cv id, where threadId = 0 --> hair root
///////////////////////////////////////////////////////////////////////////////////////////////////////////
[numthreads(NV_HAIR_BLOCK_SIZE_SIMULATE,1,1)]
void cs_main(uint threadId        : SV_GroupIndex,
             uint3 groupId        : SV_GroupID,
             uint3 globalThreadId : SV_DispatchThreadID)
{   
	// each thread group = each hair
	uint hairId = groupId.x;

	// global index for this hair's root
    uint globalRootIndex = (hairId == 0) ? 0 :g_simulationStrandOffsets.Load( hairId - 1 );
	
	// number of particles in this hair
    int n = g_simulationStrandOffsets.Load( hairId ) - globalRootIndex;

	// classifiers for thread. Each thread maps to each simulated Cv.
	int		globalCvIndex = globalRootIndex + threadId;  // global index to per hair cv attribute buffers
	bool	firstCv = (threadId == 0);	// is this first (root) cv?
	bool	lastCv  = (threadId == (n-1)); // is this last cv?
	bool	validCv = (threadId < n); // is this thread (cv id) valid?
	int		prevCv = firstCv ? threadId : threadId - 1;
	int		nextCv = lastCv ? threadId : threadId + 1;

	// we compute the nextPos at frame n+1 using info from current (n) and previous (n-1) frames.
	// prevPos (n-1) -> currentPos (n) -> nextPos(n+1)
    float4		currentPos	= float4(0,0,0,0);		// current particle position 
	float4		prevPos		= float4(0,0,0,0);		// previous particle position stored in previous frame
	float4		nextPos		= float4(0,0,0,0);		// modified position after simulation steps in this frame
	float4		interpPos   = float4(0,0,0,0);		// The interpolate position (the uncorrected position on last frame)

	float4		restPos		= float4(0,0,0,0);		// rest position in bind pose
	float4		skinnedPos  = float4(0,0,0,0);		// skinned position accounting for bone based skinning

	////////////////////////////////////////////////////////////////////////////////////////////
    // read the particle attributes into registers and shared memory (global mem load)
	////////////////////////////////////////////////////////////////////////////////////////////
    if (validCv)
    {
		prevPos								= g_previousParticlePositions[globalCvIndex];
		currentPos							= g_particlePositions[globalCvIndex];
		nextPos								= currentPos;
		restPos								= g_restMasterStrand.Load(globalCvIndex);    
		skinnedPos							= float4(0,0,0, restPos.w);
		interpPos							= currentPos;

		sharedPos[threadId]					= currentPos;
		sharedRestPos[threadId]				= restPos;
        sharedConstraintDistance[threadId]	= g_particleDistanceConstraintLengths[globalCvIndex]; 
        sharedRootDistance[threadId]		= g_particleRootDistances[globalCvIndex];
		sharedNormalizedLength[threadId]	= g_normalizedLength[globalCvIndex];
    }

    //synchronize after reading the data into shared memory 
	GroupMemoryBarrierWithGroupSync();

	// copy previous root position for inerita computation
	float4 prevRootPos = sharedPos[0];
	GroupMemoryBarrierWithGroupSync();

	////////////////////////////////////////////////////////////////////////////////////////////
	// prepare GPU skinning inputs
	////////////////////////////////////////////////////////////////////////////////////////////
	float3 restMeshNormal		= g_growthMeshRestNormals.Load(hairId).xyz;
	float3 restMeshTangent		= g_growthMeshRestTangents.Load(hairId).xyz;
	float3 skinnedMeshNormal	= float3(0,0,0);
	float3 skinnedMeshTangent	= float3(0,0,0);

	////////////////////////////////////////////////////////////////////////////////////////////
	// apply GPU skinning
	////////////////////////////////////////////////////////////////////////////////////////////
	if (g_buffer.numBones == 0)
	{
		skinnedPos = restPos;
		skinnedMeshNormal = restMeshNormal;
		skinnedMeshTangent = restMeshTangent;
	}
	else
	{
		if (g_buffer.useDualQuaterinon)
			computeSkinningDq(hairId, skinnedPos, skinnedMeshNormal, skinnedMeshTangent, 
									restPos, restMeshNormal, restMeshTangent);
		else
			computeSkinningLinear(hairId, skinnedPos, skinnedMeshNormal, skinnedMeshTangent, 
									restPos, restMeshNormal, restMeshTangent);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// write back skinning data to the root and growth mesh vertex
	////////////////////////////////////////////////////////////////////////////////////////////
	if (firstCv)
	{
		g_growthMeshVertices[hairId]	= float4(skinnedPos.xyz, 0.0f);
		//g_GrowthMeshNormals[hairId]		= float4(skinnedMeshNormal.xyz, 0.0f);

		sharedPos[0] = skinnedPos;
	}
	GroupMemoryBarrierWithGroupSync();

	////////////////////////////////////////////////////////////////////////////////////////////
	// update hair frame references
	////////////////////////////////////////////////////////////////////////////////////////////
	float4 origRootFrame		= g_origFrames[globalRootIndex];
	float4 currentRootFrame		= gfsdk_quatFromAxis(skinnedMeshNormal, skinnedMeshTangent);
	float4 rootFrameTransfer	= gfsdk_transferFrame(origRootFrame, currentRootFrame);

	float4 originalHairFrame	= g_origFrames[globalCvIndex];	
	float4 skinnedHairFrame		= gfsdk_multiply(rootFrameTransfer, originalHairFrame);
	float3 skinnedHairTangent	= gfsdk_getBasisZ(skinnedHairFrame);
	float4 currentHairFrame		= float4(0,0,0,0);

	////////////////////////////////////////////////////////////////////////////////////////////
	// tex coord
	////////////////////////////////////////////////////////////////////////////////////////////
	float2	texCoord = float2(0.0f, 0.0f);
	if (g_buffer.useStiffnessTexture || g_buffer.useRootStiffnessTexture || g_buffer.useWeightTexture)
		texCoord = g_texCoords.Load(hairId);

	////////////////////////////////////////////////////////////////////////////////////////////
	// interpolate material
	////////////////////////////////////////////////////////////////////////////////////////////
	float materialWeight = g_buffer.materialWeight;

	if (g_buffer.useWeightTexture)
		materialWeight *= SAMPLE_RED(g_weightTexture, samLinear, texCoord, 0);

	NvHair_SimulationMaterial material;
	NvHair_BlendMaterial(g_buffer.defaultMaterial, g_buffer.targetMaterial, materialWeight, material);

	////////////////////////////////////////////////////////////////////////////////////////////
	// compute hair normal
	////////////////////////////////////////////////////////////////////////////////////////////
	float3 restHairNormal		= normalize(restPos - g_buffer.modelCenterRest);

	/////////////////////////////////////////////////////
	restHairNormal.z			= saturate(restHairNormal.z) + FLT_EPSILON; // clamp negative up-direction
	/////////////////////////////////////////////////////
	
	float3 restTangent			= normalize(sharedRestPos[nextCv] - sharedRestPos[prevCv]);
	float3 biNormal				= normalize(cross(restTangent, normalize(restHairNormal)));
	float3 flippedHairNormal	= normalize(cross(biNormal, restTangent));
	float3 skinnedHairNormal	= normalize(gfsdk_rotate(rootFrameTransfer, flippedHairNormal));

	float normalWeight = (firstCv) ? 0 : material.hairNormalWeight;
	float3 hairNormal = normalize(lerp(skinnedMeshNormal, skinnedHairNormal, normalWeight));

	////////////////////////////////////////////////////////////////////////////////////////////
	// if we are not simulating, write skinned position and exit
	////////////////////////////////////////////////////////////////////////////////////////////
	if (!g_buffer.simulate)
	{
		if ( validCv )
		{
			g_particlePositions[globalCvIndex]			= skinnedPos;
			g_interpParticlePositions[globalCvIndex]	= skinnedPos;
			g_previousParticlePositions[globalCvIndex]	= skinnedPos;
			g_skinnedParticlePositions[globalCvIndex]	= skinnedPos;
			g_frames[globalCvIndex]						= skinnedHairFrame;
			g_hairNormals[globalCvIndex].xyz			= hairNormal;
			
			sharedPos[threadId] = skinnedPos;
		}

		GroupMemoryBarrierWithGroupSync();

		if (validCv)
		{
			float4 pp = firstCv ? sharedPos[0] : sharedPos[threadId - 1];
			float4 np = lastCv ? sharedPos[threadId] : sharedPos[threadId + 1];
			g_hairTangents[globalCvIndex] = normalize(np-pp);
		}

		GroupMemoryBarrierWithGroupSync();

		return;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// apply intertia constraint
	////////////////////////////////////////////////////////////////////////////////////////////
	if ( validCv && !firstCv )
		applyInertia( material.inertiaScale, sharedPos[0].xyz, prevRootPos.xyz, prevPos, currentPos, nextPos);

	GroupMemoryBarrierWithGroupSync();

	////////////////////////////////////////////////////////////////////////////////////////////
	// prepare materials
	////////////////////////////////////////////////////////////////////////////////////////////
	int		numIterations	= g_buffer.numConstraintIterations;
	float	distRatio		= sharedNormalizedLength[threadId]; 

	distRatio = float(threadId) / float(n-1);

	float	distGlobalRatio = min(1.0f, sharedRootDistance[threadId] / (g_buffer.maxHairLength + 0.0001f));

	// sample stiffnes values from textures
	float stiffness = material.stiffness;
	if (g_buffer.useStiffnessTexture)
	{
		float stiffnessSample = 1.0f;
		NV_HAIR_SAMPLE_CHANNEL(g_stiffnessTexture, samLinear, texCoord, 0, g_buffer.stiffnessChannel, stiffnessSample);
		stiffness *= stiffnessSample;
	}

	// root and tip stiffness
	float rootStiffness = material.rootStiffness;
	if (g_buffer.useRootStiffnessTexture)
	{
		float stiffnessSample = 1.0f;
		NV_HAIR_SAMPLE_CHANNEL(g_rootStiffnessTexture, samLinear, texCoord, 0, g_buffer.rootStiffnessChannel, stiffnessSample);
		rootStiffness *= stiffnessSample;
	}

	// stiffness curve
	stiffness *= gfsdk_splineInterpolate(material.stiffnessCurve, distRatio);

	// stiffness strength
	float stiffnessStrength = material.stiffnessStrength;
	stiffnessStrength *= gfsdk_splineInterpolate(material.stiffnessStrengthCurve, distRatio);

	float tipStiffness = material.tipStiffness;
	stiffness += rootStiffness * saturate(1.0 - distGlobalRatio);
	stiffness += tipStiffness * distRatio;

	float rootDistance = sharedRootDistance[threadId];

	stiffness = saturate(stiffness);

	// max distance from stiffness
	float maxDistance = 2.0f * (1.0 - stiffness) * rootDistance;

	// we want stiffness to be normalized for multiple iterations
	stiffnessStrength = saturate(pow(stiffnessStrength, numIterations));

	// damping
	float stiffnessDamping = material.stiffnessDamping;
	stiffnessDamping *= gfsdk_splineInterpolate(material.stiffnessDampingCurve, distRatio);

	// bend stiffness
	float bendStiffness = 0.9f * material.bendStiffness; // scale bend stiffness down a bit to avoid overshoot

	bendStiffness *= saturate(gfsdk_splineInterpolate(material.bendStiffnessCurve, distRatio));
	bendStiffness = saturate(pow(bendStiffness, numIterations));

	// pin stiffness adjustment
	//float pinStiffness = pow(material.pinStiffness, g_buffer.numConstraintIterations);

	// backstop radius and sphere center
	float	backStopRadius	= g_buffer.maxHairLength * material.backStopRadius;
	float3	backStopSpherePosition	= skinnedPos.xyz - skinnedMeshNormal.xyz * backStopRadius;
	
	float3 rootDelta = sharedPos[0].xyz - prevRootPos.xyz;

	////////////////////////////////////////////////////////////////////////////////////////////
	// add forces and integrate
	////////////////////////////////////////////////////////////////////////////////////////////
    if(validCv)
	{
		float damping = material.damping * (g_buffer.timeStep * 60.0f);
		float massScale = material.massScale;

		sharedPos[threadId] = addForcesAndIntegrate(hairId, massScale, damping, skinnedPos, nextPos, prevPos); 
	}
    GroupMemoryBarrierWithGroupSync();

	////////////////////////////////////////////////////////////////////////////////////////////
	//  conditions for Red-Black(-Green) iteration of overlapping constraints
	////////////////////////////////////////////////////////////////////////////////////////////
	int half = floor(n / 2.0f);
    int half2 = floor((n - 1) / 2.0f); 

	bool doBending = (bendStiffness > 0) && validCv;
	bool bendFirst = doBending && ((threadId % 3) == 0);
	bool bendSecond = doBending && ((threadId % 3) == 1);
	bool bendThird = doBending && ((threadId % 3) == 2);

	float interactionStiffness = NV_HAIR_LERP_SIMULATION_PARAM(g_buffer, interactionStiffness);
	////////////////////////////////////////////////////////////////////////////////////////////
    //iterate through the constraints for a specified number of times
	////////////////////////////////////////////////////////////////////////////////////////////
    for(int iteration=0; iteration < numIterations; iteration++)
    {
		////////////////////////////////////////////////////////////////////////////////////////////
		// Standard distance constraint (Red-Black iteration)
		////////////////////////////////////////////////////////////////////////////////////////////
        //apply distance constraints to first subset
        if (threadId < half)
            DistanceConstraint(sharedPos[threadId*2],sharedPos[threadId*2+1],sharedConstraintDistance[threadId*2].x);
        GroupMemoryBarrierWithGroupSync();

        //apply distance constraints to second subset
        if (threadId < half2)
            DistanceConstraint(sharedPos[threadId*2+1],sharedPos[threadId*2+2],sharedConstraintDistance[threadId*2+1].x);
        GroupMemoryBarrierWithGroupSync();

		////////////////////////////////////////////////////////////////////////////////////////////
		// apply bending constraints
		////////////////////////////////////////////////////////////////////////////////////////////
		float3 targetPosPrev = float3(0.0f, 0.0f, 0.0f);
		float3 targetPosNext = float3(0.0f, 0.0f, 0.0f);

		if (doBending)
		{
			float3 hairTangent = normalize(sharedPos[nextCv] - sharedPos[prevCv]);
			float4 rot	= gfsdk_rotateBetween(skinnedHairTangent, hairTangent);
			
			if (!firstCv)
				currentHairFrame = gfsdk_multiply(rot, skinnedHairFrame);
			else
				currentHairFrame = skinnedHairFrame;

			// blend from root to tip
			currentHairFrame = lerp(skinnedHairFrame, currentHairFrame, distRatio);
			//currentHairFrame = lerp(skinnedHairFrame, currentHairFrame, 0);
			
			currentHairFrame = normalize(currentHairFrame);

			prepareHairTargetPos(globalCvIndex, currentHairFrame, targetPosPrev, targetPosNext);
		}
		GroupMemoryBarrierWithGroupSync();

		if (bendFirst)
			applyBendingConstraint(threadId, n, targetPosPrev, targetPosNext, bendStiffness);
		GroupMemoryBarrierWithGroupSync();

		if (bendSecond)
			applyBendingConstraint(threadId, n, targetPosPrev, targetPosNext, bendStiffness);
		GroupMemoryBarrierWithGroupSync();

		if (bendThird)
			applyBendingConstraint(threadId, n, targetPosPrev, targetPosNext, bendStiffness);
		GroupMemoryBarrierWithGroupSync();

		////////////////////////////////////////////////////////////////////////////////////////////
		// hair anti stretch constraint (Long Range Attachment)
		////////////////////////////////////////////////////////////////////////////////////////////
        LRAConstraint(sharedPos[threadId], sharedPos[0], sharedRootDistance[threadId]);          
	    GroupMemoryBarrierWithGroupSync();
		 
		////////////////////////////////////////////////////////////////////////////////////////////
		// global distance/spring constraint
		////////////////////////////////////////////////////////////////////////////////////////////
		ApplyGlobalConstraint(rootDelta, currentPos, sharedPos[threadId], skinnedPos, maxDistance, stiffnessStrength, stiffnessDamping);
		GroupMemoryBarrierWithGroupSync();

		//////////////////////////////////////////////////////////////////////////////////////////////
		//// backstop for simple body/hair collision
		//////////////////////////////////////////////////////////////////////////////////////////////
		if (backStopRadius > 0.0f)
			ApplySphereConstraint(sharedPos[threadId], backStopSpherePosition, backStopRadius);

		GroupMemoryBarrierWithGroupSync();

		////////////////////////////////////////////////////////////////////////////////////////////
		// apply collision for capsule sphere
		////////////////////////////////////////////////////////////////////////////////////////////
		float4 positionBeforeCollision = sharedPos[threadId];

		if (g_buffer.useCollision)
		{
			ApplySphereCollision(sharedPos[threadId], skinnedPos);
			ApplyCapsuleCollision(sharedPos[threadId]);
		}

		GroupMemoryBarrierWithGroupSync();

		////////////////////////////////////////////////////////////////////////////////////////////
		// friction handling
		////////////////////////////////////////////////////////////////////////////////////////////
		if (material.friction > 0)
		{
			float4 impulse = sharedPos[threadId] - positionBeforeCollision;
			ApplyFriction(currentPos, sharedPos[threadId], impulse, material.friction);
		}
		GroupMemoryBarrierWithGroupSync();
	}                
                 
	////////////////////////////////////////////////////////////////////////////////////////////
    //finally write back the data to the global buffer
	////////////////////////////////////////////////////////////////////////////////////////////
    if (validCv)
    {
		g_particlePositions[globalCvIndex]			= sharedPos[threadId];
        g_previousParticlePositions[globalCvIndex]	= currentPos;
		g_skinnedParticlePositions[globalCvIndex]	= skinnedPos;

		// Save the interp pos
		g_interpParticlePositions[globalCvIndex]	= interpPos;

		g_frames[globalCvIndex]						= currentHairFrame;
		g_hairNormals[globalCvIndex].xyz			= hairNormal;

		float4 pp = firstCv ? sharedPos[0] : sharedPos[threadId - 1];
		float4 np = lastCv ? sharedPos[threadId] : sharedPos[threadId + 1];
		g_hairTangents[globalCvIndex] = normalize(np - pp);
    }             
}
