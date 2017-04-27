/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairApiInstance.h"

namespace nvidia {
namespace HairWorks { 

/* static */const char* ApiInstance::getText(EDebugDraw drawType)
{
	switch (drawType)
	{ 
		case DebugDraw::GUIDE_HAIRS: return "Guide Hairs";
		case DebugDraw::SKINNED_GUIDE_HAIRS: return "Skinned Guide Hairs";
		case DebugDraw::FRAMES:	return "Frames";
		case DebugDraw::NORMALS: return "Normals";
		case DebugDraw::LOCAL_POS: return "Local Pos";
		case DebugDraw::HAIR_INTERACTION: return "Hair Interaction";
		case DebugDraw::GUIDE_HAIR_CONTROL_VERTICES: return "Guide Hair Control Vertices";
		case DebugDraw::GROWTH_MESH: return "Growth Mesh";
		case DebugDraw::PIN_CONSTRAINTS: return "Pin Constraints";
		default: return "Unknown";
	}
}

} // namespace HairWorks
} // namespace nvidia
