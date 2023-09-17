// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalAnimInstance.h"
#include "SurvivalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void USurvivalAnimInstance::UpdateAnimationProperties(float deltaTime)
{

	//Check that SurvivalCharacter is not null
	if (!SurvivalCharacter)
	{
		SurvivalCharacter = Cast<ASurvivalCharacter>(TryGetPawnOwner());

	}
	//if SurvivalCharacter is not null dive into the folloing conditional  

	if (SurvivalCharacter)
	{
		//Get the spped of the character from velocity
		FVector velocity{ SurvivalCharacter->GetVelocity() };
		velocity.Z = 0.0f;
		speed = velocity.Size();

		//Is the character accelerating?
		if (SurvivalCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 1.0f)
		{
			bIsAccelerating = true;

		}
		else
		{
			bIsAccelerating = false;

		}

		//Gives us the AimRotator where our character is aiming 

	//FRotator AimRotator = SurvivalCharacter->GetBaseAimRotation();
		FRotator AimRotator = SurvivalCharacter->GetBaseAimRotation();
		//FString RotationMessage = FString::Printf(TEXT("Base aim rotation: %f"), AimRotator.Yaw);

		//this gives us the vector from character velocity
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(SurvivalCharacter->GetVelocity());
		//FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);

		//Difference between MovementRotation and AimRotator to set correctly direction variable in order
		//to set belndSpace
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotator).Yaw;

		if (SurvivalCharacter->GetVelocity().Size() > 0.0f)
		{
			DirectionBeforeStopping = Direction;
		}
		
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White, MovementRotationMessage);
		}*/



		bIsAiming = SurvivalCharacter->GetAimingBool();		



		bHasAWeapon = SurvivalCharacter->GetbHasAWeapon();

		bIsJogging = SurvivalCharacter->GetbIsJoging();
	
		
	}

	
	


}

void USurvivalAnimInstance::NativeInitializeAnimation()
{
	//Inialising SurvivalCharacter by TryGetPawnOwner
	SurvivalCharacter = Cast<ASurvivalCharacter>(TryGetPawnOwner());
}
