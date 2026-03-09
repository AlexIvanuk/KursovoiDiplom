// ParkourCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "ParkourCharacter.generated.h"

// Предварительные объявления классов (Forward Declarations)
// Это ускоряет компиляцию и избегает циклических зависимостей
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

	// Камера от первого лица
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCameraComponent;

	// Наш новый модульный компонент паркура
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParkourMovementComponent* ParkourComp;

	// --- ГЕТТЕРЫ ДЛЯ АНИМАЦИИ ---

	// Оставляем эту функцию для удобства работы Анимационного Блюпринта
	UFUNCTION(BlueprintPure, Category = "Parkour")
	bool IsSliding() const;

protected:
	virtual void BeginPlay() override;

	// --- НАСТРОЙКИ (DATA ASSET) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	UParkourSettings* ParkourData;

	// --- СИСТЕМА ВВОДА (INPUT ACTIONS) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AddJumpAction;

	// --- ОБРАБОТЧИКИ ВВОДА ---
	// Эти функции вызываются при нажатии кнопок и просто передают приказ в компонент

	void Input_Jump();
	void Input_StopJumping();
	void Input_Dash();
	void Input_SlideStart();
	void Input_SlideStop();
	void Input_AddJump();

	// Переопределение приземления для сброса прыжков в компоненте
	virtual void Landed(const FHitResult& Hit) override;

public:
	// Настройка привязок кнопок
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};