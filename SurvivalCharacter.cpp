
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
#include "Ammo.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "HospitalProject.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SpotLightComponent.h"


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

	//Main skeletal mesh of the character
	MainSkeletalMesh = GetMesh();

	//Flashlight setup
	FlashLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotlightComponent"));
	FlashLight->SetupAttachment(Camera);
	FlashLight->SetIntensity(150.0f);


	//Indicates whether the character is allowed to shoot
	bIsShoot = true;

	//Time interval between consecutive shots when shooting
	timeBetweenShootsGun = 0.5f;

	//Indicates whether the character is currently aiming
	bIsAiming = false;
	bHasBeenAiming = false;

	//Indicates whether the character has aimed while walking
	bHasBeenAimingWalk = false;

	//Maximum distance at which the character can shoot
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

	//Indicates whether the character currently has a weapon
	bHasAWeapon = false;

	//Movement speeds for different states
	WalkingSpeed = 150.0f;
	JoggingSpeed = 300.0f;
	CrouchingSpeed = 75.0f;

	//Indicates whether the character is currently jogging
	bIsJogging = false;

	//Indicates whether a gun has been equipped by the character
	bGunHasBeenEquipped = false;

	//Indicates whether the jogging key is being pressed
	bIsPressingTheJogginKey = false;

	//Indicates whether the character is currently crouching
	bIsCrouching = false;

	//Capsule heights for standing and crouching states
	StandingCapsuleHeight = 88.0f;
	CrouchingCapsuleHeight = 44.0f;

	//Interpolation speed for the character when crouching
	InterpSpeedCrouching = 10.0f;

	//Socket offsets for the camera boom in crouching and non-crouching states
	CameraBoomSoketOffsetCrouching = { 0.0f, 50.0f, 50.0f };
	CameraBoomSoketOffsetNoCrouching = { 0.0f, 50.0f, 70.0f };

	//Flags indicating whether the character has picked up ammo or a weapon
	bHasPcikedUpAmmo = false;
	bHasPickedWeapon = false;
	bFistTimePickingAWeeapon = true;

	//Icon animation properties
	HighlightedSlot = -1;

	//Current and max health of the character
	Health = 100.0f;
	MaxHealth = 100.0f;

	//Chance of being stunned when attacked
	StunChance = 0.25f;

	//Current amount of dirt on the character (when it's hit)
	CurrentDirtAmount = 0.0f; 

	//Amount of blood on the character (when it's hit)
	AmountOfBlood = 0.20f;

	//Flag indicating whether the character has been attacked
	bHasBeenAttacked = false;

	//Time the screen will show blood effects after being attacked
	timeScreenBloodActive = 15.0f;

	//The time interval after death that must elapse before the player can play again.
	timeAfterDying = 2.0f;
}

// Override the TakeDamage function to handle damage events for the character
float ASurvivalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Check if the character's health after taking damage goes below a threshold
	if (Health - DamageAmount <= -0.1f)
	{
		// If health is below threshold, set it to 0 and trigger the character's death
		Health = 0.0f;
		Die();

		// Check if the damage instigator is an enemy controller
		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			// Set a Blackboard value indicating that the character is dead
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
		}
	}
	else
	{
		Health -= DamageAmount;
		bHasBeenAttacked = true;
		TimeScreenBlood(timeScreenBloodActive);
	//Camera shake every time our player is attacked
		if (CameraShake)
		{
			UMatineeCameraShake::StartMatineeCameraShake(
				GetWorld()->GetFirstPlayerController()->PlayerCameraManager,
				CameraShake,
				1.0f,
				ECameraShakePlaySpace::CameraLocal,
				FRotator::ZeroRotator);
		}

		DirtyClothes();

	


	}

	return DamageAmount;

}

// Handle the character's death by playing a death montage, displaying a death widget,
// and scheduling a level change after a specified delay.
void ASurvivalCharacter::Die()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathtReactMontage)
	{
		AnimInstance->Montage_Play(DeathtReactMontage);

		// Display the death widget
		SetDeathWidget();

		FTimerHandle LevelChangeTimerHandle;

		// Schedule a level change after the specified delay
		GetWorldTimerManager().SetTimer(
			LevelChangeTimerHandle,
			this,
			&ASurvivalCharacter::ChangeLevelAfterDying,
			timeAfterDying,
			false);

	}



}

