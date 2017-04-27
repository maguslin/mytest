/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_CHANNEL_H
#define NV_HAIR_CHANNEL_H

#include <Nv/HairWorks/NvHairSdk.h>
#include <Nv/HairWorks/Internal/BackDoor/NvHairBackDoorChannel.h>

#include <Nv/Common/NvCoUniquePtr.h>
#include <Nv/Common/Container/NvCoArray.h>

// TODO: JS Needed cos FILE... need platform independent streams/files
#include <stdio.h>

namespace nvidia {
namespace HairWorks {

class HairChannel
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(HairChannel);

	struct Item
	{
		enum Type
		{
			TYPE_FLOAT,
			TYPE_FLOAT4,
			TYPE_FLOAT3,
			TYPE_BOOL,
			TYPE_UINT32,
			TYPE_INT32,
			TYPE_COUNT_OF,
		};

		Item(const char* name, UInt32& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_UINT32;
			m_isDesc = isDesc;
			m_uint = &v;
		}
		Item(const char* name, Int32& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_INT32;
			m_isDesc = isDesc;
			m_int = &v;
		}
		Item(const char* name, Float32& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_FLOAT;
			m_isDesc = isDesc;
			m_float = &v;
		}
		Item(const char* name, Bool& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_BOOL;
			m_isDesc = isDesc;
			m_state = &v;
		}
		Item(const char* name, gfsdk_float4& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_FLOAT4;
			m_isDesc = isDesc;
			m_float4 = &v;
		}
		Item(const char* name, gfsdk_float3& v, Bool isDesc = true)
		{
			m_command = name;
			m_type = TYPE_FLOAT3;
			m_isDesc = isDesc;
			m_float3 = &v;
		}

		const char* m_command;	// override command
		Bool m_override;		// true if we are overriding this parameter
		Bool m_isDesc;		// is part of the desc or not
		Type m_type;
		union
		{
			Float32* m_float;
			UInt32* m_uint;
			Int32* m_int;
			gfsdk_float3* m_float3;
			gfsdk_float4* m_float4;
			Bool* m_state;
		};
	};

		/// Update this descriptor
	void updateDescriptor(InstanceDescriptor& descriptor, Bool verbose = false);

		/// Write to the log file
	void logMessage(const char *fmt,...);

		/// Get the channel
	BackDoorChannel* getChannel() const { return m_channel; }

		/// Must be called after construction to be able to use. Must return success for functional channel
	Result init();
	
		/// Reset stats (renderCount etc)
	void resetStats();

		/// Ctor
	HairChannel();
		/// Dtor
	~HairChannel();

	Int	m_renderCount;
	Int	m_renderCallCount;

private:
	// disable
	void operator=(const ThisType& rhs);

	void _updateItem(Int argc, const char*const* argv, Item& item);
	void _updateDescriptor(const Item& item, InstanceDescriptor& descriptor, Bool verbose);
	IndexT _indexOfItem(const char* name) const;

	NvCo::UniquePtr<BackDoorChannel> m_channel;

	InstanceDescriptor m_override;
	int	m_overrideCount;

	NvCo::Array<Item> m_items;

	Int	m_lastRenderCount;
	Int	m_lastRenderCallCount;

	FILE* m_logFile;
};


} // namespace HairWorks
} // namespace nvidia

#endif
