/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairFileBufReadAdapter.h"
#include <Nv/Common/NvCoCommon.h>

namespace nvidia {
namespace HairWorks {

namespace NvIo = nvidia::general_NvIOStream2;

NvIo::NvFileBuf::OpenMode FileBufReadAdapter::getOpenMode() const
{
	return m_openMode;
}

NvIo::NvFileBuf::SeekType FileBufReadAdapter::isSeekable() const
{
	return SEEKABLE_READ;
}

uint32_t FileBufReadAdapter::getFileLength(void) const
{
	if (m_stream)
	{
		// Work out the total file length
		Int64 current = m_stream->tell();
		Int64 end = m_stream->seek(NvCo::SeekOrigin::END, 0);
		m_stream->seek(NvCo::SeekOrigin::START, current);
		return uint32_t(end);
	}
	return 0;
}

uint32_t FileBufReadAdapter::seekRead(uint32_t loc)
{
	if (m_stream)
	{
		if (loc == Parent::STREAM_SEEK_END)
		{
			return uint32_t(m_stream->seek(NvCo::SeekOrigin::END, 0));
		}
		else
		{
			return uint32_t(m_stream->seek(NvCo::SeekOrigin::START, loc));
		}
	}
	return 0;
}

uint32_t FileBufReadAdapter::seekWrite(uint32_t loc)
{
	NV_UNUSED(loc);
	// Can't do on read adapter
	NV_CORE_ALWAYS_ASSERT("Can't seek on read");
	return 0;
}

uint32_t FileBufReadAdapter::read(void *mem, uint32_t len)
{
	if (m_stream)
	{
		return uint32_t(m_stream->read(mem, len));
	}
	return 0;
}

uint32_t FileBufReadAdapter::peek(void *mem, uint32_t len)
{
	// Peak is the same as a read, and a seek
	if (len > 0 && m_stream)
	{
		Int64 current = m_stream->tell();
		Int64 numRead = m_stream->read(mem, len);
		m_stream->seek(NvCo::SeekOrigin::START, current);
		return uint32_t(numRead);
	}
	return 0;
}

uint32_t FileBufReadAdapter::write(const void *mem, uint32_t len)
{
	NV_UNUSED(mem);
	NV_UNUSED(len);
	// Can't do on read adapter
	NV_CORE_ALWAYS_ASSERT("Can't write on read adapter");
	return 0;
}

uint32_t FileBufReadAdapter::tellRead() const
{
	return m_stream ? uint32_t(m_stream->tell()) : 0;
}

uint32_t FileBufReadAdapter::tellWrite() const
{
	// Can't do on read adapter
	NV_CORE_ALWAYS_ASSERT("Can't tellWrite on readAdapter");
	return 0;
}

void FileBufReadAdapter::flush()
{
}

void FileBufReadAdapter::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = NV_NULL;
	}
}

} // namespace HairWorks
} // namespace nvidia

