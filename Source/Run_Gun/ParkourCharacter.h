// ParkourCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "ParkourCharacter.generated.h"

UCLASS()
class RUN_GUN_API AParkourCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AParkourCharacter();

	// --- йнлонмемр йюлепш ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCameraComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// --- оепелеммше дкъ яхярелш ббндю ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AddJumpAction;

	// --- оепелеммше дкъ пшбйю ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashCooldown = 0.7f;

	// --- оепелеммше дкъ опшфйю ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
	int32 MaxExtraJumps = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Jumping")
	int32 ExtraJumpsAvailable = 0;

	// --- оепелеммше дкъ яйнкэфемхъ ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float MinSpeedForSlide = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float SlideSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float SlideMinFriction = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float SlideMaxFriction = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float SlideSpeedInterpSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
	float SlideFrictionInterpSpeed = 5.0f;

	// --- оепелеммше дкъ йюлепш ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector StandingCameraLocation = FVector(0.f, 0.f, 64.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector CrouchingCameraLocation = FVector(0.f, 0.f, 30.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraInterpSpeed = 10.0f;

	// --- тсмйжхх леуюмхй ---
	void Dash();
	void ResetDash();

	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	void AddExtraJump();

	void StartSlide();
	void StopSlide();

private:
	bool bCanDash = true;
	bool bIsSliding = false;
	bool bWantsToCrouch = false; // <-- мнбюъ оепелеммюъ

	float DefaultGroundFriction;
	float DefaultMaxWalkSpeedCrouched;

	FTimerHandle DashCooldownTimerHandle;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};