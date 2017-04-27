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

#include "NvHairAssetHeaderInfo.h"
#include <string.h>
#include <stdlib.h>

using namespace NvParameterized;

namespace nvidia
{
namespace parameterized
{

using namespace HairWorksInfoNS;

const char* const HairWorksInfoFactory::vptr =
    NvParameterized::getVptr<HairWorksInfo, HairWorksInfo::ClassAlignment>();

const uint32_t NumParamDefs = 6;
static NvParameterized::DefinitionImpl* ParamDefTable; // now allocated in buildTree [NumParamDefs];


static const size_t ParamLookupChildrenTable[] =
{
	1, 2, 3, 4, 5,
};

#define TENUM(type) nvidia::##type
#define CHILDREN(index) &ParamLookupChildrenTable[index]
static const NvParameterized::ParamLookupNode ParamLookupTable[NumParamDefs] =
{
	{ TYPE_STRUCT, false, 0, CHILDREN(0), 5 },
	{ TYPE_STRING, false, (size_t)(&((ParametersStruct*)0)->fileVersion), NULL, 0 }, // fileVersion
	{ TYPE_STRING, false, (size_t)(&((ParametersStruct*)0)->toolVersion), NULL, 0 }, // toolVersion
	{ TYPE_STRING, false, (size_t)(&((ParametersStruct*)0)->sourcePath), NULL, 0 }, // sourcePath
	{ TYPE_STRING, false, (size_t)(&((ParametersStruct*)0)->authorName), NULL, 0 }, // authorName
	{ TYPE_STRING, false, (size_t)(&((ParametersStruct*)0)->lastModified), NULL, 0 }, // lastModified
};


bool HairWorksInfo::mBuiltFlag = false;
NvParameterized::MutexType HairWorksInfo::mBuiltFlagMutex;

HairWorksInfo::HairWorksInfo(NvParameterized::Traits* traits, void* buf, int32_t* refCount) :
	NvParameters(traits, buf, refCount)
{
	//mParameterizedTraits->registerFactory(className(), &HairWorksInfoFactoryInst);

	if (!buf) //Do not init data if it is inplace-deserialized
	{
		initDynamicArrays();
		initStrings();
		initReferences();
		initDefaults();
	}
}

HairWorksInfo::~HairWorksInfo()
{
	freeStrings();
	freeReferences();
	freeDynamicArrays();
}

void HairWorksInfo::destroy()
{
	// We cache these fields here to avoid overwrite in destructor
	bool doDeallocateSelf = mDoDeallocateSelf;
	NvParameterized::Traits* traits = mParameterizedTraits;
	int32_t* refCount = mRefCount;
	void* buf = mBuffer;

	this->~HairWorksInfo();

	NvParameters::destroy(this, traits, doDeallocateSelf, refCount, buf);
}

const NvParameterized::DefinitionImpl* HairWorksInfo::getParameterDefinitionTree(void)
{
	if (!mBuiltFlag) // Double-checked lock
	{
		NvParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);
		if (!mBuiltFlag)
		{
			buildTree();
		}
	}

	return(&ParamDefTable[0]);
}

const NvParameterized::DefinitionImpl* HairWorksInfo::getParameterDefinitionTree(void) const
{
	HairWorksInfo* tmpParam = const_cast<HairWorksInfo*>(this);

	if (!mBuiltFlag) // Double-checked lock
	{
		NvParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);
		if (!mBuiltFlag)
		{
			tmpParam->buildTree();
		}
	}

	return(&ParamDefTable[0]);
}

NvParameterized::ErrorType HairWorksInfo::getParameterHandle(const char* long_name, Handle& handle) const
{
	ErrorType Ret = NvParameters::getParameterHandle(long_name, handle);
	if (Ret != ERROR_NONE)
	{
		return(Ret);
	}

	size_t offset;
	void* ptr;

	getVarPtr(handle, ptr, offset);

	if (ptr == NULL)
	{
		return(ERROR_INDEX_OUT_OF_RANGE);
	}

	return(ERROR_NONE);
}

NvParameterized::ErrorType HairWorksInfo::getParameterHandle(const char* long_name, Handle& handle)
{
	ErrorType Ret = NvParameters::getParameterHandle(long_name, handle);
	if (Ret != ERROR_NONE)
	{
		return(Ret);
	}

	size_t offset;
	void* ptr;

	getVarPtr(handle, ptr, offset);

	if (ptr == NULL)
	{
		return(ERROR_INDEX_OUT_OF_RANGE);
	}

	return(ERROR_NONE);
}

void HairWorksInfo::getVarPtr(const Handle& handle, void*& ptr, size_t& offset) const
{
	ptr = getVarPtrHelper(&ParamLookupTable[0], const_cast<HairWorksInfo::ParametersStruct*>(&parameters()), handle, offset);
}


/* Dynamic Handle Indices */

void HairWorksInfo::freeParameterDefinitionTable(NvParameterized::Traits* traits)
{
	if (!traits)
	{
		return;
	}

	if (!mBuiltFlag) // Double-checked lock
	{
		return;
	}

	NvParameterized::MutexType::ScopedLock lock(mBuiltFlagMutex);

	if (!mBuiltFlag)
	{
		return;
	}

	for (uint32_t i = 0; i < NumParamDefs; ++i)
	{
		ParamDefTable[i].~DefinitionImpl();
	}

	traits->free(ParamDefTable);

	mBuiltFlag = false;
}

