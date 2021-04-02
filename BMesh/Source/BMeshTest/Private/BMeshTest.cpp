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


#include "BMeshTest.h"

#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"

#include "BMeshCore.h"
#include "BMeshOperators.h"

// Sets default values for this component's properties
UBMeshTestComponent::UBMeshTestComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBMeshTestComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UBMeshTestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FPrimitiveSceneProxy* UBMeshTestComponent::CreateSceneProxy()
{
	/** Represents a UBMeshTestComponent to the scene manager. */
	class FBMeshTestComponentSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FBMeshTestComponentSceneProxy(const UBMeshTestComponent* InComponent)
			:	FPrimitiveSceneProxy(InComponent)
			, BMesh(InComponent->TestBMesh)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER( STAT_BoxSceneProxy_GetDynamicMeshElements );

			const FMatrix& LocalToWorld = GetLocalToWorld();
			
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(FColor::Yellow, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected() );

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

					if (BMesh)
					{
						FBMeshOperators::DrawPrimitives(PDI, FTransform(LocalToWorld), BMesh);

						FStructProperty* ColorProperty = CastField<FStructProperty>(BMesh->VertexClass->FindPropertyByName(FName("Color")));
						if (ColorProperty && ColorProperty->Struct == TBaseStructure<FLinearColor>::Get())
						{
							for (UBMeshVertex* Vert : BMesh->Vertices)
							{
								FLinearColor* VertColorPtr = ColorProperty->ContainerPtrToValuePtr<FLinearColor>(Vert);
								PDI->DrawPoint(LocalToWorld.TransformPosition(Vert->Location), *VertColorPtr, 10.0f, SDPG_World);
							}
						}
					}
					
					//DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis( EAxis::X ), LocalToWorld.GetScaledAxis( EAxis::Y ), LocalToWorld.GetScaledAxis( EAxis::Z ), FVector(100.0f, 100.0f, 100.f), DrawColor, SDPG_World, 1.0f);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint( void ) const override { return( sizeof( *this ) + GetAllocatedSize() ); }
		uint32 GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

	private:
		UBMesh* BMesh;
	};
	return new FBMeshTestComponentSceneProxy( this );
}

void UBMeshTestComponent::HexagonTest()
{
	TestBMesh = UBMesh::Make(this);

	for (int i = 0; i < 6; ++i)
	{
		FVector Vec = FVector(1,0,0);
		Vec = Vec.RotateAngleAxis(60*i, FVector(0,1,0));
		TestBMesh->AddVertex(Vec);
	}
	TestBMesh->AddFace(TestBMesh->Vertices);
	
	MarkRenderStateDirty();
}

void UBMeshTestComponent::Test1()
{
	TestBMesh = UBMesh::Make(this);

	UBMeshVertex* v0 = TestBMesh->AddVertex(FVector(-0.5f, 0.0f, -FMath::Sqrt(3) / 6));
    UBMeshVertex* v1 = TestBMesh->AddVertex(FVector(0.5f, 0.0f, -FMath::Sqrt(3) / 6));
    UBMeshVertex* v2 = TestBMesh->AddVertex(FVector(0, 0.0f, FMath::Sqrt(3) / 3));
    UBMeshFace* f = TestBMesh->AddFace(v0, v1, v2);

    ensureMsgf(TestBMesh->Vertices.Num() == 3, TEXT("vert count"));
    ensureMsgf(TestBMesh->Loops.Num() == 3, TEXT("loop count"));
    ensureMsgf(TestBMesh->Edges.Num() == 3, TEXT("edge count"));
    ensureMsgf(TestBMesh->Faces.Num() == 1, TEXT("face count"));

    UBMeshLoop* l = TestBMesh->Loops[0];
    for (int i = 0; i < 3; ++i)
    {
        auto v = TestBMesh->Vertices[i];
        ensureMsgf(TestBMesh->Loops[i]->Face == f, TEXT("loop has face"));
        ensureMsgf(TestBMesh->Loops[i]->Edge != nullptr, TEXT("loop has edge"));
        ensureMsgf(TestBMesh->Edges[i]->Loop != nullptr, TEXT("edge has loop"));
        ensureMsgf(v->Edge != nullptr, TEXT("vertex has edge"));
        ensureMsgf(v->Edge->Vert1 == v || v->Edge->Vert2 == v, TEXT("vertex is in vertex edge"));
        ensureMsgf(l->Next != l, TEXT("loop has next"));
        ensureMsgf(l->Next->Prev == l, TEXT("loop has consistent next"));
        ensureMsgf(l->RadialNext->RadialPrev == l, TEXT("loop has consistent radial next"));
        l = l->Next;
    }
    ensureMsgf(l == TestBMesh->Loops[0], TEXT("loop loops"));

    ensureMsgf(TestBMesh->FindEdge(v0, v1) != nullptr, TEXT("edge between v0 and v1"));
    ensureMsgf(TestBMesh->FindEdge(v0, v2) != nullptr, TEXT("edge between v0 and v2"));
    ensureMsgf(TestBMesh->FindEdge(v2, v1) != nullptr, TEXT("edge between v2 and v1"));

    UE_LOG(LogTemp, Log, TEXT("TestBMesh #1 passed."));

	MarkRenderStateDirty();
}