// Set up and display the death widget if the DeathWidgetClass is defined
void ASurvivalCharacter::SetDeathWidget()
{

	if (DeathWidgetClass)
	{
		// Create an instance of the death widget
		UUserWidget* DeathWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);

		// If the widget instance is valid, mark the character as dead and add it to the viewport
		if (DeathWidgetInstance)
		{
			bIsDead = true;
			DeathWidgetInstance->AddToViewport();
		}

	}



}

// Change the level to "Lvl_MainMenu" after a specified delay
void ASurvivalCharacter::ChangeLevelAfterDying()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("Lvl_MainMenu"));

}

// Finish the death process by pausing animations and stopping movements
void ASurvivalCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	StopMovements();
}

// Stop character movements by disabling input for the player controller
void ASurvivalCharacter::StopMovements()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}

}

// Get the surface type based on a line trace from the character's location
EPhysicalSurface ASurvivalCharacter::GetSurfaceType()
{

	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.0f, 0.0f, -400.0f);
	
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		QueryParams);
		return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
		
		
	

}



// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Resetting character state and initializing default settings
	bIsDead = false;

	// Initialize camera settings if available
	if (Camera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Equip the default weapon and perform initializations
	EquipWeapon(SpawnDefaultWeapon());
	PlayerWeaponAtStart();

	// Hide the silencer mesh of the equipped weapon
	EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);

	// Set the initial combat state to no weapon
	CombatState = EComabtState::ECS_NoWeapon;
}

// Function to handle firing the weapon
void ASurvivalCharacter::FireWeapon()
{
	// Check if the character has a weapon equipped
	if (EqquipedWeapon == nullptr) return;

	// Check combat state to prevent firing in certain situations
	if (CombatState != EComabtState::ECS_Unoccupied || CombatState == EComabtState::ECS_Stunned) return;

	// Check if the weapon has ammo
	if (WeaponHasAmmo())
	{
		if (bIsShoot && bIsAiming && bHasAWeapon)
		{
			bIsShoot = false;

			//from where the MuzzleFlash will be placed
			MuzzleFlashPlaced();

			if (bIsShoot == false)
			{
				// Ensure a minimum time between shots
				if (timeBetweenShootsGun <= 0)
				{
					timeBetweenShootsGun = 0.1f;
				}

				// Set a timer for firing cooldown
				FTimerHandle TimerHandle_FireCoolDown;
				GetWorldTimerManager().SetTimer(TimerHandle_FireCoolDown,
					this,
					&ASurvivalCharacter::EnableFire,
					timeBetweenShootsGun,
					false);
			}

			//Play fire sound 
			PlayFireSound();

			//Play HipFireMontage
			PlayGunFireMontage();

			//Subtract -1 from the weapon's ammo
			EqquipedWeapon->DecreaseAmmo();

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


				//Does hit actor implement bulletHitInterface?
				if (ScreenTraceHit.GetActor())
				{
					IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(ScreenTraceHit.GetActor());

					AEnemy* HitEnemy = Cast<AEnemy>(ScreenTraceHit.GetActor());
					if (HitEnemy)
					{
						if (ScreenTraceHit.BoneName.ToString() == HitEnemy->GetHeadBoneName())
						{
							
							UGameplayStatics::ApplyDamage(
								ScreenTraceHit.GetActor(),
								EqquipedWeapon->GetHeadShotDamage(),
								GetController(),
								this,
								UDamageType::StaticClass());

						}
						else
						{
							//BodyShot
							UGameplayStatics::ApplyDamage(
								ScreenTraceHit.GetActor(),
								EqquipedWeapon->GetDamage(),
								GetController(),
								this,
								UDamageType::StaticClass());

						}
					

					}
					
					// Check if the hit actor implements BulletHitInterface
					if (BulletHitInterface)
					{
						BulletHitInterface->BulletHit_Implementation(ScreenTraceHit, this, GetController());
					}
					else
					{

						//DefaultParticles
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
	else
	{
		// Reload the weapon if out of ammo
		ReloadWeapon();
	}
}

// Function to enable shooting 
void ASurvivalCharacter::EnableFire()
{
	bIsShoot = true;
}

// Determining what is under the crosshairs on the screen, 
//allowing for interactions or calculations based on the 
//object or surface that the player is aiming at.
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

	//Get world position and direction of crorsshairs
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
		FVector End{ Start + CorsshairsWorldDirection * ShootingDistance };
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


// TraceForItems handles the detection and interaction with items under the player's crosshairs.
// It checks for weapons, ammo, and other items, displaying relevant widgets and managing inventory slot highlights.
void ASurvivalCharacter::TraceForItems()
{

	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	auto Ammo = Cast<AAmmo>(TraceHitItem);
	if (bShouldTraceForItems)
	{
		FHitResult CrosshairsHitResult;
		bool bScreenToWorld = TraceUnderCorsshairs(CrosshairsHitResult);
		TraceHitItem = Cast< AItem>(CrosshairsHitResult.GetActor());

		if (TraceHitItem && TraceHitItem->GetPickupWidget())
		{

			//Show item's pickup widget
			TraceHitItem->GetPickupWidget()->SetVisibility(true);

			TraceHitItem->GetInformationpWidget()->SetVisibility(true);


			bHasPcikedUpAmmo = true;

		}

		if (TraceHitWeapon)
		{
			bHasPickedWeapon = false;

			if (HighlightedSlot == -1)
			{
				//Not currently highlighting a slot, highlight one
				HighlightInventorySlot();
			}


		}
		else
		{
			//is there a slot being highlighted?
			if (HighlightedSlot != -1)
			{
				//Unhighlght the slot
				UnhighlightInventorySlot();
			}
		}



		//We hit an AItem last frame
		if (TraceHitItemLastFrame)
		{
			if (TraceHitItem != TraceHitItemLastFrame)
			{
				//We are hitting a different AItem this frame from last frame 
				//Or AItem is null
				TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				bHasPcikedUpAmmo = false;
			


			}
		}

		//Store a reference to HitItem for next frame
		TraceHitItemLastFrame = TraceHitItem;


	}
	else if (!bShouldTraceForItems && TraceHitItemLastFrame)
	{
		//No longer overlapping any itmes 
		//Item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);


		if (!TraceHitWeapon)
		{
			UnhighlightInventorySlot();

		}


		if (TraceHitWeapon && bHasAWeapon == false || bHasAWeapon == true)
		{
			UnhighlightInventorySlot();

		}

	}


}

//TODO: This section has to be redone when the game will have more weapons avaliable
void ASurvivalCharacter::OneKeyPressed()
{
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));
	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 0);
}

void ASurvivalCharacter::TwoKeyPressed()
{

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));
	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 1);

}

