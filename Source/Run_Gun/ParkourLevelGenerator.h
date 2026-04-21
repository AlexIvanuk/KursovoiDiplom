#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParkourLevelGenerator.generated.h"

UCLASS()
class RUN_GUN_API AParkourLevelGenerator : public AActor
{
    GENERATED_BODY()

public:
    AParkourLevelGenerator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TSubclassOf<AActor> StartRoomClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<TSubclassOf<AActor>> RoomClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 NumberOfRooms = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float RoomSpacing = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bDebugDraw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connectors")
    FString EntranceComponentName = TEXT("Entrance");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connectors")
    FString ExitComponentName = TEXT("Exit");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Branching")
    int32 MaxBranches = 3;  // ћаксимум веток от одной комнаты

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Branching")
    float BranchChance = 0.4f;  // 40% шанс создать ветку

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Branching")
    int32 MaxDepth = 5;  // ћаксимальна€ глубина ветвлени€

private:
    AActor* SpawnRoom(TSubclassOf<AActor> RoomClass, const FTransform& Transform);
    FTransform CalculateSpawnTransform(AActor* PreviousRoom, TSubclassOf<AActor> NewRoomClass);
    FTransform GetExitTransform(AActor* Room);
    FTransform GetEntranceLocalTransform(TSubclassOf<AActor> RoomClass);
    void DebugDrawConnections();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateLevel();

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void ClearLevel();

    FTimerHandle GenerationTimerHandle;
    int32 CurrentGenerationStep;
    TQueue<AActor*> GenerationQueue;

    void ProcessNextRoom();

    TArray<AActor*> SpawnedRooms;
    void DebugLog(const FString& Message, FColor Color = FColor::White, float Duration = 5.0f);

    TArray<FTransform> GetAllExits(AActor* Room);

    bool DoesOverlapWithAnyRoom(const FTransform& Transform, TSubclassOf<AActor> RoomClass);
};