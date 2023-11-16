
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
	CrouchingSpeed = 75.0f;

	bIsJogging = false;

	bGunHasBeenEquipped = false;

	bIsPressingTheJogginKey = false;

	bIsCrouching = false;


	StandingCapsuleHeight = 88.0f;
	CrouchingCapsuleHeight = 44.0f;

	InterpSpeedCrouching = 10.0f;


	CameraBoomSoketOffsetCrouching = { 0.0f, 50.0f, 50.0f };
	CameraBoomSoketOffsetNoCrouching = { 0.0f, 50.0f, 70.0f };

	bHasPcikedUpAmmo = false;
	bHasPickedWeapon = false;

	bFistTimePickingAWeeapon = true;

	//Icon animation properties
	HighlightedSlot = -1;

}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	//EqquipedWeapon->SetWeaponType(EWeaponType::EWT_NoWeapon);

	if (Camera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}


	EquipWeapon(SpawnDefaultWeapon());
	//Inventory.Add(EqquipedWeapon);


	PlayerWeaponAtStart();


	EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);



	CombatState = EComabtState::ECS_NoWeapon;





}

void ASurvivalCharacter::FireWeapon()
{
	if (EqquipedWeapon == nullptr) return;

	if (CombatState != EComabtState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		if (bIsShoot && bIsAiming && bHasAWeapon)
		{
			bIsShoot = false;

			//from where the MuzzleFlash will be placed
			MuzzleFlashPlaced();

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

				//CombatState = EComabtState::ECS_FireTimerInProgress;

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
							//TODO: HeadShot
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
					
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("HitComponent: %s"), *ScreenTraceHit.BoneName.ToString()));

					}
					

					if (BulletHitInterface)
					{
						BulletHitInterface->BulletHit_Implementation(ScreenTraceHit);
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
		ReloadWeapon();


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
			//TraceHitItem->DisableCustomDepthWrapper();


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
				if (!TraceHitWeapon)
				{
					//	TraceHitItemLastFrame->EnableCustomDepthWrapper();


				}

				if (TraceHitWeapon && bHasAWeapon == false)
				{
					//TraceHitItemLastFrame->EnableCustomDepthWrapper();
				
				//
				}


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
			//	TraceHitItemLastFrame->EnableCustomDepthWrapper();
			UnhighlightInventorySlot();

		}


		if (TraceHitWeapon && bHasAWeapon == false || bHasAWeapon == true)
		{
			//TraceHitItemLastFrame->EnableCustomDepthWrapper();
			UnhighlightInventorySlot();

		}

	}


}

void ASurvivalCharacter::OneKeyPressed()
{

	//if (EqquipedWeapon->GetSlotIndex() == 0) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));

	
	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 0);

	///EqquipedWeapon->SetWeaponType(EWeaponType::EWT_Gun);


}

void ASurvivalCharacter::TwoKeyPressed()
{

	//if (EqquipedWeapon->GetSlotIndex() == 1) return;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));


	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 1);


	/*if (EqquipedWeapon != nullptr)
	{
		EqquipedWeapon->SetWeaponType(EWeaponType::EWT_Riffle);
	}
	*/





}

void ASurvivalCharacter::ThreeKeyPressed()
{


	//if (EqquipedWeapon->GetSlotIndex() == 2) return;


	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %s"), *EqquipedWeapon->GetName()));


	ExchangeInventoryItems(EqquipedWeapon->GetSlotIndex(), 2);

	//EqquipedWeapon->SetWeaponType(EWeaponType::EWT_Gun);
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("SlotIndex: %d"), EqquipedWeapon->GetSlotIndex()));


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

	CrosshairSpreadMuliplier = 0.5f + CorsshairVelocityFactor - CrosshairAimFactor + CrosshairShootingFactor;


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
		CombatState = EComabtState::ECS_Unoccupied;

		

		//Get the Hand SocketEqquipedWeapon
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		const USkeletalMeshSocket* HandSocketRiffle = GetMesh()->GetSocketByName(FName("RightHandSocketRiffle"));


		//Check the HandSocket variable 
		if (HandSocket)
		{
			

			//HandSocketRiffle = nullptr;
			//Attach the Weapon to the hand socket
			if (WeponToEquip->GetWeaponType() == EWeaponType::EWT_Gun)
			{
				
				HandSocket->AttachActor(WeponToEquip, GetMesh());
				WeponToEquip->SetWeaponType(EWeaponType::EWT_Gun);

			}
			
			//HandSocketRiffle->AttachActor(WeponToEquip, GetMesh());
		}

		if (HandSocketRiffle )
		{
			//HandSocket = nullptr;
			//Attach the Weapon to the hand socket
			if (WeponToEquip->GetWeaponType() == EWeaponType::EWT_Riffle)
			{
				
				HandSocketRiffle->AttachActor(WeponToEquip, GetMesh());
				WeponToEquip->SetWeaponType(EWeaponType::EWT_Riffle);
			}
			
		}

		//Set EqquipedWeapon to the newly spawned Weapon
		EqquipedWeapon = WeponToEquip;
		//	Inventory.Add(WeponToEquip);
		EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);
		
	}
	else
	{
		CombatState = EComabtState::ECS_NoWeapon;

	}



}


