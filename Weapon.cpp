// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	Ammo = 32;
	WeaponType = EWeaponType::EWT_Gun;
	AmmoType = EAmmoType::EAT_GunAmmo;
	ReloadMontageSection = FName(TEXT("Reload_Gun")); 
	MagazineCapacity = 8;

	MaxCapacity = MagazineCapacity;
	
	

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::DecreaseAmmo()
{


	if (!(MagazineCapacity <= 0))
	{
		--MagazineCapacity;
	

	
	}

	
	
	
}

void AWeapon::ReloadAmmo(int32 Ammount)
{
	
	MagazineCapacity += Ammount;

	

}

int32 AWeapon::MaxCapacityMagazine() const
{
	

	return MaxCapacity;
}

bool AWeapon::IsCarryingMaxCapacity()
{
	
	if (MagazineCapacity == MaxCapacity)
	{
		return false;
	}
	else
	{
		return true;
	}
}




