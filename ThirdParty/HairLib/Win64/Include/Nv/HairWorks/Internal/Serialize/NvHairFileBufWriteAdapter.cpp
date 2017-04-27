/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairFileBufWriteAdapter.h"
#include <Nv/Common/NvCoCommon.h>

namespace nvidia {
namespace HairWorks {

namespace NvIo = nvidia::general_NvIOStream2;

NvIo::NvFileBuf::OpenMode FileBufWriteAdapter::getOpenMode(void) const
{
	return m_openMode;
}

NvIo::NvFileBuf::SeekType FileBufWriteAdapter::isSeekable(void) const
{
	return SEEKABLE_NO;
}

uint32_t FileBufWriteAdapter::getFileLength(void) const
{
	return uint32_t(m_position);
	//NV_ALWAYS_ASSERT();
	//return 0;
}

uint32_t FileBufWriteAdapter::seekRead(uint32_t loc)
{
	NV_UNUSED(loc);
	// Can't do on write
	NV_CORE_ALWAYS_ASSERT("Can't seekRead on WriteAdapter");
	return 0;
}

uint32_t FileBufWriteAdapter::seekWrite(uint32_t loc)
{
	NV_UNUSED(loc);
	// Can't do on write
	NV_CORE_ALWAYS_ASSERT("Can't seekWrite on WriteAdapter");
	return 0;
}

uint32_t FileBufWriteAdapter::read(void *mem, uint32_t len)
{
	NV_UNUSED(len);
	NV_UNUSED(mem);
	// Can't do on write
	NV_CORE_ALWAYS_ASSERT("Can't read on WriteAdapter");
	return 0;
}

uint32_t FileBufWriteAdapter::peek(void *mem, uint32_t len)
{
	NV_UNUSED(len);
	NV_UNUSED(mem);
	// Can't do on write
	NV_CORE_ALWAYS_ASSERT("Can't peek on WriteAdapter");
	return 0;
}

uint32_t FileBufWriteAdapter::write(const void *mem, uint32_t len)
{
	if (m_stream)
	{
		Int64 numBytes = m_stream->write(mem, len);
		m_position += numBytes;
		return uint32_t(numBytes);
	}
	return 0;
}

uint32_t FileBufWriteAdapter::tellRead() const
{
	// Can't do on write
	NV_CORE_ALWAYS_ASSERT("Can't tellRead on WriteAdapter");
	return 0;
}

uint32_t FileBufWriteAdapter::tellWrite() const
{
	return uint32_t(m_position);
}

void FileBufWriteAdapter::flush()
{
	if (m_stream)
	{
		m_stream->flush();
	}
}

void FileBufWriteAdapter::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = NV_NULL;
	}
}

} // namespace HairWorks
} // namespace nvidia


