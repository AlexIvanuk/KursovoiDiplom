// ParkourMovementComponent.cpp
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

void UParkourMovementComponent::Dash()
{
	if (!bCanDash || !Settings || CurrentState != EParkourState::Default || !MoveComp) return;

	bCanDash = false;
	CurrentState = EParkourState::Dashing;

	FVector DashDirection = MoveComp->GetLastInputVector().GetSafeNormal();
	if (DashDirection.IsNearlyZero()) DashDirection = CharacterOwner->GetActorForwardVector();

	MoveComp->GroundFriction = 0.0f;
	CharacterOwner->LaunchCharacter(DashDirection * Settings->DashForce, true, true);

	CharacterOwner->GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, this, &UParkourMovementComponent::ResetDash, Settings->DashCooldown);
}

void UParkourMovementComponent::ResetDash()
{
	bCanDash = true;
	if (CurrentState == EParkourState::Dashing) CurrentState = EParkourState::Default;
	if (MoveComp) MoveComp->GroundFriction = DefaultGroundFriction;
}

void UParkourMovementComponent::StartSlide()
{
	if (!Settings || CurrentState != EParkourState::Default || !MoveComp) return;

	const float GroundSpeed = FVector(CharacterOwner->GetVelocity().X, CharacterOwner->GetVelocity().Y, 0).Size();

	if (MoveComp->IsMovingOnGround() && GroundSpeed >= Settings->MinSpeedForSlide)
	{
		CurrentState = EParkourState::Sliding;
		MoveComp->GroundFriction = Settings->SlideMinFriction;
		MoveComp->MaxWalkSpeedCrouched = GroundSpeed * Settings->SlideSpeedMultiplier;
		if (Settings->SlideMontage) CharacterOwner->PlayAnimMontage(Settings->SlideMontage);
	}
	else
	{
		CurrentState = EParkourState::Crouching;
	}
	CharacterOwner->Crouch();
}

void UParkourMovementComponent::StopSlide()
{
	if (CurrentState == EParkourState::Sliding || CurrentState == EParkourState::Crouching)
	{
		CurrentState = EParkourState::Default;
		if (Settings && Settings->SlideMontage) CharacterOwner->StopAnimMontage(Settings->SlideMontage);
		if (MoveComp)
		{
			MoveComp->GroundFriction = DefaultGroundFriction;
			MoveComp->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
		}
		CharacterOwner->UnCrouch();
	}
}

void UParkourMovementComponent::RequestJump()
{
	if (!CharacterOwner || !MoveComp) return;

	CharacterOwner->Jump(); // Обычный прыжок

	if (MoveComp->IsFalling() && ExtraJumpsAvailable > 0)
	{
		ExtraJumpsAvailable--;
		CharacterOwner->LaunchCharacter(FVector(0.f, 0.f, MoveComp->JumpZVelocity), false, true);
	}
}

void UParkourMovementComponent::HandleLanded()
{
	ExtraJumpsAvailable = MaxExtraJumps;
}

void UParkourMovementComponent::AddExtraJump()
{
	MaxExtraJumps++;
	ExtraJumpsAvailable = MaxExtraJumps;
}

void UParkourMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == EParkourState::Sliding && MoveComp && Settings)
	{
		MoveComp->MaxWalkSpeedCrouched = FMath::FInterpTo(MoveComp->MaxWalkSpeedCrouched, DefaultMaxWalkSpeedCrouched, DeltaTime, Settings->SlideSpeedInterpSpeed);
		MoveComp->GroundFriction = FMath::FInterpTo(MoveComp->GroundFriction, Settings->SlideMaxFriction, DeltaTime, Settings->SlideFrictionInterpSpeed);

		const float GroundSpeed = FVector(CharacterOwner->GetVelocity().X, CharacterOwner->GetVelocity().Y, 0).Size();
		if (GroundSpeed < DefaultMaxWalkSpeedCrouched + 50.f || !MoveComp->IsMovingOnGround())
		{
			StopSlide();
		}
	}
	if (GEngine && CharacterOwner)
	{
		FString StateString;
		// Превращаем Enum в текст для наглядности
		switch (CurrentState)
		{
		case EParkourState::Default:   StateString = "Default"; break;
		case EParkourState::Crouching: StateString = "Crouching"; break;
		case EParkourState::Sliding:   StateString = "Sliding"; break;
		case EParkourState::Dashing:   StateString = "Dashing"; break;
		}

		const FString DebugMessage = FString::Printf(
			TEXT("COMPONENT STATE: %s | Speed: %.0f | Friction: %.2f"),
			*StateString,
			CharacterOwner->GetVelocity().Size(),
			MoveComp ? MoveComp->GroundFriction : 0.0f
		);

		// Выводим на экран (ключ -1 значит добавлять новую строку, 
		// но мы поставим конкретный ID, например 1, чтобы строка обновлялась, а не спамила)
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugMessage);
	}
}