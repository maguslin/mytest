// Fill out your copyright notice in the Description page of Project Settings.

#include "IPluginManager.h"
#include "testHairWorksPrivatePCH.h"
#include "UHairworksComponent.h"
#include <Nv/Common/NvCoMemoryReadStream.h>
#include "UHairworksMaterial.h"
#include "EnginePrivate.h"
#include "UHairworksAsset.h"

UHairworksComponent::UHairworksComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// No need to select
	bSelectable = false;

	// Setup shadow
	CastShadow = true;
	bAffectDynamicIndirectLighting = false;
	bAffectDistanceFieldLighting = false;
	bCastInsetShadow = true;
	bCastStaticShadow = false;

	// Setup tick
	bAutoActivate = true;
	bTickInEditor = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

FPrimitiveSceneProxy* UHairworksComponent::CreateSceneProxy()
{
	return nullptr;//new FHairworksSceneProxy(this, HairInstance.Hair->AssetId);
}

void UHairworksComponent::OnAttachmentChanged()
{
	// Parent as skeleton
	ParentSkeleton = Cast<USkinnedMeshComponent>(GetAttachParent());

	// Setup bone mapping
	SetupBoneMapping();

	// Update bone matrices
	UpdateBoneMatrices();

	// Refresh render data
	MarkRenderDynamicDataDirty();
}

FBoxSphereBounds UHairworksComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_CalcHairWorksBounds);

	if (HairInstance.Hair == nullptr || HairInstance.Hair->AssetId == NvHair::ASSET_ID_NULL)
		return FBoxSphereBounds(EForceInit::ForceInit);

	checkSlow(BoneMatrices.Num() == 0 || BoneMatrices.Num() == HairWorks::GetSDK()->getNumBones(HairInstance.Hair->AssetId));

	gfsdk_float3 HairBoundMin, HairBoundMax;
	GetSDK()->getBounds(
		HairInstance.Hair->AssetId,
		BoneMatrices.Num() > 0 ? reinterpret_cast<const gfsdk_float4x4*>(BoneMatrices.GetData()) : nullptr,
		HairBoundMin,
		HairBoundMax
	);

	FBoxSphereBounds Bounds(FBox(reinterpret_cast<FVector&>(HairBoundMin), reinterpret_cast<FVector&>(HairBoundMax)));

	return Bounds.TransformBy(LocalToWorld);
}

void UHairworksComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	// Send data for rendering
	SendHairDynamicData();
}

void UHairworksComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	// Call super
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update pin transforms. Mainly for editor
	if (SceneProxy != nullptr && HairInstance.Hair->HairMaterial->Pins.Num() > 0)
	{
		// Get pin matrices
		//auto& HairSceneProxy = static_cast<FHairworksSceneProxy&>(*SceneProxy);
		//TArray<FMatrix> PinMatrices = HairSceneProxy.GetPinMatrices();

		//// Set pin component transform
		//for (auto* ChildComponent : GetAttachChildren())
		//{
		//	auto* PinComponent = Cast<UHairWorksPinTransformComponent>(ChildComponent);
		//	if (PinComponent == nullptr)
		//		continue;

		//	if (PinComponent->PinIndex < 0 || PinComponent->PinIndex >= PinMatrices.Num())
		//		continue;

		//	FTransform PinTransform(PinMatrices[PinComponent->PinIndex]);
		//	PinComponent->SetWorldLocationAndRotation(PinTransform.GetLocation(), PinTransform.GetRotation());
		//}
	}

	// Update bone matrices.
	UpdateBoneMatrices();

	// Mark to send dynamic data
	MarkRenderDynamicDataDirty();
}

