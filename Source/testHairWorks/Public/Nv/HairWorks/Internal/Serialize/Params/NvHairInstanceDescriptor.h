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

#ifndef HEADER_NvHairInstanceDescriptor_h
#define HEADER_NvHairInstanceDescriptor_h

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

namespace HairInstanceDescriptorNS
{

struct Material_Type;

struct Material_DynamicArray1D_Type
{
	Material_Type* buf;
	bool isAllocated;
	int32_t elementSize;
	int32_t arraySizes[1];
};

struct Material_Type
{
	NvParameterized::DummyStringStruct name;
	uint32_t densityTextureChan;
	uint32_t widthTextureChan;
	uint32_t rootWidthTextureChan;
	uint32_t tipWidthTextureChan;
	uint32_t clumpScaleTextureChan;
	uint32_t clumpNoiseTextureChan;
	uint32_t clumpRoundnessTextureChan;
	uint32_t waveScaleTextureChan;
	uint32_t waveFreqTextureChan;
	uint32_t lengthTextureChan;
	uint32_t stiffnessTextureChan;
	uint32_t rootStiffnessTextureChan;
	uint32_t splineMultiplier;
	uint32_t assetType;
	uint32_t assetPriority;
	uint32_t assetGroup;
	float width;
	float widthNoise;
	float clumpNoise;
	uint32_t clumpNumSubclumps;
	float clumpRoundness;
	float clumpScale;
	bool clumpPerVertex;
	float density;
	float lengthNoise;
	float lengthScale;
	float widthRootScale;
	float widthTipScale;
	float waveRootStraighten;
	float waveScale;
	float waveScaleNoise;
	float waveFreq;
	float waveFreqNoise;
	float waveScaleStrand;
	float waveScaleClump;
	bool enableDistanceLOD;
	float distanceLODStart;
	float distanceLODEnd;
	float distanceLODFadeStart;
	float distanceLODDensity;
	float distanceLODWidth;
	bool enableDetailLOD;
	float detailLODStart;
	float detailLODEnd;
	float detailLODDensity;
	float detailLODWidth;
	uint32_t colorizeLODOption;
	bool useViewfrustrumCulling;
	bool useBackfaceCulling;
	float backfaceCullingThreshold;
	bool usePixelDensity;
	float alpha;
	float strandBlendScale;
	nvidia::NvVec4 baseColor;
	float diffuseBlend;
	float diffuseScale;
	float diffuseHairNormalWeight;
	uint32_t diffuseBoneIndex;
	nvidia::NvVec3 diffuseBoneLocalPos;
	float diffuseNoiseFreqU;
	float diffuseNoiseFreqV;
	float diffuseNoiseScale;
	float diffuseNoiseGain;
	float textureBrightness;
	nvidia::NvVec4 diffuseColor;
	nvidia::NvVec4 rootColor;
	nvidia::NvVec4 tipColor;
	float glintStrength;
	float glintCount;
	float glintExponent;
	float rootAlphaFalloff;
	float rootTipColorWeight;
	float rootTipColorFalloff;
	float shadowSigma;
	nvidia::NvVec4 specularColor;
	float specularPrimary;
	float specularNoiseScale;
	float specularEnvScale;
	float specularPrimaryBreakup;
	float specularSecondary;
	float specularSecondaryOffset;
	float specularPowerPrimary;
	float specularPowerSecondary;
	uint32_t strandBlendMode;
	bool useTextures;
	bool useShadows;
	float shadowDensityScale;
	bool castShadows;
	bool receiveShadows;
	float backStopRadius;
	float bendStiffness;
	float interactionStiffness;
	float pinStiffness;
	float collisionOffset;
	bool useCollision;
	bool useDynamicPin;
	float damping;
	float friction;
	float massScale;
	nvidia::NvVec3 gravity;
	float inertiaScale;
	float inertiaLimit;
	float rootStiffness;
	float tipStiffness;
	bool simulate;
	float stiffness;
	float stiffnessStrength;
	float stiffnessDamping;
	nvidia::NvVec4 stiffnessCurve;
	nvidia::NvVec4 stiffnessStrengthCurve;
	nvidia::NvVec4 stiffnessDampingCurve;
	nvidia::NvVec4 bendStiffnessCurve;
	nvidia::NvVec4 interactionStiffnessCurve;
	nvidia::NvVec3 wind;
	float windNoise;
	bool visualizeBones;
	bool visualizeBoundingBox;
	bool visualizeCapsules;
	bool visualizeControlVertices;
	bool visualizeCullSphere;
	bool visualizeDiffuseBone;
	bool visualizeFrames;
	bool visualizeGrowthMesh;
	bool visualizeGuideHairs;
	bool visualizeHairInteractions;
	uint32_t visualizeHairSkips;
	bool visualizeLocalPos;
	bool visualizePinConstraints;
	bool visualizeShadingNormals;
	bool visualizeSkinnedGuideHairs;
	bool drawRenderHairs;
	bool enable;
};

struct ParametersStruct
{

