// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "Item.generated.h"



UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")



};





UCLASS()
class HOSPITALPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



//Called when overlapping AreaSphere
UFUNCTION()
	void OnSphereOveral(
		UPrimitiveComponent* OverlapComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep, 
		const FHitResult& SweepResult);


//Called when EndOverlaping AreaSphere
UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);


//Set properties of the Item's componentes based on State
	virtual void SetItemProperties(EItemState State);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:


	//Skeletalmesh for the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	
	
	//Line trace collides with box to show HUD widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//Popup widget when the player looks at the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	//Popup widget when the player has pressed the key to get it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* InformationWidget;

	//Enable item traicing when overlap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	//The name which appears on the Pickup Widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	//Item count  (ammo, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 IntCount;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* ItemTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* BulletsItemTexture;

	//Background for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

	//IconItem for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;


	//AmmoIcon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;


	//State of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	//Sound played when item is picked up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickUpSound;

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	virtual void InitializeCustomDepth();
	
	//Slot in the inventory Array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	


public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() 
	{
		return PickupWidget;
	}

	FORCEINLINE UWidgetComponent* GetInformationpWidget()
	{
		return InformationWidget;
	}

	FORCEINLINE USphereComponent* GetAreaSphere()
	{
		return AreaSphere;
	}

	FORCEINLINE UBoxComponent* GetCollisionBox()
	{
		return CollisionBox;
	}

	FORCEINLINE EItemState GetItemState()
	{
		return ItemState;
	}

	void SetItemState(EItemState State);

	FORCEINLINE USkeletalMeshComponent* GetItemMesh()
	{
		return ItemMesh;
	}

	FORCEINLINE USoundCue* GetPickedUpSound()
	{
		return PickUpSound;
	}

	FORCEINLINE void SetPickedUpSound(USoundCue* newPickedUpSound)
	{
		PickUpSound = PickUpSound;
	}


	FORCEINLINE int32 GetItemCount()
	{
		return IntCount;
	}

	void DisableCustomDepthWrapper()
	{
		DisableCustomDepth();

	}

	void EnableCustomDepthWrapper()
	{
		EnableCustomDepth();

	}

	FORCEINLINE int32 GetSlotIndex()
	{
		return SlotIndex;
	}

	FORCEINLINE void SetSlotIndex(int index)
	{
		SlotIndex = index;
	}
	
	FORCEINLINE void SetInventoryItem(UTexture2D* newInventoryItem)
	{
		IconItem = newInventoryItem;
	}

	FORCEINLINE void SetAmmoIcon(UTexture2D* newAmmoIcon)
	{
		AmmoIcon = newAmmoIcon;
	}



};