void ASurvivalCharacter::ThreeKeyPressed()
{

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));
	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 2);

}

void ASurvivalCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{

	if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex > Inventory.Num()-1)) return;

	

	auto OldEquippedWeapon = EqquipedWeapon;
	auto NewWeapon = Inventory[NewItemIndex];
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	OldEquippedWeapon->GetSilencerMesh()->SetVisibility(false);

	NewWeapon->SetItemState(EItemState::EIS_Equipped);
	EquipItemDelegate.Broadcast(OldEquippedWeapon->GetSlotIndex(), NewWeapon->GetSlotIndex());
}


// Turn is responsible for rotating the player character horizontally (yaw) based on input value.
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


// LookUp is responsible for rotating the player character vertically (pitch) based on input value.
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


// CalculateCorsshairSpread calculates the crosshair spread multiplier based on various factors.
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

	CrosshairSpreadMuliplier = 0.5f + CorsshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;


}

// FinishCrosshairBulletFire initiates the process of finishing the crosshair bullet fire.
void ASurvivalCharacter::FinishCrosshairBulletFire()
{
	// Set bFiringBullet to true to indicate that the character is currently firing a bullet.
	bFiringBullet = true;
	
	//Time to set false the bFiringBullet variable 
	FTimerHandle CrosshairShootTimer;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ASurvivalCharacter::StartCorsshairBulletFire, 0.005f);

}

//reset the bFiringBullet variable, indicating the completion of crosshair bullet fire.
void ASurvivalCharacter::StartCorsshairBulletFire()
{
	bFiringBullet = false;
}


// SpawnDefaultWeapon creates an instance of the default weapon class for the character.
// Returns a pointer to the spawned weapon, or nullptr if the DefaultWeaponClass is not set
AWeapon* ASurvivalCharacter::SpawnDefaultWeapon()
{

	//Check the TSubclasOf variable 
	if (DefaultWeaponClass)
	{
		//Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);



	}
	return nullptr;


}

