// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PrimitiveViewRelevance.h"
#include "GenericPlatform.h"
/**
 * 
 */
struct  FHairworksPrimitiveViewRelevance :FPrimitiveViewRelevance
{
public:
	FGenericPlatformTypes::uint32 bHairWorks : 1;
	FHairworksPrimitiveViewRelevance() :bHairWorks(false)
	{}
};