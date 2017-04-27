
@if "%PERL%"=="" set PERL=%HW_ROOT%\build\tools\perl\5.8.8_822\bin\perl

@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairAssetDescriptor -namespace nvidia::parameterized::legacy::ver1p0 NvHairDescriptorParams1p0.pl NvHairAssetDescriptor1p0.h NvHairAssetDescriptor1p0.cpp

@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairInstanceDescriptor -namespace nvidia::parameterized::legacy::ver1p0 NvHairDescriptorParams1p0.pl NvHairInstanceDescriptor1p0.h NvHairInstanceDescriptor1p0.cpp

@REM All the regular ones named 
@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairAssetDescriptor NvHairDescriptorParams.pl NvHairAssetDescriptor.h NvHairAssetDescriptor.cpp
@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairInstanceDescriptor NvHairDescriptorParams.pl NvHairInstanceDescriptor.h NvHairInstanceDescriptor.cpp
@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairSceneDescriptor NvHairDescriptorParams.pl NvHairSceneDescriptor.h NvHairSceneDescriptor.cpp
@%PERL% %NV_PARAMETERIZED_BUILD%\GenParameterized.pl -force -only HairWorksInfo NvHairDescriptorParams.pl NvHairAssetHeaderInfo.h NvHairAssetHeaderInfo.cpp

@REM Can't use this because the filenames don't conform
@REM @%PERL% %HW_ROOT%\external\NvParameterized\1.0\trunk\build\scripts\GenParameterized.pl -force NvHwDescriptorParams.pl . .

