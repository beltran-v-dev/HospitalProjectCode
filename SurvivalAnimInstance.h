// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SurvivalAnimInstance.generated.h"





UCLASS()
class HOSPITALPROJECT_API USurvivalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:

	//Handle turning in place variables
	void TurnInPlace();



public:
	USurvivalAnimInstance();



	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float deltaTime);


//Function NativeInitializeAnimation that works like begin play bur for animationBlueprint
	virtual void NativeInitializeAnimation() override;

private:


	//Vatiable to hold the SurvivalCharacter reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess="true"))
	class ASurvivalCharacter* SurvivalCharacter;

	//Speed of the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float speed;

	//To know if the character is accelerating
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	//Direction used to blendspace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Direction;

	//Direction used to blendspace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float DirectionBeforeStopping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bHasAWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bIsJogging;

	//TIPCharacterYaw of the character this frame, only updated when standing still
	float TIPCharacterYaw;

	//TIPCharacterYawLastFrame of the character the previous frame, only updated when standing still
	float TIPCharacterYawLastFrame;

	//Float to keep track of the offset we need to rotate the rootYaw back
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TrunInPlace, meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	//Rotation curve value this frame
	float RotationCurve;

	//Rotation curve value the previous frame
	float RotationCruveLastFrame;

	//Pitch value to manage the AimOffset from UnrealEngineEditor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TrunInPlace, meta = (AllowPrivateAccess = "true"))
	float Pitch;

	//True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;



public:

	
	
};
