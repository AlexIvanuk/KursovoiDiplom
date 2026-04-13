#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourSettings.h"
#include "ParkourMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EParkourState : uint8
{
	Default,
	Crouching,
	Sliding,
	Dashing
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RUN_GUN_API UParkourMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParkourMovementComponent();

	// Связь с внешним миром
	void Initialize(class ACharacter* InOwner, UParkourSettings* InSettings);

	// Команды от игрока
	void RequestDash();
	void RequestSlideStart();
	void RequestSlideStop();
	void RequestJump();

	// События от движка
	void HandleLanded();
	void AddExtraJump();

	// Геттер для АнимБП
	UFUNCTION(BlueprintPure, Category = "Parkour")
	bool IsSliding() const { return CurrentState == EParkourState::Sliding; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Централизованная смена состояний
	void SetState(EParkourState NewState);

	// Ссылки
	UPROPERTY() class ACharacter* CharacterOwner;
	UPROPERTY() class UCharacterMovementComponent* MoveComp;
	UPROPERTY() UParkourSettings* Settings;

	// Внутренние переменные
	EParkourState CurrentState = EParkourState::Default;
	int32 MaxExtraJumps = 0;
	int32 ExtraJumpsAvailable = 0;
	bool bCanDash = true;

	float DefaultGroundFriction;
	float DefaultMaxWalkSpeedCrouched;

	// Таймеры
	FTimerHandle DashCooldownTimerHandle;
	FTimerHandle DashDurationTimerHandle;

	void StopDashing();
	void ResetDashCooldown();
};