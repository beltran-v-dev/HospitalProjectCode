// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "SurvivalCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/SkeletalMeshSocket.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"



// Enemy constructor
AEnemy::AEnemy() :
	Health(100.0f),
	MaxHealth(100.0f),
	bCanHitReact(true),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(3.0f),
	bStunned(false),
	StunChance(0.5f),
	AttackL02(TEXT("AttackL02")),
	AttackR02(TEXT("AttackR02")),
	BaseDamage(25.0f),
	LeftEfectSocket("FX_Blood_Efect_L"),
	RightEfectSocket("FX_Blood_Efect_R"),
	bCanAttack(true),
	AttackWaitTime(1.0f),
	bDying(false),
	DeathTime(5.0f)
	
{
	// Set character movement settings
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	

	//Create the combat range sphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());


	//Create left and right weapon collision boxes
	LeftHandCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHandAttack"));
	LeftHandCollision->SetupAttachment(GetMesh(), FName("Socket_LeftHand"));
	RightHandCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandAttack"));
	RightHandCollision->SetupAttachment(GetMesh(), FName("Socket_RightHand"));

	// Create the sound component for proximity-based sounds
	EnemySoundByProximity = CreateDefaultSubobject<UAudioComponent>(TEXT("EnemySoundByProximity"));
	EnemySoundByProximity->SetupAttachment(GetRootComponent());




}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();


	// Bind overlap events for agro and combat range
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);

	//Bind functions to overalp events fot hand boxes
	LeftHandCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftHandAttackOverlap);
	RightHandCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightHandAttackOverlap);
	
	//Set collision preset for hand boxes
	LeftHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftHandCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftHandCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftHandCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightHandCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightHandCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	// Set collision responses for mesh and capsule
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	//Get the AI Controller
	EnemyController = Cast<AEnemyController>(GetController());

	// Set initial values in the blackboard for patrol points and behavior tree
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}

	FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	FVector WorldPatrolPointB = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPointB);


	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPointB"), WorldPatrolPointB);

		EnemyController->RunBehaviorTree(BehaviorTree);

	}


}

// Handle the death of the enemy
void AEnemy::Die()
{
	
	if (bDying) return;
	bDying = true;

	// Play death animation montage
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && DeathMontage)
	{
	
		animInstance->Montage_Play(DeathMontage);


		// Update blackboard values and stop movement if there is an AI controller
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("isEnemyDead"), true);
			EnemyController->StopMovement();


		}
	
	}

}

// Play hit montage with the specified section and play rate
void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	// Play hit montage if hit reactions are allowed
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
	}

	// Prevent immediate hit reactions and set a timer to reset hit react
	bCanHitReact = false;
	const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);

}

// Reset hit reaction timer to enable hit reactions again
void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}



// Set the stunned state and update Blackboard if there is a valid EnemyController
void AEnemy::SetStunned(bool Stunned)
{

	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);
	}

}

// Handle the overlap event for the combat range
void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping actor is a SurvivalCharacter
	if (OtherActor == nullptr) return;
	auto Character = Cast<ASurvivalCharacter>(OtherActor);
	if (Character)
	{
		// Update the InAttackRange status and Blackboard if there is a valid EnemyController
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool("InAttackRange", bInAttackRange);
		}
	}



}

// Handle the end of overlap event for the combat range
void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the overlapping actor is a SurvivalCharacter
	if (OtherActor == nullptr) return;
	auto Character = Cast<ASurvivalCharacter>(OtherActor);
	if (Character)
	{
		// Update the InAttackRange status and Blackboard if there is a valid EnemyController
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool("InAttackRange", bInAttackRange);
		}
	}


}

// Play the attack montage with the specified section and play rate
void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	// Get the animation instance
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// Check if there is a valid animation instance and AttackMontage
	if (AnimInstance && AttackMontage)
	{
		// Play the AttackMontage and jump to the specified section
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}

	// Disable further attacks and set a timer to reset it
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(
		AttackWaitTimer,
		this,
		&AEnemy::ResetCanAttack,
		AttackWaitTime);
	
	// Update Blackboard if there is a valid EnemyController
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}

}

// Get a random attack section name
FName AEnemy::GetAttackSectionName()
{
	// Initialize the SectionName variable
	FName SectionName;

	// Generate a random number to determine the attack section
	int32 Section = FMath::RandRange(1, 2);

	// Assign the appropriate attack section name based on the random number
	switch (Section)
	{
		case 1:
			SectionName = AttackL02;
			
		break;

		case 2:
			SectionName = AttackR02;

		break;

	}

	// Return the selected SectionName
	return SectionName;
}

