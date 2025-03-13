// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "SpawnData.generated.h"

USTRUCT(BlueprintType)
struct FSpawnData
{
    GENERATED_USTRUCT_BODY()

    /** A reference to the Actor class you want to spawn or track */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftClassPtr<AActor> ClassRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 BiomeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 SpawnPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RandomRotationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ScaleRange;
    
};