void ASurvivalCharacter::SelectButtonPressed()
{
	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	auto Ammo = Cast<AAmmo>(TraceHitItem);

	if (TraceHitWeapon && !Inventory.Contains(TraceHitWeapon))
	{
		EquipNewWeapon(TraceHitWeapon);
		bHasAWeapon = true;
		
		

		if (TraceHitWeapon->GetPickedUpSound() && bHasPickedWeapon == false)
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickedUpSound());



		}

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


		if (bHasAWeapon)
		{

			bGunHasBeenEquipped = true;
			//	Inventory.Add(TraceHitWeapon);

			TraceHitWeapon->WeaponDisableCustomDepthWrapper();
			UnhighlightInventorySlot();;

			CombatState = EComabtState::ECS_Unoccupied;

			InitializeAmmoMap();


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
								//	EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);



							}
							else if (ComponentName == FName("SM_HolsterNoWeapon"))
							{

								ChildSkeletalMeshComponent->SetVisibility(true);
								//	EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);







							}

						}


					}

				}
			}

		}





		//make the wirdget thing here 



	}

	if (bHasPcikedUpAmmo == true && Ammo)
	{
		PickedAmmo(Ammo);


	}

	bHasPcikedUpAmmo = false;




}




void ASurvivalCharacter::SelectButtonReleased()
{


}

void ASurvivalCharacter::NoWeapon()
{

	if (CombatState != EComabtState::ECS_Unoccupied) return;

	//Getting the anim instance from the character
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();






	if (bHasAWeapon == true && !bIsAiming)
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
						//	EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);

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
											//EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);

											EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);

										}
										else if (ComponentName == FName("SM_HolsterNoWeapon"))
										{

											ChildSkeletalMeshComponent->SetVisibility(false);
											//EqquipedWeapon->SetItemState(EItemState::EIS_Equipped);

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
		//EqquipedWeapon->GetSilencerMesh()->SetVisibility(false);


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

void ASurvivalCharacter::EquipNewWeapon(AWeapon* NewWeaponToEquip)
{


	EquipWeapon(NewWeaponToEquip);

	NewWeaponToEquip->DisableCustomDepthWrapper();

	if (!Inventory.Contains(NewWeaponToEquip))
	{
		NewWeaponToEquip->SetSlotIndex(Inventory.Num());
		Inventory.Add(NewWeaponToEquip);
		if (bFistTimePickingAWeeapon == false)
		{
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

void ASurvivalCharacter::SetIsJogging()
{
	bIsJogging = true;
	if (bIsCrouching == false)
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = JoggingSpeed;

}

void ASurvivalCharacter::InitializeAmmoMap()
{

	AmmoMap.Add(EAmmoType::EAT_GunAmmo, EqquipedWeapon->GetAmmo());
	AmmoMap.Add(EAmmoType::EAT_RiffleAmmo, EqquipedWeapon->GetAmmo());


}

bool ASurvivalCharacter::WeaponHasAmmo()
{

	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return false;


	return EqquipedWeapon->GetMagazineCapacity() > 0;




}

void ASurvivalCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}


}

void ASurvivalCharacter::MuzzleFlashPlaced()
{

	const USkeletalMeshSocket* BarrelSocket = EqquipedWeapon->GetItemMesh()->GetSocketByName("barrelSocket");
	const FTransform socketTransform = BarrelSocket->GetSocketTransform(EqquipedWeapon->GetItemMesh());


	if (MuzzleFlash)
	{

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, socketTransform);


	}
}

void ASurvivalCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

}

