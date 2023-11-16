// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class HOSPITALPROJECT_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	//Override of SetItemProperties, we we can set AmmoMesh properites
	virtual void SetItemProperties(EItemState State) override;

	virtual void EnableCustomDepth() override;

	virtual void DisableCustomDepth() override;

	virtual void InitializeCustomDepth() override;





	

private:
	//Mesh for the ammo pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;
	
	//Ammo type for the amo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh()
	{
		return AmmoMesh;
	}

	FORCEINLINE EAmmoType GetAmmoType()
	{
		return AmmoType;
	}

	
	
};