// EquipWeapon equips the specified weapon to the character, attaching it to the appropriate hand socket.
// Updates the CombatState and sets EqquipedWeapon to the newly equipped weapon.
// If WeponToEquip is nullptr, sets CombatState to ECS_NoWeapon.
void ASurvivalCharacter::EquipWeapon(AWeapon* WeponToEquip)
{

	if (WeponToEquip)
	{
		CombatState = EComabtState::ECS_Unoccupied;

		

		//Get the Hand SocketEqquipedWeapon
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		const USkeletalMeshSocket* HandSocketRiffle = GetMesh()->GetSocketByName(FName("RightHandSocketRiffle"));


		//Check the HandSocket variable 
		if (HandSocket)
		{
			

			//Attach the Weapon to the hand socket
			if (WeponToEquip->GetWeaponType() == EWeaponType::EWT_Gun)
			{
				
				HandSocket->AttachActor(WeponToEquip, GetMesh());
				WeponToEquip->SetWeaponType(EWeaponType::EWT_Gun);

			}
			
		}

		if (HandSocketRiffle )
		{
			//Attach the Weapon to the hand socket
			if (WeponToEquip->GetWeaponType() == EWeaponType::EWT_Riffle)
			{
				
				HandSocketRiffle->AttachActor(WeponToEquip, GetMesh());
				WeponToEquip->SetWeaponType(EWeaponType::EWT_Riffle);
			}
			
		}

		//Set EqquipedWeapon to the newly spawned Weapon
		EqquipedWeapon = WeponToEquip;
		EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);
		
	}
	else
	{
		CombatState = EComabtState::ECS_NoWeapon;

	}



}

// SelectButtonPressed is triggered when the player presses the select button "E"
void ASurvivalCharacter::SelectButtonPressed()
{
	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	auto Ammo = Cast<AAmmo>(TraceHitItem);

	// If the traced item is a weapon and not already in the inventory
	if (TraceHitWeapon && !Inventory.Contains(TraceHitWeapon))
	{
		EquipNewWeapon(TraceHitWeapon);
		bHasAWeapon = true;
		
		
		// Play the pickup sound if available
		if (TraceHitWeapon->GetPickedUpSound() && bHasPickedWeapon == false)
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickedUpSound());
		}

		// Set weapon type flags based on the type of weapon
		if (TraceHitWeapon->GetWeaponType() == EWeaponType::EWT_Gun)
		{
			bisAGun = true;
			bisARiffle = false;
		}

		if (TraceHitWeapon->GetWeaponType() == EWeaponType::EWT_Riffle)
		{
			bisAGun = false;
			bisARiffle = true;
		}

		bHasPickedWeapon = true;

		// Update states if the player has a weapon
		if (bHasAWeapon)
		{
			bGunHasBeenEquipped = true;

			TraceHitWeapon->WeaponDisableCustomDepthWrapper();
			UnhighlightInventorySlot();;

			CombatState = EComabtState::ECS_Unoccupied;

			InitializeAmmoMap();

			// Modify skeletal mesh components based on weapon type
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

	if (bHasPcikedUpAmmo == true && Ammo)
	{
		PickedAmmo(Ammo);


	}

	bHasPcikedUpAmmo = false;




}





// NoWeapon is responsible for handling the character's state when they have no weapon equipped.
// It plays appropriate animations for holstering and pulling out the weapon, adjusts visibility,
// and updates relevant flags and states.
void ASurvivalCharacter::NoWeapon()
{
	// If the character is not in an unoccupied combat state, return
	if (CombatState != EComabtState::ECS_Unoccupied) return;

	//Getting the anim instance from the character
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// If the character has a weapon and is not aiming
	if (bHasAWeapon == true && !bIsAiming)
	{
		//Checking that neither AnimInstance nor EquipGunMontage are not null
		if (AnimInstance && EquipGunMontage)
		{
			// Play the EquipGunMontage and jump to the EquipGun section
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

											EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);

										}
										else if (ComponentName == FName("SM_HolsterNoWeapon"))
										{

											ChildSkeletalMeshComponent->SetVisibility(false);

											EqquipedWeapon->GetSilencerMesh()->SetVisibility(true);

										}

									}


								}

							}
						}
						bHasAWeapon = false;
						bIsShoot = false;
						EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);

					}
				}, 0.6f, false);


		}
		else
		{
			CombatState = EComabtState::ECS_NoWeapon;
		}
	}

	// If the character has no weapon, the traced item is valid, and a gun has been equipped
	if (bHasAWeapon == false && TraceHitItem && bGunHasBeenEquipped == true)
	{


		//Checking that neither AnimInstance nor EquipGunMontage are not null
		if (AnimInstance && PullsOutGunMontage)
		{
			// Play the PullsOutGunMontage and jump to the PullsOutGun section
			AnimInstance->Montage_Play(PullsOutGunMontage);
			AnimInstance->Montage_JumpToSection(FName("PullsOutGun"));

			// Delay the visibility change until the montage is done
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
				{
					if (EqquipedWeapon)
					{
						EqquipedWeapon->GetItemMesh()->SetVisibility(true);
						EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);

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
						EqquipedWeapon->GetSilencerMesh()->SetVisibility(true);

					}
				}, 0.4f, false);




		}

	}



}

