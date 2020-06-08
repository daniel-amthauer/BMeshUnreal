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

#include "BMeshOperators.h"

#include "DrawDebugHelpers.h"

#include "BMesh.h"
#include "BMeshVertex.h"
#include "BMeshEdge.h"
#include "BMeshLoop.h"
#include "BMeshFace.h"

TMap<FFieldClass*, FBMeshOperators::FPropertyLerp*> FBMeshOperators::PropertyTypeLerps;
TMap<UScriptStruct*, FBMeshOperators::FPropertyLerp*> FBMeshOperators::StructTypeLerps;

FBMeshOperators::FPropertyLerp::~FPropertyLerp()
{
	
}

void FBMeshOperators::FStructPropertyLerp::Lerp(FProperty* Property, UBMeshVertex* destination, UBMeshVertex* v1,
                                                UBMeshVertex* v2, float t)
{
	FStructProperty* TypedProperty = static_cast<FStructProperty*>(Property);
	if (FPropertyLerp* SpecificLerp = StructTypeLerps.FindRef(TypedProperty->Struct))
	{
		SpecificLerp->Lerp(Property, destination, v1, v2, t);
	}
}

void FBMeshOperators::RegisterDefaultTypes()
{
	RegisterNumericPropertyType<FIntProperty>();
	RegisterNumericPropertyType<FFloatProperty>();

	PropertyTypeLerps.Add(FStructProperty::StaticClass(), new FStructPropertyLerp());
	RegisterStructType<FVector>();
	RegisterStructType<FVector2D>();
	RegisterStructType<FVector4>();
	RegisterStructType<FLinearColor>();
}

