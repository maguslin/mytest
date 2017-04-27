// Fill out your copyright notice in the Description page of Project Settings.

#include "IPluginManager.h"
#include "HairworksFactory.h"
#include "UHairworksAsset.h"
#include "UHairworksMaterial.h"
#include "testHairWorksPrivatePCH.h"
#include "SkelImport.h"
#include <Nv/Common/NvCoMemoryReadStream.h>
#define LOCTEXT_NAMESPACE "UHairWorksFactory"
UHairworksFactory::UHairworksFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	Formats.Add(TEXT("apx;HairWorks XML Asset"));
	SupportedClass = UHairworksAsset::StaticClass(); 
}

FText UHairworksFactory::GetDisplayName() const
{
	return LOCTEXT("HairworksImportFactoryDescription", "Hairworks");
}
bool UHairworksFactory::FactoryCanImport(const FString& Filename)
{
	if (GetSDK() == nullptr)
		return false;

	TArray<uint8> Buffer;
	FFileHelper::LoadFileToArray(Buffer, *Filename);

	auto HairAssetId = NvHair::ASSET_ID_NULL;
	NvCo::MemoryReadStream ReadStream(Buffer.GetData(), Buffer.Num());
	GetSDK()->loadAsset(&ReadStream, HairAssetId);

	if (HairAssetId != NvHair::ASSET_ID_NULL)
	{
		GetSDK()->freeAsset(HairAssetId);
		return true;
	}
	else
		return false;
}
 UObject* UHairworksFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	 FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
	 auto HairAssetId = NvHair::ASSET_ID_NULL;

	 // Create UHairWorksAsset
	 NvCo::MemoryReadStream ReadStream(Buffer, BufferEnd - Buffer);
	 GetSDK()->loadAsset(&ReadStream, HairAssetId, nullptr, &GetAssetConversionSettings());
	 if (HairAssetId == NvHair::ASSET_ID_NULL)
	 {
		 FEditorDelegates::OnAssetPostImport.Broadcast(this, nullptr);
		 return nullptr;
	 }
	 auto* Hair = NewObject<UHairworksAsset>(InParent, InName, Flags);
	 Hair->AssetId = HairAssetId;

	 // Initialize hair
	 InitHairAssetInfo(*Hair);

	 // Setup import data
	 Hair->AssetImportData->Update(UFactory::CurrentFilename);

	 // Set data
	 Hair->AssetData.Append(Buffer, BufferEnd - Buffer);

	 FEditorDelegates::OnAssetPostImport.Broadcast(this, Hair);
	 return Hair;
}
//UObject* UHairworksFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
//{
//	UHairworksAsset* NewObjectAsset = ConstructObject<UHairworksAsset>(Class, InParent, Name, Flags | RF_Transactional);
//	return NewObjectAsset;
//}
 void UHairworksFactory::InitHairAssetInfo(UHairworksAsset& Hair, const NvHair::InstanceDescriptor* NewInstanceDesc)
 {
	 check(GetSDK() != nullptr);
	 auto& HairSdk = *GetSDK();
	 {
		 nvidia::Int BoneNum = HairSdk.getNumBones(Hair.AssetId);
		 Hair.BoneNames.Empty(BoneNum);
		 for (nvidia::Int Idx = 0; Idx < BoneNum; ++Idx)
		 {
			 nvidia::Char BoneName[NV_HAIR_MAX_STRING];
			 HairSdk.getBoneName(Hair.AssetId, Idx, BoneName);

			 Hair.BoneNames.Add(*FixupBoneName(BoneName));//FSkeletalMeshImportData::
		 }
	 }

	 // Bone lookup table
	 Hair.InitBoneLookupTable();

	 // Get material
	 if (Hair.bMaterials)
	 {
		 NvHair::InstanceDescriptor HairInstanceDesc;
		 if (NewInstanceDesc)
			 HairInstanceDesc = *NewInstanceDesc;
		 else
			 HairSdk.getInstanceDescriptorFromAsset(Hair.AssetId, HairInstanceDesc);

		 // sRGB conversion
		 auto ConvertColorToSRGB = [](gfsdk_float4& Color)
		 {
			 reinterpret_cast<FLinearColor&>(Color) = FLinearColor(FColor(Color.x * 255, Color.y * 255, Color.z * 255));
		 };

		 ConvertColorToSRGB(HairInstanceDesc.m_rootColor);
		 ConvertColorToSRGB(HairInstanceDesc.m_tipColor);
		 ConvertColorToSRGB(HairInstanceDesc.m_specularColor);
		 // Because of SRGB conversion, we need to use a different diffuse blend value to keep consistent with HairWorks Viewer.
		 HairInstanceDesc.m_diffuseBlend = 1 - FMath::Pow(1 - HairInstanceDesc.m_diffuseBlend, 2.2f);

		 // UE4 shadow attenuation is different from HairWorks viewer, so we use a different value to keep consistent
		 HairInstanceDesc.m_shadowSigma /= 2;
		 HairInstanceDesc.m_shadowSigma = FMath::Min(HairInstanceDesc.m_shadowSigma, 254.f / 255.f);

		 // Fill hair material
		 if (HairInstanceDesc.m_hairNormalBoneIndex >= 0 && HairInstanceDesc.m_hairNormalBoneIndex < Hair.BoneNames.Num())
			 Hair.HairMaterial->HairNormalCenter = Hair.BoneNames[HairInstanceDesc.m_hairNormalBoneIndex];
		 else
			 Hair.HairMaterial->HairNormalCenter = "";
	 }
 }
 FString UHairworksFactory::FixupBoneName(const FString &InBoneName)//for some reason i cant link the method??
 {
	 FString BoneName = InBoneName;

	 BoneName.Trim();
	 BoneName.TrimTrailing();
	 BoneName = BoneName.Replace(TEXT(" "), TEXT("-"));

	 return BoneName;
 }
#undef LOCTEXT_NAMESPACE

