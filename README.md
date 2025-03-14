# UE5 Procedural Island Example

This small project demonstrates **procedural island generation** in **Unreal Engine 5** using the new **`UDynamicMesh`** objects and related **Geometry Script APIs**. The system dynamically constructs 3D island meshes at runtime (or in-editor) based on various parameters and a seed-based randomization approach.

The project is written entirely in **C++** and uses a custom `AIslandConstructor` actor to generate and render the island geometry. You can place one of these actors in your level, set some properties (e.g., island size, number of islands, tessellation level, etc.), and then watch it create custom terrain shapes.

![Screenshot of my project](images/Island1.PNG "My Project Screenshot")

![Screenshot of my project](images/Island2.PNG "My Project Screenshot")

---

## About the Project

The goal was to explore **performance**, **usability**, and **capabilities** of the **Geometry Script** APIs, as well as to gain a deeper understanding of **procedural terrain generation** in UE5. The project:

1. **Retrieves a Seed** from a `GameInstance` interface (or uses a default seed) to ensure that each island generation is repeatable for the same seed.  
2. **Generates Multiple Islands** by spawning various conical shapes, merging them with a base plane or box, then performing mesh modifications (smoothing, solidification, plane cuts) to create landforms.  
3. **Applies Planar UV Projection** so that materials and textures can be applied easily.  
4. **Provides Configurable Parameters** via the actor’s Details Panel (e.g., maximum number of islands, island radius range, height, tessellation level).  
5. **Notifies Other Systems** (e.g., `GameMode`) once generation completes, enabling you to proceed with additional gameplay or rendering logic.

---

## Using the Code

If you want to explore or adapt this procedural island code in your own project, follow these steps:

1. **Copy and Paste Plugin**
   - Create folder named **Plugins** in your project.
   - Move the plugin to that folder.
     
2. **Include/Enable Plugin**
   - In your `.Build.cs` file, add `"IslandGenerator"` to your `PublicDependencyModuleNames`.
   - Enable the **IslandGenerator** plugin in your project’s settings.

2. **Look at the Actor Class**  
   - Start with [`IslandConstructor.h`](IslandGenerator/Source/IslandGenerator/Private/IslandConstructor.h) and [`IslandConstructor.cpp`](IslandGenerator/Source/IslandGenerator/Private/IslandConstructor.cpp).
   - The class `AIslandConstructor` extends `ADynamicMeshActor`. When placed in a level, it runs various Geometry Script functions to construct and shape the island mesh on the fly.

3. **Set Properties in the Details Panel**  
   - Choose the number of islands (`MaxNumberOfIslands`), approximate size range (`IslandSize`), height (`IslandHeight`), etc.
   - Adjust the **voxel “solidify”** resolution (`IslandGridResolution`) and **PN tessellation** level (`IslandTessellationLevel`) for performance vs. visual fidelity.

![DetailsPanel](images/DetailsPanel.PNG "Details Panel")

4. **Set Collision**
   - Change collision type to `Use Complex Collision As Simple`.
   - Check `Enable Complex Collision`.

![Collision](images/tutorial1.PNG "Collision")

5. **Regenerate**  
   - Whenever you re-compile, or if you add an in-editor function call, the mesh can be regenerated to reflect your new parameters.
   - If you’re using a **random seed**, the resulting islands will be consistent for the same seed but different across different seeds.

6. **Add your own **`GameInstance`**** 
   - Inherit from **`IIslandPluginInterface`** in your **`UGameInstance`** class.
   
```cpp

UCLASS()
class UYourGameInstance : public UGameInstance, public IIslandPluginInterface
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	virtual FRandomStream IslandSeed_Implementation() const override;
	
private:
	int32 RandomSeed;
};
```

Example logic for random seed

```cpp
void URTSGameInstance::Init()
{
	RandomSeed = UKismetMathLibrary::RandomInteger(2147483646);
}

FRandomStream URTSGameInstance::IslandSeed_Implementation() const
{
	return FRandomStream(RandomSeed);
}
```
---

## Geometry Script Highlights

Below are some of the core Geometry Script functions demonstrated in this project:

- **`AppendCone()` / `AppendBox()`** – Creates base island shapes and large ground pieces.  
- **`ApplyMeshSolidify()`** – Voxelizes the mesh for a solid volume.  
- **`ApplyIterativeSmoothingToMesh()`** – Smooths the mesh to remove sharp edges or artifacts.  
- **`ApplyMeshPlaneCut()`** – Cuts flat planes at the bottom or top to shape the island.  
- **`SetMeshUVsFromPlanarProjection()`** – Assigns UV coordinates for texturing.  

For more advanced usage, explore these headers:

```cpp
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshVoxelFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
```
## Additionally, some information about the spawner

1. **Add **`NavMeshBoundsVolume`** to your editor**
   - Set Brush Settings so that **NavMesh** covers the entire island and is at the correct height.
   - Run the game.

![BrushSettings](images/BrushSettings.PNG "Brush Settings")
