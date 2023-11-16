
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "SurvivalCharacter.generated.h"



UENUM(BlueprintType)
enum class EComabtState : uint8
{
	ECS_NoWeapon UMETA(DisplayName = "NoWeapon"),
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),


	ECS_MAX UMETA(DisplayName = "ECS_Max")

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, SlotIndex, int32, NewSlowIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);




UCLASS()
class HOSPITALPROJECT_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASurvivalCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Called when FireButton is pressed
	void FireWeapon();

	void EnableFire();

	//Line trace for item under the crosshairs
	bool TraceUnderCorsshairs(FHitResult& OutHitResult);

	//Trave for items if OverlappedItemCound > 0
	void TraceForItems();

	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();

	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	/**
	*Rotate controller based on mouse X movement
	@param Value: The input value from mouse movement
	*/
	void Turn(float Value);

	/**
	*Rotate controller based on mouse Y movement
	@param Value: The input value from mouse movement
	*/
	void LookUp(float Value);

	/**
	* Crosshair spread will be calcualted every frame and to do that we are going to use interpolation
	* becuase of that we are going to use a variable float DeltaTime to that propuse
	*/
	void CalculateCorsshairSpread(float DeltaTime);


	//Functions to set ture/flase the bFiringBullet variable in orter to spread the corsshair when we the player shoots 
	void FinishCrosshairBulletFire();
	void StartCorsshairBulletFire();

	//Spawns a default weapon and equips it
	class AWeapon* SpawnDefaultWeapon();

	//Takes a weapon and attaches it to the mesh
	void EquipWeapon(class AWeapon* WeponToEquip);

	//Detach weapon and let it fall to the ground
	//void DropWeapon();

	void SelectButtonPressed();
	void SelectButtonReleased();

	void NoWeapon();


	void IsJogging();
	void IsNotJoggin();


	void PlayerWeaponAtStart();

	//Equips TraceHitItem.
	void EquipNewWeapon(AWeapon* NewWeaponToEquip);


	//Functions to take care of jogging as the player is aiming or not 

	void SetIsNotJogging();

	void SetIsJogging();

	//Initialize the AmmoMap with ammo values
	void InitializeAmmoMap();

	//Check to make sure out wapon has ammo
	bool WeaponHasAmmo();

	//FireWeapon functions

	void PlayFireSound();
	void MuzzleFlashPlaced();
	void PlayGunFireMontage();


	//Bound to the R key
	void ReloadButtonPressed();

	//Handle reloading of the weapon
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
		void FinishReloading();

	//Checks to see if we have ammo of the EquippedWeapon's ammo type
	bool CarryingAmmo();

	void CrouchButtonPressed();

	//Interps capsule height when crouching/standing 
	void InterpCapsuleHeight(float DeltaTime);


	void PickedAmmo(class AAmmo* Ammo);

	bool bFistTimePickingAWeeapon;

	int32 GetEmptyInventorySlot();

	void HighlightInventorySlot();

	




public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Called for forwards/backwards input
	void MoveForwardBackward(float value);

	//Called for right/left input
	void MoveRightLeft(float value);

	//The variable bIsAiming will be set to true/false every time this function is called 
	void IsAiming();
	//The variable bIsAiming will be set to true/false every time this function is called 
	void IsNotAiming();

	//Set camera FOV smoothly
	void SetCameraFOV(float DeltaTime);









	//Variables
