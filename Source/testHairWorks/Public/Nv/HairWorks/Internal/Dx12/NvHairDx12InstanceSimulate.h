/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_INSTANCE_SIMULATE_H
#define NV_HAIR_DX12_INSTANCE_SIMULATE_H

#include "NvHairDx12ApiGlobal.h"

#include <Nv/HairWorks/Shader/NvHairShaderCommonTypes.h>
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12CircularResourceHeap.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx12ApiInstance.h"

namespace nvidia {
namespace HairWorks {

class Dx12InstanceSimulate
{
	NV_CO_DECLARE_CLASS_BASE(Dx12InstanceSimulate);

	typedef NvCo::Dx12CircularResourceHeap ResourceHeap;
	typedef NvCo::Dx12DescriptorSet DescriptorSet;

	static Result stepSimulation(Dx12ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* worldReference);

protected:

	static Result _dispatchSimulatePin(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor);
	static Result _dispatchSimulatePinComGather(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor);
	static Result _dispatchSimulatePinCom(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor);
	
	static Result _prepareSimulationConstantBuffer(Dx12ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* mat, ResourceHeap::Cursor& cursorOut);

	static Result _dispatchSimulate(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor);
	static Result _dispatchInteraction(Dx12ApiInstance* apiInst, const ResourceHeap::Cursor& cbCursor);
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_INSTANCE_SIMULATE_H

