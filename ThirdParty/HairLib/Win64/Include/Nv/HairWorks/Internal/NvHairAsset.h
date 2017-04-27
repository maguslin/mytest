/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_ASSET_H
#define NV_HAIR_ASSET_H

#include <Nv/HairWorks/Internal/NvHairInternal.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/Container/NvCoPodBuffer.h>

#include <Nv/HairWorks/NvHairSdk.h>

#include "NvHairMemoryAssetDescriptor.h"

#include "NvHairApiAsset.h"

namespace nvidia {
namespace HairWorks { 

class Asset
{
	NV_CO_DECLARE_CLASS_BASE(Asset);

	enum { MAX_MATERIALS = 4 };

		/// update other structures with the desc. Must be called if the desc is altered in any way.
	Result updateDescChanged(ApiGlobal* apiGlobal);

		/// Get the descriptor
	NV_FORCE_INLINE const AssetDescriptor& getDesc() const { return m_assetDesc; }
		/// Get writable desc... use with caution...
		/// As the other structures will need to be kept in sync. Call updateDescChanged to sync.
	NV_FORCE_INLINE MemoryHairAssetDescriptor& getWriteDesc(){ return m_assetDesc; }

		/// Get the amount of bones
	NV_FORCE_INLINE Int getNumBones() const { return m_assetDesc.m_numBones; }
		/// Get the bind pose
	NV_FORCE_INLINE const gfsdk_float4x4* getBindPoseMatrices() const { return m_assetDesc.m_bindPoses; }
		/// Get the bind pose parents
	NV_FORCE_INLINE const Int* getBindPoseParents() const { return m_assetDesc.m_boneParents; }
		/// Get the scaling in centimeters
	NV_FORCE_INLINE Float getUnitInCentimeters() const { return m_assetDesc.m_sceneUnit; }

		/// Calculate hair interaction buffer contents
		/// lengthsOut, and indicesOut length is equal to 6 * minCvs per hair (m_totalNumHairInteractions)
	Void calcHairInteractionBuffers(NvCo::PodBuffer<Int32>& offsetsOut, NvCo::PodBuffer<Float>& lengthsOut, NvCo::PodBuffer<Int32>& indicesOut) const;
		/// Calc texcoord buffer - one texture coordinate per master hair
		/// Function to calculate per hair texture coord to sample per hair attribute (e.g. stiffness) during compute
	Void calcTexCoordBuffer(NvCo::PodBuffer<gfsdk_float2>& texCoordsOut) const;
		/// Calculate frame buffers - ecah buffer is n
	Void calcFrameBuffers(NvCo::PodBuffer<gfsdk_float4>& framesOut, NvCo::PodBuffer<gfsdk_float4>& localPosPrevOut, NvCo::PodBuffer<gfsdk_float4>& localPosNextOut) const;

		/// Calculate total length of hair along master strands (output has m_numMasterStrandControlVertices elements)
	Void calcMasterStrandRootDistances(NvCo::PodBuffer<Float>& distancesOut) const;
			
		/// Maps a vertex index to the hair it belongs to
	Void calcVertexToHairMap(NvCo::PodBuffer<Int32>& vertexToHairMapOut) const;
		/// Calculate 
	Void calcMasterStrandIndices(NvCo::PodBuffer<UInt32>& indicesOut) const;
		
	Void calcFrameVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const;
	Void calcLocalPosVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const;
	Void calcNormalVisualizationIndices(NvCo::PodBuffer<UInt32>& indicesOut) const;

		/// Calculate the pin constraints
	Void calcPins(NvCo::PodBuffer<NvHair_Pin>& pins) const;

		/// Calculate bounds of skinned growth mesh
	Void calcSkinnedGrowthMeshBounds(const gfsdk_float4x4* skinningMatrices, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut) const;
	Void calcSkinnedGrowthMeshBounds(const gfsdk_dualquaternion* skinningDqs, gfsdk_float3& bbMinOut, gfsdk_float3& bbMaxOut) const;

		/// Ctor
	Asset():m_numInstances(0) {}

protected:
		/// Update the bounds
	Void _updateBounds();
		/// Update main features
	Result _updateMain();
	Void _calcNormalizedStrandLengths();
	Void _initCollisionCapsules();
	Void _calcGrowthBuffers();
	Void _calcMasterStrandTangentBuffer();


public:

	mutable Int m_numInstances;									///< Total number of instances using this asset

	InstanceDescriptor m_materials[MAX_MATERIALS];

	NvCo::PodBuffer<gfsdk_float4>	m_masterStrandOriginalControlVertices;	// inDesc.m_numVertices
	NvCo::PodBuffer<gfsdk_float4>	m_masterStrandControlVertices;			// inDesc.m_numVertices
	NvCo::PodBuffer<Int32>	m_masterStrandControlVertexOffsets;				// numGuideHairs
	NvCo::PodBuffer<gfsdk_float4> m_masterStrandTangents;					// m_numMasterStrandControlVertices

	NvCo::PodBuffer<UInt32>	m_faceIndices;								// numFaces * 3
	NvCo::PodBuffer<gfsdk_float2>	m_faceTexCoords;					// numFaces * 3
	NvCo::PodBuffer<gfsdk_float3>	m_faceHairIndices;					// numFaces

	NvCo::PodBuffer<gfsdk_float4>	m_boneIndices;						// numGuideHairs
	NvCo::PodBuffer<gfsdk_float4>	m_boneWeights;						// numGuideHairs

	NvCo::PodBuffer<UInt32>	m_rootIndices;								// numGuideHairs

	NvCo::PodBuffer<Float32> m_masterStrandNormalizedDistances;			/// Normalized distance along strands 0, at root to 1 at tip (m_numMasterStrandControlVertices)

	// optional mesh data for body->hair shadows and surface normal based shading
	NvCo::PodBuffer<gfsdk_float4> m_growthMeshVertices;
	NvCo::PodBuffer<gfsdk_float4> m_growthMeshNormals;
	NvCo::PodBuffer<gfsdk_float4> m_growthMeshTangents;

	NvCo::PodBuffer<gfsdk_float4> m_collisionCapsuleIndices;

	Int m_numMasterStrands;
	Int m_numMasterStrandControlVertices;

	Int m_numMasterSegmentsPerHair;
	Int m_totalNumMasterSegments;

	Int m_numFaces;

	Float m_maxHairLength;					///< Maximum length across all strands in model space

	gfsdk_float3 m_initialModelCenter;		///< (m_modelBoundsMax + m_modelBoundsMin) * 0.5
	gfsdk_float3 m_initialModelBounds[2];	///< 0 is min, 1 is max. Done as an array to make m_bbIndices work.
	Int m_initialBoundIndices[6];			///< Vertex index associated with each maximum found in m_model bounds  
	Float m_maxHairCenterDist;				///< Maximum distance of a hair to the model center

	Float m_modelSize;						///< Maximum distance along an axis of bounding box from the model center

	NvCo::UniquePtr<ApiAsset> m_apiAsset;		///< Api specific asset info

protected:
	MemoryHairAssetDescriptor m_assetDesc;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_ASSET_H
