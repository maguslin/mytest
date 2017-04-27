/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX_SIMULATE_UTIL_H
#define NV_HAIR_DX_SIMULATE_UTIL_H

#include <Nv/Common/NvCoCommon.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>
#include <Nv/HairWorks/Internal/NvHairViewInfo.h>

#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

// constant buffer definition shared with tessellation shaders
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderCommon.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

// constant buffer definition shared with pixel shaders
#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>

namespace nvidia {
namespace HairWorks {

struct DxSimulateUtil
{
		/// Calculate the Simulation material 
	static Void calcMaterial(const InstanceDescriptor& desc, float unitScale, NvHair_SimulationMaterial& matOut);
		/// Calculate the constant buffer
	static Result calcConstantBuffer(Instance* inst, float timeStep, Float simulationInterp, const gfsdk_float4x4* mat, NvHair_SimulateConstantBuffer* constantBufferOut);
};


} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX_SIMULATE_UTIL_H