	Material_DynamicArray1D_Type materials;

};

static const uint32_t checksum[] = { 0x0f163998, 0xfce1047d, 0x0aa9f1ca, 0xce6ef732, };

} // namespace HairInstanceDescriptorNS

#ifndef NV_PARAMETERIZED_ONLY_LAYOUTS
class HairInstanceDescriptor : public NvParameterized::NvParameters, public HairInstanceDescriptorNS::ParametersStruct
{
public:
	HairInstanceDescriptor(NvParameterized::Traits* traits, void* buf = 0, int32_t* refCount = 0);

	virtual ~HairInstanceDescriptor();

	virtual void destroy();

	static const char* staticClassName(void)
	{
		return("HairInstanceDescriptor");
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
		bits = 8 * sizeof(HairInstanceDescriptorNS::checksum);
		return HairInstanceDescriptorNS::checksum;
	}

	static void freeParameterDefinitionTable(NvParameterized::Traits* traits);

	const uint32_t* checksum(uint32_t& bits) const
	{
		return staticChecksum(bits);
	}

	const HairInstanceDescriptorNS::ParametersStruct& parameters(void) const
	{
		HairInstanceDescriptor* tmpThis = const_cast<HairInstanceDescriptor*>(this);
		return *(static_cast<HairInstanceDescriptorNS::ParametersStruct*>(tmpThis));
	}

	HairInstanceDescriptorNS::ParametersStruct& parameters(void)
	{
		return *(static_cast<HairInstanceDescriptorNS::ParametersStruct*>(this));
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

class HairInstanceDescriptorFactory : public NvParameterized::Factory
{
	static const char* const vptr;

public:

	virtual void freeParameterDefinitionTable(NvParameterized::Traits* traits)
	{
		HairInstanceDescriptor::freeParameterDefinitionTable(traits);
	}

	virtual NvParameterized::Interface* create(NvParameterized::Traits* paramTraits)
	{
		// placement new on this class using mParameterizedTraits

		void* newPtr = paramTraits->alloc(sizeof(HairInstanceDescriptor), HairInstanceDescriptor::ClassAlignment);
		if (!NvParameterized::IsAligned(newPtr, HairInstanceDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairInstanceDescriptor");
			paramTraits->free(newPtr);
			return 0;
		}

		memset(newPtr, 0, sizeof(HairInstanceDescriptor)); // always initialize memory allocated to zero for default values
		return NV_PARAM_PLACEMENT_NEW(newPtr, HairInstanceDescriptor)(paramTraits);
	}

	virtual NvParameterized::Interface* finish(NvParameterized::Traits* paramTraits, void* bufObj, void* bufStart, int32_t* refCount)
	{
		if (!NvParameterized::IsAligned(bufObj, HairInstanceDescriptor::ClassAlignment)
		        || !NvParameterized::IsAligned(bufStart, HairInstanceDescriptor::ClassAlignment))
		{
			NV_PARAM_TRAITS_WARNING(paramTraits, "Unaligned memory allocation for class HairInstanceDescriptor");
			return 0;
		}

		// Init NvParameters-part
		// We used to call empty constructor of HairInstanceDescriptor here
		// but it may call default constructors of members and spoil the data
		NV_PARAM_PLACEMENT_NEW(bufObj, NvParameterized::NvParameters)(paramTraits, bufStart, refCount);

		// Init vtable (everything else is already initialized)
		*(const char**)bufObj = vptr;

		return (HairInstanceDescriptor*)bufObj;
	}

	virtual const char* getClassName()
	{
		return (HairInstanceDescriptor::staticClassName());
	}

	virtual uint32_t getVersion()
	{
		return (HairInstanceDescriptor::staticVersion());
	}

	virtual uint32_t getAlignment()
	{
		return (HairInstanceDescriptor::ClassAlignment);
	}

	virtual const uint32_t* getChecksum(uint32_t& bits)
	{
		return (HairInstanceDescriptor::staticChecksum(bits));
	}
};
#endif // NV_PARAMETERIZED_ONLY_LAYOUTS

} // namespace parameterized
} // namespace nvidia

#if NV_VC
#pragma warning(pop)
#endif

#endif
