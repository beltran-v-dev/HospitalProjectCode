
// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h" 
#include "DrawDebugHelpers.h"






// Sets default values
ASurvivalCharacter::ASurvivalCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom (pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	//The camera follow at this distance behind the character 
	CameraBoom->TargetArmLength = 180.0f;
	/*Whenever our controller moves, our camera boom is going to use that rotation.
	* We are the  our control rotation for our camera boom
	*/
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f);

	//Create the followCamera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//Attach camera ot end of boom
	Camera->SetupAttachment(CameraBoom, CameraBoom->SocketName);
	//Camera does not rotate to arm
	Camera->bUsePawnControlRotation = false;

	//Don't rotate when the controller rotates. Let the controller only affect the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//Instead of rotating the character through Controller, we are going to
	//control character rotation through directional movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	//We just want to rotate with YAW at the following velocity:
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	MainSkeletalMesh = GetMesh();


	bIsShoot = true;

	timeBetweenShootsGun = 0.5f;

	bIsAiming = false;
	bHasBeenAiming = false;
	bHasBeenAimingWalk = false;

	ShootingDistance = 50'000.0f;

	//Camera field of view values 
	CameraDefaultFOV = 0.0f; //Set in BeginPlay
	CameraZoomedFOV = 35.0f;
	CameraCurrentFOV = 0.0f; //Set in BeginPlay
	ZoomInterpSpeed = 20.0f;

	//Aiming Speed
	AimginSpeedWhenIsNotAimingUp = 1.0f;
	AimginSpeedWhenIsNotAimingSides = 1.0f;
	AimingSpeedWhenAimingUp = 0.2f;
	AimingSpeedWhenAimingSides = 0.2f;

	//Crosshair spread factors
	CrosshairSpreadMuliplier = 0.0f;
	CorsshairVelocityFactor = 0.0f;
	CrosshairAimFactor = 0.0f;
	CrosshairShootingFactor = 0.0f;
	bFiringBullet = false;


}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Camera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

}

void ASurvivalCharacter::FireWeapon()
{

	//UE_LOG(LogTemp, Warning, TEXT("Fire Button"));

	if (bIsShoot && bIsAiming)
	{
		bIsShoot = false;
		


		if (MainSkeletalMesh)
		{

			FName ChildComponentName = FName("SM_Gun");


			for (USceneComponent* childComponent : MainSkeletalMesh->GetAttachChildren())
			{
				USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(childComponent);

				if (ChildSkeletalMeshComponent && childComponent->GetName() == ChildComponentName.ToString())
				{


					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("SM, %s"), *ChildSkeletalMeshComponent->GetName()));

					const USkeletalMeshSocket* BarrelSocket = ChildSkeletalMeshComponent->GetSocketByName("barrelSocket");

					const FTransform socketTransform = BarrelSocket->GetSocketTransform(ChildSkeletalMeshComponent);

					if (bIsShoot == false)
					{

						if (timeBetweenShootsGun <= 0)
						{
							timeBetweenShootsGun = 0.1f;
						}

						FTimerHandle TimerHandle_FireCoolDown;
						GetWorldTimerManager().SetTimer(TimerHandle_FireCoolDown, 
							this, 
							&ASurvivalCharacter::EnableFire, 
							timeBetweenShootsGun, 
							false);

					}

					if (MuzzleFlash)
					{

						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, socketTransform);

				

						if (FireSound)
						{
							UGameplayStatics::PlaySound2D(this, FireSound);


						}



					}

					UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
					if (AnimInstance && HipFireMontage)
					{
						AnimInstance->Montage_Play(HipFireMontage);
						AnimInstance->Montage_JumpToSection(FName("StartFire"));
					}

					//Start bullet fire for crosshairs
					FinishCrosshairBulletFire();

					//Get current size of the viewport
					FVector2D ViewportSize;

					if (GEngine && GEngine->GameViewport)
					{
						GEngine->GameViewport->GetViewportSize(ViewportSize);
					}

					//Get screen location of corsshair
					FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
					FVector CrosshairWorldPosition;
					FVector CrosshairWorldDirection;

					//Get worldposition and direction of crosshair
					bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
						UGameplayStatics::GetPlayerController(this, 0),
						CrosshairLocation,
						CrosshairWorldPosition,
						CrosshairWorldDirection);

					if (bScreenToWorld) //was the DeprojectScreenToWorld successful?
					{
						//Variables to hit our bullets: Hit, where the bullet starts and when the bullet ends
						FHitResult ScreenTraceHit;
						const FVector Start{ CrosshairWorldPosition };
						FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * ShootingDistance };

						//Tracing a a ray to the world using the previous paramters/variables
						GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

						

						//If our bullets hit an mesh then spawn the particles
						if (ScreenTraceHit.bBlockingHit)
						{
							if (impactParticles)
							{
								UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticles, ScreenTraceHit.Location);
							}
						}


					}






				}

			}


		}

	}

}

void ASurvivalCharacter::EnableFire()
{

	bIsShoot = true;

}


void ASurvivalCharacter::Turn(float Value)
{
	float TurnScaleFactor;

	if (bIsAiming)
	{
		TurnScaleFactor = AimingSpeedWhenAimingSides;
	}
	else
	{
		TurnScaleFactor = AimginSpeedWhenIsNotAimingSides;
	}

	AddControllerYawInput(Value * TurnScaleFactor);
}

