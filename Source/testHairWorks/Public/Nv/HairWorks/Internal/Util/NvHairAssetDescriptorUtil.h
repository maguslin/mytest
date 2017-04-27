/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_ASSET_DESCRIPTOR_UTIL_H
#define NV_HAIR_ASSET_DESCRIPTOR_UTIL_H

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/Common/NvCoCommon.h>

namespace nvidia {
namespace HairWorks {

struct AssetDescriptorUtil
{
	typedef AssetDescriptor Descriptor;
	typedef ConversionSettings ConversionSettings;
	typedef AssetCopySettings CopySettings;

		/// Checks if desc seems valid, writes findings to the log
	static Result check(const Descriptor& desc, NvCo::Logger* log);
		/// Convert the units of desc
	static Result convertUnits(Descriptor& desc, Float sceneUnit);
		/// Calculate a conversion matrix
	static Void calcConversionMatrix(EAxisHint sourceUpAxisHint, EHandednessHint sourceHandednessHint, EAxisHint targetUpAxisHint, EHandednessHint targetHandednessHint, gfsdk_float4x4& matOut);
		/// Transform inplace desc
	static Void transform(const gfsdk_float4x4& mat, Bool flipWinding, Descriptor& desc);
		/// Convert inplace using the settings
	static Void convert(const ConversionSettings& settings, Descriptor& desc);

		/// Linearly resample a curve defined as src points
		/// normalizedSrcLengthsOut[srcPoints + 1] holds the length of the source points, normalized into 0 - 1 range (1 being end of hair). 
		/// NOTE! If all src points are in the same place they are just copied.
	static void resampleCurve(const gfsdk_float3* srcPoints, Int numSrcPoints, Float* normalizedSrcLengthsOut, gfsdk_float3* targetPoints, Int numTargetPoints);
		/// Resample guide hairs such that they have numPointsPerHair vertices.
		/// verticesOut must be equal in size to numGuideHairs * numPointerPerHair
		/// endIndicesInOut holds the index for each hair. It will be modified .
	static void resampleGuideHairs(const gfsdk_float3* vertices, Int numGuideHairs, Int numPointsPerHair, UInt32* endIndicesInOut, gfsdk_float3* verticesOut);

		/// Calculate the maximum num points from the end indices
	static Int calcMaxNumPointsPerHair(const UInt32* endIndices, Int numGuideHairs);

		/// Inplace bone remapping. There must be the same or less bones than in the original asset.
	static Result remapBones(Descriptor& desc, const Char*const* boneNames, Int numBones);

		/// Calculates a copy using settings. NOTE! It does not actually copy any data. It will just set the dst
		/// to use the same pointer as in the src if a copy is needed. 
		/// Thus typically memory for the destination needs to be allocated and the data copied over.
		/// Just assigning to a MemoryHairAssetDescriptor will do a full memory copy.
	static Void calcCopy(const Descriptor& srcDesc, const CopySettings& settings, Descriptor& dstDesc);

		/// Deallocates, and sets to NV_NULL any pointers using Nv::deallocatePodArray
		/// Generally this is not used, as it is better to hold in MemoryHairAssetDescriptor that holds all memory in single chunk
		/// But serializer doesn't use this, so this is handy for deallocating serialized in structs
	static Void deallocate(Descriptor& desc);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_ASSET_DESCRIPTOR_UTIL_H
