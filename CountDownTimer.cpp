// Fill out your copyright notice in the Description page of Project Settings.


#include "CountDownTimer.h"

// Sets default values
ACountDownTimer::ACountDownTimer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Default countdown values
	Minutes = 1;
	Seconds = 60;

}



void ACountDownTimer::BeginPlay()
{
	Super::BeginPlay();	
}

void ACountDownTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACountDownTimer::CountDown()
{
	// Decrement seconds and minutes accordingly
	if (Seconds > 0)
	{
		Seconds = Seconds - 1;
	}
	else
	{
		if (Minutes <= 0)
		{
			Minutes = 0;
			Seconds = 0;
			GetWorldTimerManager().ClearTimer(TH_CountDown);
		}
		else
		{
			Minutes = Minutes - 1;
			Seconds = 60;
		}
	}


}

void ACountDownTimer::CallCountDown()
{

	// Set up a timer to call the CountDown function every second
	GetWorldTimerManager().SetTimer(
		TH_CountDown,
		this,
		&ACountDownTimer::CountDown,
		1.0f,
		true,
		0.0f);


}


