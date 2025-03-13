// The source code, authored by Zoxemik in 2025

#include "Spawner.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "IslandPluginInterface.h"

ASpawner::ASpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Seed.Initialize(0);

	bAsyncComplete = false;
	bAutoSpawn = true;
	bActorSwitch = true;

	Counter = 0;
	IndexCounter = 0;
	ClassRefIndex = 0;

	Step = 200.0f;
}

void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (GameInstance->GetClass()->ImplementsInterface(UIslandPluginInterface::StaticClass()))
	{
		// Retrieve the island seed from the game instance
		Seed = IIslandPluginInterface::Execute_IslandSeed(GameInstance);
	}

	AsyncLoadClasses();
}

void ASpawner::AsyncLoadClasses()
{
	if (SpawnTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnTypes is empty."));
		bAsyncComplete = true;
		return;
	}

	ClassRefIndex = 0;
	bAsyncComplete = false;
	AsyncClass();
}

void ASpawner::AsyncClass()
{
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(SpawnTypes[ClassRefIndex].ClassRef.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &ASpawner::OnAsyncClassCompleted));
}

void ASpawner::OnAsyncClassCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Async load completed for class index: %d"), ClassRefIndex);

	ClassRefIndex++;

	// If we've loaded all classes, mark it complete and optionally auto-spawn
	if (ClassRefIndex >= SpawnTypes.Num())
	{
		bAsyncComplete = true;

		if (bAutoSpawn)
		{
			SpawnRandom();
		}
	}
	else
	{
		AsyncClass();
	}
}

void ASpawner::SpawnRandom()
{

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
			Counter = 0;
			IndexCounter = 0;
	});

	FTimerDelegate NavCheckDelegate;
	NavCheckDelegate.BindUFunction(this, FName("ReadyToSpawn"));

	// Set a recurring timer (0.5s interval) to check if we're ready to spawn
	// The initial delay is -0.5s, effectively meaning “start almost immediately”
	GetWorld()->GetTimerManager().SetTimer(NavCheckHandle, NavCheckDelegate, 0.5f, true, -0.5f);
}

void ASpawner::ReadyToSpawn()
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSystem invalid!"));
		return;
	}

	// Check if the navigation system is not being built or locked
	if (bAsyncComplete && NavSystem && !NavSystem->IsNavigationBeingBuiltOrLocked(this))
	{
		GetWorld()->GetTimerManager().PauseTimer(NavCheckHandle);

		if (bActorSwitch)
		{
			if (SpawnTypes.IsValidIndex(IndexCounter))
			{
				TSubclassOf<AActor> SpawnObject = SpawnTypes[IndexCounter].ClassRef.LoadSynchronous();
				GenerateAssets(SpawnObject, SpawnTypes[IndexCounter]);
				IndexCounter++;
				if (IndexCounter >= SpawnTypes.Num())
				{
					IndexCounter = 0;
					bActorSwitch = false;
				}
				else
				{
					GetWorld()->GetTimerManager().UnPauseTimer(NavCheckHandle);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("IndexCounter is out of bounds for SpawnTypes."));
			}
		}
		else
		{
			if (SpawnInstances.IsValidIndex(IndexCounter))
			{
				UInstancedStaticMeshComponent* InstancedMeshComp = NewObject<UInstancedStaticMeshComponent>(this);
				if (InstancedMeshComp)
				{
					InstancedMeshComp->RegisterComponent();
					InstancedMeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
					InstancedMeshComp->SetStaticMesh(SpawnInstances[IndexCounter].ClassMeshRef);
					GenerateInstances(InstancedMeshComp, SpawnInstances[IndexCounter].BiomeScale, SpawnInstances[IndexCounter].BiomeCount, SpawnInstances[IndexCounter].SpawnPerBiome);
				}
				IndexCounter++;
				if (IndexCounter >= SpawnTypes.Num())
				{
					FinishSpawning();
				}
				else
				{
					GetWorld()->GetTimerManager().UnPauseTimer(NavCheckHandle);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("IndexCounter is out of bounds for SpawnInstances."));
			}
		}
	}
}

