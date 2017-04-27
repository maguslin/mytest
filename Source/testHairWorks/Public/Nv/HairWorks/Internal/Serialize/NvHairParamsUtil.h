/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "Nv.h"

#include <Nv/HairWorks/NvHairSdk.h>
#include <Nv/Common/NvCoStream.h>

namespace nvidia {
namespace HairWorks {

// Forward define
struct ParamsContext;

class ParamsUtil
{
	public:
		/// Initialize a parameter context - needed before any reading/writing can be done
	static ParamsContext* init(NvAllocatorCallback* allocatorCallback, NvErrorCallback* errorCallback);
		/// Destroy a context
	static void destroy(ParamsContext* context);

		/// Load from a stream using the context
	static Result load(ParamsContext* context, NvCo::ReadStream* stream, AssetHeaderInfo* outInfo, AssetDescriptor* assetDescOut, InstanceDescriptor*	materialsOut, char* textureNamesOut[]);

		/// Save to a stream using the context
	static Result save(ParamsContext* context, NvCo::WriteStream* stream, ESerializeFormat format, const AssetHeaderInfo* info, const AssetDescriptor* assetDesc, 
		const InstanceDescriptor* materials, int numMaterials, const char* textureNames[]);
};


} // namespace HairWorks
} // namespace nvidia