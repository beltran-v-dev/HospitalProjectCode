// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"


UCLASS()
class HOSPITALPROJECT_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	void ResetHitReactTimer();

	//Called when something overlaps with the agro sphere
	/*UFUNCTION()
		void AgroSphereOverlap(
			UPrimitiveComponent* OverlapComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);*/

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	UFUNCTION()
	void CombatRangeOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);


	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION()
	void OnLeftHandAttackOverlap(UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnRightHandAttackOverlap(UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	//Activate/Disable collision for left hand attack (boxes)
	UFUNCTION(BlueprintCallable)
	void ActivateLeftHandAttack();
	UFUNCTION(BlueprintCallable)
	void DisableLeftHandAttack();

	//Activate/Disable collision for right hand attack (boxes)
	UFUNCTION(BlueprintCallable)
	void ActivateRightHandAttack();
	UFUNCTION(BlueprintCallable)
	void DisableRightHandAttack();

	void DoDamage(class ASurvivalCharacter* MainCharacter);

	void SpawnBlood(ASurvivalCharacter* MainCharacter, FName SocketName);

	//Attemp to stun character
	void StunCharacter(ASurvivalCharacter* MainCharacter); 

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UFUNCTION()
	void DestroyEnemy();

	bool InTargetRange(AActor* Target, double Radious);


private:
	////NiagaraSystem to spawn when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* NiagaraSystemImpact;

	//Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	//Current health of the enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;

	//Maximum health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;
	
	//Name of the head bonde
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBone;

	//Montage which contains Hit and Death animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;


	FTimerHandle HitReactTimer;

	bool bCanHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	//Behavior tree for the AI Character
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	//Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	//B Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPointB;

	class AEnemyController* EnemyController;

	//OverlapSphere for when the enemy becomes hostile
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;*/

	//True when playing the get hit animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	//Chance of being stunned; 0 mean no stun chance, 1 means 100% stun chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float StunChance;

	//True when in attack range; time to attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	//Sphere for attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CombatRangeSphere;

	//Montage which contains attack animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	//The four attack montage names
	FName AttackL02;
	FName AttackR02;

	//Collision volume for the left hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftHandCollision;
	
	//Collision volume for the right hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	 UBoxComponent* RightHandCollision;

	//Base damage for enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* EnemySoundByProximity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "combat", meta = (AllowPrivateAccess = "true"))
	FName LeftEfectSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "combat", meta = (AllowPrivateAccess = "true"))
	FName RightEfectSocket;

	//True when Enemey can attack
	UPROPERTY(VisibleAnywhere, Category = "combat", meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;

	FTimerHandle AttackWaitTimer;

	//Minumun wait time between attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;

	//Death anim montage for the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	FTimerHandle DeathTimer;

	//Time after death until the enemy is destoryed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float DeathTime;

	//sound that will play when the enemy is dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	USoundCue* DeadSound;

	//Navigation

	//UPROPERTY()
	//class AAIController* EnemyControllerB;

	////Current patrol target
	//UPROPERTY(EditInstanceOnly, Category = "AI Navegation", meta = (AllowPrivateAccess = "true"))
	//AActor* PatrolTarget;

	//UPROPERTY(EditInstanceOnly, Category = "AI Navegation", meta = (AllowPrivateAccess = "true"))
	//TArray<AActor*> PatrolTargets;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navegation", meta = (AllowPrivateAccess = "true"))
	//double PatrolRadious;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauseer) override;


	FORCEINLINE	FString GetHeadBoneName()
	{
		return HeadBone;
	}

	FORCEINLINE UBehaviorTree* GetBehaviorTree()
	{
		return BehaviorTree;
	}
};
