// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "HairworksPinTransformComponent.generated.h"


UCLASS(ClassGroup = Rendering, meta=(BlueprintSpawnableComponent), HideCategories = (Collision, Base, Object, PhysicsVolume))
class  UHairworksPinTransformComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pin", meta = (ClampMin = "0"))
		int32 PinIndex;

protected:
	//~ Begin USceneComponent Interface
	virtual void OnChildAttached(USceneComponent* ChildComponent) override;

//public:	
//	// Sets default values for this component's properties
//	UHairworksPinTransformComponent();
//
//	// Called when the game starts
//	virtual void BeginPlay() override;
//	
//	// Called every frame
//	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

		
	
};
