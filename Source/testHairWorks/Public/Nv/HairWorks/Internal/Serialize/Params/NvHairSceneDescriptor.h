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

#ifndef HEADER_NvHairSceneDescriptor_h
#define HEADER_NvHairSceneDescriptor_h

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

namespace HairSceneDescriptorNS
{



struct ParametersStruct
{

	NvParameterized::DummyStringStruct densityTexture;
	NvParameterized::DummyStringStruct rootColorTexture;
	NvParameterized::DummyStringStruct tipColorTexture;
	NvParameterized::DummyStringStruct widthTexture;
	NvParameterized::DummyStringStruct rootWidthTexture;
	NvParameterized::DummyStringStruct tipWidthTexture;
	NvParameterized::DummyStringStruct stiffnessTexture;
	NvParameterized::DummyStringStruct rootStiffnessTexture;
	NvParameterized::DummyStringStruct clumpScaleTexture;
	NvParameterized::DummyStringStruct clumpRoundnessTexture;
	NvParameterized::DummyStringStruct clumpNoiseTexture;
	NvParameterized::DummyStringStruct waveScaletexture;
	NvParameterized::DummyStringStruct waveFreqTexture;
	NvParameterized::DummyStringStruct strandTexture;
	NvParameterized::DummyStringStruct lengthTexture;
	NvParameterized::DummyStringStruct specularTexture;

};

static const uint32_t checksum[] = { 0x299b335f, 0x2cad8b54, 0xcaf3c98f, 0xa3094fa7, };

} // namespace HairSceneDescriptorNS

#ifndef NV_PARAMETERIZED_ONLY_LAYOUTS
class HairSceneDescriptor : public NvParameterized::NvParameters, public HairSceneDescriptorNS::ParametersStruct
{
public:
	HairSceneDescriptor(NvParameterized::Traits* traits, void* buf = 0, int32_t* refCount = 0);

	virtual ~HairSceneDescriptor();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("HairSceneDescriptor");
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
		bits = 8 * sizeof(HairSceneDescriptorNS::checksum);
		return HairSceneDescriptorNS::checksum;
	}

	static void freeParameterDefinitionTable(NvParameterized::Traits* traits);

	const uint32_t* checksum(uint32_t& bits) const
	{
		return staticChecksum(bits);
	}

	const HairSceneDescriptorNS::ParametersStruct& parameters(void) const
	{
		HairSceneDescriptor* tmpThis = const_cast<HairSceneDescriptor*>(this);
		return *(static_cast<HairSceneDescriptorNS::ParametersStruct*>(tmpThis));
	}

	HairSceneDescriptorNS::ParametersStruct& parameters(void)
	{
		return *(static_cast<HairSceneDescriptorNS::ParametersStruct*>(this));
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

class HairSceneDescriptorFactory : public NvParameterized::Factory
{
	static const char* const vptr;

public:

	virtual void freeParameterDefinitionTable(NvParameterized::Traits* traits)
	{
		HairSceneDescriptor::freeParameterDefinitionTable(traits);
	}

	virtual NvParameterized::Interface* create(NvParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(HairSceneDescriptor), HairSceneDescriptor::ClassAlignment);
		if (!NvParameterized::IsAligned(newPtr, HairSceneDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairSceneDescriptor");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(HairSceneDescriptor)); // always initialize memory allocated to zero for default values
		return NV_PARAM_PLACEMENT_NEW(newPtr, HairSceneDescriptor)(paramTraits);
	}

	virtual NvParameterized::Interface* finish(NvParameterized::Traits* paramTraits, void* bufObj, void* bufStart, int32_t* refCount)
	{
		if (!NvParameterized::IsAligned(bufObj, HairSceneDescriptor::ClassAlignment)
		        || !NvParameterized::IsAligned(bufStart, HairSceneDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairSceneDescriptor");
			return 0;
		}

		// Init NvParameters-part
		// We used to call empty constructor of HairSceneDescriptor here
		// but it may call default constructors of members and spoil the data
		NV_PARAM_PLACEMENT_NEW(bufObj, NvParameterized::NvParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (HairSceneDescriptor*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (HairSceneDescriptor::staticClassName());
	}

	virtual uint32_t getVersion()
	{
		return (HairSceneDescriptor::staticVersion());
	}

	virtual uint32_t getAlignment()
	{
		return (HairSceneDescriptor::ClassAlignment);
	}

	virtual const uint32_t* getChecksum(uint32_t& bits)
	{
		return (HairSceneDescriptor::staticChecksum(bits));
	}
};
#endif // NV_PARAMETERIZED_ONLY_LAYOUTS

} // namespace parameterized
} // namespace nvidia

#if NV_VC
#pragma warning(pop)
#endif

#endif
