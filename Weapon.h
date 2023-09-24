// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_NoWeapon UMETA(DisplayName = "NoWeapon"),
	EWT_Rope UMETA(DisplayName = "Rope"),
	EWT_Gun UMETA(DisplayName = "Gun"),
	EWT_Riffle UMETA(DisplayName = "Riffle"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * 
 */
UCLASS()
class HOSPITALPROJECT_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:

private:

	//Max ammo that our weapon can hold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	//Maximum ammo that every magazine can hold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaxCapacity;

	
	//Type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	//Type of ammo for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//FName for the Reload Montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;


	



public:

	FORCEINLINE int32 GetAmmo()
	{
		return Ammo;
	}

	FORCEINLINE  int32 GetMagazineCapacity()
	{
		return MagazineCapacity;
	}

	//Called from character class when firing weapon
	void DecreaseAmmo();


	FORCEINLINE EWeaponType GetWeaponType()
	{
		return WeaponType;
	}

	FORCEINLINE EAmmoType GetAmmoType()
	{
		return AmmoType;
	}

	FORCEINLINE FName GetReloadMontageSection()
	{
		return ReloadMontageSection;
	}

	void ReloadAmmo(int32 Ammount);

	 int32 MaxCapacityMagazine() const;

	 bool IsCarryingMaxCapacity();

};
