
// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h" 
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"






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

	//Item trave variables 
	bShouldTraceForItems = false;


	bHasAWeapon = false;


	WalkingSpeed = 150.0f;
	JoggingSpeed = 300.0f;
	bIsJogging = false;

	bGunHasBeenEquipped = false;

	bIsPressingTheJogginKey = false;

	
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

	//Spawn the default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());


	PlayerWeaponAtStart();


}

void ASurvivalCharacter::FireWeapon()
{

	//UE_LOG(LogTemp, Warning, TEXT("Fire Button"));

	if (bIsShoot && bIsAiming && bHasAWeapon)
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


					//--------------------------------------------------------------------------------------------------








					//----------------------------------------------------------------------------------------------------------





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

					////Get current size of the viewport
					FVector2D ViewportSize;

					if (GEngine && GEngine->GameViewport)
					{
						GEngine->GameViewport->GetViewportSize(ViewportSize);
					}

					////Get screen location of corsshair
					FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
					FVector CrosshairWorldPosition;
					FVector CrosshairWorldDirection;

					////Get worldposition and direction of crosshair
					bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
						UGameplayStatics::GetPlayerController(this, 0),
						CrosshairLocation,
						CrosshairWorldPosition,
						CrosshairWorldDirection);

					//Check for crosshairs trace hit
				

					if (bScreenToWorld) //was the DeprojectScreenToWorld successful?
					{
						////Variables to hit our bullets: Hit, where the bullet starts and when the bullet ends
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

bool ASurvivalCharacter::TraceUnderCorsshairs(FHitResult& OutHitResult)
{
	//Get viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get screen space location of crosshairs
	FVector2D CrosshairsLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairsWorldPosition;
	FVector CorsshairsWorldDirection;

	//Get world position and direction of c0orsshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairsLocation,
		CrosshairsWorldPosition,
		CorsshairsWorldDirection
	);

	if (bScreenToWorld)
	{
		//Trace from corsshair world location outward
		FVector Start{ CrosshairsWorldPosition };
		FVector End{ Start + CorsshairsWorldDirection * ShootingDistance};
		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility);
	}

	if (OutHitResult.bBlockingHit)
	{
		return true;
	}
	else
	{
		return false;
	}
	return false;
}

void ASurvivalCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult CrosshairsHitResult;
		bool bScreenToWorld = TraceUnderCorsshairs(CrosshairsHitResult);
		TraceHitItem = Cast< AItem>(CrosshairsHitResult.GetActor());
		if (TraceHitItem && TraceHitItem->GetPickupWidget())
		{
			//Show item's pickup widget
			TraceHitItem->GetPickupWidget()->SetVisibility(true);
		}


		
	

		//We hit an AItem last frame
		if (TraceHitItemLastFrame)
		{	
			if (TraceHitItem != TraceHitItemLastFrame)
			{
				//We are hitting a different AItem this frame from last frame 
				//Or AItem is null
				TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
			}
		}

		//Store a reference to HitItem for next frame
		TraceHitItemLastFrame = TraceHitItem;

		
	}
	else if(!bShouldTraceForItems && TraceHitItemLastFrame)
	{
		//No longer overlapping any itmes 
		//Item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);

	}


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

AWeapon* ASurvivalCharacter::SpawnDefaultWeapon()
{

//Check the TSubclasOf variable 
	if (DefaultWeaponClass)
	{
		//Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

		//return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);

	
	}
	return nullptr;


}

void ASurvivalCharacter::EquipWeapon(AWeapon* WeponToEquip)
{

	if (WeponToEquip)
	{
		



		//Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("Pistol_Socket"));

		//Check the HandSocket variable 
		if (HandSocket)
		{
			//Attach the Weapon to the hand socket
			HandSocket->AttachActor(WeponToEquip, GetMesh());
		}

		//Set EqquipedWeapon to the newly spawned Weapon
		EqquipedWeapon = WeponToEquip;
		EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);
	}



}

