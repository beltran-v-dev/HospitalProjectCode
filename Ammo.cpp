// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"


// Ammo constructor
AAmmo::AAmmo()
{
	//Construct the AmmoMesh component and set it as the root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	// Attach other components to the root
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

	// Initialize custom depth settings
	InitializeCustomDepth();

}

// Set properties based on the state of the item
void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

	if (!AmmoMesh) return;

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

// Enable custom depth rendering for the AmmoMesh
void AAmmo::EnableCustomDepth()
{

	AmmoMesh->SetRenderCustomDepth(true);

}

// Disable custom depth rendering for the AmmoMesh
void AAmmo::DisableCustomDepth()
{

	AmmoMesh->SetRenderCustomDepth(false);

}

// Initialize custom depth settings for the AmmoMesh
void AAmmo::InitializeCustomDepth()
{

	EnableCustomDepth();

}








