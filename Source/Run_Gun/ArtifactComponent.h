// ArtifactComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArtifactTypes.h"
#include "ArtifactComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RUN_GUN_API UArtifactComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Функция подъёма предмета
    UFUNCTION(BlueprintCallable, Category = "Artifacts")
    void PickupArtifact(FArtifactRow ArtifactData);

    // Функция, отвечающая за вероятности выпадения предметов
    UFUNCTION(BlueprintCallable, Category = "Artifacts")
    static TArray<FName> RollLoot(const ULootTableAsset* LootTable);

private:
    // Список всех имен подобранных предметов (для истории)
    UPROPERTY()
    TArray<FName> CollectedArtifacts;

    // Ссылки на другие компоненты, которые мы будем менять
    UPROPERTY() class UParkourMovementComponent* ParkourComp;
    UPROPERTY() class UCombatComponent* CombatComp;
    UPROPERTY() class UHealthComponent* HealthComp;
};