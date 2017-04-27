// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "IPluginManager.h"
#include "testHairWorksPrivatePCH.h"
#include "TestDll.h"  // Only needed to load the Dll on windows
#include "DynamicRHI.h"
#define LOCTEXT_NAMESPACE "FtestHairWorksModule"

typedef int(*_myPrint)(int age1, int age);
_myPrint m_myPrintFromDll;
void* hairSdk = NULL;
NvHair::Sdk* GetSDK()
{
	return (NvHair::Sdk*)hairSdk;
}

NvHair::ConversionSettings AssetConversionSettings;
const NvHair::ConversionSettings& GetAssetConversionSettings()
{
	return AssetConversionSettings;
}
void FtestHairWorksModule::StartupModule()
{
	if (!GDynamicRHI)
	{
		return;
	}
	auto device = reinterpret_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
	//這個判斷 後面在做，需要判斷是不是dx12
	if (!device )
	{
		UE_LOG(LogTemp, Error, TEXT("Need D3D_FEATURE_LEVEL_12_0."));
		return;
	}

	// Check multi thread support
	/*if ((D3DDevice.GetCreationFlags() & D3D12_CREATE_DEVICE_SINGLETHREADED) != 0)
	{
		UE_LOG(LogHairWorks, Error, TEXT("Can't work with D3D12_CREATE_DEVICE_SINGLETHREADED."));
		return;
	}*/
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if PLATFORM_64BITS
	FString platform = "Win64/";
#else
	FString platform = "Win32/";
#endif
	FString path =  *FPaths::EnginePluginsDir();//IPluginManager::Get().FindPlugin("testHairWorks")->GetBaseDir();
	FString dllPath = path + _TEXT("/testHairWorks") + "/ThirdParty/HairLib/" + platform + "Dll/" + "NvHairWorksDx12.win64.dll";
	hairSdk = NvHair::loadSdk(TCHAR_TO_ANSI(*dllPath), NV_HAIR_VERSION);
	if (!hairSdk)
		UE_LOG(LogTemp, Log, TEXT("Load sdk failed!"))
	else
		UE_LOG(LogTemp, Log, TEXT("Load sdk Success!"))


	AssetConversionSettings.m_targetHandednessHint = NvHair::HandednessHint::LEFT;
	AssetConversionSettings.m_targetUpAxisHint = NvHair::AxisHint::Z_UP;
	//FString dllPath = path +_TEXT("/testHairWorks")+ "/ThirdParty/HairLib/" + platform + "Dll/" + "TestDll.dll";
	//hairPtr = FPlatformProcess::GetDllHandle(*dllPath);
	//if (!hairPtr)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Failed to load HairWorks library."))
	//}
	//if (importTestDll())
	//{
	//	TCHAR buff[5];
	//	_stprintf(buff, _T("%d"), myPrint(1000 , 456));
	//	UE_LOG(LogTemp, Log, TEXT("MyPrint--------------"));
	//	UE_LOG(LogTemp, Log, buff);
	//}
}
//bool FtestHairWorksModule::importTestDll()
//{
//	if (hairPtr != NULL)
//	{
//		m_myPrintFromDll = NULL;
//		FString procName = "add";	// Needs to be the exact name of the DLL method.
//		m_myPrintFromDll = (_myPrint)FPlatformProcess::GetDllExport(hairPtr, *procName);
//		if (m_myPrintFromDll != NULL)
//		{
//			return true;
//		}
//	}
//	return false;
//}
//int FtestHairWorksModule::myPrint(int age1, int age)
//{
//	if (m_myPrintFromDll != NULL)
//	{
//		int out = int(m_myPrintFromDll(age1, age)); // Call the DLL method with arguments corresponding to the exact signature and return type of the method.
//		return out;
//	}
//	return 0;	// Return an error.
//}

void FtestHairWorksModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//FPlatformProcess::FreeDllHandle(hairPtr);
	if(hairSdk)
	static_cast<NvHair::Sdk*>(hairSdk)->release();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FtestHairWorksModule, testHairWorks)