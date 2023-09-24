// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalAnimInstance.h"
#include "SurvivalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"



USurvivalAnimInstance::USurvivalAnimInstance()
{

	speed = 0.0f;
	bIsAccelerating = false;
	Direction = 0.0f;
	DirectionBeforeStopping = 0.0f;
	bIsAiming = false;
	bHasAWeapon = false;
	bIsJogging = false;
	TIPCharacterYaw = 0.0f;
	TIPCharacterYawLastFrame = 0.0f;
	RootYawOffset = 0.0f;


}

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
	
		bIsCrouching = SurvivalCharacter->GetCrouching();
	}

	
	
	TurnInPlace();


}

void USurvivalAnimInstance::NativeInitializeAnimation()
{
	//Inialising SurvivalCharacter by TryGetPawnOwner
	SurvivalCharacter = Cast<ASurvivalCharacter>(TryGetPawnOwner());
}

void USurvivalAnimInstance::TurnInPlace()
{
	if (SurvivalCharacter == nullptr) return;

	Pitch = SurvivalCharacter->GetBaseAimRotation().Pitch;

	if (speed > 0)
	{
		//DOn't want to tun in place, Character is moving
		RootYawOffset = 0.0f;
		TIPCharacterYaw = SurvivalCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCruveLastFrame = 0.0f;
		RotationCurve = 0.0f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = SurvivalCharacter->GetActorRotation().Yaw;

		//Difference betwwen CharacterYawLastFrame and CharacterYaw this frame
		const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

		//Rot Yaw Offset, updated and clamped to [-180, 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		//1.0 if turning, 0.0 if not
		const float Turning = GetCurveValue(TEXT("Turning"));
		if (Turning > 0)
		{
			RotationCurve = RotationCruveLastFrame;
			RotationCurve = GetCurveValue(TEXT("Turning"));
			const float DeltaRotation = RotationCurve - RotationCruveLastFrame;

			//If RootYawOffset >0 we are turning left, if it's <0 (negative values) we are turning right 
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float AbsRootYawOffset = FMath::Abs(RootYawOffset);
			if (AbsRootYawOffset > 45.0f)
			{
				const float YawExess = AbsRootYawOffset - 45.0f;

				RootYawOffset > 0 ? RootYawOffset -= YawExess : RootYawOffset += YawExess;

			}


		}
	

	}

}




