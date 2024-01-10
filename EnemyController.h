// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISense_Sight.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class HOSPITALPROJECT_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();

	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTargetPerceptionUpdate(AActor* actor,  FAIStimulus stimulus);

	

private:
	//Blackboard component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	//Behaviour tree component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviourTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* SightConfig;  // Sight sense configuration

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Hearing* HearingConfig;

	FAISenseID sightid;
	FAISenseID hearid;

public:

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent()
	{
		return BlackboardComponent;
	}

	FORCEINLINE UBehaviorTreeComponent* GetBehaviourTreeComponent()
	{
		return BehaviourTreeComponent;
	}
	
};
