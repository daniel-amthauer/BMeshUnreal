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

#include "UObject/Field.h"

class UBMeshEdge;
class UBMesh;
class UBMeshVertex;

/**
 * BMesh Operators are static functions manipulating BMesh objects. Their first
 * argument is the input mesh, in which they are performing changes, so it is
 * also the output (changes are "in place"). If there are other inputs, like in
 * Merge, they are not affected by the operation.
 * 
 * Operators are free to override some attributes, their docstring precises the
 * attributes they expect and that they modify. It is the responsibility of
 * calling code to save previous values if they are needed.
 * All operators assume that the provided mesh is not null.
 * 
 * This file is rather limited at this point but intends to become a bank of
 * operators, to which you are more than welcome to contribute.
 *
 * [BMesh Unreal specific]
 * In order for operators to properly interpolate attributes, the attribute type must first be registered
 * By default the following types are registered:
 * -Int (only regular 32-bit)
 * -Float (only regular 32-bit)
 * -FVector
 * -FVector2D
 * -FVector4
 * -FLinearColor
 *
 * If your vertex class adds another property which you would also like interpolated, you should register its type using the
 * RegisterNumericPropertyType (in the case of TNumericPropertyType, such as FFloatProperty)
 *
 * or RegisterStructType for any USTRUCT that can be passed as an argument to FMath::Lerp
 */
class BMESH_API FBMeshOperators
{
private:

	class FPropertyLerp
	{
	public:
		virtual void Lerp(FProperty* Property, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2, float t) = 0;
		virtual ~FPropertyLerp();
	};

	class FStructPropertyLerp : public FPropertyLerp
	{
	public:
		void Lerp(FProperty* Property, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2, float t) override;
	};

	template <typename T>
	class TNumericPropertyLerp : public FPropertyLerp
	{
	public:
		void Lerp(FProperty* Property, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2, float t) override
		{
			T* TypedProperty = static_cast<T*>(Property);
			for (int i = 0; i < TypedProperty->ArrayDim; ++i)
			{
				T::TCppType val1 = TypedProperty->GetPropertyValue_InContainer(v1, i);
				T::TCppType val2 = TypedProperty->GetPropertyValue_InContainer(v2, i);
				T::TCppType result = FMath::Lerp(val1, val2, t);
				TypedProperty->SetPropertyValue_InContainer(destination, result, i);
			}
		}
	};

	template <typename StructType>
	class TSpecificStructPropertyLerp : public FPropertyLerp
	{
		void Lerp(FProperty* Property, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2, float t) override
		{
			FStructProperty* TypedProperty = static_cast<FStructProperty*>(Property);
			for (int i = 0; i < TypedProperty->ArrayDim; ++i)
			{
				StructType* val1 = TypedProperty->ContainerPtrToValuePtr<StructType>(v1, i);
				StructType* val2 = TypedProperty->ContainerPtrToValuePtr<StructType>(v2, i);
				StructType* result = TypedProperty->ContainerPtrToValuePtr<StructType>(destination, i);
				*result = FMath::Lerp(*val1, *val2, t);
			}
		}
	};

	static TMap<FFieldClass*, FPropertyLerp*> PropertyTypeLerps;

	static TMap<UScriptStruct*, FPropertyLerp*> StructTypeLerps;

public:

	template <typename NumericPropertyType>
	static void RegisterNumericPropertyType()
	{
		FFieldClass* FieldClass = NumericPropertyType::StaticClass();
		check(!PropertyTypeLerps.Contains(FieldClass));
		PropertyTypeLerps.Add(FieldClass, new TNumericPropertyLerp<NumericPropertyType>());
	}

	template <typename StructType>
	static void RegisterStructType()
	{
		UScriptStruct* ScriptStruct = TBaseStructure<StructType>::Get();
		check(!StructTypeLerps.Contains(ScriptStruct));
		StructTypeLerps.Add(ScriptStruct, new TSpecificStructPropertyLerp<StructType>());
	}

	static void RegisterDefaultTypes();
	/**
	 * Set all attributes in destination vertex to attr[v1] * (1 - t) + attr[v2] * t
	 * Overriding attributes: all in vertex 'destination', none in others.
	 */
	static void AttributeLerp(UBMesh* mesh, UBMeshVertex* destination, UBMeshVertex* v1, UBMeshVertex* v2, float t);

	/**
	 * Subdivide a mesh, without smoothing it, trying to interpolate all
	 * available attributes as much as possible. After subdivision, all faces
	 * are quads.
	 * Overriding attributes: edge's id
	 */
	static void Subdivide(UBMesh* mesh);

	/**
	 * Subdivide triangular faces
	 * Only works on meshes that only have have triangular faces
	 * @retval whether the mesh was subdivided correctly or not
	 */
	static bool Subdivide3(UBMesh* Mesh);

	/**
	 * Merge two faces separated by an edge
	 */
	static bool MergeFaces(UBMesh* Mesh, UBMeshEdge* Edge);

	///////////////////////////////////////////////////////////////////////////
	// [SquarifyQuads}

	/**
	 * Axis local to a quad face where r0, ..., r3 are vectors from the face
	 * center to its vertices.
	 */
	static FMatrix ComputeLocalAxis(FVector r0, FVector r1, FVector r2, FVector r3);

	static float AverageRadiusLength(UBMesh* mesh);

	/**
	 * Try to make quads as square as possible (may be called iteratively).
	 * This is not a very common operation but was developed so I keep it here.
	 * This assumes that the mesh is only made of quads.
	 * Overriding attributes: vertex's id
	 * Optionally read vertex attributes:
	 *   - RestPos: a FVector telling which position attracts the vertex
	 *   - Weight: a float telling to which extent the RestPos must be
	 *             considered.
	 *   
	 * @param rate speed at which faces are squarified. A higher rate goes
	 *        faster but there is a risk for overshooting.
	 * @param uniformLength whether the size of the quads must be uniformized.
	 */
	static void SquarifyQuads(UBMesh* mesh, float rate = 1.0f, bool uniformLength = false);

	//#endregion

	///////////////////////////////////////////////////////////////////////////
	// [Merge]

	/**
	 * Add all vertices/edges/faces from another mesh, and fix attributes if
	 * needed.
	 * Overriding attributes: vertex's id (of the first mesh only)
	 */
	static void Merge(UBMesh* mesh, UBMesh* other);

	///////////////////////////////////////////////////////////////////////////
	///

	/**
     * Draw details about the BMesh structure un the viewport.
     * To be used inside of OnDrawGizmozs() in a MonoBehavior script.
     * You'll most likely need to add beforehand:
     *     Gizmos.matrix = transform.localToWorldMatrix
     */
	static void DrawPrimitives(FPrimitiveDrawInterface* PDI, FTransform LocalToWorld, UBMesh* mesh);

	static void DrawPrimitives(UWorld* World, FTransform LocalToWorld, UBMesh* mesh);

	static void DrawPrimitives(TFunction<void(FVector, FVector, FColor)> DrawLine, UBMesh* mesh);
};