void ASpawner::GenerateAssets(TSubclassOf<AActor> Class, FSpawnData SpawnParams)
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSystem invalid!"));
		return;
	}

	for (int32 i = 0; i < SpawnParams.BiomeCount; i++)
	{
		FVector RandomLocation = FVector(0);

		FNavLocation NavLocationFirst;

		bool bFoundFirst = NavSystem->GetRandomPointInNavigableRadius(FVector(0.0f, 0.0f, 0.0f), 10000.f, NavLocationFirst, NavData);
		if (bFoundFirst)
		{
			RandomLocation = NavLocationFirst.Location;
		}

		int32 MaxNumberOfLocations = UKismetMathLibrary::RandomIntegerInRangeFromStream(Seed , 0, SpawnParams.SpawnPerBiome);

		for (int32 j = 0; j < MaxNumberOfLocations; j++)
		{
			FVector RandomSpawnLocation = FVector(0);

			FNavLocation NavLocationSecond;

			bool bFoundSecond = NavSystem->GetRandomPointInNavigableRadius(RandomLocation, SpawnParams.BiomeScale, NavLocationSecond, NavData);
			if (bFoundSecond)
			{
				RandomSpawnLocation = NavLocationSecond.Location;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Navigation Mesh is not working"))
			}

			FRotator RandomRotation = FRotator(0.0f, 0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, SpawnParams.RandomRotationRange));
			FVector RandomScale = FVector(UKismetMathLibrary::RandomFloatInRange(1.0f, SpawnParams.ScaleRange + 1.0f));
			FTransform FinalSpawnLocation = UKismetMathLibrary::MakeTransform(SteppedPosition(RandomSpawnLocation), RandomRotation, RandomScale);

			UWorld* World = GetWorld();
			if (!World) { return; }

			World->SpawnActor<AActor>(Class, FinalSpawnLocation);
		}
	}
}

void ASpawner::GenerateInstances(UInstancedStaticMeshComponent* MeshClass, float Radius, int32 BiomeCount, int32 MaxSpawn)
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSystem invalid!"));
		return;
	}

	for (int32 i = 0; i < BiomeCount; i++)
	{
		FVector RandomLocation = FVector(0);

		FNavLocation NavLocationFirst;

		bool bFoundFirst = NavSystem->GetRandomPointInNavigableRadius(FVector(0.0f, 0.0f, 0.0f), 10000.f, NavLocationFirst, NavData);
		if (bFoundFirst)
		{
			RandomLocation = NavLocationFirst.Location;
		}

		int32 MaxNumberOfLocations = UKismetMathLibrary::RandomIntegerInRangeFromStream(Seed, 0, MaxSpawn);

		for (int32 j = 0; j < MaxNumberOfLocations; j++)
		{
			FVector RandomSpawnLocation = FVector(0);

			FNavLocation NavLocationSecond;

			bool bFoundSecond = NavSystem->GetRandomPointInNavigableRadius(RandomLocation, Radius, NavLocationSecond, NavData);
			if (bFoundSecond)
			{
				RandomSpawnLocation = NavLocationSecond.Location;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Navigation Mesh is not working"))
			}

			FRotator RandomRotation = FRotator(0.0f, 0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.f));
			FVector RandomScale = FVector(UKismetMathLibrary::Lerp(0.8f, 1.5f, UKismetMathLibrary::SafeDivide(UKismetMathLibrary::Subtract_VectorVector(RandomLocation, RandomSpawnLocation).Length(), Radius)));
			FTransform FinalSpawnLocation = UKismetMathLibrary::MakeTransform(SteppedPosition(RandomSpawnLocation), RandomRotation, RandomScale);

			MeshClass->AddInstance(FinalSpawnLocation, true);

			Counter++;
		}
	}
}

void ASpawner::FinishSpawning()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawn finished"))
}

FVector ASpawner::SteppedPosition(FVector Param)
{
	// Snap the X and Y coordinates to multiples of Step
	float RoundedX = FMath::RoundToFloat(Param.X / Step) * Step;
	float RoundedY = FMath::RoundToFloat(Param.Y / Step) * Step;

	return FVector(RoundedX, RoundedY, 0.0f);
}