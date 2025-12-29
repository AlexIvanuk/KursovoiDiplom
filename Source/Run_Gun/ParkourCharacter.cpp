// ParkourCharacter.cpp

#include "ParkourCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

AParkourCharacter::AParkourCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
		DefaultMaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeedCrouched;
	}
}

// --- Логика Рывка (Dash) ---

void AParkourCharacter::Dash()
{
	if (!bCanDash) return;

	bCanDash = false;
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp) return;

	FVector DashDirection = MovementComp->GetLastInputVector().GetSafeNormal();
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = GetActorForwardVector();
	}

	MovementComp->GroundFriction = 0.0f;
	LaunchCharacter(DashDirection * DashForce, true, true);

	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimerHandle, this, &AParkourCharacter::ResetDash, DashCooldown);
}

void AParkourCharacter::ResetDash()
{
	bCanDash = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	}
}

// --- Логика Прыжка (Jump) ---

void AParkourCharacter::Jump()
{
	Super::Jump();
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling() && ExtraJumpsAvailable > 0)
	{
		ExtraJumpsAvailable--;
		LaunchCharacter(FVector(0.f, 0.f, GetCharacterMovement()->JumpZVelocity), false, true);
	}
}

void AParkourCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	ExtraJumpsAvailable = MaxExtraJumps;
}

void AParkourCharacter::AddExtraJump()
{
	MaxExtraJumps++;
	ExtraJumpsAvailable = MaxExtraJumps;
}

// --- Логика Скольжения (Slide) ---

void AParkourCharacter::StartSlide()
{
	const float GroundSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();
	if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround() && GroundSpeed >= MinSpeedForSlide)
	{
		bIsSliding = true;
		Crouch();
		GetCharacterMovement()->GroundFriction = SlideMinFriction;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GroundSpeed * SlideSpeedMultiplier;
	}
	else
	{
		Crouch();
	}
}

void AParkourCharacter::StopSlide()
{
	bIsSliding = false;
	UnCrouch();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
		GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
	}
}

// --- Логика, выполняющаяся каждый кадр (Tick) ---

void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp) return;

	if (bIsSliding)
	{
		MovementComp->MaxWalkSpeedCrouched = FMath::FInterpTo(
			MovementComp->MaxWalkSpeedCrouched,
			DefaultMaxWalkSpeedCrouched,
			DeltaTime,
			SlideSpeedInterpSpeed
		);
		MovementComp->GroundFriction = FMath::FInterpTo(
			MovementComp->GroundFriction,
			SlideMaxFriction,
			DeltaTime,
			SlideFrictionInterpSpeed
		);

		const float GroundSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();
		if (GroundSpeed < DefaultMaxWalkSpeedCrouched + 50.f)
		{
			bIsSliding = false;
		}
	}
	else if (!bIsSliding && MovementComp->IsCrouching())
	{
		MovementComp->GroundFriction = DefaultGroundFriction;
		MovementComp->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
	}

	// --- ЛОГИКА ПЕРЕМЕЩЕНИЯ КАМЕРЫ ---
	if (FirstPersonCameraComponent)
	{
		const FVector TargetLocation = MovementComp->IsCrouching() ? CrouchingCameraLocation : StandingCameraLocation;
		const FVector NewLocation = FMath::VInterpTo(
			FirstPersonCameraComponent->GetRelativeLocation(),
			TargetLocation,
			DeltaTime,
			CameraInterpSpeed
		);
		FirstPersonCameraComponent->SetRelativeLocation(NewLocation);
	}

	// --- ОТЛАДКА ---
	if (GEngine)
	{
		const FString DebugMessage = FString::Printf(
			TEXT("Speed: %.0f | IsSliding: %s | Friction: %.2f | MaxCrouchSpeed: %.0f"),
			FVector(GetVelocity().X, GetVelocity().Y, 0).Size(),
			bIsSliding ? TEXT("true") : TEXT("false"),
			MovementComp->GroundFriction,
			MovementComp->MaxWalkSpeedCrouched
		);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, DebugMessage);
	}
}

// --- Привязка Ввода (Input) ---

void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AParkourCharacter::Dash);
		}
		if (AddJumpAction)
		{
			EnhancedInputComponent->BindAction(AddJumpAction, ETriggerEvent::Started, this, &AParkourCharacter::AddExtraJump);
		}
		if (SlideAction)
		{
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AParkourCharacter::StartSlide);
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopSlide);
		}
	}
}