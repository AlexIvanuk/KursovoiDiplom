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

// Структура одной записи в списке выпадения
USTRUCT(BlueprintType)
struct FLootEntry
{
    GENERATED_BODY()

    // Имя строки из таблицы DT_Artifacts
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ArtifactRowName;

    // Вес (вероятность). Чем больше, тем чаще выпадает.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 100.0f;

    // Если True, предмет выпадет максимум 1 раз за один "смертельный улов"
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnique = true;
};

// Класс ассета, который мы будем создавать в редакторе (Data Asset)
UCLASS(BlueprintType)
class RUN_GUN_API ULootTableAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Список возможных предметов
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    TArray<FLootEntry> PossibleLoot;

    // Общий шанс, что враг вообще хоть что-то уронит (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GlobalDropChance = 0.3f;

    // Сколько попыток выпадения сделать, если GlobalDropChance сработал
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    int32 MinDrops = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
    int32 MaxDrops = 2;
};