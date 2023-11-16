// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"



AAmmo::AAmmo()
{
	//Construct the AmmoMesh component and set it as the root
	AmmoMesh = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());



	

}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	InitializeCustomDepth();

}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

	switch (State)
	{
	case EItemState::EIS_Pickup:

		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_EquipInterping:

	
		GetPickupWidget()->SetVisibility(false);
		// Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);



		break;

	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
		// Set mesh properties
		GetPickupWidget()->SetVisibility(false);
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		
		break;

	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}

}

void AAmmo::EnableCustomDepth()
{

	AmmoMesh->SetRenderCustomDepth(true);

}

void AAmmo::DisableCustomDepth()
{

	AmmoMesh->SetRenderCustomDepth(false);

}

void AAmmo::InitializeCustomDepth()
{

	EnableCustomDepth();

}








