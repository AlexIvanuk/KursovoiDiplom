// ArtifactComponent.cpp
#include "ArtifactComponent.h"
#include "ParkourMovementComponent.h"
#include "CombatComponent.h"
#include "HealthComponent.h"


void UArtifactComponent::PickupArtifact(FArtifactRow ArtifactData)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (!ParkourComp) ParkourComp = Owner->FindComponentByClass<UParkourMovementComponent>();
    if (!CombatComp)  CombatComp = Owner->FindComponentByClass<UCombatComponent>();
    if (!HealthComp)  HealthComp = Owner->FindComponentByClass<UHealthComponent>();

    for (const FStatModifier& Mod : ArtifactData.Modifiers)
    {
        auto CalculateValue = [&](float CurrentValue) -> float {
            return (Mod.Operation == EModifierOp::Multiply) ? (CurrentValue * Mod.Value) : (CurrentValue + Mod.Value);
            };

        switch (Mod.Stat)
        {
        case EArtifactStatType::MaxHealth:
            if (HealthComp) {
                float OldMax = HealthComp->MaxHealth;
                HealthComp->MaxHealth = CalculateValue(HealthComp->MaxHealth);

                // Бонус: если макс. ХП выросло, добавим разницу к текущему здоровью (подлечим)
                float Diff = HealthComp->MaxHealth - OldMax;
                if (Diff > 0) HealthComp->CurrentHealth += Diff;
            }
            break;

        case EArtifactStatType::Damage:
            if (CombatComp) CombatComp->Damage = CalculateValue(CombatComp->Damage);
            break;

        case EArtifactStatType::JumpCount:
            // Прыжки обычно только прибавляются количественно
            if (ParkourComp) ParkourComp->AddExtraJump();
            break;

        case EArtifactStatType::DashForce:
            // Если в ParkourSettings DashForce сделать не const, можно менять и его!
            break;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("Picked up: %s"), *ArtifactData.Name.ToString());
}

TArray<FName> UArtifactComponent::RollLoot(const ULootTableAsset* LootTable)
{
	TArray<FName> SelectedItems;

	// Базовые проверки безопасности
	if (!LootTable || LootTable->PossibleLoot.Num() == 0)
	{
		return SelectedItems;
	}

	// 1. Первый бросок: выпадет ли вообще лут?
	if (FMath::FRand() > LootTable->GlobalDropChance)
	{
		return SelectedItems;
	}

	// 2. Копируем список лута во временный массив (пул), чтобы удалять из него уникальные предметы
	TArray<FLootEntry> Pool = LootTable->PossibleLoot;

	// 3. Определяем количество предметов для этого дропа
	int32 TargetCount = FMath::RandRange(LootTable->MinDrops, LootTable->MaxDrops);

	// 4. Цикл попыток выбить предмет
	for (int32 i = 0; i < TargetCount; i++)
	{
		if (Pool.Num() == 0) break; // Если пул пуст (выбили все уникальные), выходим

		// Считаем общую сумму весов текущего пула
		float TotalWeight = 0.0f;
		for (const FLootEntry& Entry : Pool) TotalWeight += Entry.Weight;

		// Бросаем кубик
		float RandomRoll = FMath::FRandRange(0.0f, TotalWeight);
		float CurrentWeightSum = 0.0f;

		// Ищем, в какой "отрезок" веса попало число
		for (int32 Index = 0; Index < Pool.Num(); Index++)
		{
			CurrentWeightSum += Pool[Index].Weight;

			if (RandomRoll <= CurrentWeightSum)
			{
				// Предмет выбран!
				SelectedItems.Add(Pool[Index].ArtifactRowName);

				// Если он уникальный — удаляем его из пула для следующей попытки в этом же цикле
				if (Pool[Index].bIsUnique)
				{
					Pool.RemoveAt(Index);
				}
				break; // Переходим к следующей попытке (i++)
			}
		}
	}

	return SelectedItems;
}