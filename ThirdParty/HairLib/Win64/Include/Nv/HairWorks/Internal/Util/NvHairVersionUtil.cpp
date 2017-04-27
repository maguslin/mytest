/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairVersionUtil.h"

namespace nvidia {
namespace HairWorks { 

static Void _reverse(Char* start, Char* end)
{
	const Int len = Int(end - start) / 2;
	--end;
	for (Int i = 0; i < len; i++)
	{
		// Swap
		const Char c = start[i];
		start[i] = end[-i];
		end[-i] = c;
	}
}

/* static */ Int VersionUtil::stringToVersion(const Char* in)
{	
	// Get major version number
	Int major = 0;
	for (; *in >= '0' && *in <= '9'; in++)
	{
		major = major * 10 + (*in - '0');
	}
	Int minor = 0;
	Int point = 0;

	// Minor
	if (*in)
	{
		if (in[0] == '.' && in[1] >= '0' && in[1] <= '9')
		{
			minor = in[1] - '0';
			in += 2;
		}
	}
	// Point
	if (*in)
	{
		if (in[0] == '.' && in[1] >= '0' && in[1] <= '9')
		{
			point = in[1] - '0';
			in += 2;
		}
	}

	return (*in == 0) ? (major * 100 + minor * 10 + point) : -1;
}

/* static */ Int VersionUtil::versionToString(Int version, Char* out)
{
	NV_CORE_ASSERT(version >= 0);
	const Int point = version % 10;
	version = version / 10;
	const Int minor = version % 10;
	version = version / 10;

	// Convert version to text
	Char* cur = out;
	do
	{
		*cur++ = Char('0' + (version % 10));
		version /= 10;
	} while (version);

	_reverse(out, cur);

	if (minor || point)
	{
		*cur++ = '.';
		*cur++ = Char('0' + minor);
		if (point)
		{
			*cur++ = '.';
			*cur++ = Char('0' + point);
		}
	}
	// End terminator
	*cur = 0;	
	return Int(cur - out);
}

} // namespace HairWorks
} // namespace nvidia