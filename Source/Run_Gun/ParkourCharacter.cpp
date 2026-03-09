// ParkourCharacter.cpp

#include "ParkourCharacter.h"
#include "ParkourMovementComponent.h"
#include "ParkourSettings.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AParkourCharacter::AParkourCharacter()
{
	// Персонажу больше не нужно тикать самому, так как логика теперь в компоненте
	PrimaryActorTick.bCanEverTick = false;

	// 1. Создаем компонент паркура
	ParkourComp = CreateDefaultSubobject<UParkourMovementComponent>(TEXT("ParkourComp"));

	// 2. Настраиваем камеру (прикрепляем к голове)
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Принудительная настройка видимости меша (как мы делали раньше)
	GetMesh()->SetOwnerNoSee(false);
	GetMesh()->SetOnlyOwnerSee(true);
}

void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Инициализируем компонент: даем ему ссылку на нас и на ассет настроек
	if (ParkourComp)
	{
		ParkourComp->Initialize(this, ParkourData);
	}
}

// --- ОБРАБОТКА ВВОДА (ПРОБРОС КОМАНД В КОМПОНЕНТ) ---

void AParkourCharacter::Input_Jump()
{
	if (ParkourComp) ParkourComp->RequestJump();
}

void AParkourCharacter::Input_Dash()
{
	if (ParkourComp) ParkourComp->Dash();
}

void AParkourCharacter::Input_SlideStart()
{
	if (ParkourComp) ParkourComp->StartSlide();
}

void AParkourCharacter::Input_SlideStop()
{
	if (ParkourComp) ParkourComp->StopSlide();
}

void AParkourCharacter::Input_AddJump()
{
	if (ParkourComp) ParkourComp->AddExtraJump();
}

// Переопределение приземления: сообщаем компоненту, что мы на земле
void AParkourCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (ParkourComp) ParkourComp->HandleLanded();
}

// Геттер для анимаций (теперь берет данные из компонента)
bool AParkourCharacter::IsSliding() const
{
	return ParkourComp ? ParkourComp->IsSliding() : false;
}

// --- ПРИВЯЗКА ВВОДА ---

void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Прыжок
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AParkourCharacter::Input_Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Рывок
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AParkourCharacter::Input_Dash);
		}

		// Скольжение
		if (SlideAction)
		{
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AParkourCharacter::Input_SlideStart);
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AParkourCharacter::Input_SlideStop);
		}

		// Добавление прыжка (тестовая кнопка P)
		if (AddJumpAction)
		{
			EnhancedInputComponent->BindAction(AddJumpAction, ETriggerEvent::Started, this, &AParkourCharacter::Input_AddJump);
		}
	}
}