/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_INTERNAL_H
#define NV_HAIR_INTERNAL_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoMemoryAllocator.h>

#include <math.h>
#include <float.h>

#include <stdio.h> // for vsprintf

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Thread/NvCoCriticalSection.h>

//#pragma warning(disable:4345) // warning C4345: behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized

namespace nvidia {
namespace HairWorks {

/*! Allocates an array of POD type elements. Since the type is claimed to be POD NO ctor is executed 
@param size The amount of items of T in the array */
template <typename T>
NV_FORCE_INLINE T* allocatePodArray(SizeT size)
{
	NV_CORE_ASSERT(NV_ALIGN_OF(T) <= NvCo::MemoryAllocator::DEFAULT_ALIGNMENT);
	return (T*)NvCo::MemoryAllocator::getInstance()->simpleAllocate(size * sizeof(T));
}

/*! Deallocate memory allocated with allocatePodArray
@param ptr Points to the memory returned from allocatePodArray */
template <typename T>
NV_FORCE_INLINE void deallocatePodArray(const T* ptr)
{
	NvCo::MemoryAllocator::getInstance()->simpleDeallocate(const_cast<T*>(ptr));
}

/*! Deallocate a chunk of memory, as if it was pointing to a POD array. If NV_NULL does nothing 
@param ptrInOut Pointer that is tested and deallocated, and set to NV_NULL if set */
template <typename T>
NV_FORCE_INLINE void safeDeallocatePodArray(T*& ptrInOut)
{
	const T* ptr = ptrInOut;
	if (ptr)
	{
		NvCo::MemoryAllocator::getInstance()->simpleDeallocate(const_cast<T*>(ptr));
		ptrInOut = NV_NULL;
	}
}

// Declare types
class ShaderCacheEntry;
class ShaderCache;

class Instance;

class Dx11ApiInstance;

#define NV_HAIR_THREAD_LOCK	::NvCo::ScopeCriticalSection scopeCriticalSection(m_criticalSection);

#define NV_HAIR_NUM_HAIRS_PER_FACE 64

} // namespace HairWorks
} // namespace nvidia



#endif // NV_HAIR_INTERNAL_H
