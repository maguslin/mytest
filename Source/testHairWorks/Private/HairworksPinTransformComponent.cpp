// Fill out your copyright notice in the Description page of Project Settings.

#include "IPluginManager.h"
#include "testHairWorksPrivatePCH.h"
#include "EnginePrivate.h"
#include "HairworksPinTransformComponent.h"


UHairworksPinTransformComponent::UHairworksPinTransformComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseAttachParentBound = true;
}

void UHairworksPinTransformComponent::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);

	ChildComponent->bUseAttachParentBound = true;
}

