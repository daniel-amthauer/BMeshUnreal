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

#include "BMeshEdge.generated.h"

class UBMeshVertex;
class UBMeshLoop;
class UBMeshFace;

/**
 * An edge links to vertices together, and may or may not be part of a face.
 * An edge can be shared by several faces.
 * 
 * Technical Note: The structure stores a reference to the two vertices.
 * Although the role of these two vertices is perfectly symmetrical, this
 * makes the iterations over linked list slightly trickier than expected.
 * 
 * The edge is a node of two (double) linked lists at the same time. Let's
 * recall that a (simply) linked list of Stuff is made of nodes of the form
 *     Node {
 *         Stuff value;
 *         Node next;
 *     }
 * Here we provide two "next", depending on whether the vertex that we are
 * interested in is vertex1 or vertex2. Note that a vertex stored in the
 * "vertex1" field for one edge might be stored in the "vertex2" of the
 * next one, so the function Next() is provided to return either next1 or
 * next2 depending on the vertex of interest.
 */
UCLASS(BlueprintType, Blueprintable)
class BMESH_API UBMeshEdge : public UObject
{
	GENERATED_BODY()
public:
	// [attribute]
	UPROPERTY(BlueprintReadWrite, Category="Bmesh Edge")
	int Id;

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshVertex* Vert1;

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshVertex* Vert2;

	// next edge around vert1. If you don't know whether your vertex is vert1 or vert2, use Next(v)
	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshEdge* Next1;

	// next edge around vert1
	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshEdge* Next2;

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshEdge* Prev1;

	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshEdge* Prev2;

	// first node of the list of faces that use this edge. Navigate list using radial_next
	UPROPERTY(BlueprintReadOnly, Category="Bmesh Edge")
	UBMeshLoop* Loop;

	static UBMeshEdge* MakeEdge(TSubclassOf<UBMeshEdge> EdgeClass, UBMeshVertex* Vertex1, UBMeshVertex* Vertex2);

	/**
     * Tells whether a vertex is one of the extremities of this edge.
     */
	UFUNCTION(BlueprintPure)
	bool ContainsVertex(UBMeshVertex* v) const;

	/**
	 * If one gives a vertex of the edge to this function, it returns the
	 * other vertex of the edge. Otherwise, the behavior is undefined.
	 */
	UFUNCTION(BlueprintPure)
	UBMeshVertex* OtherVertex(UBMeshVertex* v) const;

	/**
	 * If one gives a vertex of the edge to this function, it returns the
	 * next edge in the linked list of edges that use this vertex.
	 */
	UFUNCTION(BlueprintPure)
	UBMeshEdge* Next(UBMeshVertex* v) const;

	/**
	 * This is used when inserting a new Edge in the lists.
	 */
	void SetNext(UBMeshVertex* v, UBMeshEdge* other);

	/**
	 * Similar to Next() but to go backward in the double-linked list
	 */
	UFUNCTION(BlueprintPure)
	UBMeshEdge* Prev(UBMeshVertex* v) const;

	/**
	 * Similar to SetNext()
	 */
	void SetPrev(UBMeshVertex* v, UBMeshEdge* other);

	/**
	 * Return all faces that use this edge as a side.
	 */
	UFUNCTION(BlueprintPure)
	TArray<UBMeshFace*> NeighborFaces() const;

	/**
	 * Compute the barycenter of the edge's vertices
	 */
	UFUNCTION(BlueprintPure)
	FVector Center() const;
};
