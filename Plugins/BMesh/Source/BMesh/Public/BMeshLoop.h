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

#include "BMeshLoop.generated.h"

class UBMeshVertex;
class UBMeshEdge;
class UBMeshFace;

/**
 * Since a face is basically a list of edges, and the Loop object is a node
 * of this list, called so because the list must loop.
 * A loop is associated to one and only one face.
 * 
 * A loop can be seen as a list of edges, it also stores a reference to a
 * vertex for commodity but technically it could be found through the edge.
 * It may also be interpreted as a "face corner", and us hence where one
 * typically stores UVs, because a same vertex may have different UV
 * coordinate depending on the face.
 * 
 * On top of this, the loop is also used as a node of another linked list,
 * namely the radial list, that enables iterating over all the face using
 * the same edge.
 */
UCLASS()
class BMESH_API UBMeshLoop : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UBMeshVertex* Vert;

	UPROPERTY()
	UBMeshEdge* Edge;

	UPROPERTY()
	UBMeshFace* Face; // there is exactly one face using a loop

	UPROPERTY()
	UBMeshLoop* RadialPrev; // around edge

	UPROPERTY()
	UBMeshLoop* RadialNext;

	UPROPERTY()
	UBMeshLoop* Prev; // around face

	UPROPERTY()
	UBMeshLoop* Next;

	static UBMeshLoop* MakeLoop(TSubclassOf<UBMeshLoop> LoopClass, UBMeshVertex* Vertex, UBMeshEdge* Edge, UBMeshFace* Face);

protected:
	/**
	 * Insert the loop in the linked list of the face.
	 * (Used in constructor)
	 */
	void SetFace(UBMeshFace* f);

	/**
	 * Insert the loop in the radial linked list.
	 * (Used in constructor)
	 */
	void SetEdge(UBMeshEdge* e);
};
