# UE5 Procedural Island Example

This small project demonstrates **procedural island generation** in **Unreal Engine 5** using the new **`UDynamicMesh`** objects and related **Geometry Script APIs**. The system dynamically constructs 3D island meshes at runtime (or in-editor) based on various parameters and a seed-based randomization approach.

The project is written entirely in **C++** and uses a custom `AIslandConstructor` actor to generate and render the island geometry. You can place one of these actors in your level, set some properties (e.g., island size, number of islands, tessellation level, etc.), and then watch it create custom terrain shapes.

![Screenshot of my project](docs/Island1.png "My Project Screenshot")

---

## About the Project

The goal was to explore **performance**, **usability**, and **capabilities** of the **Geometry Script** APIs, as well as to gain a deeper understanding of **procedural terrain generation** in UE5. The project:

1. **Retrieves a Seed** from a `GameInstance` interface (or uses a default seed) to ensure that each island generation is repeatable for the same seed.  
2. **Generates Multiple Islands** by spawning various conical shapes, merging them with a base plane or box, then performing mesh modifications (smoothing, solidification, plane cuts) to create landforms.  
3. **Applies Planar UV Projection** so that materials and textures can be applied easily.  
4. **Provides Configurable Parameters** via the actor’s Details Panel (e.g., maximum number of islands, island radius range, height, tessellation level).  
5. **Notifies Other Systems** (e.g., `GameMode`) once generation completes, enabling you to proceed with additional gameplay or rendering logic.

This project was built and tested with **Unreal Engine 5.0.3**, but newer engine versions may require minor modifications to the Geometry Script calls.

---

## Using the Code

If you want to explore or adapt this procedural island code in your own project, follow these steps:

1. **Include/Enable Geometry Script**  
   - In your `.Build.cs` file, add `"GeometryScriptingEditor"`, `"GeometryScriptingCore"`, and `"GeometryCore"` to your `PublicDependencyModuleNames`.
   - Enable the **Geometry Script** plugin in your project’s Plugin settings.

2. **Look at the Actor Class**  
   - Start with [`IslandConstructor.h`](IslandConstructor.h) and [`IslandConstructor.cpp`](IslandConstructor.cpp).
   - The class `AIslandConstructor` extends `ADynamicMeshActor`. When placed in a level, it runs various Geometry Script functions to construct and shape the island mesh on the fly.

3. **Set Properties in the Details Panel**  
   - Choose the number of islands (`MaxNumberOfIslands`), approximate size range (`IslandSize`), height (`IslandHeight`), etc.
   - Adjust the **voxel “solidify”** resolution (`IslandGridResolution`) and **PN tessellation** level (`IslandTessellationLevel`) for performance vs. visual fidelity.

4. **Regenerate**  
   - Whenever you re-compile, or if you add an in-editor function call, the mesh can be regenerated to reflect your new parameters.
   - If you’re using a **random seed**, the resulting islands will be consistent for the same seed but different across different seeds.

*(Optional: Insert a screenshot of your Details Panel here.)*

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
