#include "HealthComponent.h"

UHealthComponent::UHealthComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void UHealthComponent::TakeDamage(float DamageAmount)
{
	if (CurrentHealth <= 0.0f) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast(); // Âûחûגאול סמבûעטו סלונעט
	}
}

void UHealthComponent::RestoreFullHealth()
{
	CurrentHealth = MaxHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}