// IsJogging is called to handle the character's jogging state.
void ASurvivalCharacter::IsJogging()
{
	// Set the flag indicating that the jogging key is pressed
	bIsPressingTheJogginKey = true;

	// If the character is not aiming, call the SetIsJogging function
	if (!bIsAiming)
	{
		SetIsJogging();
	}

}

// IsNotJogging function
void ASurvivalCharacter::IsNotJoggin()
{
	// Reset the flag indicating that the jogging key is pressed
	bIsPressingTheJogginKey = false;

	// Call the SetIsNotJogging function
	SetIsNotJogging();

}

// PlayerWeaponAtStart is called to handle the initial state of the player's weapon. No show any weapon
//Just showing where the weapon is going to be stored
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

// EquipNewWeapon is called to equip a new weapon and update the inventory.
void ASurvivalCharacter::EquipNewWeapon(AWeapon* NewWeaponToEquip)
{
	// Equip the new weapon
	EquipWeapon(NewWeaponToEquip);

	// Disable custom depth for the new weapon
	NewWeaponToEquip->DisableCustomDepthWrapper();

	// Check if the new weapon is not already in the inventory
	if (!Inventory.Contains(NewWeaponToEquip))
	{
		// Set the slot index for the new weapon and add it to the inventory
		NewWeaponToEquip->SetSlotIndex(Inventory.Num());
		Inventory.Add(NewWeaponToEquip);

		// Check if it's the first time picking up a weapon
		if (bFistTimePickingAWeeapon == false)
		{
			// Set the item state, and hide the silencer mesh for the new weapon
			NewWeaponToEquip->SetItemState(EItemState::EIS_PickedUp);
			NewWeaponToEquip->GetSilencerMesh()->SetVisibility(false);
		}

		bFistTimePickingAWeeapon = true;

	}
}


//Enables or disables the bIsJogging variale and change the MaxWalkSpeed
void ASurvivalCharacter::SetIsNotJogging()
{
	bIsJogging = false;
	ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;

}

// SetIsJogging function is called to set the character state to jogging.
void ASurvivalCharacter::SetIsJogging()
{
	bIsJogging = true;
	if (bIsCrouching == false)
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = JoggingSpeed;

}

// InitializeAmmoMap function is called to set up the initial ammo values for the equipped weapon.
void ASurvivalCharacter::InitializeAmmoMap()
{
	// Add ammo values for different ammo types to the AmmoMap
	AmmoMap.Add(EAmmoType::EAT_GunAmmo, EqquipedWeapon->GetAmmo());
	AmmoMap.Add(EAmmoType::EAT_RiffleAmmo, EqquipedWeapon->GetAmmo());
}

// Checks if the equipped weapon has ammo in its magazine.
bool ASurvivalCharacter::WeaponHasAmmo()
{
	// If there is no equipped weapon or the character does not have a weapon, return false.
	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return false;

	// Return true if the equipped weapon has ammo in its magazine.
	return EqquipedWeapon->GetMagazineCapacity() > 0;
}


// Plays the fire sound associated with the character's equipped weapon.
void ASurvivalCharacter::PlayFireSound()
{
	// Check if a fire sound is assigned to the character's equipped weapon.
	if (FireSound)
	{
		// Play the fire sound in 2D space.
		UGameplayStatics::PlaySound2D(this, FireSound);
	}


}

// Places the muzzle flash at the weapon's barrel socket location.
void ASurvivalCharacter::MuzzleFlashPlaced()
{
	// Get the barrel socket from the equipped weapon's item mesh.
	const USkeletalMeshSocket* BarrelSocket = EqquipedWeapon->GetItemMesh()->GetSocketByName("barrelSocket");
	// Get the socket transform.
	const FTransform socketTransform = BarrelSocket->GetSocketTransform(EqquipedWeapon->GetItemMesh());

	// Check if a muzzle flash particle system is assigned to the character's equipped weapon.
	if (MuzzleFlash)
	{
		// Spawn the muzzle flash particle system at the socket location.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, socketTransform);
	}
}

