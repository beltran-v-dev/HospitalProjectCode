// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PostProcessComponent.h"
#include "SurvivalCharacter.h"







// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the item mesh and set it as the root component
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	// Create the collision box and set it up
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block);


	// Create the pickup widget and set it up
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickipWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	// Create the information widget and set it up
	InformationWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InformationWidget"));
	InformationWidget->SetupAttachment(GetRootComponent());

	// Create the area sphere and set it up
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());

	// Set default values for item properties
	ItemName = "Default";
	IntCount = 0;
	ItemTexture = nullptr;
	BulletsItemTexture = nullptr;
	ItemState = EItemState::EIS_Pickup;
	SlotIndex = -1;

	

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	//Hide pickup widget
	PickupWidget->SetVisibility(false);

	InformationWidget->SetVisibility(false);

	//Setup overlap for AreaSphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOveral);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	//Set item properties based on ItemState
	SetItemProperties(ItemState);


	//Set custom depth to disable
	InitializeCustomDepth();

	

}



// Handle overlap events when another actor enters the sphere surrounding the item.
void AItem::OnSphereOveral(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	// Check if the overlapped actor is a SurvivalCharacter.
	if (OtherActor)
	{
		ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(OtherActor);

		// If it is a SurvivalCharacter, increment the OverlappedItemCount and unhighlight the inventory slot.
		if (SurvivalCharacter)
		{
			SurvivalCharacter->IncrementOverlappedItemCount(1);
			SurvivalCharacter->UnhighlightInventorySlot();
		}

	}

}

// Handle overlap events when another actor exits the sphere surrounding the item.
void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapped actor is a SurvivalCharacter.
	if (OtherActor)
	{
		ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(OtherActor);


		// If it is a SurvivalCharacter, decrement the OverlappedItemCount.
		if (SurvivalCharacter)
		{
			SurvivalCharacter->IncrementOverlappedItemCount(-1);
		}

	}

}



// SetItemProperties function is responsible for configuring the properties of the item based on its current state.
void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_Visibility,
			ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;

	case EItemState::EIS_EquipInterping:

		PickupWidget->SetVisibility(false);

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_PickedUp:

		PickupWidget->SetVisibility(false);

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_Equipped:
		// Set mesh properties

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}



}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// EnableCustomDepth function enables the rendering of custom depth for the item's mesh.
void AItem::EnableCustomDepth()
{
	ItemMesh->SetRenderCustomDepth(true);

}

// DisableCustomDepth function disables the rendering of custom depth for the item's mesh.
void AItem::DisableCustomDepth()
{
	ItemMesh->SetRenderCustomDepth(false);

}

// InitializeCustomDepth function initializes the custom depth by enabling it.
void AItem::InitializeCustomDepth()
{

	EnableCustomDepth();
}


// SetItemState function sets the current state of the item and updates its properties accordingly.
void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

