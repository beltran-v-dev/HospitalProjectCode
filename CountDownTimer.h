// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CountDownTimer.generated.h"

UCLASS()
class HOSPITALPROJECT_API ACountDownTimer : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	void CountDown();

	

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Count Time", meta = (AllowPrivateaccess = "true"))
	int32 Minutes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Count Time", meta = (AllowPrivateaccess = "true"))
	int32 Seconds;

	FTimerHandle TH_CountDown;
	

public:
	ACountDownTimer();

	UFUNCTION(BlueprintCallable)
	void CallCountDown();

	UFUNCTION(BlueprintCallable)
	int32 GetMinutes()
	{
		return Minutes;
	}

	UFUNCTION(BlueprintCallable)
	int32 GetSeconds()
	{
		return Seconds;
	}

};
