/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SLI_SYSTEM_H
#define NV_HAIR_SLI_SYSTEM_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoApiHandle.h>

namespace nvidia {
namespace HairWorks {

class SliSystem
{
	struct _Handle;

	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(SliSystem);
	typedef _Handle* Handle;

	virtual void setResourceHint(Handle& handleInOut, const NvCo::ApiBuffer& buffer, const char* bufferName) {NV_UNUSED(handleInOut); NV_UNUSED(buffer); NV_UNUSED(bufferName); }
	virtual void earlyPushStart(Handle& handleInOut, const NvCo::ApiBuffer& buffer, const char* bufferName) {NV_UNUSED(handleInOut); NV_UNUSED(buffer); NV_UNUSED(bufferName); };
	virtual void earlyPushFinish(Handle handle, const char* bufferName) { NV_UNUSED(handle);  NV_UNUSED(bufferName); }
	virtual void flushLog() {};
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_SLI_SYSTEM_H
