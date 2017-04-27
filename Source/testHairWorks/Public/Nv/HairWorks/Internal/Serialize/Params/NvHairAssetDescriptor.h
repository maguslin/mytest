// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

// This file was generated by NvParameterized/scripts/GenParameterized.pl
// Created: 2017.02.26 07:50:25

#ifndef HEADER_NvHairAssetDescriptor_h
#define HEADER_NvHairAssetDescriptor_h

#include "NvParametersTypes.h"

#ifndef NV_PARAMETERIZED_ONLY_LAYOUTS
#include "NvParameterized.h"
#include "NvParameters.h"
#include "NvParameterizedTraits.h"
#include "NvTraitsInternal.h"
#endif

namespace nvidia
{
namespace parameterized
{

#if NV_VC
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to __declspec(align())
#endif

namespace HairAssetDescriptorNS
{

struct BoneSphere_Type;
struct Pin_Type;

struct VEC3_DynamicArray1D_Type
{
	nvidia::NvVec3* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct U32_DynamicArray1D_Type
{
	uint32_t* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct VEC2_DynamicArray1D_Type
{
	nvidia::NvVec2* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct VEC4_DynamicArray1D_Type
{
	nvidia::NvVec4* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct U8_DynamicArray1D_Type
{
	uint8_t* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct STRING_DynamicArray1D_Type
{
	NvParameterized::DummyStringStruct* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct MAT44_DynamicArray1D_Type
{
	nvidia::NvMat44* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct I32_DynamicArray1D_Type
{
	int32_t* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct BoneSphere_DynamicArray1D_Type
{
	BoneSphere_Type* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct Pin_DynamicArray1D_Type
{
	Pin_Type* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct BoneSphere_Type
{
	int32_t boneSphereIndex;
	float boneSphereRadius;
	nvidia::NvVec3 boneSphereLocalPos;
};
struct Pin_Type
{
	int32_t boneSphereIndex;
	float boneSphereRadius;
	nvidia::NvVec3 boneSphereLocalPos;
	float pinStiffness;
	float influenceFallOff;
	bool useDynamicPin;
	bool doLra;
	bool useStiffnessPin;
	nvidia::NvVec4 influenceFallOffCurve;
};

struct ParametersStruct
{

	uint32_t numGuideHairs;
	uint32_t numVertices;
	VEC3_DynamicArray1D_Type vertices;
	U32_DynamicArray1D_Type endIndices;
	uint32_t numFaces;
	U32_DynamicArray1D_Type faceIndices;
	VEC2_DynamicArray1D_Type faceUVs;
	uint32_t numBones;
	VEC4_DynamicArray1D_Type boneIndices;
	VEC4_DynamicArray1D_Type boneWeights;
	U8_DynamicArray1D_Type boneNames;
	STRING_DynamicArray1D_Type boneNameList;
	MAT44_DynamicArray1D_Type bindPoses;
	I32_DynamicArray1D_Type boneParents;
	uint32_t numBoneSpheres;
	BoneSphere_DynamicArray1D_Type boneSpheres;
	uint32_t numBoneCapsules;
	U32_DynamicArray1D_Type boneCapsuleIndices;
	uint32_t numPinConstraints;
	Pin_DynamicArray1D_Type pinConstraints;
	float sceneUnit;
	uint32_t upAxis;
	uint32_t handedness;

};

static const uint32_t checksum[] = { 0xff338b78, 0x2c774cb6, 0xb32a7875, 0xe7fa6c34, };

} // namespace HairAssetDescriptorNS

#ifndef NV_PARAMETERIZED_ONLY_LAYOUTS
class HairAssetDescriptor : public NvParameterized::NvParameters, public HairAssetDescriptorNS::ParametersStruct
{
public:
	HairAssetDescriptor(NvParameterized::Traits* traits, void* buf = 0, int32_t* refCount = 0);

	virtual ~HairAssetDescriptor();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("HairAssetDescriptor");
	}

	const char* className(void) const
	{
		return(staticClassName());
	}

	static const uint32_t ClassVersion = ((uint32_t)1 << 16) + (uint32_t)1;

	static uint32_t staticVersion(void)
	{
		return ClassVersion;
	}

	uint32_t version(void) const
	{
		return(staticVersion());
	}

	static const uint32_t ClassAlignment = 8;

	static const uint32_t* staticChecksum(uint32_t& bits)
	{
		bits = 8 * sizeof(HairAssetDescriptorNS::checksum);
		return HairAssetDescriptorNS::checksum;
	}

	static void freeParameterDefinitionTable(NvParameterized::Traits* traits);

	const uint32_t* checksum(uint32_t& bits) const
	{
		return staticChecksum(bits);
	}

	const HairAssetDescriptorNS::ParametersStruct& parameters(void) const
	{
		HairAssetDescriptor* tmpThis = const_cast<HairAssetDescriptor*>(this);
		return *(static_cast<HairAssetDescriptorNS::ParametersStruct*>(tmpThis));
	}

	HairAssetDescriptorNS::ParametersStruct& parameters(void)
	{
		return *(static_cast<HairAssetDescriptorNS::ParametersStruct*>(this));
	}

	virtual NvParameterized::ErrorType getParameterHandle(const char* long_name, NvParameterized::Handle& handle) const;
	virtual NvParameterized::ErrorType getParameterHandle(const char* long_name, NvParameterized::Handle& handle);

	void initDefaults(void);

protected:

	virtual const NvParameterized::DefinitionImpl* getParameterDefinitionTree(void);
	virtual const NvParameterized::DefinitionImpl* getParameterDefinitionTree(void) const;


	virtual void getVarPtr(const NvParameterized::Handle& handle, void*& ptr, size_t& offset) const;

private:

	void buildTree(void);
	void initDynamicArrays(void);
	void initStrings(void);
	void initReferences(void);
	void freeDynamicArrays(void);
	void freeStrings(void);
	void freeReferences(void);

	static bool mBuiltFlag;
	static NvParameterized::MutexType mBuiltFlagMutex;
};

class HairAssetDescriptorFactory : public NvParameterized::Factory
{
	static const char* const vptr;

public:

	virtual void freeParameterDefinitionTable(NvParameterized::Traits* traits)
	{
		HairAssetDescriptor::freeParameterDefinitionTable(traits);
	}

	virtual NvParameterized::Interface* create(NvParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(HairAssetDescriptor), HairAssetDescriptor::ClassAlignment);
		if (!NvParameterized::IsAligned(newPtr, HairAssetDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairAssetDescriptor");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(HairAssetDescriptor)); // always initialize memory allocated to zero for default values
		return NV_PARAM_PLACEMENT_NEW(newPtr, HairAssetDescriptor)(paramTraits);
	}

	virtual NvParameterized::Interface* finish(NvParameterized::Traits* paramTraits, void* bufObj, void* bufStart, int32_t* refCount)
	{
		if (!NvParameterized::IsAligned(bufObj, HairAssetDescriptor::ClassAlignment)
		        || !NvParameterized::IsAligned(bufStart, HairAssetDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairAssetDescriptor");
			return 0;
		}

		// Init NvParameters-part
		// We used to call empty constructor of HairAssetDescriptor here
		// but it may call default constructors of members and spoil the data
		NV_PARAM_PLACEMENT_NEW(bufObj, NvParameterized::NvParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (HairAssetDescriptor*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (HairAssetDescriptor::staticClassName());
	}

	virtual uint32_t getVersion()
	{
		return (HairAssetDescriptor::staticVersion());
	}

	virtual uint32_t getAlignment()
	{
		return (HairAssetDescriptor::ClassAlignment);
	}

	virtual const uint32_t* getChecksum(uint32_t& bits)
	{
		return (HairAssetDescriptor::staticChecksum(bits));
	}
};
#endif // NV_PARAMETERIZED_ONLY_LAYOUTS

} // namespace parameterized
} // namespace nvidia

#if NV_VC
#pragma warning(pop)
#endif

#endif
