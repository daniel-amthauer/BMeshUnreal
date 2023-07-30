/*
 * Copyright (c) 2020 -- Daniel Amthauer
 * 
 * Based on BMesh for Unity by �lie Michel (c) 2020, original copyright info included below
 * as specified by the original license terms. Those terms also apply to this version.
 */

/*
 * Copyright (c) 2020 -- �lie Michel <elie@exppad.com>
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
#include "BMeshFunctionLibrary.h"

#include "BMesh.h"
#include "BMeshOperators.h"
#include "BMeshLog.h"

void UBMeshFunctionLibrary::Subdivide(UBMesh* mesh)
{
	if (!mesh)
		return;
	FBMeshOperators::Subdivide(mesh);
}

bool UBMeshFunctionLibrary::Subdivide3(UBMesh* mesh)
{
	if (!mesh)
		return false;
	return FBMeshOperators::Subdivide3(mesh);
}

bool UBMeshFunctionLibrary::MergeFaces(UBMesh* mesh, UBMeshEdge* Edge)
{
	if (!mesh || !Edge)
		return false;
	return FBMeshOperators::MergeFaces(mesh, Edge);
}

void UBMeshFunctionLibrary::SquarifyQuads(UBMesh* mesh, float rate, bool uniformLength)
{
	if (!mesh)
		return;
	FBMeshOperators::SquarifyQuads(mesh, FBMeshOperators::FSquarifyQuadsParams{.Rate = rate, .bCalculateUniformLength = uniformLength, .bNormalIsUp = true});
}

void UBMeshFunctionLibrary::SubdivideTriangleFan(TArray<UBMeshFace*> Faces)
{
	for (const auto* Face : Faces)
	{
		if (Face == nullptr)
		{
			UE_LOG(LogBMesh, Error, TEXT("Invalid face, aborting"));
			return;
		}
	}
	FBMeshOperators::SubdivideTriangleFan(Faces);
}

void UBMeshFunctionLibrary::SubdivideTriangleFanAllFaces(UBMesh* mesh)
{
	if (mesh)
	{
		auto OriginalFaces = mesh->Faces;
		FBMeshOperators::SubdivideTriangleFan(OriginalFaces);
	}
}

void UBMeshFunctionLibrary::SubdivideTriangleFanSingle(UBMeshFace* Face)
{
	if (Face == nullptr)
	{
		UE_LOG(LogBMesh, Error, TEXT("Invalid face, aborting"));
		return;
	}
	FBMeshOperators::SubdivideTriangleFan({Face});
}

void UBMeshFunctionLibrary::DrawDebugBMesh(UObject* WorldContextObject, FTransform LocalToWorld, UBMesh* mesh)
{
	if (!mesh)
		return;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		FBMeshOperators::DrawPrimitives(World, LocalToWorld, mesh);
	}
}
