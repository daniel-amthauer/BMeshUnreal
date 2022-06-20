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

void FBMeshOperators::RegisterDefaultTypeInterpolators()
{
	RegisterNumericPropertyTypeInterpolator<FIntProperty>();
	RegisterNumericPropertyTypeInterpolator<FFloatProperty>();
	RegisterNumericPropertyTypeInterpolator<FDoubleProperty>();

	PropertyTypeLerps.Add(FStructProperty::StaticClass(), new FStructPropertyLerp());
	RegisterStructTypeInterpolator<FVector>();
	RegisterStructTypeInterpolator<FVector2D>();
	RegisterStructTypeInterpolator<FVector4>();
	RegisterStructTypeInterpolator<FLinearColor>();
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
	edgeCenters.SetNum(mesh->Edges.Num());
	TArray<UBMeshEdge*> originalEdges;
	originalEdges.SetNum(mesh->Edges.Num());
	for (UBMeshEdge* e : mesh->Edges)
	{
		edgeCenters[i] = mesh->AddVertex(e->Center());
		AttributeLerp(mesh, edgeCenters[i], e->Vert1, e->Vert2, 0.5f);
		originalEdges[i] = e;
		e->Id = i++;
	}

	TArray<UBMeshFace*> originalFaces = mesh->Faces; // copy because mesh.faces changes during iterations
	for (UBMeshFace* f : originalFaces)
	{
		UBMeshVertex* faceCenter = mesh->AddVertex(f->Center());
		float w = 0;

		// Create one quad per loop in the original face
		UBMeshLoop* it = f->FirstLoop;
		do
		{
			w += 1;
			AttributeLerp(mesh, faceCenter, faceCenter, it->Vert, 1 / w);

			UBMeshVertex* quad[] = {
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
	for (UBMeshEdge* e : originalEdges)
	{
		mesh->RemoveEdge(e);
	}
}

bool FBMeshOperators::Subdivide3(UBMesh* mesh)
{
	check(mesh);
	for (auto Face : mesh->Faces)
	{
		if (Face->VertCount != 3)
			return false;
	}

	int i = 0;
	TArray<UBMeshVertex*> edgeCenters;
	edgeCenters.SetNum(mesh->Edges.Num());
	TArray<UBMeshEdge*> originalEdges;
	originalEdges.SetNum(mesh->Edges.Num());
	for (UBMeshEdge* e : mesh->Edges)
	{
		edgeCenters[i] = mesh->AddVertex(e->Center());
		AttributeLerp(mesh, edgeCenters[i], e->Vert1, e->Vert2, 0.5f);
		originalEdges[i] = e;
		e->Id = i++;
	}

	TArray<UBMeshFace*> originalFaces = mesh->Faces; // copy because mesh.faces changes during iterations
	for (UBMeshFace* f : originalFaces)
	{
		//Center tri
		{
			UBMeshVertex* tri[] = {
				edgeCenters[f->FirstLoop->Edge->Id],
				edgeCenters[f->FirstLoop->Next->Edge->Id],
				edgeCenters[f->FirstLoop->Prev->Edge->Id]
			};
			mesh->AddFace(tri);
		}
		// Create one tri per loop in the original face
		UBMeshLoop* it = f->FirstLoop;
		do
		{
			UBMeshVertex* tri[] = {
				it->Vert,
				edgeCenters[it->Edge->Id],
				edgeCenters[it->Prev->Edge->Id]
			};
			mesh->AddFace(tri);
			it = it->Next;
		}
		while (it != f->FirstLoop);

		// then get rid of the original face
		mesh->RemoveFace(f);
	}

	// Remove old edges
	for (UBMeshEdge* e : originalEdges)
	{
		mesh->RemoveEdge(e);
	}
	
	return true;
}

bool FBMeshOperators::MergeFaces(UBMesh* Mesh, UBMeshEdge* Edge)
{
	auto Faces = Edge->NeighborFaces();
	if (Faces.Num() != 2)
	{
		return false;
	}

	TArray<UBMeshVertex*> Verts;
	{
		auto* First = Edge->Loop->Next;
		auto It = First;
		do
		{
			Verts.Add(It->Vert);
			It = It->Next;
		} while (It != First);
	}
	{
		auto* First = Edge->Loop->RadialNext;
		auto It = First->Next->Next;
		do
		{
			Verts.Add(It->Vert);
			It = It->Next;
		} while (It != First);
	}
	Mesh->AddFace(Verts);
	Mesh->RemoveEdge(Edge);
	return true;
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
	for (UBMeshFace* f : mesh->Faces)
	{
		FVector c = f->Center();
		int i = 0;
		// (r for "radius")
		FVector r[4];
		for (auto vert: f->Vertices())
		{
			if (i == 4)
				break;
			r[i++] = vert->Location - c;
		}
		if (i != 4) continue;

		FMatrix localToGlobal = ComputeLocalAxis(r[0], r[1], r[2], r[3]);
		FMatrix globalToLocal = localToGlobal.GetTransposed();

		// in local coordinates (l for "local")
		FVector l0 = globalToLocal.TransformVector(r[0]); //NOT SURE IF TransformVector or TransformPosition
		FVector l1 = globalToLocal.TransformVector(r[1]);
		FVector l2 = globalToLocal.TransformVector(r[2]);
		FVector l3 = globalToLocal.TransformVector(r[3]);

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
	pointUpdates.SetNum(mesh->Vertices.Num());
	TArray<double> weights;
	weights.SetNum(mesh->Vertices.Num());

	FStructProperty* RestposProperty = CastField<FStructProperty>(mesh->VertexClass->FindPropertyByName(FName("RestPos")));
	FProperty* WeightProperty = mesh->VertexClass->FindPropertyByName(FName("Weight"));
	auto* WeightPropFloat = CastField<FFloatProperty>(WeightProperty);
	auto WeightPropDouble = CastField<FDoubleProperty>(WeightProperty);

	{
		int i = 0;

		if (RestposProperty && RestposProperty->Struct == TBaseStructure<FVector>::Get())
		{
			if (WeightProperty)
			{
				if (WeightPropFloat)
				{
					for (UBMeshVertex* v :  mesh->Vertices)
					{
						weights[i] = *WeightProperty->ContainerPtrToValuePtr<float>(v);
						auto restpos = *RestposProperty->ContainerPtrToValuePtr<FVector>(v);
						pointUpdates[i] = (restpos - v->Location) * weights[i];
						v->Id = i++;
					}
				}
				else if (WeightPropDouble)
				{
					for (UBMeshVertex* v :  mesh->Vertices)
					{
						weights[i] = *WeightProperty->ContainerPtrToValuePtr<double>(v);
						auto restpos = *RestposProperty->ContainerPtrToValuePtr<FVector>(v);
						pointUpdates[i] = (restpos - v->Location) * weights[i];
						v->Id = i++;
					}
				}
			}
			else
			{
				for (UBMeshVertex* v : mesh->Vertices)
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
			for (UBMeshVertex* v : mesh->Vertices)
			{
				weights[i] = 0.0f;
				pointUpdates[i] = FVector::ZeroVector;
				v->Id = i++;
			}
		}
	}

	// Accumulate updates
	for (UBMeshFace* f : mesh->Faces)
	{
		FVector c = f->Center();
		int i = 0;
		// (r for "radius")
		FVector r[4];
		UBMeshVertex* verts[4];
		for (auto vert: f->Vertices())
		{
			if (i == 4)
				break;
			verts[i] = vert;
			r[i++] = vert->Location - c;
		}
		if (i != 4) continue;

		FMatrix localToGlobal = ComputeLocalAxis(r[0], r[1], r[2], r[3]);
		FMatrix globalToLocal = localToGlobal.GetTransposed();

		//:local coordinates (l for "local")
		FVector l0 = globalToLocal.TransformVector(r[0]); //not sure if TransformVector or TransformPosition
		FVector l1 = globalToLocal.TransformVector(r[1]);
		FVector l2 = globalToLocal.TransformVector(r[2]);
		FVector l3 = globalToLocal.TransformVector(r[3]);

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
		pointUpdates[verts[0]->Id] += t0 - r[0];
		pointUpdates[verts[1]->Id] += t1 - r[1];
		pointUpdates[verts[2]->Id] += t2 - r[2];
		pointUpdates[verts[3]->Id] += t3 - r[3];
		weights[verts[0]->Id] += 1;
		weights[verts[1]->Id] += 1;
		weights[verts[2]->Id] += 1;
		weights[verts[3]->Id] += 1;
	}

	// Apply updates
	int i = 0;
	for (UBMeshVertex* v : mesh->Vertices)
	{
		if (weights[i] > 0)
		{
			v->Location += pointUpdates[i] * (rate / weights[i]);
		}
		++i;
	}
	//ensure verts with 1.0 weight are fully constrained to their rest pos
	if (RestposProperty && WeightPropDouble)
	{
		for (UBMeshVertex* v : mesh->Vertices)
		{
			if (*WeightPropDouble->ContainerPtrToValuePtr<double>(v) == 1.0)
			{
				v->Location = *RestposProperty->ContainerPtrToValuePtr<FVector>(v);
			}
		}
	}
}

void FBMeshOperators::SubdivideTriangleFan(TArrayView<UBMeshFace* const> Faces)
{
	for (auto* OriginalFace : Faces)
	{
		check(OriginalFace != nullptr);
		auto* Mesh = CastChecked<UBMesh>(OriginalFace->GetOuter());
		auto* Center = Mesh->AddVertex(OriginalFace->Center());
		auto Loop = OriginalFace->FirstLoop;
		do
		{
			Mesh->AddFace(Center, Loop->Vert, Loop->Next->Vert);
			Loop = Loop->Next;
		}
		while (Loop != OriginalFace->FirstLoop);
		Mesh->RemoveFace(OriginalFace);
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
		DrawLine(Start, Start + Dir, Color);
	};

	for (auto e : mesh->Edges)
	{
		DrawLine(e->Vert1->Location, e->Vert2->Location, FColor::Yellow);
	}
	for (auto l : mesh->Loops)
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
	for (auto f : mesh->Faces)
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
	for (UBMeshVertex* v : mesh->Vertices)
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
