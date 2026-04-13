#include "ParkourLevelGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AParkourLevelGenerator::AParkourLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AParkourLevelGenerator::GenerateLevel()
{
    ClearLevel();

    if (!StartRoomClass) return;
    if (RoomClasses.Num() == 0) return;

    AActor* CurrentRoom = SpawnRoom(StartRoomClass, FTransform::Identity);
    if (!CurrentRoom) return;

    SpawnedRooms.Add(CurrentRoom);

    for (int32 i = 1; i < NumberOfRooms; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, RoomClasses.Num() - 1);
        TSubclassOf<AActor> NextRoomClass = RoomClasses[RandomIndex];

        FTransform SpawnTransform = CalculateSpawnTransform(CurrentRoom, NextRoomClass);

        AActor* NewRoom = SpawnRoom(NextRoomClass, SpawnTransform);
        if (!NewRoom) continue;

        SpawnedRooms.Add(NewRoom);
        CurrentRoom = NewRoom;
    }

    if (bDebugDraw)
    {
        DebugDrawConnections();
    }
}

void AParkourLevelGenerator::ClearLevel()
{
    for (AActor* Room : SpawnedRooms)
    {
        if (Room) Room->Destroy();
    }
    SpawnedRooms.Empty();
}

AActor* AParkourLevelGenerator::SpawnRoom(TSubclassOf<AActor> RoomClass, const FTransform& Transform)
{
    if (!RoomClass) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    return GetWorld()->SpawnActor<AActor>(RoomClass, Transform, SpawnParams);
}

FTransform AParkourLevelGenerator::CalculateSpawnTransform(AActor* PreviousRoom, TSubclassOf<AActor> NewRoomClass)
{
    FTransform ExitTransform = GetExitTransform(PreviousRoom);
    FTransform EntranceLocalTransform = GetEntranceLocalTransform(NewRoomClass);

    FVector EntranceWorldOffset = ExitTransform.TransformVector(EntranceLocalTransform.GetLocation());
    FVector SpawnLocation = ExitTransform.GetLocation() - EntranceWorldOffset;

    FRotator SpawnRotation = ExitTransform.Rotator() + EntranceLocalTransform.Rotator();

    return FTransform(SpawnRotation, SpawnLocation, FVector::OneVector);
}

FTransform AParkourLevelGenerator::GetExitTransform(AActor* Room)
{
    if (!Room) return FTransform::Identity;

    TArray<UActorComponent*> Components;
    Room->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp->GetName().Contains(TEXT("Exit")))
        {
            USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
            if (SceneComp)
            {
                return SceneComp->GetComponentTransform();
            }
        }
    }

    return Room->GetActorTransform();
}

FTransform AParkourLevelGenerator::GetEntranceLocalTransform(TSubclassOf<AActor> RoomClass)
{
    if (!RoomClass) return FTransform::Identity;

    AActor* CDO = RoomClass->GetDefaultObject<AActor>();
    if (!CDO) return FTransform::Identity;

    TArray<UActorComponent*> Components;
    CDO->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp->GetName().Contains(TEXT("Entrance")))
        {
            USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
            if (SceneComp)
            {
                return SceneComp->GetRelativeTransform();
            }
        }
    }

    return FTransform::Identity;
}

void AParkourLevelGenerator::DebugDrawConnections()
{
    for (AActor* Room : SpawnedRooms)
    {
        if (!Room) continue;

        FTransform ExitTrans = GetExitTransform(Room);
        DrawDebugSphere(GetWorld(), ExitTrans.GetLocation(), 50, 12, FColor::Red, true, 10.0f);

        TArray<UActorComponent*> Components;
        Room->GetComponents(Components);

        for (UActorComponent* Comp : Components)
        {
            if (Comp->GetName().Contains(TEXT("Entrance")))
            {
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    FTransform EntranceTrans = SceneComp->GetComponentTransform();
                    DrawDebugSphere(GetWorld(), EntranceTrans.GetLocation(), 50, 12, FColor::Green, true, 10.0f);
                }
            }
        }
    }
}