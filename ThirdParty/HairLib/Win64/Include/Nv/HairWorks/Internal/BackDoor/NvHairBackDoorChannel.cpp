/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/Common/NvCoCommon.h>

// Yuk - needed for var args
#include <stdarg.h>
#include <stdio.h>

#include "NvHairBackDoorChannel.h"

#include <Nv/Common/NvCoMemoryMappedFile.h>
#include <Nv/Common/NvCoMemory.h>

namespace nvidia {
namespace HairWorks { 


//#pragma warning(disable:4996)

BackDoorChannel::BackDoorChannel():
	m_sendCount(0),
	m_receiveCount(0)
{
}

Result BackDoorChannel::init(const char* sendName, const char* receiveName) 
{
	if (m_receiveFile || m_sendFile)
	{
		return NV_FAIL;
	}
	// Open the receive stream file
	m_receiveFile = NvCo::MemoryMappedFile::create(receiveName, MAX_BACKDOOR_PACKET);
	// Open the send stream file
	m_sendFile = NvCo::MemoryMappedFile::create(sendName,MAX_BACKDOOR_PACKET);
	return m_receiveFile && m_sendFile ? NV_OK : NV_FAIL;
}

void BackDoorChannel::send(const char* fmt, ...)
{
	char buf[DATA_SIZE + 1];
	buf[STRING_SIZE] = 0;

	va_list argptr;
	va_start(argptr, fmt);
	_vsnprintf(buf, DATA_SIZE, fmt, argptr);
	va_end(argptr);

	m_sendCount++;	// Increment the send counter.
	SizeT len = strlen(buf);

	if ( len < STRING_SIZE && m_sendFile )
	{
		char* cur = (char *)m_sendFile->getBaseAddress();	// get the base address of the shared memory
		Header& header = *(Header*)cur;
		cur += sizeof(Header);

		// First copy the message.
		NvCo::Memory::copy(cur, buf, len + 1);
		// Now revised the send count semaphore so the other process knows there is a new message to process.
		header.m_count = m_sendCount;
		header.m_type = 0;
	}
}

const char*const* BackDoorChannel::getInput(Int& argc)
{
	const char** ret = NV_NULL;
	argc = 0;

	if ( m_receiveFile )
	{
		const char* cur = (const char *)m_receiveFile->getBaseAddress();
		Header& header = *(Header*)cur;
		cur += sizeof(Header);

		if ( header.m_count != m_receiveCount )
		{
			m_receiveCount = header.m_count;
			NvCo::Memory::copy(m_buffer, cur, DATA_SIZE);

			char *scan = m_buffer;
			while ( *scan == 32 ) scan++;	// skip leading spaces
			if ( *scan )	// if not EOS
			{
				argc = 1;
				m_argv[0] = scan;	// set the first argument

				while ( *scan && argc < MAX_ARGS)	// while still data and we haven't exceeded the maximum argument count.
				{
					while ( *scan && *scan != 32 ) scan++;	// scan forward to the next space
					if ( *scan == 32 )	// if we hit a space
					{
						*scan = 0; // null-terminate the argument
						scan++;	// skip to the next character
						while ( *scan == 32 ) scan++;	// skip any leading spaces
						if ( *scan )	// if there is still a valid non-space character process that as the next argument
						{
							m_argv[argc] = scan;
							argc++;
						}
					}
				}
				ret = m_argv;
			}
		}
	}

	return ret;
}

Bool BackDoorChannel::sendData(const char* ptr, SizeT size, Int dataType)
{
	if ( size > DATA_SIZE || !m_sendFile )
		return false;

	m_sendCount++;	// Increment the send counter.

	char* cur = (char *)m_sendFile->getBaseAddress();	// get the base address of the shared memory
	Header& header = *(Header*)cur;
	header.m_count = m_sendCount;
	header.m_type = Int32(dataType);
	cur += sizeof(Header);
	NvCo::Memory::copy(cur, ptr, size);
	return true;
}

Bool BackDoorChannel::receiveData(char* ptr, SizeT size, Int dataType)
{
	if (size > DATA_SIZE || !m_receiveFile)
		return false;

	const char* cur = (const char *)m_receiveFile->getBaseAddress();
	Header& header = *(Header*)cur;
	if (header.m_type != dataType)
		return false;

	cur += sizeof(Header);
	if (header.m_count != m_receiveCount)
	{
		m_receiveCount = header.m_count;
		NvCo::Memory::copy(ptr, cur, size);
	}
	return true;
}

Bool BackDoorChannel::checkMessage(const char* msg)
{
	if (!m_receiveFile)
		return false;

	const char* cur = (const char *)m_receiveFile->getBaseAddress();
	Header& header = *(Header*)cur;
	cur += sizeof(Header);

	if (header.m_count == m_receiveCount)
		return false;

	return (!strcmp(cur, msg));
}

} // namespace HairWorks
} // namespace nvidia
