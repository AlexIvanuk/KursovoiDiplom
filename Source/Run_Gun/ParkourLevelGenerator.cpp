#include "ParkourLevelGenerator.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

AParkourLevelGenerator::AParkourLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AParkourLevelGenerator::DebugLog(const FString& Message, FColor Color, float Duration)
{
    if (bDebugDraw)
    {
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
    }
}

void AParkourLevelGenerator::GenerateLevel()
{
    ClearLevel();

    if (!StartRoomClass) return;

    // Спавн стартовой комнаты
    AActor* StartRoom = SpawnRoom(StartRoomClass, FTransform::Identity);
    SpawnedRooms.Add(StartRoom);

    // Подготовка очереди
    CurrentGenerationStep = 1;
    GenerationQueue.Empty();

    // Старт пошаговой генерации
    GetWorld()->GetTimerManager().SetTimer(GenerationTimerHandle, this, &AParkourLevelGenerator::ProcessNextRoom, 0.5f, true);
}

void AParkourLevelGenerator::ProcessNextRoom()
{
    if (SpawnedRooms.Num() >= NumberOfRooms)
    {
        GetWorld()->GetTimerManager().ClearTimer(GenerationTimerHandle);
        DebugLog(TEXT("=== GENERATION FINISHED ==="), FColor::Red);
        if (bDebugDraw) DebugDrawConnections();
        return;
    }

    // Если очередь пуста, начинаем сначала
    if (GenerationQueue.IsEmpty() && CurrentGenerationStep == 1)
    {
        GenerationQueue.Enqueue(SpawnedRooms[0]);  // Стартовая комната
    }

    if (GenerationQueue.IsEmpty())
    {
        GetWorld()->GetTimerManager().ClearTimer(GenerationTimerHandle);
        DebugLog(TEXT("=== GENERATION FINISHED (no more rooms to process) ==="), FColor::Red);
        if (bDebugDraw) DebugDrawConnections();
        return;
    }

    AActor* CurrentRoom;
    GenerationQueue.Dequeue(CurrentRoom);

    // Получаем ВСЕ выходы из текущей комнаты
    TArray<FTransform> Exits = GetAllExits(CurrentRoom);

    if (Exits.Num() == 0)
    {
        DebugLog(FString::Printf(TEXT("Room %s has no exits, skipping"), *CurrentRoom->GetName()), FColor::Orange);
        return;
    }

    // Определяем сколько веток создать (1 или несколько)
    int32 NumBranches = FMath::Min(MaxBranches, Exits.Num());

    for (int32 i = 0; i < NumBranches && SpawnedRooms.Num() < NumberOfRooms; i++)
    {
        // Для первого выхода всегда создаём комнату (100% шанс)
        // Для остальных - с шансом BranchChance
        if (i > 0 && FMath::FRand() > BranchChance)
        {
            DebugLog(FString::Printf(TEXT("Branch skipped (chance)")), FColor::Yellow);
            continue;
        }

        int32 ExitIndex = i % Exits.Num();
        FTransform SelectedExit = Exits[ExitIndex];

        int32 RandomIndex = FMath::RandRange(0, RoomClasses.Num() - 1);
        TSubclassOf<AActor> NextRoomClass = RoomClasses[RandomIndex];

        FTransform EntranceLocal = GetEntranceLocalTransform(NextRoomClass);

        FVector SpawnLocation = SelectedExit.GetLocation() -
            SelectedExit.Rotator().RotateVector(EntranceLocal.GetLocation());
        FRotator SpawnRotation = SelectedExit.Rotator();

        FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector::OneVector);

        if (DoesOverlapWithAnyRoom(SpawnTransform, NextRoomClass))
        {
            DebugLog(FString::Printf(TEXT("OVERLAP! Skipping exit %d"), ExitIndex), FColor::Orange);
            continue;
        }

        AActor* NewRoom = SpawnRoom(NextRoomClass, SpawnTransform);
        if (NewRoom)
        {
            SpawnedRooms.Add(NewRoom);
            GenerationQueue.Enqueue(NewRoom);  // Добавляем в очередь для дальнейшего расширения
            DebugLog(FString::Printf(TEXT("Room %d spawned from exit %d (branch %d)"),
                SpawnedRooms.Num() - 1, ExitIndex, i), FColor::Green);
        }
    }

    CurrentGenerationStep++;
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

    // Get exit transform from previous room
    FTransform ExitTransform = GetExitTransform(PreviousRoom);
    FVector ExitLocation = ExitTransform.GetLocation();
    FRotator ExitRotation = ExitTransform.Rotator();

    // Get entrance local transform from new room
    FTransform EntranceLocalTransform = GetEntranceLocalTransform(NewRoomClass);
    FVector EntranceLocalLocation = EntranceLocalTransform.GetLocation();

    // Rotate entrance local position by exit rotation
    FVector RotatedEntranceOffset = ExitRotation.RotateVector(EntranceLocalLocation);

    // Calculate new room position
    FVector NewLocation = ExitLocation - RotatedEntranceOffset;

    // New room inherits exit rotation
    FRotator NewRotation = ExitRotation;

    // Debug
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
        FString::Printf(TEXT("Exit Rot=%s, New Rot=%s"),
            *ExitRotation.ToString(),
            *NewRotation.ToString()));

    return FTransform(NewRotation, NewLocation, FVector::OneVector);
}

