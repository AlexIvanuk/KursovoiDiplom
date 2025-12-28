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
    // Сохраняем стандартное значение трения при старте игры
    if (GetCharacterMovement())
    {
        DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
        DefaultMaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeedCrouched;
    }
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

void AParkourCharacter::StartSlide()
{
    // Получаем текущую скорость только в горизонтальной плоскости (без учета падения)
    const float GroundSpeed = FVector(GetVelocity().X, GetVelocity().Y, 0).Size();

    // Проверяем, на земле ли мы и достаточно ли быстро бежим
    if (GetCharacterMovement()->IsMovingOnGround() && GroundSpeed >= MinSpeedForSlide)
    {
        bIsSliding = true;

        // 1. Временно увеличиваем максимальную скорость в приседе
        GetCharacterMovement()->MaxWalkSpeedCrouched = GroundSpeed * SlideSpeedMultiplier;

        // 2. Устанавливаем низкое трение для гладкого скольжения.
        GetCharacterMovement()->GroundFriction = SlideFriction;

        // 3. Теперь безопасно вызываем Crouch().
        Crouch();
    }
    else
    {
        // Если скорость недостаточная, просто приседаем без скольжения
        Crouch();
    }
}

void AParkourCharacter::StopSlide()
{
    // Если мы скользили, возвращаем стандартное трение
    if (bIsSliding)
    {
        bIsSliding = false;
        GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
        GetCharacterMovement()->MaxWalkSpeedCrouched = DefaultMaxWalkSpeedCrouched;
    }

    // Встаем
    UnCrouch();
}

void AParkourCharacter::Jump()
{
    
    Super::Jump();

    if (GetCharacterMovement()->IsFalling() && ExtraJumpsAvailable > 0)
    {
        // Уменьшаем счетчик доступных прыжков
        ExtraJumpsAvailable--;

        // "Запускаем" персонажа строго вверх. 
        LaunchCharacter(FVector(0.f, 0.f, GetCharacterMovement()->JumpZVelocity), false, true);
    }
}

void AParkourCharacter::Landed(const FHitResult& Hit)
{
    // Сначала вызываем родительскую функцию Landed на случай, если в ней есть важная логика.
    Super::Landed(Hit);

    // Восстанавливаем наши дополнительные прыжки до максимального значения.
    ExtraJumpsAvailable = MaxExtraJumps;
}

void AParkourCharacter::AddExtraJump()
{
    // Увеличиваем максимальное количество прыжков
    MaxExtraJumps++;

    // Сразу же обновляем и текущие доступные прыжки, чтобы игрок мог использовать его немедленно
    ExtraJumpsAvailable = MaxExtraJumps;
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
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Проверяем, что наши переменные-ссылки на ассеты были установлены в Блюпринте
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        if (DashAction)
        {
            // Привязываем DashAction к нашей C++ функции Dash
            EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AParkourCharacter::Dash);
        }

        if (AddJumpAction)
        {
            // Привязываем AddJumpAction к нашей C++ функции AddExtraJump
            EnhancedInputComponent->BindAction(AddJumpAction, ETriggerEvent::Started, this, &AParkourCharacter::AddExtraJump);
        }
        if (SlideAction)
        {
            // При нажатии кнопки вызываем StartSlide, при отпускании - StopSlide
            EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AParkourCharacter::StartSlide);
            EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AParkourCharacter::StopSlide);
        }
    }
}