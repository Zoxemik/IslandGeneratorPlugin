// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "IslandConstructor.generated.h"

class ASpawnMarker;

UCLASS()
class AIslandConstructor : public ADynamicMeshActor
{
	GENERATED_BODY()
	
public:
	AIslandConstructor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void CreateIsland(bool SpawnMarkers);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	TSubclassOf<ASpawnMarker> SpawnMarkerBlueprint; // Always points to the center of the biome, useful for spawning objects in specific locations

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FRandomStream Seed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	int32 MaxNumberOfIslands;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float IslandRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float IslandHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	float MaxSpawnDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FVector2D IslandSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ToolTip = "Used to set the final island mesh resolution, useful for optimization"))
	int32 IslandGridResolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (ToolTip = "Used to subdivide a surface into smaller polygons, useful for optimization"))
	int32 IslandTessellationLevel;
private:
	UPROPERTY()
	TObjectPtr<UDynamicMesh> DynamicMesh;

	UPROPERTY()
	TArray<FVector> SpawnPoints;

};
