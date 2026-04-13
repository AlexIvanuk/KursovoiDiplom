#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// Создаем событие смерти, которое будет видно в Блюпринтах
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RUN_GUN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeathSignature OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float DamageAmount);

protected:
	virtual void BeginPlay() override;
};