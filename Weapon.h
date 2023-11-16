// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "WeaponType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"


//UENUM(BlueprintType)
//enum class EWeaponType : uint8
//{
//	EWT_NoWeapon UMETA(DisplayName = "NoWeapon"),
//	EWT_Rope UMETA(DisplayName = "Rope"),
//	EWT_Gun UMETA(DisplayName = "Gun"),
//	EWT_Riffle UMETA(DisplayName = "Riffle"),
//
//	EWT_MAX UMETA(DisplayName = "DefaultMax")
//};

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage;
	

};




UCLASS()
class HOSPITALPROJECT_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	virtual void EnableCustomDepth() override;

	virtual void DisableCustomDepth() override;

	virtual void InitializeCustomDepth() override;

	virtual void OnConstruction(const FTransform& Transform) override;


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

	//Sound played when there is no ammo in the magazine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* NoAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Silencer;
	
	//Data table for weapon properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	//Amount of damage caused by a bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage;

	//Amount of damage when a bullet hits the head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadShotDamage;

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

	 void SetWeaponType(EWeaponType newWeaponType)
	{
		WeaponType = newWeaponType;
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


	 FORCEINLINE USoundCue* GetNoAmmoSound()
	 {
		 return NoAmmo;
	 }

	 FORCEINLINE UStaticMeshComponent* GetSilencerMesh()
	 {
		 return Silencer;
	 }

	 void WeaponDisableCustomDepthWrapper() 
	 { 
		 DisableCustomDepth(); 
	 
	 }

	 FORCEINLINE float GetDamage()
	 {
		 return Damage;
	 }

	 FORCEINLINE float GetHeadShotDamage()
	 {
		 return HeadShotDamage;
	 }




};