void ASurvivalCharacter::LookUp(float Value)
{


	float LookUpScaleFactor;

	if (bIsAiming)
	{
		LookUpScaleFactor = AimingSpeedWhenAimingUp;
	}
	else
	{
		LookUpScaleFactor = AimginSpeedWhenIsNotAimingUp;
	}

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void ASurvivalCharacter::CalculateCorsshairSpread(float DeltaTime)
{
	//Velociy character range
	FVector2D WalkSpeedRange = { 0.0f, ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed };

	//range between 0 and 1
	FVector2D VelocityMultiplierRange = { 0.0f, 1.0f };

	//Current character velocity
	FVector currentVelocity = GetVelocity();
	currentVelocity.Z = 0.0f;

	//Calculate corsshair velocity factor (resulted clamped to have a better management)
	CorsshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		currentVelocity.Size());

	//Calculate corsshair aim factor
	if (bIsAiming) //Are we aiming?
	{
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.0f);
		
	}
	else //Not Aiming
	{
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.0f,
			DeltaTime,
			30.0f);
	}

	//Calculate crosshair shooting factor
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 1.5f, DeltaTime, 10.0f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 10.0f);

	}

	//Final calculations

	CrosshairSpreadMuliplier = 0.5f + CorsshairVelocityFactor  - CrosshairAimFactor + CrosshairShootingFactor;


}

void ASurvivalCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = true;
	FTimerHandle CrosshairShootTimer;

	//Time to set false the bFiringBullet variable 
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ASurvivalCharacter::StartCorsshairBulletFire, 0.005f);

}

void ASurvivalCharacter::StartCorsshairBulletFire()
{
	bFiringBullet = false;
}







// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Set camera FOV smoothly (interpotalion for zoom when aiming)
	ASurvivalCharacter::SetCameraFOV(DeltaTime);

	//Calcualte corsshair spread multiplier
	CalculateCorsshairSpread(DeltaTime);

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("IsHolding?, %s"), test ? TEXT("true") : TEXT("false")));

		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("IsAiming?, %s"), bHasBeenAimingWalk ? TEXT("true") : TEXT("false")));
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("velocity?, %f"), this->GetVelocity().Size()));



	}






}

// Called to bind functionality to input
void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Cheking if PlayerInputComponent is valid 
	check(PlayerInputComponent);

	//Calling the movement function from the PlayerInputCompoenent to apply the movement
	PlayerInputComponent->BindAxis("MoveForwardBackward", this, &ASurvivalCharacter::MoveForwardBackward);
	PlayerInputComponent->BindAxis("MoveRightLeft", this, &ASurvivalCharacter::MoveRightLeft);

	//Calling the mouseMovementController function from APawn to apply rotation to the 
	//CameraBoom
	PlayerInputComponent->BindAxis("Turn", this, &ASurvivalCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASurvivalCharacter::LookUp);


	//Calling the FireWeapon function 
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ASurvivalCharacter::FireWeapon);

	//Calling the IsAiming function to allow the shooting system
	PlayerInputComponent->BindAction("IsAiming", IE_Pressed, this, &ASurvivalCharacter::IsAiming);
	PlayerInputComponent->BindAction("IsAiming", IE_Released, this, &ASurvivalCharacter::IsNotAiming);



}


void ASurvivalCharacter::MoveForwardBackward(float value)
{
	if (Controller && value != 0)
	{
		//Find out which way is forward
		const FRotator Rotator = Controller->GetControlRotation();
		const FRotator YawRotator{ 0.0f, Rotator.Yaw, 0.0f };
		//This function will get the x axis direction from the rotation mateix.
		//And rotationMateix can be constructed using a rotator(YawRotator)
		const FVector Direction{ FRotationMatrix{YawRotator}.GetUnitAxis(EAxis::X) };

		//Applying the movement 
		AddMovementInput(Direction, value);





	}

}

void ASurvivalCharacter::MoveRightLeft(float value)
{

	//Find out which way is Right
	const FRotator Rotator = Controller->GetControlRotation();
	const FRotator YawRotator{ 0.0f, Rotator.Yaw, 0.0f };
	//This function will get the x axis direction from the rotation mateix.
	//And rotationMateix can be constructed using a rotator(YawRotator)
	const FVector Direction{ FRotationMatrix{YawRotator}.GetUnitAxis(EAxis::Y) };

	//Applying the movement 
	AddMovementInput(Direction, value);


}

void ASurvivalCharacter::IsAiming()
{
	bIsAiming = true;




}

void ASurvivalCharacter::IsNotAiming()
{
	bIsAiming = false;

}




void ASurvivalCharacter::SetCameraFOV(float DeltaTime)
{


	//Set current camera filed of view
	if (bIsAiming)
	{
		//Interpoalete to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);

	}
	else
	{
		//Interpoalete to zoomed default FOV

		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);

	}

	GetCamera()->SetFieldOfView(CameraCurrentFOV);

}

float ASurvivalCharacter::GetCrosshairSpreadMultiplier()
{
	return 	CrosshairSpreadMuliplier;

}






