// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"




// Sets default values
AEnemy::AEnemy() :
	Health(100.0f),
	MaxHealth(100.0f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);


	
}

void AEnemy::Die()
{
	//TODO: Die section

}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (NiagaraSystemImpact)
	{
	//	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), NImpactParticle, HitResult.Location, FRotator(0.f), true);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystemImpact, HitResult.Location, FRotator(0.f), FVector(1.f), true, true, ENCPoolMethod::None, true);

		
	}


}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauseer)
{
	if (Health - DamageAmount <= 0.0f)
	{
		Health = 0.0f;
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;



}

