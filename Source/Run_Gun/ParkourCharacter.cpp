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
	FirstPersonCameraComponent->SetupAttachment(GetMesh());
	FirstPersonCameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("CameraSocket"));
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
	// --- НОВАЯ, УЛУЧШЕННАЯ ЛОГИКА ---
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp) return;

	const float GroundSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();

	// Проверяем условия для скольжения ПРЯМО СЕЙЧАС, в момент нажатия
	if (MovementComp->IsMovingOnGround() && GroundSpeed >= MinSpeedForSlide)
	{
		bIsSliding = true; // Сразу взводим флаг скольжения

		// Применяем параметры скольжения немедленно
		MovementComp->GroundFriction = SlideMinFriction;
		MovementComp->MaxWalkSpeedCrouched = GroundSpeed * SlideSpeedMultiplier;
		if (SlideMontage)
		{
			PlayAnimMontage(SlideMontage);
		}
	}

	// И в любом случае, сообщаем движку о нашем желании присесть.
	// Если условия для скольжения не выполнились, это будет простое приседание.
	MovementComp->bWantsToCrouch = true;
}

void AParkourCharacter::StopSlide()
{
	// А эта - о "желании" встать
	GetCharacterMovement()->bWantsToCrouch = false;
	if (bIsSliding)
	{
		StopAnimMontage(SlideMontage);
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
		// Плавное замедление во время скольжения
		MovementComp->MaxWalkSpeedCrouched = FMath::FInterpTo(MovementComp->MaxWalkSpeedCrouched, DefaultMaxWalkSpeedCrouched, DeltaTime, SlideSpeedInterpSpeed);
		MovementComp->GroundFriction = FMath::FInterpTo(MovementComp->GroundFriction, SlideMaxFriction, DeltaTime, SlideFrictionInterpSpeed);

		const float GroundSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();
		// Условия для прекращения скольжения: скорость упала или мы больше не на земле
		if (GroundSpeed < DefaultMaxWalkSpeedCrouched + 50.f || !MovementComp->IsMovingOnGround())
		{
			bIsSliding = false;
		}
	}

	// Если мы НЕ скользим (просто присели или скольжение закончилось), но все еще сидим
	if (!bIsSliding && MovementComp->IsCrouching())
	{
		// Возвращаем стандартные параметры приседания
		MovementComp->GroundFriction = DefaultGroundFriction;
		MovementComp->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
	}

	// Если мы скользили, но теперь встали (UnCrouch сработал)
	if (bIsSliding && !MovementComp->IsCrouching())
	{
		bIsSliding = false;
		MovementComp->GroundFriction = DefaultGroundFriction;
		MovementComp->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
	}

	// --- ОТЛАДКА ---
	if (GEngine)
	{
		const FString DebugMessage = FString::Printf(
			TEXT("Speed: %.0f | IsSliding: %s | WantsToCrouch: %s | WantsToCrouch(in): %s | IsCrouching: %s"),
			FVector(GetVelocity().X, GetVelocity().Y, 0).Size(),
			bIsSliding ? TEXT("true") : TEXT("false"),
			bWantsToCrouch ? TEXT("true") : TEXT("false"),
			MovementComp->bWantsToCrouch ? TEXT("true") : TEXT("false"),
			MovementComp->IsCrouching() ? TEXT("true") : TEXT("false")
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

bool AParkourCharacter::IsSliding() const
{
	return bIsSliding;
}