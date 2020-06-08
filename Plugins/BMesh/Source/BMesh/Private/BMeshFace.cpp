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

#include "BMeshFace.h"

#include "BMeshVertex.h"
#include "BMeshLoop.h"

TArray<UBMeshVertex*> UBMeshFace::NeighborVertices()
{
	TArray<UBMeshVertex*> verts;
	if (FirstLoop != nullptr)
	{
		UBMeshLoop* it = FirstLoop;
		do
		{
			verts.Add(it->Vert);
			it = it->Next;
		}
		while (it != FirstLoop);
	}
	return verts;
}

UBMeshLoop* UBMeshFace::FindLoop(UBMeshVertex* v)
{
	if (FirstLoop != nullptr)
	{
		UBMeshLoop* it = FirstLoop;
		do
		{
			check(it != nullptr);
			if (it->Vert == v) return it;
			it = it->Next;
		}
		while (it != FirstLoop);
	}
	return nullptr;
}

TArray<UBMeshEdge*> UBMeshFace::NeighborEdges()
{
	TArray<UBMeshEdge*> edges;
	if (FirstLoop != nullptr)
	{
		UBMeshLoop* it = FirstLoop;
		do
		{
			edges.Add(it->Edge);
			it = it->Next;
		}
		while (it != FirstLoop);
	}
	return edges;
}

FVector UBMeshFace::Center()
{
	FVector p = FVector::ZeroVector;
	float sum = 0;
	for (UBMeshVertex* v : NeighborVertices())
	{
		p += v->Location;
		sum += 1;
	}
	return p / sum;
}
