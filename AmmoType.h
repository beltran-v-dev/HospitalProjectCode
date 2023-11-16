#pragma once
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_NoAmmo UMETA(DisplayName = "NoAmmo"),
	EAT_AmountOfRopes UMETA(DisplayName = "AmountOfRopes"),
	EAT_GunAmmo UMETA(DisplayName = "GunAmmo"),
	EAT_RiffleAmmo UMETA(DisplayName = "RiffleAmmo"),

	EAT_MAX UMETA(DisplayName = "DefaultMax")

};