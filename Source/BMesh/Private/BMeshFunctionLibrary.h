/*
 * Copyright (c) 2020 -- Daniel Amthauer
 * 
 * Based on BMesh for Unity by Élie Michel (c) 2020, original copyright info included below
 * as specified by the original license terms. Those terms also apply to this version.
 */

/*
 * Copyright (c) 2020 -- Élie Michel <elie@exppad.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BMeshFunctionLibrary.generated.h"

class UBMeshFace;
class UBMeshEdge;
class UBMesh;
class UBMeshVertex;

UCLASS()
class UBMeshFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * Subdivide a mesh, without smoothing it, trying to interpolate all
	 * available attributes as much as possible. After subdivision, all faces
	 * are quads.
	 * Overriding attributes: edge's id
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators")
	static void Subdivide(UBMesh* mesh);

	/**
	 * Subdivide triangular faces into 4 equal triangles
	 * Only works on meshes that only have triangular faces
	 * Interpolates attributes for vertices
	 * @retval whether the mesh was subdivided correctly or not
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators")
	static bool Subdivide3(UBMesh* mesh);

	/**
	 * Merge two faces separated by an edge
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators")
	static bool MergeFaces(UBMesh* mesh, UBMeshEdge* Edge);

	/**
	 * Try to make quads as square as possible (may be called iteratively).
	 * This is not a very common operation but was developed so I keep it here.
	 * This assumes that the mesh is only made of quads.
	 * Overriding attributes: vertex's id
	 * Optionally read vertex attributes:
	 *   - RestPos: a FVector telling which position attracts the vertex
	 *   - Weight: a float telling to which extent the RestPos must be
	 *             considered.
	 *   
	 * @param rate speed at which faces are squarified. A higher rate goes
	 *        faster but there is a risk for overshooting.
	 * @param uniformLength whether the size of the quads must be uniformized.
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators")
	static void SquarifyQuads(UBMesh* mesh, float rate = 1.0f, bool uniformLength = false);

	/**
	 * Subdivides all faces in array into one triangle for each edge, starting from the original face's center
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators")
	static void SubdivideTriangleFan(TArray<UBMeshFace*> Faces);

	/**
	 * Subdivides all faces into one triangle for each edge, starting from the original face's center
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators", meta = (DisplayName="Subdivide Triangle Fan"))
	static void SubdivideTriangleFanAllFaces(UBMesh* mesh);

	/**
	 * Subdivides face into one triangle for each edge, starting from the original face's center
	 */
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators", meta = (DisplayName="Subdivide Triangle Fan"))
	static void SubdivideTriangleFanSingle(UBMeshFace* Face);
	
	UFUNCTION(BlueprintCallable, Category = "BMesh|Operators", meta=(WorldContext=WorldContextObject))
	static void DrawDebugBMesh(UObject* WorldContextObject, FTransform LocalToWorld, UBMesh* mesh);
};
