// Fill out your copyright notice in the Description page of Project Settings.

#include "IPluginManager.h"
#include "UHairworksAsset.h"
#include "testHairWorksPrivatePCH.h"
#include "UHairworksMaterial.h"
#include <Nv/Common/NvCoMemoryReadStream.h>
#include <Nv/HairWorks/NvHairSdk.h> // hairworks main header file

UHairworksAsset::UHairworksAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	AssetId(NvHair::ASSET_ID_NULL)
{
}
UHairworksAsset::~UHairworksAsset()
{
	if (AssetId != NvHair::ASSET_ID_NULL)
		GetSDK()->freeAsset(AssetId);
}
void UHairworksAsset::InitBoneLookupTable()
{
	BoneNameToIdx.Empty(BoneNames.Num());
	for (auto Idx = 0; Idx < BoneNames.Num(); ++Idx)
	{
		BoneNameToIdx.Add(BoneNames[Idx], Idx);
	}
}
void UHairworksAsset::Serialize(FArchive & Ar)
{
	Super::Serialize(Ar);
}

void UHairworksAsset::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif

	if (!HasAnyFlags(RF_NeedLoad))
		HairMaterial = NewObject<UHairworksMaterial>(this, NAME_None, GetMaskedFlags(RF_PropagateToSubObjects));

	Super::PostInitProperties();
}

void UHairworksAsset::PostLoad()
{
	Super::PostLoad();

	// Preload asset
	if (GetSDK() == nullptr)
		return;

	auto& HairSdk = *GetSDK();

	check(AssetId == NvHair::ASSET_ID_NULL);
	NvCo::MemoryReadStream ReadStream(AssetData.GetData(), AssetData.Num());
	HairSdk.loadAsset(&ReadStream, AssetId, nullptr, &GetAssetConversionSettings());

	// Initialize pins
	if (AssetId != NvHair::ASSET_ID_NULL && HairMaterial->Pins.Num() == 0)
		InitPins();

	// Setup bone lookup table
	InitBoneLookupTable();
}

#if WITH_EDITOR
void UHairworksAsset::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	// Let HairWorks components to update their rendering data
	/*for (TObjectIterator<UHairWorksComponent> It; It; ++It)
	{
		if (It->HairInstance.Hair == this)
			It->MarkRenderDynamicDataDirty();
	}*/

	// Call parent
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UHairworksAsset::InitPins() const
{
	// Empty engine pins
	check(AssetId != NvHair::ASSET_ID_NULL);
	check(GetSDK() != nullptr);

	HairMaterial->Pins.Empty();

	// Get pins
	auto& HairSdk = *GetSDK();
	TArray<NvHair::Pin> Pins;
	Pins.AddDefaulted(HairSdk.getNumPins(AssetId));
	if (Pins.Num() == 0)
		return;

	HairSdk.getPins(AssetId, 0, Pins.Num(), Pins.GetData());

	// Add pin to engine pins
	for (const auto& Pin : Pins)
	{
		FHairWorksPin EnginePin;
		EnginePin.Bone = BoneNames[Pin.m_boneIndex];
		EnginePin.bDynamicPin = Pin.m_useDynamicPin;
		EnginePin.bTetherPin = Pin.m_doLra;
		EnginePin.Stiffness = Pin.m_pinStiffness;
		EnginePin.InfluenceFallOff = Pin.m_influenceFallOff;
		EnginePin.InfluenceFallOffCurve = reinterpret_cast<const FVector4&>(Pin.m_influenceFallOffCurve);

		HairMaterial->Pins.Add(EnginePin);
	}
}

