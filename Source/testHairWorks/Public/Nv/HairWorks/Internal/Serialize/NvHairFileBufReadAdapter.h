/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_FILE_BUF_READ_ADAPTER_H
#define NV_HAIR_FILE_BUF_READ_ADAPTER_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoStream.h>

#include "NvFileBuf.h"

namespace nvidia {
namespace HairWorks {

class FileBufReadAdapter: public nvidia::general_NvIOStream2::NvFileBuf
{
	public:
	typedef nvidia::general_NvIOStream2::NvFileBuf Parent;
	
		/// Ctor 
	FileBufReadAdapter(NvCo::ReadStream* stream, OpenMode openMode = NvFileBuf::OPEN_READ_ONLY):
		m_stream(stream),
		m_openMode(stream->isClosed() ? NvFileBuf::OPEN_FILE_NOT_FOUND : openMode)
	{} 

	virtual OpenMode getOpenMode(void) const NV_OVERRIDE;
	virtual SeekType isSeekable(void) const NV_OVERRIDE;
	virtual uint32_t getFileLength(void) const NV_OVERRIDE;
	virtual uint32_t seekRead(uint32_t loc) NV_OVERRIDE;
	virtual uint32_t seekWrite(uint32_t loc) NV_OVERRIDE;
	virtual uint32_t read(void *mem, uint32_t len) NV_OVERRIDE;
	virtual uint32_t peek(void *mem, uint32_t len) NV_OVERRIDE;
	virtual uint32_t write(const void *mem, uint32_t len) NV_OVERRIDE;
	virtual uint32_t tellRead() const NV_OVERRIDE;
	virtual uint32_t tellWrite() const NV_OVERRIDE;
	virtual	void flush() NV_OVERRIDE;
	virtual void close() NV_OVERRIDE;

	OpenMode m_openMode;
	NvCo::ReadStream* m_stream;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_FILE_BUF_READ_ADAPTER_H