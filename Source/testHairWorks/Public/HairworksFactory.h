// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UnrealEd.h"
#include "Factories/Factory.h"
#include "HairworksFactory.generated.h"
namespace nvidia {
	namespace HairWorks {
		enum AssetId;
		struct InstanceDescriptor;
	}
}

class UHairworksAsset;
/**
 * 
 */
UCLASS()
class  UHairworksFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
		//~ Begin UFactory Interface
		virtual FText GetDisplayName() const override;
		virtual bool FactoryCanImport(const FString& Filename) override;

		virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;

		// UFactory interface
	//	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
protected:
	static void InitHairAssetInfo(UHairworksAsset& Hair, const nvidia::HairWorks::InstanceDescriptor* NewInstanceDesc = nullptr);
	static FString FixupBoneName(const FString &InBoneName);
};