void UBMeshTestComponent::Test2()
{
	TestBMesh = UBMesh::Make(this);

	UBMeshVertex* v0 = TestBMesh->AddVertex(FVector(-1, 0, -1));
    UBMeshVertex* v1 = TestBMesh->AddVertex(FVector(-1, 0, 1));
    UBMeshVertex* v2 = TestBMesh->AddVertex(FVector(1, 0, 1));
	UBMeshVertex* v3 = TestBMesh->AddVertex(FVector(1, 0, -1));
    UBMeshFace* f = TestBMesh->AddFace(v0, v1, v2, v3);

    ensureMsgf(TestBMesh->Vertices.Num() == 4, TEXT("vert count"));
    ensureMsgf(TestBMesh->Loops.Num() == 4, TEXT("loop count"));
    ensureMsgf(TestBMesh->Edges.Num() == 4, TEXT("edge count"));
    ensureMsgf(TestBMesh->Faces.Num() == 1, TEXT("face count"));

    // Edges
    UBMeshEdge* e0 = TestBMesh->FindEdge(v0, v1);
    UBMeshEdge* e1 = TestBMesh->FindEdge(v1, v2);
    UBMeshEdge* e2 = TestBMesh->FindEdge(v2, v3);
    UBMeshEdge* e3 = TestBMesh->FindEdge(v3, v0);
    ensureMsgf(e0 != nullptr, TEXT("found edge v0->v1"));
    ensureMsgf(e1 != nullptr, TEXT("found edge v1->v2"));
    ensureMsgf(e2 != nullptr, TEXT("found edge v2->v3"));
    ensureMsgf(e3 != nullptr, TEXT("found edge v3->v0"));

    FVector expected;
    expected = FVector(-1, 0, 0);
    ensureMsgf(FVector::Dist(expected, e0->Center()) < FLT_EPSILON, TEXT("edge 0 center"));
    expected = FVector(0, 0, 1);
    ensureMsgf(FVector::Dist(expected, e1->Center()) < FLT_EPSILON, TEXT("edge 1 center"));
    expected = FVector(1, 0, 0);
    ensureMsgf(FVector::Dist(expected, e2->Center()) < FLT_EPSILON, TEXT("edge 2 center"));
    expected = FVector(0, 0, -1);
    ensureMsgf(FVector::Dist(expected, e3->Center()) < FLT_EPSILON, TEXT("edge 3 center"));

    // face
    expected = FVector(0, 0, 0);
    ensureMsgf(FVector::Dist(expected, f->Center()) < FLT_EPSILON, TEXT("face center"));

    // loop consistency
    v0->Id = 0; v1->Id = 1; v2->Id = 2; v3->Id = 3;
    UBMeshLoop* l = v0->Edge->Loop;
    UBMeshLoop* it = l;
    int prevId = it->Prev->Vert->Id;
    int forward = (prevId + 1) % 4 == it->Vert->Id ? 1 : 0;
    do
    {
        ensureMsgf((forward == 1 && (prevId + 1) % 4 == it->Vert->Id) || (it->Vert->Id + 1) % 4 == prevId, TEXT("valid quad loop order"));
        prevId = it->Vert->Id;
        it = it->Next;
    } while (it != l);

    for (int i = 0; i < 4; ++i)
    {
        auto v = TestBMesh->Vertices[i];
        ensure(TestBMesh->Loops[i]->Face == f);
        ensure(v->Edge != nullptr);
        ensure(v->Edge->Vert1 == v || v->Edge->Vert2 == v);
    }

    ensureMsgf(TestBMesh->FindEdge(v0, v1) != nullptr, TEXT("edge between v0 and v1"));

    TestBMesh->RemoveEdge(TestBMesh->Edges[0]);
    ensureMsgf(TestBMesh->Vertices.Num() == 4, TEXT("vert count after removing edge"));
    ensureMsgf(TestBMesh->Loops.Num() == 0, TEXT("loop count after removing edge"));
    ensureMsgf(TestBMesh->Edges.Num() == 3, TEXT("edge count after removing edge"));
    ensureMsgf(TestBMesh->Faces.Num() == 0, TEXT("face count after removing edge"));

    UE_LOG(LogTemp, Log, TEXT("TestBMesh #2 passed."));

	MarkRenderStateDirty();
}