FActorComponentInstanceData * UHairworksComponent::GetComponentInstanceData() const
{
	/**
	*  Component instance cached data class for HairWorks components.
	*  Copies HairInstance and HairMaterial. Because HairInstance contains instanced reference, HairMaterial, so it's not automatically copied. And HairMaterial is a instance reference, so it's not automatically copied either.
	*/
	class FHairWorksComponentInstanceData : public FPrimitiveComponentInstanceData
	{
	public:
		FHairWorksComponentInstanceData(const UHairworksComponent& SourceComponent)
			:FPrimitiveComponentInstanceData(&SourceComponent)
		{
			if (!SourceComponent.IsEditableWhenInherited())
				return;

			class FHairInstancePropertyWriter : public FObjectWriter
			{
			public:
				FHairInstancePropertyWriter(const UHairworksComponent& HairComp, TArray<uint8>& InBytes)
					: FObjectWriter(InBytes)
				{
					auto* Archetype = CastChecked<UHairworksComponent>(HairComp.GetArchetype());

					FHairworksInstance::StaticStruct()->SerializeTaggedProperties(
						*this,
						(uint8*)&HairComp.HairInstance,
						FHairworksInstance::StaticStruct(),
						(uint8*)&Archetype->HairInstance
					);

					UHairworksMaterial::StaticClass()->SerializeTaggedProperties(
						*this,
						(uint8*)HairComp.HairInstance.HairMaterial,
						UHairworksMaterial::StaticClass(),
						(uint8*)Archetype->HairInstance.HairMaterial
					);
				}

				virtual bool ShouldSkipProperty(const UProperty* InProperty) const override
				{
					return (InProperty->HasAnyPropertyFlags(CPF_Transient | CPF_ContainsInstancedReference | CPF_InstancedReference)
						|| !InProperty->HasAnyPropertyFlags(CPF_Edit | CPF_Interp));
				}

			private:
			} HairInstancePropertyWriter(SourceComponent, SavedProperties);
		}

		virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override
		{
			FPrimitiveComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);

			if (CacheApplyPhase != ECacheApplyPhase::PostUserConstructionScript || SavedProperties.Num() == 0)
				return;

			class FHairInstancePropertyReader : public FObjectReader
			{
			public:
				FHairInstancePropertyReader(UHairworksComponent& InComponent, TArray<uint8>& InBytes)
					: FObjectReader(InBytes)
				{
					FHairworksInstance::StaticStruct()->SerializeTaggedProperties(
						*this,
						(uint8*)&InComponent.HairInstance,
						FHairworksInstance::StaticStruct(),
						nullptr
					);

					UHairworksMaterial::StaticClass()->SerializeTaggedProperties(
						*this,
						(uint8*)InComponent.HairInstance.HairMaterial,
						UHairworksMaterial::StaticClass(),
						nullptr
					);
				}
			} HairInstancePropertyReader(*CastChecked<UHairworksComponent>(Component), SavedProperties);
		}

	protected:
		TArray<uint8> SavedProperties;
	};

	return new FHairWorksComponentInstanceData(*this);
}

bool UHairworksComponent::ShouldCreateRenderState() const
{
	return GetSDK() != nullptr && HairInstance.Hair != nullptr && HairInstance.Hair->AssetId != NvHair::ASSET_ID_NULL;
}

void UHairworksComponent::CreateRenderState_Concurrent()
{
	// Call super
	Super::CreateRenderState_Concurrent();

	// Setup bone mapping
	SetupBoneMapping();

	// Update bone matrices
	UpdateBoneMatrices();

	// Update proxy
	SendHairDynamicData(true);	// Ensure correct visual effect at first frame.
}

