// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "SurvivalCharacter.h"







// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	Silencer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Silencer"));
	Silencer->SetupAttachment(ItemMesh);

	Magazine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Magazine"));
	Magazine->SetupAttachment(ItemMesh, "MagazineSocket");

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility, 
		ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickipWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	InformationWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InformationWidget"));
	InformationWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());

	ItemName = "Default";

	IntCount = 0;

	ItemTexture = nullptr;

	BulletsItemTexture = nullptr;

	ItemState = EItemState::EIS_Pickup;
	
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


}

void AItem::OnSphereOveral(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	//Increment OverlappedItemCount variable +1 every time our character is overlaping
	if (OtherActor)
	{
		ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(OtherActor);

		if (SurvivalCharacter)
		{
			SurvivalCharacter->IncrementOverlappedItemCount(1);
		}
		
	}

}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//Decrement OverlappedItemCount variable -1 every time our character is Endoverlaping

	if (OtherActor)
	{
		ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(OtherActor);

		if (SurvivalCharacter)
		{
			SurvivalCharacter->IncrementOverlappedItemCount(-1);
		}

	}

}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:

		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set area sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		//Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case EItemState::EIS_EquipInterping:

		PickupWidget->SetVisibility(false);
		//SetVisiblue to true the new widget
		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set collisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:

	//	PickupWidget->SetVisibility(false);

		//Set mesh properties
		ItemMesh->SetSimulatePhysics(false);	
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set are sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaSphere->SetActive(false);


		//Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetActive(false);

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

void AItem::SetItemState(EItemState State)
{

	ItemState = State;
	SetItemProperties(State);



}