FTransform AParkourLevelGenerator::GetExitTransform(AActor* Room)
{
    if (!Room) return FTransform::Identity;

    TArray<UActorComponent*> Components;
    Room->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp->GetName() == ExitComponentName)
        {
            USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
            if (SceneComp)
            {
                FTransform WorldTrans = SceneComp->GetComponentTransform();

                DebugLog(FString::Printf(TEXT("Exit World: Loc=%s, Rot=%s"),
                    *WorldTrans.GetLocation().ToString(),
                    *WorldTrans.Rotator().ToString()), FColor::Green);
                DebugLog(FString::Printf(TEXT("Room location: %s"), *Room->GetActorLocation().ToString()), FColor::Blue);

                return WorldTrans;
            }
        }
    }

    DebugLog(TEXT("Exit NOT found!"), FColor::Red);
    return Room->GetActorTransform();
}

FTransform AParkourLevelGenerator::GetEntranceLocalTransform(TSubclassOf<AActor> RoomClass)
{
    if (!RoomClass) return FTransform::Identity;

    AActor* TempActor = GetWorld()->SpawnActor<AActor>(RoomClass, FTransform::Identity);
    if (TempActor)
    {
        TArray<UActorComponent*> Components;
        TempActor->GetComponents(Components);

        for (UActorComponent* Comp : Components)
        {
            if (Comp->GetName() == EntranceComponentName)  // Было TEXT("Entrance")
            {
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    FTransform RelativeTransform = SceneComp->GetRelativeTransform();
                    DebugLog(FString::Printf(TEXT("ENTRANCE found in temp actor! Location=%s, Rotation=%s"),
                        *RelativeTransform.GetLocation().ToString(),
                        *RelativeTransform.Rotator().ToString()), FColor::Green);

                    TempActor->Destroy();
                    return RelativeTransform;
                }
            }
        }

        TempActor->Destroy();
    }

    DebugLog(TEXT("ENTRANCE NOT FOUND!"), FColor::Red);
    return FTransform::Identity;
}

