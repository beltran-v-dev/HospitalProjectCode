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
	
	Silencer = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Silencer"));
	Silencer->SetupAttachment(GetItemMesh());
	
	


}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	InitializeCustomDepth();

	

}

void AWeapon::EnableCustomDepth()
{

	Silencer->SetRenderCustomDepth(true);
	AItem::GetItemMesh()->SetRenderCustomDepth(true);
}

void AWeapon::DisableCustomDepth()
{

	Silencer->SetRenderCustomDepth(false);
	AItem::GetItemMesh()->SetRenderCustomDepth(false);

}

void AWeapon::InitializeCustomDepth()
{
	EnableCustomDepth();


}

void AWeapon::OnConstruction(const FTransform& Transform)
{

	const FString WeaponTablePath{ TEXT("DataTable'/Game/_Game/DataTable/DT_Weapon.DT_Weapon'") };
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));


	if (WeaponTableObject)
	{

		FWeaponDataTable* WeaponDataRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_Gun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Gun"), TEXT(""));
			
		break;

		case EWeaponType::EWT_Riffle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Riffle"), TEXT(""));

			break;
		}

		if (WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazineCapacity;
			this->SetPickedUpSound(WeaponDataRow->PickUpSound);
			this->GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			this->SetInventoryItem(WeaponDataRow->InventoryIcon);
			this->SetAmmoIcon(WeaponDataRow->AmmoIcon);
			Damage = WeaponDataRow->Damage;
			HeadShotDamage = WeaponDataRow->HeadShotDamage;

		}

	}





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




