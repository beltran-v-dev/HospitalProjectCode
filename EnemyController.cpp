// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "SurvivalCharacter.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Enemy.h"



AEnemyController::AEnemyController()
{

	// Create and initialize BlackboardComponent
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	check(BlackboardComponent);

	// Create and initialize BehaviorTreeComponent
	BehaviourTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviourTreeComponent"));
	check(BehaviourTreeComponent);

	// Create and initialize AIPerceptionComponent
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	// Create Sight sense configuration
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));  

	// Create Hearing sense configurationç
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	AIPerceptionComponent->Activate();

	// Set the sight sense properties
	SightConfig->SightRadius = 600.0f;
	SightConfig->LoseSightRadius = 600.0f;
	SightConfig->PeripheralVisionAngleDegrees = 80.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//Set the hearing sense properties:
	HearingConfig->HearingRange = 600.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;


	// Configure the perception component with the sight sense
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComponent->ConfigureSense(*SightConfig);

	//Configure the perception component wowth the hearing sense 
	AIPerceptionComponent->ConfigureSense(*HearingConfig);

	// Get the ID of each sense for later reference
	 sightid = SightConfig->GetSenseID();
	 hearid = HearingConfig->GetSenseID();


}

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();

	// Bind the function to handle updates in target perception
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetPerceptionUpdate);

	

}


// Handle updates in target perception
void AEnemyController::OnTargetPerceptionUpdate(AActor* actor, FAIStimulus stimulus)
{

	// Cast the detected actor to a SurvivalCharacter
	auto CharacterB = Cast<ASurvivalCharacter>(actor);


	// Check if the stimulus is related to the Sight sense
	if (stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// Process only if the detected actor is a SurvivalCharacter
		if (CharacterB)
		{

			// Check if the character was successfully sensed
			if (stimulus.WasSuccessfullySensed())
			{

				// Update Blackboard values when the player is detected
				BlackboardComponent->SetValueAsBool("PlayJumpscare", true);
				BlackboardComponent->SetValueAsObject(FName("Target"), CharacterB);
				BlackboardComponent->SetValueAsBool(FName("CanSeePlayer"), true);
				BlackboardComponent->SetValueAsBool(FName("isPersuingPlayer"), true);
			}

			else
			{
				// Reset Blackboard values when the player is no longer detected
				BlackboardComponent->SetValueAsBool("PlayJumpscare", false);
				BlackboardComponent->SetValueAsObject(FName("Target"), nullptr);
				BlackboardComponent->SetValueAsBool(FName("CanSeePlayer"), false);
				




			}

		}
	}


	// Check if the stimulus is related to the Hearing sense
	if (stimulus.Type == HearingConfig->GetSenseID())
	{
		// Process only if the detected actor is a SurvivalCharacter
		if (CharacterB)
		{

			// Update Blackboard value with the location of the perceived noise
			BlackboardComponent->SetValueAsVector("NoiseLocation", stimulus.StimulusLocation);

		}

	}
}





// Called when the controller possesses a pawn.
void AEnemyController::OnPossess(APawn* InPawn)
{
	// Call the parent class implementation
	Super::OnPossess(InPawn);


	// Ensure the pawn is valid
	if (InPawn == nullptr) return;


	// Attempt to cast the possessed pawn to an AEnemy
	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy)
	{
		// Check if the enemy has a valid behavior tree
		if (Enemy->GetBehaviorTree())
		{

			// Initialize the Blackboard with the enemy's behavior tree Blackboard asset
			BlackboardComponent->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));

			


		}
	}

}




