// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "IPluginManager.h"
#include "testHairWorksPrivatePCH.h"
#include "TestDll.h"  // Only needed to load the Dll on windows
#include "DynamicRHI.h"
//#include "AllowWindowsPlatformTypes.h" 
//#include "d3dx12.h"
//#include "HideWindowsPlatformTypes.h"
//#include "RenderResource.h"
//#include "D3D12RHIPrivate.h"
//#include "D3D12Util.h"
//#include "D3D12Resources.h"
#include "Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h"
#include "AllowWindowsPlatformTypes.h" 
#include <d3d11.h>
#include "HideWindowsPlatformTypes.h"
#define LOCTEXT_NAMESPACE "FtestHairWorksModule"

class FHairWorksD3DHelper : public ID3DHelper
{
	virtual ID3D11DeviceContext* GetDeviceContext(const IRHICommandContext& CmdContext) override
	{
		auto& RHI = static_cast<const FD3D11DynamicRHI&>(CmdContext);
		return RHI.GetDeviceContext();
	}

	virtual ID3D11ShaderResourceView* GetShaderResourceView(FRHITexture2D* Texture) override
	{
		auto* D3D11Texture = static_cast<TD3D11Texture2D<FD3D11BaseTexture2D>*>(Texture);
		return D3D11Texture ? D3D11Texture->GetShaderResourceView() : nullptr;
	}

	virtual void CommitShaderResources(IRHICommandContext& CmdContext) override
	{
		auto& RHI = static_cast<FD3D11DynamicRHI&>(CmdContext);


		//RHI.CommitNonComputeShaderConstants();
		//RHI.CommitGraphicsResourceTables();
	}
};

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
	ID3D11Device* device = reinterpret_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
	ID3D11DeviceContext *deviceContext = NULL;
	device->GetImmediateContext(&deviceContext);
	check(deviceContext);
	
	
	//ID3D11DeviceContext* deviceContext = GDynamicRHI->RHIGetDefaultContext()
	//FD3D12Device* deviceP = static_cast<FD3D12CommandContext*>(GDynamicRHI->RHIGetDefaultContext())->GetParentDevice();
	////FD3D12CommandContext& CmdList = deviceP->GetDefaultCommandContext();//.CommandListHandle.CommandList();
	//ID3D12GraphicsCommandList* CmdList = deviceP->GetDefaultCommandContext().CommandListHandle.CommandList();
	//
	//if (!CmdList)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("No Command List Get."));
	//}

	////�@���Д� ������������Ҫ�Д��ǲ���dx12
	//if (!device )
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Need D3D_FEATURE_LEVEL_12_0."));
	//	return;
	//}
	//NvCo::Dx12TargetInfo m_targetInfo;
	//NvHair::Dx12InitInfo initInfo;
	//m_targetInfo.init();
	//m_targetInfo.m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//m_targetInfo.m_renderTargetFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//initInfo.m_targetInfo = m_targetInfo;
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
	//FString dllPath = path + _TEXT("/testHairWorks") + "/ThirdParty/HairLib/" + platform + "Dll/" + "NvHairWorksDx12.win64.dll";
	FString dllPath = path + _TEXT("/testHairWorks") + "/ThirdParty/HairLib/" + platform + "Dll/" + "NvHairWorksDx11.win64.dll";
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
	static_cast<NvHair::Sdk*>(hairSdk)->initRenderResources(NvCo::Dx11Type::wrap(device), NvCo::Dx11Type::wrap(deviceContext));
	//static_cast<NvHair::Sdk*>(hairSdk)->initRenderResources(NvCo::Dx12Type::wrap(device), NvCo::Dx12Type::wrap(CmdList), NvHair::Dx12SdkType::wrapPtr(&initInfo));
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