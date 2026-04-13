#include "ParkourMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UParkourMovementComponent::Initialize(ACharacter* InOwner, UParkourSettings* InSettings)
{
	CharacterOwner = InOwner;
	Settings = InSettings;
	if (CharacterOwner)
	{
		MoveComp = CharacterOwner->GetCharacterMovement();
		if (MoveComp)
		{
			DefaultGroundFriction = MoveComp->GroundFriction;
			DefaultMaxWalkSpeedCrouched = MoveComp->MaxWalkSpeedCrouched;
		}
	}
}

void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(true);
}

// --- МАШИНА СОСТОЯНИЙ ---
void UParkourMovementComponent::SetState(EParkourState NewState)
{
	if (CurrentState == NewState) return;

	// ВЫХОД из старого состояния
	switch (CurrentState)
	{
	case EParkourState::Sliding:
		if (Settings->SlideMontage) CharacterOwner->StopAnimMontage(Settings->SlideMontage);
		break;
	case EParkourState::Dashing:
		break;
	default: break;
	}

	CurrentState = NewState;

	// ВХОД в новое состояние
	switch (CurrentState)
	{
	case EParkourState::Default:
	case EParkourState::Crouching:
		if (MoveComp) {
			MoveComp->GroundFriction = DefaultGroundFriction;
			MoveComp->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
		}
		break;

	case EParkourState::Sliding:
		break;

	case EParkourState::Dashing:
		break;
	}
}

// --- ЛОГИКА МЕХАНИК ---

void UParkourMovementComponent::RequestDash()
{
	if (!bCanDash || !Settings || CurrentState == EParkourState::Dashing || !MoveComp) return;

	bCanDash = false;
	SetState(EParkourState::Dashing);

	FVector DashDir = MoveComp->GetLastInputVector().GetSafeNormal();
	if (DashDir.IsNearlyZero()) DashDir = CharacterOwner->GetActorForwardVector();

	MoveComp->GroundFriction = 0.0f;
	CharacterOwner->LaunchCharacter(DashDir * Settings->DashForce, true, true);

	CharacterOwner->GetWorldTimerManager().SetTimer(DashDurationTimerHandle, this, &UParkourMovementComponent::StopDashing, Settings->DashDuration);
	CharacterOwner->GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, this, &UParkourMovementComponent::ResetDashCooldown, Settings->DashCooldown);
}

void UParkourMovementComponent::StopDashing()
{
	if (CurrentState != EParkourState::Dashing) return;

	// Если после рывка мы всё еще физически в приседе (зажат Ctrl или мы под кубом)
	if (MoveComp && MoveComp->IsCrouching())
	{
		SetState(EParkourState::Crouching);
	}
	else
	{
		SetState(EParkourState::Default);
	}
}

void UParkourMovementComponent::ResetDashCooldown() { bCanDash = true; }

void UParkourMovementComponent::RequestSlideStart()
{
	if (!Settings || !MoveComp || CurrentState == EParkourState::Dashing) return;

	const float Speed = FVector(CharacterOwner->GetVelocity().X, CharacterOwner->GetVelocity().Y, 0).Size();

	if (MoveComp->IsMovingOnGround() && Speed >= Settings->MinSpeedForSlide)
	{
		MoveComp->GroundFriction = Settings->SlideMinFriction;
		MoveComp->MaxWalkSpeedCrouched = Speed * Settings->SlideSpeedMultiplier;
		if (Settings->SlideMontage) CharacterOwner->PlayAnimMontage(Settings->SlideMontage);
		SetState(EParkourState::Sliding);
	}
	else
	{
		SetState(EParkourState::Crouching);
	}
	MoveComp->bWantsToCrouch = true;
}

void UParkourMovementComponent::RequestSlideStop()
{
	if (MoveComp) MoveComp->bWantsToCrouch = false;
	// Мы не меняем стейт здесь, Tick сам поймет, когда персонаж физически встанет
}

void UParkourMovementComponent::RequestJump()
{
	if (!CharacterOwner || !MoveComp) return;
	CharacterOwner->Jump();
	if (MoveComp->IsFalling() && ExtraJumpsAvailable > 0)
	{
		ExtraJumpsAvailable--;
		CharacterOwner->LaunchCharacter(FVector(0.f, 0.f, MoveComp->JumpZVelocity), false, true);
	}
}

void UParkourMovementComponent::HandleLanded() { ExtraJumpsAvailable = MaxExtraJumps; }
void UParkourMovementComponent::AddExtraJump() { MaxExtraJumps++; ExtraJumpsAvailable = MaxExtraJumps; }

// --- ОБНОВЛЕНИЕ КАЖДЫЙ КАДР (только при Sliding) ---
void UParkourMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == EParkourState::Sliding)
	{
		MoveComp->MaxWalkSpeedCrouched = FMath::FInterpTo(MoveComp->MaxWalkSpeedCrouched, DefaultMaxWalkSpeedCrouched, DeltaTime, Settings->SlideSpeedInterpSpeed);
		MoveComp->GroundFriction = FMath::FInterpTo(MoveComp->GroundFriction, Settings->SlideMaxFriction, DeltaTime, Settings->SlideFrictionInterpSpeed);

		const float Speed = FVector(CharacterOwner->GetVelocity().X, CharacterOwner->GetVelocity().Y, 0).Size();

		// Авто-переход в присед, если замедлились
		if (Speed < DefaultMaxWalkSpeedCrouched + 10.f || !MoveComp->IsMovingOnGround())
		{
			SetState(EParkourState::Crouching);
		}
	}

	// Проверяем автоматический выход ТОЛЬКО если мы скользим или присели
	bool bIsCrouchBasedState = (CurrentState == EParkourState::Sliding || CurrentState == EParkourState::Crouching);

	if (bIsCrouchBasedState && !MoveComp->bWantsToCrouch && !MoveComp->IsCrouching())
	{
		SetState(EParkourState::Default);
	}

	// Дебаг
	if (GEngine) {
		FString StateName = StaticEnum<EParkourState>()->GetNameStringByValue((int64)CurrentState);

		float CurrentSpeed = CharacterOwner->GetVelocity().Size();
		float CurrentFriction = MoveComp->GroundFriction;

		FString DebugMessage = FString::Printf(
			TEXT("ARCH: Component-Based | STATE: %s | Speed: %.0f | Friction: %.2f"),
			*StateName,
			CurrentSpeed,
			CurrentFriction
		);

		// Используем ID 1, чтобы строка обновлялась, а не спамилась списком
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugMessage);
	}
}