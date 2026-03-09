// ParkourSettings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ParkourSettings.generated.h"

class UAnimMontage; // Предварительное объявление (Forward declaration)

UCLASS()
class RUN_GUN_API UParkourSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- НАСТРОЙКИ РЫВКА ---
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashForce = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashCooldown = 0.7f;

	// --- НАСТРОЙКИ СКОЛЬЖЕНИЯ ---
	UPROPERTY(EditAnywhere, Category = "Sliding")
	float MinSpeedForSlide = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideMinFriction = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideMaxFriction = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideSpeedInterpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	float SlideFrictionInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Sliding")
	UAnimMontage* SlideMontage;
};