private:

	//Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
		class USpringArmComponent* CameraBoom;

	//Camera that follows the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
		class UCameraComponent* Camera;

	//Randomized gunshot sound cue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		class USoundCue* FireSound;

	//Flash spawned at barrelSocket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		class UParticleSystem* MuzzleFlash;

	class USkeletalMeshComponent* MainSkeletalMesh;

	bool bIsShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		float timeBetweenShootsGun;

	//Montage for firing weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		class UAnimMontage* HipFireMontage;

	//Particles spawned upon bullent impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		UParticleSystem* impactParticles;

	//Bool to set if our player is aiming or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
		bool bIsAiming;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
		bool bHasBeenAimingWalk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
		bool bHasBeenAiming;

	//Distance which our player is going to be able to hit objects as they are shooting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		float ShootingDistance;

	//Defailt cmaera field of view value
	float CameraDefaultFOV;

	//Field of view value when we zoom in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		float CameraZoomedFOV;

	//Current field of view this frame
	float CameraCurrentFOV;

	//Interps speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
		float ZoomInterpSpeed;


	//Aiming speed when we are aiming or not

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMAX = "1.0"))
		float AimingSpeedWhenAimingUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMAX = "1.0"))
		float AimingSpeedWhenAimingSides;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMAX = "1.0"))
		float AimginSpeedWhenIsNotAimingUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMIN = "0.0", UIMAX = "1.0"))
		float AimginSpeedWhenIsNotAimingSides;

	//Bool which says if the gun has been equipped
	bool bGunHasBeenEquipped;

	//Bool to determinte if the key to fog has been pressed
	bool bIsPressingTheJogginKey;

	//crosshair spread

	//variable to determinate the spread of the corsshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = true))
		float CrosshairSpreadMuliplier;

	//Variable to change the velocity spread in the CrosshairSpreadMuliplier variable when we hare moving around
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = true))
		float CorsshairVelocityFactor;

	//Variable to change the velocity spread in the CrosshairSpreadMuliplier variable when we are aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = true))
		float CrosshairAimFactor;

	//Variable to change the velocity spread in the CrosshairSpreadMuliplier variable when we are shooting
	float CrosshairShootingFactor;

	//Endcrosshair spread

	//Variable to check if our player has shooted or not to controll the corsshair shooting movement
	bool bFiringBullet;

	//True if we should trace every frame for item
	bool bShouldTraceForItems;

	//Number of overlapped AItems
	int8 OverlappedItemCount;

	//The AItem we hit last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
		class AItem* TraceHitItemLastFrame;

	//Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class AWeapon* EqquipedWeapon;

	//Set this in Blueprints for the default weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> DefaultWeaponClass;

	//Montage to equip the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* EquipGunMontage;

	//Montage to equip the gun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* PullsOutGunMontage;

	//Bool to check if our player has an weapon equipped 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
		bool bHasAWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bisAGun;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bisARiffle;

	//Bool to check if the player is jogging 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
		bool bIsJogging;

	//Float to indicate the character velocity when it's walking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float WalkingSpeed;

	//Float to indicate the character velocity when it's Crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float CrouchingSpeed;

	//Float to indicate the character velocity when it's Jogging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float JoggingSpeed;

	//The Item currently hit by our trauce in TraceForIntems() (it could be null)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		AItem* TraceHitItem;

	//Map to keep track of ammo of different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
		TMap<EAmmoType, int32> AmmoMap;

	//Starting amount gun ammo
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
		int32 StartingGunAmmo;

	//Starting amount riffle ammo
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
		int32 StartingRiffleAmmo;

	//Combat State, can only fire or reload if Unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		EComabtState CombatState;

	//Montage for reaload Animations 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		UAnimMontage* ReloadMontage;

	//Ture when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouching;

	//CurrentHeight of the capsule
	float CurrentCapsuleHeight;

	// Height of the capsule when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float StandingCapsuleHeight;

	//Height of the capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float CrouchingCapsuleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float InterpSpeedCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSocket, meta = (AllowPrivateAccess = "true"))
		FVector CameraBoomSoketOffsetCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraSocket, meta = (AllowPrivateAccess = "true"))
		FVector CameraBoomSoketOffsetNoCrouching;

	//An array of AWeapons for our inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
		TArray<AWeapon*> Inventory;

	const int32 INVENTORY_CAPACITY = 3;

	bool bHasPcikedUpAmmo;

	bool bHasPickedWeapon;

	//Delegate for sending slot information to InventoryBar when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;
	
	//Delegate for sending slot information for playing the icon animation
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	//The index for tue current highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;


public:



	//Returns CameraBoom subObject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	//Returns CameraComponent subObject
	FORCEINLINE UCameraComponent* GetCamera() const
	{
		return Camera;
	}

	FORCEINLINE bool GetAimingBool()  const
	{
		return bIsAiming;
	}

	UFUNCTION(BlueprintCallable)
		float GetCrosshairSpreadMultiplier();

	FORCEINLINE int8 GetOverlapappedItemCound()
	{
		return OverlappedItemCount;
	}

	//Add/subtract to/from overlappedItemCount and updates bShouldTraceFromItems
	void IncrementOverlappedItemCount(int8 Amount);



	FORCEINLINE bool GetbHasAWeapon()
	{
		return bHasAWeapon;
	}

	FORCEINLINE bool GetbIsJoging()
	{
		return bIsJogging;
	}

	FORCEINLINE bool GetCrouching()
	{
		return bIsCrouching;
	}

	AWeapon* GetEqquipedWeapon() const
	{
		return EqquipedWeapon;
	}

	

	void UnhighlightInventorySlot();

};
