// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"
#include "Enemy.h"

void UZombieAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}

	if (Enemy)
	{
		FVector Velocity = Enemy->GetVelocity();
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();
	}



}
