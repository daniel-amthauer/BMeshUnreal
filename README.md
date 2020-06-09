# BMeshUnreal
An Unreal Engine 4 plugin to make runtime procedural mesh generation more flexible. Based on BMesh for Unity (https://github.com/eliemichel/BMeshUnity)

## Requirements
Requires Unreal Engine 4.25.x
The code could be made to support 4.24.x and earlier with some changes, but it's currently dependent on 4.25's FProperty instead of the previous version's UProperty.

## Installation
Copy the BMesh folder (from inside the Plugins folder) to your project's Plugins folder. The editor should prompt you to compile the plugin when opening your project.

## Usage
Currently the plugin can only be used from C++. See [`UBMeshTestComponent`](https://github.com/daniel-amthauer/BMeshUnreal/blob/master/Plugins/BMesh/Source/BMeshTest/Private/BMeshTest.cpp) in the BMeshTests module for examples of how to create meshes with it and how to visualize them in the editor. 

## Future work 
### (coming soon)
- Blueprint interface for simple prototyping
- Conversion to RuntimeMeshComponent

### (longer term)
- In-editor conversion to StaticMesh
