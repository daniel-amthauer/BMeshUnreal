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

#include "BMeshEdge.h"

#include "BMeshVertex.h"
#include "BMeshLoop.h"

UBMeshEdge* UBMeshEdge::MakeEdge(TSubclassOf<UBMeshEdge> EdgeClass, UBMeshVertex* Vertex1, UBMeshVertex* Vertex2)
{
	if (EdgeClass)
	{
		UBMeshEdge* NewEdge = NewObject<UBMeshEdge>(Vertex1->GetOuter(), *EdgeClass);
		NewEdge->Vert1 = Vertex1;
		NewEdge->Vert2 = Vertex2;
		return NewEdge;
	}
	return nullptr;
}

bool UBMeshEdge::ContainsVertex(UBMeshVertex* v) const
{
	return v == Vert1 || v == Vert2;
}

UBMeshVertex* UBMeshEdge::OtherVertex(UBMeshVertex* v) const
{
	check(ContainsVertex(v));
	return v == Vert1 ? Vert2 : Vert1;
}

UBMeshEdge* UBMeshEdge::Next(UBMeshVertex* v) const
{
	check(ContainsVertex(v));
	return v == Vert1 ? Next1 : Next2;
}

void UBMeshEdge::SetNext(UBMeshVertex* v, UBMeshEdge* other)
{
	check(ContainsVertex(v));
	if (v == Vert1) Next1 = other;
	else Next2 = other;
}

UBMeshEdge* UBMeshEdge::Prev(UBMeshVertex* v) const
{
	check(ContainsVertex(v));
	return v == Vert1 ? Prev1 : Prev2;
}

void UBMeshEdge::SetPrev(UBMeshVertex* v, UBMeshEdge* other)
{
	check(ContainsVertex(v));
	if (v == Vert1) Prev1 = other;
	else Prev2 = other;
}

TArray<UBMeshFace*> UBMeshEdge::NeighborFaces() const
{
	TArray<UBMeshFace*> Faces;
	if (Loop)
	{
		UBMeshLoop* It = Loop;
		do
		{
			Faces.Add(It->Face);
			It = It->RadialNext;
		}
		while (It != Loop);
	}
	return Faces;
}

FVector UBMeshEdge::Center() const
{
	return (Vert1->Location + Vert2->Location) * 0.5f;
}
