// The source code, authored by Zoxemik in 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IslandPluginInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIslandPluginInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IIslandPluginInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Island Generator")
	FRandomStream IslandSeed() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Island Generator")
	void IslandGenerationComplete() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Island Generator")
	void SpawningComplete();

};
