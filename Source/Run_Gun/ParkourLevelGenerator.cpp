#include "ParkourLevelGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AParkourLevelGenerator::AParkourLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

//void AParkourLevelGenerator::GenerateLevel()
//{
//    ClearLevel();
//
//    if (!StartRoomClass) return;
//    if (RoomClasses.Num() == 0) return;
//
//    AActor* CurrentRoom = SpawnRoom(StartRoomClass, FTransform::Identity);
//    if (!CurrentRoom) return;
//
//    SpawnedRooms.Add(CurrentRoom);
//
//    for (int32 i = 1; i < NumberOfRooms; i++)
//    {
//        int32 RandomIndex = FMath::RandRange(0, RoomClasses.Num() - 1);
//        TSubclassOf<AActor> NextRoomClass = RoomClasses[RandomIndex];
//
//        FTransform SpawnTransform = CalculateSpawnTransform(CurrentRoom, NextRoomClass);
//
//        AActor* NewRoom = SpawnRoom(NextRoomClass, SpawnTransform);
//        if (!NewRoom) continue;
//
//        SpawnedRooms.Add(NewRoom);
//        CurrentRoom = NewRoom;
//    }
//
//    if (bDebugDraw)
//    {
//        DebugDrawConnections();
//    }
//}

void AParkourLevelGenerator::GenerateLevel()
{
    ClearLevel();

    if (!StartRoomClass) return;
    if (RoomClasses.Num() == 0) return;

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("=== GENERATION STARTED ==="));

    // Spawn start room
    FTransform StartTransform = FTransform::Identity;
    AActor* CurrentRoom = SpawnRoom(StartRoomClass, StartTransform);
    if (!CurrentRoom) return;

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
        FString::Printf(TEXT("Start room spawned at: %s"), *StartTransform.GetLocation().ToString()));

    SpawnedRooms.Add(CurrentRoom);

    // Check Exit of start room
    FTransform FirstExit = GetExitTransform(CurrentRoom);
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
        FString::Printf(TEXT("Start room EXIT at: %s, Rotation: %s"),
            *FirstExit.GetLocation().ToString(), *FirstExit.Rotator().ToString()));

    // Generate remaining rooms
    for (int32 i = 1; i < NumberOfRooms; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, RoomClasses.Num() - 1);
        TSubclassOf<AActor> NextRoomClass = RoomClasses[RandomIndex];

        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
            FString::Printf(TEXT("--- Generating room %d ---"), i));

        // Get Entrance local transform from CDO
        FTransform EntranceLocal = GetEntranceLocalTransform(NextRoomClass);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,
            FString::Printf(TEXT("Entrance LOCAL: Location=%s, Rotation=%s"),
                *EntranceLocal.GetLocation().ToString(), *EntranceLocal.Rotator().ToString()));

        FTransform SpawnTransform = CalculateSpawnTransform(CurrentRoom, NextRoomClass);

        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange,
            FString::Printf(TEXT("Spawn location: %s"), *SpawnTransform.GetLocation().ToString()));

        AActor* NewRoom = SpawnRoom(NextRoomClass, SpawnTransform);
        if (!NewRoom) continue;

        SpawnedRooms.Add(NewRoom);
        CurrentRoom = NewRoom;

        // Check Exit of new room
        FTransform NewExit = GetExitTransform(NewRoom);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta,
            FString::Printf(TEXT("New room EXIT at: %s"), *NewExit.GetLocation().ToString()));
    }

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("=== GENERATION FINISHED ==="));

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
    if (!PreviousRoom) return FTransform::Identity;

    FTransform ExitTransform = GetExitTransform(PreviousRoom);
    FVector ExitLocation = ExitTransform.GetLocation();

    // DON'T use exit rotation! Use previous room's actor rotation
    FRotator PreviousRoomRotation = PreviousRoom->GetActorRotation();

    FTransform EntranceLocalTransform = GetEntranceLocalTransform(NewRoomClass);
    FVector EntranceLocalLocation = EntranceLocalTransform.GetLocation();

    // Calculate position
    FVector NewLocation = ExitLocation - EntranceLocalLocation;

    // Use previous room's rotation, NOT exit's rotation
    FRotator NewRotation = PreviousRoomRotation;

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
        FString::Printf(TEXT("NewRoom at: %s, Rotation: %s"),
            *NewLocation.ToString(),
            *NewRotation.ToString()));

    return FTransform(NewRotation, NewLocation, FVector::OneVector);
}

FTransform AParkourLevelGenerator::GetExitTransform(AActor* Room)
{
    if (!Room) return FTransform::Identity;

    // Try to find Exit component
    TArray<UActorComponent*> Components;
    Room->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp->GetName() == TEXT("Exit"))
        {
            USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
            if (SceneComp)
            {
                FTransform WorldTrans = SceneComp->GetComponentTransform();

                // Debug
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("Exit World: Loc=%s, Rot=%s"),
                        *WorldTrans.GetLocation().ToString(),
                        *WorldTrans.Rotator().ToString()));

                // Also print room location
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue,
                    FString::Printf(TEXT("Room location: %s"), *Room->GetActorLocation().ToString()));

                return WorldTrans;
            }
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Exit NOT found!"));
    return Room->GetActorTransform();
}

FTransform AParkourLevelGenerator::GetEntranceLocalTransform(TSubclassOf<AActor> RoomClass)
{
    if (!RoomClass) return FTransform::Identity;

    // Try to spawn a temporary actor to get components (more reliable)
    AActor* TempActor = GetWorld()->SpawnActor<AActor>(RoomClass, FTransform::Identity);
    if (TempActor)
    {
        TArray<UActorComponent*> Components;
        TempActor->GetComponents(Components);

        for (UActorComponent* Comp : Components)
        {
            FString CompName = Comp->GetName();
            if (CompName == TEXT("Entrance"))
            {
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    FTransform RelativeTransform = SceneComp->GetRelativeTransform();
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                        FString::Printf(TEXT("ENTRANCE found in temp actor! Location=%s, Rotation=%s"),
                            *RelativeTransform.GetLocation().ToString(),
                            *RelativeTransform.Rotator().ToString()));

                    TempActor->Destroy();
                    return RelativeTransform;
                }
            }
        }

        TempActor->Destroy();
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ENTRANCE NOT FOUND!"));
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