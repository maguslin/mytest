// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "UHairworksAsset.generated.h"
namespace nvidia {
	namespace HairWorks {
		enum AssetId;
	}
}
class UHairworksMaterial;
/**
 * 
 */
UCLASS(BlueprintType)
class  UHairworksAsset : public UObject
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()
#if WITH_EDITORONLY_DATA
		/** Importing data and options used for this HairWorks asset */
		UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
		class UAssetImportData* AssetImportData;
#endif
	UPROPERTY(EditAnywhere, Category = ImportSettings)
		bool bGroom = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
		bool bMaterials = true;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
		bool bConstraints = true;

	UPROPERTY(VisibleAnywhere, Category = ImportSettings)
		bool bTextures = false;

	UPROPERTY(EditAnywhere, Category = ImportSettings)
		bool bCollisions = true;


	// Begin UObject interface.
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End UObject interface.

	void InitPins()const;
	void InitBoneLookupTable();
	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly, Category = Hair)
		UHairworksMaterial* HairMaterial;

	UPROPERTY()
		TArray<uint8> AssetData;

	UPROPERTY()
		TArray<FName> BoneNames;

	nvidia::HairWorks::AssetId AssetId;

	~UHairworksAsset();

	/** Bone look up table */
	TMap<FName, int> BoneNameToIdx;

	uint32 PinsUpdateFrameNumber = -1;	// Update pin data once per frame
};

