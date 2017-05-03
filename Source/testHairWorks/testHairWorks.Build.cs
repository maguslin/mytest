// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class testHairWorks : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    private string ThirdPartyPath //这个插件引用的第三方库的目录
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
    private string MyLibPath //第三方库MyTestLib的目录
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "HairLib")); }
    }
    public testHairWorks(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
                "testHairWorks/Public",
                "../../../Source/Runtime/Engine/Public",
                "../../../Source/Runtime/Engine/Classes",
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"testHairWorks/Private",
                "../../../Source/Runtime/Engine/Private",
                "../../../Source/Runtime/Renderer/Private",
				// ... add other private include paths required here ...
			}
			);


   //     PublicDependencyModuleNames.AddRange(
			//new string[]
			//{
			//	"Core",
   //             "UnrealEd",
   //             "Engine",
			//	// ... add other public dependencies that you statically link with here ...
			//}
			//);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "UnrealEd",
                "Engine",
                "InputCore",
                "RHI",
                "Core",
                "RenderCore",
                "Renderer",
                "ShaderCore",
                "Slate",
				"SlateCore",
                "MediaAssets",
				// ... add private dependencies that you statically link with here ...	
			}
			);
        if (UEBuildConfiguration.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
        //if (UEBuildConfiguration.bBuildEditor == true)
        //{
        //    PrivateDependencyModuleNames.Add("UnrealEd");
        //}

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
        LoadThirdPartyLib(Target); //加载第三方库
    }
    public bool LoadThirdPartyLib(TargetInfo Target)
    {
        bool isLibrarySupported = false;
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))//平台判断
        {

            isLibrarySupported = true;
            System.Console.WriteLine("----- isLibrarySupported true");
            PrivateIncludePaths.AddRange(
                  new string[]
                  {
                        "../../../Source/Runtime/Windows/D3D11RHI/Private",
                        "../../../Source/Runtime/Windows/D3D11RHI/Private/Windows",
                        "../../../Source/Runtime/Windows/D3D12RHI/Private",
                        "../../../Source/Runtime/Windows/D3D12RHI/Private/Windows",
                  });
            PrivateDependencyModuleNames.AddRange(
                   new string[]
                   {
                        "D3D11RHI",
                        "D3D12RHI",
                   });
            string PlatformSubPath = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            //string LibrariesPath = Path.Combine(MyTestLibPath, "Lib");
            //PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, PlatformSubPath, "TestLib.lib"));//加载第三方静态库.lib
            //加载第三方动态库
            string DllPath = Path.Combine(MyLibPath, "Dll");
            PublicDelayLoadDLLs.Add(Path.Combine(DllPath, PlatformSubPath, "TestDll.dll"));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(DllPath, PlatformSubPath, "TestDll.dll")));
            AddEngineThirdPartyPrivateStaticDependencies(Target, "DX11");
            AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
            MinFilesUsingPrecompiledHeaderOverride = 1;
            bFasterWithoutUnity = true;
        }

        if (isLibrarySupported) //成功加载库的情况下，包含第三方库的头文件
        {
            // Include path
            System.Console.WriteLine("----- PublicIncludePaths.Add true");
            PublicIncludePaths.Add(Path.Combine(MyLibPath, "Include"));
        }

      
        return isLibrarySupported;
    }
}
