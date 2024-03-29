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

#include "BMeshVertex.h"

#include "BMeshEdge.h"
#include "BMeshFace.h"

TArray<UBMeshEdge*> UBMeshVertex::NeighborEdges() const
{
	TArray<UBMeshEdge*> Edges;
	if (Edge)
	{
		UBMeshEdge* It = Edge;
		do
		{
			Edges.Add(It);
			It = It->Next(this);
		}
		while (It != Edge);
	}
	return Edges;
}

TArray<UBMeshFace*> UBMeshVertex::NeighborFaces() const
{
	TArray<UBMeshFace*> Faces;
	if (Edge)
	{
		const UBMeshEdge* It = Edge;
		do
		{
			for (auto Face : It->NeighborFacesRange())
			{
				Faces.AddUnique(Face);
			}
			It = It->Next(this);
		}
		while (It != Edge);
	}
	return Faces;
}

void UBMeshVertex::FIteratorBase::operator++()
{
	bFirst = false;
	Current = Current->Next(Owner);
}

UBMeshVertex* UBMeshVertex::FVertexIterator::operator*() const
{
	return Current->OtherVertex(Owner);
}

UBMeshEdge* UBMeshVertex::FEdgeIterator::operator*() const
{
	return Current;
}
