/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_INSTANCE_RENDER_H
#define NV_HAIR_DX11_INSTANCE_RENDER_H


#include "NvHairDx12ApiGlobal.h"

#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12ApiInstance.h"

namespace nvidia {
namespace HairWorks {

struct RenderViewInfo;

class Dx12InstanceRender
{
	NV_CO_DECLARE_CLASS_BASE(Dx12InstanceRender);

	typedef NvCo::Dx12DescriptorSet DescriptorSet;
	typedef NvCo::Dx12CircularResourceHeap Heap;

	static Result renderHairShading(Dx12ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, const ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other);
		/// constant buffer for pixel shader
	static Void calcPixelConstantBuffer(Dx12ApiInstance* apiInst, const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut);

		/// 
	static Result dispatchCalcInterpolationDelta(Dx12ApiInstance* apiInst, Float simulationInterp);
protected:

};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_INSTANCE_RENDER_H

