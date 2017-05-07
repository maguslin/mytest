// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include <Nv/HairWorks/NvHairSdk.h> // hairworks main header file
#include <Nv/HairWorks/Platform/Win/NvHairWinLoadSdk.h>  // Only needed to load the Dll on windows
#include "AllowWindowsPlatformTypes.h" 
#include <Nv/Common/Platform/Dx11/NvCoDx11Handle.h>
#include "HideWindowsPlatformTypes.h"
//#include <Nv/Common/Platform/StdC/NvCoStdCFileReadStream.h>
#include <string>
#include "ModuleManager.h"

struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

class ID3DHelper
{
public:
	virtual ID3D11DeviceContext* GetDeviceContext(const IRHICommandContext&) = 0;
	virtual ID3D11ShaderResourceView* GetShaderResourceView(FRHITexture2D*) = 0;
	virtual void CommitShaderResources(IRHICommandContext&) = 0;
};
ID3DHelper& GetD3DHelper();
class FtestHairWorksModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool importTestDll();
	//int myPrint(int age1, int age);
private:
	//NvHair::Sdk* hairSdk;
	NvHair::AssetId assetId;
	
};

