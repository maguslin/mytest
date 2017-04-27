// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include <Nv/HairWorks/NvHairSdk.h> // hairworks main header file
#include <Nv/HairWorks/Platform/Win/NvHairWinLoadSdk.h>  // Only needed to load the Dll on windows
#include "AllowWindowsPlatformTypes.h" 
#include <Nv/Common/Platform/Dx12/NvCoDx12Handle.h>
#include <Nv/HairWorks/Platform/Dx12/NvHairDx12SdkHandle.h>
#include "HideWindowsPlatformTypes.h"
//#include <Nv/Common/Platform/StdC/NvCoStdCFileReadStream.h>
#include <string>
#include "ModuleManager.h"

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

