// The source code, authored by Zoxemik in 2025

#include "IslandConstructor.h"
#include "IslandPluginInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GeometryScript/MeshSubdivideFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshVoxelFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "Miscellaneous/SpawnMarker.h"

AIslandConstructor::AIslandConstructor()
{
	PrimaryActorTick.bCanEverTick = false;

	GetDynamicMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	GetDynamicMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Seed.Initialize(0);

	MaxNumberOfIslands = 20;

	IslandRadius = 0.0f;

	IslandHeight = 1300.f;

	MaxSpawnDistance = 9976.0f;

	IslandSize = FVector2D(800.f, 5000.f);

	IslandGridResolution = 50; //on high-end PC pref 50 on low-end 60

	IslandTessellationLevel = 2; //on highend PC pref 2 on lowend 0
}

void AIslandConstructor::BeginPlay()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (GameInstance->GetClass()->ImplementsInterface(UIslandPluginInterface::StaticClass()))
	{
		// Retrieve the island seed from the game instance
		Seed = IIslandPluginInterface::Execute_IslandSeed(GameInstance);

		CreateIsland(true);
	}
}

void AIslandConstructor::CreateIsland(bool SpawnMarkers)
{
	// Get the dynamic mesh component and reset it
	DynamicMesh = GetDynamicMeshComponent()->GetDynamicMesh();
    if (!DynamicMesh){ return; }
    DynamicMesh->Reset();

	// Clear the array of spawn points
	SpawnPoints.Empty();

	// Loop through the maximum number of islands
	for (int32 i = 0; i < MaxNumberOfIslands; ++i)
	{
		// Generate a random island radius within the specified range
		IslandRadius = UKismetMathLibrary::RandomFloatInRangeFromStream(Seed, IslandSize.X, IslandSize.Y);

		// Generate a random unit vector and scale it by half the maximum spawn distance
		FVector RandomVector = UKismetMathLibrary::RandomUnitVectorFromStream(Seed);
		FVector MaxSpawnDistanceVector = UKismetMathLibrary::Divide_VectorFloat(FVector(MaxSpawnDistance), 2.0f);

		// Calculate the random spawn point
		FVector RandomSpawnPoints = UKismetMathLibrary::Multiply_VectorVector(RandomVector, MaxSpawnDistanceVector);

		// Add the spawn point to the array and get its index
		int32 SpawnPointIndex = SpawnPoints.Add(FVector(RandomSpawnPoints.X, RandomSpawnPoints.Y, 0.0f));

		FVector SpawnPosition = SpawnPoints[SpawnPointIndex];

		FTransform IslandTransform = UKismetMathLibrary::MakeTransform(FVector(SpawnPosition.X, SpawnPosition.Y, -800.f), FRotator(0.0f));
		
		// Append a cone to the dynamic mesh at the specified transform
		FGeometryScriptPrimitiveOptions PrimitiveOptions;
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCone(DynamicMesh, PrimitiveOptions, IslandTransform, IslandRadius, IslandRadius / 4.0f, IslandHeight);

		// If spawn markers are enabled, spawn a marker at the spawn position
		if (SpawnMarkers)
		{
			UWorld* World = GetWorld();
			if (!World || !SpawnMarkerBlueprint)
			{
				UE_LOG(LogTemp, Error, TEXT("SpawnBlueprintSpawnMarker failed: World or SpawnMarkerBlueprint is NULL!"));
				return;
			}

			FTransform SpawnPositionTransform = UKismetMathLibrary::Conv_VectorToTransform(SpawnPosition);

			World->SpawnActor<ASpawnMarker>(SpawnMarkerBlueprint, SpawnPositionTransform);
		}
	}

	// Append a large box under all islands
	// make a box bigger than MaxSpawnDistance to act as a 'base'
	FTransform BoxTransform = UKismetMathLibrary::MakeTransform(FVector(0.0f, 0.0f, -800.f), FRotator(0.0f));
	float ExtendedMaxSpawnDistance = MaxSpawnDistance + 10000.f;

	FGeometryScriptPrimitiveOptions PrimitiveOptions;
	DynamicMesh = UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(DynamicMesh, PrimitiveOptions, BoxTransform, ExtendedMaxSpawnDistance, ExtendedMaxSpawnDistance, 400.f);

	// Fills in any cavities or ensures it's a solid volume
	// Uses IslandGridResolution for voxel resolution
	FGeometryScriptSolidifyOptions SolidifyOptions;
	SolidifyOptions.GridParameters.SizeMethod = EGeometryScriptGridSizingMethod::GridResolution;
	SolidifyOptions.GridParameters.GridCellSize = 0.25f;
	SolidifyOptions.GridParameters.GridResolution = IslandGridResolution;
	SolidifyOptions.bSolidAtBoundaries = false;
	SolidifyOptions.ExtendBounds = 0.0f;
	SolidifyOptions.SurfaceSearchSteps = 64;
	DynamicMesh = UGeometryScriptLibrary_MeshVoxelFunctions::ApplyMeshSolidify(DynamicMesh, SolidifyOptions);

	// Recompute normals to ensure smooth or consistent shading
	DynamicMesh = UGeometryScriptLibrary_MeshNormalsFunctions::SetPerVertexNormals(DynamicMesh);

	// Apply a mild smoothing pass to soften edges
	FGeometryScriptMeshSelection Selection;
	FGeometryScriptIterativeMeshSmoothingOptions SmoothingOptions;
	SmoothingOptions.NumIterations = 6;
	SmoothingOptions.Alpha = 0.2f;
	DynamicMesh = UGeometryScriptLibrary_MeshDeformFunctions::ApplyIterativeSmoothingToMesh(DynamicMesh, Selection, SmoothingOptions);

	// Apply PN Tessellation (subdivision) to increase mesh detail
	FGeometryScriptPNTessellateOptions TessellateOptions;
	DynamicMesh = UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyPNTessellation(DynamicMesh, TessellateOptions, IslandTessellationLevel);

	// Cut the underside of the mesh (PlaneCut) to flatten it
	FTransform BottomCutTransform = UKismetMathLibrary::MakeTransform(FVector(0.0f, 0.0f, -390.f), FRotator(180.f, 0.0f, 0.0f));
	FGeometryScriptMeshPlaneCutOptions BottomCutOptions;
	BottomCutOptions.bFillHoles = false;
	BottomCutOptions.HoleFillMaterialID = -1;
	DynamicMesh = UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(DynamicMesh, BottomCutTransform, BottomCutOptions);

	// Cut/flatten the top of the mesh
	FTransform TopCutTransform = UKismetMathLibrary::MakeTransform(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	FGeometryScriptMeshPlaneCutOptions TopCutOptions;
	DynamicMesh = UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(DynamicMesh, TopCutTransform, TopCutOptions);

	// Project UVs onto the mesh from a planar projection
	FTransform UVsPlaneTransform = UKismetMathLibrary::MakeTransform(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f), FVector(100.f, 100.f, 100.f));
	UGeometryScriptLibrary_MeshUVFunctions::SetMeshUVsFromPlanarProjection(DynamicMesh, 0, UVsPlaneTransform, Selection);

	// Clear compute meshes if used by geometry script
	ReleaseAllComputeMeshes();

	// Add a slight offset to the actor to avoid any potential z-fighting
	AddActorWorldOffset(FVector(0.0f, 0.0f, 0.05f));

	//Send Completed Event to GameMode
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode && GameMode->GetClass()->ImplementsInterface(UIslandPluginInterface::StaticClass()))
	{
		IIslandPluginInterface::Execute_IslandGenerationComplete(GameMode);
	}

}