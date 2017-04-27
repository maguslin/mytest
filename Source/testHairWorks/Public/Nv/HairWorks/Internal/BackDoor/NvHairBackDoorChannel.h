/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_BACKDOOR_CHANNEL_H
#define NV_HAIR_BACKDOOR_CHANNEL_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/NvCoMemoryMappedFile.h>

namespace nvidia { 
namespace HairWorks {

/// Based on the BackDoor class originally by John Ratcliff, and in terms of the protocol is 100% compatible
/// with that implementation.
/// This version aims to be cross platform - as long as a platform can implement a MemoryMappedFile inteface.
/// Allow for 2 way communication.
class BackDoorChannel
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(BackDoorChannel);

	struct Header
	{
		Int32 m_count;
		Int32 m_type;
	};

	enum 
	{
		MAX_BACKDOOR_PACKET = 1024, 	// maximum size of a send/receive packet
		HEADER_SIZE  = sizeof(Header), 
		DATA_SIZE = MAX_BACKDOOR_PACKET - HEADER_SIZE,
		STRING_SIZE = DATA_SIZE - 1,
		MAX_ARGS = 32
	};

		// Sends a 'command' to the other process. Uses the printf style format for convenience.
	void send(const char *str,...);	
		/// Consumes an incoming command from the other process and parses it into an argc/argv format.
	const Char*const* getInput(Int& argc);
		/// Send data
	Bool sendData(const char* ptr, SizeT size, Int dataType);
		/// Receive data
	Bool receiveData(char* ptr, SizeT size, Int dataType);
		/// Check for a message
	Bool checkMessage(const char* msg);

	/*! init
		@param sendName The name of the shared memory file to act as the 'sender' stream.
		@param receiveName The name of the shared memory file to act as the 'receive' message stream. */
	Result init(const char* sendName, const char* receiveName);

		/// Ctor
	BackDoorChannel();

protected:
	NvCo::UniquePtr<NvCo::MemoryMappedFile>	m_receiveFile;
	NvCo::UniquePtr<NvCo::MemoryMappedFile>	m_sendFile;
	const char* m_argv[MAX_ARGS];
	char m_buffer[MAX_BACKDOOR_PACKET];

	Int	m_sendCount;
	Int	m_receiveCount;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_BACKDOOR_CHANNEL_H
