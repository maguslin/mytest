/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_INSTANCE_RENDER_H
#define NV_HAIR_DX11_INSTANCE_RENDER_H


#include "NvHairDx11ApiGlobal.h"

#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

namespace nvidia {
namespace HairWorks {

struct RenderViewInfo;

class Dx11InstanceRender
{
	NV_CO_DECLARE_CLASS_BASE(Dx11InstanceRender);

	static Result renderHairShading(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& shaderSettings);
	
		/// constant buffer for pixel shader
	static Void calcPixelConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut);

	static Result prepareSimulationConstantBuffer(Dx11ApiInstance* apiInst, float timeStep, Float simulationInterp, const gfsdk_float4x4* mat);
	static Result dispatchPrepareInterpolate(Dx11ApiInstance* apiInst);

protected:

		// prepare constant buffer for tessellation stage
	static Result _prepareTessellationConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& settings, float density, float width, float densityPass = 0.0f);

	static Result _preparePixelConstantBuffer(Dx11ApiInstance* apiInst, const RenderViewInfo& frameViewInfo);

	static Void _preparePixelResources(Dx11ApiInstance* apiInst);
	static Void _prepareTessellationResources(Dx11ApiInstance* apiInst);

	static Void _cleanComputeResources(ID3D11DeviceContext* context, int numUavs, int numSrvs);

};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_INSTANCE_RENDER_H

