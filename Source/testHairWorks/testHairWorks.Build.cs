// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class testHairWorks : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    private string ThirdPartyPath //���������õĵ��������Ŀ¼
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
    private string MyLibPath //��������MyTestLib��Ŀ¼
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "HairLib")); }
    }
    public testHairWorks(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
                "testHairWorks/Public",
                "Engine/Public",
                "Engine/Classes",
                "UnrealEd/Public",
                "UnrealEd/Classes",
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"testHairWorks/Private",
                "Engine/Private",
                "UnrealEd/Private",
                "../../../../Source/Runtime/Renderer/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "UnrealEd",
                "Engine",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
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
        LoadThirdPartyLib(Target); //���ص�������
    }
    public bool LoadThirdPartyLib(TargetInfo Target)
    {
        bool isLibrarySupported = false;
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))//ƽ̨�ж�
        {

            isLibrarySupported = true;
            System.Console.WriteLine("----- isLibrarySupported true");
            PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {

                        "D3D12RHI",
                        "UnrealEd"
                    });

            PrivateIncludePaths.AddRange(
                new string[]
                {
                    // ... add other private include paths required here ...
				        "../../../../Source/Runtime/Windows/D3D12RHI/Private",
                        "../../../../Source/Runtime/Windows/D3D12RHI/Private/Windows",
                });
            string PlatformSubPath = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            //string LibrariesPath = Path.Combine(MyTestLibPath, "Lib");
            //PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, PlatformSubPath, "TestLib.lib"));//���ص�������̬��.lib
            //���ص�������̬��
            string DllPath = Path.Combine(MyLibPath, "Dll");
            PublicDelayLoadDLLs.Add(Path.Combine(DllPath, PlatformSubPath, "TestDll.dll"));
            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(DllPath, PlatformSubPath, "TestDll.dll")));
            AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
            MinFilesUsingPrecompiledHeaderOverride = 1;
            bFasterWithoutUnity = true;
        }

        if (isLibrarySupported) //�ɹ����ؿ������£��������������ͷ�ļ�
        {
            // Include path
            System.Console.WriteLine("----- PublicIncludePaths.Add true");
            PublicIncludePaths.Add(Path.Combine(MyLibPath, "Include"));
        }

      
        return isLibrarySupported;
    }
}
