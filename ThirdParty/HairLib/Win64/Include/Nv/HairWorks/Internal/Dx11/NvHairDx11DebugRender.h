/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_DEBUG_RENDER_H
#define NV_HAIR_DX11_DEBUG_RENDER_H

#include "NvHairDx11ApiGlobal.h"

namespace nvidia {
namespace HairWorks {

struct Dx11DebugRender
{
	static Result drawGuideHairs(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawSkinnedGuideHairs(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawGuideHairControlVertices(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawHairInteraction(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawGrowthMesh(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);

	static Result drawPinConstraints(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	//static Result drawHairParticles(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawFrames(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawLocalPos(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
	static Result drawNormals(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_DEBUG_RENDER_H
