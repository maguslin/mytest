/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_ASYNC_TYPES_H
#define NV_HAIR_DX12_ASYNC_TYPES_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoMemory.h>

#include <Nv/Common/Platform/Dx12/NvCoDx12CircularResourceHeap.h>
#include <Nv/Common/Platform/Dx12/NvCoDx12AsyncManager.h>

#include <Nv/HairWorks/NvHairSdk.h>

namespace nvidia {
namespace HairWorks {

class Dx12AsyncType { Dx12AsyncType(); public: enum Enum { 
	COMPUTE_STATS,
	GET_PIN_MATRICES,
	COUNT_OF,
}; };
typedef Dx12AsyncType::Enum EDx12AsyncType;

struct Dx12ComputeStatsAsync : public NvCo::Dx12Async
{
	enum { TYPE = Dx12AsyncType::COMPUTE_STATS };

	NvCo::Dx12CircularResourceHeap::Cursor m_readBackCursor;
	Stats m_stats;
};

struct Dx12GetPinMatrixAsync : public NvCo::Dx12Async
{	
	enum { TYPE = Dx12AsyncType::GET_PIN_MATRICES };

	NvCo::Dx12CircularResourceHeap::Cursor m_readBackCursor;
	Int m_numPins;								///< The amount of pins
	gfsdk_float4x4 m_modelToWorld;
	gfsdk_float4x4 m_pinMatrices[1];				///< Will be allocated contiguously in memory
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_ASYNC_TYPES_H
