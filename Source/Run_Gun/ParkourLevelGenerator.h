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

private:
    TArray<AActor*> SpawnedRooms;
};