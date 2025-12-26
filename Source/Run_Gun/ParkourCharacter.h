// ParkourCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

    UFUNCTION(BlueprintCallable, Category = "Dash")
    void Dash();

    void ResetDash();

private:
    // "Флаг", который показывает, можем ли мы делать рывок
    bool bCanDash = true;

    // Ссылка на таймер, чтобы могли им управлять
    FTimerHandle DashCooldownTimerHandle;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent * PlayerInputComponent) override;
};