/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_SLI_SYSTEM_H
#define NV_HAIR_DX11_SLI_SYSTEM_H


#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>

#include <stdio.h>
#include <dxgi.h>
#include <d3d11.h>

namespace nvidia {
namespace HairWorks {

class Dx11SliSystem : public SliSystem
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11SliSystem, SliSystem);

	// SliSystem	
	Void flushLog() NV_OVERRIDE;
	void setResourceHint(Handle& handle, const NvCo::ApiBuffer& buffer, const char* bufferName) NV_OVERRIDE;
	void earlyPushStart(Handle& handle, const NvCo::ApiBuffer& buffer, const char* bufferName) NV_OVERRIDE;
	void earlyPushFinish(Handle handle, const char* bufferName) NV_OVERRIDE;

	Result openLogFile();
	Void log(const Char* funcName, const Char* bufferName, int status);
	Void logFormat(const char* funcName, const Char* format, ...);

	Void log(const char* text);
	
		/// Dtor
	~Dx11SliSystem();

		/// Create the system. Returns NV_NULL if not possible.
	static SliSystem* create(ID3D11Device* device);

protected:

	/// Ctor. Use create to construct.
	Dx11SliSystem();

	Result _init(ID3D11Device* device);

	Bool _checkSli();

	ID3D11Device* m_device;
	FILE* m_logFile;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_SLI_SYSTEM_H
