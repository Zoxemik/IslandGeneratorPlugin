// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "SpawnData.h"
#include "SpawnInstance.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Spawner.generated.h"

class ANavigationData;

UCLASS()
class ISLANDGENERATOR_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawner();
	virtual void BeginPlay() override;

	UFUNCTION()
	void SpawnRandom();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TArray<FSpawnData> SpawnTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TArray<FSpawnInstance> SpawnInstances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TObjectPtr<ANavigationData> NavData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float Step; // Snapps spawned object to a grid

private:
	UFUNCTION()
	void AsyncLoadClasses();
	UFUNCTION()
	void AsyncClass();
	UFUNCTION()
	void OnAsyncClassCompleted();

	UFUNCTION()
	void ReadyToSpawn();

	UFUNCTION()
	void GenerateAssets(TSubclassOf<AActor> Class, FSpawnData SpawnParams);
	UFUNCTION()
	void GenerateInstances(UInstancedStaticMeshComponent* MeshClass, float Radius, int32 BiomeCount, int32 MaxSpawn);

	UFUNCTION()
	void FinishSpawning();

	UFUNCTION()
	FVector SteppedPosition(FVector Param);

	bool bAsyncComplete;
	bool bAutoSpawn;
	bool bActorSwitch;

	int32 ClassRefIndex;
	int32 IndexCounter;
	int32 Counter;

	FTimerHandle NavCheckHandle;

	FRandomStream Seed;
};
