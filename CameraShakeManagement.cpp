// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShakeManagement.h"

// CameraShakeManagement constructor
UCameraShakeManagement::UCameraShakeManagement()
{
	// Set default values for camera shake parameters
	OscillationDuration = 0.125f;
	OscillationBlendInTime = 0.025f;
	OscillationBlendOutTime = 0.025f;

	// Randomize rotational oscillation parameters within specified ranges
	RotOscillation.Pitch.Amplitude = FMath::RandRange(0.625f, 1.25f);
	RotOscillation.Pitch.Frequency = FMath::RandRange(3.125f, 4.375f);
	RotOscillation.Yaw.Amplitude = FMath::RandRange(0.625f, 1.25f);
	RotOscillation.Yaw.Frequency = FMath::RandRange(3.125f, 4.375f);


}
