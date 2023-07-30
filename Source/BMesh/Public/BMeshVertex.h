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

#include "BMeshVertex.generated.h"

class UBMeshEdge;
class UBMeshFace;

/**
* A vertex corresponds roughly to a position in space. Many primitives
* (edges, faces) can share a given vertex. Several vertices can be located
* at the very same position.
* A references a chained list of the edges that use it embeded inside the Edge
* structure (see bellow, and see implementation of NeighborEdges).
* The vertex position does not affect topological algorithm but is used by
* commodity functions that helps finding the center of an edge or a face.
*/
UCLASS(BlueprintType, Blueprintable)
class BMESH_API UBMeshVertex : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int Id;

	UPROPERTY(BlueprintReadWrite)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	UBMeshEdge* Edge;

	/**
     * List all edges reaching this vertex.
     */
	UFUNCTION(BlueprintPure, Category = "BMesh|Vertex")
	TArray<UBMeshEdge*> NeighborEdges() const;

	/**
     * Return all faces that use this vertex as a corner.
     */
	UFUNCTION(BlueprintPure, Category = "BMesh|Vertex")
	TArray<UBMeshFace*> NeighborFaces() const;

	//////////////////////////////////////////////
	/// Ranged for loop support
	//////////////////////////////////////////////

	struct FNeighborVerticesRangedForAdapter
	{
		const UBMeshVertex* Owner;
		
		struct BMESH_API FIterator
		{
			const UBMeshVertex* Owner;
			UBMeshEdge* Current;
			bool bFirst = true;
			void operator++();
			bool operator!=(const FIterator& Other) const
			{
				return Current != Other.Current || bFirst;
			}
			UBMeshVertex* operator*() const;
		};
		FIterator begin() const
		{
			FIterator It;
			It.Owner = Owner;
			It.Current = Owner->Edge;
			It.bFirst = true;
			return It;
		}
		FIterator end() const
		{
			FIterator It;
			It.Owner = nullptr;
			It.Current = Owner->Edge;
			It.bFirst = false;
			return It;
		}
	};
	
	FNeighborVerticesRangedForAdapter NeighborVerticesRange() const
	{
		FNeighborVerticesRangedForAdapter Adapter;
		Adapter.Owner = this;
		return Adapter;
	}
};
