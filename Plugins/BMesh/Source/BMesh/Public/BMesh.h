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

/**
 * Non-manifold boundary representation of a 3D mesh with arbitrary attributes.
 * This structure intends to make procedural mesh creation and arbitrary edits
 * as easy as possible while remaining efficient enough. See other comments
 * along the file to see how to use it.
 * 
 * This file only contains the data structure and basic operations such as
 * adding/removing elements. For more advanced operations, see BMeshOperators.
 * For operations related to Unity, like converting to UnityEngine.Mesh, see
 * the BMeshUnity class.
 * 
 * The basic structure is described in the paper:
 * 
 *     Gueorguieva, Stefka and Marcheix, Davi. 1994. "Non-manifold boundary
 *     representation for solid modeling."
 *     
 * We use the same terminology as Blender's dev documentation:
 *     https://wiki.blender.org/wiki/Source/Modeling/BMesh/Design
 *     
 * Arbitrary attributes can be attached topological entity, namely vertices,
 * edges, loops and faces. If you are used to Houdini's terminology, note that
 * what is called "vertex" here corresponds to Houdini's points, while what
 * Houdini calls "vertex" is close to BMesh's "loops".
 * 
 * NB: This class is not totally protected from misuse. We prefered fostering
 * ease of use over safety, so take care when you start feeling that you are
 * not fully understanding what you are doing, you'll likely mess with the
 * structure. For instance, do not add edges directly to the mesh.edges
 * list but use AddEdge, etc.
 * 
 */

#pragma once

#include "CoreMinimal.h"

#include "BMesh.generated.h"

class UBMeshVertex;
class UBMeshEdge;
class UBMeshLoop;
class UBMeshFace;

UCLASS()
class BMESH_API UBMesh : public UObject
{
	GENERATED_BODY()
    // Topological entities
public:
	UPROPERTY()
	TArray<UBMeshVertex*> vertices;
	UPROPERTY()
    TArray<UBMeshEdge*> edges;
	UPROPERTY()
    TArray<UBMeshLoop*> loops;
	UPROPERTY()
    TArray<UBMeshFace*> faces;

    UPROPERTY()
	TSubclassOf<UBMeshVertex> VertexClass;

	UPROPERTY()
	TSubclassOf<UBMeshEdge> EdgeClass;

	UPROPERTY()
	TSubclassOf<UBMeshLoop> LoopClass;

	UPROPERTY()
	TSubclassOf<UBMeshFace> FaceClass;

    ///////////////////////////////////////////////////////////////////////////
    //#region [Topology Methods]

    /**
     * Add a new vertex to the mesh.
     */
	UBMeshVertex* AddVertex(UBMeshVertex* vert);

    UBMeshVertex* AddVertex(FVector Location);

	UBMeshVertex* AddVertex(float x, float y, float z);

    /**
     * Add a new edge between two vertices. If there is already such edge,
     * return it without adding a new one.
     * If the vertices are not part of the mesh, the behavior is undefined.
     */
	UBMeshEdge* AddEdge(UBMeshVertex* vert1, UBMeshVertex* vert2);

    UBMeshEdge* AddEdge(int v1, int v2)
    {
        return AddEdge(vertices[v1], vertices[v2]);
    }

    /**
     * Add a new face that connects the array of vertices provided.
     * The vertices must be part of the mesh, otherwise the behavior is
     * undefined.
     * NB: There is no AddLoop, because a loop is an element of a face
     */
	UBMeshFace* AddFace(TArrayView<UBMeshVertex*> fVerts);

    inline UBMeshFace* AddFace(UBMeshVertex* v0, UBMeshVertex* v1)
    {
    	UBMeshVertex* Verts[] = {v0, v1};
        return AddFace(Verts);
    }

    inline UBMeshFace* AddFace(UBMeshVertex* v0, UBMeshVertex* v1, UBMeshVertex* v2)
    {
    	UBMeshVertex* Verts[] = {v0, v1, v2};
        return AddFace(Verts);
    }

    inline UBMeshFace* AddFace(UBMeshVertex* v0, UBMeshVertex* v1, UBMeshVertex* v2, UBMeshVertex* v3)
    {
    	UBMeshVertex* Verts[] = {v0, v1, v2, v3};
        return AddFace(Verts);
    }

    inline UBMeshFace* AddFace(int i0, int i1)
    {
    	UBMeshVertex* Verts[] = { vertices[i0], vertices[i1] };
        return AddFace(Verts);
    }

    inline UBMeshFace* AddFace(int i0, int i1, int i2)
    {
    	UBMeshVertex* Verts[] = { vertices[i0], vertices[i1], vertices[i2] };
        return AddFace(Verts);
    }

    inline UBMeshFace* AddFace(int i0, int i1, int i2, int i3)
    {
    	UBMeshVertex* Verts[] = { vertices[i0], vertices[i1], vertices[i2], vertices[i3] };
        return AddFace(Verts);
    }

    /**
     * Return an edge that links vert1 to vert2 in the mesh (an arbitrary one
     * if there are several such edges, which is possible with this structure).
     * Return null if there is no edge between vert1 and vert2 in the mesh->
     */
	UBMeshEdge* FindEdge(UBMeshVertex* vert1, UBMeshVertex* vert2);

    /**
     * Remove the provided vertex from the mesh.
     * Removing a vertex also removes all the edges/loops/faces that use it.
     * If the vertex was not part of this mesh, the behavior is undefined.
     */
	void RemoveVertex(UBMeshVertex* v);

    /**
     * Remove the provided edge from the mesh.
     * Removing an edge also removes all associated loops/faces.
     * If the edge was not part of this mesh, the behavior is undefined.
     */
	void RemoveEdge(UBMeshEdge* e);

    /**
     * Removing a loop also removes associated face.
     * used internally only, just RemoveFace(loop.face) outside of here.
     */
	void RemoveLoop(UBMeshLoop* l);

    /**
     * Remove the provided face from the mesh.
     * If the face was not part of this mesh, the behavior is undefined.
     * (actually almost ensured to be a true mess, but do as it pleases you :D)
     */
	void RemoveFace(UBMeshFace* f);

    struct BMESH_API FMakeParams
    {
	    TSubclassOf<UBMeshVertex> VertexClass;
    	TSubclassOf<UBMeshEdge> EdgeClass;
    	TSubclassOf<UBMeshLoop> LoopClass;
    	TSubclassOf<UBMeshFace> FaceClass;

    	FMakeParams();
    };
	static UBMesh* Make(UObject* Outer = GetTransientPackage(), FMakeParams Params = FMakeParams());
};
