// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "SpawnInstance.generated.h"

USTRUCT(BlueprintType)
struct FSpawnInstance
{
    GENERATED_USTRUCT_BODY()

    /** A reference to the Static Mesh you want to spawn or track */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TObjectPtr<UStaticMesh> ClassMeshRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 BiomeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 SpawnPerBiome;
};