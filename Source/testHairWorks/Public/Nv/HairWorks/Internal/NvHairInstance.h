/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_INSTANCE_H
#define NV_HAIR_INSTANCE_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/Container/NvCoPodBuffer.h>

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/Internal/NvHairApiAsset.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/HairWorks/Internal/NvHairAsset.h>
#include <Nv/HairWorks/Internal/NvHairLodInfo.h>

namespace nvidia {
namespace HairWorks { 

class LineRenderer;

class Instance
{
	NV_CO_DECLARE_CLASS_BASE(Instance);

	enum { MAX_VERTS_PER_SEGMENT = 4 };

	/// Get default material
	NV_FORCE_INLINE const InstanceDescriptor& getDefaultMaterial() const { return m_materials[0]; }

	/// 
	NV_FORCE_INLINE ETextureChannel getCurrentTextureChannel(ETextureType type) const { return m_materials[0].m_textureChannels[Int(type)]; }
	NV_FORCE_INLINE ETextureChannel getDefaultTextureChannel(ETextureType type) const { return m_materials[0].m_textureChannels[Int(type)]; }

	Int getStrandPointCount() const;
	Int getMaxStrandPointCount() const;

	Int getNumTessellatedPoints() const;
	Int getMaxTessellatedPoints() const;

		/// Set the number of verts per segment for tesselation
	Void setNumVertsPerSegment(Int numVerts);

		/// Set that it is active (it is active if it has been rendered, in this frame)
	NV_FORCE_INLINE Void setActive(Bool isActive) { m_isActive = isActive; }
		/// True if 'active' - as in it is being rendered
	NV_FORCE_INLINE Bool isActive() const { return m_isActive; }

		/// Get the bounds
	Void getBounds(gfsdk_float3& minOut, gfsdk_float3& maxOut, Bool growthMeshOnly) const;

	Void updateSkinningMatrices(const gfsdk_float4x4* boneMatrices);
	Void updateSkinningDqs(const gfsdk_dualquaternion* dqs);
	
		/// Draw the bones
	Void drawDebugBones(LineRenderer* renderer) const;
		/// Draw the capsules
	Void drawDebugCapsules(LineRenderer* renderer) const;
		/// Draw the bounding box
	Void drawDebugBoundingBox(LineRenderer* renderer) const;
		/// Draw shading bone
	Void drawDebugShadingBone(LineRenderer* renderer) const;
		/// Draw cull sphere
	Void drawDebugCullSphere(LineRenderer* renderer) const;

		/// Get the number of pins the instance is using
	Int getNumPins() const { return m_asset->getDesc().m_numPins; }

		/// Compute stats with densitySum
	Void computeStats(Float densitySum, Stats& statsOut) const;

		/// True if prerender will be performed on call
	Bool needsPreRender(Float simulationInterp) const { return !m_isTessellated || simulationInterp != m_simulationInterp; }
		/// Do preRender with specified simulationInterp
	Result preRender(Float simulationInterp);

		/// Default Ctor
	Instance(Asset* asset);
		/// Dtor
	~Instance();

protected:
	Void _updateShadingCenter();

	/// Update the skinning bounds
	Void _updateSkinnedBounds();

	Void _updateMatrixCollision();
	Void _updateDqCollision();

public:

	Bool m_isActive;

	const Asset* m_asset;						///< Points back to the asset that created this
	AssetId m_assetId;					///< Associated assetId

	InstanceId m_instanceId;				///< The instanceId for this instance

	InstanceDescriptor m_materials[Asset::MAX_MATERIALS];

	NvCo::UniquePtr<ApiInstance> m_apiInstance;

	ETeleportMode m_teleportMode;

	ShaderCacheEntry* m_cacheEntry;

	Int m_numVertsPerSegment;

	gfsdk_float3 m_skinnedCenter;			///< (m_modelBoundsMax + m_modelBoundsMin) * 0.5
	gfsdk_float3 m_skinnedBounds[2];		///< 0 is min, 1 is max. Done as an array to make m_bbIndices work.
	gfsdk_float3 m_prevSkinnedCenter;		///< The previous model center for last update

	// CPU buffer for skinning and simulation
	Bool m_useDqSkinning;				///< True if using dual quaternion skinning

	NvCo::PodBuffer<gfsdk_float4x4> m_skinningMatrices;		///< The input skinning matrices
	
	NvCo::PodBuffer<gfsdk_dualquaternion> m_boneDqs;			///< The bone dual quaternions
	NvCo::PodBuffer<gfsdk_float4x4> m_boneMatrices;			///< The bone matrices
	NvCo::PodBuffer<gfsdk_float4> m_collisionSpheres;			///< The collision spheres center in world space position and radius in w

	// shading bone data
	gfsdk_float3 m_shadingCenter;
	gfsdk_float3 m_shadingCenterRest;

	gfsdk_float4x4 m_modelToWorld;
	LodInfo m_lodInfo;

	Float m_simulationInterp;

	Bool m_isTessellated;
	Bool m_isSimulationStarted;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_INSTANCE_H
