/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/HairWorks/Internal/Shader/NvHairSpline.h>

cbuffer cbPerFrame : register( b0 )
{
	NvHair_SplineConstantBuffer g_buffer;
}

StructuredBuffer<float4>	g_masterStrandSb		: register(t0);
StructuredBuffer<float4>	g_masterStrandPrevSb	: register(t1);
StructuredBuffer<float4>	g_deltaParticlePos		: register(t2); // Delta particle pos
StructuredBuffer<float4>	g_masterStrandNormals	: register(t3);

//////////////////////////////////////////////////////////////////////////
struct GSOut
{
    float4 position : POSITION;
    float4 tangent	: TANGENT;
	float4 normal	: NORMAL;
};

//////////////////////////////////////////////////////////////////////////
// geometry shader for splining control hairs
//////////////////////////////////////////////////////////////////////////
[MaxVertexCount(1)]
void gs_main(point VSOut input[1], inout PointStream<GSOut> inStream )
{ 
    GSOut output = (GSOut)0;

	// hash input vertex id into various indices needed to compute spline factors   
	int vertexId				= input[0].vertexId;
	int numVertsPerSegments		= g_buffer.numVertsPerSegments;
	int	numMasterSegments		= g_buffer.numMasterSegments;
	int	numMasterCvs			= numMasterSegments + 1;
	int strandPointCounts		= g_buffer.strandPointCounts;

	int hairId					= vertexId / strandPointCounts;
	int strandVertexId			= vertexId - hairId * strandPointCounts;

	int segmentId				= strandVertexId / numVertsPerSegments;
	bool lastVertex				= (strandVertexId == (strandPointCounts - 1));

	if (lastVertex)
		segmentId = numMasterSegments - 1;

	bool firstSegment			= segmentId == 0;
	bool lastSegment			= segmentId >= (numMasterSegments - 1);
	
	int segmentVertexIndex		= strandVertexId - segmentId * numVertsPerSegments;
	float u						= segmentVertexIndex / float(numVertsPerSegments);
	int controlVertexBase		= hairId * numMasterCvs;

    // Catmull-Rom basis
    float4x4 basisMatrix = float4x4
    (
        -0.5,	1.5,   -1.5,	0.5,
		 1.0,  -2.5,	2.0,   -0.5,
		-0.5,	0.0,	0.5,	0.0,
		 0.0,	1.0,	0.0,	0.0
    );

    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);

	// control vertices and normals
	int4 cvId;

	cvId[0] = controlVertexBase + (firstSegment ? segmentId : segmentId - 1);
	cvId[1] = controlVertexBase + segmentId;
	cvId[2] = controlVertexBase + segmentId + 1;
	cvId[3] = controlVertexBase + (lastSegment ? (segmentId + 1) : (segmentId + 2));

	/////////////////////////////////////////////////////////////////////////////////
	// position
	/////////////////////////////////////////////////////////////////////////////////

	float4 position = float4(0, 0, 0, 0);
	float4x4 controlVertices;

	float interp = g_buffer.simulationInterp;
	if (interp >= 1.0f)
	{
		controlVertices[0] = g_masterStrandSb[cvId[0]];
		controlVertices[1] = g_masterStrandSb[cvId[1]];
		controlVertices[2] = g_masterStrandSb[cvId[2]];
		controlVertices[3] = g_masterStrandSb[cvId[3]];
	}
	else
	{
		// Add the delta, to the position (can just set because we know it's zero at this point)
		position = g_deltaParticlePos[controlVertexBase];

		// Lerp the control vertices between old and new
		controlVertices[0] = lerp(g_masterStrandPrevSb[cvId[0]], g_masterStrandSb[cvId[0]], interp);
		controlVertices[1] = lerp(g_masterStrandPrevSb[cvId[1]], g_masterStrandSb[cvId[1]], interp);
		controlVertices[2] = lerp(g_masterStrandPrevSb[cvId[2]], g_masterStrandSb[cvId[2]], interp);
		controlVertices[3] = lerp(g_masterStrandPrevSb[cvId[3]], g_masterStrandSb[cvId[3]], interp);
	}

	[unroll] for (int c = 0; c < 4; ++c)
		position += basis[c] * controlVertices[c];

	/////////////////////////////////////////////////////////////////////////////////
	// normal
	/////////////////////////////////////////////////////////////////////////////////

	float4x4 controlNormals;

	controlNormals[0] = g_masterStrandNormals[cvId[0]];
	controlNormals[1] = g_masterStrandNormals[cvId[1]];
	controlNormals[2] = g_masterStrandNormals[cvId[2]];
	controlNormals[3] = g_masterStrandNormals[cvId[3]];

	float4 normal = float4(0,0,0,0);
    [unroll] for (int n = 0; n < 4; ++n) 
        normal += basis[n] * controlNormals[n];

	/////////////////////////////////////////////////////////////////////////////////
    // tangents
	/////////////////////////////////////////////////////////////////////////////////
    float3x3 basisMatrixQuadratic = float3x3
    (
        0.5, -1.0, 0.5,
       -1.0,  1.0, 0.0,
        0.5,  0.5, 0.0
    );    
    
    float3 basisTangents  = mul(float3(u * u, u, 1), basisMatrixQuadratic);
    
    const float3 controlTangents[3] = 
    {
        float3(controlVertices[1].xyz - controlVertices[0].xyz),
        float3(controlVertices[2].xyz - controlVertices[1].xyz),
        float3(controlVertices[3].xyz - controlVertices[2].xyz),        
    };
    
	float4 tangent = float4(0,0,0,0);
    [unroll] for (c = 0; c < 3; ++c) 
        tangent.xyz += basisTangents[c] * controlTangents[c];

	/////////////////////////////////////////////////////////////////////////////////
	// apply world space transformation
	/////////////////////////////////////////////////////////////////////////////////
	position.xyz	=	mul(float4(position.xyz, 1),	g_buffer.modelToWorld).xyz; 
	tangent.xyz		=	mul(float4(tangent.xyz, 0),		g_buffer.modelToWorld).xyz; 
	normal.xyz		=	mul(float4(normal.xyz, 0),		g_buffer.modelToWorld).xyz; 

	/////////////////////////////////////////////////////////////////////////////////
	// write to streamout results
	/////////////////////////////////////////////////////////////////////////////////
	output.position.xyz = position.xyz;
	output.tangent = tangent;
	output.normal = normal;
      
    inStream.Append(output);  

}