void UHairworksComponent::SendHairDynamicData(bool bForceSkinning)const
{
	// Setup material
//	if (SceneProxy == nullptr)
//		return;
//
//	TSharedRef<FHairWorksSceneProxy::FDynamicRenderData> DynamicData(new FHairWorksSceneProxy::FDynamicRenderData);
//
//	DynamicData->Textures.SetNumZeroed(NvHair::ETextureType::COUNT_OF);
//	GetSDK()->getInstanceDescriptorFromAsset(HairInstance.Hair->AssetId, DynamicData->HairInstanceDesc);
//
//	FName HairNormalCenter;
//
//	// Always load from asset to propagate visualization flags
//	if (HairInstance.Hair->HairMaterial != nullptr)
//	{
//		HairInstance.Hair->HairMaterial->GetHairInstanceParameters(DynamicData->HairInstanceDesc, DynamicData->Textures);
//		HairNormalCenter = HairInstance.Hair->HairMaterial->HairNormalCenter;
//	}
//
//	// Load from component hair material
//	checkSlow(HairInstance.HairMaterial->GetOuter() == this);
//	if (HairInstance.HairMaterial != nullptr && HairInstance.bOverride)
//	{
//		NvHair::InstanceDescriptor OverideHairDesc;
//		HairInstance.HairMaterial->GetHairInstanceParameters(OverideHairDesc, DynamicData->Textures);
//		HairNormalCenter = HairInstance.HairMaterial->HairNormalCenter;
//
//		// Propagate visualization flags
//#define HairWorksMergeVisFlag(FlagName) OverideHairDesc.m_visualize##FlagName |= DynamicData->HairInstanceDesc.m_visualize##FlagName
//
//		HairWorksMergeVisFlag(Bones);
//		HairWorksMergeVisFlag(BoundingBox);
//		HairWorksMergeVisFlag(Capsules);
//		HairWorksMergeVisFlag(ControlVertices);
//		HairWorksMergeVisFlag(GrowthMesh);
//		HairWorksMergeVisFlag(GuideHairs);
//		HairWorksMergeVisFlag(HairInteractions);
//		HairWorksMergeVisFlag(PinConstraints);
//		HairWorksMergeVisFlag(ShadingNormals);
//		HairWorksMergeVisFlag(ShadingNormalBone);
//		HairWorksMergeVisFlag(SkinnedGuideHairs);
//#undef HairWorksMergeVisFlag
//
//		OverideHairDesc.m_drawRenderHairs &= DynamicData->HairInstanceDesc.m_drawRenderHairs;
//
//		if (OverideHairDesc.m_colorizeMode == NvHair::ColorizeMode::NONE)
//			OverideHairDesc.m_colorizeMode = DynamicData->HairInstanceDesc.m_colorizeMode;
//
//		DynamicData->HairInstanceDesc = OverideHairDesc;
//	}
//
//	// Disable simulation
//	if (bForceSkinning)
//		DynamicData->HairInstanceDesc.m_simulate = false;
//
//	auto* BoneIdx = HairInstance.Hair->BoneNameToIdx.Find(HairNormalCenter);
//	if (BoneIdx != nullptr)
//		DynamicData->HairInstanceDesc.m_hairNormalBoneIndex = *BoneIdx;
//	else
//		DynamicData->HairInstanceDesc.m_hairNormalWeight = 0;
//
//	// Set skinning data
//	DynamicData->BoneMatrices = BoneMatrices;
//
//	// Setup pins
//	if (HairInstance.Hair->PinsUpdateFrameNumber != GFrameNumber && HairInstance.Hair->HairMaterial->Pins.Num() > 0)
//	{
//		HairInstance.Hair->PinsUpdateFrameNumber = GFrameNumber;
//
//		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
//			HairUpdatePins,
//			NvHair::AssetId, AssetId, HairInstance.Hair->AssetId,
//			const TArray<FHairWorksPin>, EnginePins, HairInstance.Hair->HairMaterial->Pins,
//			{
//				TArray<NvHair::Pin> Pins;
//		Pins.AddDefaulted(EnginePins.Num());
//		GetSDK()->getPins(AssetId, 0, Pins.Num(), Pins.GetData());
//
//		for (auto PinIndex = 0; PinIndex < Pins.Num(); ++PinIndex)
//		{
//			auto& Pin = Pins[PinIndex];
//
//			const auto& SrcPin = EnginePins[PinIndex];
//
//			Pin.m_useDynamicPin = SrcPin.bDynamicPin;
//			Pin.m_doLra = SrcPin.bTetherPin;
//			Pin.m_pinStiffness = SrcPin.Stiffness;
//			Pin.m_influenceFallOff = SrcPin.InfluenceFallOff;
//			Pin.m_influenceFallOffCurve = reinterpret_cast<const gfsdk_float4&>(SrcPin.InfluenceFallOffCurve);
//		}
//
//		GetSDK()->setPins(AssetId, 0, Pins.Num(), Pins.GetData());
//			}
//		);
//	}
//
//	// Add pin meshes
//	DynamicData->PinMeshes.AddDefaulted(HairInstance.Hair->HairMaterial->Pins.Num());
//
//	for (auto* ChildComponent : GetAttachChildren())
//	{
//		 Find pin transform component
//		auto* PinComponent = Cast<UHairWorksPinTransformComponent>(ChildComponent);
//		if (PinComponent == nullptr)
//			continue;
//
//		if (PinComponent->PinIndex < 0 || PinComponent->PinIndex >= DynamicData->PinMeshes.Num())
//			continue;
//
//		// Collect pin meshes
//		auto& PinMeshes = DynamicData->PinMeshes[PinComponent->PinIndex];
//
//		TFunction<bool(const USceneComponent*)> AddPinMesh;
//		AddPinMesh = [&](const USceneComponent* Component)
//		{
//			// Add mesh
//			if (Component->IsPendingKill())
//				return false;
//
//			auto* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
//
//			if (PrimitiveComponent != nullptr && PrimitiveComponent->SceneProxy != nullptr && !PrimitiveComponent->IsRenderStateDirty())
//			{
//				FHairWorksSceneProxy::FPinMesh PinMesh;
//				PinMesh.Mesh = PrimitiveComponent->SceneProxy;
//				PinMesh.LocalTransform = PrimitiveComponent->GetRelativeTransform().ToMatrixWithScale();
//
//				PinMeshes.Add(PinMesh);
//			}
//
//			// Find in children
//			Component->GetAttachChildren().FindByPredicate(AddPinMesh);
//
//			return false;
//		};
//
//		AddPinMesh(PinComponent);
//	}

	// Send to proxy
	//ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
	//	HairUpdateDynamicData,
	//	FHairWorksSceneProxy&, ThisProxy, static_cast<FHairworksSceneProxy&>(*SceneProxy),
	//	TSharedRef<FHairWorksSceneProxy::FDynamicRenderData>, DynamicData, DynamicData,
	//	{
	//		ThisProxy.UpdateDynamicData_RenderThread(*DynamicData);
	//	}
	//);

	//// Force to simulate for new created instance
	//static const auto& CVarHairFrameRateIndepedentRendering = *IConsoleManager::Get().FindConsoleVariable(TEXT("r.HairWorks.FrameRateIndependentRendering"));
	//if (CVarHairFrameRateIndepedentRendering.GetInt() != 0 && bForceSkinning)
	//{
	//	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
	//		HairForceSimulation,
	//		FHairWorksSceneProxy&, ThisProxy, static_cast<FHairworksSceneProxy&>(*SceneProxy),
	//		{
	//			if (ThisProxy.GetHairInstanceId() != NvHair::INSTANCE_ID_NULL)
	//			HairWorks::GetSDK()->stepInstanceSimulation(ThisProxy.GetHairInstanceId(), 0);
	//		}
	//	);
	//}
}