#define PDEF_PTR(index) (&ParamDefTable[index])

void HairWorksInfo::buildTree(void)
{

	uint32_t allocSize = sizeof(NvParameterized::DefinitionImpl) * NumParamDefs;
	ParamDefTable = (NvParameterized::DefinitionImpl*)(mParameterizedTraits->alloc(allocSize));
	memset(ParamDefTable, 0, allocSize);

	for (uint32_t i = 0; i < NumParamDefs; ++i)
	{
		NV_PARAM_PLACEMENT_NEW(ParamDefTable + i, NvParameterized::DefinitionImpl)(*mParameterizedTraits);
	}

	// Initialize DefinitionImpl node: nodeIndex=0, longName=""
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[0];
		ParamDef->init("", TYPE_STRUCT, "STRUCT", true);






	}

	// Initialize DefinitionImpl node: nodeIndex=1, longName="fileVersion"
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[1];
		ParamDef->init("fileVersion", TYPE_STRING, NULL, true);

#ifdef NV_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "file format version", true);
		ParamDefTable[1].setHints((const NvParameterized::Hint**)HintPtrTable, 1);

#endif /* NV_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=2, longName="toolVersion"
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[2];
		ParamDef->init("toolVersion", TYPE_STRING, NULL, true);

#ifdef NV_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "authoring tool version", true);
		ParamDefTable[2].setHints((const NvParameterized::Hint**)HintPtrTable, 1);

#endif /* NV_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=3, longName="sourcePath"
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[3];
		ParamDef->init("sourcePath", TYPE_STRING, NULL, true);

#ifdef NV_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "source asset path to generate this file", true);
		ParamDefTable[3].setHints((const NvParameterized::Hint**)HintPtrTable, 1);

#endif /* NV_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=4, longName="authorName"
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[4];
		ParamDef->init("authorName", TYPE_STRING, NULL, true);

#ifdef NV_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "author name", true);
		ParamDefTable[4].setHints((const NvParameterized::Hint**)HintPtrTable, 1);

#endif /* NV_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// Initialize DefinitionImpl node: nodeIndex=5, longName="lastModified"
	{
		NvParameterized::DefinitionImpl* ParamDef = &ParamDefTable[5];
		ParamDef->init("lastModified", TYPE_STRING, NULL, true);

#ifdef NV_PARAMETERIZED_HIDE_DESCRIPTIONS

#else

		static HintImpl HintTable[1];
		static Hint* HintPtrTable[1] = { &HintTable[0], };
		HintTable[0].init("shortDescription", "last modified date time yyyy-MM-DD HH:MM:SS format. ex) 2014-02-02 14:01:02", true);
		ParamDefTable[5].setHints((const NvParameterized::Hint**)HintPtrTable, 1);

#endif /* NV_PARAMETERIZED_HIDE_DESCRIPTIONS */





	}

	// SetChildren for: nodeIndex=0, longName=""
	{
		static Definition* Children[5];
		Children[0] = PDEF_PTR(1);
		Children[1] = PDEF_PTR(2);
		Children[2] = PDEF_PTR(3);
		Children[3] = PDEF_PTR(4);
		Children[4] = PDEF_PTR(5);

		ParamDefTable[0].setChildren(Children, 5);
	}

	mBuiltFlag = true;

}
void HairWorksInfo::initStrings(void)
{
	fileVersion.isAllocated = true;
	fileVersion.buf = NULL;
	toolVersion.isAllocated = true;
	toolVersion.buf = NULL;
	sourcePath.isAllocated = true;
	sourcePath.buf = NULL;
	authorName.isAllocated = true;
	authorName.buf = NULL;
	lastModified.isAllocated = true;
	lastModified.buf = NULL;
}

void HairWorksInfo::initDynamicArrays(void)
{
}

void HairWorksInfo::initDefaults(void)
{

	freeStrings();
	freeReferences();
	freeDynamicArrays();

	initDynamicArrays();
	initStrings();
	initReferences();
}

void HairWorksInfo::initReferences(void)
{
}

void HairWorksInfo::freeDynamicArrays(void)
{
}

void HairWorksInfo::freeStrings(void)
{

	if (fileVersion.isAllocated && fileVersion.buf)
	{
		mParameterizedTraits->strfree((char*)fileVersion.buf);
	}

	if (toolVersion.isAllocated && toolVersion.buf)
	{
		mParameterizedTraits->strfree((char*)toolVersion.buf);
	}

	if (sourcePath.isAllocated && sourcePath.buf)
	{
		mParameterizedTraits->strfree((char*)sourcePath.buf);
	}

	if (authorName.isAllocated && authorName.buf)
	{
		mParameterizedTraits->strfree((char*)authorName.buf);
	}

	if (lastModified.isAllocated && lastModified.buf)
	{
		mParameterizedTraits->strfree((char*)lastModified.buf);
	}
}

void HairWorksInfo::freeReferences(void)
{
}

} // namespace parameterized
} // namespace nvidia