void ASurvivalCharacter::DropWeapon()
{

	if (EqquipedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EqquipedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
	}


}

void ASurvivalCharacter::SelectButtonPressed()
{
	//bHasAWeapon = true;

	if (TraceHitItem)
	{
		auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		SwapWeapon(TraceHitWeapon);
		bHasAWeapon = true;

		bGunHasBeenEquipped = true;

			if (bHasAWeapon)
			{
				if (MainSkeletalMesh)
				{

					TArray<FName> ChildComponentNames = { FName("SM_HolsterWeapon"), FName("SM_HolsterNoWeapon") };


					for (USceneComponent* childComponent : MainSkeletalMesh->GetAttachChildren())
					{
						USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(childComponent);

						if (ChildSkeletalMeshComponent)
						{

							FName ComponentName = *ChildSkeletalMeshComponent->GetName();  // Get the name of the child component

							if (ChildComponentNames.Contains(ComponentName))
							{

								if (ComponentName == FName("SM_HolsterWeapon"))
								{
									ChildSkeletalMeshComponent->SetVisibility(false);


								}
								else if (ComponentName == FName("SM_HolsterNoWeapon"))
								{

									ChildSkeletalMeshComponent->SetVisibility(true);


								}

							}


						}

					}
				}

			}

	}
	
	

}

void ASurvivalCharacter::SelectButtonReleased()
{


}

void ASurvivalCharacter::NoWeapon()
{

	//Getting the anim instance from the character
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();


if (bHasAWeapon == true)
{
	



	//Checking that neither AnimInstance nor EquipGunMontage are not null
	if (AnimInstance && EquipGunMontage)
	{
		
		AnimInstance->Montage_Play(EquipGunMontage);
		AnimInstance->Montage_JumpToSection(FName("EquipGun"));

		// Delay the visibility change until the montage is done
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
				if (EqquipedWeapon)
				{
					EqquipedWeapon->GetItemMesh()->SetVisibility(false);


					if (MainSkeletalMesh)
					{

						TArray<FName> ChildComponentNames = { FName("SM_HolsterWeapon"), FName("SM_HolsterNoWeapon") };


							for (USceneComponent* childComponent : MainSkeletalMesh->GetAttachChildren())
							{
								USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(childComponent);

								if (ChildSkeletalMeshComponent)
								{

									FName ComponentName = *ChildSkeletalMeshComponent->GetName();  // Get the name of the child component

								if (ChildComponentNames.Contains(ComponentName))
								{

									if (ComponentName == FName("SM_HolsterWeapon"))
									{
										ChildSkeletalMeshComponent->SetVisibility(true); 
										

									}
									else if (ComponentName == FName("SM_HolsterNoWeapon"))
									{
									
										ChildSkeletalMeshComponent->SetVisibility(false);


									}

								}


							}
							
						}
					}
					bHasAWeapon = false;
					bIsShoot = false;

				}
			}, 0.6f, false);

												
						

	}
	







	

	}

	if (bHasAWeapon == false && TraceHitItem && bGunHasBeenEquipped == true)
	{


	//Checking that neither AnimInstance nor EquipGunMontage are not null
	if (AnimInstance && PullsOutGunMontage)
	{

		AnimInstance->Montage_Play(PullsOutGunMontage);
		AnimInstance->Montage_JumpToSection(FName("PullsOutGun"));

		// Delay the visibility change until the montage is done
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (EqquipedWeapon)
				{
					EqquipedWeapon->GetItemMesh()->SetVisibility(true);


					if (MainSkeletalMesh)
					{

						TArray<FName> ChildComponentNames = { FName("SM_HolsterWeapon"), FName("SM_HolsterNoWeapon") };


						for (USceneComponent* childComponent : MainSkeletalMesh->GetAttachChildren())
						{
							USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(childComponent);

							if (ChildSkeletalMeshComponent)
							{

								FName ComponentName = *ChildSkeletalMeshComponent->GetName();  // Get the name of the child component

								if (ChildComponentNames.Contains(ComponentName))
								{

									if (ComponentName == FName("SM_HolsterWeapon"))
									{
										ChildSkeletalMeshComponent->SetVisibility(false);


									}
									else if (ComponentName == FName("SM_HolsterNoWeapon"))
									{

										ChildSkeletalMeshComponent->SetVisibility(true);


									}

								}


							}

						}
					}
					bHasAWeapon = true;
					bIsShoot = true;

				}
			}, 0.4f, false);




	}

	}



}

