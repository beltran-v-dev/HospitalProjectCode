#pragma once
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_NoWeapon UMETA(DisplayName = "NoWeapon"),
	EWT_Rope UMETA(DisplayName = "Rope"),
	EWT_Gun UMETA(DisplayName = "Gun"),
	EWT_Riffle UMETA(DisplayName = "Riffle"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};