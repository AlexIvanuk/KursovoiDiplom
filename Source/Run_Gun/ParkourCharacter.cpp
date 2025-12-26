// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
AParkourCharacter::AParkourCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AParkourCharacter::Dash()
{
    // Проверяем, можем ли мы сделать рывок. Если нет (bCanDash == false), выходим из функции.
    if (!bCanDash)
    {
        return;
    }

    // Сразу же запрещаем делать рывок снова, чтобы избежать спама
    bCanDash = false;

    // Получаем ссылку на компонент движения, он нам понадобится несколько раз
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (!MovementComp)
    {
        return; // Если компонента нет, ничего не делаем
    }

    FVector DashDirection = MovementComp->GetLastInputVector().GetSafeNormal();

    // Если игрок не нажимает никаких кнопок, делаем рывок просто вперед
    if (DashDirection.IsNearlyZero())
    {
        DashDirection = GetActorForwardVector();
    }

    // --- САМ РЫВОК ---

    // Устанавливаем трение в 0, чтобы рывок был мощным на земле
    MovementComp->GroundFriction = 0.0f;

    // "Запускаем" персонажа. Используем Launch, так как он лучше работает с физикой, чем прямая установка Velocity.
    // Галочки true, true означают XY Override и Z Override, полностью заменяя текущую скорость.
    LaunchCharacter(DashDirection * DashForce, true, true);

    // Запускаем таймер, который по окончании кулдауна вызовет нашу функцию ResetDash
    GetWorld()->GetTimerManager().SetTimer(DashCooldownTimerHandle, this, &AParkourCharacter::ResetDash, DashCooldown);
}

void AParkourCharacter::ResetDash()
{
    // Разрешаем делать рывок снова
    bCanDash = true;

    // Возвращаем трение к стандартному значению
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->GroundFriction = 8.0f; // 8.0 - это стандартное значение
    }
}

// Called every frame
void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}