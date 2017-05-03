// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "HairWorksInstance.generated.h"
class UHairworksMaterial;
class UHairworksAsset;

USTRUCT(BlueprintType)
struct  FHairworksInstance
{
	GENERATED_USTRUCT_BODY()

	/** Assign HairWorks asset here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Asset)
	UHairworksAsset* Hair;

	/** Whether override Hair Material of the HairWorks asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Asset)
	bool bOverride = false;

	/** A Hair Material to override the Hair Material of the HairWorks asset. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = Hair)
	UHairworksMaterial* HairMaterial;
};