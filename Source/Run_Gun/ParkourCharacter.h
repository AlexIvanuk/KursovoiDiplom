// ParkourCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "ParkourCharacter.generated.h"

UCLASS()
class RUN_GUN_API AParkourCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AParkourCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Сила, с которой будет произведен рывок
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashForce = 1500.0f;

    // Время в секундах, которое рывок будет на перезарядке
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashCooldown = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DashAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping")
    int32 MaxExtraJumps = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Jumping")
    int32 ExtraJumpsAvailable = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SlideAction;

    // Минимальная скорость на земле, чтобы начать скольжение, а не просто присесть
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
    float MinSpeedForSlide = 600.0f;

    // Множитель скорости в начале скольжения (чтобы был небольшой буст)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
    float SlideSpeedMultiplier = 1.2f;

    // Сила трения во время скольжения (чем выше, тем быстрее персонаж остановится)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sliding")
    float SlideFriction = 2.0f;

    // --- ФУНКЦИИ ДЛЯ СКОЛЬЖЕНИЯ ---
    void StartSlide();
    void StopSlide();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AddJumpAction;

    UFUNCTION(BlueprintCallable, Category = "Dash")
    void Dash();

    void ResetDash();

    virtual void Jump() override;
    virtual void Landed(const FHitResult& Hit) override;

    // Функция для тестовой кнопки "P", чтобы добавлять прыжки
    UFUNCTION(BlueprintCallable, Category = "Jumping")
    void AddExtraJump();

private:
    // "Флаг", который показывает, можем ли мы делать рывок
    bool bCanDash = true;

    // "Флаг", который показывает, скользим ли мы сейчас
    bool bIsSliding = false;

    // Здесь мы сохраним стандартное значение трения, чтобы вернуть его после скольжения
    float DefaultGroundFriction;
    float DefaultMaxWalkSpeedCrouched;

    // Ссылка на таймер, чтобы могли им управлять
    FTimerHandle DashCooldownTimerHandle;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent * PlayerInputComponent) override;
};