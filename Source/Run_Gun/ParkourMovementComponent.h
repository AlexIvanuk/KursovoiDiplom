// ParkourMovementComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourSettings.h"
#include "ParkourMovementComponent.generated.h"

// Переносим Enum сюда, так как это часть системы паркура
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

	// Инициализация (связываем с персонажем)
	void Initialize(class ACharacter* InOwner, UParkourSettings* InSettings);

	// Публичные команды
	void Dash();
	void StartSlide();
	void StopSlide();
	void RequestJump();
	void HandleLanded();
	void AddExtraJump();

	// Геттеры для АнимБП
	UFUNCTION(BlueprintPure, Category = "Parkour")
	bool IsSliding() const { return CurrentState == EParkourState::Sliding; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	EParkourState CurrentState = EParkourState::Default;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Ссылки на владельца
	UPROPERTY()
	class ACharacter* CharacterOwner;

	UPROPERTY()
	class UCharacterMovementComponent* MoveComp;

	UPROPERTY()
	UParkourSettings* Settings;

	// Внутренние переменные
	int32 MaxExtraJumps = 0;
	int32 ExtraJumpsAvailable = 0;
	bool bCanDash = true;

	float DefaultGroundFriction;
	float DefaultMaxWalkSpeedCrouched;

	FTimerHandle DashCooldownTimerHandle;
	void ResetDash();
};