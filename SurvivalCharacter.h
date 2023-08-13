
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivalCharacter.generated.h"

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

	
	//Variable to check if our player has shooted or not to controll the corsshair shooting movement
	bool bFiringBullet;



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



};