void ASurvivalCharacter::ReloadButtonPressed()
{

	ReloadWeapon();


}

void ASurvivalCharacter::ReloadWeapon()
{

	if (CombatState != EComabtState::ECS_Unoccupied) return;

	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return;

	//Do we have ammo of the correct type?

	if (CarryingAmmo() && EqquipedWeapon->IsCarryingMaxCapacity())
	{
		CombatState = EComabtState::ECS_Reloading;

		FName MontageSectionName(TEXT("Reload_Gun"));

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
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

void ASurvivalCharacter::FinishReloading()
{

	CombatState = EComabtState::ECS_Unoccupied;


	if (EqquipedWeapon == nullptr) return;

	//Update the ammoMap

	auto ammoType = EqquipedWeapon->GetAmmoType();

	if (AmmoMap.Contains(ammoType))
	{
		//Ammo that we are carrying
		int32 CarriedAmmo = AmmoMap[ammoType];

		//Space left in the magazine
		int32 MagazineEmptySpace = EqquipedWeapon->MaxCapacityMagazine() - EqquipedWeapon->GetMagazineCapacity();


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

bool ASurvivalCharacter::CarryingAmmo()
{

	if (EqquipedWeapon == nullptr || bHasAWeapon == false) return false;

	auto AmmoType = EqquipedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}


	return false;
}

void ASurvivalCharacter::CrouchButtonPressed()
{
	if (!bIsJogging)
	{
		bIsCrouching = !bIsCrouching;
	}


	if (bIsCrouching == true)
	{
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = CrouchingSpeed;






	}

	if (bIsCrouching == false && bIsJogging == false)
	{
		ASurvivalCharacter::GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;



	}

}

void ASurvivalCharacter::InterpCapsuleHeight(float DeltaTime)
{

	float TargetCapsuleHeight{};
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



	const float InterpHeight = FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHeight,
		DeltaTime,
		InterpSpeedCrouching);

	//Negative values if cruching; Positive values if standing
	const float DeltaCapsuleHeight = InterpHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset{ 0.0f, 0.0f, -DeltaCapsuleHeight };

	GetMesh()->AddLocalOffset(MeshOffset);


	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHeight);



}

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

	if (EqquipedWeapon->GetAmmo() == 0)
	{

		ReloadWeapon();
		Ammo->Destroy();
	}

	if (TraceHitItem->GetPickedUpSound())
	{
		UGameplayStatics::PlaySound2D(this, Ammo->GetPickedUpSound());
	}

	Ammo->Destroy();


}

int32 ASurvivalCharacter::GetEmptyInventorySlot()
{

	for (int i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}


	return -1; // Inventory is full
}

void ASurvivalCharacter::HighlightInventorySlot()
{
	 int32 EmptySlot = GetEmptyInventorySlot();

	 HighlightIconDelegate.Broadcast(EmptySlot, true);

	 HighlightedSlot = EmptySlot;

	 

}

void ASurvivalCharacter::UnhighlightInventorySlot()
{

	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
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

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("HasAWeapon?, %s"), bHasAWeapon ? TEXT("true") : TEXT("false")));

		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("IsJoggin?, %s"), bIsJogging ? TEXT("true") : TEXT("false")));
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("velocity?, %f"), this->GetVelocity().Size()));

		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("ammountMagazine?, %d"), EqquipedWeapon->MaxCapacityMagazine()));

		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("IsCrouching?, %s"), bIsCrouching ? TEXT("true") : TEXT("false")));

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


	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &ASurvivalCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::CrouchButtonPressed);


	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &ASurvivalCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &ASurvivalCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &ASurvivalCharacter::ThreeKeyPressed);






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



		const int a = 1;

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

	if (!bHasAWeapon)
	{
		bIsAiming = false;

	}
	else
	{
		bIsAiming = true;

	}



	if (bIsAiming && bIsPressingTheJogginKey && bHasAWeapon)
	{

		SetIsNotJogging();
	}






}

void ASurvivalCharacter::IsNotAiming()
{
	bIsAiming = false;


	if (bIsPressingTheJogginKey && bHasAWeapon)
	{
		SetIsJogging();
	}








}





void ASurvivalCharacter::SetCameraFOV(float DeltaTime)
{


	//Set current camera filed of view
	//if (bIsAiming && bHasAWeapon == true && bIsJogging == false)
	if ((bIsAiming == true && bIsJogging == false) && bHasAWeapon == true)
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




