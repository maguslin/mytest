/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_VERSION_UTIL_H
#define NV_HAIR_VERSION_UTIL_H

#include <Nv/Common/NvCoCommon.h>

namespace nvidia {
namespace HairWorks {

/*! Simple utility that provides functions for converting between version strings, and numbers and back again */
struct VersionUtil
{
		/*! \brief Converts a version number into a string of format major.minor.point
		\param [in] version The version number 
		\param [out] out Buffer to hold the string. Must be large enough to hold output. 16 chars will be more than enough.
		\return The number of characters in the string NOT including terminating \0 */
	static Int versionToString(Int version, Char* out);
		/*! \brief Given a \0 terminated string in format major.minor.point returns the version number. NOTE minor and point can only be a single digit
		\param [in] in Terminated string that holds version number.
		\return If >= 0 it is the version number. < 0 indicates an error, and the input couldn't be parsed. */
	static Int stringToVersion(const Char* in);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_VERSION_UTIL_H
