/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_DEBUG_RENDER_H
#define NV_HAIR_DX12_DEBUG_RENDER_H

#include "NvHairDx12ApiGlobal.h"
#include "NvHairDx12ApiInstance.h"

namespace nvidia {
namespace HairWorks {

struct Dx12DebugRender
{
	typedef NvCo::Dx12CircularResourceHeap Heap;
	typedef NvCo::Dx12DescriptorSet DescriptorSet;

	static Result drawGuideHairs(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawGrowthMesh(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);

	static Result drawSkinnedGuideHairs(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawFrames(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawLocalPos(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawNormals(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawHairInteraction(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawGuideHairControlVertices(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);

	//static Result drawHairParticles(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawPinConstraints(Dx12ApiInstance* apiInst, const ViewInfo& viewInfo);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX12_DEBUG_RENDER_H