void AParkourLevelGenerator::DebugDrawConnections()
{
    int32 RoomIndex = 0;

    for (AActor* Room : SpawnedRooms)
    {
        if (!Room) continue;

        // Room number and name
        FVector TextLocation = Room->GetActorLocation() + FVector(0, 0, 150);
        DrawDebugString(GetWorld(),
            TextLocation,
            FString::Printf(TEXT("[%d] %s"), RoomIndex, *Room->GetName()),
            nullptr,
            FColor::White,
            -1.0f,
            true);

        // Draw entrance (green)
        TArray<UActorComponent*> Components;
        Room->GetComponents(Components);

        for (UActorComponent* Comp : Components)
        {
            if (Comp->GetName().Contains(TEXT("Entrance")))
            {
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    DrawDebugSphere(GetWorld(), SceneComp->GetComponentLocation(), 40, 12, FColor::Green, true, 10.0f);
                }
            }
        }

        // Draw all exits (red)
        for (UActorComponent* Comp : Components)
        {
            if (Comp->GetName().Contains(TEXT("Exit")))
            {
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    DrawDebugSphere(GetWorld(), SceneComp->GetComponentLocation(), 40, 12, FColor::Red, true, 10.0f);

                    // Draw direction arrow
                    DrawDebugLine(GetWorld(),
                        SceneComp->GetComponentLocation(),
                        SceneComp->GetComponentLocation() + SceneComp->GetForwardVector() * 100,
                        FColor::Red, true, 10.0f, 0, 3.0f);
                }
            }
        }

        // Draw ALL collision boxes
        for (UActorComponent* Comp : Components)
        {
            if (Comp->GetName().Contains(TEXT("Room_Bounds")))
            {
                UBoxComponent* Box = Cast<UBoxComponent>(Comp);
                if (Box)
                {
                    DrawDebugBox(GetWorld(),
                        Box->GetComponentLocation(),
                        Box->GetScaledBoxExtent(),
                        Box->GetComponentQuat(),
                        FColor::Purple, true, 10.0f, 0, 2.0f);
                }
            }
        }

        RoomIndex++;
    }
}

TArray<FTransform> AParkourLevelGenerator::GetAllExits(AActor* Room)
{
    TArray<FTransform> Exits;

    if (!Room) return Exits;

    TArray<UActorComponent*> Components;
    Room->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        // Ищем ЛЮБОЙ компонент, содержащий "Exit" в имени
        if (Comp->GetName().Contains(TEXT("Exit")))
        {
            USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
            if (SceneComp)
            {
                Exits.Add(SceneComp->GetComponentTransform());
                DebugLog(FString::Printf(TEXT("Found exit: %s at %s"),
                    *Comp->GetName(),
                    *SceneComp->GetComponentLocation().ToString()), FColor::Cyan);
            }
        }
    }

    DebugLog(FString::Printf(TEXT("Room has %d exits"), Exits.Num()), FColor::Yellow);
    return Exits;
}

// Исправленная проверка пересечения
bool AParkourLevelGenerator::DoesOverlapWithAnyRoom(const FTransform& Transform, TSubclassOf<AActor> RoomClass)
{
    if (!RoomClass || SpawnedRooms.Num() == 0) return false;

    AActor* TempActor = GetWorld()->SpawnActor<AActor>(RoomClass, Transform);
    if (!TempActor) return false;

    TArray<UPrimitiveComponent*> TempPrimitives;
    TempActor->GetComponents<UPrimitiveComponent>(TempPrimitives);

    bool bOverlaps = false;

    for (UPrimitiveComponent* TempPrim : TempPrimitives)
    {
        if (!TempPrim->GetName().Contains(TEXT("Room_Bounds"))) continue;

        // Проверяем с каждой существующей комнатой
        for (AActor* ExistingRoom : SpawnedRooms)
        {
            TArray<UPrimitiveComponent*> ExistingPrimitives;
            ExistingRoom->GetComponents<UPrimitiveComponent>(ExistingPrimitives);

            for (UPrimitiveComponent* ExistingPrim : ExistingPrimitives)
            {
                if (!ExistingPrim->GetName().Contains(TEXT("Room_Bounds"))) continue;

                // ИСПРАВЛЕННАЯ ПРОВЕРКА - используем GetWorld()->OverlapComponent
                FCollisionQueryParams QueryParams;
                QueryParams.AddIgnoredActor(TempActor);
                QueryParams.AddIgnoredActor(ExistingRoom);

                FBoxSphereBounds TempBounds = TempPrim->Bounds;
                FBoxSphereBounds ExistingBounds = ExistingPrim->Bounds;

                // Простая проверка на пересечение bounding box с учётом поворота
                if (TempBounds.GetBox().Intersect(ExistingBounds.GetBox()))
                {
                    bOverlaps = true;
                    DebugLog(FString::Printf(TEXT("OVERLAP! %s vs %s"),
                        *TempActor->GetName(), *ExistingRoom->GetName()), FColor::Orange);
                    break;
                }
            }
            if (bOverlaps) break;
        }
        if (bOverlaps) break;
    }

    TempActor->Destroy();
    return bOverlaps;
}