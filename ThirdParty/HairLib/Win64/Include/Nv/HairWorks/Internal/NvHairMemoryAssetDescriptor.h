/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_MEMORY_ASSET_DESCRIPTOR_H
#define NV_HAIR_MEMORY_ASSET_DESCRIPTOR_H

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/Common/Container/NvCoPodBuffer.h>

#include "NvHairLayoutCalculator.h"

namespace nvidia {
namespace HairWorks {

/*! A hair asset descriptor that stores it's contents contiguously in a buffer */
struct MemoryHairAssetDescriptor: AssetDescriptor
{
	NV_CO_DECLARE_CLASS(MemoryHairAssetDescriptor, AssetDescriptor);
	typedef AssetCopySettings CopySettings;

	enum Layout
	{
		LAYOUT_VERTICES,
		LAYOUT_END_INDICES,
		LAYOUT_FACE_INDICES,
		LAYOUT_FACE_UVS,
		LAYOUT_BONE_INDICES,
		LAYOUT_BONE_WEIGHTS,
		LAYOUT_BONE_NAMES,
		LAYOUT_BIND_POSES,
		LAYOUT_BONE_PARENTS,
		LAYOUT_TEXTURE_NAMES,
		LAYOUT_COUNT_OF,
	};

		/// Set copy of data using layout calculator
	Void setCopy(const AssetDescriptor& desc, LayoutCalculator& calc);
		/// =
	ThisType& operator=(const AssetDescriptor& desc);

		/// Inplace swap
	Void swap(ThisType& rhs)
	{
		typedef AssetDescriptor Desc;
		Desc& rhsDesc = rhs;
		Desc& thisDesc = *this;
		NvCo::Op::swap(rhsDesc, thisDesc);
		m_memory.swap(rhs.m_memory);
	}

		/// Resample the guide hairs
	Void resampleGuideHairs(Int numTargetPointsPerHair);

		/// Set a copy from srcDesc, using copy settings 
	Void setCopy(const AssetDescriptor& srcDesc, const CopySettings& settings);

	MemoryHairAssetDescriptor() {}
		/// Construct
	MemoryHairAssetDescriptor(const AssetDescriptor& rhs);

	/// Calculate a layout in memory
	static Void calcLayout(const AssetDescriptor& desc, LayoutCalculator& calc);

	NvCo::PodBuffer<UInt8> m_memory;			///< This stores all of the members 
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_MEMORY_ASSET_DESCRIPTOR_H
