# BMeshUnreal
Based on BMesh for Unity (https://github.com/eliemichel/BMeshUnity)
It provides a half-edge data structure inspired by Blender's BMesh, which makes many mesh manipulation operations simpler.

It's useful when using mesh data for logical instead of visual purposes (e.g. irregular grids). It might also be used as a more flexible intermediate representation for certain mesh operators, but this use case is currently missing a conversion operator to regular Unreal Engine mesh structures.

It is accessible from Blueprints and each of the mesh elements can be customized to carry more information, which will be automatically interpolated by operations such as subdivisions as long as it's of one of the following types:
- Int
- Float/Double
- FVector, FVector2D, FVector4
- FLinearColor

This can be extended by the user with other types, but it must be done from C++ (See ['here'](Source/BMesh/Private/BMeshModule.cpp) and ['here'](Source/BMesh/Private/BMeshOperators.cpp) for an example of how this is done with the *FBMeshOperators::RegisterDefaultTypeInterpolators* function.

## Usage
See [`UBMeshTestComponent`](Source/BMeshTest/Private/BMeshTest.cpp) in the BMeshTests module for examples of how to create meshes with it from C++ and how to visualize them in the editor. 

To use from Blueprints simply create a BMesh object, and select your preferred class for each mesh element. The default classes include only a location for vertices, and no extra attributes for any other elements, aside from an Id integer which can be used as a temporary attribute in many mesh operators.

Here's an example that produces a hexagonal face that's then subdivided into a triangle fan
![Blueprint code that creates a BMesh object with a hexagonal face which is subdivided into a triangle fan](Docs/BlueprintExample.png?raw=true)

This is the result

![Result of the previous code visualized with debug lines](Docs/BlueprintResult.png?raw=true)

There are several operators included to subdivide the mesh in different ways, as well as a few others.


## Requirements
Requires Unreal Engine 4.25.x or greater

## Installation
Copy the BMesh folder to your project's Plugins folder. The editor should prompt you to compile the plugin when opening your project.
If you're using Git as your source control, you can also add the repo (or a fork of it) directly as a submodule to your Plugins folder.

## Future work 
~~- Conversion to RuntimeMeshComponent~~
- In-editor conversion to StaticMesh, and/or DynamicMesh
