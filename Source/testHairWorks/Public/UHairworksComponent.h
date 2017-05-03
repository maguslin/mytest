// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "HairworksInstance.h"
#include "UHairworksComponent.generated.h"
namespace nvidia {
	namespace HairWorks {
		enum InstanceId;
	}
}
/**
 * 
 */
UCLASS()
class  UHairworksComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hair, meta = (ShowOnlyInnerProperties))
		FHairworksInstance HairInstance;

	//~ Begin UPrimitiveComponent interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual void OnAttachmentChanged() override;
	//~ End UPrimitiveComponent interface

	//~ Begin UActorComponent interface
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual bool ShouldCreateRenderState() const override;
	virtual void CreateRenderState_Concurrent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual FActorComponentInstanceData* GetComponentInstanceData() const override;
	//~ End UActorComponent interface

	//~ Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent interface.

	//~ Begin UObject interface.
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostInitProperties() override;
	//~ End UObject interface.

	/** Creates a unique instance of the hair material (and sets the override flag on the instance) so the material can be modified at runtime */
	UFUNCTION(BlueprintCallable, Category = "Hair")
		void CreateHairMaterialDynamic();

	/** Obtains the hair material, used in conjunction with the CreateHairMaterialDynamic function to obtain the unique instance hair material */
	UFUNCTION(BlueprintPure, Category = "Hair")
		UHairworksMaterial* GetHairMaterial();

protected:
	/** Send data for rendering */
	void SendHairDynamicData(bool bForceSkinning = false)const;

	/** Bone mapping */
	void SetupBoneMapping();

	/** Update bones */
	void UpdateBoneMatrices();

	/** Parent skeleton */
	UPROPERTY()
		USkinnedMeshComponent* ParentSkeleton;

	/** Bone remapping */
	TArray<uint16> BoneIndices;

	/** Skinning data*/
	TArray<FMatrix> BoneMatrices;
};