// Handle overlap events for left hand attack
void AEnemy::OnLeftHandAttackOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapped actor is a SurvivalCharacter
	auto Character = Cast<ASurvivalCharacter>(OtherActor);
	if (Character)
	{
		// Perform actions on the character, such as dealing damage, spawning blood effects, and stunning
		DoDamage(Character);
		SpawnBlood(Character, LeftEfectSocket);
		StunCharacter(Character);
	}

}

// Handle overlap events for right hand attack
void AEnemy::OnRightHandAttackOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapped actor is a SurvivalCharacter
	auto Character = Cast<ASurvivalCharacter>(OtherActor);
	if (Character)
	{
		// Perform actions on the character, such as dealing damage, spawning blood effects, and stunning
		DoDamage(Character);
		SpawnBlood(Character, RightEfectSocket);
		StunCharacter(Character);

	}
	
}

// Applies damage to the character
void AEnemy::DoDamage(ASurvivalCharacter* MainCharacter)
{
	// Ensure the MainCharacter is valid
	if (MainCharacter == nullptr) return;
	
	// Apply damage to the MainCharacter using ApplyDamage function
		UGameplayStatics::ApplyDamage(MainCharacter, BaseDamage, EnemyController, this, UDamageType::StaticClass());

		// Play melee impact sound if available
		if (MainCharacter->GetMelleImpactSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, MainCharacter->GetMelleImpactSound(), GetActorLocation());
		}

	

}


// Stuns the MainCharacter based on a random chance and the StunChance property of the enemy.
void AEnemy::StunCharacter(ASurvivalCharacter* MainCharacter)
{
	// Ensure MainCharacter is valid
	if (MainCharacter)
	{
		// Generate a random value between 0 and 1
		float Stun = FMath::FRandRange(0.0f, 1.0f);

		// Check if the random value is less than or equal to the StunChance of the MainCharacter
		if (Stun <= MainCharacter->GetStunChance())
		{
			// If true, stun the MainCharacter
			MainCharacter->Stun();
		}
	}


}

// Resets the ability to attack after a specified wait time.
void AEnemy::ResetCanAttack()
{
	// Set bCanAttack to true after the specified wait time
	bCanAttack = true;

	// Update the Blackboard value if the EnemyController is valid
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}

}

// Initiates the final steps after the enemy's death, such as pausing animations and setting a timer for destruction.
void AEnemy::FinishDeath()
{

	// Pause animations
	GetMesh()->bPauseAnims = true;

	// Disable collision for the capsule component
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set a timer to call the DestroyEnemy function after the specified DeathTime
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);

}

// Destroys the enemy actor.
void AEnemy::DestroyEnemy()
{
	Destroy();

}


// Checks if the given target is within the specified radius from the enemy.
bool AEnemy::InTargetRange(AActor* Target, double Radious)
{
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	
	return DistanceToTarget <= Radious;
}


// Activates the collision for the left hand attack.
void AEnemy::ActivateLeftHandAttack()
{
	LeftHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

// Disables the collision for the left hand attack.
void AEnemy::DisableLeftHandAttack()
{
	LeftHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}


// Activates the collision for the right hand attack.
void AEnemy::ActivateRightHandAttack()
{

	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

// Disables the collision for the right hand attack.
void AEnemy::DisableRightHandAttack()
{
	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);


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


// Called when the enemy is hit by a bullet.
void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	// Play impact sound if available.
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	// Spawn impact particle system if available.
	if (NiagaraSystemImpact)
	{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystemImpact, HitResult.Location, FRotator(0.f), FVector(1.f), true, true, ENCPoolMethod::None, true);

		
	}

}


// Called when the enemy takes damage.
float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauseer)
{
	//Set the target blackboard key to agro the character
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauseer);
	}

	// Check if the enemy is already dead.
	if (Health - DamageAmount <= 0.0f)
	{
		Health = 0.0f;

		// Stop any proximity sound.
		EnemySoundByProximity->Stop();

		// Play dead sound if available.
		if (DeadSound)
		{
			UGameplayStatics::PlaySound2D(this, DeadSound);
		}


		// Initiate the death sequence.
		Die();
	}
	else
	{
		// Reduce health by the damage amount.
		Health -= DamageAmount;
	}


	
	if (bDying) return DamageAmount;

	// Determine if the bullet hit causes stun.
	float Stunned = FMath::FRandRange(0.0f, 1.0f);
	if (Stunned <= StunChance)
	{
		//Stun the enemy
		PlayHitMontage(FName("HitReactBack"));
		SetStunned(true);

	}


	return DamageAmount;



}