// Plays the gun fire montage for hip-fire animations.
void ASurvivalCharacter::PlayGunFireMontage()
{
	// Get the animation instance from the character's mesh.
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// Check if the animation instance and hip-fire montage are valid.
	if (AnimInstance && HipFireMontage)
	{
		// Play the hip-fire montage and jump to the "StartFire" section.
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

}

// Initiates the reload process when the reload button is pressed "r"
void ASurvivalCharacter::ReloadButtonPressed()
{
	// Call the function to reload the equipped weapon.
	ReloadWeapon();

}

// Initiates the reload process for the equipped weapon.
void ASurvivalCharacter::ReloadWeapon()
{
	// Check if the character is in a valid combat state for reloading.
	if (CombatState != EComabtState::ECS_Unoccupied || CombatState == EComabtState::ECS_Stunned) return;

	// Check if the character has a weapon and is holding a weapon.
	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return;

	// Check if the character is carrying the appropriate ammo type and the weapon is not at max capacity.
	if (CarryingAmmo() && EqquipedWeapon->IsCarryingMaxCapacity())
	{
		// Set the combat state to reloading.
		CombatState = EComabtState::ECS_Reloading;

		// Get the montage section name for reloading.
		FName MontageSectionName(TEXT("Reload_Gun"));

		// Get the animation instance from the character's mesh.
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		// Check if the animation instance and reload montage are valid.
		if (AnimInstance && ReloadMontage)
		{
			// Play the reload montage and jump to the specified section.
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EqquipedWeapon->GetReloadMontageSection());
		}
	}
	else
	{	//If the wapon doesn't have ammo, play a sound
		if (EqquipedWeapon->GetNoAmmoSound())
		{
			UGameplayStatics::PlaySound2D(this, EqquipedWeapon->GetNoAmmoSound());
		}
	}



}

// Completes the reloading process for the equipped weapon.
void ASurvivalCharacter::FinishReloading()
{

	// Check if the character is stunned, preventing the completion of the reload.
	if (CombatState == EComabtState::ECS_Stunned) return;

	// Set the combat state to unoccupied.
	CombatState = EComabtState::ECS_Unoccupied;

	// Check if the character has a weapon.
	if (EqquipedWeapon == nullptr) return;

	// Update the ammoMap based on the weapon's ammo type.
	auto ammoType = EqquipedWeapon->GetAmmoType();

	if (AmmoMap.Contains(ammoType))
	{
		//Ammo that we are carrying
		int32 CarriedAmmo = AmmoMap[ammoType];

		//Space left in the magazine
		int32 MagazineEmptySpace = EqquipedWeapon->MaxCapacityMagazine() - EqquipedWeapon->GetMagazineCapacity();


		// Reload the weapon based on available ammo.
		if (MagazineEmptySpace < CarriedAmmo)
		{
			EqquipedWeapon->ReloadAmmo(MagazineEmptySpace);
			CarriedAmmo -= MagazineEmptySpace;
			AmmoMap.Add(ammoType, CarriedAmmo);
		}
		else
		{
			EqquipedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(ammoType, CarriedAmmo);
		}



	}


}

// Checks if the character is carrying ammo for the equipped weapon.
bool ASurvivalCharacter::CarryingAmmo()
{
	// If the character has no weapon, or no weapon is equipped, return false.
	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return false;

	// Get the ammo type of the equipped weapon.
	auto AmmoType = EqquipedWeapon->GetAmmoType();

	// Check if the ammo map contains the specified ammo type.
	if (AmmoMap.Contains(AmmoType))
	{
		// Return true if the character has more than 0 ammo of the specified type.
		return AmmoMap[AmmoType] > 0;
	}


	return false;
}


// Handles the crouch button press.
void ASurvivalCharacter::CrouchButtonPressed()
{
	// If not jogging, toggle crouching state.
	if (!bIsJogging)
	{
		bIsCrouching = !bIsCrouching;
	}

	// Adjust character movement speed based on crouching state.
	if (bIsCrouching == true)
	{
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = CrouchingSpeed;

	}

	// If not crouching and not jogging, revert to walking speed.
	if (bIsCrouching == false && bIsJogging == false)
	{
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;

	}

}

