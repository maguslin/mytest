/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_INSTANCE_SIMULATE_H
#define NV_HAIR_DX11_INSTANCE_SIMULATE_H

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

class Dx11InstanceSimulate
{
	NV_CO_DECLARE_CLASS_BASE(Dx11InstanceSimulate);

	static Result stepSimulation(Dx11ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* worldReference);

protected:
	static Result _dispatchSimulatePinComGather(Dx11ApiInstance* apiInst);
	static Result _dispatchSimulatePinCom(Dx11ApiInstance* apiInst);
	static Result _dispatchSimulatePin(Dx11ApiInstance* apiInst);
	static Result _dispatchInteraction(Dx11ApiInstance* apiInst);
	static Result _dispatchSimulate(Dx11ApiInstance* apiInst);

	static Result _prepareSimulationConstantBuffer(Dx11ApiInstance* apiInst, float timeStep, const gfsdk_float4x4* mat);
	
	static Void _cleanComputeResources(ID3D11DeviceContext* context, int numUavs, int numSrvs);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_INSTANCE_SIMULATE_H