void ASurvivalCharacter::IsJogging()
{


	bIsPressingTheJogginKey = true;


	if (!bIsAiming)
	{
	
		SetIsJogging();
	}
	


}

void ASurvivalCharacter::IsNotJoggin()
{
	bIsPressingTheJogginKey = false;


	SetIsNotJogging();
	


}

void ASurvivalCharacter::PlayerWeaponAtStart()
{

	if (EqquipedWeapon)
	{
		EqquipedWeapon->GetItemMesh()->SetVisibility(false);


		if (MainSkeletalMesh)
		{

			TArray<FName> ChildComponentNames = { FName("SM_HolsterWeapon"), FName("SM_HolsterNoWeapon") };


			for (USceneComponent* childComponent : MainSkeletalMesh->GetAttachChildren())
			{
				USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(childComponent);

				if (ChildSkeletalMeshComponent)
				{

					FName ComponentName = *ChildSkeletalMeshComponent->GetName();  // Get the name of the child component

					if (ChildComponentNames.Contains(ComponentName))
					{

						if (ComponentName == FName("SM_HolsterWeapon"))
						{
							ChildSkeletalMeshComponent->SetVisibility(false);


						}
						else if (ComponentName == FName("SM_HolsterNoWeapon"))
						{

							ChildSkeletalMeshComponent->SetVisibility(false);


						}

					}


				}

			}
		}


	}


	
}

void ASurvivalCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	EquipWeapon(WeaponToSwap);



}


//Enables or disables the bIsJogging variale and change the MaxWalkSpeed

void ASurvivalCharacter::SetIsNotJogging()
{
	bIsJogging = false;
	ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;

}

void ASurvivalCharacter::SetIsJogging()
{
	bIsJogging = true;
	ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = JoggingSpeed;

}

	







// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Set camera FOV smoothly (interpotalion for zoom when aiming)
	ASurvivalCharacter::SetCameraFOV(DeltaTime);

	//Calcualte corsshair spread multiplier
	CalculateCorsshairSpread(DeltaTime);

	//Check overlappedItemCount, then trace for items
	TraceForItems();


	if (GEngine) 
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("HasAWeapon?, %s"), bHasAWeapon ? TEXT("true") : TEXT("false")));

		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("IsJoggin?, %s"), bIsJogging ? TEXT("true") : TEXT("false")));
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

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ASurvivalCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &ASurvivalCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("NoWeapon", IE_Pressed, this, &ASurvivalCharacter::NoWeapon);



	PlayerInputComponent->BindAction("Jogging", IE_Pressed, this, &ASurvivalCharacter::IsJogging);
	PlayerInputComponent->BindAction("Jogging", IE_Released, this, &ASurvivalCharacter::IsNotJoggin);

	


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
	

	if (bIsAiming && bIsPressingTheJogginKey)
	{

		SetIsNotJogging();
	}

		


	

}

void ASurvivalCharacter::IsNotAiming()
{
	bIsAiming = false;

	
	if (bIsPressingTheJogginKey)
	{
		SetIsJogging();
	}

	



		


}
	




void ASurvivalCharacter::SetCameraFOV(float DeltaTime)
{


	//Set current camera filed of view
	//if (bIsAiming && bHasAWeapon == true && bIsJogging == false)
	if ((bIsAiming == true && bIsJogging == false)  && bHasAWeapon == true )
	{
		//Interpoalete to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
		//ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
		//bIsJogging = false;
		//IsNotJoggin();

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

void ASurvivalCharacter::IncrementOverlappedItemCount(int8 Amount)
{

	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;

	}
}