void UHairworksComponent::SetupBoneMapping()
{
	if (HairInstance.Hair == nullptr || ParentSkeleton == nullptr || ParentSkeleton->SkeletalMesh == nullptr)
		return;

	auto& Bones = ParentSkeleton->SkeletalMesh->RefSkeleton.GetRefBoneInfo();
	BoneIndices.SetNumUninitialized(HairInstance.Hair->BoneNames.Num());

	for (auto Idx = 0; Idx < BoneIndices.Num(); ++Idx)
	{
		BoneIndices[Idx] = Bones.IndexOfByPredicate(
			[&](const FMeshBoneInfo& BoneInfo) {return BoneInfo.Name == HairInstance.Hair->BoneNames[Idx]; }
		);
	}
}

void UHairworksComponent::UpdateBoneMatrices()
{
	if (ParentSkeleton == nullptr)
	{
		BoneMatrices.Empty();
		return;
	}

	BoneMatrices.Init(FMatrix::Identity, BoneIndices.Num());

	for (auto Idx = 0; Idx < BoneIndices.Num(); ++Idx)
	{
		const uint16& IdxInParent = BoneIndices[Idx];
		if (IdxInParent >= ParentSkeleton->GetSpaceBases().Num())
			continue;

		const auto Matrix = ParentSkeleton->GetSpaceBases()[IdxInParent].ToMatrixWithScale();

		BoneMatrices[Idx] = ParentSkeleton->SkeletalMesh->RefBasesInvMatrix[IdxInParent] * Matrix;
	}
}

void UHairworksComponent::Serialize(FArchive & Ar)
{
	Super::Serialize(Ar);

	// When it's duplicated in Blueprints editor, instanced reference is shared, not duplicated. This should be a bug. So we have to duplicate hair material, which is a instanced reference, by ourselves.
	if (Ar.IsLoading() && HairInstance.HairMaterial->GetOuter() != this)
	{
		HairInstance.HairMaterial = CastChecked<UHairworksMaterial>(StaticDuplicateObject(HairInstance.HairMaterial, this));
	}

	// Fix object flag for old assets
	HairInstance.HairMaterial->SetFlags(GetMaskedFlags(RF_PropagateToSubObjects));
}

void UHairworksComponent::PostInitProperties()
{
	// Inherits parent flags. One purpose is to avoid "Graph is linked to private object(s) in an external package." error in UPackage::SavePackage(). Another purpose is to inherit archetype flag.
	if (!HasAnyFlags(RF_NeedLoad))
		HairInstance.HairMaterial = NewObject<UHairworksMaterial>(this, NAME_None, GetMaskedFlags(RF_PropagateToSubObjects));

	Super::PostInitProperties();
}

void UHairworksComponent::CreateHairMaterialDynamic()
{
	if (HairInstance.Hair->HairMaterial != nullptr)
	{
		UHairworksMaterial* NewHairMaterial = DuplicateObject<UHairworksMaterial>(HairInstance.Hair->HairMaterial, this);
		if (NewHairMaterial)
		{
			HairInstance.HairMaterial = NewHairMaterial;
			HairInstance.bOverride = true;
		}
	}
}

UHairworksMaterial* UHairworksComponent::GetHairMaterial()
{
	return HairInstance.HairMaterial;
}