// Interpolates the capsule height based on crouching state.
void ASurvivalCharacter::InterpCapsuleHeight(float DeltaTime)
{

	float TargetCapsuleHeight{};

	// Set the target capsule height and camera boom socket offset based on crouching state.
	if (bIsCrouching)
	{
		TargetCapsuleHeight = CrouchingCapsuleHeight;
		CameraBoom->SocketOffset = CameraBoomSoketOffsetCrouching;

	}
	else
	{
		TargetCapsuleHeight = StandingCapsuleHeight;
		CameraBoom->SocketOffset = CameraBoomSoketOffsetNoCrouching;
	}


	// Interpolate the capsule height.
	const float InterpHeight = FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHeight,
		DeltaTime,
		InterpSpeedCrouching);

	//Negative values if cruching; Positive values if standing
	const float DeltaCapsuleHeight = InterpHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset{ 0.0f, 0.0f, -DeltaCapsuleHeight };

	// Apply the mesh offset.
	GetMesh()->AddLocalOffset(MeshOffset);

	// Set the new capsule half height.
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHeight);



}

// Handles picking up ammo and updating the AmmoMap.
void ASurvivalCharacter::PickedAmmo(AAmmo* Ammo)
{

	//Check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		//Get amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		//Set the amount of ammo in the Map for this type 
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	// If the equipped weapon has no ammo, reload.
	if (EqquipedWeapon->GetAmmo() == 0)
	{
		ReloadWeapon();
		Ammo->Destroy();
	}

	// Play the picked-up sound if available.
	if (TraceHitItem->GetPickedUpSound())
	{
		UGameplayStatics::PlaySound2D(this, Ammo->GetPickedUpSound());
	}

	// Destroy the Ammo actor.
	Ammo->Destroy();


}

// Retrieves the index of the first empty slot in the inventory.
int32 ASurvivalCharacter::GetEmptyInventorySlot()
{

	for (int i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	// If there are empty slots and the inventory is not full, return the next index.
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}



	return -1; // Inventory is full
}


// Highlights an empty slot in the inventory.
void ASurvivalCharacter::HighlightInventorySlot()
{
	 int32 EmptySlot = GetEmptyInventorySlot();

	 HighlightIconDelegate.Broadcast(EmptySlot, true);

	 HighlightedSlot = EmptySlot;

}

// Ends the stun state, allowing the character to resume normal combat.
void ASurvivalCharacter::EndStun()
{
	// Set combat state to unoccupied after the stun.
	CombatState = EComabtState::ECS_Unoccupied;

	// If aiming when stunned, call the IsAiming function to restore aiming state.
	if (bIsAiming)
	{
		IsAiming();
	}


}


// Applies dirt (blood or other substances) to the character's clothes based on specified parameters.
void ASurvivalCharacter::DirtyClothes()
{

	// Check if the main skeletal mesh is valid.
	if (MainSkeletalMesh)
	{
		// Define the names of child components representing clothes.
		TArray<FName> ChildComponentNames = { FName("SM_Jacket"), FName("SM_Trousers") };

		// Iterate through the attached children of the main skeletal mesh.
		for (USceneComponent* ChildComponent : MainSkeletalMesh->GetAttachChildren())
		{
			USkeletalMeshComponent* ChildSkeletalMeshComponent = Cast<USkeletalMeshComponent>(ChildComponent);

			// Check if the child component is a skeletal mesh component and has a valid name.
			if (ChildSkeletalMeshComponent && ChildComponentNames.Contains(ChildComponent->GetFName()))
			{
				
				// Handle trousers material.
				if (ChildComponent->GetFName() == FName("SM_Trousers"))
				{
					Material = ChildSkeletalMeshComponent->GetMaterial(0);
					if (Material)
					{
						if (!DynamicMaterial)
						{
							DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
							ChildSkeletalMeshComponent->SetMaterial(0, DynamicMaterial);
							CurrentDirtAmount = AmountOfBlood;
						}

						CurrentDirtAmount += AmountOfBlood;
						DynamicMaterial->SetScalarParameterValue(TEXT("Dirt Amount"), CurrentDirtAmount);
					}
				}

				// Handle jacket material.
				 if (ChildComponent->GetFName() == FName("SM_Jacket"))
				{
					
					Material2 = ChildSkeletalMeshComponent->GetMaterial(0);
					
					if (Material2)
					{
						if (!TrousersMaterials)
						{
							TrousersMaterials = UMaterialInstanceDynamic::Create(Material2, this);
							ChildSkeletalMeshComponent->SetMaterial(0, TrousersMaterials);
							CurrentDirtAmount = AmountOfBlood;
						}
						
							
					
							CurrentDirtAmount += AmountOfBlood;
							TrousersMaterials->SetScalarParameterValue(TEXT("Dirt Amount"), CurrentDirtAmount);
						
						
					}
				}
				

				
			}
		}
	}



}

