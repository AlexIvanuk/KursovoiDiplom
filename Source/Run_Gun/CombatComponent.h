#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RUN_GUN_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	// Универсальная функция выстрела
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire(FVector Start, FVector Direction);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Range = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitEffect;
};