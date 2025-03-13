// The source code, authored by Zoxemik in 2025

#include "Miscellaneous/SpawnMarker.h"

ASpawnMarker::ASpawnMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
}

