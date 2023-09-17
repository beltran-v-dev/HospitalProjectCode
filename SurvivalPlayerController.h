// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SurvivalPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HOSPITALPROJECT_API ASurvivalPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void  BeginPlay() override;
	
public:

	ASurvivalPlayerController();

private:

//Reference to the Overall HUD Overlay blueprint class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

//Variable to hold the HUD Overlay widget after creating it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;

	
};
