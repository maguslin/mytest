/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#define NV_HAIR_USE_NVAPI 1
#define NV_HAIR_USE_NVAPI_LOG 1

#include "NvHairDx11SliSystem.h"
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>

// For the log
#include <stdio.h>

#if NV_HAIR_USE_NVAPI 
#	include "nvapi.h"
#endif

namespace nvidia {
namespace HairWorks {

#ifdef NV_HAIR_USE_NVAPI 

/* static */SliSystem* Dx11SliSystem::create(ID3D11Device* device)
{
	ThisType* impl = new Dx11SliSystem;
	Result res = impl->_init(device);
	if (NV_FAILED(res))
	{
		delete impl;
		return NV_NULL;
	}
	return impl;
}

Dx11SliSystem::Dx11SliSystem() :
	m_logFile(NV_NULL),
	m_device(NV_NULL)
{
}

Dx11SliSystem::~Dx11SliSystem()
{
	if (m_logFile)
	{
		fclose(m_logFile);
	}
}

Bool Dx11SliSystem::_checkSli()
{
	NV_GET_CURRENT_SLI_STATE state;
	state.version = NV_GET_CURRENT_SLI_STATE_VER;
	NvAPI_Status status = NvAPI_D3D_GetCurrentSLIState(m_device, &state);
	return status == NVAPI_OK && state.numAFRGroups > 1;
}

Result Dx11SliSystem::_init(ID3D11Device* device)
{
	openLogFile();
	if (NVAPI_OK == NvAPI_Initialize())
		log("NVAPI Initialized\n");
	else
		log("NVAPI Initialize failed!\n");

	Bool hasSli = _checkSli();
	if (hasSli)
		log("Init Render Resource - SLI Enabled\n");
	else
		log("Init Render Resource - SLI Disabled\n");
	flushLog();
	m_device = device;
	return hasSli ? NV_OK : NV_FAIL;
}

Result Dx11SliSystem::openLogFile()
{
	if (m_logFile)
	{
		return NV_FAIL;
	}
	const char* val = getenv("NVAPI_LOGFILE_NAME");
	if (val == NV_NULL)
	{
		return NV_FAIL;
	}
	m_logFile = fopen(val, "w+");
	return NV_OK;
}

Void Dx11SliSystem::flushLog()
{
	if (m_logFile)
	{
		fflush(m_logFile);
	}
}

void Dx11SliSystem::log(const char* funcName, const char* bufferName, int status)
{
	if (m_logFile)
	{
		if (status == NVAPI_OK)
			fprintf(m_logFile, "%s for %s: OK\n", funcName, bufferName);
		else
			fprintf(m_logFile, "%s for %s: ERROR (%d)\n", funcName, bufferName, status);
	}
}

Void Dx11SliSystem::log(const char* text)
{
	if (m_logFile)
	{
		fprintf(m_logFile, "%s\n", text);
	}
}

void Dx11SliSystem::logFormat(const char* funcName, const Char* format, ...)
{
	NV_UNUSED(funcName);

	if (!m_logFile)
	{
		return;
	}
	va_list va;
	va_start(va, format);
	char msg[1024];
	vsprintf(msg, format, va);
	va_end(va);
	fprintf(m_logFile, msg);
}

void Dx11SliSystem::setResourceHint( Handle& handleInOut, const NvCo::ApiBuffer& bufferIn, const char* bufferName)
{
	ID3D11Buffer* buffer = NvCo::Dx11Type::cast<ID3D11Buffer>(bufferIn);
	NV_CORE_ASSERT(buffer);
	if (!buffer) return;

	NVDX_ObjectHandle& handle = (NVDX_ObjectHandle&)handleInOut;
	// no need to set it again
	if (handle != NVDX_OBJECT_NONE)
		return;
	
	::NvU32 hint = 1;
	if ( NVAPI_OK == NvAPI_D3D_GetObjectHandleForResource( m_device, buffer, &handle ))
	{
		NvAPI_Status status = NvAPI_D3D_SetResourceHint( m_device, handle, NVAPI_D3D_SRH_CATEGORY_SLI, NVAPI_D3D_SRH_SLI_APP_CONTROLLED_INTERFRAME_CONTENT_SYNC, &hint);
		if (status != NVAPI_OK)
			handle = NVDX_OBJECT_NONE;
		log("SetResourceHint", bufferName, status);
	}
	else
	{
		handle = NVDX_OBJECT_NONE;
		logFormat("SetResourceHint", "ERROR - Cannot get object handle for %s\n", bufferName);
	}
}

void Dx11SliSystem::earlyPushStart( Handle& handleInOut,  const NvCo::ApiBuffer& bufferIn, const char* bufferName)
{
	ID3D11Buffer* buffer = NvCo::Dx11Type::cast<ID3D11Buffer>(bufferIn);
	NV_CORE_ASSERT(buffer);
	if (!buffer) return;

	NVDX_ObjectHandle& handle = (NVDX_ObjectHandle&)handleInOut;
	NvAPI_Status status = NVAPI_OK;
	if (handle == NVDX_OBJECT_NONE)
		status = NvAPI_D3D_GetObjectHandleForResource( m_device, buffer, &handle);

	if ( status == NVAPI_OK )
	{
		status = NvAPI_D3D_BeginResourceRendering( m_device, handle, NVAPI_D3D_RR_FLAG_FORCE_KEEP_CONTENT );
		if (status != NVAPI_OK)
			handle = NVDX_OBJECT_NONE;

		log("BeginResourceRendering", bufferName, status);
	}
	else
	{
		handle = NVDX_OBJECT_NONE;
		logFormat("BeginResourceRendering", "ERROR - Cannot get object handle for %s\n", bufferName);
	}
}

void Dx11SliSystem::earlyPushFinish(Handle handleIn, const char* bufferName)
{
	NVDX_ObjectHandle handle = (NVDX_ObjectHandle)handleIn;

	if (handle != NVDX_OBJECT_NONE)
	{
		NvAPI_Status status = NvAPI_D3D_EndResourceRendering( m_device, handle, 0 );
		log("EndResourceRendering", bufferName, status);
	}
}

#else // NV_HAIR_USE_NVAPI 
/* static */SliSystem* Dx11SliSystem::create(ID3D11Device* device)
{
	return NV_NULL;
}
#endif // NV_HAIR_USE_NVAPI 


} // namespace HairWorks
} // namespace nvidia
