#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HealthComponent.h"

UCombatComponent::UCombatComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatComponent::Fire(FVector Start, FVector Direction)
{
	FHitResult Hit;
	FVector End = Start + (Direction * Range);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); // Ќе стрел€ть в себ€

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		// 1. Ёффект попадани€
		if (HitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, Hit.ImpactPoint);
		}

		// 2. Ћогика урона
		if (AActor* HitActor = Hit.GetActor())
		{
			// »щем компонент здоровь€ у того, в кого попали
			UHealthComponent* Health = HitActor->FindComponentByClass<UHealthComponent>();
			if (Health)
			{
				Health->TakeDamage(Damage);
			}
		}
	}
}