// Initiates a timer to reset the "HasBeenAttacked" state after a specified duration.
void ASurvivalCharacter::TimeScreenBlood(float ScreenBloodTime = 2.0f)
{

	FTimerHandle TimerHandleScreenBoodTime;
	GetWorldTimerManager().SetTimer(TimerHandleScreenBoodTime,
		this,
		&ASurvivalCharacter::ResetHasBeenAttacked,
		ScreenBloodTime,
		false);


}

// Resets the "HasBeenAttacked" state and clears dirt amounts on clothing materials.
void ASurvivalCharacter::ResetHasBeenAttacked()
{

	bHasBeenAttacked = false;	
	CurrentDirtAmount = 0.0f;
	TrousersMaterials->SetScalarParameterValue(TEXT("Dirt Amount"), CurrentDirtAmount);
	DynamicMaterial->SetScalarParameterValue(TEXT("Dirt Amount"), CurrentDirtAmount);


}

// Unhighlights the currently highlighted inventory slot.
void ASurvivalCharacter::UnhighlightInventorySlot()
{

	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

// Initiates a stun for the character, playing the hit react montage
void ASurvivalCharacter::Stun()
{
	if (Health <= 0.0f) return;

	CombatState = EComabtState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}


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

	//Interpolating the capsule height based on crouching/standing
	InterpCapsuleHeight(DeltaTime);


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

	// Calling SelectButtonPressed when the key "E"is pressed
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ASurvivalCharacter::SelectButtonPressed);
	

	//Calling the NoWeapon function to store the weapon
	PlayerInputComponent->BindAction("NoWeapon", IE_Pressed, this, &ASurvivalCharacter::NoWeapon);


	//PlayerInputComponent for joggin
	PlayerInputComponent->BindAction("Jogging", IE_Pressed, this, &ASurvivalCharacter::IsJogging);
	PlayerInputComponent->BindAction("Jogging", IE_Released, this, &ASurvivalCharacter::IsNotJoggin);

	//PlayerInputComponent for reaload weaon
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &ASurvivalCharacter::ReloadButtonPressed);

	//PlayerInputComponent for crouching
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::CrouchButtonPressed);

	//PlayerInputComponent for different weapong in our inventory
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &ASurvivalCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &ASurvivalCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &ASurvivalCharacter::ThreeKeyPressed);

}


// Handles the character's forward and backward movement based on the input value.
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

		const int a = 1;

	}

}

// Handles the character's right and left movement based on the input value.
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

// Handles the character entering aiming mode.
void ASurvivalCharacter::IsAiming()
{
	// If the character does not have a weapon, aiming is not possible.
	if (!bHasAWeapon)
	{
		bIsAiming = false;

	}
	else
	{
		// Character is now aiming.
		bIsAiming = true;

		// Play aiming sound if available.
		if (AimingSound)
		{
			UGameplayStatics::PlaySound2D(this, AimingSound);
		}

	}

	// If aiming while pressing the jogging key and having a weapon, exit jogging.
	if (bIsAiming && bIsPressingTheJogginKey && bHasAWeapon)
	{
		SetIsNotJogging();
	}

}

// Handles the character exiting aiming mode.
void ASurvivalCharacter::IsNotAiming()
{
	// Character is no longer aiming.
	bIsAiming = false;

	// If pressing the jogging key and having a weapon, enter jogging mode.
	if (bIsPressingTheJogginKey && bHasAWeapon)
	{
		SetIsJogging();
	}
}


// Sets the camera's field of view based on aiming and jogging states.
void ASurvivalCharacter::SetCameraFOV(float DeltaTime)
{
	//Set current camera filed of view
	//if (bIsAiming && bHasAWeapon == true && bIsJogging == false)
	if ((bIsAiming == true && bIsJogging == false) && bHasAWeapon == true)
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


// Returns the current crosshair spread multiplier.
float ASurvivalCharacter::GetCrosshairSpreadMultiplier()
{
	return 	CrosshairSpreadMuliplier;

}

// Increment or decrement the count of overlapped items based on the given amount.
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
