/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairMemoryAssetDescriptor.h"

#include <Nv/HairWorks/Internal/Util/NvHairAssetDescriptorUtil.h>

namespace nvidia {
namespace HairWorks {

/* static */Void MemoryHairAssetDescriptor::calcLayout(const AssetDescriptor& desc, LayoutCalculator& calc)
{
	calc.clear();

	// NOTE! Must match LAYOUT enumeration
	calc.addArray(desc.m_vertices, desc.m_numVertices);		
	calc.addArray(desc.m_endIndices, desc.m_numGuideHairs);		
	calc.addArray(desc.m_faceIndices, desc.m_numFaces * 3);				
	calc.addArray(desc.m_faceUvs, desc.m_numFaces * 3);		

	calc.addArrayConditional((desc.m_numBones > 0), desc.m_boneIndices, desc.m_numGuideHairs);		
	calc.addArrayConditional((desc.m_numBones > 0), desc.m_boneWeights, desc.m_numGuideHairs);		

	calc.addArrayConditional((desc.m_boneNames && desc.m_numBones), desc.m_boneNames, desc.m_numBones * NV_HAIR_MAX_STRING); 
	calc.addArrayConditional((desc.m_bindPoses && desc.m_numBones > 0), desc.m_bindPoses, desc.m_numBones);	
	calc.addArrayConditional((desc.m_boneParents && desc.m_numBones), desc.m_boneParents, desc.m_numBones);		
	calc.addArrayConditional((desc.m_textureNames != NV_NULL), desc.m_textureNames, NV_HAIR_MAX_STRING * Int(TextureType::COUNT_OF));
}

Void MemoryHairAssetDescriptor::setCopy(const AssetDescriptor& desc, LayoutCalculator& calc)
{
	NV_CORE_ASSERT(this != &desc);
	// First just copy the variables over
	*(Parent*)this = desc;

	m_memory.setSize(calc.getSizeInBytes());
	UInt8* base = m_memory.begin();

	m_vertices = calc.copyAt(LAYOUT_VERTICES, base, desc.m_vertices, desc.m_numVertices);
	m_endIndices = calc.copyAt(LAYOUT_END_INDICES, base, desc.m_endIndices, desc.m_numGuideHairs);
	m_faceIndices = calc.copyAt(LAYOUT_FACE_INDICES, base, desc.m_faceIndices, desc.m_numFaces * 3);
	m_faceUvs = calc.copyAt(LAYOUT_FACE_UVS, base, desc.m_faceUvs, desc.m_numFaces * 3);

	m_boneIndices = calc.copyAt(LAYOUT_BONE_INDICES, base, desc.m_boneIndices, desc.m_numGuideHairs);
	m_boneWeights = calc.copyAt(LAYOUT_BONE_WEIGHTS, base, desc.m_boneWeights, desc.m_numGuideHairs);

	m_boneNames = calc.copyAt(LAYOUT_BONE_NAMES, base, desc.m_boneNames, desc.m_numBones * NV_HAIR_MAX_STRING);
	m_bindPoses = calc.copyAt(LAYOUT_BIND_POSES, base, desc.m_bindPoses, desc.m_numBones);
	m_boneParents = calc.copyAt(LAYOUT_BONE_PARENTS, base, desc.m_boneParents, desc.m_numBones);
	m_textureNames = calc.copyAt(LAYOUT_TEXTURE_NAMES, base, desc.m_textureNames, NV_HAIR_MAX_STRING * Int(TextureType::COUNT_OF));
}

MemoryHairAssetDescriptor& MemoryHairAssetDescriptor::operator=(const AssetDescriptor& rhs)
{
	if (&rhs == this)
	{
		return *this;
	}
	
	// Keep in scope the old buffer during copy
	// This means we can copy even when the src contains bits of memory in this. 
	// BUT! After the assignment the rhs will be pointing to freed memory!
	NvCo::PodBuffer<UInt8> memory;
	memory.swap(m_memory);

	// Do the copy
	LayoutCalculator calc;
	calcLayout(rhs, calc);
	setCopy(rhs, calc);

	return *this;
}

MemoryHairAssetDescriptor::MemoryHairAssetDescriptor(const AssetDescriptor& rhs):
	Parent(rhs)
{
	LayoutCalculator calc;
	calcLayout(*this, calc);
	setCopy(rhs, calc);
}

Void MemoryHairAssetDescriptor::resampleGuideHairs(Int numTargetPointsPerHair)
{
	Int  numTargetVertices = numTargetPointsPerHair * m_numGuideHairs;
	NvCo::PodBuffer<gfsdk_float3> vertices(numTargetVertices);
	AssetDescriptorUtil::resampleGuideHairs(m_vertices, m_numGuideHairs, numTargetPointsPerHair, m_endIndices, vertices);
	// Set the vertices to this
	m_vertices = vertices.begin();
	m_numVertices = numTargetVertices;
	// Create a new copy - to remove aliasing over the vertices array
	ThisType dst(*this);
	// Swap back in place
	swap(dst);
}

Void MemoryHairAssetDescriptor::setCopy(const AssetDescriptor& srcDesc, const CopySettings& settings)
{
	// Copy the current descriptor
	AssetDescriptor dst(*this);
	// Doing calcCopy optionally sets fields in dst depending on settings. (if not set will take original value). 
	AssetDescriptorUtil::calcCopy(srcDesc, settings, dst);
	// Set with the copied fields
	*this = dst;
}

} // namespace HairWorks
} // namespace nvidia
