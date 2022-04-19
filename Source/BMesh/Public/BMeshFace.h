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

#include "BMeshFace.generated.h"

class UBMeshVertex;
class UBMeshLoop;
class UBMeshEdge;

/**
* A face is almost nothing more than a loop. Having a different structure
* makes sens only 1. for clarity, because loops are a less intuitive
* object and 2. to store face attributes.
*/
UCLASS(BlueprintType, Blueprintable)
class BMESH_API UBMeshFace : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category="Bmesh Face")
	int Id; // [attribute]

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Face")
	int VertCount; // stored for commodity, can be recomputed easily

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Face")
	UBMeshLoop* FirstLoop; // navigate list using next

	/**
    * Get the list of vertices used by the face, ordered.
    */
	UFUNCTION(BlueprintPure)
	TArray<UBMeshVertex*> NeighborVertices();

	/**
	 * Assuming the vertex is part of the face, return the loop such that
	 * loop->Vert = v. Return null otherwise.
	 */
	UFUNCTION(BlueprintPure)
	UBMeshLoop* FindLoop(UBMeshVertex* v);

	/**
	 * Get the list of edges around the face.
	 * It is guaranteed to match the order of NeighborVertices(), so that
	 * edge[0] = Vert[0]-->Vert[1], edge[1] = Vert[1]-->Vert[2], etc.
	 */
	UFUNCTION(BlueprintPure)
	TArray<UBMeshEdge*> NeighborEdges();

	/**
	 * Compute the barycenter of the face vertices
	 */
	UFUNCTION(BlueprintPure)
	FVector Center();
};
