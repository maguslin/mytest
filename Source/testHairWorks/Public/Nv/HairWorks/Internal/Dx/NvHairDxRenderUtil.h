/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX_RENDER_UTIL_H
#define NV_HAIR_DX_RENDER_UTIL_H

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

struct DxRenderUtil
{

	static Void calcPixelConstantBuffer(ApiGlobal* glob, Instance* inst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& extConstantBuffer);

	static Void calcTessellationConstantBuffer(Instance* inst, const RenderViewInfo& frameViewInfo, float density, float width, float densityPass, NvHair_TessellationConstantBuffer& constantBufferOut);

	private:
	static void _calcMaterial(const InstanceDescriptor& desc, NvHair_Material& matOut);
	static void _calcTessellationMaterial(const InstanceDescriptor& desc, float density, float width, float scale, NvHair_TessellationMaterial& matOut);
};


} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX_RENDER_UTIL_H
