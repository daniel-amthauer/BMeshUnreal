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
 
#include "BMesh.h"

#include "BMeshVertex.h"
#include "BMeshEdge.h"
#include "BMeshLoop.h"
#include "BMeshFace.h"

UBMeshVertex* UBMesh::AddVertex(UBMeshVertex* vert)
{
	vertices.Add(vert);
	return vert;
}

UBMeshVertex* UBMesh::AddVertex(FVector Location)
{
    UBMeshVertex* Vertex = NewObject<UBMeshVertex>(this, *VertexClass);
    Vertex->Location = Location;
    return AddVertex(Vertex);
}

UBMeshVertex* UBMesh::AddVertex(float x, float y, float z)
{
	return AddVertex(FVector(x, y, z));
}

UBMeshEdge* UBMesh::AddEdge(UBMeshVertex* vert1, UBMeshVertex* vert2)
{
	check(vert1 != vert2);

	UBMeshEdge* edge = FindEdge(vert1, vert2);
	if (edge != nullptr) return edge;

	edge = UBMeshEdge::MakeEdge(EdgeClass, vert1, vert2);
	edges.Add(edge);

	// Insert in vert1's edge list
	if (vert1->Edge == nullptr)
	{
		vert1->Edge = edge;
		edge->Next1 = edge->Prev1 = edge;
	}
	else
	{
		edge->Next1 = vert1->Edge->Next(vert1);
		edge->Prev1 = vert1->Edge;
		edge->Next1->SetPrev(vert1, edge);
		edge->Prev1->SetNext(vert1, edge);
	}

	// Same for vert2 -- TODO avoid code duplication
	if (vert2->Edge == nullptr)
	{
		vert2->Edge = edge;
		edge->Next2 = edge->Prev2 = edge;
	}
	else
	{
		edge->Next2 = vert2->Edge->Next(vert2);
		edge->Prev2 = vert2->Edge;
		edge->Next2->SetPrev(vert2, edge);
		edge->Prev2->SetNext(vert2, edge);
	}

	return edge;
}

UBMeshFace* UBMesh::AddFace(TArrayView<UBMeshVertex*> fVerts)
{
	if (fVerts.Num() == 0) return nullptr;
	for (auto v : fVerts) check(v != nullptr);

	TArray<UBMeshEdge*, TInlineAllocator<6>> fEdges;
	fEdges.SetNum(fVerts.Num());

	int i, i_prev = fVerts.Num() - 1;
	for (i = 0; i < fVerts.Num(); ++i)
	{
		fEdges[i_prev] = AddEdge(fVerts[i_prev], fVerts[i]);
		i_prev = i;
	}

	UBMeshFace* f = NewObject<UBMeshFace>(this, *FaceClass);
	faces.Add(f);

	for (i = 0; i < fVerts.Num(); ++i)
	{
		UBMeshLoop* loop = UBMeshLoop::MakeLoop(LoopClass, fVerts[i], fEdges[i], f);
		loops.Add(loop);
	}

	f->VertCount = fVerts.Num();
	return f;
}

UBMeshEdge* UBMesh::FindEdge(UBMeshVertex* vert1, UBMeshVertex* vert2)
{
	check(vert1 != vert2);
	if (vert1->Edge == nullptr || vert2->Edge == nullptr) return nullptr;

	UBMeshEdge* e1 = vert1->Edge;
	UBMeshEdge* e2 = vert2->Edge;
	do
	{
		if (e1->ContainsVertex(vert2)) return e1;
		if (e2->ContainsVertex(vert1)) return e2;
		e1 = e1->Next(vert1);
		e2 = e2->Next(vert2);
	}
	while (e1 != vert1->Edge && e2 != vert2->Edge);
	return nullptr;
}

void UBMesh::RemoveVertex(UBMeshVertex* v)
{
	check(vertices.Contains(v));
	while (v->Edge != nullptr)
	{
		RemoveEdge(v->Edge);
	}

	vertices.Remove(v);
}

void UBMesh::RemoveEdge(UBMeshEdge* e)
{
	check(edges.Contains(e));
	while (e->Loop != nullptr)
	{
		RemoveLoop(e->Loop);
	}

	// Remove reference in vertices
	if (e == e->Vert1->Edge) e->Vert1->Edge = e->Next1 != e ? e->Next1 : nullptr;
	if (e == e->Vert2->Edge) e->Vert2->Edge = e->Next2 != e ? e->Next2 : nullptr;

	// Remove from linked lists
	e->Prev1->SetNext(e->Vert1, e->Next1);
	e->Next1->SetPrev(e->Vert1, e->Prev1);

	e->Prev2->SetNext(e->Vert2, e->Next2);
	e->Next2->SetPrev(e->Vert2, e->Prev2);

	edges.Remove(e);
}

void UBMesh::RemoveLoop(UBMeshLoop* l)
{
	if (l->Face != nullptr) // nullptr iff loop is called from RemoveFace
	{
		// Trigger removing other loops, and this one again with l->face == nullptr
		RemoveFace(l->Face);
		return;
	}

	// remove from radial linked list
	if (l->RadialNext == l)
	{
		l->Edge->Loop = nullptr;
	}
	else
	{
		l->RadialPrev->RadialNext = l->RadialNext;
		l->RadialNext->RadialPrev = l->RadialPrev;
		if (l->Edge->Loop == l)
		{
			l->Edge->Loop = l->RadialNext;
		}
	}

	// forget other loops of the same face so thet they get released from memory
	l->Next = nullptr;
	l->Prev = nullptr;

	loops.Remove(l);
}

void UBMesh::RemoveFace(UBMeshFace* f)
{
	check(faces.Contains(f));
	UBMeshLoop* l = f->FirstLoop;
	UBMeshLoop* nextL = nullptr;
	while (nextL != f->FirstLoop)
	{
		nextL = l->Next;
		l->Face = nullptr; // prevent infinite recursion, because otherwise RemoveLoop calls RemoveFace
		RemoveLoop(l);
		l = nextL;
	}
	faces.Remove(f);
}

UBMesh::FMakeParams::FMakeParams()
{
	VertexClass = UBMeshVertex::StaticClass();
	EdgeClass = UBMeshEdge::StaticClass();
	LoopClass = UBMeshLoop::StaticClass();
	FaceClass = UBMeshFace::StaticClass();
}

UBMesh* UBMesh::Make(UObject* Outer, FMakeParams Params)
{
	UBMesh* NewMesh = NewObject<UBMesh>(Outer);
	NewMesh->VertexClass = Params.VertexClass;
	NewMesh->EdgeClass = Params.EdgeClass;
	NewMesh->LoopClass = Params.LoopClass;
	NewMesh->FaceClass = Params.FaceClass;
	return NewMesh;
}