void UBMeshTestComponent::Test3()
{
	TestBMesh = UBMesh::Make(this);

	UBMeshVertex* v0 = TestBMesh->AddVertex(FVector(-1, 0, -1));
    UBMeshVertex* v1 = TestBMesh->AddVertex(FVector(-1, 0, 1));
    UBMeshVertex* v2 = TestBMesh->AddVertex(FVector(1, 0, 1));
	UBMeshVertex* v3 = TestBMesh->AddVertex(FVector(1, 0, -1));
    UBMeshFace* f0 = TestBMesh->AddFace(v0, v1, v2);
	UBMeshFace* f1 = TestBMesh->AddFace(v2, v1, v3);

	ensureMsgf(TestBMesh->Vertices.Num() == 4, TEXT("vert count"));
    ensureMsgf(TestBMesh->Loops.Num() == 6, TEXT("loop count"));
    ensureMsgf(TestBMesh->Edges.Num() == 5, TEXT("edge count"));
    ensureMsgf(TestBMesh->Faces.Num() == 2, TEXT("face count"));

	ensureMsgf(v0->NeighborFaces().Num() == 1, TEXT("v0 has one neighbor face (found count: %d)"), v0->NeighborFaces().Num());
	ensureMsgf(v1->NeighborFaces().Num() == 2, TEXT("v1 has two neighbor face (found count: %d)"), v1->NeighborFaces().Num());

	for (UBMeshLoop* l : TestBMesh->Loops)
	{
		ensureMsgf(l->Next != nullptr, TEXT("loop has a next loop"));
		ensureMsgf(l->Prev != nullptr, TEXT("loop has a prev loop"));
	}

	ensureMsgf(f0->FindLoop(v0) != nullptr, TEXT("loop with vertex v0 does not exist in face f0"));
	ensureMsgf(f0->FindLoop(v0)->Vert != nullptr, TEXT("loop with vertex v0 has v0 as corner"));
	ensureMsgf(f0->FindLoop(v1) != nullptr, TEXT("loop with vertex v1 does not exist in face f0"));
	ensureMsgf(f0->FindLoop(v1)->Vert != nullptr, TEXT("loop with vertex v1 has v1 as corner"));
	ensureMsgf(f0->FindLoop(v3) == nullptr, TEXT("loop with vertex v3 should not exist in face f0"));

	UBMeshEdge* e0 = nullptr;
	for (UBMeshEdge* e : TestBMesh->Edges)
	{
		if ((e->Vert1 == v1 && e->Vert2 == v2) || (e->Vert1 == v2 && e->Vert2 == v1))
		{
			e0 = e;
			break;
		}
	}

	ensureMsgf(e0 != nullptr, TEXT("found edge between v1 and v2"));
	TestBMesh->RemoveEdge(e0);
	ensureMsgf(TestBMesh->Vertices.Num() == 4, TEXT("vert count after removing edge"));
	ensureMsgf(TestBMesh->Loops.Num() == 0, TEXT("loop count after removing edge"));
	ensureMsgf(TestBMesh->Edges.Num() == 4, TEXT("edge count after removing edge"));
	ensureMsgf(TestBMesh->Faces.Num() == 0, TEXT("face count after removing edge"));

    for (UBMeshLoop* l : TestBMesh->Loops)
	{
		ensureMsgf(l->Next != nullptr, TEXT("loop still has a next loop"));
		ensureMsgf(l->Prev != nullptr, TEXT("loop still has a prev loop"));
	}

    UE_LOG(LogTemp, Log, TEXT("TestBMesh #3 passed."));

	MarkRenderStateDirty();
}

void UBMeshTestComponent::CustomClassLerpTest()
{
	UBMesh::FMakeParams Params;
	Params.VertexClass = UBMeshVertex_Test::StaticClass();
	TestBMesh = UBMesh::Make(this, Params);

	UBMeshVertex_Test* v0 = Cast<UBMeshVertex_Test>(TestBMesh->AddVertex(FVector(-1, 0, -1)));
    UBMeshVertex_Test* v1 = Cast<UBMeshVertex_Test>(TestBMesh->AddVertex(FVector(-1, 0, 1)));
    UBMeshVertex_Test* v2 = Cast<UBMeshVertex_Test>(TestBMesh->AddVertex(FVector(1, 0, 1)));
	UBMeshVertex_Test* v3 = Cast<UBMeshVertex_Test>(TestBMesh->AddVertex(FVector(1, 0, -1)));

	v0->Color = FLinearColor::Red;
	v1->Color = FLinearColor::Green;

	FBMeshOperators::AttributeLerp(TestBMesh, v2, v0, v1, 0.5f);
	ensureMsgf(v2->Color == FMath::Lerp(v0->Color, v1->Color, 0.5f), TEXT("Color attribute is properly interpolated"));

	UBMeshFace* f0 = TestBMesh->AddFace(v0, v1, v2, v3);

	MarkRenderStateDirty();
}

FBoxSphereBounds UBMeshTestComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox BoundingBox(ForceInit);

	// Bounds are tighter if the box is generated from pre-transformed Vertices.

	if (TestBMesh)
	{
		for (int32 Index = 0; Index < TestBMesh->Vertices.Num(); ++Index)
		{
			BoundingBox += LocalToWorld.TransformPosition(TestBMesh->Vertices[Index]->Location);
		}
	}

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = BoundingBox.GetExtent();
	NewBounds.Origin = BoundingBox.GetCenter();
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();

	return NewBounds;
}
