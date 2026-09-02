// Copyright Epic Games, Inc. All Rights Reserved.

#include "SphericalWorldActor.h"

#include "Materials/Material.h"
#include "ProceduralMeshComponent.h"

ASphericalWorldActor::ASphericalWorldActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	SetRootComponent(ProcMesh);

	BaseMaterial = TSoftObjectPtr<UMaterial>(
		FSoftObjectPath(TEXT("/Game/Spherical/M_Spherical.M_Spherical")));
}

void ASphericalWorldActor::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterial* Mat = BaseMaterial.LoadSynchronous())
	{
		ProcMesh->SetMaterial(0, Mat);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SphericalWorldActor: no material at '%s'. Create it or set BaseMaterial."), *BaseMaterial.ToString());
	}

	BuildGeometry();
}

void ASphericalWorldActor::BuildGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	const int32 Half = FMath::Max(1, GridSize) / 2;
	const float H = CubeSize * 0.5f;
	const float InnerRadius = FMath::Max(1.0f, Spacing * 0.9f);

	for (int32 IX = -Half; IX <= Half; ++IX)
	{
		for (int32 IY = -Half; IY <= Half; ++IY)
		{
			for (int32 IZ = -Half; IZ <= Half; ++IZ)
			{
				const FVector Center(IX * Spacing, IY * Spacing, IZ * Spacing);
				const float R = Center.Size();
				if (R > MaxRadius || R < InnerRadius)
				{
					continue;
				}

				// 8 corners.
				FVector C[8] =
				{
					Center + FVector(-H, -H, -H), // 0
					Center + FVector(+H, -H, -H), // 1
					Center + FVector(+H, +H, -H), // 2
					Center + FVector(-H, +H, -H), // 3
					Center + FVector(-H, -H, +H), // 4
					Center + FVector(+H, -H, +H), // 5
					Center + FVector(+H, +H, +H), // 6
					Center + FVector(-H, +H, +H), // 7
				};

				struct FFace { int32 I[4]; FVector N; };
				const FFace Faces[6] =
				{
					{ {0, 1, 2, 3}, FVector( 0.f,  0.f, -1.f) },
					{ {4, 5, 6, 7}, FVector( 0.f,  0.f,  1.f) },
					{ {0, 3, 7, 4}, FVector(-1.f,  0.f,  0.f) },
					{ {1, 2, 6, 5}, FVector( 1.f,  0.f,  0.f) },
					{ {0, 1, 5, 4}, FVector( 0.f, -1.f,  0.f) },
					{ {3, 2, 6, 7}, FVector( 0.f,  1.f,  0.f) },
				};

				const int32 Fi = IX + Half;
				const int32 Fj = IY + Half;
				const int32 Fk = IZ + Half;
				static const uint8 Levels[3] = { 64, 160, 255 };
				const FColor Color(Levels[Fi % 3], Levels[Fj % 3], Levels[Fk % 3]);

				for (const FFace& Face : Faces)
				{
					const int32 Base = Vertices.Num();
					for (int32 K = 0; K < 4; ++K)
					{
						Vertices.Add(C[Face.I[K]]);
						Normals.Add(Face.N);
						UVs.Add(FVector2D::ZeroVector);
						Colors.Add(Color);
					}
					Triangles.Add(Base + 0);
					Triangles.Add(Base + 1);
					Triangles.Add(Base + 2);
					Triangles.Add(Base + 0);
					Triangles.Add(Base + 2);
					Triangles.Add(Base + 3);
				}
			}
		}
	}

	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, /*bCreateCollision=*/false);
	ProcMesh->bUseComplexAsSimpleCollision = false;

	UE_LOG(LogTemp, Log, TEXT("SphericalWorldActor: built %d vertices (%d cubes)."), Vertices.Num(), Vertices.Num() / 24);
}
