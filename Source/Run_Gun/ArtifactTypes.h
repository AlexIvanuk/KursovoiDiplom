#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArtifactTypes.generated.h"

// 1. Статы, которые можем менять
UENUM(BlueprintType)
enum class EArtifactStatType : uint8 {
    MaxHealth,
    Damage,
    DashForce,
    DashCooldown,
    JumpCount,
    MovementSpeed
};

// 2. Как именно мы их меняем?
UENUM(BlueprintType)
enum class EModifierOp : uint8 {
    Add,      // Прибавить значение
    Multiply  // Умножить
};

// 3. Структура одного изменения
USTRUCT(BlueprintType)
struct FStatModifier {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EArtifactStatType Stat; // Обновили тип здесь

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EModifierOp Operation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Value;
};

// 4. Главная структура для таблицы
USTRUCT(BlueprintType)
struct FArtifactRow : public FTableRowBase {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic")
    TArray<FStatModifier> Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    UStaticMesh* PickupMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    FLinearColor GlowColor;
};