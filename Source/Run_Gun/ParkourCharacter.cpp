#include "ParkourCharacter.h"
#include "ParkourMovementComponent.h"
#include "CombatComponent.h"
#include "HealthComponent.h"
#include "ParkourSettings.h"
#include "ArtifactComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AParkourCharacter::AParkourCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Создаем компонент паркура
	ParkourComp = CreateDefaultSubobject<UParkourMovementComponent>(TEXT("ParkourComp"));

	// Создаем боевой компонент
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	
	// Создаем компонент здоровья
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	
	// Создаем компонент артефактов
	ArtifactComp = CreateDefaultSubobject<UArtifactComponent>(TEXT("ArtifactComp"));

	// Настраиваем камеру (прикрепляем к сокету головы меша)
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Инициализируем компонент, передавая ему ссылку на себя и на настройки
	if (ParkourComp)
	{
		ParkourComp->Initialize(this, ParkourData);
	}
}

void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Прыжок
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, ParkourComp, &UParkourMovementComponent::RequestJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Рывок (Dash)
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, ParkourComp, &UParkourMovementComponent::RequestDash);
		}

		// Скольжение (Slide)
		if (SlideAction)
		{
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, ParkourComp, &UParkourMovementComponent::RequestSlideStart);
			EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, ParkourComp, &UParkourMovementComponent::RequestSlideStop);
		}

		// Доп. прыжок (тестовая кнопка P)
		if (AddJumpAction)
		{
			EnhancedInputComponent->BindAction(AddJumpAction, ETriggerEvent::Started, ParkourComp, &UParkourMovementComponent::AddExtraJump);
		}

		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AParkourCharacter::Input_Fire);
		}
	}
}

void AParkourCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Уведомляем компонент о приземлении
	if (ParkourComp)
	{
		ParkourComp->HandleLanded();
	}
}

bool AParkourCharacter::IsCharacterSliding() const
{
	return ParkourComp ? ParkourComp->IsSliding() : false;
}

bool AParkourCharacter::IsCharacterCrouching() const
{
	return ParkourComp ? ParkourComp->IsCrouching() : false;
}

void AParkourCharacter::Input_Fire()
{
	if (CombatComp && FirstPersonCameraComponent)
	{
		CombatComp->Fire(
			FirstPersonCameraComponent->GetComponentLocation(),
			FirstPersonCameraComponent->GetForwardVector()
		);
	}
}