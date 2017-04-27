/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_LAYOUT_CALCULATOR_H
#define NV_HAIR_LAYOUT_CALCULATOR_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

namespace nvidia {
namespace HairWorks { 

/*! A class used to work out how allocations can be laid out contiguously in memory. */
struct LayoutCalculator
{
	enum { MAX_OFFSETS = 32 };

	Void clear()
	{
		m_alignment = 1;
		m_offset = 0;
		m_numOffsets = 0;
	}
		/// Add entries which should be null
	SizeT addNull(Int numSkip)
	{
		NV_CORE_ASSERT(m_numOffsets + numSkip <= MAX_OFFSETS);
		for (Int i = 0; i < numSkip; i++)
		{
			m_offsets[m_numOffsets + i] = -1;
		}
		m_numOffsets += numSkip;
		return 0;
	}
		/// Add a type, returns the offset to it's start
	template <typename T>
	SizeT addType()
	{
		NV_CORE_ASSERT(m_numOffsets < MAX_OFFSETS);
		// Align
		const SizeT align = NV_ALIGN_OF(T);
		m_alignment = align > m_alignment ? align : m_alignment;
		const SizeT offset = (m_offset + align - 1) & ~(align - 1);
		m_offsets[m_numOffsets++] = offset;
		m_offset = offset + sizeof(T);
		return offset;
	}

		/// Add an array, returns the offset to it's start
	template <typename T>
	SizeT addArray(const T* ptr, SizeT size)
	{
		NV_CORE_ASSERT(m_numOffsets < MAX_OFFSETS);
		NV_CORE_ASSERT(size == 0 || (size > 0 && ptr));
		NV_UNUSED(ptr);
		// Align
		const SizeT align = NV_ALIGN_OF(T);
		m_alignment = align > m_alignment ? align : m_alignment;
		const SizeT offset = (m_offset + align - 1) & ~(align - 1);
		m_offsets[m_numOffsets++] = offset;
		m_offset = offset + size * sizeof(T);
		return offset;
	}
	template <typename T>
	SizeT addArrayConditional(Bool cond, const T* ptr, SizeT size)
	{
		return cond ? addArray(ptr, size) : addNull(1);
	}

		// Get at the specified index
	template <typename T> 
	T* getAt(Int i, Void* base)
	{
		NV_CORE_ASSERT( i > = 0 && i < MAX_OFFSETS);
		PtrDiffT offset = m_offsets[i];
		return (offset >= 0) ? (T*)(((UInt8*)base) + offset) : NV_NULL;
	}
	template <typename T>
	T* copyAt(Int i, Void* base, const T* src, Int size)
	{
		return (T*)_copy(i, base, src, sizeof(T) * size);
	}

		/// Add
	SizeT add(SizeT align, SizeT totalSize)
	{
		NV_CORE_ASSERT(m_numOffsets < MAX_OFFSETS);
		NV_CORE_ASSERT(((align - 1) & align) == 0);
		m_alignment = align > m_alignment ? align : m_alignment;
		const SizeT offset = (m_offset + align - 1) & ~(align - 1);
		m_offsets[m_numOffsets++] = offset;
		m_offset = offset + totalSize;
		return offset;
	}
		/// Fix the offset such that it's alignment is correct
	Void alignOverall() { m_offset = (m_offset + m_alignment - 1) & ~(m_alignment - 1); }

		/// Get the total size in bytes
	SizeT getSizeInBytes() const { return m_offset; }

		/// Ctor
	LayoutCalculator() { clear(); }

protected:
	Void* _copy(Int i, Void* dstBase, const Void* srcBase, Int totalSizeInBytes)
	{
		NV_CORE_ASSERT(i >= 0 && i < MAX_OFFSETS);
		PtrDiffT offset = m_offsets[i];
		if (offset >= 0)
		{
			UInt8* dst = ((UInt8*)dstBase) + offset;
			NvCo::Memory::copy(dst, srcBase, totalSizeInBytes);
			return dst;
		}
		return NV_NULL;
	}

	SizeT m_alignment;				///< The alignment the whole structure will need
	SizeT m_offset;					///< Current offset 
	Int m_numOffsets;				///< Total number of allocations
	PtrDiffT m_offsets[MAX_OFFSETS];	///< The offsets. If < 0, it's NULL
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_LAYOUT_CALCULATOR_H