void FBMeshOperators::AttributeLerp(UBMesh* mesh, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2,
                                    float t)
{
	check(v1 && v2 && v1->GetClass() == v2->GetClass());
	for (TFieldIterator<FProperty> PropertyIt(*mesh->VertexClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		if ((*PropertyIt)->GetOwnerClass() == UBMeshVertex::StaticClass())
			continue;
		if (FPropertyLerp* PropertyLerp = PropertyTypeLerps.FindRef((*PropertyIt)->GetClass()))
		{
			PropertyLerp->Lerp(*PropertyIt, destination, v1, v2, t);
		}
	}
}

void FBMeshOperators::Subdivide(UBMesh* mesh)
{
	int i = 0;
	TArray<UBMeshVertex*> edgeCenters;
	edgeCenters.SetNum(mesh->edges.Num());
	TArray<UBMeshEdge*> originalEdges;
	originalEdges.SetNum(mesh->edges.Num());
	for(UBMeshEdge* e : mesh->edges)
	{
		edgeCenters[i] = mesh->AddVertex(e->Center());
		AttributeLerp(mesh, edgeCenters[i], e->Vert1, e->Vert2, 0.5f);
		originalEdges[i] = e;
		e->Id = i++;
	}

	TArray<UBMeshFace*> originalFaces = mesh->faces; // copy because mesh.faces changes during iterations
	for(UBMeshFace* f : originalFaces)
	{
		UBMeshVertex* faceCenter = mesh->AddVertex(f->Center());
		float w = 0;

		// Create one quad per loop in the original face
		UBMeshLoop* it = f->FirstLoop;
		do
		{
			w += 1;
			AttributeLerp(mesh, faceCenter, faceCenter, it->Vert, 1 / w);

			UBMeshVertex* quad [] = {
				it->Vert,
				edgeCenters[it->Edge->Id],
				faceCenter,
				edgeCenters[it->Prev->Edge->Id]
			};
			mesh->AddFace(quad);
			it = it->Next;
		}
		while (it != f->FirstLoop);

		// then get rid of the original face
		mesh->RemoveFace(f);
	}

	// Remove old edges
	for(UBMeshEdge* e : originalEdges)
	{
		mesh->RemoveEdge(e);
	}
}

FMatrix FBMeshOperators::ComputeLocalAxis(FVector r0, FVector r1, FVector r2, FVector r3)
{
    FVector Z = (
              FVector::CrossProduct(r0, r1).GetSafeNormal()
            + FVector::CrossProduct(r1, r2).GetSafeNormal()
            + FVector::CrossProduct(r2, r3).GetSafeNormal()
            + FVector::CrossProduct(r3, r0).GetSafeNormal()
        ).GetSafeNormal();
    FVector X = r0.GetSafeNormal();
    FVector Y = FVector::CrossProduct(Z, X);
    FMatrix localToGlobal = FMatrix(X, Y, Z, FVector::ZeroVector);
    return localToGlobal;
}

float FBMeshOperators::AverageRadiusLength(UBMesh* mesh)
{
	float lengthsum = 0;
	float weightsum = 0;
	for(UBMeshFace* f : mesh->faces)
	{
		FVector c = f->Center();
		TArray<UBMeshVertex*> verts = f->NeighborVertices();
		if (verts.Num() != 4) continue;
		// (r for "radius")
		FVector r0 = verts[0]->Location - c;
		FVector r1 = verts[1]->Location - c;
		FVector r2 = verts[2]->Location - c;
		FVector r3 = verts[3]->Location - c;

		FMatrix localToGlobal = ComputeLocalAxis(r0, r1, r2, r3);
		FMatrix globalToLocal = localToGlobal.GetTransposed();

		// in local coordinates (l for "local")
		FVector l0 = globalToLocal.TransformVector(r0); //NOT SURE IF TransformVector or TransformPosition
		FVector l1 = globalToLocal.TransformVector(r1);
		FVector l2 = globalToLocal.TransformVector(r2);
		FVector l3 = globalToLocal.TransformVector(r3);

		// Rotate vectors (rl for "rotated local")
		FVector rl0 = l0;
		FVector rl1 = FVector(l1.Y, -l1.X, l1.Z);
		FVector rl2 = FVector(-l2.X, -l2.Y, l2.Z);
		FVector rl3 = FVector(-l3.Y, l3.X, l3.Z);

		FVector average = (rl0 + rl1 + rl2 + rl3) / 4;

		lengthsum += average.Size();
		weightsum += 1;
	}
	return lengthsum / weightsum;
}

void FBMeshOperators::SquarifyQuads(UBMesh* mesh, float rate, bool uniformLength)
{
	float avg = 0;
	if (uniformLength)
	{
		avg = AverageRadiusLength(mesh);
	}

	TArray<FVector> pointUpdates;
	pointUpdates.SetNum(mesh->vertices.Num());
	TArray<float> weights;
	weights.SetNum(mesh->vertices.Num());

	FStructProperty* RestposProperty = CastField<FStructProperty>(mesh->VertexClass->FindPropertyByName(FName("RestPos")));
	FFloatProperty* WeightProperty = CastField<FFloatProperty>(mesh->VertexClass->FindPropertyByName(FName("Weight")));

	int i = 0;

	if (RestposProperty && RestposProperty->Struct == TBaseStructure<FVector>::Get())
	{
		if (WeightProperty)
		{
			for(UBMeshVertex* v : mesh->vertices)
			{
				weights[i] = *WeightProperty->ContainerPtrToValuePtr<float>(v);
				auto restpos = *RestposProperty->ContainerPtrToValuePtr<FVector>(v);
				pointUpdates[i] = (restpos - v->Location) * weights[i];
				v->Id = i++;
			}
		}
		else
		{
			for(UBMeshVertex* v : mesh->vertices)
			{
				weights[i] = 1;
				auto restpos = *RestposProperty->ContainerPtrToValuePtr<FVector>(v);
				pointUpdates[i] = (restpos - v->Location) * weights[i];
				v->Id = i++;
			}
		}
	}
	else
	{
		for(UBMeshVertex* v : mesh->vertices)
		{
			weights[i] = 0.0f;
			pointUpdates[i] = FVector::ZeroVector;
			v->Id = i++;
		}
	}

	// Accumulate updates
	for(UBMeshFace* f : mesh->faces)
	{
		FVector c = f->Center();
		TArray<UBMeshVertex*> verts = f->NeighborVertices();
		if (verts.Num() != 4) continue;
		// (r for "radius")
		FVector r0 = verts[0]->Location - c;
		FVector r1 = verts[1]->Location - c;
		FVector r2 = verts[2]->Location - c;
		FVector r3 = verts[3]->Location - c;

		FMatrix localToGlobal = ComputeLocalAxis(r0, r1, r2, r3);
		FMatrix globalToLocal = localToGlobal.GetTransposed();

		//:local coordinates (l for "local")
		FVector l0 = globalToLocal.TransformVector(r0); //not sure if TransformVector or TransformPosition
		FVector l1 = globalToLocal.TransformVector(r1);
		FVector l2 = globalToLocal.TransformVector(r2);
		FVector l3 = globalToLocal.TransformVector(r3);

		bool switch03 = false;
		if (l1.GetSafeNormal().Y < l3.GetSafeNormal().Y)
		{
			switch03 = true;
			auto tmp = l3;
			l3 = l1;
			l1 = tmp;
		}
		// now 0->1->2->3 is:direct trigonometric order

		// Rotate vectors (rl for "rotated local")
		FVector rl0 = l0;
		FVector rl1 = FVector(l1.Y, -l1.X, l1.Z);
		FVector rl2 = FVector(-l2.X, -l2.Y, l2.Z);
		FVector rl3 = FVector(-l3.Y, l3.X, l3.Z);

		FVector average = (rl0 + rl1 + rl2 + rl3) / 4;
		if (uniformLength)
		{
			average = average.GetSafeNormal() * avg;
		}

		// Rotate back (lt for "local target")
		FVector lt0 = average;
		FVector lt1 = FVector(-average.Y, average.X, average.Z);
		FVector lt2 = FVector(-average.X, -average.Y, average.Z);
		FVector lt3 = FVector(average.Y, -average.X, average.Z);

		// Switch back
		if (switch03)
		{
			auto tmp = lt3;
			lt3 = lt1;
			lt1 = tmp;
		}

		// Back to global (t for "target")
		FVector t0 = localToGlobal.TransformVector(lt0);
		FVector t1 = localToGlobal.TransformVector(lt1);
		FVector t2 = localToGlobal.TransformVector(lt2);
		FVector t3 = localToGlobal.TransformVector(lt3);

		// Accumulate
		pointUpdates[verts[0]->Id] += t0 - r0;
		pointUpdates[verts[1]->Id] += t1 - r1;
		pointUpdates[verts[2]->Id] += t2 - r2;
		pointUpdates[verts[3]->Id] += t3 - r3;
		weights[verts[0]->Id] += 1;
		weights[verts[1]->Id] += 1;
		weights[verts[2]->Id] += 1;
		weights[verts[3]->Id] += 1;
	}

	// Apply updates
	i = 0;
	for(UBMeshVertex* v:mesh->vertices)
	{
		if (weights[i] > 0)
		{
			v->Location += pointUpdates[i] * (rate / weights[i]);
		}
		++i;
	}
}

void FBMeshOperators::DrawPrimitives(FPrimitiveDrawInterface* PDI, FTransform LocalToWorld, UBMesh* mesh)
{
	auto DrawLine = [=](FVector A, FVector B, FColor Color)
	{
		PDI->DrawLine(LocalToWorld.TransformPosition(A), LocalToWorld.TransformPosition(B), Color, SDPG_World);
	};
	DrawPrimitives(DrawLine, mesh);
}

void FBMeshOperators::DrawPrimitives(UWorld* World, FTransform LocalToWorld, UBMesh* mesh)
{
	auto DrawLine = [=](FVector A, FVector B, FColor Color)
	{
		::DrawDebugLine(World, LocalToWorld.TransformPosition(A), LocalToWorld.TransformPosition(B), Color);
	};
	DrawPrimitives(DrawLine, mesh);
}

void FBMeshOperators::DrawPrimitives(TFunction<void(FVector, FVector, FColor)> DrawLine, UBMesh* mesh)
{
	auto DrawRay = [=](FVector Start, FVector Dir, FColor Color)
	{
		DrawLine(Start, Start+Dir, Color);
	};
	
	for(auto e : mesh->edges)
	{
		DrawLine(e->Vert1->Location, e->Vert2->Location, FColor::Yellow);
	}
	for(auto l : mesh->loops)
	{
		UBMeshVertex* Vert = l->Vert;
		UBMeshVertex* other = l->Edge->OtherVertex(Vert);
		DrawRay(Vert->Location, (other->Location - Vert->Location) * 0.1f, FColor::Red);

		UBMeshLoop* nl = l->Next;
		UBMeshVertex* nother = nl->Edge->ContainsVertex(Vert) ? nl->Edge->OtherVertex(Vert) : nl->Edge->OtherVertex(other);
		FVector no = Vert->Location + (other->Location - Vert->Location) * 0.1f;
		DrawRay(no, (nother->Location - no) * 0.1f, FColor::Red);
	}
	int i = 0;
	for(auto f : mesh->faces)
	{
		FVector c = f->Center();
		DrawLine(c, f->FirstLoop->Vert->Location, FColor::Green);
		DrawRay(c, (f->FirstLoop->Next->Vert->Location - c) * 0.2f, FColor::Green);
#if WITH_EDITOR
            //Handles->Label(c, "f" + i);
            ++i;
#endif // WITH_EDITOR
	}

	i = 0;
	for(UBMeshVertex* v : mesh->vertices)
	{
#if WITH_EDITOR
		//auto uv = v->attributes["uv"] as BMesh->FloatAttributeValue;
            //Handles->Label(v->Location, "" + i);
            ++i;
#endif // WITH_EDITOR
	}
}

//void FBMeshOperators::Merge(UBMesh* mesh, UBMesh* other)
//{
//	var newVerts = new Vertex[other.vertices.Count];
//	int i = 0;
//	foreach(Vertex v in other.vertices)
//	{
//		newVerts[i] = mesh.AddVertex(v.point);
//		AttributeLerp(mesh, newVerts[i], v, v, 1); // copy all attributes
//		v.id = i;
//		++i;
//	}
//	foreach(Edge e in other.edges)
//	{
//		mesh.AddEdge(newVerts[e.vert1.id], newVerts[e.vert2.id]);
//	}
//	foreach(Face f in other.faces)
//	{
//		var neighbors = f.NeighborVertices();
//		var newNeighbors = new Vertex[neighbors.Count];
//		int j = 0;
//		foreach(var v in neighbors)
//		{
//			newNeighbors[j] = newVerts[v.id];
//			++j;
//		}
//		mesh.AddFace(newNeighbors);
//	}
//}
