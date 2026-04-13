// ParkourCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "ParkourCharacter.generated.h"

// Предварительные объявления классов (ускоряют компиляцию)
class UCameraComponent;
class UParkourMovementComponent;
class UParkourSettings;

UCLASS()
class RUN_GUN_API AParkourCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AParkourCharacter();

	// --- КОМПОНЕНТЫ ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParkourMovementComponent* ParkourComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UHealthComponent* HealthComp;

protected:
	virtual void BeginPlay() override;

	// --- НАСТРОЙКИ (DATA ASSET) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	UParkourSettings* ParkourData;

	// --- СИСТЕМА ВВОДА ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AddJumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* FireAction;

	// --- СОБЫТИЯ ДВИЖКА ---
	virtual void Landed(const FHitResult& Hit) override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Геттер для Анимационного Блюпринта (теперь берет данные из компонента)
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsCharacterSliding() const;

	void Input_Fire();
};